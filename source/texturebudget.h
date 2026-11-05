// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the texture budget, the Resources part. The game places a streamed texture or buffer only while their video
// memory stays below one of four budgets from gameconfig.xml, chosen by the texture quality - 3000 MB at the highest - and
// evicts what it cannot place, so a heavy texture load goes missing however much memory the card has. VIDEO_MEMORY_BUDGET
// in [RESOURCES] raises each budget below it to it as the game's start-up sets them, and STREAMED_OBJECTS the most streamed
// textures and buffers the game places, 98304 in its own file, in the same call; with DIAGNOSTIC_MODE=1 TweaksV also logs how
// close the streamed textures and buffers come to the budget in use and to the most, and how much of the game's descriptor
// heap they take, the one thing behind that number, which has no check at its end.
#pragma once

namespace TweaksV
{
	// The texture budget, the Resources part (SPECIFICATIONS 11.12), started by the part's entry of the feature table, from
	// DllMain, while [RESOURCES] is on: finds and checks the budget's sites, redirects the call after the start-up's stores
	// of the budgets when VIDEO_MEMORY_BUDGET or STREAMED_OBJECTS asks for a raise, and with DIAGNOSTIC_MODE=1 starts the
	// measurement.
	void InitTextureBudget();
}
