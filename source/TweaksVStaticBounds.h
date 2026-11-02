// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part: the game's pool of static collision bounds, whose running out ends heavy map
// mods in an access violation, given at least STATICBOUNDS_SIZE slots (SPECIFICATIONS 11.15).
#pragma once

namespace TweaksV
{
	// The StaticBounds part, started by its entry of the feature table, from DllMain, while [STATICBOUNDS] is on: the two
	// functions the game makes its pools through are found and hooked, all or none, so that the pool named StaticBounds
	// gets the larger of the game's size and STATICBOUNDS_SIZE when the game makes it; a thread checks, 60 seconds after the
	// session's start, that the game has made it, and reports it when it has not.
	void InitStaticBounds();
}
