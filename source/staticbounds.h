// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part: the game's store of static collision bounds, whose running out ends heavy map
// mods at a trap, given at least STATICBOUNDS_SIZE slots (SPECIFICATIONS 11.15).
#pragma once

namespace TweaksV
{
	// The StaticBounds part, started by its entry of the feature table, from DllMain, while [STATICBOUNDS] is on: the one call
	// the game reads the store's size from is found and redirected, so that the store named StaticBounds gets the larger of the
	// game's size and STATICBOUNDS_SIZE when the game makes it; a thread checks, 60 seconds after the session's start, that the
	// game has asked, reports it when it has not, and reads the store back.
	void InitStaticBounds();
}
