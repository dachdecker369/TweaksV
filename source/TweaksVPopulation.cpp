// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the two model budgets, measured first (see TweaksVPopulation.h).
//
// The game's population streaming, one object of its data, counts the bytes of the vehicle models and of the ped models it
// holds. Each budget is an entry of a table by a level, times the Population Variety's multiplier in story mode, the entry of
// another table alone in a network game: on 1.0.1158.16 with no gameconfig.xml block, 71.5 MB x 6 for the vehicles and 93.5 MB
// x 6 for the peds at level 3 and the slider's top. Short of the budget, by a margin, the game asks for no further model and
// releases what nothing uses, so that a load with many add-on models repeats those it holds; nothing is logged or shown
// (PATTERNS.md, "Population"). The module measures both, as one, since they share the object, its tables' set-up and the
// crash context's line that leads to them:
//   DIAGNOSTIC_MODE  the measurement, and nothing is looked for without it: every site found once and checked as a writing half
//                    would, logged, and read once a second by a thread beside the summary's: the level, the variety, the budget
//                    and what is in use, the models each list holds, how often the game stands at its stop, the pool VehicleStruct,
//                    the ambient cars, the peds of a model. 10 seconds in, and then every minute while a high or the samples at the
//                    stop change, one line per budget, its last words how long no high has moved, which is what a drive for its
//                    numbers goes by; and, as soon as the game has filled them and whenever they change, a line of the tables with
//                    the values of gameconfig.xml, which is the only proof that a raise there has taken
//   DRY_RUN          changes nothing here: nothing is written either way
// A pattern missing, a site not as expected or a thread that cannot start is logged, not reported - the measurement is an aid
// (ISSUES.md, T-DIAG) - and nothing more is measured. A writer of a story table besides the set-up's three stores would undo a
// raise and not the measurement, so it is a line and the measurement goes on.
// Written for TweaksV from the research's own findings in the game's code (PATTERNS.md); the idea of the two budgets, the key
// VehicleModelBudget and "fewer repeated ped models", is that of Resource Extensions [Enhanced] by Koukol, whose plugin was not read
// (CREDITS.md).

#include "TweaksVPopulation.h"

#include <windows.h>
#include <xmmintrin.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLock.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSummary.h"

#ifdef TWEAKSV_TEST_BUILD
#include <cwchar>
#endif

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "Population";
		constexpr DWORD kSampleMs = 1000;		// the state moves by one model a second at most

		// ---- the sites (PATTERNS.md, "Population")

		constexpr const GamePattern& kContext = Pattern("O-E-C1");			// the crash context's line: the object and both getters
		constexpr const GamePattern& kVehicleTables = Pattern("O-E-V1");	// the check before a vehicle model's request: the level, the flag, the tables
		constexpr const GamePattern& kVehicleCheck = Pattern("O-E-V2");		// the same check: the variety, the margin, the bytes in use
		constexpr const GamePattern& kVehiclePool = Pattern("O-E-V3");		// the release's test of the pool VehicleStruct
		constexpr const GamePattern& kSetUpHead = Pattern("O-E-V4");		// the tables' set-up, its head
		constexpr const GamePattern& kAmbientRead = Pattern("O-E-V5");		// the crash context's read of the ambient cars, right before O-E-C1
		constexpr const GamePattern& kSetUpCall = Pattern("O-E-V6");		// the one call of the set-up, which a writing half would redirect
		constexpr const GamePattern& kVehicleStores = Pattern("O-E-V7");	// the set-up's three stores of the vehicles' story entries
		constexpr const GamePattern& kPedSum = Pattern("O-E-P1");			// the update's sum of the ped memory
		constexpr const GamePattern& kPedGate = Pattern("O-E-P2");			// the gate before a ped model's request
		constexpr const GamePattern& kPedListsRead = Pattern("O-E-P3");		// the release's ped lists, and the model count
		constexpr const GamePattern& kPedsOfModel = Pattern("O-E-P4");		// the read of a model's peds
		constexpr const GamePattern* const kPedStores[3] = { &Pattern("O-E-P5"), &Pattern("O-E-P6"), &Pattern("O-E-P7") };	// the set-up's ped story entries
		constexpr const GamePattern& kPedVariety = Pattern("O-E-P8");		// the settings' store of the ped variety

		constexpr uint32_t kMargin = 1500000;	// what the game adds to a table's entry, and stays short of the budget by
		constexpr int64_t kMarginTakenOff = 0x100000000 - int64_t{ kMargin };	// the imm32 of the add that stays short of it

		// O-E-C1: "mov esi, [rip+disp32]", the ped memory's display (+2); "mov edi, [rip+disp32]", the vehicles' (+11); "lea rbx,
		// [rip+disp32]", the object (+21); each getter's call, the ped's (+28), the vehicle's (+41); "lea rcx, [rip+disp32]", the
		// line's text, its disp32 right past the match (+54).
		constexpr size_t kPedDisplayAt = 2, kVehicleDisplayAt = 11, kObjectAt = 21, kPedGetterCallAt = 28, kVehicleGetterCallAt = 41, kTextAt = 54;
		static_assert(PatternHolds(kContext, 0, { 0x8B, 0x35 }) && PatternHolds(kContext, 9, { 0x8B, 0x3D }) && PatternHolds(kContext, 18, { 0x48, 0x8D, 0x1D }) &&
			PatternHolds(kContext, kPedGetterCallAt, { 0xE8 }) && PatternHolds(kContext, kVehicleGetterCallAt, { 0xE8 }) &&
			PatternHolds(kContext, kTextAt - 3, { 0x48, 0x8D, 0x0D }) && PatternByte(kContext, kTextAt) == -1,
			"TweaksV: O-E-C1 holds the two displays' loads, the object's lea, the two getters' calls and the text's lea ending the match");
		constexpr char kContextText[] = "Ped Memory Use/Budget : %d/%d\n";

		// A getter, the same bytes for both, so that it is no pattern of the table but a check at its call's target:
		// the level (+2); "cmp byte [rip+disp32], 0", the network flag (+8, the imm8 behind its disp32); the network table (+18); the
		// story table (+35); the variety (+51). The doubling at its end is the crash line's alone.
		constexpr int16_t kGetter[] = { 0x8B, 0x05, -1, -1, -1, -1, 0x80, 0x3D, -1, -1, -1, -1, 0x00, 0x74, 0x11, 0x48, 0x8D, 0x0D, -1, -1, -1, -1,
			0x8B, 0x04, 0x81, 0xF3, 0x48, 0x0F, 0x2A, 0xC0, 0xEB, 0x17, 0x48, 0x8D, 0x0D, -1, -1, -1, -1, 0x8B, 0x04, 0x81, 0xF3, 0x48, 0x0F, 0x2A, 0xC0,
			0xF3, 0x0F, 0x59, 0x05, -1, -1, -1, -1, 0xF3, 0x0F, 0x2C, 0xC0, 0x01, 0xC0, 0xC3 };
		constexpr size_t kGetterBytes = sizeof(kGetter) / sizeof(kGetter[0]);
		constexpr size_t kGetterLevelAt = 2, kGetterFlagAt = 8, kGetterNetworkAt = 18, kGetterStoryAt = 35, kGetterVarietyAt = 51;
		static_assert(kGetterBytes == 62 && kGetter[kGetterLevelAt] == -1 && kGetter[kGetterFlagAt] == -1 && kGetter[kGetterFlagAt + 4] == 0x00 &&
			kGetter[kGetterNetworkAt] == -1 && kGetter[kGetterStoryAt] == -1 && kGetter[kGetterVarietyAt] == -1, "TweaksV: the getter's five operands");

		// O-E-V1, in the check before a vehicle model's request: the level (+2), the flag (+9), the story table (+16), the network table (+23).
		constexpr size_t kChecksLevelAt = 2, kChecksFlagAt = 9, kChecksStoryAt = 16, kChecksNetworkAt = 23;
		static_assert(PatternHolds(kVehicleTables, 0, { 0x8B, 0x05 }) && PatternHolds(kVehicleTables, 6, { 0x0F, 0xB6, 0x0D }) && PatternHolds(kVehicleTables, 13, { 0x4C, 0x8D, 0x35 }) &&
			PatternHolds(kVehicleTables, 20, { 0x48, 0x8D, 0x1D }), "TweaksV: O-E-V1 loads the level, the flag and the two tables");

		// O-E-V2: the variety's mulss (+16), the margin (+26), the use's compare (+32), the request's call, its rel32 right past the match (+42).
		constexpr size_t kChecksVarietyAt = 16, kRequestAt = 42;
		constexpr size_t kVehicleUsedAt = 0x6214;
		static_assert(PatternHolds(kVehicleCheck, 12, { 0xF3, 0x0F, 0x59, 0x05 }) && PatternImm32(kVehicleCheck, 26) == kMarginTakenOff && PatternImm32(kVehicleCheck, 32) == kVehicleUsedAt &&
			PatternHolds(kVehicleCheck, kRequestAt - 1, { 0xE8 }) && PatternByte(kVehicleCheck, kRequestAt) == -1,
			"TweaksV: O-E-V2 multiplies by the variety, adds the margin, compares the use at +0x6214 and ends with the request's call");

		// O-E-V3: "mov rax, [rip+disp32]", the pool's pointer (+3); its used at +0x28, the top two bits off by the shifts (+9), its size at
		// +0x18 (+18), the margin the game releases unused models from (+21).
		constexpr size_t kPoolPointerAt = 3, kPoolUsedAt = 0x28, kPoolSizeAt = 0x18;
		constexpr int32_t kPoolMargin = 20;
		static_assert(PatternByte(kVehiclePool, 9) == kPoolUsedAt && PatternHolds(kVehiclePool, 10, { 0xC1, 0xE5, 0x02, 0xC1, 0xFD, 0x02 }) && PatternByte(kVehiclePool, 18) == kPoolSizeAt &&
			PatternByte(kVehiclePool, 21) == kPoolMargin, "TweaksV: O-E-V3 reads the pool's used at +0x28 and its size at +0x18, with the margin 20");

		// O-E-V4, the set-up's head, 7 bytes behind its start: the configuration's holder (+3), its +0x18 (+10), the tunables (+14),
		// then the first of the twelve blocks of one entry, 0x38 bytes each: story peds, network peds, story vehicles, network vehicles.
		constexpr size_t kHeadAt = 7, kHolderAt = 3, kTunablesAt = 14, kFirstBlockAt = 18, kBlockBytes = 0x38;
		constexpr size_t kConfigurationAt = 0x18;
		constexpr int64_t kConfiguredAt[2] = { 0x50C, 0x4F4 };	// in the configuration: Level1Kb of the vehicles' and the peds' story mode, three dwords each
		static_assert(PatternByte(kSetUpHead, 10) == kConfigurationAt && PatternImm32(kSetUpHead, kFirstBlockAt + 2) == kConfiguredAt[1] &&
			PatternImm32(kVehicleStores, 2) == kConfiguredAt[0], "TweaksV: O-E-V4 reads the configuration at +0x18, its first block +0x4F4, and O-E-V7's +0x50C");
		static_assert(PatternImm32(*kPedStores[1], 2) == kConfiguredAt[1] + 4 && PatternImm32(*kPedStores[2], 2) == kConfiguredAt[1] + 8, "TweaksV: O-E-P6 and O-E-P7 read the next two");

		// O-E-V5: the ambient cars (+3), 27 bytes before O-E-C1, whose first two bytes end the match.
		constexpr size_t kAmbientAt = 3, kAmbientBeforeContext = 27;
		static_assert(PatternHolds(kAmbientRead, 0, { 0x44, 0x8B, 0x35 }) && PatternHolds(kAmbientRead, kAmbientBeforeContext, { 0x8B, 0x35 }) && PatternByte(kAmbientRead, kAmbientBeforeContext + 2) == -1,
			"TweaksV: O-E-V5 reads the ambient cars and ends with O-E-C1's first two bytes");

		// O-E-V6: the set-up's call, "call rel32" at +7.
		constexpr size_t kSetUpCallAt = 7;
		static_assert(PatternHolds(kSetUpCall, kSetUpCallAt, { 0xE8 }), "TweaksV: O-E-V6 holds the set-up's call at +7");

		// O-E-V7, three blocks, and O-E-P5 to P7, one each: each block's "mov [rip+disp32], eax", its disp32 at +45 of the block, past
		// the match in the last; the margin added before it at +39.
		constexpr size_t kBlockStoreAt = 45;
		constexpr size_t kVehicleStoreAt[3] = { kBlockStoreAt, kBlockBytes + kBlockStoreAt, 2 * kBlockBytes + kBlockStoreAt };
		static_assert(PatternImm32(kVehicleStores, 39) == kMargin && PatternImm32(kVehicleStores, kBlockBytes + 39) == kMargin && PatternImm32(kVehicleStores, 2 * kBlockBytes + 39) == kMargin &&
			PatternHolds(kVehicleStores, kVehicleStoreAt[0] - 2, { 0x89, 0x05 }) && PatternHolds(kVehicleStores, kVehicleStoreAt[1] - 2, { 0x89, 0x05 }) && PatternHolds(kVehicleStores, kVehicleStoreAt[2] - 2, { 0x89, 0x05 }) &&
			PatternByte(kVehicleStores, kVehicleStoreAt[2]) == -1, "TweaksV: O-E-V7 holds three blocks' margins and stores, the last store's disp32 past the match");
		static_assert(PatternImm32(*kPedStores[0], 39) == kMargin && PatternImm32(*kPedStores[1], 39) == kMargin && PatternImm32(*kPedStores[2], 39) == kMargin &&
			PatternHolds(*kPedStores[0], kBlockStoreAt - 2, { 0x89, 0x05 }) && PatternHolds(*kPedStores[1], kBlockStoreAt - 2, { 0x89, 0x05 }) && PatternHolds(*kPedStores[2], kBlockStoreAt - 2, { 0x89, 0x05 }) &&
			PatternByte(*kPedStores[0], kBlockStoreAt) == -1 && PatternByte(*kPedStores[1], kBlockStoreAt) == -1 && PatternByte(*kPedStores[2], kBlockStoreAt) == -1,
			"TweaksV: O-E-P5 to P7 each hold a block's margin and store, the store's disp32 past the match");
		static_assert(PatternHolds(*kPedStores[0], 0, { 0x69, 0xBB, 0xF4, 0x04 }) && PatternHolds(kSetUpHead, kFirstBlockAt, { 0x69, 0xBB, 0xF4, 0x04 }), "TweaksV: O-E-P5 is the block behind O-E-V4's head");

		// O-E-P1: the update's sum of the ped memory, its store's disp32 right past the match (+26).
		constexpr size_t kPedSumStoreAt = 26;
		constexpr size_t kPedUsedAt = 0x620C;
		static_assert(PatternImm32(kPedSum, 8) == kPedUsedAt && PatternHolds(kPedSum, kPedSumStoreAt - 2, { 0x89, 0x05 }) && PatternByte(kPedSum, kPedSumStoreAt) == -1,
			"TweaksV: O-E-P1 adds the use at +0x620C and stores the sum, its disp32 past the match");

		// O-E-P2, the gate: the override (+4) and the schedule's number (+16) that give the reserve, the use (+24), its load of the level
		// right past the match (+44); and further on, read after the check of its reach, the population's list read (+0x7A), the floor
		// of 6 models at level 3 (+0xEA) and the request's call (+0x102).
		constexpr size_t kOverrideAt = 0x6208, kScheduleAt = 0x6204;
		constexpr size_t kGateLevelAt = 44, kGateListAt = 0x7A, kGateFloorAt = 0xEA, kGateRequestAt = 0x102, kCallBytes = 5;
		constexpr uint8_t kGateList[] = { 0x41, 0x8B, 0x9C, 0xAC, 0x04, 0x4E, 0x00, 0x00 };
		constexpr uint8_t kGateFloor[] = { 0x83, 0xFD, 0x06, 0x0F, 0x9C, 0xC0, 0x83, 0xF9, 0x03, 0x0F, 0x94, 0xC1 };
		static_assert(PatternImm32(kPedGate, 4) == kOverrideAt && PatternImm32(kPedGate, 16) == kScheduleAt && PatternImm32(kPedGate, 24) == kPedUsedAt &&
			PatternImm32(kPedGate, 32) == kMarginTakenOff && PatternImm32(kPedGate, 38) == kMarginTakenOff && PatternHolds(kPedGate, kGateLevelAt - 2, { 0x8B, 0x0D }) && PatternByte(kPedGate, kGateLevelAt) == -1,
			"TweaksV: O-E-P2 reads the override, the schedule's number and the use, adds the two margins and ends with the level's load");

		// O-E-P3: the ped lists, 256 dwords each, the word of the model count right past the match (+46). The vehicles' lists are the
		// same shape; no pattern pins them, so a wrong one would be a wrong number (PATTERNS.md, "When Game Updates").
		constexpr size_t kModelCountAt = 46;
		constexpr size_t kListEntries = 256;
		constexpr uint32_t kListFree = 0xFFFF;
		constexpr size_t kPopulationListAt = 0x4E04, kPedOutsideListAt = 0x5204, kPedNoGroupListAt = 0x5604, kPedDirectListAt = 0x5E04;
		static_assert(PatternImm32(kPedListsRead, 3) == kPopulationListAt && PatternImm32(kPedListsRead, 10) == kPedNoGroupListAt && PatternImm32(kPedListsRead, 17) == kPedOutsideListAt &&
			PatternImm32(kPedListsRead, 24) == kPedDirectListAt && PatternHolds(kPedListsRead, kModelCountAt - 3, { 0x0F, 0xB7, 0x05 }) && PatternByte(kPedListsRead, kModelCountAt) == -1,
			"TweaksV: O-E-P3 names the four ped lists and ends with the model count's load");

		// O-E-P4: the model info table (+3), its stride (+13), a model's peds at +0x158 (+23). A model's streamed flag beside it.
		constexpr size_t kModelTableAt = 3, kModelStrideAt = 13, kPedsAt = 0x158, kModelFlagsAt = 0x286;
		constexpr uint8_t kStreamedBit = 0x02;
		static_assert(PatternHolds(kPedsOfModel, 0, { 0x48, 0x8B, 0x0D }) && PatternHolds(kPedsOfModel, 9, { 0x48, 0x0F, 0xAF, 0x05 }) && PatternImm32(kPedsOfModel, 23) == kPedsAt,
			"TweaksV: O-E-P4 loads the table and its stride and reads a model's peds at +0x158");

		// O-E-P8: the settings' store of the ped variety, right past the match (+76), after the vectors of its factor (+11) and its
		// sum (+18), and its most (+26).
		constexpr size_t kFactorAt = 11, kAddedAt = 18, kMostAt = 26, kVarietyStoreAt = 76;
		static_assert(PatternHolds(kPedVariety, kFactorAt - 3, { 0x0F, 0x59, 0x05 }) && PatternHolds(kPedVariety, kAddedAt - 3, { 0x0F, 0x58, 0x05 }) && PatternHolds(kPedVariety, kMostAt - 4, { 0xF3, 0x0F, 0x10, 0x0D }) &&
			PatternHolds(kPedVariety, kVarietyStoreAt - 4, { 0xF3, 0x0F, 0x11, 0x1D }) && PatternByte(kPedVariety, kVarietyStoreAt) == -1,
			"TweaksV: O-E-P8 multiplies, adds, loads the most and stores the variety, its disp32 past the match");

		// The object's fields no pattern pins (PATTERNS.md, "When Game Updates"): only logged, so a wrong one is a wrong number.
		constexpr size_t kVehicleOverAt = 0x6224, kPedOverAt = 0x621C;
		constexpr size_t kModsAt[2] = { 0x6234, 0x6238 };
		constexpr size_t kWeaponsAt = 0x623C, kInFlightAt = 0x62BC;
		constexpr size_t kObjectBytes = kInFlightAt + sizeof(uint32_t);	// the object's fields read, from its start

		constexpr size_t kWritersLooked = 16;		// the writers of a story table the scan looks for: the set-up's three, and more besides them
		constexpr float kTopVariety = 6.0f;			// the Population Variety's multiplier at the slider's top
		constexpr uint32_t kGameLevels = 3;			// the top level; a level above it is a moment of change
		constexpr int32_t kMegabyte = 1000000;		// a high counts in whole MB, so that noise moves none

		enum { kVehicles, kPeds, kBudgets };
		constexpr const char* kNames[kBudgets] = { "vehicle", "ped" };
		constexpr size_t kUsedAt[kBudgets] = { kVehicleUsedAt, kPedUsedAt };
		constexpr size_t kOverAt[kBudgets] = { kVehicleOverAt, kPedOverAt };
		constexpr uint32_t kGameKb[kBudgets][3] = { { 32000, 56000, 70000 }, { 54000, 72000, 92000 } };	// the configuration's constructor's defaults on 1.0.1158.16, which no pattern pins

		// The lists a budget's models are filed in, each with its words for the log: the population's or traffic's first.
		struct ListAt
		{
			size_t at;
			const char* words;
		};
		constexpr size_t kMostLists = 5;
		constexpr ListAt kLists[kBudgets][kMostLists] = {
			{ { 0x3600, "for traffic" }, { 0x3A00, "outside the pop cycle" }, { 0x3E00, "trains and helicopters" }, { 0x4200, "boats" }, { 0x4A00, "dropped" } },
			{ { kPopulationListAt, "for the population" }, { kPedOutsideListAt, "outside the schedule" }, { kPedNoGroupListAt, "in no group" }, { kPedDirectListAt, "asked for directly" }, { 0, nullptr } },
		};

		// What the sites lead to, set before the measurement's threads start.
		struct BudgetSites
		{
			const uint8_t* level = nullptr;
			const uint8_t* story = nullptr;		// 4 dwords, the network table's 4 right behind them
			const uint8_t* network = nullptr;
			const uint8_t* variety = nullptr;
		};
		struct Sites
		{
			const uint8_t* object = nullptr;
			const uint8_t* flag = nullptr;			// a network game in progress, a byte
			BudgetSites budgets[kBudgets];
			const uint8_t* pool = nullptr;			// VehicleStruct's pointer
			const uint8_t* ambient = nullptr;
			const uint8_t* holder = nullptr;		// the configuration's holder
			const uint8_t* modelCount = nullptr;	// a word
			const uint8_t* modelTable = nullptr;
			const uint8_t* modelStride = nullptr;
		};
		Sites g_sites;

		// ---- the samples, kept under one lock: the sampling thread's and the summary's

		enum class PoolRead { NotMade, Unreadable, Read };

		struct BudgetState
		{
			uint64_t samples = 0;				// samples of this budget: a level above the top makes none
			uint64_t atStop = 0;				// of them, those at or past the stop
			uint32_t oddLevel = 0;				// the level the last sample found above the top, else 0
			uint32_t level = 0;					// the last sample's ...
			uint32_t tables[8] = {};			// the story table's 4 entries and the network table's
			float variety = 0;
			uint32_t entry = 0;
			int32_t budget = 0;
			int32_t stop = 0;
			int32_t used = 0;
			int32_t over = 0;
			uint32_t lists[kMostLists] = {};
			uint32_t models = 0;
			uint32_t lowestLevel = UINT32_MAX;	// and what the samples have seen
			float highestVariety = 0;
			int32_t mostUsed = 0;
			uint32_t mostModels = 0;
			uint64_t lastHighMs = 0;			// when one of this budget's highs last moved, or its first sample
		};
		struct State
		{
			bool unreadable = false;		// the last sample could not read the game's data
			uint64_t fillSeenMs = 0;		// when a sample first found the object's override not 0, with none since that found it 0
			bool filled = false;			// the game has run its set-up
			bool network = false;
			BudgetState budgets[kBudgets];
			uint32_t inFlight = 0;			// the vehicles' ...
			int32_t weapons = 0;
			int32_t mods[2] = {};
			PoolRead pool = PoolRead::NotMade;
			int32_t poolUsed = 0;
			int32_t poolSize = 0;
			int32_t mostPoolUsed = 0;
			int32_t ambient = 0;
			int32_t mostAmbient = 0;
			uint32_t streamed = 0;			// the peds': the population's models with the streamed flag, and those not read
			uint32_t unreadModels = 0;
			uint32_t mostStreamed = 0;
			int32_t mostPeds = 0;
		};
		SRWLOCK g_lock = SRWLOCK_INIT;
		State g_state;

		// Each field aligned, as the checks hold it, so read whole; nothing written.
		template <typename T> T Field(size_t at)
		{
			return Load<T>(g_sites.object + at);
		}

		// A list ends at a free entry, or a value not below the model count.
		bool EndsList(uint32_t index, uint32_t modelCount)
		{
			return index == kListFree || index >= modelCount;
		}

		// The entries of a list, up to its end.
		uint32_t ListCount(size_t at, uint32_t modelCount)
		{
			uint32_t count = 0;
			while (count < kListEntries && !EndsList(Field<uint32_t>(at + count * sizeof(uint32_t)), modelCount))
				count++;
			return count;
		}

		// The budget as the game's getter has it without its doubling: the cast of a float, whose overflow gives the lowest int.
		int32_t BudgetOf(uint32_t entry, float variety, bool network)
		{
			const float bytes = static_cast<float>(static_cast<int64_t>(entry));
			return _mm_cvttss_si32(_mm_set_ss(network ? bytes : bytes * variety));
		}

		// The game's data, each read checked once a sample, as the ray tracing buffer's.
		bool DataIsReadable()
		{
			const Sites& s = g_sites;
			if (!IsReadable(s.object, kObjectBytes) || !IsReadable(s.flag, sizeof(uint8_t)) || !IsReadable(s.pool, sizeof(uint64_t)) || !IsReadable(s.ambient, sizeof(int32_t)) ||
				!IsReadable(s.modelCount, sizeof(uint16_t)) || !IsReadable(s.modelTable, sizeof(uint64_t)) || !IsReadable(s.modelStride, sizeof(uint64_t)))
				return false;
			for (const BudgetSites& budget : s.budgets)
				if (!IsReadable(budget.level, sizeof(uint32_t)) || !IsReadable(budget.story, 2 * 4 * sizeof(uint32_t)) || !IsReadable(budget.variety, sizeof(float)))
					return false;
			return true;
		}

		// One budget's sample: a level above the top is taken for a moment of change, and makes none. reserve is what the game
		// stays short of the budget by; the stop wraps as the game's add does, so that a budget the cast overflowed never reads as reached.
		void SampleBudget(size_t index, bool network, uint32_t reserve, uint32_t modelCount, uint64_t now, BudgetState& b)
		{
			const BudgetSites& sites = g_sites.budgets[index];
			const uint32_t level = Load<uint32_t>(sites.level);
			if (level > kGameLevels)
			{
				b.oddLevel = level;
				return;
			}
			b.oddLevel = 0;
			b.level = level;
			for (size_t i = 0; i < 8; i++)
				b.tables[i] = Load<uint32_t>(sites.story + i * sizeof(uint32_t));
			b.variety = Load<float>(sites.variety);
			b.entry = b.tables[(network ? 4 : 0) + level];
			b.budget = BudgetOf(b.entry, b.variety, network);
			b.stop = static_cast<int32_t>(static_cast<uint32_t>(b.budget) - reserve);
			b.used = Field<int32_t>(kUsedAt[index]);
			b.over = Field<int32_t>(kOverAt[index]);
			b.models = 0;
			for (size_t i = 0; i < kMostLists; i++)
			{
				b.lists[i] = kLists[index][i].words ? ListCount(kLists[index][i].at, modelCount) : 0;
				b.models += b.lists[i];
			}
			if (b.samples++ == 0)
				b.lastHighMs = now;
			if (b.used >= b.stop)
				b.atStop++;
			if (level < b.lowestLevel)
				b.lowestLevel = level;
			if (b.variety > b.highestVariety)
				b.highestVariety = b.variety;
			if (b.used > b.mostUsed)
			{
				if (b.used / kMegabyte > b.mostUsed / kMegabyte)
					b.lastHighMs = now;
				b.mostUsed = b.used;
			}
			if (b.models > b.mostModels)
			{
				b.mostModels = b.models;
				b.lastHighMs = now;
			}
		}

		// VehicleStruct, on the game's heap: its used with the top two bits off, as the game's shifts take them, and its size.
		void SamplePool(uint64_t now, State& state)
		{
			const uint8_t* const pool = Load<const uint8_t*>(g_sites.pool);
			if (!pool)
			{
				state.pool = PoolRead::NotMade;
				return;
			}
			uint32_t used = 0;
			int32_t size = 0;
			if (!CopyMemorySafely(&used, pool + kPoolUsedAt, sizeof(used)) || !CopyMemorySafely(&size, pool + kPoolSizeAt, sizeof(size)))
			{
				state.pool = PoolRead::Unreadable;
				return;
			}
			state.pool = PoolRead::Read;
			state.poolUsed = static_cast<int32_t>(used << 2) >> 2;
			state.poolSize = size;
			if (state.poolUsed > state.mostPoolUsed)
			{
				state.mostPoolUsed = state.poolUsed;
				state.budgets[kVehicles].lastHighMs = now;
			}
		}

		// The population's models, each through its model info on the game's heap: its peds and its streamed flag. Each entry is read
		// once and its end test goes by what was read, since the game may release a model meanwhile; a pointer that fails skips that
		// model. A value past the model count would read another table's bytes as a pointer, and no access violation need come of it.
		void SampleModels(uint32_t modelCount, uint64_t now, State& state)
		{
			const uint8_t* const table = Load<const uint8_t*>(g_sites.modelTable);
			const uint64_t stride = Load<uint64_t>(g_sites.modelStride);
			state.streamed = 0;
			state.unreadModels = 0;
			for (uint32_t i = 0; i < kListEntries; i++)
			{
				const uint32_t index = Field<uint32_t>(kPopulationListAt + i * sizeof(uint32_t));
				if (EndsList(index, modelCount))
					break;
				const uint8_t* info = nullptr;
				int32_t peds = 0;
				uint8_t flags = 0;
				if (!CopyMemorySafely(&info, table + uint64_t{ index } * stride, sizeof(info)) || !CopyMemorySafely(&peds, info + kPedsAt, sizeof(peds)) ||
					!CopyMemorySafely(&flags, info + kModelFlagsAt, sizeof(flags)))
				{
					state.unreadModels++;
					continue;
				}
				if (flags & kStreamedBit)
					state.streamed++;
				if (peds > state.mostPeds)
				{
					state.mostPeds = peds;
					state.budgets[kPeds].lastHighMs = now;
				}
			}
			if (state.streamed > state.mostStreamed)
			{
				state.mostStreamed = state.streamed;
				state.budgets[kPeds].lastHighMs = now;
			}
		}

		// True once the game has laid out the lists a sample counts. A list of zeros would count 256 entries of model 0, in highs that
		// nothing lowers. A laid-out list holds an entry that is not 0 whether or not it starts with model 0, its free ones 0xFFFF.
		bool ListsAreLaidOut()
		{
			for (const auto& lists : kLists)
				for (const ListAt& list : lists)
				{
					bool laidOut = !list.words;
					for (size_t i = 0; i < kListEntries && !laidOut; i++)
						laidOut = Field<uint32_t>(list.at + i * sizeof(uint32_t)) != 0;
					if (!laidOut)
						return false;
				}
			return true;
		}

		// One sample of both budgets, with the lock held. Before the game's init the object is zeros, and the tables hold other
		// numbers, which must not read as a change: its override, -1 after that, tells, though the tables' set-up comes last in the
		// init and the findings do not give when the override is stored before it, nor the lists. So the fill counts from a second
		// sample running that finds the override not 0 and the lists laid out, half a sampling period or more after the first.
		void TakeSample()
		{
			State& state = g_state;
			state.unreadable = !DataIsReadable();
			if (state.unreadable)
				return;
			const uint64_t now = GetTickCount64();
			const int32_t override = Field<int32_t>(kOverrideAt);
			if (!state.filled)
			{
				if (override == 0 || !ListsAreLaidOut())
					state.fillSeenMs = 0;
				else if (state.fillSeenMs == 0)
					state.fillSeenMs = now;
				if (state.fillSeenMs == 0 || now - state.fillSeenMs < kSampleMs / 2)
					return;
				state.filled = true;
			}
			state.network = Load<uint8_t>(g_sites.flag) != 0;
			const uint32_t modelCount = Load<uint16_t>(g_sites.modelCount);
			const int32_t scenarioPeds = override < 0 ? Field<int32_t>(kScheduleAt) : override;
			SampleBudget(kVehicles, state.network, kMargin, modelCount, now, state.budgets[kVehicles]);
			SampleBudget(kPeds, state.network, kMargin + static_cast<uint32_t>(scenarioPeds >> 1) * kMargin, modelCount, now, state.budgets[kPeds]);
			state.inFlight = Field<uint32_t>(kInFlightAt);
			state.weapons = Field<int32_t>(kWeaponsAt);
			state.mods[0] = Field<int32_t>(kModsAt[0]);
			state.mods[1] = Field<int32_t>(kModsAt[1]);
			state.ambient = Load<int32_t>(g_sites.ambient);
			if (state.ambient > state.mostAmbient)
			{
				state.mostAmbient = state.ambient;
				state.budgets[kVehicles].lastHighMs = now;
			}
			SamplePool(now, state);
			SampleModels(modelCount, now, state);
		}

		// The sampling thread: a sample a second. Each pass inside a try, so nothing leaves the thread; it never ends.
		DWORD WINAPI SampleBudgets(LPVOID)
		{
			for (;;)
			{
				Sleep(kSampleMs);
				try
				{
					const ExclusiveLock lock(g_lock);
					TakeSample();
				}
				catch (...)
				{
				}
			}
		}

		// What the summary has said, on its thread alone: a table's line that could not read the configuration is still to come.
		bool g_tablesSaid[kBudgets] = {};
		uint32_t g_saidTables[kBudgets][8] = {};
		bool g_tablesUnread[kBudgets] = {};
		uint64_t g_tablesTries = 0;
		bool g_varietySaid[kBudgets] = {};

		// How long no high may move before one more line says so: two minutes, two summaries' periods, in a test build by
		// TWEAKSV_TEST_SUMMARY_MS too, so that a suite sees it within the minute a fake game runs.
		uint64_t NoHighLineMs()
		{
#ifdef TWEAKSV_TEST_BUILD
			wchar_t text[8]{};
			const DWORD length = GetEnvironmentVariableW(L"TWEAKSV_TEST_SUMMARY_MS", text, 8);
			if (length > 0 && length < 8)
			{
				const long ms = wcstol(text, nullptr, 10);
				if (ms >= 100 && ms <= 60000)
					return 2 * static_cast<uint64_t>(ms);
			}
#endif
			return 2 * uint64_t{ kSummaryEveryMs };
		}

		// A sample of its own, so that a summary is fresh and comes without the sampling thread too; then what changes a summary:
		// whether it could read and the game has filled the tables, each budget's highs and its time at the stop, whether its highs
		// have stood still past NoHighLineMs, so that a drive's last line says so, its level, its tables and its variety, the rest in
		// whole MB or as counted; and every time, while a table's line is still to come.
		uint64_t Numbers()
		{
			const ExclusiveLock lock(g_lock);
			TakeSample();
			const State& state = g_state;
			const uint64_t now = GetTickCount64();
			uint64_t hash = 14695981039346656037ull;
			const auto mix = [&hash](uint64_t value) { hash = (hash ^ value) * 1099511628211ull; };
			mix(state.unreadable);
			mix(state.filled);
			mix(state.network);
			for (const BudgetState& b : state.budgets)
			{
				mix(b.atStop);
				mix(b.samples != 0 && now - b.lastHighMs > NoHighLineMs());
				mix(b.level);
				mix(b.lowestLevel);
				mix(std::bit_cast<uint32_t>(b.variety));
				mix(b.mostUsed / kMegabyte);
				mix(b.mostModels);
				for (const uint32_t entry : b.tables)
					mix(entry);
			}
			mix(static_cast<uint64_t>(state.pool));
			mix(static_cast<uint64_t>(state.mostPoolUsed));
			mix(static_cast<uint64_t>(state.mostAmbient));
			mix(state.mostStreamed);
			mix(state.unreadModels);
			mix(static_cast<uint64_t>(state.mostPeds));
			if (g_tablesUnread[kVehicles] || g_tablesUnread[kPeds])
				g_tablesTries++;
			mix(g_tablesTries);
			return hash;
		}

		// ---- the lines

		// Bytes as MB of 1,000,000, as the game's defaults are decimal, with one decimal: "428.7".
		std::string Megabytes(int64_t bytes)
		{
			const int64_t tenths = (bytes < 0 ? bytes - 50000 : bytes + 50000) / 100000;
			const int64_t size = tenths < 0 ? -tenths : tenths;
			return (tenths < 0 ? "-" : "") + std::to_string(size / 10) + "." + std::to_string(size % 10);
		}

		// A multiplier with two decimals: "6.00".
		std::string Decimals(float value)
		{
			const long long hundredths = std::llround(static_cast<double>(value) * 100);
			const long long size = hundredths < 0 ? -hundredths : hundredths;
			const long long rest = size % 100;
			return (hundredths < 0 ? "-" : "") + std::to_string(size / 100) + (rest < 10 ? ".0" : ".") + std::to_string(rest);
		}

		// "a, b and c"
		std::string Three(const uint32_t* values)
		{
			return std::to_string(values[0]) + ", " + std::to_string(values[1]) + " and " + std::to_string(values[2]);
		}

		// The three values of gameconfig.xml a story table's entries come from; on the game's heap, behind the configuration's holder.
		bool ReadConfigured(size_t index, uint32_t (&values)[3])
		{
			if (!IsReadable(g_sites.holder, sizeof(uint64_t)))
				return false;
			const uint8_t* const holder = Load<const uint8_t*>(g_sites.holder);
			const uint8_t* configuration = nullptr;
			return holder && CopyMemorySafely(&configuration, holder + kConfigurationAt, sizeof(configuration)) && configuration &&
				CopyMemorySafely(values, configuration + kConfiguredAt[index], sizeof(values));
		}

		// The tables, whenever they change: all eight entries in bytes, and whether the story table's are what gameconfig.xml gives -
		// the only proof that a raise there has taken, since a misspelled tag is ignored without a word. False while the configuration
		// cannot be read, so that the line comes again, the proof with it.
		bool LogTables(size_t index, const BudgetState& b)
		{
			std::string line = "DIAGNOSTIC: " + std::string(kNames[index]) + " model budget's tables: story " + std::to_string(b.tables[0]) + ", " + std::to_string(b.tables[1]) + ", " +
				std::to_string(b.tables[2]) + " and " + std::to_string(b.tables[3]) + " bytes, network " + std::to_string(b.tables[4]) + ", " + std::to_string(b.tables[5]) + ", " +
				std::to_string(b.tables[6]) + " and " + std::to_string(b.tables[7]) + " bytes";
			uint32_t configured[3] = {};
			if (!ReadConfigured(index, configured))
			{
				WriteLog(kTag, line + "; the configuration at " + DescribeAddress(g_sites.holder) + " could not be read this time.");
				return false;
			}
			line += "; the configuration's story Level1Kb to Level3Kb: " + Three(configured);
			for (uint32_t level = 1; level <= kGameLevels; level++)
			{
				const uint32_t expected = configured[level - 1] * 1000 + kMargin;
				if (b.tables[level] != expected)
				{
					WriteLog(kTag, line + " - level " + std::to_string(level) + "'s entry holds " + std::to_string(b.tables[level]) + " bytes, not the " + std::to_string(expected) +
						" they give: a tunable or another mod has changed it, or the game's set-up has not run yet.");
					return true;
				}
			}
			if (memcmp(configured, kGameKb[index], sizeof(configured)) == 0)
				WriteLog(kTag, line + ", the game's defaults - the story entries are theirs.");
			else
				WriteLog(kTag, line + ", not the defaults of 1.0.1158.16, " + Three(kGameKb[index]) + " - the story entries are theirs: gameconfig.xml, or a game update, has changed them.");
			return true;
		}

		// A budget's line up to its models: what is in use of what, and where the game stops asking.
		std::string InUse(size_t index, const BudgetState& b, bool network)
		{
			const std::string entry = (network ? "the network table's " : "the level's ") + Megabytes(b.entry) + " MB" + (network ? ", no variety" : " x variety " + Decimals(b.variety));
			std::string line = "DIAGNOSTIC: " + std::string(kNames[index]) + " model budget: " + Megabytes(b.used);
			if (b.budget < 0)
				line += " MB in use at level " + std::to_string(b.level) + " (" + entry + ", which the game's cast to int overflows: nothing stops its requests)";
			else
				line += " of " + Megabytes(b.budget) + " MB in use at level " + std::to_string(b.level) + " (" + entry + "), the requests stop at " + Megabytes(b.stop) + " MB";
			return line + ", over the budget by " + Megabytes(b.over) + " MB, at most " + Megabytes(b.mostUsed) + " MB so far; ";
		}

		// A budget's line after its models: how long the game stands at its stop, and the lowest level a script has set.
		std::string AtStop(const BudgetState& b)
		{
			std::string line = "; at the stop in " + std::to_string(b.atStop) + " of " + std::to_string(b.samples) + " samples";
			if (b.lowestLevel < b.level)
				line += "; at level " + std::to_string(b.lowestLevel) + " at the lowest so far";
			return line;
		}

		// A budget's last words: how long none of its highs has moved, which is what a drive for its numbers goes by.
		std::string NoNewHigh(const BudgetState& b)
		{
			return "; no new high for " + std::to_string((GetTickCount64() - b.lastHighMs) / 1000) + " s.";
		}

		void LogVehicles(const State& state)
		{
			const BudgetState& b = state.budgets[kVehicles];
			std::string line = InUse(kVehicles, b, state.network) + "vehicle models loaded: ";
			for (size_t i = 0; i < kMostLists; i++)
				line += std::to_string(b.lists[i]) + " " + kLists[kVehicles][i].words + ", ";
			line += std::to_string(state.inFlight) + " in flight, at most " + std::to_string(b.mostModels) + " at once";
			if (b.models)
				line += ", about " + Megabytes((int64_t{ b.used } - state.weapons) / b.models) + " MB each";
			line += AtStop(b) + "; VehicleStruct ";
			if (state.pool == PoolRead::NotMade)
				line += "not made yet";
			else if (state.pool == PoolRead::Unreadable)
				line += "could not be read this time";
			else
				line += std::to_string(state.poolUsed) + " of " + std::to_string(state.poolSize) + " slots, at most " + std::to_string(state.mostPoolUsed) +
					" so far, unused models released past " + std::to_string(state.poolSize - kPoolMargin);
			WriteLog(kTag, line + "; of the bytes: weapons " + Megabytes(state.weapons) + " MB, mods " + Megabytes(state.mods[0]) + " and " + Megabytes(state.mods[1]) +
				" MB; ambient cars on the map " + std::to_string(state.ambient) + ", at most " + std::to_string(state.mostAmbient) + " so far" + NoNewHigh(b));
		}

		void LogPeds(const State& state)
		{
			const BudgetState& b = state.budgets[kPeds];
			std::string line = InUse(kPeds, b, state.network) + "ped models loaded: " + std::to_string(b.lists[0]) + " " + kLists[kPeds][0].words + ", " +
				std::to_string(state.streamed) + " of them streamed";
			for (size_t i = 1; kLists[kPeds][i].words; i++)
				line += ", " + std::to_string(b.lists[i]) + " " + kLists[kPeds][i].words;
			line += ", at most " + std::to_string(b.mostModels) + " at once";
			if (b.models)
				line += ", about " + Megabytes(int64_t{ b.used } / b.models) + " MB each";
			line += AtStop(b) + "; at most " + std::to_string(state.mostPeds) + " peds of one model; at most " + std::to_string(state.mostStreamed) + " streamed at once";
			if (state.unreadModels)
				line += "; " + std::to_string(state.unreadModels) + " model infos could not be read this time";
			WriteLog(kTag, line + NoNewHigh(b));
		}

		void LogSummary(uint64_t)
		{
			State state;
			{
				const ExclusiveLock lock(g_lock);
				state = g_state;
			}
			if (state.unreadable)
			{
				WriteLog(kTag, "DIAGNOSTIC: the model budgets at " + DescribeAddress(g_sites.object) + " could not be read this time.");
				return;
			}
			if (!state.filled)
			{
				WriteLog(kTag, "DIAGNOSTIC: the model budgets are not filled yet - the game has not started its population.");
				return;
			}
			for (size_t index = 0; index < kBudgets; index++)
			{
				const BudgetState& b = state.budgets[index];
				if (b.samples == 0)
				{
					WriteLog(kTag, "DIAGNOSTIC: " + std::string(kNames[index]) + " model budget: no sample yet - its level reads " + std::to_string(b.oddLevel) + ", above the game's " +
						std::to_string(kGameLevels) + ".");
					continue;
				}
				if (!g_tablesSaid[index] || memcmp(g_saidTables[index], b.tables, sizeof(b.tables)) != 0)
				{
					g_tablesUnread[index] = !LogTables(index, b);
					g_tablesSaid[index] = !g_tablesUnread[index];
					memcpy(g_saidTables[index], b.tables, sizeof(b.tables));
				}
				if (!g_varietySaid[index] && b.highestVariety > kTopVariety)
				{
					g_varietySaid[index] = true;
					WriteLog(kTag, "DIAGNOSTIC: " + std::string(kNames[index]) + " model budget: the variety has been " + Decimals(b.highestVariety) + ", above the " + Decimals(kTopVariety) +
						" of the game's slider - settings.xml or another mod has changed it.");
				}
				if (index == kVehicles)
					LogVehicles(state);
				else
					LogPeds(state);
			}
		}

		// The first summary 10 seconds in, then one a minute while the numbers change.
		constexpr Measurement kMeasurement = { "population summary", kFirstSummaryMs, &Numbers, &LogSummary };

		// ---- the sites, found and checked

		void NotMeasured(const std::string& why)
		{
			WriteLog(kTag, "DIAGNOSTIC: the model budgets are not measured: " + why);
		}

		// True if the count values of T at at lie inside the game, aligned to their size, so that Load reads each whole, and can be
		// read; else why not.
		template <typename T> bool DataOf(const std::string& what, const uint8_t* at, size_t count, const GamePattern& pattern, std::string& why)
		{
			const std::string fault = DataFault(at, count * sizeof(T), alignof(T));
			if (fault.empty())
				return true;
			why = "the " + what + " at " + DescribeAddress(at) + At(pattern) + " " + fault + ".";
			return false;
		}

		// True if a site reads what the game's own code at another site reads, else why not.
		bool Same(const std::string& what, const uint8_t* at, const uint8_t* expected, const std::string& reader, const GamePattern& pattern, std::string& why)
		{
			if (at == expected)
				return true;
			why = "the " + what + " at " + DescribeAddress(at) + At(pattern) + " is not the one " + reader + " reads, at " + DescribeAddress(expected) + ".";
			return false;
		}

		// A getter, at its call's target in the crash context's line: its bytes and its five operands, the network table right behind the story table.
		bool CheckGetter(size_t index, const uint8_t* call, const uint8_t*& flag, std::string& why)
		{
			const std::string name = std::string(kNames[index]) + " budget's";
			const uint8_t* const getter = CallTarget(call);
			if (!IsInsideGameImage(getter))
			{
				why = "the " + name + " getter's call at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" + At(kContext) + ".";
				return false;
			}
			if (!DataOf<uint8_t>(name + " getter", getter, kGetterBytes, kContext, why))
				return false;
			for (size_t i = 0; i < kGetterBytes; i++)
				if (kGetter[i] >= 0 && getter[i] != kGetter[i])
				{
					why = "the " + name + " getter at " + DescribeAddress(getter) + At(kContext) + " is not as found: " + HexBytes(getter, kGetterBytes) + ".";
					return false;
				}
			BudgetSites& budget = g_sites.budgets[index];
			budget.level = Rel32Target(getter + kGetterLevelAt);
			flag = Rel32Target(getter + kGetterFlagAt) + 1;	// the cmp's imm8 lies behind its disp32
			budget.network = Rel32Target(getter + kGetterNetworkAt);
			budget.story = Rel32Target(getter + kGetterStoryAt);
			budget.variety = Rel32Target(getter + kGetterVarietyAt);
			if (budget.network != budget.story + 4 * sizeof(uint32_t))
			{
				why = "the " + name + " network table at " + DescribeAddress(budget.network) + At(kContext) + " does not lie right behind its story table at " + DescribeAddress(budget.story) + ".";
				return false;
			}
			if (!DataOf<uint32_t>(name + " level", budget.level, 1, kContext, why) || !DataOf<uint8_t>(name + " network flag", flag, 1, kContext, why) ||
				!DataOf<uint32_t>(name + " tables", budget.story, 2 * 4, kContext, why) || !DataOf<float>(name + " variety", budget.variety, 1, kContext, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: " + name + " getter at " + DescribeAddress(getter) + ": level at " + DescribeAddress(budget.level) + ", story mode's table at " + DescribeAddress(budget.story) +
				", network's at " + DescribeAddress(budget.network) + ", variety at " + DescribeAddress(budget.variety) + ", network flag at " + DescribeAddress(flag) + At(kContext));
			return true;
		}

		// "RVA 0x1, RVA 0x2 and RVA 0x3"
		std::string ThreeAddresses(const uint8_t* const (&addresses)[3])
		{
			return DescribeAddress(addresses[0]) + ", " + DescribeAddress(addresses[1]) + " and " + DescribeAddress(addresses[2]);
		}

		// Check W2 of a writing half: a story table's three stores, each at its entry, the first entry being 1. The disp32s given, past a match or not.
		bool CheckStores(size_t index, const uint8_t* const (&disps)[3], const std::string& labels, std::string& why)
		{
			const uint8_t* const story = g_sites.budgets[index].story;
			const uint8_t* stores[3] = {};
			const uint8_t* entries[3] = {};
			for (size_t i = 0; i < 3; i++)
			{
				stores[i] = disps[i] - 2;	// "mov [rip+disp32], eax" starts two bytes before its disp32
				entries[i] = Rel32Target(disps[i]);
				if (entries[i] != story + (i + 1) * sizeof(uint32_t))
				{
					why = "the " + std::string(kNames[index]) + " story table's store at " + DescribeAddress(stores[i]) + " leads to " + DescribeAddress(entries[i]) + ", not to its entry " +
						std::to_string(i + 1) + " at " + DescribeAddress(story + (i + 1) * sizeof(uint32_t)) + " (" + labels + ").";
					return false;
				}
			}
			WriteLog(kTag, "DIAGNOSTIC: " + std::string(kNames[index]) + " story entries' stores at " + ThreeAddresses(stores) + " -> " + ThreeAddresses(entries) + " (" + labels + ")");
			return true;
		}

		// Check W3 of a writing half: the scan for stores of any width into a story table, which should find those three and no other
		// (SPECIFICATIONS 8). A writer besides them would undo a raise, which is none of a measurement's, so it is a line and nothing
		// stops. The scan skips what it cannot read, so that one which misses a store the patterns pin proves nothing of the others.
		void LogWriters(size_t index, const uint8_t* const (&disps)[3], const std::string& labels)
		{
			const uint8_t* const story = g_sites.budgets[index].story;
			const std::vector<uint8_t*> writers = Scanner::FindRipRelativeStores(story, 4 * sizeof(uint32_t), kWritersLooked);
			const uint8_t* const stores[3] = { disps[0] - 2, disps[1] - 2, disps[2] - 2 };
			size_t found = 0;
			const uint8_t* missed = nullptr;
			for (const uint8_t* const store : stores)
				if (std::find(writers.begin(), writers.end(), store) != writers.end())
					found++;
				else if (!missed)
					missed = store;
			if (missed && writers.size() < kWritersLooked)
			{
				WriteLog(kTag, "DIAGNOSTIC: writers of the " + std::string(kNames[index]) + " story table at " + DescribeAddress(story) + ": the scan finds " + std::to_string(found) +
					" of the set-up's 3 stores, not the one at " + DescribeAddress(missed) + ", so it proves nothing about other writers (" + labels + ")");
				return;
			}
			size_t others = 0;
			std::string besides;
			for (const uint8_t* const writer : writers)
				if (writer != stores[0] && writer != stores[1] && writer != stores[2])
					besides += (others++ == 0 ? "" : ", ") + DescribeAddress(writer);
			WriteLog(kTag, "DIAGNOSTIC: writers of the " + std::string(kNames[index]) + " story table at " + DescribeAddress(story) + ": the set-up's stores at " + ThreeAddresses(stores) + ", " +
				(others == 0 ? std::string("none besides them") : std::to_string(others) + (writers.size() == kWritersLooked ? " or more" : "") + " besides them: " + besides) + " (" + labels + ")");
		}

		// Every site found and checked as a writing half would, each logged; false at the first that is not as the research found it, and why.
		bool FindAndCheck(std::string& why)
		{
			// 1. every pattern looked for, so that the log names each one missing; then what is read past a match, in one check
			const uint8_t* const context = Scanner::FindOnce(kContext, why);
			const uint8_t* const vehicleTables = Scanner::FindOnce(kVehicleTables, why);
			const uint8_t* const vehicleCheck = Scanner::FindOnce(kVehicleCheck, why);
			const uint8_t* const vehiclePool = Scanner::FindOnce(kVehiclePool, why);
			const uint8_t* const setUpHead = Scanner::FindOnce(kSetUpHead, why);
			const uint8_t* const ambientRead = Scanner::FindOnce(kAmbientRead, why);
			const uint8_t* const setUpCall = Scanner::FindOnce(kSetUpCall, why);
			const uint8_t* const vehicleStores = Scanner::FindOnce(kVehicleStores, why);
			const uint8_t* const pedSum = Scanner::FindOnce(kPedSum, why);
			const uint8_t* const pedGate = Scanner::FindOnce(kPedGate, why);
			const uint8_t* const pedLists = Scanner::FindOnce(kPedListsRead, why);
			const uint8_t* const pedsOfModel = Scanner::FindOnce(kPedsOfModel, why);
			const uint8_t* const pedStores[3] = { Scanner::FindOnce(*kPedStores[0], why), Scanner::FindOnce(*kPedStores[1], why), Scanner::FindOnce(*kPedStores[2], why) };
			const uint8_t* const pedVariety = Scanner::FindOnce(kPedVariety, why);
			if (!context || !vehicleTables || !vehicleCheck || !vehiclePool || !setUpHead || !ambientRead || !setUpCall || !vehicleStores || !pedSum || !pedGate || !pedLists || !pedsOfModel ||
				!pedStores[0] || !pedStores[1] || !pedStores[2] || !pedVariety)
			{
				why += ".";
				return false;
			}
			const struct
			{
				const uint8_t* match;
				const GamePattern& pattern;
				size_t reach;
			} pastMatch[] = { { context, kContext, kTextAt + sizeof(int32_t) }, { vehicleCheck, kVehicleCheck, kRequestAt + sizeof(int32_t) },
				{ vehicleStores, kVehicleStores, kVehicleStoreAt[2] + sizeof(int32_t) }, { pedSum, kPedSum, kPedSumStoreAt + sizeof(int32_t) },
				{ pedGate, kPedGate, kGateRequestAt + kCallBytes }, { pedLists, kPedListsRead, kModelCountAt + sizeof(int32_t) },
				{ pedStores[0], *kPedStores[0], kBlockStoreAt + sizeof(int32_t) }, { pedStores[1], *kPedStores[1], kBlockStoreAt + sizeof(int32_t) },
				{ pedStores[2], *kPedStores[2], kBlockStoreAt + sizeof(int32_t) }, { pedVariety, kPedVariety, kVarietyStoreAt + sizeof(int32_t) } };
			for (const auto& site : pastMatch)
				if (!IsReadable(site.match, site.reach))
				{
					why = "the code past " + std::string(site.pattern.label) + "'s match at " + DescribeAddress(site.match) + " cannot be read.";
					return false;
				}

			// 2. the crash context's line: its text, the object, and the two getters, which give both budgets' data
			const uint8_t* const text = Rel32Target(context + kTextAt);
			if (!DataOf<char>("crash context's text", text, sizeof(kContextText), kContext, why))
				return false;
			if (memcmp(text, kContextText, sizeof(kContextText)) != 0)
			{
				why = "the crash context's text at " + DescribeAddress(text) + At(kContext) + " is not \"Ped Memory Use/Budget : %d/%d\".";
				return false;
			}
			g_sites.object = Rel32Target(context + kObjectAt);
			if (!DataOf<uint32_t>("population object", g_sites.object, kObjectBytes / sizeof(uint32_t), kContext, why))
				return false;
			const uint8_t* const pedDisplay = Rel32Target(context + kPedDisplayAt);
			WriteLog(kTag, "DIAGNOSTIC: crash context's line at " + DescribeAddress(context) + ": its text at " + DescribeAddress(text) + ", the population object at " + DescribeAddress(g_sites.object) +
				", the memories it prints at " + DescribeAddress(pedDisplay) + " and " + DescribeAddress(Rel32Target(context + kVehicleDisplayAt)) + At(kContext));
			const uint8_t* flags[kBudgets] = {};
			if (!CheckGetter(kPeds, context + kPedGetterCallAt, flags[kPeds], why) || !CheckGetter(kVehicles, context + kVehicleGetterCallAt, flags[kVehicles], why))
				return false;
			if (!Same("network flag", flags[kVehicles], flags[kPeds], "the ped budget's getter", kContext, why))
				return false;
			g_sites.flag = flags[kPeds];
			BudgetSites& vehicles = g_sites.budgets[kVehicles];
			BudgetSites& peds = g_sites.budgets[kPeds];

			// 3. the check before a vehicle model's request reads what the vehicle getter reads
			const std::string getter = "the vehicle budget's getter";
			if (!Same("level", Rel32Target(vehicleTables + kChecksLevelAt), vehicles.level, getter, kVehicleTables, why) ||
				!Same("network flag", Rel32Target(vehicleTables + kChecksFlagAt), g_sites.flag, getter, kVehicleTables, why) ||
				!Same("story table", Rel32Target(vehicleTables + kChecksStoryAt), vehicles.story, getter, kVehicleTables, why) ||
				!Same("network table", Rel32Target(vehicleTables + kChecksNetworkAt), vehicles.network, getter, kVehicleTables, why) ||
				!Same("variety", Rel32Target(vehicleCheck + kChecksVarietyAt), vehicles.variety, getter, kVehicleCheck, why))
				return false;
			const uint8_t* const request = Rel32Target(vehicleCheck + kRequestAt);
			if (!IsInsideGameImage(request))
			{
				why = "the vehicle model's request at " + DescribeAddress(vehicleCheck + kRequestAt - 1) + " is not a call into the game (another mod may have hooked it already)" + At(kVehicleCheck) + ".";
				return false;
			}
			WriteLog(kTag, "DIAGNOSTIC: vehicle budget's check at " + DescribeAddress(vehicleTables) + At(kVehicleTables) + " and " + DescribeAddress(vehicleCheck) + ": in use at +" + Hex(kVehicleUsedAt) +
				" of the object, the margin " + std::to_string(kMargin) + ", the request at " + DescribeAddress(request) + At(kVehicleCheck));

			// 4. the pool VehicleStruct's pointer and the ambient cars, right before the crash context's line
			g_sites.pool = Rel32Target(vehiclePool + kPoolPointerAt);
			if (!DataOf<uint64_t>("VehicleStruct pool's pointer", g_sites.pool, 1, kVehiclePool, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: VehicleStruct pool's pointer at " + DescribeAddress(g_sites.pool) + ": its used at +" + Hex(kPoolUsedAt) + ", its size at +" + Hex(kPoolSizeAt) +
				", unused models released from " + std::to_string(kPoolMargin) + " below it" + At(kVehiclePool));
			g_sites.ambient = Rel32Target(ambientRead + kAmbientAt);
			if (ambientRead + kAmbientBeforeContext != context)
			{
				why = "the ambient cars' read at " + DescribeAddress(ambientRead) + At(kAmbientRead) + " does not lie right before the crash context's line at " + DescribeAddress(context) + ".";
				return false;
			}
			if (!DataOf<int32_t>("ambient cars' count", g_sites.ambient, 1, kAmbientRead, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: ambient cars on the map at " + DescribeAddress(g_sites.ambient) + At(kAmbientRead));

			// 5. check W1 of a writing half: the set-up's one call, to the function that holds the head and the blocks of the entries
			const uint8_t* const setUp = setUpHead - kHeadAt;
			const uint8_t* const call = setUpCall + kSetUpCallAt;
			const uint8_t* const target = CallTarget(call);
			if (!IsInsideGameImage(target))
			{
				why = "the tables' set-up call at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" + At(kSetUpCall) + ".";
				return false;
			}
			if (target != setUp)
			{
				why = "the tables' set-up call at " + DescribeAddress(call) + " leads to " + DescribeAddress(target) + ", not to the set-up at " + DescribeAddress(setUp) + At(kSetUpHead) + ".";
				return false;
			}
			std::string detail;
			if (!IsFunctionStart(setUp, detail))
			{
				why = "the tables' set-up at " + DescribeAddress(setUp) + " is not a function start (" + detail + ")" + At(kSetUpHead) + ".";
				return false;
			}
			if (Scanner::FindCallsTo(setUp, 2).size() != 1)
			{
				why = "the game code calls the tables' set-up at " + DescribeAddress(setUp) + " in 2 or more places, not the expected 1" + At(kSetUpCall) + ".";
				return false;
			}
			const struct
			{
				const uint8_t* match;
				const GamePattern& pattern;
				size_t block;
			} blocks[] = { { pedStores[0], *kPedStores[0], 0 }, { pedStores[1], *kPedStores[1], 1 }, { pedStores[2], *kPedStores[2], 2 }, { vehicleStores, kVehicleStores, 6 } };
			for (const auto& block : blocks)
				if (block.match != setUpHead + kFirstBlockAt + block.block * kBlockBytes)
				{
					why = std::string(block.pattern.label) + " at " + DescribeAddress(block.match) + " does not lie in the tables' set-up, where its block " + std::to_string(block.block) + " is, at " +
						DescribeAddress(setUpHead + kFirstBlockAt + block.block * kBlockBytes) + ".";
					return false;
				}
			g_sites.holder = Rel32Target(setUpHead + kHolderAt);
			if (!DataOf<uint64_t>("configuration's holder", g_sites.holder, 1, kSetUpHead, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: tables' set-up call at " + DescribeAddress(call) + " -> " + DescribeAddress(setUp) + " (" + detail + "), its only call; the configuration's holder at " +
				DescribeAddress(g_sites.holder) + ", the tunables at " + DescribeAddress(Rel32Target(setUpHead + kTunablesAt)) + " (" + kSetUpHead.label + ", " + kSetUpCall.label + ")");

			// 6. check W2 of a writing half: the two story tables' stores
			const uint8_t* const vehicleDisps[3] = { vehicleStores + kVehicleStoreAt[0], vehicleStores + kVehicleStoreAt[1], vehicleStores + kVehicleStoreAt[2] };
			const uint8_t* const pedDisps[3] = { pedStores[0] + kBlockStoreAt, pedStores[1] + kBlockStoreAt, pedStores[2] + kBlockStoreAt };
			const std::string vehicleLabel = kVehicleStores.label;
			const std::string pedLabels = std::string(kPedStores[0]->label) + ", " + kPedStores[1]->label + ", " + kPedStores[2]->label;
			if (!CheckStores(kVehicles, vehicleDisps, vehicleLabel, why) || !CheckStores(kPeds, pedDisps, pedLabels, why))
				return false;

			// 7. the ped budget's sites: the sum's store, the gate, the lists and their model count, a model's peds, the variety's store
			if (!Same("ped memory's display", Rel32Target(pedSum + kPedSumStoreAt), pedDisplay, "the crash context's line", kPedSum, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: ped memory's sum at " + DescribeAddress(pedSum) + " -> " + DescribeAddress(pedDisplay) + ", the gate reading its +" + Hex(kPedUsedAt) + " alone" + At(kPedSum));
			if (!Same("ped budget's level", Rel32Target(pedGate + kGateLevelAt), peds.level, "the ped budget's getter", kPedGate, why))
				return false;
			if (memcmp(pedGate + kGateListAt, kGateList, sizeof(kGateList)) != 0)
			{
				why = "the ped budget's gate at " + DescribeAddress(pedGate) + " holds " + HexBytes(pedGate + kGateListAt, sizeof(kGateList)) + " at +" + Hex(kGateListAt) + ", not " + HexBytes(kGateList, sizeof(kGateList)) + At(kPedGate) + ".";
				return false;
			}
			if (memcmp(pedGate + kGateFloorAt, kGateFloor, sizeof(kGateFloor)) != 0)
			{
				why = "the ped budget's gate at " + DescribeAddress(pedGate) + " holds " + HexBytes(pedGate + kGateFloorAt, sizeof(kGateFloor)) + " at +" + Hex(kGateFloorAt) + ", not " + HexBytes(kGateFloor, sizeof(kGateFloor)) + At(kPedGate) + ".";
				return false;
			}
			const uint8_t* const pedRequest = CallTarget(pedGate + kGateRequestAt);
			if (!IsInsideGameImage(pedRequest))
			{
				why = "the ped model's request at " + DescribeAddress(pedGate + kGateRequestAt) + " is not a call into the game (another mod may have hooked it already)" + At(kPedGate) + ".";
				return false;
			}
			WriteLog(kTag, "DIAGNOSTIC: ped budget's gate at " + DescribeAddress(pedGate) + ": in use at +" + Hex(kPedUsedAt) + " of the object, the reserve from +" + Hex(kOverrideAt) + " or +" + Hex(kScheduleAt) +
				", the request at " + DescribeAddress(pedRequest) + At(kPedGate));
			g_sites.modelCount = Rel32Target(pedLists + kModelCountAt);
			if (!DataOf<uint16_t>("model count", g_sites.modelCount, 1, kPedListsRead, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: ped lists' read at " + DescribeAddress(pedLists) + ": the lists at +" + Hex(kPopulationListAt) + ", +" + Hex(kPedOutsideListAt) + ", +" + Hex(kPedNoGroupListAt) + " and +" + Hex(kPedDirectListAt) +
				" of the object, the model count at " + DescribeAddress(g_sites.modelCount) + At(kPedListsRead));
			g_sites.modelTable = Rel32Target(pedsOfModel + kModelTableAt);
			g_sites.modelStride = Rel32Target(pedsOfModel + kModelStrideAt);
			if (!DataOf<uint64_t>("model info table", g_sites.modelTable, 1, kPedsOfModel, why) || !DataOf<uint64_t>("model info table's stride", g_sites.modelStride, 1, kPedsOfModel, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: a model's peds at " + DescribeAddress(pedsOfModel) + ": the model info table at " + DescribeAddress(g_sites.modelTable) + ", its stride at " + DescribeAddress(g_sites.modelStride) +
				", the peds at +" + Hex(kPedsAt) + At(kPedsOfModel));
			if (!Same("ped budget's variety", Rel32Target(pedVariety + kVarietyStoreAt), peds.variety, "the ped budget's getter", kPedVariety, why))
				return false;
			const uint8_t* const factor = Rel32Target(pedVariety + kFactorAt);
			const uint8_t* const added = Rel32Target(pedVariety + kAddedAt);
			const uint8_t* const most = Rel32Target(pedVariety + kMostAt);
			if (!DataOf<float>("ped variety's factor", factor, 1, kPedVariety, why) || !DataOf<float>("ped variety's sum", added, 1, kPedVariety, why) || !DataOf<float>("ped variety's most", most, 1, kPedVariety, why))
				return false;
			WriteLog(kTag, "DIAGNOSTIC: ped variety's store at " + DescribeAddress(pedVariety + kVarietyStoreAt - 4) + " -> " + DescribeAddress(peds.variety) + ": the setting x " + Decimals(ValueAt<float>(factor)) + " + " +
				Decimals(ValueAt<float>(added)) + ", at most " + Decimals(ValueAt<float>(most)) + At(kPedVariety));

			// 8. check W3 of a writing half: a line
			LogWriters(kVehicles, vehicleDisps, vehicleLabel);
			LogWriters(kPeds, pedDisps, pedLabels);
			return true;
		}
	}

	void MeasurePopulation()
	{
		std::string why;
		if (!FindAndCheck(why))
		{
			NotMeasured(why);
			return;
		}
		if (!StartMeasurement<kMeasurement>())	// its line is written
		{
			NotMeasured("its summary thread could not be started.");
			return;
		}
		const bool sampled = StartModuleThread(SampleBudgets, "population sampling");	// its line is written; each summary takes a sample of its own
		WriteLog(kTag, "DIAGNOSTIC: the model budgets, in the population object at " + DescribeAddress(g_sites.object) + At(kContext) + (sampled ? ": sampled every second" :
			": sampled once a summary alone, their sampling thread not started") + ", nothing written; their tables are logged once the game has filled them and whenever they change, "
			"each budget's line after 10 seconds, then every minute while its highs or its samples at the stop change.");
	}
}
