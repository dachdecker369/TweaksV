// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the sirens per vehicle's damage (D23): the room behind a model's bone structure, its fill, and the game's own damage
// loops raised to read it, so that a hit, an impact and a wreck reach lights 21 to 32 as they do lights 1 to 20.
#pragma once

namespace TweaksV
{
	struct SirenWrites;

	// The damage's half of the part's one write: finds and checks its sites and adds its writes to list, writing nothing. False,
	// reported through SirensNotRaised, at the first site that is not as the findings have it.
	bool FindSirenDamage(int sirens, SirenWrites& list);
}
