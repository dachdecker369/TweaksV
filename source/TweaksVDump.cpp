// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - TEST_DUMP_CODE, TEST_DUMP_DATA and TEST_DUMP_SCRIPT, test builds only (see TweaksVDump.h).

#include "TweaksVDump.h"

#ifdef TWEAKSV_TEST_BUILD

#include <windows.h>
#include <psapi.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "TweaksVFiles.h"
#include "TweaksVIni.h"
#include "TweaksVLog.h"
#include "TweaksVPatch.h"
#include "TweaksVScripts.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "TweaksV";	// the switches are TweaksV's own, not a part's
		constexpr char kMagic[8] = { 'T', 'W', 'K', 'V', 'D', 'U', 'M', 'P' };
		constexpr uint32_t kFormatVersion = 1;
		constexpr unsigned long kMaxSeconds = 3600;
		constexpr WORD kMaxSections = 96;				// as many as Windows loads
		constexpr uint64_t kMaxBytes = 1ull << 30;		// 1 GB for the image and for a copy - the game is about 90 MB
		constexpr DWORD kReadable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
			PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

		// One copy of the game: the whole file, and what the log says about it.
		struct Copy
		{
			std::vector<uint8_t> file;
			std::string saved;		// " HEADERS .text ..." - the sections in it
			std::string leftOut;	// the sections the game writes to, not saved
			uint64_t unreadable = 0;
		};

		struct Part
		{
			std::string name;
			uint32_t rva;
			uint32_t size;
			uint32_t characteristics;
		};

		unsigned long g_seconds = 0;	// set once, before the thread starts

		// A section's name for the log and the file; a byte that cannot be printed becomes '?'.
		std::string SectionName(const BYTE (&raw)[IMAGE_SIZEOF_SHORT_NAME])
		{
			std::string name;
			for (BYTE c : raw)
			{
				if (!c)
					break;
				name += (c < 0x20 || c > 0x7E) ? '?' : static_cast<char>(c);
			}
			return name;
		}

		// A section's bytes in memory, as the game's section table gives them.
		uint64_t SectionLength(const IMAGE_SECTION_HEADER& section)
		{
			return section.Misc.VirtualSize ? section.Misc.VirtualSize : section.SizeOfRawData;
		}

		// True if text is a number of seconds from 1 to kMaxSeconds, in digits and nothing else.
		bool ParseSeconds(const std::string& text, unsigned long& seconds)
		{
			char* end = nullptr;
			seconds = strtoul(text.c_str(), &end, 10);
			return text[0] >= '0' && text[0] <= '9' && *end == '\0' && seconds >= 1 && seconds <= kMaxSeconds;
		}

		// Copies size bytes of the game from begin to out: what can be read as it is, the rest as zeros.
		// Returns how many bytes could not be read. A guard page is never touched.
		uint64_t CopyRange(const uint8_t* begin, size_t size, uint8_t* out)
		{
			uint64_t unreadable = 0;
			for (size_t done = 0; done < size;)
			{
				const uint8_t* at = begin + done;
				size_t take = size - done;
				bool readable = false;
				MEMORY_BASIC_INFORMATION info{};
				if (VirtualQuery(at, &info, sizeof(info)) == sizeof(info))
				{
					const uint8_t* regionEnd = static_cast<const uint8_t*>(info.BaseAddress) + info.RegionSize;
					if (regionEnd > at)
						take = std::min<size_t>(take, static_cast<size_t>(regionEnd - at));
					readable = info.State == MEM_COMMIT && (info.Protect & kReadable) && !(info.Protect & (PAGE_GUARD | PAGE_NOACCESS));
				}
				if (readable)
					memcpy(out + done, at, take);
				else
				{
					memset(out + done, 0, take);
					unreadable += take;
				}
				done += take;
			}
			return unreadable;
		}

		// The game's NT headers in memory, read only where they lie inside its image; nullptr if they are not
		// those of a program Windows loads.
		const IMAGE_NT_HEADERS* NtHeaders(const uint8_t* base, uint64_t imageSize)
		{
			const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
			const bool dosOk = imageSize >= sizeof(IMAGE_DOS_HEADER) && imageSize <= kMaxBytes && dos->e_magic == IMAGE_DOS_SIGNATURE &&
				dos->e_lfanew > 0 && static_cast<uint64_t>(dos->e_lfanew) + sizeof(IMAGE_NT_HEADERS) <= imageSize;
			const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + (dosOk ? dos->e_lfanew : 0));
			return dosOk && nt->Signature == IMAGE_NT_SIGNATURE ? nt : nullptr;
		}

		// The game in memory and its section table, which lies inside its image.
		struct GameImage
		{
			const uint8_t* base = nullptr;
			uint64_t size = 0;
			const IMAGE_NT_HEADERS* nt = nullptr;
			const IMAGE_SECTION_HEADER* sections = nullptr;
			WORD sectionCount = 0;
		};

		// Finds the game - or false, reported under key as "... - <what> is not saved.", if it is not in memory
		// or its headers there are not what Windows loads.
		bool FindGame(const char* key, const char* what, GameImage& game)
		{
			const HMODULE module = GetModuleHandleW(nullptr);
			MODULEINFO image{};
			if (!module || !GetModuleInformation(GetCurrentProcess(), module, &image, sizeof(image)))
			{
				ReportFailure(kTag, std::string(key) + ": the game was not found in memory - " + what + " is not saved.");
				return false;
			}
			game.base = static_cast<const uint8_t*>(image.lpBaseOfDll);
			game.size = image.SizeOfImage;

			// the headers are read only where they lie inside the image
			game.nt = NtHeaders(game.base, game.size);
			game.sections = game.nt ? IMAGE_FIRST_SECTION(game.nt) : nullptr;
			game.sectionCount = game.nt ? game.nt->FileHeader.NumberOfSections : 0;
			if (!game.nt || game.sectionCount == 0 || game.sectionCount > kMaxSections ||
				reinterpret_cast<const uint8_t*>(game.sections + game.sectionCount) > game.base + game.size)
			{
				ReportFailure(kTag, std::string(key) + ": the game's headers in memory are not those of a program Windows "
					"loads - " + what + " is not saved.");
				return false;
			}
			return true;
		}

		// A copy of the game as it is now - or nullptr, reported, if its headers in memory are not what
		// Windows loads. May throw std::bad_alloc.
		std::unique_ptr<Copy> TakeCopy(uint32_t secondsAfterStart)
		{
			GameImage game;
			if (!FindGame("TEST_DUMP_CODE", "its code", game))
				return nullptr;
			const uint8_t* const base = game.base;
			const uint64_t imageSize = game.size;
			const IMAGE_NT_HEADERS* const nt = game.nt;
			const IMAGE_SECTION_HEADER* const sections = game.sections;
			const WORD count = game.sectionCount;

			auto copy = std::make_unique<Copy>();
			std::vector<Part> parts;
			const uint64_t headers = std::min<uint64_t>(nt->OptionalHeader.SizeOfHeaders, imageSize);
			if (headers)
				parts.push_back({ "HEADERS", 0, static_cast<uint32_t>(headers), 0 });
			for (WORD i = 0; i < count; i++)
			{
				const IMAGE_SECTION_HEADER& section = sections[i];
				const std::string name = SectionName(section.Name);
				if ((section.Characteristics & IMAGE_SCN_MEM_WRITE) && !(section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
				{
					copy->leftOut += " " + name;	// the game's data: never saved
					continue;
				}
				const uint64_t length = SectionLength(section);
				if (section.VirtualAddress >= imageSize || length == 0)
					continue;
				parts.push_back({ name, section.VirtualAddress,
					static_cast<uint32_t>(std::min<uint64_t>(length, imageSize - section.VirtualAddress)), section.Characteristics });
			}

			uint64_t total = sizeof(DumpHeader) + parts.size() * sizeof(DumpSection);
			for (const Part& part : parts)
				total += part.size;
			if (total > kMaxBytes)
			{
				ReportFailure(kTag, "TEST_DUMP_CODE: the game's sections add up to more than 1 GB - its code is not saved.");
				return nullptr;
			}
			copy->file.resize(static_cast<size_t>(total));

			DumpHeader header{};
			memcpy(header.magic, kMagic, sizeof(kMagic));
			header.formatVersion = kFormatVersion;
			header.sectionCount = static_cast<uint32_t>(parts.size());
			header.imageBase = reinterpret_cast<uintptr_t>(base);
			header.imageSize = static_cast<uint32_t>(imageSize);
			header.timeDateStamp = nt->FileHeader.TimeDateStamp;
			header.secondsAfterStart = secondsAfterStart;
			memcpy(copy->file.data(), &header, sizeof(header));

			uint64_t at = sizeof(DumpHeader) + parts.size() * sizeof(DumpSection);
			for (size_t i = 0; i < parts.size(); i++)
			{
				const Part& part = parts[i];
				DumpSection entry{};
				memcpy(entry.name, part.name.data(), std::min<size_t>(part.name.size(), sizeof(entry.name)));
				entry.rva = part.rva;
				entry.size = part.size;
				entry.characteristics = part.characteristics;
				entry.fileOffset = at;
				const uint64_t unreadable = CopyRange(base + part.rva, part.size, copy->file.data() + at);
				entry.unreadable = static_cast<uint32_t>(unreadable);
				memcpy(copy->file.data() + sizeof(DumpHeader) + i * sizeof(DumpSection), &entry, sizeof(entry));
				copy->unreadable += unreadable;
				copy->saved += " " + part.name;
				at += part.size;
			}
			return copy;
		}

		// Writes bytes to %LOCALAPPDATA%\TweaksV\name, the folder created if need be and a file of that name
		// replaced; reported, what naming the switch and the file, if it cannot be. True once written whole.
		bool SaveFile(const std::string& what, const wchar_t* name, const std::vector<uint8_t>& bytes)
		{
			const std::wstring folder = GetFallbackFolder();
			if (folder.empty())
			{
				ReportFailure(kTag, what + " was not saved: LOCALAPPDATA is not set.");
				return false;
			}
			CreateDirectoryW(folder.c_str(), nullptr);
			const OwnFile file = OpenOwnFile(folder, name, FileFolder::Own, OPEN_ALWAYS, true);
			if (file.handle == INVALID_HANDLE_VALUE)
			{
				ReportFailure(kTag, what + " was not saved: " + file.Reason() + ".");
				return false;
			}

			DWORD error = ERROR_SUCCESS;
			for (size_t done = 0; error == ERROR_SUCCESS && done < bytes.size();)
			{
				const DWORD chunk = static_cast<DWORD>(std::min<size_t>(bytes.size() - done, 16u << 20));
				DWORD wrote = 0;
				if (!WriteFile(file.handle, bytes.data() + done, chunk, &wrote, nullptr))
					error = GetLastError();
				else if (wrote != chunk)
					error = ERROR_DISK_FULL;
				done += wrote;
			}
			CloseHandle(file.handle);
			if (error != ERROR_SUCCESS)
			{
				ReportFailure(kTag, what + " is incomplete: it could not be written to the end (" + DescribeError(error) + ").");
				return false;
			}
			return true;
		}

		// Writes a copy to %LOCALAPPDATA%\TweaksV\name; reported if it cannot be.
		void SaveCopy(const Copy& copy, const wchar_t* name, uint32_t secondsAfterStart)
		{
			if (!SaveFile("TEST_DUMP_CODE: " + ToUtf8(name), name, copy.file))
				return;

			const std::wstring folder = GetFallbackFolder();
			char size[32];
			snprintf(size, sizeof(size), "%.1f MB", static_cast<double>(copy.file.size()) / (1024.0 * 1024.0));
			WriteLog(kTag, "TEST BUILD: " + ToUtf8(name) + " saved, the game as it was " + std::to_string(secondsAfterStart) +
				" s after TweaksV started" + (secondsAfterStart ? std::string() : std::string(", before anything was patched")) + ": " +
				size + " in " + ToUtf8(folder) + " - sections" + copy.saved + "; " +
				(copy.unreadable ? std::to_string(copy.unreadable) + " bytes unreadable, saved as zeros" : std::string("none unreadable")) +
				"; not saved (the game writes to them):" + (copy.leftOut.empty() ? std::string(" none") : copy.leftOut) + ".");
		}

		DWORD WINAPI CopyThread(void* parameter)
		{
			std::unique_ptr<Copy> first(static_cast<Copy*>(parameter));
			try
			{
				SaveCopy(*first, L"GameCode-start.bin", 0);
				first.reset();	// not kept while waiting
				Sleep(g_seconds * 1000);
				if (const std::unique_ptr<Copy> later = TakeCopy(g_seconds))
					SaveCopy(*later, L"GameCode-later.bin", g_seconds);
			}
			catch (...)
			{
				ReportFailure(kTag, "TEST_DUMP_CODE: not enough memory for a copy of the game's code - not saved.");
			}
			return 0;
		}

		// ---- TEST_DUMP_SCRIPT

		constexpr char kScriptMagic[8] = { 'T', 'W', 'K', 'V', 'S', 'C', 'R', 'P' };
		constexpr uint32_t kScriptFormatVersion = 1;
		constexpr size_t kMaxScriptName = 63;
		static_assert(Scripts::kNameRead == kMaxScriptName + 1, "TweaksV: a program's name is read within the longest name and its zero");
		constexpr uint32_t kMaxScriptCopies = 16;			// a session

		// Set before the hook can call: the name asked for, in lower case, and its hash as the game makes it
		std::string g_script;
		uint32_t g_scriptHash = 0;
		uint64_t g_imageBase = 0;
		uint32_t g_imageSize = 0;
		uint32_t g_timeDateStamp = 0;
		// On the game's thread that places the programs
		std::atomic<uint32_t> g_placements{ 0 };	// the program's placements, saved or not
		std::atomic<bool> g_firstPlacement{ true };
		std::atomic<bool> g_halfMatchLogged{ false };
		std::atomic<bool> g_unreadableLogged{ false };
		std::atomic<bool> g_capLogged{ false };

		// One copy in the making, from before the game resolves the natives to after.
		struct ScriptCopy
		{
			ScriptDumpHeader header{};
			std::vector<uint8_t> file;	// the whole file; the header goes in last
			std::wstring name;			// Script-<name>-<n>.bin
		};

		std::string Ordinal(uint32_t n)
		{
			const uint32_t lastTwo = n % 100;
			const char* const suffix = lastTwo >= 11 && lastTwo <= 13 ? "th" : n % 10 == 1 ? "st" : n % 10 == 2 ? "nd" : n % 10 == 3 ? "rd" : "th";
			return std::to_string(n) + suffix;
		}

		// "1 page", "2 pages"
		std::string Counted(uint32_t n, const char* thing)
		{
			return std::to_string(n) + " " + thing + (n == 1 ? "" : "s");
		}

		// True if a name Scripts::ReadName read is the one asked for, A to Z taken as a to z.
		bool IsAskedFor(const char (&name)[Scripts::kNameRead])
		{
			const size_t length = strnlen(name, Scripts::kNameRead);
			if (length != g_script.size())
				return false;
			for (size_t i = 0; i < length; i++)
				if ((name[i] >= 'A' && name[i] <= 'Z' ? name[i] - 'A' + 'a' : name[i]) != g_script[i])
					return false;
			return true;
		}

		// size bytes from address to out where they can be read, as zeros where not; the bytes that could not.
		uint32_t CopyBlock(const uint8_t* address, size_t size, uint8_t* out)
		{
			if (size == 0)
				return 0;
			if (IsReadable(address, size))
			{
				memcpy(out, address, size);
				return 0;
			}
			memset(out, 0, size);
			return static_cast<uint32_t>(size);
		}

		// length bytes from the pages the page table at table lists, page after page, to out, each page as
		// CopyBlock copies it; the bytes that could not be read.
		uint32_t CopyPages(const uint8_t* table, uint32_t length, uint8_t* out)
		{
			const uint32_t pages = Scripts::PageCount(length);
			if (pages == 0)
				return 0;
			if (!IsReadable(table, size_t{ pages } * sizeof(void*)))
			{
				memset(out, 0, length);
				return length;
			}
			uint32_t unreadable = 0;
			for (uint32_t page = 0; page < pages; page++)
				unreadable += CopyBlock(ValueAt<const uint8_t*>(table, size_t{ page } * sizeof(void*)), Scripts::PageLength(length, page),
					out + size_t{ page } * Scripts::kPageSize);
			return unreadable;
		}

		// Before the game resolves a program's natives: the program asked for gets its copy started - the file
		// allocated and the native table copied while it still holds the hashes. A program with only the name
		// or only the hash is logged once; the first program of all is logged, whatever its name.
		void* BeforeResolution(uint8_t* program, uint32_t placed)
		{
			if (!IsReadable(program, Scripts::kProgramSize))
			{
				if (!g_unreadableLogged.exchange(true))
					WriteLog(kTag, "TEST BUILD: TEST_DUMP_SCRIPT: a script program at " + DescribeAddress(program) + " cannot be read, so it "
						"is not looked at (logged once).");
				return nullptr;
			}
			const uint32_t hash = ValueAt<uint32_t>(program, Scripts::kNameHashOffset);
			char name[Scripts::kNameRead];
			const bool named = Scripts::ReadName(ValueAt<const char*>(program, Scripts::kNameOffset), name);
			const auto shown = [&] { return named ? Printable(name) : std::string("(a name that cannot be read)"); };
			if (g_firstPlacement.exchange(false))
				WriteLog(kTag, "TEST BUILD: TEST_DUMP_SCRIPT: the first script program the game placed is " + shown() + " (hash " + Hex32(hash) +
					"), on thread " + std::to_string(GetCurrentThreadId()) + " - the hook runs.");

			const bool hashMatches = hash == g_scriptHash;
			const bool nameMatches = named && IsAskedFor(name);
			if (!hashMatches && !nameMatches)
				return nullptr;
			if (!hashMatches || !nameMatches)
			{
				if (!g_halfMatchLogged.exchange(true))
					WriteLog(kTag, "TEST BUILD: TEST_DUMP_SCRIPT: a script program " + (hashMatches ? "has the hash of " + g_script + ", " +
						Hex32(g_scriptHash) + ", but is named " + shown() : "is named " + shown() + " but has the hash " + Hex32(hash) + ", not " +
						Hex32(g_scriptHash)) + " - it is not saved (logged once).");
				return nullptr;
			}

			const uint32_t placement = g_placements.fetch_add(1) + 1;
			if (placement > kMaxScriptCopies)
			{
				if (!g_capLogged.exchange(true))
					WriteLog(kTag, "TEST BUILD: " + std::to_string(kMaxScriptCopies) + " copies of " + g_script + " saved - from now on its "
						"placements are only counted.");
				return nullptr;
			}
			const std::wstring file = L"Script-" + FromUtf8(g_script) + L"-" + std::to_wstring(placement) + L".bin";
			const std::string what = "TEST_DUMP_SCRIPT: " + ToUtf8(file);
			const uint32_t codeLength = ValueAt<uint32_t>(program, Scripts::kCodeLengthOffset);
			const uint32_t nativeCount = ValueAt<uint32_t>(program, Scripts::kNativeCountOffset);
			const uint32_t stringLength = ValueAt<uint32_t>(program, Scripts::kStringLengthOffset);
			std::string beyond;
			if (codeLength > Scripts::kMaxLength)
				beyond = "its code length, " + std::to_string(codeLength) + " bytes, is beyond 16 MB.";
			else if (nativeCount > Scripts::kMaxNatives)
				beyond = "its native count, " + std::to_string(nativeCount) + ", is beyond " + std::to_string(Scripts::kMaxNatives) + ".";
			else if (stringLength > Scripts::kMaxLength)
				beyond = "its string length, " + std::to_string(stringLength) + " bytes, is beyond 16 MB.";
			if (!beyond.empty())
			{
				ReportFailure(kTag, what + " was not saved: " + beyond);
				return nullptr;
			}

			try
			{
				auto copy = std::make_unique<ScriptCopy>();
				copy->name = file;
				ScriptDumpHeader& header = copy->header;
				memcpy(header.magic, kScriptMagic, sizeof(kScriptMagic));
				header.formatVersion = kScriptFormatVersion;
				header.headerSize = sizeof(ScriptDumpHeader);
				memcpy(header.name, name, sizeof(header.name));
				header.nameHash = hash;
				header.codeLength = codeLength;
				header.nativeCount = nativeCount;
				header.stringLength = stringLength;
				header.placement = placement;
				header.threadId = GetCurrentThreadId();
				header.programsPlaced = placed;
				header.imageBase = g_imageBase;
				header.imageSize = g_imageSize;
				header.timeDateStamp = g_timeDateStamp;
				const uint64_t natives = uint64_t{ nativeCount } * sizeof(uint64_t);
				header.programOffset = sizeof(ScriptDumpHeader);
				header.codeOffset = header.programOffset + Scripts::kProgramSize;
				header.nativesBeforeOffset = header.codeOffset + codeLength;
				header.nativesAfterOffset = header.nativesBeforeOffset + natives;
				header.stringsOffset = header.nativesAfterOffset + natives;
				copy->file.resize(static_cast<size_t>(header.stringsOffset + stringLength));
				header.unreadable = CopyBlock(ValueAt<const uint8_t*>(program, Scripts::kNativesOffset), static_cast<size_t>(natives),
					copy->file.data() + header.nativesBeforeOffset);
				return copy.release();
			}
			catch (...)
			{
				ReportFailure(kTag, what + " was not saved: not enough memory for a copy of " + g_script + ".");
				return nullptr;
			}
		}

		// After the game has resolved the natives: the rest of the copy - the program object, its code, the native
		// table as the game left it, its strings - and the file.
		void AfterResolution(uint8_t* program, uint8_t resolved, void* context)
		{
			const std::unique_ptr<ScriptCopy> copy(static_cast<ScriptCopy*>(context));
			if (!copy)
				return;
			ScriptDumpHeader& header = copy->header;
			uint8_t* const file = copy->file.data();
			const bool readable = IsReadable(program, Scripts::kProgramSize);
			const auto pointer = [&](size_t offset) { return readable ? ValueAt<const uint8_t*>(program, offset) : nullptr; };
			header.resolved = resolved;
			header.unreadable += CopyBlock(program, Scripts::kProgramSize, file + header.programOffset);
			header.unreadable += CopyPages(pointer(Scripts::kCodePagesOffset), header.codeLength, file + header.codeOffset);
			header.unreadable += CopyBlock(pointer(Scripts::kNativesOffset), static_cast<size_t>(header.nativesAfterOffset - header.nativesBeforeOffset),
				file + header.nativesAfterOffset);
			header.unreadable += CopyPages(pointer(Scripts::kStringPagesOffset), header.stringLength, file + header.stringsOffset);
			memcpy(file, &header, sizeof(header));

			try
			{
				if (!SaveFile("TEST_DUMP_SCRIPT: " + ToUtf8(copy->name), copy->name.c_str(), copy->file))
					return;
				WriteLog(kTag, "TEST BUILD: " + ToUtf8(copy->name) + " saved in " + ToUtf8(GetFallbackFolder()) + ": " + g_script + " placed the " +
					Ordinal(header.placement) + " time, as the game's " + Ordinal(header.programsPlaced) + " script program, on thread " +
					std::to_string(header.threadId) + " - code " + Counted(header.codeLength, "byte") + " in " +
					Counted(Scripts::PageCount(header.codeLength), "page") + ", " + Counted(header.nativeCount, "native") + ", " +
					(resolved == 1 ? "every one resolved" : "not every one resolved") + ", strings " + Counted(header.stringLength, "byte") + "; " +
					(header.unreadable ? Counted(header.unreadable, "byte") + " unreadable, saved as zeros" : std::string("none unreadable")) + ".");
			}
			catch (...)
			{
				ReportFailure(kTag, "TEST_DUMP_SCRIPT: " + ToUtf8(copy->name) + " was not saved: not enough memory.");
			}
		}

		constexpr Scripts::User kScriptDumper = { kTag, &BeforeResolution, &AfterResolution };

		// ---- TEST_DUMP_DATA

		constexpr char kDataMagic[8] = { 'T', 'W', 'K', 'V', 'D', 'A', 'T', 'A' };
		constexpr uint32_t kDataFormatVersion = 1;
		constexpr size_t kMaxWindows = 16;
		constexpr uint32_t kMaxWindowBytes = 64 * 1024;
		constexpr size_t kMaxDataValue = 254;	// a key's value is read up to 255 characters: one that long may be cut short

		// One window: its place, and its file - the header and the window's bytes - from the first copy on.
		struct DataWindow
		{
			uint32_t rva = 0;
			uint32_t size = 0;
			uint32_t unreadable = 0;
			std::vector<uint8_t> file;
		};

		// What the key asks for and what the files say about the game: set before the thread starts.
		struct DataDump
		{
			const uint8_t* base = nullptr;
			uint32_t imageSize = 0;
			uint32_t timeDateStamp = 0;
			uint32_t seconds = 0;
			std::vector<DataWindow> windows;
		};

		// True if text is a number in hex of 1 to 8 digits, 0x in front or not.
		bool ParseHex(std::string_view text, uint32_t& number)
		{
			if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
				text.remove_prefix(2);
			if (text.empty() || text.size() > 8)
				return false;
			number = 0;
			for (const char c : text)
			{
				const int digit = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
				if (digit < 0)
					return false;
				number = number * 16 + static_cast<uint32_t>(digit);
			}
			return true;
		}

		// Reads the key's seconds and windows into dump - or false, with reason saying why it is not what the key
		// takes. Where the windows lie is checked afterwards, against the game.
		bool ParseDataDump(const std::string& value, DataDump& dump, std::string& reason)
		{
			if (value.size() > kMaxDataValue)
			{
				reason = "it is longer than the " + std::to_string(kMaxDataValue) + " characters a key can hold whole";
				return false;
			}
			const size_t comma = value.find(',');
			unsigned long seconds = 0;
			if (!ParseSeconds(value.substr(0, comma), seconds))
			{
				reason = "it does not start with a number of seconds from 1 to " + std::to_string(kMaxSeconds);
				return false;
			}
			dump.seconds = seconds;
			if (comma == std::string::npos)
			{
				reason = "it names no window";
				return false;
			}
			for (size_t begin = comma + 1, end = 0; end != std::string::npos; begin = end + 1)
			{
				end = value.find(',', begin);
				const std::string window = value.substr(begin, end == std::string::npos ? end : end - begin);
				const size_t colon = window.find(':');
				DataWindow parsed;
				if (colon == std::string::npos || !ParseHex(std::string_view(window).substr(0, colon), parsed.rva) ||
					!ParseHex(std::string_view(window).substr(colon + 1), parsed.size))
				{
					reason = "\"" + window + "\" is not a window of <rva>:<bytes>, both in hex";
					return false;
				}
				if (dump.windows.size() == kMaxWindows)
				{
					reason = "it names more than " + std::to_string(kMaxWindows) + " windows";
					return false;
				}
				if (parsed.size == 0 || parsed.size > kMaxWindowBytes)
				{
					reason = "the window " + window + " is not of 1 byte to 64 KB (10000 in hex)";
					return false;
				}
				for (const DataWindow& other : dump.windows)
					if (other.rva == parsed.rva)
					{
						reason = "the window at " + Hex(parsed.rva) + " is named twice, and its files have one name";
						return false;
					}
				dump.windows.push_back(parsed);
			}
			return true;
		}

		// Where the section the game's section table names .data lies; false if it names none.
		bool FindDataSection(const GameImage& game, uint64_t& begin, uint64_t& end)
		{
			for (WORD i = 0; i < game.sectionCount; i++)
			{
				const IMAGE_SECTION_HEADER& section = game.sections[i];
				if (SectionName(section.Name) == ".data" && section.VirtualAddress < game.size)
				{
					begin = section.VirtualAddress;
					end = std::min(begin + SectionLength(section), game.size);
					return true;
				}
			}
			return false;
		}

		// The window as it is now, as the whole of its file.
		void TakeWindow(const DataDump& dump, DataWindow& window, uint32_t secondsAfterStart)
		{
			DataDumpHeader header{};
			memcpy(header.magic, kDataMagic, sizeof(kDataMagic));
			header.formatVersion = kDataFormatVersion;
			header.headerSize = sizeof(DataDumpHeader);
			header.imageBase = reinterpret_cast<uintptr_t>(dump.base);
			header.imageSize = dump.imageSize;
			header.timeDateStamp = dump.timeDateStamp;
			header.secondsAfterStart = secondsAfterStart;
			header.rva = window.rva;
			header.size = window.size;
			window.unreadable = static_cast<uint32_t>(CopyRange(dump.base + window.rva, window.size, window.file.data() + sizeof(header)));
			header.unreadable = window.unreadable;
			memcpy(window.file.data(), &header, sizeof(header));
		}

		// Writes a window's file to %LOCALAPPDATA%\TweaksV; reported if it cannot be.
		void SaveWindow(const DataWindow& window, const wchar_t* moment, uint32_t secondsAfterStart)
		{
			const std::wstring name = L"GameData-" + FromUtf8(Hex(window.rva).substr(2)) + L"-" + moment + L".bin";
			if (!SaveFile("TEST_DUMP_DATA: " + ToUtf8(name), name.c_str(), window.file))
				return;
			WriteLog(kTag, "TEST BUILD: " + ToUtf8(name) + " saved, " + Counted(window.size, "byte") + " of the game's .data at RVA " + Hex(window.rva) +
				" as they were " + std::to_string(secondsAfterStart) + " s after TweaksV started" +
				(secondsAfterStart ? std::string() : std::string(", before anything was patched")) + ", in " + ToUtf8(GetFallbackFolder()) + "; " +
				(window.unreadable ? Counted(window.unreadable, "byte") + " unreadable, saved as zeros" : std::string("none unreadable")) + ".");
		}

		DWORD WINAPI DataThread(void* parameter)
		{
			const std::unique_ptr<DataDump> dump(static_cast<DataDump*>(parameter));
			try
			{
				for (const DataWindow& window : dump->windows)
					SaveWindow(window, L"start", 0);
				Sleep(dump->seconds * 1000);
				for (DataWindow& window : dump->windows)	// the windows at one moment, then the files
					TakeWindow(*dump, window, dump->seconds);
				for (const DataWindow& window : dump->windows)
					SaveWindow(window, L"later", dump->seconds);
			}
			catch (...)
			{
				ReportFailure(kTag, "TEST_DUMP_DATA: not enough memory for a file of the game's data - not saved.");
			}
			return 0;
		}
	}

	void StartCodeDump()
	{
		const std::string& value = GetText(kTestDumpCode);
		if (value.empty())
			return;
		unsigned long seconds = 0;
		if (!ParseSeconds(value, seconds))
		{
			ReportFailure(kTag, "TweaksV.ini: [TWEAKSV] TEST_DUMP_CODE=" + value + " is not a number of seconds from 1 to " +
				std::to_string(kMaxSeconds) + " - the game's code is not saved.");
			return;
		}
		g_seconds = seconds;
		try
		{
			std::unique_ptr<Copy> first = TakeCopy(0);	// now: before any part of TweaksV changes the game
			if (!first)
				return;
			// DllMain does not wait for the thread: it writes this copy, waits and takes the second
			const HANDLE thread = CreateThread(nullptr, 0, CopyThread, first.get(), 0, nullptr);
			if (!thread)
			{
				ReportFailure(kTag, "TEST_DUMP_CODE: its thread could not be started (" + DescribeError(GetLastError()) +
					") - the game's code is not saved.");
				return;
			}
			static_cast<void>(first.release());	// the thread owns it now
			CloseHandle(thread);
		}
		catch (...)
		{
			ReportFailure(kTag, "TEST_DUMP_CODE: not enough memory for a copy of the game's code - not saved.");
		}
	}

	void StartDataDump()
	{
		const std::string& value = GetText(kTestDumpData);
		if (value.empty())
			return;
		const auto refuse = [&](const std::string& reason)
		{
			ReportFailure(kTag, "TweaksV.ini: [TWEAKSV] TEST_DUMP_DATA=" + value + ": " + reason + " - the game's data is not saved.");
		};
		try
		{
			auto dump = std::make_unique<DataDump>();
			std::string reason;
			if (!ParseDataDump(value, *dump, reason))
			{
				refuse(reason);
				return;
			}
			GameImage game;
			if (!FindGame("TEST_DUMP_DATA", "its data", game))
				return;
			uint64_t begin = 0, end = 0;
			if (!FindDataSection(game, begin, end))
			{
				refuse("the game's section table names no .data");
				return;
			}
			for (const DataWindow& window : dump->windows)
				if (window.rva < begin || uint64_t{ window.rva } + window.size > end)
				{
					refuse("the window of " + Hex(window.size) + " bytes at " + Hex(window.rva) + " is not wholly inside the game's .data, " +
						Hex(begin) + " to " + Hex(end));
					return;
				}

			dump->base = game.base;
			dump->imageSize = static_cast<uint32_t>(game.size);
			dump->timeDateStamp = game.nt->FileHeader.TimeDateStamp;
			for (DataWindow& window : dump->windows)	// now: before any part of TweaksV changes the game
			{
				window.file.resize(sizeof(DataDumpHeader) + window.size);
				TakeWindow(*dump, window, 0);
			}
			// DllMain does not wait for the thread: it writes these copies, waits and takes the second
			const HANDLE thread = CreateThread(nullptr, 0, DataThread, dump.get(), 0, nullptr);
			if (!thread)
			{
				ReportFailure(kTag, "TEST_DUMP_DATA: its thread could not be started (" + DescribeError(GetLastError()) +
					") - the game's data is not saved.");
				return;
			}
			static_cast<void>(dump.release());	// the thread owns it now
			CloseHandle(thread);
		}
		catch (...)
		{
			ReportFailure(kTag, "TEST_DUMP_DATA: not enough memory for a copy of the game's data - not saved.");
		}
	}

	void StartScriptDump()
	{
		const std::string& value = GetText(kTestDumpScript);
		if (value.empty())
			return;
		bool isName = value.size() <= kMaxScriptName;
		for (const char c : value)
			isName = isName && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
		if (!isName)
		{
			ReportFailure(kTag, "TweaksV.ini: [TWEAKSV] TEST_DUMP_SCRIPT=" + value + " is not a script's name of 1 to " +
				std::to_string(kMaxScriptName) + " letters, digits and _ - no script program is saved.");
			return;
		}
		const std::wstring folder = GetFallbackFolder();
		if (folder.empty())
		{
			ReportFailure(kTag, "TEST_DUMP_SCRIPT: LOCALAPPDATA is not set - no script program is saved.");
			return;
		}

		// what the hook needs, before it can run: the name and its hash, and the game for the files' headers
		g_script = value;
		for (char& c : g_script)
			c = static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
		g_scriptHash = Scripts::NameHash(g_script);
		const HMODULE module = GetModuleHandleW(nullptr);
		MODULEINFO image{};
		if (module && GetModuleInformation(GetCurrentProcess(), module, &image, sizeof(image)))
		{
			g_imageBase = reinterpret_cast<uintptr_t>(image.lpBaseOfDll);
			g_imageSize = image.SizeOfImage;
			if (const IMAGE_NT_HEADERS* const nt = NtHeaders(static_cast<const uint8_t*>(image.lpBaseOfDll), image.SizeOfImage))
				g_timeDateStamp = nt->FileHeader.TimeDateStamp;
		}

		std::string reason;
		if (!Scripts::AddUser(kScriptDumper, reason))
		{
			ReportFailure(kTag, "TEST_DUMP_SCRIPT: the script program " + g_script + " is not saved this session: " + reason);
			return;
		}
		if (IsDryRun())
			WriteLog(kTag, "TEST BUILD: TEST_DUMP_SCRIPT=" + value + ": DRY_RUN - the call is not redirected, so no script program is saved.");
		else
			WriteLog(kTag, "TEST BUILD: TEST_DUMP_SCRIPT=" + value + ": the script program " + g_script + " (hash " + Hex32(g_scriptHash) +
				") is saved each time the game places it, at most " + std::to_string(kMaxScriptCopies) + " times, as Script-" + g_script +
				"-<n>.bin in " + ToUtf8(folder) + ".");
	}
}

#endif
