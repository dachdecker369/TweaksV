// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the mods folder's one walk and the spelling of a path in the list (see TweaksVModsFolder.h).

#include "TweaksVModsFolder.h"

#include <cstring>
#include <cwchar>
#include <string>
#include <utility>
#include <vector>

#ifdef TWEAKSV_TEST_BUILD
#include "TweaksVLog.h"
#endif

namespace TweaksV
{
	namespace
	{
		// A find handle closed on every way out, an exception's too.
		struct FindHandle
		{
			HANDLE handle;
			~FindHandle() { if (handle != INVALID_HANDLE_VALUE) FindClose(handle); }
		};

		// Half of a character outside the Basic Multilingual Plane - or a half alone, which is not valid UTF-16.
		bool IsSurrogate(wchar_t unit)
		{
			return IS_HIGH_SURROGATE(unit) || IS_LOW_SURROGATE(unit);
		}

		// ListSpelling for a path with a byte outside ASCII, through UTF-16 on the stack, the letters
		// upcased by LCMapStringEx's file-system casing (no LCMAP_LINGUISTIC_CASING). False if the path is
		// longer than kMaxModsPath bytes or not valid UTF-8 - the game's function would read such bytes as
		// replacement characters, so the path names no file of the mods folder. Windows' file systems upcase
		// a name unit by unit, by a table of the Basic Multilingual Plane, so a character outside it - a
		// surrogate pair - keeps its case: only the runs between surrogates are upcased.
		bool FoldOutsideAscii(const char* path, size_t size, char (&out)[kMaxFolded + 1], size_t& length)
		{
			length = 0;
			wchar_t wide[kMaxModsPath];
			const int units = size <= kMaxModsPath
				? MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, static_cast<int>(size), wide, static_cast<int>(kMaxModsPath)) : 0;
			if (units <= 0)
				return false;
			int kept = 0;
			for (int i = 0; i < units; i++)
			{
				const wchar_t c = wide[i] == L'\\' ? L'/' : wide[i];
				if (c == L'/' && kept > 0 && wide[kept - 1] == L'/')
					continue;
				wide[kept++] = c;
			}
			wchar_t upper[kMaxModsPath];
			for (int start = 0; start < kept;)
			{
				int end = start;
				while (end < kept && !IsSurrogate(wide[end]))
					end++;
				if (end == start)
				{
					upper[start] = wide[start];
					start++;
					continue;
				}
				const int run = end - start;
				if (LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_UPPERCASE, wide + start, run, upper + start, run, nullptr, nullptr, 0) != run)
					return false;
				start = end;
			}
			const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, upper, kept, out, static_cast<int>(kMaxFolded), nullptr, nullptr);
			if (bytes <= 0)
				return false;
			length = static_cast<size_t>(bytes);
			out[length] = '\0';
			return true;
		}

		// The next name of a listing, FindNextFileW's, the last error saying why when there is none - in test builds
		// TWEAKSV_TEST_MODS_FAIL_LISTING=<folder> in the environment has the listing of that folder, by the path a walk gives
		// it, fail after its first name but "." and "..", with the error of a share that drops, read once and said once, so
		// that a suite reaches a listing that fails half-way (SPECIFICATIONS 13.1).
		bool NextName(HANDLE find, WIN32_FIND_DATAW& found, const std::wstring& folder)
		{
#ifdef TWEAKSV_TEST_BUILD
			static const std::wstring failing = []
			{
				wchar_t text[MAX_PATH]{};
				const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_MODS_FAIL_LISTING", text, MAX_PATH);
				if (length == 0 || length >= MAX_PATH)
					return std::wstring();
				const std::wstring value(text, length);
				WriteLog("RageOpenV", "TEST BUILD: the listing of " + ToUtf8(value) + " fails after its first name with Windows error " +
					std::to_string(ERROR_NETNAME_DELETED) + " (TWEAKSV_TEST_MODS_FAIL_LISTING=" + ToUtf8(value) + ").");
				return value;
			}();
			if (folder == failing && wcscmp(found.cFileName, L".") != 0 && wcscmp(found.cFileName, L"..") != 0)
			{
				SetLastError(ERROR_NETNAME_DELETED);
				return false;
			}
#else
			(void)folder;
#endif
			return FindNextFileW(find, &found) != FALSE;
		}
	}

	bool ListSpelling(const char* path, char (&out)[kMaxFolded + 1], size_t& length)
	{
		length = 0;
		if (path[0] == '\\' || path[0] == '/' || (path[0] != '\0' && path[1] == ':'))
			return false;
		for (size_t i = 0; path[i]; i++)
		{
			if (i == kMaxModsPath)
				return false;
			if (static_cast<unsigned char>(path[i]) >= 0x80)
				return FoldOutsideAscii(path, i + strnlen(path + i, kMaxModsPath + 1 - i), out, length);
			char c = path[i] == '\\' ? '/' : path[i];
			if (c == '/' && length > 0 && out[length - 1] == '/')
				continue;
			if (c >= 'a' && c <= 'z')
				c = static_cast<char>(c - 'a' + 'A');
			out[length++] = c;
		}
		out[length] = '\0';
		return length > 0;
	}

	size_t ModsEntryLimit()
	{
#ifdef TWEAKSV_TEST_BUILD
		static const size_t limit = []
		{
			wchar_t text[8]{};
			const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_MODS_ENTRIES", text, 8);
			if (length > 0 && length < 8)
			{
				wchar_t* end = nullptr;
				const long entries = wcstol(text, &end, 10);
				if (end && *end == L'\0' && entries >= 1 && entries <= static_cast<long>(kMaxModsEntries))
				{
					WriteLog("RageOpenV", "TEST BUILD: the walk of the mods folder stops at " + std::to_string(entries) + " files and folders instead of " +
						std::to_string(kMaxModsEntries) + " (TWEAKSV_TEST_MODS_ENTRIES=" + std::to_string(entries) + ").");
					return static_cast<size_t>(entries);
				}
			}
			return kMaxModsEntries;
		}();
		return limit;
#else
		return kMaxModsEntries;
#endif
	}

	ModsWalk WalkModsFolder(const wchar_t* root, const ModsVisitor& visitor)
	{
		struct Folder { std::wstring path; std::string below; size_t depth; };
		std::vector<Folder> todo{ { root, std::string(), 1 } };
		const auto leftOut = [&visitor](ModsEntryKind kind) { visitor.entry(visitor.context, { kind }); };
		const size_t limit = ModsEntryLimit();
		ModsWalk walk;
		size_t entries = 0;
		std::wstring path;	// the name's, kept from name to name
		std::string below;
		while (!todo.empty() && !walk.stopped)
		{
			const Folder folder = std::move(todo.back());
			todo.pop_back();
			{
				WIN32_FIND_DATAW found{};
				const FindHandle find{ FindFirstFileExW((folder.path + L"\\*").c_str(), FindExInfoBasic, &found, FindExSearchNameMatch, nullptr,
					FIND_FIRST_EX_LARGE_FETCH) };
				if (find.handle == INVALID_HANDLE_VALUE)
				{
					const DWORD error = GetLastError();
					if (error == ERROR_FILE_NOT_FOUND)	// nothing in it, not even "." - a drive's root behind a junction
						continue;
					if (folder.below.empty())
					{
						walk.rootError = error;
						break;
					}
					visitor.entry(visitor.context, { ModsEntryKind::FolderNotListed, folder.path, folder.below, {}, error });
					continue;
				}
				do
				{
					if (wcscmp(found.cFileName, L".") == 0 || wcscmp(found.cFileName, L"..") == 0)
						continue;
					char name[kMaxModsPath + 1];
					const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, found.cFileName, -1, name, sizeof(name), nullptr, nullptr);
					if (size <= 1)
					{
						leftOut(size == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER ? ModsEntryKind::TooLong : ModsEntryKind::NotUnicode);
						continue;
					}
					below.assign(folder.below);
					if (!below.empty())
						below += '/';
					below += name;
					if (below.size() > kMaxModsPath)
					{
						leftOut(ModsEntryKind::TooLong);
						continue;
					}
					// the lookups' fold: it refuses none of these paths - relative, valid UTF-8 and short enough - but one it
					// refused would be left out of every walk alike, never of one alone
					char spelling[kMaxFolded + 1];
					size_t length = 0;
					if (!ListSpelling(below.c_str(), spelling, length))
					{
						leftOut(ModsEntryKind::NotUnicode);
						continue;
					}
					if (++entries > limit)
					{
						walk.stopped = true;
						break;
					}
					path.assign(folder.path).append(L"\\").append(found.cFileName);
					const ModsEntryKind kind = (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ? ModsEntryKind::File
						: folder.depth < kMaxModsDepth ? ModsEntryKind::Folder : ModsEntryKind::FolderTooDeep;
					visitor.entry(visitor.context, { kind, path, below, std::string_view(spelling, length) });
					if (kind == ModsEntryKind::Folder)
						todo.push_back({ path, below, folder.depth + 1 });
				} while (NextName(find.handle, found, folder.path));
				if (!walk.stopped)
				{
					// no more names, or a listing that failed half-way, which must not pass for the end of the folder
					const DWORD error = GetLastError();
					if (error != ERROR_NO_MORE_FILES)
					{
						if (folder.below.empty() && entries == 0)	// nothing handed on: a root that cannot be listed
						{
							walk.rootError = error;
							break;
						}
						visitor.entry(visitor.context, { ModsEntryKind::FolderNotListed, folder.path, folder.below, {}, error, true });
					}
				}
			}
			if (visitor.folderClosed)
				visitor.folderClosed(visitor.context);
		}
		return walk;
	}
}
