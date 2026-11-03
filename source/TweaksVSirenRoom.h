// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - what the two halves of the sirens per vehicle share (TweaksVSirenCount, the lights; TweaksVSirenDamage, the damage):
// the room behind a model's bone structure, and the one list their writes go in. The game's bone structure is a pool element of
// 0x34C bytes whose signed bytes name the bones of siren1 to siren20 and of their glass. The part raises the pool's stride and
// the allocation by 64 bytes, so that the room lies behind every field of the game's, which do not move: 32 bytes of the sirens'
// bones, then 32 of the glass's, siren1's and siren_glass1's first (PLAN.md, D23).
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "TweaksVPatch.h"

namespace TweaksV
{
	inline constexpr int kGameSirens = 20;				// siren1 to siren20: the sirens the game lights, and the glass bones it names

	inline constexpr size_t kRoomSirenBonesAt = 0x34C;	// in the bone structure: the game's own size, where the room's first byte lies, siren1's
	inline constexpr size_t kRoomGlassBonesAt = 0x36C;	// in the bone structure: siren_glass1's byte in the room
	inline constexpr size_t kRoomEnd = 0x38C;			// the bone structure's raised size, the pool's stride and the allocation

	// What the part's one write holds: each half adds to it, and nothing is written until both have found and checked every site
	// (InitSirensPerVehicle, TweaksVSirenCount.cpp).
	struct SirenWrites
	{
		std::vector<CodeWrite> writes;
		std::vector<CallRedirect> redirects;
		std::vector<FunctionHook> hooks;
	};

	// "The sirens per vehicle are not raised this session: <why>", the part's one problem, for a check that fails: the box, and
	// the vehicles stay at 20.
	void SirensNotRaised(const std::string& why);
}
