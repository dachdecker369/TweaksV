// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - failure collection, the one start-up summary message box and the boxes of the reports after it.
#pragma once

#include <windows.h>

#include <string>

namespace TweaksV
{
	// Starts thread with an extra reference to TweaksV.asi, handed to the thread, which drops it when it
	// ends (FreeLibraryAndExitThread), so TweaksV.asi cannot be unloaded while the thread still runs its
	// code. False if it cannot start - logged, what naming the thread. Safe to call from DllMain.
	bool StartModuleThread(LPTHREAD_START_ROUTINE thread, const char* what);

	// Records a problem for the start-up summary and logs it: a part that cannot do its job. Once the summary has been shown,
	// a box of its own says that a part stopped working. Thread-safe.
	void ReportFailure(const char* plugin, const std::string& what);

	// What a part has found, which gives the finding's box its heading and its last line.
	enum class Finding
	{
		LegacyPacks,	// the check for packs made for GTA V Legacy
		RefusedFiles,	// a guard of WeaponLimits, of the files it kept from the game
	};

	// Logs a problem that is no part failing: the part is doing its job and has found what the player must know. Never an item
	// of the start-up summary, which says that TweaksV could not apply something: it comes in a box of its own, a second later,
	// whether the summary has been shown or not. Thread-safe.
	void ReportFinding(const char* plugin, Finding finding, const std::string& what);

	// One message box listing everything reported so far - only if ERROR_POPUP=1
	// and there is something to report. Shown at most once. Never call it from DllMain.
	void ShowStartupSummary();

	// The same, from a worker thread - safe to start from DllMain. If that thread cannot start, the next problem reported
	// tries again.
	void ShowStartupSummaryLater();
}
