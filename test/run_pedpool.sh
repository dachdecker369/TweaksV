#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The ped pool, a limit measured first (pedpool.cpp): with DIAGNOSTIC_MODE=1 the fake game's sites found, each once, checked and logged, the
# pool read from behind the game's obfuscation, whose accessor the fake runs over its own encoding, and its numbers summed up 10 seconds in and
# again when they change: the slots in use, the most a sample of the sampling thread saw, the mission and reuse peds of a walk of the slots, the
# population's counts and caps, the spatial array; the pool not made yet and made later; full, past the fixed arrays, both; the game's number
# not the pool's; each thing a read can find changed or gone, the walk bounded by a count outside its limits; each pattern missing and each check
# failing on its own, each logged, never a problem, nothing measured; each thread refused; the plain game; DRY_RUN, which changes nothing; the
# budget's pattern found once through its part, whatever that part did; without DIAGNOSTIC_MODE nothing looked for. In every run the sites as
# placed: the module writes nothing. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=92
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# What the fake reports of its sites and its pool
PLACED='ped pool: sites as placed, not made |'
MADE='ped pool: sites as placed, made, its accessor decodes it |'
# The module's lines, word for word (an apostrophe as any character)
pp_line() { log_line "\[PedPool\] $1\$"; }
SUMMARY='DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 165.'
NOT_MEASURED='DIAGNOSTIC: the ped pool is not measured:'
ANNOUNCED='DIAGNOSTIC: the ped pool is measured, through its pointer at RVA 0x[0-9A-F]* (N-E-A1): the slots in use and the spatial array are sampled every second and logged after 10 seconds, then every minute while they change.'
ANNOUNCED_UNSAMPLED='DIAGNOSTIC: the ped pool is measured, through its pointer at RVA 0x[0-9A-F]* (N-E-A1), its numbers not sampled: the slots in use and the spatial array are logged after 10 seconds, then every minute while they change.'
# the line that comes at once, at the first sample past the game's fixed arrays
PAST='DIAGNOSTIC: the ped pool.s spatial array holds 300 of 300 - past the 256 of the game.s fixed arrays: expect memory corruption.'
PAST_ANY='\[PedPool\] DIAGNOSTIC: the ped pool.s spatial array holds'
RVA='RVA 0x[0-9A-F]*'
OUTSIDE='0x[0-9A-F]* (outside the game)'

# The runs, side by side, each in a folder of its own: pp NAME INI_CHANGES [OPTION...], in the background, at most half as many at a time as the suites; each
# with DIAGNOSTIC_MODE=1 unless its changes say otherwise. pp_in NAME takes its checks there, once they have all ended.
game_folder=$B
running=0
pp() {
	local name=$1 change=$2; shift 2
	if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
	running=$((running + 1))
	(
		B=$game_folder/pp/$name
		mkdir -p "$B" && cd "$B" || exit 1
		cp ../../TweaksV.asi ../../TweaksV_test.asi .
		fresh_ini; setkey DIAGNOSTIC_MODE 1; eval "$change"
		game ../../full_game.exe "$@" > /dev/null
	) &
}
pp_in() { B=$game_folder/pp/$1; cd "$B" || exit 1; out=$(tr -d '\r' < harness.out); }
# the patterns and the checks, a run each
LABELS="N-E-A1 N-E-M1 N-E-M2 N-E-C1 N-E-I1 N-E-S1 H-E2 N-E-K1 N-E-K2 N-E-R1 N-E-R2 N-E-R3 N-E-R4 N-E-D1 N-E-D2 N-E-D3 N-E-D4 N-E-D5 N-E-D6"
declare -A BAD
BAD[ppflagout]="the pool.s flag at $OUTSIDE is not inside the game (N-E-A1)."
BAD[ppvalueodd]="the pool.s value at $RVA is not aligned to 8 bytes (N-E-A1)."
BAD[ppkeysno]="the pool.s keys at $RVA cannot be read (N-E-A1)."
BAD[ppproxyout]="the pool allocator.s proxy at $OUTSIDE is not inside the game (N-E-M1)."
BAD[pplookupout]="the pool.s lookup call at $RVA is not a call into the game (another mod may have hooked it already) (N-E-M2)."
BAD[ppfilelookup]="the number.s lookup call at $RVA leads to $RVA, not the pool.s lookup at $RVA (N-E-C1)."
BAD[ppsetterout]="the number.s setter call at $RVA is not a call into the game (another mod may have hooked it already) (N-E-C1)."
BAD[ppsetterno]="the number.s setter at $RVA cannot be read (N-E-C1)."
BAD[ppsetterfirst]="the number.s setter at $RVA holds 8B 0D [0-9A-F ]* C3, not 89 0D, a disp32 and C3 (N-E-C1)."
BAD[ppsettersecond]="the number.s setter at $RVA holds 89 15 [0-9A-F ]* C3, not 89 0D, a disp32 and C3 (N-E-C1)."
BAD[ppsetterbytes]="the number.s setter at $RVA holds 89 0D [0-9A-F ]* CC, not 89 0D, a disp32 and C3 (N-E-C1)."
BAD[ppnumberodd]="the game.s number of peds at $RVA is not aligned to 4 bytes (N-E-C1)."
BAD[ppintelligence]="PedIntelligence.s lookup call at $RVA leads to $RVA, not the pool.s lookup at $RVA (N-E-I1)."
BAD[ppstore]="the spatial array.s store at $RVA holds 4C 89 1D, not 48 89 1D (N-E-S1)."
BAD[ppspatialout]="the spatial array.s pointer at $OUTSIDE is not inside the game (N-E-S1)."
BAD[ppstoreunread]="the spatial array.s store at $RVA cannot be read (N-E-S1)."
BAD[ppcountout]="the count of scenario peds on foot at $OUTSIDE is not inside the game (N-E-K1)."
BAD[ppcapodd]="the cap of scenario peds at $RVA is not aligned to 4 bytes (N-E-K2)."
BAD[ppgetterout]="the copy.s first getter call at $RVA is not a call into the game (another mod may have hooked it already) (N-E-R1)."
BAD[ppgetters]="the copy.s two getter calls at $RVA and $RVA lead to $RVA and $RVA (N-E-R1)."
BAD[ppresetout]="the timeslice list.s count at $OUTSIDE is not inside the game (N-E-R2)."
BAD[ppfiller]="the timeslice list.s reset.s call at $RVA leads to $RVA, not the filler at $RVA (N-E-R3)."
BAD[ppfillerout]="the filler.s call at $RVA is not a call into the game (another mod may have hooked it already) (N-E-R3)."
BAD[ppfillerunread]="the filler.s call at $RVA cannot be read (N-E-R3)."
BAD[pprestore]="the priority function.s restore at $RVA holds 48 81 C4 49 10 00 00, not 48 81 C4 48 10 00 00 (N-E-R4)."
BAD[pprestoreunread]="the priority function.s restore at $RVA cannot be read (N-E-R4)."
BAD[pppointer]="the spatial array.s pointer the priority function reads, $RVA, is not the one its maker stores, $RVA (N-E-R4)."

rm -rf pp
pp off 'setkey DIAGNOSTIC_MODE 0' pprun wait
pp plain_run : pprun wait
pp notmade : wait
pp dry 'setkey DRY_RUN 1' pprun wait
pp full : pprun ppfull wait
pp over : pprun ppspatialover wait
pp both : pprun ppfull ppspatialover wait
pp number : pprun ppnumber wait
pp stride : pprun ppstride wait
pp countzero : pprun ppcountzero wait
pp countbig : pprun ppcountbig wait
pp inusebig : pprun ppinusebig wait
pp inusepeak : pprun ppinusepeak wait
pp spatialfault : pprun ppspatialfault wait
pp spatialbig : pprun ppspatialbig wait
pp noflags : pprun ppnoflags wait
pp nopeds : pprun ppnopeds wait
pp nospatial : pprun ppnospatial wait
pp badspatial : pprun ppbadspatial wait
pp freed : pprun ppfreed wait
pp gone : pprun ppgone wait
pp heapoff 'setkey ENABLED 0 HEAP_ADJUSTER' pprun wait
pp heapmod : pprun pmraised wait
pp more 'export ASI=TweaksV_test.asi TWEAKSV_TEST_SUMMARY_MS=2000' pprun ppmore
pp late 'export ASI=TweaksV_test.asi TWEAKSV_TEST_SUMMARY_MS=2000' pprun pplate
pp sampling 'export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 ped pool sampling"' pprun wait
pp summary 'export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 ped pool summary"' pprun
pp diagoff 'setkey DIAGNOSTIC_MODE 0; export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 ped pool summary"'
pp diagoff2 'setkey DIAGNOSTIC_MODE 0; export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 ped pool sampling"'
for label in $LABELS; do pp "missing-$label" "export ASI=TweaksV_test.asi; addkey TEST_BREAK_PATTERN $label"; done
for option in "${!BAD[@]}"; do pp "$option" : "$option"; done
wait

echo "== N1 DIAGNOSTIC_MODE=0: nothing looked for, nothing measured"
pp_in off; echo "      $out"
check "no pattern of the ped pool looked for, and no line of its own in a run past where its first summary would come" "log_lacks 'N-E-' && log_lacks '\[PedPool\]'"
check "the sites as placed, the pool as the fake made it; no problem of its own; every page its own protection" "has '$MADE' && log_lacks '\[PedPool\] PROBLEM' && has 'protect: all restored (RX)'"

echo "== N2 DIAGNOSTIC_MODE=1: every site found, checked and logged; the numbers 10 s in"
pp_in plain_run; echo "      $out"
check "the 18 patterns of its own, each once, and the budget's" "[ \$(grep -c -E 'Pattern N-E-[A-Z][0-9] found: .* 1 match,' TweaksV.log) -eq 18 ] && [ \$(count 'Pattern H-E2 found') -eq 1 ]"
check "the pointer's three fields" "pp_line 'DIAGNOSTIC: the pool.s pointer.s decode at $RVA: its flag at $RVA, its value at $RVA, its keys at $RVA (N-E-A1)'"
check "the maker's call, its stride" "pp_line 'DIAGNOSTIC: the pool.s maker at $RVA: the pool allocator.s proxy at $RVA, slots of 0x1980 bytes (N-E-M1)'"
check "the three lookups, one function; the setter and the number" "pp_line 'DIAGNOSTIC: the pool.s lookup call at $RVA -> $RVA (N-E-M2)' && pp_line 'DIAGNOSTIC: the number.s lookup call at $RVA -> $RVA, its setter call at $RVA -> $RVA, the number at $RVA (N-E-C1)' && pp_line 'DIAGNOSTIC: PedIntelligence.s lookup call at $RVA -> $RVA (N-E-I1)'"
check "the spatial array's pointer; the budget a raised pool draws on, as its part left it" "pp_line 'DIAGNOSTIC: the spatial array.s capacity from the pool.s slots at $RVA, its pointer.s store at $RVA -> $RVA (N-E-S1)' && pp_line 'DIAGNOSTIC: the pool memory budget a raised pool draws on, at $RVA, holds 0x10000000 bytes (H-E2)'"
check "the counts and the caps" "pp_line 'DIAGNOSTIC: the population.s counts at $RVA: ambient in cars at $RVA, on foot at $RVA; scenario in cars at $RVA, on foot at $RVA (N-E-K1)' && pp_line 'DIAGNOSTIC: the population.s caps at $RVA: ambient on foot at $RVA and in cars at $RVA, which is which a hint, scenario at $RVA (N-E-K2)'"
check "the three frames of a raise to come" "pp_line 'DIAGNOSTIC: the copy.s getter calls at $RVA and $RVA -> $RVA, the clamp.s undoing at $RVA: 8B 03 31 FF 85 C0 0F 4F F8 89 7E 10 (N-E-R1)' && pp_line 'DIAGNOSTIC: the timeslice list.s reset at $RVA: its count at $RVA, its call -> $RVA (N-E-R2)' && pp_line 'DIAGNOSTIC: the filler.s frame at $RVA: its function at $RVA, its call at $RVA -> $RVA (N-E-R3)' && pp_line 'DIAGNOSTIC: the priority function.s frame at $RVA holds 0x1048, given back at $RVA, the spatial array.s pointer at $RVA (N-E-R4)'"
check "the six defaults, their numbers" "pp_line 'DIAGNOSTIC: fwAnimDirector.s pool.s default at $RVA holds 1024 (N-E-D1)' && pp_line 'DIAGNOSTIC: the Creature.s pool.s default at $RVA holds 1024 (N-E-D2)' && pp_line 'DIAGNOSTIC: the MotionTree.s pool.s default at $RVA holds 2389 (N-E-D3)' && pp_line 'DIAGNOSTIC: the Expressions. pool.s default at $RVA holds 682 (N-E-D4)' && pp_line 'DIAGNOSTIC: the RagDoll.s pool.s default at $RVA holds 341 (N-E-D5)' && pp_line 'DIAGNOSTIC: the FacialRig.s pool.s default at $RVA holds 341 (N-E-D6)'"
check "the announcement, word for word" "pp_line '$ANNOUNCED'"
check "the summary, word for word: the settled numbers, the most the sampling thread saw before them, skipped peds and free slots' stale bytes not counted, the top bits of the slots in use masked; once" "pp_line '$SUMMARY' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -eq 1 ]"
check "the fake's own encoding is the game's: its accessor decodes it; the sites as placed, every page its own protection" "has '$MADE' && has 'protect: all restored (RX)'"
check "the spatial array never past 256: no line of it at once" "log_lacks '$PAST_ANY'"
check "a measurement reports nothing: no problem of its own" "log_lacks '\[PedPool\] PROBLEM'"

echo "== N3 nothing made yet: said so, nothing read past the flag"
pp_in notmade; echo "      $out"
check "the module's line, the sites as placed" "pp_line 'DIAGNOSTIC: no ped pool yet - the game has not made it.' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -eq 0 ] && has '$PLACED'"

echo "== N4 DRY_RUN changes nothing here"
pp_in dry
check "the same lines, the same numbers" "pp_line '$ANNOUNCED' && pp_line '$SUMMARY' && has '$MADE' && log_lacks '\[PedPool\] DRY_RUN'"

echo "== N5 the pool full, the spatial array past the game's fixed arrays, each said at the end of the line"
pp_in full
check "at most the pool's slots: the pool has been full" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 256 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 165 - the pool has been full: the game makes no more peds until one goes.'"
pp_in over
check "at most 300 in the spatial array: past the 256 of the game's fixed arrays" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 300, at most 300 - past the 256 of the game.s fixed arrays: expect memory corruption.'"
check "said at once by the sampling thread, word for word, once, before the first summary, as what follows may end the game before it" "pp_line '$PAST' && [ \$(count '$PAST_ANY') -eq 1 ] && [ \$(grep -n '$PAST_ANY' TweaksV.log | cut -d: -f1) -lt \$(grep -n '\[PedPool\] DIAGNOSTIC: ped pool: ' TweaksV.log | head -n 1 | cut -d: -f1) ]"
pp_in both
check "both" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 256 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 300, at most 300 - the pool has been full: the game makes no more peds until one goes - past the 256 of the game.s fixed arrays: expect memory corruption.'"

echo "== N6 what a read can find changed: each said, no crash, no problem"
pp_in number
check "the game's number not the pool's: its line, then the numbers" "pp_line 'DIAGNOSTIC: ped pool: the game.s number of peds, 300, is not the pool.s 256 slots - a mod changed one of them, and a pool larger than the number overruns two of the game.s arrays.' && pp_line '$SUMMARY'"
pp_in plain_run
check "the two agreeing: no such line" "log_lacks '\[PedPool\] DIAGNOSTIC: ped pool: the game.s number of peds, '"
pp_in stride
check "slots of another size: not read" "pp_line 'DIAGNOSTIC: the ped pool at $OUTSIDE has slots of 0x1000 bytes, not 0x1980: it is not read (another mod may have made it, or the game has changed).' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -eq 0 ]"
pp_in countzero
check "a count of 0: taken for data being changed, nothing walked" "pp_line 'DIAGNOSTIC: the ped pool at $OUTSIDE could not be read this time: its slots, 0, are outside 1..16384 (the game may have been changing it).'"
pp_in countbig
check "a count of 20000: taken for data being changed, nothing walked" "pp_line 'DIAGNOSTIC: the ped pool at $OUTSIDE could not be read this time: its slots, 20000, are outside 1..16384 (the game may have been changing it).' && has 'protect: '"
pp_in inusebig
check "more in use than it has slots, in every reading: taken for data being changed" "pp_line 'DIAGNOSTIC: the ped pool at $OUTSIDE could not be read this time (the game may have been changing it).' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -eq 0 ]"
pp_in inusepeak
check "257 in use only until it settles: none of it kept for the most, which says 143 and not that the pool has been full" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 143 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 165.' && log_lacks '\[PedPool\] .*has been full'"
pp_in noflags
check "the flags that cannot be read: the walk given up, the rest of the line as it is" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission and reuse not counted this time; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 165.'"
pp_in nopeds
check "the peds that cannot be read: the walk given up, the rest of the line as it is" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission and reuse not counted this time; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 165.'"
pp_in nospatial
check "the spatial array not made" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array not made yet.'"
pp_in badspatial
check "a count past its capacity: not read, and not kept for the most" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array not read this time.'"
check "nor said at once" "log_lacks '$PAST_ANY' && log_lacks '\[PedPool\] .*past the 256'"
pp_in spatialfault
check "the spatial array's head that faults: not read this time, no crash" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array not read this time.' && has 'protect: '"
pp_in spatialbig
check "a capacity of 20000: taken for data being changed, its 300 kept neither for the most nor said at once" "pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array not read this time.' && log_lacks '$PAST_ANY' && log_lacks '\[PedPool\] .*past the 256'"
pp_in freed
check "the pool's head freed: could not be read this time, no crash" "pp_line 'DIAGNOSTIC: the ped pool at $OUTSIDE could not be read this time (the game may have been changing it).' && has 'protect: '"
pp_in gone
check "the pointer's fields no longer readable: said, no crash" "pp_line 'DIAGNOSTIC: the ped pool.s pointer at $RVA cannot be read.' && has 'protect: '"

echo "== N7 the budget's pattern through its part: found once whatever the part did"
pp_in heapoff; echo "      $out"
check "[HEAP_ADJUSTER] ENABLED=0: the pattern looked for all the same, once, for the module's line; the budget the game's own; no line of the part; measured" "[ \$(count 'Pattern H-E2 found') -eq 1 ] && pp_line 'DIAGNOSTIC: the pool memory budget a raised pool draws on, at $RVA, holds 0x08800000 bytes (H-E2)' && log_lacks '\[HeapAdjuster\]' && pp_line '$SUMMARY'"
pp_in heapmod
check "another mod's budget: what it holds, the measurement not stopped; the part's line of it, the pattern once" "[ \$(count 'Pattern H-E2 found') -eq 1 ] && pp_line 'DIAGNOSTIC: the pool memory budget a raised pool draws on, at $RVA, holds 0x0C800000 bytes (H-E2)' && pp_line '$SUMMARY' && log_has 'another mod or a game update already set it to 200 MB'"

echo "== N8 the test build, its summaries every 2 s: again when the numbers change, and only then; the pool made late"
pp_in more; echo "      $out"
check "the test build's line for this measurement; the first summary, then a second with the new numbers, no third" "log_line '\[TweaksV\] TEST BUILD: the ped pool summary comes every 2000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=2000).\$' && pp_line '$SUMMARY' && pp_line 'DIAGNOSTIC: ped pool: 150 of 256 slots in use, at most 171 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 140 of 256, at most 165.' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -eq 2 ]"
pp_in late
check "the pool made 11 s in: not made yet at the first summary, its numbers once it settled" "pp_line 'DIAGNOSTIC: no ped pool yet - the game has not made it.' && [ \$(count 'no ped pool yet') -eq 1 ] && pp_line '$SUMMARY' && [ \$(count '\[PedPool\] DIAGNOSTIC: ped pool: ') -le 2 ]"

echo "== N9 a thread that cannot start (the test build's TWEAKSV_TEST_REFUSE_THREAD): logged, not reported"
pp_in sampling
check "no sampling thread: said, and measured all the same, the most so far those of the summaries" "pp_line 'DIAGNOSTIC: the ped pool is not sampled, so its most so far are those of the summaries: its thread could not be started.' && pp_line '$ANNOUNCED_UNSAMPLED' && log_lacks '\[PedPool\].*sampled every second' && pp_line 'DIAGNOSTIC: ped pool: 143 of 256 slots in use, at most 143 so far; mission 12, reuse 4; scenario 40 on foot and 2 in cars, cap 40; ambient 61 on foot and 18 in cars, caps 64 and 16 (which is which a hint); spatial array 139 of 256, at most 139.' && log_lacks '\[PedPool\] PROBLEM'"
pp_in summary
check "no summary's thread: not measured, nothing announced, no problem" "pp_line '$NOT_MEASURED its summary thread could not be started.' && log_lacks '\[PedPool\] DIAGNOSTIC: the ped pool is measured' && log_lacks '\[PedPool\] PROBLEM'"
pp_in diagoff
check "DIAGNOSTIC_MODE=0: no start of its summary's thread, which the test build would refuse and say so" "log_has 'TWEAKSV_TEST_REFUSE_THREAD=1 ped pool summary' && log_lacks 'The ped pool summary thread could not be started' && log_lacks 'N-E-'"
pp_in diagoff2
check "DIAGNOSTIC_MODE=0: no start of its sampling thread, likewise" "log_has 'TWEAKSV_TEST_REFUSE_THREAD=1 ped pool sampling' && log_lacks 'The ped pool sampling thread could not be started' && log_lacks 'N-E-'"

echo "== N10 each pattern missing (the test build's TEST_BREAK_PATTERN), and each check failing on its own: its line, nothing measured, no problem of its own, the sites as placed"
for label in $LABELS; do
	pp_in "missing-$label"
	check "$label missing" "log_has 'TEST BUILD: pattern $label is treated as not found' && pp_line '$NOT_MEASURED $label was not found.' && log_lacks '\[PedPool\] DIAGNOSTIC: the ped pool is measured' && log_lacks '\[PedPool\] PROBLEM' && has '$PLACED'"
done
for option in ppflagout ppvalueodd ppkeysno ppproxyout pplookupout ppfilelookup ppsetterout ppsetterno ppsetterfirst ppsettersecond ppsetterbytes ppnumberodd ppintelligence ppstore ppspatialout ppstoreunread ppcountout ppcapodd ppgetterout ppgetters ppresetout ppfiller ppfillerout ppfillerunread pprestore pppointer pprestoreunread; do
	pp_in "$option"
	check "$option: ${BAD[$option]}" "pp_line \"\$NOT_MEASURED \${BAD[$option]}\" && log_lacks '\[PedPool\] DIAGNOSTIC: the ped pool is measured' && log_lacks '\[PedPool\] PROBLEM' && has '$PLACED'"
done
B=$game_folder; cd "$B" && rm -rf pp

echo "== N11 the plain game: no pattern, each missing one named by the scanner, the first by the module's line, no crash"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "the module's line, the scanner's for each of the 18, no problem of its own" "grep -q 'loaded' harness.out && pp_line '$NOT_MEASURED N-E-A1 was not found.' && [ \$(grep -c -E 'Pattern N-E-[A-Z][0-9] not found' TweaksV.log) -eq 18 ] && log_lacks '\[PedPool\] PROBLEM'"

echo
echo "PED POOL SCENARIOS: $pass passed, $fail failed"
