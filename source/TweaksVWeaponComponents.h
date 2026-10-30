// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the weapon components limit of the WeaponLimits part to come, measured first. The game keeps every
// weapon component definition of all weaponcomponents.meta files in one list of 470 entries and writes past its
// end when a mod load brings more. With DIAGNOSTIC_MODE=1 the module finds and checks every site the part's
// writing half will use and logs how many definitions the game holds, from how many files; it writes nothing.
#pragma once

namespace TweaksV
{
	// Finds and checks the sites of the weapon component list, its count and its pool's size, and starts the
	// measurement. Called once by StartFeatures, with DIAGNOSTIC_MODE=1 only (the table of the parts measured first).
	void MeasureWeaponComponents();

	// The weapon components' half of the WeaponLimits part (SPECIFICATIONS 11.10.2), started by the part's entry of
	// the feature table, from DllMain, while [WEAPON_LIMITS] is on, after the weapon infos' half. It changes nothing yet.
	void InitWeaponComponents();
}
