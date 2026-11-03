// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the sirens per vehicle: the lights past 20, and the measurement that came first (see TweaksVSirenCount.h).
//
// The game draws a vehicle's siren lights in one function, the siren lights' renderer, which the vehicle
// lights' function calls once a frame for each vehicle whose sirens are on. It lights 20 sirens at most: its
// loop runs over 20 lights, and a model's bone structure names the bones of siren1 to siren20 and of their
// glass, siren_glass1 to siren_glass20, as signed bytes - so a model made for more sirens keeps the bones past
// siren20 dark, without an error, and a bone past index 127 cannot be named at all (PATTERNS.md,
// "SirenSettings").
//   SIRENS_PER_VEHICLE  above 20, the raise, every structure of the game staying at 20, the lights past 20 by a second call
//                    of the game's own renderer (D23). On a game build its renderer was read on (D40) and with the light
//                    stores raised (D39) every site is found and checked, with those of the damage (TweaksVSirenDamage), and
//                    all of it is one write. The game's parser drops a siren setting's item 21 and on: a hook on the array
//                    member's tree reader reads up to SIRENS_PER_VEHICLE - 20 more, by the game's own reader of an element, into
//                    a siren setting of TweaksV's own, made by the game's constructor in zeroed memory, kept under a hash of the
//                    20 lights the game did keep, in a table of 255, each published whole; the later of two with the same 20
//                    lights and others past them replaces the first. A vehicle's siren state grows by a second state and a
//                    stand-in vehicle behind the first. The wrapper of the vehicle lights' call of the renderer calls the game's
//                    renderer as ever, then looks the model up in the table of 1,024 and, for a model with an own setting and
//                    bones past siren20 in the room behind its bone structure (TweaksVSirenRoom.h), calls it again with the
//                    stand-in, the second state and a block of its own that stands in for the model, so that the renderer's
//                    light 1 is siren21. Both calls of the siren settings getter, the renderer's and the vehicle lights', are redirected
//                    to one plain function: a block gets its own setting, position 0 none, a position below the array's count that entry, and
//                    any other, an ID that found no setting, an empty setting that lights nothing, so that the game never reads past its
//                    array (D41). A table that is full is reported, once, and what does not fit stays at 20.
// Before the part raised anything, the module measured whether a mod load reaches the limit, and it still does:
//   DIAGNOSTIC_MODE  the measurement, and nothing is looked for without it: the vehicle lights' call of the
//                    renderer goes through TweaksV's call stub to a wrapper that calls the game's renderer
//                    first, with the game's arguments, and then counts the vehicle's model once, in a fixed
//                    table of 1,024 models: on its first pass the siren and glass bones its bone structure
//                    names and 64 lookups by name - siren1 to siren32 and siren_glass1 to siren_glass32 -
//                    through the game's own find-bone-by-name, on every later pass one lookup in the table;
//                    10 seconds after the first model is counted, and then every minute while the numbers
//                    change, the log says how many models were seen, which carry siren bones past siren20
//                    that the game never lights and which lose siren bones past bone 127
//                    With the raise the wrapper is the raise's, the call redirected once, and the summary says too which
//                    models the second call lights, what the tables hold and what did not fit them.
//   DRY_RUN          the call is not redirected and nothing is counted: the line that would redirect it
// Every site is found and checked first: the call must lead into the game, to the function the renderer's bone
// read lies in by the game's unwind data, and find-bone-by-name's match must be a function start by it too; each
// lookup by name is guarded against an access violation, since no pattern pins what it is handed. For the raise
// each of these is the part's problem, nothing written; for the measurement a pattern missing, a site not as
// expected, a summary thread that cannot start or a call that cannot be redirected is logged, not reported - the
// measurement is an aid (ISSUES.md, T-DIAG) - and nothing is redirected.
// Written for TweaksV from the research's own findings in the game's code (PATTERNS.md); the idea of more sirens comes
// from SirenSetting Limit Adjuster by cp702 - none of its code was read or used (CREDITS.md).

#include "TweaksVSirenCount.h"

#include <windows.h>

#include <array>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVGame.h"
#include "TweaksVLightCapacity.h"
#include "TweaksVLock.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSirenDamage.h"
#include "TweaksVSirenIds.h"
#include "TweaksVSirenRoom.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "SirenSettings";

		// ---- the sites (PATTERNS.md, "SirenSettings", the siren count)

		constexpr const GamePattern& kCallPattern = Pattern("I-E-B1");		// the vehicle lights' call of the siren lights' renderer
		constexpr const GamePattern& kBoneReadPattern = Pattern("I-E-B2");	// the renderer's read of a siren's bone
		constexpr const GamePattern& kGlassReadPattern = Pattern("I-E-B3");	// a read of a siren's glass bone
		constexpr const GamePattern& kFindBonePattern = Pattern("I-E-B4");	// the start of find-bone-by-name
		constexpr size_t kRendererCallAt = 13;		// in I-E-B1's match: "call rel32" of the renderer, the call redirected
		constexpr size_t kGetterCallAt = 24;		// in I-E-B1's match: "call rel32" of the siren settings getter, its rel32 past the match
		constexpr size_t kCallSize = 5;
		static_assert(PatternByte(kCallPattern, 0) == 0x48 && PatternByte(kCallPattern, 1) == 0x89 && PatternByte(kCallPattern, 2) == 0x6C &&
			PatternByte(kCallPattern, 3) == 0x24 && PatternByte(kCallPattern, 4) == 0x28 && PatternByte(kCallPattern, 21) == 0x48 &&
			PatternByte(kCallPattern, 22) == 0x89 && PatternByte(kCallPattern, 23) == 0xE9,
			"TweaksV: I-E-B1 hands the renderer rbp as its 6th argument (mov [rsp+28h], rbp) and the getter the same (mov rcx, rbp): the model info");
		static_assert(PatternByte(kCallPattern, kRendererCallAt) == 0xE8 && PatternByte(kCallPattern, kGetterCallAt) == 0xE8,
			"TweaksV: I-E-B1 holds the renderer's call at +13 and the getter's at +24");

		// The model info leads through two pointers from its +0xB0 to the bone structure, whose signed byte at 0x10 +
		// a hierarchy ID is the index of that ID's bone, or below 0 for none: siren n's at 0x98 + n, its glass's at
		// 0xAC + n. find-bone-by-name takes the skeleton data - the skeleton's first field - and a name.
		constexpr size_t kBonesAt = 0xB0;			// in the model info: the pointer to the bone structure's holder (I-E-B2, +3)
		constexpr size_t kSirenBonesAt = 0x99;		// in the bone structure: siren1's byte, then siren2's ... siren20's (I-E-B2, +24)
		constexpr size_t kGlassBonesAt = 0xAD;		// in the bone structure: siren_glass1's byte ... siren_glass20's (I-E-B3, +4)
		constexpr size_t kBoneCountAt = 0x5E;		// in the skeleton data: the number of bones, a uint16 (I-E-B4, +18)
		constexpr size_t kNameHashAt = 0x18;		// in the model info: the hash of the model's name, which names it in the log
		constexpr size_t kBoneIndexAt = 0x42;		// in the bone find-bone-by-name returns: its index, a uint16
		constexpr size_t kBonesRead = 3;			// in I-E-B2's match: the disp32 of "mov rax, [rdi+disp32]", kBonesAt
		constexpr size_t kSirenBonesRead = 24;		// in I-E-B2's match: the disp32 of "movsx eax, byte [rax+rbp+disp32]", kSirenBonesAt
		constexpr size_t kGlassBonesRead = 4;		// in I-E-B3's match: the disp32 of "movsx edx, byte [rax+rbx+disp32]", kGlassBonesAt
		constexpr size_t kBoneCountRead = 18;		// in I-E-B4's match: the disp8 of "cmp word [rcx+disp8], 0", kBoneCountAt
		static_assert(PatternByte(kBoneReadPattern, kBonesRead) == kBonesAt && PatternByte(kBoneReadPattern, kSirenBonesRead) == kSirenBonesAt,
			"TweaksV: I-E-B2 reads the model info's +0xB0 and siren1's byte at +0x99");
		static_assert(PatternByte(kGlassReadPattern, kGlassBonesRead) == kGlassBonesAt, "TweaksV: I-E-B3 reads siren_glass1's byte at +0xAD");
		static_assert(PatternByte(kFindBonePattern, kBoneCountRead) == kBoneCountAt, "TweaksV: I-E-B4 tests the bone count at +0x5E first");

		constexpr int kLookedUp = 32;				// siren1 to siren32: what packs made for more name, the top of the game's 32-bit sets
		constexpr int kNameableBones = 128;			// a bone's index as a signed byte: 0 to 127
		static_assert(kGlassBonesAt == kSirenBonesAt + kGameSirens, "TweaksV: the glass bones' bytes follow the sirens'");

		// "siren1" to "siren32" and "siren_glass1" to "siren_glass32", made when TweaksV is compiled.
		struct BoneName
		{
			char text[16];
		};
		consteval BoneName MakeBoneName(std::string_view prefix, int n)
		{
			BoneName name{};
			size_t i = 0;
			for (size_t c = 0; c < prefix.size() && c + 3 < sizeof(name.text); c++)	// room for two digits and the end
				name.text[i++] = prefix[c];
			if (n >= 10)
				name.text[i++] = static_cast<char>('0' + n / 10 % 10);
			name.text[i] = static_cast<char>('0' + n % 10);
			return name;
		}
		consteval std::array<BoneName, kLookedUp> MakeBoneNames(std::string_view prefix)
		{
			std::array<BoneName, kLookedUp> names{};
			for (int n = 1; n <= kLookedUp; n++)
				names[n - 1] = MakeBoneName(prefix, n);
			return names;
		}
		constexpr std::array<BoneName, kLookedUp> kBoneNames[2] = { MakeBoneNames("siren"), MakeBoneNames("siren_glass") };
		static_assert(std::string_view(kBoneNames[0][0].text) == "siren1" && std::string_view(kBoneNames[0][kLookedUp - 1].text) == "siren32" &&
			std::string_view(kBoneNames[1][kLookedUp - 1].text) == "siren_glass32", "TweaksV: the siren bones' names");
		constexpr const char* kKinds[2] = { "siren", "siren_glass" };

		// The renderer: the vehicle, three floats, a flag byte, the model info, the skeleton, a dword and the coronas'
		// list. The stack's arguments are passed on as 64-bit values, so that each slot reaches the renderer as the
		// game's call left it.
		using SirenLights = uint64_t (*)(void* vehicle, float a, float b, float c, uint64_t flag, const uint8_t* model, const uint8_t* skeleton,
			uint64_t value, void* coronas);
		using FindBone = const uint8_t* (*)(const uint8_t* skeletonData, const char* name);

		SirenLights g_renderer = nullptr;	// the game's renderer, set before the call is redirected
		FindBone g_findBone = nullptr;

		// ---- the game's data, for the raise (PATTERNS.md, "SirenSettings", the sirens per vehicle)

		constexpr const GamePattern& kGetterCallPattern = Pattern("I-E-Z1");	// the renderer's call of the siren settings getter
		constexpr const GamePattern& kStateSizePattern = Pattern("I-E-Z2");		// the siren state's allocation
		constexpr const GamePattern& kReaderPattern = Pattern("I-E-Z3");		// the start of the array member's tree reader
		constexpr const GamePattern& kBrokenReadPattern = Pattern("I-E-Z4");	// the renderer's read of a vehicle's broken sirens
		constexpr const GamePattern& kFlagsReadPattern = Pattern("I-E-Z5");		// the renderer's read of a vehicle's flag byte
		constexpr const GamePattern& kGroupReadPattern = Pattern("I-E-Z6");		// the renderer's reads for its group lights
		constexpr const GamePattern& kWalkPattern = Pattern("I-E-Z7");			// the tree reader's walk over a node's children

		// All the renderer reads of a vehicle but its address, and so all a stand-in holds (the findings' 3.6)
		constexpr size_t kVehiclePointerAt = 0x48;	// a pointer, whose +0x20 it keeps for one float's store at its end
		constexpr size_t kVehicleKindAt = 0x6C;		// a word, which chooses the matrix of the group lights
		constexpr size_t kVehicleMatrixAt = 0x80;	// the 16 bytes of that matrix, read by a movaps
		constexpr size_t kVehicleMatrixSize = 16;
		constexpr size_t kVehicleTimeAt = 0xDC;		// a word
		constexpr size_t kVehicleBrokenAt = 0x860;	// a dword, a bit per siren that is broken
		constexpr size_t kVehicleFlagsAt = 0x986;	// a byte
		constexpr size_t kVehicleStateAt = 0x1380;	// the pointer to its siren state

		constexpr size_t kGetterCallAtZ1 = 24;		// in I-E-Z1's match: "call rel32" of the siren settings getter
		constexpr size_t kStateSizeAtZ2 = 1;		// in I-E-Z2's match: the imm32 of "mov ecx, imm32", the allocation's size

		// A pattern pins these two bytes, a word of a module's offsets
		consteval bool PinsWord(const GamePattern& pattern, size_t at, size_t value)
		{
			return PatternByte(pattern, at) == static_cast<int>(value & 0xFF) && PatternByte(pattern, at + 1) == static_cast<int>(value >> 8);
		}
		static_assert(PatternByte(kGetterCallPattern, kGetterCallAtZ1) == 0xE8 && PatternByte(kGetterCallPattern, 11) == kVehiclePointerAt &&
			PatternByte(kGetterCallPattern, 33) == kVehicleTimeAt && PinsWord(kGetterCallPattern, 47, kVehicleStateAt),
			"TweaksV: I-E-Z1 holds the getter's call and the renderer's reads of the vehicle's +0x48, +0xDC and +0x1380");
		static_assert(PinsWord(kBrokenReadPattern, 4, kVehicleBrokenAt) && PatternByte(kBrokenReadPattern, 16) == kBonesAt &&
			PatternByte(kBrokenReadPattern, 37) == kSirenBonesAt, "TweaksV: I-E-Z4 holds the renderer's reads of the vehicle's +0x860, the model info's +0xB0 and siren1's byte");
		static_assert(PinsWord(kFlagsReadPattern, 4, kVehicleFlagsAt), "TweaksV: I-E-Z5 holds the renderer's read of the vehicle's +0x986");
		static_assert(PatternByte(kGroupReadPattern, 4) == kVehicleKindAt && PatternByte(kGroupReadPattern, 30) == kVehicleMatrixAt,
			"TweaksV: I-E-Z6 holds the renderer's reads of the vehicle's +0x6C and +0x80");

		// A vehicle's siren state, and behind it what the raise adds to its one allocation (the findings' 8.3, layer B). The game
		// frees it whole; its operator new gives 16-aligned memory, which the stand-in's matrix needs.
		constexpr uint32_t kGameStateSize = 0x188;
		constexpr size_t kSecondStateAt = 0x190;	// in the allocation: the second call's state
		constexpr size_t kStandInAt = 0x320;		// and its stand-in for the vehicle
		constexpr size_t kStandInSize = kVehicleStateAt + sizeof(void*);
		constexpr uint32_t kRaisedStateSize = kStandInAt + kStandInSize;
		constexpr size_t kStateStartAt = 0x00;		// in a state: an int, the start time
		constexpr size_t kStateBeatAt = 0x08;		// an int, -1 when it is made
		constexpr size_t kSecondOwnerAt = kGameStateSize;	// in the allocation, in the 8 free bytes behind the first state: the vehicle the second was made for
		static_assert(kRaisedStateSize == 0x16A8 && kSecondOwnerAt + sizeof(void*) <= kSecondStateAt && kSecondStateAt + kGameStateSize <= kStandInAt && kSecondStateAt % 16 == 0 &&
			kStandInAt % 16 == 0 && kVehicleMatrixAt % 16 == 0, "TweaksV: the state, the second state and the stand-in, whose matrix lies 16-aligned");
		static_assert(PatternByte(kStateSizePattern, kStateSizeAtZ2) == (kGameStateSize & 0xFF) && PatternByte(kStateSizePattern, kStateSizeAtZ2 + 1) == (kGameStateSize >> 8),
			"TweaksV: I-E-Z2 holds the state's size, 0x188");

		// A siren setting (the findings' 3.3): its scalars, its head and tail lights, whether it uses real lights, its 20 lights
		constexpr size_t kSettingSize = 0x5E8;
		constexpr size_t kScalarsAt = 0x04;			// timeMultiplier to sequencerBpm, 8 dwords
		constexpr size_t kBpmAt = 0x20;				// sequencerBpm, a dword the renderer turns into a float, 200 in a new setting
		constexpr size_t kHeadTailAt = 0x24;		// the head and tail lights' sequencers and multiples, up to the next
		constexpr size_t kRealLightsAt = 0x38;		// useRealLights, a byte
		constexpr size_t kLightsAt = 0x3C;
		constexpr size_t kLightSize = 0x48;
		constexpr size_t kLightGroupAt = 0x40;		// in a light: its group, a byte, of the renderer's 4
		constexpr uint8_t kLightGroups = 4;
		constexpr size_t kLightsBytes = kGameSirens * kLightSize;
		static_assert(kLightsBytes == 1440 && kLightsAt + kLightsBytes == 0x5DC, "TweaksV: 20 lights of 0x48 bytes up to the count at +0x5DC");

		// In a light, its rotation and its flashiness, the same layout: delta and start as floats, a byte not 0 for the direction reversed.
		constexpr size_t kSequencesAt[2] = { 0x00, 0x14 };
		constexpr size_t kDeltaAt = 0x00;
		constexpr size_t kStartAt = 0x04;
		constexpr size_t kDirectionAt = 0x11;

		// The parser (the findings' 3.3): a member object - the vtable, the member data, an array's element's member object - whose
		// slot 1 reads a node of the tree (rdx) into an object (r8): an array's into the structure that holds it, an element's into
		// the element, no more arguments and no result read; a node's first child and next sibling; the fields of the member data.
		constexpr size_t kMemberDataAt = 0x08;
		constexpr size_t kElementMemberAt = 0x10;
		constexpr size_t kReaderSlot = 1;
		constexpr size_t kFirstChildAt = 0x38;
		constexpr size_t kNextSiblingAt = 0x30;
		constexpr size_t kDataOffsetAt = 0x08;		// a member's offset in its structure, 0 until the structure is registered
		constexpr size_t kDataTypeAt = 0x10;
		constexpr size_t kDataSubtypeAt = 0x11;
		constexpr size_t kDataElementSizeAt = 0x20;
		constexpr size_t kDataCapacityAt = 0x28;
		constexpr size_t kDataElementAt = 0x30;
		constexpr uint8_t kArrayType = 0x0D;
		constexpr uint8_t kFixedArraySubtype = 0x01;
		constexpr uint8_t kStructureType = 0x0C;	// the element's
		constexpr size_t kLightsMember = 19;		// in I-E-P1's tables: the lights
		constexpr size_t kWalkFirstAt = 3, kWalkDataAt = 20, kWalkCapacityAt = 34, kWalkNextAt = 47, kWalkSizeAt = 72;	// I-E-Z7, each a disp8
		static_assert(PatternByte(kWalkPattern, kWalkFirstAt) == kFirstChildAt && PatternByte(kWalkPattern, kWalkDataAt) == kMemberDataAt &&
			PatternByte(kWalkPattern, kWalkCapacityAt) == kDataCapacityAt && PatternByte(kWalkPattern, kWalkNextAt) == kNextSiblingAt &&
			PatternByte(kWalkPattern, kWalkSizeAt) == kDataElementSizeAt, "TweaksV: I-E-Z7 holds the reader's reads of the node's +0x38 and +0x30 and the member data's +0x28 and +0x20");
		static_assert(PatternByte(kReaderPattern, 31) == 0x48 && PatternByte(kReaderPattern, 32) == 0x89 && PatternByte(kReaderPattern, 33) == 0xD7 &&
			PatternByte(kReaderPattern, 34) == 0x49 && PatternByte(kReaderPattern, 35) == 0x89 && PatternByte(kReaderPattern, 36) == 0xCD,
			"TweaksV: I-E-Z3 keeps rdx, the node, in rdi and rcx, the member object, in r13, which I-E-Z7's walk reads");

		using SettingConstructor = void* (*)(uint8_t* setting);
		using MemberReader = uint64_t (*)(const uint8_t* member, const uint8_t* node, uint8_t* object);

		// What a position past the game's settings gets in the getter's place (D41), shared by every thread: zero but for sequencerBpm, the
		// game's own default, which takes the renderer's one division by it away. It lights nothing, and the vehicle lights' head and tail lights go
		// as with no setting.
		consteval std::array<uint8_t, kSettingSize> MakeEmptySetting()
		{
			std::array<uint8_t, kSettingSize> setting{};
			setting[kBpmAt] = 200;
			return setting;
		}
		constexpr std::array<uint8_t, kSettingSize> kEmptySetting = MakeEmptySetting();
		static_assert(kSettingSize == kSirenSettingSize, "TweaksV: a siren setting's size, as the IDs' module has it");

		// ---- the own settings: read from a file on the parser's thread and kept in a table under a lock; read on the renderer's threads without one

		constexpr size_t kOwnSettings = 255;	// a vehicle can use 255 settings, and so at most that many have lights of their own
		constexpr int kMostItems = 65535;		// the items of a node that are counted

		// One entry, found by its key: whole before g_ownCount says so, and a later setting whole before it is stored here.
		struct OwnSetting
		{
			uint64_t key;							// the hash of the game's 20 lights
			std::atomic<const uint8_t*> setting;	// a siren setting of the game's shape, its lights those past 20
			std::atomic<uint32_t> lights;			// how many of those were read
			std::atomic<bool> scalarsGiven;			// a model's first pass has given it its setting's scalars
		};
		OwnSetting g_own[kOwnSettings];
		std::atomic<uint32_t> g_ownCount{ 0 };
		SRWLOCK g_ownLock = SRWLOCK_INIT;		// between the parser's threads; the renderer's never take it
		uint8_t* g_ownObjects = nullptr;		// the table's settings, allocated at the first
		size_t g_ownUsed = 0;					// those in use, a setting replaced among them
		bool g_ownFull = false;					// reported
		bool g_ownNoMemory = false;				// reported
		std::atomic<bool> g_ownNotRead{ false };	// reported
		uint64_t g_settingLines = 0;			// the lines of settings read

		std::atomic<uint32_t> g_ownTwins{ 0 };		// settings read that were there already
		std::atomic<uint32_t> g_ownReplaced{ 0 };	// settings read that replaced one with the same 20 lights
		std::atomic<uint32_t> g_ownNotFit{ 0 };		// settings the table had no room for, or that could not be read
		std::atomic<uint64_t> g_secondCalls{ 0 };	// the renderer's second calls
		std::atomic<uint32_t> g_sharedScalars{ 0 };	// models that changed the scalars of a setting another model uses

		int g_lightsPast = 0;					// SIRENS_PER_VEHICLE - 20
		const uint8_t* g_lightsData = nullptr;	// the lights' member data, the one array the hook reads on
		SettingConstructor g_constructor = nullptr;
		MemberReader g_arrayReader = nullptr;	// the tree reader's trampoline
		const uint8_t* g_getter = nullptr;		// the game's siren settings getter, for the measurement's line once the calls are redirected
		const uint8_t* g_carcolsPointer = nullptr;	// the game's pointer to its carcols data
		uint8_t* g_call = nullptr;				// I-E-B1's match, once the raise has looked for it
		bool g_raised = false;					// the part's one write is done, so that the wrapper in the renderer's call is the raise's
		bool g_wouldRaise = false;				// with DRY_RUN: every site found, and the line that would redirect the call written
		bool g_counting = false;				// the measurement counts, on a first pass in the raise's wrapper too
#ifdef TWEAKSV_TEST_BUILD
		int g_testPasses = 0;					// test builds only: TEST_SIREN_PASSES, so many more of the wrapper's calls in each pass
#endif

		// A light's bytes that hold no member, which a copy need not keep and the heap leaves as it likes (the findings' 3.3)
		constexpr bool IsLightPadding(size_t at)
		{
			return at == 0x13 || at == 0x27 || (at >= 0x35 && at < 0x38);
		}

		// The hash of a siren setting's 20 lights, their members alone, never 0, so that it is the same at the parser and in the game's array, and for one
		// file read twice: false if they cannot be read, since the array's growth can move a setting.
		bool KeyOfLights(const uint8_t* lights, uint64_t& key)
		{
			uint8_t copy[kLightsBytes];
			if (!CopyMemorySafely(copy, lights, sizeof(copy)))
				return false;
			uint64_t hash = 0xCBF29CE484222325ull;
			for (size_t i = 0; i < sizeof(copy); i++)
				if (!IsLightPadding(i % kLightSize))
					hash = (hash ^ copy[i]) * 0x100000001B3ull;
			key = hash ? hash : 1;
			return true;
		}

		OwnSetting* FindOwn(uint64_t key)
		{
			const uint32_t count = g_ownCount.load(std::memory_order_acquire);
			for (uint32_t i = 0; i < count; i++)
				if (g_own[i].key == key)
					return &g_own[i];
			return nullptr;
		}

		// The real setting's scalars and whether it uses real lights: each dword whole, since a renderer may be reading them, which
		// finds a value of one model or the other's. Its head and tail lights stay zero, so that the second call runs them not twice.
		void CopyScalars(uint8_t* own, const uint8_t* setting)
		{
			for (size_t at = kScalarsAt; at < kHeadTailAt; at += sizeof(uint32_t))
				*reinterpret_cast<volatile uint32_t*>(own + at) = ValueAt<uint32_t>(setting, at);
			own[kRealLightsAt] = setting[kRealLightsAt];
		}

		// Each light, once, as the renderer will take it. Its start, so that the renderer's delta x i, or delta x (20 - i) with its direction reversed,
		// gives the light its true index: its place among the own lights is 20 less. Its group, an index the renderer does not check, which reaches
		// the return address from group 9: past the game's 20 a file's lights come to the renderer only through here, and one past the 4 is group 0.
		void CorrectLights(uint8_t* lights, int kept)
		{
			for (int k = 0; k < kept; k++)
			{
				uint8_t* const light = lights + k * kLightSize;
				for (const size_t sequence : kSequencesAt)
				{
					uint8_t* const at = light + sequence;
					const float delta = ValueAt<float>(at, kDeltaAt);
					const float start = ValueAt<float>(at, kStartAt);
					const float corrected = at[kDirectionAt] ? start - kGameSirens * delta : start + kGameSirens * delta;
					memcpy(at + kStartAt, &corrected, sizeof(corrected));
				}
				if (light[kLightGroupAt] >= kLightGroups)
					light[kLightGroupAt] = 0;
			}
		}

		// A node's items, up to kMostItems, or false on an access violation. The game has just counted them itself.
		bool CountItems(const uint8_t* node, int& items)
		{
			__try
			{
				for (const uint8_t* child = ValueAt<const uint8_t*>(node, kFirstChildAt); child && items < kMostItems; child = ValueAt<const uint8_t*>(child, kNextSiblingAt))
					items++;
				return true;
			}
			__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return false;
			}
		}

		// The own setting made by the game's constructor in the zeroed memory at own, and into its lights the items past the game's 20, as many
		// as SIRENS_PER_VEHICLE gives, each by the game's own reader of an element; false on an access violation.
		bool ReadOwn(uint8_t* own, const uint8_t* element, const uint8_t* node, int& kept)
		{
			__try
			{
				g_constructor(own);
				memset(own + kHeadTailAt, 0, kRealLightsAt - kHeadTailAt);
				const MemberReader read = ValueAt<const MemberReader*>(element)[kReaderSlot];
				int item = 0;
				for (const uint8_t* child = ValueAt<const uint8_t*>(node, kFirstChildAt); child && kept < g_lightsPast; child = ValueAt<const uint8_t*>(child, kNextSiblingAt), item++)
					if (item >= kGameSirens)
						read(element, child, own + kLightsAt + kept++ * kLightSize);
				return true;
			}
			__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return false;
			}
		}

		// A line of the settings read, with DIAGNOSTIC_MODE: capped, as a load can bring any number of them.
		void SettingLine(const std::string& line)
		{
			if (!IsDiagnosticMode())
				return;
			const uint64_t n = ++g_settingLines;
			if (n <= kMaxLinesPerKind)
				WriteLog(kTag, "DIAGNOSTIC: " + line);
			else if (n == kMaxLinesPerKind + 1)
				WriteLog(kTag, "DIAGNOSTIC: " + std::to_string(kMaxLinesPerKind) + " lines of siren settings with lights past 20 written - from now on they are only counted; the summary every minute has the numbers.");
		}

		// What one setting with more than 20 items, read into read, does to the table, under the lock: a setting that stays is copied whole
		// into the table's memory before anything leads to it, and one it replaces stays where it is, since a renderer may be reading it.
		void KeepOwn(const uint8_t* read, uint64_t key, int items, int kept)
		{
			char head[96];
			snprintf(head, sizeof(head), "a siren setting of %d%s lights read (key 0x%016llX): ", items, items >= kMostItems ? " or more" : "", static_cast<unsigned long long>(key));
			OwnSetting* const same = FindOwn(key);
			if (same)
			{
				const uint8_t* const before = same->setting.load(std::memory_order_relaxed);
				if (same->lights.load(std::memory_order_relaxed) == static_cast<uint32_t>(kept) && memcmp(read + kLightsAt, before + kLightsAt, kLightsBytes) == 0)
				{
					g_ownTwins.fetch_add(1, std::memory_order_relaxed);
					SettingLine(head + std::string("the twin of one read before, dropped."));
					return;
				}
			}
			if (g_ownUsed >= kOwnSettings)
			{
				g_ownNotFit.fetch_add(1, std::memory_order_relaxed);
				SettingLine(head + std::string("no room for it in the table."));
				if (!g_ownFull)
				{
					g_ownFull = true;
					ReportFailure(kTag, "Some siren settings' lights past 20 are not kept this session: the table of " + std::to_string(kOwnSettings) +
						" siren settings with lights past 20 is full, and the next ones keep 20 lights.");
				}
				return;
			}
			uint8_t* const own = g_ownObjects + g_ownUsed++ * kSettingSize;
			memcpy(own, read, kSettingSize);
			if (same)
			{
				CopyScalars(own, same->setting.load(std::memory_order_relaxed));
				same->lights.store(static_cast<uint32_t>(kept), std::memory_order_relaxed);
				same->setting.store(own, std::memory_order_release);
				g_ownReplaced.fetch_add(1, std::memory_order_relaxed);
				SettingLine(head + std::string("the same 20 lights as one read before and others past them, so it replaces that one."));
				return;
			}
			const uint32_t count = g_ownCount.load(std::memory_order_relaxed);
			OwnSetting& entry = g_own[count];
			entry.key = key;
			entry.lights.store(static_cast<uint32_t>(kept), std::memory_order_relaxed);
			entry.setting.store(own, std::memory_order_relaxed);
			g_ownCount.store(count + 1, std::memory_order_release);
			std::string line = head + std::to_string(kept) + " past 20 kept in a setting of its own, " + std::to_string(count + 1) + " of " + std::to_string(kOwnSettings);
			if (items - kGameSirens > kept)
				line += ", and " + std::to_string(items - kGameSirens - kept) + " more than SIRENS_PER_VEHICLE gives dropped";
			SettingLine(line + ".");
		}

		// A setting's lights past 20 that cannot be read: it keeps 20, reported once, as no pattern pins what the hook is handed.
		void NotRead(const char* why)
		{
			g_ownNotFit.fetch_add(1, std::memory_order_relaxed);
			if (!g_ownNotRead.exchange(true))
				ReportFailure(kTag, std::string("Some siren settings' lights past 20 are not kept this session: ") + why + ", and such a setting keeps 20 lights.");
		}

		// A lights array the game has just read: its items past 20, if it has any, into an own setting.
		void KeepLightsPast20(const uint8_t* member, const uint8_t* node, const uint8_t* object)
		{
			int items = 0;
			if (!CountItems(node, items))
			{
				NotRead("a lights array's node could not be read");
				return;
			}
			if (items <= kGameSirens)
				return;
			const uint8_t* const element = ValueAt<const uint8_t*>(member, kElementMemberAt);
			if (!element || !IsReadable(element, kMemberDataAt + sizeof(void*)) || ValueAt<const uint8_t*>(element, kMemberDataAt) != ValueAt<const uint8_t*>(g_lightsData, kDataElementAt))
			{
				NotRead("a lights array's element is not the one its member data names");
				return;
			}
			uint64_t key = 0;
			if (!KeyOfLights(object + ValueAt<uint64_t>(g_lightsData, kDataOffsetAt), key))	// the offset as registered, at the read
			{
				NotRead("the siren setting a lights array is read into could not be read");
				return;
			}
			// read with no lock taken, since the game's reader of an element runs on: it may wait for what another parser's thread holds
			alignas(16) uint8_t own[kSettingSize] = {};
			int kept = 0;
			if (!ReadOwn(own, element, node, kept))
			{
				NotRead("reading the lights past 20 of one ended in an access violation");
				return;
			}
			CorrectLights(own + kLightsAt, kept);
			const ExclusiveLock lock(g_ownLock);
			if (!g_ownObjects)
			{
				if (g_ownNoMemory)
				{
					g_ownNotFit.fetch_add(1, std::memory_order_relaxed);
					return;
				}
				g_ownObjects = static_cast<uint8_t*>(VirtualAlloc(nullptr, kOwnSettings * kSettingSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
#ifdef TWEAKSV_TEST_BUILD
				// test builds only: TWEAKSV_TEST_SIREN_WRITE_FAILS=6 takes this memory for none, what no other way reaches
				wchar_t failure[4]{};
				if (GetEnvironmentVariableW(L"TWEAKSV_TEST_SIREN_WRITE_FAILS", failure, 4) == 1 && failure[0] == L'6')
				{
					VirtualFree(g_ownObjects, 0, MEM_RELEASE);
					g_ownObjects = nullptr;
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				}
#endif
				if (!g_ownObjects)
				{
					g_ownNoMemory = true;
					g_ownNotFit.fetch_add(1, std::memory_order_relaxed);
					ReportFailure(kTag, "The lights past 20 of the siren settings are not kept this session: the memory for them could not be had (" + DescribeError(GetLastError()) +
						"), and the vehicles keep 20 sirens.");
					return;
				}
			}
			KeepOwn(own, key, items, kept);
		}

		// The game's reader of an array member, hooked: the game's first, then one compare per array read in parsing, and only the lights
		// go further. What the game's reader left in its last error stands.
		uint64_t HookedArrayReader(const uint8_t* member, const uint8_t* node, uint8_t* object)
		{
			const uint64_t result = g_arrayReader(member, node, object);
			if (ValueAt<const uint8_t*>(member, kMemberDataAt) == g_lightsData)
			{
				const DWORD lastError = GetLastError();
				try
				{
					KeepLightsPast20(member, node, object);
				}
				catch (...)
				{
					// a line that could not be written: what is kept stands, and nothing throws into the game
				}
				SetLastError(lastError);
			}
			return result;
		}

		// ---- the vehicle models seen: a fixed table, filled by compare-exchange - no lock, no allocation

		constexpr int kSlotBits = 10;
		constexpr size_t kModelSlots = size_t{ 1 } << kSlotBits;	// 1,024
		constexpr size_t kProbes = 32;								// the slots a model is looked for in, from its hash on
		static_assert(kModelSlots <= kMaxLinesPerKind, "TweaksV: a line per model stays below the cap of the lines per event");

		enum : uint8_t
		{
			kReading = 0,	// claimed, its first pass under way
			kCounted = 1,	// its first pass done
		};

		// What the second call hands the renderer in a model info's place, its slot's own. Of a model info the renderer reads only what
		// it hands the getter and the bone structure, through two pointers from +0xB0 (the findings' 3.6).
		struct ModelBlock
		{
			uint8_t head[kBonesAt];
			const int8_t* const* holder;					// +0xB0: the holder of the bone structure, the next field
			const int8_t* structure;						// bones, as the renderer's bone structure
			const OwnSetting* own;							// what the renderer's getter gives for this block
			int8_t bones[kSirenBonesAt + kGameSirens];		// +0x99 + k: the bone of siren(21 + k); -1 for none, and in every other byte
		};
		static_assert(offsetof(ModelBlock, holder) == kBonesAt, "TweaksV: a block has the bone structure's holder where a model info has it");
		ModelBlock g_blocks[kModelSlots];

		// One vehicle model: what its first pass found, written before state, read by the summary after it. Index 0
		// of each pair is the sirens, 1 the glass bones.
		struct ModelSlot
		{
			std::atomic<const uint8_t*> model{ nullptr };	// the model info, never freed while the game runs
			std::atomic<uint8_t> state{ kReading };
			const char* unreadable;		// what could not be read, or nullptr
			const ModelBlock* block;	// with the raise: its block, if the second call lights it
			const char* notLit;			// with the raise: why not, when it has no block
			uint32_t hash;				// the hash of the model's name
			uint16_t bones;				// its skeleton's bones
			uint16_t lostBone;			// the highest index of a siren bone found by name that its bone structure does not name
			uint8_t inTable[2];			// of siren1 to siren20: the bones its bone structure names
			uint8_t byName[2];			// of siren1 to siren32: the bones found by name
			uint8_t highest[2];			// the highest n of siren<n> found by name
			uint8_t past[2];			// found by name past siren20, never lit but by the raise
			uint8_t lost[2];			// of siren1 to siren20: found by name past bone 127, not named by its bone structure
			uint8_t unnamed[2];			// of siren1 to siren20: found by name below bone 128, not named by its bone structure
			uint8_t missed[2];			// of siren1 to siren20: named by its bone structure, not found by name
			uint8_t moved[2];			// of siren1 to siren20: found by name at another index than its bone structure names
			uint8_t lit;				// with the raise: its bones past siren20 that the second call lights
			bool written;				// its line written (the summary's thread only)
		};
		ModelSlot g_models[kModelSlots];

		std::atomic<uint64_t> g_passes{ 0 };		// the renderer's calls counted
		std::atomic<uint64_t> g_notHeld{ 0 };		// passes of models the table could not hold
		std::atomic<uint32_t> g_counted{ 0 };		// models whose first pass is done
		std::atomic<bool> g_firstCounted{ false };
		HANDLE g_firstModel = nullptr;				// set at the first model counted: the summary's clock
		std::atomic<bool> g_modelsFull{ false };	// the raise's table full, reported

		size_t SlotOf(const uint8_t* model)
		{
			return static_cast<size_t>((reinterpret_cast<uintptr_t>(model) * 0x9E3779B97F4A7C15ull) >> (64 - kSlotBits));
		}

		// value from address, when IsReadable says the bytes there can be read: the game's data is never trusted.
		template <typename T> bool Read(const uint8_t* address, T& value)
		{
			if (!IsReadable(address, sizeof(T)))
				return false;
			memcpy(&value, address, sizeof(T));
			return true;
		}

		// find-bone-by-name, an access violation in it caught - false then. The function is the game's own, but what it is
		// handed, the renderer's 7th argument, no pattern pins: after a game update it may be no skeleton.
		bool FindBoneSafely(const uint8_t* data, const char* name, const uint8_t*& bone)
		{
			__try
			{
				bone = g_findBone(data, name);
				return true;
			}
			__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return false;
			}
		}

		// A model's first pass: its bone structure's siren and glass bytes, its skeleton's bones, and the 64 lookups by
		// name. Every pointer of the game's goes through IsReadable before it is read, and every lookup through
		// FindBoneSafely: one that faults makes the model one that could not be read, looked at no more.
		void ReadModel(ModelSlot& slot, const uint8_t* model, const uint8_t* skeleton)
		{
			Read(model + kNameHashAt, slot.hash);
			const uint8_t* holder = nullptr;
			const uint8_t* structure = nullptr;
			int8_t table[2][kGameSirens] = {};
			if (!Read(model + kBonesAt, holder) || !Read(holder, structure) || !IsReadable(structure + kSirenBonesAt, sizeof(table)))
			{
				slot.unreadable = "its bone structure (the model info's +0xB0)";
				return;
			}
			memcpy(table, structure + kSirenBonesAt, sizeof(table));	// siren1 to siren20, then siren_glass1 to siren_glass20
			const uint8_t* data = nullptr;
			if (!Read(skeleton, data) || !Read(data + kBoneCountAt, slot.bones))
			{
				slot.unreadable = "its skeleton data (the renderer's 7th argument, its first field)";
				return;
			}
			for (int kind = 0; kind < 2; kind++)
				for (int n = 1; n <= kLookedUp; n++)
				{
					const bool named = n <= kGameSirens && table[kind][n - 1] >= 0;
					if (named)
						slot.inTable[kind]++;
					const uint8_t* bone = nullptr;
					if (slot.bones && !FindBoneSafely(data, kBoneNames[kind][n - 1].text, bone))
					{
						slot.unreadable = "find-bone-by-name faulted on its skeleton data (an access violation)";
						return;
					}
					if (!bone)
					{
						if (named)
							slot.missed[kind]++;
						continue;
					}
					slot.byName[kind]++;
					slot.highest[kind] = static_cast<uint8_t>(n);
					if (n > kGameSirens)
					{
						slot.past[kind]++;
						continue;
					}
					uint16_t index = 0;
					const bool indexRead = Read(bone + kBoneIndexAt, index);
					if (!named && indexRead && index >= kNameableBones)	// past what a signed byte names
					{
						slot.lost[kind]++;
						if (index > slot.lostBone)
							slot.lostBone = index;
					}
					else if (!named)
						slot.unnamed[kind]++;
					else if (indexRead && index != static_cast<uint16_t>(table[kind][n - 1]))
						slot.moved[kind]++;
				}
		}

		// A model's setting as the game's getter gives it, bounded (D41): none at position 0; below the array's count that entry, the count read
		// before the pointer, since the array's growth stores the pointer first and the count after; any other position, an ID that found no
		// setting, the empty setting. One byte, two loads and two compares.
		const uint8_t* RealSettingOf(const uint8_t* model)
		{
			const uint8_t position = model[kModelSirenPositionAt];
			if (position == 0)
				return nullptr;
			const uint8_t* const carcols = Load<const uint8_t*>(g_carcolsPointer);
			if (!carcols || position >= Load<uint16_t>(carcols + kCarcolsSirenCountAt))
				return kEmptySetting.data();
			return Load<const uint8_t*>(carcols + kCarcolsSirenSettingsAt) + position * kSirenSettingSize;
		}

		// With the raise, on a model's first pass: its block, if the second call is to light it, or the reason it is not. It takes its bones
		// from the room behind the model's bone structure, and an own setting by the hash of the 20 lights its setting holds.
		void MakeBlock(ModelSlot& slot, size_t index, const uint8_t* model)
		{
			const uint8_t* holder = nullptr;
			const uint8_t* structure = nullptr;
			int8_t room[kLookedUp - kGameSirens];
			const size_t past = static_cast<size_t>(g_lightsPast);
			if (!Read(model + kBonesAt, holder) || !holder || !Read(holder, structure) || !structure || !IsReadable(structure + kRoomSirenBonesAt + kGameSirens, past))
			{
				slot.notLit = "its bone structure could not be read";
				return;
			}
			memcpy(room, structure + kRoomSirenBonesAt + kGameSirens, past);
			uint8_t bones = 0;
			for (size_t k = 0; k < past; k++)
				if (room[k] >= 0)
					bones++;
			if (!bones)
			{
				slot.notLit = "it has no bones past siren20 in the room behind its bone structure";
				return;
			}
			const uint8_t* const setting = RealSettingOf(model);
			if (!setting)
			{
				slot.notLit = "it has no siren setting";
				return;
			}
			if (setting == kEmptySetting.data())
			{
				slot.notLit = "its siren setting's position lies past the game's settings";
				return;
			}
			uint8_t head[kLightsAt];
			uint64_t key = 0;
			if (!CopyMemorySafely(head, setting, sizeof(head)) || !KeyOfLights(setting + kLightsAt, key))
			{
				slot.notLit = "its siren setting could not be read";
				return;
			}
			OwnSetting* const own = FindOwn(key);
			if (!own)
			{
				slot.notLit = "its siren setting has no lights past 20";
				return;
			}
			// its scalars, the last model's where two models with the same 20 lights differ in them, the limit of a key of the lights alone
			uint8_t* const ownSetting = const_cast<uint8_t*>(own->setting.load(std::memory_order_acquire));
			const bool asGiven = memcmp(ownSetting + kScalarsAt, head + kScalarsAt, kHeadTailAt - kScalarsAt) == 0 && ownSetting[kRealLightsAt] == head[kRealLightsAt];
			if (own->scalarsGiven.exchange(true, std::memory_order_relaxed) && !asGiven)
				g_sharedScalars.fetch_add(1, std::memory_order_relaxed);
			if (!asGiven)
				CopyScalars(ownSetting, head);
			ModelBlock& block = g_blocks[index];
			memset(block.bones, 0xFF, sizeof(block.bones));
			memcpy(block.bones + kSirenBonesAt, room, past);
			block.structure = block.bones;
			block.holder = &block.structure;
			block.own = own;
			slot.block = &block;
			slot.lit = bones;
		}

		// Claimed by this pass: the first pass's work, the caller's last error kept around it, and the summary's clock
		// started at the first model counted.
		void FirstPass(ModelSlot& slot, const uint8_t* model, const uint8_t* skeleton)
		{
			const DWORD lastError = GetLastError();
			if (g_counting)
				ReadModel(slot, model, skeleton);
			if (g_raised)
				MakeBlock(slot, static_cast<size_t>(&slot - g_models), model);
			slot.state.store(kCounted, std::memory_order_release);
			g_counted.fetch_add(1, std::memory_order_release);
			if (!g_firstCounted.load(std::memory_order_relaxed) && !g_firstCounted.exchange(true) && g_firstModel)
				SetEvent(g_firstModel);
			SetLastError(lastError);
		}

		// Each pass: one lookup of the model in the table - its first pass claims a free slot and reads it, any later
		// one finds it. A model that finds neither among its slots is counted as not held, and gets no slot back.
		const ModelSlot* CountPass(const uint8_t* model, const uint8_t* skeleton)
		{
			g_passes.fetch_add(1, std::memory_order_relaxed);
			if (!model)
				return nullptr;
			size_t at = SlotOf(model);
			for (size_t probe = 0; probe < kProbes; probe++, at = (at + 1) & (kModelSlots - 1))
			{
				ModelSlot& slot = g_models[at];
				const uint8_t* held = slot.model.load(std::memory_order_acquire);
				if (held == model)
					return &slot;
				if (held)
					continue;
				if (slot.model.compare_exchange_strong(held, model, std::memory_order_acq_rel))
				{
					FirstPass(slot, model, skeleton);
					return &slot;
				}
				if (held == model)	// another thread claimed it for this model just now
					return &slot;
			}
			g_notHeld.fetch_add(1, std::memory_order_relaxed);
			return nullptr;
		}

		// The game's call of the renderer, redirected: the game's renderer first, with the game's arguments, then the
		// count - on the game's thread, once a frame for each vehicle whose sirens are on.
		uint64_t CountedSirenLights(void* vehicle, float a, float b, float c, uint64_t flag, const uint8_t* model, const uint8_t* skeleton,
			uint64_t value, void* coronas)
		{
			const uint64_t result = g_renderer(vehicle, a, b, c, flag, model, skeleton, value, coronas);
			CountPass(model, skeleton);
			return result;
		}

		// ---- the second call

		// Both calls of the siren settings getter, redirected: a block, which is no model info, gets its own setting, a model what the game's getter
		// gives it, bounded. It touches no more than the game's does, and neither caller holds anything in a volatile register across the call.
		const uint8_t* SirenSettingsOf(const uint8_t* model)
		{
			if (reinterpret_cast<uintptr_t>(model) - reinterpret_cast<uintptr_t>(g_blocks) < sizeof(g_blocks))
				return reinterpret_cast<const ModelBlock*>(model)->own->setting.load(std::memory_order_acquire);
			return RealSettingOf(model);
		}

		// The renderer once more, on the part's own stand-ins behind the vehicle's state: the vehicle's bits past 20 are the stand-in's
		// bits from 0 on, so that its light 1 is siren21. The state of the second call follows the game's: its resets touch the first, so
		// a first state with another start time than the second's has been made anew, and the second is made anew as the game makes it. So is one made
		// for another vehicle, or in none: the game's constructor knows nothing of it, the heap gives a block back as an earlier car left it, and that can hold this very start time.
		void SecondCall(const uint8_t* vehicle, float a, float b, float c, uint64_t flag, const ModelBlock* block, const uint8_t* skeleton, uint64_t value, void* coronas)
		{
			uint8_t* const state = ValueAt<uint8_t*>(vehicle, kVehicleStateAt);
			if (!state)
				return;
			uint8_t* const second = state + kSecondStateAt;
			uint8_t* const standIn = state + kStandInAt;
			const int32_t start = ValueAt<int32_t>(state, kStateStartAt);
			if (ValueAt<const uint8_t*>(state, kSecondOwnerAt) != vehicle || ValueAt<int32_t>(second, kStateStartAt) != start)
			{
				memset(second, 0, kGameStateSize);
				memset(standIn, 0, kStandInSize);
				const int32_t beat = -1;
				memcpy(second + kStateBeatAt, &beat, sizeof(beat));
				memcpy(second + kStateStartAt, &start, sizeof(start));
				memcpy(state + kSecondOwnerAt, &vehicle, sizeof(vehicle));
			}
			memcpy(standIn + kVehiclePointerAt, vehicle + kVehiclePointerAt, sizeof(void*));
			memcpy(standIn + kVehicleKindAt, vehicle + kVehicleKindAt, sizeof(uint16_t));
			memcpy(standIn + kVehicleMatrixAt, vehicle + kVehicleMatrixAt, kVehicleMatrixSize);
			memcpy(standIn + kVehicleTimeAt, vehicle + kVehicleTimeAt, sizeof(uint16_t));
			standIn[kVehicleFlagsAt] = vehicle[kVehicleFlagsAt];
			const uint32_t broken = ValueAt<uint32_t>(vehicle, kVehicleBrokenAt) >> kGameSirens;
			memcpy(standIn + kVehicleBrokenAt, &broken, sizeof(broken));
			memcpy(standIn + kVehicleStateAt, &second, sizeof(second));
			g_secondCalls.fetch_add(1, std::memory_order_relaxed);
			g_renderer(standIn, a, b, c, flag, reinterpret_cast<const uint8_t*>(block), skeleton, value, coronas);
		}

		// The game's call of the renderer, redirected, with the raise: the game's renderer first, with the game's arguments, then one lookup
		// of the model in the table, the measurement's when it counts, and a second call for a model with a block; the first call's result and
		// its last error go back. A model the table cannot hold stays at 20, reported once.
		uint64_t SirenLightsTwice(void* vehicle, float a, float b, float c, uint64_t flag, const uint8_t* model, const uint8_t* skeleton,
			uint64_t value, void* coronas)
		{
			const uint64_t result = g_renderer(vehicle, a, b, c, flag, model, skeleton, value, coronas);
			const ModelSlot* const slot = CountPass(model, skeleton);
			if (slot)
			{
				if (slot->state.load(std::memory_order_acquire) == kCounted && slot->block)
				{
					const DWORD lastError = GetLastError();
					SecondCall(static_cast<const uint8_t*>(vehicle), a, b, c, flag, slot->block, skeleton, value, coronas);
					SetLastError(lastError);
				}
			}
			else if (model && !g_modelsFull.load(std::memory_order_relaxed) && !g_modelsFull.exchange(true))
			{
				const DWORD lastError = GetLastError();
				try
				{
					ReportFailure(kTag, "Some vehicle models' lights past 20 are not lit this session: the table of " + std::to_string(kModelSlots) +
						" vehicle models is full, and the next ones keep 20 sirens.");
				}
				catch (...)
				{
					// nothing throws into the game; the summary's numbers still say that the table is full
				}
				SetLastError(lastError);
			}
#ifdef TWEAKSV_TEST_BUILD
			// test builds only: TEST_SIREN_PASSES more of both calls, so that real light records and coronas go the game's own way into a job's lists, the
			// moved stores and the scene store, past numbers no normal load reaches
			if (g_testPasses)
			{
				const DWORD lastError = GetLastError();
				const ModelBlock* const block = slot && slot->state.load(std::memory_order_acquire) == kCounted ? slot->block : nullptr;
				for (int pass = 0; pass < g_testPasses; pass++)
				{
					g_renderer(vehicle, a, b, c, flag, model, skeleton, value, coronas);
					if (block)
						SecondCall(static_cast<const uint8_t*>(vehicle), a, b, c, flag, block, skeleton, value, coronas);
				}
				SetLastError(lastError);
			}
#endif
			return result;
		}

		// ---- the summary (DIAGNOSTIC_MODE), on its own thread

		uint64_t Numbers()
		{
			return g_counted.load() | (g_notHeld.load() ? uint64_t{ 1 } << 32 : 0) | (uint64_t{ g_ownCount.load() } << 33) |
				(uint64_t{ g_ownTwins.load() + g_ownReplaced.load() + g_ownNotFit.load() + g_sharedScalars.load() } << 41);
		}

		// "12 and 12", "1 and 0" - a pair of a model's numbers, the sirens' first.
		std::string Pair(const uint8_t (&numbers)[2])
		{
			return std::to_string(numbers[0]) + " and " + std::to_string(numbers[1]);
		}

		// A model's line: what its bone structure names and what is found by name, and where the two part.
		std::string ModelLine(const ModelSlot& slot)
		{
			char head[64];
			snprintf(head, sizeof(head), "DIAGNOSTIC: vehicle model 0x%08X", slot.hash);
			if (slot.unreadable)
				return std::string(head) + " could not be read: " + slot.unreadable + ".";
			std::string line = std::string(head) + ", " + std::to_string(slot.bones) + " bones - siren and glass bones by name: " + Pair(slot.byName) +
				", up to ";
			for (int kind = 0; kind < 2; kind++)
				line += (kind ? " and " : "") + (slot.highest[kind] ? std::string(kKinds[kind]) + std::to_string(slot.highest[kind]) :
					"no " + std::string(kKinds[kind]));
			line += "; named by its bone structure: " + Pair(slot.inTable);
			if (slot.past[0] || slot.past[1])
				line += (g_raised ? "; past siren20: " : "; past siren20, never lit: ") + Pair(slot.past);
			if (slot.block)
				line += "; lit past siren20 by the second call: " + std::to_string(slot.lit) + " bones, the " +
					std::to_string(slot.block->own->lights.load(std::memory_order_relaxed)) + " lights past 20 of a setting of its own";
			else if (g_raised && (slot.past[0] || slot.past[1]))
				line += std::string("; not lit past siren20: ") + (slot.notLit ? slot.notLit : "no reason known");
			if (slot.lost[0] || slot.lost[1])
				line += "; lost past bone " + std::to_string(kNameableBones - 1) + ", found by name but not named by its bone structure: " +
					Pair(slot.lost) + " (the highest bone " + std::to_string(slot.lostBone) + ")";
			if (slot.unnamed[0] || slot.unnamed[1])
				line += "; found by name below bone " + std::to_string(kNameableBones) + " but not named by its bone structure: " + Pair(slot.unnamed);
			if (slot.missed[0] || slot.missed[1])
				line += "; named by its bone structure but not found by name: " + Pair(slot.missed);
			if (slot.moved[0] || slot.moved[1])
				line += "; found by name at another bone than its bone structure names: " + Pair(slot.moved);
			return line + ".";
		}

		// A model gets a line of its own when the limit binds it, or when it could not be read or its bones by name and
		// its bone structure disagree - which says the measurement itself is not right for it.
		bool WorthALine(const ModelSlot& slot)
		{
			return slot.unreadable || slot.block || slot.past[0] || slot.past[1] || slot.lost[0] || slot.lost[1] || slot.unnamed[0] || slot.unnamed[1] ||
				slot.missed[0] || slot.missed[1] || slot.moved[0] || slot.moved[1];
		}

		// The models counted so far, then a line for each model that is worth one and has none yet.
		void LogSummary(uint64_t)
		{
			size_t seen = 0, past = 0, lost = 0, unreadable = 0, lit = 0;
			const ModelSlot* most = nullptr;	// the model whose bone structure names the most sirens - of two, the lower hash
			std::vector<ModelSlot*> lines;
			for (ModelSlot& slot : g_models)
			{
				if (!slot.model.load(std::memory_order_acquire) || slot.state.load(std::memory_order_acquire) != kCounted)
					continue;
				seen++;
				if (slot.block)
					lit++;
				if (slot.unreadable)
					unreadable++;
				else
				{
					if (slot.past[0] || slot.past[1])
						past++;
					if (slot.lost[0] || slot.lost[1])
						lost++;
					if (!most || slot.inTable[0] > most->inTable[0] || (slot.inTable[0] == most->inTable[0] && slot.hash < most->hash))
						most = &slot;
				}
				if (!slot.written && WorthALine(slot))
					lines.push_back(&slot);
			}
			std::string line = "DIAGNOSTIC: " + std::to_string(seen) + " vehicle models with sirens seen in " + std::to_string(g_passes.load()) +
				" passes of the siren lights' renderer - the most sirens a model's bone structure names: ";
			if (most)
			{
				char model[32];
				snprintf(model, sizeof(model), " (model 0x%08X)", most->hash);
				line += std::to_string(most->inTable[0]) + " of " + std::to_string(kGameSirens) + model;
			}
			else
				line += "none read";
			line += "; " + std::to_string(past) + " with siren bones past siren20, " + (g_raised ? std::to_string(lit) + " lit past siren20 by the second call" :
				std::string("which the game never lights")) + "; " + std::to_string(lost) +
				" losing siren bones past bone " + std::to_string(kNameableBones - 1) + ", which a bone structure cannot name; " +
				std::to_string(unreadable) + " that could not be read";
			if (const uint64_t notHeld = g_notHeld.load())
				line += "; the table full, " + std::to_string(notHeld) + " passes of models it could not hold not counted";
			WriteLog(kTag, line + ".");
			if (g_raised)
				WriteLog(kTag, "DIAGNOSTIC: the sirens per vehicle at " + std::to_string(kGameSirens + g_lightsPast) + ": " + std::to_string(g_ownCount.load()) +
					" siren settings with lights past 20 in the table of " + std::to_string(kOwnSettings) + ", " + std::to_string(g_ownTwins.load()) + " twins dropped, " +
					std::to_string(g_ownReplaced.load()) + " replaced by a later one with the same 20 lights, " + std::to_string(g_ownNotFit.load()) + " that did not fit or could not be read; " +
					std::to_string(g_sharedScalars.load()) + " models that changed the scalars of a setting another model shares; " + std::to_string(g_secondCalls.load()) +
					" second calls of the renderer.");
			for (ModelSlot* slot : lines)
			{
				WriteLog(kTag, ModelLine(*slot));
				slot->written = true;
			}
		}

		// The first summary 10 seconds after the first model counted, then one a minute while the models change.
		constexpr Measurement kMeasurement = { "siren count summary", kFirstSummaryMs, &Numbers, &LogSummary, &g_firstModel };

		// ---- the sites, found and checked

		void NotMeasured(const std::string& why)
		{
			WriteLog(kTag, "DIAGNOSTIC: the siren count is not measured: " + why);
		}

		// "I-E-B2 at RVA ... does not lie in the siren lights' renderer at RVA ...: the game's unwind data puts it in the function at RVA ...."
		std::string NotInFunction(const GamePattern& pattern, const uint8_t* match, const std::string& what, const uint8_t* function, const uint8_t* start)
		{
			return std::string(pattern.label) + " at " + DescribeAddress(match) + " does not lie in " + what + " at " + DescribeAddress(function) + ": " +
				(start ? "the game's unwind data puts it in the function at " + DescribeAddress(start) : std::string("the game's unwind data names no function there")) + ".";
		}

		// ---- the raise

		bool NotRaised(const std::string& why)
		{
			SirensNotRaised(why);
			return false;
		}

		// A line of the checks, with DIAGNOSTIC_MODE
		void Note(const std::string& line)
		{
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: " + line);
		}

		// The lights' member of the siren settings, by the two tables of their parser registration, as the game has it from the start: its
		// offset in the table and in its data once registered, its type, a fixed array of 20 elements of 0x48 bytes, and its element's data if
		// it has it yet, which the hook asks again.
		bool CheckLightsMember(const SirenSettingSites& sites)
		{
			const char* const label = "I-E-P1";
			const uint8_t* const entry = sites.memberTable + kLightsMember * sizeof(uint64_t);
			const uint8_t* const offset = sites.offsetTable + kLightsMember * sizeof(uint32_t);
			if (!IsInsideGameImage(entry) || !IsReadable(entry, sizeof(uint64_t)) || !IsInsideGameImage(offset) || !IsReadable(offset, sizeof(uint32_t)))
				return NotRaised("the siren settings' parser tables have no lights member: its entries at " + DescribeAddress(entry) + " and " + DescribeAddress(offset) +
					" are not inside the game, or cannot be read (" + label + ").");
			const uint32_t offsetInTable = ValueAt<uint32_t>(offset);
			if (offsetInTable != kLightsAt)
				return NotRaised("the siren settings' lights member lies at +" + Hex(offsetInTable) + ", not +" + Hex(kLightsAt) + " (" + label + ").");
			const uint8_t* const data = ValueAt<const uint8_t*>(entry);
			if (!IsInsideGameData(data) || !IsReadable(data, kDataElementAt + sizeof(void*)))
				return NotRaised("the siren settings' lights member's data at " + DescribeAddress(data) + " do not lie in the game's .data, or cannot be read (" + label + ").");
			const uint64_t offsetField = ValueAt<uint64_t>(data, kDataOffsetAt);
			const uint8_t type = data[kDataTypeAt];
			const uint8_t subtype = data[kDataSubtypeAt];
			const uint64_t elementSize = ValueAt<uint64_t>(data, kDataElementSizeAt);
			const uint32_t capacity = ValueAt<uint32_t>(data, kDataCapacityAt);
			const uint8_t* const element = ValueAt<const uint8_t*>(data, kDataElementAt);
			Note("the siren settings' lights member: entry " + std::to_string(kLightsMember) + " of their parser tables, at +" + Hex(offsetInTable) + ", its data at " + DescribeAddress(data) +
				", offset field " + Hex(offsetField) + ", type " + Hex(type) + ", subtype " + Hex(subtype) + ", element size " + Hex(elementSize) + ", capacity " + std::to_string(capacity) +
				", its element's data at " + DescribeAddress(element) + " (" + label + ")");
			if (offsetField != 0 && offsetField != kLightsAt)
				return NotRaised("the siren settings' lights member's offset field at " + DescribeAddress(data + kDataOffsetAt) + " holds " + Hex(offsetField) + ", not +" + Hex(kLightsAt) +
					" (" + label + ").");
			if (type != kArrayType || subtype != kFixedArraySubtype)
				return NotRaised("the siren settings' lights member's type at " + DescribeAddress(data + kDataTypeAt) + " is " + Hex(type) + " and " + Hex(subtype) + ", not " +
					Hex(kArrayType) + " and " + Hex(kFixedArraySubtype) + ", a fixed array (" + label + ").");
			if (elementSize != kLightSize || capacity != static_cast<uint32_t>(kGameSirens))
				return NotRaised("the siren settings' lights member holds " + std::to_string(capacity) + " elements of " + Hex(elementSize) + " bytes, not " + std::to_string(kGameSirens) +
					" of " + Hex(kLightSize) + " (another mod may have raised them already) (" + label + ").");
			if (element && (!IsInsideGameData(element) || !IsReadable(element, kDataTypeAt + 1) || element[kDataTypeAt] != kStructureType))
				return NotRaised("the siren settings' lights member's element's data at " + DescribeAddress(element) + " are not a structure's in the game's .data (" + label + ").");
			g_lightsData = data;
			return true;
		}

		// The lights' half of the part's one write: finds and checks its sites and adds its writes to the list, writing nothing.
		// False, reported through SirensNotRaised, at the first site that is not as the findings have it.
		bool FindSirenLights(int sirens, SirenWrites& list)
		{
			// 1. every pattern, each looked for, so that the log names every one missing; the constructor and the parser registration from the
			// module that looked for them before its writes changed what they match
			std::string notFound;
			g_call = Scanner::FindOnce(kCallPattern, notFound);
			uint8_t* const getterMatch = Scanner::FindOnce(kGetterCallPattern, notFound);
			uint8_t* const stateSize = Scanner::FindOnce(kStateSizePattern, notFound);
			uint8_t* const reader = Scanner::FindOnce(kReaderPattern, notFound);
			const uint8_t* const brokenRead = Scanner::FindOnce(kBrokenReadPattern, notFound);
			const uint8_t* const flagsRead = Scanner::FindOnce(kFlagsReadPattern, notFound);
			const uint8_t* const groupRead = Scanner::FindOnce(kGroupReadPattern, notFound);
			const uint8_t* const walk = Scanner::FindOnce(kWalkPattern, notFound);
			const std::optional<SirenSettingSites> sites = FindSirenSettingSites(notFound);
			if (!notFound.empty())
				return NotRaised(notFound + ".");

			// 2. the vehicle lights' two calls, each into the game: the renderer's, and the getter's after it
			uint8_t* const rendererCall = g_call + kRendererCallAt;
			const uint8_t* const getterCall = g_call + kGetterCallAt;
			const uint8_t* const renderer = CallTarget(rendererCall);
			const uint8_t* const getter = IsReadable(getterCall, kCallSize) ? CallTarget(getterCall) : nullptr;
			if (!IsInsideGameImage(renderer))
				return NotRaised("the siren lights' renderer call at " + DescribeAddress(rendererCall) + " is not a call into the game (another mod may have hooked it already) (" + kCallPattern.label + ").");
			if (!getter || !IsInsideGameImage(getter))
				return NotRaised("the siren settings getter call at " + DescribeAddress(getterCall) + " is not a call into the game (another mod may have hooked it already) (" + kCallPattern.label + ").");
			if (getter != sites->getter)
				return NotRaised("the siren settings getter call at " + DescribeAddress(getterCall) + " leads to " + DescribeAddress(getter) + ", not to the getter at " +
					DescribeAddress(sites->getter) + " (another mod may have hooked it already) (I-E-M1).");

			// 3. what the second call rests on, each read inside the renderer by the game's unwind data; the renderer's call of the getter the same getter's
			const struct { const uint8_t* match; const GamePattern& pattern; } inRenderer[] = { { getterMatch, kGetterCallPattern }, { brokenRead, kBrokenReadPattern },
				{ flagsRead, kFlagsReadPattern }, { groupRead, kGroupReadPattern } };
			for (const auto& site : inRenderer)
				if (const uint8_t* const start = FunctionStartOf(site.match); start != renderer)
					return NotRaised(NotInFunction(site.pattern, site.match, "the siren lights' renderer", renderer, start));
			uint8_t* const rendererGetterCall = getterMatch + kGetterCallAtZ1;
			const uint8_t* const rendererGetter = CallTarget(rendererGetterCall);
			if (rendererGetter != getter)
				return NotRaised("the renderer's siren settings getter call at " + DescribeAddress(rendererGetterCall) + " leads to " + DescribeAddress(rendererGetter) + ", not to the getter at " +
					DescribeAddress(getter) + " (another mod may have hooked it already) (" + kGetterCallPattern.label + ").");
			Note("the renderer's siren settings getter call at " + DescribeAddress(rendererGetterCall) + " -> " + DescribeAddress(rendererGetter) + ", in the function at " + DescribeAddress(renderer) +
				" (unwind data) (" + kGetterCallPattern.label + ")");
			Note("the vehicle's broken sirens offset at " + DescribeAddress(brokenRead + 4) + " holds " + Hex(ValueAt<uint32_t>(brokenRead, 4)) + " (" + kBrokenReadPattern.label + ")");
			Note("the vehicle's flag byte offset at " + DescribeAddress(flagsRead + 4) + " holds " + Hex(ValueAt<uint32_t>(flagsRead, 4)) + " (" + kFlagsReadPattern.label + ")");
			Note("the vehicle's kind offset at " + DescribeAddress(groupRead + 4) + " " + Holds(groupRead + 4) + ", its matrix's offset at " + DescribeAddress(groupRead + 30) + " " +
				Holds(groupRead + 30) + " (" + kGroupReadPattern.label + ")");

			// 4. the state's size
			Note("the siren state's size at " + DescribeAddress(stateSize + kStateSizeAtZ2) + " holds " + Hex(ValueAt<uint32_t>(stateSize, kStateSizeAtZ2)) + " (" + kStateSizePattern.label + ")");

			// 5. the array member's tree reader, hooked at its start, and its walk over a node's children inside it, by the game's unwind data
			if (const uint8_t* const start = FunctionStartOf(reader); start != reader)
				return NotRaised("the array member's tree reader at " + DescribeAddress(reader) + " is not a function start (" + (start ? "inside the function at " + DescribeAddress(start) +
					" (unwind data)" : std::string("no unwind data")) + ") (" + kReaderPattern.label + ").");
			if (const uint8_t* const start = FunctionStartOf(walk); start != reader)
				return NotRaised(NotInFunction(kWalkPattern, walk, "the array member's tree reader", reader, start));
			Note("the array member's tree reader at " + DescribeAddress(reader) + ", a function start (unwind data) (" + kReaderPattern.label + ")");
			Note("a node's first child offset at " + DescribeAddress(walk + kWalkFirstAt) + " " + Holds(walk + kWalkFirstAt) + ", its next sibling offset at " + DescribeAddress(walk + kWalkNextAt) +
				" " + Holds(walk + kWalkNextAt) + ", in the function at " + DescribeAddress(reader) + " (unwind data) (" + kWalkPattern.label + ")");

			// 6. the siren setting's constructor, which TweaksV calls, and the lights' member
			std::string detail;
			if (!IsFunctionStart(sites->constructor, detail))
				return NotRaised("the siren setting's constructor at " + DescribeAddress(sites->constructor) + " is not a function start (" + detail + ") (I-E-C1).");
			Note("the siren setting's constructor at " + DescribeAddress(sites->constructor) + ", " + detail + " (I-E-C1)");
			if (!CheckLightsMember(*sites))
				return false;

			// 7. the part's writes, added to the list; the hook and the calls run nothing until they are written
			g_lightsPast = sirens - kGameSirens;
			g_constructor = reinterpret_cast<SettingConstructor>(const_cast<uint8_t*>(sites->constructor));
			g_getter = sites->getter;
			g_carcolsPointer = sites->carcolsPointer;
			list.hooks.push_back({ reader, reinterpret_cast<const void*>(&HookedArrayReader), reinterpret_cast<void**>(&g_arrayReader), "The array member's tree reader" });
			list.redirects.push_back({ rendererCall, reinterpret_cast<const void*>(&SirenLightsTwice), "siren lights' renderer call", reinterpret_cast<void**>(&g_renderer) });
			list.redirects.push_back({ rendererGetterCall, reinterpret_cast<const void*>(&SirenSettingsOf), "renderer's siren settings getter call", nullptr });
			list.redirects.push_back({ g_call + kGetterCallAt, reinterpret_cast<const void*>(&SirenSettingsOf), "siren settings getter call", nullptr });
			list.writes.push_back({ stateSize + kStateSizeAtZ2, BytesOf(kRaisedStateSize), "siren state's size" });
			return true;
		}

		// The part's hooks off again, its writes not made; false if one stays on, which passes the calls on, and keeps settings no call asks for. A line about
		// that which cannot be written does not stop the next.
		bool TakeHooksBack(const std::vector<FunctionHook>& hooks)
		{
			bool allOff = true;
			for (const FunctionHook& hook : hooks)
				try
				{
					if (!DisableHook(hook.function, kTag))
						allOff = false;
				}
				catch (...)
				{
					allOff = false;
				}
			return allOff;
		}
	}

	void SirensNotRaised(const std::string& why)
	{
		ReportFailure(kTag, "The sirens per vehicle are not raised this session: " + why);
	}

	void InitSirensPerVehicle()
	{
		const int sirens = GetNumber(kSirensPerVehicle);
		if (sirens <= kGameSirens)
		{
			WriteLog(kTag, "SIRENS_PER_VEHICLE=" + std::to_string(sirens) + " - the sirens per vehicle are not raised.");
			return;
		}

		// 1. the game's build: no pattern proves that the renderer reads nothing new of its stand-ins, so the raise runs only on
		// a build whose renderer was read (D40)
		constexpr GameVersion kRendererReadOn{ 1, 0, 1158, 16 };
		GameVersion version;
		const bool known = ReadGameVersion(version);
		if (version.Number() != kRendererReadOn.Number())
		{
			SirensNotRaised("this game build is " + (known ? std::to_string(version.major) + "." + std::to_string(version.minor) + "." +
				std::to_string(version.build) + "." + std::to_string(version.revision) : std::string("not known")) + ", and TweaksV has read the "
				"siren lights' renderer of 1.0.1158.16 only. Vehicles keep 20 sirens until a new TweaksV.");
			return;
		}

		// 2. the light stores: the game's own lose lights, and overflow, at more sirens than its 20 (D39)
		if (!LightStoresRaised())
		{
			SirensNotRaised("the light stores are not raised ([LIGHTCAPACITY] in TweaksV.ini), and the game's own do not take " + std::to_string(sirens) +
				" sirens a vehicle. Vehicles keep 20 sirens.");
			return;
		}

		// 3. every site of both halves found and checked, nothing written yet
		SirenWrites list;
		if (!FindSirenLights(sirens, list) || !FindSirenDamage(sirens, list))
			return;

#ifdef TWEAKSV_TEST_BUILD
		// test builds only: TWEAKSV_TEST_SIREN_WRITE_FAILS in the environment reaches what comes of the one write failing once the hook is on: 1 it is refused, as a
		// page Windows will not make writable would, by a write without an address, which stops all of it; 2 an exception in its place; 3 refused, and a hook
		// that cannot then be switched off, being off already; 4 an exception once it is done; 5 the hook refused, by a function that is none, since Windows' dynamic code
		// policy stops the light stores before it
		wchar_t failure[4]{};
		const wchar_t writeFails = GetEnvironmentVariableW(L"TWEAKSV_TEST_SIREN_WRITE_FAILS", failure, 4) == 1 ? failure[0] : L'0';
		if (writeFails >= L'1' && writeFails <= L'5')
			WriteLog(kTag, std::string("TEST BUILD: the sirens' one write ") + (writeFails == L'1' ? "is refused" : writeFails == L'2' ? "throws in its place" :
				writeFails == L'3' ? "is refused and its hook cannot be switched off" : writeFails == L'5' ? "has its hook refused" : "throws once it is done") + " (TWEAKSV_TEST_SIREN_WRITE_FAILS=" +
				static_cast<char>(writeFails) + ").");
		if (writeFails == L'1' || writeFails == L'3')
			list.writes.push_back({ nullptr, {}, "write a test build refuses" });
		if (writeFails == L'5')
			list.hooks.front().function = nullptr;

		// TEST_SIREN_PASSES under [TWEAKSV], read before the wrapper can run
		const std::string& testPasses = GetText(kTestSirenPasses);
		if (!testPasses.empty())
		{
			char* end = nullptr;
			const unsigned long passes = strtoul(testPasses.c_str(), &end, 10);
			if (*end != '\0' || passes > 31)
				ReportFailure(kTag, "TweaksV.ini: [TWEAKSV] TEST_SIREN_PASSES=" + testPasses + " is not a number from 0 to 31, so it is ignored.");
			else
				g_testPasses = static_cast<int>(passes);
		}
#endif

		// 4. one write, all or nothing: the hooks first, since a hook can be taken back, then the calls and the constants together
		const bool dryRun = IsDryRun();
		if (dryRun)
			for (const FunctionHook& hook : list.hooks)
			{
				std::string what = hook.what;
				what[0] = static_cast<char>(tolower(static_cast<unsigned char>(what[0])));
				WriteLog(kTag, "DRY_RUN: would hook " + what + " at " + DescribeAddress(hook.function) + ".");
			}
		else if (!list.hooks.empty() && !HookFunctions(list.hooks, kTag))
		{
			SirensNotRaised("the functions could not be hooked (details above). Nothing is changed.");
			return;
		}
#ifdef TWEAKSV_TEST_BUILD
		if (writeFails == L'3' && !dryRun)
			TakeHooksBack(list.hooks);
#endif
		bool written = false;
		bool threw = false;
		try
		{
#ifdef TWEAKSV_TEST_BUILD
			if (writeFails == L'2')
				throw 2;
#endif
			written = RedirectCalls(list.redirects, list.writes, kTag);
#ifdef TWEAKSV_TEST_BUILD
			if (writeFails == L'4')
				throw 4;
#endif
		}
		catch (...)
		{
			// a copy of the writes that could not be made, with the hooks on - or a line after the writes, which are in then: the renderer's call says which
			threw = true;
			written = g_renderer && CallTarget(list.redirects.front().site) != reinterpret_cast<const uint8_t*>(g_renderer);
		}
		if (!written)
		{
			const bool hooksOff = dryRun || TakeHooksBack(list.hooks);
			SirensNotRaised(std::string(threw ? "its writes ended in an unexpected exception." : "its writes could not be made (details above).") +
				(hooksOff ? " Nothing is changed." : " Its hook stays on and passes the calls on (details above); nothing else is changed."));
			return;
		}
		g_wouldRaise = dryRun;
		g_raised = !dryRun;
		if (!dryRun)
			WriteLog(kTag, "The sirens per vehicle are raised to " + std::to_string(sirens) + ".");
#ifdef TWEAKSV_TEST_BUILD
		if (!dryRun && g_testPasses)
			WriteLog(kTag, "TEST BUILD: each pass of the siren lights' renderer runs " + std::to_string(g_testPasses) + " more times, the second call with it (TEST_SIREN_PASSES=" +
				std::to_string(g_testPasses) + ", up to 31).");
#endif
	}

	void MeasureSirenCount()
	{
		// 1. the four sites, each looked for, so that the log names every one missing; the raise has looked for the call already
		std::string notFound;
		uint8_t* const call = g_call ? g_call : Scanner::FindOnce(kCallPattern, notFound);
		const uint8_t* const boneRead = Scanner::FindOnce(kBoneReadPattern, notFound);
		const uint8_t* const glassRead = Scanner::FindOnce(kGlassReadPattern, notFound);
		const uint8_t* const findBone = Scanner::FindOnce(kFindBonePattern, notFound);
		if (!call || !boneRead || !glassRead || !findBone)
		{
			NotMeasured(notFound + ".");
			return;
		}

		// 2. the vehicle lights' two calls, each into the game: the renderer's, and the getter's after it, which takes the
		// same model info
		uint8_t* const rendererCall = call + kRendererCallAt;
		const uint8_t* const getterCall = call + kGetterCallAt;
		const uint8_t* const renderer = g_raised ? reinterpret_cast<const uint8_t*>(g_renderer) : CallTarget(rendererCall);
		const uint8_t* const getter = g_raised ? g_getter : IsReadable(getterCall, kCallSize) ? CallTarget(getterCall) : nullptr;
		WriteLog(kTag, "DIAGNOSTIC: siren lights' renderer call at " + DescribeAddress(rendererCall) + " -> " + DescribeAddress(renderer) +
			", siren settings getter call at " + DescribeAddress(getterCall) + " -> " +
			(getter ? DescribeAddress(getter) : std::string("cannot be read")) + " (" + kCallPattern.label + ")");
		if (!IsInsideGameImage(renderer))
		{
			NotMeasured("the siren lights' renderer call at " + DescribeAddress(rendererCall) + " is not a call into the game (another mod may "
				"have hooked it already) (" + kCallPattern.label + ").");
			return;
		}
		if (!getter || !IsInsideGameImage(getter))
		{
			NotMeasured("the siren settings getter call at " + DescribeAddress(getterCall) + " is not a call into the game (another mod may "
				"have hooked it already) (" + kCallPattern.label + ").");
			return;
		}

		// 3. the renderer is the function its bone read lies in, by the game's unwind data
		const uint8_t* const function = FunctionStartOf(boneRead);
		WriteLog(kTag, "DIAGNOSTIC: bone structure offset at " + DescribeAddress(boneRead + kBonesRead) + " " + Holds(boneRead + kBonesRead) +
			", siren1's bone offset at " + DescribeAddress(boneRead + kSirenBonesRead) + " " + Holds(boneRead + kSirenBonesRead) + ", in " +
			(function ? "the function at " + DescribeAddress(function) + " (unwind data)" : std::string("no function the game's unwind data names")) +
			" (" + kBoneReadPattern.label + ")");
		if (function != renderer)
		{
			NotMeasured(NotInFunction(kBoneReadPattern, boneRead, "the siren lights' renderer", renderer, function));
			return;
		}

		// 4. the glass bones' bytes, and find-bone-by-name: a function start by the game's unwind data, as the wrapper calls it
		WriteLog(kTag, "DIAGNOSTIC: siren_glass1's bone offset at " + DescribeAddress(glassRead + kGlassBonesRead) + " " +
			Holds(glassRead + kGlassBonesRead) + " (" + kGlassReadPattern.label + ")");
		const uint8_t* const findBoneFunction = FunctionStartOf(findBone);
		const std::string start = findBoneFunction == findBone ? std::string("a function start (unwind data)") : findBoneFunction ?
			"inside the function at " + DescribeAddress(findBoneFunction) + " (unwind data)" : std::string("no unwind data");
		WriteLog(kTag, "DIAGNOSTIC: find-bone-by-name at " + DescribeAddress(findBone) + ", " + start + ", bone count offset at " +
			DescribeAddress(findBone + kBoneCountRead) + " " + Holds(findBone + kBoneCountRead) + " (" + kFindBonePattern.label + ")");
		if (findBoneFunction != findBone)
		{
			NotMeasured("find-bone-by-name at " + DescribeAddress(findBone) + " is not a function start (" + start + ") (" + kFindBonePattern.label + ").");
			return;
		}
		g_findBone = reinterpret_cast<FindBone>(const_cast<uint8_t*>(findBone));

		// 5. a dry run: the line that would redirect the call, the raise's if it would, nothing counted
		const std::vector<CallRedirect> redirect = { { rendererCall, reinterpret_cast<const void*>(&CountedSirenLights), "siren lights' renderer call",
			reinterpret_cast<void**>(&g_renderer) } };
		if (IsDryRun())
		{
			if (!g_wouldRaise)
				RedirectCalls(redirect, {}, kTag);
			NotMeasured("DRY_RUN - the siren lights' renderer call is not redirected, so nothing is counted.");
			return;
		}

		// 6. the summary's thread, waiting for the first model counted - then the call, unless the raise's wrapper is in it, which counts too:
		// a measurement that cannot sum up redirects nothing, and a call that cannot be redirected counts nothing
		g_firstModel = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (!g_firstModel)
		{
			NotMeasured("its summary's event could not be made (" + DescribeError(GetLastError()) + ").");
			return;
		}
		if (!StartMeasurement<kMeasurement>())	// its line is written
		{
			NotMeasured("its summary thread could not be started.");
			return;
		}
		if (!g_raised && !RedirectCalls(redirect, {}, kTag))
		{
			NotMeasured("the siren lights' renderer call at " + DescribeAddress(rendererCall) + " could not be redirected (details above).");
			return;
		}
		g_counting = true;
		WriteLog(kTag, "DIAGNOSTIC: the siren lights' renderer call at " + DescribeAddress(rendererCall) + " is redirected (" + kCallPattern.label +
			"): the vehicle models with sirens on, each counted once with its siren bones looked up by name up to siren" +
			std::to_string(kLookedUp) + ", are logged 10 seconds after the first, then every minute while they change.");
	}
}
