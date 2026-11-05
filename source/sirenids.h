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
#include <string>

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
	// found is all it takes, whatever the sites after it hold - but the part goes through its patterns in order and stops at
	// the first missing or not as the findings have it, so one before I-E-T4 leaves nothing here.
	std::optional<SirenSettingsArray> SirenSettingsNow();

	// What the game's getter of a model's siren setting reads (I-E-M1): the model's position in the array, a byte, 0 for none, and in the
	// carcols data the array and its count, a word, a setting this many bytes.
	inline constexpr size_t kModelSirenPositionAt = 0x53B;
	inline constexpr size_t kCarcolsSirenSettingsAt = 0x48;
	inline constexpr size_t kCarcolsSirenCountAt = 0x50;
	inline constexpr size_t kSirenSettingSize = 0x5E8;

	// What the sirens per vehicle take from this module's sites: the constructor of a siren setting, the two tables of
	// their parser registration, each entry a member's, and the getter with the pointer it reads.
	struct SirenSettingSites
	{
		const uint8_t* constructor;		// I-E-C1's match
		const uint8_t* memberTable;		// I-E-P1's: the members' data, a pointer each
		const uint8_t* offsetTable;		// and their offsets in a siren setting, a dword each
		const uint8_t* getter;			// I-E-M1's match
		const uint8_t* carcolsPointer;	// the game's pointer to its carcols data, as the getter reads it
	};

	// Those sites, for the module that runs right after InitSirenIds: the matches it kept, since its writes change what I-E-C1
	// matches, or, with the IDs left alone, a search of their own, kept too. Nothing without all three; why then gets the reason in
	// Scanner::NotAsOften's words unless it holds one already, and the carcols pointer inside the game and readable is what it checks.
	std::optional<SirenSettingSites> FindSirenSettingSites(std::string& why);

	// With LARGE_SIREN_IDS=0 or the part off: finds and checks every site of the limit, logs each one and starts the
	// measurement; writes nothing to the game. With the part at work it does nothing: InitSirenIds measures then. Started
	// once by StartFeatures, with DIAGNOSTIC_MODE=1 only (kMeasuredFirst, features.cpp).
	void MeasureSirenIdsWhileOff();

	// The SirenSettings part's siren setting IDs (SPECIFICATIONS 11.14), started by its entry of the feature table, from
	// DllMain, while [SIRENSETTINGS] is on: LARGE_SIREN_IDS=1 widens them, every site checked first, all of it one write.
	void InitSirenIds();
}
