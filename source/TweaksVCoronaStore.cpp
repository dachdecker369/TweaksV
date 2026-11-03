// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the corona store (see TweaksVCoronaStore.h).
//
// The game holds a frame's coronas in one store, 1088 records of 0x30 bytes with its count right behind them. A general add (G-E-I1), two
// copies of it inlined (G-E-G1) and the LOD lights' job, whose batch lands all at once under a clamp of its own (G-E-I8), fill it; past the
// 1088 each drops its corona without a word. One copy into the draw list reads the records, and everything after that reads the copy, so
// nothing behind the store knows its size. So the records move and the count stays where it is:
//   the new store  N records and 0x180 bytes past them, since the copy always reads 8 records more, near the game; 16-aligned, as Windows
//                  gives memory, since the adds write with movaps
//   the writes     13, through the part's one write (PATTERNS.md, "LightCapacity"): the five that name the store now name the new one; the five
//                  limits take N, the batch's clamp among them; the three that reach the count from the store's start reach it from the new
//                  one; and the general add's nop before its loop turns the old store its eleven callers hand it into the new one
//   the clamp      must move with the store: at a count past 1088 the old clamp lowers the count and asks for a copy of about 200 GB. So every
//                  site is found and checked before anything is written, and any site missing refuses the part as a whole
//   the sets       nothing gates this raise by the game's build, so a later build whose patterns still match may hold more than the research
//                  found: a second caller of what hands the store out gets its count from the old store, and a 20th reference may be a new
//                  batch with a clamp of its own. So both are counted first, and more than the research found refuses, as for the part's own stores
// At the game's own 1088 nothing of it is looked for or written. A dry run finds and checks everything and writes nothing.
// With DIAGNOSTIC_MODE the store counts what it refuses, each by a stub in the form of the lighting capacity's: four jumps through a
// lock inc on to where they led, and the batch's six bytes through a stub that adds what the batch could not place. The stubs lie near the
// game, read+execute, use no stack and keep every register but the flags, which their targets write anew before anything reads them, and
// the batch's rax, which the moved lea writes; the counters lie on a read/write page of their own. They run only when a corona is refused,
// and at each batch. A stub that cannot be made is logged, not reported - an aid (ISSUES.md, T-DIAG) - and the raise goes on without it.
// If the part refuses after this has said yes it calls DiscardCoronaStore, and the store's memory and the stubs' go back.
// Written for TweaksV from the findings of its own research in the game's code; the idea of raising what many siren lights reach comes from
// the page of Resource Extensions [Enhanced] by Koukol, whose plugin was not read (CREDITS.md).

#include "TweaksVCoronaStore.h"

#include <windows.h>

#include <atomic>
#include <cstring>
#include <cwchar>
#include <iterator>

#include "TweaksVConflicts.h"
#include "TweaksVLightCapacity.h"
#include "TweaksVLog.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "LightCapacity";

		// ---- the sites (PATTERNS.md, "LightCapacity")

		enum Site : size_t { kI1, kI2, kI3, kI4, kI5, kI6, kI7, kI8, kI9, kJ1, kJ2, kJ3, kJ4, kJ5, kSites };
		constexpr const GamePattern* kSitePatterns[kSites] = { &Pattern("G-E-I1"), &Pattern("G-E-I2"), &Pattern("G-E-I3"), &Pattern("G-E-I4"), &Pattern("G-E-I5"), &Pattern("G-E-I6"), &Pattern("G-E-I7"),
			&Pattern("G-E-I8"), &Pattern("G-E-I9"), &Pattern("G-E-J1"), &Pattern("G-E-J2"), &Pattern("G-E-J3"), &Pattern("G-E-J4"), &Pattern("G-E-J5") };
		constexpr const GamePattern& P(Site site)
		{
			return *kSitePatterns[site];
		}

		// A corona, the store's count right behind the game's own, and the 8 records past the count the copy always reads; a job's own scratch. The stack
		// a draw takes per corona and what the render threads have of it, which the key's maximum stays under, with 64 KB to spare.
		constexpr uint32_t kCoronaBytes = 0x30, kCountAt = 0xCC00, kPastRecords = 0x180;
		constexpr uint32_t kScratchLists = 0x1C00, kScratchBytes = 0xE800;
		constexpr uint32_t kStackPerCorona = 0x24, kStackCommitted = 0x80000, kStackSpare = 0x10000;

		// The sites' fields, from their matches. J2's last and J3's last lie past their matches.
		constexpr size_t kAddLeaRelAt = 3, kAddLoadAt = 17, kAddLimitAt = 24, kAddJumpAt = 28, kAddJumpRelAt = 30, kAddExchangeAt = 45;	// I1
		constexpr size_t kInlinedCountAt = 2, kInlinedLimitAt = 9, kInlinedJumpRelAt = 15;													// G1, the part's
		constexpr size_t kInlinedStoreAt = 11;																								// I2, I3
		constexpr size_t kCopyCountAt = 2, kCopyPastAt = 20, kCopyStoreAt = 8;																// I4; I5 the store
		constexpr size_t kJobStoreAt = 3, kJobCallAt = 17;																					// I6
		constexpr size_t kHandOutAddAt = 6;																									// I7
		constexpr size_t kBatchMoveAt = 1, kBatchCompareAt = 41, kBatchSixAt = 56, kBatchSix = 6;											// I8
		constexpr size_t kLodLimitAt = 7, kLodJumpRelAt = 13;																				// I9
		constexpr size_t kPointerAt = 7, kRenderCountAt = 16, kExtrasAt = 33;																// J2
		constexpr size_t kMarkAt = 3, kDrawCountAt = 15;																					// J3
		constexpr size_t kRel32 = sizeof(int32_t);
		constexpr size_t kStoreReferences = 19, kHandOutCalls = 1;	// the complete sets, by the research (PATTERNS.md): the store's references a scan sees, and what hands it out's callers

		static_assert(kCountAt == kGameCoronas * kCoronaBytes && kPastRecords == 8 * kCoronaBytes, "TweaksV: the store's count right behind its 1088 coronas, and 8 more read by its copy");
		static_assert(kCoronaStore.minValue == static_cast<int>(kGameCoronas) && (uint64_t{ static_cast<uint32_t>(kCoronaStore.maxValue) } + 8) * kStackPerCorona <= kStackCommitted - kStackSpare &&
			static_cast<uint32_t>(kCoronaStore.maxValue) + 8 <= 0xFFFF, "TweaksV: CORONA_STORE from the game's own to what a draw's stack and its word of an index take");

		// Every constant the raise rests on is the one its pattern pins: the nop that is rewritten, the instructions around each field, the job's sizes. G-E-G1 is
		// the part's, and the code names a pattern once, so that its three fields stand on the suite, which runs both inlined adds through what this writes.
		static_assert(PatternHolds(P(kI1), 0, { 0x66, 0x66, 0x66, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0, 0, 0, 0, 0 }) && PatternHolds(P(kI1), 14, { 0x44, 0x8B, 0x81 }) &&
			PatternImm32(P(kI1), kAddLoadAt) == kCountAt && PatternHolds(P(kI1), 21, { 0x49, 0x81, 0xF8 }) && PatternImm32(P(kI1), kAddLimitAt) == kGameCoronas - 1 &&
			PatternHolds(P(kI1), kAddJumpAt, { 0x0F, 0x87 }) && PatternHolds(P(kI1), 41, { 0xF0, 0x0F, 0xB1, 0xB1 }) && PatternImm32(P(kI1), kAddExchangeAt) == kCountAt && PatternHolds(P(kI1), 49, { 0x75, 0xDB }),
			"TweaksV: G-E-I1 is the nop, then mov r8d, [rcx+0xCC00], cmp r8, 0x43F, ja, and lock cmpxchg [rcx+0xCC00], esi, its jne back past the nop");
		static_assert(PatternHolds(P(kI2), 8, { 0x48, 0x8D, 0x0D }) && PatternHolds(P(kI3), 8, { 0x48, 0x8D, 0x0D }) && PatternHolds(P(kI4), 0, { 0x8B, 0x05 }) && PatternHolds(P(kI4), 19, { 0x05 }) &&
			PatternImm32(P(kI4), kCopyPastAt) == kPastRecords && PatternHolds(P(kI5), 5, { 0x48, 0x8D, 0x15 }) && PatternHolds(P(kI6), 0, { 0x48, 0x8D, 0x0D }) && PatternHolds(P(kI6), kJobCallAt, { 0xE8 }),
			"TweaksV: G-E-I2, I3, I5 and I6 load the store through a lea, G-E-I4 reads the count and adds 0x180, G-E-I6 calls what hands the store out");
		static_assert(PatternHolds(P(kI7), 3, { 0x48, 0x81, 0xC1 }) && PatternImm32(P(kI7), kHandOutAddAt) == kCountAt, "TweaksV: G-E-I7 adds 0xCC00 to the store for the count");
		static_assert(PatternHolds(P(kI8), 0, { 0xBD }) && PatternImm32(P(kI8), kBatchMoveAt) == kGameCoronas && PatternHolds(P(kI8), 39, { 0x81, 0xFA }) && PatternImm32(P(kI8), kBatchCompareAt) == kGameCoronas,
			"TweaksV: G-E-I8 is the batch's mov ebp, 0x440 and its cmp edx, 0x440");
		static_assert(PatternImm32(P(kI9), kLodLimitAt) == kGameCoronas - 1 && PatternHolds(P(kI9), 11, { 0x0F, 0x87 }), "TweaksV: G-E-I9 checks the job's own coronas against 0x43F and jumps when full");
		static_assert(PatternImm32(P(kJ1), 3) == kCountAt && PatternImm32(P(kJ1), 8) == kScratchLists && PatternImm32(P(kJ1), 35) == kScratchBytes, "TweaksV: G-E-J1 lays the job's scratch out as 1088 coronas and 0x1C00 bytes of lists");
		static_assert(PatternHolds(P(kJ2), 4, { 0x48, 0x89, 0x05 }) && PatternHolds(P(kJ2), 14, { 0x89, 0x15 }) && PatternHolds(P(kJ2), 31, { 0x89, 0x15 }) && PatternHolds(P(kJ3), 0, { 0xF0, 0xFF, 0x05 }) && PatternHolds(P(kJ3), 12, { 0x48, 0x63, 0x3D }),
			"TweaksV: G-E-J2 stores the render pointer, the render count and the extras' count, G-E-J3 marks the draw and reads the render count");

		// How far the stack a draw takes: J4 and J5 pin it, and nothing here reads them but the log.
		static_assert(PatternHolds(P(kJ4), 3, { 0x48, 0xC1, 0xE0, 0x05 }) && PatternHolds(P(kJ5), 0, { 0x48, 0x8D, 0x04, 0xBD }), "TweaksV: G-E-J4 takes 0x20 bytes a corona, G-E-J5 4 bytes");

		// What the raise changed, for the accessor: the room it wrote and the stubs' counters. The first stays 0 on a dry run.
		struct Counters
		{
			uint64_t refused;		// by any add, and what a batch could not place
			uint64_t lodRefused;	// by the LOD lights' job's own 1088
			uint32_t lodBrought;	// by its last batch
		};
		std::atomic<uint32_t> g_room{ 0 };
		std::atomic<const Counters*> g_counters{ nullptr };
		uint8_t* g_storeMemory = nullptr;	// what DiscardCoronaStore gives back; both DllMain's alone
		uint8_t* g_stubMemory = nullptr;

		// The DIAGNOSTIC lines: this module runs without DIAGNOSTIC_MODE too.
		void Note(const std::string& line)
		{
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: " + line);
		}

		struct Found
		{
			uint8_t* at[kSites] = {};
			uint8_t* inlined[2] = {};
			const uint8_t* store = nullptr;
		};

		// What keeps the store's own sites from being as the findings have them, in a sentence; "" when nothing does, each logged as it passes.
		std::string SitesFault(Found& f)
		{
			// the store, as the five name it, and its count right behind it
			f.store = Rel32Target(f.at[kI2] + kInlinedStoreAt);
			const uint8_t* const count = f.store + kCountAt;
			if (!IsInsideGameImage(f.store) || !IsInsideGameImage(count + sizeof(uint32_t) - 1))
				return "the corona store at " + DescribeAddress(f.store) + At(P(kI2)) + " is not inside the game (another mod may have moved it already).";
			const struct { const char* what; const uint8_t* named; Site site; } namers[] = {
				{ "second inlined add", Rel32Target(f.at[kI3] + kInlinedStoreAt), kI3 },
				{ "copy to the draw list", Rel32Target(f.at[kI5] + kCopyStoreAt), kI5 },
				{ "LOD lights' job", Rel32Target(f.at[kI6] + kJobStoreAt), kI6 } };
			for (const auto& namer : namers)
				if (namer.named != f.store)
					return std::string("the ") + namer.what + " names the corona store at " + DescribeAddress(namer.named) + ", the first inlined add the one at " + DescribeAddress(f.store) + At(P(namer.site)) + ".";
			Note("the corona store at " + DescribeAddress(f.store) + ", its count at " + DescribeAddress(count) + ": the inlined adds' stores at " + DescribeAddress(f.at[kI2] + kInlinedStoreAt) + At(P(kI2)) +
				" and " + DescribeAddress(f.at[kI3] + kInlinedStoreAt) + At(P(kI3)));
			const std::vector<uint8_t*> references = Scanner::FindRipRelativeReferences(f.store, kStoreReferences + 1);
			std::string list;
			for (size_t i = 0; i < references.size(); i++)
				list += (i == 0 ? "" : i + 1 == references.size() ? " and " : ", ") + DescribeAddress(references[i]);
			Note("the corona store at " + DescribeAddress(f.store) + " is referenced in " + (references.size() > kStoreReferences ? std::to_string(kStoreReferences + 1) + " places or more" : std::to_string(references.size()) + " places") + " of the game's code: " + list);
			if (references.size() > kStoreReferences)	// fewer is a fact for the boot: the scan sees a lea or a store of a pointer
				return "the game's code references the corona store at " + DescribeAddress(f.store) + " in " + std::to_string(kStoreReferences + 1) + " places or more, not " + std::to_string(kStoreReferences) + ", as the research found them.";
			const uint8_t* const copied = Rel32Target(f.at[kI4] + kCopyCountAt);
			if (copied != count)
				return "the copy to the draw list reads the count at " + DescribeAddress(copied) + ", not the one right behind the 1088 coronas, at " + DescribeAddress(count) + At(P(kI4)) + ".";
			Note("the copy to the draw list at " + DescribeAddress(f.at[kI5]) + ": its count at " + DescribeAddress(f.at[kI4] + kCopyCountAt) + At(P(kI4)) + ", its store at " + DescribeAddress(f.at[kI5] + kCopyStoreAt) + At(P(kI5)));
			for (size_t i = 0; i < std::size(f.inlined); i++)
			{
				const uint8_t* const read = Rel32Target(f.inlined[i] + kInlinedCountAt);
				if (read != count)
					return std::string(i == 0 ? "the first" : "the second") + " inlined add reads the count at " + DescribeAddress(read) + ", not the one right behind the 1088 coronas, at " + DescribeAddress(count) + " (G-E-G1).";
			}
			Note("the inlined adds' limits at " + DescribeAddress(f.inlined[0] + kInlinedLimitAt) + " and " + DescribeAddress(f.inlined[1] + kInlinedLimitAt) + " hold " + std::to_string(kGameCoronas - 1) + ", their counts the same");

			// the general add
			Note("the general add at " + DescribeAddress(f.at[kI1]) + ": its count at " + Hex(kCountAt) + " of the store at " + DescribeAddress(f.at[kI1] + kAddLoadAt) + " and " + DescribeAddress(f.at[kI1] + kAddExchangeAt) +
				", its limit at " + DescribeAddress(f.at[kI1] + kAddLimitAt) + " holds " + std::to_string(kGameCoronas - 1) + " (N-1, " + P(kI1).label + ") -> game size " + std::to_string(kGameCoronas));

			// the LOD lights' job: what hands the store out, the batch's clamp, its own limit and its scratch
			const uint8_t* const handOut = CallTarget(f.at[kI6] + kJobCallAt);
			if (handOut != f.at[kI7])
				return "the LOD lights' job's call at " + DescribeAddress(f.at[kI6] + kJobCallAt) + " leads to " + DescribeAddress(handOut) + ", not to what hands the store out, at " + DescribeAddress(f.at[kI7]) + " (another mod may have hooked it already)" + At(P(kI6)) + ".";
			if (Scanner::FindCallsTo(f.at[kI7], kHandOutCalls + 1).size() > kHandOutCalls)
				return "the game's code calls what hands the corona store out, at " + DescribeAddress(f.at[kI7]) + ", in " + std::to_string(kHandOutCalls + 1) + " places or more, not " + std::to_string(kHandOutCalls) + At(P(kI7)) + ".";
			Note("the LOD lights' job's parameters at " + DescribeAddress(f.at[kI6]) + At(P(kI6)) + ", their call of what hands the store out at " + DescribeAddress(f.at[kI7]) + At(P(kI7)));
			Note("the LOD lights' batch: its clamp at " + DescribeAddress(f.at[kI8] + kBatchMoveAt) + " and " + DescribeAddress(f.at[kI8] + kBatchCompareAt) + " holds " + std::to_string(kGameCoronas) + At(P(kI8)));
			Note("the LOD lights' job's own limit at " + DescribeAddress(f.at[kI9] + kLodLimitAt) + " holds " + std::to_string(kGameCoronas - 1) + " (N-1, " + P(kI9).label + ") -> game size " + std::to_string(kGameCoronas) + ": left as it is");
			Note("the LOD lights' job's scratch at " + DescribeAddress(f.at[kJ1]) + ": " + Hex(kCountAt) + " bytes of coronas, " + Hex(kScratchLists) + " of lists, " + Hex(kScratchBytes) + " in all" + At(P(kJ1)));

			// the drawing side, which reads the copy: its one count, and every address in the game
			const uint8_t* const pointer = Rel32Target(f.at[kJ2] + kPointerAt);
			const uint8_t* const renderCount = Rel32Target(f.at[kJ2] + kRenderCountAt);
			if (!IsReadable(f.at[kJ2] + kExtrasAt, kRel32))
				return "the draw list's command's store of the extras' count at " + DescribeAddress(f.at[kJ2] + kExtrasAt) + " cannot be read" + At(P(kJ2)) + ".";
			const uint8_t* const extras = Rel32Target(f.at[kJ2] + kExtrasAt);
			if (!IsReadable(f.at[kJ3] + kDrawCountAt, kRel32))
				return "the draw's read of the render count at " + DescribeAddress(f.at[kJ3] + kDrawCountAt) + " cannot be read" + At(P(kJ3)) + ".";
			const uint8_t* const mark = Rel32Target(f.at[kJ3] + kMarkAt);
			const uint8_t* const drawCount = Rel32Target(f.at[kJ3] + kDrawCountAt);
			const struct { const char* what; const uint8_t* at; Site site; } inside[] = {
				{ "draw list's render pointer", pointer, kJ2 }, { "draw list's render count", renderCount, kJ2 }, { "draw list's extras' count", extras, kJ2 }, { "draw's mark", mark, kJ3 } };
			for (const auto& one : inside)
				if (!IsInsideGameImage(one.at))
					return std::string("the ") + one.what + " at " + DescribeAddress(one.at) + At(P(one.site)) + " is not inside the game.";
			if (drawCount != renderCount)
				return "the draw reads the render count at " + DescribeAddress(drawCount) + ", the draw list's command stores it at " + DescribeAddress(renderCount) + At(P(kJ3)) + ".";
			Note("the draw list's command at " + DescribeAddress(f.at[kJ2]) + ": the render pointer at " + DescribeAddress(pointer) + ", the render count at " + DescribeAddress(renderCount) + ", the extras' count at " + DescribeAddress(extras) + At(P(kJ2)));
			Note("the draw at " + DescribeAddress(f.at[kJ3]) + ": its mark at " + DescribeAddress(mark) + ", the render count at " + DescribeAddress(drawCount) + At(P(kJ3)));
			Note("the draw's stack, 0x20 bytes a corona at " + DescribeAddress(f.at[kJ4]) + At(P(kJ4)) + " and 4 bytes a corona at " + DescribeAddress(f.at[kJ5]) + At(P(kJ5)));
			return {};
		}

		// ---- the new store

#ifdef TWEAKSV_TEST_BUILD
		// test builds only: TWEAKSV_TEST_CORONA_MEMORY in the environment reaches what comes of memory that cannot be had or reached (SPECIFICATIONS 13.1): "none" takes
		// the near memory for the store for none, "far" gives the store memory out of reach of the game's code, "no stubs" and "far stubs" do the same to its stubs.
		bool MemorySwitch(const wchar_t* value, const char* words)
		{
			wchar_t text[16]{};
			if (GetEnvironmentVariableW(L"TWEAKSV_TEST_CORONA_MEMORY", text, 16) >= 16 || wcscmp(text, value) != 0)
				return false;
			WriteLog(kTag, std::string("TEST BUILD: ") + words + " (TWEAKSV_TEST_CORONA_MEMORY).");
			return true;
		}
#endif

		uint8_t* MakeStore(size_t bytes)
		{
#ifdef TWEAKSV_TEST_BUILD
			if (MemorySwitch(L"none", "the near memory for the corona store is taken for none"))
				return nullptr;
			if (MemorySwitch(L"far", "the corona store is given memory out of reach"))
				return static_cast<uint8_t*>(VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
#endif
			return static_cast<uint8_t*>(AllocateNearGame(bytes));
		}

		// True if a rel32 or a disp32 that counts from from reaches target.
		bool WithinReach(const uint8_t* from, const uint8_t* target)
		{
			const intptr_t distance = target - from;
			return distance >= INT32_MIN && distance <= INT32_MAX;
		}

		// The bytes of the rel32 at field that leads to target, known to be within reach.
		std::vector<uint8_t> LeadTo(const uint8_t* field, const uint8_t* target)
		{
			return BytesOf(static_cast<int32_t>(target - (field + kRel32)));
		}

		// The 13 writes, in the order of the findings' table; false, with why, if the new store is out of reach of its count. The sites lie in the game with
		// the count, and near memory is 1 GB from the image at most, far more than a rel32 lacks, so that what reaches the one reaches them.
		bool AddWrites(const Found& f, uint8_t* store, uint32_t room, std::vector<CodeWrite>& writes, std::string& why)
		{
			const uint8_t* const count = f.store + kCountAt;
			if (!WithinReach(store, count))
			{
				why = "the new corona store at " + DescribeAddress(store) + " is out of reach of its count at " + DescribeAddress(count) + ".";
				return false;
			}
			const std::vector<uint8_t> fromStore = BytesOf(static_cast<int32_t>(count - store));
			const std::vector<uint8_t> limit = BytesOf(room - 1);
			const std::vector<uint8_t> clamp = BytesOf(room);

			// the nop's 14 bytes: lea rcx, [the new store] and a nop of 7
			const std::vector<uint8_t> lea = LeadTo(f.at[kI1] + kAddLeaRelAt, store);
			std::vector<uint8_t> nop = { 0x48, 0x8D, 0x0D };
			nop.insert(nop.end(), lea.begin(), lea.end());
			nop.insert(nop.end(), { 0x0F, 0x1F, 0x80, 0, 0, 0, 0 });
			writes.push_back({ f.at[kI1], nop, "corona store's lea in its general add" });

			writes.push_back({ f.at[kI1] + kAddLoadAt, fromStore, "corona store's count in its general add's load" });
			writes.push_back({ f.at[kI1] + kAddLimitAt, limit, "corona store's limit in its general add" });
			writes.push_back({ f.at[kI1] + kAddExchangeAt, fromStore, "corona store's count in its general add's exchange" });
			writes.push_back({ f.at[kI2] + kInlinedStoreAt, LeadTo(f.at[kI2] + kInlinedStoreAt, store), "corona store in the first inlined add" });
			writes.push_back({ f.inlined[0] + kInlinedLimitAt, limit, "corona store's limit in the first inlined add" });
			writes.push_back({ f.at[kI3] + kInlinedStoreAt, LeadTo(f.at[kI3] + kInlinedStoreAt, store), "corona store in the second inlined add" });
			writes.push_back({ f.inlined[1] + kInlinedLimitAt, limit, "corona store's limit in the second inlined add" });
			writes.push_back({ f.at[kI5] + kCopyStoreAt, LeadTo(f.at[kI5] + kCopyStoreAt, store), "corona store in the copy to the draw list" });
			writes.push_back({ f.at[kI6] + kJobStoreAt, LeadTo(f.at[kI6] + kJobStoreAt, store), "corona store in the LOD lights' job's parameters" });
			writes.push_back({ f.at[kI7] + kHandOutAddAt, fromStore, "corona store's count in what hands it out" });
			writes.push_back({ f.at[kI8] + kBatchMoveAt, clamp, "corona store's clamp in the LOD lights' batch" });
			writes.push_back({ f.at[kI8] + kBatchCompareAt, clamp, "corona store's comparison in the LOD lights' batch" });
			return true;
		}

		// ---- the counting stubs

		// A refusal counted, then on to where its jump led - 22 bytes, every register as it was, rsp untouched, the counter reached rip-relative.
		constexpr uint8_t kStub[] = {
			0xF0, 0x48, 0xFF, 0x05, 0, 0, 0, 0,		// lock inc qword [rip+disp32]
			0xFF, 0x25, 0, 0, 0, 0,					// jmp qword [rip+0]: on to where the jump led,
			0, 0, 0, 0, 0, 0, 0, 0 };				// whose address follows
		constexpr size_t kCounterDispAt = 4, kCounterEnd = 8, kTargetAt = 14;
		static_assert(sizeof(kStub) == kTargetAt + sizeof(uint64_t), "TweaksV: a counting stub ends in its target");

		// The batch's, with rsi what it brought, edx the new count and ecx the old: what it could not place added, what it brought kept, the six bytes
		// it replaces, and on past them. rax is free, since the moved lea writes it, and nothing after it reads the flags, which the moved sub leaves as ever.
		constexpr uint8_t kBatchStub[] = {
			0x89, 0xF0,								// mov eax, esi
			0x29, 0xD0,								// sub eax, edx
			0x01, 0xC8,								// add eax, ecx: what it could not place, zero-extended
			0xF0, 0x48, 0x01, 0x05, 0, 0, 0, 0,		// lock add qword [rip+disp32], rax
			0x89, 0x35, 0, 0, 0, 0,					// mov dword [rip+disp32], esi
			0x29, 0xCA,								// sub edx, ecx
			0x48, 0x8D, 0x04, 0x49,					// lea rax, [rcx+rcx*2]
			0xFF, 0x25, 0, 0, 0, 0,					// jmp qword [rip+0]: on past the six bytes,
			0, 0, 0, 0, 0, 0, 0, 0 };				// whose address follows
		constexpr size_t kRefusedDispAt = 10, kRefusedEnd = 14, kBroughtDispAt = 16, kBroughtEnd = 20, kMovedAt = 20, kBatchTargetAt = 32, kBatchJumpBytes = 5;
		static_assert(sizeof(kBatchStub) == kBatchTargetAt + sizeof(uint64_t), "TweaksV: the batch's stub ends in its target");
		static_assert(PatternHolds(P(kI8), kBatchSixAt, kBatchStub + kMovedAt, kBatchSix), "TweaksV: the batch's stub holds the six bytes of G-E-I8 it replaces");

		// Where each jump leads on 1.0.1158.16 (PATTERNS.md), up to the first instruction that writes the flags: none before it reads one, so a lock inc on the way
		// changes nothing the game sees.
		constexpr uint8_t kAddWayOut[] = { 0x0F, 0x28, 0x34, 0x24, 0x0F, 0x28, 0x7C, 0x24, 0x10, 0x44, 0x0F, 0x28, 0x44, 0x24, 0x20, 0x48, 0x83, 0xC4, 0x38 };
		constexpr uint8_t kFirstInlinedWayOut[] = { 0x0F, 0x28, 0x74, 0x24, 0x20, 0x0F, 0x28, 0x7C, 0x24, 0x30, 0x44, 0x0F, 0x28, 0x44, 0x24, 0x40, 0x44, 0x0F, 0x28, 0x4C, 0x24, 0x50, 0x48, 0x83, 0xC4, 0x68 };
		constexpr uint8_t kSecondInlinedWayOut[] = { 0x42, 0xC6, 0x44, 0x2E, 0x6B, 0x00, 0x66, 0x90, 0x48, 0xFF, 0xC3, 0x41, 0x8B, 0x47, 0x04, 0x48, 0x39, 0xC3 };
		constexpr uint8_t kLodWayOut[] = { 0x4C, 0x8B, 0x44, 0x24, 0x58, 0x44, 0x0F, 0x28, 0xDC, 0x0F, 0x28, 0xA4, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x41, 0x0F, 0x50, 0xC1, 0x84, 0xC0 };

		// The four jumps: where their rel32 lies, what it is called and what it must lead to; the first three count into one counter, the last into another.
		struct Jump
		{
			uint8_t* rel;
			const char* what;
			const uint8_t* wayOut;
			size_t wayOutSize;
			const char* label;
			uint64_t Counters::* counter;
		};
		constexpr size_t kJumps = 4, kStubSlot = 32, kJumpRelFromStart = 2;

		void NotCounted(const std::string& why)
		{
			Note("the corona store's refused coronas are not counted: " + why);
		}

		// True if a jump leads into the game, to what the findings have there; else logged.
		bool JumpHolds(const Jump& jump)
		{
			const std::string at = "the " + std::string(jump.what) + " at " + DescribeAddress(jump.rel - kJumpRelFromStart);
			const std::string label = std::string(" (") + jump.label + ")";
			if (!IsReadable(jump.rel, kRel32))	// the inlined adds' lies past G-E-G1's match
			{
				NotCounted(at + " cannot be read" + label + ".");
				return false;
			}
			const uint8_t* const target = Rel32Target(jump.rel);
			if (!IsInsideGameImage(target))
			{
				NotCounted(at + " does not lead into the game (another mod may have hooked it already)" + label + ".");
				return false;
			}
			if (!IsReadable(target, jump.wayOutSize))
			{
				NotCounted(at + " leads to " + DescribeAddress(target) + ", which cannot be read" + label + ".");
				return false;
			}
			if (memcmp(target, jump.wayOut, jump.wayOutSize) != 0)
			{
				NotCounted(at + " leads to " + DescribeAddress(target) + ", which holds " + HexBytes(target, jump.wayOutSize) + ", not what the research found there" + label + ".");
				return false;
			}
			Note(at + " -> " + DescribeAddress(target) + ": " + HexBytes(target, jump.wayOutSize) + label);
			return true;
		}

		// A rel32 from just past end to target, both within one block of two pages: always within reach.
		int32_t WithinBlock(const uint8_t* end, const void* target)
		{
			return static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(end));
		}

		// The stubs on the first page of a block near the game, their counters on its second, read/write; the first page read+execute once written, never writable
		// and executable at once. Null, logged, if they cannot be made.
		uint8_t* MakeStubs(const Jump (&jumps)[kJumps], const uint8_t* pastSix, size_t page)
		{
			uint8_t* block = static_cast<uint8_t*>(AllocateNearGame(2 * page));
#ifdef TWEAKSV_TEST_BUILD
			if (block && MemorySwitch(L"no stubs", "the near memory for the corona store's stubs is taken for none"))
			{
				VirtualFree(block, 0, MEM_RELEASE);
				block = nullptr;
			}
			if (block && MemorySwitch(L"far stubs", "the corona store's stubs are given memory out of reach"))
			{
				VirtualFree(block, 0, MEM_RELEASE);
				block = static_cast<uint8_t*>(VirtualAlloc(nullptr, 2 * page, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
			}
#endif
			if (!block)
			{
				NotCounted("no memory near the game could be had for its stubs.");
				return nullptr;
			}
			Counters* const counters = reinterpret_cast<Counters*>(block + page);
			for (size_t i = 0; i < kJumps; i++)
			{
				uint8_t* const stub = block + i * kStubSlot;
				memcpy(stub, kStub, sizeof(kStub));
				const int32_t disp = WithinBlock(stub + kCounterEnd, &(counters->*jumps[i].counter));
				const uint64_t target = reinterpret_cast<uintptr_t>(Rel32Target(jumps[i].rel));
				memcpy(stub + kCounterDispAt, &disp, sizeof(disp));
				memcpy(stub + kTargetAt, &target, sizeof(target));
			}
			uint8_t* const batch = block + kJumps * kStubSlot;
			memcpy(batch, kBatchStub, sizeof(kBatchStub));
			const int32_t refused = WithinBlock(batch + kRefusedEnd, &counters->refused);
			const int32_t brought = WithinBlock(batch + kBroughtEnd, &counters->lodBrought);
			const uint64_t target = reinterpret_cast<uintptr_t>(pastSix);
			memcpy(batch + kRefusedDispAt, &refused, sizeof(refused));
			memcpy(batch + kBroughtDispAt, &brought, sizeof(brought));
			memcpy(batch + kBatchTargetAt, &target, sizeof(target));
			DWORD old = 0;
			if (!VirtualProtect(block, page, PAGE_EXECUTE_READ, &old))
			{
				const DWORD error = GetLastError();
				VirtualFree(block, 0, MEM_RELEASE);
				NotCounted("its stubs could not be made read+execute (" + DescribeError(error) + ").");
				return nullptr;
			}
			FlushInstructionCache(GetCurrentProcess(), block, page);
			return block;
		}

		// The counters, an aid: with DIAGNOSTIC_MODE and not on a dry run, the jumps and the batch through their stubs, added to writes. Whatever stops them is logged and the raise goes on.
		void AddCounters(const Found& f, std::vector<CodeWrite>& writes)
		{
			if (!IsDiagnosticMode())
				return;
			const Jump jumps[kJumps] = {
				{ f.at[kI1] + kAddJumpRelAt, "jump when the corona store is full in its general add", kAddWayOut, sizeof(kAddWayOut), P(kI1).label, &Counters::refused },
				{ f.inlined[0] + kInlinedJumpRelAt, "jump when the corona store is full in the first inlined add", kFirstInlinedWayOut, sizeof(kFirstInlinedWayOut), "G-E-G1", &Counters::refused },
				{ f.inlined[1] + kInlinedJumpRelAt, "jump when the corona store is full in the second inlined add", kSecondInlinedWayOut, sizeof(kSecondInlinedWayOut), "G-E-G1", &Counters::refused },
				{ f.at[kI9] + kLodJumpRelAt, "jump when the LOD lights' job's own coronas are full", kLodWayOut, sizeof(kLodWayOut), P(kI9).label, &Counters::lodRefused } };
			for (const Jump& jump : jumps)
				if (!JumpHolds(jump))
					return;
			uint8_t* const six = f.at[kI8] + kBatchSixAt;
			if (IsDryRun())
			{
				WriteLog(kTag, "DRY_RUN: would redirect the four jumps when the corona store is full and the LOD lights' batch at " + DescribeAddress(six) + At(P(kI8)) + " to counting stubs - the refused coronas are not counted.");
				return;
			}
			const size_t page = PageSize();
			uint8_t* const block = MakeStubs(jumps, six + kBatchSix, page);
			if (!block)
				return;
			const uint8_t* const batch = block + kJumps * kStubSlot;
			bool inReach = WithinReach(six + kBatchJumpBytes, batch);
			for (size_t i = 0; i < kJumps; i++)
				inReach = inReach && WithinReach(jumps[i].rel + kRel32, block + i * kStubSlot);
			if (!inReach)
			{
				VirtualFree(block, 0, MEM_RELEASE);	// no jump leads to it yet
				NotCounted("its stubs are out of reach of the game code.");
				return;
			}
			std::vector<CodeWrite> redirects;
			for (size_t i = 0; i < kJumps; i++)
				redirects.push_back({ jumps[i].rel, LeadTo(jumps[i].rel, block + i * kStubSlot), jumps[i].what });
			std::vector<uint8_t> toBatch = { 0xE9 };
			const std::vector<uint8_t> rel = BytesOf(static_cast<int32_t>(batch - (six + kBatchJumpBytes)));
			toBatch.insert(toBatch.end(), rel.begin(), rel.end());
			toBatch.push_back(0x90);
			redirects.push_back({ six, toBatch, "LOD lights' batch's way to its counting stub" });
			writes.insert(writes.end(), redirects.begin(), redirects.end());
			g_stubMemory = block;
			g_counters.store(reinterpret_cast<const Counters*>(block + page), std::memory_order_release);
			Note("the four jumps when the corona store is full and the LOD lights' batch at " + DescribeAddress(six) + At(P(kI8)) + " are redirected to counting stubs once the part's one write is done.");
		}
	}

	bool FindCoronaStore(uint8_t* firstInlinedAdd, uint8_t* secondInlinedAdd, std::vector<CodeWrite>& writes, std::string& why)
	{
		const uint32_t room = static_cast<uint32_t>(GetNumber(kCoronaStore));
		if (room <= kGameCoronas)
			return true;

		// 1. every site looked for, so that the log names each one missing
		Found found;
		found.inlined[0] = firstInlinedAdd;
		found.inlined[1] = secondInlinedAdd;
		std::string notFound;
		for (size_t i = 0; i < kSites; i++)
			found.at[i] = Scanner::FindOnce(*kSitePatterns[i], notFound);
		if (!notFound.empty())
		{
			why = notFound + ".";
			return false;
		}

		// 2. every site checked, each logged - the first not as the findings have it ends here
		why = SitesFault(found);
		if (!why.empty())
			return false;

		// 3. the new store and the writes; the counters last, since the raise goes on without them
		const size_t bytes = size_t{ room } * kCoronaBytes + kPastRecords;
		uint8_t* const store = MakeStore(bytes);
		if (!store)
		{
			why = "no memory near the game could be had for the new corona store of " + std::to_string(room) + " coronas.";
			return false;
		}
		std::vector<CodeWrite> added;
		if (!AddWrites(found, store, room, added, why))
		{
			VirtualFree(store, 0, MEM_RELEASE);	// nothing points at it: nothing was written
			return false;
		}
		Note("the new corona store at " + DescribeAddress(store) + ": " + std::to_string(room) + " coronas of " + Hex(kCoronaBytes) + " bytes and " + Hex(kPastRecords) + " past them, " + std::to_string(bytes) + " bytes");
		AddCounters(found, added);
		writes.insert(writes.end(), added.begin(), added.end());
		if (IsDryRun())
		{
			VirtualFree(store, 0, MEM_RELEASE);	// a dry run writes nothing, so nothing points at it
			WriteLog(kTag, "DRY_RUN: would move the corona store (5 references) and raise its limit from " + std::to_string(kGameCoronas) + " to " + std::to_string(room) + " (5 sites), with 3 offsets of its count.");
			return true;
		}
		g_storeMemory = store;
		g_room.store(room, std::memory_order_release);
		return true;
	}

	void DiscardCoronaStore()
	{
		g_room.store(0, std::memory_order_release);
		g_counters.store(nullptr, std::memory_order_release);
		if (g_storeMemory)
			VirtualFree(g_storeMemory, 0, MEM_RELEASE);
		if (g_stubMemory)
			VirtualFree(g_stubMemory, 0, MEM_RELEASE);
		g_storeMemory = nullptr;
		g_stubMemory = nullptr;
	}

	CoronaStoreNumbers CoronaStoreNow()
	{
		CoronaStoreNumbers numbers = { kGameCoronas, false, 0, 0, 0 };
		const uint32_t room = g_room.load(std::memory_order_acquire);
		if (room == 0 || !LightStoresRaised())
			return numbers;
		numbers.room = room;
		if (const Counters* const counters = g_counters.load(std::memory_order_acquire))
		{
			numbers.counted = true;
			numbers.refused = Load<uint64_t>(&counters->refused);
			numbers.lodRefused = Load<uint64_t>(&counters->lodRefused);
			numbers.lodBrought = Load<uint32_t>(&counters->lodBrought);
		}
		return numbers;
	}
}

#ifdef TWEAKSV_TEST_BUILD
// test builds only: the accessor, for the fake game's report (SPECIFICATIONS 13.1): room, counted, refused, the LOD lights' job's own refused, its last batch.
extern "C" __declspec(dllexport) void TweaksVTestCoronaStoreNow(uint64_t numbers[5])
{
	const TweaksV::CoronaStoreNumbers now = TweaksV::CoronaStoreNow();
	numbers[0] = now.room;
	numbers[1] = now.counted ? 1 : 0;
	numbers[2] = now.refused;
	numbers[3] = now.lodRefused;
	numbers[4] = now.lodBrought;
}
#endif
