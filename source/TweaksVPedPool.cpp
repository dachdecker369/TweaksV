// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the ped pool, measured first (see TweaksVPedPool.h).
//
// The game keeps its peds in one pool of 256 slots of 6,528 bytes, sized by Peds in gameconfig.xml; in normal play it does not bind, as
// the population's own caps stop ambient and scenario peds first, and a full pool is graceful: the one allocation returns null and its
// callers cope. Raising the number alone is a trap, though: three of the game's arrays stay at 256 with no check, so past 256 peds alive
// the game corrupts memory, long after it has booted well (PATTERNS.md, "Ped pool"). Before anything is raised, this module measures how
// close a load comes:
//   DIAGNOSTIC_MODE  the measurement, and nothing is looked for without it: every site a raise would rely on is found and checked; a
//                    thread reads the slots in use and the spatial array's count once a second and keeps the most of each; 10 seconds
//                    in, and then every minute while they change, the log says how many of the pool's slots are in use, how many of
//                    those are mission peds and how many wait for reuse, the population's counts against their caps, and the spatial
//                    array's count
//   DRY_RUN          changes nothing here: the module writes nothing to the game
// Everything is checked first, in the order of the patterns, and the first thing not as the research found it ends the checks. Every read
// of the game's data goes through IsReadable, of its heap data through CopyMemorySafely, and a walk of the slots is bounded by the pool's
// own count. A pattern missing, a site not as expected or a thread that cannot start is logged, not reported - the measurement is an aid
// (NOTES.md, T-DIAG) - and the module measures nothing more.
// Written for TweaksV from the findings of its own research in the game's code (PATTERNS.md); the idea of a larger pool is that of Resource
// Extensions [Enhanced] by Koukol, whose plugin was not read (CREDITS.md).

#include "TweaksVPedPool.h"

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>

#include "TweaksVConflicts.h"
#include "TweaksVHeapAdjuster.h"
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
		constexpr const char* kTag = "PedPool";
		constexpr const char* kNotMeasured = "DIAGNOSTIC: the ped pool is not measured: ";

		// ---- the sites (PATTERNS.md, "Ped pool")

		constexpr const GamePattern& kAccessor = Pattern("N-E-A1");		// the pool's accessor, which decodes its pointer
		constexpr const GamePattern& kMaker = Pattern("N-E-M1");		// the maker's call of the pool's constructor
		constexpr const GamePattern& kPoolLookup = Pattern("N-E-M2");	// the pool's size looked up in gameconfig.xml
		constexpr const GamePattern& kFileValue = Pattern("N-E-C1");	// the number of peds, from the same entry, handed to its setter
		constexpr const GamePattern& kIntelligence = Pattern("N-E-I1");	// PedIntelligence's size, which follows the number
		constexpr const GamePattern& kSpatial = Pattern("N-E-S1");		// the spatial array, made by the pool's slots
		constexpr const GamePattern& kCounters = Pattern("N-E-K1");		// the population's counts, as the crash log reads them
		constexpr const GamePattern& kCaps = Pattern("N-E-K2");			// the population's caps, as each update stores them
		constexpr const GamePattern& kCopy = Pattern("N-E-R1");			// a copy of the peds into a fixed array of 256
		constexpr const GamePattern& kReset = Pattern("N-E-R2");		// the timeslice list's reset and its call of the filler
		constexpr const GamePattern& kFiller = Pattern("N-E-R3");		// the filler's frame of three fixed arrays
		constexpr const GamePattern& kPriority = Pattern("N-E-R4");		// the priority function's frame of a fixed array

		// N-E-A1: "movzx eax, byte [rip+disp32]", the flag; "mov rcx, [rip+disp32]", the keys; "rol rcx, 0x1E"; "mov rdx, [rip+disp32]",
		// the value; "xor rdx, rcx"; "rol rdx, 0x20"; "and cl, 0x1F"; "add cl, 2"; "rol rdx, cl"; "not rdx" - what DecodePoolPointer does.
		constexpr size_t kFlagAt = 3, kKeysAt = 17, kValueAt = 28;
		constexpr int kRol = 0x1E, kAdd = 2;
		static_assert(PatternHolds(kAccessor, 0, { 0x0F, 0xB6, 0x05 }) && PatternHolds(kAccessor, kKeysAt - 3, { 0x48, 0x8B, 0x0D }) && PatternHolds(kAccessor, kKeysAt + 4, { 0x48, 0xC1, 0xC1, kRol }) &&
			PatternHolds(kAccessor, kValueAt - 3, { 0x48, 0x8B, 0x15 }) && PatternHolds(kAccessor, kValueAt + 4, { 0x48, 0x31, 0xCA, 0x48, 0xC1, 0xC2, 0x20, 0x80, 0xE1, 0x1F, 0x80, 0xC1, kAdd, 0x48, 0xD3, 0xC2, 0x48, 0xF7, 0xD2 }),
			"TweaksV: N-E-A1 holds the three loads and the decode DecodePoolPointer does");

		// N-E-M1: "lea rax, [rip+disp32]", the allocator's proxy; then the stride handed to the constructor.
		constexpr size_t kProxyAt = 3, kStrideAt = 16;
		constexpr int32_t kStride = 0x1980;		// a slot: 6,528 bytes
		static_assert(PatternHolds(kMaker, 0, { 0x48, 0x8D, 0x05 }) && PatternImm32(kMaker, kStrideAt) == kStride, "TweaksV: N-E-M1 holds the proxy's lea and the stride 0x1980");

		// The lookups' calls: N-E-M2 and N-E-I1 at +8, N-E-C1 at +11 with the setter's at +18, a function of "mov [rip+disp32], ecx; ret".
		constexpr size_t kLookupCallAt = 8, kFileLookupCallAt = 11, kSetterCallAt = 18;
		static_assert(PatternHolds(kPoolLookup, kLookupCallAt, { 0xE8 }) && PatternHolds(kIntelligence, kLookupCallAt, { 0xE8 }) && PatternHolds(kFileValue, kFileLookupCallAt, { 0xE8 }) &&
			PatternHolds(kFileValue, kSetterCallAt, { 0xE8 }), "TweaksV: N-E-M2, N-E-I1 and N-E-C1 hold their calls");
		constexpr size_t kSetterSize = 7, kSetterNumberAt = 2;

		// N-E-S1: at a fixed distance past the match, "mov [rip+disp32], rbx", the array's pointer.
		constexpr size_t kSpatialStoreAt = 63, kSpatialPointerAt = 66, kSpatialStoreSize = 7;

		// N-E-K1: four "mov r32, [rip+disp32]"; N-E-K2: three stores, the last of an XMM register. Which of the first two caps is in cars and
		// which on foot is a hint of the research's (PATTERNS.md, "Ped pool"), so that the line says so.
		constexpr size_t kAmbientCarsAt = 2, kAmbientFootAt = 8, kScenarioCarsAt = 14, kScenarioFootAt = 20;
		constexpr size_t kCapCarsAt = 6, kCapFootAt = 20, kCapScenarioAt = 28;
		static_assert(PatternHolds(kCounters, 0, { 0x8B, 0x05 }) && PatternHolds(kCounters, kAmbientFootAt - 2, { 0x8B, 0x0D }) && PatternHolds(kCounters, kScenarioCarsAt - 2, { 0x8B, 0x35 }) &&
			PatternHolds(kCounters, kScenarioFootAt - 2, { 0x8B, 0x3D }) && PatternHolds(kCaps, kCapCarsAt - 2, { 0x89, 0x05 }) && PatternHolds(kCaps, kCapFootAt - 2, { 0x89, 0x05 }) &&
			PatternHolds(kCaps, kCapScenarioAt - 4, { 0x66, 0x0F, 0x7E, 0x3D }), "TweaksV: N-E-K1 holds the four counts' loads, N-E-K2 the three caps' stores");

		// N-E-R1: two calls of the number's getter, and the 12 bytes that undo their clamp.
		constexpr size_t kCopyFirstCallAt = 2, kCopySecondCallAt = 15, kCopyClampAt = 23, kCopyClampSize = 12;
		static_assert(PatternHolds(kCopy, kCopyFirstCallAt, { 0xE8 }) && PatternHolds(kCopy, kCopySecondCallAt, { 0xE8 }) && PatternByte(kCopy, kCopyClampAt) == 0x8B &&
			PatternByte(kCopy, kCopyClampAt + kCopyClampSize - 1) == 0x10 && PatternByte(kCopy, kCopyClampAt + kCopyClampSize) == -1, "TweaksV: N-E-R1 holds the getter's two calls and the clamp's undoing, ending the match");

		// N-E-R2: "mov word [rip+disp32], 0", the list's count, its operand's end past the imm16; the call of the filler.
		constexpr size_t kResetCountAt = 3, kResetCountEnd = 9, kResetCallAt = 9;
		static_assert(PatternHolds(kReset, 0, { 0x66, 0xC7, 0x05 }) && PatternHolds(kReset, kResetCountEnd - 2, { 0x00, 0x00, 0xE8 }), "TweaksV: N-E-R2 holds the count's store and the filler's call");

		// N-E-R3: 5 bytes into the filler; a call past the frame's three leas, its rel32 past the match.
		constexpr size_t kFillerFrameAt = 5, kFillerCallAt = 28;
		static_assert(PatternHolds(kFiller, 0, { 0x48, 0x81, 0xEC, 0x28, 0x0C, 0x00, 0x00 }) && PatternHolds(kFiller, kFillerCallAt, { 0xE8 }) && PatternByte(kFiller, kFillerCallAt + 1) == -1,
			"TweaksV: N-E-R3 holds the filler's frame and its call, whose rel32 lies past the match");

		// N-E-R4: "mov eax, 0x1048", the stack probe's size; the spatial array's pointer; at a fixed distance, the frame given back.
		constexpr size_t kPrioritySizeAt = 1, kPriorityPointerAt = 16, kPriorityRestoreAt = 0x308;
		constexpr uint8_t kPriorityRestore[] = { 0x48, 0x81, 0xC4, 0x48, 0x10, 0x00, 0x00 };
		static_assert(PatternImm32(kPriority, kPrioritySizeAt) == 0x1048 && PatternHolds(kPriority, kPriorityPointerAt - 3, { 0x48, 0x8B, 0x05 }), "TweaksV: N-E-R4 holds the frame's size and the pointer's load");

		// N-E-D1 to D6: a pool of the anim director's family, its default after "mov r8d, imm32" (the imm32 at +7), which does not follow
		// the number and traps the game when full.
		struct Default
		{
			const GamePattern* pattern;
			const char* what;
		};
		constexpr const GamePattern& kDefaultDirector = Pattern("N-E-D1"), & kDefaultCreature = Pattern("N-E-D2"), & kDefaultMotionTree = Pattern("N-E-D3"),
			& kDefaultExpressions = Pattern("N-E-D4"), & kDefaultRagDoll = Pattern("N-E-D5"), & kDefaultFacialRig = Pattern("N-E-D6");
		constexpr Default kDefaults[] = { { &kDefaultDirector, "fwAnimDirector's" }, { &kDefaultCreature, "the Creature's" }, { &kDefaultMotionTree, "the MotionTree's" },
			{ &kDefaultExpressions, "the Expressions'" }, { &kDefaultRagDoll, "the RagDoll's" }, { &kDefaultFacialRig, "the FacialRig's" } };
		constexpr size_t kDefaultAt = 7;
		consteval bool DefaultsAsPinned()
		{
			for (const Default& entry : kDefaults)
				if (!PatternHolds(*entry.pattern, kDefaultAt - 2, { 0x41, 0xB8 }) || PatternImm32(*entry.pattern, kDefaultAt) <= 0 || !PatternHolds(*entry.pattern, kDefaultAt + 4, { 0xE8 }))
					return false;
			return true;
		}
		static_assert(DefaultsAsPinned(), "TweaksV: N-E-D1 to N-E-D6 hold a default's mov and the lookup's call");

		// ---- the game's data, as its code lays it out (PATTERNS.md, "Ped pool")

		// The pool: its slots and a flag byte per slot, bit 7 set when free; the slots' number and their size; those in use, in the low 30 bits.
		constexpr size_t kPoolSlotsAt = 0x08, kPoolFlagsAt = 0x10, kPoolCountAt = 0x18, kPoolStrideAt = 0x1C, kPoolInUseAt = 0x28, kPoolHeadSize = kPoolInUseAt + sizeof(uint32_t);
		constexpr uint32_t kPoolInUseMask = 0x3FFFFFFF;
		constexpr uint8_t kSlotFree = 0x80;
		constexpr int32_t kMaxSlots = 16384;	// a count outside 1..16384 is taken for data being changed

		// A ped: its type, of which the mission's is 6 of bits 1 to 3; a flag every loop of the game skips it by; the flag of a ped waiting for reuse.
		constexpr size_t kPedTypeAt = 0xDA, kPedSkippedAt = 0x1091, kPedReuseAt = 0x1468;
		constexpr uint8_t kPedTypeMask = 0x0E, kPedTypeMission = 0x06, kPedSkipped = 0x10, kPedReuse = 0x10;

		// The spatial array: its capacity and its count.
		constexpr size_t kSpatialCapacityAt = 0x58, kSpatialCountAt = 0x5C, kSpatialHeadSize = kSpatialCountAt + sizeof(int32_t);
		constexpr int32_t kSpatialFixedArrays = 256;	// past this the game's fixed arrays overrun
		constexpr const char* kPastFixedArrays = " - past the 256 of the game's fixed arrays: expect memory corruption";

		// What the sites lead to, set before the measurement's threads start.
		struct Data
		{
			const uint8_t* flag = nullptr;
			const uint8_t* value = nullptr;
			const uint8_t* keys = nullptr;
			const uint8_t* number = nullptr;
			const uint8_t* spatial = nullptr;
			const uint8_t* ambientFoot = nullptr;
			const uint8_t* ambientCars = nullptr;
			const uint8_t* scenarioFoot = nullptr;
			const uint8_t* scenarioCars = nullptr;
			const uint8_t* capCars = nullptr;
			const uint8_t* capFoot = nullptr;
			const uint8_t* capScenario = nullptr;
		};
		Data g_data;

		// ---- the checks

		void Site(const std::string& line, const GamePattern& pattern)
		{
			WriteLog(kTag, "DIAGNOSTIC: " + line + At(pattern));
		}

		// True if a T at at lies inside the game, aligned to its size, so that Load reads it whole, and can be read; else why not.
		template <typename T> bool DataOf(const char* what, const uint8_t* at, const GamePattern& pattern, std::string& why)
		{
			const std::string data = std::string(what) + " at " + DescribeAddress(at);
			if (!IsInsideGameImage(at) || !IsInsideGameImage(at + sizeof(T) - 1))
				why = data + " is not inside the game" + At(pattern) + ".";
			else if (reinterpret_cast<uintptr_t>(at) % sizeof(T) != 0)
				why = data + " is not aligned to " + std::to_string(sizeof(T)) + " bytes" + At(pattern) + ".";
			else if (!IsReadable(at, sizeof(T)))
				why = data + " cannot be read" + At(pattern) + ".";
			else
				return true;
			return false;
		}

		// True if the call at call leads into the game; else why not.
		bool LeadsIntoTheGame(const char* what, const uint8_t* call, const GamePattern& pattern, std::string& why)
		{
			if (IsInsideGameImage(CallTarget(call)))
				return true;
			why = std::string(what) + " at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" + At(pattern) + ".";
			return false;
		}

		// True if the size bytes past at are readable and hold bytes; else why not.
		bool HoldsBytes(const char* what, const uint8_t* at, const uint8_t* bytes, size_t size, const GamePattern& pattern, std::string& why)
		{
			if (!IsReadable(at, size))
				why = std::string(what) + " at " + DescribeAddress(at) + " cannot be read" + At(pattern) + ".";
			else if (memcmp(at, bytes, size) != 0)
				why = std::string(what) + " at " + DescribeAddress(at) + " holds " + HexBytes(at, size) + ", not " + HexBytes(bytes, size) + At(pattern) + ".";
			else
				return true;
			return false;
		}

		// Every pattern found, each once, and every site checked in the order of the patterns, each logged; false at the first that is not
		// as the research found it, and why.
		bool FindAndCheck(std::string& why)
		{
			// 1. every pattern looked for, so that the log names each one missing; the budget's through its part, which keeps its pattern
			const uint8_t* const accessor = Scanner::FindOnce(kAccessor, why);
			const uint8_t* const maker = Scanner::FindOnce(kMaker, why);
			const uint8_t* const poolLookup = Scanner::FindOnce(kPoolLookup, why);
			const uint8_t* const fileValue = Scanner::FindOnce(kFileValue, why);
			const uint8_t* const intelligence = Scanner::FindOnce(kIntelligence, why);
			const uint8_t* const spatial = Scanner::FindOnce(kSpatial, why);
			const uint8_t* const budget = FindPoolMemoryBudget(why);
			const uint8_t* const counters = Scanner::FindOnce(kCounters, why);
			const uint8_t* const caps = Scanner::FindOnce(kCaps, why);
			const uint8_t* const copy = Scanner::FindOnce(kCopy, why);
			const uint8_t* const reset = Scanner::FindOnce(kReset, why);
			const uint8_t* const filler = Scanner::FindOnce(kFiller, why);
			const uint8_t* const priority = Scanner::FindOnce(kPriority, why);
			const uint8_t* defaults[std::size(kDefaults)] = {};
			bool found = accessor && maker && poolLookup && fileValue && intelligence && spatial && budget && counters && caps && copy && reset && filler && priority;
			for (size_t i = 0; i < std::size(kDefaults); i++)
			{
				defaults[i] = Scanner::FindOnce(*kDefaults[i].pattern, why);
				found = found && defaults[i];
			}
			if (!found)
			{
				why += ".";
				return false;
			}

			// 2. the accessor's three fields
			g_data.flag = Rel32Target(accessor + kFlagAt);
			g_data.value = Rel32Target(accessor + kValueAt);
			g_data.keys = Rel32Target(accessor + kKeysAt);
			if (!DataOf<uint8_t>("the pool's flag", g_data.flag, kAccessor, why) || !DataOf<uint64_t>("the pool's value", g_data.value, kAccessor, why) ||
				!DataOf<uint64_t>("the pool's keys", g_data.keys, kAccessor, why))
				return false;
			Site("the pool's pointer's decode at " + DescribeAddress(accessor) + ": its flag at " + DescribeAddress(g_data.flag) + ", its value at " + DescribeAddress(g_data.value) +
				", its keys at " + DescribeAddress(g_data.keys), kAccessor);

			// 3. the maker's call
			const uint8_t* const proxy = Rel32Target(maker + kProxyAt);
			if (!IsInsideGameImage(proxy))
			{
				why = "the pool allocator's proxy at " + DescribeAddress(proxy) + " is not inside the game" + At(kMaker) + ".";
				return false;
			}
			Site("the pool's maker at " + DescribeAddress(maker) + ": the pool allocator's proxy at " + DescribeAddress(proxy) + ", slots of " + Hex(kStride) + " bytes", kMaker);

			// 4. one lookup for the three sizes that name it, into the game
			const uint8_t* const poolCall = poolLookup + kLookupCallAt;
			if (!LeadsIntoTheGame("the pool's lookup call", poolCall, kPoolLookup, why))
				return false;
			const uint8_t* const lookup = CallTarget(poolCall);
			Site("the pool's lookup call at " + DescribeAddress(poolCall) + " -> " + DescribeAddress(lookup), kPoolLookup);

			// 5. the file's value and its setter, whose store gives the number of peds
			const uint8_t* const fileCall = fileValue + kFileLookupCallAt;
			if (CallTarget(fileCall) != lookup)
			{
				why = "the number's lookup call at " + DescribeAddress(fileCall) + " leads to " + DescribeAddress(CallTarget(fileCall)) + ", not the pool's lookup at " + DescribeAddress(lookup) + At(kFileValue) + ".";
				return false;
			}
			const uint8_t* const setterCall = fileValue + kSetterCallAt;
			if (!LeadsIntoTheGame("the number's setter call", setterCall, kFileValue, why))
				return false;
			const uint8_t* const setter = CallTarget(setterCall);
			if (!IsReadable(setter, kSetterSize))
			{
				why = "the number's setter at " + DescribeAddress(setter) + " cannot be read" + At(kFileValue) + ".";
				return false;
			}
			if (setter[0] != 0x89 || setter[1] != 0x0D || setter[kSetterSize - 1] != 0xC3)
			{
				why = "the number's setter at " + DescribeAddress(setter) + " holds " + HexBytes(setter, kSetterSize) + ", not 89 0D, a disp32 and C3" + At(kFileValue) + ".";
				return false;
			}
			g_data.number = Rel32Target(setter + kSetterNumberAt);
			if (!DataOf<int32_t>("the game's number of peds", g_data.number, kFileValue, why))
				return false;
			Site("the number's lookup call at " + DescribeAddress(fileCall) + " -> " + DescribeAddress(lookup) + ", its setter call at " + DescribeAddress(setterCall) + " -> " +
				DescribeAddress(setter) + ", the number at " + DescribeAddress(g_data.number), kFileValue);

			// 6. PedIntelligence's size, through the same lookup
			const uint8_t* const intelligenceCall = intelligence + kLookupCallAt;
			if (CallTarget(intelligenceCall) != lookup)
			{
				why = "PedIntelligence's lookup call at " + DescribeAddress(intelligenceCall) + " leads to " + DescribeAddress(CallTarget(intelligenceCall)) + ", not the pool's lookup at " +
					DescribeAddress(lookup) + At(kIntelligence) + ".";
				return false;
			}
			Site("PedIntelligence's lookup call at " + DescribeAddress(intelligenceCall) + " -> " + DescribeAddress(lookup), kIntelligence);

			// 7. the spatial array's pointer, at a fixed distance
			constexpr uint8_t kSpatialStore[] = { 0x48, 0x89, 0x1D };
			const uint8_t* const spatialStore = spatial + kSpatialStoreAt;
			if (!IsReadable(spatialStore, kSpatialStoreSize))
			{
				why = "the spatial array's store at " + DescribeAddress(spatialStore) + " cannot be read" + At(kSpatial) + ".";
				return false;
			}
			if (!HoldsBytes("the spatial array's store", spatialStore, kSpatialStore, sizeof(kSpatialStore), kSpatial, why))
				return false;
			g_data.spatial = Rel32Target(spatial + kSpatialPointerAt);
			if (!DataOf<uint64_t>("the spatial array's pointer", g_data.spatial, kSpatial, why))
				return false;
			Site("the spatial array's capacity from the pool's slots at " + DescribeAddress(spatial) + ", its pointer's store at " + DescribeAddress(spatialStore) + " -> " +
				DescribeAddress(g_data.spatial), kSpatial);

			// 8. the budget a raised pool draws on, as its part leaves it
			WriteLog(kTag, "DIAGNOSTIC: the pool memory budget a raised pool draws on, at " + DescribeAddress(budget) + ", holds " + Hex32(ValueAt<uint32_t>(budget)) + " bytes (H-E2)");

			// 9. the population's counts and caps
			g_data.ambientCars = Rel32Target(counters + kAmbientCarsAt);
			g_data.ambientFoot = Rel32Target(counters + kAmbientFootAt);
			g_data.scenarioCars = Rel32Target(counters + kScenarioCarsAt);
			g_data.scenarioFoot = Rel32Target(counters + kScenarioFootAt);
			if (!DataOf<int32_t>("the count of ambient peds in cars", g_data.ambientCars, kCounters, why) || !DataOf<int32_t>("the count of ambient peds on foot", g_data.ambientFoot, kCounters, why) ||
				!DataOf<int32_t>("the count of scenario peds in cars", g_data.scenarioCars, kCounters, why) || !DataOf<int32_t>("the count of scenario peds on foot", g_data.scenarioFoot, kCounters, why))
				return false;
			Site("the population's counts at " + DescribeAddress(counters) + ": ambient in cars at " + DescribeAddress(g_data.ambientCars) + ", on foot at " + DescribeAddress(g_data.ambientFoot) +
				"; scenario in cars at " + DescribeAddress(g_data.scenarioCars) + ", on foot at " + DescribeAddress(g_data.scenarioFoot), kCounters);
			g_data.capCars = Rel32Target(caps + kCapCarsAt);
			g_data.capFoot = Rel32Target(caps + kCapFootAt);
			g_data.capScenario = Rel32Target(caps + kCapScenarioAt);
			if (!DataOf<int32_t>("the first cap of ambient peds", g_data.capCars, kCaps, why) || !DataOf<int32_t>("the second cap of ambient peds", g_data.capFoot, kCaps, why) ||
				!DataOf<int32_t>("the cap of scenario peds", g_data.capScenario, kCaps, why))
				return false;
			Site("the population's caps at " + DescribeAddress(caps) + ": ambient on foot at " + DescribeAddress(g_data.capFoot) + " and in cars at " + DescribeAddress(g_data.capCars) + ", which is which a hint, scenario at " +
				DescribeAddress(g_data.capScenario), kCaps);

			// 10. the copy into a fixed array: both calls to one getter, inside the game
			const uint8_t* const firstGetter = copy + kCopyFirstCallAt;
			const uint8_t* const secondGetter = copy + kCopySecondCallAt;
			if (!LeadsIntoTheGame("the copy's first getter call", firstGetter, kCopy, why) || !LeadsIntoTheGame("the copy's second getter call", secondGetter, kCopy, why))
				return false;
			if (CallTarget(firstGetter) != CallTarget(secondGetter))
			{
				why = "the copy's two getter calls at " + DescribeAddress(firstGetter) + " and " + DescribeAddress(secondGetter) + " lead to " + DescribeAddress(CallTarget(firstGetter)) + " and " +
					DescribeAddress(CallTarget(secondGetter)) + At(kCopy) + ".";
				return false;
			}
			Site("the copy's getter calls at " + DescribeAddress(firstGetter) + " and " + DescribeAddress(secondGetter) + " -> " + DescribeAddress(CallTarget(firstGetter)) +
				", the clamp's undoing at " + DescribeAddress(copy + kCopyClampAt) + ": " + HexBytes(copy + kCopyClampAt, kCopyClampSize), kCopy);

			// 11. the timeslice list's reset, whose call leads to the filler: a pair of patterns that must find one function
			const uint8_t* const resetCount = reset + kResetCountEnd + ValueAt<int32_t>(reset + kResetCountAt);
			const uint8_t* const resetCall = reset + kResetCallAt;
			if (!DataOf<uint16_t>("the timeslice list's count", resetCount, kReset, why))
				return false;
			const uint8_t* const fillerFunction = filler - kFillerFrameAt;
			if (CallTarget(resetCall) != fillerFunction)
			{
				why = "the timeslice list's reset's call at " + DescribeAddress(resetCall) + " leads to " + DescribeAddress(CallTarget(resetCall)) + ", not the filler at " +
					DescribeAddress(fillerFunction) + At(kFiller) + ".";
				return false;
			}
			Site("the timeslice list's reset at " + DescribeAddress(reset) + ": its count at " + DescribeAddress(resetCount) + ", its call -> " + DescribeAddress(fillerFunction), kReset);
			const uint8_t* const fillerCall = filler + kFillerCallAt;
			if (!IsReadable(fillerCall, 5))
			{
				why = "the filler's call at " + DescribeAddress(fillerCall) + " cannot be read" + At(kFiller) + ".";
				return false;
			}
			if (!LeadsIntoTheGame("the filler's call", fillerCall, kFiller, why))
				return false;
			Site("the filler's frame at " + DescribeAddress(filler) + ": its function at " + DescribeAddress(fillerFunction) + ", its call at " + DescribeAddress(fillerCall) + " -> " +
				DescribeAddress(CallTarget(fillerCall)), kFiller);

			// 12. the priority function's frame: given back as it is taken, and over the same spatial array
			const uint8_t* const restore = priority + kPriorityRestoreAt;
			if (!HoldsBytes("the priority function's restore", restore, kPriorityRestore, sizeof(kPriorityRestore), kPriority, why))
				return false;
			const uint8_t* const priorityPointer = Rel32Target(priority + kPriorityPointerAt);
			if (priorityPointer != g_data.spatial)
			{
				why = "the spatial array's pointer the priority function reads, " + DescribeAddress(priorityPointer) + ", is not the one its maker stores, " + DescribeAddress(g_data.spatial) + At(kPriority) + ".";
				return false;
			}
			Site("the priority function's frame at " + DescribeAddress(priority + kPrioritySizeAt) + " holds " + Hex(ValueAt<uint32_t>(priority + kPrioritySizeAt)) + ", given back at " + DescribeAddress(restore) +
				", the spatial array's pointer at " + DescribeAddress(priorityPointer), kPriority);

			// 13. the six defaults, which their patterns pin
			for (size_t i = 0; i < std::size(kDefaults); i++)
				Site(std::string(kDefaults[i].what) + " pool's default at " + DescribeAddress(defaults[i] + kDefaultAt) + " holds " + std::to_string(ValueAt<uint32_t>(defaults[i] + kDefaultAt)), *kDefaults[i].pattern);
			return true;
		}

		// ---- the measurement

		constexpr DWORD kSampleMs = 1000;

		// What the sampling thread keeps, which it alone writes; a summary adds what it reads itself.
		std::atomic<uint32_t> g_mostInUse{ 0 };
		std::atomic<int32_t> g_mostSpatial{ 0 };

		// What has been read at once: the pool and the spatial array. Heap data the game may change: read through CopyMemorySafely.
		enum class PoolState
		{
			NoPointer,		// the fields that hold its pointer cannot be read
			NotMade,		// the flag says the pool is not made yet
			NotRead,		// its head cannot be read, or says more in use than it has slots: data being changed
			BadCount,		// a count outside 1..16384: data being changed
			BadStride,		// slots of another size: not the pool the research found
			Read,
		};
		struct Reading
		{
			PoolState state = PoolState::NoPointer;
			const uint8_t* pool = nullptr;
			int32_t slots = 0;
			int32_t stride = 0;
			uint32_t inUse = 0;
			const uint8_t* slotsAt = nullptr;
			const uint8_t* flagsAt = nullptr;
			bool spatialMade = false;	// its pointer is set
			bool spatialRead = false;	// its head could be read and is plausible
			int32_t spatialCapacity = 0;
			int32_t spatialCount = 0;
		};

		Reading ReadNow()
		{
			Reading reading;
			if (IsReadable(g_data.flag, sizeof(uint8_t)) && IsReadable(g_data.value, sizeof(uint64_t)) && IsReadable(g_data.keys, sizeof(uint64_t)))
			{
				const uint64_t pool = DecodePoolPointer(Load<uint8_t>(g_data.flag), Load<uint64_t>(g_data.value), Load<uint64_t>(g_data.keys), kRol, kAdd);
				reading.state = PoolState::NotMade;
				if (pool)
				{
					reading.pool = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(pool));
					uint8_t head[kPoolHeadSize];
					reading.state = PoolState::NotRead;
					if (CopyMemorySafely(head, reading.pool, sizeof(head)))
					{
						reading.slots = ValueAt<int32_t>(head, kPoolCountAt);
						reading.stride = ValueAt<int32_t>(head, kPoolStrideAt);
						reading.inUse = ValueAt<uint32_t>(head, kPoolInUseAt) & kPoolInUseMask;
						reading.slotsAt = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ValueAt<uint64_t>(head, kPoolSlotsAt)));
						reading.flagsAt = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ValueAt<uint64_t>(head, kPoolFlagsAt)));
						reading.state = reading.slots < 1 || reading.slots > kMaxSlots ? PoolState::BadCount : reading.stride != kStride ? PoolState::BadStride :
							reading.inUse > static_cast<uint32_t>(reading.slots) ? PoolState::NotRead : PoolState::Read;
					}
				}
			}
			if (IsReadable(g_data.spatial, sizeof(uint64_t)))
				if (const uint64_t spatial = Load<uint64_t>(g_data.spatial))
				{
					reading.spatialMade = true;
					uint8_t head[kSpatialHeadSize];
					if (CopyMemorySafely(head, reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(spatial)), sizeof(head)))
					{
						reading.spatialCapacity = ValueAt<int32_t>(head, kSpatialCapacityAt);
						reading.spatialCount = ValueAt<int32_t>(head, kSpatialCountAt);
						reading.spatialRead = reading.spatialCapacity >= 1 && reading.spatialCapacity <= kMaxSlots && reading.spatialCount >= 0 && reading.spatialCount <= reading.spatialCapacity;
					}
				}
			return reading;
		}

		// The most of each so far, this reading's among them; this thread alone writes them when it is the sampling thread, and a summary
		// that finds more stores it too, the larger staying.
		void KeepTheMost(const Reading& reading)
		{
			if (reading.state == PoolState::Read)
			{
				uint32_t most = g_mostInUse.load(std::memory_order_relaxed);
				while (reading.inUse > most && !g_mostInUse.compare_exchange_weak(most, reading.inUse, std::memory_order_relaxed))
				{
				}
			}
			if (reading.spatialRead)
			{
				int32_t most = g_mostSpatial.load(std::memory_order_relaxed);
				while (reading.spatialCount > most && !g_mostSpatial.compare_exchange_weak(most, reading.spatialCount, std::memory_order_relaxed))
				{
				}
			}
		}

		// The sampling thread: a reading a second, and a line of its own at once the first time the spatial array is seen past the game's fixed
		// arrays, as the corruption that follows may end the game before the next summary - a reading a second can still miss it. Each pass
		// inside a try, so nothing leaves the thread; it never ends.
		DWORD WINAPI SamplePool(LPVOID)
		{
			bool pastSaid = false;
			for (;;)
			{
				Sleep(kSampleMs);
				try
				{
					const Reading reading = ReadNow();
					KeepTheMost(reading);
					if (!pastSaid && reading.spatialRead && reading.spatialCount > kSpatialFixedArrays)
					{
						pastSaid = true;
						WriteLog(kTag, "DIAGNOSTIC: the ped pool's spatial array holds " + std::to_string(reading.spatialCount) + " of " + std::to_string(reading.spatialCapacity) + kPastFixedArrays + ".");
					}
				}
				catch (...)
				{
				}
			}
		}

		// The summary's state, kept by its thread alone: the reading of the last summary, the most at that moment and how many readings had news.
		Reading g_logged;
		uint32_t g_loggedMostInUse = 0;
		int32_t g_loggedMostSpatial = 0;
		uint64_t g_news = 0;

		// Changes with every reading that has something new to say: the first; the pool's state or address; the slots in use, the most, the
		// spatial array.
		uint64_t Numbers()
		{
			const Reading now = ReadNow();
			KeepTheMost(now);
			const uint32_t mostInUse = g_mostInUse.load(std::memory_order_relaxed);
			const int32_t mostSpatial = g_mostSpatial.load(std::memory_order_relaxed);
			const bool news = g_news == 0 || now.state != g_logged.state || now.pool != g_logged.pool || now.slots != g_logged.slots || now.stride != g_logged.stride ||
				now.inUse != g_logged.inUse || mostInUse != g_loggedMostInUse || now.spatialMade != g_logged.spatialMade || now.spatialRead != g_logged.spatialRead ||
				now.spatialCount != g_logged.spatialCount || mostSpatial != g_loggedMostSpatial;
			if (news)
			{
				g_logged = now;
				g_loggedMostInUse = mostInUse;
				g_loggedMostSpatial = mostSpatial;
				g_news++;
			}
			return g_news;
		}

		// A count of the game's that has been read as it is, or 0 if its address cannot be read: plain ints, for the line alone.
		int32_t Count(const uint8_t* at)
		{
			return IsReadable(at, sizeof(int32_t)) ? Load<int32_t>(at) : 0;
		}

		// The walk of the slots, for the summary alone and bounded by the pool's own count: the flags at once, and of each live slot the ped's
		// three bytes, a ped every loop of the game skips not counted. False if something could not be read: the game may have been changing it.
		uint8_t g_slotFlags[kMaxSlots];		// the summary's thread alone: no allocation for a line
		bool CountPeds(const Reading& reading, int& mission, int& reuse)
		{
			const size_t slots = static_cast<size_t>(reading.slots);
			if (!CopyMemorySafely(g_slotFlags, reading.flagsAt, slots))
				return false;
			for (size_t i = 0; i < slots; i++)
			{
				if (g_slotFlags[i] & kSlotFree)
					continue;
				const uint8_t* const ped = reading.slotsAt + i * static_cast<size_t>(reading.stride);
				uint8_t skipped = 0, type = 0, flags = 0;
				if (!CopyMemorySafely(&skipped, ped + kPedSkippedAt, 1) || !CopyMemorySafely(&type, ped + kPedTypeAt, 1) || !CopyMemorySafely(&flags, ped + kPedReuseAt, 1))
					return false;
				if (skipped & kPedSkipped)
					continue;
				if ((type & kPedTypeMask) == kPedTypeMission)
					mission++;
				if (flags & kPedReuse)
					reuse++;
			}
			return true;
		}

		void LogSummary(uint64_t)
		{
			const Reading& now = g_logged;
			switch (now.state)
			{
			case PoolState::NoPointer:
				WriteLog(kTag, "DIAGNOSTIC: the ped pool's pointer at " + DescribeAddress(g_data.value) + " cannot be read.");
				return;
			case PoolState::NotMade:
				WriteLog(kTag, "DIAGNOSTIC: no ped pool yet - the game has not made it.");
				return;
			case PoolState::NotRead:
				WriteLog(kTag, "DIAGNOSTIC: the ped pool at " + DescribeAddress(now.pool) + " could not be read this time (the game may have been changing it).");
				return;
			case PoolState::BadCount:
				WriteLog(kTag, "DIAGNOSTIC: the ped pool at " + DescribeAddress(now.pool) + " could not be read this time: its slots, " + std::to_string(now.slots) + ", are outside 1.." +
					std::to_string(kMaxSlots) + " (the game may have been changing it).");
				return;
			case PoolState::BadStride:
				WriteLog(kTag, "DIAGNOSTIC: the ped pool at " + DescribeAddress(now.pool) + " has slots of " + Hex(static_cast<uint32_t>(now.stride)) + " bytes, not " + Hex(kStride) +
					": it is not read (another mod may have made it, or the game has changed).");
				return;
			case PoolState::Read:
				break;
			}

			// the number must equal the pool's slots: a pool larger than the number overruns two arrays of the game's
			const int32_t number = Count(g_data.number);
			if (number != now.slots)
				WriteLog(kTag, "DIAGNOSTIC: ped pool: the game's number of peds, " + std::to_string(number) + ", is not the pool's " + std::to_string(now.slots) +
					" slots - a mod changed one of them, and a pool larger than the number overruns two of the game's arrays.");

			int mission = 0, reuse = 0;
			const bool counted = CountPeds(now, mission, reuse);
			std::string line = "DIAGNOSTIC: ped pool: " + std::to_string(now.inUse) + " of " + std::to_string(now.slots) + " slots in use, at most " + std::to_string(g_loggedMostInUse) + " so far; " +
				(counted ? "mission " + std::to_string(mission) + ", reuse " + std::to_string(reuse) : std::string("mission and reuse not counted this time")) +
				"; scenario " + std::to_string(Count(g_data.scenarioFoot)) + " on foot and " + std::to_string(Count(g_data.scenarioCars)) + " in cars, cap " + std::to_string(Count(g_data.capScenario)) +
				"; ambient " + std::to_string(Count(g_data.ambientFoot)) + " on foot and " + std::to_string(Count(g_data.ambientCars)) + " in cars, caps " + std::to_string(Count(g_data.capFoot)) +
				" and " + std::to_string(Count(g_data.capCars)) + " (which is which a hint); " +
				(now.spatialRead ? "spatial array " + std::to_string(now.spatialCount) + " of " + std::to_string(now.spatialCapacity) + ", at most " + std::to_string(g_loggedMostSpatial) :
					std::string(now.spatialMade ? "spatial array not read this time" : "spatial array not made yet"));
			if (g_loggedMostInUse >= static_cast<uint32_t>(now.slots))
				line += " - the pool has been full: the game makes no more peds until one goes";
			if (g_loggedMostSpatial > kSpatialFixedArrays)
				line += kPastFixedArrays;
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "ped pool summary", kFirstSummaryMs, &Numbers, &LogSummary };
	}

	void MeasurePedPool()
	{
		std::string why;
		if (!FindAndCheck(why))
		{
			WriteLog(kTag, kNotMeasured + why);
			return;
		}
		if (!StartMeasurement<kMeasurement>())	// its line is written
		{
			WriteLog(kTag, std::string(kNotMeasured) + "its summary thread could not be started.");
			return;
		}
		const bool sampled = StartModuleThread(SamplePool, "ped pool sampling");
		if (!sampled)
			WriteLog(kTag, "DIAGNOSTIC: the ped pool is not sampled, so its most so far are those of the summaries: its thread could not be started.");
		WriteLog(kTag, "DIAGNOSTIC: the ped pool is measured, through its pointer at " + DescribeAddress(g_data.value) + At(kAccessor) + (sampled ?
			": the slots in use and the spatial array are sampled every second and logged after 10 seconds, then every minute while they change." :
			", its numbers not sampled: the slots in use and the spatial array are logged after 10 seconds, then every minute while they change."));
	}
}
