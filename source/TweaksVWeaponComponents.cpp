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
//                    the relocation core to one of 16384 entries - its 17 leas re-pointed, 6 of them in the game's
//                    second, protected code - and its count left where it is (the core's follower: the list's size
//                    is the distance to it); the call that reads the pool's size from the game's configuration is
//                    redirected so that the pool gets exactly this many slots, also when the configuration names
//                    more, so that the pool's trap always comes first. The 470 definitions one file may hold stay.
//   DIAGNOSTIC_MODE  every site logged, and the measurement: 10 seconds in and then every minute while they
//                    change, the definitions in the list and the weaponcomponents.meta files loaded; at every
//                    reading the 17 loads are checked to lead to the moved list still, and a load the game's
//                    protected code put back is reported, once
// All of it in one write, all or nothing; a list another mod moved, a pool size call another mod hooked, a load
// more or fewer than 17 are refused with the reason. Written for TweaksV from its own findings in the game's code
// (PATTERNS.md, "WeaponLimits"); that the limit exists, and that the pool's size must grow with it, is what
// alexguirre's Weapon Limits Adjuster for GTA V Legacy says on its page - none of its code, nor FiveM's, from
// which its components half is adapted, was read or used (CREDITS.md).

#include "TweaksVWeaponComponents.h"

#include <windows.h>

#include <array>
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
#include "TweaksVRelocation.h"
#include "TweaksVSettings.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "WeaponLimits";

		// ---- the list, its count and the pool's size (PATTERNS.md, "WeaponLimits")

		constexpr uint32_t kEntrySize = 8;					// an entry of the list: a definition's pointer
		constexpr uint32_t kMaxEntries = 16384;				// the moved list always has this many entries, whatever WEAPON_COMPONENTS_SIZE says
		static_assert(kMaxEntries == static_cast<uint32_t>(kWeaponComponentsSize.maxValue), "TweaksV: WEAPON_COMPONENTS_SIZE's maximum must be the list's size");
		constexpr size_t kListLoads = 17;					// the "lea r64, [rip+disp32]" that load the list - every one of them, on 1.0.1158.16
		constexpr char kLeaPrefixes[] = "\x48\x4C";			// the REX bytes the game's loads of the list start with
		constexpr uint32_t kPoolNameHash = 0xBAE33CCF;		// the game's hash of the pool's name, "cweaponcomponentinfo"

		constexpr const GamePattern& kRebuildPattern = Pattern("W-E-C1");	// the rebuild: the count's reset, the files' count, its lea of the list
		constexpr const GamePattern& kSortPattern = Pattern("W-E-C2");		// the sort: the count, its lea of the list, the entry size
		constexpr const GamePattern& kUpdatePattern = Pattern("W-E-C3");	// the rebuild's update of the count, before its unbounded store
		constexpr const GamePattern& kPoolPattern = Pattern("W-E-C4");		// the pool's size: the configuration, the pool's name and default, the call

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
		constexpr RipOperand kConfiguration = { 3, 7 };		// W-E-C4: "mov rcx, [rip+disp32]" - the game's configuration
		constexpr size_t kHashAt = 8;						// W-E-C4: the imm32 of "mov edx, imm32" - the pool's name
		constexpr size_t kDefaultAt = 14;					// W-E-C4: the imm32 of "mov r8d, imm32" - the pool's default size
		constexpr size_t kSizeCallAt = 18;					// W-E-C4: "call rel32" - the size lookup, the call the part redirects

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
		static_assert(PatternHolds(kPoolPattern, kConfiguration.field - 3, "\x48\x8B\x0D") && kConfiguration.end == kConfiguration.field + 4,
			"TweaksV: W-E-C4 holds the configuration's mov rcx, [rip+disp32] at +0");
		static_assert(PatternHolds(kPoolPattern, kHashAt - 1, "\xBA") && PatternImm32(kPoolPattern, kHashAt) == kPoolNameHash,
			"TweaksV: W-E-C4 holds the pool's name, mov edx, 0xBAE33CCF, at +7");
		static_assert(PatternHolds(kPoolPattern, kDefaultAt - 2, "\x41\xB8") && PatternImm32(kPoolPattern, kDefaultAt) == -1,
			"TweaksV: W-E-C4 holds the pool's default, mov r8d, imm32, at +12, the imm32 read");
		static_assert(PatternHolds(kPoolPattern, kSizeCallAt, "\xE8"), "TweaksV: W-E-C4 holds the size lookup's call at +18");

		// The anchors on the core, in this order: the rebuild - its lea of the list, and its reset of the count, the
		// variable that follows the list, whose distance from it is the list's size; the sort's lea of the list; the
		// count's update and the pool's size call, which the part reads itself.
		enum : size_t { kRebuild, kSort, kUpdate, kPool };
		constexpr Anchor kAnchors[] = {
			ReferenceAndLimit(kRebuildPattern, Rel32(kRebuildLoad.field, kRebuildLea, 3),
				Follower(kCountReset.field - 2, kResetOpcode, 2, kCountReset.field, kCountReset.end)),
			Reference(kSortPattern, Rel32(kSortLoad.field, kSortLea, 3)),
			Site(kUpdatePattern),
			Site(kPoolPattern),
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

		// ---- the pool's size: the game's call that reads it from its configuration, redirected

		// The game's pool size lookup a call leads to, or null when the call does not lead into the game - another mod's
		// hook. The lookup serves every pool of the game, so only the call is redirected, never the lookup.
		const uint8_t* PoolSizeLookupOf(const uint8_t* call)
		{
			const uint8_t* const lookup = CallTarget(call);
			return lookup && IsInsideGameImage(lookup) ? lookup : nullptr;
		}

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

		// ---- the measurement (DIAGNOSTIC_MODE): the list's count and the files' count, read as they are, and the loads

		const volatile int32_t* g_count = nullptr;		// the list's count: the definitions of every file loaded
		const volatile uint16_t* g_files = nullptr;		// the game's count of the files' blobs: the weaponcomponents.meta files loaded
		uint32_t g_gameSize = 0;						// the game's own list: 470 entries on 1.0.1158.16
		uint32_t g_size = 0;							// the room the list in use gives: WEAPON_COMPONENTS_SIZE as used, the game's own with DRY_RUN
		const uint8_t* g_moved = nullptr;				// the moved list, which every load must lead to - none with DRY_RUN
		std::array<const uint8_t*, kListLoads> g_loads{};	// the rel32 of each load of the list
		bool g_putBackSaid = false;						// the measurement's thread alone reads and writes it
		constexpr uint64_t kUnreadable = uint64_t{ 1 } << 48;	// the two counts cannot be read
		constexpr uint64_t kPutBack = uint64_t{ 1 } << 49;		// a load of the list does not lead to the moved list

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
			const uint64_t putBack = LoadPutBack() ? kPutBack : 0;
			if (!IsReadable(const_cast<const int32_t*>(g_count), sizeof(int32_t)) || !IsReadable(const_cast<const uint16_t*>(g_files), sizeof(uint16_t)))
				return kUnreadable | putBack;
			return uint64_t{ static_cast<uint32_t>(*g_count) } | uint64_t{ *g_files } << 32 | putBack;
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
			else
				ReportFailure(kTag, "The weapon components limit is not kept this session: " + load + " leads to " + DescribeAddress(Rel32Target(field)) +
					" again, not to the moved list at " + DescribeAddress(g_moved) + " - the game has put it back, and a search through it reads "
					"the game's own list, empty since the move: the game is likely to crash.");
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
			if (static_cast<uint32_t>(count) >= g_size)
				line += " - it is full: one more definition crashes the game while it loads";
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
		uint8_t* const pool = plan.matches[kPool][0];

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

		// 3. the pool's size call: into the game - not another mod's hook - and the pool's default a plausible size; the
		// pool's name is pinned by W-E-C4
		uint8_t* const sizeCall = pool + kSizeCallAt;
		const uint8_t* const lookup = PoolSizeLookupOf(sizeCall);
		const uint32_t poolDefault = Imm32At(pool + kDefaultAt);
		if (diagnostic)
			WriteLog(kTag, "DIAGNOSTIC: pool size call at " + DescribeAddress(sizeCall) + " -> " + DescribeAddress(CallTarget(sizeCall)) + " for the pool " +
				Hex32(Imm32At(pool + kHashAt)) + ", its default at " + DescribeAddress(pool + kDefaultAt) + " holds " + std::to_string(poolDefault) +
				", the game's configuration at " + DescribeAddress(Target(pool, kConfiguration)) + " (" + kPoolPattern.label + ")");
		if (!lookup)
		{
			RelocationFailed(kSpec, "the pool size call at " + DescribeAddress(sizeCall) + " is not a call into the game (another mod may have hooked it "
				"already) (" + kPoolPattern.label + ").");
			return;
		}
		if (poolDefault == 0 || poolDefault > kMaxEntries)
		{
			RelocationFailed(kSpec, "the pool's default size at " + DescribeAddress(pool + kDefaultAt) + " holds " + std::to_string(poolDefault) +
				", which is no plausible size (" + kPoolPattern.label + ").");
			return;
		}

		// 4. one write, all or nothing: the 17 loads to the moved list and the pool's size call to PoolSize
		g_poolSize = static_cast<int32_t>(plan.size);
		const std::string callAt = DescribeAddress(sizeCall);
		const std::vector<CallRedirect> redirects = { { sizeCall, reinterpret_cast<const void*>(&PoolSize), "pool size call",
			reinterpret_cast<void**>(&g_lookup) } };
		if (!ApplyRelocation(kSpec, plan, {}, redirects, " and redirect the pool size call at " + callAt))
			return;
		if (!IsDryRun())
			WriteLog(kTag, "The pool size call at " + callAt + " is redirected: the weapon components' pool gets " + std::to_string(plan.size) +
				" slots, the list's size.");

		// 5. the measurement (DIAGNOSTIC_MODE): the list's count and the files' count, and the 17 loads checked at every reading
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
		for (size_t i = 0; i < kListLoads && i < plan.references.size(); i++)
			g_loads[i] = plan.references[i];
		WriteLog(kTag, std::string("DIAGNOSTIC: the weapon component definitions in the list and the weaponcomponents.meta files are logged after "
			"10 seconds, then every minute while the numbers change") + (IsDryRun() ? " (DRY_RUN: the game's own list is in use)." :
			", and the list's 17 loads are checked at each reading to lead to the moved list still."));
		StartMeasurement<kMeasurement>();	// one that cannot start is logged
	}
}
