// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the weapon components limit, the WeaponLimits part's second half. The game keeps every weapon component
// definition of all weaponcomponents.meta files in one list of 470 entries and in a pool of 470 slots, and ends in its
// trap - or writes past the list's end - when a mod load brings more. The module moves the list on the relocation core
// and gives the pool the list's new size, WEAPON_COMPONENTS_SIZE, through the call that reads the pool's size.
#pragma once

namespace TweaksV
{
	// The weapon components' half of the WeaponLimits part (SPECIFICATIONS 11.10.2), started by the part's entry of the
	// feature table, from DllMain, while [WEAPON_LIMITS] is on, after the weapon infos' half: the list moved and the pool's
	// size call redirected, all or nothing, and with DIAGNOSTIC_MODE=1 the measurement.
	void InitWeaponComponents();
}
