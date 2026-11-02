// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the render queues of streamed peds and vehicles, measured first (see TweaksVRenderQueues.h).
//
// What a streamed ped, its props or a vehicle with mods draws in a frame the game keeps in three families of four arrays, one array
// per frame, each entry a render gfx from a pool of its family. The peds' and the props' arrays hold a constant 120 entries, and their
// add has no check and, for the peds, no dedupe: entry 121 lies past the end of a heap block, memory corruption with nothing said. The
// vehicles' arrays are as large as their render pool, and deduped. A full pool stops the game by its trap, but for the vehicles' render
// pool, whose vehicle silently loses its mods. Beside them, on the same path, the draw list's references, 10,368 a frame: past them a
// streaming reference is taken and never released (PATTERNS.md, "Render queues"). Before anything is raised this module measures
// whether a mod load comes near them:
//   DIAGNOSTIC_MODE  the measurement, and nothing is looked for without it: a thread reads the twelve arrays' counts and capacities,
//                    the six pools' and the references' every 20 ms, and at each new most of a family the number of different entries
//                    in that array, which gives the entries per ped per frame; 10 seconds in, and then every minute while a most
//                    changes, the log says how near each family has come. An array seen past its capacity is a line of the log at
//                    once, since a crash may be what comes of it, and nothing else
//   DRY_RUN          changes nothing here: the module writes nothing to begin with
// Every site its writing half will use is found and checked first, every value read at run time: each pool's three fields inside the
// game's .data, side by side, with a rotation and an addend a pool's code can hold; the adds, the inits and the flips agreeing on each
// family's arrays, index and lock; every size lookup a call into the game, all to one function. A pattern missing or a site not as
// expected is logged, not reported - the measurement is an aid (NOTES.md, T-DIAG) - and nothing is measured. Racing the game is
// harmless: every read of its memory goes through CopyMemorySafely, at worst one frame's count half done and a copy of pointers that
// are only compared, and a pool's pointer is believed only when the pool's stride is the one its code hands the pool. What stays wrong
// or unreadable for about a second, 50 samples in a row, is said once and not read further.
// Written for TweaksV from the findings of its own research in the game's code (PATTERNS.md); that these queues are a limit is the
// idea of Resource Extensions [Enhanced] by Koukol, whose plugin was not read (CREDITS.md).

#include "TweaksVRenderQueues.h"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>

#include "TweaksVConflicts.h"
#include "TweaksVLock.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "RenderQueues";
		constexpr const char* kNotMeasured = "DIAGNOSTIC: the render queues are not measured: ";

		// ---- the sites (PATTERNS.md, "Render queues")

		constexpr const GamePattern& kPedAdd = Pattern("U-E-S3");			// the streamed peds' add, from its reference on
		constexpr const GamePattern& kPedInit = Pattern("U-E-S4");			// their four arrays made, with their capacities
		constexpr const GamePattern& kPropAdd = Pattern("U-E-P3");			// the ped props' add
		constexpr const GamePattern& kPropInit = Pattern("U-E-P4");			// their four arrays made
		constexpr const GamePattern& kVehicleAdd = Pattern("U-E-V3");		// the streamed vehicles' add
		constexpr const GamePattern& kVehicleWalk = Pattern("U-E-V4");		// the head of its walk of the current array, its dedupe
		constexpr const GamePattern& kVehicleSizes = Pattern("U-E-V5");		// the vehicles' two sizes looked up and stored
		constexpr const GamePattern& kVehicleInit = Pattern("U-E-V6");		// their arrays made from the size
		constexpr const GamePattern& kFlips = Pattern("U-E-F1");			// the frame's three calls of the flips
		constexpr const GamePattern& kReferences = Pattern("U-E-D1");		// the draw list's first array of references made
		constexpr const GamePattern& kRotation = Pattern("U-E-D2");			// the draw list's arrays rotated: their index
		constexpr const GamePattern& kManager = Pattern("U-E-D3");			// the draw list manager made and stored

		// Each pool: the site that takes a slot of it, which pins it by its stride, and the lookup of its size. In the make's match each
		// field's rel32 and the two bytes of its decoding, all wildcards, since a build changes them. The names come from memory of the
		// game's gameconfig.xml, each backed by its hash at its lookup, so they are labels for the log: the code matches hashes.
		struct PoolSite
		{
			const GamePattern* make;
			const GamePattern* lookup;
			const char* name;
			size_t flagAt, keyAt, rolAt, valueAt, addAt;
			int32_t stride;			// what the make hands the pool, the proof at run time that its pointer is decoded right
			size_t lookupCallAt;	// in the lookup's match: "call rel32", its rel32 past the match
			int onlyAdd;			// the one add its code rotates by, or below 0 where it rotates by any
		};
		enum : size_t { kPedRequests, kPedRender, kPropRequests, kPropRender, kVehicleRequests, kVehicleRender, kPoolCount };
		constexpr PoolSite kPools[kPoolCount] = {
			{ &Pattern("U-E-S1"), &Pattern("U-E-N1"), "StreamPed req data", 3, 10, 17, 21, 36, 0x508, 18, 1 },
			{ &Pattern("U-E-S2"), &Pattern("U-E-N2"), "StreamPed render data", 17, 24, 31, 35, 51, 0x2E0, 18, -1 },
			{ &Pattern("U-E-P1"), &Pattern("U-E-N3"), "PedProp req data", 3, 10, 17, 21, 33, 0x80, 18, -1 },
			{ &Pattern("U-E-P2"), &Pattern("U-E-N4"), "PedProp render data", 3, 10, 17, 21, 37, 0x3C, 18, -1 },
			{ &Pattern("U-E-V1"), &Pattern("U-E-N5"), "CVehicleStreamRequestGfx", 3, 10, 17, 21, 37, 0x1C8, 15, -1 },
			{ &Pattern("U-E-V2"), &Pattern("U-E-N6"), "CVehicleStreamRenderGfx", 3, 10, 17, 21, 33, 0xBB0, 15, -1 },
		};
		consteval bool PoolsAsPinned()
		{
			for (const PoolSite& pool : kPools)
			{
				const GamePattern& make = *pool.make;
				for (const size_t at : { pool.flagAt, pool.keyAt, pool.valueAt })	// "<opcode> [rip+disp32]", its rel32 a wildcard
					if ((PatternByte(make, at - 1) & 0xC7) != 0x05 || PatternByte(make, at) != -1 || PatternByte(make, at + 3) != -1)
						return false;
				if (!PatternHolds(make, pool.flagAt - 3, { 0x0F, 0xB6 }) || !PatternHolds(make, pool.keyAt - 3, { 0x48, 0x8B }) ||
					!PatternHolds(make, pool.valueAt - 3, { 0x48, 0x8B }) || !PatternHolds(make, pool.rolAt - 3, { 0x48, 0xC1 }) ||
					(PatternByte(make, pool.rolAt - 1) & 0xF8) != 0xC0 || PatternByte(make, pool.rolAt) != -1 || PatternByte(make, pool.addAt) != -1 ||
					PatternByte(*pool.lookup, pool.lookupCallAt) != 0xE8 || PatternByte(*pool.lookup, pool.lookupCallAt + 1) != -1)
					return false;
				const bool addedByLea = PatternHolds(make, pool.addAt - 2, { 0x8D, 0x48 }), addedToCl = PatternHolds(make, pool.addAt - 2, { 0x80, 0xC1 });
				if (!addedByLea && !addedToCl)
					return false;
			}
			return PatternImm32(*kPools[kPedRequests].make, 75) == kPools[kPedRequests].stride && PatternImm32(*kPools[kPedRender].make, 75) == kPools[kPedRender].stride &&
				PatternImm32(*kPools[kPropRequests].make, 62) == kPools[kPropRequests].stride && PatternImm32(*kPools[kPropRender].make, 59) == kPools[kPropRender].stride &&
				PatternImm32(*kPools[kVehicleRequests].make, 61) == kPools[kVehicleRequests].stride;
		}
		static_assert(PoolsAsPinned(), "TweaksV: each make holds its flag, key, value, rol and add where kPools has them, and hands the pool its stride; each lookup its call");
		// U-E-S1 does not rotate with a rol by cl: it shifts left by c + add and right by c ^ 63, which is 64 - (c + add), a rotation, at an add of 1 alone.
		static_assert(PatternHolds(*kPools[kPedRequests].make, 43, { 0x34, 0x3F }) && kPools[kPedRequests].onlyAdd == 1, "TweaksV: U-E-S1 pairs c ^ 63 with c + add");

		// A pool as the game keeps it. Its count, the low 30 bits of its word, is read as the game reads it, sign-extended.
		constexpr size_t kFieldsSize = 0x18;		// its pointer's three fields: the flag at +0, the value at +8, the key at +0x10
		constexpr size_t kPoolCapacityAt = 0x18, kPoolStrideAt = 0x1C, kPoolCountAt = 0x28, kPoolRead = 0x30;
		static_assert(PatternHolds(*kPools[kPedRender].make, 0, { 0x8B, 0x42, kPoolCountAt, 0xC1, 0xE0, 0x02, 0xC1, 0xF8, 0x02, 0x39, 0x42, kPoolCapacityAt }),
			"TweaksV: U-E-S2 tests a pool's count at +0x28, its low 30 bits, against its capacity at +0x18");

		// The two jumps a full pool takes: to the trap, a CC, and in the vehicles' render pool's make to the branch that takes its mods.
		constexpr size_t kTrapJumpAt = 12, kFullJumpAt = 76;
		static_assert(PatternHolds(*kPools[kPedRender].make, kTrapJumpAt, { 0x7E, 0x61 }) && PatternHolds(*kPools[kVehicleRender].make, kFullJumpAt, { 0x7E, 0x6B }),
			"TweaksV: U-E-S2's jump at +12 and U-E-V2's at +76, each a jle rel8");

		// A family's arrays: four of 16 bytes - a pointer, the count and the capacity words - and its index and lock, the one each add takes.
		constexpr size_t kArrayCount = 4, kArraySize = 16, kArrayCountAt = 8, kArrayCapacityAt = 0xA;
		enum : size_t { kPeds, kProps, kVehicles, kFamilyCount };
		struct FamilySite
		{
			const char* what;			// for the log: "streamed peds"
			const char* summary;		// what the summary's line starts with, which the ped props' own measurement's words do not match
			size_t render, requests;	// its two pools
			const char* renderFull;		// what comes of one more in its render pool
		};
		constexpr const char* kFullStops = " - FULL: one more stops the game";
		constexpr FamilySite kFamilies[kFamilyCount] = {
			{ "streamed peds", "streamed peds", kPedRender, kPedRequests, kFullStops },
			{ "ped props", "props of peds", kPropRender, kPropRequests, kFullStops },
			{ "streamed vehicles", "streamed vehicles", kVehicleRender, kVehicleRequests, " - FULL: one more loses its mods" },
		};

		// The adds: "movsxd rax, [rip+disp32]", the index; "shl rax, 4", an array's 16 bytes; "lea rcx, [rip+disp32]", the arrays - the
		// vehicles' in r14, from the walk's lea; the count at +8.
		constexpr size_t kPedIndexAt = 11, kPedArraysAt = 22, kPropIndexAt = 8, kPropArraysAt = 19, kVehicleIndexAt = 12, kWalkLockAt = 6, kWalkIndexAt = 18, kWalkArraysAt = 29;
		static_assert(PatternHolds(kPedAdd, kPedIndexAt - 3, { 0x48, 0x63, 0x05 }) && PatternHolds(kPedAdd, 15, { 0x48, 0xC1, 0xE0, 0x04 }) &&
			PatternHolds(kPedAdd, kPedArraysAt - 3, { 0x48, 0x8D, 0x0D }) && PatternHolds(kPedAdd, 30, { 0x0F, 0xB7, 0x6C, 0x08, kArrayCountAt }) &&
			PatternHolds(kPropAdd, kPropIndexAt - 3, { 0x48, 0x63, 0x05 }) && PatternHolds(kPropAdd, 12, { 0x48, 0xC1, 0xE0, 0x04 }) &&
			PatternHolds(kPropAdd, kPropArraysAt - 3, { 0x48, 0x8D, 0x0D }) && PatternHolds(kPropAdd, 27, { 0x0F, 0xB7, 0x5C, 0x08, kArrayCountAt }) &&
			PatternHolds(kVehicleAdd, kVehicleIndexAt - 3, { 0x48, 0x63, 0x05 }) && PatternHolds(kVehicleAdd, 16, { 0x48, 0xC1, 0xE0, 0x04 }) &&
			PatternHolds(kVehicleAdd, 24, { 0x42, 0x0F, 0xB7, 0x54, 0x30, kArrayCountAt }) && PatternHolds(kVehicleWalk, kWalkLockAt - 3, { 0x48, 0x8D, 0x0D }) &&
			PatternHolds(kVehicleWalk, kWalkIndexAt - 3, { 0x48, 0x63, 0x0D }) && PatternHolds(kVehicleWalk, kWalkArraysAt - 3, { 0x4C, 0x8D, 0x35 }),
			"TweaksV: the adds and the walk hold their index, their arrays and their lock where the module reads them, an array of 16 bytes with its count at +8");

		// The peds' and the props' inits: four blocks, each "mov ecx, size", the allocator's call, the array's store, then the capacity's
		// store, which ends past its imm16: its rel32 counts from there.
		constexpr size_t kInitBlocksAt = 24, kInitBlockSize = 31, kBlockArrayAt = 18, kBlockCapacityAt = 25, kBlockEntriesAt = 29, kBlockEnd = 31;
		consteval bool InitAsPinned(const GamePattern& init)
		{
			for (size_t i = 0; i < kArrayCount; i++)
			{
				const size_t block = kInitBlocksAt + i * kInitBlockSize;
				if (!PatternHolds(init, block, { 0xB9, 0xC0, 0x03, 0x00, 0x00, 0xBA, 0x08, 0x00, 0x00, 0x00, 0xE8 }) || !PatternHolds(init, block + kBlockArrayAt - 3, { 0x48, 0x89,
					0x05 }) ||
					!PatternHolds(init, block + kBlockCapacityAt - 3, { 0x66, 0xC7, 0x05 }) || !PatternHolds(init, block + kBlockEntriesAt, { 0x78, 0x00 }))
					return false;
			}
			static_assert(0x3C0 == 0x78 * sizeof(uint64_t));
			return true;
		}
		static_assert(InitAsPinned(kPedInit) && InitAsPinned(kPropInit), "TweaksV: U-E-S4 and U-E-P4 hold four blocks of 31 bytes from +24, 120 entries of 8 bytes each");

		// The vehicles' two sizes: each lookup's call and its result's store, the request pool's first; then the arrays' size read and the
		// first array's store.
		constexpr size_t kSizeCallAt[2] = { 18, 47 }, kSizeStoreAt[2] = { 25, 54 }, kInitSizeAt = 3, kInitArrayAt = 44;
		static_assert(PatternByte(kVehicleSizes, kSizeCallAt[0]) == 0xE8 && PatternByte(kVehicleSizes, kSizeCallAt[1]) == 0xE8 && PatternHolds(kVehicleSizes,
			kSizeStoreAt[0] - 2, { 0x89, 0x05 }) &&
			PatternHolds(kVehicleSizes, kSizeStoreAt[1] - 2, { 0x89, 0x05 }) && PatternHolds(kVehicleInit, kInitSizeAt - 3, { 0x48, 0x63, 0x35 }) &&
			PatternHolds(kVehicleInit, kInitArrayAt - 3, { 0x48, 0x89, 0x05 }),
			"TweaksV: U-E-V5 holds its two calls and stores and U-E-V6 its size's load and the first array's store where the module reads them");

		// The frame's three calls of the flips. Each flip starts its work 6 or 7 bytes on: its lock taken, its index read and 1 added modulo 4.
		constexpr size_t kFlipCallsAt[kFamilyCount] = { 11, 16, 21 };
		static_assert(PatternByte(kFlips, kFlipCallsAt[0]) == 0xE8 && PatternByte(kFlips, kFlipCallsAt[1]) == 0xE8 && PatternByte(kFlips, kFlipCallsAt[2]) == 0xE8,
			"TweaksV: U-E-F1 holds the flips' three calls");
		constexpr int kFlipHead[] = { 0x48, 0x8D, 0x0D, -1, -1, -1, -1, 0xE8, -1, -1, -1, -1, 0x8B, 0x05, -1, -1, -1, -1, 0x8D, 0x48, 0x01, 0x8D, 0x50, 0x04, 0x85, 0xC9,
			0x0F, 0x49, 0xD1, 0x83, 0xE2, 0xFC, 0xF7, 0xDA, 0x44, 0x8D, 0x04, 0x10, 0x41, 0xFF, 0xC0 };
		constexpr size_t kFlipHeadSize = sizeof(kFlipHead) / sizeof(kFlipHead[0]), kFlipLockAt = 3, kFlipIndexAt = 14;

		// The draw list: its manager, and in it the four arrays of references, each an array of the same 16 bytes; their index, which the
		// measurement does not read, since it reads all four.
		constexpr uint32_t kManagerBytes = 0x14BA0;
		constexpr size_t kReferencesAt = 0x5D0, kReferencesIndexAt = 0xAC, kManagerStoreAt = 21, kManagerAt = 24, kReferencesEntriesAt = 14;
		static_assert(PatternHolds(kReferences, 0, { 0x48, 0x89, 0x86, kReferencesAt & 0xFF, kReferencesAt >> 8, 0, 0 }) &&
			PatternHolds(kReferences, 7, { 0x66, 0xC7, 0x86, (kReferencesAt + kArrayCapacityAt) & 0xFF, (kReferencesAt + kArrayCapacityAt) >> 8, 0, 0 }) &&
			PatternHolds(kRotation, 6, { 0x8B, 0x91, kReferencesIndexAt, 0, 0, 0 }) && PatternImm32(kManager, 1) == kManagerBytes && PatternHolds(kManager, kManagerStoreAt,
			{ 0x48, 0x89, 0x35 }) &&
			kReferencesAt + kArrayCount * kArraySize <= kManagerBytes,
			"TweaksV: U-E-D1 stores the first array of references at +0x5D0, its capacity at +0x5DA; U-E-D2 its index at +0xAC; U-E-D3 the manager of 0x14BA0 bytes");

		// Every site the measurement reads, found and checked.
		struct Sites
		{
			const uint8_t* fields[kPoolCount] = {};		// each pool's three fields
			int rol[kPoolCount] = {};					// and the two bytes of its decoding
			int add[kPoolCount] = {};
			const uint8_t* arrays[kFamilyCount] = {};	// each family's first array
			const uint8_t* index[kFamilyCount] = {};
			const uint8_t* lock[kFamilyCount] = {};
			const uint8_t* manager = nullptr;			// the draw list manager's global
		};

		// True if the size bytes at at are the game's data, aligned, so that a word of it reads whole, and can be read; else why not.
		bool GameData(const std::string& what, const uint8_t* at, size_t size, size_t alignment, const GamePattern& pattern, std::string& why)
		{
			const std::string fault = DataFault(at, size, alignment, false, IsInsideGameData, "the game's data");
			if (fault.empty())
				return true;
			why = "the " + what + " at " + DescribeAddress(at) + At(pattern) + " " + fault + ".";
			return false;
		}

		// A pool's make: the two bytes of its decoding as a pool's code can hold them, and its three fields side by side in the game's data.
		bool CheckPool(size_t i, const uint8_t* match, Sites& sites, std::string& why)
		{
			const PoolSite& pool = kPools[i];
			const std::string name = std::string("pool of ") + pool.name;
			const int rol = match[pool.rolAt], add = match[pool.addAt];
			if (rol < 1 || rol > 63)
			{
				why = "the " + name + "'s rol at " + DescribeAddress(match + pool.rolAt) + " holds " + std::to_string(rol) + ", not 1 to 63" + At(*pool.make) + ".";
				return false;
			}
			if (add > 63)
			{
				why = "the " + name + "'s add at " + DescribeAddress(match + pool.addAt) + " holds " + std::to_string(add) + ", not 0 to 63" + At(*pool.make) + ".";
				return false;
			}
			if (pool.onlyAdd >= 0 && add != pool.onlyAdd)
			{
				why = "the " + name + "'s add at " + DescribeAddress(match + pool.addAt) + " holds " + std::to_string(add) + ", not " + std::to_string(pool.onlyAdd) +
					", the one its code rotates by" +
					At(*pool.make) + ".";
				return false;
			}
			const uint8_t* const flag = Rel32Target(match + pool.flagAt);
			const uint8_t* const value = Rel32Target(match + pool.valueAt);
			const uint8_t* const key = Rel32Target(match + pool.keyAt);
			if (value != flag + 0x08 || key != flag + 0x10)
			{
				why = "the " + name + "'s flag at " + DescribeAddress(flag) + ", value at " + DescribeAddress(value) + " and key at " + DescribeAddress(key) +
					" do not lie 8 bytes apart" + At(*pool.make) + ".";
				return false;
			}
			if (!GameData(name + "'s fields", flag, kFieldsSize, sizeof(uint64_t), *pool.make, why))
				return false;
			sites.fields[i] = flag;
			sites.rol[i] = rol;
			sites.add[i] = add;
			WriteLog(kTag, "DIAGNOSTIC: " + name + " at " + DescribeAddress(match) + ": its flag, value and key at " + DescribeAddress(flag) + ", rol " + Hex(static_cast<uint64_t>(rol)) +
				", add " + Hex(static_cast<uint64_t>(add)) + ", stride " + Hex(static_cast<uint64_t>(pool.stride)) + At(*pool.make));
			return true;
		}

		// The target of a jle rel8 at at.
		const uint8_t* Rel8Target(const uint8_t* at)
		{
			return at + 2 + static_cast<int8_t>(at[1]);
		}

		// The two jumps of a full pool: the streamed peds' render pool's to the trap, a CC past the match, the vehicles' inside its function
		// by the game's unwind data.
		bool CheckFullJumps(const uint8_t* pedRender, const uint8_t* vehicleRender, std::string& why)
		{
			const GamePattern& pedMake = *kPools[kPedRender].make;
			const uint8_t* const trapJump = pedRender + kTrapJumpAt;
			const uint8_t* const trap = Rel8Target(trapJump);
			const std::string pedJump = std::string("the ") + kPools[kPedRender].name + "'s full pool's jump at " + DescribeAddress(trapJump) + " leads to " + DescribeAddress(trap);
			if (!IsReadable(trap, 1))
			{
				why = pedJump + ", which cannot be read" + At(pedMake) + ".";
				return false;
			}
			if (*trap != 0xCC)
			{
				why = pedJump + ", which holds " + HexBytes(trap, 1) + ", not CC" + At(pedMake) + ".";
				return false;
			}
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kPools[kPedRender].name + "'s full pool's jump at " + DescribeAddress(trapJump) + " -> " + DescribeAddress(trap) +
				", the trap: CC" + At(pedMake));

			const GamePattern& vehicleMake = *kPools[kVehicleRender].make;
			const uint8_t* const fullJump = vehicleRender + kFullJumpAt;
			const uint8_t* const branch = Rel8Target(fullJump);
			const uint8_t* const function = FunctionStartOf(fullJump);
			const std::string vehicleJump = std::string("the ") + kPools[kVehicleRender].name + "'s full pool's jump at " + DescribeAddress(fullJump) + " leads to " +
				DescribeAddress(branch);
			if (!function)
			{
				why = vehicleJump + ", and the game's unwind data names no function there" + At(vehicleMake) + ".";
				return false;
			}
			if (FunctionStartOf(branch) != function)
			{
				why = vehicleJump + ", outside its function at " + DescribeAddress(function) + " (unwind data)" + At(vehicleMake) + ".";
				return false;
			}
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kPools[kVehicleRender].name + "'s full pool's jump at " + DescribeAddress(fullJump) + " -> " + DescribeAddress(branch) +
				", in its function at " + DescribeAddress(function) + " (unwind data)" + At(vehicleMake));
			return true;
		}

		// A family's four arrays in the game's data.
		bool CheckArrays(size_t family, const uint8_t* arrays, const GamePattern& pattern, Sites& sites, std::string& why)
		{
			if (!GameData(std::string(kFamilies[family].what) + "' arrays", arrays, kArrayCount * kArraySize, sizeof(uint64_t), pattern, why))
				return false;
			sites.arrays[family] = arrays;
			return true;
		}

		// The peds' or the props' init: its four arrays, 16 bytes apart from the ones its add loads, each capacity at its array's +0xA.
		bool CheckInit(size_t family, const uint8_t* match, const GamePattern& add, const GamePattern& init, const Sites& sites, std::string& why)
		{
			const char* const what = kFamilies[family].what;
			for (size_t i = 0; i < kArrayCount; i++)
			{
				const uint8_t* const block = match + kInitBlocksAt + i * kInitBlockSize;
				const uint8_t* const array = Rel32Target(block + kBlockArrayAt);
				const uint8_t* const capacity = block + kBlockEnd + ValueAt<int32_t>(block, kBlockCapacityAt);
				if (array != sites.arrays[family] + i * kArraySize)
				{
					why = i == 0 ?
						std::string("the ") + what + "' add loads their arrays at " + DescribeAddress(sites.arrays[family]) + At(add) + ", their init stores them at " +
						DescribeAddress(array) + At(init) + "." :
						std::string("the ") + what + "' array " + std::to_string(i) + " at " + DescribeAddress(array) + " does not lie " + std::to_string(i * kArraySize) +
						" bytes past their first at " +
							DescribeAddress(sites.arrays[family]) + At(init) + ".";
					return false;
				}
				if (capacity != array + kArrayCapacityAt)
				{
					why = std::string("the ") + what + "' array " + std::to_string(i) + " at " + DescribeAddress(array) + " has its capacity at " + DescribeAddress(capacity) +
						", not at its +0xA" + At(init) + ".";
					return false;
				}
			}
			const uint8_t* const firstBlock = match + kInitBlocksAt;
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + what + "' init at " + DescribeAddress(match) + ": 4 arrays from " + DescribeAddress(sites.arrays[family]) + ", 16 bytes apart, " +
				std::to_string(ValueAt<uint16_t>(firstBlock, kBlockEntriesAt)) + " entries each, their capacities at their +0xA" + At(init));
			return true;
		}

		// The adds and the inits: each family's arrays, in the game's data, as its add loads them and its init stores them, and the vehicles'
		// size as its init reads it where its second lookup stored it. A family's lock lies 8 bytes past its index, as the research found -
		// the walk names the vehicles', the flips all three, and what lies there is only compared.
		bool CheckQueues(const uint8_t* pedAdd, const uint8_t* pedInit, const uint8_t* propAdd, const uint8_t* propInit, const uint8_t* vehicleAdd, const uint8_t* walk,
			const uint8_t* sizes,
			const uint8_t* vehicleInit, Sites& sites, std::string& why)
		{
			const auto logAdd = [](size_t family, const uint8_t* match, const GamePattern& pattern, const Sites& found)
			{
				WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[family].what + "' add at " + DescribeAddress(match) + ": its index at " +
					DescribeAddress(found.index[family]) + ", its arrays at " +
					DescribeAddress(found.arrays[family]) + At(pattern));
			};

			sites.index[kPeds] = Rel32Target(pedAdd + kPedIndexAt);
			sites.lock[kPeds] = sites.index[kPeds] + sizeof(uint64_t);
			if (!CheckArrays(kPeds, Rel32Target(pedAdd + kPedArraysAt), kPedAdd, sites, why))
				return false;
			logAdd(kPeds, pedAdd, kPedAdd, sites);
			if (!CheckInit(kPeds, pedInit, kPedAdd, kPedInit, sites, why))
				return false;

			sites.index[kProps] = Rel32Target(propAdd + kPropIndexAt);
			sites.lock[kProps] = sites.index[kProps] + sizeof(uint64_t);
			if (!CheckArrays(kProps, Rel32Target(propAdd + kPropArraysAt), kPropAdd, sites, why))
				return false;
			logAdd(kProps, propAdd, kPropAdd, sites);
			if (!CheckInit(kProps, propInit, kPropAdd, kPropInit, sites, why))
				return false;

			const uint8_t* const vehicleIndex = Rel32Target(vehicleAdd + kVehicleIndexAt);
			sites.index[kVehicles] = Rel32Target(walk + kWalkIndexAt);
			sites.lock[kVehicles] = Rel32Target(walk + kWalkLockAt);
			if (vehicleIndex != sites.index[kVehicles])
			{
				why = std::string("the ") + kFamilies[kVehicles].what + "' add reads the index at " + DescribeAddress(vehicleIndex) + At(kVehicleAdd) + ", their walk at " +
					DescribeAddress(sites.index[kVehicles]) + At(kVehicleWalk) + ".";
				return false;
			}
			if (!CheckArrays(kVehicles, Rel32Target(walk + kWalkArraysAt), kVehicleWalk, sites, why))
				return false;
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[kVehicles].what + "' add at " + DescribeAddress(vehicleAdd) + ": its index at " +
				DescribeAddress(vehicleIndex) + At(kVehicleAdd));
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[kVehicles].what + "' walk at " + DescribeAddress(walk) + ": its lock at " +
				DescribeAddress(sites.lock[kVehicles]) + ", its index at " +
				DescribeAddress(sites.index[kVehicles]) + ", its arrays at " + DescribeAddress(sites.arrays[kVehicles]) + At(kVehicleWalk));
			const uint8_t* const firstArray = Rel32Target(vehicleInit + kInitArrayAt);
			if (firstArray != sites.arrays[kVehicles])
			{
				why = std::string("the ") + kFamilies[kVehicles].what + "' walk loads their arrays at " + DescribeAddress(sites.arrays[kVehicles]) + At(kVehicleWalk) +
					", their init stores the first at " +
					DescribeAddress(firstArray) + At(kVehicleInit) + ".";
				return false;
			}
			const uint8_t* const requestsSize = Rel32Target(sizes + kSizeStoreAt[0]);
			const uint8_t* const renderSize = Rel32Target(sizes + kSizeStoreAt[1]);
			const uint8_t* const initSize = Rel32Target(vehicleInit + kInitSizeAt);
			if (renderSize != initSize)
			{
				why = std::string("the ") + kFamilies[kVehicles].what + "' second size is stored at " + DescribeAddress(renderSize) + At(kVehicleSizes) +
					", their init reads their arrays' size at " +
					DescribeAddress(initSize) + At(kVehicleInit) + ".";
				return false;
			}
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[kVehicles].what + "' two sizes at " + DescribeAddress(sizes) + ": stored at " + DescribeAddress(requestsSize) +
				" and " +
				DescribeAddress(renderSize) + At(kVehicleSizes));
			WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[kVehicles].what + "' init at " + DescribeAddress(vehicleInit) + ": their arrays' size read at " +
				DescribeAddress(initSize) +
				", the first at " + DescribeAddress(firstArray) + At(kVehicleInit));
			return true;
		}

		// A call of the size lookup: readable - its rel32 may lie past the match - into the game, and to the same function as the others.
		bool CheckLookup(const std::string& name, const uint8_t* call, const GamePattern& pattern, const uint8_t*& lookup, std::string& why)
		{
			const std::string line = "the size lookup call of " + name + " at " + DescribeAddress(call);
			if (!IsReadable(call, 5))
			{
				why = line + " cannot be read" + At(pattern) + ".";
				return false;
			}
			const uint8_t* const target = CallTarget(call);
			if (!IsInsideGameImage(target))
			{
				why = line + " is not a call into the game (another mod may have hooked it already)" + At(pattern) + ".";
				return false;
			}
			if (lookup && target != lookup)
			{
				why = line + " leads to " + DescribeAddress(target) + ", the ones before it to " + DescribeAddress(lookup) + At(pattern) + ".";
				return false;
			}
			lookup = target;
			WriteLog(kTag, "DIAGNOSTIC: size lookup call of " + name + " at " + DescribeAddress(call) + " -> " + DescribeAddress(target) + At(pattern));
			return true;
		}

		// True if the bytes at at are a flip's head; the caller has checked that they can be read.
		bool IsFlipHead(const uint8_t* at)
		{
			for (size_t i = 0; i < kFlipHeadSize; i++)
				if (kFlipHead[i] >= 0 && at[i] != kFlipHead[i])
					return false;
			return true;
		}

		// The frame's three calls: each into the game, to a flip with the head the research found, each of the three families' by its index,
		// which the family's add reads, and its lock. In what order the frame calls them no pattern says, so each is told by its index.
		bool CheckFlips(const uint8_t* match, const Sites& sites, std::string& why)
		{
			bool found[kFamilyCount] = {};
			for (const size_t callAt : kFlipCallsAt)
			{
				const uint8_t* const call = match + callAt;
				const uint8_t* const flip = CallTarget(call);
				const std::string line = "the flip call at " + DescribeAddress(call);
				if (!IsInsideGameImage(flip))
				{
					why = line + " is not a call into the game (another mod may have hooked it already)" + At(kFlips) + ".";
					return false;
				}
				if (!IsReadable(flip, 7 + kFlipHeadSize))
				{
					why = line + " leads to " + DescribeAddress(flip) + ", which cannot be read" + At(kFlips) + ".";
					return false;
				}
				const uint8_t* const head = IsFlipHead(flip + 6) ? flip + 6 : IsFlipHead(flip + 7) ? flip + 7 : nullptr;
				if (!head)
				{
					why = line + " leads to " + DescribeAddress(flip) + ", which does not hold a flip's head 6 or 7 bytes on" + At(kFlips) + ".";
					return false;
				}
				const uint8_t* const lock = Rel32Target(head + kFlipLockAt);
				const uint8_t* const index = Rel32Target(head + kFlipIndexAt);
				size_t family = 0;
				while (family < kFamilyCount && sites.index[family] != index)
					family++;
				if (family == kFamilyCount)
				{
					why = "the flip at " + DescribeAddress(flip) + " reads the index at " + DescribeAddress(index) + ", which no add reads" + At(kFlips) + ".";
					return false;
				}
				const std::string what = std::string("the ") + kFamilies[family].what + "'";
				if (found[family])
				{
					why = "two of the frame's calls lead to flips of " + what + " index at " + DescribeAddress(index) + At(kFlips) + ".";
					return false;
				}
				found[family] = true;
				if (lock != sites.lock[family])
				{
					why = what + " flip at " + DescribeAddress(flip) + " takes the lock at " + DescribeAddress(lock) + ", not at " + DescribeAddress(sites.lock[family]) + At(kFlips) + ".";
					return false;
				}
				WriteLog(kTag, std::string("DIAGNOSTIC: ") + kFamilies[family].what + "' flip, called at " + DescribeAddress(call) + ", at " + DescribeAddress(flip) +
					": its head at " + DescribeAddress(head) +
					", its lock at " + DescribeAddress(lock) + ", its index at " + DescribeAddress(index) + At(kFlips));
			}
			return true;
		}

		// The draw list: what its patterns pin, and the manager's global in the game's data.
		bool CheckDrawList(const uint8_t* references, const uint8_t* rotation, const uint8_t* manager, Sites& sites, std::string& why)
		{
			const uint8_t* const global = Rel32Target(manager + kManagerAt);
			if (!GameData("draw list manager's global", global, sizeof(uint64_t), sizeof(uint64_t), kManager, why))
				return false;
			sites.manager = global;
			WriteLog(kTag, "DIAGNOSTIC: the draw list's references at " + DescribeAddress(references) + ": the first array at " + Hex(kReferencesAt) +
				" of its manager, its capacity holds " +
				std::to_string(ValueAt<uint16_t>(references, kReferencesEntriesAt)) + At(kReferences));
			WriteLog(kTag, "DIAGNOSTIC: the draw list's rotation at " + DescribeAddress(rotation) + ": the references' index at " + Hex(kReferencesIndexAt) + " of its manager" +
				At(kRotation));
			WriteLog(kTag, "DIAGNOSTIC: the draw list manager made at " + DescribeAddress(manager) + ", " + Hex(kManagerBytes) + " bytes, its global at " +
				DescribeAddress(global) + At(kManager));
			return true;
		}

		// Every pattern looked for, each once - all of them, so that the log names each one missing - and every site checked; false at the
		// first that is not as the research found it, and why.
		bool FindAndCheck(Sites& sites, std::string& why)
		{
			uint8_t* makes[kPoolCount] = {};
			uint8_t* lookups[kPoolCount] = {};
			for (size_t i = 0; i < kPoolCount; i++)
			{
				makes[i] = Scanner::FindOnce(*kPools[i].make, why);
				lookups[i] = Scanner::FindOnce(*kPools[i].lookup, why);
			}
			uint8_t* const pedAdd = Scanner::FindOnce(kPedAdd, why);
			uint8_t* const pedInit = Scanner::FindOnce(kPedInit, why);
			uint8_t* const propAdd = Scanner::FindOnce(kPropAdd, why);
			uint8_t* const propInit = Scanner::FindOnce(kPropInit, why);
			uint8_t* const vehicleAdd = Scanner::FindOnce(kVehicleAdd, why);
			uint8_t* const walk = Scanner::FindOnce(kVehicleWalk, why);
			uint8_t* const sizes = Scanner::FindOnce(kVehicleSizes, why);
			uint8_t* const vehicleInit = Scanner::FindOnce(kVehicleInit, why);
			uint8_t* const flips = Scanner::FindOnce(kFlips, why);
			uint8_t* const references = Scanner::FindOnce(kReferences, why);
			uint8_t* const rotation = Scanner::FindOnce(kRotation, why);
			uint8_t* const manager = Scanner::FindOnce(kManager, why);
			if (!why.empty())	// FindOnce's words for the first missing
			{
				why += ".";
				return false;
			}

			for (size_t i = 0; i < kPoolCount; i++)
				if (!CheckPool(i, makes[i], sites, why))
					return false;
			if (!CheckFullJumps(makes[kPedRender], makes[kVehicleRender], why) ||
				!CheckQueues(pedAdd, pedInit, propAdd, propInit, vehicleAdd, walk, sizes, vehicleInit, sites, why))
				return false;

			const uint8_t* lookup = nullptr;	// the vehicles' two sizes' calls and each pool's lead to one function, the game's size lookup
			if (!CheckLookup("VehicleStreamRequest", sizes + kSizeCallAt[0], kVehicleSizes, lookup, why) || !CheckLookup("VehicleStreamRender", sizes + kSizeCallAt[1],
				kVehicleSizes, lookup, why))
				return false;
			for (size_t i = 0; i < kPoolCount; i++)
				if (!CheckLookup(kPools[i].name, lookups[i] + kPools[i].lookupCallAt, *kPools[i].lookup, lookup, why))
					return false;
			return CheckFlips(flips, sites, why) && CheckDrawList(references, rotation, manager, sites, why);
		}

		// ---- the measurement

		constexpr DWORD kSampleMs = 20;				// three of a family's four arrays hold a whole past frame: this sees every frame up to about 150 fps
		constexpr uint32_t kMaxEntriesRead = 4096;	// of an array at a new most: the different ones are counted among these
		// Samples in a row, about a second: what stays wrong that long is no race with the game, and each read that fails is an access
		// violation through every handler in the process, so that a pointer that leads nowhere is not retried for good.
		constexpr uint32_t kGiveUpAfter = 50;
		constexpr const char* kNotReadFurther = "not read further (see above)";
		constexpr const char* kPastCapacity = " - PAST ITS CAPACITY: the game wrote past the array's end, a crash may follow";

		enum class State : uint8_t { NotMade, Read, NotReadFurther };

		struct PoolReading
		{
			State state = State::NotMade;
			uint32_t wrong = 0;		// samples in a row not read, or with another stride than its code hands it
			int32_t capacity = 0;
			int32_t latest = 0;
			int32_t most = 0;
		};

		// A family's arrays: the most entries any of them has held at a sample, the capacity of that array - of the first until then, 0 until
		// the game has made them - how many of those entries differ, and whether any array has been seen past its capacity.
		struct ArraysReading
		{
			uint32_t most = 0;
			uint32_t capacity = 0;
			uint32_t different = 0;
			uint32_t wrong = 0;		// samples in a row with an array not read
			bool entriesRead = false;
			bool past = false;
			bool notReadFurther = false;
		};

		struct ReferencesReading
		{
			State state = State::NotMade;
			uint32_t wrong = 0;		// samples in a row not read
			uint32_t most = 0;
			uint32_t capacity = 0;
		};

		struct Reading
		{
			PoolReading pools[kPoolCount];
			ArraysReading arrays[kFamilyCount];
			ReferencesReading references;
		};

		// True if a summary would have nothing new to say: a most, a capacity or a state is new, the latest numbers alone are not.
		bool SameMosts(const Reading& a, const Reading& b)
		{
			for (size_t i = 0; i < kPoolCount; i++)
				if (a.pools[i].state != b.pools[i].state || a.pools[i].capacity != b.pools[i].capacity || a.pools[i].most != b.pools[i].most)
					return false;
			for (size_t i = 0; i < kFamilyCount; i++)
				if (a.arrays[i].most != b.arrays[i].most || a.arrays[i].capacity != b.arrays[i].capacity || a.arrays[i].different != b.arrays[i].different ||
					a.arrays[i].entriesRead != b.arrays[i].entriesRead || a.arrays[i].past != b.arrays[i].past || a.arrays[i].notReadFurther != b.arrays[i].notReadFurther)
					return false;
			return a.references.state == b.references.state && a.references.most == b.references.most && a.references.capacity == b.references.capacity;
		}

		Sites g_sites;							// set before the threads start
		std::atomic<bool> g_stopSampling{ false };	// no summary's thread came to read for the sampling thread
		SRWLOCK g_readingLock = SRWLOCK_INIT;
		Reading g_published;					// what the sampling thread last read, under the lock
		Reading g_sampled;						// the sampling thread's own
		uint64_t g_entries[kMaxEntriesRead];	// the sampling thread's own: an array's entries, to be sorted
		Reading g_logged;						// the summary's thread's own: what the last summary said
		uint64_t g_news = 0;

		// True at the sample that makes kGiveUpAfter in a row not read as they should be: what they read is then read no further, said once.
		bool GiveUp(uint32_t& wrong)
		{
			return ++wrong >= kGiveUpAfter;
		}

		void StopPool(PoolReading& reading, const std::string& what)
		{
			reading.state = State::NotReadFurther;
			WriteLog(kTag, "DIAGNOSTIC: the pool of " + what + ": it is not read further.");
		}

		// One pool's numbers. A flag that is clear: not made yet, or gone. A read that fails, or another stride than its code hands the pool,
		// which says that the pointer is not the pool's, or not yet or no more - the game's allocator need not zero, and a teardown may
		// come before the flag clears - is believed in nothing: kGiveUpAfter of them in a row end the pool's reading.
		void SamplePool(size_t i)
		{
			PoolReading& reading = g_sampled.pools[i];
			if (reading.state == State::NotReadFurther)
				return;
			const PoolSite& pool = kPools[i];
			struct Fields
			{
				uint64_t flag;		// its low byte
				uint64_t value;
				uint64_t key;
			} fields;
			static_assert(sizeof(Fields) == kFieldsSize);
			if (!CopyMemorySafely(&fields, g_sites.fields[i], sizeof(fields)))
			{
				if (GiveUp(reading.wrong))
					StopPool(reading, std::string(pool.name) + "'s fields at " + DescribeAddress(g_sites.fields[i]) + " cannot be read");
				return;
			}
			const uint64_t object = DecodePoolPointer(static_cast<uint8_t>(fields.flag), fields.value, fields.key, g_sites.rol[i], g_sites.add[i]);
			if (!object)
			{
				reading.latest = 0;
				reading.wrong = 0;
				return;
			}
			const uint8_t* const at = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(object));
			uint8_t bytes[kPoolRead];
			if (!CopyMemorySafely(bytes, at, sizeof(bytes)))
			{
				if (GiveUp(reading.wrong))
					StopPool(reading, std::string(pool.name) + " at " + DescribeAddress(at) + " cannot be read");
				return;
			}
			const int32_t stride = ValueAt<int32_t>(bytes, kPoolStrideAt);
			if (stride != pool.stride)
			{
				if (GiveUp(reading.wrong))
					StopPool(reading, std::string(pool.name) + " at " + DescribeAddress(at) + " has stride " + std::to_string(stride) + ", not " + std::to_string(pool.stride));
				return;
			}
			reading.wrong = 0;
			const int32_t count = static_cast<int32_t>(ValueAt<uint32_t>(bytes, kPoolCountAt) << 2) >> 2;
			reading.state = State::Read;
			reading.capacity = ValueAt<int32_t>(bytes, kPoolCapacityAt);
			reading.latest = count > 0 ? count : 0;
			if (reading.latest > reading.most)
				reading.most = reading.latest;
		}

		// How many of an array's entries differ, the first kMaxEntriesRead of them, those not yet written, which the fresh array holds as 0,
		// left out. Rare by nature: a new most. A sort in the thread's buffer, and the entries only compared.
		void CountDifferent(const uint8_t* array, uint32_t count, ArraysReading& reading)
		{
			const uint32_t entries = std::min(count, kMaxEntriesRead);
			uint64_t pointer = 0;
			reading.entriesRead = CopyMemorySafely(&pointer, array, sizeof(pointer)) && pointer != 0 &&
				CopyMemorySafely(g_entries, reinterpret_cast<const void*>(static_cast<uintptr_t>(pointer)), entries * sizeof(uint64_t));
			if (!reading.entriesRead)
				return;
			std::sort(g_entries, g_entries + entries);
			uint32_t different = 0;
			for (uint32_t i = 0; i < entries; i++)
				if (g_entries[i] != 0 && (i == 0 || g_entries[i] != g_entries[i - 1]))
					different++;
			reading.different = different;
		}

		// A family's arrays. The first time one is seen past its capacity it is said at once, a line of the log and no box (D30), since what
		// that does may be a crash long before the next summary, which would leave the log's last word on this family a false all-clear.
		void SampleArrays(size_t family)
		{
			ArraysReading& reading = g_sampled.arrays[family];
			if (reading.notReadFurther)
				return;
			const FamilySite& site = kFamilies[family];
			bool allRead = true;
			for (size_t i = 0; i < kArrayCount; i++)
			{
				const uint8_t* const array = g_sites.arrays[family] + i * kArraySize;
				uint16_t words[2] = {};		// the count and the capacity, side by side
				static_assert(kArrayCapacityAt == kArrayCountAt + sizeof(uint16_t));
				if (!CopyMemorySafely(words, array + kArrayCountAt, sizeof(words)))
				{
					allRead = false;
					continue;
				}
				if (reading.most == 0)
					reading.capacity = words[1];
				if (words[0] > words[1] && !reading.past)
				{
					reading.past = true;
					WriteLog(kTag, std::string("DIAGNOSTIC: ") + site.summary + ": a frame's array at " + std::to_string(words[0]) + " entries of " + std::to_string(words[1]) +
						kPastCapacity + ".");
				}
				if (words[0] > reading.most)
				{
					reading.most = words[0];
					reading.capacity = words[1];
					CountDifferent(array, words[0], reading);
				}
			}
			if (allRead)
				reading.wrong = 0;
			else if (GiveUp(reading.wrong))
			{
				reading.notReadFurther = true;
				WriteLog(kTag, std::string("DIAGNOSTIC: the ") + site.what + "' arrays at " + DescribeAddress(g_sites.arrays[family]) + " cannot be read: they are not read further.");
			}
		}

		void SampleReferences()
		{
			ReferencesReading& reading = g_sampled.references;
			if (reading.state == State::NotReadFurther)
				return;
			uint64_t manager = 0;
			if (!CopyMemorySafely(&manager, g_sites.manager, sizeof(manager)))
			{
				if (GiveUp(reading.wrong))
				{
					reading.state = State::NotReadFurther;
					WriteLog(kTag, "DIAGNOSTIC: the draw list manager's global at " + DescribeAddress(g_sites.manager) + " cannot be read: the references are not read further.");
				}
				return;
			}
			if (!manager)
			{
				reading.wrong = 0;
				return;
			}
			const uint8_t* const at = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(manager)) + kReferencesAt;
			uint8_t arrays[kArrayCount * kArraySize];
			if (!CopyMemorySafely(arrays, at, sizeof(arrays)))
			{
				if (GiveUp(reading.wrong))
				{
					reading.state = State::NotReadFurther;
					WriteLog(kTag, "DIAGNOSTIC: the draw list's references at " + DescribeAddress(at) + " cannot be read: they are not read further.");
				}
				return;
			}
			reading.wrong = 0;
			reading.state = State::Read;
			for (size_t i = 0; i < kArrayCount; i++)
			{
				reading.most = std::max<uint32_t>(reading.most, ValueAt<uint16_t>(arrays, i * kArraySize + kArrayCountAt));
				reading.capacity = std::max<uint32_t>(reading.capacity, ValueAt<uint16_t>(arrays, i * kArraySize + kArrayCapacityAt));
			}
		}

		// The sampling thread: every array, pool and the references every 20 ms, each pass inside a try, so nothing leaves the thread. It ends
		// only when no summary's thread came to read for it.
		DWORD WINAPI SampleQueues(LPVOID module)
		{
			while (!g_stopSampling.load())
			{
				Sleep(kSampleMs);
				try
				{
					for (size_t i = 0; i < kPoolCount; i++)
						SamplePool(i);
					for (size_t i = 0; i < kFamilyCount; i++)
						SampleArrays(i);
					SampleReferences();
					ExclusiveLock lock(g_readingLock);
					g_published = g_sampled;
				}
				catch (...)
				{
				}
			}
			FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
		}

		// Changes with every reading that has something new to say: the first, a new most, a capacity, a pool made.
		uint64_t Numbers()
		{
			Reading now;
			{
				SharedLock lock(g_readingLock);
				now = g_published;
			}
			if (g_news == 0 || !SameMosts(now, g_logged))
			{
				g_logged = now;
				g_news++;
			}
			return g_news;
		}

		// "render data 14 in use of 120, at most 31", "requests 0 of 55, at most 9", "render data not made yet",
		// "requests not read further (see above), until then at most 9 of 55"
		std::string DescribePool(const char* label, const char* inUse, const PoolReading& pool, const char* full)
		{
			const std::string text = std::string(label) + " ";
			const std::string capacity = std::to_string(pool.capacity), most = std::to_string(pool.most);
			const char* const isFull = pool.capacity > 0 && pool.most >= pool.capacity ? full : "";
			switch (pool.state)
			{
			case State::NotMade:
				return text + "not made yet";
			case State::NotReadFurther:
				return text + kNotReadFurther + (pool.capacity > 0 ? ", until then at most " + most + " of " + capacity + isFull : std::string());
			case State::Read:
				break;
			}
			return text + std::to_string(pool.latest) + inUse + " of " + capacity + ", at most " + most + isFull;
		}

		// "a frame's array at most 63 entries of 120, 21 different in that frame"
		std::string DescribeArrays(const ArraysReading& arrays)
		{
			const bool nothingYet = arrays.capacity == 0 && arrays.most == 0;
			const std::string text = "a frame's array ";
			if (nothingYet)
				return text + (arrays.notReadFurther ? kNotReadFurther : "not made yet");
			std::string numbers = "at most " + std::to_string(arrays.most) + " entries of " + std::to_string(arrays.capacity);
			if (arrays.most != 0 && !arrays.entriesRead)
				numbers += ", the entries could not be read";
			else if (arrays.most != 0)
				numbers += ", " + std::to_string(arrays.different) + " different " + (arrays.most > kMaxEntriesRead ? "among the first " + std::to_string(kMaxEntriesRead) + " " :
					std::string()) + "in that frame";
			if (arrays.past)
				numbers += kPastCapacity;
			return text + (arrays.notReadFurther ? std::string(kNotReadFurther) + ", until then " : std::string()) + numbers;
		}

		void LogSummary(uint64_t)
		{
			const Reading& now = g_logged;
			for (size_t i = 0; i < kFamilyCount; i++)
			{
				const FamilySite& family = kFamilies[i];
				WriteLog(kTag, std::string("DIAGNOSTIC: ") + family.summary + ": " + DescribePool("render data", " in use", now.pools[family.render], family.renderFull) + "; " +
					DescribePool("requests", "", now.pools[family.requests], kFullStops) + "; " + DescribeArrays(now.arrays[i]) + ".");
			}
			const ReferencesReading& references = now.references;
			const std::string numbers = "at most " + std::to_string(references.most) + " of " + std::to_string(references.capacity) + " in a frame" +
				(references.capacity > 0 && references.most >= references.capacity ? " - FULL: past it a streaming reference is taken and never released, a silent leak" : "");
			std::string line = "DIAGNOSTIC: the draw list's references: ";
			if (references.state == State::NotMade)
				line += "not made yet";
			else if (references.state == State::Read)
				line += numbers;
			else
				line += kNotReadFurther + (references.capacity > 0 ? ", until then " + numbers : std::string());
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "render queues summary", kFirstSummaryMs, &Numbers, &LogSummary };
	}

	void MeasureRenderQueues()
	{
		std::string why;
		if (!FindAndCheck(g_sites, why))
		{
			WriteLog(kTag, kNotMeasured + why);
			return;
		}
		if (!StartModuleThread(SampleQueues, "render queues sampling"))	// its line is written
		{
			WriteLog(kTag, std::string(kNotMeasured) + "its sampling thread could not be started.");
			return;
		}
		if (!StartMeasurement<kMeasurement>())	// its line is written
		{
			g_stopSampling = true;
			WriteLog(kTag, std::string(kNotMeasured) + "its summary thread could not be started.");
			return;
		}
		WriteLog(kTag, "DIAGNOSTIC: the render queues are read every " + std::to_string(kSampleMs) +
			" ms: how near their pools, their arrays and the draw list's references have come is logged after " +
			std::to_string(kFirstSummaryMs / 1000) + " seconds, then every minute while a most changes.");
	}
}
