// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the weapon infos, the WeaponLimits part's half for the weapons.meta files (see TweaksVWeaponInfos.h).
//
// The game parses each weapons.meta file - each data file of the type 0x4E - into one weapon info blob of 248 bytes
// and holds the blobs in one block on its heap: the weapon info manager's array, in the game's data, holds the
// block's pointer at +0, the number of blobs loaded at +8 and the room at +0xA, both 16-bit. A C++ static
// initializer of the game sets the array up once, with two immediates - "mov edx, 160" hands the number to the
// call that allocates and constructs that many blobs, "mov word [capacity], 160" gives the array its room - and the
// game's loader adds a blob for each file without any check, so a 161st file is parsed into the memory after the
// block. WEAPON_INFO_FILES in [WEAPON_LIMITS] (160..16384) is the room to give it. Two jobs, each done whole or not
// at all and left undone alone, so that a raise refused keeps the guard and a guard that cannot be set up keeps the
// raise:
//   - the raise: W-E-I1, the initializer's allocation, and W-E-I2, the loader's head, are found once each, and every
//     site is checked - the two immediates equal and not 0, since the game's exit destructor destroys as many blobs
//     as the capacity says; the call into the game, its target beginning with the allocation's bytes, which take the
//     number from edx alone, and its element size; the block's store and the capacity's store, the second 0xA after
//     the first - the array, inside the game; the loader's count read and written at the array + 8, its block read
//     at the array, its blob size the allocation's element size. Then the raise's own checks: exactly 2 references
//     to the array's address in the game's code, the block's store one of them, so that a second store of a block
//     would show; and the array still empty - no block, capacity 0 - which proves on every boot that its
//     initializer runs after TweaksV's DllMain. The two immediates are then raised together in one WriteCode, from
//     DllMain, before the initializer runs. Immediates that hold the setting or more already - another mod's
//     raise, or a later game build's own - are left as they are;
//   - the guard: the loader's start, W-E-I2's match - 0x18, a function start, is hooked. A file that comes once the
//     count has reached the room - the room the part gave the array, or the array's own while that is less - is
//     refused: the game's loader is not called, so nothing is written past the block. The refusals are counted and
//     the first refused file's name is kept, read bounded; 10 seconds after the first, off the game's thread, they
//     are reported once - in the start-up summary or in a box of their own (TweaksVSummary) - and later ones are
//     logged. A test build gives the guard the game's own room instead, with TWEAKSV_TEST_GUARD_ROOM=1 (GuardRoom);
//   - with DIAGNOSTIC_MODE=1 every site and every write is logged, the guard writes a line per file, before the
//     game's loader runs, and a measurement reads the count and the capacity back - after IsReadable, 16 bits each,
//     without a lock - 10 seconds in and then every minute while they change: the weapons.meta files loaded against
//     the room the game's code gives and the room the array has.
// DRY_RUN: nothing is written and nothing hooked - the would lines are logged, and the measurement reads the game's
// own array.
// Written for TweaksV from the facts its own research found in the running game (PATTERNS.md); no line of anyone's
// code is in it. The idea - the limit, a capacity handed as an immediate to the call that allocates the blobs, raised
// in place - comes from Weapon Limits Adjuster by alexguirre, its MIT half (CREDITS.md): "CWeaponInfoBlob
// Adjuster/dllmain.cpp" at revision 698f729, the only file of its code the research read.

#include "TweaksVWeaponInfos.h"

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "WeaponLimits";

		// W-E-I1, in the initializer: "mov edx, imm32" - the number of blobs allocated -, the call that allocates
		// them, "mov [rip+disp32], rax" - the block into the array - and "mov word [rip+disp32], imm16" - the capacity,
		// whose imm16 follows its disp32, so that the instruction ends 2 bytes after the disp32
		constexpr const GamePattern& kInitPattern = Pattern("W-E-I1");
		constexpr size_t kNumberAt = 10;			// BA imm32
		constexpr size_t kCallAt = 15;				// E8 rel32
		constexpr size_t kBlockStoreAt = 20;		// 48 89 05 disp32
		constexpr size_t kCapacityStoreAt = 27;		// 66 C7 05 disp32 imm16
		constexpr size_t kCapacityStoreSize = 9;
		static_assert(PatternByte(kInitPattern, kNumberAt) == 0xBA && PatternByte(kInitPattern, kCallAt) == 0xE8 &&
			PatternByte(kInitPattern, kBlockStoreAt) == 0x48 && PatternByte(kInitPattern, kBlockStoreAt + 1) == 0x89 &&
			PatternByte(kInitPattern, kBlockStoreAt + 2) == 0x05 && PatternByte(kInitPattern, kCapacityStoreAt) == 0x66 &&
			PatternByte(kInitPattern, kCapacityStoreAt + 1) == 0xC7 && PatternByte(kInitPattern, kCapacityStoreAt + 2) == 0x05 &&
			PatternByte(kInitPattern, kCapacityStoreAt + kCapacityStoreSize) == 0x48,
			"TweaksV: W-E-I1 holds the mov edx, the call, the block's store and the capacity's store at +10, +15, +20 and +27");

		// W-E-I2, the loader's head: "movzx eax, word [rip+disp32]" and "mov word [rip+disp32], cx" - the count, read
		// and written back one higher -, "imul r13, rax, imm32" - the blob's size - and "add r13, [rip+disp32]" - the
		// block: the new blob is the block + the old count x the size. The loader starts 0x18 bytes before the match.
		constexpr const GamePattern& kLoaderPattern = Pattern("W-E-I2");
		constexpr size_t kCountReadAt = 0;			// 0F B7 05 disp32
		constexpr size_t kCountWriteAt = 10;		// 66 89 0D disp32
		constexpr size_t kBlobSizeAt = 17;			// 4C 69 E8 imm32
		constexpr size_t kBlockReadAt = 24;			// 4C 03 2D disp32
		constexpr size_t kLoaderStart = 0x18;		// the loader starts this many bytes before the match
		static_assert(PatternByte(kLoaderPattern, kCountReadAt) == 0x0F && PatternByte(kLoaderPattern, kCountReadAt + 1) == 0xB7 &&
			PatternByte(kLoaderPattern, kCountReadAt + 2) == 0x05 && PatternByte(kLoaderPattern, kCountWriteAt) == 0x66 &&
			PatternByte(kLoaderPattern, kCountWriteAt + 1) == 0x89 && PatternByte(kLoaderPattern, kCountWriteAt + 2) == 0x0D &&
			PatternByte(kLoaderPattern, kBlobSizeAt) == 0x4C && PatternByte(kLoaderPattern, kBlobSizeAt + 1) == 0x69 &&
			PatternByte(kLoaderPattern, kBlobSizeAt + 2) == 0xE8 && PatternByte(kLoaderPattern, kBlockReadAt) == 0x4C &&
			PatternByte(kLoaderPattern, kBlockReadAt + 1) == 0x03 && PatternByte(kLoaderPattern, kBlockReadAt + 2) == 0x2D,
			"TweaksV: W-E-I2 holds the count's read and write, the blob size's imul and the block's add at +0, +10, +17 and +24");
		constexpr size_t kDispAt = 3;				// in each of these instructions: the disp32 after its prefix, opcode and ModRM

		// The allocation W-E-I1's call leads to begins "push rsi; push rdi; sub rsp, 28h; mov esi, edx; movsxd rdi, edx;
		// imul rcx, rdi, imm32" - the number from edx alone, times the element size, the imm32 after these bytes. Read
		// at the call's target, not looked for: a pattern of it matches twice in the game.
		constexpr uint8_t kAllocationHead[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x89, 0xD6, 0x48, 0x63, 0xFA, 0x48, 0x69, 0xCF };
		constexpr size_t kAllocationRead = sizeof(kAllocationHead) + sizeof(uint32_t);	// the bytes and the element size

		// The weapon info manager's array of blobs, in the game's data
		constexpr size_t kCountOffset = 8;			// the blobs loaded, 16-bit
		constexpr size_t kCapacityOffset = 0xA;		// the room, 16-bit
		constexpr size_t kArraySize = 0xC;			// the block's pointer, the count and the capacity
		constexpr size_t kExpectedReferences = 2;	// every lea and mov store of its address: the block's store and the initializer's lea
		static_assert(kWeaponInfoFiles.maxValue <= UINT16_MAX, "TweaksV: WEAPON_INFO_FILES must fit the array's 16-bit capacity");

		bool g_diagnostic = false;		// DIAGNOSTIC_MODE, read once: the hook never reads the settings

		template <typename T> T ValueAt(const uint8_t* address)
		{
			T value{};
			memcpy(&value, address, sizeof(value));
			return value;
		}

		std::string Hex(uint32_t value)
		{
			char text[16];
			snprintf(text, sizeof(text), "0x%X", value);
			return text;
		}

		// " (W-E-I1)" - the pattern a line's site lies in
		std::string In(const GamePattern& pattern)
		{
			return std::string(" (") + pattern.label + ")";
		}

		void Diagnostic(const std::string& text)
		{
			if (g_diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: " + text);
		}

		// ---- the sites both jobs build on

		struct Sites
		{
			uint8_t* init = nullptr;			// W-E-I1's match, in the initializer
			uint8_t* loader = nullptr;			// W-E-I2's match, in the loader
			uint8_t* allocatedAt = nullptr;		// the imm32 of "mov edx": the blobs the game allocates
			uint8_t* capacityAt = nullptr;		// the imm16 of the capacity's store
			uint32_t gameRoom = 0;				// what both hold: the game's own room
			uint32_t elementSize = 0;			// the allocation's, the loader's blob size
			const uint8_t* array = nullptr;		// the weapon info manager's array of blobs
		};

		// Finds and checks every site both jobs need; the reason for the first that is not as expected, or "".
		std::string FindSites(Sites& sites)
		{
			// 1. the two patterns, both looked for, so that the log names each one missing
			sites.init = Scanner::Find(kInitPattern);
			sites.loader = Scanner::Find(kLoaderPattern);
			if (!sites.init || !sites.loader)
				return std::string(sites.init ? kLoaderPattern.label : kInitPattern.label) + " was not found.";
			const uint8_t* const init = sites.init;
			const uint8_t* const loader = sites.loader;

			// 2. W-E-I1's two immediates, the blobs allocated and the array's capacity: equal - the exit destructor destroys
			// as many blobs as the capacity says - and not 0; the imm16 holds no more than the 16-bit count can
			sites.allocatedAt = sites.init + kNumberAt + 1;
			sites.capacityAt = sites.init + kCapacityStoreAt + kCapacityStoreSize - sizeof(uint16_t);
			const uint32_t allocated = ValueAt<uint32_t>(sites.allocatedAt);
			const uint32_t capacity = ValueAt<uint16_t>(sites.capacityAt);
			Diagnostic("limit at " + DescribeAddress(sites.allocatedAt) + " holds " + std::to_string(allocated) + In(kInitPattern) +
				": the blobs the game allocates");
			Diagnostic("limit at " + DescribeAddress(sites.capacityAt) + " holds " + std::to_string(capacity) + In(kInitPattern) +
				": the array's capacity");
			if (allocated != capacity)
				return "the two limits of the weapon info blob array disagree: " + std::to_string(allocated) + " at " +
					DescribeAddress(sites.allocatedAt) + " and " + std::to_string(capacity) + " at " + DescribeAddress(sites.capacityAt) + " (" +
					kInitPattern.label + "; another mod may have changed one).";
			if (allocated == 0)
				return "the limits of the weapon info blob array hold 0" + In(kInitPattern) + ".";
			sites.gameRoom = allocated;

			// 3. the call to the allocation: into the game, the allocation's first bytes there and its element size
			const uint8_t* const call = init + kCallAt;
			const uint8_t* const allocation = CallTarget(call);
			const bool readable = IsInsideGameImage(allocation) && IsReadable(allocation, kAllocationRead);
			Diagnostic("allocation call at " + DescribeAddress(call) + " -> " + DescribeAddress(allocation) + In(kInitPattern) +
				(readable ? ": " + HexBytes(allocation, kAllocationRead) : std::string()));
			if (!IsInsideGameImage(allocation))
				return "the allocation call at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" +
					In(kInitPattern) + ".";
			if (!readable || memcmp(allocation, kAllocationHead, sizeof(kAllocationHead)) != 0)
				return "the allocation at " + DescribeAddress(allocation) + (readable ? " does not begin with the bytes it is expected to" :
					" cannot be read") + In(kInitPattern) + ".";
			sites.elementSize = ValueAt<uint32_t>(allocation + sizeof(kAllocationHead));

			// 4. the block's store, which gives the array, and the capacity's store, 0xA after it: the array inside the game
			const uint8_t* const blockStore = init + kBlockStoreAt + kDispAt;
			const uint8_t* const array = Rel32Target(blockStore);
			const uint8_t* const capacityStore = init + kCapacityStoreAt + kDispAt;
			const uint8_t* const capacityField = Rel32Target(capacityStore) + sizeof(uint16_t);	// the instruction ends after its imm16
			Diagnostic("reference at " + DescribeAddress(blockStore) + " -> " + DescribeAddress(array) + In(kInitPattern) +
				": the block's store - the weapon info blob array");
			Diagnostic("reference at " + DescribeAddress(capacityStore) + " -> " + DescribeAddress(capacityField) + In(kInitPattern) +
				": the capacity's store");
			if (!IsInsideGameImage(array) || !IsInsideGameImage(array + kArraySize - 1))
				return "the weapon info blob array at " + DescribeAddress(array) + In(kInitPattern) + " is not inside the game.";
			if (capacityField != array + kCapacityOffset)
				return "the capacity's store at " + DescribeAddress(capacityStore) + " leads to " + DescribeAddress(capacityField) +
					", not to the array's capacity at " + DescribeAddress(array + kCapacityOffset) + In(kInitPattern) + ".";
			sites.array = array;

			// 5. W-E-I2, the loader: its count read and written at the array + 8, its block read at the array, its blob
			// size the allocation's element size
			const struct
			{
				const uint8_t* field;
				const char* what;
				const uint8_t* expected;
			} loaderReferences[] = {
				{ loader + kCountReadAt + kDispAt, "the loader's read of the count", array + kCountOffset },
				{ loader + kCountWriteAt + kDispAt, "the loader's write of the count", array + kCountOffset },
				{ loader + kBlockReadAt + kDispAt, "the loader's read of the block", array },
			};
			for (const auto& reference : loaderReferences)
			{
				const uint8_t* const target = Rel32Target(reference.field);
				Diagnostic("reference at " + DescribeAddress(reference.field) + " -> " + DescribeAddress(target) + In(kLoaderPattern) + ": " +
					reference.what);
				if (target != reference.expected)
					return std::string(reference.what) + " at " + DescribeAddress(reference.field) + " leads to " + DescribeAddress(target) +
						", not to " + DescribeAddress(reference.expected) + In(kLoaderPattern) + ".";
			}
			const uint8_t* const blobSizeAt = loader + kBlobSizeAt + kDispAt;
			const uint32_t blobSize = ValueAt<uint32_t>(blobSizeAt);
			Diagnostic("blob size at " + DescribeAddress(blobSizeAt) + " holds " + Hex(blobSize) + In(kLoaderPattern));
			if (blobSize != sites.elementSize)
				return "the loader's blob size, " + Hex(blobSize) + " at " + DescribeAddress(blobSizeAt) + ", is not the allocation's element size, " +
					Hex(sites.elementSize) + In(kLoaderPattern) + ".";
			if (blobSize == 0)
				return "the blob size of the loader and the allocation is 0" + In(kLoaderPattern) + ".";
			return std::string();
		}

		// The array's state as TweaksV finds it, for a line: " (no block, 0 loaded, capacity 0)"
		std::string ArrayState(uint64_t block, uint32_t loaded, uint32_t room)
		{
			return " (" + std::string(block ? "a block" : "no block") + ", " + std::to_string(loaded) + " loaded, capacity " +
				std::to_string(room) + ")";
		}

		// ---- the raise

		enum class Raise { Raised, Already, Refused };

		struct RaiseResult
		{
			Raise outcome = Raise::Refused;
			uint32_t room = 0;		// the room the array gets - with DRY_RUN the room it would get
		};

		void RaiseFailed(const std::string& why)
		{
			ReportFailure(kTag, "The weapon info blob limit is not raised this session: " + why);
		}

		// The raise's own checks, then its one write: both immediates to WEAPON_INFO_FILES.
		RaiseResult RaiseLimit(const Sites& sites)
		{
			const RaiseResult refused = { Raise::Refused, sites.gameRoom };

			// 1. every lea and mov store of the array's address in the game's code: exactly the block's store, which gave the
			// array and so is always among them, and the initializer's lea - a second store of a block, or another
			// reference, would show here
			const std::vector<uint8_t*> references = Scanner::FindRipRelativeReferences(sites.array, kExpectedReferences + 1);
			for (const uint8_t* reference : references)
				Diagnostic("reference at " + DescribeAddress(reference + kDispAt) + " -> " + DescribeAddress(sites.array) + " (scan)");
			if (references.size() != kExpectedReferences)
			{
				RaiseFailed("the game code holds " + std::to_string(references.size()) + (references.size() > kExpectedReferences ? " or more" : "") +
					(references.size() == 1 ? " reference" : " references") + " to the weapon info blob array, not the expected " +
					std::to_string(kExpectedReferences) + " (another mod may have changed one, or the game has changed).");
				return refused;
			}

			// 2. the array still empty - no block, capacity 0, the game's data as the game was loaded: its initializer has
			// not run yet, so the immediates it runs with can still be raised
			if (!IsReadable(sites.array, kArraySize))
			{
				RaiseFailed("the weapon info blob array at " + DescribeAddress(sites.array) + " cannot be read.");
				return refused;
			}
			const uint64_t block = *reinterpret_cast<const volatile uint64_t*>(sites.array);
			const uint32_t loaded = *reinterpret_cast<const volatile uint16_t*>(sites.array + kCountOffset);
			const uint32_t room = *reinterpret_cast<const volatile uint16_t*>(sites.array + kCapacityOffset);
			if (block != 0 || room != 0)
			{
				RaiseFailed("the weapon info blob array at " + DescribeAddress(sites.array) + " was set up before TweaksV started" +
					ArrayState(block, loaded, room) + ": its initializer ran before TweaksV's DllMain.");
				return refused;
			}
			Diagnostic("the weapon info blob array at " + DescribeAddress(sites.array) + " is still empty at TweaksV's start" +
				ArrayState(block, loaded, room) + ": its initializer runs after TweaksV's DllMain, so its limit can be raised in time.");

			// 3. the room to give it: WEAPON_INFO_FILES, 160..16384 - immediates that hold as much or more already, another
			// mod's raise or a later game build's own, are left as they are
			const uint32_t setting = static_cast<uint32_t>(GetNumber(kWeaponInfoFiles));
			if (sites.gameRoom >= setting)
			{
				WriteLog(kTag, "The weapon info blob limit is " + std::to_string(sites.gameRoom) + " already (WEAPON_INFO_FILES=" + std::to_string(setting) +
					") - nothing to patch.");
				return { Raise::Already, sites.gameRoom };
			}

			// 4. one write of both immediates, all or nothing
			const std::string change = " (" + std::to_string(sites.gameRoom) + " -> " + std::to_string(setting) + ")";
			if (!WriteCode({ { sites.allocatedAt, BytesOf(setting), "weapon info blob array size" + change },
				{ sites.capacityAt, BytesOf(static_cast<uint16_t>(setting)), "weapon info blob array capacity" + change } }, kTag))
			{
				RaiseFailed("the game code could not be made writable (details in TweaksV.log).");
				return refused;
			}
			if (!IsDryRun())
				WriteLog(kTag, "The weapon info blob limit is raised from " + std::to_string(sites.gameRoom) + " to " + std::to_string(setting) +
					" weapons.meta files.");
			return { Raise::Raised, setting };
		}

		// ---- the guard, on the game's loader

		constexpr size_t kNameRead = 127;	// the most of a file's name read and kept: its data file entry holds it at +0

		// The loader's arguments are passed on as they come: the data file entry in rcx, whatever its callers hand in
		// rdx, r8 and r9. What it returns is passed back; a file refused gets 0 - the pack loader that mounts packs later
		// returns true after it either way.
		using Loader = uintptr_t (*)(void* entry, void* second, void* third, void* fourth);
		Loader g_loader = nullptr;					// the game's loader, through MinHook's trampoline

		// Set by InitWeaponInfos before the hook can run
		const volatile uint16_t* g_count = nullptr;
		const volatile uint16_t* g_capacity = nullptr;
		uint32_t g_room = 0;						// the room the guard keeps the count below
		uint32_t g_gameRoom = 0;					// the game's own, for the lines per file
		bool g_refusing = false;					// the guard refuses: its report's thread runs
		std::string g_advice;						// what the report says to do, by the raise's outcome
		HANDLE g_refusedEvent = nullptr;			// set at the first refusal: the report's clock

		// On the game's loading thread
		std::atomic<uint32_t> g_refused{ 0 };		// the files refused this session
		char g_firstRefused[kNameRead + 1] = {};	// the first one's name, kept before g_refusedEvent is set
		uint32_t g_refusedRoom = 0;					// the room it met, kept with it
		std::atomic<uint64_t> g_fileLines{ 0 };		// DIAGNOSTIC_MODE's lines per file, capped

		// A file's name for a line: at most kNameRead bytes from the entry the game's loader got - copied safely, as it
		// comes from the game - printable ASCII, anything else a '?'.
		void ReadName(const void* entry, char (&name)[kNameRead + 1])
		{
			if (!entry || !CopyMemorySafely(name, entry, kNameRead))
			{
				strcpy_s(name, "(a name that cannot be read)");
				return;
			}
			name[kNameRead] = '\0';
			if (name[0] == '\0')
			{
				strcpy_s(name, "(no name)");
				return;
			}
			for (char& c : name)
			{
				const unsigned char byte = static_cast<unsigned char>(c);
				if (byte == 0)
					break;
				if (byte < 0x20 || byte > 0x7E)
					c = '?';
			}
		}

		// DIAGNOSTIC_MODE's line for a file, capped; nothing may throw back into the game.
		void LogFile(uint32_t number, bool refused, uint32_t room, const char* name)
		{
			try
			{
				const uint64_t n = g_fileLines.fetch_add(1, std::memory_order_relaxed) + 1;
				if (n > kMaxLinesPerKind)
				{
					if (n == kMaxLinesPerKind + 1)
						WriteLog(kTag, "DIAGNOSTIC: " + std::to_string(kMaxLinesPerKind) + " lines of weapons.meta files written - from now on they are "
							"only counted; the summary every minute has the numbers.");
					return;
				}
				std::string line = "DIAGNOSTIC: weapons.meta file " + std::to_string(number);
				if (refused)
					line += " refused (the room, " + std::to_string(room) + ", is full)";
				else if (number > g_gameRoom)
					line += " (past the game's own " + std::to_string(g_gameRoom) + ")";
				WriteLog(kTag, line + ": " + name);
			}
			catch (...)
			{
			}
		}

		// The hook: one 16-bit comparison per file, the game's loader called last, so that its own last error stands. A
		// file refused keeps the game's last error as it came.
		uintptr_t LoaderHook(void* entry, void* second, void* third, void* fourth)
		{
			const uint32_t count = *g_count;
			const uint32_t capacity = *g_capacity;
			const uint32_t room = capacity < g_room ? capacity : g_room;
			const bool refuse = g_refusing && count >= room;
			if (refuse || g_diagnostic)
			{
				const DWORD lastError = GetLastError();
				char name[kNameRead + 1] = {};
				ReadName(entry, name);
				if (refuse && g_refused.fetch_add(1) == 0)
				{
					memcpy(g_firstRefused, name, sizeof(name));
					g_refusedRoom = room;
					SetEvent(g_refusedEvent);
				}
				if (g_diagnostic)
					LogFile(count + 1, refuse, room, name);
				SetLastError(lastError);
				if (refuse)
					return 0;
			}
			return g_loader(entry, second, third, fourth);
		}

		// The report, on a thread of its own 10 seconds after the first refusal and then every minute while more come:
		// the first time a problem - the start-up summary's, or a box of its own after it - and then a line. The thread
		// waits for the event the first refusal sets, after it kept the file's name and its room.
		uint64_t Refusals()
		{
			return g_refused.load();
		}

		bool g_reported = false;	// the report's thread alone uses it

		void ReportRefusals(uint64_t refused)
		{
			const std::string room = "the game's room for weapons.meta files, " + std::to_string(g_refusedRoom) + ", is full";
			if (!g_reported)
			{
				g_reported = true;
				const std::string advice = g_refusedRoom < g_room ? " - the array has less room than the " + std::to_string(g_room) +
					" TweaksV gave it (another mod may have changed its limit after TweaksV)." : g_advice;
				ReportFailure(kTag, std::string("The weapons.meta file ") + g_firstRefused +
					(refused == 1 ? " was" : " and " + std::to_string(refused - 1) + " more were") + " not loaded: " + room + advice);
				return;
			}
			WriteLog(kTag, std::to_string(refused) + " weapons.meta files in all were not loaded this session: " + room + ".");
		}

		constexpr Measurement kGuardReport = { "weapon infos guard report", kFirstSummaryMs, &Refusals, &ReportRefusals, &g_refusedEvent };

		template <typename F> void** Original(F*& variable) { return reinterpret_cast<void**>(&variable); }

		void GuardFailed(const std::string& why)
		{
			ReportFailure(kTag, "The weapon info blob limit is not guarded this session: " + why);
		}

		// The guard's own check: the loader's start, where it is hooked - inside the game and a function start. Where the
		// game's unwind data covers the match, the function it names must begin at the start; the byte before the start
		// counts only for code without unwind data (TweaksV's fake game), so that a shorter prologue after a game update
		// cannot put the hook into the padding between functions. The reason it is not, or "".
		std::string CheckLoaderStart(const Sites& sites)
		{
			const uint8_t* const start = sites.loader - kLoaderStart;
			const std::string where = DescribeAddress(start) + " (" + kLoaderPattern.label + " - " + Hex(kLoaderStart) + ")";
			if (!IsInsideGameImage(start) || !IsReadable(start, 2))	// the two bytes StartsWithJump reads
				return "the loader at " + where + " is not readable game code.";
			DWORD64 imageBase = 0;
			if (const PRUNTIME_FUNCTION entry = RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(sites.loader), &imageBase, nullptr))
			{
				const auto begin = reinterpret_cast<const uint8_t*>(imageBase + entry->BeginAddress);
				Diagnostic("the loader at " + where + ": " + (begin == start ? std::string("a function start (unwind data)") :
					"inside the function at " + DescribeAddress(begin) + " (unwind data)"));
				if (begin != start)
					return "the loader at " + where + " is not a function start: " + kLoaderPattern.label + " lies in the function at " +
						DescribeAddress(begin) + " (unwind data).";
				return std::string();
			}
			std::string detail;
			const bool functionStart = IsFunctionStart(start, detail);
			Diagnostic("the loader at " + where + ": " + detail);
			if (!functionStart)
				return "the loader at " + where + " is not a function start (" + detail + ").";
			return std::string();
		}

		// The hook and its report's thread, both or neither.
		void GuardLimit(const Sites& sites, const RaiseResult& raised)
		{
			uint8_t* const start = sites.loader - kLoaderStart;
			const uint32_t room = static_cast<uint32_t>(GuardRoom(kTag, "weapon infos guard", static_cast<int>(raised.room),
				static_cast<int>(sites.gameRoom)));
			if (IsDryRun())
			{
				WriteLog(kTag, "DRY_RUN: would hook the weapon info loader at " + DescribeAddress(start) + " so that a weapons.meta file past the room, " +
					std::to_string(room) + ", is refused.");
				return;
			}
			if (StartsWithJump(start))
				WriteLog(kTag, "The weapon info loader at " + DescribeAddress(start) + " already starts with a jump - another mod may have hooked "
					"it; the guard goes on top and chains through.");

			// what the hook and the report use from their first call on
			g_count = reinterpret_cast<const volatile uint16_t*>(sites.array + kCountOffset);
			g_capacity = reinterpret_cast<const volatile uint16_t*>(sites.array + kCapacityOffset);
			g_room = room;
			const uint32_t most = static_cast<uint32_t>(kWeaponInfoFiles.maxValue);
			if (raised.outcome == Raise::Refused)
				g_advice = ": its limit was not raised this session (see above).";
			else if (raised.room < most)
				g_advice = " - raise WEAPON_INFO_FILES in TweaksV.ini above " + std::to_string(raised.room) + " (at most " + std::to_string(most) + ").";
			else
				g_advice = ", the most WEAPON_INFO_FILES allows.";
			g_refusedEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
			if (!g_refusedEvent)
			{
				const DWORD error = GetLastError();
				GuardFailed("no event for its report could be made (" + DescribeError(error) + ").");
				return;
			}

			if (!HookFunctions({ { start, reinterpret_cast<const void*>(&LoaderHook), Original(g_loader), "the weapon info loader" } }, kTag))
			{
				GuardFailed("the weapon info loader at " + DescribeAddress(start) + " could not be hooked (details in TweaksV.log).");
				return;
			}
			if (!StartMeasurement<kGuardReport>())	// logged; the hook goes off again, or - should that fail - passes every file on
			{
				DisableHook(start, kTag);
				GuardFailed("the thread that reports the refused files could not be started (details in TweaksV.log).");
				return;
			}
			g_refusing = true;
			WriteLog(kTag, "The weapon info blob limit is guarded: the weapon info loader at " + DescribeAddress(start) + " is hooked, and a "
				"weapons.meta file past the room, " + std::to_string(room) + ", is refused instead of being written past the array's end.");
		}

		// ---- the measurement: the array's count and capacity, read as they are, and the files the guard refused

		constexpr uint64_t kUnreadable = uint64_t{ 1 } << 63;	// in the place of the numbers
		constexpr uint64_t kMaxCounted = 0x7FFFFFFF;			// the refusals, in bits 32 to 62

		const volatile uint8_t* g_array = nullptr;	// the weapon info manager's array of blobs

		// The count, the capacity and the refusals, after IsReadable and without a lock: each 16 bits, aligned, read in
		// one load.
		uint64_t ArrayNumbers()
		{
			if (!IsReadable(const_cast<const uint8_t*>(g_array), kArraySize))
				return kUnreadable;
			const uint16_t count = *reinterpret_cast<const volatile uint16_t*>(g_array + kCountOffset);
			const uint16_t capacity = *reinterpret_cast<const volatile uint16_t*>(g_array + kCapacityOffset);
			const uint64_t refused = g_refused.load();
			return count | uint64_t{ capacity } << 16 | (refused < kMaxCounted ? refused : kMaxCounted) << 32;
		}

		void LogArray(uint64_t numbers)
		{
			if (numbers == kUnreadable)
			{
				WriteLog(kTag, "DIAGNOSTIC: the weapon info blob array at " + DescribeAddress(const_cast<const uint8_t*>(g_array)) + " cannot be read.");
				return;
			}
			const uint32_t count = numbers & 0xFFFF;
			const uint32_t capacity = (numbers >> 16) & 0xFFFF;
			const uint64_t refused = numbers >> 32;
			std::string line = "DIAGNOSTIC: " + std::to_string(count) + " weapons.meta files loaded (the game's own array holds " +
				std::to_string(g_gameRoom) + (capacity == 0 ? std::string(", and the array is not set up yet") : ", this one " + std::to_string(capacity)) + ")";
			if (count > g_gameRoom && count <= capacity)
				line += " - the game's own array would have overflowed by " + std::to_string(count - g_gameRoom) + " of them";
			const uint32_t room = capacity < g_room ? capacity : g_room;
			if (count > capacity)
				line += " - past its end: the game's memory is already damaged";
			else if (g_refusing && refused > 0)
				line += " - the guard's room, " + std::to_string(room) + ", is full: " + std::to_string(refused) + (refused == 1 ? " file was" : " files were") +
					" refused";
			else if (g_refusing && count >= room && capacity != 0)
				line += " - the guard's room, " + std::to_string(room) + ", is full: the next file is refused";
			else if (!g_refusing && count == capacity && capacity != 0)
				line += " - it is full: the next file would be written past its end";
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one a minute while the numbers change.
		constexpr Measurement kMeasurement = { "weapon infos summary", kFirstSummaryMs, &ArrayNumbers, &LogArray };
	}

	void InitWeaponInfos()
	{
		g_diagnostic = IsDiagnosticMode();

		// 1. the sites both jobs build on, and the guard's own: a failure of the first ends both jobs
		Sites sites;
		if (const std::string why = FindSites(sites); !why.empty())
		{
			ReportFailure(kTag, "The weapon info blob limit is neither raised nor guarded this session: " + why);
			return;
		}
		g_gameRoom = sites.gameRoom;
		const std::string loaderWhy = CheckLoaderStart(sites);

		// 2. the raise, then the guard, each on its own
		const RaiseResult raised = RaiseLimit(sites);
		if (loaderWhy.empty())
			GuardLimit(sites, raised);
		else
			GuardFailed(loaderWhy);

		// 3. the measurement
		if (!g_diagnostic)
			return;
		g_array = sites.array;
		WriteLog(kTag, "DIAGNOSTIC: the weapon info blob array at " + DescribeAddress(sites.array) + In(kInitPattern) + ", blobs of " +
			std::to_string(sites.elementSize) + " bytes: the weapons.meta files loaded and its room are logged after 10 seconds, then every minute "
			"while the numbers change.");
		StartMeasurement<kMeasurement>();	// one that cannot start is logged
	}
}
