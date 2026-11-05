// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the two model budgets of the population, a limit of a part to come, measured first. The game keeps a budget in
// bytes for the vehicle models and one for the ped models it holds in memory, and asks for no more past it, so a load with
// many add-on models repeats fewer of them than it could, without an error. With DIAGNOSTIC_MODE=1 the module samples both
// budgets once a second and logs how much of each is in use, how long the game stands at its stop, the tables the budgets
// come from and how many models they hold (SPECIFICATIONS 11.16.4).
#pragma once

namespace TweaksV
{
	// Finds and checks the sites of the two model budgets and starts their measurement, which writes nothing. Called by the
	// table of the parts measured first, with DIAGNOSTIC_MODE=1 only.
	void MeasurePopulation();
}
