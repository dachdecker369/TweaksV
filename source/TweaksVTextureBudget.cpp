// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the texture budget, the Resources part (see TweaksVTextureBudget.h).
//
// The game counts the video memory of the textures and buffers its streaming engine places in one object of its data, the
// counters: the buffers' and the textures' bytes and numbers, the most bytes of both so far (its own high-water mark), the
// most objects allowed and four budgets in bytes, the last two written once at the start-up from gameconfig.xml's
// ConfigGraphics - MaxStreamingResources, 98304 in the game's own file, and MaxStreamingMemory_0..3_Base, 1500, 1500, 3000
// and 3000 MB. An index the texture quality gives picks the budget in use. A resource is placed only while the objects stay
// below the most and the bytes below the budget; otherwise the engine evicts what it cannot place and textures go missing,
// however much memory the card has, and an update check evicts ahead of time once the bytes pass a factor of the budget.
// VIDEO_MEMORY_BUDGET in [RESOURCES] raises each of the four budgets that is below it to it and lowers none
// (SPECIFICATIONS 11.12):
//   0                the default: the game's own budgets - nothing is looked for, but with DIAGNOSTIC_MODE=1
//   1 to 1499        raised to 1500, the lowest budget the game's own file gives, and reported
//   1500 to 16384    once Q-E-T1 to Q-E-T4 are found once each and every site checked - the start-up's stores of the most and
//                    the four budgets and the call after them (T1, T2), the budget's getter with the index (T3), the placement's
//                    check of the counters (T4), every address inside the game and aligned, the sites agreeing on the counters -
//                    the scanner's search for stores has found the start-up's two the budgets' only writers, the most and the
//                    budgets are still 0, the start-up still to come, and the index can be read, the call after the stores
//                    is redirected through a call stub to a function that raises the budgets the game has just written, logs
//                    the four it found and the index in use, and calls the game's function; nothing else is written
//   DRY_RUN          everything found and checked, the call not redirected
//   DIAGNOSTIC_MODE  every site logged, the budgets' writers among them; and the measurement, with Q-E-T5, the update check
//                    with its switch, flag and factors: the counters read 10 seconds in and again every minute that brings a
//                    new high, a change of the update check or of the budgets; the update check's switch and factors logged
//                    at the first summary and again if they change, a summary before the game has read gameconfig.xml saying
//                    that its budgets are not set yet
// Every read of the game's data - the counters, the index, the flag, the switch and its factors, and the most and the budgets
// at TweaksV's start - is an aligned read after IsReadable, the raise's of the budgets and the index after TweaksV's start
// found them readable; the raise writes the four budgets alone, right after the game's own stores made them, on the game's
// start-up thread.
// Windows' own budget for the game's video memory is not read: VIDEO_MEMORY_BUDGET has a documented maximum in its place,
// since the update check evicts against Windows' budget anyway.
// Written for TweaksV from the findings of its own research in the game's code (PATTERNS.md, "Resources"); the idea of a
// larger budget is that of zombieguy's Resource Adjuster for the Legacy game, which published no code - none was read.

#include "TweaksVTextureBudget.h"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "Resources";

		// ---- the counters, one object of the game's data (PATTERNS.md, "Resources"): offsets from its start

		constexpr size_t kBuffersBytes = 0x00;		// qword: the streamed buffers' video memory in bytes
		constexpr size_t kTexturesBytes = 0x08;		// qword: the streamed textures'
		constexpr size_t kBuffersNumber = 0x10;		// dword: how many buffers
		constexpr size_t kTexturesNumber = 0x14;	// dword: how many textures
		constexpr size_t kHighest = 0x18;			// qword: the most bytes of both so far, the game's own high-water mark
		constexpr size_t kMost = 0x20;				// dword: the most objects allowed (MaxStreamingResources)
		constexpr size_t kBudgets = 0x28;			// qwords: the budgets in bytes, one per index
		constexpr size_t kBudgetCount = 4;			// the indices 0 to 3 (MaxStreamingMemory_0..3_Base)
		constexpr size_t kCountersSize = kBudgets + kBudgetCount * sizeof(uint64_t);
		constexpr size_t kWritersLooked = 16;		// the budgets' writers the scan looks for: the start-up's two, and more besides them

		// VIDEO_MEMORY_BUDGET: 0, the game's own, or a budget from the lowest the game's own file gives, 1500 MB
		// (MaxStreamingMemory_0_Base and _1_Base), to the setting's maximum
		constexpr int kLowestBudgetMb = 1500;
		static_assert(kVideoMemoryBudget.minValue == 0 && kVideoMemoryBudget.defaultValue == 0 && kVideoMemoryBudget.maxValue > kLowestBudgetMb,
			"TweaksV: VIDEO_MEMORY_BUDGET is 0, the game's own, or from 1500 MB to its maximum");

		// The update check's switch, a dword, and its three factors, floats, one block of the game's data
		constexpr size_t kRaiseFactor = 4;			// it evicts once the bytes pass this much of the budget
		constexpr size_t kLowerFactor = 8;			// and stops once they are below this much
		constexpr size_t kWindowsFactor = 12;		// it evicts too once the game's video memory passes this much of Windows' budget
		constexpr size_t kSwitchSize = 16;

		// ---- the sites (PATTERNS.md, "Resources")

		constexpr const GamePattern& kStoreMost = Pattern("Q-E-T1");		// the start-up's store of the most, then of budgets 0 and 1
		constexpr const GamePattern& kStoreBudgets = Pattern("Q-E-T2");	// its store of budgets 2 and 3, then the call after them
		constexpr const GamePattern& kGetter = Pattern("Q-E-T3");			// the budget's getter
		constexpr const GamePattern& kPlacement = Pattern("Q-E-T4");		// a resource's placement: its check of the counters
		constexpr const GamePattern& kUpdate = Pattern("Q-E-T5");			// the update check

		// Q-E-T1: "mov ecx, [rax+disp32]", the configuration's most (+2 its offset); "mov [rip+disp32], ecx" (+6, its
		// rel32 at +8); "pmovsxdq xmm0, [rax+disp32]", budgets 0 and 1 in MB (+12, their offset at +17). Past the
		// match "psllq xmm0, 20" and "movdqu [rip+disp32], xmm0" (+21, the store's rel32 at +30) - the same two
		// instructions Q-E-T2 holds at +9 for budgets 2 and 3 - and Q-E-T2 itself (+34).
		constexpr size_t kMostOffsetAt = 2, kMostStoreAt = 6, kBudgets01OffsetAt = 17, kShiftAt = 21, kBudgets01StoreAt = 26;
		constexpr size_t kStoreBudgetsAt = 34;
		static_assert(PatternHolds(kStoreMost, 0, { 0x8B, 0x88 }) && PatternHolds(kStoreMost, kMostStoreAt, { 0x89, 0x0D }) &&
			PatternHolds(kStoreMost, 12, { 0x66, 0x0F, 0x38, 0x25, 0x80 }), "TweaksV: Q-E-T1 holds the loads and the store of the most");
		constexpr uint8_t kShiftAndStore[] = { 0x66, 0x0F, 0x73, 0xF0, 0x14, 0xF3, 0x0F, 0x7F, 0x05 };
		static_assert(PatternHolds(kStoreBudgets, 9, kShiftAndStore, sizeof(kShiftAndStore)) &&
			kShiftAt + sizeof(kShiftAndStore) == kBudgets01StoreAt + 4 && kBudgets01StoreAt + 8 == kStoreBudgetsAt,
			"TweaksV: Q-E-T2 holds at +9 the shift and the store that follow Q-E-T1");

		// Q-E-T2: "pmovsxdq xmm0, [rax+disp32]", budgets 2 and 3 (+5 their offset); the shift and "movdqu
		// [rip+disp32], xmm0" (+14, its rel32 at +18); then the call after the stores (+40), its rel32 past the match.
		constexpr size_t kBudgets23OffsetAt = 5, kBudgets23StoreAt = 14, kCallAt = 40, kCallSize = 5;
		static_assert(PatternHolds(kStoreBudgets, kBudgets23StoreAt, { 0xF3, 0x0F, 0x7F, 0x05 }) && PatternByte(kStoreBudgets, kCallAt) == 0xE8,
			"TweaksV: Q-E-T2 holds the store of budgets 2 and 3 at +14 and the call at +40");

		// Q-E-T3: "movsxd rax, [rip+disp32]", the index (its rel32 at +3), and "mov rax, [rcx+rax*8+0x28]": the budgets
		// at +0x28 of the counters, which the caller hands it in rcx.
		constexpr size_t kIndexAt = 3;
		static_assert(PatternHolds(kGetter, 0, { 0x48, 0x63, 0x05 }) && PatternByte(kGetter, 11) == static_cast<int>(kBudgets),
			"TweaksV: Q-E-T3 loads the index and reads the budgets at +0x28 of the counters");

		// Q-E-T4: the rel32 of its loads - the buffers' number (+2), the textures' number (+8), the most (+16, of a
		// "cmp"), the buffers' bytes (+25) and the textures' bytes (+32, past the match) - and the field each must be.
		struct PlacementLoad { size_t at; size_t field; const char* what; };
		constexpr PlacementLoad kPlacementLoads[] = { { 2, kBuffersNumber, "buffers' number" }, { 8, kTexturesNumber, "textures' number" },
			{ 16, kMost, "most objects" }, { 25, kBuffersBytes, "buffers' bytes" }, { 32, kTexturesBytes, "textures' bytes" } };
		constexpr size_t kPlacementCountersAt = 25, kPlacementPastAt = 32;
		static_assert(PatternHolds(kPlacement, 0, { 0x8B, 0x05 }) && PatternHolds(kPlacement, 6, { 0x8B, 0x0D }) && PatternHolds(kPlacement, 14, { 0x3B, 0x0D }) &&
			PatternHolds(kPlacement, 22, { 0x48, 0x8B, 0x05 }) && PatternHolds(kPlacement, 29, { 0x48, 0x8B, 0x35 }),
			"TweaksV: Q-E-T4 holds the loads of the counters");

		// Q-E-T5: "cmp dword [rip+disp32], 0", the switch (its disp32 at +2, an imm8 after it); "lea rcx, [rip+disp32]",
		// the counters handed to the getter (+16); the buffers' and the textures' bytes again (+23, +30); the getter's
		// call (+34). Past the match, at fixed distances: "cmp byte [rip+disp32], 0" (+0x4B), the flag, and the
		// factors' "mulss xmm, [rip+disp32]", the lower at +0x54 and the raise at +0xA0.
		constexpr size_t kSwitchAt = 2, kUpdateCountersAt = 16, kUpdateBuffersAt = 23, kUpdateTexturesAt = 30, kGetterCallAt = 34;
		constexpr size_t kFlagAt = 0x4B, kFlagSize = 7, kLowerAt = 0x54, kRaiseAt = 0xA0, kMulssMaxSize = 9;
		static_assert(PatternHolds(kUpdate, 0, { 0x83, 0x3D }) && PatternByte(kUpdate, 6) == 0x00 && PatternHolds(kUpdate, 13, { 0x48, 0x8D, 0x0D }) &&
			PatternHolds(kUpdate, 20, { 0x48, 0x8B, 0x3D }) && PatternHolds(kUpdate, 27, { 0x48, 0x8B, 0x35 }) && PatternByte(kUpdate, kGetterCallAt) == 0xE8,
			"TweaksV: Q-E-T5 holds the switch's compare, the loads of the counters and the getter's call");

		// What the checks found, set once everything is checked, before the call is redirected and the measurement's thread
		// starts
		const uint8_t* g_counters = nullptr;
		const uint8_t* g_index = nullptr;
		const uint8_t* g_getter = nullptr;
		const uint8_t* g_flag = nullptr;
		const uint8_t* g_switch = nullptr;
		uint8_t* g_budgets = nullptr;					// the four budgets, which the raise writes
		uint8_t* g_call = nullptr;						// the call after the start-up's stores, which the raise redirects
		size_t g_otherWriters = 0;						// the budgets' writers besides the start-up's two stores
		const uint8_t* g_firstOtherWriter = nullptr;

		// The raise: the budget in bytes and the game's function behind the call, both set before the call is redirected,
		// and the budgets the game had set, kept by the raise for the measurement's summary and published by g_raised;
		// g_raising is up around the raise's writes, so that a reading among them is made again.
		using NextStep = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t);
		uint64_t g_raiseTo = 0;
		NextStep g_nextStep = nullptr;
		std::atomic<uint64_t> g_gameBudgets[kBudgetCount] = {};
		std::atomic<bool> g_raising{ false };
		std::atomic<bool> g_raised{ false };

		// The 4 bytes of an operand in the game's code, which need not be aligned.
		uint32_t Imm32(const uint8_t* at)
		{
			uint32_t value = 0;
			memcpy(&value, at, sizeof(value));
			return value;
		}

		// What keeps the size bytes from p from being data of the game that is read unit bytes at a time, each read whole: "not
		// inside the game", or "not aligned to 8 bytes", where a read could straddle a cache line and tear; "" when nothing does.
		std::string Misplaced(const uint8_t* p, size_t size, size_t unit)
		{
			if (!IsInsideGameImage(p) || !IsInsideGameImage(p + size - 1))
				return "not inside the game";
			if (reinterpret_cast<uintptr_t>(p) % unit != 0)
				return "not aligned to " + std::to_string(unit) + " bytes";
			return {};
		}

		// Where "mulss xmm, [rip+disp32]" at p leads - F3, maybe a REX, 0F 59, a ModRM of mod 00 and r/m 101, the
		// disp32 - or nullptr when p holds no such instruction; p readable for kMulssMaxSize bytes.
		const uint8_t* MulssTarget(const uint8_t* p)
		{
			const size_t rex = (p[1] & 0xF0) == 0x40 ? 1 : 0;
			if (p[0] != 0xF3 || p[1 + rex] != 0x0F || p[2 + rex] != 0x59 || (p[3 + rex] & 0xC7) != 0x05)
				return nullptr;
			return Rel32Target(p + 4 + rex);
		}

		// ---- the checks: the raise's and the measurement's

		// The patterns' matches: Q-E-T1 to Q-E-T4 for the raise and the measurement, Q-E-T5 for the measurement alone, with why
		// the first of the four and why Q-E-T5 have none.
		struct Matches
		{
			uint8_t* storeMost = nullptr;
			uint8_t* storeBudgets = nullptr;
			uint8_t* getter = nullptr;
			uint8_t* placement = nullptr;
			uint8_t* update = nullptr;
			std::string sitesWhy;
			std::string updateWhy;
		};

		// Every pattern the jobs need, looked for before anything is checked, so that the scanner names each one missing. The
		// reasons have no point: they take theirs from NotRaised and NotMeasured.
		Matches FindPatterns(bool measure)
		{
			Matches found;
			found.storeMost = Scanner::FindOnce(kStoreMost, found.sitesWhy);
			found.storeBudgets = Scanner::FindOnce(kStoreBudgets, found.sitesWhy);
			found.getter = Scanner::FindOnce(kGetter, found.sitesWhy);
			found.placement = Scanner::FindOnce(kPlacement, found.sitesWhy);
			if (measure)
				found.update = Scanner::FindOnce(kUpdate, found.updateWhy);
			return found;
		}

		// A site's line, with DIAGNOSTIC_MODE only.
		void LogSite(const std::string& line)
		{
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: " + line);
		}

		// The raise not done: a problem of the start-up summary.
		void NotRaised(const std::string& why)
		{
			ReportFailure(kTag, "The texture budget is not raised this session: " + why + ".");
		}

		// The measurement not started - logged, not reported, an aid (NOTES.md, T-DIAG).
		void NotMeasured(const std::string& why)
		{
			WriteLog(kTag, "DIAGNOSTIC: the texture budget is not measured: " + why + ".");
		}

		// Q-E-T1 to Q-E-T4, the sites the raise and the measurement share: each checked as the research found it - every
		// address inside the game and aligned to the size of its reads, the sites agreeing on the counters - and logged with
		// DIAGNOSTIC_MODE, the budgets' writers among them. False at the first that is not as expected, with the reason in why;
		// a writer besides the start-up's stores is not one of them: it refuses the raise alone, which it could undo.
		bool CheckSites(const Matches& found, std::string& why)
		{
			const auto fail = [&why](const std::string& reason)
			{
				why = reason;
				return false;
			};
			const uint8_t* const storeMost = found.storeMost;
			const uint8_t* const storeBudgets = found.storeBudgets;
			const uint8_t* const getter = found.getter;
			const uint8_t* const placement = found.placement;
			if (!found.sitesWhy.empty())
				return fail(found.sitesWhy);

			// The start-up's stores: the most, then budgets 0 and 1 past Q-E-T1, then budgets 2 and 3 right after them
			const uint8_t* const most = Rel32Target(storeMost + kMostStoreAt + 2);
			if (const std::string wrong = Misplaced(most, sizeof(uint32_t), sizeof(uint32_t)); !wrong.empty())
				return fail("the most objects at " + DescribeAddress(most) + At(kStoreMost) + " are " + wrong);
			LogSite("store of the most objects at " + DescribeAddress(storeMost + kMostStoreAt) + " -> " + DescribeAddress(most) +
				", from the configuration's +" + Hex(Imm32(storeMost + kMostOffsetAt)) + At(kStoreMost));
			const uint8_t* const shift = storeMost + kShiftAt;
			if (!IsReadable(shift, kStoreBudgetsAt - kShiftAt))
				return fail("the code after " + std::string(kStoreMost.label) + " at " + DescribeAddress(shift) + " cannot be read");
			if (memcmp(shift, kShiftAndStore, sizeof(kShiftAndStore)) != 0)
				return fail("the code after " + std::string(kStoreMost.label) + " at " + DescribeAddress(shift) + " is not the store of budgets 0 and 1: " +
					HexBytes(shift, kStoreBudgetsAt - kShiftAt));
			const uint8_t* const budgets = Rel32Target(storeMost + kBudgets01StoreAt + 4);
			if (const std::string wrong = Misplaced(budgets, kBudgetCount * sizeof(uint64_t), sizeof(uint64_t)); !wrong.empty())
				return fail("the budgets at " + DescribeAddress(budgets) + At(kStoreMost) + " are " + wrong);
			LogSite("store of budgets 0 and 1 at " + DescribeAddress(storeMost + kBudgets01StoreAt) + " -> " + DescribeAddress(budgets) +
				", from the configuration's +" + Hex(Imm32(storeMost + kBudgets01OffsetAt)) + " in MB" + At(kStoreMost));
			if (storeBudgets != storeMost + kStoreBudgetsAt)
				return fail("the store of budgets 2 and 3 at " + DescribeAddress(storeBudgets) + At(kStoreBudgets) +
					" does not follow the store of budgets 0 and 1, which ends at " + DescribeAddress(storeMost + kStoreBudgetsAt));
			const uint8_t* const budgets23 = Rel32Target(storeBudgets + kBudgets23StoreAt + 4);
			if (budgets23 != budgets + 2 * sizeof(uint64_t))
				return fail("budgets 2 and 3 at " + DescribeAddress(budgets23) + At(kStoreBudgets) + " do not lie 16 bytes after budgets 0 and 1 at " +
					DescribeAddress(budgets));
			LogSite("store of budgets 2 and 3 at " + DescribeAddress(storeBudgets + kBudgets23StoreAt) + " -> " + DescribeAddress(budgets23) +
				", from the configuration's +" + Hex(Imm32(storeBudgets + kBudgets23OffsetAt)) + " in MB" + At(kStoreBudgets));

			// Every writer of the budgets, by the scanner's search for stores of any width: the start-up's two, and any besides
			// them - which would change the budgets again after the start-up, so that a raise would not hold
			const std::vector<uint8_t*> writers = Scanner::FindRipRelativeStores(budgets, kBudgetCount * sizeof(uint64_t), kWritersLooked);
			const uint8_t* const startUpStores[] = { storeMost + kBudgets01StoreAt, storeBudgets + kBudgets23StoreAt };
			if (writers.size() < kWritersLooked)
				for (const uint8_t* const store : startUpStores)
					if (std::find(writers.begin(), writers.end(), store) == writers.end())
						return fail("the scan for the budgets' writers does not find the start-up's store at " + DescribeAddress(store) +
							(store == startUpStores[0] ? At(kStoreMost) : At(kStoreBudgets)));
			std::string besides;
			size_t others = 0;
			const uint8_t* firstOther = nullptr;
			for (const uint8_t* const writer : writers)
				if (writer != startUpStores[0] && writer != startUpStores[1])
				{
					besides += (others++ ? ", " : "") + DescribeAddress(writer);
					if (!firstOther)
						firstOther = writer;
				}
			LogSite("writers of the budgets at " + DescribeAddress(budgets) + ": the start-up's stores at " + DescribeAddress(startUpStores[0]) + " and " +
				DescribeAddress(startUpStores[1]) + (others == 0 ? std::string(", none besides them") : ", and " + std::to_string(others) +
				(writers.size() == kWritersLooked ? " or more" : "") + " besides them: " + besides) + " (" + kStoreMost.label + ", " + kStoreBudgets.label + ")");

			// The call after the stores, the one the raise redirects: a call into the game
			uint8_t* const call = found.storeBudgets + kCallAt;
			const uint8_t* const callee = IsReadable(call, kCallSize) ? CallTarget(call) : nullptr;
			LogSite("call after the budgets' stores at " + DescribeAddress(call) + " -> " + (callee ? DescribeAddress(callee) : std::string("not a call")) +
				At(kStoreBudgets));
			if (!callee || !IsInsideGameImage(callee))
				return fail("the call after the budgets' stores at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it "
					"already)" + At(kStoreBudgets));

			// The getter: the index
			const uint8_t* const index = Rel32Target(getter + kIndexAt);
			if (const std::string wrong = Misplaced(index, sizeof(int32_t), sizeof(int32_t)); !wrong.empty())
				return fail("the index at " + DescribeAddress(index) + At(kGetter) + " is " + wrong);
			LogSite("budget getter at " + DescribeAddress(getter) + ": the index at " + DescribeAddress(index) + ", the budgets at +" + Hex(kBudgets) +
				" of the counters" + At(kGetter));

			// The placement's check: every load a field of the counters, the most and the budgets those the start-up writes
			const uint8_t* const counters = Rel32Target(placement + kPlacementCountersAt);
			if (const std::string wrong = Misplaced(counters, kCountersSize, sizeof(uint64_t)); !wrong.empty())
				return fail("the counters at " + DescribeAddress(counters) + At(kPlacement) + " are " + wrong);
			if (!IsReadable(placement + kPlacementPastAt, sizeof(int32_t)))
				return fail("the load of the textures' bytes after " + std::string(kPlacement.label) + " at " + DescribeAddress(placement + kPlacementPastAt) +
					" cannot be read");
			for (const PlacementLoad& load : kPlacementLoads)
				if (Rel32Target(placement + load.at) != counters + load.field)
					return fail(std::string("the placement's check loads the ") + load.what + " from " + DescribeAddress(Rel32Target(placement + load.at)) +
						", not from +" + Hex(load.field) + " of the counters at " + DescribeAddress(counters) + At(kPlacement));
			if (counters + kMost != most)
				return fail("the most objects the placement's check reads at " + DescribeAddress(counters + kMost) + " are not those " + kStoreMost.label +
					" stores at " + DescribeAddress(most));
			if (counters + kBudgets != budgets)
				return fail("the budgets at " + DescribeAddress(budgets) + At(kStoreMost) + " are not at +" + Hex(kBudgets) + " of the counters at " +
					DescribeAddress(counters) + At(kPlacement));
			LogSite("placement check at " + DescribeAddress(placement) + ": the numbers at " + DescribeAddress(counters + kBuffersNumber) + " and " +
				DescribeAddress(counters + kTexturesNumber) + ", the most at " + DescribeAddress(counters + kMost) + ", the bytes at " +
				DescribeAddress(counters + kBuffersBytes) + " and " + DescribeAddress(counters + kTexturesBytes) + At(kPlacement));

			g_counters = counters;
			g_index = index;
			g_getter = getter;
			g_budgets = const_cast<uint8_t*>(budgets);
			g_call = call;
			g_otherWriters = others;
			g_firstOtherWriter = firstOther;
			return true;
		}

		// Q-E-T5, the measurement's own: the update check's switch, the counters, the getter's call; past the match its flag
		// and its factors - each checked and logged as above, after CheckSites. False at the first that is not as expected,
		// with the reason in why.
		bool CheckUpdateCheck(const Matches& found, std::string& why)
		{
			const auto fail = [&why](const std::string& reason)
			{
				why = reason;
				return false;
			};
			if (!found.updateWhy.empty())
				return fail(found.updateWhy);
			const uint8_t* const update = found.update;
			const uint8_t* const counters = g_counters;
			const uint8_t* const getter = g_getter;
			const uint8_t* const switchAt = Rel32Target(update + kSwitchAt) + 1;	// the disp32 ends a byte before the instruction
			if (const std::string wrong = Misplaced(switchAt, kSwitchSize, sizeof(uint32_t)); !wrong.empty())
				return fail("the update check's switch and factors at " + DescribeAddress(switchAt) + At(kUpdate) + " are " + wrong);
			if (Rel32Target(update + kUpdateCountersAt) != counters || Rel32Target(update + kUpdateBuffersAt) != counters + kBuffersBytes ||
				Rel32Target(update + kUpdateTexturesAt) != counters + kTexturesBytes)
				return fail("the update check at " + DescribeAddress(update) + " does not read the counters at " + DescribeAddress(counters) + At(kUpdate));
			const uint8_t* const getterCall = update + kGetterCallAt;
			const uint8_t* const called = IsReadable(getterCall, kCallSize) ? CallTarget(getterCall) : nullptr;
			if (called != getter)
				return fail("the update check's call at " + DescribeAddress(getterCall) + " does not lead to the budget getter at " + DescribeAddress(getter) +
					At(kUpdate));
			LogSite("update check at " + DescribeAddress(update) + ": its switch at " + DescribeAddress(switchAt) + ", the counters at " +
				DescribeAddress(counters) + ", the getter's call at " + DescribeAddress(getterCall) + " -> " + DescribeAddress(called) + At(kUpdate));
			const uint8_t* const compare = update + kFlagAt;
			if (!IsReadable(compare, kRaiseAt + kMulssMaxSize - kFlagAt))
				return fail("the update check's code after " + std::string(kUpdate.label) + " at " + DescribeAddress(compare) + " cannot be read");
			if (compare[0] != 0x80 || compare[1] != 0x3D || compare[6] != 0x00)
				return fail("the update check's code at " + DescribeAddress(compare) + " is not the compare of its flag: " + HexBytes(compare, kFlagSize) +
					At(kUpdate));
			const uint8_t* const flag = Rel32Target(compare + 2) + 1;	// an imm8 follows the disp32 here too
			if (const std::string wrong = Misplaced(flag, 1, 1); !wrong.empty())
				return fail("the update check's flag at " + DescribeAddress(flag) + At(kUpdate) + " is " + wrong);
			const uint8_t* const lower = update + kLowerAt;
			const uint8_t* const raise = update + kRaiseAt;
			if (MulssTarget(lower) != switchAt + kLowerFactor)
				return fail("the update check's code at " + DescribeAddress(lower) + " is not the multiplication by its factor at " +
					DescribeAddress(switchAt + kLowerFactor) + ": " + HexBytes(lower, kMulssMaxSize) + At(kUpdate));
			if (MulssTarget(raise) != switchAt + kRaiseFactor)
				return fail("the update check's code at " + DescribeAddress(raise) + " is not the multiplication by its factor at " +
					DescribeAddress(switchAt + kRaiseFactor) + ": " + HexBytes(raise, kMulssMaxSize) + At(kUpdate));
			LogSite("update check's flag at " + DescribeAddress(compare) + " -> " + DescribeAddress(flag) + ", its factors at " + DescribeAddress(lower) +
				" -> " + DescribeAddress(switchAt + kLowerFactor) + " and " + DescribeAddress(raise) + " -> " + DescribeAddress(switchAt + kRaiseFactor) +
				At(kUpdate));

			g_flag = flag;
			g_switch = switchAt;
			return true;
		}

		// ---- the measurement (DIAGNOSTIC_MODE): the counters, read as they are

		// A value of the game's data that the game's threads may write meanwhile: aligned, as the checks hold it, so read whole.
		template <typename T> T Load(const uint8_t* at)
		{
			return *reinterpret_cast<const volatile T*>(at);
		}

		struct Reading
		{
			bool readable = false;
			uint64_t buffers = 0, textures = 0, highest = 0;			// bytes
			uint32_t buffersNumber = 0, texturesNumber = 0, most = 0;
			uint64_t budgets[kBudgetCount] = {};
			bool raised = false;										// the raise has run: gameBudgets holds the budgets the game had
			bool raiseNotEnded = false;									// its flag still up after kRaiseWaitMs: no summary from this reading
			uint64_t gameBudgets[kBudgetCount] = {};
			int32_t index = 0;
			uint8_t flag = 0;
			bool updateCheckReadable = false;
			uint32_t updateCheck[kSwitchSize / sizeof(uint32_t)] = {};	// the switch and its three factors, as they lie
		};

		constexpr ULONGLONG kRaiseWaitMs = 1000;	// how long a reading waits for the raise's writes before it says they did not end

		Reading ReadCounters()
		{
			Reading reading;
			if (IsReadable(g_switch, kSwitchSize))
			{
				reading.updateCheckReadable = true;
				for (size_t i = 0; i < std::size(reading.updateCheck); i++)
					reading.updateCheck[i] = Load<uint32_t>(g_switch + i * sizeof(uint32_t));
			}
			if (!IsReadable(g_counters, kCountersSize) || !IsReadable(g_index, sizeof(int32_t)) || !IsReadable(g_flag, 1))
				return reading;
			reading.readable = true;
			reading.buffers = Load<uint64_t>(g_counters + kBuffersBytes);
			reading.textures = Load<uint64_t>(g_counters + kTexturesBytes);
			reading.buffersNumber = Load<uint32_t>(g_counters + kBuffersNumber);
			reading.texturesNumber = Load<uint32_t>(g_counters + kTexturesNumber);
			reading.highest = Load<uint64_t>(g_counters + kHighest);
			reading.most = Load<uint32_t>(g_counters + kMost);
			// the raise's record before the budgets, so that a reading never pairs the budgets the game had with budgets read
			// before the raise; a raise that comes while they are read is taken with them, the budgets read again - and again
			// while its flag is up, so that a half-raised set never passes as the game's own: its writes are a few instructions,
			// made once, so a flag still up after kRaiseWaitMs says they did not end, and such a reading is not used
			const ULONGLONG started = GetTickCount64();
			for (;;)
			{
				reading.raised = g_raised.load(std::memory_order_acquire);
				for (size_t i = 0; i < kBudgetCount; i++)
				{
					reading.gameBudgets[i] = reading.raised ? g_gameBudgets[i].load(std::memory_order_relaxed) : 0;
					reading.budgets[i] = Load<uint64_t>(g_counters + kBudgets + i * sizeof(uint64_t));
				}
				std::atomic_thread_fence(std::memory_order_acquire);	// the budgets read before the flags
				if (!g_raising.load(std::memory_order_acquire) && (reading.raised || !g_raised.load(std::memory_order_acquire)))
					break;
				if (GetTickCount64() - started >= kRaiseWaitMs)
				{
					reading.raiseNotEnded = true;
					break;
				}
				YieldProcessor();
			}
			reading.index = Load<int32_t>(g_index);
			reading.flag = Load<uint8_t>(g_flag);
			return reading;
		}

		uint64_t Bytes(const Reading& reading)	// of both, the textures' and the buffers'
		{
			return reading.textures > UINT64_MAX - reading.buffers ? UINT64_MAX : reading.textures + reading.buffers;
		}

		uint64_t Objects(const Reading& reading)
		{
			return uint64_t{ reading.buffersNumber } + reading.texturesNumber;
		}

		std::string Mb(uint64_t bytes)
		{
			return std::to_string(bytes >> 20);
		}

		// "1500, 1500, 3000 and 3000": the four budgets in MB.
		std::string FourMb(const uint64_t (&budgets)[kBudgetCount])
		{
			return Mb(budgets[0]) + ", " + Mb(budgets[1]) + ", " + Mb(budgets[2]) + " and " + Mb(budgets[3]);
		}

		// The measurement's state, kept by its thread alone: the reading of the last summary, TweaksV's own highs of
		// every reading, how many readings had something new to say, whether a raise that did not end has been said, and the
		// update check as its line last gave it.
		Reading g_logged;
		uint64_t g_mostBytes = 0, g_mostObjects = 0;
		uint64_t g_loggedMostMb = 0, g_loggedMostObjects = 0;
		uint64_t g_news = 0;
		bool g_raiseNotEndedSaid = false;
		bool g_updateCheckShown = false;
		Reading g_updateCheckLine;

		bool SameUpdateCheck(const Reading& a, const Reading& b)
		{
			return a.updateCheckReadable == b.updateCheckReadable && memcmp(a.updateCheck, b.updateCheck, sizeof(a.updateCheck)) == 0;
		}

		// Changes with every reading that has something new to say: the first; a new high - of the bytes in MB, of
		// the objects, of the game's own mark in MB; the update check's flag raised or lowered, its switch or a factor
		// changed; another index, budget or most; the raise run since; the counters readable or not. A reading that met a raise
		// that did not end changes nothing: it is said once, and no summary comes of it.
		uint64_t BudgetNumbers()
		{
			const Reading now = ReadCounters();
			if (now.raiseNotEnded)
			{
				if (!g_raiseNotEndedSaid)
					WriteLog(kTag, "DIAGNOSTIC: texture budget: the raise of the budgets has not ended after 1 second: no summary comes until it has.");
				g_raiseNotEndedSaid = true;
				return g_news;
			}
			if (now.readable)
			{
				g_mostBytes = std::max(g_mostBytes, Bytes(now));
				g_mostObjects = std::max(g_mostObjects, Objects(now));
			}
			const bool news = g_news == 0 || now.readable != g_logged.readable || !SameUpdateCheck(now, g_logged) || (now.readable &&
				((g_mostBytes >> 20) > g_loggedMostMb || g_mostObjects > g_loggedMostObjects || (now.highest >> 20) > (g_logged.highest >> 20) ||
				now.flag != g_logged.flag || now.index != g_logged.index || now.most != g_logged.most || now.raised != g_logged.raised ||
				memcmp(now.budgets, g_logged.budgets, sizeof(now.budgets)) != 0));
			if (news)
			{
				g_logged = now;
				g_loggedMostMb = g_mostBytes >> 20;
				g_loggedMostObjects = g_mostObjects;
				g_news++;
			}
			return g_news;
		}

		std::string Factor(uint32_t bits)
		{
			char text[32];
			snprintf(text, sizeof(text), "%.3g", static_cast<double>(std::bit_cast<float>(bits)));
			return text;
		}

		// The update check's switch and factors, at the first summary and again whenever they changed: they lie in the game's
		// data, which the research could not see, and whether the game sets them only after its start is not known.
		void LogUpdateCheck(const Reading& now)
		{
			if (g_updateCheckShown && SameUpdateCheck(now, g_updateCheckLine))
				return;
			g_updateCheckShown = true;
			g_updateCheckLine = now;
			if (!now.updateCheckReadable)
			{
				WriteLog(kTag, "DIAGNOSTIC: texture budget: the update check's switch and factors at " + DescribeAddress(g_switch) + " cannot be read.");
				return;
			}
			const auto on = static_cast<int32_t>(now.updateCheck[0]);
			WriteLog(kTag, "DIAGNOSTIC: texture budget: the update check's switch holds " + std::to_string(on) + (on ? " (on)" :
				" (off: nothing is evicted ahead of time)") + ", its factors " + Factor(now.updateCheck[kRaiseFactor / sizeof(uint32_t)]) +
				" of the budget to start evicting, " + Factor(now.updateCheck[kLowerFactor / sizeof(uint32_t)]) + " to stop, and " +
				Factor(now.updateCheck[kWindowsFactor / sizeof(uint32_t)]) + " of the video memory Windows gives the game to start evicting too.");
		}

		void LogBudget(uint64_t news)
		{
			if (news == 0)	// the measurement's first turn, its reading not used: the first summary comes with the first that is
				return;
			LogUpdateCheck(g_logged);
			const Reading& now = g_logged;
			if (!now.readable)
			{
				WriteLog(kTag, "DIAGNOSTIC: texture budget: the counters at " + DescribeAddress(g_counters) + " cannot be read.");
				return;
			}
			const bool known = now.index >= 0 && now.index < static_cast<int32_t>(kBudgetCount);
			const uint64_t budget = known ? now.budgets[static_cast<size_t>(now.index)] : 0;
			// the budgets as the game set them, and where the raise has changed them, as they are now - both from this one reading
			uint64_t game[kBudgetCount] = {};
			for (size_t i = 0; i < kBudgetCount; i++)
				game[i] = now.raised ? now.gameBudgets[i] : now.budgets[i];
			std::string line = "DIAGNOSTIC: texture budget: " + Mb(Bytes(now)) + " MB";
			line += known ? " of the " + Mb(budget) + " MB budget for the texture quality's index " + std::to_string(now.index) :
				" - the texture quality's index " + std::to_string(now.index) + " has no budget";
			line += " (MaxStreamingMemory_0..3_Base: " + FourMb(game) + " MB" + (memcmp(game, now.budgets, sizeof(game)) != 0 ? ", raised to " +
				FourMb(now.budgets) + " MB by VIDEO_MEMORY_BUDGET" : std::string()) + ") - textures " + Mb(now.textures) + " MB, buffers " + Mb(now.buffers) +
				" MB; " + std::to_string(Objects(now)) + " of the " + std::to_string(now.most) + " objects allowed (MaxStreamingResources) - " +
				std::to_string(now.texturesNumber) + " textures, " + std::to_string(now.buffersNumber) + " buffers; the most so far " + Mb(now.highest) +
				" MB by the game's own mark, " + Mb(g_mostBytes) + " MB and " + std::to_string(g_mostObjects) + " objects at TweaksV's readings; the update "
				"check's flag is " + (now.flag ? "up: it evicts ahead of time" : "down");
			// all 0 before the game's start-up has read gameconfig.xml, which can come after the first summary: no limit then
			if (now.most == 0 && std::all_of(std::begin(now.budgets), std::end(now.budgets), [](uint64_t bytes) { return bytes == 0; }))
				line += " - the budgets and the most are not set yet (the game sets them when it reads gameconfig.xml)";
			if (known && budget != 0 && Bytes(now) >= budget)
				line += " - at the budget: the streaming engine evicts what it cannot place (texture loss)";
			if (now.most != 0 && Objects(now) >= now.most)
				line += " - at the most objects: the streaming engine evicts what it cannot place (texture loss)";
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "texture budget summary", kFirstSummaryMs, &BudgetNumbers, &LogBudget };

		// The measurement (DIAGNOSTIC_MODE), after CheckSites: its own site, the update check, checked, then its thread
		// started. An aid: what stops it is logged, not reported, and an exception ends it alone.
		void StartBudgetMeasurement(const Matches& found)
		{
			try
			{
				std::string why;
				if (!CheckUpdateCheck(found, why))
				{
					NotMeasured(why);
					return;
				}
				WriteLog(kTag, "DIAGNOSTIC: the texture budget's counters at " + DescribeAddress(g_counters) + At(kPlacement) + ": the streamed textures and "
					"buffers are logged after 10 seconds, then every minute while their highs, the update check or the budget in use change.");
				StartMeasurement<kMeasurement>();	// one that cannot start is logged
			}
			catch (...)
			{
				WriteLog(kTag, "DIAGNOSTIC: unexpected exception - the texture budget's measurement is skipped for this session.");
			}
		}

		// ---- the raise

		// The most objects and the four budgets still 0 at TweaksV's start, read after IsReadable: the game's start-up has not
		// stored them yet, so the call after its stores, which the raise redirects, is still to come. A start-up that ran before
		// TweaksV's DllMain - a loader that starts TweaksV late, a game build that stores them earlier - has made that call
		// already, and a redirect would never run. The index must be readable too: the raise reads it, on the game's start-up
		// thread. False with the reason in why.
		bool StartUpToCome(std::string& why)
		{
			const uint8_t* const most = g_counters + kMost;
			const std::string where = "the most objects at " + DescribeAddress(most) + " and the budgets at " + DescribeAddress(g_budgets);
			if (!IsReadable(g_counters, kCountersSize))
			{
				why = where + " cannot be read";
				return false;
			}
			if (!IsReadable(g_index, sizeof(int32_t)))
			{
				why = "the index at " + DescribeAddress(g_index) + At(kGetter) + " cannot be read";
				return false;
			}
			const uint32_t mostObjects = Load<uint32_t>(most);
			uint64_t budgets[kBudgetCount] = {};
			for (size_t i = 0; i < kBudgetCount; i++)
				budgets[i] = Load<uint64_t>(g_budgets + i * sizeof(uint64_t));
			if (mostObjects != 0 || std::any_of(std::begin(budgets), std::end(budgets), [](uint64_t bytes) { return bytes != 0; }))
			{
				why = where + " were set before TweaksV started (MaxStreamingResources: " + std::to_string(mostObjects) + ", MaxStreamingMemory_0..3_Base: " +
					FourMb(budgets) + " MB): the game's start-up ran before TweaksV's DllMain, and the call after its stores with it";
				return false;
			}
			LogSite(where + " are still 0 at TweaksV's start: the game's start-up sets them after TweaksV's DllMain, so the call after its stores "
				"is still to come");
			return true;
		}

		// The game's call after its start-up has stored the budgets, redirected: each budget below VIDEO_MEMORY_BUDGET raised
		// to it, none lowered, then the game's function, with its result handed back - on the start-up thread, before any
		// streaming. The research did not read that function's arguments: the start-up sets none up between its stores and
		// the call, and gives it no room on the stack but the 32 bytes every call gets, so it takes four registers at most,
		// which are handed on as they came. Nothing may throw out of here, and the game's function is always called.
		uint64_t RaiseBudgets(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth)
		{
			uint64_t had[kBudgetCount] = {};
			uint64_t now[kBudgetCount] = {};
			bool raised = false;
			for (size_t i = 0; i < kBudgetCount; i++)
			{
				had[i] = Load<uint64_t>(g_budgets + i * sizeof(uint64_t));
				now[i] = had[i];
				if (had[i] < g_raiseTo)
				{
					now[i] = g_raiseTo;
					raised = true;
				}
				g_gameBudgets[i].store(had[i], std::memory_order_relaxed);
			}
			// the record kept and the flag up before the first write, the flag down after the record is published (ReadCounters)
			g_raising.store(true, std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_release);
			for (size_t i = 0; i < kBudgetCount; i++)
				if (now[i] != had[i])
					*reinterpret_cast<volatile uint64_t*>(g_budgets + i * sizeof(uint64_t)) = now[i];
			g_raised.store(true, std::memory_order_release);
			g_raising.store(false, std::memory_order_release);
			const int32_t index = Load<int32_t>(g_index);
			try
			{
				std::string line = "The texture budgets of gameconfig.xml (MaxStreamingMemory_0..3_Base: " + FourMb(had) + " MB) are ";
				line += raised ? "raised to " + Mb(g_raiseTo) + " MB where they are below it: " + FourMb(now) + " MB now" :
					"at least " + Mb(g_raiseTo) + " MB each - left as they are";
				line += index >= 0 && index < static_cast<int32_t>(kBudgetCount) ? "; the texture quality's index " + std::to_string(index) + " uses " +
					Mb(now[static_cast<size_t>(index)]) + " MB." : "; the texture quality's index " + std::to_string(index) + " has no budget.";
				WriteLog(kTag, line);
			}
			catch (...)
			{
			}
			return g_nextStep(first, second, third, fourth);
		}
	}

	void InitTextureBudget()
	{
		// the budget asked for: 0, the game's own, or one from the lowest the game's own file gives on
		const int asked = GetNumber(kVideoMemoryBudget);
		const int budgetMb = asked != 0 && asked < kLowestBudgetMb ? kLowestBudgetMb : asked;
		if (budgetMb != asked)
			ReportFailure(kTag, std::string("TweaksV.ini: [") + kResourcesSection.name + "] " + kVideoMemoryBudget.key + "=" + std::to_string(asked) +
				" is outside 0 or " + std::to_string(kLowestBudgetMb) + ".." + std::to_string(kVideoMemoryBudget.maxValue) + " - using " +
				std::to_string(budgetMb) + ".");
		const bool measure = IsDiagnosticMode();
		bool raise = budgetMb != 0;
		if (!raise)
		{
			WriteLog(kTag, std::string(kVideoMemoryBudget.key) + "=0 - the texture budget is not raised.");
			if (!measure)
				return;
		}

		// the sites the raise and the measurement share, all checked before anything is written
		const Matches found = FindPatterns(measure);
		std::string why;
		if (!CheckSites(found, why))
		{
			if (raise)
				NotRaised(why);
			if (measure)
				NotMeasured(why);
			return;
		}

		// the raise, refused while the budgets have a possible writer besides the start-up's stores - it could undo the raise
		// later, and the search counts a load whose bytes read as a store too (SPECIFICATIONS 8) - and
		// once the start-up has stored them before TweaksV started: the call the raise redirects is made already
		if (raise && g_otherWriters != 0)
		{
			NotRaised("the budgets at " + DescribeAddress(g_budgets) + " have " + std::to_string(g_otherWriters) + (g_otherWriters == 1 ? " possible writer" : " possible writers") +
				" besides the start-up's stores, the first at " + DescribeAddress(g_firstOtherWriter) + ", which could undo the raise later");
			raise = false;
		}
		else if (raise && !StartUpToCome(why))
		{
			NotRaised(why);
			raise = false;
		}
		if (raise)
		{
			g_raiseTo = static_cast<uint64_t>(budgetMb) << 20;
			const std::vector<CallRedirect> redirect = { { g_call, reinterpret_cast<const void*>(&RaiseBudgets), "call after the budgets' stores",
				reinterpret_cast<void**>(&g_nextStep) } };
			if (!RedirectCalls(redirect, {}, kTag))
				NotRaised("the call after the budgets' stores at " + DescribeAddress(g_call) + " could not be redirected (details in TweaksV.log)");
			else if (IsDryRun())
				WriteLog(kTag, "DRY_RUN: the texture budgets below " + std::to_string(budgetMb) + " MB would be raised to it when the game's start-up sets them.");
			else
				WriteLog(kTag, "The call after the budgets' stores at " + DescribeAddress(g_call) + " is redirected: the texture budgets below " +
					std::to_string(budgetMb) + " MB are raised to it when the game's start-up sets them.");
		}

		// the measurement, with DIAGNOSTIC_MODE, the budget at 0 too
		if (measure)
			StartBudgetMeasurement(found);
	}
}
