// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - failure collection, the one start-up summary message box and the boxes of the reports after it.

#include "TweaksVSummary.h"

#include <windows.h>

#include <iterator>
#include <string>
#include <vector>

#include "TweaksVLock.h"
#include "TweaksVLog.h"
#include "TweaksVSettings.h"

namespace TweaksV
{
	namespace
	{
		constexpr size_t kMaxItems = 40;
		constexpr int kMaxLateBoxes = 3;	// per report: a part that keeps failing cannot fill the screen with boxes

		SRWLOCK g_lock = SRWLOCK_INIT;
		std::vector<std::string> g_items;
		size_t g_dropped = 0;
		LONG g_shown = 0;
		bool g_summaryDue = false;		// under g_lock: the summary's thread has been asked for and has not started

		// A report outside the start-up summary, which a thread of its own shows: its box's heading and last line, what the log
		// calls one and its count, and what waits for its box.
		struct LateReport
		{
			const wchar_t* heading;
			const wchar_t* lastLine;
			const char* name;
			const char* counted;
			std::vector<std::string> items{};	// under g_lock
			size_t dropped = 0;					// under g_lock: beyond kMaxItems, counted for the box
			int boxes = 0;						// only that thread uses it
		};
		constexpr const char* kFinding = "finding";
		constexpr const char* kFindings = "finding(s)";
		LateReport g_late[] = {	// the findings in the order of their enum, then the part that stopped working
			{ L"TweaksV: its check of the packs in the mods folder found:", L"The game continues with the packs as they are.", kFinding, kFindings },
			{ L"TweaksV: WeaponLimits refused files the game has no room for:", L"The game continues without the files refused.", kFinding, kFindings },
			{ L"TweaksV: a part stopped working after the start-up:", L"The game continues without the affected parts.",
				"problem after the start-up summary", "problem(s)" },
		};
		constexpr size_t kPartStopped = std::size(g_late) - 1;
		static_assert(static_cast<size_t>(Finding::RefusedFiles) + 1 == kPartStopped, "TweaksV: a finding needs its row in g_late");
		bool g_lateThread = false;				// under g_lock: that thread runs

		void ShowBox(const std::wstring& heading, const std::wstring& lastLine, const std::vector<std::string>& items, size_t dropped)
		{
#ifdef TWEAKSV_TEST_BUILD
			// test builds only: TWEAKSV_TEST_NO_BOXES=1 in the environment keeps the box off the screen, for
			// unattended test runs - also those in which TweaksV.ini is not read and cannot turn boxes off
			wchar_t noBoxes[4]{};
			if (GetEnvironmentVariableW(L"TWEAKSV_TEST_NO_BOXES", noBoxes, 4) == 1 && noBoxes[0] == L'1')
			{
				WriteLog("TweaksV", "TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): " + ToUtf8(heading) + " ... " + ToUtf8(lastLine));
				return;
			}
#endif
			std::wstring text = heading + L"\n\n";
			for (const auto& item : items)
				text += L"• " + FromUtf8(item) + L"\n\n";
			if (dropped > 0)
				text += L"• ... and " + std::to_wstring(dropped) + L" more.\n\n";
			text += lastLine + L" Details are in TweaksV.log:\n" + GetLogPathW() + L"\n\nTo stop this message, set ERROR_POPUP=0 in TweaksV.ini.";
			MessageBoxW(nullptr, text.c_str(), L"TweaksV", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
		}

		// Nothing may leave a thread of this module: an exception that does ends the game. The problems a
		// box could not show are in the log already (ReportFailure writes them first).
		DWORD WINAPI SummaryThread(LPVOID module)
		{
			try
			{
				ShowStartupSummary();
			}
			catch (...)
			{
				try
				{
					WriteLog("TweaksV", "Unexpected exception while showing the start-up summary - the box is not shown; "
						"the problems are logged above.");
				}
				catch (...)
				{
				}
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// What waits for the report's box: its box, or a line that says why there is none.
		void ShowLate(LateReport& report, const std::vector<std::string>& items, size_t dropped)
		{
			const std::string number = std::to_string(items.size() + dropped) + " " + report.counted;
			if (!IsOn(kErrorPopup))
				WriteLog("TweaksV", Capitalised(report.name) + " not shown (ERROR_POPUP=0): " + number + ", see above.");
			else if (report.boxes >= kMaxLateBoxes)
				WriteLog("TweaksV", Capitalised(report.name) + " not shown: " + std::to_string(kMaxLateBoxes) +
					" message boxes were shown already this session.");
			else
			{
				++report.boxes;
				WriteLog("TweaksV", std::string("Showing a ") + report.name + " (" + number + ").");
				ShowBox(report.heading, report.lastLine, items, dropped);
			}
		}

		// Shows what the reports hold, a second after the first of it (more usually follows at once), a box per
		// report, until nothing is left.
		DWORD WINAPI LateThread(LPVOID module)
		{
			try
			{
				for (;;)
				{
					Sleep(1000);
					struct Taken
					{
						std::vector<std::string> items;
						size_t dropped = 0;
					} taken[std::size(g_late)];
					bool any = false;
					{
						ExclusiveLock lock(g_lock);
						for (size_t i = 0; i < std::size(g_late); i++)
						{
							taken[i].items.swap(g_late[i].items);
							taken[i].dropped = g_late[i].dropped;
							g_late[i].dropped = 0;
							any = any || !taken[i].items.empty();
						}
						if (!any)
							g_lateThread = false;	// under the lock, so a new problem starts a new thread
					}
					if (!any)
						break;

					for (size_t i = 0; i < std::size(g_late); i++)
						if (!taken[i].items.empty())
							ShowLate(g_late[i], taken[i].items, taken[i].dropped);
				}
			}
			catch (...)
			{
				try
				{
					WriteLog("TweaksV", "Unexpected exception while showing a problem after the start-up summary - the box "
						"is not shown; the problem is logged above.");
				}
				catch (...)
				{
				}
				ExclusiveLock lock(g_lock);
				g_lateThread = false;	// the next problem starts a new thread
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// The next problem reported, once it is recorded: the start-up summary's thread tried again, if it could not start.
		void TrySummaryAgain()
		{
			bool due = false;
			{
				ExclusiveLock lock(g_lock);
				due = g_summaryDue;
			}
			if (due)
				ShowStartupSummaryLater();
		}

		// An item for the report's box; LateThread, unless it runs, is started for it. If it cannot start, the item waits for
		// the next one, which tries again.
		void AddLate(LateReport& report, const std::string& item)
		{
			bool start = false;
			{
				ExclusiveLock lock(g_lock);
				if (report.items.size() < kMaxItems)
					report.items.push_back(item);
				else
					++report.dropped;	// its box counts it
				start = !g_lateThread;
				g_lateThread = true;
			}
			if (start && !StartModuleThread(LateThread, "late report"))
			{
				ExclusiveLock lock(g_lock);
				g_lateThread = false;
			}
		}

#ifdef TWEAKSV_TEST_BUILD
		// test builds only: TWEAKSV_TEST_REFUSE_THREAD=<n> <what> in the environment (n 1..9) has Windows' refusal of a new thread
		// come for the first n starts of the thread StartModuleThread names what, read once and said once, so that a suite
		// reaches what comes of a thread that cannot start (SPECIFICATIONS 13.1).
		bool ThreadRefused(const char* what)
		{
			struct Refusal
			{
				std::string what;
				LONG left = 0;
			};
			static Refusal refusal = []
			{
				wchar_t text[64]{};
				const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_REFUSE_THREAD", text, 64);
				if (length < 3 || length >= 64 || text[0] < L'1' || text[0] > L'9' || text[1] != L' ')
					return Refusal{};
				const std::string value = ToUtf8(text);
				Refusal made{ value.substr(2), text[0] - L'0' };
				WriteLog("TweaksV", "TEST BUILD: Windows refuses the first " + std::to_string(made.left) + " start(s) of the " + made.what +
					" thread (TWEAKSV_TEST_REFUSE_THREAD=" + value + ").");
				return made;
			}();
			if (refusal.what != what || InterlockedDecrement(&refusal.left) < 0)
				return false;
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return true;
		}
#endif
	}

	bool StartModuleThread(LPTHREAD_START_ROUTINE thread, const char* what)
	{
		HMODULE module = nullptr;
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(thread), &module))
		{
			const DWORD error = GetLastError();
			WriteLog("TweaksV", std::string("The ") + what + " thread could not be started (" + DescribeError(error) + ").");
			return false;
		}
		// Allowed inside DllMain as long as DllMain does not wait for the thread: it starts
		// running once the loader lock is released.
#ifdef TWEAKSV_TEST_BUILD
		const HANDLE handle = ThreadRefused(what) ? nullptr : CreateThread(nullptr, 0, thread, module, 0, nullptr);
#else
		const HANDLE handle = CreateThread(nullptr, 0, thread, module, 0, nullptr);
#endif
		if (!handle)
		{
			const DWORD error = GetLastError();
			FreeLibrary(module);
			WriteLog("TweaksV", std::string("The ") + what + " thread could not be started (" + DescribeError(error) + ").");
			return false;
		}
		CloseHandle(handle);
		return true;
	}

	void ReportFailure(const char* plugin, const std::string& what)
	{
		WriteLog(plugin, "PROBLEM: " + what);

		const std::string item = std::string("[") + plugin + "] " + what;	// built outside the lock: it can throw
		bool late = true;
		{
			ExclusiveLock lock(g_lock);
			if (InterlockedCompareExchange(&g_shown, 0, 0) == 0)	// the start-up summary is still to come
			{
				if (g_items.size() < kMaxItems)
					g_items.push_back(item);
				else
					++g_dropped;
				late = false;
			}
		}
		if (late)
			AddLate(g_late[kPartStopped], item);
		TrySummaryAgain();
	}

	void ReportFinding(const char* plugin, Finding finding, const std::string& what)
	{
		WriteLog(plugin, "PROBLEM: " + what);
		AddLate(g_late[static_cast<size_t>(finding)], std::string("[") + plugin + "] " + what);
		TrySummaryAgain();
	}

	void ShowStartupSummary()
	{
		// From here on, a reported problem is shown on its own (LateThread): the list is taken under
		// the same lock ReportFailure adds to it with, so no problem is lost in between. Taken by a
		// swap, which cannot throw under the lock; the list is not needed again.
		std::vector<std::string> items;
		size_t dropped = 0;
		bool first = false;
		{
			ExclusiveLock lock(g_lock);
			first = InterlockedExchange(&g_shown, 1) == 0;
			items.swap(g_items);
			dropped = g_dropped;
		}

		if (!first || items.empty())
			return;
		if (!IsOn(kErrorPopup))
		{
			WriteLog("TweaksV", "Start-up summary not shown (ERROR_POPUP=0): " + std::to_string(items.size() + dropped) +
				" problem(s), see above.");
			return;
		}

		WriteLog("TweaksV", "Showing the start-up summary (" + std::to_string(items.size() + dropped) + " problem(s)).");
		ShowBox(L"TweaksV could not apply everything:", L"The game continues without the affected parts.", items, dropped);
	}

	void ShowStartupSummaryLater()
	{
		{
			ExclusiveLock lock(g_lock);
			g_summaryDue = true;
		}
		if (StartModuleThread(SummaryThread, "start-up summary"))
		{
			ExclusiveLock lock(g_lock);
			g_summaryDue = false;
		}
	}
}
