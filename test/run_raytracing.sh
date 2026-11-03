#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The RayTracing part (TweaksVRayTracing): the ray tracing buffer of moving geometry and its two checks raised together in the
# full fake game, whose three sites are code it runs - nothing looked for with DYNAMIC_BVH_VERTICES=0 and DIAGNOSTIC_MODE off, the
# part off, the raise and nothing else, the cap, a value below the game's own, DRY_RUN; each check failing on its own, with the
# part's problem and nothing written, another mod's raise left as it is, each pattern missing or twice, the plain game; with
# DIAGNOSTIC_MODE every site logged and the measurement - the refusal jumps through counting stubs while two threads of the fake
# run the checks at once, every register the refusals find as the checks left it and the counts exact, the frames sampled and
# summed up 10 seconds in, again when the most taken grows and after a frame's flip; DRY_RUN alike, Windows' dynamic code
# policy, and a buffer the game has made already, whose refusal jumps are left as they are. Fake LOCALAPPDATA: the real one is
# never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=70
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# What the fake reports: the buffer as its set-up made it, the checks it ran, its sites and their five fields TweaksV may write
GAME_MADE='rt buffer: made with 4194304 vertices (stride word 0x7014), 1 maker call, flags 0x89, view 1, r9 0'
GAME_BUFFER="$GAME_MADE |"
RAISED_BUFFER='rt buffer: made with 8388608 vertices (stride word 0x7014), 1 maker call, flags 0x89, view 1, r9 0'
GAME_SITES='rt sites: as placed, count 4194304, checks 4194304 and 4194304, jumps to the game and the game |'
RAISED_SITES='rt sites: as placed, count 8388608, checks 8388608 and 8388608, jumps to the game and the game |'
STUBS='jumps to a stub, RX, on to its refusal and a stub, RX, on to its refusal |'
CHECKED='checks took 1, refused 1210 (5324359445 vertices, 10 ropes), registers kept 1210 of 1210 |'
# The part's lines (an apostrophe as any character)
rt_line() { log_line "\[RayTracing\] $1\$"; }
JOB='The ray tracing buffer of moving geometry is not raised this session: '
NOT_MEASURED='DIAGNOSTIC: the ray tracing buffer of moving geometry is not measured: '
ZERO='DYNAMIC_BVH_VERTICES=0 - the ray tracing buffer of moving geometry is not raised.'
MEASURED='DIAGNOSTIC: the ray tracing buffer of moving geometry is measured: the last finished frame.s vertices are sampled every 50 ms and logged after 10 seconds, then every minute while the most taken or the refusals change.'
FIRST_REFUSAL='DIAGNOSTIC: ray tracing buffer of moving geometry: the first geometry refused - the buffer is full, and an object it refuses has no ray traced shadow, reflection or occlusion in that frame.'
SUMMARY='DIAGNOSTIC: ray tracing buffer of moving geometry, '
FULL=' - the buffer is full: those objects had no ray traced shadow, reflection or occlusion in those frames.'
REFUSED="1210 geometries refused (5324359445 vertices in all), 10 of them ropes$FULL"

# rva NAME [PLUS]: "RVA 0x..." of what the fake's report places at NAME (set-up, maker, object, geometry, ropes, slot, counters,
# reset, hole, edge), plus PLUS
rva() {
	local at
	at=$(sed -nE "s/.*rt rvas:[^|]*[ ,]$1 (0x[0-9A-F]+)[ ,|].*/\1/p" <<< "$out" | head -n 1)
	[ -n "$at" ] || { echo "RVA-of-$1-not-reported"; return; }
	printf 'RVA 0x%X' $(( at + ${2:-0} ))
}
# patterns_found: the scanner's line for each pattern, found once where the fake has it
patterns_found() {
	log_has "Pattern T-E-B1 found: .* -> $(rva set-up 4), 1 match" && log_has "Pattern T-E-C1 found: .* -> $(rva geometry 42), 1 match" &&
		log_has "Pattern T-E-C2 found: .* -> $(rva ropes 42), 1 match"
}
# sites_logged HOLDS: the part's lines of the sites, word for word, with the fake's addresses, the immediates holding HOLDS
sites_logged() {
	rt_line "DIAGNOSTIC: buffer.s count at $(rva set-up 13) holds $1, the buffer maker.s call at $(rva set-up 28) -> $(rva maker), a function start (unwind data) (T-E-B1)" &&
	rt_line "DIAGNOSTIC: buffer object.s store at $(rva set-up 33) -> $(rva object), which holds 0: the buffer is not made yet (T-E-B1)" &&
	rt_line "DIAGNOSTIC: geometry.s check at $(rva geometry 84) holds $1, its frame slot at $(rva slot), its counters at $(rva counters), its refusal jump at $(rva geometry 90) -> $(rva geometry 112): 49 89 5F 38 49 89 4F 40 (T-E-C1)" &&
	rt_line "DIAGNOSTIC: ropes. check at $(rva ropes 81) holds $1, its frame slot at $(rva slot), its counters at $(rva counters), its refusal jump at $(rva ropes 87) -> $(rva ropes 109): 49 89 5F 38 49 89 4F 40 (T-E-C2)" &&
	log_has "\[RayTracing\] DIAGNOSTIC: the counters at $(rva counters) are referenced in 3 places of the game.s code: .*$(rva geometry 50)" &&
	log_has "\[RayTracing\] DIAGNOSTIC: the counters at $(rva counters) are referenced in 3 places of the game.s code: .*$(rva ropes 50)" &&
	log_has "\[RayTracing\] DIAGNOSTIC: the counters at $(rva counters) are referenced in 3 places of the game.s code: .*$(rva reset)"
}
# redirected: both refusal jumps rewritten and the redirect said, word for word
redirected() {
	rt_line "DIAGNOSTIC: geometry.s refusal jump at $(rva geometry 90): 0F 87 10 00 00 00 -> 0F 87 [0-9A-F ]*" &&
	rt_line "DIAGNOSTIC: ropes. refusal jump at $(rva ropes 87): 0F 87 10 00 00 00 -> 0F 87 [0-9A-F ]*" &&
	rt_line "DIAGNOSTIC: the refusal jumps at $(rva geometry 90) (T-E-C1) and $(rva ropes 87) (T-E-C2) are redirected to counting stubs: every geometry and rope the full buffer refuses is counted."
}
summaries() { count "$SUMMARY"; }
# at_ms PATTERN: the time of the first log line that matches, in ms of the day; first_after_10s: the first summary 10 seconds after
# the announcement - 9.9 at least, 20 at most on a busy machine
at_ms() {
	tr -d '\r' < TweaksV.log | grep -m1 -- "$1" | sed -nE 's/^([0-9]{2}):([0-9]{2}):([0-9]{2})\.([0-9]{3}) .*/\1 \2 \3 \4/p' |
		{ read -r h m s f && echo $(( (10#$h * 3600 + 10#$m * 60 + 10#$s) * 1000 + 10#$f )); }
}
first_after_10s() {
	local a f d
	a=$(at_ms 'ray tracing buffer of moving geometry is measured'); f=$(at_ms "$SUMMARY")
	[ -n "$a" ] && [ -n "$f" ] || return 1
	d=$(( f - a )); (( d < 0 )) && d=$(( d + 86400000 ))
	(( d >= 9900 && d <= 20000 ))
}

echo "== R1 defaults, DYNAMIC_BVH_VERTICES=0 and DIAGNOSTIC_MODE=0: nothing looked for or written"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "no pattern of the part looked for; its one line, word for word, and nothing more" "log_lacks 'T-E-' && rt_line '$ZERO' && [ \$(count '\[RayTracing\]') -eq 1 ]"
check "the sites as placed, the buffer made by the game's set-up at the game's own size, every page's protection its own" "has '$GAME_BUFFER' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R2 [RAYTRACING] ENABLED=0, with DIAGNOSTIC_MODE=1 and DYNAMIC_BVH_VERTICES=8388608: the part does nothing"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 RAYTRACING; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe)
check "no line of the part, no pattern looked for; the header says it is off and gives its value" "log_lacks '\[RayTracing\]' && log_lacks 'T-E-' && log_line 'Features: .*, Resources on, RayTracing off, LightCapacity off, SirenSettings on, StaticBounds on\$' && log_line '\[TweaksV\] RayTracing: DYNAMIC_BVH_VERTICES=8388608\$'"
check "nothing written: the sites as placed, the buffer at the game's own size" "has '$GAME_BUFFER' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R3 DYNAMIC_BVH_VERTICES=8388608: the buffer's count and its two checks raised, nothing else written"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe); echo "      $out"
check "the raise, word for word" "rt_line 'The ray tracing buffer of moving geometry is raised from 4194304 to 8388608 vertices (80 to 160 MB).'"
check "the three immediates 8388608 and the rest of the sites as placed, the jumps to the game; the buffer made with 8388608 vertices" "has '$RAISED_SITES' && has '$RAISED_BUFFER |'"
check "no problem, no DIAGNOSTIC line, no measurement; every page's protection its own" "log_lacks 'PROBLEM' && log_lacks 'DIAGNOSTIC: ' && log_lacks 'ray tracing buffer summary' && has 'protect: all restored (RX)'"

echo "== R4 DYNAMIC_BVH_VERTICES=30000000: above the cap, clamped and reported by the settings, raised to 26843545"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 30000000; out=$(game full_game.exe)
check "the settings' problem and the raise, word for word" "log_has 'PROBLEM: TweaksV.ini: \[RAYTRACING\] DYNAMIC_BVH_VERTICES=30000000 is outside 0..26843545 - using 26843545.' && rt_line 'The ray tracing buffer of moving geometry is raised from 4194304 to 26843545 vertices (80 to 512 MB).'"
check "the three immediates and the buffer at 26843545, the rest as placed" "has 'rt sites: as placed, count 26843545, checks 26843545 and 26843545, jumps to the game and the game |' && has 'rt buffer: made with 26843545 vertices' && has 'protect: all restored (RX)'"

echo "== R5 DYNAMIC_BVH_VERTICES=1000: below the game's own, raised to it and reported - nothing to patch"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 1000; out=$(game full_game.exe)
check "the part's problem and its line, word for word; the one problem" "log_has 'PROBLEM: TweaksV.ini: \[RAYTRACING\] DYNAMIC_BVH_VERTICES=1000 is outside 0 or 4194304..26843545 - using 4194304.' && rt_line 'The ray tracing buffer of moving geometry holds 4194304 vertices already (DYNAMIC_BVH_VERTICES=1000) - nothing to patch.' && log_has 'Start-up summary not shown (ERROR_POPUP=0): 1 problem'"
check "nothing written" "has '$GAME_BUFFER' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R5b DYNAMIC_BVH_VERTICES=4194304, the game's own size: nothing to patch, nothing reported"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 4194304; out=$(game full_game.exe)
check "the part's line, word for word; no problem; nothing written" "rt_line 'The ray tracing buffer of moving geometry holds 4194304 vertices already (DYNAMIC_BVH_VERTICES=4194304) - nothing to patch.' && log_lacks 'PROBLEM' && has '$GAME_BUFFER' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R6 DRY_RUN=1 with DYNAMIC_BVH_VERTICES=8388608: everything found and checked, the lines say what would be raised, nothing written"
fresh_ini; setkey DRY_RUN 1; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe)
check "the three writes, word for word" "rt_line 'DRY_RUN: would patch the ray tracing buffer.s count: 00 00 40 00 -> 00 00 80 00' && rt_line 'DRY_RUN: would patch the geometry.s capacity check: 00 00 40 00 -> 00 00 80 00' && rt_line 'DRY_RUN: would patch the ropes. capacity check: 00 00 40 00 -> 00 00 80 00'"
check "the raise that would be made, word for word, with the three addresses" "rt_line \"DRY_RUN: would raise the ray tracing buffer of moving geometry from 4194304 to 8388608 vertices ($(rva set-up 13), $(rva geometry 86) and $(rva ropes 83)).\""
check "no problem; nothing written" "log_lacks 'PROBLEM' && has '$GAME_BUFFER' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R7 DIAGNOSTIC_MODE=1, the key at 0: every site logged, the refusal jumps through counting stubs, the checks run from two threads, the summary 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe rtcheck wait); echo "      $out"
check "T-E-B1, T-E-C1 and T-E-C2 found once each, where the fake has them" "patterns_found"
check "the key's line; the lines of the sites, word for word, with the fake's addresses: the buffer not made yet, the refusals' first bytes, the counters' 3 references" "rt_line '$ZERO' && sites_logged 4194304"
check "both refusal jumps rewritten to the stubs, the redirect and the announcement word for word" "redirected && rt_line '$MEASURED'"
check "the fake's checks: one range taken, 1210 refusals - two threads at once - every register they found as the checks left it, rcx's upper half and rsp included" "has '$CHECKED'"
check "both jumps lead to a stub, read+execute, which leads on to the game's refusal; the rest of the sites as placed, nothing raised" "has 'rt sites: as placed, count 4194304, checks 4194304 and 4194304, $STUBS' && has '$GAME_MADE'"
check "the first refusal said once, word for word" "rt_line '$FIRST_REFUSAL' && [ \$(count 'the first geometry refused') -eq 1 ]"
check "the summary 10 s in, word for word: the last finished frame, the most, the refusals counted exactly - 64-bit, ecx zero-extended - the ropes among them" "rt_line \"${SUMMARY}made with 4194304 vertices of 20 bytes: the last frame took 2811203 of 4194304 vertices (54 of 80 MB), at most 2811203 so far; $REFUSED\" && first_after_10s"
check "one summary, no problem; every page's protection its own" "[ \$(summaries) -eq 1 ] && log_lacks 'PROBLEM' && has 'protect: all restored (RX)'"

echo "== R8 DIAGNOSTIC_MODE=1 and DYNAMIC_BVH_VERTICES=8388608: the raise, every write logged, and the measurement of the raised buffer"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe rtcheck wait)
check "the sites logged with the game's own 4194304, then the three writes and the raise, word for word" "sites_logged 4194304 && rt_line \"DIAGNOSTIC: ray tracing buffer.s count at $(rva set-up 13): 00 00 40 00 -> 00 00 80 00\" && rt_line \"DIAGNOSTIC: geometry.s capacity check at $(rva geometry 86): 00 00 40 00 -> 00 00 80 00\" && rt_line \"DIAGNOSTIC: ropes. capacity check at $(rva ropes 83): 00 00 40 00 -> 00 00 80 00\" && rt_line 'The ray tracing buffer of moving geometry is raised from 4194304 to 8388608 vertices (80 to 160 MB).'"
check "the jumps redirected; the fake's checks counted and their registers kept; the buffer made with 8388608 vertices" "redirected && has '$CHECKED' && has '$RAISED_BUFFER' && has 'rt sites: as placed, count 8388608, checks 8388608 and 8388608, $STUBS'"
check "the summary 10 s in, word for word, of the raised buffer" "rt_line \"${SUMMARY}made with 8388608 vertices of 20 bytes: the last frame took 2811203 of 8388608 vertices (54 of 160 MB), at most 2811203 so far; $REFUSED\" && first_after_10s && [ \$(summaries) -eq 1 ] && log_lacks 'PROBLEM'"

echo "== R9 the test build, TWEAKSV_TEST_SUMMARY_MS=500: a summary when the most taken grows, and after the frame flips to the other slot"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe rtgrow)
check "the fake grew the frame and flipped it after the first summary" "has ', grown and flipped after the first summary |'"
check "three summaries, word for word: the game's frame, the grown one, then the other slot's after the flip - none while nothing changes" "rt_line \"${SUMMARY}made with 4194304 vertices of 20 bytes: the last frame took 2811203 of 4194304 vertices (54 of 80 MB), at most 2811203 so far; no geometry refused.\" && rt_line \"${SUMMARY}made with 4194304 vertices of 20 bytes: the last frame took 3987120 of 4194304 vertices (76 of 80 MB), at most 3987120 so far; no geometry refused.\" && rt_line \"${SUMMARY}made with 4194304 vertices of 20 bytes: the last frame took 4100000 of 4194304 vertices (78 of 80 MB), at most 4100000 so far; no geometry refused.\" && [ \$(summaries) -eq 3 ]"
check "no refusal said, no problem" "log_lacks 'the first geometry refused' && log_lacks 'PROBLEM'"

echo "== R10 TEST_BREAK_PATTERN for each pattern, DYNAMIC_BVH_VERTICES=8388608 (test build, ERROR_POPUP=1): the part's problem in the box, nothing written"
for label in T-E-B1 T-E-C1 T-E-C2; do
	fresh_ini; setkey ERROR_POPUP 1; setkey DYNAMIC_BVH_VERTICES 8388608; addkey TEST_BREAK_PATTERN "$label"; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the scanner's line and the problem, word for word, in the box; nothing written" "log_has 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).' && rt_line 'PROBLEM: ${JOB}$label was not found.' && log_has 'TEST BUILD: message box not shown' && has '$GAME_SITES' && has 'protect: all restored (RX)'"
done

echo "== R11 each check failing on its own, DYNAMIC_BVH_VERTICES=8388608: the part's problem word for word, nothing written"
refused() {	# refused OPTION SITES WHY: the fake with OPTION; its sites as SITES, and the part's problem WHY
	fresh_ini; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe "$1")
	check "$1: the problem, word for word, the one problem; nothing written" "rt_line \"PROBLEM: $JOB$3\" && log_has 'Start-up summary not shown (ERROR_POPUP=0): 1 problem' && has '$2' && has 'protect: all restored (RX)'"
}
refused rtmakerhooked "$GAME_SITES" "the buffer maker.s call at \$(rva set-up 28) is not a call into the game (another mod may have hooked it already) (T-E-B1)."
refused rtmakerinside "$GAME_SITES" "the buffer maker.s call at \$(rva set-up 28) leads to \$(rva maker 16), which is not a function start (inside the function at \$(rva maker) (unwind data)) (T-E-B1)."
refused rtslot "$GAME_SITES" "the two checks load different frame slots: \$(rva slot) (T-E-C1) and \$(rva slot 4) (T-E-C2)."
refused rtcounters "$GAME_SITES" "the two checks load different counters: \$(rva counters) (T-E-C1) and \$(rva counters 4) (T-E-C2)."
refused rtextra "$GAME_SITES" "the game.s code references the counters at \$(rva counters) in 4 places or more, not 3 - the two checks and the frame.s reset, as the research found them."
refused rtjumphooked 'rt sites: as placed, count 4194304, checks 4194304 and 4194304, jumps to elsewhere and the game |' "the geometry.s refusal jump at \$(rva geometry 90) does not lead into the game (another mod may have hooked it already) (T-E-C1)."
refused rtmismatch 'rt sites: as placed, count 4194304, checks 4194304 and 8388608, jumps to the game and the game |' "the buffer.s count and its two checks disagree: 4194304 at \$(rva set-up 13) (T-E-B1), 4194304 at \$(rva geometry 86) (T-E-C1) and 8388608 at \$(rva ropes 83) (T-E-C2)."
refused rtmade "$GAME_SITES" "the game has made the buffer already - its object at \$(rva object) is set (T-E-B1) - and a raise of its checks alone would let the game write past it."
refused rtnounwind "$GAME_SITES" "the buffer maker.s call at \$(rva set-up 28) leads to \$(rva reset), which is not a function start (no unwind data) (T-E-B1)."
refused rtobjectout "$GAME_SITES" "the buffer object.s global at 0x[0-9A-F]* (outside the game) (T-E-B1) is not inside the game."
refused rtobjectunread "$GAME_SITES" "the buffer object.s global at \$(rva hole) (T-E-B1) cannot be read."
refused rtslotout "$GAME_SITES" "the frame slot at 0x[0-9A-F]* (outside the game) (T-E-C2) is not inside the game."
refused rtslotunread "$GAME_SITES" "the frame slot at \$(rva hole) (T-E-C1) cannot be read."
refused rtcountersout "$GAME_SITES" "the counters at 0x[0-9A-F]* (outside the game) (T-E-C1) are not inside the game."
refused rtcountersend "$GAME_SITES" "the counters at RVA 0x[0-9A-F]* (T-E-C1) are not inside the game."
refused rtcountersunread "$GAME_SITES" "the counters at \$(rva hole) (T-E-C2) cannot be read."
refused rtjumpunread "$GAME_SITES" "the ropes. refusal jump at \$(rva edge 4094) cannot be read (T-E-C2)."
refused rtropeshooked 'rt sites: as placed, count 4194304, checks 4194304 and 4194304, jumps to the game and elsewhere |' "the ropes. refusal jump at \$(rva ropes 87) does not lead into the game (another mod may have hooked it already) (T-E-C2)."
refused rtnoreset "$GAME_SITES" "the game.s code references the counters at \$(rva counters) in 2 places, not 3 - the two checks and the frame.s reset, as the research found them."
refused rtobjectodd "$GAME_SITES" "the buffer object.s global at \$(rva object 4) (T-E-B1) is not aligned to 8 bytes."
refused rtslotodd "$GAME_SITES" "the frame slot at \$(rva slot 2) (T-E-C1) is not aligned to 4 bytes."
refused rtcountersodd "$GAME_SITES" "the counters at \$(rva counters 2) (T-E-C2) are not aligned to 4 bytes."
refused rtsetuptwice "$GAME_SITES" "the game code holds T-E-B1 2 or more times, not the expected 1."
refused rtgeometrytwice "$GAME_SITES" "the game code holds T-E-C1 2 or more times, not the expected 1."
refused rtropestwice "$GAME_SITES" "the game code holds T-E-C2 2 or more times, not the expected 1."

echo "== R12 another mod raised the buffer and its checks to 8388608, DYNAMIC_BVH_VERTICES=8388608: left as it is, logged"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe rtraised)
check "the line, word for word, not a problem; the sites as the other mod left them" "rt_line '${JOB}another mod already set it to 8388608 vertices - left as it is.' && log_lacks 'PROBLEM' && has '$RAISED_SITES' && has '$RAISED_BUFFER |' && has 'protect: all restored (RX)'"

echo "== R13 a check failing with the key at 0 and DIAGNOSTIC_MODE=1: logged, not reported - no measurement, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe rtslot)
check "the measurement's line, word for word; no problem, no redirect, no announcement" "rt_line \"${NOT_MEASURED}the two checks load different frame slots: $(rva slot) (T-E-C1) and $(rva slot 4) (T-E-C2).\" && log_lacks 'PROBLEM' && log_lacks 'redirected to counting stubs' && log_lacks '\[RayTracing\] .*is measured' && has '$GAME_SITES'"

echo "== R14 the plain game: the patterns missing - a problem with the key raised, a DIAGNOSTIC line with the key at 0"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game plain_game.exe)
check "the scanner's three lines and the part's problem, word for word" "[ \$(grep -c -E 'Pattern T-E-(B1|C1|C2) not found: ' TweaksV.log) -eq 3 ] && rt_line 'PROBLEM: ${JOB}T-E-B1 was not found.'"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "with the key at 0 and DIAGNOSTIC_MODE=1: the measurement's line, no problem of the part's" "rt_line '${NOT_MEASURED}T-E-B1 was not found.' && log_lacks '\[RayTracing\] PROBLEM'"

echo "== R15 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the sites logged, the jumps not redirected, the frames summed up, the refusals not counted"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe rtcheck wait)
check "the sites logged; the redirect that would be made, word for word; the announcement" "patterns_found && sites_logged 4194304 && rt_line \"DRY_RUN: would redirect the refusal jumps at $(rva geometry 90) (T-E-C1) and $(rva ropes 87) (T-E-C2) to counting stubs - the refusals are not counted.\" && rt_line '$MEASURED'"
check "the jumps lead to the game's refusals, which the fake's checks reach with their registers as left; no refusal said" "has 'rt sites: as placed, count 4194304, checks 4194304 and 4194304, jumps to the game and the game |' && has '$CHECKED' && log_lacks 'the first geometry refused'"
check "the summary 10 s in, word for word, the refusals not counted; no problem" "rt_line \"${SUMMARY}made with 4194304 vertices of 20 bytes: the last frame took 2811203 of 4194304 vertices (54 of 80 MB), at most 2811203 so far; the refusals are not counted (DRY_RUN).\" && first_after_10s && log_lacks 'PROBLEM'"

echo "== R16 Windows' dynamic code policy, DYNAMIC_BVH_VERTICES=8388608 and DIAGNOSTIC_MODE=1: the raise reported, the refusals not counted, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DYNAMIC_BVH_VERTICES 8388608; out=$(game full_game.exe nodyncode)
check "the part's problem and the stubs' line, word for word" "rt_line 'PROBLEM: ${JOB}the game code could not be changed (details in TweaksV.log).' && log_has '\[RayTracing\] DIAGNOSTIC: the ray tracing buffer.s refusals are not counted: its stubs could not be made read+execute ('"
check "nothing written: the sites as placed, the jumps to the game" "has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R17 the buffer made already, DIAGNOSTIC_MODE=1: the sites logged, the frames sampled, the refusal jumps left as they are - the game's threads may run the checks"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe rtmade)
check "the buffer object's line, the stubs' line and the announcement, word for word" "rt_line \"DIAGNOSTIC: buffer object.s store at $(rva set-up 33) -> $(rva object), which holds 0xBAD0B1EC7000: the buffer is made already (T-E-B1)\" && rt_line 'DIAGNOSTIC: the ray tracing buffer.s refusals are not counted: the game has made the buffer already - its threads may run the checks while their jumps are rewritten.' && rt_line '$MEASURED'"
check "no jump rewritten or redirected, no problem; the sites as placed" "log_lacks 'refusal jump at RVA 0x[0-9A-F]*: 0F 87' && log_lacks 'redirected to counting stubs' && log_lacks 'PROBLEM' && has '$GAME_SITES' && has 'protect: all restored (RX)'"

echo "== R17b Windows refuses the sampling thread (the test build's TWEAKSV_TEST_REFUSE_THREAD), DIAGNOSTIC_MODE=1: logged, not reported, and the announcement does not say sampled"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 ray tracing buffer sampling' ASI=TweaksV_test.asi game full_game.exe)
check "the refusal's lines and the announcement's other wording, word for word; no problem of the part" "log_line '\[TweaksV\] The ray tracing buffer sampling thread could not be started (Windows error [0-9]*.*)\.\$' && rt_line 'DIAGNOSTIC: the ray tracing buffer.s frames are not sampled: their thread could not be started.' && rt_line 'DIAGNOSTIC: the ray tracing buffer of moving geometry is measured, its frames not sampled: the rest is logged after 10 seconds, then every minute while it changes.' && log_lacks '\[RayTracing\] $MEASURED' && log_lacks '\[RayTracing\] PROBLEM'"

echo "== R18 a pattern twice with the key at 0 and DIAGNOSTIC_MODE=1: the scanner's line and both matches, logged, not reported - no measurement, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe rtsetuptwice)
check "the measurement's line, word for word, the scanner's two matches; no problem, no redirect, no announcement" "rt_line \"${NOT_MEASURED}the game code holds T-E-B1 2 or more times, not the expected 1.\" && log_has 'Pattern T-E-B1 found: .*, 2 matches, bytes:' && log_has '  T-E-B1 match 2 of 2 at RVA 0x' && log_lacks 'PROBLEM' && log_lacks 'redirected to counting stubs' && log_lacks '\[RayTracing\] .*is measured' && has '$GAME_SITES'"

echo
echo "RAY TRACING SCENARIOS: $pass passed, $fail failed"
