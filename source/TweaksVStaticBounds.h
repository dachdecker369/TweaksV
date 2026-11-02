// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the StaticBounds part: the game's pool of static collision bounds, whose running out ends heavy map
// mods in an access violation, given at least STATICBOUNDS_SIZE slots (SPECIFICATIONS 11.15).
#pragma once

namespace TweaksV
{
	// The StaticBounds part, started by its entry of the feature table, from DllMain, while [STATICBOUNDS] is on. It
	// changes nothing yet.
	void InitStaticBounds();
}
