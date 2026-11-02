// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the siren setting IDs, the SirenSettings part's first job (see TweaksVSirenIds.h).
//
// A siren setting of carcols.meta - the lights and the pattern of an emergency vehicle's sirens - carries an ID, and a
// vehicle's variation in carvariations.meta names its siren setting by that ID. On the whole load path the ID is one
// byte: the parser keeps a larger number's low byte, and a table of 256 bytes maps an ID to the setting's position in
// the game's array, a byte too, in which index 0 means no sirens. So the game has 256 IDs and 255 settings a vehicle can
// use, and it binds silently: a pack's setting whose ID is in use is dropped, a clash within one file moves the
// later setting to the first free ID, and a setting placed past position 255 is reached as another one.
//   LARGE_SIREN_IDS  the part's job, IDs of 16 bits, 0 to 65535, 255 staying "none". Every site is found and checked, then
//                    all of it goes in one write: the ID table moved on the relocation core to 65,536 entries of a byte
//                    each - a fixed size, every 16-bit ID having its entry - with its four lea and the four limits of the
//                    free-ID searches; the nine byte loads and the two byte stores of an ID made word ones; a pack's
//                    dedupe's compare loop rewritten as a word compare, one byte longer, inside the NOP before it; the
//                    defaults of the three constructors, the high byte of an ID no file gives; the types of the two
//                    parser members that carry an ID, a byte's made a word's, in the game's data - the one write TweaksV
//                    makes outside code and its own memory; and the vehicle metadata's call of the base loader redirected,
//                    so that the moved table is filled with "free" whenever the game fills its own. The position stays
//                    a byte, so 255 settings stay the most.
//   DIAGNOSTIC_MODE  with the part: every site logged, the measurement over the table in use, 10 seconds in and then every
//                    minute while the numbers change - the siren settings loaded against the 255 a vehicle can use, the
//                    IDs in use against the table's entries, the settings past position 255, the IDs given to more than
//                    one setting and the IDs themselves in their settings' order; and the settings the game drops and
//                    moves, per file: a hook on a pack's carcols callback and one on the base builder, and a pack's
//                    dedupe's load of an ID through a stub, which keeps the IDs the pack's file gave, since the file
//                    is read inside the callback. With LARGE_SIREN_IDS=0 or the part off the measurement goes on, through
//                    the table of the parts measured first, over the game's own table and IDs of a byte, nothing written, so
//                    that the base game's settings and IDs can be compared with the part off and on: what the
//                    game's PSO loader makes of a member wider than its binary file's rests on that comparison.
//   DRY_RUN          every site found and checked, the writes named in one line, none made; the measurement over the game's
//                    own table and IDs of a byte.
// The sites: the parser registrations of the two members that carry an ID - the siren setting's member 0 at +0x00, the
// variation's member 6 at +0x4B - with their offsets, and their member data's offset field and type; the simple
// member's XML reader, which stores a byte or a word by that type; the four lea of the ID table, which a scan of the
// game's code must find exactly; in the loaders, the nine byte loads and the two byte stores of an ID, the four checks
// for "none" (0xFF), the two IDs given when none is free, the two IDs stored into a variation and the four free-ID searches'
// limits of 256; a pack's dedupe and its compare loop; the three constructors that give an ID its default; the vehicle
// metadata's init, its last store into the table and its call of the base loader; and the getter of a model's siren
// setting, for the measurement. What is checked and left: the four compares with "none", now on the 16-bit value; the
// two IDs given when none is free, since a search that ends at 0x10000 leaves si 0x00FF, "none", when sil is set; the
// dedupe's own check for none on the low byte, so that a pack's ID n*256+255 skips the dedupe and gets a free ID when
// its own is taken; the two IDs stored into a variation, which nothing reads afterwards.
// For the part, a site not as the findings have it is a problem and nothing is written; for the measurement alone, with
// the part off, it is logged and nothing is measured - an aid, not a problem (NOTES.md, T-DIAG). A type other than a byte's is
// another mod's work for the part, and for the measurement a line that stops nothing. The measurement reads the game's data
// without a lock and writes nothing: the carcols data's siren settings through CopyMemorySafely, since their array grows
// while packs load, and the ID table after IsReadable.
// Written for TweaksV from the findings of its own research in the game's code; the idea comes from SirenSetting Limit
// Adjuster by cp702 - none of its code was read or used (CREDITS.md).

#include "TweaksVSirenIds.h"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "TweaksVConflicts.h"
#include "TweaksVLog.h"
#include "TweaksVMeasure.h"
#include "TweaksVPatch.h"
#include "TweaksVPatterns.h"
#include "TweaksVRelocation.h"
#include "TweaksVScanner.h"
#include "TweaksVSettings.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "SirenSettings";

		// ---- the game's data (PATTERNS.md, "SirenSettings")

		constexpr size_t kIds = 256;				// the ID table: per ID the position of the setting that has it, a byte; kNone when free
		constexpr uint32_t kWideIds = 65536;		// and as the part moves it, for IDs of 16 bits: a fixed size, every ID having its entry
		constexpr uint8_t kNone = 0xFF;				// no ID given, and a free entry of the table
		constexpr size_t kPositions = 256;			// the positions an index reaches - it is a byte
		constexpr size_t kUsable = kPositions - 1;	// and the ones a vehicle can use: index 0 means no sirens
		constexpr size_t kSettingSize = 0x5E8;		// a siren setting, its ID the byte at +0x00
		constexpr size_t kSettingsAt = 0x48;		// in the carcols data: the siren settings' array, then their count and capacity
		constexpr size_t kTableLastFill = 0xF0;		// the vehicle metadata's init fills the table 16 bytes at a time, the last here
		constexpr size_t kMemberOffsetAt = 0x08;	// in a member's data: its offset, written when the parser registers its structure
		constexpr size_t kMemberTypeAt = 0x10;		// and its type: 0x01 or 0x02 store a byte, 0x03 or 0x04 a word
		constexpr uint8_t kTypeToWord = 2;			// a byte's type plus this: the word's of the same sign

		// ---- the patterns, in the order of PATTERNS.md

		enum Match : size_t { kP1, kP2, kP3, kT1, kT2, kT3, kT4, kD1, kD2, kC1, kC2, kC3, kR1, kM1, kMatchCount };
		constexpr const GamePattern* kLimitPatterns[kMatchCount] = { &Pattern("I-E-P1"), &Pattern("I-E-P2"), &Pattern("I-E-P3"),
			&Pattern("I-E-T1"), &Pattern("I-E-T2"), &Pattern("I-E-T3"), &Pattern("I-E-T4"), &Pattern("I-E-D1"), &Pattern("I-E-D2"),
			&Pattern("I-E-C1"), &Pattern("I-E-C2"), &Pattern("I-E-C3"), &Pattern("I-E-R1"), &Pattern("I-E-M1") };

		constexpr size_t kTableLoads = 4;		// the lea of the ID table, T1 to T4's: every one the game has
		constexpr char kLeaPrefixes[] = "\x48\x4C";	// the REX bytes they start with
		constexpr char kCmpRsi[] = "\x48\x81\xFE";		// cmp rsi, imm32: a free-ID search's limit
		constexpr int kFirstLimitAtT2 = 0x85, kSecondLimitAtT2 = 0xCD;
		constexpr int kFirstLimitAtT4 = 0x81, kSecondLimitAtT4 = 0xC9;

		// The part's sites on the relocation core: the table's lea in T1 to T4 and the limits of T2 and T4, the free-ID searches'
		// "cmp rsi, N"; the other patterns the part reads itself, by their place, which is their Match. The lea are the scan's to
		// find; these give the table's address and must agree with it.
		constexpr Anchor kAnchors[kMatchCount] = {
			Site(*kLimitPatterns[kP1]),
			Site(*kLimitPatterns[kP2]),
			Site(*kLimitPatterns[kP3]),
			Reference(*kLimitPatterns[kT1], Rel32(10, "\x4C\x8D\x05", 3)),
			ReferenceAndLimits(*kLimitPatterns[kT2], Rel32(23, "\x48\x8D\x1D", 3), Imm32(kFirstLimitAtT2, kCmpRsi, 3, kFirstLimitAtT2 + 3, 1, 0),
				Imm32(kSecondLimitAtT2, kCmpRsi, 3, kSecondLimitAtT2 + 3, 1, 0)),
			Reference(*kLimitPatterns[kT3], Rel32(10, "\x4C\x8D\x05", 3)),
			ReferenceAndLimits(*kLimitPatterns[kT4], Rel32(16, "\x48\x8D\x2D", 3), Imm32(kFirstLimitAtT4, kCmpRsi, 3, kFirstLimitAtT4 + 3, 1, 0),
				Imm32(kSecondLimitAtT4, kCmpRsi, 3, kSecondLimitAtT4 + 3, 1, 0)),
			Site(*kLimitPatterns[kD1]),
			Site(*kLimitPatterns[kD2]),
			Site(*kLimitPatterns[kC1]),
			Site(*kLimitPatterns[kC2]),
			Site(*kLimitPatterns[kC3]),
			Site(*kLimitPatterns[kR1]),
			Site(*kLimitPatterns[kM1]),
		};

		// A fixed size, no setting; 0xFF, a free ID, in the entries past the game's own
		constexpr RelocationSpec kSpec = { kTag, "siren setting ID table", "siren setting ID table size", "The siren setting IDs' limit is not raised",
			sizeof(uint8_t), kWideIds, nullptr, kAnchors, kTableLoads, kLeaPrefixes, 2, 0, kNone };

		// "0F B6 2C 0F", bytes as PATTERNS.md writes them: how many, and the one at an index
		constexpr size_t HexSize(std::string_view hex)
		{
			return (hex.size() + 1) / 3;
		}
		constexpr uint8_t HexByte(std::string_view hex, size_t index)
		{
			const auto digit = [](char c) { return c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10; };
			return static_cast<uint8_t>(digit(hex[3 * index]) * 16 + digit(hex[3 * index + 1]));
		}
		consteval bool IsHexText(std::string_view hex)
		{
			const auto isHex = [](char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); };
			if (hex.size() % 3 != 2)
				return false;
			for (size_t i = 0; i < hex.size(); i++)
				if (i % 3 == 2 ? hex[i] != ' ' : !isHex(hex[i]))
					return false;
			return true;
		}

		// The little-endian dword of four bytes of a text
		constexpr uint32_t HexDword(std::string_view hex, size_t index)
		{
			return HexByte(hex, index) | static_cast<uint32_t>(HexByte(hex, index + 1)) << 8 | static_cast<uint32_t>(HexByte(hex, index + 2)) << 16 |
				static_cast<uint32_t>(HexByte(hex, index + 3)) << 24;
		}

		// True if a match's pattern pins these bytes at this offset - for the checks, when TweaksV is compiled, that the
		// module reads a match where its pattern says what lies there
		consteval bool Pins(Match match, size_t at, std::string_view hex)
		{
			for (size_t i = 0; i < HexSize(hex); i++)
				if (PatternByte(*kLimitPatterns[match], at + i) != HexByte(hex, i))
					return false;
			return true;
		}

		// True if a match's pattern pins this one byte, a constant of the module, at this offset
		consteval bool PinsByte(Match match, size_t at, size_t value)
		{
			return PatternByte(*kLimitPatterns[match], at) == static_cast<int>(value);
		}

		constexpr size_t kNameHashAt = 6;		// P1, P2: the imm32 of "mov dword [rip+disp32], imm32" (C7 05), the structure's name hash
		constexpr size_t kMemberTableAt = 23;	// P1, P2: the rel32 of "lea rax, [rip+disp32]" (48 8D 05 at +20), the member data's pointers
		constexpr size_t kOffsetsAt = 37;		// P1, P2: the rel32 of the next "lea rax" (48 8D 05 at +34), past the match: the offsets
		static_assert(Pins(kP1, 0, "C7 05") && Pins(kP1, 20, "48 8D 05") && Pins(kP1, 34, "48 8D 05") && PatternByte(*kLimitPatterns[kP1], kOffsetsAt) == -1 &&
			Pins(kP2, 0, "C7 05") && Pins(kP2, 20, "48 8D 05") && Pins(kP2, 34, "48 8D 05") && PatternByte(*kLimitPatterns[kP2], kOffsetsAt) == -1,
			"TweaksV: I-E-P1 and I-E-P2 hold the name hash's store and the two lea of their tables, the second's rel32 past the match");
		constexpr size_t kReaderMemberAt = 3;	// P3: the member data in the member, the disp8 of "mov rax, [rcx+disp8]" (48 8B 41)
		constexpr size_t kReaderTypeAt = 7;		// P3: the type in the member data, the disp8 of "movzx ecx, byte [rax+disp8]" (0F B6 48)
		constexpr size_t kReaderTypesAt = 11;	// P3: the highest type, the imm8 of "cmp rcx, imm8" (48 83 F9), then "ja" (0F 87)
		static_assert(Pins(kP3, 0, "48 8B 41 08 0F B6 48 10 48 83 F9 21 0F 87") && PinsByte(kP3, kReaderTypeAt, kMemberTypeAt),
			"TweaksV: I-E-P3 holds the XML reader's reads of the member data and its type, at +0x10");
		constexpr size_t kTableLeaAt[kTableLoads] = { 10, 23, 10, 16 };	// in T1 to T4: the rel32 of each one's lea of the table
		static_assert(Pins(kT1, 7, "4C 8D 05") && Pins(kT2, 20, "48 8D 1D") && PatternByte(*kLimitPatterns[kT2], 23) == -1 && Pins(kT3, 7, "4C 8D 05") &&
			Pins(kT4, 13, "48 8D 2D") && PatternByte(*kLimitPatterns[kT4], 16) == -1,
			"TweaksV: I-E-T1 to I-E-T4 hold lea r8, lea rbx, lea r8 and lea rbp of the ID table, the second's and the fourth's rel32 past the match");
		static_assert(PinsByte(kT2, 4, kSettingsAt + 8) && PinsByte(kT2, 17, kSettingsAt),
			"TweaksV: I-E-T2 reads the siren settings' count at +0x50 and their array at +0x48 of the carcols data");
		constexpr size_t kCarcolsAtT4 = 3;		// T4: the disp32 of "mov r8, [rip+disp32]" (4C 8B 05), the carcols pointer
		constexpr size_t kCarcolsAtM1 = 15;		// M1: the disp32 of "mov rcx, [rip+disp32]" (48 8B 0D at +12), the same pointer
		static_assert(Pins(kT4, 0, "4C 8B 05") && Pins(kM1, 12, "48 8B 0D"), "TweaksV: I-E-T4 and I-E-M1 read the carcols pointer");
		constexpr size_t kLoopAt = 12;			// D2: the compare loop, "cmp [rbp+rsi], dl" (38 54 35 00)
		constexpr size_t kLoopJzAt = 18;		// D2: the rel32 of the loop's "jz rel32" (0F 84 at +16)
		constexpr size_t kLoopPaddingAt = 2;	// D2: the 10-byte NOP before the loop, where the part's starts
		constexpr size_t kLoopEnd = 34;			// D2: right after the loop's "jnz" back to it
		static_assert(Pins(kD2, 0, "31 F6 66 2E 0F 1F 84 00 00 00 00 00") && Pins(kD2, kLoopAt, "38 54 35 00 0F 84") &&
			Pins(kD2, kLoopJzAt + 4, "48 81 C6 E8 05 00 00 48 39 F3 75 EA") && PatternByte(*kLimitPatterns[kD2], kLoopEnd) == -1,
			"TweaksV: I-E-D2 holds the dedupe's compare loop, its jz and its jnz back, and before them a NOP of 10 bytes");
		constexpr size_t kDedupeLoadAt = 0x0B;	// D1: "movzx edx, byte [r13+r15]", the ID of a pack's setting
		constexpr size_t kDedupeLoadEnd = 0x11;	// D1: then only "movzx edi, word [rbx+0x50]" before "cmp dl, 0xFF": no flag is read
		constexpr std::string_view kDedupeLoadWritten = "43 0F B7 54 3D 00";
		static_assert(Pins(kD1, kDedupeLoadAt, "43 0F B6 54 3D 00") && Pins(kD1, kDedupeLoadEnd, "0F B7 7B 50 80 FA FF"),
			"TweaksV: I-E-D1 holds the load of a pack's ID, then a movzx and a compare with none, so that the flags before the compare are free");
		constexpr size_t kFillAt = 4;			// R1: the disp32 of "movdqa [rip+disp32], xmm0" (66 0F 7F 05), the table's last fill
		constexpr size_t kLoaderCallAt = 13;	// R1: "call rel32" (E8), the base loader
		static_assert(Pins(kR1, 0, "66 0F 7F 05") && Pins(kR1, kLoaderCallAt, "E8"), "TweaksV: I-E-R1 holds the table's last fill and the base loader's call");
		constexpr size_t kModelIndexAt = 3;		// M1: a model's siren setting index, the disp32 of "movzx eax, byte [rcx+disp32]" (0F B6 81)
		constexpr size_t kGetterSizeAt = 22;	// M1: a setting's size, the imm32 of "imul rax, rax, imm32" (48 69 C0)
		constexpr size_t kGetterArrayAt = 29;	// M1: the settings' array, the disp8 of "add rax, [rcx+disp8]" (48 03 41)
		static_assert(Pins(kM1, 0, "0F B6 81") && Pins(kM1, 19, "48 69 C0") && Pins(kM1, kGetterSizeAt, "E8 05 00 00") &&
			HexDword("E8 05 00 00", 0) == kSettingSize && Pins(kM1, 26, "48 03 41") && PinsByte(kM1, kGetterArrayAt, kSettingsAt),
			"TweaksV: I-E-M1 holds a setting's size, 0x5E8, and the settings' array at +0x48");

		// ---- the sites the part changes, or relies on and leaves as they are: the game's bytes there

		enum class Kind { Bytes, Limit };	// Limit: a free-ID search's "cmp rsi, imm32", the imm32 the table's entries

		struct ByteSite
		{
			Match match;
			int offset;					// from the match's start; negative before it
			std::string_view bytes;		// the game's bytes there
			const char* what;			// for the log
			Kind kind = Kind::Bytes;
			std::string_view written;	// what the part writes there, as many bytes; empty: left as it is. A limit is the core's
		};

		constexpr ByteSite kSites[] = {
			// the base loader's loop over the variations (T1): the variation's ID and the setting's ID
			{ kT1, 0x93, "0F B6 2C 0F", "siren setting ID load", Kind::Bytes, "0F B7 2C 0F" },
			{ kT1, 0x97, "48 81 FD FF 00 00 00", "siren setting ID check for none" },
			{ kT1, 0xC4, "0F B6 1C 2B", "siren setting ID load", Kind::Bytes, "0F B7 1C 2B" },
			{ kT1, 0xC8, "88 1C 0F", "variation's siren setting ID store" },
			// the base builder's siren part (T2): a setting's ID, a new one stored, the two free-ID searches
			{ kT2, 0x29, "40 88 31", "siren setting ID store", Kind::Bytes, "66 89 31" },
			{ kT2, 0x30, "0F B6 2C 39", "siren setting ID load", Kind::Bytes, "0F B7 2C 39" },
			{ kT2, 0x4B, "0F B6 2C 39", "siren setting ID load", Kind::Bytes, "0F B7 2C 39" },
			{ kT2, 0x4F, "48 81 FD FF 00 00 00", "siren setting ID check for none" },
			{ kT2, 0x85, "48 81 FE 00 01 00 00", "free-ID search limit", Kind::Limit },
			{ kT2, 0xCD, "48 81 FE 00 01 00 00", "free-ID search limit", Kind::Limit },
			{ kT2, 0xD6, "40 B6 FF", "siren setting ID given when none is free" },
			// a pack's loop over its variations (T3), as the base one
			{ kT3, 0x8D, "0F B6 3C 03", "siren setting ID load", Kind::Bytes, "0F B7 3C 03" },
			{ kT3, 0x91, "48 81 FF FF 00 00 00", "siren setting ID check for none" },
			{ kT3, 0xBE, "0F B6 14 17", "siren setting ID load", Kind::Bytes, "0F B7 14 17" },
			{ kT3, 0xC2, "88 14 03", "variation's siren setting ID store" },
			// a pack's builder (T4), as the base one
			{ kT4, 0x21, "40 88 33", "siren setting ID store", Kind::Bytes, "66 89 33" },
			{ kT4, 0x28, "0F B6 0C 3B", "siren setting ID load", Kind::Bytes, "0F B7 0C 3B" },
			{ kT4, 0x42, "0F B6 34 3B", "siren setting ID load", Kind::Bytes, "0F B7 34 3B" },
			{ kT4, 0x46, "48 81 FE FF 00 00 00", "siren setting ID check for none" },
			{ kT4, 0x81, "48 81 FE 00 01 00 00", "free-ID search limit", Kind::Limit },
			{ kT4, 0xC9, "48 81 FE 00 01 00 00", "free-ID search limit", Kind::Limit },
			{ kT4, 0xD2, "40 B6 FF", "siren setting ID given when none is free" },
			// a pack's dedupe (D1): the pack's ID, and its check for none, on the low byte
			{ kD1, kDedupeLoadAt, "43 0F B6 54 3D 00", "pack dedupe's siren setting ID load", Kind::Bytes, kDedupeLoadWritten },
			{ kD1, 0x15, "80 FA FF", "pack dedupe's check for none" },
			// the siren setting's constructor (C1): the store the part replaces - +0x1C is written again before the
			// constructor returns - with the high byte of the ID's default, and the ID's default, which then leaves it 0x00FF
			{ kC1, 0x08, "C7 41 1C 00 00 00 00", "siren setting constructor's first store at +0x1C", Kind::Bytes, "66 C7 01 FF 00 66 90" },
			{ kC1, 0xA0, "C6 06 FF", "siren setting constructor's ID default" },
			{ kC1, 0xC9, "89 46 1C", "siren setting constructor's second store at +0x1C" },
			// the variation's constructors, in place (C2) and allocating (C3): the stores from +0x30 to the IDs' defaults, the
			// part's with shorter zero stores and +0x4C = 0, the high byte of the ID's default, in the same bytes
			{ kC2, -0x0F, "C7 41 30 00 00 00 00 48 C7 41 38 00 00 00 00 C7 41 40 00 00 00 00 C7 41 48 00 00 FF FF C3",
				"variation's in-place constructor's stores", Kind::Bytes,
				"83 61 30 00 48 C7 41 38 00 00 00 00 83 61 40 00 C7 41 48 00 00 FF FF C6 41 4C 00 C3 90 90" },
			{ kC3, -0x0F, "C7 40 30 00 00 00 00 48 C7 40 38 00 00 00 00 C7 40 40 00 00 00 00 C7 40 48 00 00 FF FF 48 83 C4 28 C3",
				"variation's allocating constructor's stores", Kind::Bytes,
				"83 60 30 00 48 C7 40 38 00 00 00 00 83 60 40 00 C7 40 48 00 00 FF FF C6 40 4C 00 48 83 C4 28 C3 90 90" },
		};

		// Every site's bytes well-formed and, where it shares bytes with its match, the pattern's; a limit is "cmp rsi, imm32"
		// with the table's entries, left to the core; what the part writes is as long as what lies there
		consteval bool SitesAreValid()
		{
			for (const ByteSite& site : kSites)
			{
				if (!IsHexText(site.bytes) || (!site.written.empty() && (!IsHexText(site.written) || site.written.size() != site.bytes.size() || site.kind == Kind::Limit)))
					return false;
				for (size_t i = 0; i < HexSize(site.bytes); i++)
				{
					const int at = site.offset + static_cast<int>(i);
					const int pinned = at < 0 ? -1 : PatternByte(*kLimitPatterns[site.match], static_cast<size_t>(at));
					if (pinned != -1 && pinned != HexByte(site.bytes, i))
						return false;
				}
				if (site.kind == Kind::Limit && (HexSize(site.bytes) != 7 || site.bytes.substr(0, 8) != "48 81 FE" || HexDword(site.bytes, 3) != kIds))
					return false;
			}
			return true;
		}
		static_assert(SitesAreValid(), "TweaksV: a site of the siren setting IDs is malformed or disagrees with its pattern");

		// The IDs' loads and stores the part makes word ones: what the plan and the suite count, 9 and 2
		consteval size_t WrittenIn(std::initializer_list<Match> matches)
		{
			size_t written = 0;
			for (const ByteSite& site : kSites)
				for (const Match match : matches)
					if (site.match == match && !site.written.empty())
						written++;
			return written;
		}
		constexpr size_t kIdCodeWrites = WrittenIn({ kT1, kT2, kT3, kT4, kD1 });
		static_assert(kIdCodeWrites == 11 && WrittenIn({ kC1, kC2, kC3 }) == 3, "TweaksV: the part makes 11 loads and stores of an ID word ones and rewrites 3 constructors");

		// ---- the checks

		uint8_t* g_matches[kMatchCount] = {};
		const uint8_t* g_table = nullptr;			// the ID table in use: the game's own until the part has moved it
		uint32_t g_tableIds = kIds;					// its entries: more than the game's own once the part has made IDs words
		const uint8_t* g_carcolsPointer = nullptr;	// the game's pointer to its carcols data
		bool g_forThePart = false;					// the checks are the part's, which writes after them, not the measurement's alone
		std::string g_why;							// why the IDs are not widened, or the limit not measured, for its line

		bool Fail(std::string why)
		{
			g_why = std::move(why);
			return false;
		}

		// A line of the checks: the measurement's alone come with DIAGNOSTIC_MODE only, and the part writes its own with it alone
		void Note(const std::string& line)
		{
			if (IsDiagnosticMode())
				WriteLog(kTag, "DIAGNOSTIC: " + line);
		}

		// size bytes at address inside the game and readable: what a site outside its match must be, and a table or a
		// member's data the game's code leads to - the scan read the matches alone
		bool Readable(const void* address, size_t size, std::string_view what, const char* label)
		{
			const auto first = static_cast<const uint8_t*>(address);
			if (!IsInsideGameImage(first) || !IsInsideGameImage(first + size - 1))
				return Fail("the " + std::string(what) + " is not inside the game: " + DescribeAddress(address) + " (" + label + ").");
			if (!IsReadable(address, size))
				return Fail("the " + std::string(what) + " at " + DescribeAddress(address) + " cannot be read (" + label + ").");
			return true;
		}

		template <typename T> T ReadAt(const uint8_t* at)
		{
			T value{};
			memcpy(&value, at, sizeof(value));
			return value;
		}

		// A match's sites from the table, each logged; the first one not as the game has it ends the checks
		bool CheckSitesOf(Match match)
		{
			const char* const label = kLimitPatterns[match]->label;
			for (const ByteSite& site : kSites)
			{
				if (site.match != match || (site.kind == Kind::Limit && g_forThePart))	// a limit is the core's then
					continue;
				const uint8_t* const at = g_matches[match] + site.offset;
				const size_t size = HexSize(site.bytes);
				if (!Readable(at, size, site.what, label))
					return false;
				for (size_t i = 0; i < size; i++)
					if (at[i] != HexByte(site.bytes, i))
						return Fail("the " + std::string(site.what) + " at " + DescribeAddress(at) + " holds " + HexBytes(at, size) + ", not " +
							std::string(site.bytes) + " (" + label + ").");
				if (site.kind == Kind::Limit)	// as the relocation core logs a limit
					Note("limit at " + DescribeAddress(at + 3) + " holds " + std::to_string(kIds) + " (N, " + label + ") -> game size " + std::to_string(kIds));
				else
					Note(std::string(site.what) + " at " + DescribeAddress(at) + ": " + HexBytes(at, size) + " (" + label + ")");
			}
			return true;
		}

		// A parser registration (P1, P2) and the member of its structure that carries a siren setting ID
		struct Registration
		{
			Match match;
			const char* structure;		// "vehicle variations'"
			const char* member;			// "variation's siren setting ID member"
			size_t id;					// the member's index
			uint32_t idOffset;			// and its offset in the structure
			size_t neighbour;			// the member that bounds the room the ID would widen into
			uint32_t neighbourOffset;
			size_t members;				// how many members the structure has, its table ending in a null pointer; 0: not checked
		};
		constexpr Registration kSirenSetting = { kP1, "siren settings'", "siren setting's ID member", 0, 0x00, 2, 0x04, 0 };
		constexpr Registration kVariation = { kP2, "vehicle variations'", "variation's siren setting ID member", 6, 0x4B, 5, 0x4A, 7 };
		static_assert(kP1 == 0 && kP2 == 1, "TweaksV: the two registrations' members are kept by their match");
		const uint8_t* g_memberData[2] = {};		// the two ID members' data, by their match: logged again at the first summary
		bool g_widened = false;						// the part has written, so the measurement reads IDs of a word

		// The ID member's offset field and its type as the game holds them now, logged. For the measurement alone a type other
		// than a byte's (0x01 or 0x02) is a finding, logged too, and stops nothing - the measurement does not depend on it, and
		// with the part off another mod may have widened it. The game may register its structures after TweaksV has started,
		// so the offset field may still be 0.
		void LogMemberData(const Registration& r, const uint8_t* data, const std::string& head)
		{
			const char* const label = kLimitPatterns[r.match]->label;
			const uint8_t type = data[kMemberTypeAt];
			Note(std::string(r.member) + head + "its data at " + DescribeAddress(data) + ", offset field " + Hex(ReadAt<uint64_t>(data + kMemberOffsetAt)) +
				", type " + Hex(type) + " (" + label + ")");
			if (type != 0x01 && type != 0x02 && !g_widened)
				Note("the " + std::string(r.member) + "'s type at " + DescribeAddress(data + kMemberTypeAt) + " is " + Hex(type) +
					", not 0x1 or 0x2, which store a byte - a finding for the part (" + label + ").");
		}

		// The tables the registration leads to, inside the game; the member and its neighbour at their offsets, and as many
		// members as the findings have; the member's data inside the game, read, never written
		bool CheckRegistration(const Registration& r)
		{
			const uint8_t* const match = g_matches[r.match];
			const char* const label = kLimitPatterns[r.match]->label;
			if (!Readable(match + kOffsetsAt, sizeof(int32_t), "lea of the " + std::string(r.structure) + " offsets table", label))
				return false;
			const uint8_t* const memberTable = Rel32Target(match + kMemberTableAt);
			const uint8_t* const offsetTable = Rel32Target(match + kOffsetsAt);
			Note(std::string(r.structure) + " parser registration at " + DescribeAddress(match) + ": name hash " + Hex(ReadAt<uint32_t>(match + kNameHashAt)) +
				", member data table at " + DescribeAddress(memberTable) + ", offsets table at " + DescribeAddress(offsetTable) + " (" + label + ")");
			const size_t pointers = std::max(r.id, r.members) + 1;	// the member's pointer, and the null after the last member
			if (!Readable(memberTable, pointers * sizeof(uint64_t), std::string(r.structure) + " member data table", label) ||
				!Readable(offsetTable, (std::max(r.id, r.neighbour) + 1) * sizeof(uint32_t), std::string(r.structure) + " offsets table", label))
				return false;
			const uint32_t idOffset = ReadAt<uint32_t>(offsetTable + r.id * sizeof(uint32_t));
			const uint32_t neighbourOffset = ReadAt<uint32_t>(offsetTable + r.neighbour * sizeof(uint32_t));
			if (idOffset != r.idOffset || neighbourOffset != r.neighbourOffset)
				return Fail("the " + std::string(r.structure) + " members " + std::to_string(r.id) + " and " + std::to_string(r.neighbour) + " are at +" +
					Hex(idOffset) + " and +" + Hex(neighbourOffset) + ", not +" + Hex(r.idOffset) + " and +" + Hex(r.neighbourOffset) + " (" + label + ").");
			std::string count;
			if (r.members)
			{
				size_t found = 0;
				while (found < pointers && ReadAt<const uint8_t*>(memberTable + found * sizeof(uint64_t)))
					found++;
				if (found != r.members)
					return Fail("the " + std::string(r.structure) + " parser registration has " + std::to_string(found) +
						(found == pointers ? " or more" : "") + " members, not " + std::to_string(r.members) + " (" + label + ").");
				count = " of " + std::to_string(r.members);
			}
			const uint8_t* const data = ReadAt<const uint8_t*>(memberTable + r.id * sizeof(uint64_t));
			if (!Readable(data, kMemberTypeAt + 1, std::string(r.member) + "'s data", label))
				return false;
			g_memberData[r.match] = data;
			LogMemberData(r, data, ": member " + std::to_string(r.id) + count + " at +" + Hex(idOffset) + ", member " + std::to_string(r.neighbour) +
				" at +" + Hex(neighbourOffset) + "; ");
			if (!g_forThePart)
				return true;
			// what the part writes over: a byte's type, and an offset field that is the member's, or not written yet
			const uint8_t type = data[kMemberTypeAt];
			const uint64_t offsetField = ReadAt<uint64_t>(data + kMemberOffsetAt);
			if (type != 0x01 && type != 0x02)
				return Fail("the " + std::string(r.member) + "'s type at " + DescribeAddress(data + kMemberTypeAt) + " is " + Hex(type) +
					", not 0x1 or 0x2, which store a byte (another mod may have widened it already) (" + label + ").");
			if (offsetField != 0 && offsetField != r.idOffset)
				return Fail("the " + std::string(r.member) + "'s offset field at " + DescribeAddress(data + kMemberOffsetAt) + " holds " + Hex(offsetField) +
					", not +" + Hex(r.idOffset) + " (" + label + ").");
			return true;
		}

		// The two ID members once more, at the first summary: what the game holds there once it has started
		void LogMembersAgain()
		{
			for (const Registration* r : { &kSirenSetting, &kVariation })
			{
				const uint8_t* const data = g_memberData[r->match];
				if (IsReadable(data, kMemberTypeAt + 1))
					LogMemberData(*r, data, ", at the first summary: ");
				else
					Note(std::string(r->member) + ", at the first summary: its data at " + DescribeAddress(data) + " cannot be read (" +
						kLimitPatterns[r->match]->label + ")");
			}
		}

		// The simple member's XML reader: where it finds a member's data and type - pinned by its pattern, logged
		bool CheckReader()
		{
			const uint8_t* const match = g_matches[kP3];
			Note("simple member's XML reader at " + DescribeAddress(match) + ": the member data at +" + Hex(match[kReaderMemberAt]) + " of the member, its type at +" +
				Hex(match[kReaderTypeAt]) + ", types up to " + Hex(match[kReaderTypesAt]) + " (" + kLimitPatterns[kP3]->label + ")");
			return true;
		}

		// T4's carcols pointer, inside the game - the getter's must be the same
		bool CheckCarcolsPointer()
		{
			const uint8_t* const match = g_matches[kT4];
			const uint8_t* const pointer = Rel32Target(match + kCarcolsAtT4);
			Note("carcols pointer load at " + DescribeAddress(match) + " -> " + DescribeAddress(pointer) + " (" + kLimitPatterns[kT4]->label + ")");
			if (!Readable(pointer, sizeof(void*), "carcols pointer", kLimitPatterns[kT4]->label))
				return false;
			g_carcolsPointer = pointer;
			return true;
		}

		// The ID table: T1 to T4's lea, each logged as the relocation core logs a reference, must load one table inside
		// the game, and a scan of the game's code must find those four and no more; each match's sites with it
		bool CheckTable()
		{
			for (size_t i = 0; i < kTableLoads; i++)
			{
				const Match match = static_cast<Match>(kT1 + i);
				const char* const label = kLimitPatterns[match]->label;
				if (match == kT4 && !CheckCarcolsPointer())
					return false;
				const uint8_t* const field = g_matches[match] + kTableLeaAt[i];
				if (!Readable(field, sizeof(int32_t), "lea of the siren setting ID table", label))
					return false;
				const uint8_t* const target = Rel32Target(field);
				Note("reference at " + DescribeAddress(field) + " -> " + DescribeAddress(target) + " (" + label + ")");
				if (!g_table)
				{
					if (!IsInsideGameImage(target) || !IsInsideGameImage(target + kIds - 1))
						return Fail("the siren setting ID table is not inside the game: " + DescribeAddress(target) + " (another mod may have moved it already).");
					g_table = target;
				}
				else if (target != g_table)
					return Fail("the references to the siren setting ID table do not all point to the game's own table: " + DescribeAddress(g_table) +
						" and " + DescribeAddress(target) + " (" + label + ").");
				if (!CheckSitesOf(match))
					return false;
			}
			Note("the siren setting ID table is at " + DescribeAddress(g_table) + " - " + std::to_string(kIds) + " entries of 1 byte");
			const std::vector<uint8_t*> loads = Scanner::FindRipRelativeLeas(g_table, kTableLoads + 1);
			for (const uint8_t* load : loads)
				Note("reference at " + DescribeAddress(load) + " -> " + DescribeAddress(g_table) + " (lea scan)");
			if (loads.size() != kTableLoads)
				return Fail("the game code loads the siren setting ID table in " + std::to_string(loads.size()) + (loads.size() > kTableLoads ? " or more" : "") +
					" places, not the expected " + std::to_string(kTableLoads) + " (another mod may have moved some of them, or the game has changed).");
			return true;
		}

		// A pack's dedupe: its compare loop, and where its jz leads - inside the game
		bool CheckDedupeLoop()
		{
			const uint8_t* const match = g_matches[kD2];
			const char* const label = kLimitPatterns[kD2]->label;
			const uint8_t* const target = Rel32Target(match + kLoopJzAt);
			Note("pack dedupe's compare loop at " + DescribeAddress(match + kLoopAt) + ", its jz -> " + DescribeAddress(target) + " (" + label + ")");
			if (!IsInsideGameImage(target))
				return Fail("the jz of the pack dedupe's compare loop at " + DescribeAddress(match + kLoopJzAt - 2) + " does not lead into the game (another "
					"mod may have changed it) (" + label + ").");
			return true;
		}

		// The vehicle metadata's init: its last store into the ID table, then the call of the base loader, into the game
		bool CheckLoaderCall()
		{
			const uint8_t* const match = g_matches[kR1];
			const char* const label = kLimitPatterns[kR1]->label;
			const uint8_t* const fill = Rel32Target(match + kFillAt);
			const uint8_t* const call = match + kLoaderCallAt;
			const uint8_t* const loader = CallTarget(call);
			Note("base loader call at " + DescribeAddress(call) + " -> " + DescribeAddress(loader) + ", after the siren setting ID table's last fill at " +
				DescribeAddress(match) + " -> " + DescribeAddress(fill) + " (" + label + ")");
			if (fill != g_table + kTableLastFill)
				return Fail("the store before the base loader call at " + DescribeAddress(match) + " fills " + DescribeAddress(fill) + ", not the siren "
					"setting ID table's last 16 bytes at " + DescribeAddress(g_table + kTableLastFill) + " (" + label + ").");
			if (!IsInsideGameImage(loader))
				return Fail("the base loader call at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already) (" +
					label + ").");
			return true;
		}

		// The getter of a model's siren setting - what the measurement reads, pinned by its pattern - and its carcols pointer,
		// which must be T4's
		bool CheckGetter()
		{
			const uint8_t* const match = g_matches[kM1];
			const char* const label = kLimitPatterns[kM1]->label;
			const uint8_t* const pointer = Rel32Target(match + kCarcolsAtM1);
			Note("siren settings getter at " + DescribeAddress(match) + ": a model's siren setting index at +" + Hex(ReadAt<uint32_t>(match + kModelIndexAt)) +
				", the settings at +" + Hex(match[kGetterArrayAt]) + " of the carcols data, " + Hex(ReadAt<uint32_t>(match + kGetterSizeAt)) +
				" bytes each; the carcols pointer at " + DescribeAddress(pointer) + " (" + label + ")");
			if (pointer != g_carcolsPointer)
				return Fail("the carcols pointer the siren settings getter reads, " + DescribeAddress(pointer) + ", is not the one a pack's builder reads, " +
					DescribeAddress(g_carcolsPointer) + " (" + label + ").");
			return true;
		}

		// ---- the measurement: the siren settings loaded and the IDs in use, read as they are

		enum class State : uint64_t { Counted, NotLoaded, Unreadable };

		// The carcols data's siren settings, as the game keeps them at kSettingsAt
		struct Settings
		{
			const uint8_t* array;
			uint16_t count;
			uint16_t capacity;
		};
		static_assert(offsetof(Settings, count) == 8 && offsetof(Settings, capacity) == 10, "TweaksV: the siren settings' count and capacity follow their array");
		constexpr size_t kSettingsSize = offsetof(Settings, capacity) + sizeof(uint16_t);

		constexpr size_t kListedIds = 1024;		// the IDs the summary names, in their settings' order - the first of a load are the base game's
		constexpr size_t kIdsPerLine = 128;		// at most 7 characters each: a line of the log holds 2 KB

		// What the last count found, for the summary - both run on the measurement's thread
		struct Counted
		{
			uint64_t settings;
			uint64_t inUse;			// the table's entries that are not free
			uint64_t pastByte;		// those of IDs past 255
			uint64_t givenTwice;	// the IDs that more than one setting carries
			uint16_t listed[kListedIds];
		};
		Counted g_counted = {};
		const uint8_t* g_unreadable = nullptr;	// what could not be read, for the summary

		std::atomic<uint64_t> g_dropped{ 0 }, g_moved{ 0 };	// the settings the game dropped and moved so far, from the hooks
		bool g_hooked = false;								// the hooks are on: the summary has those two numbers too

		// The settings, read as they are this moment - plausible, or false
		bool ReadSettings(const uint8_t* carcols, Settings& settings)
		{
			settings = {};
			return CopyMemorySafely(&settings, carcols + kSettingsAt, kSettingsSize) && settings.count <= settings.capacity &&
				(settings.count == 0 || settings.array);
		}

		// The numbers the summary says, in one value that changes when they do: the state on top, below it a hash of them
		uint64_t Numbers(State state)
		{
			constexpr uint64_t kPrime = 0x100000001B3;
			uint64_t hash = 0xCBF29CE484222325;
			if (state == State::Counted)
			{
				for (const uint64_t number : { g_counted.settings, g_counted.inUse, g_counted.pastByte, g_counted.givenTwice, g_dropped.load(), g_moved.load() })
					hash = (hash ^ number) * kPrime;
				for (size_t i = 0; i < std::min<uint64_t>(g_counted.settings, kListedIds); i++)
					hash = (hash ^ g_counted.listed[i]) * kPrime;
			}
			return static_cast<uint64_t>(state) << 56 | hash >> 8;
		}

		// The settings loaded, the IDs in use, those past 255 and the IDs given to more than one setting - or why they cannot be
		// counted. The game's pointer and table after IsReadable; the settings through CopyMemorySafely, their ID a byte
		// each or, once the part has written, a word, and read again afterwards: an array the game grew meanwhile is not
		// counted this time. Nothing is allocated: the IDs seen go into two bitmaps on the stack.
		uint64_t SettingsNumbers()
		{
			if (!IsReadable(g_carcolsPointer, sizeof(void*)))
			{
				g_unreadable = g_carcolsPointer;
				return Numbers(State::Unreadable);
			}
			const uint8_t* const carcols = *reinterpret_cast<const uint8_t* const volatile*>(g_carcolsPointer);
			if (!carcols)
				return Numbers(State::NotLoaded);
			Settings settings{}, after{};
			if (!ReadSettings(carcols, settings))
			{
				g_unreadable = carcols + kSettingsAt;
				return Numbers(State::Unreadable);
			}
			uint64_t seen[kWideIds / 64] = {}, twice[kWideIds / 64] = {};
			for (size_t i = 0; i < settings.count; i++)
			{
				uint16_t id = 0;
				if (!CopyMemorySafely(&id, reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(settings.array) + i * kSettingSize), g_widened ? sizeof(uint16_t) : sizeof(uint8_t)))
				{
					g_unreadable = settings.array;
					return Numbers(State::Unreadable);
				}
				if (i < kListedIds)
					g_counted.listed[i] = id;
				const uint64_t bit = uint64_t{ 1 } << (id % 64);
				if (seen[id / 64] & bit)
					twice[id / 64] |= bit;
				seen[id / 64] |= bit;
			}
			if (!ReadSettings(carcols, after) || after.array != settings.array || after.count != settings.count)
			{
				g_unreadable = settings.array;
				return Numbers(State::Unreadable);
			}
			if (!IsReadable(g_table, g_tableIds))
			{
				g_unreadable = g_table;
				return Numbers(State::Unreadable);
			}
			const auto table = reinterpret_cast<const volatile uint8_t*>(g_table);
			uint64_t inUse = 0, pastByte = 0;
			for (size_t id = 0; id < g_tableIds; id++)
				if (table[id] != kNone)
				{
					inUse++;
					if (id >= kIds)
						pastByte++;
				}
			uint64_t givenTwice = 0;
			for (const uint64_t word : twice)
				givenTwice += static_cast<uint64_t>(std::popcount(word));
			g_counted.settings = settings.count;
			g_counted.inUse = inUse;
			g_counted.pastByte = pastByte;
			g_counted.givenTwice = givenTwice;
			return Numbers(State::Counted);
		}

		bool g_firstSummary = true;	// the measurement's thread alone reads and writes it

		// The IDs in their settings' order, so that a start with the part and one without it can be compared
		void LogListedIds()
		{
			const size_t listed = static_cast<size_t>(std::min<uint64_t>(g_counted.settings, kListedIds));
			for (size_t first = 0; first < listed; first += kIdsPerLine)
			{
				const size_t end = std::min(first + kIdsPerLine, listed);
				std::string line = "siren setting IDs of settings " + std::to_string(first) + " to " + std::to_string(end - 1) + " of " + std::to_string(g_counted.settings) + ":";
				for (size_t i = first; i < end; i++)
					line += (i > first ? ", " : " ") + std::to_string(g_counted.listed[i]);
				Note(line);
			}
		}

		void LogSettings(uint64_t numbers)
		{
			if (g_firstSummary)
			{
				g_firstSummary = false;
				LogMembersAgain();
			}
			const auto state = static_cast<State>(numbers >> 56);
			if (state == State::NotLoaded)
				Note("no siren settings loaded yet - the game has not read carcols.ymt.");
			else if (state == State::Unreadable)
				Note("the siren settings at " + DescribeAddress(g_unreadable) + " could not be read this time (the game may have been changing them).");
			else
			{
				const uint64_t settings = g_counted.settings;
				const uint64_t past = settings > kPositions ? settings - kPositions : 0;
				Note("siren settings loaded: " + std::to_string(settings) + " (vehicles can use " + std::to_string(kUsable) + " at most); siren setting IDs in use: " +
					std::to_string(g_counted.inUse) + " of " + std::to_string(g_tableIds) + (g_widened ? ", " + std::to_string(g_counted.pastByte) + " of them past 255" : std::string()) +
					"; settings past position " + std::to_string(kPositions - 1) + ", which no vehicle can reach: " + std::to_string(past) + "; IDs given to more than one setting: " +
					std::to_string(g_counted.givenTwice) +
					(g_hooked ? "; settings the game dropped so far: " + std::to_string(g_dropped.load()) + ", moved: " + std::to_string(g_moved.load()) : std::string()) + ".");
				LogListedIds();
			}
		}

		// The first summary 10 seconds in, then one a minute while the numbers change.
		constexpr Measurement kMeasurement = { "siren settings summary", kFirstSummaryMs, &SettingsNumbers, &LogSettings };

		void AnnounceMeasurement()
		{
			Note("the siren settings, through the carcols pointer at " + DescribeAddress(g_carcolsPointer) + " (" + kLimitPatterns[kM1]->label +
				"), and the siren setting ID table at " + DescribeAddress(g_table) + ", " + std::to_string(g_tableIds) + " IDs (" + kLimitPatterns[kT1]->label + " to " +
				kLimitPatterns[kT4]->label + "): the settings loaded and the IDs in use are logged after 10 seconds, then every minute while the numbers change.");
			StartMeasurement<kMeasurement>();	// one that cannot start is logged
		}

		// The measurement alone: every pattern is looked for, so that the log names each one missing, and its line the first;
		// every site in the order of the patterns, and the first that is not as the findings have it ends the checks
		void Measure()
		{
			std::string notFound;
			for (size_t i = 0; i < kMatchCount; i++)
				g_matches[i] = Scanner::FindOnce(*kLimitPatterns[i], notFound);
			if (!notFound.empty())
			{
				Note("the siren settings are not measured: " + notFound + ".");
				return;
			}
			if (!CheckRegistration(kSirenSetting) || !CheckRegistration(kVariation) || !CheckReader() || !CheckTable() || !CheckSitesOf(kD1) ||
				!CheckDedupeLoop() || !CheckSitesOf(kC1) || !CheckSitesOf(kC2) || !CheckSitesOf(kC3) || !CheckLoaderCall() || !CheckGetter())
			{
				Note("the siren settings are not measured: " + g_why);
				return;
			}
			AnnounceMeasurement();
		}

		// ---- the settings the game drops and moves, per file (DIAGNOSTIC_MODE, with the part's writes)

		constexpr uint32_t kRecordedIds = 4096;	// the IDs of one pack's file kept; more are counted

		// What the stub keeps: the IDs of a pack's settings as the dedupe reads them, on the read/write memory behind it
		struct DedupeRecord
		{
			volatile uint32_t read;			// how many it has read since the callback's hook set this to 0
			uint16_t ids[kRecordedIds];
		};
		DedupeRecord* g_record = nullptr;

		// The dedupe's load of a pack's ID in a stub of its own: the load, a word's, then the ID kept, every register but rdx as it
		// was - the flags are free, by what I-E-D1 pins after the load - and on, back behind the load. The pack's file is read
		// inside the callback, so nothing else shows its IDs: the ones the game drops never reach its array.
		constexpr uint8_t kDedupeStub[] = {
			0x43, 0x0F, 0xB7, 0x54, 0x3D, 0x00,		// movzx edx, word [r13+r15]
			0x50,									// push rax
			0x51,									// push rcx
			0xB8, 0x01, 0x00, 0x00, 0x00,			// mov eax, 1
			0xF0, 0x0F, 0xC1, 0x05, 0, 0, 0, 0,		// lock xadd [rip+disp32], eax: this ID's place, the record's read
			0x3D, 0, 0, 0, 0,						// cmp eax, imm32: the IDs the record keeps
			0x73, 0x0B,								// jae +11: no room, only counted
			0x48, 0x8D, 0x0D, 0, 0, 0, 0,			// lea rcx, [rip+disp32]: the record's IDs
			0x66, 0x89, 0x14, 0x41,					// mov [rcx+rax*2], dx
			0x59,									// pop rcx
			0x58,									// pop rax
			0xE9, 0, 0, 0, 0 };						// jmp rel32: back behind the load
		constexpr size_t kStubReadDispAt = 17, kStubReadEnd = 21, kStubRoomAt = 22, kStubIdsDispAt = 31, kStubIdsEnd = 35, kStubBackAt = 42;
		static_assert(sizeof(kDedupeStub) == kStubBackAt + sizeof(int32_t), "TweaksV: the dedupe's stub ends with its rel32 back");
		static_assert(kDedupeStub[0] == HexByte(kDedupeLoadWritten, 0) && kDedupeStub[2] == HexByte(kDedupeLoadWritten, 2) && HexSize(kDedupeLoadWritten) == 6,
			"TweaksV: the dedupe's stub starts with the load the part writes");
		constexpr size_t kJumpSize = 5;			// "jmp rel32", then a NOP in the load's sixth byte

		void NotCounted(const std::string& why)
		{
			Note("the siren settings dropped and moved are not counted: " + why);
		}

		// The stub on the first page of a block near the game, read+execute once written, the record behind it. Null, logged, if not.
		uint8_t* MakeDedupeStub()
		{
			const size_t page = PageSize();
			uint8_t* const block = static_cast<uint8_t*>(AllocateNearGame(page + sizeof(DedupeRecord)));
			if (!block)
			{
				NotCounted("no memory near the game could be had for the stub of a pack's dedupe.");
				return nullptr;
			}
			uint8_t* const record = block + page;
			const uint8_t* const back = g_matches[kD1] + kDedupeLoadEnd;
			const intptr_t backDistance = back - (block + sizeof(kDedupeStub));
			const intptr_t toStub = block - (g_matches[kD1] + kDedupeLoadAt + kJumpSize);
			if (backDistance < INT32_MIN || backDistance > INT32_MAX || toStub < INT32_MIN || toStub > INT32_MAX)
			{
				VirtualFree(block, 0, MEM_RELEASE);
				NotCounted("the stub of a pack's dedupe is out of reach of the game code.");
				return nullptr;
			}
			memcpy(block, kDedupeStub, sizeof(kDedupeStub));
			const auto put = [block](size_t at, int64_t value)
			{
				const int32_t field = static_cast<int32_t>(value);
				memcpy(block + at, &field, sizeof(field));
			};
			put(kStubReadDispAt, record + offsetof(DedupeRecord, read) - (block + kStubReadEnd));
			put(kStubRoomAt, kRecordedIds);
			put(kStubIdsDispAt, record + offsetof(DedupeRecord, ids) - (block + kStubIdsEnd));
			put(kStubBackAt, backDistance);
			DWORD old = 0;
			if (!VirtualProtect(block, page, PAGE_EXECUTE_READ, &old))
			{
				const DWORD error = GetLastError();
				VirtualFree(block, 0, MEM_RELEASE);
				NotCounted("the stub of a pack's dedupe could not be made read+execute (" + DescribeError(error) + ").");
				return nullptr;
			}
			FlushInstructionCache(GetCurrentProcess(), block, page);
			g_record = reinterpret_cast<DedupeRecord*>(record);
			return block;
		}

		// A list of up to this many in a line, then "..."
		constexpr size_t kListedPerLine = 16;

		void AddToList(std::string& list, size_t& listed, const std::string& item)
		{
			if (listed < kListedPerLine)
				list += (listed ? ", " : "") + item;
			else if (listed == kListedPerLine)
				list += ", ...";
			listed++;
		}

		// The IDs of the settings now, words: false if they cannot be read this moment
		bool ReadIds(const uint8_t* carcols, std::vector<uint16_t>& ids)
		{
			ids.clear();
			Settings settings{};
			if (!carcols || !ReadSettings(carcols, settings))
				return false;
			ids.resize(settings.count);
			for (size_t i = 0; i < settings.count; i++)
				if (!CopyMemorySafely(&ids[i], reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(settings.array) + i * kSettingSize), sizeof(uint16_t)))
					return false;
			return true;
		}

		const uint8_t* LoadedCarcols()
		{
			return IsReadable(g_carcolsPointer, sizeof(void*)) ? *reinterpret_cast<const uint8_t* const volatile*>(g_carcolsPointer) : nullptr;
		}

		// A pack's file's name for its line: its data file entry holds it at +0, read bounded and safely, as it comes from the
		// game; printable ASCII, anything else a '?'
		std::string FileName(const void* entry)
		{
			char name[128] = {};
			if (!entry || !CopyMemorySafely(name, entry, sizeof(name) - 1))
				return "(a name that cannot be read)";
			if (name[0] == '\0')
				return "(no name)";
			for (char& c : name)
				if (c != '\0' && (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) > 0x7E))
					c = '?';
			return name;
		}

		std::atomic<uint64_t> g_fileLines{ 0 };		// the lines per file written

		// Counts a line per file; true while the kind still gets lines of its own
		bool LineLeft()
		{
			const uint64_t n = g_fileLines.fetch_add(1, std::memory_order_relaxed) + 1;
			if (n == kMaxLinesPerKind + 1)
				Note(std::to_string(kMaxLinesPerKind) + " lines of carcols files written - from now on they are only counted; the summary every minute has the numbers.");
			return n <= kMaxLinesPerKind;
		}

		// What a pack's file did, by the dedupe's rule over what its stub read: a setting whose ID's low byte is not 0xFF and whose
		// ID is loaded already, those its own file appended before it among them, is dropped; the next that is not comes
		// next in the array, and an ID other than its own there has been moved.
		void LogPack(const std::string& name, const std::vector<uint16_t>& before, const std::vector<uint16_t>& after, uint32_t read)
		{
			const size_t added = after.size() > before.size() ? after.size() - before.size() : 0;
			const size_t dropped = read > added ? read - added : 0;
			std::vector<uint64_t> loaded(kWideIds / 64);
			const auto isLoaded = [&loaded](uint16_t id) { return (loaded[id / 64] >> (id % 64) & 1) != 0; };
			const auto setLoaded = [&loaded](uint16_t id) { loaded[id / 64] |= uint64_t{ 1 } << (id % 64); };
			for (const uint16_t id : before)
				setLoaded(id);
			std::string taken, movedList;
			size_t listedTaken = 0, moved = 0, next = before.size();
			for (uint32_t i = 0; i < std::min(read, kRecordedIds); i++)
			{
				const uint16_t id = g_record->ids[i];
				if ((id & 0xFF) != kNone && isLoaded(id))
				{
					AddToList(taken, listedTaken, std::to_string(id));
					continue;
				}
				setLoaded(id);
				if (next >= after.size())
					continue;
				const uint16_t now = after[next++];
				if (id != kNone && now != id)
					AddToList(movedList, moved, std::to_string(id) + " to " + std::to_string(now));
			}
			g_dropped.fetch_add(dropped, std::memory_order_relaxed);
			g_moved.fetch_add(moved, std::memory_order_relaxed);
			if (!LineLeft())
				return;
			Note("a pack's carcols, " + name + ", brought " + std::to_string(read) + " siren settings: " + std::to_string(added) + " added, " + std::to_string(dropped) +
				" dropped" + (taken.empty() ? std::string() : ", their IDs taken (" + taken + ")") + ", " + std::to_string(moved) + " moved" +
				(movedList.empty() ? std::string() : " (" + movedList + ")") + ".");
		}

		// The base builder's work on the settings of the base file, whose IDs it has before it: an ID that changed has moved, and
		// none has been given one
		void LogBase(const std::vector<uint16_t>& before, const std::vector<uint16_t>& after)
		{
			std::string movedList;
			size_t moved = 0, given = 0;
			for (size_t i = 0; i < std::min(before.size(), after.size()); i++)
			{
				if (before[i] == after[i])
					continue;
				if (before[i] == kNone)
					given++;
				else
					AddToList(movedList, moved, std::to_string(before[i]) + " to " + std::to_string(after[i]));
			}
			g_moved.fetch_add(moved, std::memory_order_relaxed);
			if (LineLeft())
				Note("the base carcols brought " + std::to_string(after.size()) + " siren settings: " + std::to_string(moved) + " moved" +
					(movedList.empty() ? std::string() : " (" + movedList + ")") + ", " + std::to_string(given) + " without an ID given one.");
		}

		// The game's two functions behind the hooks, with whatever arguments they have: the registers passed on, the result back
		using GameFunction = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t);
		GameFunction g_packCarcols = nullptr, g_baseBuilder = nullptr;

		// On the thread that loads the game's files: nothing throws out of them, and the game's last error stands.
		uint64_t HookedPackCarcols(uint64_t self, uint64_t entry, uint64_t third, uint64_t fourth)
		{
			std::vector<uint16_t> before;
			bool ready = false;
			try
			{
				g_record->read = 0;
				ready = ReadIds(LoadedCarcols(), before);
			}
			catch (...)
			{
			}
			const uint64_t result = g_packCarcols(self, entry, third, fourth);
			const DWORD lastError = GetLastError();
			try
			{
				std::vector<uint16_t> after;
				if (ready && ReadIds(LoadedCarcols(), after))
					LogPack(FileName(reinterpret_cast<const void*>(entry)), before, after, g_record->read);
			}
			catch (...)
			{
			}
			SetLastError(lastError);
			return result;
		}

		uint64_t HookedBaseBuilder(uint64_t carcols, uint64_t second, uint64_t third, uint64_t fourth)
		{
			std::vector<uint16_t> before;
			bool ready = false;
			try
			{
				ready = ReadIds(reinterpret_cast<const uint8_t*>(carcols), before);
			}
			catch (...)
			{
			}
			const uint64_t result = g_baseBuilder(carcols, second, third, fourth);
			const DWORD lastError = GetLastError();
			try
			{
				std::vector<uint16_t> after;
				if (ready && ReadIds(reinterpret_cast<const uint8_t*>(carcols), after))
					LogBase(before, after);
			}
			catch (...)
			{
			}
			SetLastError(lastError);
			return result;
		}

		// The two functions' starts, found and checked: null, logged, if they cannot be hooked
		constexpr const GamePattern& kPackPattern = Pattern("I-E-H1");
		constexpr const GamePattern& kBuilderPattern = Pattern("I-E-H2");

		struct DropHooks
		{
			uint8_t* pack = nullptr;
			uint8_t* builder = nullptr;
		};

		bool CheckFunctionStart(const uint8_t* start, const char* what, const char* label)
		{
			std::string detail;
			const bool isStart = IsFunctionStart(start, detail);
			Note(std::string(what) + " at " + DescribeAddress(start) + ", " + detail + " (" + label + ")");
			if (!isStart)
				NotCounted(std::string(label) + " at " + DescribeAddress(start) + " is not where a function starts: " + detail + ".");
			return isStart;
		}

		DropHooks FindDropHooks()
		{
			std::string notFound;
			DropHooks hooks;
			hooks.pack = Scanner::FindOnce(kPackPattern, notFound);
			hooks.builder = Scanner::FindOnce(kBuilderPattern, notFound);
			if (!notFound.empty())
			{
				NotCounted(notFound + ".");
				return {};
			}
			if (!CheckFunctionStart(hooks.pack, "a pack's carcols callback", kPackPattern.label) ||
				!CheckFunctionStart(hooks.builder, "the base builder", kBuilderPattern.label))
				return {};
			return hooks;
		}

		// ---- the base loader's call: the moved table filled whenever the game fills its own

		const uint8_t* g_tableField = nullptr;		// the rel32 of a lea of the table: what the game's code loads
		GameFunction g_baseLoader = nullptr;		// the game's function behind the call

		// The vehicle metadata's init calls the base loader right after its last store into its own table; it hands it no
		// argument the bytes before the call show, and the registers go on all the same. Nothing here can throw, and the game's
		// function comes last.
		uint64_t FillTableThenLoad(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth)
		{
			memset(const_cast<uint8_t*>(Rel32Target(g_tableField)), kNone, kWideIds);
			return g_baseLoader(first, second, third, fourth);
		}

		// ---- the part's writes

		std::vector<uint8_t> BytesOfText(std::string_view hex)
		{
			std::vector<uint8_t> bytes(HexSize(hex));
			for (size_t i = 0; i < bytes.size(); i++)
				bytes[i] = HexByte(hex, i);
			return bytes;
		}

		// The dedupe's compare loop as a word compare, one byte longer, from the NOP before it to where it ended: its jz's target
		// the same, its jnz back to its new start, and a NOP in what is left
		std::vector<uint8_t> WrittenLoop()
		{
			const uint8_t* const match = g_matches[kD2];
			std::vector<uint8_t> loop = BytesOfText("66 39 54 35 00 0F 84 00 00 00 00 48 81 C6 E8 05 00 00 48 39 F3 75 E9 66 0F 1F 84 00 00 00 00 00");
			constexpr size_t kJzRelAt = 7;
			const int32_t rel = static_cast<int32_t>(Rel32Target(match + kLoopJzAt) - (match + kLoopPaddingAt + kJzRelAt + sizeof(int32_t)));
			memcpy(loop.data() + kJzRelAt, &rel, sizeof(rel));
			return loop;
		}

		// Every write of the part's own, the core's aside: the table's rows, the dedupe's load through its stub if it has one, its
		// loop, and the two members' types
		std::vector<CodeWrite> PartWrites(const uint8_t* dedupeStub)
		{
			std::vector<CodeWrite> writes;
			for (const ByteSite& site : kSites)
				if (!site.written.empty())
					writes.push_back({ g_matches[site.match] + site.offset, BytesOfText(site.written), site.what });
			if (dedupeStub)
				for (CodeWrite& write : writes)
					if (write.address == g_matches[kD1] + kDedupeLoadAt)
					{
						const int32_t rel = static_cast<int32_t>(dedupeStub - (write.address + kJumpSize));
						write.bytes = { 0xE9, 0, 0, 0, 0, 0x90 };
						memcpy(write.bytes.data() + 1, &rel, sizeof(rel));
						write.what += " through a stub";
					}
			writes.push_back({ g_matches[kD2] + kLoopPaddingAt, WrittenLoop(), "pack dedupe's compare loop" });
			for (const Registration* r : { &kSirenSetting, &kVariation })
			{
				uint8_t* const type = const_cast<uint8_t*>(g_memberData[r->match]) + kMemberTypeAt;
				writes.push_back({ type, { static_cast<uint8_t>(*type + kTypeToWord) }, std::string(r->member) + "'s type" });
			}
			return writes;
		}

		// With DIAGNOSTIC_MODE the settings dropped and moved: the two hooks, an aid - a part whose hooks cannot be made still does its job
		void HookDrops(const DropHooks& hooks)
		{
			if (!hooks.pack)
				return;
			const std::vector<FunctionHook> functions = {
				{ hooks.pack, reinterpret_cast<const void*>(&HookedPackCarcols), reinterpret_cast<void**>(&g_packCarcols), "A pack's carcols callback" },
				{ hooks.builder, reinterpret_cast<const void*>(&HookedBaseBuilder), reinterpret_cast<void**>(&g_baseBuilder), "The base builder" } };
			if (!HookFunctions(functions, kTag))
			{
				NotCounted("the two functions could not be hooked (details above).");
				return;
			}
			g_hooked = true;
			Note("a pack's carcols callback at " + DescribeAddress(hooks.pack) + " (" + kPackPattern.label + ") and the base builder at " + DescribeAddress(hooks.builder) + " (" +
				kBuilderPattern.label + ") are hooked, and a pack's dedupe load goes through a stub: the settings dropped and moved are logged per file.");
		}
	}

	void MeasureSirenIdsWhileOff()
	{
		if (!IsFeatureOn(kSirenSettingsSection) || !IsOn(kLargeSirenIds))
			Measure();
	}

	void InitSirenIds()
	{
		if (!IsOn(kLargeSirenIds))
		{
			WriteLog(kTag, "LARGE_SIREN_IDS=0 - the siren setting IDs' limit is not raised.");
			return;
		}

		// 1. the table, its loads and its limits on the core, every pattern with them
		RelocationPlan plan;
		if (!FindRelocation(kSpec, plan))
			return;
		for (size_t i = 0; i < kMatchCount; i++)
			g_matches[i] = plan.matches[i][0];
		g_table = plan.array;
		g_forThePart = true;

		// 2. every other site, in the order of the patterns; the first that is not as the findings have it leaves the game as it is
		if (!CheckRegistration(kSirenSetting) || !CheckRegistration(kVariation) || !CheckReader() || !CheckCarcolsPointer() || !CheckSitesOf(kT1) ||
			!CheckSitesOf(kT2) || !CheckSitesOf(kT3) || !CheckSitesOf(kT4) || !CheckSitesOf(kD1) || !CheckDedupeLoop() || !CheckSitesOf(kC1) || !CheckSitesOf(kC2) ||
			!CheckSitesOf(kC3) || !CheckLoaderCall() || !CheckGetter())
		{
			RelocationFailed(kSpec, g_why);
			return;
		}

		// 3. with DIAGNOSTIC_MODE, the two hooks' sites and the dedupe's stub: an aid, nothing of it stops the part
		const bool diagnostic = IsDiagnosticMode();
		const bool dryRun = IsDryRun();
		DropHooks hooks;
		uint8_t* dedupeStub = nullptr;
		if (diagnostic)
		{
			hooks = FindDropHooks();
			if (hooks.pack && !dryRun)
			{
				dedupeStub = MakeDedupeStub();
				if (!dedupeStub)
					hooks = {};
			}
		}

		// 4. one write, all or nothing: the table's 4 references and 4 limits, the 11 loads and stores, the loop, the 3 constructors,
		// the 2 members' types and the base loader's call; DRY_RUN names them in one line, as the core's other parts do
		uint8_t* const loaderCall = g_matches[kR1] + kLoaderCallAt;
		const std::vector<CodeWrite> writes = PartWrites(dedupeStub);
		const std::vector<CallRedirect> redirects = { { loaderCall, reinterpret_cast<const void*>(&FillTableThenLoad), "base loader call",
			reinterpret_cast<void**>(&g_baseLoader) } };
		g_tableField = plan.references[0];
		if (dryRun && hooks.pack)
			WriteLog(kTag, "DRY_RUN: would hook a pack's carcols callback at " + DescribeAddress(hooks.pack) + " and the base builder at " + DescribeAddress(hooks.builder) +
				" and take a pack's dedupe load through a stub, to count the settings dropped and moved.");
		if (!ApplyRelocation(kSpec, plan, writes, redirects, " and widen the IDs' " + std::to_string(kIdCodeWrites) + " loads and stores, a pack's dedupe's loop, the 3 constructors' "
			"defaults and the 2 parser members' types, and redirect the base loader call at " + DescribeAddress(loaderCall)))
		{
			if (dedupeStub)
				VirtualFree(dedupeStub, 0, MEM_RELEASE);	// nothing leads to it: nothing was written
			return;
		}
		if (!dryRun)
		{
			g_table = Rel32Target(g_tableField);
			g_tableIds = kWideIds;
			g_widened = true;
			WriteLog(kTag, "The siren setting IDs' limit is raised to 65535: their " + std::to_string(kIdCodeWrites) + " loads and stores, a pack's dedupe's loop, the 3 constructors' "
				"defaults and the 2 parser members' types are written, and the base loader call at " + DescribeAddress(loaderCall) +
				" is redirected, so that the moved table is filled whenever the game fills its own.");
		}
		if (!diagnostic)
			return;

		// 5. with DIAGNOSTIC_MODE, the hooks and the measurement over the table in use
		if (!dryRun)
			HookDrops(hooks);
		AnnounceMeasurement();
	}
}
