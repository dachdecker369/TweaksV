// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part: the game's pool of static collision bounds, whose running out ends heavy map
// mods in an access violation, given at least STATICBOUNDS_SIZE slots (SPECIFICATIONS 11.13).
#pragma once

namespace TweaksV
{
	// The StaticBounds part, started by its entry of the feature table, from DllMain, while [STATICBOUNDS] is on: the
	// call that asks the game's configuration for the pool's size is found, checked and redirected, so that the pool
	// gets the larger of the game's size and STATICBOUNDS_SIZE when the game makes it; a thread checks, 60 seconds after
	// the session's start, that the game has asked, and reports it when it has not.
	void InitStaticBounds();
}
