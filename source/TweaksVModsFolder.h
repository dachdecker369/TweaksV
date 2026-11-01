// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the mods folder's one walk, which RageOpenV's list and its check for packs made for GTA V Legacy both go by, so
// that the two list the same files the same way and stop at the same name; its limits; and the spelling of a path in the
// list, which the walk and RageOpenV's lookups share (SPECIFICATIONS 11.6).
#pragma once

#include <windows.h>

#include <cstddef>
#include <string_view>

namespace TweaksV
{
	// ---- the limits

	constexpr size_t kMaxPathText = 259;			// bytes of a game path the game's file functions read (MAX_PATH less the terminator)
	constexpr char kModsPrefix[] = "mods/";
	constexpr size_t kModsPrefixSize = sizeof(kModsPrefix) - 1;
	// A path of the list, and a request redirected, at most: a hook hands the game "mods/" and the path, and its functions
	// read kMaxPathText bytes of that - a longer path would be cut short.
	constexpr size_t kMaxModsPath = kMaxPathText - kModsPrefixSize;
	constexpr size_t kMaxFolded = 3 * kMaxModsPath;	// what the fold can make of kMaxModsPath bytes: 3 bytes of UTF-8 per UTF-16 unit
	constexpr size_t kMaxModsEntries = 200000;		// files and folders a walk hands on at most
	constexpr size_t kMaxModsDepth = 32;			// folders deep, the mods folder the first

	// ---- the spelling of a path in the list

	// The list's spelling of a game path, as Windows compares file names: '/' between the names, a
	// doubled '/' inside collapsed, the letters in Windows' file-system upper case - on the bytes for a
	// path of ASCII alone, through UTF-16 for any other. Upper case, not lower: the file systems compare
	// upcased names, and the two differ - the Kelvin sign and k share a lower case, not an upper case.
	// False for an empty path; one longer than kMaxModsPath bytes, which the game's function would read
	// cut short behind "mods/" - checked before the fold, since an upper case can take more bytes than
	// its letter, so a longer request can fold to a listed name; one that is not valid UTF-8; and one
	// that is not relative - it starts with a separator or a drive ("C:"): only a relative path names a
	// file of the game folder.
	bool ListSpelling(const char* path, char (&out)[kMaxFolded + 1], size_t& length);

	// ---- the walk

	// The mods folder as a walk lists it: relative to the current directory, where the game's own file functions resolve
	// "mods/..." too. So a walk reaches what the game reaches: what Windows refuses the game - a full path past MAX_PATH, in
	// a process that is not long-path aware - it refuses the walk, and whoever opens a name by the path the walk gives it.
	constexpr wchar_t kModsRoot[] = L"mods";

	enum class ModsEntryKind
	{
		File,				// a file of the list
		Folder,				// a folder of the list, which the walk goes on to list
		FolderTooDeep,		// a folder of the list at kMaxModsDepth: what it holds is not read
		FolderNotListed,	// a folder handed on as Folder before, which could not be listed: error says why
		NotUnicode,			// a name left out: not valid Unicode (an unpaired surrogate), which UTF-8 cannot spell, or one the fold refuses
		TooLong,			// a name left out: its path would be longer than kMaxModsPath bytes
	};

	// A name, as the walk hands it on: valid during the call. A name left out has only its kind.
	struct ModsEntry
	{
		ModsEntryKind kind;
		std::wstring_view path;			// as Windows opens it: mods\update\update.rpf
		std::string_view below;			// its path below the folder walked, in UTF-8, '/' between the names: update/update.rpf
		std::string_view spelling;		// that path as the lookups spell it (ListSpelling) - empty for a folder not listed
		DWORD error = ERROR_SUCCESS;	// why a folder could not be listed
	};

	// Who a walk reports to: entry for every name, while its folder's find handle is open, and folderClosed, which may be
	// null, once a folder's names have all been handed on and the handle is closed. Either may throw, which ends the walk,
	// every handle closed.
	struct ModsVisitor
	{
		void* context;
		void (*entry)(void* context, const ModsEntry& entry);
		void (*folderClosed)(void* context);
	};

	struct ModsWalk
	{
		DWORD rootError = ERROR_SUCCESS;	// why the folder walked could not be listed: nothing was handed on
		bool stopped = false;				// the walk stopped at its limit of files and folders
	};

	// Walks root, kModsRoot unless a test asks otherwise, on the calling thread: a folder's names in the order Windows lists
	// them, then the folders it holds, each folder listed pushed and the last pushed read next, so that every walk of one
	// folder comes to its names in one order. A name that is not valid Unicode, whose path below root is longer than
	// kMaxModsPath bytes or that the fold refuses is left out, not counted: the game cannot ask for it. Every other name
	// counts, one spelled like an earlier name too, and the first past kMaxModsEntries ends the walk, not handed on. A folder
	// at depth kMaxModsDepth is handed on, but what it holds is not read; a folder that cannot be listed is handed on again
	// as such and the walk goes on - only root ends it, in rootError; a folder that holds nothing, not even "."
	// (ERROR_FILE_NOT_FOUND: a drive's root behind a junction), is empty.
	ModsWalk WalkModsFolder(const wchar_t* root, const ModsVisitor& visitor);
}
