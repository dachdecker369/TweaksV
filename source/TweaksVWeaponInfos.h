// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the weapon infos, the WeaponLimits part's half for the weapons.meta files. The game parses each
// weapons.meta file into one weapon info blob, and its array of blobs has room for 160 of them on 1.0.1158.16: a 161st
// file is written past the array's end, into the game's heap, and the game crashes while loading or later. The half
// raises that room to WEAPON_INFO_FILES and guards it: a file past the room is refused and reported instead.
#pragma once

namespace TweaksV
{
	// The weapon infos' half of the WeaponLimits part (SPECIFICATIONS 11.10.1), started by the part's entry of the
	// feature table, from DllMain, while [WEAPON_LIMITS] is on: the raise, the guard on the game's loader and, with
	// DIAGNOSTIC_MODE=1, the measurement.
	void InitWeaponInfos();
}
