#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The render queues, a limit measured first (TweaksVRenderQueues): with DIAGNOSTIC_MODE=1 the fake game's 24 sites found, each once,
# checked and logged at the fake's own RVAs, and its six pools, its three families of arrays and its draw list read every 20 ms while the
# fake runs the patterns' own bytes - each pool's pointer decoded by its make, each add writing what the module reads - and summed up 10
# seconds in, word for word, and again when a most changes and not when only the latest numbers do; r and s of the fake's own, read and
# not assumed; a pool not made, a stride wrong for 50 samples in a row and for shorter spells, a pool, the entries or the manager that cannot
# be read, tried 50 times and no more, each pool full, the arrays past their capacity, said at once, and with repeats, past 4,096 entries, the
# page of the game's data taken away; each pattern missing, each held
# twice, each of the module's checks failing on its own: logged, never a problem, nothing measured; a thread that cannot start. In every
# run nothing written: the sites and the game's data as placed, no stub of its own, every page its own protection; without
# DIAGNOSTIC_MODE nothing looked for, and DRY_RUN changes nothing. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=213
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

RVA='RVA 0x[0-9A-F]*'
OUTSIDE='0x[0-9A-F]* (outside the game)'
TAG='\[RenderQueues\] '
NOT_MEASURED='DIAGNOSTIC: the render queues are not measured: '
SUMMED='DIAGNOSTIC: the draw list.s references: '
# The fake's sites and data, as placed: nothing written - the data's pages read-only besides, so that a write would fault. The slots
# its load takes through the sites' own decoding, none wrong.
PLACED='render queues: built, sites as placed, data as left |'
LOAD='66 slots through the decoding of their sites, 0 wrong, no room for 0, 8 size lookups, 0 wrong, 5 frames, 81 references let go, 0 wrong'
rq_line() { log_line "${TAG}$1\$"; }
PEDS='DIAGNOSTIC: streamed peds: render data 14 in use of 120, at most 31; requests 0 of 55, at most 9; a frame.s array at most 63 entries of 120, 21 different in that frame.'
PROPS='DIAGNOSTIC: props of peds: render data 3 in use of 120, at most 8; requests 0 of 50, at most 4; a frame.s array at most 9 entries of 120, 8 different in that frame.'
VEHICLES='DIAGNOSTIC: streamed vehicles: render data 5 in use of 60, at most 11; requests 0 of 30, at most 3; a frame.s array at most 9 entries of 60, 9 different in that frame.'
REFERENCES='DIAGNOSTIC: the draw list.s references: at most 4210 of 10368 in a frame.'
FULL_STOPS=' - FULL: one more stops the game'
FULL_MODS=' - FULL: one more loses its mods'
PAST=' - PAST ITS CAPACITY: the game wrote past the array.s end, a crash may follow'
ALL_ARRAYS_NOT_MADE='render data not made yet; requests not made yet; a frame.s array not made yet.'
# The module's lines, the scanner's left out, each from what follows its tag.
rq_lines() { tr -d '\r' < TweaksV.log | sed -n 's/^.*\[RenderQueues\] //p'; }
# What every run but the ones that measure leaves: the module's one line, no thread, no problem, the game as placed.
measured_nothing() {
	log_lacks 'the render queues are read every' && log_lacks "${SUMMED}" && log_lacks 'PROBLEM' && has "$PLACED" && has 'protect: all restored (RX)'
}

# Every run goes side by side, each in a game folder of its own, its checks once all are done.
game_folder=$B
rm -rf "$B/runs"; mkdir -p "$B/runs"
# in_own_folder NAME: a game folder of its own for this run, with both builds, its results kept for the checks below
in_own_folder() {
	B=$game_folder/runs/$1
	mkdir -p "$B" && cd "$B" || exit 1
	cp ../../TweaksV.asi ../../TweaksV_test.asi .
}
# a_run NAME OPTION...: the fake with these options, DIAGNOSTIC_MODE=1 and the test build, in the background; KEYS, if set, the line
# addkey adds, as "KEY VALUE"; RELEASE=1 the Release build, the one that ships; OFF=1 DIAGNOSTIC_MODE=0, DRY=1 DRY_RUN=1, PLAIN=1 the plain
# fake game; its $out in out.txt; a NAME's colon a dash in its folder's name
a_run() {
	local name=${1//:/-}; shift
	(
		in_own_folder "$name"
		fresh_ini
		[ -n "${OFF:-}" ] || setkey DIAGNOSTIC_MODE 1
		[ -z "${DRY:-}" ] || setkey DRY_RUN 1
		[ -z "${PLAIN:-}" ] || setkey LARGE_SIREN_IDS 0 SIRENSETTINGS
		[ -z "${KEYS:-}" ] || addkey ${KEYS}
		ASI=$([ -n "${RELEASE:-}" ] && echo TweaksV.asi || echo TweaksV_test.asi) game "../../$([ -n "${PLAIN:-}" ] && echo plain_game.exe || echo full_game.exe)" "$@" > out.txt
	) &
	running=$((running + 1))
	if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
}
# of_run NAME: the checks below read that run
of_run() { B=$game_folder/runs/${1//:/-}; cd "$B" && out=$(< out.txt); }
running=0

OFF=1 RELEASE=1 a_run off rqload wait
OFF=1 PLAIN=1 RELEASE=1 a_run offnogame
RELEASE=1 a_run plaingame rqrvas rqload wait
DRY=1 RELEASE=1 a_run dryrun rqload wait
PLAIN=1 RELEASE=1 a_run nogame
# The fake's refusals, each a check failing on its own: NAME is rqbad's WHAT and N, its line word for word.
POOLS=('StreamPed req data' 'StreamPed render data' 'PedProp req data' 'PedProp render data' 'CVehicleStreamRequestGfx' 'CVehicleStreamRenderGfx')
LABELS=(U-E-S1 U-E-S2 U-E-P1 U-E-P2 U-E-V1 U-E-V2)
LOOKUPS=(U-E-N1 U-E-N2 U-E-N3 U-E-N4 U-E-N5 U-E-N6)
FAMILIES=('streamed peds' 'ped props' 'streamed vehicles')
ADD_LABELS=(U-E-S3 U-E-P3 U-E-V4)
INIT_LABELS=(U-E-S4 U-E-P4 U-E-V6)
HOOKED='(another mod may have hooked it already)'
refusals=()
refusal() { refusals+=("$1"$'\t'"$2"); a_run "$1" "rqbad:$1"; }
for i in 0 1 2 3 4 5; do
	pool=${POOLS[i]}; label=${LABELS[i]}
	refusal "rol0:$i" "the pool of $pool.s rol at $RVA holds 0, not 1 to 63 ($label)."
	refusal "rol64:$i" "the pool of $pool.s rol at $RVA holds 64, not 1 to 63 ($label)."
	refusal "add64:$i" "the pool of $pool.s add at $RVA holds 64, not 0 to 63 ($label)."
	refusal "value:$i" "the pool of $pool.s flag at $RVA, value at $RVA and key at $RVA do not lie 8 bytes apart ($label)."
	refusal "key:$i" "the pool of $pool.s flag at $RVA, value at $RVA and key at $RVA do not lie 8 bytes apart ($label)."
	refusal "out:$i" "the pool of $pool.s fields at $RVA ($label) is not inside the game.s data."
	refusal "end:$i" "the pool of $pool.s fields at $RVA ($label) is not inside the game.s data."
	refusal "odd:$i" "the pool of $pool.s fields at $RVA ($label) is not aligned to 8 bytes."
	refusal "unread:$i" "the pool of $pool.s fields at $RVA ($label) cannot be read."
	refusal "lookupout:$i" "the size lookup call of $pool at $RVA is not a call into the game $HOOKED (${LOOKUPS[i]})."
	refusal "lookupother:$i" "the size lookup call of $pool at $RVA leads to $RVA, the ones before it to $RVA (${LOOKUPS[i]})."
	refusal "edgelookup:$i" "the size lookup call of $pool at $RVA cannot be read (${LOOKUPS[i]})."
done
refusal add2:0 "the pool of StreamPed req data.s add at $RVA holds 2, not 1, the one its code rotates by (U-E-S1)."
refusal trap "the StreamPed render data.s full pool.s jump at $RVA leads to $RVA, which holds 90, not CC (U-E-S2)."
refusal edgetrap "the StreamPed render data.s full pool.s jump at $RVA leads to $RVA, which cannot be read (U-E-S2)."
refusal v2short "the CVehicleStreamRenderGfx.s full pool.s jump at $RVA leads to $RVA, outside its function at $RVA (unwind data) (U-E-V2)."
refusal v2nounwind "the CVehicleStreamRenderGfx.s full pool.s jump at $RVA leads to $RVA, and the game.s unwind data names no function there (U-E-V2)."
for f in 0 1 2; do
	what=${FAMILIES[f]}; add=${ADD_LABELS[f]}; init=${INIT_LABELS[f]}
	refusal "arraysout:$f" "the $what. arrays at $RVA ($add) is not inside the game.s data."
	refusal "arraysodd:$f" "the $what. arrays at $RVA ($add) is not aligned to 8 bytes."
	refusal "arraysunread:$f" "the $what. arrays at $RVA ($add) cannot be read."
	if [ "$f" != 2 ]; then
		refusal "arraysother:$f" "the $what. add loads their arrays at $RVA ($add), their init stores them at $RVA ($init)."
	else
		refusal "arraysother:$f" "the $what. walk loads their arrays at $RVA ($add), their init stores the first at $RVA ($init)."
	fi
	refusal "fliphead:$f" "the flip call at $RVA leads to $RVA, which does not hold a flip.s head 6 or 7 bytes on (U-E-F1)."
	refusal "flipindex:$f" "the flip at $RVA reads the index at $RVA, which no add reads (U-E-F1)."
	refusal "fliplock:$f" "the $what. flip at $RVA takes the lock at $RVA, not at $RVA (U-E-F1)."
	refusal "flipout:$f" "the flip call at $RVA is not a call into the game $HOOKED (U-E-F1)."
	refusal "flipunread:$f" "the flip call at $RVA leads to $RVA, which cannot be read (U-E-F1)."
done
for f in 0 1; do
	what=${FAMILIES[f]}; init=${INIT_LABELS[f]}
	for i in 1 2 3; do
		refusal "initarray:$f$i" "the $what. array $i at $RVA does not lie $((i * 16)) bytes past their first at $RVA ($init)."
	done
	for i in 0 3; do
		refusal "initcapacity:$f$i" "the $what. array $i at $RVA has its capacity at $RVA, not at its +0xA ($init)."
	done
done
refusal vehicleindex "the streamed vehicles. add reads the index at $RVA (U-E-V3), their walk at $RVA (U-E-V4)."
refusal vehicleinitarray "the streamed vehicles. walk loads their arrays at $RVA (U-E-V4), their init stores the first at $RVA (U-E-V6)."
refusal vehiclesize "the streamed vehicles. second size is stored at $RVA (U-E-V5), their init reads their arrays. size at $RVA (U-E-V6)."
refusal sizecall:0 "the size lookup call of VehicleStreamRequest at $RVA is not a call into the game $HOOKED (U-E-V5)."
refusal sizecall:1 "the size lookup call of VehicleStreamRender at $RVA is not a call into the game $HOOKED (U-E-V5)."
refusal flipsame "two of the frame.s calls lead to flips of the streamed vehicles. index at $RVA (U-E-F1)."
refusal managerout "the draw list manager.s global at $RVA (U-E-D3) is not inside the game.s data."
refusal managerodd "the draw list manager.s global at $RVA (U-E-D3) is not aligned to 8 bytes."
refusal managerunread "the draw list manager.s global at $RVA (U-E-D3) cannot be read."
LABELS_ALL=(U-E-S1 U-E-S2 U-E-S3 U-E-S4 U-E-P1 U-E-P2 U-E-P3 U-E-P4 U-E-V1 U-E-V2 U-E-V3 U-E-V4 U-E-V5 U-E-V6 U-E-N1 U-E-N2 U-E-N3 U-E-N4 U-E-N5 U-E-N6 U-E-F1 U-E-D1 U-E-D2 U-E-D3)
for label in "${LABELS_ALL[@]}"; do
	KEYS="TEST_BREAK_PATTERN $label" a_run "missing-$label" rqasked
	KEYS="TEST_TWICE_PATTERN $label" a_run "twice-$label" rqasked
done
# The runs that measure, or show that nothing is measured, 10 seconds and more each
KEYS="TEST_BREAK_PATTERN U-E-S1" a_run stopped rqasked wait
a_run nothing rqasked wait
a_run nomake rqnomake:1 rqnomake:2 rqnomake:5 rqload wait
a_run rs rqrs rqload wait
a_run stride rqstride rqmade wait
a_run stridezero rqstridezero rqmade wait
a_run stridebrief rqstridebrief rqload wait
a_run poolimage rqpoolimage:2 rqmade wait
a_run poolgone rqpoolgone:1 rqload wait
a_run nomanager rqnomanager rqload wait
a_run managergone rqmanagergone rqload wait
a_run entriesgone rqentriesgone rqmade wait
a_run nulls rqnulls rqmade wait
a_run gone rqgone rqload wait
a_run full rqfull wait
a_run fullrefs rqfullrefs wait
a_run past rqpast wait
a_run pastnow rqpast
a_run many rqmany wait
a_run datagone rqdatagone rqload wait
export TWEAKSV_TEST_SUMMARY_MS=2000
a_run grow rqgrow rqload wait
a_run latest rqlatest rqload wait
unset TWEAKSV_TEST_SUMMARY_MS
export TWEAKSV_TEST_REFUSE_THREAD='1 render queues sampling'
a_run nosampling rqasked wait
export TWEAKSV_TEST_REFUSE_THREAD='1 render queues summary'
a_run nosummary rqstride rqmade wait
unset TWEAKSV_TEST_REFUSE_THREAD
wait

echo "== U1 DIAGNOSTIC_MODE=0: nothing looked for, nothing measured, nothing of the fake's changed"
of_run off; echo "      $out"
check "no render queues pattern looked for, no line of the module, no problem" "log_lacks 'U-E-' && log_lacks '$TAG' && log_lacks 'PROBLEM'"
check "the fake's load ran on its sites all the same, the sites and the data as placed, every page its own protection" "has '$PLACED' && has '$LOAD' && has 'protect: all restored (RX)'"
of_run offnogame
check "the plain fake game, where a search logs its miss, as the other parts' do: no render queues pattern looked for" "log_has 'Pattern [A-Z]-E[-A-Z0-9]* not found' && log_lacks 'U-E-' && log_lacks '$TAG'"

echo "== U2 DIAGNOSTIC_MODE=1, the plain game: every site's line at the fake's RVA, the patterns' own bytes run, the summary 10 s in, word for word"
of_run plaingame; echo "      $out"
rvas=$(tr '|' '\n' <<< "$out" | sed -n 's/^ *rq rvas: //p')
rq_rva() { for pair in $rvas; do [ "${pair%%=*}" = "$1" ] && { echo "RVA ${pair#*=}"; return; }; done; echo "RVA of $1 not reported"; }
check "the 24 patterns each found exactly once" "[ \$(grep -c -E 'Pattern U-E-[A-Z][0-9] found: .* 1 match,' TweaksV.log) -eq 24 ]"
# The module's lines in their order, each with what the fake reports. Each pool: its make, its fields, the stride its code hands it.
pool_line() { echo "DIAGNOSTIC: pool of $2 at $(rq_rva "$1"): its flag, value and key at $(rq_rva "$3"), rol $4, add $5, stride $6 ($7)"; }
lookup_line() { echo "DIAGNOSTIC: size lookup call of $1 at $(rq_rva "$2") -> $(rq_rva LK) ($3)"; }
flip_line() { echo "DIAGNOSTIC: $1' flip, called at $(rq_rva "K$2"), at $(rq_rva "FF$2"): its head at $(rq_rva "H$2"), its lock at $(rq_rva "L$3"), its index at $(rq_rva "I$3") (U-E-F1)"; }
{
	pool_line S1 'StreamPed req data' F0 0x1B 0x1 0x508 U-E-S1
	pool_line S2 'StreamPed render data' F1 0x1C 0x3 0x2E0 U-E-S2
	pool_line P1 'PedProp req data' F2 0x1B 0x2 0x80 U-E-P1
	pool_line P2 'PedProp render data' F3 0x1D 0x5 0x3C U-E-P2
	pool_line V1 'CVehicleStreamRequestGfx' F4 0x1D 0x5 0x1C8 U-E-V1
	pool_line V2 'CVehicleStreamRenderGfx' F5 0x1E 0x3 0xBB0 U-E-V2
	echo "DIAGNOSTIC: StreamPed render data's full pool's jump at $(rq_rva TJ) -> $(rq_rva T), the trap: CC (U-E-S2)"
	echo "DIAGNOSTIC: CVehicleStreamRenderGfx's full pool's jump at $(rq_rva VJ) -> $(rq_rva VB), in its function at $(rq_rva VF) (unwind data) (U-E-V2)"
	echo "DIAGNOSTIC: streamed peds' add at $(rq_rva S3): its index at $(rq_rva I0), its arrays at $(rq_rva A0) (U-E-S3)"
	echo "DIAGNOSTIC: streamed peds' init at $(rq_rva S4): 4 arrays from $(rq_rva A0), 16 bytes apart, 120 entries each, their capacities at their +0xA (U-E-S4)"
	echo "DIAGNOSTIC: ped props' add at $(rq_rva P3): its index at $(rq_rva I1), its arrays at $(rq_rva A1) (U-E-P3)"
	echo "DIAGNOSTIC: ped props' init at $(rq_rva P4): 4 arrays from $(rq_rva A1), 16 bytes apart, 120 entries each, their capacities at their +0xA (U-E-P4)"
	echo "DIAGNOSTIC: streamed vehicles' add at $(rq_rva V3): its index at $(rq_rva I2) (U-E-V3)"
	echo "DIAGNOSTIC: streamed vehicles' walk at $(rq_rva V4): its lock at $(rq_rva L2), its index at $(rq_rva I2), its arrays at $(rq_rva A2) (U-E-V4)"
	echo "DIAGNOSTIC: streamed vehicles' two sizes at $(rq_rva V5): stored at $(rq_rva ZR) and $(rq_rva ZV) (U-E-V5)"
	echo "DIAGNOSTIC: streamed vehicles' init at $(rq_rva V6): their arrays' size read at $(rq_rva ZV), the first at $(rq_rva A2) (U-E-V6)"
	lookup_line VehicleStreamRequest CA U-E-V5
	lookup_line VehicleStreamRender CB U-E-V5
	lookup_line 'StreamPed req data' C0 U-E-N1
	lookup_line 'StreamPed render data' C1 U-E-N2
	lookup_line 'PedProp req data' C2 U-E-N3
	lookup_line 'PedProp render data' C3 U-E-N4
	lookup_line CVehicleStreamRequestGfx C4 U-E-N5
	lookup_line CVehicleStreamRenderGfx C5 U-E-N6
	flip_line 'streamed vehicles' 0 2
	flip_line 'streamed peds' 1 0
	flip_line 'ped props' 2 1
	echo "DIAGNOSTIC: the draw list's references at $(rq_rva D1): the first array at 0x5D0 of its manager, its capacity holds 10368 (U-E-D1)"
	echo "DIAGNOSTIC: the draw list's rotation at $(rq_rva D2): the references' index at 0xAC of its manager (U-E-D2)"
	echo "DIAGNOSTIC: the draw list manager made at $(rq_rva D3), 0x14BA0 bytes, its global at $(rq_rva MG) (U-E-D3)"
	echo "DIAGNOSTIC: the render queues are read every 20 ms: how near their pools, their arrays and the draw list's references have come is logged after 10 seconds, then every minute while a most changes."
	echo "DIAGNOSTIC: streamed peds: render data 14 in use of 120, at most 31; requests 0 of 55, at most 9; a frame's array at most 63 entries of 120, 21 different in that frame."
	echo "DIAGNOSTIC: props of peds: render data 3 in use of 120, at most 8; requests 0 of 50, at most 4; a frame's array at most 9 entries of 120, 8 different in that frame."
	echo "DIAGNOSTIC: streamed vehicles: render data 5 in use of 60, at most 11; requests 0 of 30, at most 3; a frame's array at most 9 entries of 60, 9 different in that frame."
	echo "DIAGNOSTIC: the draw list's references: at most 4210 of 10368 in a frame."
} > want.txt
rq_lines > got.txt
check "every line of the module, word for word and in its order, each site at the fake's own RVA, then the four of the summary" "diff want.txt got.txt > diff.txt"
[ -s diff.txt ] && sed 's/^/      /' diff.txt | cut -c1-240
check "the fake's load ran the patterns' own bytes: its slots through each make's decoding of each pool's pointer, none wrong, every size lookup" "has '$LOAD'"
check "nothing written: the sites and the data as placed, the module's reads their only company, every page its own protection, no problem" "has '$PLACED' && has 'protect: all restored (RX)' && log_lacks 'PROBLEM'"
plain_diag=$(rq_lines)
diag_stubs=$(grep -o 'stubs: [0-9]*, [^|]*' <<< "$out")

echo "== U3 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the same lines, since the module writes nothing to begin with"
of_run dryrun
check "the module's lines the same, word for word; no problem" "[ \"\$(rq_lines)\" = \"\$plain_diag\" ] && log_lacks 'PROBLEM' && has '$PLACED'"

echo "== U4 the plain fake game: none of it there - logged, not a problem"
of_run nogame
check "the scanner's 24 misses and the module's line, word for word, the first pattern named; none of it a problem" "[ \$(grep -c -E 'Pattern U-E-[A-Z][0-9] not found: ' TweaksV.log) -eq 24 ] && rq_line '${NOT_MEASURED}U-E-S1 was not found.' && log_lacks '${TAG}PROBLEM' && [ \$(count '$TAG') -eq 1 ]"

echo "== U5 each check failing on its own: its line word for word, nothing measured, the game as placed, no problem"
for refused in "${refusals[@]}"; do
	name=${refused%%$'\t'*}; line=${refused#*$'\t'}
	of_run "$name"
	check "$name: $line" "rq_line \"\${NOT_MEASURED}\$line\" && measured_nothing"
done

echo "== U6 each pattern missing and each held twice (the test build): the module's line in the scanner's words, nothing measured"
for label in "${LABELS_ALL[@]}"; do
	of_run "missing-$label"
	check "$label missing: the scanner's test line and the module's, word for word" "log_has 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN)\.' && rq_line '${NOT_MEASURED}$label was not found.' && measured_nothing"
	of_run "twice-$label"
	check "$label held twice: the scanner's test line and the module's, word for word" "log_has 'TEST BUILD: pattern $label is treated as held once more than the game code holds it (TEST_TWICE_PATTERN)\.' && rq_line '${NOT_MEASURED}the game code holds $label 2 or more times, not the expected 1.' && measured_nothing"
done

echo "== U6b a pattern missing, 11 s more: no thread started, no summary"
of_run stopped
check "the module's line alone, no summary, as the plain fake game's words: the module stopped before any thread" "rq_line '${NOT_MEASURED}U-E-S1 was not found.' && measured_nothing"

echo "== U7 nothing made yet, a summary all the same"
of_run nothing
check "every pool, every array and the references not made yet, word for word, one summary, no problem" "rq_line 'DIAGNOSTIC: streamed peds: $ALL_ARRAYS_NOT_MADE' && rq_line 'DIAGNOSTIC: props of peds: $ALL_ARRAYS_NOT_MADE' && rq_line 'DIAGNOSTIC: streamed vehicles: $ALL_ARRAYS_NOT_MADE' && rq_line 'DIAGNOSTIC: the draw list.s references: not made yet.' && [ \$(count '$SUMMED') -eq 1 ] && log_lacks 'PROBLEM' && has '$PLACED'"

echo "== U8 r and s of the fake's own, each pool's pointer encrypted by them: read, not assumed, but the first pool's s, whose code rotates at 1 alone (U5's add2)"
of_run rs
check "the numbers the same, word for word, each make's own bytes decoding each pointer" "rq_line '$PEDS' && rq_line '$PROPS' && rq_line '$VEHICLES' && rq_line '$REFERENCES' && has '$LOAD' && has '$PLACED'"
check "each pool's site line with its rol and its add as the fake placed them" "rq_line 'DIAGNOSTIC: pool of StreamPed req data at $RVA: its flag, value and key at $RVA, rol 0x21, add 0x1, stride 0x508 (U-E-S1)' && rq_line 'DIAGNOSTIC: pool of StreamPed render data at $RVA: its flag, value and key at $RVA, rol 0xD, add 0x9, stride 0x2E0 (U-E-S2)' && rq_line 'DIAGNOSTIC: pool of CVehicleStreamRequestGfx at $RVA: its flag, value and key at $RVA, rol 0x3F, add 0x3F, stride 0x1C8 (U-E-V1)' && rq_line 'DIAGNOSTIC: pool of CVehicleStreamRenderGfx at $RVA: its flag, value and key at $RVA, rol 0x1, add 0x1F, stride 0xBB0 (U-E-V2)'"

echo "== U9 a pool not made: its flag's bit 0 clear, its other bits, its value and its key set; the others read"
of_run nomake
check "the three not made yet, each in its place; the others' numbers" "rq_line 'DIAGNOSTIC: streamed peds: render data not made yet; requests 0 of 55, at most 9; a frame.s array at most 0 entries of 120.' && rq_line 'DIAGNOSTIC: props of peds: render data 3 in use of 120, at most 8; requests not made yet; a frame.s array at most 9 entries of 120, 8 different in that frame.' && rq_line 'DIAGNOSTIC: streamed vehicles: render data not made yet; requests 0 of 30, at most 3; a frame.s array at most 0 entries of 60.' && log_lacks 'PROBLEM' && has '$PLACED'"

echo "== U10 a pool's stride not as its code hands it, 50 samples in a row: the line once, the pool not read further; shorter spells say nothing"
of_run stride
check "each of the six pools' line once, word for word, its address as every line gives one" "[ \$(count 'has stride ') -eq 6 ] && rq_line 'DIAGNOSTIC: the pool of StreamPed req data at $OUTSIDE has stride 1289, not 1288: it is not read further.' && rq_line 'DIAGNOSTIC: the pool of StreamPed render data at $OUTSIDE has stride 737, not 736: it is not read further.' && rq_line 'DIAGNOSTIC: the pool of PedProp req data at $OUTSIDE has stride 129, not 128: it is not read further.' && rq_line 'DIAGNOSTIC: the pool of PedProp render data at $OUTSIDE has stride 61, not 60: it is not read further.' && rq_line 'DIAGNOSTIC: the pool of CVehicleStreamRequestGfx at $OUTSIDE has stride 457, not 456: it is not read further.' && rq_line 'DIAGNOSTIC: the pool of CVehicleStreamRenderGfx at $OUTSIDE has stride 2993, not 2992: it is not read further.'"
check "the summary says so in each pool's place, the arrays read beside them; no problem" "rq_line 'DIAGNOSTIC: streamed peds: render data not read further (see above); requests not read further (see above); a frame.s array at most 0 entries of 120.' && rq_line 'DIAGNOSTIC: streamed vehicles: render data not read further (see above); requests not read further (see above); a frame.s array at most 0 entries of 60.' && log_lacks 'PROBLEM' && has '$PLACED'"
of_run stridezero
check "a stride of 0 that stays: the same line, memory of 0 being no pool either" "[ \$(count 'has stride 0, not ') -eq 6 ] && rq_line 'DIAGNOSTIC: streamed peds: render data not read further (see above); requests not read further (see above); a frame.s array at most 0 entries of 120.' && log_lacks 'PROBLEM'"
of_run stridebrief
check "wrong for more than 50 samples in all, never 50 in a row: no line, the numbers those of the plain game" "log_lacks 'has stride ' && rq_line '$PEDS' && rq_line '$PROPS' && rq_line '$VEHICLES' && rq_line '$REFERENCES' && has '$LOAD' && log_lacks 'PROBLEM'"
of_run poolimage
check "a pointer that leads inside the game: its RVA in the line, no address that moves with each start" "rq_line 'DIAGNOSTIC: the pool of PedProp req data at $RVA has stride 0, not 128: it is not read further.' && [ \$(count 'has stride ') -eq 1 ] && log_lacks 'PROBLEM'"

echo "== U11 what cannot be read: tried 50 times, each a first-chance fault through every handler in the process, then said once and no more - never a fault of the game's, never a problem"
of_run poolgone
check "a pointer that cannot be read: the line once, its address as every line gives one, 50 faults, the summary saying so, the others read" "rq_line 'DIAGNOSTIC: the pool of StreamPed render data at $OUTSIDE cannot be read: it is not read further.' && [ \$(count '${TAG}.*cannot be read') -eq 1 ] && has 'render queues faults: 50 |' && rq_line 'DIAGNOSTIC: streamed peds: render data not read further (see above); requests 0 of 55, at most 9; a frame.s array at most 0 entries of 120.' && rq_line '$PROPS' && log_lacks 'PROBLEM'"
of_run nomanager
check "the manager not made: the references not made yet, the rest read" "rq_line 'DIAGNOSTIC: the draw list.s references: not made yet.' && rq_line '$VEHICLES' && has 'render queues faults: 0 |' && log_lacks 'PROBLEM'"
of_run managergone
check "a manager that cannot be read: the line once, 50 faults, the summary saying so" "rq_line 'DIAGNOSTIC: the draw list.s references at $OUTSIDE cannot be read: they are not read further.' && [ \$(count '${TAG}.*cannot be read') -eq 1 ] && has 'render queues faults: 50 |' && rq_line 'DIAGNOSTIC: the draw list.s references: not read further (see above).' && log_lacks 'PROBLEM'"
of_run entriesgone
check "entries that cannot be read, or an array's pointer of 0: the count kept, the entries said; a pool's count below 0 read as none; no problem" "rq_line 'DIAGNOSTIC: streamed peds: render data 0 in use of 120, at most 0; requests 0 of 55, at most 0; a frame.s array at most 40 entries of 120, the entries could not be read.' && rq_line 'DIAGNOSTIC: streamed vehicles: render data 0 in use of 60, at most 0; requests 0 of 30, at most 0; a frame.s array at most 7 entries of 60, the entries could not be read.' && log_lacks 'PROBLEM'"
of_run nulls
check "entries not yet written, as a fresh array holds them, 0: left out of the different ones" "rq_line 'DIAGNOSTIC: props of peds: render data 0 in use of 120, at most 0; requests 0 of 50, at most 0; a frame.s array at most 10 entries of 120, 5 different in that frame.' && log_lacks 'PROBLEM'"
of_run gone
check "made and read, then a flag cleared, a pointer or a manager that cannot be read: nothing said to be not made, what was read stands beside the words, 50 faults each" "rq_line 'DIAGNOSTIC: streamed peds: render data 0 in use of 120, at most 31; requests 0 of 55, at most 9; a frame.s array at most 63 entries of 120, 21 different in that frame.' && rq_line 'DIAGNOSTIC: props of peds: render data not read further (see above), until then at most 8 of 120; requests 0 of 50, at most 4; a frame.s array at most 9 entries of 120, 8 different in that frame.' && rq_line 'DIAGNOSTIC: the draw list.s references: not read further (see above), until then at most 4210 of 10368 in a frame.' && rq_line 'DIAGNOSTIC: the pool of PedProp render data at $OUTSIDE cannot be read: it is not read further.' && has 'render queues faults: 100 |' && log_lacks 'PROBLEM'"
of_run datagone
faults=$(sed -n 's/.*render queues faults: \([0-9]*\) |.*/\1/p' <<< "$out")
check "the page of the game's data taken away: each read's line once, ten in all, word for word, the fake ends" "[ \$(count '${TAG}.*cannot be read: .*not read further\.') -eq 10 ] && rq_line 'DIAGNOSTIC: the pool of StreamPed req data.s fields at $RVA cannot be read: it is not read further.' && rq_line 'DIAGNOSTIC: the ped props. arrays at $RVA cannot be read: they are not read further.' && rq_line 'DIAGNOSTIC: the draw list manager.s global at $RVA cannot be read: the references are not read further.' && has 'protect: all restored (RX)' && log_lacks 'PROBLEM'"
check "and the summary: what was read stands, and no read goes on past its 50: at most 950 faults, 6 and 1 sources of one read and 3 of four" "rq_line 'DIAGNOSTIC: streamed peds: render data not read further (see above), until then at most 31 of 120; requests not read further (see above), until then at most 9 of 55; a frame.s array not read further (see above), until then at most 63 entries of 120, 21 different in that frame.' && rq_line 'DIAGNOSTIC: the draw list.s references: not read further (see above), until then at most 4210 of 10368 in a frame.' && [ '$faults' -ge 900 ] && [ '$faults' -le 950 ]"

echo "== U12 every pool at its capacity: FULL, in the words of what one more does"
of_run full
check "the peds' and the props' pools stop the game" "rq_line 'DIAGNOSTIC: streamed peds: render data 120 in use of 120, at most 120$FULL_STOPS; requests 55 of 55, at most 55$FULL_STOPS; a frame.s array at most 63 entries of 120, 21 different in that frame.' && rq_line 'DIAGNOSTIC: props of peds: render data 120 in use of 120, at most 120$FULL_STOPS; requests 50 of 50, at most 50$FULL_STOPS; a frame.s array at most 9 entries of 120, 8 different in that frame.'"
check "the vehicles' render pool takes the vehicle's mods, their request pool stops the game; the sites' decoding never short of room" "rq_line 'DIAGNOSTIC: streamed vehicles: render data 60 in use of 60, at most 60$FULL_MODS; requests 30 of 30, at most 30$FULL_STOPS; a frame.s array at most 9 entries of 60, 9 different in that frame.' && has ', 0 wrong, no room for 0, ' && log_lacks 'PROBLEM'"
of_run fullrefs
check "the draw list's references at their capacity: what past it does" "rq_line 'DIAGNOSTIC: the draw list.s references: at most 10368 of 10368 in a frame - FULL: past it a streaming reference is taken and never released, a silent leak.' && log_lacks 'PROBLEM'"

echo "== U13 the arrays past their capacity: a line of the log, at once, no box (D30)"
of_run pastnow
check "the fake ends long before a summary, as a crash may: each family's first past in the log all the same, word for word" "rq_line 'DIAGNOSTIC: streamed peds: a frame.s array at [0-9]* entries of 120$PAST.' && rq_line 'DIAGNOSTIC: props of peds: a frame.s array at [0-9]* entries of 120$PAST.' && rq_line 'DIAGNOSTIC: streamed vehicles: a frame.s array at [0-9]* entries of 60$PAST.' && log_lacks '$SUMMED'"
of_run past
check "each family's line once, 11 seconds on" "[ \$(count 'a frame.s array at [0-9]* entries of ') -eq 3 ]"
check "each family past its capacity, word for word: the peds' 130, the props' 125, the vehicles' 70 of 60 with their render pool named larger" "rq_line 'DIAGNOSTIC: streamed peds: render data 120 in use of 120, at most 120$FULL_STOPS; requests 9 of 55, at most 9; a frame.s array at most 130 entries of 120, 65 different in that frame$PAST.' && rq_line 'DIAGNOSTIC: props of peds: render data 120 in use of 120, at most 120$FULL_STOPS; requests 4 of 50, at most 4; a frame.s array at most 125 entries of 120, 100 different in that frame$PAST.' && rq_line 'DIAGNOSTIC: streamed vehicles: render data 80 in use of 80, at most 80$FULL_MODS; requests 3 of 30, at most 3; a frame.s array at most 70 entries of 60, 70 different in that frame$PAST.'"
check "no problem, no box, the game as placed" "log_lacks 'PROBLEM' && log_lacks 'Start-up summary' && has '$PLACED'"
of_run many
check "5,000 entries: the different ones counted among the first 4,096" "rq_line 'DIAGNOSTIC: streamed peds: render data 14 in use of 120, at most 31; requests 0 of 55, at most 9; a frame.s array at most 5000 entries of 120, 31 different among the first 4096 in that frame$PAST.' && log_lacks 'PROBLEM'"

echo "== U14 the test build, its summaries every 2 s: again when a most changes, and not when only the latest numbers do"
of_run grow
check "the test build's line for this measurement" "log_line '\[TweaksV\] TEST BUILD: the render queues summary comes every 2000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=2000).\$'"
check "the first summary, then one with every family's new most" "rq_line '$PEDS' && rq_line '$REFERENCES' && rq_line 'DIAGNOSTIC: streamed peds: render data 32 in use of 120, at most 32; requests 0 of 55, at most 9; a frame.s array at most 75 entries of 120, 25 different in that frame.' && rq_line 'DIAGNOSTIC: props of peds: render data 3 in use of 120, at most 8; requests 1 of 50, at most 4; a frame.s array at most 13 entries of 120, 12 different in that frame.' && rq_line 'DIAGNOSTIC: streamed vehicles: render data 11 in use of 60, at most 11; requests 0 of 30, at most 3; a frame.s array at most 12 entries of 60, 12 different in that frame.' && rq_line 'DIAGNOSTIC: the draw list.s references: at most 5000 of 10368 in a frame.' && log_lacks 'PROBLEM' && has '$PLACED'"
of_run latest
check "only the latest numbers changed: the one summary" "rq_line '$PEDS' && [ \$(count '$SUMMED') -eq 1 ] && log_lacks 'PROBLEM' && has '$PLACED'"

echo "== U15 a thread that cannot start (the test build): logged, not a problem"
of_run nosampling
check "the sampling thread refused: its line and the module's, word for word; no summary thread, no summary" "log_line '\[TweaksV\] The render queues sampling thread could not be started (Windows error 8' && rq_line '${NOT_MEASURED}its sampling thread could not be started.' && measured_nothing"
of_run nosummary
check "the summary thread refused: its line and the module's, word for word; no summary" "log_line '\[TweaksV\] The render queues summary thread could not be started (Windows error 8' && rq_line '${NOT_MEASURED}its summary thread could not be started.' && measured_nothing"
check "and the sampling thread ended with it: the wrong strides of the pools the fake makes later are never read, so never said" "log_lacks 'has stride '"

echo "== U16 no stub of its own: the fake's stubs are the other parts'"
of_run stopped
off_stubs=$(grep -o 'stubs: [0-9]*, [^|]*' <<< "$out")
check "with the module stopped at its first pattern the same stubs as with it measuring" "[ -n '$diag_stubs' ] && [ '$off_stubs' = '$diag_stubs' ]"
B=$game_folder
cd "$B" && rm -rf runs

echo
echo "RENDER QUEUES SCENARIOS: $pass passed, $fail failed"
