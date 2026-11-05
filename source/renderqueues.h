// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the render queues of streamed peds and vehicles, a limit measured first. The game's per-frame arrays of streamed peds
// and of their props hold 120 entries with no check on their add, so that past them it writes past a heap block; the pools behind
// them stop the game when full, or take a vehicle's mods. With DIAGNOSTIC_MODE=1 the module reads how near a mod load comes, and
// writes nothing (SPECIFICATIONS 11.16).
#pragma once

namespace TweaksV
{
	// Finds and checks the render queues' sites and starts their measurement. Called by the table of the parts measured first, with
	// DIAGNOSTIC_MODE=1 only.
	void MeasureRenderQueues();
}
