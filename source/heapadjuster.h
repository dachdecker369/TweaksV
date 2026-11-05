// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the heap adjuster: the game's memory heap, which HEAP_SIZE in [HEAP_ADJUSTER] enlarges, and the budget of the
// allocator the game's pools draw on, which POOL_MEMORY raises.
#pragma once

#include <cstdint>
#include <string>

namespace TweaksV
{
	// Sets the heap size and the pool memory budget, as [HEAP_ADJUSTER] asks. Called once by DllMain.
	void InitHeapAdjuster();

	// The pool memory budget's imm32 in the game's start-up code, or nullptr with why set unless it holds a reason already: for
	// the ped pool's measurement, since the code looks for a pattern in one place, H-E2 here, which works with the part off too.
	// Called on the thread of the start-up.
	uint8_t* FindPoolMemoryBudget(std::string& why);
}
