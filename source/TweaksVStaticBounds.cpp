// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part (see TweaksVStaticBounds.h).
//
// The game keeps the static collision bounds of the map - every building, prop and interior placed in it, a heavy
// map mod's MLOs and expansions among them - in one pool, the pool named StaticBounds, and a full pool ends the game in
// an access violation. The game makes its pools through two functions (PATTERNS.md, "StaticBounds"): the set-up
// (C-E-P1), which takes the pool, its name, a third argument, the stride and the allocator, and the allocator (C-E-A1),
// which takes the pool, whose capacity is set by then - the size gameconfig.xml gives it, else the game's default - and
// allocates its room. STATICBOUNDS_SIZE in [STATICBOUNDS], 1 to 100000, 16000 by default, is the least the pool gets:
// the set-up's hook notes a pool of stride 16 named StaticBounds, and the allocator's hook raises the noted pool's
// capacity, its stride checked again, to the key before the game's allocator runs - so a gameconfig.xml that gives the pool
// more keeps it - and logs the sizes once. Both functions are hooked in DllMain, all or none, and each match is its
// function's start, which its pattern pins: the set-up's down to its ret, the allocator's from its pushes on.
// Both hooks stay on, so that a pool made a second time is raised as well, and no thread of the game is frozen in the
// middle of its start to take them off. The set-up's hook tests the stride before it touches the name, the allocator's
// compares one pointer, and neither touches anything else of a pool that is not the static bounds pool.
// The hooks raise the pool only if the game makes it after TweaksV's DllMain: a thread of the part checks, 60 seconds
// after the session's start - the first script program the game places, which the script programs' hook sees: the Blips
// part's, which the part joins, or while TweaksV.ini switches Blips off the part's own; the game makes its pools as the
// session starts - that the game has made the pool, and reports a problem, with the part's box, when it has not: the game
// made the pool before TweaksV started, or makes it another way. While a plugin that does the Blips part's job is
// installed, whatever TweaksV.ini says, no check is made, logged. A check that cannot be made - its events, the hook or its
// thread not there - is reported, the hooks staying: nothing else would say when the game does not make the pool.
// A size above the key's default on a pool memory budget not raised - the HeapAdjuster part off, by TweaksV.ini or a plugin
// found, or POOL_MEMORY at the game's own - is a problem of the start-up summary, the raise made all the same.
//   DRY_RUN          nothing is hooked, nothing checked; the line says what would be
//   DIAGNOSTIC_MODE  the two sites at their RVAs, with what IsFunctionStart says of each, which refuses nothing since
//                    the patterns pin it; every making of the pool, and its time after the part's start
// Written for TweaksV from the Static Bounds Limit Adjuster v1.0.2 by Nochala (3_Washing_Machines), commit 61507bc:
//   Copyright (C) 2026 Nochala
//   License: GPL-3.0-or-later - https://github.com/Nochala/StaticboundsLimitAdjuster (THIRD_PARTY_NOTICES.md)
// Adapted for TweaksV on 2026-11-02: its two patterns of the game's code, of the set-up and of the allocator, its two
// hooks on them with the pool told by its name and stride, its offsets of the capacity and the stride, its rule - the larger
// of the two sizes - and its key, STATICBOUNDS_SIZE, with its default and its maximum. Its other routes, the size lookup's
// call and its hook on the lookup, are left out, as is its taking the hooks off after the first allocation: a pattern
// missing or a hook not made is reported, as every part does.

#include "TweaksVStaticBounds.h"

#include <windows.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVScripts.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "StaticBounds";

		// ---- the two functions a pool is made through (PATTERNS.md, "StaticBounds")

		constexpr const GamePattern& kSetUpPattern = Pattern("C-E-P1");
		constexpr const GamePattern& kAllocatorPattern = Pattern("C-E-A1");
		constexpr const char* kSetUpWhat = "the pools' set-up";
		constexpr const char* kAllocatorWhat = "the pools' allocator";

		constexpr size_t kCapacityAt = 0x18;		// a pool's capacity, an int32
		constexpr size_t kStrideAt = 0x1C;			// a pool's stride, an int32
		constexpr uint32_t kStride = 16;			// the static bounds pool's
		constexpr char kName[] = "StaticBounds";	// its name, as the game hands it to the set-up

		// Checked when TweaksV is compiled: the offsets are the bytes the patterns pin - the set-up's mov [rax+0x1C], r9d (the
		// stride, its fourth argument) and mov dword [rax+0x18], 0, its ret; the allocator's movsxd of both.
		static_assert(PatternHolds(kSetUpPattern, 11, { 0x44, 0x89, 0x48, kStrideAt }) && PatternHolds(kSetUpPattern, 22, { 0xC7, 0x40, kCapacityAt }) &&
			PatternHolds(kSetUpPattern, 36, { 0xC3 }) && PatternByte(kSetUpPattern, 37) == -1, "TweaksV: C-E-P1 is the set-up, down to its ret");
		static_assert(PatternHolds(kAllocatorPattern, 9, { 0x48, 0x63, 0x51, kCapacityAt }) && PatternHolds(kAllocatorPattern, 13, { 0x48, 0x63, 0x79, kStrideAt }),
			"TweaksV: C-E-A1 reads the capacity and the stride at these offsets");

		// The set-up's mov rcx, [rsp+disp8], its allocator: the disp8 is the pattern's one wildcard, and the hook hands on five
		// arguments, so the job is done only where the set-up reads its fifth from the stack slot of a fifth.
		constexpr size_t kAllocatorSlotAt = 7;
		constexpr uint8_t kAllocatorSlot = 0x28;
		static_assert(PatternHolds(kSetUpPattern, 3, { 0x48, 0x8B, 0x4C, 0x24 }) && PatternByte(kSetUpPattern, kAllocatorSlotAt) == -1,
			"TweaksV: C-E-P1 reads the allocator from the stack at a byte it leaves open");

		// The job not done, for the start-up summary.
		void Fail(const std::string& why)
		{
			ReportFailure(kTag, "The static bounds pool's size is not raised this session: " + why);
		}

		// ---- the hooks' state, set before they go on

		// The allocator's four argument registers are handed on as they came, whatever it reads of them: its pattern's
		// call of the memory allocator loads no r8 and no r9 of its own (C-E-A1, +38).
		using SetUpFn = uintptr_t (*)(void* pool, const char* name, uintptr_t third, uint32_t stride, void* allocator);
		using AllocatorFn = uintptr_t (*)(void* pool, uintptr_t second, uintptr_t third, uintptr_t fourth);
		SetUpFn g_setUp = nullptr;					// the game's functions, behind the hooks
		AllocatorFn g_allocator = nullptr;
		std::atomic<void*> g_noted{ nullptr };		// the static bounds pool the set-up saw last, until another takes its place
		int32_t g_size = 0;							// STATICBOUNDS_SIZE
		bool g_diagnostic = false;					// DIAGNOSTIC_MODE, read once at the start: the hooks never read a switch
		ULONGLONG g_start = 0;						// the part's start, for the time of each making
		std::atomic<uint64_t> g_makingLines{ 0 };	// the lines of the makings, up to the cap
		std::atomic<bool> g_sizesSaid{ false };		// the pool's sizes are said once
		std::atomic<bool> g_noSizeSaid{ false };	// a capacity below 0 is reported once
		std::atomic<bool> g_strideSaid{ false };	// a stride that changed is reported once
		std::atomic<bool> g_afterAllSaid{ false };	// a making after the check's report is said once

		// ---- the check that the game makes the pool: whichever comes first, the game's first allocation of the noted pool or
		// the check's end, takes the state - a making after the check has reported is said once, with the sizes

		constexpr DWORD kCheckSeconds = 60;			// the check's time after the session's start, by when the game has made its pools
		enum : int { kWaiting, kMade, kReported };
		std::atomic<int> g_check{ kWaiting };
		HANDLE g_madeEvent = nullptr;				// set at the first allocation of the noted pool, so that the check's thread ends early
		HANDLE g_sessionEvent = nullptr;			// set at the session's start: the first script program the game places
		std::atomic<bool> g_sessionStarted{ false };
		ULONGLONG g_sessionTick = 0;				// when the session started, written before its event is set
		DWORD g_checkSeconds = kCheckSeconds;

		// "1 second", "60 seconds"
		std::string Seconds(DWORD n)
		{
			return std::to_string(n) + (n == 1 ? " second" : " seconds");
		}

		// The check's time after the session's start: 60 seconds - in test builds TWEAKSV_TEST_STATICBOUNDS_CHECK=<seconds> in the
		// environment (1..60) instead, so that a suite sees the check within the minute a fake game runs, said once.
		DWORD CheckSeconds()
		{
#ifdef TWEAKSV_TEST_BUILD
			wchar_t text[8]{};
			const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_STATICBOUNDS_CHECK", text, 8);
			if (length > 0 && length < 8)
			{
				wchar_t* end = nullptr;
				const long seconds = wcstol(text, &end, 10);
				if (end && *end == L'\0' && seconds >= 1 && seconds <= static_cast<long>(kCheckSeconds))
				{
					WriteLog(kTag, "TEST BUILD: the static bounds check comes " + Seconds(static_cast<DWORD>(seconds)) + " after the session's start instead of " +
						Seconds(kCheckSeconds) + " (TWEAKSV_TEST_STATICBOUNDS_CHECK=" + std::to_string(seconds) + ").");
					return static_cast<DWORD>(seconds);
				}
			}
#endif
			return kCheckSeconds;
		}

		// One of the check's two events, "made" or "session", or nullptr - in test builds also while
		// TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=<its name> in the environment, said once, so that a suite reaches a check that
		// cannot be made.
		HANDLE MakeEvent(const char* name)
		{
#ifdef TWEAKSV_TEST_BUILD
			char text[8]{};
			const DWORD length = GetEnvironmentVariableA("TWEAKSV_TEST_STATICBOUNDS_NO_EVENT", text, 8);
			if (length > 0 && length < 8 && strcmp(text, name) == 0)
			{
				WriteLog(kTag, std::string("TEST BUILD: the static bounds check's ") + name + " event is not made (TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=" + name + ").");
				return nullptr;
			}
#else
			(void)name;
#endif
			return CreateEventW(nullptr, TRUE, FALSE, nullptr);
		}

		// The check not made, for the start-up summary: the hooks stay, and nothing says whether the game makes the pool.
		void CheckNotDone(const std::string& why)
		{
			ReportFailure(kTag, "The check that the game makes the static bounds pool is not done this session: " + why);
		}

		// The script programs' hook calls this for every program the game places, on the game's thread: the first is the
		// session's start, which starts the check's clock. Nothing more is done here.
		void* SessionStarts(uint8_t*, uint32_t)
		{
			if (!g_sessionStarted.load(std::memory_order_relaxed) && !g_sessionStarted.exchange(true))
			{
				g_sessionTick = GetTickCount64();
				SetEvent(g_sessionEvent);
			}
			return nullptr;
		}

		// The check's thread: from the session's start, until the game has made the pool or for the check's time - then, if it
		// has still not, the problem, after the start-up summary in a box of its own. A session that never starts leaves it
		// waiting. Nothing leaves the thread, which drops the reference to the module StartModuleThread handed it as it ends.
		DWORD WINAPI CheckMade(LPVOID module)
		{
			WaitForSingleObject(g_sessionEvent, INFINITE);
			try
			{
				if (g_diagnostic)
					WriteLog(kTag, "DIAGNOSTIC: the session started " + std::to_string(g_sessionTick - g_start) + " ms after the part's start (the first script "
						"program placed): the static bounds check's clock runs.");
			}
			catch (...)
			{
			}
			WaitForSingleObject(g_madeEvent, g_checkSeconds * 1000);
			int waiting = kWaiting;
			if (g_check.compare_exchange_strong(waiting, kReported))
			{
				try
				{
					if (g_noted.load(std::memory_order_acquire))
						Fail("the game set the pool up but did not allocate it through " + std::string(kAllocatorWhat) + " within " + Seconds(g_checkSeconds) +
							" of the session's start, and the pool keeps the game's size.");
					else
						Fail("the game did not make the pool within " + Seconds(g_checkSeconds) + " of the session's start - it made the pool before TweaksV "
							"started, or makes it another way, and the pool keeps the game's size.");
				}
				catch (...)
				{
				}
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// ---- the hooks

		// A line of a making with DIAGNOSTIC_MODE, up to the cap: the set-up and the allocation are one kind of line.
		void MakingLine(const std::string& what, const void* pool)
		{
			const uint64_t line = g_makingLines.fetch_add(1, std::memory_order_relaxed) + 1;
			if (line <= kMaxLinesPerKind)
				WriteLog(kTag, "DIAGNOSTIC: the static bounds pool at " + DescribeAddress(pool) + " " + what + ", " + std::to_string(GetTickCount64() - g_start) +
					" ms after the part's start.");
			else if (line == kMaxLinesPerKind + 1)
				WriteLog(kTag, "DIAGNOSTIC: " + std::to_string(kMaxLinesPerKind) + " lines of the pool's makings written - from now on they are only counted.");
		}

		// True if name, wherever the game's second argument leads, is the pool's name, its terminating zero included. It is
		// copied through CopyMemorySafely, since nothing but the original's word says that the argument is a pointer to a name.
		bool IsPoolName(const char* name)
		{
			char copy[sizeof(kName)];
			return CopyMemorySafely(copy, name, sizeof(copy)) && memcmp(copy, kName, sizeof(copy)) == 0;
		}

		// The set-up's hook, on the game's thread, for every pool the game makes: the static bounds pool is noted; a set-up of
		// anything else at the noted address ends the note, since the address holds another pool from now on. Nothing may
		// throw out of here, and the set-up's result and the last error are the game's.
		uintptr_t SetUpHook(void* pool, const char* name, uintptr_t third, uint32_t stride, void* allocator)
		{
			if (stride == kStride && IsPoolName(name))
			{
				g_noted.store(pool, std::memory_order_release);
				if (g_diagnostic)
				{
					const DWORD lastError = GetLastError();
					try
					{
						MakingLine("is set up", pool);
					}
					catch (...)
					{
					}
					SetLastError(lastError);
				}
			}
			else
			{
				void* noted = pool;
				g_noted.compare_exchange_strong(noted, nullptr, std::memory_order_acq_rel);	// one step: another thread may note a pool meanwhile
			}
			return g_setUp(pool, name, third, stride, allocator);
		}

		// The noted pool, as the game hands it to its allocator, on the game's thread: its capacity raised to STATICBOUNDS_SIZE,
		// its stride checked again, before the game's allocator runs, what happened logged before it runs too, so that the log
		// holds it should the game's allocation fail. Both fields are read and written as the game's own code does
		// right before and right after: the game's set-up has written them, its allocator reads them next. The first
		// allocation ends the check. Nothing may throw out of here, and the last error is the game's.
		void RaisePool(uint8_t* pool)
		{
			const DWORD lastError = GetLastError();
			volatile int32_t& capacityField = *reinterpret_cast<volatile int32_t*>(pool + kCapacityAt);
			const int32_t stride = *reinterpret_cast<const volatile int32_t*>(pool + kStrideAt);
			const int32_t game = capacityField;
			const bool strideKept = stride == static_cast<int32_t>(kStride);
			const int32_t size = strideKept && game >= 0 && game < g_size ? g_size : game;
			if (size != game)
				capacityField = size;
			int checked = kWaiting;
			if (g_check.compare_exchange_strong(checked, kMade) && g_madeEvent)
				SetEvent(g_madeEvent);
			try
			{
				if (g_diagnostic)
					MakingLine("is allocated with " + std::to_string(size) + " slots, the game's " + std::to_string(game), pool);
				if (checked == kReported && !g_afterAllSaid.exchange(true))
					WriteLog(kTag, "The game made the static bounds pool after all, after the check had reported it missing.");
				if (!strideKept)
				{
					if (!g_strideSaid.exchange(true))
						Fail("the pool at " + DescribeAddress(pool) + " holds the stride " + std::to_string(stride) + " when the game allocates it, not the " +
							std::to_string(kStride) + " it was set up with - it is left as the game made it.");
				}
				else if (game < 0)
				{
					if (!g_noSizeSaid.exchange(true))
						Fail("the game gives the static bounds pool the size " + std::to_string(game) + ", which is none - left as the game gave it.");
				}
				else if (!g_sizesSaid.exchange(true))
				{
					if (size != game)
						WriteLog(kTag, "The size the game gives the static bounds pool, " + std::to_string(game) + ", is raised to " + std::to_string(size) +
							" (STATICBOUNDS_SIZE).");
					else
						WriteLog(kTag, "The size the game gives the static bounds pool, " + std::to_string(game) + ", is kept: it is not below STATICBOUNDS_SIZE=" +
							std::to_string(g_size) + ".");
				}
			}
			catch (...)
			{
			}
			SetLastError(lastError);
		}

		// The allocator's hook, on the game's thread, for every pool the game makes.
		uintptr_t AllocatorHook(void* pool, uintptr_t second, uintptr_t third, uintptr_t fourth)
		{
			if (pool && pool == g_noted.load(std::memory_order_acquire))
				RaisePool(static_cast<uint8_t*>(pool));
			return g_allocator(pool, second, third, fourth);
		}

		// ---- what the raise spends

		// A slot takes 17 bytes of the pool memory budget (PATTERNS.md, "A pool's memory"), which only the HeapAdjuster part raises, and past that
		// budget the game writes through a null pointer, with nothing on screen. A size above the key's default on a budget not raised is one
		// problem of the start-up summary, the raise made all the same; at the default nothing, since a box at every start would say little.
		void CheckBudget()
		{
			const bool partOff = !IsFeatureOn(kHeapSection);
			if (g_size <= kStaticBoundsSize.defaultValue || (!partOff && GetNumber(kPoolMemory) != kPoolMemory.minValue))
				return;
			const std::string above = "STATICBOUNDS_SIZE=" + std::to_string(g_size) + " is above its default of " + std::to_string(kStaticBoundsSize.defaultValue) + ", and ";
			ReportFailure(kTag, "The static bounds pool may not fit the pool memory budget this session: " + above + (partOff ?
				"the HeapAdjuster part, which raises that budget, is off. A pool past the budget crashes the game: switch the part on, or lower STATICBOUNDS_SIZE." :
				"POOL_MEMORY=" + std::to_string(kPoolMemory.minValue) + " leaves that budget at the game's own. A pool past the budget crashes the game: raise POOL_MEMORY, or lower STATICBOUNDS_SIZE."));
		}
	}

	void InitStaticBounds()
	{
		g_start = GetTickCount64();
		g_diagnostic = IsDiagnosticMode();

		// 1. both patterns looked for, so that the log names each one missing
		std::string notFound;
		uint8_t* const setUp = Scanner::FindOnce(kSetUpPattern, notFound);
		uint8_t* const allocator = Scanner::FindOnce(kAllocatorPattern, notFound);
		if (!setUp || !allocator)
		{
			Fail(notFound + ".");
			return;
		}

		// 2. each match is its function's start, which its pattern pins: what IsFunctionStart says is a fact for the log
		if (g_diagnostic)
		{
			std::string detail;
			IsFunctionStart(setUp, detail);
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kSetUpWhat + " at " + DescribeAddress(setUp) + At(kSetUpPattern) + ": " + detail);
			IsFunctionStart(allocator, detail);
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kAllocatorWhat + " at " + DescribeAddress(allocator) + At(kAllocatorPattern) + ": " + detail);
		}

		// the set-up reads its allocator where the hook hands it on, the one byte its pattern leaves open
		if (setUp[kAllocatorSlotAt] != kAllocatorSlot)
		{
			Fail(std::string(kSetUpWhat) + " at " + DescribeAddress(setUp) + " reads its allocator from the stack at +" + Hex(setUp[kAllocatorSlotAt]) +
				", not at +" + Hex(kAllocatorSlot) + ", the slot of a fifth argument, the last the hook hands on" + At(kSetUpPattern) + ".");
			return;
		}

		g_size = GetNumber(kStaticBoundsSize);
		const std::string where = std::string(kSetUpWhat) + " at " + DescribeAddress(setUp) + " and " + kAllocatorWhat + " at " + DescribeAddress(allocator);
		if (IsDryRun())
		{
			WriteLog(kTag, "DRY_RUN: would hook " + where + " so that the static bounds pool gets at least " + std::to_string(g_size) +
				" slots (STATICBOUNDS_SIZE) when the game makes it.");
			return;
		}

		// 3. both hooked, all or none - the check's event made first, so that the game's first allocation finds it
		g_madeEvent = MakeEvent("made");
		const std::vector<FunctionHook> hooks = {
			{ setUp, reinterpret_cast<const void*>(&SetUpHook), reinterpret_cast<void**>(&g_setUp), kSetUpWhat },
			{ allocator, reinterpret_cast<const void*>(&AllocatorHook), reinterpret_cast<void**>(&g_allocator), kAllocatorWhat } };
		if (!HookFunctions(hooks, kTag))
		{
			if (g_madeEvent)
				CloseHandle(g_madeEvent);
			g_madeEvent = nullptr;
			Fail(where + " could not be hooked (details in TweaksV.log).");
			return;
		}
		WriteLog(kTag, Capitalised(where) + " are hooked: the static bounds pool gets at least " + std::to_string(g_size) + " slots (STATICBOUNDS_SIZE) when the "
			"game makes it, more where the game gives it more.");
		CheckBudget();

		// 4. the check, from the session's start - the first script program the game places, seen through the script programs'
		// hook: the part joins the hook the Blips part has made while Blips is on, and makes it itself, the first part to use
		// it, while TweaksV.ini switches Blips off. While a plugin that does Blips' job on the same script programs is installed
		// - BlipLimitFix.asi or Resource Extensions, closed source, as the conflict check found them, whatever TweaksV.ini
		// says of Blips - the part makes no hook and leaves the script programs to it, logged. A check that cannot be made
		// otherwise - its events, the hook or its thread not there - is reported, the hooks staying
		if (IsPluginInstalledFor(kBlipsSection))
		{
			WriteLog(kTag, "The static bounds check is not made this session: it starts with the session, which TweaksV sees through the script programs' hook, and a "
				"plugin that does the Blips part's job on the script programs is installed (named above) - TweaksV leaves the script programs to it.");
			return;
		}
		g_sessionEvent = MakeEvent("session");
		if (!g_madeEvent || !g_sessionEvent)
		{
			CheckNotDone("its events could not be made.");
			return;
		}
		std::string why;
		if (!Scripts::AddUser({ kTag, &SessionStarts, nullptr }, why))
		{
			CheckNotDone(why);
			return;
		}
		g_checkSeconds = CheckSeconds();
		if (!StartModuleThread(&CheckMade, "static bounds check"))
		{
			CheckNotDone("its thread could not be started (details in TweaksV.log).");
			return;
		}
		WriteLog(kTag, "The static bounds check waits for the session's start - the first script program the game places - and then " +
			Seconds(g_checkSeconds) + " for the game to make the pool.");
	}
}
