// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part (see TweaksVStaticBounds.h).
//
// The game keeps the static collision bounds of the map - every building, prop and interior placed in it, a heavy
// map mod's MLOs and expansions among them - in one pool, the pool named StaticBounds (the game's hash of the name:
// 0x2CDFE406), and a full pool ends the game in an access violation. The game makes the pool once, at its start,
// with the size its pool size lookup gives: the lookup takes the game's configuration, a pool's name and its
// default, and returns the size gameconfig.xml gives that pool, else the default. STATICBOUNDS_SIZE in
// [STATICBOUNDS], 1 to 100000, 16000 by default, is the least the pool gets: the call of the lookup that names the
// pool is redirected through a call stub to a function that calls the game's lookup with the game's arguments and
// returns the larger of the game's size and STATICBOUNDS_SIZE - so a gameconfig.xml that gives the pool more keeps
// it - and logs the sizes once.
// The call is found as the original finds it: the lookup is where the CRoadBlock pool's size call leads (C-E-L1),
// inside the game; the call is the first call of the lookup in the 64 bytes after a load of the pool's name, "mov edx,
// 0x2CDFE406" (C-E-H1), and exactly one load may be followed by one. A call another mod has redirected leads
// elsewhere, so it is no call of the lookup: the job is then reported and not done, and nothing is changed.
// The redirect raises the pool only if the game asks for its size after TweaksV's DllMain: a thread of the part
// checks, 60 seconds after the session's start - the first script program the game places, which the script
// programs' hook sees while the Blips part, whose hook it is, is on; the game makes its pools as the session starts -
// that the game has asked the redirected call, and reports a problem, with the part's box, when it has not: the game
// made the pool before TweaksV started, or asks for its size another way. Without the hook no check is made, logged.
//   DRY_RUN          nothing is redirected, nothing checked; the line says what would be
//   DIAGNOSTIC_MODE  the lookup and the call at their RVAs, every load of the pool's name without a call after it,
//                    and when the game asked
// Written for TweaksV from the Static Bounds Limit Adjuster v1.0.2 by Nochala (3_Washing_Machines), commit 61507bc:
//   Copyright (C) 2026 Nochala
//   License: GPL-3.0-or-later - https://github.com/Nochala/StaticboundsLimitAdjuster (THIRD_PARTY_NOTICES.md)
// Adapted for TweaksV on 2026-10-30: its two patterns of the game's code, its search for the call, its rule - the
// larger of the two sizes, a size the game gives below 0 left as it is - and its key, STATICBOUNDS_SIZE, with its
// default and its maximum. Its hooks on the pool's set-up and its allocator, which it tries first on Enhanced, and its
// hook on the lookup itself are left out: a call that cannot be redirected is reported, as every part does.

#include "TweaksVStaticBounds.h"

#include <windows.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
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

		// ---- the pool's size call (PATTERNS.md, "StaticBounds")

		constexpr uint32_t kStaticBoundsHash = 0x2CDFE406;	// the game's hash of the pool's name, "StaticBounds"
		constexpr uint32_t kRoadBlockHash = 0xF72CC701;		// the CRoadBlock pool's, whose size call leads to the same lookup
		constexpr size_t kNameLoadSize = 5;					// "mov edx, imm32": a load of a pool's name
		constexpr size_t kCallSize = 5;						// "call rel32"
		constexpr size_t kCallWindow = 64;					// the bytes after a load of the name that the call lies in, whole
		constexpr size_t kMaxNameLoads = 16;				// the loads of the name the part looks through: more is a changed game

		constexpr const GamePattern& kNamePattern = Pattern("C-E-H1");		// a load of the pool's name: its size call follows
		constexpr const GamePattern& kLookupPattern = Pattern("C-E-L1");	// the CRoadBlock pool's size call: the lookup
		constexpr size_t kLookupCallAt = 11;								// C-E-L1: "call rel32" - the pool size lookup

		// True if a pattern holds "mov edx, imm32" with this imm32 at index at - checked when TweaksV is compiled, so
		// that the hashes above and the offsets read are the bytes the patterns pin.
		consteval bool HoldsNameLoad(const GamePattern& pattern, size_t at, uint32_t hash)
		{
			if (PatternByte(pattern, at) != 0xBA)
				return false;
			for (size_t i = 0; i < 4; i++)
				if (PatternByte(pattern, at + 1 + i) != static_cast<int>((hash >> (8 * i)) & 0xFF))
					return false;
			return true;
		}
		static_assert(HoldsNameLoad(kNamePattern, 0, kStaticBoundsHash) && PatternByte(kNamePattern, kNameLoadSize) == -1,
			"TweaksV: C-E-H1 is the load of the pool's name, mov edx, 0x2CDFE406, and no more");
		static_assert(HoldsNameLoad(kLookupPattern, 0, kRoadBlockHash) && PatternByte(kLookupPattern, kLookupCallAt) == 0xE8,
			"TweaksV: C-E-L1 holds the CRoadBlock pool's name, mov edx, 0xF72CC701, at +0 and the lookup's call at +11");

		std::string Hex32(uint32_t value)
		{
			char text[16];
			snprintf(text, sizeof(text), "0x%08X", value);
			return text;
		}

		// The pool size lookup a call leads to, checked as TweaksV checks a call it redirects or reads a target from: a
		// "call rel32" that can be read and leads into the game - another mod's hook would lead outside it. nullptr if not.
		// The weapon components' pool size call (W-E-C4) leads to the same lookup, so one helper can check both.
		const uint8_t* PoolSizeLookup(const uint8_t* call)
		{
			const uint8_t* const lookup = IsReadable(call, kCallSize) ? CallTarget(call) : nullptr;
			return lookup && IsInsideGameImage(lookup) ? lookup : nullptr;
		}

		// The call of lookup after a load of the pool's name: the first "call rel32" wholly inside the 64 bytes after the
		// load that leads to it, or nullptr - also when those bytes cannot be read. An E8 inside another instruction leads
		// elsewhere and is passed over, as the original's search passes it over.
		uint8_t* CallAfterLoad(uint8_t* load, const uint8_t* lookup)
		{
			uint8_t* const window = load + kNameLoadSize;
			if (!IsReadable(window, kCallWindow))
				return nullptr;
			for (size_t at = 0; at + kCallSize <= kCallWindow; at++)
				if (window[at] == 0xE8 && CallTarget(window + at) == lookup)
					return window + at;
			return nullptr;
		}

		// The job not done, for the start-up summary; the caller then leaves the game as it is.
		void Fail(const std::string& why)
		{
			ReportFailure(kTag, "The static bounds pool's size is not raised this session: " + why);
		}

		// ---- the redirected call

		using PoolSizeLookupFn = int32_t (*)(void* configuration, uint32_t pool, int32_t defaultSize);
		PoolSizeLookupFn g_lookup = nullptr;		// the game's lookup behind the call
		const uint8_t* g_call = nullptr;			// the call, for the lines
		int32_t g_size = 0;							// STATICBOUNDS_SIZE
		bool g_diagnostic = false;					// DIAGNOSTIC_MODE, read once at the start: the call never reads a switch
		ULONGLONG g_start = 0;						// TweaksV's start, for the time of the game's first ask
		std::atomic<bool> g_sizeSaid{ false };		// the pool's sizes are said once
		std::atomic<bool> g_otherSaid{ false };		// a call for another pool is reported once

		// ---- the check that the game asks: whichever comes first, the game's first ask for the static bounds pool's size or
		// the check's end, takes the state - an ask after the check has reported is said once, with the sizes

		constexpr DWORD kCheckSeconds = 60;			// the check's time after the session's start, by when the game has made its pools
		enum : int { kWaiting, kAsked, kReported };
		std::atomic<int> g_check{ kWaiting };
		HANDLE g_askedEvent = nullptr;				// set at the first ask, so that the check's thread ends early
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

		// The check's thread: from the session's start, until the game has asked or for the check's time - then, if the game has
		// still not asked, the problem, after the start-up summary in a box of its own. A session that never starts leaves it
		// waiting. Nothing leaves the thread, which drops the reference to the module StartModuleThread handed it as it ends.
		DWORD WINAPI CheckAsked(LPVOID module)
		{
			WaitForSingleObject(g_sessionEvent, INFINITE);
			try
			{
				if (g_diagnostic)
					WriteLog(kTag, "DIAGNOSTIC: the session started " + std::to_string(g_sessionTick - g_start) + " ms after TweaksV's start (the first script "
						"program placed): the static bounds check's clock runs.");
			}
			catch (...)
			{
			}
			WaitForSingleObject(g_askedEvent, g_checkSeconds * 1000);
			int waiting = kWaiting;
			if (g_check.compare_exchange_strong(waiting, kReported))
			{
				try
				{
					Fail("the game did not ask the redirected call at " + DescribeAddress(g_call) + " for the pool's size within " + Seconds(g_checkSeconds) +
						" of the session's start - it made the pool before TweaksV started, or asks for its size another way, and the pool keeps the "
						"game's size.");
				}
				catch (...)
				{
				}
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// The game calls this through the call stub, on its thread, when it makes the pool, with the arguments it hands its
		// lookup: the static bounds pool gets the larger of the game's size and STATICBOUNDS_SIZE; a size below 0, which is
		// none, and the size of another pool are passed on as the game gives them. The first ask ends the check - one for
		// another pool too, which has a problem of its own, so that the check does not report the call a second time. Nothing
		// may throw out of here, and the game's last error after its lookup is the one the call leaves.
		int32_t StaticBoundsPoolSize(void* configuration, uint32_t pool, int32_t defaultSize)
		{
			const int32_t game = g_lookup(configuration, pool, defaultSize);
			const DWORD lastError = GetLastError();
			const bool ours = pool == kStaticBoundsHash;
			const int32_t size = ours && game >= 0 && game < g_size ? g_size : game;
			int checked = kWaiting;
			if (g_check.compare_exchange_strong(checked, kAsked) && g_askedEvent)	// a call for another pool too: reported on its own below
				SetEvent(g_askedEvent);
			std::atomic<bool>& said = ours ? g_sizeSaid : g_otherSaid;
			if (!said.exchange(true))
			{
				try
				{
					const std::string sizes = std::to_string(game);
					if (ours && g_diagnostic)
						WriteLog(kTag, "DIAGNOSTIC: the game's first ask for the static bounds pool's size came " +
							std::to_string(GetTickCount64() - g_start) + " ms after TweaksV's start.");
					if (ours && checked == kReported)
						WriteLog(kTag, "The game's ask for the static bounds pool's size came after all, after the check had reported it missing.");
					if (!ours)
						Fail("the redirected call at " + DescribeAddress(g_call) + " asks for the size of the pool " + Hex32(pool) +
							", not of the static bounds pool, " + Hex32(kStaticBoundsHash) + " - its size, " + sizes + ", is passed on as the game gave it.");
					else if (game < 0)
						Fail("the game's configuration gives the static bounds pool the size " + sizes + ", which is none - passed on as the game gave it.");
					else if (size != game)
						WriteLog(kTag, "The pool size the game's configuration gives for the static bounds, " + sizes + ", is raised to " +
							std::to_string(size) + " (STATICBOUNDS_SIZE).");
					else
						WriteLog(kTag, "The pool size the game's configuration gives for the static bounds, " + sizes +
							", is kept: it is not below STATICBOUNDS_SIZE=" + std::to_string(g_size) + ".");
				}
				catch (...)
				{
				}
			}
			SetLastError(lastError);
			return size;
		}
	}

	void InitStaticBounds()
	{
		g_start = GetTickCount64();
		g_diagnostic = IsDiagnosticMode();

		// 1. both patterns looked for, so that the log names each one missing: the loads of the pool's name, every one up
		// to the bound, and the CRoadBlock pool's size call, which must be unique
		const std::vector<uint8_t*> loads = Scanner::FindAll(kNamePattern, kMaxNameLoads + 1);
		uint8_t* const roadBlock = Scanner::Find(kLookupPattern);
		if (loads.empty() || !roadBlock)
		{
			Fail(std::string(loads.empty() ? kNamePattern.label : kLookupPattern.label) + " was not found.");
			return;
		}
		if (loads.size() > kMaxNameLoads)
		{
			Fail("the game code holds " + std::string(kNamePattern.label) + " " + std::to_string(loads.size()) + " or more times, more than the " +
				std::to_string(kMaxNameLoads) + " the part looks through (the game has changed).");
			return;
		}

		// 2. the pool size lookup: where the CRoadBlock pool's size call leads - a call into the game
		const uint8_t* const roadBlockCall = roadBlock + kLookupCallAt;
		const uint8_t* const lookup = PoolSizeLookup(roadBlockCall);
		if (IsDiagnosticMode())
			WriteLog(kTag, "DIAGNOSTIC: pool size call at " + DescribeAddress(roadBlockCall) + " -> " + DescribeAddress(CallTarget(roadBlockCall)) +
				" for the pool " + Hex32(kRoadBlockHash) + " (" + kLookupPattern.label + "): the pool size lookup");
		if (!lookup)
		{
			Fail("the CRoadBlock pool's size call at " + DescribeAddress(roadBlockCall) + " is not a call into the game (another mod may have hooked it "
				"already) (" + kLookupPattern.label + ").");
			return;
		}

		// 3. the static bounds pool's size call: the first call of the lookup in the 64 bytes after a load of the pool's name,
		// after exactly one of the loads
		uint8_t* call = nullptr;
		for (uint8_t* const load : loads)
		{
			uint8_t* const found = CallAfterLoad(load, lookup);
			if (!found)
			{
				if (IsDiagnosticMode())
					WriteLog(kTag, "DIAGNOSTIC: the load of the pool's name at " + DescribeAddress(load) + " (" + kNamePattern.label + ") is followed by no "
						"call of the pool size lookup within " + std::to_string(kCallWindow) + " bytes");
				continue;
			}
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: pool size call at " + DescribeAddress(found) + " -> " + DescribeAddress(lookup) + " for the pool " +
					Hex32(kStaticBoundsHash) + ", its name loaded at " + DescribeAddress(load) + " (" + kNamePattern.label + ")");
			if (call)
			{
				Fail("the pool size lookup is called after two loads of the static bounds pool's name, at " + DescribeAddress(call) + " and " +
					DescribeAddress(found) + " - which call sizes the pool cannot be told (" + kNamePattern.label + ").");
				return;
			}
			call = found;
		}
		if (!call)
		{
			Fail("no call of the pool size lookup at " + DescribeAddress(lookup) + " follows a load of the static bounds pool's name within " +
				std::to_string(kCallWindow) + " bytes (" + kNamePattern.label + "; another mod may have redirected the call already).");
			return;
		}

		// 4. the call redirected: the pool gets at least STATICBOUNDS_SIZE when the game makes it - the check's event made first,
		// so that the game's first ask finds it
		g_size = GetNumber(kStaticBoundsSize);
		g_call = call;
		const bool dryRun = IsDryRun();
		if (!dryRun)
			g_askedEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		const std::vector<CallRedirect> redirects = { { call, reinterpret_cast<const void*>(&StaticBoundsPoolSize), "static bounds pool's size call",
			reinterpret_cast<void**>(&g_lookup) } };
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
		WriteLog(kTag, "The static bounds pool's size call at " + DescribeAddress(call) + " is redirected: the pool gets at least " + std::to_string(g_size) +
			" slots (STATICBOUNDS_SIZE) when the game makes it, more where the game's configuration gives it more.");

		// 5. the check, from the session's start - the first script program the game places, seen through the script programs'
		// hook, which the part joins only while the Blips part, whose hook it is, is on: the part never makes that hook itself
		// (Resource Extensions, for one, switches Blips off). A check that cannot be made is logged, the redirect staying; a
		// thread that cannot start is reported
		g_sessionEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		std::string why;
		if (!IsFeatureOn(kBlipsSection))
			why = "it starts with the session, which TweaksV sees through the script programs' hook, and the Blips part, whose hook that is, is off.";
		else if (!g_askedEvent || !g_sessionEvent)
			why = "its events could not be made.";
		else if (!Scripts::AddUser({ kTag, &SessionStarts, nullptr }, why) && why.empty())
			why = "the script programs' hook is not there.";
		if (!why.empty())
		{
			WriteLog(kTag, "The static bounds check is not made this session: " + why);
			return;
		}
		g_checkSeconds = CheckSeconds();
		if (!StartModuleThread(&CheckAsked, "static bounds check"))
		{
			ReportFailure(kTag, "The check that the game asks for the static bounds pool's size is not done this session: its thread could not be started "
				"(details in TweaksV.log).");
			return;
		}
		WriteLog(kTag, "The static bounds check waits for the session's start - the first script program the game places - and then " +
			Seconds(g_checkSeconds) + " for the game to ask for the pool's size.");
	}
}
