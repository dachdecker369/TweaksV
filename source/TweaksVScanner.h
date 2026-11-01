// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the scanner: finds TweaksV's patterns (TweaksVPatterns.h) in the running game's code, every
// load of one address, every call of one function and every store to a range of memory. The pattern parser is an altered version of ScriptHookVDotNet's memory
// scanner (crosire, kagikn and contributors; the C++ port by Chiheb-Bacha), zlib license - the notice
// is in TweaksVScanner.cpp and THIRD_PARTY_NOTICES.md. The search itself is TweaksV's own.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "TweaksVPatterns.h"

namespace TweaksV
{
	namespace Scanner
	{
		// The one match of a pattern in GTA5_Enhanced.exe's code, or nullptr: a miss is logged with the
		// pattern's label, and so is a pattern that matches twice or more, which is not used - the site it
		// is meant for must be unique.
		uint8_t* Find(const GamePattern& pattern);

		// Find, for a part that looks for all its patterns and refuses its job with the first reason: nullptr, with
		// NotAsOften's words in why when why is still empty, unless the pattern matches exactly once.
		uint8_t* FindOnce(const GamePattern& pattern, std::string& why);

		// Why a search found a pattern other than as often as it must: "A-E-T1 was not found", "the game code holds A-E-T1 2 or
		// more times, not the expected 1" - "or more" for a search that stopped at the match it asked for over expected, "1
		// time, not the expected 2" for one that found too few. No point: the part's sentence ends with its own.
		std::string NotAsOften(const GamePattern& pattern, size_t found, size_t expected);

		// Every match of a pattern, in address order, at most limit of them (a site the game must have
		// exactly n times is asked for n + 1 - the decals part's "add decal" check). A miss is logged like
		// Find's; DIAGNOSTIC_MODE logs the matches as it does for Find.
		std::vector<uint8_t*> FindAll(const GamePattern& pattern, size_t limit);

		// Every "lea r64, [rip+disp32]" (a REX.W prefix, opcode 8D, ModRM mod 00 r/m 101: 7 bytes) in the
		// game's code that loads target, in address order, at most limit of them. For a static array the
		// game loads in places the bytes around them cannot tell apart (the relocation core): one unique
		// site gives the address, this finds every load of it. Nothing is logged.
		std::vector<uint8_t*> FindRipRelativeLeas(const void* target, size_t limit);

		// The same for every "lea r64, [rip+disp32]" and every "mov [rip+disp32], r64" (opcode 8D or 89, the
		// same prefix and ModRM): for an array whose patterns name its references, the relocation core checks
		// that they are all the game has. Nothing is logged.
		std::vector<uint8_t*> FindRipRelativeReferences(const void* target, size_t limit);

		// Every "call rel32" (E8 and a rel32: 5 bytes) in the game's code that leads to target, in address order, at
		// most limit of them. For a function whose callers a part must know all of: the PedProps part checks
		// that the game calls its per-anchor count and its metadata lookup exactly as often as the research found,
		// so that no caller a game update adds is left out. Nothing is logged.
		std::vector<uint8_t*> FindCallsTo(const void* target, size_t limit);

		// Every instruction in the game's code that writes to memory at [rip+disp32] (a ModRM of mod 00 and r/m 101) and so
		// to any of the size bytes from target, in address order, at most limit of them, each given by its first byte, its
		// prefixes counted: a mov of 1 to 8 bytes from a register or an immediate; a read-modify-write of the integer unit -
		// add, or, adc, sbb, and, sub, xor, inc, dec, not, neg, the shifts and rotations, xchg, xadd, cmpxchg, setcc, bts,
		// btr, btc, pop; and an SSE or AVX move or extract of 1 to 32 bytes. The bytes before an operand are read every way
		// an instruction could start among them - any of them may end the instruction before - and the operand counts when
		// one reading is such a store: one counted by mistake is safe where a missed writer is not. For data the game writes
		// once that a part changes after it: a writer the research did not find would undo the change later (the texture
		// budget's check that the start-up's two stores are the budgets' only writers). Nothing is logged.
		std::vector<uint8_t*> FindRipRelativeStores(const void* target, size_t size, size_t limit);
	}
}
