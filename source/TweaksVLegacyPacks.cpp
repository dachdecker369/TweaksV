// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the check for packs made for GTA V Legacy (see TweaksVLegacyPacks.h).
//
// A pack made for GTA V Legacy loads in Enhanced, but its models and textures keep Legacy's resource versions, and a
// vehicle made of them traps the game as it is created, with no word on screen; converted with CodeWalker, such a
// pack works (my second measurement test of 2.5.0, PLAN.md D17). RageOpenV starts the check once its file functions
// serve the mods folder; a thread of its own then reads, once, and writes nothing but the log:
//   the packs      every .rpf file in the mods folder - in the current directory, where the hooks serve it from -
//                  by the one walk RageOpenV's list is read by (TweaksVModsFolder.h), so the two list the same files
//                  the same way and stop at the same name; a folder that cannot be listed, or not to its end, is
//                  skipped, as RageOpenV skips it, but the mods folder itself, which RageOpenV has just listed, is a
//                  problem when none of its names can be handed on: the check is not done. A pack is opened by the
//                  path the walk gives it, as the game's file functions open it, so Windows refuses the check what
//                  it refuses the game: a pack is never read past the game's reach
//   the tables     a pack's header of 16 bytes - the magic RPF7, the entry count, the names' length, the encryption
//                  - then its entry table and the names behind it, read through a handle that shares reading, writing
//                  and deleting, so that the game opens the pack as if the check were not there; an archive whose
//                  table is not encrypted - OPEN, or 0, which names no encryption - is read, one that is not OPEN is
//                  not: the game's own archives are encrypted, and a pack made with OpenIV or CodeWalker is OPEN. Were
//                  a table of 0 encrypted after all, its first entry would not be the root folder, and the pack would
//                  be named as not checked. A resource's version is the top four bits of its system flags and of its
//                  graphics flags; a binary entry named .rpf is an archive nested in it, stored uncompressed at its
//                  offset in blocks of 512 bytes, and read the same way
//   the verdict    a model (yft 162, ydr or ydd 165) or a texture dictionary (ytd 13) with Legacy's version makes the
//                  pack one made for Legacy - Enhanced's are 171, 159 and 5; such a pack gets a line, and so does a
//                  pack that cannot be read, or not in full, named as not checked with the reason - also one that holds
//                  archives that are not OPEN, with their number; one problem names them all - but the packs not read only
//                  for archives that are not OPEN, their own or nested ones, which their lines alone name - and is shown
//                  in a box as ERROR_POPUP allows; the last line counts them
//   the bounds     1,048,576 entries per entry table; the table and the names inside their archive, and a nested
//                  archive inside the one that holds it, past that one's names - so each is smaller than the one
//                  that holds it; 8 levels of nesting, 4,096 archives and 64 MB of tables per pack, 1 GB of tables
//                  for the whole check - a pack past one of them is not checked in full; each kind of line 20,000 at
//                  most, then only counted
//   DIAGNOSTIC_MODE  a line per pack read - its archives, its entries, its models and textures by version - and the
//                  check's bytes and time
// With DRY_RUN the mods folder is not used, so its packs are not checked. Written for TweaksV from the RPF7 facts in
// PATTERNS.md ("RageOpenV") and the versions my second test read from the packs' tables.

#include "TweaksVLegacyPacks.h"

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVModsFolder.h"
#include "TweaksVPatch.h"
#include "TweaksVRpf.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		using namespace Rpf;	// the RPF7 facts, which RageOpenV shares

		constexpr const char* kTag = "RageOpenV";

		// ---- the names a file can reach

		constexpr size_t kMaxNameSize = 256;			// a name and its zero
		constexpr size_t kNamesReached = 0x10000 + kMaxNameSize;	// a file's name offset is 16 bits: the names it can reach

		// The four kinds of resource the check reads, each with Legacy's version (Enhanced's are 171, 159, 159 and 5).
		struct Kind
		{
			std::string_view extension;
			unsigned legacy;
		};
		constexpr std::array<Kind, 4> kKinds = { { { ".yft", 162 }, { ".ydr", 165 }, { ".ydd", 165 }, { ".ytd", 13 } } };
		constexpr size_t kVersions = 256;				// a version is two nibbles

		// ---- the bounds (the walk's - files and folders, depth, a path's bytes - are in TweaksVModsFolder.h)

		constexpr size_t kMaxNesting = 8;						// levels of archives nested in a pack
		constexpr size_t kMaxArchives = 4096;					// per pack, its own included
		constexpr uint64_t kMaxPackTableBytes = 64ull << 20;	// entry tables and names read for one pack...
		constexpr uint64_t kMaxTableBytes = 1ull << 30;			// ...and for the whole check
		constexpr size_t kMaxNamed = 10;						// packs of each kind the problem names; the rest it counts

		// ---- reading

		uint32_t Get24(const uint8_t* at)
		{
			return static_cast<uint32_t>(at[0]) | (static_cast<uint32_t>(at[1]) << 8) | (static_cast<uint32_t>(at[2]) << 16);
		}

		// A handle closed on every way out, an exception's too.
		struct FileHandle
		{
			HANDLE handle;
			~FileHandle() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
		};

		// Reads exactly size bytes at offset: ERROR_SUCCESS, else the Windows error - ERROR_HANDLE_EOF when the file
		// ends first.
		DWORD ReadAt(HANDLE file, uint64_t offset, void* buffer, size_t size)
		{
			auto* out = static_cast<uint8_t*>(buffer);
			while (size > 0)
			{
				OVERLAPPED at{};
				at.Offset = static_cast<DWORD>(offset);
				at.OffsetHigh = static_cast<DWORD>(offset >> 32);
				DWORD read = 0;
				if (!ReadFile(file, out, static_cast<DWORD>(std::min<size_t>(size, 1u << 24)), &read, &at))
				{
					const DWORD error = GetLastError();
					return error != ERROR_SUCCESS ? error : ERROR_READ_FAULT;
				}
				if (read == 0)
					return ERROR_HANDLE_EOF;
				out += read;
				offset += read;
				size -= read;
			}
			return ERROR_SUCCESS;
		}

		// The name at a file's name offset: its bytes up to their zero, inside the names read. False when there is no
		// zero there - the offset past the names, or a name longer than 255 bytes.
		bool NameAt(const std::vector<uint8_t>& names, size_t offset, std::string_view& name)
		{
			if (offset >= names.size())
				return false;
			const char* const start = reinterpret_cast<const char*>(names.data()) + offset;
			const void* const end = memchr(start, 0, std::min(names.size() - offset, kMaxNameSize));
			if (!end)
				return false;
			name = std::string_view(start, static_cast<size_t>(static_cast<const char*>(end) - start));
			return true;
		}

		// Whether name ends with extension, given in lower case, in any case.
		bool HasExtension(std::string_view name, std::string_view extension)
		{
			if (name.size() < extension.size())
				return false;
			for (size_t i = 0, at = name.size() - extension.size(); i < extension.size(); i++, at++)
			{
				const char c = name[at] >= 'A' && name[at] <= 'Z' ? static_cast<char>(name[at] - 'A' + 'a') : name[at];
				if (c != extension[i])
					return false;
			}
			return true;
		}

		std::string EncryptionText(uint32_t encryption)
		{
			return "encryption " + Hex32(encryption) + (encryption == kAesTag ? ", AES" : (encryption & kNgMask) == kNgTag ? ", NG" : "");
		}

		// ---- the check, on its one thread

		// A kind of line that comes per pack: kMaxLinesPerKind of them, then only counted (the last line has the numbers).
		struct LineKind
		{
			const char* name;
			bool diagnostic;
			uint64_t count = 0;
		};

		bool Counted(LineKind& kind)
		{
			if (++kind.count <= kMaxLinesPerKind)
				return true;
			if (kind.count == kMaxLinesPerKind + 1)
				WriteLog(kTag, std::string(kind.diagnostic ? "DIAGNOSTIC: " : "") + std::to_string(kMaxLinesPerKind) + " lines of " + kind.name +
					" written - from now on they are only counted.");
			return false;
		}

		// An archive of a pack to read - the pack's own, or one nested in it: where it lies in the file, how deeply it is
		// nested, and its name, the nested archives' names from the pack down ("vehicles.rpf/lights.rpf"; empty for the
		// pack's own).
		struct Archive
		{
			uint64_t start;
			uint64_t size;
			size_t depth;
			std::string name;
		};

		// What reading one pack found.
		struct Pack
		{
			std::array<std::array<uint64_t, kVersions>, kKinds.size()> versions{};	// its resources of the four kinds by version
			uint64_t legacy = 0;			// those with Legacy's version
			std::string firstLegacy;		// the first of them, and the nested archive that holds it
			uint64_t archives = 0;			// read: its own and the nested ones
			uint64_t nestedNotOpen = 0;		// nested archives that are not OPEN, not read
			uint64_t entries = 0;
			uint64_t tableBytes = 0;
			bool notOpen = false;			// the pack itself is not OPEN: not read
			std::string notChecked;			// why it, or a part of it, was not read - the first reason; empty if it was read in full
		};

		enum class Outcome { Read, NotRead, StopPack, StopAll };

		struct Check
		{
			bool diagnostic = false;
			uint64_t packs = 0;
			uint64_t legacyPacks = 0;
			uint64_t notCheckedPacks = 0;
			uint64_t notOpenPacks = 0;		// among them, those not OPEN
			uint64_t archives = 0;
			uint64_t tableBytes = 0;
			bool walkStopped = false;		// at the walk's limit, where RageOpenV's list stops
			bool limitReached = false;		// at kMaxTableBytes: every pack from there on is not checked
			std::vector<std::string> legacyNamed;		// the first kMaxNamed of each, for the problem
			std::vector<std::string> notCheckedNamed;
			std::vector<std::pair<std::wstring, std::string>> folderPacks;	// the .rpf files of the folder being listed: the path, the name for the lines
			LineKind legacyLines{ "packs made for GTA V Legacy", false };
			LineKind notCheckedLines{ "packs not checked", false };
			LineKind packLines{ "packs read", true };
			std::vector<uint8_t> table;		// the buffers, kept from archive to archive
			std::vector<uint8_t> names;
		};

		std::string LimitReason()
		{
			return "the check had read " + std::to_string(kMaxTableBytes >> 20) + " MB of entry tables and names before it";
		}

		// why an archive that is not OPEN is not read, for the lines that name one
		constexpr const char* kOnlyOpen = " - only archives that are not encrypted are read, and the game's own archives are encrypted";

		// A resource of the four kinds counted by its version - the top four bits of its system flags (+8) and of its
		// graphics flags (+12) - and the first with Legacy's version kept.
		void CountResource(Pack& pack, const Archive& archive, std::string_view name, const uint8_t* entry)
		{
			const unsigned version = ((ValueAt<uint32_t>(entry + kEntryDataSizeField) >> 28) << 4) | (ValueAt<uint32_t>(entry + kEntryEncryptedField) >> 28);
			for (size_t kind = 0; kind < kKinds.size(); kind++)
			{
				if (!HasExtension(name, kKinds[kind].extension))
					continue;
				pack.versions[kind][version]++;
				if (version == kKinds[kind].legacy && pack.legacy++ == 0)
					pack.firstLegacy = Printable(name) + (archive.name.empty() ? std::string() : " in its nested archive " + archive.name);
				return;
			}
		}

		// Reads one archive of a pack - its header, entry table and names, within the bounds - counts its resources of
		// the four kinds and queues the archives nested in it. A damaged archive gives the pack its reason and is left,
		// the pack's other archives still read; a pack past a bound is left (StopPack), and every pack from there on
		// when the whole check is (StopAll).
		Outcome ReadArchive(Check& check, HANDLE file, const Archive& archive, Pack& pack, std::vector<Archive>& archives)
		{
			const std::string it = archive.name.empty() ? "it" : "its nested archive " + archive.name;
			const auto notRead = [&pack](std::string why, Outcome outcome = Outcome::NotRead)
			{
				if (pack.notChecked.empty())
					pack.notChecked = std::move(why);
				return outcome;
			};
			if (archive.size < kHeaderSize)
				return notRead(it + " is shorter than an archive's header (" + std::to_string(archive.size) + " bytes)");
			uint8_t header[kHeaderSize];
			if (const DWORD error = ReadAt(file, archive.start, header, sizeof(header)); error != ERROR_SUCCESS)
				return notRead(it + " could not be read (" + DescribeError(error) + ")");
			const uint32_t count = ValueAt<uint32_t>(header + kHeaderCountField);
			const uint32_t namesSize = ValueAt<uint32_t>(header + kHeaderNamesField);
			const uint32_t encryption = ValueAt<uint32_t>(header + kHeaderEncryptionField);
			if (ValueAt<uint32_t>(header) != kMagic)
				return notRead(it + " is not an RPF7 archive (its first bytes are " + HexBytes(header, 4) + ")");
			if (encryption != kOpenTag && encryption != 0)	// 0 names no encryption: its table is read as it is
			{
				if (!archive.name.empty())
				{
					pack.nestedNotOpen++;
					return Outcome::NotRead;
				}
				pack.notOpen = true;
				return notRead("it is not OPEN (" + EncryptionText(encryption) + ")" + kOnlyOpen);
			}
			if (count == 0)
				return notRead(it + " has no entries");
			if (count > static_cast<uint32_t>(kMaxEntries))
				return notRead(it + " has " + std::to_string(count) + " entries, more than " + std::to_string(kMaxEntries));
			const uint64_t tableSize = static_cast<uint64_t>(count) * kEntrySize;
			const uint64_t tableEnd = kHeaderSize + tableSize + namesSize;
			if (tableEnd > archive.size)
				return notRead(it + " has an entry table and names that run past its end (" + std::to_string(tableEnd) + " of " +
					std::to_string(archive.size) + " bytes)");
			const size_t namesRead = static_cast<size_t>(std::min<uint64_t>(namesSize, kNamesReached));
			const uint64_t bytes = tableSize + namesRead;
			if (pack.tableBytes + bytes > kMaxPackTableBytes)
				return notRead("its entry tables and names pass " + std::to_string(kMaxPackTableBytes >> 20) + " MB", Outcome::StopPack);
			if (check.tableBytes + bytes > kMaxTableBytes)
			{
				check.limitReached = true;
				return notRead(LimitReason(), Outcome::StopAll);
			}
			pack.tableBytes += bytes;
			check.tableBytes += bytes;
			check.table.resize(static_cast<size_t>(tableSize));
			check.names.resize(namesRead);
			DWORD error = ReadAt(file, archive.start + kHeaderSize, check.table.data(), check.table.size());
			if (error == ERROR_SUCCESS && namesRead > 0)
				error = ReadAt(file, archive.start + kHeaderSize + tableSize, check.names.data(), namesRead);
			if (error != ERROR_SUCCESS)
				return notRead(it + " could not be read (" + DescribeError(error) + ")");
			if (ValueAt<uint32_t>(check.table.data() + 4) != kFolderType)
				return notRead(it + " has no folder as its first entry");
			pack.archives++;
			check.archives++;
			pack.entries += count;
			for (uint32_t i = 1; i < count; i++)	// the first entry is the root folder
			{
				const uint8_t* const entry = check.table.data() + static_cast<size_t>(i) * kEntrySize;
				const uint32_t type = ValueAt<uint32_t>(entry + kEntryTypeField);
				if (type == kFolderType)
					continue;
				std::string_view name;
				if (!NameAt(check.names, ValueAt<uint16_t>(entry + kEntryNameField), name))
					return notRead(it + " has a name outside its names (entry " + std::to_string(i) + ")");
				if ((type & kResourceBit) != 0)
				{
					CountResource(pack, archive, name, entry);
					continue;
				}
				if (!HasExtension(name, ".rpf"))
					continue;
				// an archive nested in this one: its bytes not encrypted in it, stored (no compressed size at +2), at its
				// block (+5) and of its size (+8) inside this archive, past this one's names
				const std::string nested = (archive.name.empty() ? std::string() : archive.name + "/") + Printable(name);
				const uint64_t start = static_cast<uint64_t>(Get24(entry + kEntryBlockField)) * kBlockSize;
				const uint64_t size = ValueAt<uint32_t>(entry + kEntryDataSizeField);
				if (ValueAt<uint32_t>(entry + kEntryEncryptedField) != 0)
					pack.nestedNotOpen++;
				else if (Get24(entry + kEntryStoredField) != 0)
					notRead("its nested archive " + nested + " is compressed");
				else if (start < tableEnd || start > archive.size || size > archive.size - start)
					notRead("its nested archive " + nested + " lies outside the archive that holds it");
				else if (archive.depth >= kMaxNesting)
					notRead("its nested archive " + nested + " is nested deeper than " + std::to_string(kMaxNesting) + " levels");
				else if (archives.size() >= kMaxArchives)
					return notRead("it holds more than " + std::to_string(kMaxArchives) + " archives", Outcome::StopPack);
				else
					archives.push_back({ archive.start + start, size, archive.depth + 1, nested });
			}
			return Outcome::Read;
		}

		// Reads one pack: its own archive, then every archive nested in it, in the order they are found.
		void ReadPack(Check& check, const std::wstring& path, Pack& pack)
		{
			const FileHandle file{ CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) };
			if (file.handle == INVALID_HANDLE_VALUE)
			{
				const DWORD error = GetLastError();
				pack.notChecked = "it could not be opened (" + DescribeError(error) + ")";
				return;
			}
			LARGE_INTEGER size{};
			if (!GetFileSizeEx(file.handle, &size))
			{
				const DWORD error = GetLastError();
				pack.notChecked = "its size could not be read (" + DescribeError(error) + ")";
				return;
			}
			std::vector<Archive> archives{ { 0, static_cast<uint64_t>(size.QuadPart), 0, std::string() } };
			for (size_t i = 0; i < archives.size(); i++)
			{
				const Archive archive = archives[i];	// a copy: reading it may queue more
				const Outcome outcome = ReadArchive(check, file.handle, archive, pack, archives);
				if (outcome == Outcome::StopPack || outcome == Outcome::StopAll)
					break;
			}
		}

		// "2 yft 162, 1 ytd 13": the pack's resources of the four kinds by version - with Legacy's version alone, or all.
		std::string Versions(const Pack& pack, bool legacyOnly)
		{
			std::string text;
			for (size_t kind = 0; kind < kKinds.size(); kind++)
				for (size_t version = 0; version < kVersions; version++)
				{
					const uint64_t count = pack.versions[kind][version];
					if (count == 0 || (legacyOnly && version != kKinds[kind].legacy))
						continue;
					text += (text.empty() ? "" : ", ") + std::to_string(count) + " " + std::string(kKinds[kind].extension.substr(1)) + " " +
						std::to_string(version);
				}
			return text;
		}

		// The pack's lines, and what the problem and the last line need of it.
		void Conclude(Check& check, const std::string& name, const Pack& pack)
		{
			check.packs++;
			if (check.diagnostic && pack.archives > 0 && Counted(check.packLines))
			{
				const std::string versions = Versions(pack, false);
				WriteLog(kTag, "DIAGNOSTIC: the pack " + name + ": " + std::to_string(pack.archives) + (pack.archives == 1 ? " archive" : " archives (" +
					std::to_string(pack.archives - 1) + " nested)") + (pack.nestedNotOpen ? ", " + std::to_string(pack.nestedNotOpen) +
					" nested not OPEN" : std::string()) + ", " + std::to_string(pack.entries) + (pack.entries == 1 ? " entry; " : " entries; ") +
					(versions.empty() ? "no yft, ydr, ydd or ytd" : versions) + ".");
			}
			if (pack.legacy > 0)
			{
				check.legacyPacks++;
				if (check.legacyNamed.size() < kMaxNamed)
					check.legacyNamed.push_back(name);
				if (Counted(check.legacyLines))
					WriteLog(kTag, "The pack " + name + " was made for GTA V Legacy: " + Versions(pack, true) + " - the first " + pack.firstLegacy + ".");
			}
			// a pack not read for any reason but archives that are not OPEN - its own, or ones nested in it - is named in the problem
			// too; one with nothing but those in its line alone, since the game's own archives are encrypted
			const bool damaged = !pack.notChecked.empty() && !pack.notOpen;
			std::string why = pack.notChecked;
			if (pack.nestedNotOpen > 0)
				why += (why.empty() ? std::string() : "; ") + std::to_string(pack.nestedNotOpen) + (pack.nestedNotOpen == 1 ? " archive nested in it is" :
					" archives nested in it are") + " not OPEN" + (why.empty() ? kOnlyOpen : "");
			if (why.empty())
				return;
			check.notCheckedPacks++;
			if (!damaged)
				check.notOpenPacks++;
			else if (check.notCheckedNamed.size() < kMaxNamed)
				check.notCheckedNamed.push_back(name);
			if (Counted(check.notCheckedLines))
				WriteLog(kTag, "The pack " + name + " was not checked for GTA V Legacy versions" + (pack.archives > 0 ? " in full" : "") + ": " + why + ".");
		}

		void CheckPack(Check& check, const std::wstring& path, const std::string& name)
		{
			Pack pack;
			if (check.limitReached)
				pack.notChecked = LimitReason();
			else
				ReadPack(check, path, pack);
			Conclude(check, name, pack);
		}

		// A name of the walk: a file named .rpf, in any case, is a pack, read once its folder's find handle is closed. Its
		// name for the lines is its path from the current directory, as the walk gives it.
		void PackEntry(void* context, const ModsEntry& entry)
		{
			if (entry.kind != ModsEntryKind::File || !HasExtension(entry.below, ".rpf"))
				return;
			static_cast<Check*>(context)->folderPacks.emplace_back(entry.path, ToUtf8(entry.path));
		}

		// A folder's packs in the order Windows lists them, every find handle closed.
		void CheckFolderPacks(void* context)
		{
			Check& check = *static_cast<Check*>(context);
			for (const auto& [path, name] : check.folderPacks)
				CheckPack(check, path, name);
			check.folderPacks.clear();
		}

		// "a, b, and 7 more"
		std::string Named(const std::vector<std::string>& named, uint64_t total)
		{
			std::string text;
			for (const std::string& name : named)
				text += (text.empty() ? "" : ", ") + name;
			if (total > named.size())
				text += ", and " + std::to_string(total - named.size()) + " more";
			return text;
		}

		// The one problem - the packs made for Legacy, the packs not checked but those not OPEN - and the last line.
		void Report(const Check& check, ULONGLONG started)
		{
			const uint64_t notChecked = check.notCheckedPacks - check.notOpenPacks;
			std::string problem;
			if (check.legacyPacks == 1)
				problem = "A pack in the mods folder was made for GTA V Legacy, and a Legacy vehicle stops the game as it is created - convert it for "
					"Enhanced with CodeWalker, or take it out: " + Named(check.legacyNamed, 1) + ".";
			else if (check.legacyPacks > 1)
				problem = std::to_string(check.legacyPacks) + " packs in the mods folder were made for GTA V Legacy, and a Legacy vehicle stops the game "
					"as it is created - convert them for Enhanced with CodeWalker, or take them out: " + Named(check.legacyNamed, check.legacyPacks) + ".";
			if (notChecked > 0)
			{
				if (!problem.empty())
					problem += " Not checked for Legacy versions";
				else
					problem = (notChecked == 1 ? std::string("A pack in the mods folder") : std::to_string(notChecked) + " packs in the mods folder") +
						" could not be checked for GTA V Legacy versions";
				problem += " (TweaksV.log says why): " + Named(check.notCheckedNamed, notChecked) + ".";
			}
			if (!problem.empty())
				ReportFailure(kTag, problem);
			if (check.diagnostic)
			{
				char numbers[96];
				snprintf(numbers, sizeof(numbers), "%.1f MB of entry tables and names from %llu archives in %.2f s",
					static_cast<double>(check.tableBytes) / 1048576.0, static_cast<unsigned long long>(check.archives),
					static_cast<double>(GetTickCount64() - started) / 1000.0);
				WriteLog(kTag, std::string("DIAGNOSTIC: the check read ") + numbers + ".");
			}
			std::string line = "Packs checked for GTA V Legacy versions: ";
			if (check.packs == 0)
				line += "none in the mods folder";
			else
			{
				line += std::to_string(check.packs) + " in the mods folder, " + (check.legacyPacks ? std::to_string(check.legacyPacks) : "none") +
					" made for Legacy";
				if (check.notCheckedPacks > 0)
					line += ", " + std::to_string(check.notCheckedPacks) + " not checked" + (check.notOpenPacks ? " (" +
						std::to_string(check.notOpenPacks) + " of them not OPEN)" : std::string());
			}
			if (check.walkStopped)
				line += " - the walk stopped at " + std::to_string(ModsEntryLimit()) + " files and folders, where RageOpenV's list stops";
			WriteLog(kTag, line + ".");
		}

		std::wstring g_root;		// what the check walks: kModsRoot, unless a test build asks otherwise - set before the thread starts, never changed
		std::atomic<bool> g_started{ false };

		DWORD WINAPI CheckThread(LPVOID module)
		{
			try	// nothing may leave a thread of this module
			{
				try
				{
					Check check;
					check.diagnostic = IsDiagnosticMode();
					const ULONGLONG started = GetTickCount64();
					const ModsWalk walk = WalkModsFolder(g_root.c_str(), { &check, &PackEntry, &CheckFolderPacks });
					check.walkStopped = walk.stopped;
					if (walk.rootError != ERROR_SUCCESS)	// RageOpenV has just listed it, so the check cannot say it found nothing
						ReportFailure(kTag, "The check for packs made for GTA V Legacy is not done this session: the mods folder could not be read (" +
							DescribeError(walk.rootError) + ").");
					else
						Report(check, started);
				}
				catch (...)
				{
					ReportFailure(kTag, "The check for packs made for GTA V Legacy is not done this session: an unexpected exception ended it.");
				}
			}
			catch (...)
			{
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}
	}

	void StartLegacyPacksCheck()
	{
		if (g_started.exchange(true))
			return;
		const auto fail = [](const std::string& why)
		{
			ReportFailure(kTag, "The check for packs made for GTA V Legacy is not done this session: " + why);
		};
		try
		{
			g_root = kModsRoot;
#ifdef TWEAKSV_TEST_BUILD
			// test builds only: TWEAKSV_TEST_LEGACY_FOLDER=<folder> in the environment has the check read that folder in place of
			// the mods folder, so that a suite reaches a folder the check cannot list - RageOpenV's own reading of the mods folder
			// stops at one first
			wchar_t other[MAX_PATH]{};
			const DWORD otherLength = GetEnvironmentVariableW(L"TWEAKSV_TEST_LEGACY_FOLDER", other, MAX_PATH);
			if (otherLength > 0 && otherLength < MAX_PATH)
			{
				g_root.assign(other, otherLength);
				WriteLog(kTag, "TEST BUILD: the check for packs made for GTA V Legacy reads " + ToUtf8(g_root) + " in place of the mods folder "
					"(TWEAKSV_TEST_LEGACY_FOLDER).");
			}
#endif
			if (!StartModuleThread(CheckThread, "Legacy packs check"))
				fail("its thread could not be started (details in TweaksV.log).");
		}
		catch (...)
		{
			try
			{
				fail("an unexpected exception kept it from starting.");
			}
			catch (...)
			{
			}
		}
	}
}
