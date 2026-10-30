// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the game's pool size lookup, the one function the game asks for the size of every pool it makes, found once a
// session through the weapon components' pool size call (W-E-C4) for the parts that work on a pool's size call: the weapon
// components, which redirect that call, and StaticBounds, which looks for the static bounds pool's call of the same lookup.
#pragma once

#include <cstdint>
#include <string>

#include "TweaksVPatterns.h"

namespace TweaksV
{
	// The pattern the lookup is found by: the weapon components' pool size call (PATTERNS.md, "Weapon components") - the one
	// use of W-E-C4 in the code, its sites checked when TweaksV is compiled (TweaksVPools.cpp). The parts name it in their
	// lines by its label; none looks for it itself.
	inline constexpr const GamePattern& kPoolSizeCallPattern = Pattern("W-E-C4");

	// The weapon components' pool size call - "mov rcx, [rip+disp32]" of the game's configuration, "mov edx, imm32" of the
	// pool's name, "mov r8d, imm32" of its default and "call rel32" of the game's pool size lookup, which takes the three and
	// returns gameconfig.xml's size for the pool, else the default - as FindPoolSizeCall found it: addresses in the game's
	// code, null where nothing was found.
	struct PoolSizeCall
	{
		uint8_t* call = nullptr;				// the call of the lookup (+18), which the weapon components redirect - null when W-E-C4 was not found
		const uint8_t* target = nullptr;		// where the call led when it was found, before a part redirected it - null when it cannot be read
		const uint8_t* lookup = nullptr;		// the game's pool size lookup: the target, when it lies inside the game - null when not
		uint32_t pool = 0;						// the pool's name the call hands the lookup (+8): 0xBAE33CCF, "cweaponcomponentinfo"
		const uint8_t* defaultSize = nullptr;	// the imm32 of the pool's default size the call hands the lookup (+14)
		const uint8_t* configuration = nullptr;	// where the game keeps its configuration's pointer, which the call loads (+3)
	};

	// W-E-C4 looked for the first time a part asks - the scanner's lines written once - and its call checked as TweaksV
	// checks a call it redirects or takes a target from: a "call rel32" that can be read and leads into the game, another
	// mod's hook leading outside it. The outcome is kept for the session, so that every part that asks gets the same,
	// whichever of them is on, and the lookup as the game had it, before a part redirected the call. From DllMain only.
	// False when the lookup is not known, with reason for the part's own PROBLEM line - "W-E-C4 was not found." or "the
	// weapon components' pool size call at RVA ... is not a call into the game (another mod may have hooked it already)
	// (W-E-C4)."; what was found is in found all the same, for the part's DIAGNOSTIC line.
	bool FindPoolSizeCall(PoolSizeCall& found, std::string& reason);
}
