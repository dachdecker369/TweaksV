// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the RayTracing part (see raytracing.h).
//
// The game skins every moving object its ray tracing keeps - vehicles, peds, trees, grass, tyres, cables, ropes - into one
// global vertex buffer each frame, the output of its "bvh_skinning" compute effect, which the BLAS refits read before the
// TLASes of the sun shadows, the reflections and the occlusion are built. It holds 4,194,304 vertices of 20 bytes, 80 MB,
// and is made once, at the renderer's start after DllMain, by a call that hands the buffer maker its count as an immediate.
// Each frame hands out ranges of it from 0, by a counter per frame slot, and two checks - one for every moving object's
// geometry, one for the ropes - refuse a range whose end would pass the same immediate: a refused object gets no refit and
// no instance in that frame's TLASes, so its ray traced shadow, reflection and occlusion are missing, and as the order of
// the objects changes from frame to frame they come and go - the shadows under cars that go missing or flicker with many
// high-poly vehicles (PATTERNS.md, "RayTracing"). The part raises the buffer and its two checks together:
//   DYNAMIC_BVH_VERTICES  0, the game's own and the default: nothing is looked for or written but with DIAGNOSTIC_MODE;
//                         4,194,304 to 26,843,545 vertices otherwise - a value below the game's own is raised to it and
//                         reported, one above the cap clamped and reported by the settings. The cap keeps a raw view of
//                         the buffer within 2^27 32-bit elements, and far below the 26 bits the ray tracing shaders read an
//                         object's first vertex in and the game's 32-bit sizes
//   DIAGNOSTIC_MODE       every site logged, and the measurement, also with the key at 0: a thread reads the frame slot and
//                         the last finished frame's counter every 50 ms, and the checks' two refusal jumps lead through
//                         counting stubs, unless the game has made the buffer already and its threads may run the checks;
//                         10 seconds in, and then every minute while the most taken or the refusals change,
//                         the log says how much of the buffer the frames take and how many geometries it refused
//   DRY_RUN               everything found and checked, nothing written: the lines say what would be raised and
//                         redirected, and the refusals are not counted
// Every site is found and checked before the one write: T-E-B1, T-E-C1 and T-E-C2 once each; the buffer maker's call into
// the game, to a function start by the game's unwind data; the buffer object's global inside the game, aligned and still 0 -
// the buffer not made yet, since a raise of the checks alone would let the game write past it; both checks loading one frame
// slot and one pair of counters inside the game and aligned, which the game's code references in exactly three places - the
// checks and the frame's reset - and their refusal jumps leading into the game; the three immediates holding one value - the
// game's own, raised; another mod's, left as it is; disagreeing, refused. The stubs run on the game's worker threads, so
// they count with lock instructions into 64-bit counters, add the vertex count in ecx zero-extended, keep every register
// but rax and the flags, and lie near the game, within reach of the jumps, read+execute, their counters on a read/write
// page of their own.
// Written for TweaksV from the findings of its own research in the game's code (PATTERNS.md); the idea of a larger buffer
// is that of Resource Extensions [Enhanced] by Koukol, whose plugin was not read (CREDITS.md).

#include "raytracing.h"

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "conflicts.h"
#include "log.h"
#include "measure.h"
#include "patch.h"
#include "patterns.h"
#include "scanner.h"
#include "settings.h"
#include "summary.h"

namespace TweaksV
{
	namespace
	{
		constexpr const char* kTag = "RayTracing";
		constexpr const char* kJob = "The ray tracing buffer of moving geometry is not raised this session: ";
		constexpr const char* kNotMeasured = "DIAGNOSTIC: the ray tracing buffer of moving geometry is not measured: ";

		// The bytes a pattern's match spans.
		consteval size_t PatternSize(const GamePattern& pattern)
		{
			size_t size = 1;
			for (const char* c = pattern.text; *c; c++)
				if (*c == ' ')
					size++;
			return size;
		}

		// ---- the sites (PATTERNS.md, "RayTracing")

		constexpr const GamePattern& kSetUp = Pattern("T-E-B1");		// the buffer's set-up: its count and the buffer maker's call
		constexpr const GamePattern& kGeometry = Pattern("T-E-C1");		// the check of every moving object's geometry
		constexpr const GamePattern& kRopes = Pattern("T-E-C2");		// the check of the ropes

		// T-E-B1: "mov dword [rsp+20h], 0x89", the maker's flags; "mov ecx, imm32", the count (its imm32 at +9); "mov edx, 20",
		// the stride; "mov r8b, 1"; "xor r9d, r9d"; "call rel32", the buffer maker (+24); "mov [rip+disp32], rax", the store of
		// the buffer object (+29), its disp32 ending the match (+32).
		constexpr size_t kCountAt = 9, kMakerCallAt = 24, kObjectStoreAt = 29, kObjectAt = 32;
		constexpr uint32_t kStride = 20;	// bytes per vertex: a position and two packed vectors
		static_assert(PatternHolds(kSetUp, 0, { 0xC7, 0x44, 0x24, 0x20, 0x89 }) && PatternHolds(kSetUp, kCountAt - 1, { 0xB9 }) &&
			PatternHolds(kSetUp, 13, { 0xBA, static_cast<int>(kStride), 0x00, 0x00, 0x00 }) && PatternHolds(kSetUp, kMakerCallAt, { 0xE8 }) &&
			PatternHolds(kSetUp, kObjectStoreAt, { 0x48, 0x89, 0x05 }) && PatternSize(kSetUp) == kObjectAt + 4,
			"TweaksV: T-E-B1 holds the count's mov at +8, the stride 20, the maker's call at +24 and the object's store ending the match");

		// T-E-C1 and T-E-C2, the two checks: "mov e?x, [rip+disp32]", the frame slot (its disp32 at +2); "lea r64, [rip+disp32]",
		// the counters, a dword per slot (+8, its disp32 at +11); "mov r10d, 1"; then "lea e?x, [rdi+rcx]", the slot's counter plus
		// the vertex count in ecx, on which the stubs' mov eax, ecx rests; "cmp e?x, imm32" of that sum against the capacity and
		// "ja rel32" to the refusal, its rel32 right past the match.
		struct Check
		{
			const GamePattern* pattern;
			const char* what;		// for the log: "geometry's"
			size_t capacityAt;		// the cmp's imm32
			size_t jumpAt;			// the ja, 0F 87 rel32
		};
		constexpr Check kChecks[] = { { &kGeometry, "geometry's", 44, 48 }, { &kRopes, "ropes'", 41, 45 } };
		constexpr size_t kSlotAt = 2, kCountersLeaAt = 8, kCountersAt = 11, kJumpSize = 6;
		consteval bool ChecksAsPinned()
		{
			for (const Check& check : kChecks)
			{
				const GamePattern& pattern = *check.pattern;
				const size_t sumAt = check.capacityAt - 5;	// the lea of the sum: 8D, its ModRM with a SIB (mod 00, r/m 100), the SIB naming rcx
				if (!PatternHolds(pattern, 0, { 0x8B }) || (PatternByte(pattern, 1) & 0xC7) != 0x05 || !PatternHolds(pattern, kCountersLeaAt, { 0x48, 0x8D }) ||
					(PatternByte(pattern, kCountersLeaAt + 2) & 0xC7) != 0x05 || !PatternHolds(pattern, 15, { 0x41, 0xBA, 0x01, 0x00, 0x00, 0x00 }) ||
					!PatternHolds(pattern, sumAt, { 0x8D }) || (PatternByte(pattern, sumAt + 1) & 0xC7) != 0x04 || !PatternHolds(pattern, sumAt + 2, { 0x0F }) ||
					!PatternHolds(pattern, check.capacityAt - 2, { 0x81, 0xF8 | ((PatternByte(pattern, sumAt + 1) >> 3) & 7) }) ||
					!PatternHolds(pattern, check.jumpAt, { 0x0F, 0x87 }) || PatternSize(pattern) != check.jumpAt + 2)
					return false;
			}
			return true;
		}
		static_assert(ChecksAsPinned(), "TweaksV: T-E-C1 and T-E-C2 hold the slot's load, the counters' lea, the lea adding ecx, the cmp of that sum and the ja, its rel32 past the match");

		constexpr uint32_t kGameVertices = 0x400000;	// the game's own: 4,194,304 vertices, 80 MB (1.0.1158.16)
		constexpr uint32_t kMaxVertices = 26843545;		// the cap: a raw view of the buffer within 2^27 32-bit elements
		static_assert(kMaxVertices == static_cast<uint32_t>(kDynamicBvhVertices.maxValue), "TweaksV: DYNAMIC_BVH_VERTICES' maximum is the cap");
		static_assert(uint64_t{ kMaxVertices } * kStride / 4 <= (uint64_t{ 1 } << 27) && kMaxVertices < (uint32_t{ 1 } << 26) && kGameVertices < kMaxVertices,
			"TweaksV: the cap keeps a raw view within 2^27 32-bit elements and every first vertex within the 26 bits the shaders read");

		// Every site, found and checked.
		struct Sites
		{
			uint8_t* count = nullptr;				// T-E-B1's imm32: the buffer's count
			const uint8_t* object = nullptr;		// the buffer object's global
			bool made = false;						// it was set at the start: the buffer made already
			const uint8_t* slot = nullptr;			// the frame slot, a dword: 0 or 1
			const uint8_t* counters = nullptr;		// the counters, a dword per slot
			uint8_t* capacity[2] = {};				// each check's imm32, in the order of kChecks
			uint8_t* jump[2] = {};					// each check's ja
			const uint8_t* refusal[2] = {};			// where each ja leads
			uint32_t vertices = 0;					// what the three immediates hold
		};

		// "RVA 0x1, RVA 0x2 and RVA 0x3"
		std::string Addresses(const std::vector<uint8_t*>& addresses)
		{
			std::string text;
			for (size_t i = 0; i < addresses.size(); i++)
				text += (i == 0 ? "" : i + 1 == addresses.size() ? " and " : ", ") + DescribeAddress(addresses[i]);
			return text;
		}

		// The bytes of so many vertices in MB, rounded: "80".
		std::string Megabytes(uint64_t vertices)
		{
			return std::to_string((vertices * kStride + (uint64_t{ 1 } << 19)) >> 20);
		}

		// True if the count values of T at at lie inside the game, aligned to their size, so that Load reads each whole, and can be
		// read; else why not.
		template <typename T> bool DataOf(const char* what, const uint8_t* at, size_t count, const GamePattern& pattern, std::string& why)
		{
			const std::string fault = DataFault(at, count * sizeof(T), sizeof(T), count > 1);
			if (fault.empty())
				return true;
			why = std::string("the ") + what + " at " + DescribeAddress(at) + At(pattern) + " " + fault + ".";
			return false;
		}

		// T-E-B1, T-E-C1 and T-E-C2 found and every site checked - with DIAGNOSTIC_MODE each logged; false at the first that is
		// not as the research found it, and why.
		bool FindAndCheck(Sites& sites, std::string& why)
		{
			const bool diagnostic = IsDiagnosticMode();
			uint8_t* const setUp = Scanner::FindOnce(kSetUp, why);	// all three looked for, so that the log names each one missing
			uint8_t* const matches[2] = { Scanner::FindOnce(*kChecks[0].pattern, why), Scanner::FindOnce(*kChecks[1].pattern, why) };
			if (!setUp || !matches[0] || !matches[1])
			{
				why += ".";
				return false;
			}

			// T-E-B1: the count; the buffer maker's call into the game, to a function start by the game's unwind data; the
			// buffer object's global inside the game - 0 until the game makes the buffer
			sites.count = setUp + kCountAt;
			const uint8_t* const call = setUp + kMakerCallAt;
			const uint8_t* const maker = CallTarget(call);
			if (!IsInsideGameImage(maker))
			{
				why = "the buffer maker's call at " + DescribeAddress(call) + " is not a call into the game (another mod may have hooked it already)" +
					At(kSetUp) + ".";
				return false;
			}
			const uint8_t* const function = FunctionStartOf(maker);
			if (function != maker)
			{
				why = "the buffer maker's call at " + DescribeAddress(call) + " leads to " + DescribeAddress(maker) + ", which is not a function start (" +
					(function ? "inside the function at " + DescribeAddress(function) + " (unwind data)" : std::string("no unwind data")) + ")" + At(kSetUp) + ".";
				return false;
			}
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: buffer's count at " + DescribeAddress(sites.count) + " holds " + std::to_string(ValueAt<uint32_t>(sites.count)) +
					", the buffer maker's call at " + DescribeAddress(call) + " -> " + DescribeAddress(maker) + ", a function start (unwind data)" + At(kSetUp));
			sites.object = Rel32Target(setUp + kObjectAt);
			if (!DataOf<uint64_t>("buffer object's global", sites.object, 1, kSetUp, why))
				return false;
			const uint64_t object = Load<uint64_t>(sites.object);
			sites.made = object != 0;
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: buffer object's store at " + DescribeAddress(setUp + kObjectStoreAt) + " -> " + DescribeAddress(sites.object) +
					", which holds " + (sites.made ? Hex(object) + ": the buffer is made already" : std::string("0: the buffer is not made yet")) + At(kSetUp));

			// T-E-C1 and T-E-C2: one frame slot and one pair of counters inside the game; each one's refusal jump into the game
			for (size_t i = 0; i < 2; i++)
			{
				const Check& check = kChecks[i];
				uint8_t* const match = matches[i];
				const uint8_t* const slot = Rel32Target(match + kSlotAt);
				const uint8_t* const counters = Rel32Target(match + kCountersAt);
				if (!DataOf<uint32_t>("frame slot", slot, 1, *check.pattern, why) || !DataOf<uint32_t>("counters", counters, 2, *check.pattern, why))
					return false;
				if (i == 0)
				{
					sites.slot = slot;
					sites.counters = counters;
				}
				else if (slot != sites.slot || counters != sites.counters)
				{
					why = slot != sites.slot ?
						"the two checks load different frame slots: " + DescribeAddress(sites.slot) + At(*kChecks[0].pattern) + " and " + DescribeAddress(slot) + At(*check.pattern) + "." :
						"the two checks load different counters: " + DescribeAddress(sites.counters) + At(*kChecks[0].pattern) + " and " + DescribeAddress(counters) + At(*check.pattern) + ".";
					return false;
				}
				sites.capacity[i] = match + check.capacityAt;
				sites.jump[i] = match + check.jumpAt;
				const uint8_t* const rel = sites.jump[i] + 2;
				if (!IsReadable(rel, sizeof(int32_t)))
				{
					why = std::string("the ") + check.what + " refusal jump at " + DescribeAddress(sites.jump[i]) + " cannot be read" + At(*check.pattern) + ".";
					return false;
				}
				sites.refusal[i] = Rel32Target(rel);
				if (!IsInsideGameImage(sites.refusal[i]))
				{
					why = std::string("the ") + check.what + " refusal jump at " + DescribeAddress(sites.jump[i]) + " does not lead into the game (another mod may have "
						"hooked it already)" + At(*check.pattern) + ".";
					return false;
				}
				if (diagnostic)
					WriteLog(kTag, "DIAGNOSTIC: " + std::string(check.what) + " check at " + DescribeAddress(sites.capacity[i] - 2) + " holds " +
						std::to_string(ValueAt<uint32_t>(sites.capacity[i])) + ", its frame slot at " + DescribeAddress(slot) + ", its counters at " +
						DescribeAddress(counters) + ", its refusal jump at " + DescribeAddress(sites.jump[i]) + " -> " + DescribeAddress(sites.refusal[i]) + ": " +
						(IsReadable(sites.refusal[i], 8) ? HexBytes(sites.refusal[i], 8) : std::string("cannot be read")) + At(*check.pattern));
			}

			// The counters' references in the game's code: the two checks' and the frame's reset, as the research found them
			const std::vector<uint8_t*> references = Scanner::FindRipRelativeReferences(sites.counters, 4);
			const std::string places = references.size() > 3 ? "4 places or more" : std::to_string(references.size()) + " places";
			if (diagnostic)
				WriteLog(kTag, "DIAGNOSTIC: the counters at " + DescribeAddress(sites.counters) + " are referenced in " + places + " of the game's code: " +
					Addresses(references));
			if (references.size() != 3)
			{
				why = "the game's code references the counters at " + DescribeAddress(sites.counters) + " in " + places + ", not 3 - the two checks and the "
					"frame's reset, as the research found them.";
				return false;
			}

			// The buffer's count and its two checks: one value
			const uint32_t count = ValueAt<uint32_t>(sites.count);
			const uint32_t geometry = ValueAt<uint32_t>(sites.capacity[0]);
			const uint32_t ropes = ValueAt<uint32_t>(sites.capacity[1]);
			if (geometry != count || ropes != count)
			{
				why = "the buffer's count and its two checks disagree: " + std::to_string(count) + " at " + DescribeAddress(sites.count) + At(kSetUp) + ", " +
					std::to_string(geometry) + " at " + DescribeAddress(sites.capacity[0]) + At(kGeometry) + " and " + std::to_string(ropes) + " at " +
					DescribeAddress(sites.capacity[1]) + At(kRopes) + ".";
				return false;
			}
			sites.vertices = count;
			return true;
		}

		// ---- the raise

		// Raises the buffer and its two checks to the key's value in one write, all or nothing; returns their size afterwards.
		uint32_t Raise(const Sites& sites, int requested)
		{
			const std::string key = std::string(kDynamicBvhVertices.key) + "=" + std::to_string(requested);
			if (sites.vertices != kGameVertices)
			{
				WriteLog(kTag, std::string(kJob) + "another mod already set it to " + std::to_string(sites.vertices) + " vertices - left as it is.");
				return sites.vertices;
			}
			uint32_t size = static_cast<uint32_t>(requested);	// within 1..kMaxVertices: the settings clamp it
			if (size < kGameVertices)
			{
				size = kGameVertices;
				ReportFailure(kTag, "TweaksV.ini: [" + std::string(kRayTracingSection.name) + "] " + key + " is outside 0 or " + std::to_string(kGameVertices) +
					".." + std::to_string(kMaxVertices) + " - using " + std::to_string(size) + ".");
			}
			if (size == kGameVertices)
			{
				WriteLog(kTag, "The ray tracing buffer of moving geometry holds " + std::to_string(kGameVertices) + " vertices already (" + key + ") - nothing to patch.");
				return kGameVertices;
			}
			if (sites.made)
			{
				ReportFailure(kTag, std::string(kJob) + "the game has made the buffer already - its object at " + DescribeAddress(sites.object) + " is set" + At(kSetUp) +
					" - and a raise of its checks alone would let the game write past it.");
				return kGameVertices;
			}
			const std::vector<uint8_t> bytes = BytesOf(size);
			if (!WriteCode({ { sites.count, bytes, "ray tracing buffer's count" }, { sites.capacity[0], bytes, "geometry's capacity check" },
				{ sites.capacity[1], bytes, "ropes' capacity check" } }, kTag))
			{
				ReportFailure(kTag, std::string(kJob) + "the game code could not be changed (details in TweaksV.log).");
				return kGameVertices;
			}
			const std::string change = std::to_string(kGameVertices) + " to " + std::to_string(size) + " vertices";
			if (IsDryRun())
			{
				WriteLog(kTag, "DRY_RUN: would raise the ray tracing buffer of moving geometry from " + change + " (" + DescribeAddress(sites.count) + ", " +
					DescribeAddress(sites.capacity[0]) + " and " + DescribeAddress(sites.capacity[1]) + ").");
				return kGameVertices;
			}
			WriteLog(kTag, "The ray tracing buffer of moving geometry is raised from " + change + " (" + Megabytes(kGameVertices) + " to " + Megabytes(size) + " MB).");
			return size;
		}

		// ---- the measurement (DIAGNOSTIC_MODE)

		constexpr DWORD kSampleMs = 50;		// a frame takes 10 to 20 ms on a fast PC: a sample catches one in three to five

		// The buffer object, as the buffer maker keeps it (PATTERNS.md): its count of vertices, a dword, and its stride, 12 bits of a word.
		constexpr size_t kObjectCountAt = 0x08, kObjectStrideAt = 0x0C;
		constexpr uint32_t kObjectStrideMask = 0xFFF;

		// What the sites lead to, set before the measurement's threads start, and the buffer's size after the part's work.
		const uint8_t* g_slot = nullptr;
		const uint8_t* g_counters = nullptr;
		const uint8_t* g_object = nullptr;
		uint32_t g_capacity = 0;

		// The sampling thread's: the last finished frame's vertices as last sampled, the most of every sample, how many samples.
		std::atomic<uint32_t> g_latest{ 0 };
		std::atomic<uint32_t> g_most{ 0 };
		std::atomic<uint64_t> g_samples{ 0 };
		std::atomic<bool> g_sampled{ false };	// the sampling thread runs

		// A stub's counters, on the read/write page of the stubs' block: the geometries refused and their vertices.
		struct Refusals
		{
			uint64_t geometries;
			uint64_t vertices;
		};
		enum class Counting { NotCounted, DryRun, Counted };
		std::atomic<const Refusals*> g_refusals{ nullptr };		// one per check, set once the jumps lead to the stubs
		std::atomic<Counting> g_counting{ Counting::NotCounted };

		// Both checks' refusals together, and the ropes' alone.
		struct Refused
		{
			uint64_t geometries = 0;
			uint64_t vertices = 0;
			uint64_t ropes = 0;
		};

		// Each counter read once, so that the ropes never come out more than the geometries they are counted in.
		Refused ReadRefused()
		{
			Refused refused;
			if (const Refusals* const counters = g_refusals.load(std::memory_order_acquire))
			{
				refused.ropes = Load<uint64_t>(&counters[1].geometries);	// kChecks' second, the ropes'
				refused.geometries = Load<uint64_t>(&counters[0].geometries) + refused.ropes;
				refused.vertices = Load<uint64_t>(&counters[0].vertices) + Load<uint64_t>(&counters[1].vertices);
			}
			return refused;
		}

		// One sample: the frame slot, the other slot's counter - the last finished frame's total, since the frame under way counts
		// into the slot's own and the other is reset only after the next flip - and the slot again; a flip between the two reads
		// makes the counter possibly the new frame's, and the sample is dropped. Aligned reads of the game's data, nothing written.
		void Sample()
		{
			if (!IsReadable(g_slot, sizeof(uint32_t)) || !IsReadable(g_counters, 2 * sizeof(uint32_t)))
				return;
			const uint32_t slot = Load<uint32_t>(g_slot);
			if (slot > 1)
				return;
			const uint32_t finished = Load<uint32_t>(g_counters + (1 - slot) * sizeof(uint32_t));
			if (Load<uint32_t>(g_slot) != slot)
				return;
			g_latest.store(finished, std::memory_order_relaxed);
			if (finished > g_most.load(std::memory_order_relaxed))	// this thread alone writes it
				g_most.store(finished, std::memory_order_relaxed);
			g_samples.fetch_add(1, std::memory_order_relaxed);
		}

		// The sampling thread: a sample every 50 ms, and a line of its own at the first refusal the stubs count, so that the
		// moment shows. Each pass inside a try, so nothing leaves the thread; it never ends.
		DWORD WINAPI SampleFrames(LPVOID)
		{
			bool firstRefusalSaid = false;
			for (;;)
			{
				Sleep(kSampleMs);
				try
				{
					Sample();
					if (!firstRefusalSaid && ReadRefused().geometries != 0)
					{
						firstRefusalSaid = true;
						WriteLog(kTag, "DIAGNOSTIC: ray tracing buffer of moving geometry: the first geometry refused - the buffer is full, and an object it "
							"refuses has no ray traced shadow, reflection or occlusion in that frame.");
					}
				}
				catch (...)
				{
				}
			}
		}

		// What a summary says, read at once.
		struct Reading
		{
			uint64_t samples = 0;
			uint32_t latest = 0;
			uint32_t most = 0;
			bool objectRead = false;	// the buffer object's global could be read
			uint64_t object = 0;		// what it holds: the buffer object, or 0 while the buffer is not made
			bool fieldsRead = false;	// the object's count and stride could be read
			uint32_t count = 0;
			uint32_t stride = 0;
			Refused refused;
		};

		Reading ReadNow()
		{
			Reading reading;
			reading.samples = g_samples.load(std::memory_order_relaxed);
			reading.latest = g_latest.load(std::memory_order_relaxed);
			reading.most = g_most.load(std::memory_order_relaxed);
			reading.objectRead = IsReadable(g_object, sizeof(uint64_t));
			if (reading.objectRead)
				reading.object = Load<uint64_t>(g_object);
			if (reading.object)	// heap data of the game's, which it may free: read under a guard
			{
				uint8_t fields[kObjectStrideAt + sizeof(uint16_t) - kObjectCountAt] = {};
				reading.fieldsRead = CopyMemorySafely(fields, reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(reading.object)) + kObjectCountAt,
					sizeof(fields));
				if (reading.fieldsRead)
				{
					reading.count = ValueAt<uint32_t>(fields);
					reading.stride = ValueAt<uint16_t>(fields + (kObjectStrideAt - kObjectCountAt)) & kObjectStrideMask;
				}
			}
			reading.refused = ReadRefused();
			return reading;
		}

		// The summary's state, kept by its thread alone: the reading of the last summary and how many readings had news.
		Reading g_logged;
		uint64_t g_news = 0;

		// Changes with every reading that has something new to say: the first; a new most; a refusal counted; the buffer made.
		uint64_t Numbers()
		{
			const Reading now = ReadNow();
			const bool news = g_news == 0 || now.most != g_logged.most || now.refused.geometries != g_logged.refused.geometries ||
				now.objectRead != g_logged.objectRead || now.object != g_logged.object || now.fieldsRead != g_logged.fieldsRead;
			if (news)
			{
				g_logged = now;
				g_news++;
			}
			return g_news;
		}

		void LogSummary(uint64_t)
		{
			const Reading& now = g_logged;
			std::string line = "DIAGNOSTIC: ray tracing buffer of moving geometry, ";
			if (!now.objectRead)
				line += "its object's global at " + DescribeAddress(g_object) + " cannot be read";
			else if (!now.object)
				line += "not made yet";
			else if (!now.fieldsRead)
				line += "made, its object at " + Hex(now.object) + " cannot be read";
			else
				line += "made with " + std::to_string(now.count) + " vertices of " + std::to_string(now.stride) + " bytes";
			if (!g_sampled.load())
				line += ": its frames are not sampled (see above)";
			else if (now.samples == 0)
				line += ": no frame sampled yet";
			else
				line += ": the last frame took " + std::to_string(now.latest) + " of " + std::to_string(g_capacity) + " vertices (" + Megabytes(now.latest) + " of " +
					Megabytes(g_capacity) + " MB), at most " + std::to_string(now.most) + " so far";
			switch (g_counting.load())
			{
			case Counting::Counted:
				if (now.refused.geometries == 0)
					line += "; no geometry refused";
				else
					line += "; " + std::to_string(now.refused.geometries) + (now.refused.geometries == 1 ? " geometry" : " geometries") + " refused (" +
						std::to_string(now.refused.vertices) + " vertices in all)" + (now.refused.ropes ? ", " + std::to_string(now.refused.ropes) + " of them ropes" :
						std::string()) + " - the buffer is full: those objects had no ray traced shadow, reflection or occlusion in those frames";
				break;
			case Counting::DryRun:
				line += "; the refusals are not counted (DRY_RUN)";
				break;
			case Counting::NotCounted:
				line += "; the refusals are not counted (see above)";
				break;
			}
			WriteLog(kTag, line + ".");
		}

		// The first summary 10 seconds in, then one every minute that has something new to say.
		constexpr Measurement kMeasurement = { "ray tracing buffer summary", kFirstSummaryMs, &Numbers, &LogSummary };

		// ---- the counting stubs

		// A stub: the geometry refused counted and its vertex count added, then on to the game's refusal - 32 bytes, every
		// register but rax and the flags as it was, rsp untouched, the counters reached rip-relative.
		constexpr uint8_t kStub[] = {
			0xF0, 0x48, 0xFF, 0x05, 0, 0, 0, 0,		// lock inc qword [rip+disp32]: the geometries refused
			0x89, 0xC8,								// mov eax, ecx: the vertex count the check added, zero-extended into rax
			0xF0, 0x48, 0x01, 0x05, 0, 0, 0, 0,		// lock add qword [rip+disp32], rax: their vertices
			0xFF, 0x25, 0, 0, 0, 0,					// jmp qword [rip+0]: on to the game's refusal,
			0, 0, 0, 0, 0, 0, 0, 0 };				// whose address follows
		constexpr size_t kStubSize = sizeof(kStub);
		constexpr size_t kGeometriesDispAt = 4, kGeometriesEnd = 8, kVerticesDispAt = 14, kVerticesEnd = 18, kTargetAt = 24;
		static_assert(kStubSize == 32 && kTargetAt + sizeof(uint64_t) == kStubSize, "TweaksV: a counting stub is 32 bytes, its target last");

		void NotCounted(const std::string& why)
		{
			WriteLog(kTag, "DIAGNOSTIC: the ray tracing buffer's refusals are not counted: " + why);
		}

		// The two stubs, one per check, on the first page of a block near the game, their counters on its second, read/write;
		// the first page read+execute once written, never writable and executable at once. Null, logged, if they cannot be made.
		uint8_t* MakeStubs(const Sites& sites, size_t page, Refusals*& counters)
		{
			uint8_t* const block = static_cast<uint8_t*>(AllocateNearGame(2 * page));
			if (!block)
			{
				NotCounted("no memory near the game could be had for its stubs.");
				return nullptr;
			}
			counters = reinterpret_cast<Refusals*>(block + page);
			for (size_t i = 0; i < 2; i++)
			{
				uint8_t* const stub = block + i * kStubSize;
				memcpy(stub, kStub, kStubSize);
				const auto rel = [stub](size_t end, const void* target)	// within one block of two pages: always within reach
				{
					return static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(stub + end));
				};
				const int32_t geometries = rel(kGeometriesEnd, &counters[i].geometries);
				const int32_t vertices = rel(kVerticesEnd, &counters[i].vertices);
				const uint64_t target = reinterpret_cast<uintptr_t>(sites.refusal[i]);
				memcpy(stub + kGeometriesDispAt, &geometries, sizeof(geometries));
				memcpy(stub + kVerticesDispAt, &vertices, sizeof(vertices));
				memcpy(stub + kTargetAt, &target, sizeof(target));
			}
			DWORD old = 0;
			if (!VirtualProtect(block, page, PAGE_EXECUTE_READ, &old))
			{
				const DWORD error = GetLastError();
				VirtualFree(block, 0, MEM_RELEASE);
				NotCounted("its stubs could not be made read+execute (" + DescribeError(error) + ").");
				return nullptr;
			}
			FlushInstructionCache(GetCurrentProcess(), block, page);
			return block;
		}

		// Each check's refusal jump pointed at its stub, both in one write or neither.
		bool RedirectRefusals(const Sites& sites, const uint8_t* block)
		{
			std::vector<CodeWrite> writes;
			for (size_t i = 0; i < 2; i++)
			{
				const intptr_t distance = reinterpret_cast<intptr_t>(block + i * kStubSize) - reinterpret_cast<intptr_t>(sites.jump[i] + kJumpSize);
				if (distance < INT32_MIN || distance > INT32_MAX)
				{
					NotCounted("its stubs are out of reach of the " + std::string(kChecks[i].what) + " refusal jump at " + DescribeAddress(sites.jump[i]) + ".");
					return false;
				}
				std::vector<uint8_t> jump = { 0x0F, 0x87 };
				const std::vector<uint8_t> rel = BytesOf(static_cast<int32_t>(distance));
				jump.insert(jump.end(), rel.begin(), rel.end());
				writes.push_back({ sites.jump[i], jump, std::string(kChecks[i].what) + " refusal jump" });
			}
			if (!WriteCode(writes, kTag))
			{
				NotCounted("the game code could not be changed (details above).");
				return false;
			}
			return true;
		}

		// With DIAGNOSTIC_MODE: the stubs made, the summary's thread and the sampling thread started, then the jumps redirected -
		// a measurement that cannot sum up redirects nothing. A buffer the game has made already gets no stubs: its threads may run
		// the checks then, and a jump's rel32 can cross a cache line, so a check could read it half written. Nothing of it is
		// reported: an aid (SPECIFICATIONS 12).
		void Measure(const Sites& sites, uint32_t capacity)
		{
			g_slot = sites.slot;
			g_counters = sites.counters;
			g_object = sites.object;
			g_capacity = capacity;
			const std::string jumps = DescribeAddress(sites.jump[0]) + At(kGeometry) + " and " + DescribeAddress(sites.jump[1]) + At(kRopes);

			uint8_t* block = nullptr;
			Refusals* counters = nullptr;
			if (sites.made)
				NotCounted("the game has made the buffer already - its threads may run the checks while their jumps are rewritten.");
			else if (IsDryRun())
			{
				g_counting = Counting::DryRun;
				WriteLog(kTag, "DRY_RUN: would redirect the refusal jumps at " + jumps + " to counting stubs - the refusals are not counted.");
			}
			else
				block = MakeStubs(sites, PageSize(), counters);

			if (!StartMeasurement<kMeasurement>())	// its line is written
			{
				if (block)
					VirtualFree(block, 0, MEM_RELEASE);	// no jump leads to it yet
				WriteLog(kTag, std::string(kNotMeasured) + "its summary thread could not be started.");
				return;
			}
			if (StartModuleThread(SampleFrames, "ray tracing buffer sampling"))
				g_sampled = true;
			else
				WriteLog(kTag, "DIAGNOSTIC: the ray tracing buffer's frames are not sampled: their thread could not be started.");
			if (block)
			{
				if (RedirectRefusals(sites, block))
				{
					g_refusals.store(counters, std::memory_order_release);
					g_counting = Counting::Counted;
					WriteLog(kTag, "DIAGNOSTIC: the refusal jumps at " + jumps + " are redirected to counting stubs: every geometry and rope the full buffer "
						"refuses is counted.");
				}
				else
					VirtualFree(block, 0, MEM_RELEASE);	// no jump leads to it: WriteCode wrote nothing
			}
			WriteLog(kTag, g_sampled.load() ? "DIAGNOSTIC: the ray tracing buffer of moving geometry is measured: the last finished frame's vertices are sampled every 50 ms "
				"and logged after 10 seconds, then every minute while the most taken or the refusals change." :
				"DIAGNOSTIC: the ray tracing buffer of moving geometry is measured, its frames not sampled: the rest is logged after 10 seconds, then every minute while it changes.");
		}
	}

	void InitRayTracing()
	{
		const int requested = GetNumber(kDynamicBvhVertices);
		const bool diagnostic = IsDiagnosticMode();
		if (requested == 0)
		{
			WriteLog(kTag, "DYNAMIC_BVH_VERTICES=0 - the ray tracing buffer of moving geometry is not raised.");
			if (!diagnostic)
				return;	// nothing looked for, nothing written
		}

		// 1. every site found and checked - a job not done is a problem, a measurement not made a line
		Sites sites;
		std::string why;
		if (!FindAndCheck(sites, why))
		{
			if (requested != 0)
				ReportFailure(kTag, kJob + why);
			if (diagnostic)
				WriteLog(kTag, kNotMeasured + why);
			return;
		}

		// 2. the raise, in one write; 3. with DIAGNOSTIC_MODE the measurement, of the buffer as the part leaves it
		const uint32_t capacity = requested != 0 ? Raise(sites, requested) : sites.vertices;
		if (diagnostic)
			Measure(sites, capacity);
	}
}
