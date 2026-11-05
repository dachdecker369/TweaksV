// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the corona store, the second half of the LightCapacity part (D35). The game's store holds 1088 coronas a frame and drops
// the rest without a word, the vehicles' first, since they are added last. CORONA_STORE above that moves the store to memory of the
// part's own, its adds, its copy to the draw list and the LOD lights' job with it, in the part's one write.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "patch.h"

namespace TweaksV
{
	inline constexpr uint32_t kGameCoronas = 1088;	// the coronas the game's store holds

	// The store's half of the part's one write, called by InitLightCapacity once the part's own sites are checked and before it writes:
	// with CORONA_STORE above the game's it finds and checks its sites and adds what it would write to writes, writing nothing; at the
	// game's own it looks for nothing, adds nothing and returns true. The two sites are G-E-G1's matches, the part's, a pattern being used
	// once. False, why saying what stopped it in a sentence of its own, and the part refuses as a whole - "The light stores are not raised
	// this session: <why>" - since the LOD lights' job must move with the store or the game faults, and with the part the sirens stay at 20.
	bool FindCoronaStore(uint8_t* firstInlinedAdd, uint8_t* secondInlinedAdd, std::vector<CodeWrite>& writes, std::string& why);

	// Gives back the memory FindCoronaStore took: the part calls it when, that having said yes, its one write will not come or has failed, so
	// that nothing leads to that memory. It holds none on a dry run, at the game's own or after a false.
	void DiscardCoronaStore();

	// What the store's line of the part's measurement says, on any thread.
	struct CoronaStoreNumbers
	{
		uint32_t room;			// the coronas it holds this session: the game's own until the part's one write is done
		bool counted;			// the counters below run: the raise written, and DIAGNOSTIC_MODE=1
		uint64_t refused;		// the coronas it refused so far
		uint64_t lodRefused;	// the coronas the LOD lights' job refused by its own 1088
		uint32_t lodBrought;	// the coronas of the LOD lights' last batch
	};
	CoronaStoreNumbers CoronaStoreNow();
}
