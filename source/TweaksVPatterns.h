// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - every byte pattern TweaksV looks for in GTA5_Enhanced.exe, defined once in kPatterns with
// the label PATTERNS.md gives it. After a game update this table is the one place in the code to
// change, and PATTERNS.md the one in the docs; the tests check that both list the same patterns. The
// code that uses a pattern names it with Pattern("label"), which is checked when TweaksV is compiled.
#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string_view>

namespace TweaksV
{
	struct GamePattern
	{
		const char* label;	// as in PATTERNS.md: H- Heap, M- Modkit, P- Packfile, B- BoxStreamer, D- Decals, R- RageOpenV, S- StraightToStoryMode, Y- the script programs, L- Blips, W- WeaponLimits, I- SirenSettings, A- PedProps, Q- Resources, T- RayTracing, C- StaticBounds, G- LightCapacity, O- Population, N- PedPool, U- RenderQueues
		const char* text;	// the bytes in hex, one space apart; ? matches any byte
	};

	// In the order of PATTERNS.md; what each one finds is described there.
	inline constexpr GamePattern kPatterns[] = {
		// HeapAdjuster: the heap size, then the budget of the allocator the game's pools draw on
		{ "H-E1", "BA ? ? ? ? 45 31 C0 41 B1 01 E8 ? ? ? ? 48 8D 0D" },
		{ "H-E2", "48 8D 0D ? ? ? ? 48 8D 15 ? ? ? ? 41 B8 00 00 01 00 41 B9 ? ? ? ? E8" },

		// ModkitLimit: the references to the modkit array, then its hard-coded limits
		{ "M-E-R1", "31 c9 48 8d 05 ? ? ? ? eb" },
		{ "M-E-R2", "89 c2 48 8d 0d ? ? ? ? eb" },
		{ "M-E-R3", "41 0f b7 c2 48 8d 0d ? ? ? ? 0f b7 04 41" },
		{ "M-E-R4", "57 48 83 ec ? 48 8d 0d ? ? ? ? 41 b8" },
		{ "M-E-R5", "48 8d 3d ? ? ? ? 0f b7 3c 47" },
		{ "M-E-R6", "77 ? 48 8d 15 ? ? ? ? 0f b7 14 42" },
		{ "M-E-L1", "0f b7 c0 66 ba ? ? 3d ? ? ? ? 77" },
		{ "M-E-L2", "48 83 c5 ? 48 83 c0 ? 48 83 c7" },
		{ "M-E-L3", "74 ? 48 83 c5 ? 48 83 c3 ? 48 83 c0 ? 48 3d" },
		{ "M-E-L4", "48 83 c7 ? 48 83 c2 ? 48 83 c5 ? 48 81 fd" },
		{ "M-E-L5", "48 83 c7 ? 48 83 c2 ? 48 83 c6 ? 48 81 fe" },
		{ "M-E-L6", "41 b8 ? ? ? ? b2 ? e8 ? ? ? ? 66 0f 76 c0" },
		{ "M-E-L7", "0f b7 41 ? 66 bf ? ? 48 3d" },
		{ "M-E-L8", "41 0f b7 ca 66 b8 ? ? 81 f9" },

		// PackfileLimit: the direct size (one pattern per range of game builds), the
		// indirect size, the streaming configuration call, then the references to the packfile array -
		// R31 again per range of game builds
		{ "P-E-D3", "48 83 C6 ? 48 81 F9" },
		{ "P-E-D2", "49 83 c3 ? 48 81 f9 ? ? ? ? 75 ? eb" },
		{ "P-E-D1", "48 81 f9 ? ? ? ? 74 ? 48 83 3c ca 00" },
		{ "P-E-I1", "3b 05 ? ? ? ? 0f 8d ? ? ? ? 89 c0" },
		{ "P-E-H1", "89 48 ? 48 8d 0d ? ? ? ? 48 89 08 48 89 c1 e8" },
		{ "P-E-R01", "48 8d 15 ? ? ? ? 48 8b 3c ca 48 8b 2f" },
		{ "P-E-R02", "48 8d 3d ? ? ? ? 48 8b 0c cf 48 85 c9" },
		{ "P-E-R03", "48 8d 3d ? ? ? ? 48 8b 0c 39 48 85 c9" },
		{ "P-E-R04", "89 c0 48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 74" },
		{ "P-E-R05", "89 c0 48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 0f 84" },
		{ "P-E-R06", "48 8d 0d ? ? ? ? 4a 8b 0c c9 48 85 c9 0f 84 ? ? ? ? 48 8b 01" },
		{ "P-E-R07", "48 8d 0d ? ? ? ? 48 8b 1c 08 48 8b 03" },
		{ "P-E-R08", "48 8d 0d ? ? ? ? 4a 8b 0c c1 48 85 c9" },
		{ "P-E-R09", "48 83 ec ? 48 89 ce 48 8d 05 ? ? ? ? 48 89 01 c6 81" },
		{ "P-E-R10", "48 c1 e8 ? 48 8d 0d ? ? ? ? 48 8b 0c c1 48 8b 01" },
		{ "P-E-R11", "48 8d 0d ? ? ? ? 48 83 3c c1 ? 40 0f 95 c6 eb" },
		{ "P-E-R12", "48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 ff 90 ? ? ? ? 48 8b 48" },
		{ "P-E-R13", "48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 4c 8d 44 24" },
		{ "P-E-R14", "48 8d 0d ? ? ? ? 4c 8b 24 08 49 8b 04 24 4c 8d 44 24" },
		{ "P-E-R15", "48 8d 0d ? ? ? ? 48 8b 0c 08 48 85 c9 75" },
		{ "P-E-R16", "48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 0f 84 ? ? ? ? 48 89 f5" },
		{ "P-E-R17", "48 83 ec ? 89 d7 48 89 ce 48 8d 05 ? ? ? ? 48 89 01 c6 81 c1" },
		{ "P-E-R18", "48 8d 0d ? ? ? ? 48 8b 3c 08 48 8b 07 48 89 f9" },
		{ "P-E-R19", "48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 ff 90 ? ? ? ? 48 8b 40" },
		{ "P-E-R20", "48 8d 0d ? ? ? ? 48 8b 0c d1 48 85 c9 75" },
		{ "P-E-R21", "48 8d 0d ? ? ? ? 4c 8b 24 c1 4d 85 e4 0f 84" },
		{ "P-E-R22", "48 8d 0d ? ? ? ? 48 8b 0c c1 48 8b 01 ff 90 ? ? ? ? 48 8b 38" },
		{ "P-E-R23", "48 89 2d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 48 c7 44 08" },
		{ "P-E-R24", "48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 44 00 00" },
		{ "P-E-R25", "48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 90" },
		{ "P-E-R26", "48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 66 66 2e 0f 1f 84 00 00 00 00 00" },
		{ "P-E-R27", "48 89 2d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 84 00 00 00 00 00" },
		{ "P-E-R28", "48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 66 66 66 2e 0f 1f 84 00 00 00 00 00" },
		{ "P-E-R29", "48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 40 00" },
		{ "P-E-R30", "48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 48 c7 44 08" },
		{ "P-E-R31a", "BF 08 00 00 00 BE 10 00 00 00 4C 8D 05" },
		{ "P-E-R31b", "41 ? ? ? ? ? ? ? ? ? ? 41 bb ? ? ? ? ? 8d" },
		{ "P-E-R32", "48 89 cf 31 f6 4c 8d 3d ? ? ? ? eb 10" },
		{ "P-E-R33", "75 0c cc 48 8d 0d ? ? ? ? 48 8b 0c d1" },

		// BoxStreamer: the four sites that hand the traversal list's capacity to the tree search; the three
		// that end in "4C 8D 2D" are followed by the lea that loads the list, whose other loads are found by
		// their address (TweaksVBoxStreamer.cpp)
		{ "B-E-L1", "48 8B 44 24 40 48 8B 48 08 C7 44 24 20 ? ? ? ? 48 8D 54 24 60 4C 8D 44 24 70 4D 89 F1" },
		{ "B-E-L2", "48 8B 81 08 01 00 00 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D" },
		{ "B-E-L3", "0F 29 44 24 40 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D" },
		{ "B-E-L4", "48 8B 01 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D" },

		// Decals: the parser's "full" check (the count's address and N) and its lea of the array (L1, L2);
		// FindById's head (three constant sites and a lea), PickVariant's head (two sites) and the texture
		// override's five sites (L3..L9); the decal manager's start-up loop, its start (with a lea) and its
		// end (L10, L11); the "add decal" check that seven decal-placing functions share (L12, which matches
		// 7 times) and the four that differ (L13..L16). The array's other loads are found by their address
		// (TweaksVDecals.cpp)
		{ "D-E-L1", "8B 35 ? ? ? ? 48 81 FE ? ? ? ? 77 ? 48 89 F9 FF 15" },
		{ "D-E-L2", "4C 8D 2D ? ? ? ? F3 44 0F 10 0D ? ? ? ? F3 44 0F 10 15" },
		{ "D-E-L3", "56 85 C9 74 ? C7 02 04 01 00 00 41 C7 00 00 00 00 00 44 8B 1D" },
		{ "D-E-L4", "B8 04 01 00 00 81 F9 03 01 00 00 77 ? 41 89 D0 8B 05" },
		{ "D-E-L5", "45 31 C9 41 BB 04 01 00 00 31 C0 45 31 D2 EB" },
		{ "D-E-L6", "41 BB 04 01 00 00 48 8D 3D ? ? ? ? 31 C0 45 31 D2" },
		{ "D-E-L7", "41 81 FB 04 01 00 00 89 C3 41 0F 44 D9" },
		{ "D-E-L8", "81 FD 04 01 00 00 89 D8 41 0F 44 C1" },
		{ "D-E-L9", "41 81 FB 04 01 00 00 41 0F 44 C1 48 8D 3D" },
		{ "D-E-L10", "B8 D8 07 00 00 48 8D 0D ? ? ? ? 0F 57 C0" },
		{ "D-E-L11", "48 05 20 08 00 00 48 3D 18 59 00 00 0F 85" },
		{ "D-E-L12", "E8 ? ? ? ? 3D 03 01 00 00 0F 8F" },
		{ "D-E-L13", "44 89 C3 41 81 F8 03 01 00 00 77" },
		{ "D-E-L14", "89 C6 81 FE 03 01 00 00 0F 87" },
		{ "D-E-L15", "E8 ? ? ? ? 3D 03 01 00 00 7F" },
		{ "D-E-L16", "89 C3 31 C0 81 FB 03 01 00 00 0F 8F" },

		// RageOpenV: the console, the RPF cache check, the 8 file functions and the 5 encryption calls (the
		// newmods patterns went with the newmods folder; PATTERNS.md keeps the removed rows)
		{ "R-E-C1", "ff 15 ? ? ? ? 48 85 c0 74 ? 48 89 c1 31 d2" },
		{ "R-E-K1", "e8 ? ? ? ? 48 8d 0d ? ? ? ? 48 8d 15 ? ? ? ? 48 8d 35" },
		{ "R-E-F1", "41 56 56 57 53 48 81 ec ? ? ? ? 4c 89 c7 66 c7 ? ? ? ? ? 4c 8d 74 24" },
		{ "R-E-F2", "66 c7 01 ? ? 29 d9 d1 e9 66 89 4c 24 ? 31 ff 4c 8d 44 ? ? 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0" },
		{ "R-E-F3", "66 c7 01 ? ? 29 d9 d1 e9 66 89 4c 24 ? 31 ff 4c 8d 44 24 ? 48 89 d9 31 d2 ff 15 ? ? ? ? 8b 4c 24" },
		{ "R-E-F4", "41 56 56 57 53 48 81 ec ? ? ? ? 66 c7 ? ? ? ? ? 4c 8d 74 24" },
		{ "R-E-F5", "41 57 41 56 56 57 53 48 81 ec 60 02 00 00 48 89 d3 48 89 cf" },
		{ "R-E-F6", "66 c7 01 00 00 29 d9 d1 e9 66 89 4c 24 70 4c 8d 44 24 48 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0 74 14 8b 44 24 64 8b 4c 24 68 48 c1 e0 20 48 09 c1 48 89 4c 24 38" },
		{ "R-E-F7", "66 c7 01 00 00 29 d9 d1 e9 66 89 4c 24 70 4c 8d 44 24 48 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0 74 0a 48 8b 44 24 5c 48 89 44 24 38" },
		{ "R-E-F8", "56 57 53 48 81 ec 60 02 00 00 48 89 d7 48 89 ce c7 44 24 3c ff ff ff ff 48 89 d1 ba 3a 00 00 00" },
		{ "R-E-E1", "e8 ? ? ? ? 80 7c 24 2e ? 74 ? 48 8b 56" },
		{ "R-E-E2", "e8 ? ? ? ? 8b 46 ? 85 c0 74 ? b9" },
		{ "R-E-E3", "e8 ? ? ? ? 48 8b 46 ? 66 c7 00 00 ? eb" },
		{ "R-E-E4", "e8 ? ? ? ? 48 8b 46 ? 8b 4c 24 ? 48 8d 14 08" },
		{ "R-E-E5", "c6 86 ? ? ? ? ? 48 89 f1 4c 89 fa 45 89 f0 45 31 c9" },

		// StraightToStoryMode: the splash screen, the legal screen, the landing page
		{ "S-E1", "0F 85 A9 00 00 00 48 8D 54" },
		{ "S-E2", "00 E9 ? 05 00 00 E8 ? 07" },
		{ "S-E3", "E8 ? ? ? ? 8B 0D ? ? ? ? 83 C1 ? 84 C0" },

		// The script programs (TweaksVScripts.cpp): the call in a program's resource constructor that resolves
		// its natives, where the module's hook is (H1); the constructor's loop over the native table, which
		// reads the code length's and the native table's offsets (N1); the script interpreter's prologue,
		// which reads the code page table's (I1)
		{ "Y-E-H1", "4C 89 E1 E8 ? ? ? ? 41 BE FF 3F 00 00 41 8B 44 24 70" },
		{ "Y-E-N1", "41 8B 4C 24 1C 01 C1 49 8B 54 24 40 48 D3 04 C2" },
		{ "Y-E-I1", "4D 8B 50 10 49 63 41 1C 4C 8D 7C C1 F8 49 63 41 18 4C 8D 0C C1" },

		// Blips (TweaksVBlips.cpp), for its measurement with DIAGNOSTIC_MODE only: the blip pool's allocator, whose
		// lea loads the pool (P1), and its update of the pool's high-water mark, whose cmp reads the mark (P2)
		{ "L-E-P1", "BA 04 00 00 00 31 C0 48 8D 1D ? ? ? ? 31 F6" },
		{ "L-E-P2", "48 89 3B 39 35 ? ? ? ? 7F 09 8D 46 01 89 05" },

		// The parts of the parts' coding round, one group per limit, looked for as SPECIFICATIONS 4.2 says: WeaponLimits (W),
		// SirenSettings (I), whose two limits are measured first (SPECIFICATIONS 11.16), PedProps (A), Resources (Q)
		// and RayTracing (T)
		// WeaponLimits, the weapon infos (TweaksVWeaponInfos.cpp), looked for on every boot the part is on: the game's
		// initializer of the weapon info blob array - the number of blobs allocated, the call that allocates them, the
		// block's store and the capacity's (I1), the two immediates the part raises - and the loader's head - the count
		// read and written, the blob's size and the block (I2), 0x18 bytes into the loader the part's guard hooks
		{ "W-E-I1", "C7 05 ? ? ? ? 00 00 00 00 BA ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? ? ? 48 8D 0D" },
		{ "W-E-I2", "0F B7 05 ? ? ? ? 8D 48 01 66 89 0D ? ? ? ? 4C 69 E8 ? ? ? ? 4C 03 2D ? ? ? ?" },

		// WeaponLimits' weapon components (TweaksVWeaponComponents.cpp): the rebuild of the list of every weapon component
		// definition - its count's reset, the files' count and its lea of the list (C1) - its sort, with its lea of the list
		// and the entry size (C2), its update of the count (C3), the pool's size call, with the pool's name and default (C4),
		// and the loader of one weaponcomponents.meta, 0x13 bytes into it, where the guard hooks its head - the blob's size,
		// allocated and zeroed, and the blob array's count, capacity and pointer (C5)
		{ "W-E-C1", "C7 05 ? ? ? ? 00 00 00 00 44 0F B7 05 ? ? ? ? 66 45 85 C0 0F 84 ? ? ? ? 45 31 C9 4C 8D 15 ? ? ? ?" },
		{ "W-E-C2", "48 63 15 ? ? ? ? EB 02 31 D2 48 8D 0D ? ? ? ? 4C 8D 0D ? ? ? ? 41 B8 08 00 00 00" },
		{ "W-E-C3", "48 63 15 ? ? ? ? 8D 4A 01 89 0D ? ? ? ? 4D 89 04 D2" },
		{ "W-E-C4", "48 8B 0D ? ? ? ? BA CF 3C E3 BA 41 B8 ? ? ? ? E8 ? ? ? ? 89 C3" },
		{ "W-E-C5", "49 89 D7 49 89 CE B9 D8 0E 00 00 E8 ? ? ? ? 49 89 C4 41 B8 D8 0E 00 00 48 89 C1 31 D2 E8 ? ? ? ? 0F B7 1D ? ? ? ? 66 3B 1D ? ? ? ? 75 ? 44 0F B7 EB 49 8D 6D 10 48 8B 35 ? ? ? ?" },

		// SirenSettings, the siren setting IDs (TweaksVSirenIds.cpp): the parser registrations of the siren settings (P1) and the
		// vehicle variations (P2), whose tables lead to the members that carry an ID, and the simple member's XML reader (P3); the
		// loaders' four lea of the ID table with the ID's byte loads and stores and the free-ID searches' limits - the base
		// variations' loop (T1), the base builder (T2), a pack's variations (T3) and a pack's builder (T4); a pack's dedupe (D1)
		// and its compare loop (D2); the constructors of a siren setting (C1) and of a variation, in place (C2) and allocating
		// (C3); the vehicle metadata's init, its last fill of the table and its call of the base loader (R1); the getter of a
		// model's siren setting (M1); with DIAGNOSTIC_MODE and the part at work, a pack's carcols callback (H1) and the base
		// builder (H2), which it hooks, each at its function's start
		{ "I-E-P1", "C7 05 ? ? ? ? 0D 9B EB 84 0F 57 C0 0F 11 05 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05" },
		{ "I-E-P2", "C7 05 ? ? ? ? A9 FE EA 18 0F 57 C0 0F 11 05 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05" },
		{ "I-E-P3", "48 8B 41 08 0F B6 48 10 48 83 F9 21 0F 87" },
		{ "I-E-T1", "B9 4B 00 00 00 31 D2 4C 8D 05 ? ? ? ? 4C 8D 0D" },
		{ "I-E-T2", "41 0F B7 46 50 48 85 C0 0F 84 ? ? ? ? 49 8B 4E 48 31 D2 48 8D 1D" },
		{ "I-E-T3", "B8 4B 00 00 00 31 C9 4C 8D 05 ? ? ? ? 48 8D 2D" },
		{ "I-E-T4", "4C 8B 05 ? ? ? ? 49 8B 58 48 89 CA 48 8D 2D" },
		{ "I-E-D1", "4D 8B 6F 48 4D 69 FE E8 05 00 00 43 0F B6 54 3D 00 0F B7 7B 50 80 FA FF 74" },
		{ "I-E-D2", "31 F6 66 2E 0F 1F 84 00 00 00 00 00 38 54 35 00 0F 84 ? ? ? ? 48 81 C6 E8 05 00 00 48 39 F3 75 EA" },
		{ "I-E-C1", "56 48 83 EC 20 48 89 CE C7 41 1C 00 00 00 00 B8 83 00 00 00 0F 57 C0" },
		{ "I-E-C2", "C7 41 40 00 00 00 00 C7 41 48 00 00 FF FF C3" },
		{ "I-E-C3", "C7 40 40 00 00 00 00 C7 40 48 00 00 FF FF 48 83 C4 28 C3" },
		{ "I-E-R1", "66 0F 7F 05 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 80 3D" },
		{ "I-E-M1", "0F B6 81 3B 05 00 00 48 85 C0 74 13 48 8B 0D ? ? ? ? 48 69 C0 E8 05 00 00 48 03 41 48 C3" },
		{ "I-E-H1", "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 68 80 3A 00 0F 84 ? ? ? ?" },
		{ "I-E-H2", "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 28 49 89 CE 0F B7 71 10 89" },

		// [siren count]
		// SirenSettings, the siren count (TweaksVSirenCount.cpp): the vehicle lights' calls of the siren lights' renderer, which the
		// measurement redirects, and of the siren settings getter (B1); the renderer's read of a siren's bone through the model info's
		// bone structure (B2); a read of a siren's glass bone (B3); the start of find-bone-by-name, which tests the bone count first (B4)
		{ "I-E-B1", "48 89 6C 24 28 0F 95 44 24 20 4C 89 E1 E8 ? ? ? ? 49 89 EE 48 89 E9 E8" },
		{ "I-E-B2", "48 8B 87 B0 00 00 00 48 85 C0 74 ? 48 8B 00 48 85 C0 74 ? 0F BE 84 28 99 00 00 00" },
		{ "I-E-B3", "0F BE 94 18 AD 00 00 00 48 8B 8E C0 09 00 00" },
		{ "I-E-B4", "41 57 41 56 41 55 41 54 56 57 53 48 83 EC 20 66 83 79 5E 00" },

		// PedProps (TweaksVPedProps.cpp): the eleven narrowings to 8 bits the part widens - an anchor's total in the check
		// the set-prop native asks (T1) and in the helmet code after its four calls of the per-anchor count (T2..T5), a
		// worn prop's index before the metadata lookup (G1, which matches 2 times, G3..G6) - and the per-anchor count's
		// start (C1), where T2..T5's calls lead; then, for the measurement, the count's loop (C2), which with C1 pins the
		// variation data's layout, and the model info store's lookup followed by a ped's type (S1)
		{ "A-E-T1", "0F B6 C3 44 39 C0 7E ? E8 ? ? ? ? 0F B6 C0 44 39 F0 0F 9F C0" },
		{ "A-E-T2", "48 8B 0E 31 D2 E8 ? ? ? ? 0F B6 C0 39 C5 0F 4C C5 83 FD FF" },
		{ "A-E-T3", "48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 C7 0F 4C C7 83 FF FF" },
		{ "A-E-T4", "48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 F8 76 ? 89 7E 18 88 5E 20" },
		{ "A-E-T5", "48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 C7 7C ? 48 8B 0E 31 D2 E8" },
		{ "A-E-G1", "48 8B 84 24 ? ? ? ? 48 8B 88 40 02 00 00 44 0F B6 C6 40 0F B6 D5 E8" },
		{ "A-E-G3", "41 8B 44 04 08 83 F8 FF 0F 84 ? ? ? ? 0F B6 E8 48 89 F9 89 F2 41 89 E8 E8" },
		{ "A-E-G4", "8B 44 38 08 31 FF 83 F8 FF 74 ? 48 8B 49 20 48 8B 89 40 02 00 00 44 0F B6 C0 0F B6 D2 E8" },
		{ "A-E-G5", "41 8B 44 07 08 85 C0 0F 88 ? ? ? ? 44 0F B6 C0 48 89 F9 44 89 F2 E8" },
		{ "A-E-G6", "E8 ? ? ? ? 44 0F B6 F7 45 0F B6 FF 4C 89 E9 44 89 FA 45 89 F0 E8" },
		{ "A-E-C1", "56 57 48 8B 41 20 48 8B 88 40 02 00 00 44 0F B7 41 08 31 C0 4D 85 C0 74 ? 4C 8B 09 45 31 DB EB" },
		{ "A-E-C2", "44 0F B7 54 39 08 41 0F B6 CA 01 C8 49 FF C3 4D 39 C3 74 ? 4B 8B 0C D9 0F B7 71 60 45 31 D2 48 85 F6 74 ? 48 8B 49 58 48 C1 E6 03 48 8D 34 76 31 FF" },
		{ "A-E-S1", "3B 05 ? ? ? ? 0F 8D ? ? ? ? 48 8B 0D ? ? ? ? 89 C2 C1 EA 05 8B 0C 91 0F A3 C1 0F 83 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 ? 44 89 7C 24 34 48 98 48 0F AF 05 ? ? ? ? 4C 8B 3C 01 BB 02 00 00 00 4D 85 FF 74 ? 41 0F B6 87 9D 00 00 00 24 1F 3C 06" },

		// Resources, the texture budget (TweaksVTextureBudget.cpp): the start-up's store of the most streamed objects and its
		// load of budgets 0 and 1 (T1), its store of budgets 2 and 3 and the call after the stores (T2), the budget's getter
		// and the index it reads (T3), a resource's placement and its loads of the counters (T4), the update check and its
		// switch (T5)
		{ "Q-E-T1", "8B 88 4C 05 00 00 89 0D ? ? ? ? 66 0F 38 25 80 50 05 00 00" },
		{ "Q-E-T2", "66 0F 38 25 80 58 05 00 00 66 0F 73 F0 14 F3 0F 7F 05 ? ? ? ? 4C 8D 35 ? ? ? ? 4C 89 35 ? ? ? ? 48 83 EC 20 E8" },
		{ "Q-E-T3", "48 63 05 ? ? ? ? 48 8B 44 C1 28 C3" },
		{ "Q-E-T4", "8B 05 ? ? ? ? 8B 0D ? ? ? ? 01 C1 3B 0D ? ? ? ? 73 ? 48 8B 05 ? ? ? ? 48 8B 35" },
		{ "Q-E-T5", "83 3D ? ? ? ? 00 0F 84 ? ? ? ? 48 8D 0D ? ? ? ? 48 8B 3D ? ? ? ? 48 8B 35 ? ? ? ? E8" },

		// RayTracing (TweaksVRayTracing.cpp): the set-up of the ray tracing buffer of moving geometry - its count, its stride, the
		// call of the buffer maker and the store of the buffer object (B1) - and the two checks of a frame's ranges against its
		// capacity, every moving object's geometry's (C1) and the ropes' (C2), each with the frame slot, the counters, the
		// capacity and the refusal jump
		{ "T-E-B1", "C7 44 24 20 89 00 00 00 B9 ? ? ? ? BA 14 00 00 00 41 B0 01 45 31 C9 E8 ? ? ? ? 48 89 05 ? ? ? ?" },
		{ "T-E-C1", "8B 2D ? ? ? ? 31 D2 48 8D 1D ? ? ? ? 41 BA 01 00 00 00 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 8B 3C AB 8D 34 0F 81 FE ? ? ? ? 0F 87" },
		{ "T-E-C2", "8B 15 ? ? ? ? 31 ED 48 8D 35 ? ? ? ? 41 BA 01 00 00 00 ? ? ? ? ? ? ? ? ? ? ? ? 8B 3C 96 8D 1C 0F 81 FB ? ? ? ? 0F 87" },

		// StaticBounds (TweaksVStaticBounds.cpp), the patterns of Nochala's Static Bounds Limit Adjuster: the two functions the
		// game makes its pools through - the set-up, the whole of it down to its ret (P1), and the allocator, from its pushes
		// on to its call of the memory allocator (A1)
		{ "C-E-P1", "48 89 C8 48 8B 4C 24 ? 48 89 08 44 89 48 1C 0F 57 C0 0F 11 40 08 C7 40 18 00 00 00 00 81 60 28 00 00 00 C0 C3" },
		{ "C-E-A1", "56 57 48 83 EC ? 48 89 CE 48 63 51 18 48 63 79 1C 48 0F AF FA 48 83 C7 07 48 83 E7 F8 48 8B 09 48 01 FA 48 8B 01 FF 50 08" },

		// LightCapacity, the lighting capacity (TweaksVLightCapacity.cpp): the frame's two stores of vehicle lights, reset (A1) and
		// drained (A2, A3); a job's 5 vehicles (B1) and its scratch's size in its two kicks (B2, B3); add-to-a-job's-list (C1), the
		// job's scratch (C2, C3) and its lists' start values (C4); the scene store's check (D1) and its byte index read back (D2),
		// its two sides (E1, E2), the ray traced lists (E3, F1), their upload (F2) and the reflection lists (F3); the corona store,
		// twice in one count (G1), and the drain's add to it (G2); the worker's frame (H1) and the job's scratch in it (H2)
		{ "G-E-A1", "48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 89 15 ? ? ? ? 4C 8D 0D ? ? ? ?" },
		{ "G-E-A2", "48 8D 1D ? ? ? ? 48 39 1D ? ? ? ? 0F 86" },
		{ "G-E-A3", "48 8D 35 ? ? ? ? 48 39 35 ? ? ? ? 76 ? 0F 1F 44 00 00 48 89 F1 31 D2 45 31 C0 45 31 C9 E8" },
		{ "G-E-B1", "69 05 ? ? ? ? CD CC CC CC 05 99 99 99 19 3D 32 33 33 33 77" },
		{ "G-E-B2", "C7 44 1A 70 80 B1 00 00 F0 FF 05" },
		{ "G-E-B3", "42 C7 44 0A 70 80 B1 00 00" },
		{ "G-E-C1", "8B 41 14 3B 41 1C 0F 83 ? ? ? ? 48 89 D7 49 89 CE 48 8B 59 08" },
		{ "G-E-C2", "4C 8D 48 0F 49 83 E1 F0 4C 89 CA 48 29 C2 49 8D A9 80 25 00 00 BB 00 8C 00 00 48 29 D3 48 81 C2 FF 73 FF FF 45 31 C0 48 81 FA 7F 4E FF FF BE 80 B1 00 00" },
		{ "G-E-C3", "48 05 00 8C 00 00 48 81 FA 00 8C 00 00" },
		{ "G-E-C4", "48 89 7C 24 68 0F 28 0D ? ? ? ?" },
		{ "G-E-D1", "48 63 05 ? ? ? ? 48 3D FF 03 00 00 0F 8F ? ? ? ? 48 63 0D ? ? ? ? 48 8D 15 ? ? ? ? 8D 59 01 89 1D ? ? ? ? 88 04 CA" },
		{ "G-E-D2", "0F B6 1C C1 F3 0F 11 74 C1 04 4A 8D 14 C1 E8" },
		{ "G-E-E1", "48 69 C7 00 00 07 00 48 8D 15 ? ? ? ? 48 01 C2 48 89 15 ? ? ? ? 48 8D 05 ? ? ? ? 44 8B 04 B8 44 89 05 ? ? ? ? 48 69 C7 0C 9A 02 00 48 8D 0D ? ? ? ? 48 01 C1 48 89 0D" },
		{ "G-E-E2", "48 69 C8 00 00 07 00 48 8D 15 ? ? ? ? 48 01 CA 48 89 15 ? ? ? ? 48 8D 0D ? ? ? ? C7 04 81 00 00 00 00 48 69 C8 0C 9A 02 00 48 8D 15" },
		{ "G-E-E3", "48 69 C8 10 20 07 00 48 8D 15 ? ? ? ? 48 8D 34 0A 48 89 35 ? ? ? ? 8B 04 11" },
		{ "G-E-F1", "4C 8B 25 ? ? ? ? 45 8B 34 24 41 81 FE FF 03 00 00 0F 8F" },
		{ "G-E-F2", "44 0F B6 05 ? ? ? ? 48 81 C1 E0 70 00 00 BA 00 02 00 00 E8" },
		{ "G-E-F3", "4B 63 0C 88 48 81 F9 FF 02 00 00" },
		{ "G-E-G1", "8B 0D ? ? ? ? 48 81 F9 3F 04 00 00 0F 87" },
		{ "G-E-G2", "44 8B 81 00 CC 00 00 49 81 F8 3F 04 00 00 0F 87" },
		{ "G-E-H1", "B8 58 80 01 00 E8 ? ? ? ? 48 29 C4" },
		{ "G-E-H2", "48 8D 44 24 50 49 89 04 24 4C 89 E1 41 FF 54 24 60" },
		// The two model budgets (TweaksVPopulation.cpp), measured first: the crash context's line, which leads to the population object and both getters (C1); the vehicle pass's sites, the check before a request (V1, V2), the pool's test (V3), the tables' set-up's head (V4), the ambient cars (V5), and a writing half's call and stores (V6, V7); the ped pass's, the sum, the gate, the lists, a model's peds (P1 to P4), and a writing half's stores and the variety's store (P5 to P8)
		{ "O-E-C1", "8B 35 ? ? ? ? C1 FE 14 8B 3D ? ? ? ? C1 FF 14 48 8D 1D ? ? ? ? 48 89 D9 E8 ? ? ? ? 89 C5 C1 FD 14 48 89 D9 E8 ? ? ? ? 89 C3 C1 FB 14 48 8D 0D" },
		{ "O-E-V1", "8B 05 ? ? ? ? 0F B6 0D ? ? ? ? 4C 8D 35 ? ? ? ? 48 8D 1D ? ? ? ? EB" },
		{ "O-E-V2", "41 8B 14 96 0F 57 C0 F3 48 0F 2A C2 F3 0F 59 05 ? ? ? ? F3 0F 2C D0 81 C2 A0 1C E9 FF 39 97 14 62 00 00 7D ? 48 89 F9 E8" },
		{ "O-E-V3", "48 8B 05 ? ? ? ? 8B 68 28 C1 E5 02 C1 FD 02 2B 68 18 83 C5 14 85 ED 0F 8E" },
		{ "O-E-V4", "48 8B 05 ? ? ? ? 48 8B 58 18 48 8B 35 ? ? ? ? 69 BB F4 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 40 67 D1 1F" },
		{ "O-E-V5", "44 8B 35 ? ? ? ? 8B 05 ? ? ? ? 89 84 24 ? ? ? ? 44 8B 3D ? ? ? ? 8B 35" },
		{ "O-E-V6", "41 0F B7 C6 89 34 87 E8 ? ? ? ? 0F 28 74 24 40 48 83 C4 58" },
		{ "O-E-V7", "69 BB 0C 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05 ? ? ? ? 48 8B 35 ? ? ? ? 69 BB 10 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05 ? ? ? ? 48 8B 35 ? ? ? ? 69 BB 14 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05" },
		{ "O-E-P1", "8B 86 10 62 00 00 03 86 0C 62 00 00 03 86 2C 62 00 00 03 86 30 62 00 00 89 05" },
		{ "O-E-P2", "41 8B 8C 24 08 62 00 00 85 C9 79 08 41 8B 8C 24 04 62 00 00 41 8B AC 24 0C 62 00 00 D1 F9 69 D9 A0 1C E9 FF 81 C3 A0 1C E9 FF 8B 0D" },
		{ "O-E-P3", "48 8D 82 04 4E 00 00 4C 8D 8A 04 56 00 00 4C 8D 82 04 52 00 00 48 81 C2 04 5E 00 00 48 89 44 24 20 48 8D 4C 24 70 E8 ? ? ? ? 0F B7 05" },
		{ "O-E-P4", "48 8B 0D ? ? ? ? 48 98 48 0F AF 05 ? ? ? ? 48 8B 04 01 8B 80 58 01 00 00 42 89 44 BC 40" },
		{ "O-E-P5", "69 BB F4 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 40 67 D1 1F E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05" },
		{ "O-E-P6", "69 BB F8 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA D0 3C 99 0A E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05" },
		{ "O-E-P7", "69 BB FC 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 5C 10 5F F4 E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05" },
		{ "O-E-P8", "F2 0F 10 86 B4 00 00 00 0F 59 05 ? ? ? ? 0F 58 05 ? ? ? ? F3 0F 10 0D ? ? ? ? 0F 28 D0 0F C2 15 ? ? ? ? 01 66 0F 38 25 D2 66 0F 50 C2 F3 0F 10 15 ? ? ? ? 0F 28 DA A8 01 75 07 0F 28 D9 F3 0F 5D D8 F3 0F 11 1D" },
		// The ped pool, measured first (TweaksVPedPool.cpp): for its reads the accessor, the maker, the lookups, the spatial array, the counts and caps; then the
		// sites its writing half will rely on, the copy and the two frames of a fixed array and the anim director's family's six defaults
		{ "N-E-A1", "0F B6 05 ? ? ? ? A8 01 75 03 31 C0 C3 48 8B 0D ? ? ? ? 48 C1 C1 1E 48 8B 15 ? ? ? ? 48 31 CA 48 C1 C2 20 80 E1 1F 80 C1 02 48 D3 C2 48 F7 D2" },
		{ "N-E-M1", "48 8D 05 ? ? ? ? 48 89 44 24 28 C7 44 24 20 80 19 00 00" },
		{ "N-E-M2", "BA 17 21 A1 8D 41 89 D8 E8 ? ? ? ? 89 C3" },
		{ "N-E-C1", "BA 17 21 A1 8D 41 B8 01 00 00 00 E8 ? ? ? ? 89 C1 E8 ? ? ? ?" },
		{ "N-E-I1", "BA 84 C5 4A 39 41 89 D8 E8 ? ? ? ? 89 C3" },
		{ "N-E-S1", "48 63 72 18 48 83 C6 03 48 83 E6 FC" },
		{ "N-E-K1", "8B 05 ? ? ? ? 8B 0D ? ? ? ? 8B 35 ? ? ? ? 8B 3D ? ? ? ? 44 89 74 24 50" },
		{ "N-E-K2", "F3 0F 2C C2 89 05 ? ? ? ? F3 0F 59 C1 F3 0F 2C C0 89 05 ? ? ? ? 66 0F 7E 3D ? ? ? ?" },
		{ "N-E-R1", "8B 3A E8 ? ? ? ? 39 C7 7D 04 8B 03 EB 05 E8 ? ? ? ? 89 46 10 8B 03 31 FF 85 C0 0F 4F F8 89 7E 10" },
		{ "N-E-R2", "66 C7 05 ? ? ? ? 00 00 E8 ? ? ? ? 44 0F B7 05" },
		{ "N-E-R3", "48 81 EC 28 0C 00 00 48 8D 8C 24 20 08 00 00 48 8D 94 24 20 04 00 00 4C 8D 44 24 20 E8" },
		{ "N-E-R4", "B8 48 10 00 00 E8 ? ? ? ? 48 29 C4 48 8B 05 ? ? ? ? 48 63 70 5C" },
		{ "N-E-D1", "BA 10 D8 5E CA 41 B8 00 04 00 00 E8" },
		{ "N-E-D2", "BA CA 6C 77 36 41 B8 00 04 00 00 E8" },
		{ "N-E-D3", "BA 7A EE AE CF 41 B8 55 09 00 00 E8" },
		{ "N-E-D4", "BA 46 99 C5 3C 41 B8 AA 02 00 00 E8" },
		{ "N-E-D5", "BA 29 BB 69 BF 41 B8 55 01 00 00 E8" },
		{ "N-E-D6", "BA BE 10 0E 21 41 B8 55 01 00 00 E8" },

		// The render queues of streamed peds and vehicles, measured first (TweaksVRenderQueues.cpp): each pool's make, which holds what decodes
		// its pointer and pins it by its stride - the streamed peds' requests (S1) and render data (S2), the props' (P1, P2), the vehicles' (V1, V2) -
		// each family's add (S3, P3, V3) and init (S4, P4, V6), the vehicles' walk (V4) and their two sizes (V5), the pools' size lookups a raise
		// redirects (N1 to N6), the frame's calls of the three flips (F1), and the draw list's references, their capacity (D1), index (D2) and manager (D3)
		{ "U-E-S1", "0F B6 05 ? ? ? ? 48 8B 05 ? ? ? ? 48 C1 C0 ? 48 8B 15 ? ? ? ? 48 31 C2 48 C1 C2 20 24 1F 8D 48 ? 48 89 D5 48 D3 E5 34 3F 89 C1 48 D3 EA 48 09 EA 48 F7 D2 0F B6 05 ? ? ? ? A8 01 B9 00 00 00 00 48 0F 45 CA BA 08 05 00 00 E8" },
		{ "U-E-S2", "8B 42 28 C1 E0 02 C1 F8 02 39 42 18 7E 61 0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 15 ? ? ? ? 31 C9 F6 C2 01 48 0F 45 C8 BA E0 02 00 00 E8" },
		{ "U-E-S3", "66 41 FF 86 D8 02 00 00 48 63 05 ? ? ? ? 48 C1 E0 04 48 8D 0D ? ? ? ? 48 8B 14 08 0F B7 6C 08 08 8D 5D 01 66 89 5C 08 08 4C 89 34 EA" },
		{ "U-E-S4", "B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 80 3D" },
		{ "U-E-P1", "0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 80 E1 1F 80 C1 ? 48 D3 C0 48 C1 C0 20 48 F7 D0 0F B6 0D ? ? ? ? 45 31 FF F6 C1 01 49 0F 44 C7 BA 80 00 00 00 48 89 C1 E8 ? ? ? ? 49 89 C5 C7 00 FF FF FF FF 66 C7 40 04 00 7F" },
		{ "U-E-P2", "0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 0D ? ? ? ? F6 C1 01 48 0F 44 C7 BA 3C 00 00 00 48 89 C1 E8" },
		{ "U-E-P3", "66 41 FF 47 38 48 63 05 ? ? ? ? 48 C1 E0 04 48 8D 0D ? ? ? ? 48 8B 14 08 0F B7 5C 08 08 8D 7B 01 66 89 7C 08 08 4C 89 3C DA" },
		{ "U-E-P4", "B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 48 83 C4 28 C3" },
		{ "U-E-V1", "0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 15 ? ? ? ? 31 C9 F6 C2 01 48 0F 45 C8 BA C8 01 00 00 E8 ? ? ? ? 48 89 C6 48 89 C7 48 8D 05" },
		{ "U-E-V2", "0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 80 E1 1F 80 C1 ? 48 D3 C0 48 C1 C0 20 48 F7 D0 0F B6 0D ? ? ? ? 31 D2 F6 C1 01 48 0F 45 D0 8B 42 28 C1 E0 02 C1 F8 02 48 8B 5F 48 39 42 18 7E 6B" },
		{ "U-E-V3", "66 41 FF 84 24 A8 0B 00 00 48 63 05 ? ? ? ? 48 C1 E0 04 4A 8B 0C 30 42 0F B7 54 30 08 8D 5A 01 66 42 89 5C 30 08 4C 89 24 D1" },
		{ "U-E-V4", "49 89 CC 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 0D ? ? ? ? 48 C1 E1 04 4C 8D 35 ? ? ? ? 42 0F B7 44 31 08 48 85 C0" },
		{ "U-E-V5", "48 8B 0D ? ? ? ? BA F4 BF D4 90 41 B8 01 00 00 00 E8 ? ? ? ? 89 05 ? ? ? ? 48 8B 0D ? ? ? ? BA 67 52 1D E1 41 B8 01 00 00 00 E8 ? ? ? ? 89 05 ? ? ? ?" },
		{ "U-E-V6", "48 63 35 ? ? ? ? 48 85 F6 0F 84 ? ? ? ? 48 8D 0C F5 00 00 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 63 3D ? ? ? ? 48 89 05 ? ? ? ? 66 89 35" },
		{ "U-E-N1", "48 8B 0D ? ? ? ? BA F6 68 B9 0A 41 B8 37 00 00 00 E8" },
		{ "U-E-N2", "48 8B 0D ? ? ? ? BA 34 0C 37 2D 41 B8 78 00 00 00 E8" },
		{ "U-E-N3", "48 8B 0D ? ? ? ? BA 2D 46 FB 85 41 B8 32 00 00 00 E8" },
		{ "U-E-N4", "48 8B 0D ? ? ? ? BA B8 8E 9C 53 41 B8 78 00 00 00 E8" },
		{ "U-E-N5", "48 8B 0D ? ? ? ? BA 86 3C 75 54 41 89 D8 E8" },
		{ "U-E-N6", "48 8B 0D ? ? ? ? BA 89 F0 B5 C2 41 89 D8 E8" },
		{ "U-E-F1", "56 57 48 83 EC 28 89 D6 48 89 CF E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 89 F9 89 F2 48 83 C4 28 5F 5E E9" },
		{ "U-E-D1", "48 89 86 D0 05 00 00 66 C7 86 DA 05 00 00 80 28 B9 00 88 02 00 BA 08 00 00 00 E8" },
		{ "U-E-D2", "8B 81 A4 00 00 00 8B 91 AC 00 00 00 FF C0 83 E0 03 89 81 A4 00 00 00 FF C2 83 E2 03 89 91 AC 00 00 00 C3" },
		{ "U-E-D3", "B9 A0 4B 01 00 E8 ? ? ? ? 48 89 C6 48 89 C1 E8 ? ? ? ? 48 89 35 ? ? ? ?" },
	};

	// A text the scanner can use: bytes as two hex digits or ?, one space apart, at least one of them
	// not ? (a pattern of only ? would match anywhere).
	consteval bool IsPatternText(std::string_view text)
	{
		const auto hex = [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); };
		bool aByte = false;
		size_t i = 0;
		while (i < text.size())
		{
			if (text[i] == '?')
				i += 1;
			else if (i + 1 < text.size() && hex(text[i]) && hex(text[i + 1]))
			{
				aByte = true;
				i += 2;
			}
			else
				return false;
			if (i < text.size() && (text[i] != ' ' || i + 1 == text.size()))
				return false;
			i += 1;
		}
		return aByte;
	}

	consteval bool PatternsAreValid()
	{
		for (size_t i = 0; i < std::size(kPatterns); i++)
		{
			if (!IsPatternText(kPatterns[i].text))
				return false;
			for (size_t j = 0; j < i; j++)
				if (std::string_view(kPatterns[i].label) == kPatterns[j].label)
					return false;
		}
		return true;
	}
	static_assert(PatternsAreValid(), "TweaksV: a pattern in kPatterns is malformed, or a label is used twice");

	// The entry with this label - checked when TweaksV is compiled: a label that is not in kPatterns
	// does not build.
	consteval const GamePattern& Pattern(std::string_view label)
	{
		for (const GamePattern& pattern : kPatterns)
			if (label == pattern.label)
				return pattern;
		throw "TweaksV: this pattern is not in kPatterns";
	}

	// The byte a pattern holds at index, or -1 for a wildcard and an index past its end - for the checks,
	// when TweaksV is compiled, that a constant is the byte the pattern pins it with.
	consteval int PatternByte(const GamePattern& pattern, size_t index)
	{
		const auto hex = [](char c) { return c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10; };
		const std::string_view text = pattern.text;
		size_t i = 0;
		for (size_t token = 0; i < text.size() && token < index; token++)
			i += text[i] == '?' ? 2 : 3;	// a token and the space after it
		if (i >= text.size() || text[i] == '?')
			return -1;
		return hex(text[i]) * 16 + hex(text[i + 1]);
	}

	// True if the pattern holds these bytes from index at on, none of them a wildcard - for the checks, when TweaksV is
	// compiled, that an instruction the code reads is the one its pattern pins.
	consteval bool PatternHolds(const GamePattern& pattern, size_t at, std::initializer_list<int> bytes)
	{
		for (const int byte : bytes)
			if (PatternByte(pattern, at++) != byte)
				return false;
		return true;
	}
	consteval bool PatternHolds(const GamePattern& pattern, size_t at, const uint8_t* bytes, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			if (PatternByte(pattern, at + i) != bytes[i])
				return false;
		return true;
	}
	consteval bool PatternHolds(const GamePattern& pattern, size_t at, std::string_view bytes)
	{
		for (size_t i = 0; i < bytes.size(); i++)
			if (PatternByte(pattern, at + i) != static_cast<uint8_t>(bytes[i]))
				return false;
		return true;
	}

	// The imm32 or dword a pattern holds at index at, or -1 when a byte of it is a wildcard.
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
}
