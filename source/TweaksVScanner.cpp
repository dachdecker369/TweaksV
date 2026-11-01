// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the scanner (see TweaksVScanner.h).
//
// The pattern parser, ParsePattern below, is an altered version of ScriptHookVDotNet's memory scanner:
//   Copyright (C) 2015 crosire, kagikn
//   Copyright (C) 2025 Chiheb-Bacha - the C++ port, which came to TweaksV through his plugins
//   License: zlib - https://github.com/scripthookvdotnet/scripthookvdotnet#license (the text is in
//   THIRD_PARTY_NOTICES.md)
//   Source: https://github.com/Chiheb-Bacha/ScriptHookVDotNetEnhanced/blob/main/source/core/MemScanner.cs
// Altered for TweaksV: a malformed token is refused instead of throwing inside DllMain - "??" among
// them, so that one ? is the only wildcard, as the pattern table has it - and a miss logs and returns
// nullptr instead of ending the game.
//
// The search is TweaksV's own. The original's Boyer-Moore-Horspool search went through the whole
// image almost byte by byte (a wildcard near the end of a pattern limits its skips), about 0.6 s at
// every game start. This one searches only the game's executable sections, and only the parts that
// can be read, so a page the game protects is skipped instead of crashing the game. It compares a
// pattern's two rarest bytes at 16 positions at a time (SSE2) and checks the whole pattern only where
// both match - about ten times faster.

#include "TweaksVScanner.h"

#include <windows.h>
#include <psapi.h>
#include <emmintrin.h>
#include <intrin.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "TweaksVLog.h"
#include "TweaksVSettings.h"

namespace TweaksV
{
	namespace Scanner
	{
		namespace
		{
			constexpr const char* kTag = "Scanner";

			struct Range
			{
				uint8_t* begin;
				uint8_t* end;
			};

			// GTA5_Enhanced.exe in memory and its executable sections, read once. If the headers show no
			// executable section, the whole image is searched.
			struct Image
			{
				uint8_t* base = nullptr;
				uint64_t size = 0;
				std::vector<Range> code;
			};

			const Image& GameImage()
			{
				static const Image image = []
				{
					Image result;
					const HMODULE module = GetModuleHandleW(nullptr);
					MODULEINFO info{};
					if (!module || !GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)))
						return result;
					const auto base = reinterpret_cast<uint8_t*>(info.lpBaseOfDll);
					result.base = base;
					result.size = info.SizeOfImage;

					// the headers are read only where they lie inside the image
					const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
					const bool dosOk = result.size >= sizeof(IMAGE_DOS_HEADER) && dos->e_magic == IMAGE_DOS_SIGNATURE &&
						dos->e_lfanew > 0 && static_cast<uint64_t>(dos->e_lfanew) + sizeof(IMAGE_NT_HEADERS) <= result.size;
					const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + (dosOk ? dos->e_lfanew : 0));
					const IMAGE_SECTION_HEADER* section = dosOk ? IMAGE_FIRST_SECTION(nt) : nullptr;
					if (dosOk && nt->Signature == IMAGE_NT_SIGNATURE &&
						reinterpret_cast<const uint8_t*>(section + nt->FileHeader.NumberOfSections) <= base + result.size)
					{
						for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
						{
							if (!(section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE))
								continue;
							const uint64_t start = section[i].VirtualAddress;
							const uint64_t length = section[i].Misc.VirtualSize ? section[i].Misc.VirtualSize : section[i].SizeOfRawData;
							if (start >= result.size || length == 0)
								continue;
							result.code.push_back({ base + start, base + std::min<uint64_t>(start + length, result.size) });
						}
					}
					if (result.code.empty())
						result.code.push_back({ base, base + result.size });
					std::sort(result.code.begin(), result.code.end(), [](const Range& a, const Range& b) { return a.begin < b.begin; });
					return result;
				}();
				return image;
			}

			// The readable parts of the code, neighbouring readable regions joined. Read for every search:
			// TweaksV itself changes protections between two searches.
			std::vector<Range> ReadableCode()
			{
				constexpr DWORD kReadable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
					PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
				std::vector<Range> runs;
				for (const Range& range : GameImage().code)
				{
					for (uint8_t* at = range.begin; at < range.end;)
					{
						MEMORY_BASIC_INFORMATION info{};
						if (VirtualQuery(at, &info, sizeof(info)) != sizeof(info))
							break;
						uint8_t* next = static_cast<uint8_t*>(info.BaseAddress) + info.RegionSize;
						if (next <= at)
							break;
						if (next > range.end)
							next = range.end;
						if (info.State == MEM_COMMIT && (info.Protect & kReadable) && !(info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
						{
							if (!runs.empty() && runs.back().end == at)
								runs.back().end = next;
							else
								runs.push_back({ at, next });
						}
						at = next;
					}
				}
				return runs;
			}

			// How often each byte occurs in the first 256 KB of the game's code. The search compares a
			// pattern's two rarest bytes first; this only changes how fast it is, never what it finds.
			const std::array<uint32_t, 256>& ByteCounts()
			{
				static const std::array<uint32_t, 256> counts = []
				{
					std::array<uint32_t, 256> result{};
					const std::vector<Range> code = ReadableCode();
					if (!code.empty())
					{
						const size_t n = std::min<size_t>(static_cast<size_t>(code[0].end - code[0].begin), 256 * 1024);
						for (size_t i = 0; i < n; i++)
							result[code[0].begin[i]]++;
					}
					return result;
				}();
				return counts;
			}

			// The parser (the altered ScriptHookVDotNet code): "48 8B ? 05" into bytes and a mask, 'x' for a
			// byte that must match and '?' for any. False on a token that is not two hex digits or ?, so
			// that nothing throws inside DllMain - the rule IsPatternText checks the table by when TweaksV is
			// compiled (TweaksVPatterns.h).
			bool ParsePattern(const std::string& pat, std::vector<uint8_t>& outPattern, std::string& outMask)
			{
				outPattern.clear();
				outMask.clear();

				std::istringstream ss(pat);
				std::string token;

				while (ss >> token)
				{
					if (token == "?")
					{
						outPattern.push_back(0);
						outMask.push_back('?');
					}
					else
					{
						// exactly two hex digits; anything else is malformed, and nothing here can throw
						const auto hex = [](char c) -> int
						{
							if (c >= '0' && c <= '9') return c - '0';
							if (c >= 'a' && c <= 'f') return c - 'a' + 10;
							if (c >= 'A' && c <= 'F') return c - 'A' + 10;
							return -1;
						};
						if (token.size() != 2 || hex(token[0]) < 0 || hex(token[1]) < 0)
							return false;
						outPattern.push_back(static_cast<uint8_t>(hex(token[0]) * 16 + hex(token[1])));
						outMask.push_back('x');
					}
				}
				return true;
			}

			// A parsed pattern and the positions of its two rarest fixed bytes.
			struct Compiled
			{
				std::vector<uint8_t> bytes;
				std::vector<uint8_t> fixed;	// 1: the byte must match; 0: a wildcard
				size_t first = 0;
				size_t second = 0;
			};

			// False for a malformed pattern and for one that is only wildcards (it would match everywhere).
			bool Compile(const std::string& text, Compiled& pattern)
			{
				std::string mask;
				if (!ParsePattern(text, pattern.bytes, mask))
					return false;
				std::vector<size_t> fixedAt;
				pattern.fixed.assign(pattern.bytes.size(), 0);
				for (size_t i = 0; i < pattern.bytes.size(); i++)
				{
					if (mask[i] != 'x')
						continue;
					pattern.fixed[i] = 1;
					fixedAt.push_back(i);
				}
				if (fixedAt.empty())
					return false;
				const auto& counts = ByteCounts();
				std::stable_sort(fixedAt.begin(), fixedAt.end(),
					[&](size_t a, size_t b) { return counts[pattern.bytes[a]] < counts[pattern.bytes[b]]; });
				pattern.first = fixedAt[0];
				pattern.second = fixedAt.size() > 1 ? fixedAt[1] : fixedAt[0];
				return true;
			}

			bool Matches(const uint8_t* at, const Compiled& pattern)
			{
				for (size_t i = 0; i < pattern.bytes.size(); i++)
					if (pattern.fixed[i] && at[i] != pattern.bytes[i])
						return false;
				return true;
			}

			// The first match that starts in [begin, end) and ends inside it. Every load stays below end.
			uint8_t* FindIn(const Compiled& pattern, uint8_t* begin, uint8_t* end)
			{
				const size_t length = pattern.bytes.size();
				const size_t available = static_cast<size_t>(end - begin);
				if (available < length)
					return nullptr;
				const size_t lastStart = available - length;
				const __m128i a = _mm_set1_epi8(static_cast<char>(pattern.bytes[pattern.first]));
				const __m128i b = _mm_set1_epi8(static_cast<char>(pattern.bytes[pattern.second]));
				size_t at = 0;
				for (; at + 15 <= lastStart; at += 16)
				{
					const __m128i x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(begin + at + pattern.first));
					const __m128i y = _mm_loadu_si128(reinterpret_cast<const __m128i*>(begin + at + pattern.second));
					unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi8(x, a), _mm_cmpeq_epi8(y, b))));
					while (mask)
					{
						unsigned long bit = 0;
						_BitScanForward(&bit, mask);
						if (Matches(begin + at + bit, pattern))
							return begin + at + bit;
						mask &= mask - 1;
					}
				}
				for (; at <= lastStart; at++)
					if (Matches(begin + at, pattern))
						return begin + at;
				return nullptr;
			}

			// The first match at or after from (nullptr: from the start) in the readable code.
			uint8_t* FindInCode(const Compiled& pattern, const std::vector<Range>& code, uint8_t* from)
			{
				for (const Range& range : code)
				{
					if (from && range.end <= from)
						continue;
					uint8_t* const begin = from && from > range.begin ? from : range.begin;
					if (uint8_t* found = FindIn(pattern, begin, range.end))
						return found;
				}
				return nullptr;
			}

			// DIAGNOSTIC_MODE: the RVA, the number of matches and the bytes found (a pattern that matches
			// more than once should be made unique). A pattern that matches more than once also gets one
			// line per match with the bytes around it, so that a longer pattern can be chosen that matches
			// only the place it is meant for.
			void LogPatternDiagnostics(const GamePattern& gamePattern, const Compiled& pattern, const std::vector<Range>& code,
				uint8_t* address)
			{
				const int maxCount = 100;
				std::vector<uint8_t*> found;	// every match, for the lines per match below
				for (uint8_t* cur = address; cur && (int)found.size() < maxCount; cur = FindInCode(pattern, code, cur + 1))
					found.push_back(cur);
				const int matches = (int)found.size();

				const Image& image = GameImage();
				auto rva = [&image](const uint8_t* at)	// 0 outside the game
				{
					return at >= image.base && at < image.base + image.size ? (unsigned long long)(at - image.base) : 0ULL;
				};
				char text[96];
				snprintf(text, sizeof(text), " -> RVA 0x%llX, %d%s match%s, bytes:",
					rva(address), matches, matches >= maxCount ? "+" : "", matches == 1 ? "" : "es");

				std::string line = std::string("Pattern ") + gamePattern.label + " found: " + gamePattern.text + text;
				for (size_t i = 0; i < pattern.bytes.size() && i < 32; i++)
				{
					char byteText[4];
					snprintf(byteText, sizeof(byteText), " %02X", address[i]);
					line += byteText;
				}
				WriteLog(kTag, line);

				if (matches < 2)
					return;
				// the first 10 matches: 16 bytes before each and 48 from its start, "|" marks the start; never
				// outside the readable part of the code the match is in
				for (int m = 0; m < matches && m < 10; m++)
				{
					const uint8_t* match = found[m];
					const Range* in = nullptr;
					for (const Range& range : code)
						if (match >= range.begin && match < range.end)
							in = &range;
					if (!in)
						continue;
					const uint8_t* from = match - in->begin >= 16 ? match - 16 : in->begin;
					const uint8_t* to = in->end - match >= 48 ? match + 48 : in->end;
					snprintf(text, sizeof(text), "  %s match %d of %d%s at RVA 0x%llX:", gamePattern.label, m + 1, matches,
						matches >= maxCount ? "+" : "", rva(match));
					line = text;
					for (const uint8_t* at = from; at < to; at++)
					{
						char byteText[6];
						snprintf(byteText, sizeof(byteText), at == match ? " | %02X" : " %02X", *at);
						line += byteText;
					}
					WriteLog(kTag, line);
				}
			}

#ifdef TWEAKSV_TEST_BUILD
			// Test builds only (-p:TweaksVTestBuild=true): TweaksV.ini [TWEAKSV] TEST_BREAK_PATTERN names one
			// pattern by its label in PATTERNS.md (e.g. S-E2), which is then treated as not found - to see
			// that its part is skipped cleanly and the rest still works.
			bool IsBrokenForTest(const GamePattern& pattern)
			{
				const std::string& broken = GetText(kTestBreakPattern);
				return !broken.empty() && _stricmp(broken.c_str(), pattern.label) == 0;
			}
#endif
		}

		uint8_t* Find(const GamePattern& gamePattern)
		{
			const std::vector<uint8_t*> found = FindAll(gamePattern, 2);
			if (found.size() > 1)	// what would be written over cannot be known
			{
				WriteLog(kTag, std::string("Pattern ") + gamePattern.label + " matches 2 or more times - not used (the site must be unique).");
				return nullptr;
			}
			return found.empty() ? nullptr : found[0];
		}

		// The search behind Find, carried on from each match until limit matches are found.
		std::vector<uint8_t*> FindAll(const GamePattern& gamePattern, size_t limit)
		{
			std::vector<uint8_t*> found;
			const std::string label = gamePattern.label;
#ifdef TWEAKSV_TEST_BUILD
			if (IsBrokenForTest(gamePattern))
			{
				WriteLog(kTag, "TEST BUILD: pattern " + label + " is treated as not found (TEST_BREAK_PATTERN).");
				return found;
			}
#endif
			Compiled pattern;
			if (!Compile(gamePattern.text, pattern))	// not for a pattern of kPatterns, which are checked when TweaksV is compiled
			{
				WriteLog(kTag, "Pattern " + label + " is not valid (treated as not found): " + gamePattern.text);
				return found;
			}

			const std::vector<Range> code = ReadableCode();
			for (uint8_t* at = FindInCode(pattern, code, nullptr); at && found.size() < limit; at = FindInCode(pattern, code, at + 1))
				found.push_back(at);
			if (found.empty())
			{
				WriteLog(kTag, "Pattern " + label + " not found: " + gamePattern.text);
				return found;
			}

			if (IsDiagnosticMode())
				LogPatternDiagnostics(gamePattern, pattern, code, found[0]);

			return found;
		}

		namespace
		{
			// Every "<REX.W> <opcode> <ModRM mod 00 r/m 101> disp32" whose displacement leads to target, the
			// opcode one of the two given, looked for the way Find looks for a pattern: the opcode and the
			// ModRM at 16 positions at a time, the prefix and the displacement only where both match.
			std::vector<uint8_t*> FindRipRelative(const void* target, size_t limit, uint8_t opcodeA, uint8_t opcodeB)
			{
				constexpr size_t kLength = 7;
				std::vector<uint8_t*> found;
				const auto wanted = reinterpret_cast<intptr_t>(target);
				const auto loads = [&found, wanted](uint8_t* at)	// at[1] is the opcode, at[2] a ModRM 00 ... 101
				{
					if ((at[0] & 0xF8) != 0x48)	// a REX prefix with W
						return;
					int32_t disp = 0;
					memcpy(&disp, at + 3, sizeof(disp));
					if (reinterpret_cast<intptr_t>(at + kLength) + disp == wanted)
						found.push_back(at);
				};
				const __m128i a = _mm_set1_epi8(static_cast<char>(opcodeA));
				const __m128i b = _mm_set1_epi8(static_cast<char>(opcodeB));
				const __m128i modrmBits = _mm_set1_epi8(static_cast<char>(0xC7));
				const __m128i modrm = _mm_set1_epi8(0x05);
				for (const Range& range : ReadableCode())
				{
					const size_t available = static_cast<size_t>(range.end - range.begin);
					if (available < kLength)
						continue;
					const size_t lastStart = available - kLength;	// the whole instruction stays inside the range
					size_t at = 0;
					for (; at + 15 <= lastStart && found.size() < limit; at += 16)
					{
						const __m128i x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(range.begin + at + 1));
						const __m128i y = _mm_loadu_si128(reinterpret_cast<const __m128i*>(range.begin + at + 2));
						const __m128i opcode = _mm_or_si128(_mm_cmpeq_epi8(x, a), _mm_cmpeq_epi8(x, b));
						unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_and_si128(opcode,
							_mm_cmpeq_epi8(_mm_and_si128(y, modrmBits), modrm))));
						while (mask && found.size() < limit)
						{
							unsigned long bit = 0;
							_BitScanForward(&bit, mask);
							loads(range.begin + at + bit);
							mask &= mask - 1;
						}
					}
					for (; at <= lastStart && found.size() < limit; at++)
						if ((range.begin[at + 1] == opcodeA || range.begin[at + 1] == opcodeB) && (range.begin[at + 2] & 0xC7) == 0x05)
							loads(range.begin + at);
				}
				return found;
			}
		}

		std::vector<uint8_t*> FindRipRelativeLeas(const void* target, size_t limit)
		{
			return FindRipRelative(target, limit, 0x8D, 0x8D);
		}

		std::vector<uint8_t*> FindRipRelativeReferences(const void* target, size_t limit)
		{
			return FindRipRelative(target, limit, 0x8D, 0x89);	// lea r64, [rip+disp32]; mov [rip+disp32], r64
		}

		// Every "E8 rel32" whose rel32 leads to target, looked for the way FindRipRelative looks for its loads: two
		// bytes at 16 positions at a time - the opcode, and the rel32's lowest byte, which a call at p to target must
		// hold as the lowest byte of target - (p + 5); over 16 positions in a row those bytes count down by one each -
		// and the whole rel32 only where both match.
		std::vector<uint8_t*> FindCallsTo(const void* target, size_t limit)
		{
			constexpr size_t kLength = 5;
			std::vector<uint8_t*> found;
			const auto wanted = reinterpret_cast<intptr_t>(target);
			const auto calls = [&found, wanted](uint8_t* at)	// at[0] is E8
			{
				int32_t rel = 0;
				memcpy(&rel, at + 1, sizeof(rel));
				if (reinterpret_cast<intptr_t>(at + kLength) + rel == wanted)
					found.push_back(at);
			};
			const __m128i opcode = _mm_set1_epi8(static_cast<char>(0xE8));
			const __m128i steps = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
			for (const Range& range : ReadableCode())
			{
				const size_t available = static_cast<size_t>(range.end - range.begin);
				if (available < kLength)
					continue;
				const size_t lastStart = available - kLength;	// the whole call stays inside the range
				size_t at = 0;
				for (; at + 15 <= lastStart && found.size() < limit; at += 16)
				{
					uint8_t* const begin = range.begin + at;
					const auto lowest = static_cast<char>(wanted - reinterpret_cast<intptr_t>(begin + kLength));
					const __m128i lows = _mm_sub_epi8(_mm_set1_epi8(lowest), steps);	// the rel32's lowest byte at begin + i
					const __m128i x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(begin));
					const __m128i y = _mm_loadu_si128(reinterpret_cast<const __m128i*>(begin + 1));
					unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_and_si128(_mm_cmpeq_epi8(x, opcode), _mm_cmpeq_epi8(y, lows))));
					while (mask && found.size() < limit)
					{
						unsigned long bit = 0;
						_BitScanForward(&bit, mask);
						calls(begin + bit);
						mask &= mask - 1;
					}
				}
				for (; at <= lastStart && found.size() < limit; at++)
					if (range.begin[at] == 0xE8)
						calls(range.begin + at);
			}
			return found;
		}

		namespace
		{
			// What an instruction writes through its memory operand: the most bytes it can write - 0 for one that writes
			// nothing there - and the size of the immediate that follows its disp32, which the rip it counts from lies past;
			// altImmediate, when not 0, the size the immediate has if the 66 prefix in front of it is the instruction's own.
			struct StoreShape
			{
				uint32_t width = 0;
				uint32_t immediate = 0;
				uint32_t altImmediate = 0;
			};

			// The prefixes in front of an opcode: a REX right before it, the legacy prefixes before that, four at most. A
			// prefix that narrows a store - 66, F2, F3 - is not counted for its width, since the byte may belong to the
			// instruction before: a store is taken for at most as wide as it can be, never narrower.
			struct Prefixes
			{
				const uint8_t* start;
				bool rexW = false;
				bool operand16 = false;	// 66
				bool repz = false;		// F3
				bool repnz = false;		// F2
			};

			Prefixes ReadPrefixes(const uint8_t* opcode, const uint8_t* begin)
			{
				Prefixes prefixes{ opcode };
				if (prefixes.start > begin && (prefixes.start[-1] & 0xF0) == 0x40)
				{
					prefixes.rexW = (prefixes.start[-1] & 0x08) != 0;
					prefixes.start--;
				}
				for (int i = 0; i < 4 && prefixes.start > begin; i++, prefixes.start--)
				{
					const uint8_t byte = prefixes.start[-1];
					if (byte == 0x66)
						prefixes.operand16 = true;
					else if (byte == 0xF3)
						prefixes.repz = true;
					else if (byte == 0xF2)
						prefixes.repnz = true;
					else if (byte != 0xF0 && byte != 0x2E && byte != 0x3E && byte != 0x26 && byte != 0x36 && byte != 0x64 && byte != 0x65)
						break;	// lock and the segments count; anything else belongs to the instruction before
				}
				return prefixes;
			}

			// The one-byte opcodes; reg is the ModRM's reg field, which picks the job in a group.
			StoreShape OneByteStore(uint8_t opcode, int reg, const Prefixes& prefixes)
			{
				const uint32_t full = prefixes.rexW ? 8 : 4;
				const uint32_t immediate16 = prefixes.operand16 ? 2 : 0;
				switch (opcode)
				{
				case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:	// add, or, adc, sbb, and, sub, xor r/m8, r8
				case 0x86: case 0x88:															// xchg, mov r/m8, r8
					return { 1, 0 };
				case 0x01: case 0x09: case 0x11: case 0x19: case 0x21: case 0x29: case 0x31:	// the same of r/m, r
				case 0x87: case 0x89:
					return { full, 0 };
				case 0x8F: return reg == 0 ? StoreShape{ 8, 0 } : StoreShape{};						// pop r/m64
				case 0xC6: return reg == 0 ? StoreShape{ 1, 1 } : StoreShape{};						// mov r/m8, imm8
				case 0xC7: return reg == 0 ? StoreShape{ full, 4, immediate16 } : StoreShape{};		// mov r/m, imm32 (imm16)
				case 0x80: return reg != 7 ? StoreShape{ 1, 1 } : StoreShape{};						// add .. xor r/m8, imm8; /7 compares
				case 0x81: return reg != 7 ? StoreShape{ full, 4, immediate16 } : StoreShape{};
				case 0x83: return reg != 7 ? StoreShape{ full, 1 } : StoreShape{};
				case 0xC0: return { 1, 1 };																// the rotations and shifts by imm8
				case 0xC1: return { full, 1 };
				case 0xD0: case 0xD2: return { 1, 0 };													// by 1, by cl
				case 0xD1: case 0xD3: return { full, 0 };
				case 0xF6: return reg == 2 || reg == 3 ? StoreShape{ 1, 0 } : StoreShape{};				// not, neg r/m8
				case 0xF7: return reg == 2 || reg == 3 ? StoreShape{ full, 0 } : StoreShape{};
				case 0xFE: return reg <= 1 ? StoreShape{ 1, 0 } : StoreShape{};						// inc, dec r/m8
				case 0xFF: return reg <= 1 ? StoreShape{ full, 0 } : StoreShape{};
				default: return {};
				}
			}

			// The opcodes after 0F.
			StoreShape TwoByteStore(uint8_t opcode, int reg, const Prefixes& prefixes)
			{
				const uint32_t full = prefixes.rexW ? 8 : 4;
				if (opcode >= 0x90 && opcode <= 0x9F)	// setcc r/m8
					return { 1, 0 };
				switch (opcode)
				{
				case 0x11: case 0x29: case 0x2B: return { 16, 0 };	// movups, movupd, movss, movsd; movaps, movapd; movntps, movntpd
				case 0x13: case 0x17: return { 8, 0 };				// movlps, movlpd, movhps, movhpd to m64
				case 0x7E: return prefixes.repz ? StoreShape{} : StoreShape{ full, 0 };	// movd, movq to r/m - F3 0F 7E loads
				case 0x7F: return { prefixes.operand16 || prefixes.repz ? 16u : 8u, 0 };	// movdqa, movdqu; movq m64, mm
				case 0xD6: return prefixes.operand16 ? StoreShape{ 8, 0 } : StoreShape{};	// movq m64, xmm
				case 0xE7: return { prefixes.operand16 ? 16u : 8u, 0 };					// movntdq; movntq
				case 0xC3: return { full, 0 };											// movnti
				case 0xAB: case 0xB3: case 0xBB: return { full, 0 };					// bts, btr, btc r/m, r
				case 0xBA: return reg >= 5 ? StoreShape{ full, 1 } : StoreShape{};		// bts, btr, btc r/m, imm8; /4 is bt
				case 0xA4: case 0xAC: return { full, 1 };								// shld, shrd by imm8
				case 0xA5: case 0xAD: return { full, 0 };								// by cl
				case 0xB0: case 0xC0: return { 1, 0 };									// cmpxchg, xadd r/m8
				case 0xB1: case 0xC1: return { full, 0 };
				case 0xC7: return reg == 1 ? StoreShape{ prefixes.rexW ? 16u : 8u, 0 } : StoreShape{};	// cmpxchg8b, cmpxchg16b
				default: return {};
				}
			}

			// The opcodes after 0F 38 and 0F 3A.
			StoreShape ThreeByteStore(uint8_t map, uint8_t opcode, const Prefixes& prefixes)
			{
				if (map == 0x38)	// movbe m, r - F2 0F 38 F1 is crc32, a load
					return opcode == 0xF1 && !prefixes.repnz ? StoreShape{ prefixes.rexW ? 8u : 4u, 0 } : StoreShape{};
				if (!prefixes.operand16)
					return {};
				switch (opcode)		// 66 0F 3A: pextrb, pextrw, pextrd and pextrq, extractps to memory, each with an imm8
				{
				case 0x14: return { 1, 1 };
				case 0x15: return { 2, 1 };
				case 0x16: return { prefixes.rexW ? 8u : 4u, 1 };
				case 0x17: return { 4, 1 };
				default: return {};
				}
			}

			// The AVX opcodes: map 1 is 0F, 2 0F 38, 3 0F 3A; pp 1 is 66, 2 F3, 3 F2; wide the 256-bit form.
			StoreShape VexStore(int map, uint8_t opcode, int pp, bool wide, bool rexW)
			{
				const uint32_t vector = wide ? 32 : 16;
				if (map == 1)
					switch (opcode)
					{
					case 0x11: return { pp == 2 ? 4u : pp == 3 ? 8u : vector, 0 };		// vmovups, vmovupd, vmovss, vmovsd
					case 0x13: case 0x17: return { 8, 0 };								// vmovlps, vmovlpd, vmovhps, vmovhpd
					case 0x29: case 0x2B: return { vector, 0 };							// vmovaps, vmovapd, vmovntps, vmovntpd
					case 0x7E: return pp == 1 ? StoreShape{ rexW ? 8u : 4u, 0 } : StoreShape{};	// vmovd, vmovq to r/m - F3 loads
					case 0x7F: return pp == 1 || pp == 2 ? StoreShape{ vector, 0 } : StoreShape{};	// vmovdqa, vmovdqu
					case 0xD6: return pp == 1 ? StoreShape{ 8, 0 } : StoreShape{};		// vmovq m64
					case 0xE7: return pp == 1 ? StoreShape{ vector, 0 } : StoreShape{};	// vmovntdq
					default: return {};
					}
				if (pp != 1)
					return {};
				if (map == 2)	// vmaskmovps, vmaskmovpd, vpmaskmovd and vpmaskmovq to memory
					return opcode == 0x2E || opcode == 0x2F || opcode == 0x8E ? StoreShape{ vector, 0 } : StoreShape{};
				switch (opcode)	// the extracts, each with an imm8
				{
				case 0x14: return { 1, 1 };
				case 0x15: return { 2, 1 };
				case 0x16: return { rexW ? 8u : 4u, 1 };
				case 0x17: return { 4, 1 };
				case 0x19: case 0x39: return { 16, 1 };	// vextractf128, vextracti128
				case 0x1D: return { wide ? 16u : 8u, 1 };	// vcvtps2ph
				default: return {};
				}
			}

			// The prefixes read again with the nearest F3 or F2 before opcode - and so the bytes before it - taken for the
			// instruction before: for a prefix that makes a store a load. Called only while prefixes holds one of them.
			Prefixes WithoutRep(const uint8_t* opcode, const Prefixes& prefixes)
			{
				const uint8_t* at = opcode - 1;
				while (at > prefixes.start && *at != 0xF3 && *at != 0xF2)
					at--;
				return ReadPrefixes(opcode, at + 1);
			}

			// The instruction whose ModRM lies at modrm, read from the bytes before it - none before begin - every way the
			// processor could read them if an instruction started among them, since any of them may end the instruction before
			// instead: as one with a VEX prefix (C4 with a map of 1 to 3, or C5), then with the escapes 0F 38, 0F 3A and 0F, then
			// with a one-byte opcode, the last two each with the prefixes before it; a reading whose F3 or F2 makes a store a load (movq's,
			// crc32's) once more without that prefix and the bytes before it. The first byte of the instruction under the first
			// reading that counts says is a store of the range, or nullptr when none is.
			template <typename Counts>
			const uint8_t* StoreStart(const uint8_t* modrm, const uint8_t* begin, Counts&& counts)
			{
				const auto before = [modrm, begin](ptrdiff_t back) { return modrm - back >= begin ? static_cast<int>(modrm[-back]) : -1; };
				const uint8_t opcode = modrm[-1];
				const int reg = (modrm[0] >> 3) & 7;
				if (before(4) == 0xC4 && before(3) >= 0 && (before(3) & 0x1F) >= 1 && (before(3) & 0x1F) <= 3 && before(2) >= 0 &&
					counts(VexStore(before(3) & 0x1F, opcode, before(2) & 3, (before(2) & 4) != 0, (before(2) & 0x80) != 0)))
					return modrm - 4;	// C4, R X B and the map, W vvvv L pp
				if (before(3) == 0xC5 && before(2) >= 0 && counts(VexStore(1, opcode, before(2) & 3, (before(2) & 4) != 0, false)))
					return modrm - 3;	// C5, R vvvv L pp
				if (before(3) == 0x0F && (before(2) == 0x38 || before(2) == 0x3A))
					for (Prefixes prefixes = ReadPrefixes(modrm - 3, begin);; prefixes = WithoutRep(modrm - 3, prefixes))
					{
						if (counts(ThreeByteStore(static_cast<uint8_t>(before(2)), opcode, prefixes)))
							return prefixes.start;
						if (!prefixes.repz && !prefixes.repnz)
							break;
					}
				if (before(2) == 0x0F)
					for (Prefixes prefixes = ReadPrefixes(modrm - 2, begin);; prefixes = WithoutRep(modrm - 2, prefixes))
					{
						if (counts(TwoByteStore(opcode, reg, prefixes)))
							return prefixes.start;
						if (!prefixes.repz && !prefixes.repnz)
							break;
					}
				const Prefixes prefixes = ReadPrefixes(modrm - 1, begin);
				return counts(OneByteStore(opcode, reg, prefixes)) ? prefixes.start : nullptr;
			}
		}

		// Every ModRM of mod 00 and r/m 101 is found the way FindRipRelative finds its loads, 16 positions at a time, and its
		// disp32 checked to lead near the range - within the 4 bytes of an immediate and the 32 of a store before it - before
		// the bytes in front of it are read every way an instruction could start among them; one that writes a byte of the
		// range under any reading is counted, given by that reading's first byte.
		std::vector<uint8_t*> FindRipRelativeStores(const void* target, size_t size, size_t limit)
		{
			constexpr size_t kOperand = 5;			// the ModRM and the disp32
			constexpr intptr_t kReach = 4 + 32;		// an immediate after the disp32, the widest store
			std::vector<uint8_t*> found;
			const auto first = reinterpret_cast<intptr_t>(target);
			const intptr_t end = first + static_cast<intptr_t>(size);
			const auto writes = [first, end](intptr_t operand, uint32_t immediate, uint32_t width)	// operand: where it lies without an immediate
			{
				const intptr_t at = operand + static_cast<intptr_t>(immediate);
				return at < end && at + static_cast<intptr_t>(width) > first;
			};
			const auto stores = [&found, &writes, first, end](uint8_t* modrm, const uint8_t* begin)
			{
				int32_t disp = 0;
				memcpy(&disp, modrm + 1, sizeof(disp));
				const intptr_t operand = reinterpret_cast<intptr_t>(modrm + kOperand) + disp;
				if (operand >= end || operand + kReach <= first)
					return;
				const uint8_t* const start = StoreStart(modrm, begin, [&writes, operand](const StoreShape& shape)
				{
					return shape.width && (writes(operand, shape.immediate, shape.width) || (shape.altImmediate && writes(operand, shape.altImmediate, shape.width)));
				});
				if (start)
					found.push_back(const_cast<uint8_t*>(start));
			};
			const __m128i modrmBits = _mm_set1_epi8(static_cast<char>(0xC7));
			const __m128i ripRelative = _mm_set1_epi8(0x05);
			for (const Range& range : ReadableCode())
			{
				const size_t available = static_cast<size_t>(range.end - range.begin);
				if (available < kOperand + 1)
					continue;
				const size_t lastModrm = available - kOperand;	// the disp32 stays inside the range
				size_t at = 1;									// an opcode before it
				for (; at + 15 <= lastModrm && found.size() < limit; at += 16)
				{
					const __m128i x = _mm_loadu_si128(reinterpret_cast<const __m128i*>(range.begin + at));
					unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(x, modrmBits), ripRelative)));
					while (mask && found.size() < limit)
					{
						unsigned long bit = 0;
						_BitScanForward(&bit, mask);
						stores(range.begin + at + bit, range.begin);
						mask &= mask - 1;
					}
				}
				for (; at <= lastModrm && found.size() < limit; at++)
					if ((range.begin[at] & 0xC7) == 0x05)
						stores(range.begin + at, range.begin);
			}
			return found;
		}
	}
}
