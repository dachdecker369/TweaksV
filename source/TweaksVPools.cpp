// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the game's pool size lookup (see TweaksVPools.h).
//
// The game makes each of its pools with the size one function gives, the pool size lookup at RVA 0x149060 on 1.0.1158.16:
// each pool's maker loads the game's configuration, the pool's name hash and its default and calls it (PATTERNS.md, "Weapon
// components" and "StaticBounds"). TweaksV knows one such call from its own findings in the game's code, the weapon
// components' (W-E-C4), and takes the lookup from there. W-E-C4 is looked for once a session and the outcome kept: the
// weapon components redirect this very call, and StaticBounds, which comes after them in the order of the parts, looks for
// the static bounds pool's call of the same lookup - so both get the same, the lookup the game's own and not the call stub
// the weapon components put there, whichever part asks first and whichever of them is on.
// Written for TweaksV from its own findings in the game's code; no code of another project is used.

#include "TweaksVPools.h"

#include <cstddef>
#include <cstring>

#include "TweaksVConflicts.h"
#include "TweaksVPatch.h"
#include "TweaksVScanner.h"

namespace TweaksV
{
	namespace
	{
		// W-E-C4's sites, counted from its match
		constexpr size_t kConfigurationAt = 3;		// the disp32 of "mov rcx, [rip+disp32]": the configuration's pointer
		constexpr size_t kConfigurationEnd = 7;		// where that instruction ends, which its disp32 counts from
		constexpr size_t kPoolAt = 8;				// the imm32 of "mov edx, imm32": the pool's name
		constexpr size_t kDefaultAt = 14;			// the imm32 of "mov r8d, imm32": the pool's default size
		constexpr size_t kCallAt = 18;				// "call rel32": the call of the lookup
		constexpr size_t kCallSize = 5;
		constexpr uint32_t kPoolName = 0xBAE33CCF;	// the game's hash of the pool's name, "cweaponcomponentinfo"

		// The imm32 a pattern holds at index at, or -1 when a byte of it is a wildcard - for the checks below, made when
		// TweaksV is compiled, that each offset above reads the instruction its comment names.
		consteval int64_t PatternImm32(const GamePattern& pattern, size_t at)
		{
			int64_t value = 0;
			for (size_t i = 0; i < 4; i++)
			{
				const int byte = PatternByte(pattern, at + i);
				if (byte < 0)
					return -1;
				value |= static_cast<int64_t>(byte) << (8 * i);
			}
			return value;
		}
		static_assert(PatternByte(kPoolSizeCallPattern, kConfigurationAt - 3) == 0x48 && PatternByte(kPoolSizeCallPattern, kConfigurationAt - 2) == 0x8B &&
			PatternByte(kPoolSizeCallPattern, kConfigurationAt - 1) == 0x0D && kConfigurationEnd == kConfigurationAt + 4 &&
			PatternImm32(kPoolSizeCallPattern, kConfigurationAt) == -1, "TweaksV: W-E-C4 holds the configuration's mov rcx, [rip+disp32] at +0, its disp32 read");
		static_assert(PatternByte(kPoolSizeCallPattern, kPoolAt - 1) == 0xBA && PatternImm32(kPoolSizeCallPattern, kPoolAt) == kPoolName,
			"TweaksV: W-E-C4 holds the pool's name, mov edx, 0xBAE33CCF, at +7");
		static_assert(PatternByte(kPoolSizeCallPattern, kDefaultAt - 2) == 0x41 && PatternByte(kPoolSizeCallPattern, kDefaultAt - 1) == 0xB8 &&
			PatternImm32(kPoolSizeCallPattern, kDefaultAt) == -1, "TweaksV: W-E-C4 holds the pool's default, mov r8d, imm32, at +12, the imm32 read");
		static_assert(PatternByte(kPoolSizeCallPattern, kCallAt) == 0xE8 && PatternImm32(kPoolSizeCallPattern, kCallAt + 1) == -1,
			"TweaksV: W-E-C4 holds the lookup's call, call rel32, at +18, its rel32 read");

		// The outcome, kept for the session - DllMain's thread alone writes and reads it.
		enum class State { NotTried, Found, Failed };
		State g_state = State::NotTried;
		PoolSizeCall g_found;
		std::string g_reason;	// why the lookup is not known
	}

	bool FindPoolSizeCall(PoolSizeCall& found, std::string& reason)
	{
		if (g_state == State::NotTried)
		{
			// the outcome is kept only once it is whole: an exception on the way leaves the next part to try again
			PoolSizeCall call;
			std::string why;
			uint8_t* const match = Scanner::Find(kPoolSizeCallPattern);
			if (!match)
				why = std::string(kPoolSizeCallPattern.label) + " was not found.";
			else
			{
				// every site lies inside the match, which the scan has read
				call.call = match + kCallAt;
				memcpy(&call.pool, match + kPoolAt, sizeof(call.pool));
				call.defaultSize = match + kDefaultAt;
				call.configuration = Rel32Target(match + kConfigurationAt);
				call.target = IsReadable(call.call, kCallSize) ? CallTarget(call.call) : nullptr;
				if (call.target && IsInsideGameImage(call.target))
					call.lookup = call.target;
				else
					why = "the weapon components' pool size call at " + DescribeAddress(call.call) + " is not a call into the game (another mod may "
						"have hooked it already) (" + kPoolSizeCallPattern.label + ").";
			}
			g_found = call;
			g_reason = why;
			g_state = call.lookup ? State::Found : State::Failed;
		}
		found = g_found;
		if (g_state != State::Found)
			reason = g_reason;
		return g_state == State::Found;
	}
}
