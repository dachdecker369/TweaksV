// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the check for packs made for GTA V Legacy, a job of the RageOpenV part: once the mods folder is in use,
// a thread of its own reads the resource versions in the entry tables of the folder's archives, the archives nested
// in them too, and names every pack that carries Legacy models or textures - in the log and in one box, since a
// Legacy vehicle stops the game as it is created (SPECIFICATIONS 11.6).
#pragma once

namespace TweaksV
{
	// Starts the check on a thread of its own, once. Called by RageOpenV's start-up on the game's thread, once the
	// mods folder's file functions are hooked, so the game never waits for the check. A check that cannot start is
	// reported. Never throws.
	void StartLegacyPacksCheck();
}
