// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the weapon components limit, the WeaponLimits part's second half (see TweaksVWeaponComponents.h).
//
// The game keeps every weapon component definition of all weaponcomponents.meta files - each scope, clip,
// suppressor, flashlight, grip and tint a weapon can take - together in one sorted list: a static array of 470
// pointers with its count, an int32, right after it, so the list's size is the distance from the array to its
// count and no instruction carries it. After each weaponcomponents.meta it loads, the game rebuilds the list -
// the count set to 0, every file's definitions copied in without a bound, then sorted by their hash - and 15
// binary searches, bounded by the count, read it. Every definition also takes a slot of a pool, 470 unless the
// game's configuration names another size, and a full pool ends the game in a trap while it loads; a list past
// its 470 would take the 471st definition over its count and then write through the garbage.
//   WEAPON_COMPONENTS_SIZE  the room for definitions, from the game's own 470 up to 16384. The list is moved on
//                    the relocation core to one of 16854 entries - its 17 leas re-pointed, 6 of them in the game's
//                    second, protected code - and its count left where it is (the core's follower: the list's size
//                    is the distance to it); the call that reads the pool's size from the game's configuration -
//                    W-E-C4, which TweaksVPools finds once a session for this part and StaticBounds - is
//                    redirected so that the pool gets this many slots and 470 more, also when the configuration
//                    names more, so that the pool's trap always comes first. The 470 definitions one file may hold
//                    stay.
//   DIAGNOSTIC_MODE  every site logged, a line per weaponcomponents.meta the guard sees, and the measurement: 10
//                    seconds in and then every minute while they change, the definitions in the list, the files
//                    loaded and those refused; at every reading the 17 loads are checked to lead to the moved list
//                    still, and a load the game's protected code put back is reported, once
// The guard (D12): the loader of one weaponcomponents.meta (W-E-C5) is hooked at its head, and a file that comes once
// the definitions already loaded - the sum of the files' blobs, read bounded - have reached the room is refused before
// the game makes any of its definitions, as is a file of a name refused before; the first refusal is a problem, in a
// box, the others are logged. The pool gets the room and 470 more, the most one file brings, and the list room for
// 16384 + 470, so that the last file let in always fits. A read of the blobs that does not add up lets the file
// through and switches the guard off for the session, reported. In a test build TWEAKSV_TEST_GUARD_ROOM=1 gives the
// guard the game's own room, 470 (GuardRoom). The hook is MinHook's, its trampoline writable and executable (T-HARDEN).
// All of it in one write, all or nothing; a list another mod moved, a pool size call another mod hooked, a load
// more or fewer than 17 are refused with the reason. Written for TweaksV from its own findings in the game's code
// (PATTERNS.md, "WeaponLimits"); that the limit exists, and that the pool's size must grow with it, is what
// alexguirre's Weapon Limits Adjuster for GTA V Legacy says on its page - none of its code, nor FiveM's, from
// which its components half is adapted, was read or used (CREDITS.md).

#include "TweaksVWeaponComponents.h"

#include <windows.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVPools.h"
#include "TweaksVRelocation.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "WeaponLimits";

		// ---- the list, its count and the pool's size (PATTERNS.md, "WeaponLimits")

		constexpr uint32_t kEntrySize = 8;					// an entry of the list: a definition's pointer
		constexpr uint32_t kPerFile = 470;					// the most definitions one file brings: its blob's array of pointers
		constexpr uint32_t kMaxEntries = 16384 + kPerFile;	// the moved list always has this many entries: the largest room and the last file let in
		static_assert(kMaxEntries == static_cast<uint32_t>(kWeaponComponentsSize.maxValue) + kPerFile,
			"TweaksV: the moved list must hold WEAPON_COMPONENTS_SIZE's maximum and one file more");
		constexpr size_t kListLoads = 17;					// the "lea r64, [rip+disp32]" that load the list - every one of them, on 1.0.1158.16
		constexpr char kLeaPrefixes[] = "\x48\x4C";			// the REX bytes the game's loads of the list start with

		constexpr const GamePattern& kRebuildPattern = Pattern("W-E-C1");	// the rebuild: the count's reset, the files' count, its lea of the list
		constexpr const GamePattern& kSortPattern = Pattern("W-E-C2");		// the sort: the count, its lea of the list, the entry size
		constexpr const GamePattern& kUpdatePattern = Pattern("W-E-C3");	// the rebuild's update of the count, before its unbounded store
		// the pool's size - the configuration, the pool's name and default, the call - is W-E-C4, TweaksVPools' (kPoolSizeCallPattern)

		// A rip-relative operand in a match: where its disp32 lies, and where its instruction ends - the address it
		// leads to is counted from there, past an immediate that may follow the disp32.
		struct RipOperand
		{
			size_t field;
			size_t end;
		};
		constexpr RipOperand kCountReset = { 2, 10 };		// W-E-C1: "mov dword [rip+disp32], 0" - the count, set to 0
		constexpr RipOperand kFilesRead = { 14, 18 };		// W-E-C1: "movzx r8d, word [rip+disp32]" - the files' count
		constexpr RipOperand kRebuildLoad = { 34, 38 };		// W-E-C1: "lea r10, [rip+disp32]" - the list
		constexpr RipOperand kSortCount = { 3, 7 };			// W-E-C2: "movsxd rdx, dword [rip+disp32]" - the count
		constexpr RipOperand kSortLoad = { 14, 18 };		// W-E-C2: "lea rcx, [rip+disp32]" - the list
		constexpr size_t kEntrySizeAt = 27;					// W-E-C2: the imm32 of "mov r8d, 8" - qsort's entry size
		constexpr RipOperand kUpdateRead = { 3, 7 };		// W-E-C3: "movsxd rdx, dword [rip+disp32]" - the count
		constexpr RipOperand kUpdateWrite = { 12, 16 };		// W-E-C3: "mov [rip+disp32], ecx" - the count

		constexpr const GamePattern& kLoaderPattern = Pattern("W-E-C5");	// the loader of one weaponcomponents.meta: the blob, the blob array
		constexpr size_t kLoaderHead = 0x13;				// W-E-C5's match lies this far into the loader, whose head the guard hooks
		constexpr uint32_t kBlobSize = 0xED8;				// W-E-C5's +0x07 and +0x15: a file's blob, allocated and zeroed
		constexpr RipOperand kBlobsCount = { 0x26, 0x2A };	// W-E-C5: "movzx ebx, word [rip+disp32]" - the blobs' count, a uint16
		constexpr RipOperand kBlobsCapacity = { 0x2D, 0x31 };	// W-E-C5: "cmp bx, word [rip+disp32]" - the blob array's capacity, the count + 2
		constexpr RipOperand kBlobsArray = { 0x3E, 0x42 };	// W-E-C5: "mov rsi, [rip+disp32]" - the blob array's pointer, the count - 8
		constexpr size_t kBlobCountAt = 0xEC0;				// a blob's definitions' count, an int32, after its kPerFile pointers from +0x10
		static_assert((kBlobCountAt - 0x10) / kEntrySize == kPerFile && kBlobCountAt + sizeof(int32_t) <= kBlobSize,
			"TweaksV: a blob holds kPerFile definitions and their count");
		constexpr size_t kEntryNameSize = 0x90;				// a data file entry: the file's name, NUL-terminated, before its type
		constexpr size_t kEntryTypeAt = 0x90;				// the entry's data file type, an int32
		constexpr int32_t kComponentsFileType = 0x4F;		// the data file type of weaponcomponents.meta
		constexpr size_t kCallableAt = 0x38;				// the loader's callback, passed by value: its callable's pointer
		constexpr size_t kDestroyAt = 0x20;					// the callable's vtable: its destroy
		constexpr size_t kMaxNames = 1024;					// the refused files' names the guard keeps, as 64-bit hashes

		constexpr char kResetOpcode[] = "\xC7\x05";			// mov dword [rip+disp32], imm32
		constexpr char kRebuildLea[] = "\x4C\x8D\x15";		// lea r10, [rip+disp32]
		constexpr char kSortLea[] = "\x48\x8D\x0D";			// lea rcx, [rip+disp32]

		// The bytes a pattern holds from index at, none of them a wildcard - checked when TweaksV is compiled, so that
		// each offset above reads the instruction its comment names.
		consteval bool PatternHolds(const GamePattern& pattern, size_t at, std::string_view bytes)
		{
			for (size_t i = 0; i < bytes.size(); i++)
				if (PatternByte(pattern, at + i) != static_cast<uint8_t>(bytes[i]))
					return false;
			return true;
		}
		// The imm32 a pattern holds at index at, or -1 when a byte of it is a wildcard.
		consteval int64_t PatternImm32(const GamePattern& pattern, size_t at)
		{
			int64_t value = 0;
			for (size_t i = 0; i < 4; i++)
			{
				const int byte = PatternByte(pattern, at + i);
				if (byte < 0)
					return -1;
				value |= static_cast<int64_t>(byte) << (8 * i);
			}
			return value;
		}
		static_assert(PatternHolds(kRebuildPattern, kCountReset.field - 2, kResetOpcode) && PatternImm32(kRebuildPattern, kCountReset.field + 4) == 0 &&
			kCountReset.end == kCountReset.field + 8, "TweaksV: W-E-C1 holds the count's reset, mov dword [rip+disp32], 0, at +0");
		static_assert(PatternHolds(kRebuildPattern, kFilesRead.field - 4, "\x44\x0F\xB7\x05") && kFilesRead.end == kFilesRead.field + 4,
			"TweaksV: W-E-C1 holds the files' count's movzx r8d, word [rip+disp32] at +10");
		static_assert(PatternHolds(kRebuildPattern, kRebuildLoad.field - 3, kRebuildLea) && kRebuildLoad.end == kRebuildLoad.field + 4,
			"TweaksV: W-E-C1 holds the list's lea r10, [rip+disp32] at +31");
		static_assert(PatternHolds(kSortPattern, kSortCount.field - 3, "\x48\x63\x15") && kSortCount.end == kSortCount.field + 4,
			"TweaksV: W-E-C2 holds the count's movsxd rdx, dword [rip+disp32] at +0");
		static_assert(PatternHolds(kSortPattern, kSortLoad.field - 3, kSortLea) && kSortLoad.end == kSortLoad.field + 4,
			"TweaksV: W-E-C2 holds the list's lea rcx, [rip+disp32] at +11");
		static_assert(PatternHolds(kSortPattern, kEntrySizeAt - 2, "\x41\xB8") && PatternImm32(kSortPattern, kEntrySizeAt) == kEntrySize,
			"TweaksV: W-E-C2 holds qsort's entry size, mov r8d, 8, at +25");
		static_assert(PatternHolds(kUpdatePattern, kUpdateRead.field - 3, "\x48\x63\x15") && kUpdateRead.end == kUpdateRead.field + 4 &&
			PatternHolds(kUpdatePattern, kUpdateWrite.field - 2, "\x89\x0D") && kUpdateWrite.end == kUpdateWrite.field + 4,
			"TweaksV: W-E-C3 holds the count's movsxd rdx, dword [rip+disp32] at +0 and its mov [rip+disp32], ecx at +10");
		static_assert(PatternHolds(kLoaderPattern, 0x06, "\xB9") && PatternImm32(kLoaderPattern, 0x07) == kBlobSize &&
			PatternHolds(kLoaderPattern, 0x13, "\x41\xB8") && PatternImm32(kLoaderPattern, 0x15) == kBlobSize,
			"TweaksV: W-E-C5 holds the blob's size, 0xED8, allocated at +0x07 and zeroed at +0x15");
		static_assert(PatternHolds(kLoaderPattern, kBlobsCount.field - 3, "\x0F\xB7\x1D") && kBlobsCount.end == kBlobsCount.field + 4 &&
			PatternHolds(kLoaderPattern, kBlobsCapacity.field - 3, "\x66\x3B\x1D") && kBlobsCapacity.end == kBlobsCapacity.field + 4 &&
			PatternHolds(kLoaderPattern, kBlobsArray.field - 3, "\x48\x8B\x35") && kBlobsArray.end == kBlobsArray.field + 4,
			"TweaksV: W-E-C5 holds the blobs' count's movzx at +0x23, the capacity's cmp at +0x2A and the array's mov at +0x3B");

		// The anchors on the core, in this order: the rebuild - its lea of the list, and its reset of the count, the
		// variable that follows the list, whose distance from it is the list's size; the sort's lea of the list; the
		// count's update, which the part reads itself. The pool's size call is TweaksVPools', which StaticBounds asks too.
		enum : size_t { kRebuild, kSort, kUpdate };
		constexpr Anchor kAnchors[] = {
			ReferenceAndLimit(kRebuildPattern, Rel32(kRebuildLoad.field, kRebuildLea, 3),
				Follower(kCountReset.field - 2, kResetOpcode, 2, kCountReset.field, kCountReset.end)),
			Reference(kSortPattern, Rel32(kSortLoad.field, kSortLea, 3)),
			Site(kUpdatePattern),
		};

		constexpr RelocationSpec kSpec = { kTag, "weapon component list", "weapon component list size", "The weapon components limit is not raised",
			kEntrySize, kMaxEntries, &kWeaponComponentsSize, kAnchors, kListLoads, kLeaPrefixes, 2, 0 };

		// Where a rip-relative operand of a match leads.
		const uint8_t* Target(const uint8_t* match, const RipOperand& operand)
		{
			int32_t disp = 0;
			memcpy(&disp, match + operand.field, sizeof(disp));
			return match + operand.end + disp;
		}

		uint32_t Imm32At(const uint8_t* address)
		{
			uint32_t value = 0;
			memcpy(&value, address, sizeof(value));
			return value;
		}

		// True if the size bytes at p lie inside the game and can be read.
		bool InsideGame(const void* p, size_t size)
		{
			return IsInsideGameImage(p) && IsInsideGameImage(static_cast<const uint8_t*>(p) + size - 1) && IsReadable(p, size);
		}

		std::string Hex32(uint32_t value)
		{
			char text[16];
			snprintf(text, sizeof(text), "0x%08X", value);
			return text;
		}

		// "1 weaponcomponents.meta file", "23 weaponcomponents.meta files"
		std::string Counted(uint64_t n, const char* thing)
		{
			return std::to_string(n) + " " + thing + (n == 1 ? "" : "s");
		}

		// ---- the pool's size: the game's call that reads it from its configuration, redirected - the call alone, never the
		// lookup, which serves every pool of the game

		using PoolSizeLookup = int32_t (*)(const void* configuration, uint32_t hash, int32_t defaultSize);
		PoolSizeLookup g_lookup = nullptr;	// the game's lookup behind the call
		int32_t g_poolSize = 0;				// the slots the pool gets: the list's new size
		volatile LONG g_poolSaid = 0;		// the pool's line is written once

		// The game calls this when its weapon system makes the weapon components' pool - again when it makes the pool
		// anew - on a thread of its own: nothing may throw out of here, and the game's lookup is always called, so that
		// its configuration is read as it would be.
		int32_t PoolSize(const void* configuration, uint32_t hash, int32_t defaultSize)
		{
			const int32_t given = g_lookup(configuration, hash, defaultSize);
			if (InterlockedExchange(&g_poolSaid, 1) == 0)
			{
				try
				{
					WriteLog(kTag, "The pool size the game's configuration gives for the weapon components, " + std::to_string(given) +
						", is replaced by " + std::to_string(g_poolSize) + ".");
				}
				catch (...)
				{
				}
			}
			return g_poolSize;
		}

		// ---- the guard (D12): the loader of one weaponcomponents.meta hooked at its head

		using Loader = uint64_t (*)(void* entry, void* callback);
		Loader g_loader = nullptr;						// the game's loader, through MinHook's trampoline
		const uint8_t* g_blobs = nullptr;				// the blob array: its pointer, then the count and the capacity, uint16 each
		uint32_t g_room = 0;							// the definitions loaded at which a file is refused
		std::atomic<bool> g_guardOn{ false };
		std::atomic<uint32_t> g_refused{ 0 };			// the files refused this session
		char g_firstRefused[kEntryNameSize + 1] = {};	// the first one's name, written before g_refused counts it
		// the loader's thread alone: the hashes of the names refused, and the guard's lines
		uint64_t g_names[kMaxNames] = {};
		size_t g_nameCount = 0;
		bool g_namesFull = false;
		uint64_t g_lines = 0;

		// The definitions already loaded: the sum of the blobs' counts, each read bounded - the count at most the capacity, the
		// pointers and the counts copied safely, each count 0..470. False, why saying what, when a read does not add up.
		bool LoadedDefinitions(uint64_t& sum, std::string& why)
		{
			uint8_t* array = nullptr;
			uint16_t numbers[2] = {};	// the count and the capacity
			if (!CopyMemorySafely(&array, g_blobs, sizeof(array)) || !CopyMemorySafely(numbers, g_blobs + sizeof(array), sizeof(numbers)))
			{
				why = "the blob array at " + DescribeAddress(g_blobs) + " cannot be read";
				return false;
			}
			if (numbers[0] > numbers[1])
			{
				why = "the blob array holds " + std::to_string(numbers[0]) + " blobs, past its capacity of " + std::to_string(numbers[1]);
				return false;
			}
			sum = 0;
			uint8_t* pointers[64];
			for (uint32_t i = 0; i < numbers[0]; i += 64)
			{
				const uint32_t n = numbers[0] - i < 64 ? numbers[0] - i : 64;
				if (!CopyMemorySafely(pointers, array + static_cast<size_t>(i) * sizeof(void*), n * sizeof(void*)))
				{
					why = "the blob array's pointers at " + DescribeAddress(array) + " cannot be read";
					return false;
				}
				for (uint32_t j = 0; j < n; j++)
				{
					int32_t definitions = 0;
					if (!CopyMemorySafely(&definitions, pointers[j] + kBlobCountAt, sizeof(definitions)))
					{
						why = "blob " + std::to_string(i + j) + " at " + DescribeAddress(pointers[j]) + " cannot be read";
						return false;
					}
					if (definitions < 0 || static_cast<uint32_t>(definitions) > kPerFile)
					{
						why = "blob " + std::to_string(i + j) + " holds " + std::to_string(definitions) + " definitions, outside 0.." + std::to_string(kPerFile);
						return false;
					}
					sum += static_cast<uint32_t>(definitions);
				}
			}
			return true;
		}

		// The file's name from its entry, read bounded: the type at +0x90 must be weaponcomponents.meta's, the name the bytes up
		// to the first NUL within 0x90, any byte outside printable ASCII shown as '?'; hash, the name's 64-bit FNV-1a as read.
		bool EntryName(const void* entry, char (&name)[kEntryNameSize + 1], uint64_t& hash)
		{
			uint8_t bytes[kEntryNameSize + sizeof(int32_t)];
			if (!entry || !CopyMemorySafely(bytes, entry, sizeof(bytes)))
				return false;
			int32_t type = 0;
			memcpy(&type, bytes + kEntryTypeAt, sizeof(type));
			size_t length = 0;
			while (length < kEntryNameSize && bytes[length])
				length++;
			if (type != kComponentsFileType || length == 0)
				return false;
			hash = 14695981039346656037ull;
			for (size_t i = 0; i < length; i++)
			{
				hash = (hash ^ bytes[i]) * 1099511628211ull;
				name[i] = bytes[i] >= 0x20 && bytes[i] < 0x7F ? static_cast<char>(bytes[i]) : '?';
			}
			name[length] = '\0';
			return true;
		}

		bool Refused(uint64_t hash)
		{
			for (size_t i = 0; i < g_nameCount; i++)
				if (g_names[i] == hash)
					return true;
			return false;
		}

		// A refused file's name kept for the session, 1024 at most - past them, refusals by the count alone, said once.
		void Remember(uint64_t hash)
		{
			if (g_nameCount < kMaxNames)
				g_names[g_nameCount++] = hash;
			else if (!g_namesFull)
			{
				g_namesFull = true;
				WriteLog(kTag, "The weapon components' guard keeps " + std::to_string(kMaxNames) + " refused names at most: from now on a file is refused "
					"by the definitions loaded alone.");
			}
		}

		// The refused file's callback, which the loader would have destroyed at its end: its callable destroyed through its
		// vtable, dl true when the callable lies outside the argument, then its pointer cleared - the vtable and the destroy
		// inside the game first.
		void DestroyCallback(void* callback)
		{
			uint8_t* const argument = static_cast<uint8_t*>(callback);
			void* callable = nullptr;
			const uint8_t* vtable = nullptr;
			const void* destroy = nullptr;
			if (!argument || !CopyMemorySafely(&callable, argument + kCallableAt, sizeof(callable)) || !callable ||
				!CopyMemorySafely(&vtable, callable, sizeof(vtable)) || !IsInsideGameImage(vtable) ||
				!CopyMemorySafely(&destroy, vtable + kDestroyAt, sizeof(destroy)) || !IsInsideGameImage(destroy))
				return;
			reinterpret_cast<void (*)(void*, bool)>(const_cast<void*>(destroy))(callable, callable != argument);
			void* const none = nullptr;
			memcpy(argument + kCallableAt, &none, sizeof(none));
		}

		// Whether the guard refuses the file of entry: the definitions loaded reached the room, or its name was refused before.
		// A read that does not add up lets the file through and switches the guard off, reported.
		bool Refuse(const void* entry)
		{
			char name[kEntryNameSize + 1] = {};
			uint64_t hash = 0;
			const bool named = EntryName(entry, name, hash);
			const char* const file = named ? name : "a weaponcomponents.meta whose name cannot be read";	// no string made per file
			uint64_t sum = 0;
			std::string why;
			if (!LoadedDefinitions(sum, why))
			{
				g_guardOn = false;
				ReportFailure(kTag, "The weapon components' guard is off this session: " + why + " - " + file + " and the files after it go to the game "
					"unchecked, and past " + std::to_string(kPerFile) + " definitions above the room the game crashes while it loads.");
				return false;
			}
			const bool before = named && Refused(hash);
			const bool refuse = before || sum >= g_room;
			const bool diagnostic = IsDiagnosticMode();
			if ((refuse || diagnostic) && g_lines <= kMaxLinesPerKind)
			{
				if (g_lines++ == kMaxLinesPerKind)
					WriteLog(kTag, std::to_string(kMaxLinesPerKind) + " lines of weapon components files written - from now on they are only counted.");
				else
					WriteLog(kTag, std::string(refuse ? "Weapon components file " : "DIAGNOSTIC: weapon components file ") + file + ": " + std::to_string(sum) +
						" definitions loaded, the room " + std::to_string(g_room) + " - " + (before ? "refused, as before." : refuse ? "refused." : "let in."));
			}
			if (!refuse)
				return false;
			if (named && !before)
				Remember(hash);
			if (g_refused.load() == 0)
			{
				memcpy(g_firstRefused, name, sizeof(g_firstRefused));
				ReportFailure(kTag, std::string("Weapon component files are refused this session: ") + file + ", the first, came once " + std::to_string(sum) +
					" definitions were loaded, the room being " + std::to_string(g_room) + " - the weapon components of the files refused are missing in "
					"the game. Raise WEAPON_COMPONENTS_SIZE in TweaksV.ini (at most " + std::to_string(kWeaponComponentsSize.maxValue) + "); the log names "
					"every file refused.");
			}
			g_refused++;
			return true;
		}

		// The game calls this for each weaponcomponents.meta it loads, the base game's and the packs', on the thread that
		// applies the data file changes: nothing may throw out of here, a file let in goes to the game's loader as it came, and
		// the game's last error is kept - the guard's reads and lines may change it, and a refused file never reaches the loader.
		uint64_t GuardedLoader(void* entry, void* callback)
		{
			const DWORD lastError = GetLastError();
			bool refuse = false;
			if (g_guardOn.load())
			{
				try
				{
					refuse = Refuse(entry);
				}
				catch (...)
				{
				}
			}
			SetLastError(lastError);
			if (!refuse)
				return g_loader(entry, callback);
			DestroyCallback(callback);
			SetLastError(lastError);
			return 0;
		}

		// The guard's job not done: reported - the list's move and the pool's size stay.
		void GuardNotMade(const std::string& why)
		{
			ReportFailure(kTag, "The weapon components' guard is not made this session: " + why);
		}

		// Finds the loader by W-E-C5, checks it and hooks its head - with DRY_RUN says what it would hook. files: the blobs'
		// count W-E-C1 reads, which W-E-C5's must be.
		void MakeGuard(const RelocationPlan& plan, const uint8_t* files)
		{
			const bool diagnostic = IsDiagnosticMode();
			uint8_t* const match = Scanner::Find(kLoaderPattern);
			if (!match)
			{
				GuardNotMade(std::string(kLoaderPattern.label) + " was not found.");
				return;
			}
			// the head a function start: where the game's unwind data covers the match, the function it names must begin at the
			// head, so that a shorter prologue after a game update cannot put the hook into the padding between functions; the
			// byte before the head counts only for code without unwind data (TweaksV's fake game)
			uint8_t* const head = match - kLoaderHead;
			const std::string where = DescribeAddress(head) + " (" + kLoaderPattern.label + " - 0x13)";
			std::string detail = "not readable game code";
			bool start = IsInsideGameImage(head) && IsReadable(head, kLoaderHead);
			DWORD64 imageBase = 0;
			if (start)
			{
				if (const PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(match), &imageBase, nullptr))
				{
					const auto begin = reinterpret_cast<const uint8_t*>(imageBase + function->BeginAddress);
					start = begin == head;
					detail = start ? std::string("a function start (unwind data)") : std::string(kLoaderPattern.label) + " lies in the function at " +
						DescribeAddress(begin) + " (unwind data)";
				}
				else
					start = IsFunctionStart(head, detail);
			}
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: the weapon components' loader at " + where + ": " + detail);
			if (!start)
			{
				GuardNotMade("the loader at " + where + " is not a function start (" + detail + ").");
				return;
			}
			if (StartsWithJump(head))
			{
				GuardNotMade("the loader at " + DescribeAddress(head) + " starts with a jump (another mod may have hooked it already).");
				return;
			}
			const uint8_t* const count = Target(match, kBlobsCount);
			const uint8_t* const capacity = Target(match, kBlobsCapacity);
			const uint8_t* const array = Target(match, kBlobsArray);
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: the blob array's count at " + DescribeAddress(count) + ", capacity at " + DescribeAddress(capacity) +
					", pointer at " + DescribeAddress(array) + " (" + kLoaderPattern.label + ")");
			if (count != files)
			{
				GuardNotMade("the blobs' count " + std::string(kLoaderPattern.label) + " reads at " + DescribeAddress(count) + " is not the one " +
					kRebuildPattern.label + " reads at " + DescribeAddress(files) + ".");
				return;
			}
			if (capacity != count + sizeof(uint16_t) || array != count - sizeof(void*) || !InsideGame(array, sizeof(void*) + 2 * sizeof(uint16_t)))
			{
				GuardNotMade("the blob array's pointer at " + DescribeAddress(array) + " and capacity at " + DescribeAddress(capacity) + " do not lie 8 bytes "
					"before and 2 bytes after its count at " + DescribeAddress(count) + " inside the game (" + kLoaderPattern.label + ").");
				return;
			}
			const uint32_t room = static_cast<uint32_t>(GuardRoom(kTag, "weapon component guard", static_cast<int>(plan.size), static_cast<int>(plan.gameSize)));
			if (IsDryRun())
			{
				WriteLog(kTag, "DRY_RUN: would hook the weapon components' loader at " + DescribeAddress(head) + " to refuse the files that come once " +
					std::to_string(room) + " definitions are loaded.");
				return;
			}
			g_blobs = array;
			g_room = room;
			g_guardOn = true;
			if (!HookFunctions({ { head, reinterpret_cast<const void*>(&GuardedLoader), reinterpret_cast<void**>(&g_loader), "weapon components' loader" } }, kTag))
			{
				g_guardOn = false;
				GuardNotMade("the loader at " + DescribeAddress(head) + " could not be hooked (details in TweaksV.log).");
				return;
			}
			WriteLog(kTag, "The weapon components' loader at " + DescribeAddress(head) + " is hooked: a weaponcomponents.meta file that comes once " +
				std::to_string(room) + " definitions are loaded is refused.");
		}

		// ---- the measurement (DIAGNOSTIC_MODE): the list's count and the files' count, read as they are, and the loads

		const volatile int32_t* g_count = nullptr;		// the list's count: the definitions of every file loaded
		const volatile uint16_t* g_files = nullptr;		// the game's count of the files' blobs: the weaponcomponents.meta files loaded
		uint32_t g_gameSize = 0;						// the game's own list: 470 entries on 1.0.1158.16
		uint32_t g_size = 0;							// the room the list in use gives: WEAPON_COMPONENTS_SIZE as used, the game's own with DRY_RUN
		const uint8_t* g_moved = nullptr;				// the moved list, which every load must lead to - none with DRY_RUN
		const uint8_t* g_gameList = nullptr;			// the game's own list, which a load put back leads to again
		std::array<const uint8_t*, kListLoads> g_loads{};	// the rel32 of each load of the list
		bool g_putBackSaid = false;						// the measurement's thread alone reads and writes it
		constexpr uint64_t kUnreadable = uint64_t{ 1 } << 48;	// the two counts cannot be read
		constexpr uint64_t kPutBack = uint64_t{ 1 } << 49;		// a load of the list does not lead to the moved list
		constexpr int kRefusedShift = 50;						// the files refused, 0..0x3FFF, above them

		// The first load that no longer leads to the moved list - the game's protected code may put a load back - or null.
		const uint8_t* LoadPutBack()
		{
			if (!g_moved)
				return nullptr;
			for (const uint8_t* field : g_loads)
				if (!IsReadable(field, sizeof(int32_t)) || Rel32Target(field) != g_moved)
					return field;
			return nullptr;
		}

		// The count in the low 32 bits, the files above them - both read without a lock, after IsReadable - and whether a
		// load was put back.
		uint64_t Definitions()
		{
			const uint32_t refused = g_refused.load();
			const uint64_t flags = (LoadPutBack() ? kPutBack : 0) | uint64_t{ refused < 0x3FFF ? refused : 0x3FFF } << kRefusedShift;
			if (!IsReadable(const_cast<const int32_t*>(g_count), sizeof(int32_t)) || !IsReadable(const_cast<const uint16_t*>(g_files), sizeof(uint16_t)))
				return kUnreadable | flags;
			return uint64_t{ static_cast<uint32_t>(*g_count) } | uint64_t{ *g_files } << 32 | flags;
		}

		// A load put back, reported once: a search through it reads the game's own list, empty since the move, against
		// the count of the moved one - it reads the null pointers there, and the game is likely to crash. A load that cannot
		// be read any more is reported once too, as what it is.
		void ReportPutBack()
		{
			const uint8_t* const field = LoadPutBack();
			if (g_putBackSaid || !field)
				return;
			g_putBackSaid = true;
			const std::string load = "the load of the weapon component list at " + DescribeAddress(field - 3);
			if (!IsReadable(field, sizeof(int32_t)))
				ReportFailure(kTag, "The weapon components limit may not be kept this session: " + load + " cannot be read any more, so whether "
					"it still leads to the moved list at " + DescribeAddress(g_moved) + " cannot be told.");
			else if (Rel32Target(field) == g_gameList)
				ReportFailure(kTag, "The weapon components limit is not kept this session: " + load + " leads to the game's own list at " +
					DescribeAddress(g_gameList) + " again, not to the moved list at " + DescribeAddress(g_moved) + " - the game has put it back, and a "
					"search through it reads the game's own list, empty since the move: the game is likely to crash.");
			else
				ReportFailure(kTag, "The weapon components limit is not kept this session: " + load + " leads to " + DescribeAddress(Rel32Target(field)) +
					", not to the moved list at " + DescribeAddress(g_moved) + " - the game or another mod has changed it, and a search through it no "
					"longer reads the moved list: the game is likely to crash.");
		}

		void LogDefinitions(uint64_t numbers)
		{
			if (numbers & kPutBack)
				ReportPutBack();
			if (numbers & kUnreadable)
			{
				WriteLog(kTag, "DIAGNOSTIC: weapon components: the list's count at " + DescribeAddress(const_cast<const int32_t*>(g_count)) +
					" or the files' count at " + DescribeAddress(const_cast<const uint16_t*>(g_files)) + " cannot be read.");
				return;
			}
			const int32_t count = static_cast<int32_t>(static_cast<uint32_t>(numbers));
			const std::string files = Counted(static_cast<uint16_t>(numbers >> 32), "weaponcomponents.meta file");
			const uint32_t refused = g_refused.load();
			const std::string gameSize = std::to_string(g_gameSize);
			// past the list in use, the definition after its last entry was stored where the list ends: what the count
			// reads is no count
			if (!g_moved && (count < 0 || static_cast<uint32_t>(count) > g_gameSize))
			{
				WriteLog(kTag, "DIAGNOSTIC: the weapon component list's count reads " + std::to_string(count) + ", from " + files +
					" - outside the game's own list of " + gameSize + ": the list has overflowed, a definition past its end written over the count "
					"and the memory after it, and the game is likely to crash.");
				return;
			}
			if (count < 0 || static_cast<uint32_t>(count) > kMaxEntries)
			{
				WriteLog(kTag, "DIAGNOSTIC: the weapon component list's count reads " + std::to_string(count) + ", from " + files +
					" - outside the moved list of " + std::to_string(kMaxEntries) + ": the game's data is damaged, and the game is likely to crash.");
				return;
			}
			std::string line = "DIAGNOSTIC: " + Counted(static_cast<uint32_t>(count), "weapon component definition") + " from " + files +
				" (the game's own list holds " + gameSize + (g_moved ? ", this one " + std::to_string(g_size) + ")" : std::string(" and is in use)"));
			if (g_moved && static_cast<uint32_t>(count) > g_gameSize)
				line += " - the game's own list would have overflowed by " + std::to_string(static_cast<uint32_t>(count) - g_gameSize);
			if (!g_moved && static_cast<uint32_t>(count) >= g_size)
				line += " - it is full: one more definition crashes the game while it loads";
			else if (g_moved && static_cast<uint32_t>(count) >= g_size + kPerFile)
				line += " - the pool is full: one more definition crashes the game while it loads";
			else if (g_moved && static_cast<uint32_t>(count) >= g_size)
				line += g_guardOn.load() ? " - the room is used up: the guard refuses every further file" :
					" - the room is used up, and no guard refuses further files: past " + std::to_string(g_size + kPerFile) + " the game crashes while it loads";
			if (refused > 0)
				line += " - " + Counted(refused, "file") + " refused, the first " + (g_firstRefused[0] ? std::string(g_firstRefused) : std::string("not named"));
			if (numbers & kPutBack)
				line += " - a load of the list has changed, reported above";
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one a minute while the numbers change.
		constexpr Measurement kMeasurement = { "weapon components summary", kFirstSummaryMs, &Definitions, &LogDefinitions };
	}

	void InitWeaponComponents()
	{
		// 1. the list, its size and its 17 loads, on the core: the rebuild's and the sort's leas lead to one list inside the
		// game, the count the rebuild resets follows it a whole number of entries later, and exactly 17 leas with a 48 or
		// 4C prefix load it
		RelocationPlan plan;
		if (!FindRelocation(kSpec, plan))
			return;
		const bool diagnostic = IsDiagnosticMode();
		const uint8_t* const rebuild = plan.matches[kRebuild][0];
		const uint8_t* const sort = plan.matches[kSort][0];
		const uint8_t* const update = plan.matches[kUpdate][0];

		// 2. the count: the sort's read and the update's read and write lead to the one the rebuild resets, an int32 inside
		// the game; the entry size is pinned by W-E-C2, the files' count read for the measurement
		const uint8_t* const count = Target(rebuild, kCountReset);
		struct CountReference
		{
			const uint8_t* match;
			RipOperand operand;
			const GamePattern& pattern;
		};
		const CountReference references[] = { { sort, kSortCount, kSortPattern }, { update, kUpdateRead, kUpdatePattern },
			{ update, kUpdateWrite, kUpdatePattern } };
		for (const CountReference& reference : references)
		{
			const uint8_t* const target = Target(reference.match, reference.operand);
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: count reference at " + DescribeAddress(reference.match + reference.operand.field) + " -> " +
					DescribeAddress(target) + " (" + reference.pattern.label + ")");
			if (target != count)
			{
				RelocationFailed(kSpec, "the references to the weapon component list's count do not all point to one count: " + DescribeAddress(count) +
					" and " + DescribeAddress(target) + " (" + reference.pattern.label + ").");
				return;
			}
		}
		if (!InsideGame(count, sizeof(int32_t)))
		{
			RelocationFailed(kSpec, "the weapon component list's count at " + DescribeAddress(count) + " cannot be read.");
			return;
		}
		const uint8_t* const files = Target(rebuild, kFilesRead);
		if (diagnostic)
		{
			WriteLog(kTag, "DIAGNOSTIC: entry size at " + DescribeAddress(sort + kEntrySizeAt) + " holds " + std::to_string(Imm32At(sort + kEntrySizeAt)) +
				" (" + kSortPattern.label + ")");
			WriteLog(kTag, "DIAGNOSTIC: files' count reference at " + DescribeAddress(rebuild + kFilesRead.field) + " -> " + DescribeAddress(files) +
				" (" + kRebuildPattern.label + ")");
		}

		// 3. the pool's size call (W-E-C4), which TweaksVPools finds once a session for this part and StaticBounds: into the
		// game - not another mod's hook - and the pool's default a plausible size; the pool's name is pinned by W-E-C4
		PoolSizeCall pool;
		std::string why;
		const bool lookupFound = FindPoolSizeCall(pool, why);
		if (diagnostic && pool.call)
			WriteLog(kTag, "DIAGNOSTIC: pool size call at " + DescribeAddress(pool.call) + " -> " + DescribeAddress(pool.target) + " for the pool " +
				Hex32(pool.pool) + ", its default at " + DescribeAddress(pool.defaultSize) + " holds " + std::to_string(Imm32At(pool.defaultSize)) +
				", the game's configuration at " + DescribeAddress(pool.configuration) + " (" + kPoolSizeCallPattern.label + ")");
		if (!lookupFound)
		{
			RelocationFailed(kSpec, why);
			return;
		}
		const uint32_t poolDefault = Imm32At(pool.defaultSize);
		if (poolDefault == 0 || poolDefault > kMaxEntries)
		{
			RelocationFailed(kSpec, "the pool's default size at " + DescribeAddress(pool.defaultSize) + " holds " + std::to_string(poolDefault) +
				", which is no plausible size (" + kPoolSizeCallPattern.label + ").");
			return;
		}

		// 4. one write, all or nothing: the 17 loads to the moved list and the pool's size call to PoolSize
		uint8_t* const sizeCall = pool.call;
		g_poolSize = static_cast<int32_t>(plan.size + kPerFile);
		const std::string callAt = DescribeAddress(sizeCall);
		const std::vector<CallRedirect> redirects = { { sizeCall, reinterpret_cast<const void*>(&PoolSize), "pool size call",
			reinterpret_cast<void**>(&g_lookup) } };
		if (!ApplyRelocation(kSpec, plan, {}, redirects, " and redirect the pool size call at " + callAt))
			return;
		if (!IsDryRun())
			WriteLog(kTag, "The pool size call at " + callAt + " is redirected: the weapon components' pool gets " + std::to_string(plan.size + kPerFile) +
				" slots, the room of " + std::to_string(plan.size) + " and " + std::to_string(kPerFile) + " for the last file let in.");

		// 5. the guard: the loader hooked at its head - a job of its own, whose failure leaves the move and the pool
		MakeGuard(plan, files);

		// 6. the measurement (DIAGNOSTIC_MODE): the list's count and the files' count, the files refused, and the 17 loads checked
		// at every reading
		if (!diagnostic)
			return;
		if (!InsideGame(files, sizeof(uint16_t)))
		{
			WriteLog(kTag, "DIAGNOSTIC: the weapon components are not measured: the count of weaponcomponents.meta files is not inside the game: " +
				DescribeAddress(files) + " (" + kRebuildPattern.label + ").");
			return;
		}
		g_count = reinterpret_cast<const volatile int32_t*>(count);
		g_files = reinterpret_cast<const volatile uint16_t*>(files);
		g_gameSize = plan.gameSize;
		g_size = IsDryRun() ? plan.gameSize : plan.size;
		g_moved = IsDryRun() ? nullptr : Rel32Target(plan.references[0]);
		g_gameList = plan.array;
		for (size_t i = 0; i < kListLoads && i < plan.references.size(); i++)
			g_loads[i] = plan.references[i];
		WriteLog(kTag, std::string("DIAGNOSTIC: the weapon component definitions in the list and the weaponcomponents.meta files are logged after "
			"10 seconds, then every minute while the numbers change") + (IsDryRun() ? " (DRY_RUN: the game's own list is in use)." :
			", and the list's 17 loads are checked at each reading to lead to the moved list still."));
		StartMeasurement<kMeasurement>();	// one that cannot start is logged
	}
}
