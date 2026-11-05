// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - stand-alone plugins installed next to TweaksV that do the job of one of its parts.

#include "conflicts.h"

#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>

#include "features.h"
#include "files.h"
#include "ini.h"
#include "log.h"
#include "settings.h"
#include "summary.h"

namespace TweaksV
{
	namespace
	{
		// Per section of kSections, whether CheckForConflicts found a plugin that does that part's job, whatever
		// TweaksV.ini says of the part - written by the check, in DllMain before the parts start, and read by them
		// afterwards, on the same thread.
		bool g_pluginFound[std::size(kSections)] = {};

		void NotePluginFound(const SettingsSection& section)
		{
			for (size_t i = 0; i < std::size(kSections); i++)
				if (kSections[i] == &section)
					g_pluginFound[i] = true;
		}
	}

	void CheckForConflicts()
	{
		int found = 0;
		for (const wchar_t* folder : kLoaderFolders)
		{
			for (const Feature& feature : Features())
			{
				for (const wchar_t* file : feature.oldPlugins)
				{
					if (!file)
						continue;
					const std::wstring path = GetGameFolder() + folder + L"\\" + file;
					if (!FileExists(path))
						continue;

					++found;
					NotePluginFound(*feature.section);
					DisableFeatureForSession(*feature.section);
					ReportFailure("TweaksV", "Old plugin found: " + ToUtf8(path) + ". TweaksV's " + feature.section->feature +
						" part is switched off for this session so nothing is patched twice. "
						"Delete the old file - TweaksV replaces it.");
				}

				for (const PeerPlugin* peer : feature.peerPlugins)
				{
					if (!peer)
						continue;
					const std::wstring path = GetGameFolder() + folder + L"\\" + peer->file;
					if (!FileExists(path))
						continue;

					++found;
					NotePluginFound(*feature.section);	// whatever TweaksV.ini says of the part
					// a part TweaksV.ini switches off gets a line, not a problem - the file's own switch, not
					// IsFeatureOn: an old plugin found before may have switched the part off already, and
					// this one is still said, so the result does not depend on the order of the finds
					if (!IsEnabledInIni(*feature.section))
					{
						WriteLog("TweaksV", "Conflict check: " + ToUtf8(path) + " is installed; TweaksV's " +
							feature.section->feature + " part is off in TweaksV.ini already.");
						continue;
					}
					DisableFeatureForSession(*feature.section);
					ReportFailure("TweaksV", ToUtf8(path) + " is installed and does this job too (" + peer->description +
						"): TweaksV's " + feature.section->feature + " part is switched off while it is there, so nothing "
						"is patched twice. To stop this message, set ENABLED=0 under [" + feature.section->name +
						"] in TweaksV.ini.");
				}
			}
		}

		if (found == 0)
			WriteLog("TweaksV", "Conflict check: none of the plugins that do the job of one of TweaksV's parts were found.");
	}

	bool IsPluginInstalledFor(const SettingsSection& section)
	{
		for (size_t i = 0; i < std::size(kSections); i++)
			if (kSections[i] == &section)
				return g_pluginFound[i];
		return false;
	}

	bool IsInsideGameImage(const void* p)
	{
		static const std::pair<uintptr_t, uintptr_t> image = []
		{
			const auto base = reinterpret_cast<const BYTE*>(GetModuleHandleW(nullptr));
			const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
			if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE)
				return std::pair<uintptr_t, uintptr_t>(0, 0);
			const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE)
				return std::pair<uintptr_t, uintptr_t>(0, 0);
			const auto start = reinterpret_cast<uintptr_t>(base);
			return std::pair<uintptr_t, uintptr_t>(start, start + nt->OptionalHeader.SizeOfImage);
		}();

		const auto address = reinterpret_cast<uintptr_t>(p);
		return address >= image.first && address < image.second;
	}

	bool IsInsideGameData(const void* p)
	{
		static const std::pair<uintptr_t, uintptr_t> data = []
		{
			const auto base = reinterpret_cast<const BYTE*>(GetModuleHandleW(nullptr));
			const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
			if (!base || dos->e_magic != IMAGE_DOS_SIGNATURE)
				return std::pair<uintptr_t, uintptr_t>(0, 0);
			const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE)
				return std::pair<uintptr_t, uintptr_t>(0, 0);
			const IMAGE_SECTION_HEADER* const sections = IMAGE_FIRST_SECTION(nt);
			for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
				if (memcmp(sections[i].Name, ".data\0\0", 8) == 0)
				{
					const auto start = reinterpret_cast<uintptr_t>(base) + sections[i].VirtualAddress;
					return std::pair<uintptr_t, uintptr_t>(start, start + sections[i].Misc.VirtualSize);
				}
			return std::pair<uintptr_t, uintptr_t>(0, 0);
		}();

		const auto address = reinterpret_cast<uintptr_t>(p);
		return address >= data.first && address < data.second;
	}
}
