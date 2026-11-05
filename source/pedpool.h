// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the ped pool, a limit of a part to come, measured first. The game keeps its peds in a pool of 256 slots, sized by Peds in
// gameconfig.xml, and raising that number alone is a trap, as three of the game's arrays stay at 256. With DIAGNOSTIC_MODE=1 the module
// finds and checks the sites a raise would rely on and logs how close a load comes to the pool, the population's caps and the spatial
// array (SPECIFICATIONS 11.16).
#pragma once

namespace TweaksV
{
	// Finds and checks the ped pool's sites and starts its measurement, which writes nothing. Called by the table of the parts measured
	// first, with DIAGNOSTIC_MODE=1 only.
	void MeasurePedPool();
}
