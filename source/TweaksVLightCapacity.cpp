// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the lighting capacity, measured first (see TweaksVLightCapacity.h).
//
// A vehicle's lights go through a job's list, 80 lights and 200 coronas for 5 vehicles, where a light past the room is silently
// dropped; at the job's end into the frame's two stores, 400 lights and 928 coronas, with no check at all, over the stores' own
// write pointers; once a frame into the scene store, whose 1024 keep the nearer light but replace a wrong slot, an index of a
// byte; and from there into the ray traced list of 1024, whose upload takes 512, and the corona store of 1088. 300 vehicles a
// frame have room, unchecked as well (PATTERNS.md, "LightCapacity"). Before a part raises anything, this module measures which
// of these layers a mod load reaches:
//   DIAGNOSTIC_MODE  the measurement, and nothing is looked for without it: a thread reads the game's counters every 10 ms,
//                    aligned values of its .data, and keeps each one's most, of the minute and of the session; the jump a
//                    job's full list takes leads through a counting stub, the module's one write, unless the game's frames have
//                    begun and its threads may run it; 10 seconds in, and then every minute while the numbers change, the log says
//                    how far each layer filled, how many lights the jobs refused and what the siren settings ask for. A number
//                    seen past its room is a line of its own at once
//   DRY_RUN          everything found and checked and the sampling as ever; the jump left as it is, so the refused lights are
//                    not counted
// All 22 patterns are found first, those a raise will use with them, and every site checked as a writing half would: the stores'
// and the lists' distances, the stores' references by a scan, each site naming what the others name, every address inside the game, every value read aligned and
// readable; the constants the patterns pin are compared when TweaksV is compiled. A pattern missing, a site not as expected, a
// thread that cannot start or a stub that cannot be made is logged, not reported - the measurement is an aid (NOTES.md, T-DIAG).
// The stub runs on the game's worker threads, so it counts with a lock instruction into a 64-bit counter, keeps every register,
// uses no stack, changes only the flags, which the jump's target writes anew before anything reads them, and lies near the
// game, read+execute, its counter on a read/write page of its own. It runs only when a list is full.
// Written for TweaksV from the findings of its own research in the game's code (PATTERNS.md); the idea of many siren lights
// reaching this capacity comes from the page of Resource Extensions [Enhanced] by Koukol, whose plugin was not read (CREDITS.md).

#include "TweaksVLightCapacity.h"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSirenIds.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "LightCapacity";

		// ---- the sites (PATTERNS.md, "LightCapacity")

		// In the order of the table, each looked for once but G1, which the game holds twice.
		enum Site : size_t { kA1, kA2, kA3, kB1, kB2, kB3, kC1, kC2, kC3, kC4, kD1, kD2, kE1, kE2, kE3, kF1, kF2, kF3, kG1, kG2, kH1, kH2, kSites };
		constexpr const GamePattern* kSitePatterns[kSites] = { &Pattern("G-E-A1"), &Pattern("G-E-A2"), &Pattern("G-E-A3"), &Pattern("G-E-B1"), &Pattern("G-E-B2"),
			&Pattern("G-E-B3"), &Pattern("G-E-C1"), &Pattern("G-E-C2"), &Pattern("G-E-C3"), &Pattern("G-E-C4"), &Pattern("G-E-D1"), &Pattern("G-E-D2"), &Pattern("G-E-E1"),
			&Pattern("G-E-E2"), &Pattern("G-E-E3"), &Pattern("G-E-F1"), &Pattern("G-E-F2"), &Pattern("G-E-F3"), &Pattern("G-E-G1"), &Pattern("G-E-G2"), &Pattern("G-E-H1"),
			&Pattern("G-E-H2") };
		constexpr const GamePattern& P(Site site)
		{
			return *kSitePatterns[site];
		}
		constexpr size_t kG1Matches = 2;	// the corona store's check, in two functions
		constexpr size_t kStoreReferences = 3;	// the places that load each of the frame's two stores: the reset, the drain and the constructor

		// What a light and a corona take, and the room in each layer. The stores' come from A1's distances, which the module checks;
		// the vehicles' 300 is in no pattern: arrays of that length, each with its count right behind it (PATTERNS.md).
		constexpr uint32_t kLightBytes = 0x1C0, kCoronaBytes = 0x30;
		constexpr uint32_t kStoreLights = 400, kStoreCoronas = 928, kJobLights = 80, kJobCoronas = 200, kJobVehicles = 5;
		constexpr uint32_t kVehiclesRoom = 300, kSceneRoom = 1024, kRayTracedRoom = 1024, kUploadRoom = 512, kReflectionRoom = 768, kCoronaRoom = 1088;
		constexpr uint32_t kCoronaStoreBytes = kStoreCoronas * kCoronaBytes, kLightStoreBytes = kStoreLights * kLightBytes;
		constexpr uint32_t kJobCoronaBytes = kJobCoronas * kCoronaBytes, kJobLightBytes = kJobLights * kLightBytes, kJobBytes = kJobCoronaBytes + kJobLightBytes;
		constexpr uint32_t kSceneBuffer = 0x70000, kRayTracedList = 0x72010, kReflectionBuffer = 0x29A0C, kCoronaCountAt = 0xCC00;
		constexpr uint32_t kUploadRecordsAt = 0x70E0, kWorkerFrame = 0x18058, kScratchAt = 0x50;
		constexpr uint32_t kSortEntries = kSceneRoom, kSortEntryBytes = 8;

		// -N-1, as a site's imm32 holds it.
		constexpr int64_t MinusOneMore(uint32_t value)
		{
			return int64_t{ 0x100000000 } - value - 1;
		}

		// The sites' rel32 and disp32, from their matches. B1's is followed by the instruction's imm32, so the address it names
		// counts from past that; A3's call, E1's last store and F2's call lie past their matches.
		constexpr size_t kCoronaStoreAt = 3, kCoronaPointerAt = 10, kLightStoreAt = 24, kLightPointerAt = 31;	// A1; A2 the first two; A3 the same two for lights
		constexpr size_t kDrainStoreAt = 3, kDrainPointerAt = 10, kDrainCallAt = 32;							// A2, A3
		constexpr size_t kVehiclesAt = 2, kVehiclesEnd = 10;													// B1
		constexpr size_t kJumpAt = 6, kJumpRelAt = 8, kJumpSize = 6;											// C1: "jae rel32"
		constexpr size_t kStartValuesAt = 8;																	// C4
		constexpr size_t kSceneCountAt = 3, kSceneLimitAt = 9, kSortCountAt = 22, kSortListAt = 29, kSortStoreAt = 38, kByteStoreAt = 42;	// D1
		constexpr size_t kStoreBaseAt = 10, kRenderPointerAt = 20, kBufferCountsAt = 27, kRenderCountAt = 38, kReflectionsAt = 52, kReflectionPointerAt = 62;	// E1
		constexpr size_t kCurrentPointerAt = 20;																// E2; the store as E1's
		constexpr size_t kRayTracedAt = 10, kRayTracedPointerAt = 21;											// E3
		constexpr size_t kCheckedPointerAt = 3, kRayTracedLimitAt = 14;											// F1
		constexpr size_t kUploadRecordsImmAt = 11, kUploadImmAt = 16, kUploadCallAt = 20;						// F2
		constexpr size_t kReflectionLimitAt = 7;																// F3
		constexpr size_t kCoronaCountRead = 2, kCoronaLimitAt = 9, kCoronaAddLimitAt = 10;						// G1; G2 the limit

		// Every constant the measurement counts by is the one its pattern pins.
		static_assert(PatternHolds(P(kB1), 0, { 0x69, 0x05 }) && PatternHolds(P(kB1), kVehiclesAt + 4, { 0xCD, 0xCC, 0xCC, 0xCC, 0x05, 0x99, 0x99, 0x99, 0x19, 0x3D, 0x32, 0x33, 0x33, 0x33, 0x77 }),
			"TweaksV: G-E-B1 is imul eax, [rip+disp32], imm32, its disp32 at +2 and its imm32 past it, then a job's 5 vehicles in the three constants' test");
		static_assert(kVehiclesEnd == kVehiclesAt + 4 + 4 && kJobVehicles * 60 == kVehiclesRoom, "TweaksV: G-E-B1's disp32 counts from past its imm32; 60 jobs of 5 vehicles");
		static_assert(PatternImm32(P(kB2), 4) == kJobBytes && PatternImm32(P(kB3), 5) == kJobBytes, "TweaksV: G-E-B2 and G-E-B3 give a job its scratch of 0xB180 bytes");
		static_assert(PatternHolds(P(kC1), 0, { 0x8B, 0x41, 0x14, 0x3B, 0x41, 0x1C }) && PatternHolds(P(kC1), kJumpAt, { 0x0F, 0x83 }),
			"TweaksV: G-E-C1 compares a list's count with its capacity and jumps when they are not below: its jae at +6, its rel32 at +8");
		static_assert(PatternImm32(P(kC2), 17) == kJobCoronaBytes && PatternImm32(P(kC2), 22) == kJobLightBytes && PatternImm32(P(kC2), 32) == MinusOneMore(kJobLightBytes) &&
			PatternImm32(P(kC2), 42) == MinusOneMore(kJobBytes) && PatternImm32(P(kC2), 47) == kJobBytes, "TweaksV: G-E-C2 lays a job's scratch out as 200 coronas and 80 lights");
		static_assert(PatternImm32(P(kC3), 2) == kJobLightBytes && PatternImm32(P(kC3), 9) == kJobLightBytes, "TweaksV: G-E-C3 holds the job's lights' bytes twice");
		static_assert(PatternHolds(P(kC4), 5, { 0x0F, 0x28, 0x0D }), "TweaksV: G-E-C4 loads a job's lists' start values through movaps xmm1, [rip+disp32]");
		static_assert(PatternImm32(P(kD1), kSceneLimitAt) == kSceneRoom - 1 && PatternHolds(P(kD1), kByteStoreAt, { 0x88, 0x04, 0xCA }) && PatternHolds(P(kD2), 0, { 0x0F, 0xB6, 0x1C, 0xC1 }),
			"TweaksV: G-E-D1 checks the scene store against 1023 and stores a sort entry's index as a byte, which G-E-D2 reads back as one");
		static_assert(PatternImm32(P(kE1), 3) == kSceneBuffer && PatternImm32(P(kE1), 45) == kReflectionBuffer && PatternImm32(P(kE2), 3) == kSceneBuffer &&
			PatternImm32(P(kE2), 41) == kReflectionBuffer && PatternImm32(P(kE3), 3) == kRayTracedList, "TweaksV: G-E-E1, G-E-E2 and G-E-E3 hold the buffers' sizes");
		static_assert(kSceneBuffer == kSceneRoom * kLightBytes, "TweaksV: a scene store's buffer holds 1024 lights");
		static_assert(PatternImm32(P(kF1), kRayTracedLimitAt) == kRayTracedRoom - 1 && PatternImm32(P(kF2), kUploadRecordsImmAt) == kUploadRecordsAt && PatternImm32(P(kF2), kUploadImmAt) == kUploadRoom &&
			PatternImm32(P(kF3), kReflectionLimitAt) == kReflectionRoom - 1, "TweaksV: G-E-F1 checks against 1023, G-E-F2 uploads 512 at +0x70E0, G-E-F3 checks against 767");
		static_assert(PatternImm32(P(kG1), kCoronaLimitAt) == kCoronaRoom - 1 && PatternImm32(P(kG2), 3) == kCoronaCountAt && PatternImm32(P(kG2), kCoronaAddLimitAt) == kCoronaRoom - 1 &&
			kCoronaCountAt == kCoronaRoom * kCoronaBytes, "TweaksV: G-E-G1 and G-E-G2 check the corona store against 1087, its count right behind its 1088 coronas");
		static_assert(PatternImm32(P(kH1), 1) == kWorkerFrame && PatternByte(P(kH2), 4) == kScratchAt && kJobBytes <= kWorkerFrame - kScratchAt,
			"TweaksV: G-E-H1 and G-E-H2 give a job's scratch the worker's frame from +0x50, which holds it");

		// The siren setting, as the module reads it (PATTERNS.md, "Siren count"): a byte that gives each of its lights a real light of
		// its own, its 20 lights of 0x48 bytes, and in each the flag that makes it a light at all.
		constexpr size_t kRealLightsAt = 0x38, kSirenLightsAt = 0x3C, kSirenLightBytes = 0x48, kSirenLightFlagAt = 0x45;
		constexpr size_t kSirenLights = 20, kSirenBytesRead = kSirenLightsAt + kSirenLights * kSirenLightBytes;

		// The start values of a job's lists: their two counts and capacities, the coronas' first.
		constexpr uint32_t kStartValues[4] = { 0, 0, kJobCoronas, kJobLights };

		// Where the function's jump when full leads, on 1.0.1158.16 (PATTERNS.md): add rsp, 0x20; pop rbx; pop rdi; pop rsi; pop r14; pop r15;
		// ret. None of them reads a flag, and add writes them anew, which the stub's lock inc rests on.
		constexpr uint8_t kWayOut[] = { 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5F, 0x5E, 0x41, 0x5E, 0x41, 0x5F, 0xC3 };

		// What the measurement reads, every one checked before its threads start.
		struct Reads
		{
			const uint8_t* vehicles = nullptr;			// a dword: the vehicles of this frame
			const uint8_t* coronaStore = nullptr;
			const uint8_t* lightStore = nullptr;
			const uint8_t* coronaPointer = nullptr;		// a qword: where the jobs write the next coronas
			const uint8_t* lightPointer = nullptr;		// a qword: where the jobs write the next lights
			const uint8_t* sceneCount = nullptr;		// a dword: the scene store being filled
			const uint8_t* renderCount = nullptr;		// a dword: the one the frame draws
			const uint8_t* sortMark = nullptr;			// a byte the store's add sorts its list by; who sets it, and when, the research does not say
			const uint8_t* rayTraced = nullptr;			// a dword each, kRayTracedList bytes apart: the two ray traced lists' counts
			const uint8_t* reflections = nullptr;		// three dwords each, kReflectionBuffer bytes apart: the reflection lists' counts
			const uint8_t* coronas = nullptr;			// a dword: the corona store's count
		};
		Reads g_reads;

		void Note(const std::string& line)
		{
			WriteLog(kTag, "DIAGNOSTIC: " + line);
		}

		bool Fail(const std::string& why)
		{
			Note("the lighting capacity is not measured: " + why);
			return false;
		}

		// True if the size bytes at at lie inside the game; else logged, what said to be more than one with many.
		bool InsideGame(const char* what, const uint8_t* at, size_t size, Site site, bool many = false)
		{
			if (IsInsideGameImage(at) && IsInsideGameImage(at + size - 1))
				return true;
			return Fail(std::string("the ") + what + " at " + DescribeAddress(at) + At(P(site)) + (many ? " are" : " is") + " not inside the game.");
		}

		// True if the count values of T at at lie inside the game, aligned to their size, so that Load reads each whole, and can be
		// read; else logged. The game keeps them mapped from then on: its .data, read without asking again.
		template <typename T> bool DataOf(const char* what, const uint8_t* at, size_t count, Site site)
		{
			const size_t size = count * sizeof(T);
			if (!InsideGame(what, at, size, site, count > 1))
				return false;
			if (reinterpret_cast<uintptr_t>(at) % sizeof(T) != 0)
				return Fail(std::string("the ") + what + " at " + DescribeAddress(at) + At(P(site)) + (count > 1 ? " are" : " is") + " not aligned to " + std::to_string(sizeof(T)) + " bytes.");
			if (!IsReadable(at, size))
				return Fail(std::string("the ") + what + " at " + DescribeAddress(at) + At(P(site)) + " cannot be read.");
			return true;
		}

		// Where a call past its pattern's match leads, that call's rel32 read only once it is known readable; nullptr, logged, if it
		// cannot be read or leads outside the game.
		const uint8_t* CallPastMatch(const char* what, const uint8_t* call, Site site)
		{
			if (!IsReadable(call, 5))
			{
				Fail(std::string("the ") + what + " at " + DescribeAddress(call) + " cannot be read" + At(P(site)) + ".");
				return nullptr;
			}
			const uint8_t* const target = CallTarget(call);
			if (!IsInsideGameImage(target))
			{
				Fail(std::string("the ") + what + " at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" + At(P(site)) + ".");
				return nullptr;
			}
			return target;
		}

		// "the scene store's limit at RVA ... holds 1023 (N-1, G-E-D1) -> game size 1024", as the relocation core logs a limit (SPECIFICATIONS 6.3).
		std::string LimitLine(const char* what, const uint8_t* at, uint32_t room, Site site)
		{
			return std::string(what) + " at " + DescribeAddress(at) + " holds " + std::to_string(room - 1) + " (N-1, " + P(site).label + ") -> game size " + std::to_string(room);
		}

		// The sites' matches, as FindAll and FindOnce found them.
		struct Matches
		{
			uint8_t* at[kSites] = {};
			const uint8_t* secondG1 = nullptr;
		};

		// True unless the game's code loads a store from a fourth place: the frame's reset, the drain and the constructor are all the research
		// found. The scan sees a lea or a store of a pointer: fewer than three is a fact for the boot and refuses nothing, since the research
		// gives the constructor's two by their addresses, not their instructions.
		bool CheckStoreReferences(const char* what, const uint8_t* store)
		{
			const std::vector<uint8_t*> references = Scanner::FindRipRelativeReferences(store, kStoreReferences + 1);
			const std::string places = references.size() > kStoreReferences ? std::to_string(kStoreReferences + 1) + " places or more" : std::to_string(references.size()) + " places";
			std::string list;
			for (size_t i = 0; i < references.size(); i++)
				list += (i == 0 ? "" : i + 1 == references.size() ? " and " : ", ") + DescribeAddress(references[i]);
			Note(std::string("the ") + what + " at " + DescribeAddress(store) + " is referenced in " + places + " of the game's code: " + list);
			if (references.size() <= kStoreReferences)
				return true;
			return Fail(std::string("the game's code references the ") + what + " at " + DescribeAddress(store) + " in " + places + ", not " + std::to_string(kStoreReferences) +
				" - the frame's reset, the drain and the constructor, as the research found them.");
		}

		// A1, A2, A3: the frame's two stores and their write pointers, as the reset sets them and the drain reads them; the stores' sizes
		// are their distances, the write pointers lying right behind the lights' store.
		bool CheckFrameStores(const Matches& m)
		{
			const uint8_t* const reset = m.at[kA1];
			g_reads.coronaStore = Rel32Target(reset + kCoronaStoreAt);
			g_reads.coronaPointer = Rel32Target(reset + kCoronaPointerAt);
			g_reads.lightStore = Rel32Target(reset + kLightStoreAt);
			g_reads.lightPointer = Rel32Target(reset + kLightPointerAt);
			Note("the frame's reset at " + DescribeAddress(reset) + ": the vehicle coronas' store at " + DescribeAddress(g_reads.coronaStore) + ", their write pointer at " +
				DescribeAddress(g_reads.coronaPointer) + ", the vehicle lights' store at " + DescribeAddress(g_reads.lightStore) + ", their write pointer at " +
				DescribeAddress(g_reads.lightPointer) + At(P(kA1)));
			if (!InsideGame("vehicle coronas' store", g_reads.coronaStore, kCoronaStoreBytes, kA1) || !InsideGame("vehicle lights' store", g_reads.lightStore, kLightStoreBytes, kA1) ||
				!DataOf<uint64_t>("vehicle coronas' write pointer", g_reads.coronaPointer, 1, kA1) || !DataOf<uint64_t>("vehicle lights' write pointer", g_reads.lightPointer, 1, kA1))
				return false;
			if (g_reads.lightStore - g_reads.coronaStore != kCoronaStoreBytes)
				return Fail("the vehicle lights' store at " + DescribeAddress(g_reads.lightStore) + " does not lie 928 coronas past the vehicle coronas' at " +
					DescribeAddress(g_reads.coronaStore) + At(P(kA1)) + ".");
			if (g_reads.coronaPointer - g_reads.lightStore != kLightStoreBytes)
				return Fail("the vehicle coronas' write pointer at " + DescribeAddress(g_reads.coronaPointer) + " does not lie 400 lights past the vehicle lights' store at " +
					DescribeAddress(g_reads.lightStore) + At(P(kA1)) + ".");
			if (g_reads.lightPointer - g_reads.coronaPointer != sizeof(uint64_t))
				return Fail("the vehicle lights' write pointer at " + DescribeAddress(g_reads.lightPointer) + " does not lie right behind the vehicle coronas' at " +
					DescribeAddress(g_reads.coronaPointer) + At(P(kA1)) + ".");

			const uint8_t* const coronas = m.at[kA2];
			const uint8_t* const coronaStore = Rel32Target(coronas + kDrainStoreAt);
			const uint8_t* const coronaPointer = Rel32Target(coronas + kDrainPointerAt);
			Note("the drain's coronas at " + DescribeAddress(coronas) + ": their store at " + DescribeAddress(coronaStore) + ", their write pointer at " + DescribeAddress(coronaPointer) + At(P(kA2)));
			if (coronaStore != g_reads.coronaStore || coronaPointer != g_reads.coronaPointer)
				return Fail("the drain's coronas name the store at " + DescribeAddress(coronaStore) + " and the write pointer at " + DescribeAddress(coronaPointer) +
					", not those of the frame's reset" + At(P(kA2)) + ".");

			const uint8_t* const lights = m.at[kA3];
			const uint8_t* const lightStore = Rel32Target(lights + kDrainStoreAt);
			const uint8_t* const lightPointer = Rel32Target(lights + kDrainPointerAt);
			const uint8_t* const call = CallPastMatch("drain's add-scene-light call", lights + kDrainCallAt, kA3);
			if (!call)
				return false;
			Note("the drain's lights at " + DescribeAddress(lights) + ": their store at " + DescribeAddress(lightStore) + ", their write pointer at " + DescribeAddress(lightPointer) +
				", add-scene-light's call at " + DescribeAddress(lights + kDrainCallAt) + " -> " + DescribeAddress(call) + At(P(kA3)));
			if (lightStore != g_reads.lightStore || lightPointer != g_reads.lightPointer)
				return Fail("the drain's lights name the store at " + DescribeAddress(lightStore) + " and the write pointer at " + DescribeAddress(lightPointer) +
					", not those of the frame's reset" + At(P(kA3)) + ".");
			return CheckStoreReferences("vehicle coronas' store", g_reads.coronaStore) && CheckStoreReferences("vehicle lights' store", g_reads.lightStore);
		}

		// B1, B2, B3: a job's 5 vehicles, and the size each kick gives its scratch.
		bool CheckJobs(const Matches& m)
		{
			const uint8_t* const test = m.at[kB1];
			g_reads.vehicles = test + kVehiclesEnd + ValueAt<int32_t>(test + kVehiclesAt);
			Note("a job's 5 vehicles' test at " + DescribeAddress(test) + ": the vehicles' count at " + DescribeAddress(g_reads.vehicles) + At(P(kB1)));
			if (!DataOf<uint32_t>("vehicles' count", g_reads.vehicles, 1, kB1))
				return false;
			for (const Site site : { kB2, kB3 })
				Note("a job's scratch size in its kick at " + DescribeAddress(m.at[site]) + " holds " + Hex(kJobBytes) + At(P(site)));
			return true;
		}

		// C1: add-to-a-job's-list. Its jump when full leads into the game, to the function's way out, which reads no flags.
		// The function's start is a fact for the boot; nothing here rests on it.
		bool CheckAddToList(const Matches& m, uint8_t*& jump, const uint8_t*& wayOut)
		{
			const uint8_t* const check = m.at[kC1];
			jump = m.at[kC1] + kJumpAt;
			wayOut = Rel32Target(check + kJumpRelAt);
			const uint8_t* const function = FunctionStartOf(check);
			const std::string where = function ? "in the function at " + DescribeAddress(function) + " (unwind data)" : std::string("no unwind data");
			if (!IsInsideGameImage(wayOut))
				return Fail("the jump when a job's list is full, at " + DescribeAddress(jump) + ", does not lead into the game (another mod may have hooked it already)" + At(P(kC1)) + ".");
			if (!IsReadable(wayOut, sizeof(kWayOut)))
				return Fail("the jump when a job's list is full, at " + DescribeAddress(jump) + ", leads to " + DescribeAddress(wayOut) + ", which cannot be read" + At(P(kC1)) + ".");
			Note("add-to-a-job's-list's check at " + DescribeAddress(check) + ", " + where + "; its jump when full at " + DescribeAddress(jump) + " -> " + DescribeAddress(wayOut) + ": " +
				HexBytes(wayOut, sizeof(kWayOut)) + At(P(kC1)));
			if (memcmp(wayOut, kWayOut, sizeof(kWayOut)) != 0)
				return Fail("the jump when a job's list is full, at " + DescribeAddress(jump) + ", leads to " + DescribeAddress(wayOut) + ", which holds " + HexBytes(wayOut, sizeof(kWayOut)) +
					", not the function's way out" + At(P(kC1)) + ".");
			return true;
		}

		// C2, C3, C4: a job's scratch, its constants pinned by the patterns, and its lists' start values through the rel32.
		bool CheckScratch(const Matches& m)
		{
			Note("a job's scratch at " + DescribeAddress(m.at[kC2]) + ": " + Hex(kJobCoronaBytes) + " bytes of coronas, " + Hex(kJobLightBytes) + " of lights, " + Hex(kJobBytes) + " in all" + At(P(kC2)));
			Note("a job's lights' bytes at " + DescribeAddress(m.at[kC3]) + " hold " + Hex(kJobLightBytes) + At(P(kC3)));
			const uint8_t* const values = Rel32Target(m.at[kC4] + kStartValuesAt);
			if (!DataOf<uint32_t>("job's lists' start values", values, std::size(kStartValues), kC4))
				return false;
			uint32_t read[std::size(kStartValues)] = {};
			memcpy(read, values, sizeof(read));
			Note("a job's lists' start values at " + DescribeAddress(values) + ": " + std::to_string(read[0]) + ", " + std::to_string(read[1]) + ", " + std::to_string(read[2]) + ", " +
				std::to_string(read[3]) + At(P(kC4)));
			if (memcmp(read, kStartValues, sizeof(read)) != 0)
				return Fail("a job's lists' start values at " + DescribeAddress(values) + " are not 0, 0, 200, 80" + At(P(kC4)) + ".");
			return true;
		}

		// D1, D2: the scene store's check, its count, and its sort list with the count and the mark right behind it.
		bool CheckSceneStore(const Matches& m)
		{
			const uint8_t* const store = m.at[kD1];
			g_reads.sceneCount = Rel32Target(store + kSceneCountAt);
			const uint8_t* const sortCount = Rel32Target(store + kSortCountAt);
			const uint8_t* const sortList = Rel32Target(store + kSortListAt);
			const uint8_t* const sortStore = Rel32Target(store + kSortStoreAt);
			g_reads.sortMark = sortCount + sizeof(uint32_t);
			Note(LimitLine("the scene store's limit", store + kSceneLimitAt, kSceneRoom, kD1) + ": its count at " +
				DescribeAddress(g_reads.sceneCount) + ", its sort list at " + DescribeAddress(sortList) + ", the sort list's count at " + DescribeAddress(sortCount) + ", its mark at " +
				DescribeAddress(g_reads.sortMark) + ", an index stored as a byte at " + DescribeAddress(store + kByteStoreAt) + ": 88 04 CA");
			if (!DataOf<uint32_t>("scene store's count", g_reads.sceneCount, 1, kD1) || !InsideGame("scene store's sort list", sortList, kSortEntries * kSortEntryBytes, kD1) ||
				!DataOf<uint8_t>("scene store's sort mark", g_reads.sortMark, 1, kD1))
				return false;
			if (sortCount - sortList != kSortEntries * kSortEntryBytes)
				return Fail("the sort list's count at " + DescribeAddress(sortCount) + " does not lie right behind its 1024 entries at " + DescribeAddress(sortList) + At(P(kD1)) + ".");
			if (sortStore != sortCount)
				return Fail("the sort list's count is read at " + DescribeAddress(sortCount) + " and stored at " + DescribeAddress(sortStore) + At(P(kD1)) + ".");
			Note("the sort list's index read as a byte at " + DescribeAddress(m.at[kD2]) + ": 0F B6 1C C1" + At(P(kD2)));
			return true;
		}

		// E1, E2: the scene store's render side and update side, one store, and what the render side keeps for the frame; E3 and F1: the
		// ray traced lists and their current pointer, the same in both; F2: their upload; F3: the reflection lists' check.
		bool CheckRenderSides(const Matches& m)
		{
			const uint8_t* const render = m.at[kE1];
			const uint8_t* const store = Rel32Target(render + kStoreBaseAt);
			const uint8_t* const renderPointer = Rel32Target(render + kRenderPointerAt);
			const uint8_t* const bufferCounts = Rel32Target(render + kBufferCountsAt);
			g_reads.renderCount = Rel32Target(render + kRenderCountAt);
			g_reads.reflections = Rel32Target(render + kReflectionsAt);
			const uint8_t* const reflectionPointerRel = render + kReflectionPointerAt;
			if (!IsReadable(reflectionPointerRel, sizeof(int32_t)))
				return Fail("the reflection lists' render pointer's store at " + DescribeAddress(reflectionPointerRel) + " cannot be read" + At(P(kE1)) + ".");
			const uint8_t* const reflectionPointer = Rel32Target(reflectionPointerRel);
			Note("the scene store's render side at " + DescribeAddress(render) + ": the store at " + DescribeAddress(store) + ", its render pointer at " + DescribeAddress(renderPointer) +
				", the buffers' counts at " + DescribeAddress(bufferCounts) + ", the render count at " + DescribeAddress(g_reads.renderCount) + ", the reflection lists at " +
				DescribeAddress(g_reads.reflections) + ", their render pointer at " + DescribeAddress(reflectionPointer) + At(P(kE1)));
			if (!InsideGame("scene store", store, 2 * uint64_t{ kSceneBuffer }, kE1) || !InsideGame("scene store's render pointer", renderPointer, sizeof(void*), kE1) ||
				!InsideGame("scene store's buffers' counts", bufferCounts, 2 * sizeof(uint32_t), kE1, true) || !DataOf<uint32_t>("scene store's render count", g_reads.renderCount, 1, kE1) ||
				!DataOf<uint32_t>("reflection lists' counts", g_reads.reflections, 3, kE1) ||
				!DataOf<uint32_t>("second reflection lists' counts", g_reads.reflections + kReflectionBuffer, 3, kE1) ||
				!InsideGame("reflection lists' render pointer", reflectionPointer, sizeof(void*), kE1))
				return false;

			const uint8_t* const update = m.at[kE2];
			const uint8_t* const updateStore = Rel32Target(update + kStoreBaseAt);
			const uint8_t* const current = Rel32Target(update + kCurrentPointerAt);
			Note("the scene store's update side at " + DescribeAddress(update) + ": the store at " + DescribeAddress(updateStore) + ", its current pointer at " + DescribeAddress(current) + At(P(kE2)));
			if (updateStore != store)
				return Fail("the scene store's update side names the store at " + DescribeAddress(updateStore) + ", its render side the one at " + DescribeAddress(store) + At(P(kE2)) + ".");
			if (!InsideGame("scene store's current pointer", current, sizeof(void*), kE2))
				return false;

			const uint8_t* const lists = m.at[kE3];
			g_reads.rayTraced = Rel32Target(lists + kRayTracedAt);
			const uint8_t* const listPointer = Rel32Target(lists + kRayTracedPointerAt);
			Note("the ray traced lists at " + DescribeAddress(g_reads.rayTraced) + ", " + Hex(kRayTracedList) + " bytes each, their current pointer at " + DescribeAddress(listPointer) + At(P(kE3)));
			if (!DataOf<uint32_t>("ray traced list's count", g_reads.rayTraced, 1, kE3) || !DataOf<uint32_t>("second ray traced list's count", g_reads.rayTraced + kRayTracedList, 1, kE3) ||
				!InsideGame("ray traced lists' current pointer", listPointer, sizeof(void*), kE3))
				return false;

			const uint8_t* const checked = m.at[kF1];
			const uint8_t* const checkedPointer = Rel32Target(checked + kCheckedPointerAt);
			Note(LimitLine("the ray traced list's limit", checked + kRayTracedLimitAt, kRayTracedRoom, kF1) + ", its current pointer at " + DescribeAddress(checkedPointer));
			if (checkedPointer != listPointer)
				return Fail("the ray traced list's check reads the current pointer at " + DescribeAddress(checkedPointer) + ", not the one at " + DescribeAddress(listPointer) + At(P(kF1)) + ".");

			const uint8_t* const upload = m.at[kF2];
			const uint8_t* const builder = CallPastMatch("ray traced upload's builder call", upload + kUploadCallAt, kF2);
			if (!builder)
				return false;
			Note("the ray traced upload at " + DescribeAddress(upload) + ": " + std::to_string(kUploadRoom) + " lights at " + Hex(kUploadRecordsAt) + " of its object, the builder's call at " +
				DescribeAddress(upload + kUploadCallAt) + " -> " + DescribeAddress(builder) + At(P(kF2)));
			Note(LimitLine("the reflection lists' limit", m.at[kF3] + kReflectionLimitAt, kReflectionRoom, kF3));
			return true;
		}

		// G1, G2: the corona store's count, one for both of G1's matches. H1, H2: the worker's frame and a job's scratch in it, which a raise
		// of a job's lists will rest on.
		bool CheckCoronasAndWorker(const Matches& m)
		{
			g_reads.coronas = Rel32Target(m.at[kG1] + kCoronaCountRead);
			const uint8_t* const second = Rel32Target(m.secondG1 + kCoronaCountRead);
			Note("the corona store's two limits at " + DescribeAddress(m.at[kG1] + kCoronaLimitAt) + " and " + DescribeAddress(m.secondG1 + kCoronaLimitAt) + " hold " + std::to_string(kCoronaRoom - 1) + " (N-1, " + P(kG1).label + ") -> game size " +
				std::to_string(kCoronaRoom) + ", their counts at " + DescribeAddress(g_reads.coronas) + " and " + DescribeAddress(second));
			if (second != g_reads.coronas)
				return Fail("the corona store's two checks read different counts: " + DescribeAddress(g_reads.coronas) + " and " + DescribeAddress(second) + At(P(kG1)) + ".");
			if (!DataOf<uint32_t>("corona store's count", g_reads.coronas, 1, kG1))
				return false;
			Note("the drain's add to the corona store at " + DescribeAddress(m.at[kG2]) + ": its count at " + Hex(kCoronaCountAt) + " of the store, " + LimitLine("its limit", m.at[kG2] + kCoronaAddLimitAt, kCoronaRoom, kG2));
			Note("the worker's frame at " + DescribeAddress(m.at[kH1]) + " holds " + Hex(kWorkerFrame) + At(P(kH1)));
			Note("a job's scratch from the worker's rsp+" + Hex(kScratchAt) + " at " + DescribeAddress(m.at[kH2]) + At(P(kH2)));
			return true;
		}

		// ---- the measurement

		constexpr DWORD kSampleMs = 10;		// sees every frame up to 100 fps

		// The numbers sampled and what each has room for, in the order of the summary. The two stores' come from their write pointers.
		enum Number : size_t { kVehicles, kVehicleLights, kVehicleCoronas, kSceneLights, kRayTraced, kReflections, kCoronas, kNumbers };
		constexpr uint32_t kRoom[kNumbers] = { kVehiclesRoom, kStoreLights, kStoreCoronas, kSceneRoom, kRayTracedRoom, kReflectionRoom, kCoronaRoom };
		constexpr const char* kNames[kNumbers] = { "vehicles", "vehicle lights", "vehicle coronas", "scene lights", "ray traced list", "reflection lists", "coronas" };

		// The sampling thread's marks since the last reading, each number's most and how many samples saw what.
		struct Sampled
		{
			std::atomic<uint32_t> most[kNumbers] = {};
			std::atomic<uint64_t> samples{ 0 };
			std::atomic<uint64_t> markedAtRoom{ 0 };	// samples with the scene store at its room and the sort list marked: true whatever sets the mark
			std::atomic<uint64_t> pastUpload{ 0 };		// samples with the ray traced list past what its upload takes
		};
		Sampled g_sampled;
		std::atomic<bool> g_sampling{ false };			// the sampling thread runs

		enum class Counting { NotCounted, DryRun, Counted };
		std::atomic<const uint64_t*> g_refusedCounter{ nullptr };	// the stub's, set once the jump leads to it
		std::atomic<Counting> g_counting{ Counting::NotCounted };

		uint64_t ReadRefused()
		{
			const uint64_t* const counter = g_refusedCounter.load(std::memory_order_acquire);
			return counter ? Load<uint64_t>(counter) : 0;
		}

		// A number's first value past its room is a line of its own, at once, and no mark: what lies there then is no count of the game's.
		bool g_saidPast[kNumbers] = {};		// the sampling thread's alone
		void Keep(Number number, uint32_t value)
		{
			if (value > kRoom[number])
			{
				if (!g_saidPast[number])
				{
					g_saidPast[number] = true;
					Note(std::string(kNames[number]) + " seen at " + std::to_string(value) + ", past the " + std::to_string(kRoom[number]) + " the game has room for" +
						(number == kVehicles ? " - the game has overwritten memory." : "."));
				}
				return;
			}
			std::atomic<uint32_t>& most = g_sampled.most[number];
			if (value > most.load(std::memory_order_relaxed))	// a reading between the load and the store only moves this value to the next minute
				most.store(value, std::memory_order_relaxed);
		}

		// A store's write pointer gives its number; 0 means the game's reset has not run yet, and one outside its store means the jobs have
		// written past it: the crash is next.
		void KeepPointer(Number number, const uint8_t* pointerAt, const uint8_t* store, uint32_t bytes, const char* what)
		{
			const uint64_t pointer = Load<uint64_t>(pointerAt);
			if (pointer == 0)
				return;
			const uint64_t base = reinterpret_cast<uintptr_t>(store);
			if (pointer < base || pointer > base + uint64_t{ kRoom[number] } * bytes)
			{
				if (!g_saidPast[number])
				{
					g_saidPast[number] = true;
					Note(std::string(what) + " write pointer " + DescribeAddress(reinterpret_cast<const void*>(static_cast<uintptr_t>(pointer))) + " is outside its store - the game has overwritten memory.");
				}
				return;
			}
			Keep(number, static_cast<uint32_t>((pointer - base) / bytes));
		}

		// One sample: aligned reads of the game's data, which its threads may write meanwhile. Nothing written.
		void Sample()
		{
			const Reads& reads = g_reads;
			Keep(kVehicles, Load<uint32_t>(reads.vehicles));
			KeepPointer(kVehicleLights, reads.lightPointer, reads.lightStore, kLightBytes, "vehicle lights'");
			KeepPointer(kVehicleCoronas, reads.coronaPointer, reads.coronaStore, kCoronaBytes, "vehicle coronas'");
			const uint32_t scene = std::max(Load<uint32_t>(reads.renderCount), Load<uint32_t>(reads.sceneCount));
			Keep(kSceneLights, scene);
			const uint32_t rayTraced = std::max(Load<uint32_t>(reads.rayTraced), Load<uint32_t>(reads.rayTraced + kRayTracedList));
			Keep(kRayTraced, rayTraced);
			uint32_t reflections = 0;
			for (size_t i = 0; i < 3; i++)
				reflections = std::max({ reflections, Load<uint32_t>(reads.reflections + i * sizeof(uint32_t)), Load<uint32_t>(reads.reflections + kReflectionBuffer + i * sizeof(uint32_t)) });
			Keep(kReflections, reflections);
			Keep(kCoronas, Load<uint32_t>(reads.coronas));
			g_sampled.samples.fetch_add(1, std::memory_order_relaxed);	// first, and read last, so that a reading's others hold no more samples than it says
			if (scene == kSceneRoom && Load<uint8_t>(reads.sortMark) != 0)
				g_sampled.markedAtRoom.fetch_add(1, std::memory_order_relaxed);
			if (rayTraced > kUploadRoom && rayTraced <= kRayTracedRoom)
				g_sampled.pastUpload.fetch_add(1, std::memory_order_relaxed);
		}

		// The sampling thread: a sample every 10 ms, and a line of its own at the first light a job refused, so that the moment shows.
		// Each pass inside a try, so nothing leaves the thread; it never ends.
		DWORD WINAPI SampleNumbers(LPVOID)
		{
			bool firstRefusalSaid = false;
			for (;;)
			{
				Sleep(kSampleMs);
				try
				{
					Sample();
					if (!firstRefusalSaid && ReadRefused() != 0)
					{
						firstRefusalSaid = true;
						Note("the first light a job's list refused - the list is full, and the job's later vehicles lose lights in that frame.");
					}
				}
				catch (...)
				{
				}
			}
		}

		// The numbers of a stretch of time, as a summary says them.
		struct Marks
		{
			uint32_t most[kNumbers] = {};
			uint64_t samples = 0;
			uint64_t markedAtRoom = 0;
			uint64_t pastUpload = 0;
			uint64_t refused = 0;
		};

		// What the siren settings ask for, read at a summary (PATTERNS.md): how many there are, how many give a real light per lit siren,
		// and the most lights one of those has.
		struct Sirens
		{
			bool read = false;
			uint64_t settings = 0;
			uint64_t real = 0;
			uint32_t lights = 0;
		};

		// True if the settings, each through CopyMemorySafely, were read whole: the array may grow while packs load, so an array or a count
		// that changed meanwhile means another try at the next summary.
		bool ReadSirens(Sirens& sirens)
		{
			const std::optional<SirenSettingsArray> settings = SirenSettingsNow();
			if (!settings)
				return false;
			Sirens counted;
			counted.read = true;
			counted.settings = settings->count;
			uint8_t bytes[kSirenBytesRead];
			for (size_t i = 0; i < settings->count; i++)
			{
				if (!CopyMemorySafely(bytes, settings->array + i * settings->stride, sizeof(bytes)))
					return false;
				if (bytes[kRealLightsAt] == 0)
					continue;
				counted.real++;
				uint32_t lights = 0;
				for (size_t light = 0; light < kSirenLights; light++)
					if (bytes[kSirenLightsAt + light * kSirenLightBytes + kSirenLightFlagAt] != 0)
						lights++;
				counted.lights = std::max(counted.lights, lights);
			}
			const std::optional<SirenSettingsArray> again = SirenSettingsNow();
			if (!again || again->array != settings->array || again->count != settings->count)
				return false;
			sirens = counted;
			return true;
		}

		// The summary's state, kept by its thread alone: the stretch since the last reading, the whole session, the settings as last read,
		// how many readings had news, and whether a summary was written yet.
		Marks g_minute;
		Marks g_session;
		Sirens g_sirens;
		uint64_t g_news = 0;
		bool g_summaryWritten = false;

		bool SameNews(const Marks& a, const Marks& b)
		{
			return memcmp(a.most, b.most, sizeof(a.most)) == 0 && (a.markedAtRoom != 0) == (b.markedAtRoom != 0) && (a.pastUpload != 0) == (b.pastUpload != 0) && a.refused == b.refused;
		}

		// Changes with every reading that has something new to say: the first; a most of the minute or of the session other than the
		// last; lights refused; the store marked at its room or the upload passed where they were not; the settings read anew. How many samples
		// saw what is no news by itself.
		uint64_t Numbers()
		{
			Marks minute;
			for (size_t i = 0; i < kNumbers; i++)
				minute.most[i] = g_sampled.most[i].exchange(0, std::memory_order_relaxed);
			minute.markedAtRoom = g_sampled.markedAtRoom.exchange(0, std::memory_order_relaxed);
			minute.pastUpload = g_sampled.pastUpload.exchange(0, std::memory_order_relaxed);
			minute.samples = g_sampled.samples.exchange(0, std::memory_order_relaxed);
			const uint64_t refused = ReadRefused();
			minute.refused = refused - g_session.refused;

			Marks session = g_session;
			for (size_t i = 0; i < kNumbers; i++)
				session.most[i] = std::max(session.most[i], minute.most[i]);
			session.samples += minute.samples;
			session.markedAtRoom += minute.markedAtRoom;
			session.pastUpload += minute.pastUpload;
			session.refused = refused;

			Sirens sirens = g_sirens;
			ReadSirens(sirens);

			const bool news = g_news == 0 || !SameNews(minute, g_minute) || !SameNews(session, g_session) || sirens.read != g_sirens.read ||
				sirens.settings != g_sirens.settings || sirens.real != g_sirens.real || sirens.lights != g_sirens.lights;
			g_minute = minute;
			g_session = session;
			g_sirens = sirens;
			if (news)
				g_news++;
			return g_news;
		}

		std::string Said(const Marks& marks)
		{
			if (!g_sampling.load())
				return "its numbers are not sampled (see above), ";
			const auto number = [&marks](Number which)
			{
				return std::string(kNames[which]) + " " + std::to_string(marks.most[which]) + " of " + std::to_string(kRoom[which]);
			};
			const uint32_t rayTraced = marks.most[kRayTraced];
			return number(kVehicles) + ", " + number(kVehicleLights) + ", " + number(kVehicleCoronas) + ", " + number(kSceneLights) + " (at its room with the sort list marked in " +
				std::to_string(marks.markedAtRoom) + " of " + std::to_string(marks.samples) + " samples), " + number(kRayTraced) + " (past " + std::to_string(kUploadRoom) + " in " +
				std::to_string(marks.pastUpload) + " samples" + (rayTraced > kUploadRoom ? ", by " + std::to_string(rayTraced - kUploadRoom) + " at most" : std::string()) + "), " +
				number(kReflections) + ", " + number(kCoronas) + ", ";
		}

		std::string SaidRefused(const Marks& marks)
		{
			switch (g_counting.load())
			{
			case Counting::Counted:
				return "a job's refused lights " + std::to_string(marks.refused);
			case Counting::DryRun:
				return "a job's refused lights are not counted (DRY_RUN)";
			case Counting::NotCounted:
				break;
			}
			return "a job's refused lights are not counted (see above)";
		}

		// The first says the session so far; a later one the minute's first, which is what a scene reached, then the session.
		void LogSummary(uint64_t)
		{
			if (g_summaryWritten)
				Note("the most this minute: " + Said(g_minute) + SaidRefused(g_minute) + ".");
			g_summaryWritten = true;
			Note("the most so far: " + Said(g_session) + SaidRefused(g_session) + "; " + (g_sirens.read ? "siren settings " + std::to_string(g_sirens.settings) + ", real lights in " +
				std::to_string(g_sirens.real) + ", up to " + std::to_string(g_sirens.lights) + " lights." : std::string("siren settings not read yet.")));
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "lighting capacity summary", kFirstSummaryMs, &Numbers, &LogSummary };

		// ---- the counting stub

		// The light refused counted, then on to the function's way out - 22 bytes, every register as it was, rsp untouched, the counter
		// reached rip-relative.
		constexpr uint8_t kStub[] = {
			0xF0, 0x48, 0xFF, 0x05, 0, 0, 0, 0,		// lock inc qword [rip+disp32]: the light refused
			0xFF, 0x25, 0, 0, 0, 0,					// jmp qword [rip+0]: on to the function's way out,
			0, 0, 0, 0, 0, 0, 0, 0 };				// whose address follows
		constexpr size_t kCounterDispAt = 4, kCounterEnd = 8, kTargetAt = 14;
		static_assert(sizeof(kStub) == kTargetAt + sizeof(uint64_t), "TweaksV: the counting stub ends in its target");

		void NotCounted(const std::string& why)
		{
			Note("a job's refused lights are not counted: " + why);
		}

#ifdef TWEAKSV_TEST_BUILD
		// test builds only: TWEAKSV_TEST_LIGHT_STUB in the environment reaches what comes of a stub or a jump Windows refuses (SPECIFICATIONS 13.1): "no memory" takes
		// the near memory for none; "left writable" leaves the stub as it is written, unrun, so that under the fake's dynamic code policy the refusal comes at the jump's page.
		bool StubSwitch(const wchar_t* value, const char* words)
		{
			wchar_t text[16]{};
			if (GetEnvironmentVariableW(L"TWEAKSV_TEST_LIGHT_STUB", text, 16) >= 16 || wcscmp(text, value) != 0)
				return false;
			WriteLog(kTag, std::string("TEST BUILD: ") + words + " (TWEAKSV_TEST_LIGHT_STUB).");
			return true;
		}
#endif

		// The stub on the first page of a block near the game, its counter on its second, read/write; the first page read+execute once
		// written, never writable and executable at once. Null, logged, if it cannot be made.
		uint8_t* MakeStub(const uint8_t* wayOut, size_t page)
		{
			uint8_t* block = static_cast<uint8_t*>(AllocateNearGame(2 * page));
#ifdef TWEAKSV_TEST_BUILD
			if (block && StubSwitch(L"no memory", "the near memory is taken for none"))
			{
				VirtualFree(block, 0, MEM_RELEASE);
				block = nullptr;
			}
#endif
			if (!block)
			{
				NotCounted("no memory near the game could be had for its stub.");
				return nullptr;
			}
			memcpy(block, kStub, sizeof(kStub));
			const int32_t disp = static_cast<int32_t>((block + page) - (block + kCounterEnd));	// within one block of two pages: always within reach
			const uint64_t target = reinterpret_cast<uintptr_t>(wayOut);
			memcpy(block + kCounterDispAt, &disp, sizeof(disp));
			memcpy(block + kTargetAt, &target, sizeof(target));
#ifdef TWEAKSV_TEST_BUILD
			if (StubSwitch(L"left writable", "the stub is left writable"))
				return block;
#endif
			DWORD old = 0;
			if (!VirtualProtect(block, page, PAGE_EXECUTE_READ, &old))
			{
				const DWORD error = GetLastError();
				VirtualFree(block, 0, MEM_RELEASE);
				NotCounted("its stub could not be made read+execute (" + DescribeError(error) + ").");
				return nullptr;
			}
			FlushInstructionCache(GetCurrentProcess(), block, page);
			return block;
		}

		// The jump pointed at the stub. In DllMain, and with the game's frames not begun (MeasureLightCapacity): none of its threads runs it yet.
		bool RedirectJump(uint8_t* jump, const uint8_t* block)
		{
			const intptr_t distance = reinterpret_cast<intptr_t>(block) - reinterpret_cast<intptr_t>(jump + kJumpSize);
			if (distance < INT32_MIN || distance > INT32_MAX)
			{
				NotCounted("its stub is out of reach of the jump at " + DescribeAddress(jump) + ".");
				return false;
			}
			std::vector<uint8_t> bytes = { jump[0], jump[1] };
			const std::vector<uint8_t> rel = BytesOf(static_cast<int32_t>(distance));
			bytes.insert(bytes.end(), rel.begin(), rel.end());
			if (!WriteCode({ { jump, bytes, "jump when a job's list is full" } }, kTag))
			{
				NotCounted("the game code could not be changed (details above).");
				return false;
			}
			return true;
		}
	}

	void MeasureLightCapacity()
	{
		// 1. every site looked for, so that the log names each one missing
		Matches matches;
		std::string notFound;
		for (size_t i = 0; i < kSites; i++)
		{
			if (i != kG1)
			{
				matches.at[i] = Scanner::FindOnce(*kSitePatterns[i], notFound);
				continue;
			}
			const std::vector<uint8_t*> found = Scanner::FindAll(*kSitePatterns[i], kG1Matches + 1);
			if (found.size() == kG1Matches)
			{
				matches.at[i] = found[0];
				matches.secondG1 = found[1];
			}
			else if (notFound.empty())
				notFound = Scanner::NotAsOften(*kSitePatterns[i], found.size(), kG1Matches);
		}
		if (!notFound.empty())
		{
			Fail(notFound + ".");
			return;
		}

		// 2. every site checked, each logged - the first not as the findings have it ends here
		uint8_t* jump = nullptr;
		const uint8_t* wayOut = nullptr;
		if (!CheckFrameStores(matches) || !CheckJobs(matches) || !CheckAddToList(matches, jump, wayOut) || !CheckScratch(matches) || !CheckSceneStore(matches) ||
			!CheckRenderSides(matches) || !CheckCoronasAndWorker(matches))
			return;

		// 3. the stub made, the summary's thread and the sampling thread started, then the jump redirected: a measurement that cannot sum
		// up redirects nothing. A dry run or frames already running redirect nothing, and the numbers are read all the same.
		uint8_t* block = nullptr;
		const std::string site = DescribeAddress(jump) + At(P(kC1));
		if (Load<uint64_t>(g_reads.coronaPointer) != 0)	// the reset has run: the game's worker threads may run the jump, and a rel32 being written can be read half written
			NotCounted("the game's frames run already - its threads may run the jump while it is rewritten.");
		else if (IsDryRun())
		{
			g_counting = Counting::DryRun;
			WriteLog(kTag, "DRY_RUN: would redirect the jump at " + site + " to a counting stub - a job's refused lights are not counted.");
		}
		else
			block = MakeStub(wayOut, PageSize());
		if (!StartMeasurement<kMeasurement>())	// its line is written
		{
			if (block)
				VirtualFree(block, 0, MEM_RELEASE);	// no jump leads to it yet
			Fail("its summary thread could not be started.");
			return;
		}
		if (StartModuleThread(SampleNumbers, "lighting capacity sampling"))
			g_sampling = true;
		else
			Note("the lighting capacity's numbers are not sampled: their thread could not be started.");
		if (block)
		{
			if (RedirectJump(jump, block))
			{
				g_refusedCounter.store(reinterpret_cast<const uint64_t*>(block + PageSize()), std::memory_order_release);
				g_counting = Counting::Counted;
				Note("the jump at " + site + " is redirected to a counting stub: every light a job's full list refuses is counted.");
			}
			else
				VirtualFree(block, 0, MEM_RELEASE);	// no jump leads to it: WriteCode wrote nothing
		}
		Note(g_sampling.load() ? "the lighting capacity is measured: its numbers are sampled every 10 ms and logged after 10 seconds, then every minute while they change." :
			"the lighting capacity is measured, its numbers not sampled: the rest is logged after 10 seconds, then every minute while it changes.");
	}
}
