// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the sirens per vehicle's damage (see TweaksVSirenDamage.h).
//
// A model's bone structure names the bones of siren1 to siren20 and of their glass by signed bytes, and six functions of the
// game's damage code run over those bytes: a hit near a siren breaks it, an impact marks it and another function hides what
// the impact marked, a wreck breaks every siren whose bone exists, and four loops give the glass's fragment children their
// flags. None of them reads anything else sized by 20 - only the bytes, the bits of the 32-bit sets and the bones - so they
// are raised by constants alone (PATTERNS.md, "SirenSettings", the sirens per vehicle's damage):
//   the room        the structures' pool's stride and the allocation grow by 64 bytes, kRoomSirenBonesAt on
//   its fill        the call of the function that makes a structure is redirected; the wrapper calls it, then fills the room,
//                   at a model's load, never per frame: to 0xFF, the game's 40 bytes copied in, as they stand after the game's
//                   fills, so that a plane's gear doors at those IDs keep their slots, then the game's own fill once more, on a
//                   table of 24 bones of TweaksV's own, siren21 and siren_glass21 on, whose IDs lie in the room
//   the 19 writes   each loop's bound and each read raised to the room, the loop of a hit by its counter, since its reads have
//                   1-byte displacements
// The room is 64 bytes whatever SIRENS_PER_VEHICLE says, and the wrapper fills all of it; only the loops' bounds take the key.
// The loop of the stored damage state stays at 20: it reads 20 bytes of sync data, and past them other fields (D23). The game's
// own table stays what every reader not raised reads, and S's own fields do not move.
// Nothing is written here: FindSirenDamage finds and checks every site and adds to the part's one write, which
// InitSirensPerVehicle makes (TweaksVSirenCount.cpp), so DRY_RUN and DIAGNOSTIC_MODE are the patch module's.
// Written for TweaksV from the research's own findings in the game's code (PATTERNS.md); no code of anyone's was read.

#include "TweaksVSirenDamage.h"

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
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"
#include "TweaksVSirenRoom.h"
#include "TweaksVSummary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "SirenSettings";

		// ---- the sites (PATTERNS.md, "SirenSettings", the sirens per vehicle's damage)

		enum SiteName : size_t
		{
			kHitStart, kHitEnd, kHitBone, kHitGlass,	// the hit: its counter's start, its end and the index made from it, the checked read, the glass
			kImpactBound, kImpactBone, kImpactGlass,	// the impact
			kHideBound, kHideBone,						// the bones the impact marked, hidden
			kWreckBound, kWreckBone,					// the wreck
			kFlagsBound, kFlagsGlass,					// the glass's fragment children's flags
			kClassStart, kClassGlass,					// the same in three classes, from -20: each pattern matches 3 times
			kPoolStride, kAllocation, kMakerCall, kFill,	// the room: its pool and allocation, the redirected call, the game's fill
			kSiteCount
		};
		struct Site
		{
			const GamePattern* pattern;
			size_t matches;		// how often the game's code holds it
		};
		constexpr Site kSites[kSiteCount] = {
			{ &Pattern("I-E-X1"), 1 }, { &Pattern("I-E-X2"), 1 }, { &Pattern("I-E-X4"), 1 }, { &Pattern("I-E-X3"), 1 },
			{ &Pattern("I-E-L05"), 1 }, { &Pattern("I-E-N04"), 1 }, { &Pattern("I-E-N05"), 1 },
			{ &Pattern("I-E-L03"), 1 }, { &Pattern("I-E-N03"), 1 },
			{ &Pattern("I-E-L04"), 1 }, { &Pattern("I-E-N02"), 1 },
			{ &Pattern("I-E-L07"), 1 }, { &Pattern("I-E-N06"), 1 },
			{ &Pattern("I-E-L08"), 3 }, { &Pattern("I-E-N09"), 3 },
			{ &Pattern("I-E-Y1"), 1 }, { &Pattern("I-E-Y2"), 1 }, { &Pattern("I-E-Y3"), 1 }, { &Pattern("I-E-Y4"), 1 },
		};

		// The bone structure's byte at kBonesByIdAt + a hierarchy ID is that ID's bone: siren1's at 0x99, siren_glass1's at 0xAD. The
		// loop of a hit counts in those IDs, so its counter moves until kBonesByIdAt + it lies in the room.
		constexpr int32_t kBonesByIdAt = 0x10;
		constexpr int32_t kSirenBonesAt = 0x99;
		constexpr int32_t kGlassBonesAt = 0xAD;
		constexpr int32_t kRoomSirens = kRoomSirenBonesAt;
		constexpr int32_t kRoomGlass = kRoomGlassBonesAt;
		constexpr int32_t kHitFirstId = kRoomSirens - kBonesByIdAt;
		constexpr int kRoomBones = kRoomGlass - kRoomSirens;				// a kind's bones in the room: 32, the top of the game's 32-bit sets
		static_assert(kRoomEnd == kRoomGlassBonesAt + kRoomBones && kRoomBones == 32 && kGlassBonesAt == kSirenBonesAt + kGameSirens,
			"TweaksV: the room holds 32 sirens' bones, then 32 of the glass's");

		// One operand of the raise, in every match of its site: what the game's code holds there, and what it holds raised -
		// raised + perSiren x SIRENS_PER_VEHICLE - a byte or a dword.
		struct Operand
		{
			SiteName site;
			size_t at;			// in the match
			size_t size;		// 1 or 4 bytes
			int32_t game;
			int32_t raised;
			int32_t perSiren;
			const char* what;	// for the write's line, which names its pattern too
		};
		constexpr Operand kOperands[] = {
			{ kHitStart, 1, 4, 0x89, kHitFirstId, 0, "siren hit loop's start" },
			{ kHitEnd, 3, 4, 0x9D, kHitFirstId, 1, "siren hit loop's end" },
			{ kHitEnd, 15, 4, -0x89, -kHitFirstId, 0, "siren hit loop's index" },
			{ kHitGlass, 4, 1, kBonesByIdAt + kGameSirens, kBonesByIdAt + kRoomBones, 0, "siren hit loop's glass bone" },
			{ kImpactBound, 3, 1, kGameSirens, 0, 1, "siren impact loop's bound" },
			{ kImpactBone, 4, 4, kSirenBonesAt, kRoomSirens, 0, "siren impact loop's bone" },
			{ kImpactGlass, 4, 4, kGlassBonesAt, kRoomGlass, 0, "siren impact loop's glass bone" },
			{ kHideBound, 3, 1, kGameSirens, 0, 1, "hidden sirens' loop's bound" },
			{ kHideBone, 4, 4, kSirenBonesAt, kRoomSirens, 0, "hidden sirens' loop's bone" },
			{ kWreckBound, 3, 1, kGameSirens, 0, 1, "wrecked sirens' loop's bound" },
			{ kWreckBone, 4, 4, kSirenBonesAt, kRoomSirens, 0, "wrecked sirens' loop's bone" },
			{ kFlagsBound, 3, 1, kGameSirens, 0, 1, "siren glass flags' loop's bound" },
			{ kFlagsGlass, 4, 4, kGlassBonesAt, kRoomGlass, 0, "siren glass flags' loop's glass bone" },
			{ kClassStart, 3, 4, -kGameSirens, 0, -1, "siren glass flags' class's start" },
			{ kClassGlass, 4, 4, kGlassBonesAt + kGameSirens, kRoomGlass, 1, "siren glass flags' class's glass bone" },
			{ kPoolStride, 4, 4, kRoomSirens, kRoomEnd, 0, "bone structures' pool's stride" },
			{ kAllocation, 1, 4, kRoomSirens, kRoomEnd, 0, "bone structure's allocation" },
		};

		// Every operand a byte its pattern pins, the game's own value - so a site another mod changed is a pattern not found - and
		// the findings' 19 damage writes.
		consteval bool OperandsPinned()
		{
			for (const Operand& operand : kOperands)
				for (size_t i = 0; i < operand.size; i++)
					if (PatternByte(*kSites[operand.site].pattern, operand.at + i) != static_cast<int>((static_cast<uint32_t>(operand.game) >> (8 * i)) & 0xFF))
						return false;
			return true;
		}
		static_assert(OperandsPinned(), "TweaksV: every operand the raise writes is the game's own value, pinned by its pattern");
		consteval size_t DamageWrites()
		{
			size_t writes = 0;
			for (const Operand& operand : kOperands)
				if (operand.site != kPoolStride && operand.site != kAllocation)
					writes += kSites[operand.site].matches;
			return writes;
		}
		static_assert(DamageWrites() == 19, "TweaksV: the findings' 19 damage writes");

		// Each site of a loop lies behind the one before it, in the same loop: the findings have them 0x1E to 0x172 bytes apart, and
		// the matches of the three classes pair up in address order.
		struct LoopStep
		{
			SiteName first, then;
		};
		constexpr LoopStep kLoopSteps[] = { { kHitStart, kHitEnd }, { kHitEnd, kHitBone }, { kHitBone, kHitGlass },
			{ kImpactBound, kImpactBone }, { kImpactBone, kImpactGlass }, { kHideBound, kHideBone }, { kWreckBound, kWreckBone },
			{ kFlagsBound, kFlagsGlass }, { kClassStart, kClassGlass } };
		constexpr ptrdiff_t kLoopBytes = 0x200;
		consteval bool StepsPair()
		{
			for (const LoopStep& step : kLoopSteps)
				if (kSites[step.first].matches != kSites[step.then].matches)
					return false;
			return true;
		}
		static_assert(StepsPair(), "TweaksV: a loop's sites match equally often");

		// The model info leads through two pointers from its +0xB0 to the bone structure, as the glass flags' loop reads it; the
		// structure's first 16 bytes are a set of 128 bits, one per bone index. A signed byte names a bone below 128.
		constexpr size_t kMakerCallAt = 3;		// in I-E-Y3's match: "call rel32" of the function that makes a structure
		constexpr uint8_t kPoolLoad[] = { 0x48, 0x8B, 0x0D };	// right before I-E-Y2's match, by the findings: "mov rcx, [rip+disp32]", the pool's pointer
		constexpr size_t kHitBoneRead = 5;		// in I-E-X4's match: the disp8 of "movzx r12d, byte [rax+rdi+0x10]", the one read it leaves
		constexpr size_t kHolderAt = 0xB0;		// in the model info: the pointer to the bone structure's holder
		constexpr size_t kHolderRead = 13;		// in I-E-L07's match: the disp32 of "mov rax, [rax+0xB0]"
		constexpr size_t kNameHashAt = 0x18;	// in the model info: the hash of the model's name, which names it in the log
		constexpr int kSetWordBits = 32;
		static_assert(PatternByte(*kSites[kMakerCall].pattern, kMakerCallAt) == 0xE8, "TweaksV: I-E-Y3 holds the call at +3");
		static_assert(PatternByte(*kSites[kHitBone].pattern, kHitBoneRead) == kBonesByIdAt, "TweaksV: I-E-X4 reads a bone by its ID at +0x10");
		static_assert(PatternImm32(*kSites[kFlagsBound].pattern, kHolderRead) == kHolderAt, "TweaksV: I-E-L07 reads the model info's +0xB0");

		// ---- the bones past siren20: names and tags made when TweaksV is compiled

		// The tag the game's own lookup knows a bone by: letters folded to upper case, an ELF-style hash, modulo 0xFE8F, plus 0x170 -
		// so its fill finds a bone whatever the case of its name.
		consteval uint16_t BoneTag(std::string_view name)
		{
			uint32_t hash = 0;
			for (const char c : name)
			{
				hash = (hash << 4) + static_cast<uint8_t>(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c);
				const uint32_t top = hash & 0xF0000000u;
				hash = (hash ^ (top >> 24)) & ~top;
			}
			return static_cast<uint16_t>(hash % 0xFE8Fu + 0x170u);
		}
		static_assert(BoneTag("chassis") == 0xCD54 && BoneTag("siren1") == 0x61A7 && BoneTag("siren20") == 0x1642 && BoneTag("siren_glass1") == 0xF688,
			"TweaksV: the tags the game's own table holds, read by the data window's boot");
		static_assert(BoneTag("siren21") == 0x1643 && BoneTag("siren32") == 0x1674 && BoneTag("siren_glass21") == 0x4055 && BoneTag("siren_glass32") == 0x4026,
			"TweaksV: the tags past siren20, as the findings have them");

		constexpr int kPastGame = kRoomBones - kGameSirens;		// siren21 to siren32

		struct BoneName
		{
			char text[16];
		};
		consteval BoneName MakeBoneName(std::string_view prefix, int n)
		{
			BoneName name{};
			size_t i = 0;
			for (const char c : prefix)
				name.text[i++] = c;
			name.text[i++] = static_cast<char>('0' + n / 10);
			name.text[i] = static_cast<char>('0' + n % 10);
			return name;
		}
		consteval std::array<BoneName, 2 * kPastGame> MakeBoneNames()
		{
			std::array<BoneName, 2 * kPastGame> names{};
			for (int k = 0; k < kPastGame; k++)
			{
				names[k] = MakeBoneName("siren", kGameSirens + 1 + k);
				names[kPastGame + k] = MakeBoneName("siren_glass", kGameSirens + 1 + k);
			}
			return names;
		}
		constexpr std::array<BoneName, 2 * kPastGame> kBoneNames = MakeBoneNames();
		static_assert(std::string_view(kBoneNames[0].text) == "siren21" && std::string_view(kBoneNames[kPastGame - 1].text) == "siren32" &&
			std::string_view(kBoneNames[kPastGame].text) == "siren_glass21" && std::string_view(kBoneNames[2 * kPastGame - 1].text) == "siren_glass32",
			"TweaksV: the bones past siren20's names");

		// The game's fill reads a table of these in order, until a null name, with a tag per entry, and writes the byte of each bone
		// it finds at kBonesByIdAt + its ID, which it does not check: so an ID can lie in the room.
		struct FillEntry
		{
			const char* name;
			int32_t id;
		};
		static_assert(sizeof(FillEntry) == 16 && offsetof(FillEntry, id) == 8, "TweaksV: the game's fill reads entries of 16 bytes, a name and an ID");
		consteval std::array<FillEntry, 2 * kPastGame + 1> MakeFillTable()
		{
			std::array<FillEntry, 2 * kPastGame + 1> table{};
			for (int k = 0; k < kPastGame; k++)
			{
				table[k] = { kBoneNames[k].text, kRoomSirens + kGameSirens - kBonesByIdAt + k };
				table[kPastGame + k] = { kBoneNames[kPastGame + k].text, kRoomGlass + kGameSirens - kBonesByIdAt + k };
			}
			return table;
		}
		consteval std::array<uint16_t, 2 * kPastGame> MakeFillTags()
		{
			std::array<uint16_t, 2 * kPastGame> tags{};
			for (int k = 0; k < 2 * kPastGame; k++)
				tags[k] = BoneTag(kBoneNames[k].text);
			return tags;
		}
		constexpr std::array<FillEntry, 2 * kPastGame + 1> kFillTable = MakeFillTable();
		constexpr std::array<uint16_t, 2 * kPastGame> kFillTags = MakeFillTags();
		constexpr uint32_t kFillFlag = 1;		// as the game's own fills after its first
		static_assert(kFillTable[0].id == 0x350 && kFillTable[kPastGame].id == 0x370, "TweaksV: siren21's and siren_glass21's IDs in the room");

		// ---- the wrapper, at a model's load

		// The function that makes a structure, called with the model info; every register argument is handed on, since the
		// findings name only the first, and its rax back.
		using MakeBones = uint64_t (*)(const uint8_t* model, uint64_t second, uint64_t third, uint64_t fourth);
		using FillBones = void (*)(const uint8_t* model, const FillEntry* table, const uint16_t* tags, uint32_t flag);
		MakeBones g_maker = nullptr;	// set before the call is redirected
		FillBones g_fill = nullptr;

		std::atomic<bool> g_roomProblem{ false };	// one problem, naming the first model
		std::atomic<uint64_t> g_roomLines{ 0 };

		// The bone structure of a model the game has just made it for, the game's data never trusted: false if a pointer cannot be
		// read; nullptr in structure where the holder or the structure is null, which the game's own readers allow, with nothing to fill.
		bool ReadStructure(const uint8_t* model, uint8_t*& structure)
		{
			const uint8_t* holder = nullptr;
			structure = nullptr;
			if (!CopyMemorySafely(&holder, model + kHolderAt, sizeof(holder)))
				return false;
			return !holder || CopyMemorySafely(&structure, holder, sizeof(structure));
		}

		// The room, each access violation caught - false then: the room to 0xFF, the game's bytes copied in, then the game's own fill.
		// What the fill leaves in a byte is the low byte of a bone's index, so a bone past 127 reads below 0: past siren20 it becomes
		// -1, a bone not there, since the raised loops ask for -1 and no such byte ever reached them before; and for parity with the
		// game's own code, which clears the bits of the bones with IDs, a bone it names has its bit of the set cleared (no reader of
		// that set was found: a hint, PATTERNS.md). A fill that faults leaves all of them -1, whatever it had written.
		bool FillSafely(const uint8_t* model, uint8_t* structure)
		{
			__try
			{
				memset(structure + kRoomSirens, 0xFF, kRoomBones * 2);
				memcpy(structure + kRoomSirens, structure + kSirenBonesAt, kGameSirens);
				memcpy(structure + kRoomGlass, structure + kGlassBonesAt, kGameSirens);
			}
			__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				return false;
			}
			bool filled = true;
			__try
			{
				g_fill(model, kFillTable.data(), kFillTags.data(), kFillFlag);
			}
			__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
			{
				filled = false;
			}
			for (const int32_t kind : { kRoomSirens, kRoomGlass })
				for (int k = kGameSirens; k < kRoomBones; k++)
				{
					int8_t& bone = reinterpret_cast<int8_t&>(structure[kind + k]);
					if (!filled || bone < 0)
						bone = -1;
					else
						reinterpret_cast<uint32_t*>(structure)[bone / kSetWordBits] &= ~(uint32_t{ 1 } << (bone % kSetWordBits));
				}
			return filled;
		}

		// With DIAGNOSTIC_MODE=1: a line for each model the room gives bones past siren20, which says that the game's fill found them.
		void LogRoom(uint32_t hash, const uint8_t* structure)
		{
			int found[2] = {}, highest[2] = {};
			for (int kind = 0; kind < 2; kind++)
				for (int k = kGameSirens; k < kRoomBones; k++)
					if (static_cast<int8_t>(structure[(kind ? kRoomGlass : kRoomSirens) + k]) >= 0)
					{
						found[kind]++;
						highest[kind] = k + 1;
					}
			if (!found[0] && !found[1])
				return;
			const uint64_t line = g_roomLines.fetch_add(1, std::memory_order_relaxed) + 1;
			if (line == kMaxLinesPerKind + 1)
				WriteLog(kTag, "DIAGNOSTIC: " + std::to_string(kMaxLinesPerKind) + " lines of vehicle models' rooms written - from now on they are not written.");
			if (line > kMaxLinesPerKind)
				return;
			char model[32];
			snprintf(model, sizeof(model), "vehicle model 0x%08X", hash);
			const auto upTo = [&](int kind, const char* name) { return highest[kind] ? name + std::to_string(highest[kind]) : "no " + std::string(name); };
			WriteLog(kTag, std::string("DIAGNOSTIC: the room of ") + model + " holds " + std::to_string(found[0]) + " siren and " + std::to_string(found[1]) +
				" glass bones past siren20, up to " + upTo(0, "siren") + " and " + upTo(1, "siren_glass") + ".");
		}

		// What keeps a model from its room is said once, in the box of a part that stopped working; such a model's sirens past 20
		// stay dark and unbroken, and the first 20 work as ever if its structure could be read.
		void RoomNotFilled(uint32_t hash, const char* why)
		{
			if (g_roomProblem.exchange(true))
				return;
			char model[32];
			snprintf(model, sizeof(model), "vehicle model 0x%08X", hash);
			ReportFailure(kTag, std::string("The bones past siren20 are not looked up for every vehicle model this session: ") + model + " " + why +
				", so its sirens past 20 stay dark. Only the first such model is named.");
		}

		void FillRoom(const uint8_t* model)
		{
			try
			{
				uint32_t hash = 0;
				CopyMemorySafely(&hash, model + kNameHashAt, sizeof(hash));
				uint8_t* structure = nullptr;
				if (!ReadStructure(model, structure))
					RoomNotFilled(hash, "has a bone structure that cannot be read (the model info's +0xB0)");
				else if (structure && !FillSafely(model, structure))
					RoomNotFilled(hash, "faulted while its room was filled (an access violation)");
				else if (structure && IsDiagnosticMode())
					LogRoom(hash, structure);
			}
			catch (...)
			{
				// a line that could not be written: nothing throws into the game
			}
		}

		// The game's call of the function that makes a structure, redirected: that function first, then the room, the caller's last
		// error kept around it - on the game's thread, once per model, never per frame.
		uint64_t MadeBones(const uint8_t* model, uint64_t second, uint64_t third, uint64_t fourth)
		{
			const uint64_t result = g_maker(model, second, third, fourth);
			const DWORD lastError = GetLastError();
			FillRoom(model);
			SetLastError(lastError);
			return result;
		}

		// value's first size bytes as they lie in memory, for a write.
		std::vector<uint8_t> OperandBytes(int32_t value, size_t size)
		{
			return size == 1 ? BytesOf(static_cast<uint8_t>(value)) : BytesOf(value);
		}
	}

	bool FindSirenDamage(int sirens, SirenWrites& list)
	{
		// 1. every site, each looked for, so that the log names every one missing
		std::array<std::vector<uint8_t*>, kSiteCount> found;
		std::string notFound;
		for (size_t i = 0; i < kSiteCount; i++)
			if (kSites[i].matches == 1)
			{
				if (uint8_t* const match = Scanner::FindOnce(*kSites[i].pattern, notFound))
					found[i].push_back(match);
			}
			else
			{
				found[i] = Scanner::FindAll(*kSites[i].pattern, kSites[i].matches + 1);
				if (found[i].size() != kSites[i].matches && notFound.empty())
					notFound = Scanner::NotAsOften(*kSites[i].pattern, found[i].size(), kSites[i].matches);
			}
		if (!notFound.empty())
		{
			SirensNotRaised(notFound + ". Vehicles keep 20 sirens.");
			return false;
		}

		// 2. each loop's sites in their order, in one loop
		for (const LoopStep& step : kLoopSteps)
			for (size_t m = 0; m < found[step.first].size(); m++)
			{
				const uint8_t* const first = found[step.first][m];
				const uint8_t* const then = found[step.then][m];
				if (then <= first || then - first > kLoopBytes)
				{
					SirensNotRaised(std::string(kSites[step.then].pattern->label) + " at " + DescribeAddress(then) + " does not lie behind " +
						kSites[step.first].pattern->label + " at " + DescribeAddress(first) + " in one loop. Vehicles keep 20 sirens.");
					return false;
				}
			}

		// 3. the call of the function that makes a structure: into the game, to the function the allocation lies in, by the game's
		// unwind data
		uint8_t* const makerCall = found[kMakerCall][0] + kMakerCallAt;
		const uint8_t* const allocation = found[kAllocation][0];
		const uint8_t* const fill = found[kFill][0];
		const uint8_t* const maker = CallTarget(makerCall);
		if (!maker || !IsInsideGameImage(maker))
		{
			SirensNotRaised("the bone structure maker's call at " + DescribeAddress(makerCall) + " is not a call into the game (another mod may have hooked it "
				"already)" + At(*kSites[kMakerCall].pattern) + ". Vehicles keep 20 sirens.");
			return false;
		}
		const uint8_t* const allocating = FunctionStartOf(allocation);
		if (allocating != maker)
		{
			SirensNotRaised(std::string(kSites[kAllocation].pattern->label) + " at " + DescribeAddress(allocation) + " does not lie in the bone structure maker at " +
				DescribeAddress(maker) + ": " + (allocating ? "the game's unwind data puts it in the function at " + DescribeAddress(allocating) :
				std::string("the game's unwind data names no function there")) + ". Vehicles keep 20 sirens.");
			return false;
		}

		// 4. the pool not made yet: every structure must be made with the room, and one asked of a pool of the old stride traps. The
		// load before the allocation is read by its shape, since no pattern pins it; another shape leaves this check unmade, said.
		const uint8_t* const load = allocation - sizeof(kPoolLoad) - sizeof(int32_t);
		const uint8_t* const pool = IsReadable(load, sizeof(kPoolLoad) + sizeof(int32_t)) && memcmp(load, kPoolLoad, sizeof(kPoolLoad)) == 0 ?
			Rel32Target(load + sizeof(kPoolLoad)) : nullptr;
		const bool poolRead = pool && IsInsideGameImage(pool) && IsReadable(pool, sizeof(uint64_t));
		const uint64_t poolHolds = poolRead ? ValueAt<uint64_t>(pool) : 0;
		if (poolHolds)
		{
			SirensNotRaised("the bone structures' pool at " + DescribeAddress(pool) + " is made already, so TweaksV has started too late to give its structures their "
				"room" + At(*kSites[kAllocation].pattern) + ". Vehicles keep 20 sirens.");
			return false;
		}
		if (IsDiagnosticMode())
			WriteLog(kTag, poolRead ? "DIAGNOSTIC: the bone structures' pool at " + DescribeAddress(pool) + " is not made yet" + At(*kSites[kAllocation].pattern) :
				"DIAGNOSTIC: the bone structures' pool cannot be read: no load of it before the allocation at " + DescribeAddress(allocation) + At(*kSites[kAllocation].pattern));

		// 5. the game's fill: a function start by the game's unwind data, as the wrapper calls it
		const uint8_t* const filling = FunctionStartOf(fill);
		if (filling != fill)
		{
			SirensNotRaised("the bone structures' fill at " + DescribeAddress(fill) + " is not a function start (" + (filling ? "inside the function at " +
				DescribeAddress(filling) + " (unwind data)" : std::string("no unwind data")) + ")" + At(*kSites[kFill].pattern) + ". Vehicles keep 20 sirens.");
			return false;
		}
		g_fill = reinterpret_cast<FillBones>(const_cast<uint8_t*>(fill));
		if (IsDiagnosticMode())
		{
			WriteLog(kTag, "DIAGNOSTIC: bone structure maker's call at " + DescribeAddress(makerCall) + " -> " + DescribeAddress(maker) + ", its allocation at " +
				DescribeAddress(allocation) + ", the game's fill at " + DescribeAddress(fill) + ", each by the unwind data (I-E-Y3, I-E-Y2, I-E-Y4)");
			WriteLog(kTag, "DIAGNOSTIC: siren hit loop's bone read at " + DescribeAddress(found[kHitBone][0] + kHitBoneRead) + " " +
				Holds(found[kHitBone][0] + kHitBoneRead) + At(*kSites[kHitBone].pattern) + ", left as it is");
		}

		// 6. the writes, each named by its pattern, added to the part's one write
		for (const Operand& operand : kOperands)
		{
			const std::vector<uint8_t*>& matches = found[operand.site];
			for (size_t m = 0; m < matches.size(); m++)
				list.writes.push_back({ matches[m] + operand.at, OperandBytes(operand.raised + operand.perSiren * sirens, operand.size),
					std::string(operand.what) + (matches.size() > 1 ? " " + std::to_string(m + 1) + " of " + std::to_string(matches.size()) : std::string()) +
					At(*kSites[operand.site].pattern) });
		}
		list.redirects.push_back({ makerCall, reinterpret_cast<const void*>(&MadeBones), "bone structure maker's call", reinterpret_cast<void**>(&g_maker) });
		return true;
	}
}
