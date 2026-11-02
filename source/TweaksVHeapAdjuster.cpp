// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the heap adjuster (see TweaksVHeapAdjuster.h).
//
// The game sizes its memory heap with one immediate in its start-up code: the imm32 of a "mov edx,
// imm32" (H-E1 matches the instruction; the size, in bytes, is at +1). HEAP_SIZE in [HEAP_ADJUSTER]
// sets it in MB, from the size the game holds up to 2047, the most whole MB that imm32 holds as a
// positive int32; a value below the game's own is raised to it and reported. The instruction and the
// value it holds are checked before the one write. A heap that is not the game's own - 1024 MB or more,
// up to the 4095 MB the imm32 holds when the mov zero-extends it - was set by another heap adjuster and
// is left as it is, logged: what another mod has already changed, TweaksV does not change again
// (README, "Other mods").
// A pool of the game's does not draw on that heap: its memory comes from an allocator with a budget of
// its own, 136 MB, past which an allocation returns null and the game writes through it - the crash at
// RVA 0x15A514 (PATTERNS.md, "A pool's memory"). The same start-up code hands it over as the imm32 of a "mov
// r9d, imm32" (H-E2; the budget, in bytes, is at +22), and POOL_MEMORY sets it in MB, from the game's own
// up to 2047, written as the heap's is and when: a ceiling on a counter, which allocates nothing itself.
// Any other value found there is another mod's, left as it is, logged. With DIAGNOSTIC_MODE a thread reads
// the allocator's use and its least room so far, 10 seconds in and then every minute while they change.
// Written for TweaksV; the pattern and the idea of the heap come from HeapAdjusterEnhanced by Chiheb-Bacha,
// which builds on the CitizenFX Collective's heap adjuster as updated by Dilapidated (CREDITS.md); the
// budget from the findings of TweaksV's own research in the game's code (PATTERNS.md).

#include "TweaksVHeapAdjuster.h"

#include <windows.h>

#include <cstdint>
#include <cstring>
#include <string>

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
		constexpr const char* kTag = "HeapAdjuster";
		constexpr const char* kBudgetJob = "The pool memory budget is not changed this session: ";
		constexpr const char* kNotMeasured = "DIAGNOSTIC: the pool memory is not measured: ";

		constexpr uint32_t kMegabyte = 1024 * 1024;
		constexpr int kMaxHeapMb = 2047;		// the most whole MB the game's imm32 holds as a positive int32
		static_assert(kMaxHeapMb == kHeapSize.maxValue, "TweaksV: HEAP_SIZE's maximum must be the imm32's");
		constexpr int kAdjustedFromMb = 1024;	// a heap of this size or more is not the game's own: another adjuster set it

		// H-E2 (PATTERNS.md, "A pool's memory"): "lea rcx, [rip+disp32]", the allocator (its disp32 at +3); "lea rdx, [rip+disp32]", the
		// heap it takes small blocks from; "mov r8d, 0x10000", the block it counts as big; "mov r9d, imm32", its budget in bytes (at
		// +22); "call rel32", its constructor.
		constexpr const GamePattern& kBudgetSetUp = Pattern("H-E2");
		constexpr size_t kAllocatorAt = 3, kBudgetAt = 22;
		static_assert(PatternHolds(kBudgetSetUp, 0, { 0x48, 0x8D, 0x0D }) && PatternHolds(kBudgetSetUp, kBudgetAt - 2, { 0x41, 0xB9 }) &&
			PatternHolds(kBudgetSetUp, kBudgetAt + 4, { 0xE8 }), "TweaksV: H-E2 holds the allocator's lea, the budget's mov at +20 and the constructor's call after it");
		constexpr int kGameBudgetMb = 136;		// the game's own (1.0.1158.16)
		constexpr uint32_t kGameBudget = kGameBudgetMb * kMegabyte;
		static_assert(kGameBudgetMb == kPoolMemory.minValue && kMaxHeapMb == kPoolMemory.maxValue,
			"TweaksV: POOL_MEMORY goes from the game's own budget to the most whole MB the game's imm32 holds as a positive int32");

		// Logs why for the start-up summary; the caller then leaves the game as it is.
		void Fail(const std::string& why)
		{
			ReportFailure(kTag, "The heap size is not changed this session: " + why);
		}

		void AdjustHeap()
		{
			// 1. the site: the imm32 of "mov edx, imm32" (BA imm32), one byte into the match
			std::string notFound;
			uint8_t* const match = Scanner::FindOnce(Pattern("H-E1"), notFound);
			if (!match)
			{
				Fail(notFound + ".");
				return;
			}
			uint8_t* const site = match + 1;
			if (match[0] != 0xBA)
			{
				Fail("the game code found is not the expected instruction (" + HexBytes(match, 5) + ").");
				return;
			}

			// 2. the size the game holds: a whole number of MB - up to 4095, as the mov zero-extends its imm32;
			// 1024 MB or more is another heap adjuster's, whatever it is
			uint32_t bytes = 0;
			memcpy(&bytes, site, sizeof(bytes));
			if (bytes == 0 || bytes % kMegabyte != 0)
			{
				Fail("the game code found holds an unexpected value (" + Hex32(bytes) + ").");
				return;
			}
			const int found = static_cast<int>(bytes / kMegabyte);
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: heap size at " + DescribeAddress(site) + " holds " + std::to_string(found) + " MB");
			if (found >= kAdjustedFromMb)
			{
				WriteLog(kTag, "The heap size is not changed this session: another heap adjuster already set it to " + std::to_string(found) +
					" MB - left as it is.");
				return;
			}

			// 3. the size to use: [the size found, 2047 MB] - smaller would shrink the heap; a value outside is
			// reported, so that the start-up summary names the size really used
			const int requested = GetNumber(kHeapSize);
			int size = requested;
			if (size < found)
				size = found;
			if (size > kMaxHeapMb)
				size = kMaxHeapMb;
			if (size != requested)
				ReportFailure(kTag, "TweaksV.ini: HEAP_SIZE=" + std::to_string(requested) + " is outside " + std::to_string(found) + ".." +
					std::to_string(kMaxHeapMb) + " - using " + std::to_string(size) + ".");
			if (size == found)
			{
				WriteLog(kTag, "Heap size is " + std::to_string(found) + " MB already (HEAP_SIZE=" + std::to_string(requested) + ") - nothing to patch.");
				return;
			}

			// 4. one write; the arithmetic is 64-bit, as 2047 MB only just fits the imm32
			const uint32_t newBytes = static_cast<uint32_t>(static_cast<uint64_t>(size) * kMegabyte);
			const std::string change = std::to_string(found) + " MB -> " + std::to_string(size) + " MB";
			if (!WriteCode({ { site, BytesOf(newBytes), "heap size (" + change + ")" } }, kTag))
			{
				Fail("the game code could not be made writable (details in TweaksV.log).");
				return;
			}
			if (!IsDryRun())
				WriteLog(kTag, "Heap size patched: " + change + ".");
		}

		// "136 MB", or the bytes themselves for a value that is no whole number of MB - for what another mod may have put there.
		std::string Budget(uint32_t bytes)
		{
			return bytes % kMegabyte == 0 ? std::to_string(bytes / kMegabyte) + " MB" : Hex32(bytes) + " bytes";
		}

		// The budget raised in one write; the budget's imm32, found once for this part and the ped pool's measurement, the caller's.
		void RaisePoolMemory(uint8_t* match, const std::string& notFound)
		{
			if (!match)
			{
				ReportFailure(kTag, kBudgetJob + notFound + ".");
				return;
			}

			// 1. the budget the game holds: its own 136 MB, or another mod's, whatever it is
			uint8_t* const site = match + kBudgetAt;
			const uint32_t bytes = ValueAt<uint32_t>(site);
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: pool memory budget at " + DescribeAddress(site) + " holds " + Budget(bytes) + ", its allocator at " +
					DescribeAddress(Rel32Target(match + kAllocatorAt)) + At(kBudgetSetUp));
			if (bytes != kGameBudget)
			{
				WriteLog(kTag, std::string(kBudgetJob) + "another mod already set it to " + Budget(bytes) + " - left as it is.");
				return;
			}

			// 2. the budget to use, within the key's limits: its minimum is the game's own
			const int size = GetNumber(kPoolMemory);
			if (size == kGameBudgetMb)
			{
				WriteLog(kTag, "Pool memory budget is " + std::to_string(kGameBudgetMb) + " MB already (POOL_MEMORY=" + std::to_string(size) + ") - nothing to patch.");
				return;
			}

			// 3. one write; the arithmetic is 64-bit, as 2047 MB only just fits the imm32
			const uint32_t newBytes = static_cast<uint32_t>(static_cast<uint64_t>(size) * kMegabyte);
			const std::string change = std::to_string(kGameBudgetMb) + " MB -> " + std::to_string(size) + " MB";
			if (!WriteCode({ { site, BytesOf(newBytes), "pool memory budget (" + change + ")" } }, kTag))
			{
				ReportFailure(kTag, std::string(kBudgetJob) + "the game code could not be made writable (details in TweaksV.log).");
				return;
			}
			if (!IsDryRun())
				WriteLog(kTag, "Pool memory budget patched: " + change + ".");
		}

		// ---- the measurement (DIAGNOSTIC_MODE)

		// The allocator, as its constructor and its allocation lay it out (PATTERNS.md, "A pool's memory"): its budget, what is in use,
		// the room left and the least room so far, 64 bits each; the blocks it has given out, a word. Its fields are the game's data,
		// which the game's threads write under its lock and this reads without: each read whole, the whole not all of one moment.
		constexpr size_t kAllocatorBudgetAt = 0x20, kUsedAt = 0x28, kRoomAt = 0x30, kLeastRoomAt = 0x38, kBlocksAt = 0x40;
		constexpr int kFatalBlocks = 900;		// the allocator traps the game at this many blocks
		constexpr uint64_t kNearlySpent = kMegabyte;

		// What the site leads to, set before the measurement's thread starts.
		const uint8_t* g_budgetSite = nullptr;
		const uint8_t* g_allocator = nullptr;

		// What a summary says, read at once.
		struct Reading
		{
			bool read = false;			// the allocator and the site could be read
			uint64_t budget = 0;		// its field: 0 until the game has set it up
			uint64_t used = 0;
			uint64_t room = 0;
			uint64_t leastRoom = 0;
			uint32_t blocks = 0;
			uint32_t site = 0;			// what the site holds now
		};

		Reading ReadNow()
		{
			Reading reading;
			reading.read = IsReadable(g_budgetSite, sizeof(uint32_t)) && IsReadable(g_allocator, kBlocksAt + sizeof(uint16_t));
			if (reading.read)
			{
				reading.site = ValueAt<uint32_t>(g_budgetSite);
				reading.budget = Load<uint64_t>(g_allocator + kAllocatorBudgetAt);
				reading.used = Load<uint64_t>(g_allocator + kUsedAt);
				reading.room = Load<uint64_t>(g_allocator + kRoomAt);
				reading.leastRoom = Load<uint64_t>(g_allocator + kLeastRoomAt);
				reading.blocks = Load<uint16_t>(g_allocator + kBlocksAt);
			}
			return reading;
		}

		// The summary's state, kept by its thread alone: the reading of the last summary and how many readings had news.
		Reading g_logged;
		uint64_t g_news = 0;

		// Changes with every reading that has something new to say: the first; the budget, what is in use, the least room or the
		// blocks changed.
		uint64_t Numbers()
		{
			const Reading now = ReadNow();
			const bool news = g_news == 0 || now.read != g_logged.read || now.budget != g_logged.budget || now.used != g_logged.used ||
				now.leastRoom != g_logged.leastRoom || now.blocks != g_logged.blocks;
			if (news)
			{
				g_logged = now;
				g_news++;
			}
			return g_news;
		}

		// "134.2" - the bytes in MB, a tenth at most, cut rather than rounded: 1 MB less a byte stays under 1.
		std::string Tenths(uint64_t bytes)
		{
			const uint64_t tenths = ((bytes >> 10) * 10) >> 10;
			return std::to_string(tenths / 10) + "." + std::to_string(tenths % 10);
		}

		// "136" - a budget in MB, the tenth only where it has one.
		std::string BudgetMb(uint64_t bytes)
		{
			return bytes % kMegabyte == 0 ? std::to_string(bytes / kMegabyte) : Tenths(bytes);
		}

		void LogSummary(uint64_t)
		{
			const Reading& now = g_logged;
			if (!now.read)
			{
				WriteLog(kTag, "DIAGNOSTIC: pool memory: its allocator at " + DescribeAddress(g_allocator) + " or its site at " + DescribeAddress(g_budgetSite) + " cannot be read.");
				return;
			}
			if (now.budget == 0)
			{
				WriteLog(kTag, "DIAGNOSTIC: no pool memory yet - the game has not set its allocator up.");
				return;
			}
			if (now.budget != now.site)
				WriteLog(kTag, "DIAGNOSTIC: pool memory: its allocator at " + DescribeAddress(g_allocator) + " holds a budget of " + BudgetMb(now.budget) + " MB, not the " +
					Budget(now.site) + " its site holds: the game set it up before TweaksV started, or its layout is not as the research found it.");
			else if (now.used + now.room != now.budget)
				WriteLog(kTag, "DIAGNOSTIC: pool memory: " + Tenths(now.used) + " MB in use and " + Tenths(now.room) + " MB of room do not add up to its budget of " +
					BudgetMb(now.budget) + " MB: the game may have been changing them, or its layout is not as the research found it.");
			std::string line = "DIAGNOSTIC: pool memory: " + Tenths(now.used) + " of " + BudgetMb(now.budget) + " MB in use, at least " + Tenths(now.leastRoom) +
				" MB free so far, " + std::to_string(now.blocks) + " blocks (" + std::to_string(kFatalBlocks) + " ends the game)";
			if (now.leastRoom < kNearlySpent)
				line += " - nearly spent: a pool made now can crash the game at 0x15A514";
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "pool memory summary", kFirstSummaryMs, &Numbers, &LogSummary };

		// With DIAGNOSTIC_MODE: the allocator checked and the summary's thread started, of the budget as the part leaves it. Nothing of it
		// is reported: an aid (SPECIFICATIONS 12).
		void MeasurePoolMemory(const uint8_t* match, const std::string& notFound)
		{
			if (!match)
			{
				WriteLog(kTag, kNotMeasured + notFound + ".");
				return;
			}
			const uint8_t* const allocator = Rel32Target(match + kAllocatorAt);
			if (!IsInsideGameImage(allocator) || !IsInsideGameImage(allocator + kBlocksAt + sizeof(uint16_t) - 1))
			{
				WriteLog(kTag, kNotMeasured + ("its allocator at " + DescribeAddress(allocator) + " is not inside the game") + At(kBudgetSetUp) + ".");
				return;
			}
			if (reinterpret_cast<uintptr_t>(allocator) % sizeof(uint64_t) != 0)
			{
				WriteLog(kTag, kNotMeasured + ("its allocator at " + DescribeAddress(allocator) + " is not aligned to 8 bytes") + At(kBudgetSetUp) + ".");
				return;
			}
			if (!IsReadable(allocator, kBlocksAt + sizeof(uint16_t)))
			{
				WriteLog(kTag, kNotMeasured + ("its allocator at " + DescribeAddress(allocator) + " cannot be read") + At(kBudgetSetUp) + ".");
				return;
			}
			g_budgetSite = match + kBudgetAt;
			g_allocator = allocator;
			if (!StartMeasurement<kMeasurement>())	// its line is written
			{
				WriteLog(kTag, std::string(kNotMeasured) + "its summary thread could not be started.");
				return;
			}
			WriteLog(kTag, "DIAGNOSTIC: the pool memory is measured, through its allocator at " + DescribeAddress(allocator) + At(kBudgetSetUp) +
				": its use and its least room so far are logged after 10 seconds, then every minute while they change.");
		}

		// H-E2's match, looked for once on the thread of the start-up: the part and the ped pool's measurement each ask for it, and the
		// scanner's line and a miss come once, whichever asks first.
		bool g_budgetLookedFor = false;
		uint8_t* g_budgetMatch = nullptr;
		std::string g_budgetNotFound;

		uint8_t* FindBudgetSetUp()
		{
			if (!g_budgetLookedFor)
			{
				g_budgetLookedFor = true;
				g_budgetMatch = Scanner::FindOnce(kBudgetSetUp, g_budgetNotFound);
			}
			return g_budgetMatch;
		}
	}

	uint8_t* FindPoolMemoryBudget(std::string& why)
	{
		uint8_t* const match = FindBudgetSetUp();
		if (match)
			return match + kBudgetAt;
		if (why.empty())
			why = g_budgetNotFound;
		return nullptr;
	}

	void InitHeapAdjuster()
	{
		AdjustHeap();
		uint8_t* const match = FindBudgetSetUp();
		RaisePoolMemory(match, g_budgetNotFound);
		if (IsDiagnosticMode())
			MeasurePoolMemory(match, g_budgetNotFound);
	}
}
