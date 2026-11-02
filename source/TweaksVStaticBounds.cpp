// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part (see TweaksVStaticBounds.h).
//
// The game keeps the static collision bounds of the map - every building, prop and interior placed in it, a heavy
// map mod's MLOs and expansions among them - in one store, the store named StaticBounds, and a full store ends the game
// at a trap. The store's size is read once, from one call (PATTERNS.md, "StaticBounds"): the store's init (C-E-L1) hashes the
// store's name and asks the game's pool size lookup - the size gameconfig.xml gives it, else the default - and everything
// follows what comes back: the pool, the name map, the module's count, the streaming range and the box streamer's arrays.
// That is why gameconfig.xml's value carries a load, and a write to the pool's capacity alone does not: the 8001st name of
// start 3 of 2.7.0's proof found the name map's free list empty and the game trapped, the slot taken from a raised pool.
// STATICBOUNDS_SIZE in [STATICBOUNDS], 1 to 32767, 16000 by default, is the least the store gets: that call is redirected in
// DllMain, in the way of WeaponComponents' W-E-C4, to a function that asks the lookup and, for the store's hash alone, gives
// back the larger of the answer and the key - so a gameconfig.xml that gives the store more keeps it - and logs the sizes
// once. 32767 is the largest size whose every slot is sound: a slot past it comes out negative in an interior proxy, one
// past 65535 is cut in an entity's tag, and a size past 65535 wraps the box streamer's counts.
// The redirect raises the store only if the game asks after TweaksV's DllMain: a thread of the part checks, 60 seconds
// after the session's start - the game placing blip_controller, which the script programs' hook sees: the Blips part's,
// which the part joins, or while TweaksV.ini switches Blips off the part's own - that the game has asked, and reports a
// problem, with the part's box, when it has not: the game asked before TweaksV started, or makes the store another way.
// While a plugin that does the Blips part's job is installed, whatever TweaksV.ini says, no check is made, logged. A check
// that cannot be made - its events, the hook or its thread not there - is reported, the redirect staying: nothing else
// would say when the game does not ask.
// The same thread reads the store back once, through C-E-D1, whose lea names it: its count, its pool and its streamer
// differing is a problem, since the load would end at the smallest, and so is more than 32767 slots in use. C-E-D1 missing is
// a line, the raise going on.
// A size above the key's default on a pool memory budget not raised - the HeapAdjuster part off, by TweaksV.ini or a plugin
// found, or POOL_MEMORY at the game's own - is a problem of the start-up summary, the raise made all the same.
//   DRY_RUN          nothing is written, nothing checked; the line says what would be
//   DIAGNOSTIC_MODE  the call and the store at their RVAs; when the game asked, after the part's start
// The key's name, its default and the rule - the larger of the two sizes - and the hash are those of the Static Bounds Limit
// Adjuster v1.0.2 by Nochala (3_Washing_Machines), commit 61507bc:
//   Copyright (C) 2026 Nochala
//   License: GPL-3.0-or-later - https://github.com/Nochala/StaticboundsLimitAdjuster (THIRD_PARTY_NOTICES.md)
// The route is TweaksV's own, from the findings of its research in the game's code (PATTERNS.md).

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

		// ---- the store's size call and the store (PATTERNS.md, "StaticBounds")

		constexpr const GamePattern& kSizeCallPattern = Pattern("C-E-L1");
		constexpr const GamePattern& kStorePattern = Pattern("C-E-D1");
		constexpr const char* kWhat = "static bounds store's size call";

		constexpr uint32_t kStoreHash = 0x2CDFE406;	// the game's hash of the store's name, StaticBounds
		constexpr int kSoundSlots = 32767;			// the largest size whose every slot is sound, and the key's maximum
		static_assert(kSoundSlots == kStaticBoundsSize.maxValue, "TweaksV: STATICBOUNDS_SIZE goes up to the largest size whose every slot is sound");

		constexpr size_t kCallAt = 8;				// in C-E-L1's match: "call rel32", the lookup
		constexpr size_t kStoreLeaAt = 3;			// in C-E-D1's match: the disp32 of "lea rcx, [rip+disp32]", the store

		// The store, as its init and its streamer leave it: its offset of the pool is the one the init's lea pins, and the others no pattern holds
		constexpr uint8_t kPoolAt = 0x38;
		constexpr size_t kBaseAt = 0x08, kCountAt = 0x10, kPoolCapacityAt = 0x50, kInUseAt = 0x60, kBucketsAt = 0x88, kNamesAt = 0x90, kMadeAt = 0x98,
			kStreamerWordAt = 0xF0, kStreamerCountAt = 0xF8, kStoreRead = kStreamerCountAt + sizeof(uint32_t);
		constexpr uint32_t kInUseMask = 0x3FFFFFFF;	// the word at +0x60: its low 30 bits, the slots in use
		static_assert(kPoolCapacityAt == kPoolAt + 0x18u && kInUseAt == kPoolAt + 0x28u, "TweaksV: a pool's capacity at +0x18 and its bits at +0x28 (PATTERNS.md)");

		// Checked when TweaksV is compiled: the bytes the patterns pin. C-E-L1: mov rcx, rbx; mov edx, eax; mov r8d, edi, the three arguments;
		// the call; mov r14d, eax and lea rcx, [rsi+0x38] and mov edx, eax, what comes back handed to the pool. C-E-D1: lea rcx, [rip+disp32], the store
		static_assert(PatternHolds(kSizeCallPattern, 0, { 0x48, 0x89, 0xD9, 0x89, 0xC2, 0x41, 0x89, 0xF8, 0xE8 }) && PatternImm32(kSizeCallPattern, kCallAt + 1) == -1 &&
			PatternHolds(kSizeCallPattern, kCallAt + 5, { 0x41, 0x89, 0xC6, 0x48, 0x8D, 0x4E, kPoolAt, 0x89, 0xC2, 0xE8 }), "TweaksV: C-E-L1 is the store's size call, its answer handed to the store's pool");
		static_assert(PatternHolds(kStorePattern, 0, { 0x48, 0x8D, 0x0D }) && PatternImm32(kStorePattern, kStoreLeaAt) == -1, "TweaksV: C-E-D1 holds the store's lea at +0, its disp32 read");

		// The job not done, for the start-up summary.
		void Fail(const std::string& why)
		{
			ReportFailure(kTag, "The static bounds store's size is not raised this session: " + why);
		}

		// ---- the call's state, set before it is redirected

		using LookupFn = int32_t (*)(const void* configuration, uint32_t hash, int32_t fallback);
		LookupFn g_lookup = nullptr;				// the game's function, behind the redirect
		int32_t g_size = 0;							// STATICBOUNDS_SIZE
		bool g_diagnostic = false;					// DIAGNOSTIC_MODE, read once at the start
		ULONGLONG g_start = 0;						// the part's start, for the time the game asks
		const uint8_t* g_store = nullptr;			// the store, for the read-back: null while C-E-D1 gives none
		std::atomic<bool> g_sizesSaid{ false };		// the store's sizes are said once
		std::atomic<bool> g_noSizeSaid{ false };	// a size below 0 is reported once
		std::atomic<bool> g_afterAllSaid{ false };	// the game's asking after the check's report is said once

		// ---- the check that the game asks: whichever comes first, the game's first asking or the check's end, takes the state

		constexpr DWORD kCheckSeconds = 60;			// the check's time after the session's start, by when the game has made its stores
		constexpr DWORD kSettleMs = 1000;			// from an asking the thread has just seen to the read-back: the init runs on, on the game's thread
		constexpr uint32_t kSessionScript = Scripts::NameHash("blip_controller");	// its placement is the session's start
		enum : int { kWaiting, kAsked, kReported };
		std::atomic<int> g_check{ kWaiting };
		HANDLE g_askedEvent = nullptr;				// set at the first asking, so that the check's thread goes on early
		HANDLE g_sessionEvent = nullptr;			// set at the session's start: the game placing blip_controller
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

		// One of the check's two events, "asked" or "session", or nullptr - in test builds also while
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

		// The check not made, for the start-up summary: the redirect stays, and nothing says whether the game asks.
		void CheckNotDone(const std::string& why)
		{
			ReportFailure(kTag, "The check that the game asks for the static bounds store's size is not done this session: " + why);
		}

		// The script programs' hook calls this for every program the game places, on the game's thread: blip_controller's first
		// placement, told by its hash as the Blips part reads it, is the session's start. The first program of all comes some 24
		// seconds earlier, perhaps before the landing page, where a player may wait as long as they like. Nothing more is done here.
		void* SessionStarts(uint8_t* program, uint32_t)
		{
			if (!g_sessionStarted.load(std::memory_order_relaxed) && IsReadable(program, Scripts::kProgramSize) &&
				ValueAt<uint32_t>(program, Scripts::kNameHashOffset) == kSessionScript && !g_sessionStarted.exchange(true))
			{
				g_sessionTick = GetTickCount64();
				SetEvent(g_sessionEvent);
			}
			return nullptr;
		}

		// The store read back, once, all of it through CopyMemorySafely, since it is the game's data and the game goes on: what its size
		// reached. Its count, its pool and its streamer differing is a problem, and so is more in use than the slots that are sound.
		void ReadBack()
		{
			if (!g_store)
				return;
			uint8_t copy[kStoreRead];
			if (!CopyMemorySafely(copy, g_store, sizeof(copy)))
			{
				WriteLog(kTag, "The static bounds store's read-back is not done: the store at " + DescribeAddress(g_store) + " cannot be read" + At(kStorePattern) + ".");
				return;
			}
			if (ValueAt<uint8_t>(copy, kMadeAt) == 0)
			{
				WriteLog(kTag, "The static bounds store's read-back is not done: the game has not finished making the store.");
				return;
			}
			const int32_t count = ValueAt<int32_t>(copy, kCountAt);
			const int32_t pool = ValueAt<int32_t>(copy, kPoolCapacityAt);
			const uint32_t streamer = ValueAt<uint32_t>(copy, kStreamerCountAt);
			const uint32_t inUse = ValueAt<uint32_t>(copy, kInUseAt) & kInUseMask;
			WriteLog(kTag, "The static bounds store's read-back: count " + std::to_string(count) + ", pool " + std::to_string(pool) + ", name map " +
				std::to_string(ValueAt<uint32_t>(copy, kBucketsAt)) + " buckets with " + std::to_string(ValueAt<uint32_t>(copy, kNamesAt)) + " names, streamer " +
				std::to_string(streamer) + " (its word " + std::to_string(ValueAt<uint16_t>(copy, kStreamerWordAt)) + "), " + std::to_string(inUse) +
				" slots in use, its streaming base " + std::to_string(ValueAt<int32_t>(copy, kBaseAt)) + ".");
			if (pool != count || streamer != static_cast<uint32_t>(count))
				Fail("the store's count " + std::to_string(count) + ", its pool " + std::to_string(pool) + " and its streamer " + std::to_string(streamer) +
					" differ, and the load would end at the smallest (another mod may have changed one).");
			if (inUse > static_cast<uint32_t>(kSoundSlots))
				ReportFailure(kTag, "The static bounds store holds " + std::to_string(inUse) + " slots in use this session, past the " + std::to_string(kSoundSlots) +
					" whose every slot is sound: gameconfig.xml gives it " + std::to_string(count) + ", and an interior whose slot lies past them can lose its collision.");
		}

		// The check's thread: from the session's start, until the game has asked or for the check's time - then, if it has still not, the
		// problem, after the start-up summary in a box of its own, and if it has, the read-back. A session that never starts leaves it
		// waiting. Nothing leaves the thread, which drops the reference to the module StartModuleThread handed it as it ends.
		DWORD WINAPI CheckAsked(LPVOID module)
		{
			WaitForSingleObject(g_sessionEvent, INFINITE);
			try
			{
				if (g_diagnostic)
					WriteLog(kTag, "DIAGNOSTIC: the session started " + std::to_string(g_sessionTick - g_start) + " ms after the part's start (blip_controller "
						"placed): the static bounds check's clock runs.");
			}
			catch (...)
			{
			}
			WaitForSingleObject(g_askedEvent, g_checkSeconds * 1000);
			int waiting = kWaiting;
			const bool late = g_check.compare_exchange_strong(waiting, kReported);
			try
			{
				if (late)
					Fail("the game did not ask for the store's size within " + Seconds(g_checkSeconds) + " of the session's start - it asked before TweaksV "
						"started, or makes the store another way, and the store keeps the game's size.");
				else
				{
					Sleep(kSettleMs);
					ReadBack();
				}
			}
			catch (...)
			{
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// ---- the redirect

		// The store's init calls this in the place of the game's pool size lookup, on the game's thread, as other inits of its kind do for
		// their stores: a hash other than the store's goes through. The store's first asking ends the check. Both sizes are said before
		// the game goes on with them, so that the log holds them should the store's making fail. Nothing may throw out of here, and the
		// last error is the game's.
		int32_t StoreSize(const void* configuration, uint32_t hash, int32_t fallback)
		{
			const int32_t game = g_lookup(configuration, hash, fallback);
			if (hash != kStoreHash)
				return game;
			const int32_t size = game >= 0 && game < g_size ? g_size : game;
			const DWORD lastError = GetLastError();
			int checked = kWaiting;
			if (g_check.compare_exchange_strong(checked, kAsked) && g_askedEvent)
				SetEvent(g_askedEvent);
			try
			{
				if (checked == kReported && !g_afterAllSaid.exchange(true))
					WriteLog(kTag, "The game's lookup of the static bounds store's size came after all, after the check had reported it missing.");
				if (game < 0)
				{
					if (!g_noSizeSaid.exchange(true))
						Fail("the game gives the store the size " + std::to_string(game) + ", which is none - left as the game gave it.");
				}
				else if (!g_sizesSaid.exchange(true))
				{
					if (g_diagnostic)
						WriteLog(kTag, "DIAGNOSTIC: the game's lookup of the static bounds store's size came " + std::to_string(GetTickCount64() - g_start) + " ms after the part's start.");
					if (size != game)
						WriteLog(kTag, "The size the game gives the static bounds store, " + std::to_string(game) + ", is raised to " + std::to_string(size) +
							" (STATICBOUNDS_SIZE).");
					else
						WriteLog(kTag, "The size the game gives the static bounds store, " + std::to_string(game) + ", is kept: it is not below STATICBOUNDS_SIZE=" +
							std::to_string(g_size) + ".");
				}
			}
			catch (...)
			{
			}
			SetLastError(lastError);
			return size;
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

		// The store for the read-back, from C-E-D1's match, or null with the line that says why: the raise goes on without it.
		const uint8_t* FindStore()
		{
			constexpr const char* kNoReadBack = "The static bounds store's read-back is not done this session: ";
			std::string notFound;
			const uint8_t* const match = Scanner::FindOnce(kStorePattern, notFound);
			if (!match)
			{
				WriteLog(kTag, kNoReadBack + notFound + ".");
				return nullptr;
			}
			const uint8_t* const store = Rel32Target(match + kStoreLeaAt);
			if (const std::string fault = DataFault(store, kStoreRead, sizeof(uint64_t)); !fault.empty())
			{
				WriteLog(kTag, kNoReadBack + ("the store at " + DescribeAddress(store) + " " + fault) + At(kStorePattern) + ".");
				return nullptr;
			}
			if (g_diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: the static bounds store at " + DescribeAddress(store) + At(kStorePattern));
			return store;
		}
	}

	void InitStaticBounds()
	{
		g_start = GetTickCount64();
		g_diagnostic = IsDiagnosticMode();

		// 1. both patterns looked for, so that the log names each one missing: the size call is the job, the store only the read-back's
		std::string notFound;
		uint8_t* const match = Scanner::FindOnce(kSizeCallPattern, notFound);
		g_store = FindStore();
		if (!match)
		{
			Fail(notFound + ".");
			return;
		}

		// 2. the call leads into the game: else another mod has redirected it
		uint8_t* const call = match + kCallAt;
		const uint8_t* const target = CallTarget(call);
		if (g_diagnostic)
			WriteLog(kTag, std::string("DIAGNOSTIC: the ") + kWhat + " at " + DescribeAddress(call) + " -> " + (target ? DescribeAddress(target) : std::string("not a call")) + At(kSizeCallPattern));
		if (!target || !IsInsideGameImage(target))
		{
			Fail(std::string("the ") + kWhat + " at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" + At(kSizeCallPattern) + ".");
			return;
		}

		// 3. redirected, its original set before the rewrite - the check's event made first, so that the game's first asking finds it
		g_size = GetNumber(kStaticBoundsSize);
		const bool dryRun = IsDryRun();
		if (!dryRun)
			g_askedEvent = MakeEvent("asked");
		const std::vector<CallRedirect> redirects = { { call, reinterpret_cast<const void*>(&StoreSize), kWhat, reinterpret_cast<void**>(&g_lookup) } };
		if (!RedirectCalls(redirects, {}, kTag))
		{
			if (g_askedEvent)
				CloseHandle(g_askedEvent);
			g_askedEvent = nullptr;
			Fail("the game code could not be changed (details in TweaksV.log).");
			return;
		}
		if (dryRun)
			return;
		WriteLog(kTag, Capitalised(std::string("the ") + kWhat) + " at " + DescribeAddress(call) + " is redirected: the store gets at least " + std::to_string(g_size) +
			" slots (STATICBOUNDS_SIZE), more where the game gives it more.");
		CheckBudget();

		// 4. the check, from the session's start - the game placing blip_controller, seen through the script programs'
		// hook: the part joins the hook the Blips part has made while Blips is on, and makes it itself, the first part to use
		// it, while TweaksV.ini switches Blips off. While a plugin that does Blips' job on the same script programs is installed
		// - BlipLimitFix.asi or Resource Extensions, closed source, as the conflict check found them, whatever TweaksV.ini
		// says of Blips - the part makes no hook and leaves the script programs to it, logged. A check that cannot be made
		// otherwise - its events, the hook or its thread not there - is reported, the redirect staying
		if (IsPluginInstalledFor(kBlipsSection))
		{
			WriteLog(kTag, "The static bounds check and its read-back are not made this session: they start with the session, which TweaksV sees through the script programs' hook, "
				"and a plugin that does the Blips part's job on the script programs is installed (named above) - TweaksV leaves the script programs to it.");
			return;
		}
		g_sessionEvent = MakeEvent("session");
		if (!g_askedEvent || !g_sessionEvent)
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
		if (!StartModuleThread(&CheckAsked, "static bounds check"))
		{
			CheckNotDone("its thread could not be started (details in TweaksV.log).");
			return;
		}
		WriteLog(kTag, "The static bounds check waits for the session's start - the game placing blip_controller - and then " +
			Seconds(g_checkSeconds) + " for the game to ask for the store's size.");
	}
}
