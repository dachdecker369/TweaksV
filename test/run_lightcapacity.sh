#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The light stores and the lighting capacity (TweaksVLightCapacity). The measurement, with the part off, as base.ini has it: with DIAGNOSTIC_MODE=1
# the fake game's 24 sites found, checked and logged, the counters sampled and summed up 10 seconds in and again when they change, its one write - the
# jumps when a job's list and the ray traced list are full through counting stubs and a job's two copies through stubs that keep its most, while threads run
# that code at once, the counts exact - and nothing else; every number at its room and past it, the write pointers outside their stores, each line once and no box; the siren settings' numbers on none, 0, 33 and 255 settings and an
# array that takes no reads; each pattern missing, each check failing on its own, a number less for the three sites that cost only their number, a jump another mod redirected, the plain game, DRY_RUN and Windows' dynamic
# code policy - each logged, never a problem, nothing written; without DIAGNOSTIC_MODE nothing looked for. The part, on as the default file has it: its one
# write, every byte of it and nothing else, every page its own protection; a job's lists as a movaps loads them, a frame past the game's numbers and the
# index read back, the sites run as code; the measurement over the moved stores, from what the part found; DRY_RUN, frames that have begun, memory or a page
# Windows refuses, each check and each pattern again - each a problem with nothing written; the sirens' question through it all. Fake LOCALAPPDATA: the
# real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=293
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# What the fake reports: the sites but the jump's rel32 as placed, and where the jump leads
COUNTS_PLACED='lc counts: ray jump as placed, corona copy as placed, light copy as placed |'
PLACED="lc sites: as placed, jump as placed | $COUNTS_PLACED"
STUB='lc sites: as placed, jump to a stub, RX, on to its way out, its counter RW, holding '
COPIES_STUBS='corona copy to a stub, RX, on to the copy, its most RW, light copy to a stub, RX, on to the copy, its most RW |'
COUNTS_STUBS="lc counts: ray jump to a stub, RX, on to its way out, its counter RW, holding 0, $COPIES_STUBS"
JOBS='lc jobs: added 160, refused 199840'
# The part's: every byte it writes as it must and every other as placed; the stores and the start values the sites name; a job's lists as its movaps loads them
RAISED="lc sites: raised, jump as placed | $COUNTS_PLACED"
RAISED_STUB='lc sites: raised, jump to a stub, RX, on to its way out, its counter RW, holding '
MOVED='lc stores: moved, coronas RW, lights RW, the drain names the same, the start values read-only, loaded 0 0 300 180 |'
STORES_PLACED='lc stores: as placed |'
RAISED_JOBS='lc jobs: added 360, refused 199640'
part_ini() { STORES_ON=1 fresh_ini; }
# The module's lines, word for word (an apostrophe as any character)
lc_line() { log_line "\[LightCapacity\] $1\$"; }
lc_lacks() { log_lacks "\[LightCapacity\] .*$1"; }
NOT_MEASURED='DIAGNOSTIC: the lighting capacity is not measured:'
OUT='0x[0-9A-F]* (outside the game)'
NOT_RAISED='PROBLEM: The light stores are not raised this session:'
STATUS_CORONAS='The vehicle coronas. store moved and its limit raised from 928 to 18000 entries.'
STATUS_LIGHTS='The vehicle lights. store moved and its limit raised from 400 to 10800 entries.'
STATUS_LISTS='A job.s lists patched: 80 lights and 200 coronas -> 180 lights and 300 coronas.'
STATUS_INDEX='Scene store.s index patched: 88 04 CA -> 89 04 CA, 0F B6 1C C1 -> 8B 1C C1 90.'
SIREN_REFUSAL='\[SirenSettings\] PROBLEM: The sirens per vehicle are not raised this session: the light stores are not raised (\[LIGHTCAPACITY\] in TweaksV.ini), and the game.s own do not take 32 sirens a vehicle. Vehicles keep 20 sirens.$'
MEASURED='DIAGNOSTIC: the lighting capacity is measured: its numbers are sampled every 10 ms and logged after 10 seconds, then every minute while they change.'
REDIRECTED='DIAGNOSTIC: the jump at RVA 0x[0-9A-F]* (G-E-C1) is redirected to a counting stub: every light a job.s full list refuses is counted.'
SITES='\[LightCapacity\] DIAGNOSTIC: .*(\(N-1, \)\?G-E-[A-H][1-4])\( -> game size .*\)\?$'
site_lines() { tr -d '\r' < TweaksV.log | grep -c -- "$SITES"; }
N='[0-9]*'
FIRST_UNCOUNTED="vehicles 87 of 300, vehicle lights 143 of 400, vehicle coronas 512 of 928, scene lights 811 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 640 of 1024 (past 512 in $N samples, by 128 at most), reflection lists 212 of 768, coronas 900 of 1088"
FIRST_NUMBERS=${FIRST_UNCOUNTED/by 128 at most)/by 128 at most, 0 refused)}
MOST_0='a job.s most 0 of 80 lights and 0 of 200 coronas'
SIRENS_24='siren settings 24, real lights in 0, up to 0 lights.'
SO_FAR='\[LightCapacity\] DIAGNOSTIC: the most so far'
THIS_MINUTE='\[LightCapacity\] DIAGNOSTIC: the most this minute'
# An RVA from the fake's report: rva 'corona pointer'; rva_plus NAME N: that RVA plus N
rva() { sed -n 's/.*lc rvas: \([^|]*\) |.*/, \1/p' <<< "$out" | sed -n "s/.*, $1 \(0x[0-9A-F]*\).*/\1/p"; }
rva_plus() { printf '0x%X' $(( $(rva "$1") + $2 )); }
# in_order RVA...: "RVA 0x1, RVA 0x2 and RVA 0x3", in address order, as a scan finds them, since the linker lays the fake's sites out as it likes
in_order() {
	local rvas=($(for a in "$@"; do printf '%d\n' "$a"; done | sort -n)) text="" i
	for ((i = 0; i < ${#rvas[@]}; i++)); do
		[ "$i" = 0 ] || { [ "$i" = $(( ${#rvas[@]} - 1 )) ] && text+=" and " || text+=", "; }
		text+=$(printf 'RVA 0x%X' "${rvas[i]}")
	done
	echo "$text"
}

echo "== G1 DIAGNOSTIC_MODE=0, the part off: nothing looked for, nothing written"
fresh_ini; out=$(game full_game.exe lcnumbers lcjobs lcframe lcindex); echo "      $out"
stubs_off=$(grep -o 'stubs: [0-9]*, [^|]*' <<< "$out")
check "no pattern of the part looked for, no line of the part" "log_lacks 'G-E-' && log_lacks '\[LightCapacity\]'"
check "the sites and the jump as placed, the lists' calls through the game's own jump; every page its own protection" "has '$PLACED' && has '$STORES_PLACED' && has '$JOBS' && has 'protect: all restored (RX)'"
check "the game's own: the frame past its numbers not run on its stores, and its bug, the index past a byte read back as its low byte, the sites run as code" "has 'lc frame: not run |' && has 'lc index: 700 read back as 188 |'"

echo "== G2 DIAGNOSTIC_MODE=1: the sites found and logged, the numbers sampled, the jump through its stub while two threads run it, the summary 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcnumbers lcjobs wait); echo "      $out"
check "the 21 patterns each found exactly once, G-E-G1 twice" "[ \$(grep -c -E 'Pattern G-E-[A-H][1-4] found: .* 1 match,' TweaksV.log) -eq 21 ] && log_has 'Pattern G-E-G1 found: .* 2 matches,'"
check "the frame's reset and the stores, word for word, at the fake's RVAs" "lc_line 'DIAGNOSTIC: the frame.s reset at RVA $(rva reset): the vehicle coronas. store at RVA $(rva coronas), their write pointer at RVA $(rva 'corona pointer'), the vehicle lights. store at RVA $(rva lights), their write pointer at RVA $(rva 'light pointer') (G-E-A1)'"
check "the drain's two, the second with its call past the match" "lc_line 'DIAGNOSTIC: the drain.s coronas at RVA $(rva 'drain coronas'): their store at RVA $(rva coronas), their write pointer at RVA $(rva 'corona pointer') (G-E-A2)' && lc_line 'DIAGNOSTIC: the drain.s lights at RVA $(rva 'drain lights'): their store at RVA $(rva lights), their write pointer at RVA $(rva 'light pointer'), add-scene-light.s call at RVA $(rva_plus 'drain lights' 32) -> RVA $(rva called) (G-E-A3)'"
check "a job's 5 vehicles, its disp32 counted from past the imm32, and the two kicks' scratch" "lc_line 'DIAGNOSTIC: a job.s 5 vehicles. test at RVA $(rva 'job test'): the vehicles. count at RVA $(rva vehicles) (G-E-B1)' && lc_line 'DIAGNOSTIC: a job.s scratch size in its kick at RVA $(rva kick) holds 0xB180 (G-E-B2)' && lc_line 'DIAGNOSTIC: a job.s scratch size in its kick at RVA $(rva 'last kick') holds 0xB180 (G-E-B3)'"
check "add-to-a-job's-list, its jump when full and the 12 bytes it leads to" "lc_line 'DIAGNOSTIC: add-to-a-job.s-list.s check at RVA $(rva_plus add 11), no unwind data; its jump when full at RVA $(rva_plus add 17) -> RVA $(rva_plus add 37): 48 83 C4 20 5B 5F 5E 41 5E 41 5F C3 (G-E-C1)'"
check "a job's scratch and its lists' start values" "lc_line 'DIAGNOSTIC: a job.s scratch at RVA $(rva scratch): 0x2580 bytes of coronas, 0x8C00 of lights, 0xB180 in all (G-E-C2)' && lc_line 'DIAGNOSTIC: a job.s lights. bytes at RVA $(rva 'scratch lights') hold 0x8C00 (G-E-C3)' && lc_line 'DIAGNOSTIC: a job.s lists. start values at RVA $(rva 'start values'): 0, 0, 200, 80 (G-E-C4)'"
check "the scene store's add, with the sort list's count and mark, and the byte read back" "lc_line 'DIAGNOSTIC: the scene store.s limit at RVA $(rva_plus 'scene add' 9) holds 1023 (N-1, G-E-D1) -> game size 1024: its count at RVA $(rva 'scene count'), its sort list at RVA $(rva 'sort list'), the sort list.s count at RVA $(rva 'sort count'), its mark at RVA $(rva 'sort mark'), an index stored as a byte at RVA $(rva_plus 'scene add' 42): 88 04 CA' && lc_line 'DIAGNOSTIC: the sort list.s index read as a byte at RVA $(rva 'byte read'): 0F B6 1C C1 (G-E-D2)'"
check "the scene store's two sides, one store" "lc_line 'DIAGNOSTIC: the scene store.s render side at RVA $(rva render): the store at RVA $(rva store), its render pointer at RVA $(rva 'render pointer'), the buffers. counts at RVA $(rva 'buffer counts'), the render count at RVA $(rva 'render count'), the reflection lists at RVA $(rva reflections), their render pointer at RVA $(rva 'reflection pointer') (G-E-E1)' && lc_line 'DIAGNOSTIC: the scene store.s update side at RVA $(rva update): the store at RVA $(rva store), its current pointer at RVA $(rva 'current pointer') (G-E-E2)'"
check "the ray traced lists, their check, their upload and the reflection lists' check" "lc_line 'DIAGNOSTIC: the ray traced lists at RVA $(rva store), 0x72010 bytes each, their current pointer at RVA $(rva 'ray pointer') (G-E-E3)' && lc_line 'DIAGNOSTIC: the ray traced list.s limit at RVA $(rva_plus 'ray check' 14) holds 1023 (N-1, G-E-F1) -> game size 1024, its current pointer at RVA $(rva 'ray pointer')' && lc_line 'DIAGNOSTIC: the ray traced upload at RVA $(rva upload): 512 lights at 0x70E0 of its object, the builder.s call at RVA $(rva_plus upload 20) -> RVA $(rva called) (G-E-F2)' && lc_line 'DIAGNOSTIC: the reflection lists. limit at RVA $(rva_plus 'reflection check' 7) holds 767 (N-1, G-E-F3) -> game size 768'"
low=$(rva_plus 'corona check' 9); high=$(rva_plus 'corona check too' 9); [ $((low)) -lt $((high)) ] || { low=$(rva_plus 'corona check too' 9); high=$(rva_plus 'corona check' 9); }
check "the corona store, its two checks with one count; the drain's add; the worker's frame and a job's scratch in it" "lc_line 'DIAGNOSTIC: the corona store.s two limits at RVA $low and RVA $high hold 1087 (N-1, G-E-G1) -> game size 1088, their counts at RVA $(rva 'corona count') and RVA $(rva 'corona count')' && lc_line 'DIAGNOSTIC: the drain.s add to the corona store at RVA $(rva 'corona add'): its count at 0xCC00 of the store, its limit at RVA $(rva_plus 'corona add' 10) holds 1087 (N-1, G-E-G2) -> game size 1088' && lc_line 'DIAGNOSTIC: the worker.s frame at RVA $(rva 'worker frame') holds 0x18058 (G-E-H1)' && lc_line 'DIAGNOSTIC: a job.s scratch from the worker.s rsp+0x50 at RVA $(rva 'worker call') (G-E-H2)'"
check "each store's references, the reset's, the drain's and the constructor's, in address order, by the scan" "lc_line \"DIAGNOSTIC: the vehicle coronas. store at RVA $(rva coronas) is referenced in 3 places of the game.s code: $(in_order $(rva reset) $(rva 'drain coronas') $(rva constructor))\" && lc_line \"DIAGNOSTIC: the vehicle lights. store at RVA $(rva lights) is referenced in 3 places of the game.s code: $(in_order $(rva_plus reset 21) $(rva 'drain lights') $(rva_plus constructor 7))\""
check "every site logged once, 22 lines" "[ \$(site_lines) -eq 22 ]"
check "the one write: the jump's old bytes and new, once; its announcement and the measurement's, word for word" "[ \$(count 'jump when a job.s list is full at RVA') -eq 1 ] && lc_line 'DIAGNOSTIC: jump when a job.s list is full at RVA $(rva_plus add 17): 0F 83 0E 00 00 00 -> 0F 83\( [0-9A-F][0-9A-F]\)\{4\}' && lc_line '$REDIRECTED' && lc_line '$MEASURED'"
check "the jump leads to a stub, read+execute, on to the way out, its counter on a read/write page: both threads' 199,840 refusals counted exactly, the rest of the sites as placed" "has '${STUB}199840 |' && has '$JOBS'"
check "the ray traced list's jump and a job's two copies, word for word, at the fake's RVAs; G-E-K1 and G-E-K2 each found once" "lc_line 'DIAGNOSTIC: the jump when the ray traced list is full at RVA $(rva_plus 'ray check' 18) (G-E-F1) -> RVA $(rva 'ray way out'): 48 83 C4 28 5B 5F 5E 41 5C 41 5E 41 5F C3' && lc_line 'DIAGNOSTIC: a job.s coronas. copy at RVA $(rva 'corona end'): its call at RVA $(rva_plus 'corona end' 10) -> RVA $(rva copy) (G-E-K1)' && lc_line 'DIAGNOSTIC: a job.s lights. copy at RVA $(rva 'light end'): its call at RVA $(rva_plus 'light end' 15) -> RVA $(rva copy) (G-E-K2)' && [ \$(grep -c -E 'Pattern G-E-K[12] found: .* 1 match,' TweaksV.log) -eq 2 ]"
BYTE4='\( [0-9A-F][0-9A-F]\)\{4\}'
check "their three writes, with the old bytes and the new, and their two announcements, word for word" "lc_line 'DIAGNOSTIC: jump when the ray traced list is full at RVA $(rva_plus 'ray check' 18): 0F 8F 04 00 00 00 -> 0F 8F$BYTE4' && lc_line 'DIAGNOSTIC: call of a job.s coronas. copy at RVA $(rva_plus 'corona end' 10): E8$BYTE4 -> E8$BYTE4' && lc_line 'DIAGNOSTIC: call of a job.s lights. copy at RVA $(rva_plus 'light end' 15): E8$BYTE4 -> E8$BYTE4' && lc_line 'DIAGNOSTIC: the jump at RVA $(rva_plus 'ray check' 18) (G-E-F1) is redirected to a counting stub: every light the full ray traced list refuses is counted.' && lc_line 'DIAGNOSTIC: the calls at RVA $(rva_plus 'corona end' 10) (G-E-K1) and RVA $(rva_plus 'light end' 15) (G-E-K2) are redirected to stubs: a job.s most coronas and lights are kept.'"
check "their stubs: read+execute, on to the way out and to the copy, the counters read/write" "has '$COUNTS_STUBS'"
check "the line at the first light refused, once" "lc_line 'DIAGNOSTIC: the first light a job.s list refused - the list is full, and the job.s later vehicles lose lights in that frame.' && [ \$(count 'the first light a job.s list refused') -eq 1 ]"
check "the first summary, the session alone, word for word: the larger of each two buffers, the refusals, the settings" "lc_line \"DIAGNOSTIC: the most so far: $FIRST_NUMBERS, $MOST_0, a job.s refused lights 199840; $SIRENS_24\" && [ \$(count '$THIS_MINUTE') -eq 0 ] && [ \$(count '$SO_FAR') -eq 1 ]"
check "the samples: at most 33 ms apart, so 300 at least in 10 s; every one with the ray traced list past 512, a few at the most missing as the reading falls between; the sort list marked below the store's room, so none of them at its room" "summary=\$(grep '$SO_FAR' TweaksV.log); samples=\$(sed -n 's/.*sort list marked in 0 of \([0-9]*\) samples.*/\1/p' <<< \"\$summary\"); past=\$(sed -n 's/.*(past 512 in \([0-9]*\) samples.*/\1/p' <<< \"\$summary\"); [ \"\$samples\" -ge 300 ] && [ \"\$past\" -ge \$(( samples - 3 )) ] && [ \"\$past\" -le \"\$samples\" ]"
check "no problem of the part; the other parts' call stubs as ever, every page its own protection" "lc_lacks PROBLEM && has 'protect: all restored (RX)' && has 'stubs: 13, all RX'"

echo "== G2b the game's frames run already, the write pointers set at TweaksV's start: no stub, the numbers sampled all the same"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcrunning lcjobs); echo "      $out"
check "the line, word for word; the jump as placed, both threads through the game's own way out; the measurement announced; no problem" "lc_line 'DIAGNOSTIC: the counting stubs are not made: the game.s frames run already - its threads may run their sites while they are rewritten.' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_line '$MEASURED' && lc_lacks PROBLEM"

echo "== G2c the constructor's references in a form the scan does not see: two places each, a fact for the boot, nothing refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcnoctor); echo "      $out"
check "the two lines, word for word; the measurement announced" "lc_line \"DIAGNOSTIC: the vehicle coronas. store at RVA $(rva coronas) is referenced in 2 places of the game.s code: $(in_order $(rva reset) $(rva 'drain coronas'))\" && lc_line \"DIAGNOSTIC: the vehicle lights. store at RVA $(rva lights) is referenced in 2 places of the game.s code: $(in_order $(rva_plus reset 21) $(rva 'drain lights'))\" && lc_line '$MEASURED'"

echo "== G2d Windows refuses the summary's thread (the test build's TWEAKSV_TEST_REFUSE_THREAD): not measured, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 lighting capacity summary' ASI=TweaksV_test.asi game full_game.exe lcjobs); echo "      $out"
check "the thread's line and the module's, word for word; the sites and the jump as placed, both threads through the game's own way out; no announcement; no problem" "log_has 'The lighting capacity summary thread could not be started (Windows error 8)\.' && lc_line '$NOT_MEASURED its summary thread could not be started.' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_lacks 'the lighting capacity is measured' && lc_lacks PROBLEM"

echo "== G2e Windows refuses the sampling thread: the numbers not sampled, the jump's stub counting and summed up"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 lighting capacity sampling' ASI=TweaksV_test.asi game full_game.exe lcnumbers lcjobs wait); echo "      $out"
check "the thread's line and the module's, word for word; the shorter announcement" "log_has 'The lighting capacity sampling thread could not be started (Windows error 8)\.' && lc_line 'DIAGNOSTIC: the lighting capacity.s numbers are not sampled: their thread could not be started.' && lc_line 'DIAGNOSTIC: the lighting capacity is measured, its numbers not sampled: the rest is logged after 10 seconds, then every minute while it changes.'"
check "the stub made and counting; the summary says the numbers are not sampled and gives the refusals; no first refusal's line, the sampling thread's" "has '${STUB}199840 |' && has '$JOBS' && lc_line '$REDIRECTED' && lc_line \"DIAGNOSTIC: the most so far: its numbers are not sampled (see above), $MOST_0, a job.s refused lights 199840; $SIRENS_24\" && lc_lacks 'the first light a job.s list refused' && lc_lacks PROBLEM"

echo "== G2f the ray traced list's add and a job's ends run as code, through their stubs"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcray lcends wait); echo "      $out"
check "the full list's 976 refusals counted exactly; 40,000 ends with every copy's arguments as they were" "has 'lc counts: ray jump to a stub, RX, on to its way out, its counter RW, holding 976, $COPIES_STUBS' && has 'lc ray: added 1024, refused 976 |' && has 'lc ends: 40000 ends, 0 wrong |'"
check "the summary: the list at its room with its refusals, a job's most the most of both threads" "lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 1024 of 1024 (past 512 in $N samples, by 512 at most, 976 refused), reflection lists 0 of 768, coronas 0 of 1088, a job.s most 79 of 80 lights and 199 of 200 coronas, a job.s refused lights 0; $SIRENS_24\""
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe lcray lcends lcrounds); echo "      $out"
check "a second round, 10 refused and lower ends: the minute's own numbers" "has 'the second round run' && log_has 'the most this minute: .*, 10 refused)' && log_has 'the most this minute: .*a job.s most 40 of 80 lights' && log_has 'the most this minute: .* and 100 of 200 coronas'"
check "the session keeps the most of both and the sum; the ray traced list's refusals count a job's as none" "log_has 'the most so far: .*, 986 refused), .*a job.s most 79 of 80 lights and 199 of 200 coronas, a job.s refused lights 0;' && has 'holding 986, $COPIES_STUBS'"

echo "== G3 the test build, its readings every 500 ms: again when the numbers change, the minute and the session apart, and no more once they stand"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe lcsteps); echo "      $out"
check "the test build's line for this measurement; the fake changed the numbers after the first summary" "log_line '\[TweaksV\] TEST BUILD: the lighting capacity summary comes every 500 ms after its first (TWEAKSV_TEST_SUMMARY_MS=500).\$' && has 'the numbers changed after the first summary'"
LATER_MINUTE="vehicles 20 of 300, vehicle lights 200 of 400, vehicle coronas 100 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in $N of $N samples), ray traced list 700 of 1024 (past 512 in $N samples, by 188 at most, 0 refused), reflection lists 300 of 768, coronas 950 of 1088, $MOST_0, a job.s refused lights 0"
LATER_SESSION="vehicles 87 of 300, vehicle lights 200 of 400, vehicle coronas 512 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in $N of $N samples), ray traced list 700 of 1024 (past 512 in $N samples, by 188 at most, 0 refused), reflection lists 300 of 768, coronas 950 of 1088, $MOST_0, a job.s refused lights 0"
check "the first summary's numbers, then the minute's own, lower ones among them, each the larger from the second buffer" "lc_line \"DIAGNOSTIC: the most so far: $FIRST_NUMBERS, $MOST_0, a job.s refused lights 0; $SIRENS_24\" && lc_line \"DIAGNOSTIC: the most this minute: $LATER_MINUTE.\""
check "the session keeps the most of both" "lc_line \"DIAGNOSTIC: the most so far: $LATER_SESSION; $SIRENS_24\""
check "the sort list marked with the store at its room shows in the minute: not 0 of its samples" "minute=\$(grep 'the most this minute: vehicles 20 ' TweaksV.log | head -n 1); marked=\$(sed -n 's/.*sort list marked in \([0-9]*\) of .*/\1/p' <<< \"\$minute\"); [ \"\$marked\" -gt 0 ]"
check "at most two summaries after the first in six readings or more: nothing new, nothing logged" "[ \$(count '$SO_FAR') -ge 2 ] && [ \$(count '$SO_FAR') -le 3 ] && [ \$(count '$THIS_MINUTE') -eq \$(( \$(count '$SO_FAR') - 1 )) ]"
check "no line past a room, no problem" "lc_lacks 'past the [0-9]* the game has room for' && lc_lacks 'outside its store' && lc_lacks PROBLEM"

echo "== G4 every number at its room, the write pointers at their stores' ends; Windows' dynamic code policy: no stub, so the refusals not counted, and the sampling goes on"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcfull lcjobs nodyncode wait); echo "      $out"
check "the summary at every room, no line past one, the upload passed by its most" "lc_line \"DIAGNOSTIC: the most so far: vehicles 300 of 300, vehicle lights 400 of 400, vehicle coronas 928 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 1024 of 1024 (past 512 in $N samples, by 512 at most), reflection lists 768 of 768, coronas 1088 of 1088, a job.s refused lights are not counted (see above); $SIRENS_24\" && lc_lacks 'past the [0-9]* the game has room for' && lc_lacks 'outside its store'"
check "the stub's line, word for word; the jump as placed, the game's own way out taken by both threads" "lc_line 'DIAGNOSTIC: the counting stubs are not made: they could not be made read+execute (.*).' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub'"
check "the measurement announced all the same; no problem of the part" "lc_line '$MEASURED' && lc_lacks PROBLEM"

echo "== G4b the test build's TWEAKSV_TEST_LIGHT_STUB: no near memory for the stub; the jump's page refused by Windows' dynamic code policy, the stub left writable to get there"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_LIGHT_STUB='no memory' ASI=TweaksV_test.asi game full_game.exe lcjobs); echo "      $out"
check "no near memory: the switch's line and the module's, word for word; the jump as placed, both threads through the game's own way out; no redirect announced, the measurement announced; no problem" "lc_line 'TEST BUILD: the near memory is taken for none (TWEAKSV_TEST_LIGHT_STUB).' && lc_line 'DIAGNOSTIC: the counting stubs are not made: no memory near the game could be had for them.' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_lacks 'jump when a job.s list is full at' && lc_line '$MEASURED' && lc_lacks PROBLEM"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_LIGHT_STUB='left writable' ASI=TweaksV_test.asi game full_game.exe lcjobs nodyncode); echo "      $out"
check "the jump's page refused: the switch's line, WriteCode's on the jump's page and the module's, word for word; the jump as placed, both threads through the game's own way out, so the freed stub is not led to; no redirect announced, the measurement announced; no problem" "lc_line 'TEST BUILD: the stub is left writable (TWEAKSV_TEST_LIGHT_STUB).' && lc_line \"The game code at RVA $(rva_plus add 17) could not be made writable (.*\" && lc_line 'DIAGNOSTIC: the counting stubs are not made: the game code could not be changed (details above).' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_lacks 'jump when a job.s list is full at' && lc_line '$MEASURED' && lc_lacks PROBLEM"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_LIGHT_STUB='no memory' game full_game.exe lcjobs)
check "the Release build ignores the switch: no line of it, the jump through its stub, counted" "lc_lacks 'TEST BUILD' && has '${STUB}199840 |' && lc_line '$REDIRECTED'"

echo "== G5 every number past its room, the write pointers past their stores' ends: a line each, once, at once - no mark, no box"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcpast wait); echo "      $out"
check "the write pointers outside their stores, each with its address, word for word" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer RVA $(rva_plus lights $((401 * 0x1C0))) is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer RVA $(rva_plus coronas $((929 * 0x30))) is outside its store - the game has overwritten memory.'"
check "the vehicles past their room, the memory overwritten; the others past theirs" "lc_line 'DIAGNOSTIC: vehicles seen at 301, past the 300 the game has room for - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: scene lights seen at 1025, past the 1024 the game has room for.' && lc_line 'DIAGNOSTIC: ray traced list seen at 1025, past the 1024 the game has room for.' && lc_line 'DIAGNOSTIC: reflection lists seen at 769, past the 768 the game has room for.' && lc_line 'DIAGNOSTIC: coronas seen at 1089, past the 1088 the game has room for.'"
check "each of the seven once, though every sample of 10 s sees them" "[ \$(count 'past the [0-9]* the game has room for') -eq 5 ] && [ \$(count 'is outside its store') -eq 2 ]"
check "the summary keeps no value past a room: the numbers 0, the upload not passed" "lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples, 0 refused), reflection lists 0 of 768, coronas 0 of 1088, $MOST_0, a job.s refused lights 0; $SIRENS_24\""
check "no problem of the part, so no box" "lc_lacks PROBLEM"

echo "== G5b the write pointers below their stores"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcbelow)
check "outside their stores too, each line once" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer RVA $(rva_plus lights -$((0x1C0))) is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer RVA $(rva_plus coronas -$((0x30))) is outside its store - the game has overwritten memory.' && [ \$(count 'is outside its store') -eq 2 ]"

echo "== G6 the siren settings, through the siren IDs' accessor: none yet, then 0, 33 and 255, then an array that takes no reads for a while"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe simany sinone lcsirens); echo "      $out"
SIRENS="DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples, 0 refused), reflection lists 0 of 768, coronas 0 of 1088, $MOST_0, a job.s refused lights 0;"
check "no carcols data: the summary says the settings are not read yet" "lc_line \"$SIRENS siren settings not read yet.\""
check "0 settings; 33, a third with real lights and at most 12 lights among those, the rest's 20 not counted; 255, 85 of them with real lights, up to 20" "lc_line \"$SIRENS siren settings 0, real lights in 0, up to 0 lights.\" && lc_line \"$SIRENS siren settings 33, real lights in 11, up to 12 lights.\" && lc_line \"$SIRENS siren settings 255, real lights in 85, up to 20 lights.\""
check "an array that takes no reads: no line of the new count while it lasts, the numbers read at the next summary after it" "has 'settings 0 33 255, 0 lines while they took no reads, 40 after' && lc_line \"$SIRENS siren settings 40, real lights in 14, up to 12 lights.\""
check "no problem of the part" "lc_lacks PROBLEM"

echo "== G7 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the sites found and checked, the numbers sampled, the jump left as it is"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe lcjobs wait); echo "      $out"
check "the 22 site lines; the three would lines, word for word; no write, the jump as placed; both threads through the game's own way out" "[ \$(site_lines) -eq 22 ] && lc_line 'DRY_RUN: would redirect the jump at RVA $(rva_plus add 17) (G-E-C1) to a counting stub - a job.s refused lights are not counted.' && lc_line 'DRY_RUN: would redirect the jump at RVA $(rva_plus 'ray check' 18) (G-E-F1) to a counting stub - the ray traced list.s refused lights are not counted.' && lc_line 'DRY_RUN: would redirect the calls at RVA $(rva_plus 'corona end' 10) (G-E-K1) and RVA $(rva_plus 'light end' 15) (G-E-K2) to stubs - a job.s most is not counted.' && lc_lacks 'jump when a job.s list is full at RVA' && lc_lacks 'is redirected to a counting stub' && has '$PLACED' && has '$JOBS'"
check "the numbers sampled all the same, all 0 with no reset run; the summary says the refusals are not counted; no first refusal's line" "lc_line '$MEASURED' && lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples), reflection lists 0 of 768, coronas 0 of 1088, a job.s refused lights are not counted (DRY_RUN); $SIRENS_24\" && lc_lacks 'the first light a job.s list refused' && lc_lacks 'is outside its store'"
check "no problem" "lc_lacks PROBLEM && has 'protect: all restored (RX)'"

echo "== G8 none of the game code present (plain fake game): logged, not a problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game plain_game.exe)
check "the scanner's 22 misses and the module's line, word for word; none of it a problem" "[ \$(grep -c -E 'Pattern G-E-[A-H][1-4] not found: ' TweaksV.log) -eq 22 ] && lc_line '$NOT_MEASURED G-E-A1 was not found.' && lc_lacks PROBLEM"

echo "== G13b the measurement's own two patterns missing (the test build), then the summary with both numbers less"
for label in G-E-K1 G-E-K2; do
	fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the scanner's test line and the line, word for word; the rest measured and its two jumps through their stubs; no problem" "grep -q 'TEST BUILD: pattern $label is treated as not found' TweaksV.log && lc_line 'DIAGNOSTIC: a job.s most is not counted: $label was not found.' && lc_line '$MEASURED' && has 'holding 0, corona copy as placed, light copy as placed |' && lc_lacks PROBLEM"
done
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcout f1jump lcout k1call wait); echo "      $out"
check "the summary says the rest: no refused of the ray traced list, no job's most, a job's refused lights all the same" "lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples), reflection lists 0 of 768, coronas 0 of 1088, a job.s refused lights 0; $SIRENS_24\""

echo "== G14 the part on, DIAGNOSTIC_MODE=0: its one write and its four lines"
part_ini; out=$(game full_game.exe lcjobs lcframe lcindex); echo "      $out"
check "the four lines, word for word, and no other under its tag; no pattern's line, no diagnostic line" "lc_line '$STATUS_CORONAS' && lc_line '$STATUS_LIGHTS' && lc_line '$STATUS_LISTS' && lc_line '$STATUS_INDEX' && [ \$(count '\[LightCapacity\]') -eq 4 ] && log_lacks 'G-E-' && lc_lacks DIAGNOSTIC"
check "the writes and nothing else: every byte of the 22 sites as the part must leave it or as placed, the jump as placed" "has '$RAISED'"
check "the stores moved, read/write, the drain naming what the reset names; the start values read-only and 16-aligned, for G-E-C4's movaps run as code" "has '$MOVED'"
check "every page its own protection; no call stub of its own, the other parts' as with the part off" "has 'protect: all restored (RX)' && has '$stubs_off'"
check "a job's lists as its movaps loads them: 180 lights fit each of the two threads', the rest through the game's own way out" "has '$RAISED_JOBS'"
check "a frame past the game's numbers, the sites run as code: 60 full jobs, every record where the drain finds it, unbroken" "has 'lc frame: 60 jobs of 300 coronas and 180 lights, the reset and the drain one, drained 18000 and 10800, 0 and 0 wrong |'"
check "the index read back whole, the two instructions run as code" "has 'lc index: 700 read back as 700 |'"
check "no problem" "log_lacks PROBLEM"

echo "== G15 the part on, DIAGNOSTIC_MODE=1: the sites found once, the writes logged, the measurement over the moved stores, its stub counting at the raised lists"
part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcnumbers lcjobs lcends wait); echo "      $out"
check "the 21 patterns each found exactly once, G-E-G1 twice: the measurement looks for nothing again; every site logged once, 22 lines" "[ \$(grep -c -E 'Pattern G-E-[A-H][1-4] found: .* 1 match,' TweaksV.log) -eq 21 ] && log_has 'Pattern G-E-G1 found: .* 2 matches,' && [ \$(site_lines) -eq 22 ]"
check "the sites' lines name the game's stores, as the part found them: the frame's reset, word for word" "lc_line 'DIAGNOSTIC: the frame.s reset at RVA $(rva reset): the vehicle coronas. store at RVA $(rva coronas), their write pointer at RVA $(rva 'corona pointer'), the vehicle lights. store at RVA $(rva lights), their write pointer at RVA $(rva 'light pointer') (G-E-A1)'"
check "the constants' writes, each with its old bytes and its new, word for word" "lc_line 'DIAGNOSTIC: scene store.s index store at RVA $(rva_plus 'scene add' 42): 88 04 CA -> 89 04 CA' && lc_line 'DIAGNOSTIC: scene store.s index read at RVA $(rva 'byte read'): 0F B6 1C C1 -> 8B 1C C1 90' && lc_line 'DIAGNOSTIC: job.s scratch size in its kick at RVA $(rva_plus kick 4): 80 B1 00 00 -> 40 73 01 00' && lc_line 'DIAGNOSTIC: job.s scratch size in its last kick at RVA $(rva_plus 'last kick' 5): 80 B1 00 00 -> 40 73 01 00' && lc_line 'DIAGNOSTIC: job.s coronas. bytes at RVA $(rva_plus scratch 17): 80 25 00 00 -> 40 38 00 00' && lc_line 'DIAGNOSTIC: job.s lights. bytes at RVA $(rva_plus scratch 22): 00 8C 00 00 -> 00 3B 01 00' && lc_line 'DIAGNOSTIC: job.s lights. bytes, as -N-1 at RVA $(rva_plus scratch 32): FF 73 FF FF -> FF C4 FE FF' && lc_line 'DIAGNOSTIC: job.s scratch size, as -N-1 at RVA $(rva_plus scratch 42): 7F 4E FF FF -> BF 8C FE FF' && lc_line 'DIAGNOSTIC: job.s scratch size at RVA $(rva_plus scratch 47): 80 B1 00 00 -> 40 73 01 00' && lc_line 'DIAGNOSTIC: job.s lights. bytes, added at RVA $(rva_plus 'scratch lights' 2): 00 8C 00 00 -> 00 3B 01 00' && lc_line 'DIAGNOSTIC: job.s lights. bytes, compared at RVA $(rva_plus 'scratch lights' 9): 00 8C 00 00 -> 00 3B 01 00'"
BYTES='[0-9A-F][0-9A-F] [0-9A-F][0-9A-F] [0-9A-F][0-9A-F] [0-9A-F][0-9A-F]'
check "the five rel32 writes, word for word, and the line that says where they lead" "lc_line \"DIAGNOSTIC: vehicle coronas. store in the frame.s reset at RVA $(rva_plus reset 3): $BYTES -> $BYTES\" && lc_line \"DIAGNOSTIC: vehicle lights. store in the frame.s reset at RVA $(rva_plus reset 24): $BYTES -> $BYTES\" && lc_line \"DIAGNOSTIC: vehicle coronas. store in the drain at RVA $(rva_plus 'drain coronas' 3): $BYTES -> $BYTES\" && lc_line \"DIAGNOSTIC: vehicle lights. store in the drain at RVA $(rva_plus 'drain lights' 3): $BYTES -> $BYTES\" && lc_line \"DIAGNOSTIC: job.s lists. start values at RVA $(rva_plus 'start load' 8): $BYTES -> $BYTES\" && lc_line 'DIAGNOSTIC: the stores moved: the vehicle coronas. at $OUT, the vehicle lights. at $OUT, a job.s lists. start values at $OUT'"
check "the writes and nothing else, as the part leaves them; the stub counting both threads' 199,640 refusals at the raised lists; every page its own protection; no problem" "has '${RAISED_STUB}199640 |' && has '$MOVED' && has '$RAISED_JOBS' && lc_line '$REDIRECTED' && has 'protect: all restored (RX)' && lc_lacks PROBLEM"
RAISED_NUMBERS=${FIRST_NUMBERS/143 of 400/143 of 10800}
RAISED_NUMBERS=${RAISED_NUMBERS/512 of 928/512 of 18000}
check "the first summary over the moved stores, word for word: of 10800 and of 18000, a job's most of 180 and of 300, the rest as ever" "lc_line \"DIAGNOSTIC: the most so far: $RAISED_NUMBERS, a job.s most 79 of 180 lights and 199 of 300 coronas, a job.s refused lights 199640; $SIRENS_24\""

echo "== G16 the part on: a frame past the game's numbers measured, the stores at their rooms and no line past them; then past them, then below them"
part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcframe wait); echo "      $out"
check "the frame run, the summary at the moved stores' rooms, the rest as they stood; no line past a room, no problem" "has 'lc frame: 60 jobs of 300 coronas and 180 lights, the reset and the drain one, drained 18000 and 10800, 0 and 0 wrong |' && lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 10800 of 10800, vehicle coronas 18000 of 18000, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples, 0 refused), reflection lists 0 of 768, coronas 0 of 1088, a job.s most 180 of 180 lights and 300 of 300 coronas, a job.s refused lights 0; $SIRENS_24\" && lc_lacks 'is outside its store' && lc_lacks 'past the [0-9]* the game has room for' && lc_lacks PROBLEM"
part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcpast)
check "a record past the moved stores' ends: outside their stores, each line once, no box" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer $OUT is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer $OUT is outside its store - the game has overwritten memory.' && [ \$(count 'is outside its store') -eq 2 ] && lc_lacks PROBLEM"
part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcbelow)
check "below the moved stores: outside them too, each line once" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer $OUT is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer $OUT is outside its store - the game has overwritten memory.' && [ \$(count 'is outside its store') -eq 2 ]"

echo "== G17 the part on, DRY_RUN=1 with DIAGNOSTIC_MODE=1: everything found and checked, the would lines, nothing written, the measurement over the game's own stores; the sirens' dry run goes on"
part_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe lcnumbers lcjobs lcindex wait); echo "      $out"
check "the would lines of the stores and the start values, word for word" "lc_line 'DRY_RUN: would move the vehicle coronas. store (2 references) and raise its limit from 928 to 18000.' && lc_line 'DRY_RUN: would move the vehicle lights. store (2 references) and raise its limit from 400 to 10800.' && lc_line 'DRY_RUN: would lead a job.s lists. start values at RVA $(rva_plus 'start load' 8) (G-E-C4) to TweaksV.s own: 0, 0, 300, 180.'"
check "the would lines of the constants, each with its old bytes and its new" "lc_line 'DRY_RUN: would patch the scene store.s index store: 88 04 CA -> 89 04 CA' && lc_line 'DRY_RUN: would patch the scene store.s index read: 0F B6 1C C1 -> 8B 1C C1 90' && lc_line 'DRY_RUN: would patch the job.s scratch size: 80 B1 00 00 -> 40 73 01 00' && [ \$(count '\[LightCapacity\] DRY_RUN: would patch the') -eq 11 ]"
check "nothing written: no status line, the sites and the stores as placed, the game's own lists and index; every page its own protection" "lc_lacks 'moved and its limit' && lc_lacks 'patched:' && has '$PLACED' && has '$STORES_PLACED' && has '$JOBS' && has 'lc index: 700 read back as 188 |' && has 'protect: all restored (RX)'"
check "the numbers over the game's own stores; the refusals not counted; no problem" "lc_line \"DIAGNOSTIC: the most so far: $FIRST_UNCOUNTED, a job.s refused lights are not counted (DRY_RUN); $SIRENS_24\" && lc_lacks PROBLEM"
SIRENS_ON=1 fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the part's would-write counts as its one write: the sirens ask and go on" "lc_line 'DRY_RUN: would move the vehicle lights. store (2 references) and raise its limit from 400 to 10800.' && log_lacks 'the light stores are not raised'"

echo "== G18 the sirens' question: true once the one write is done"
SIRENS_ON=1 fresh_ini; setkey ENABLED 0 LIGHTCAPACITY; out=$(game full_game.exe)
check "the part off: the sirens refuse, word for word; the sites and the stores as placed" "log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$STORES_PLACED'"
SIRENS_ON=1 fresh_ini; setkey CORONA_STORE 1088 LIGHTCAPACITY; out=$(game full_game.exe)
check "the part on, the corona store at the game's own: written, and the sirens do not refuse" "has '$RAISED' && lc_lacks PROBLEM && log_lacks 'the light stores are not raised'"
SIRENS_ON=1 fresh_ini; addkey TEST_BREAK_PATTERN G-E-H2; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "a pattern missing: the part's problem and the sirens' refusal, nothing written" "lc_line '$NOT_RAISED G-E-H2 was not found.' && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$STORES_PLACED'"
SIRENS_ON=1 fresh_ini; out=$(TWEAKSV_TEST_THROWS=LightCapacity ASI=TweaksV_test.asi game full_game.exe)
check "the part's start throws: skipped, as any part, the sirens refuse, nothing written" "lc_line 'PROBLEM: unexpected exception - this part is skipped for this session' && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$STORES_PLACED'"

part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcother a1lights)
check "a check failing with DIAGNOSTIC_MODE=1: the part's problem and the measurement's line in the same words, nothing looked for again, nothing written" "lc_line \"$NOT_RAISED the vehicle lights. store at RVA 0x[0-9A-F]* does not lie 928 coronas past the vehicle coronas. at RVA 0x[0-9A-F]* (G-E-A1).\" && lc_line \"$NOT_MEASURED the vehicle lights. store at RVA 0x[0-9A-F]* does not lie 928 coronas past the vehicle coronas. at RVA 0x[0-9A-F]* (G-E-A1).\" && [ \$(count 'Pattern G-E-A1 found') -eq 1 ] && has '$PLACED' && has '$STORES_PLACED'"

echo "== G19 the part on, the game's frames run already, the write pointers set at TweaksV's start: a problem, nothing written"
part_ini; out=$(game full_game.exe lcrunning lcjobs); echo "      $out"
check "the problem, word for word; the sites and the stores as placed, a job's lists the game's own; every page its own protection" "lc_line '$NOT_RAISED the game.s frames run already - its threads may run the sites while they are rewritten.' && has '$PLACED' && has '$STORES_PLACED' && has '$JOBS' && has 'protect: all restored (RX)'"
part_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcrunning lcjobs wait); echo "      $out"
check "with DIAGNOSTIC_MODE=1: the problem, the measurement's own line, the numbers over the game's own stores" "lc_line '$NOT_RAISED the game.s frames run already - its threads may run the sites while they are rewritten.' && lc_line 'DIAGNOSTIC: the counting stubs are not made: the game.s frames run already - its threads may run their sites while they are rewritten.' && lc_line '$MEASURED' && log_has 'vehicle lights 143 of 400, vehicle coronas 512 of 928'"

part_ini; out=$(game full_game.exe lcrunninglights)
check "the lights' write pointer alone set: the frames have begun all the same: the problem, word for word; nothing written" "lc_line '$NOT_RAISED the game.s frames run already - its threads may run the sites while they are rewritten.' && has '$PLACED' && has '$STORES_PLACED'"

echo "== G20 what Windows refuses: the game code's pages (its dynamic code policy); the part's memory (the test build's TWEAKSV_TEST_LIGHT_STUB)"
SIRENS_ON=1 fresh_ini; setkey CORONA_STORE 1088 LIGHTCAPACITY; out=$(game full_game.exe nodyncode)
check "the write failing with the sirens at 32: the part's problem and the sirens' refusal, so that no 32 sirens run over the game's own stores; nothing written" "lc_line '$NOT_RAISED the game code could not be changed (details in TweaksV.log).' && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$STORES_PLACED'"
part_ini; out=$(game full_game.exe lcjobs nodyncode); echo "      $out"
check "the jump's page refused: WriteCode's line and the problem, word for word; the sites and the stores as placed, a job's lists the game's own" "lc_line \"The game code at RVA 0x[0-9A-F]* could not be made writable (.*\" && lc_line '$NOT_RAISED the game code could not be changed (details in TweaksV.log).' && has '$PLACED' && has '$STORES_PLACED' && has '$JOBS' && lc_lacks 'moved and its limit'"
part_ini; out=$(TWEAKSV_TEST_LIGHT_STUB='no stores' ASI=TweaksV_test.asi game full_game.exe); echo "      $out"
check "no near memory: the switch's line and the problem, word for word; nothing written" "lc_line 'TEST BUILD: the near memory is taken for none for the stores (TWEAKSV_TEST_LIGHT_STUB).' && lc_line '$NOT_RAISED no memory near the game could be had for the stores.' && has '$PLACED' && has '$STORES_PLACED' && lc_lacks 'moved and its limit'"
part_ini; out=$(TWEAKSV_TEST_LIGHT_STUB='values refused' ASI=TweaksV_test.asi game full_game.exe)
check "the start values' page refused: the switch's line and the problem, word for word; nothing written" "lc_line 'TEST BUILD: the start values. page is taken as refused (TWEAKSV_TEST_LIGHT_STUB).' && lc_line '$NOT_RAISED a job.s lists. start values could not be made read-only (.*).' && has '$PLACED' && has '$STORES_PLACED' && lc_lacks 'moved and its limit'"
part_ini; out=$(TWEAKSV_TEST_LIGHT_STUB='stores out of reach' ASI=TweaksV_test.asi game full_game.exe)
check "out of reach: the switch's line and the problem, word for word; nothing written" "lc_line 'TEST BUILD: the stores are taken as out of reach (TWEAKSV_TEST_LIGHT_STUB).' && lc_line \"$NOT_RAISED the part.s memory at $OUT is out of reach of the vehicle coronas. store in the frame.s reset at RVA $(rva_plus reset 3).\" && has '$PLACED' && has '$STORES_PLACED' && lc_lacks 'moved and its limit'"
part_ini; out=$(TWEAKSV_TEST_LIGHT_STUB='no stores' game full_game.exe)
check "the Release build ignores the switch: no line of it, the stores moved" "lc_lacks 'TEST BUILD' && has '$RAISED' && has '$MOVED'"

echo "== G21 the part on, none of the game code present (plain fake game)"
part_ini; out=$(game plain_game.exe)
check "the scanner's 22 misses and the problem, word for word" "[ \$(grep -c -E 'Pattern G-E-[A-H][1-4] not found: ' TweaksV.log) -eq 22 ] && lc_line '$NOT_RAISED G-E-A1 was not found.' && lc_lacks 'moved and its limit' && lc_lacks DIAGNOSTIC"

# The cases of each check failing and each pattern missing are queued, then run side by side, each in a game folder of its own, half as many at a time as the suites,
# and checked in order, since each run takes seconds and there are about 110, twice: the measurement's, with the part off, and the part's. section TEXT: a heading.
# not_measured OPTIONS LINE: the fake's options, and the module's line that names why. broken LABEL: the test build with that pattern treated as not found. The
# measurement's: nothing written, nothing started, no problem. The part's: its problem, the only one of the part, nothing written, nothing started.
CASES=()
section() { CASES+=("== $1"); }
not_measured() { CASES+=("$1|$2"); }
broken() { CASES+=("!$1"); }
a_number_less() { CASES+=("~$1|$2"); }
HERE='RVA 0x[0-9A-F]*'

section "G9 the frame's stores, each check failing on its own"
not_measured 'lcout a1coronas' "the vehicle coronas. store at $OUT (G-E-A1) is not inside the game."
not_measured 'lcend a1lights' "the vehicle lights. store at $HERE (G-E-A1) is not inside the game."
not_measured 'lcout a1coronap' "the vehicle coronas. write pointer at $OUT (G-E-A1) is not inside the game."
not_measured 'lcodd a1coronap' "the vehicle coronas. write pointer at $HERE (G-E-A1) is not aligned to 8 bytes."
not_measured 'lcunread a1coronap' "the vehicle coronas. write pointer at $HERE (G-E-A1) cannot be read."
not_measured 'lcout a1lightp' "the vehicle lights. write pointer at $OUT (G-E-A1) is not inside the game."
not_measured 'lcodd a1lightp' "the vehicle lights. write pointer at $HERE (G-E-A1) is not aligned to 8 bytes."
not_measured 'lcunread a1lightp' "the vehicle lights. write pointer at $HERE (G-E-A1) cannot be read."
not_measured 'lcother a1lights' "the vehicle lights. store at $HERE does not lie 928 coronas past the vehicle coronas. at $HERE (G-E-A1)."
not_measured 'lcother a1coronap' "the vehicle coronas. write pointer at $HERE does not lie 400 lights past the vehicle lights. store at $HERE (G-E-A1)."
not_measured 'lcother a1lightp' "the vehicle lights. write pointer at $HERE does not lie right behind the vehicle coronas. at $HERE (G-E-A1)."
not_measured 'lcother a2coronas' "the drain.s coronas name the store at $HERE and the write pointer at $HERE, not those of the frame.s reset (G-E-A2)."
not_measured 'lcother a2coronap' "the drain.s coronas name the store at $HERE and the write pointer at $HERE, not those of the frame.s reset (G-E-A2)."
not_measured 'lcother a3lights' "the drain.s lights name the store at $HERE and the write pointer at $HERE, not those of the frame.s reset (G-E-A3)."
not_measured 'lcother a3lightp' "the drain.s lights name the store at $HERE and the write pointer at $HERE, not those of the frame.s reset (G-E-A3)."
not_measured 'lcfourthcoronas' "the game.s code references the vehicle coronas. store at $HERE in 4 places or more, not 3 - the frame.s reset, the drain and the constructor, as the research found them."
not_measured 'lcfourthlights' "the game.s code references the vehicle lights. store at $HERE in 4 places or more, not 3 - the frame.s reset, the drain and the constructor, as the research found them."
not_measured 'lcout a3call' "the drain.s add-scene-light call at $HERE is not a call into the game (another mod may have hooked it already) (G-E-A3)."
not_measured 'lcedge a3call rtjumpunread' "the drain.s add-scene-light call at $HERE cannot be read (G-E-A3)."

section "G10 a job's vehicles, its jump and its start values"
not_measured 'lcout b1count' "the vehicles. count at $OUT (G-E-B1) is not inside the game."
not_measured 'lcodd b1count' "the vehicles. count at $HERE (G-E-B1) is not aligned to 4 bytes."
not_measured 'lcunread b1count' "the vehicles. count at $HERE (G-E-B1) cannot be read."
not_measured 'lcout c1jump lcjobs' "the jump when a job.s list is full, at $HERE, does not lead into the game (another mod may have hooked it already) (G-E-C1)."
not_measured 'lcunread c1jump lcjobs' "the jump when a job.s list is full, at $HERE, leads to $HERE, which cannot be read (G-E-C1)."
not_measured 'lcother c1jump lcjobs' "the jump when a job.s list is full, at $HERE, leads to $HERE, which holds [0-9A-F ]*, not the function.s way out (G-E-C1)."
not_measured 'lcout c4values' "the job.s lists. start values at $OUT (G-E-C4) are not inside the game."
not_measured 'lcodd c4values' "the job.s lists. start values at $HERE (G-E-C4) are not aligned to 16 bytes."
not_measured 'lcunread c4values' "the job.s lists. start values at $HERE (G-E-C4) cannot be read."
not_measured 'lcstartvalues wait' "a job.s lists. start values at $HERE are not 0, 0, 200, 80 (G-E-C4)."
not_measured 'lcmodded' "G-E-C3 was not found."

section "G11 the scene store"
not_measured 'lcout d1count' "the scene store.s count at $OUT (G-E-D1) is not inside the game."
not_measured 'lcodd d1count' "the scene store.s count at $HERE (G-E-D1) is not aligned to 4 bytes."
not_measured 'lcunread d1count' "the scene store.s count at $HERE (G-E-D1) cannot be read."
not_measured 'lcout d1sortlist' "the scene store.s sort list at $OUT (G-E-D1) is not inside the game."
not_measured 'lcout d1sortcount' "the scene store.s sort mark at $OUT (G-E-D1) is not inside the game."
not_measured 'lcunread d1sortcount' "the scene store.s sort mark at $HERE (G-E-D1) cannot be read."
not_measured 'lcother d1sortlist' "the sort list.s count at $HERE does not lie right behind its 1024 entries at $HERE (G-E-D1)."
not_measured 'lcother d1sortstore' "the sort list.s count is read at $HERE and stored at $HERE (G-E-D1)."
not_measured 'lcout e1store' "the scene store at $OUT (G-E-E1) is not inside the game."
not_measured 'lcout e1renderp' "the scene store.s render pointer at $OUT (G-E-E1) is not inside the game."
not_measured 'lcout e1counts' "the scene store.s buffers. counts at $OUT (G-E-E1) are not inside the game."
not_measured 'lcout e1rendercount' "the scene store.s render count at $OUT (G-E-E1) is not inside the game."
not_measured 'lcodd e1rendercount' "the scene store.s render count at $HERE (G-E-E1) is not aligned to 4 bytes."
not_measured 'lcunread e1rendercount' "the scene store.s render count at $HERE (G-E-E1) cannot be read."
not_measured 'lcout e1reflections' "the reflection lists. counts at $OUT (G-E-E1) are not inside the game."
not_measured 'lcodd e1reflections' "the reflection lists. counts at $HERE (G-E-E1) are not aligned to 4 bytes."
not_measured 'lcunread e1reflections' "the reflection lists. counts at $HERE (G-E-E1) cannot be read."
not_measured 'lcend e1reflections' "the second reflection lists. counts at $OUT (G-E-E1) are not inside the game."
not_measured 'lcout e1reflp' "the reflection lists. render pointer at $OUT (G-E-E1) is not inside the game."
not_measured 'lcedge e1reflp rtjumpunread' "the reflection lists. render pointer.s store at $HERE cannot be read (G-E-E1)."
not_measured 'lcother e2store' "the scene store.s update side names the store at $HERE, its render side the one at $HERE (G-E-E2)."
not_measured 'lcout e2current' "the scene store.s current pointer at $OUT (G-E-E2) is not inside the game."

section "G12 the ray traced lists, their upload and the corona store"
not_measured 'lcout e3lists' "the ray traced list.s count at $OUT (G-E-E3) is not inside the game."
not_measured 'lcodd e3lists' "the ray traced list.s count at $HERE (G-E-E3) is not aligned to 4 bytes."
not_measured 'lcunread e3lists' "the ray traced list.s count at $HERE (G-E-E3) cannot be read."
not_measured 'lcend e3lists' "the second ray traced list.s count at $OUT (G-E-E3) is not inside the game."
not_measured 'lcout e3pointer' "the ray traced lists. current pointer at $OUT (G-E-E3) is not inside the game."
not_measured 'lcother f1pointer' "the ray traced list.s check reads the current pointer at $HERE, not the one at $HERE (G-E-F1)."
not_measured 'lcout f2call' "the ray traced upload.s builder call at $HERE is not a call into the game (another mod may have hooked it already) (G-E-F2)."
not_measured 'lcedge f2call rtjumpunread' "the ray traced upload.s builder call at $HERE cannot be read (G-E-F2)."
not_measured 'lcother g1second' "the corona store.s two checks read different counts: $HERE and $HERE (G-E-G1)."
not_measured 'lcout g1first lcout g1second' "the corona store.s count at $OUT (G-E-G1) is not inside the game."
not_measured 'lcodd g1first lcodd g1second' "the corona store.s count at $HERE (G-E-G1) is not aligned to 4 bytes."
not_measured 'lcunread g1first lcunread g1second' "the corona store.s count at $HERE (G-E-G1) cannot be read."
not_measured 'lcg1once' "the game code holds G-E-G1 1 time, not the expected 2."
not_measured 'lcg1thrice' "the game code holds G-E-G1 3 or more times, not the expected 2."

section "G12b the ray traced list's jump and a job's two copies: that number not counted, the line says why, the measurement goes on, and the part never asks"
RAY_LESS="the ray traced list.s refused lights are not counted: the jump when the ray traced list is full, at $HERE"
MOST_LESS="a job.s most is not counted:"
a_number_less 'lcout f1jump' "$RAY_LESS, does not lead into the game (another mod may have hooked it already) (G-E-F1)."
a_number_less 'lcunread f1jump' "$RAY_LESS, leads to $HERE, which cannot be read (G-E-F1)."
a_number_less 'lcother f1jump' "$RAY_LESS, leads to $HERE, which holds [0-9A-F ]*, not the function.s way out (G-E-F1)."
a_number_less 'lcedge f1jump rtjumpunread' "$RAY_LESS, cannot be read (G-E-F1)."
a_number_less 'lcout k1call' "$MOST_LESS the call of a job.s coronas. copy at $HERE is not a call into the game (another mod may have hooked it already) (G-E-K1)."
a_number_less 'lcout k2call' "$MOST_LESS the call of a job.s lights. copy at $HERE is not a call into the game (another mod may have hooked it already) (G-E-K2)."
a_number_less 'lcedge k1call rtjumpunread' "$MOST_LESS the call of a job.s coronas. copy at $HERE cannot be read (G-E-K1)."
a_number_less 'lcedge k2call rtjumpunread' "$MOST_LESS the call of a job.s lights. copy at $HERE cannot be read (G-E-K2)."
a_number_less 'lcother k2call' "$MOST_LESS a job.s two copies call different functions: $HERE and $HERE (G-E-K2)."

section "G13 TEST_BREAK_PATTERN (test build): each of the 22 patterns missing"
for label in $(sed -nE 's/^\s*\{ "(G-E-[A-H][1-4])", .*/\1/p' "$R/source/TweaksVPatterns.h"); do
	broken "$label"
done

# run_case FOLDER CASE [part], in the background: the part on and no DIAGNOSTIC_MODE where it says so, and no run longer than it need be
game_folder=$B
run_case() {
	B=$game_folder/cases/$1
	mkdir -p "$B" && cd "$B" || exit 1
	cp ../../TweaksV.asi ../../TweaksV_test.asi .
	local options=${2%%|*}
	options=${options#\~}
	if [ -n "${3:-}" ]; then part_ini; options=${options/ wait/}; else fresh_ini; setkey DIAGNOSTIC_MODE 1; fi
	case "$2" in
		"!"*) addkey TEST_BREAK_PATTERN "${2#!}"; ASI=TweaksV_test.asi game ../../full_game.exe lcnumbers > /dev/null ;;
		*) game ../../full_game.exe $options > /dev/null ;;
	esac
}
n=0; running=0
for entry in "${CASES[@]}"; do
	[[ "$entry" == "== "* ]] && continue
	n=$((n + 1))
	for mode in "" part; do
		run_case "$mode$n" "$entry" $mode &
		running=$((running + 1))
		if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
	done
done
wait
n=0
for entry in "${CASES[@]}"; do
	if [[ "$entry" == "== "* ]]; then echo "$entry"; continue; fi
	n=$((n + 1))
	B=$game_folder/cases/$n
	cd "$B" || exit 1
	out=$(tr -d '\r' < harness.out)
	NOTHING="has '$PLACED' && lc_lacks 'the lighting capacity is measured' && lc_lacks 'jump when a job.s list is full at' && lc_lacks PROBLEM"
	if [[ "$entry" == "~"* ]]; then
		options=${entry%%|*}; options=${options#\~}
		check "$options: the line, word for word; the rest of the stubs made and the numbers measured; no problem" "lc_line 'DIAGNOSTIC: ${entry#*|}' && lc_line '$MEASURED' && lc_line '$REDIRECTED' && has '${STUB}0 |' && lc_lacks PROBLEM && { [[ '$options' != *f1jump* ]] || has 'lc counts: ray jump as placed, $COPIES_STUBS'; } && { [[ '$options' != *k?call* ]] || has 'corona copy as placed, light copy as placed |'; }"
	elif [[ "$entry" == "!"* ]]; then
		label=${entry#!}
		check "$label: the scanner's test line and the module's, word for word; the sites as placed; no announcement; no problem" "grep -q 'TEST BUILD: pattern $label is treated as not found' TweaksV.log && lc_line '$NOT_MEASURED $label was not found.' && $NOTHING"
	else
		options=${entry%%|*}
		check "$options: the line, word for word; the sites as placed, no announcement, the fake runs no jump that leads elsewhere and, with 11 s more, no summary comes; no problem" "lc_line '$NOT_MEASURED ${entry#*|}' && $NOTHING && { [[ '$options' != *lcjobs* ]] || has 'lc jobs: not run'; } && { [[ '$options' != *wait* ]] || lc_lacks 'the most so far'; }"
	fi
done
echo "== G22 the same with the part on: each pattern missing and each check failing refuses it, with nothing written"
n=0
for entry in "${CASES[@]}"; do
	[[ "$entry" == "== "* ]] && continue
	n=$((n + 1))
	B=$game_folder/cases/part$n
	cd "$B" || exit 1
	out=$(tr -d '\r' < harness.out)
	NOTHING="[ \$(count '\[LightCapacity\] PROBLEM') -eq 1 ] && has '$PLACED' && has '$STORES_PLACED' && lc_lacks 'moved and its limit' && lc_lacks 'patched:' && lc_lacks DIAGNOSTIC && has 'protect: all restored (RX)'"
	if [[ "$entry" == "~"* ]]; then
		options=${entry%%|*}; options=${options#\~}
		check "$options: the measurement's alone, not looked at without DIAGNOSTIC_MODE: the part on, its one write made, no problem" "has '$RAISED' && has '$MOVED' && lc_lacks PROBLEM && lc_lacks DIAGNOSTIC"
	elif [[ "$entry" == "!"* ]]; then
		label=${entry#!}
		check "$label: the part's problem, word for word, the only one of the part; the sites and the stores as placed" "lc_line '$NOT_RAISED $label was not found.' && $NOTHING"
	else
		options=${entry%%|*}
		check "${options/ wait/}: the part's problem, word for word, the only one of the part; the sites and the stores as placed" "lc_line '$NOT_RAISED ${entry#*|}' && $NOTHING && { [[ '$options' != *lcjobs* ]] || has 'lc jobs: not run'; }"
	fi
done
B=$game_folder
cd "$B" && rm -rf cases

echo
echo "LIGHT CAPACITY SCENARIOS: $pass passed, $fail failed"
