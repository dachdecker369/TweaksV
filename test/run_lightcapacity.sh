#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The lighting capacity, a limit measured first (TweaksVLightCapacity): with DIAGNOSTIC_MODE=1 the fake game's 22 sites found, checked and
# logged, the counters sampled and summed up 10 seconds in and again when they change, the one write - the jump when a job's list is full
# through a counting stub while two threads run that code at once, the count exact - and nothing else; every number at its room and past it,
# the write pointers outside their stores, each line once and no box; the siren settings' numbers on none, 0, 33 and 255 settings and an array
# that takes no reads; each pattern missing, each check failing on its own, a jump another mod redirected, the plain game, DRY_RUN and Windows'
# dynamic code policy - each logged, never a problem, nothing written; without DIAGNOSTIC_MODE nothing looked for. Fake LOCALAPPDATA: the real
# one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# What the fake reports: the sites but the jump's rel32 as placed, and where the jump leads
PLACED='lc sites: as placed, jump as placed |'
STUB='lc sites: as placed, jump to a stub, RX, on to its way out, its counter RW, holding '
JOBS='lc jobs: added 160, refused 199840'
# The module's lines, word for word (an apostrophe as any character)
lc_line() { log_line "\[LightCapacity\] $1\$"; }
lc_lacks() { log_lacks "\[LightCapacity\] .*$1"; }
NOT_MEASURED='DIAGNOSTIC: the lighting capacity is not measured:'
MEASURED='DIAGNOSTIC: the lighting capacity is measured: its numbers are sampled every 10 ms and logged after 10 seconds, then every minute while they change.'
REDIRECTED='DIAGNOSTIC: the jump at RVA 0x[0-9A-F]* (G-E-C1) is redirected to a counting stub: every light a job.s full list refuses is counted.'
SITES='\[LightCapacity\] DIAGNOSTIC: .*(\(N-1, \)\?G-E-[A-H][1-4])\( -> game size .*\)\?$'
site_lines() { tr -d '\r' < TweaksV.log | grep -c -- "$SITES"; }
N='[0-9]*'
FIRST_NUMBERS="vehicles 87 of 300, vehicle lights 143 of 400, vehicle coronas 512 of 928, scene lights 811 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 640 of 1024 (past 512 in $N samples, by 128 at most), reflection lists 212 of 768, coronas 900 of 1088"
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

echo "== G1 DIAGNOSTIC_MODE=0: nothing looked for, nothing written"
fresh_ini; out=$(game full_game.exe lcnumbers lcjobs); echo "      $out"
check "no pattern of the part looked for, no line of the part" "log_lacks 'G-E-' && log_lacks '\[LightCapacity\]'"
check "the sites and the jump as placed, the lists' calls through the game's own jump; every page its own protection" "has '$PLACED' && has '$JOBS' && has 'protect: all restored (RX)'"

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
check "the line at the first light refused, once" "lc_line 'DIAGNOSTIC: the first light a job.s list refused - the list is full, and the job.s later vehicles lose lights in that frame.' && [ \$(count 'the first light a job.s list refused') -eq 1 ]"
check "the first summary, the session alone, word for word: the larger of each two buffers, the refusals, the settings" "lc_line \"DIAGNOSTIC: the most so far: $FIRST_NUMBERS, a job.s refused lights 199840; $SIRENS_24\" && [ \$(count '$THIS_MINUTE') -eq 0 ] && [ \$(count '$SO_FAR') -eq 1 ]"
check "the samples: at most 33 ms apart, so 300 at least in 10 s; every one with the ray traced list past 512, a few at the most missing as the reading falls between; the sort list marked below the store's room, so none of them at its room" "summary=\$(grep '$SO_FAR' TweaksV.log); samples=\$(sed -n 's/.*sort list marked in 0 of \([0-9]*\) samples.*/\1/p' <<< \"\$summary\"); past=\$(sed -n 's/.*(past 512 in \([0-9]*\) samples.*/\1/p' <<< \"\$summary\"); [ \"\$samples\" -ge 300 ] && [ \"\$past\" -ge \$(( samples - 3 )) ] && [ \"\$past\" -le \"\$samples\" ]"
check "no problem of the part; the other parts' call stubs as ever, every page its own protection" "lc_lacks PROBLEM && has 'protect: all restored (RX)' && has 'stubs: 12, all RX'"

echo "== G2b the game's frames run already, the write pointers set at TweaksV's start: no stub, the numbers sampled all the same"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcrunning lcjobs); echo "      $out"
check "the line, word for word; the jump as placed, both threads through the game's own way out; the measurement announced; no problem" "lc_line 'DIAGNOSTIC: a job.s refused lights are not counted: the game.s frames run already - its threads may run the jump while it is rewritten.' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_line '$MEASURED' && lc_lacks PROBLEM"

echo "== G2c the constructor's references in a form the scan does not see: two places each, a fact for the boot, nothing refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcnoctor); echo "      $out"
check "the two lines, word for word; the measurement announced" "lc_line \"DIAGNOSTIC: the vehicle coronas. store at RVA $(rva coronas) is referenced in 2 places of the game.s code: $(in_order $(rva reset) $(rva 'drain coronas'))\" && lc_line \"DIAGNOSTIC: the vehicle lights. store at RVA $(rva lights) is referenced in 2 places of the game.s code: $(in_order $(rva_plus reset 21) $(rva 'drain lights'))\" && lc_line '$MEASURED'"

echo "== G2d Windows refuses the summary's thread (the test build's TWEAKSV_TEST_REFUSE_THREAD): not measured, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 lighting capacity summary' ASI=TweaksV_test.asi game full_game.exe lcjobs); echo "      $out"
check "the thread's line and the module's, word for word; the sites and the jump as placed, both threads through the game's own way out; no announcement; no problem" "log_has 'The lighting capacity summary thread could not be started (Windows error 8)\.' && lc_line '$NOT_MEASURED its summary thread could not be started.' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub' && lc_lacks 'the lighting capacity is measured' && lc_lacks PROBLEM"

echo "== G2e Windows refuses the sampling thread: the numbers not sampled, the jump's stub counting and summed up"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 lighting capacity sampling' ASI=TweaksV_test.asi game full_game.exe lcnumbers lcjobs wait); echo "      $out"
check "the thread's line and the module's, word for word; the shorter announcement" "log_has 'The lighting capacity sampling thread could not be started (Windows error 8)\.' && lc_line 'DIAGNOSTIC: the lighting capacity.s numbers are not sampled: their thread could not be started.' && lc_line 'DIAGNOSTIC: the lighting capacity is measured, its numbers not sampled: the rest is logged after 10 seconds, then every minute while it changes.'"
check "the stub made and counting; the summary says the numbers are not sampled and gives the refusals; no first refusal's line, the sampling thread's" "has '${STUB}199840 |' && has '$JOBS' && lc_line '$REDIRECTED' && lc_line \"DIAGNOSTIC: the most so far: its numbers are not sampled (see above), a job.s refused lights 199840; $SIRENS_24\" && lc_lacks 'the first light a job.s list refused' && lc_lacks PROBLEM"

echo "== G3 the test build, its readings every 500 ms: again when the numbers change, the minute and the session apart, and no more once they stand"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe lcsteps); echo "      $out"
check "the test build's line for this measurement; the fake changed the numbers after the first summary" "log_line '\[TweaksV\] TEST BUILD: the lighting capacity summary comes every 500 ms after its first (TWEAKSV_TEST_SUMMARY_MS=500).\$' && has 'the numbers changed after the first summary'"
LATER_MINUTE="vehicles 20 of 300, vehicle lights 200 of 400, vehicle coronas 100 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in $N of $N samples), ray traced list 700 of 1024 (past 512 in $N samples, by 188 at most), reflection lists 300 of 768, coronas 950 of 1088, a job.s refused lights 0"
LATER_SESSION="vehicles 87 of 300, vehicle lights 200 of 400, vehicle coronas 512 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in $N of $N samples), ray traced list 700 of 1024 (past 512 in $N samples, by 188 at most), reflection lists 300 of 768, coronas 950 of 1088, a job.s refused lights 0"
check "the first summary's numbers, then the minute's own, lower ones among them, each the larger from the second buffer" "lc_line \"DIAGNOSTIC: the most so far: $FIRST_NUMBERS, a job.s refused lights 0; $SIRENS_24\" && lc_line \"DIAGNOSTIC: the most this minute: $LATER_MINUTE.\""
check "the session keeps the most of both" "lc_line \"DIAGNOSTIC: the most so far: $LATER_SESSION; $SIRENS_24\""
check "the sort list marked with the store at its room shows in the minute: not 0 of its samples" "minute=\$(grep 'the most this minute: vehicles 20 ' TweaksV.log | head -n 1); marked=\$(sed -n 's/.*sort list marked in \([0-9]*\) of .*/\1/p' <<< \"\$minute\"); [ \"\$marked\" -gt 0 ]"
check "at most two summaries after the first in six readings or more: nothing new, nothing logged" "[ \$(count '$SO_FAR') -ge 2 ] && [ \$(count '$SO_FAR') -le 3 ] && [ \$(count '$THIS_MINUTE') -eq \$(( \$(count '$SO_FAR') - 1 )) ]"
check "no line past a room, no problem" "lc_lacks 'past the [0-9]* the game has room for' && lc_lacks 'outside its store' && lc_lacks PROBLEM"

echo "== G4 every number at its room, the write pointers at their stores' ends; Windows' dynamic code policy: no stub, so the refusals not counted, and the sampling goes on"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcfull lcjobs nodyncode wait); echo "      $out"
check "the summary at every room, no line past one, the upload passed by its most" "lc_line \"DIAGNOSTIC: the most so far: vehicles 300 of 300, vehicle lights 400 of 400, vehicle coronas 928 of 928, scene lights 1024 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 1024 of 1024 (past 512 in $N samples, by 512 at most), reflection lists 768 of 768, coronas 1088 of 1088, a job.s refused lights are not counted (see above); $SIRENS_24\" && lc_lacks 'past the [0-9]* the game has room for' && lc_lacks 'outside its store'"
check "the stub's line, word for word; the jump as placed, the game's own way out taken by both threads" "lc_line 'DIAGNOSTIC: a job.s refused lights are not counted: its stub could not be made read+execute (.*).' && has '$PLACED' && has '$JOBS' && lc_lacks 'is redirected to a counting stub'"
check "the measurement announced all the same; no problem of the part" "lc_line '$MEASURED' && lc_lacks PROBLEM"

echo "== G5 every number past its room, the write pointers past their stores' ends: a line each, once, at once - no mark, no box"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcpast wait); echo "      $out"
check "the write pointers outside their stores, each with its address, word for word" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer RVA $(rva_plus lights $((401 * 0x1C0))) is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer RVA $(rva_plus coronas $((929 * 0x30))) is outside its store - the game has overwritten memory.'"
check "the vehicles past their room, the memory overwritten; the others past theirs" "lc_line 'DIAGNOSTIC: vehicles seen at 301, past the 300 the game has room for - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: scene lights seen at 1025, past the 1024 the game has room for.' && lc_line 'DIAGNOSTIC: ray traced list seen at 1025, past the 1024 the game has room for.' && lc_line 'DIAGNOSTIC: reflection lists seen at 769, past the 768 the game has room for.' && lc_line 'DIAGNOSTIC: coronas seen at 1089, past the 1088 the game has room for.'"
check "each of the seven once, though every sample of 10 s sees them" "[ \$(count 'past the [0-9]* the game has room for') -eq 5 ] && [ \$(count 'is outside its store') -eq 2 ]"
check "the summary keeps no value past a room: the numbers 0, the upload not passed" "lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples), reflection lists 0 of 768, coronas 0 of 1088, a job.s refused lights 0; $SIRENS_24\""
check "no problem of the part, so no box" "lc_lacks PROBLEM"

echo "== G5b the write pointers below their stores"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lcbelow)
check "outside their stores too, each line once" "lc_line 'DIAGNOSTIC: vehicle lights. write pointer RVA $(rva_plus lights -$((0x1C0))) is outside its store - the game has overwritten memory.' && lc_line 'DIAGNOSTIC: vehicle coronas. write pointer RVA $(rva_plus coronas -$((0x30))) is outside its store - the game has overwritten memory.' && [ \$(count 'is outside its store') -eq 2 ]"

echo "== G6 the siren settings, through the siren IDs' accessor: none yet, then 0, 33 and 255, then an array that takes no reads for a while"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe simany sinone lcsirens); echo "      $out"
SIRENS="DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples), reflection lists 0 of 768, coronas 0 of 1088, a job.s refused lights 0;"
check "no carcols data: the summary says the settings are not read yet" "lc_line \"$SIRENS siren settings not read yet.\""
check "0 settings; 33, a third with real lights and at most 12 lights among those, the rest's 20 not counted; 255, 85 of them with real lights, up to 20" "lc_line \"$SIRENS siren settings 0, real lights in 0, up to 0 lights.\" && lc_line \"$SIRENS siren settings 33, real lights in 11, up to 12 lights.\" && lc_line \"$SIRENS siren settings 255, real lights in 85, up to 20 lights.\""
check "an array that takes no reads: no line of the new count while it lasts, the numbers read at the next summary after it" "has 'settings 0 33 255, 0 lines while they took no reads, 40 after' && lc_line \"$SIRENS siren settings 40, real lights in 14, up to 12 lights.\""
check "no problem of the part" "lc_lacks PROBLEM"

echo "== G7 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the sites found and checked, the numbers sampled, the jump left as it is"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe lcjobs wait); echo "      $out"
check "the 22 site lines; the would line, word for word; no write, the jump as placed; both threads through the game's own way out" "[ \$(site_lines) -eq 22 ] && lc_line 'DRY_RUN: would redirect the jump at RVA $(rva_plus add 17) (G-E-C1) to a counting stub - a job.s refused lights are not counted.' && lc_lacks 'jump when a job.s list is full at RVA' && lc_lacks 'is redirected to a counting stub' && has '$PLACED' && has '$JOBS'"
check "the numbers sampled all the same, all 0 with no reset run; the summary says the refusals are not counted; no first refusal's line" "lc_line '$MEASURED' && lc_line \"DIAGNOSTIC: the most so far: vehicles 0 of 300, vehicle lights 0 of 400, vehicle coronas 0 of 928, scene lights 0 of 1024 (at its room with the sort list marked in 0 of $N samples), ray traced list 0 of 1024 (past 512 in 0 samples), reflection lists 0 of 768, coronas 0 of 1088, a job.s refused lights are not counted (DRY_RUN); $SIRENS_24\" && lc_lacks 'the first light a job.s list refused' && lc_lacks 'is outside its store'"
check "no problem" "lc_lacks PROBLEM && has 'protect: all restored (RX)'"

echo "== G8 none of the game code present (plain fake game): logged, not a problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game plain_game.exe)
check "the scanner's 22 misses and the module's line, word for word; none of it a problem" "[ \$(grep -c -E 'Pattern G-E-[A-H][1-4] not found: ' TweaksV.log) -eq 22 ] && lc_line '$NOT_MEASURED G-E-A1 was not found.' && lc_lacks PROBLEM"

# The cases of each check failing and each pattern missing are queued, then run side by side, each in a game folder of its own, half as many at a time as the suites,
# and checked in order, since each run takes seconds and there are about 90. section TEXT: a heading. not_measured OPTIONS LINE: the fake's options, and the module's
# line that names why. broken LABEL: the test build with that pattern treated as not found. Each: nothing written, nothing started, no problem.
CASES=()
section() { CASES+=("== $1"); }
not_measured() { CASES+=("$1|$2"); }
broken() { CASES+=("!$1"); }
OUT='0x[0-9A-F]* (outside the game)'
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
not_measured 'lcodd c4values' "the job.s lists. start values at $HERE (G-E-C4) are not aligned to 4 bytes."
not_measured 'lcunread c4values' "the job.s lists. start values at $HERE (G-E-C4) cannot be read."
not_measured 'lcstartvalues wait' "a job.s lists. start values at $HERE are not 0, 0, 200, 80 (G-E-C4)."

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

section "G13 TEST_BREAK_PATTERN (test build): each of the 22 patterns missing"
for label in $(sed -nE 's/^\s*\{ "(G-E-[A-Za-z0-9-]+)", .*/\1/p' "$R/source/TweaksVPatterns.h"); do
	broken "$label"
done

# run_case N CASE, in the background
game_folder=$B
run_case() {
	B=$game_folder/cases/$1
	mkdir -p "$B" && cd "$B" || exit 1
	cp ../../TweaksV.asi ../../TweaksV_test.asi .
	fresh_ini; setkey DIAGNOSTIC_MODE 1
	case "$2" in
		"!"*) addkey TEST_BREAK_PATTERN "${2#!}"; ASI=TweaksV_test.asi game ../../full_game.exe lcnumbers > /dev/null ;;
		*) game ../../full_game.exe ${2%%|*} > /dev/null ;;
	esac
}
n=0; running=0
for entry in "${CASES[@]}"; do
	[[ "$entry" == "== "* ]] && continue
	n=$((n + 1))
	run_case "$n" "$entry" &
	running=$((running + 1))
	if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
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
	if [[ "$entry" == "!"* ]]; then
		label=${entry#!}
		check "$label: the scanner's test line and the module's, word for word; the sites as placed; no announcement; no problem" "grep -q 'TEST BUILD: pattern $label is treated as not found' TweaksV.log && lc_line '$NOT_MEASURED $label was not found.' && $NOTHING"
	else
		options=${entry%%|*}
		check "$options: the line, word for word; the sites as placed, no announcement, the fake runs no jump that leads elsewhere and, with 11 s more, no summary comes; no problem" "lc_line '$NOT_MEASURED ${entry#*|}' && $NOTHING && { [[ '$options' != *lcjobs* ]] || has 'lc jobs: not run'; } && { [[ '$options' != *wait* ]] || lc_lacks 'the most so far'; }"
	fi
done
B=$game_folder
cd "$B" && rm -rf cases

echo
echo "LIGHT CAPACITY SCENARIOS: $pass passed, $fail failed"
