// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the siren setting IDs, the SirenSettings part's first job. A siren setting of carcols.meta and the vehicle
// variation that names it carry its ID in one byte, and a table of 256 bytes maps an ID to the setting's position in the
// game's array, a byte too: so the game has 256 IDs and 255 settings a vehicle can use, and a pack whose IDs clash with
// others' loses its settings silently. LARGE_SIREN_IDS=1 makes the IDs 16 bits on the whole load path, the table's
// 65,536 entries a byte each, so that 255 settings stay the most and packs choose among 65,535 IDs. With DIAGNOSTIC_MODE=1
// the module logs how many settings and IDs the game and the mods use, and which settings the game dropped or moved -
// with the part off or LARGE_SIREN_IDS=0 too, over the game's own table, so that a start with it and one without compare.
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace TweaksV
{
	// Where the game's siren settings lie this moment, as its carcols data has them: their array, how many it holds and the
	// bytes from one setting to the next. For another module's reads, through CopyMemorySafely: the array grows while packs
	// load, so these hold for one read alone.
	struct SirenSettingsArray
	{
		const uint8_t* array;
		size_t count;
		size_t stride;
	};

	// The siren settings now, on any thread once StartFeatures has run, through the carcols pointer of I-E-T4 - nothing while
	// the game has none loaded or they cannot be read this moment, and nothing until this module has looked for its sites:
	// with LARGE_SIREN_IDS=1 on every boot its part is on, and with DIAGNOSTIC_MODE=1 while the part or the key is off. I-E-T4
	// found is all it takes, whatever the other sites hold - but the part looks for its patterns in order and stops at the
	// first missing, so one missing before I-E-T4 leaves nothing here.
	std::optional<SirenSettingsArray> SirenSettingsNow();

	// With LARGE_SIREN_IDS=0 or the part off: finds and checks every site of the limit, logs each one and starts the
	// measurement; writes nothing to the game. With the part at work it does nothing: InitSirenIds measures then. Started
	// once by StartFeatures, with DIAGNOSTIC_MODE=1 only (kMeasuredFirst, TweaksVFeatures.cpp).
	void MeasureSirenIdsWhileOff();

	// The SirenSettings part's siren setting IDs (SPECIFICATIONS 11.14), started by its entry of the feature table, from
	// DllMain, while [SIRENSETTINGS] is on: LARGE_SIREN_IDS=1 widens them, every site checked first, all of it one write.
	void InitSirenIds();
}
