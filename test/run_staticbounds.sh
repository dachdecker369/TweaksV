#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The StaticBounds part (TweaksVStaticBounds): the fake game's two functions its pools are made through, the set-up (C-E-P1)
# and the allocator (C-E-A1), run as code and hooked, all or none, so that the static bounds pool gets the larger of its size
# and STATICBOUNDS_SIZE when the fake makes it, said once, and the fake's eight other pools stay as they are: the size below and
# above the key, the key at its limits and beyond; ENABLED=0 and both old plugins; DRY_RUN; DIAGNOSTIC_MODE with both sites and
# every making; each check failing on its own - each pattern missing, each held twice, another mod's jump on each function, each
# hook not made with the other taken back, no code to be changed - each reported with nothing written; the pool made twice, its
# address taken by another pool, its stride changed before its allocation, a capacity below 0; what IsFunctionStart says, which
# refuses nothing; the lines' cap; the plain game; the check that the game makes the pool, its clock from the session's start, the
# first script program placed (the test build's TWEAKSV_TEST_STATICBOUNDS_CHECK): made in time, made before TweaksV loaded, never
# made, its name unreadable, set up and not allocated, made after the check, the hook made by the part with the Blips part off in
# TweaksV.ini and the check reporting, no hook and no check with BlipLimitFix.asi installed or with Resource Extensions installed and
# the Blips part off, no report without a session; a check that cannot be made, the problem word for word, once, the hooks on and the
# pool raised all the same, no check and no hook of the script programs made: Y-E-H1 missing with the Blips part on, the part's problem
# beside Blips' in the start-up summary's box, each of the hook's three patterns missing with the Blips part off, the part's problem
# alone in the box, the natives' resolution call another mod redirected, each of the two events not made (the test build's
# TWEAKSV_TEST_STATICBOUNDS_NO_EVENT), in the box, which the Release build, another value and DRY_RUN leave alone, and beside
# BlipLimitFix.asi only the line, no problem. In every run the sites as placed but for the hooks' jumps, and every page its own
# protection. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata scripts

RVA='RVA 0x[0-9A-F]*'
TAG='\[StaticBounds\] '
NOT_RAISED="${TAG}PROBLEM: The static bounds pool.s size is not raised this session: "
OTHERS='other pools 8 of 8 left alone, set-ups 9 of 9 and allocations 9 of 9 good'
HOOKED='static bounds: set-up hooked, allocator hooked, the rest as placed, '
PLACED='static bounds: set-up as placed, allocator as placed, the rest as placed, '
# the report's field: the pool made through TweaksV's hooks with 16000 slots - or as the game made it, nothing written
RAISED="${HOOKED}the pool made once: 16000 slots, 272000 bytes asked, $OTHERS"
UNTOUCHED="${PLACED}the pool made once: 12000 slots, 204000 bytes asked, $OTHERS"
# hooks_made N: the hooks' line, word for word, for STATICBOUNDS_SIZE=N; made N: the check's line, word for word, for its time N
# ("10 seconds"); sizes G WORDS: the sizes' line, word for word
hooks_made() { log_line "${TAG}The pools. set-up at $RVA and the pools. allocator at $RVA are hooked: the static bounds pool gets at least $1 slots (STATICBOUNDS_SIZE) when the game makes it, more where the game gives it more.\$"; }
made() { log_line "${TAG}The static bounds check waits for the session.s start - the first script program the game places - and then $1 for the game to make the pool.\$"; }
sizes() { log_line "${TAG}The size the game gives the static bounds pool, $1, $2\$"; }
said() { count 'The size the game gives the static bounds pool'; }
# quiet: no problem of the part; kept: the game left as it was - the pool made through its own functions, every page its own protection
quiet() { log_lacks '\[StaticBounds\] PROBLEM'; }
kept() { has "$UNTOUCHED" && has 'protect: all restored (RX)' && log_lacks 'are hooked: the static bounds pool'; }
# rva_of SED: what SED's s///p takes from TweaksV.log - the RVA or the time a line gives
rva_of() { tr -d '\r' < TweaksV.log | sed -n "$1"; }

echo "== SB1 the defaults: both functions hooked, the pool made with 16000 slots in the place of the game's 12000, said once, the eight other pools as the game made them"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the hooks' line, word for word, once" "hooks_made 16000 && [ \$(count 'are hooked: the static bounds pool') -eq 1 ]"
check "the pool raised before the game's allocator ran, the room for 16000 slots of 16 bytes and a byte each asked; the others left alone, every call's result, arguments and last error the game's" "has '$RAISED'"
check "the sizes said once, word for word" "sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the hooks and nothing else: the sites as placed but for the two jumps, every page its own protection, no problem, no DIAGNOSTIC line" "has 'protect: all restored (RX)' && quiet && log_lacks '\[StaticBounds\] DIAGNOSTIC'"

echo "== SB2 the configuration gives the pool 24500, more than STATICBOUNDS_SIZE: kept"
fresh_ini; out=$(game full_game.exe sbbig)
check "the pool keeps 24500, said once, word for word" "has '${HOOKED}the pool made once: 24500 slots, 416500 bytes asked, $OTHERS' && sizes 24500 'is kept: it is not below STATICBOUNDS_SIZE=16000.' && [ \$(said) -eq 1 ]"
check "the hooks made all the same, nothing else written, no problem" "hooks_made 16000 && has 'protect: all restored (RX)' && quiet"

echo "== SB3 STATICBOUNDS_SIZE=100000, the key's maximum: raised to it"
fresh_ini; setkey STATICBOUNDS_SIZE 100000; out=$(game full_game.exe)
check "100000 slots, each line word for word" "has 'the pool made once: 100000 slots, 1700000 bytes asked, $OTHERS' && hooks_made 100000 && sizes 12000 'is raised to 100000 (STATICBOUNDS_SIZE).' && quiet"

echo "== SB4 STATICBOUNDS_SIZE=1, below the game's size: the game's size kept"
fresh_ini; setkey STATICBOUNDS_SIZE 1; out=$(game full_game.exe)
check "12000 slots through the hooks, the line word for word" "has '${HOOKED}the pool made once: 12000 slots, 204000 bytes asked, $OTHERS' && sizes 12000 'is kept: it is not below STATICBOUNDS_SIZE=1.' && quiet"

echo "== SB5 STATICBOUNDS_SIZE=200000, beyond the key's range: 100000 used, reported"
fresh_ini; setkey STATICBOUNDS_SIZE 200000; out=$(game full_game.exe)
check "the setting's problem, word for word, and 100000 slots" "log_has 'PROBLEM: TweaksV.ini: \[STATICBOUNDS\] STATICBOUNDS_SIZE=200000 is outside 1..100000 - using 100000.' && has 'the pool made once: 100000 slots' && quiet"

echo "== SB6 [STATICBOUNDS] ENABLED=0, with DIAGNOSTIC_MODE=1: nothing looked for, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 STATICBOUNDS; out=$(game full_game.exe)
check "the header says off, no line of the part, its patterns not looked for" "log_has 'StaticBounds off' && log_lacks '\[StaticBounds\]' && log_lacks 'C-E-'"
check "the pools made through the game's functions: nothing written" "kept"

echo "== SB7 the old plugins, StaticBoundLimitAdjuster.asi in the game folder and StaticBoundsLimitAdjuster.asi in scripts\\: each switches the part off"
for old in StaticBoundLimitAdjuster.asi scripts/StaticBoundsLimitAdjuster.asi; do
	fresh_ini; touch "$old"; out=$(game full_game.exe); rm -f "$old"
	name=$(basename "$old")
	check "$name: reported word for word, the header says off" "log_line 'PROBLEM: Old plugin found: .*$name. TweaksV.s StaticBounds part is switched off for this session so nothing is patched twice. Delete the old file - TweaksV replaces it.\$' && log_has 'StaticBounds off'"
	check "$name: no line of the part, nothing written" "log_lacks '\[StaticBounds\]' && kept"
done

echo "== SB8 DRY_RUN=1: nothing hooked, the line says what would be"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the would line, word for word, and not the hooks'" "log_line '${TAG}DRY_RUN: would hook the pools. set-up at $RVA and the pools. allocator at $RVA so that the static bounds pool gets at least 16000 slots (STATICBOUNDS_SIZE) when the game makes it.\$' && log_lacks 'are hooked: the static bounds pool'"
check "the pools made through the game's functions, no sizes line, nothing written, no problem" "kept && [ \$(said) -eq 0 ] && quiet"

echo "== SB9 DIAGNOSTIC_MODE=1: both patterns found once, the sites at their RVAs, what IsFunctionStart says of each, the making's lines"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
setup=$(rva_of "s/.*Pattern C-E-P1 found: .* -> RVA \(0x[0-9A-F]*\), 1 match,.*/\1/p")
alloc=$(rva_of "s/.*Pattern C-E-A1 found: .* -> RVA \(0x[0-9A-F]*\), 1 match,.*/\1/p")
check "C-E-P1 and C-E-A1 found exactly once each" "[ -n '$setup' ] && [ -n '$alloc' ] && [ \$(grep -c -E 'Pattern C-E-(P1|A1) found: .* 1 match,' TweaksV.log) -eq 2 ]"
check "each site at the RVA of its match, with what IsFunctionStart says, word for word" "log_line '${TAG}DIAGNOSTIC: the pools. set-up at RVA ${setup:-none} (C-E-P1): no unwind data, byte before it 90\$' && log_line '${TAG}DIAGNOSTIC: the pools. allocator at RVA ${alloc:-none} (C-E-A1): no unwind data, byte before it 90\$'"
check "the making's two lines, once each, word for word, with their times" "log_line '${TAG}DIAGNOSTIC: the static bounds pool at $RVA is set up, [0-9]* ms after TweaksV.s start.\$' && log_line '${TAG}DIAGNOSTIC: the static bounds pool at $RVA is allocated with 16000 slots, the game.s 12000, [0-9]* ms after TweaksV.s start.\$' && [ \$(count 'DIAGNOSTIC: the static bounds pool at') -eq 2 ]"
check "the hooks and the sizes as without DIAGNOSTIC_MODE, no problem" "hooks_made 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && quiet"

echo "== SB10 TEST_BREAK_PATTERN for each pattern (test build): reported by its label, nothing hooked - the other looked for all the same"
for label in C-E-P1 C-E-A1; do
	fresh_ini; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the scanner's test line once and the problem, word for word, once" "[ \$(count 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && log_line '${NOT_RAISED}$label was not found.\$' && [ \$(count 'is not raised this session') -eq 1 ]"
	check "$label: nothing written - the pools made through the game's functions" "kept"
done

echo "== SB11 each pattern twice in the game's code: named as twice, not as missing, nothing hooked"
for pair in "sbp1twice C-E-P1" "sba1twice C-E-A1"; do
	option=${pair% *}; label=${pair#* }
	fresh_ini; out=$(game full_game.exe $option)
	check "$label: the problem, word for word, once" "log_line '${NOT_RAISED}the game code holds $label 2 or more times, not the expected 1.\$' && [ \$(count 'is not raised this session') -eq 1 ]"
	check "$label: the scanner's line; nothing hooked, the pool made as the game made it" "log_has 'Pattern $label matches 2 or more times - not used (the site must be unique).' && has 'set-up as placed, allocator as placed' && has 'the pool made once: 12000 slots, 204000 bytes asked, $OTHERS' && has 'protect: all restored (RX)'"
done

echo "== SB12 another mod's jump on each function: its pattern not found, nothing hooked, the pools made through the other mod's stub"
for pair in "sbp1jump C-E-P1" "sba1jump C-E-A1"; do
	option=${pair% *}; label=${pair#* }
	fresh_ini; out=$(game full_game.exe $option)
	check "$label: the problem, word for word; the other mod's jump left where it led" "log_line '${NOT_RAISED}$label was not found.\$' && has 'set-up as placed, allocator as placed, the rest as placed'"
	check "$label: the pool made as the game made it, every call good, every page its own protection" "has 'the pool made once: 12000 slots, 204000 bytes asked, $OTHERS' && has 'protect: all restored (RX)' && log_lacks 'are hooked: the static bounds pool'"
done

echo "== SB13 each function's page not executable while TweaksV starts: MinHook refuses that hook, the other taken back"
fresh_ini; out=$(game full_game.exe sbp1noexec)
check "the set-up, the first of the two: its line and the problem, word for word; nothing written" "log_line '${TAG}the pools. set-up could not be hooked (MinHook: MH_ERROR_NOT_EXECUTABLE) - none of the 2 hooks is on.\$' && log_line '${NOT_RAISED}the pools. set-up at $RVA and the pools. allocator at $RVA could not be hooked (details in TweaksV.log).\$' && kept"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sba1noexec)
check "the allocator, the set-up's hook made already: taken back; its line and the problem, word for word" "log_line '${TAG}the pools. allocator could not be hooked (MinHook: MH_ERROR_NOT_EXECUTABLE) - none of the 2 hooks is on.\$' && log_line '${NOT_RAISED}the pools. set-up at $RVA and the pools. allocator at $RVA could not be hooked (details in TweaksV.log).\$'"
check "the set-up as placed and no making noted, the pool made through the game's functions" "kept && log_lacks 'DIAGNOSTIC: the static bounds pool at'"

echo "== SB14 Windows' dynamic code policy: no hook can be made - reported, nothing written"
fresh_ini; out=$(game full_game.exe nodyncode)
check "the hook's line and the problem, word for word; the pools made through the game's functions" "log_line '${TAG}the pools. set-up could not be hooked (MinHook: MH_ERROR_MEMORY_ALLOC) - none of the 2 hooks is on.\$' && log_line '${NOT_RAISED}the pools. set-up at $RVA and the pools. allocator at $RVA could not be hooked (details in TweaksV.log).\$' && kept"

echo "== SB15 the pool made a second time, at another address, with DIAGNOSTIC_MODE=1: raised as well, the sizes said once, each making in its lines"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sbtwice)
check "16000 slots both times, the others left alone, every call good" "has '${HOOKED}the pool made 2 times: 16000 and 16000 slots, 272000 and 272000 bytes asked, other pools 8 of 8 left alone, set-ups 10 of 10 and allocations 10 of 10 good'"
check "the sizes said once; both makings' lines; no problem" "[ \$(said) -eq 1 ] && [ \$(count 'DIAGNOSTIC: the static bounds pool at .* is set up') -eq 2 ] && [ \$(count 'is allocated with 16000 slots, the game.s 12000') -eq 2 ] && quiet"

echo "== SB16 another pool of stride 16 set up and allocated at the pool's address: the note ends with its set-up, left alone"
fresh_ini; out=$(game full_game.exe sbreuse)
check "the pool raised, the ninth other pool left alone with its 600 slots, every call good" "has 'the pool made once: 16000 slots, 272000 bytes asked, other pools 9 of 9 left alone, set-ups 10 of 10 and allocations 10 of 10 good' && [ \$(said) -eq 1 ] && quiet"

echo "== SB17 the game gives the pool -1: left as the game gave it, reported once"
fresh_ini; out=$(game full_game.exe sbnegative)
check "the problem, word for word, once; -1 through the hooks; no sizes line" "log_line '${NOT_RAISED}the game gives the static bounds pool the size -1, which is none - left as the game gave it.\$' && [ \$(count 'which is none') -eq 1 ] && has '${HOOKED}the pool made once: -1 slots' && [ \$(said) -eq 0 ]"

echo "== SB18 the pool's stride 24 by the time the game allocates it: left as the game made it, reported once"
fresh_ini; out=$(game full_game.exe sbstride)
check "the problem, word for word, once; the pool's 12000 slots of 24 bytes through the hooks; no sizes line" "log_line '${NOT_RAISED}the pool at $RVA holds the stride 24 when the game allocates it, not the 16 it was set up with - it is left as the game made it.\$' && [ \$(count 'holds the stride') -eq 1 ] && has '${HOOKED}the pool made once: 12000 slots, 300000 bytes asked, $OTHERS' && [ \$(said) -eq 0 ]"

echo "== SB19 the byte before each function 48 (DIAGNOSTIC_MODE=1): what IsFunctionStart says is a line, and refuses nothing"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sbnotstart)
check "both lines say it, word for word; the hooks made, the pool raised, no problem" "log_line '${TAG}DIAGNOSTIC: the pools. set-up at $RVA (C-E-P1): no unwind data, byte before it 48\$' && log_line '${TAG}DIAGNOSTIC: the pools. allocator at $RVA (C-E-A1): no unwind data, byte before it 48\$' && hooks_made 16000 && has '$RAISED' && quiet"

echo "== SB20 the pool made 10,001 times (DIAGNOSTIC_MODE=1): 20,000 lines of the makings, then only counted"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sbflood)
check "20000 lines, then the cap's line once; the sizes said once" "[ \$(count 'DIAGNOSTIC: the static bounds pool at') -eq 20000 ] && [ \$(count 'DIAGNOSTIC: 20000 lines of the pool.s makings written - from now on they are only counted.') -eq 1 ] && [ \$(said) -eq 1 ]"
check "every making raised, every call good" "has 'the pool made 10001 times: 16000 and 16000 slots, 272000 and 272000 bytes asked, other pools 8 of 8 left alone, set-ups 10009 of 10009 and allocations 10009 of 10009 good' && quiet"

echo "== SB21 none of the game code present (plain fake game): both patterns looked for, the first one missing reported"
fresh_ini; out=$(game plain_game.exe)
check "the harness survived" "grep -q 'harness: .* loaded' harness.out"
check "both looked for and missing, the problem names the first" "[ \$(grep -c -E 'Pattern C-E-(P1|A1) not found: ' TweaksV.log) -eq 2 ] && log_line '${NOT_RAISED}C-E-P1 was not found.\$'"

NOT_MADE="${NOT_RAISED}the game did not make the pool within 2 seconds of the session.s start - it made the pool before TweaksV started, or makes it another way, and the pool keeps the game.s size.\$"
AFTER_ALL='The game made the static bounds pool after all, after the check had reported it missing.'

echo "== SB22 the check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=10, DIAGNOSTIC_MODE=1, a script program placed, past the check's time): the game makes the pool in time - timed, nothing reported"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=10
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe script wait)
allocated=$(rva_of "s/.*${TAG}DIAGNOSTIC: the static bounds pool at $RVA is allocated with 16000 slots, the game.s 12000, \([0-9]*\) ms after TweaksV.s start.\$/\1/p")
session=$(rva_of "s/.*${TAG}DIAGNOSTIC: the session started \([0-9]*\) ms after TweaksV.s start (the first script program placed): the static bounds check.s clock runs.\$/\1/p")
check "the test build's line and the check's line, once each; the hooks' line" "[ \$(count 'TEST BUILD: the static bounds check comes 10 seconds after the session.s start instead of 60 seconds (TWEAKSV_TEST_STATICBOUNDS_CHECK=10).') -eq 1 ] && made '10 seconds' && [ \$(count 'The static bounds check waits') -eq 1 ] && hooks_made 16000"
check "the session's start and the making timed, once each, the making within the check's 10 seconds of the start; the sizes said once" "[ '${session:-none}' != none ] && [ '${allocated:-none}' != none ] && [ \$(( ${allocated:-99999} - ${session:-0} )) -lt 10000 ] && [ \$(said) -eq 1 ]"
check "past the check's time: nothing reported, the pool raised" "log_lacks 'did not make the pool' && quiet && has '$RAISED'"

echo "== SB23 the pool made before TweaksV loaded, never again (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, ERROR_POPUP=1): reported, with the part's box"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait)
check "the problem, word for word, once" "log_line '$NOT_MADE' && [ \$(count 'did not make the pool') -eq 1 ]"
check "shown after the start-up summary, in a box of its own" "log_has 'Showing a problem after the start-up summary (1 problem(s)).' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"
check "the pool made through the game's functions before TweaksV, the hooks made all the same, no sizes line" "has 'the pool made once: 12000 slots, 204000 bytes asked, $OTHERS' && has 'set-up hooked, allocator hooked' && [ \$(said) -eq 0 ] && log_lacks 'after all, after the check'"

echo "== SB24 the pool never made, the others made (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2): reported"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnever script sbwait)
check "the problem, word for word, once; the pool not made, the others left alone" "log_line '$NOT_MADE' && [ \$(count 'did not make the pool') -eq 1 ] && has '${HOOKED}the pool not made, other pools 8 of 8 left alone, set-ups 8 of 8 and allocations 8 of 8 good'"

echo "== SB25 the pool's name on a page of no access (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2): it cannot be told, left alone - and reported"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnoname script sbwait)
check "the problem, word for word, once; the pool at the game's size through the hooks" "log_line '$NOT_MADE' && [ \$(count 'did not make the pool') -eq 1 ] && has '${HOOKED}the pool made once: 12000 slots, 204000 bytes asked, $OTHERS'"

echo "== SB26 the pool set up, never allocated (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2): reported as what it is"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnoalloc script sbwait)
check "the problem, word for word, once; no sizes line" "log_line '${NOT_RAISED}the game set the pool up but did not allocate it through the pools. allocator within 2 seconds of the session.s start, and the pool keeps the game.s size.\$' && [ \$(count 'did not allocate it') -eq 1 ] && has 'the pool not made' && [ \$(said) -eq 0 ]"

echo "== SB27 the pool made 4 seconds late, after the check of 2 seconds (test build): reported, then made after all"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sblate script)
check "the problem first, then the line that the game made it after all, then the sizes, each once" "[ \$(count 'did not make the pool') -eq 1 ] && [ \$(count '$AFTER_ALL') -eq 1 ] && [ \$(grep -n 'did not make the pool' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'after all, after the check' TweaksV.log | cut -d: -f1) ] && sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the pool raised all the same" "has '$RAISED'"

echo "== SB28 [BLIPS] ENABLED=0 (test build, the pool made before TweaksV, a script program placed): the part makes the script programs' hook itself, and the check reports"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait)
check "the hook made by the part, its line under the part's tag, word for word; the program placed through TweaksV's stub; nothing of the Blips part" "log_line '${TAG}The natives. resolution call at RVA 0x[0-9A-F]* is redirected: every script program the game places passes TweaksV, before and after the game resolves its natives.\$' && has 'script: 1 placed on thread' && has 'through a TweaksV stub' && log_lacks '\[Blips\]'"
check "the check made, and its problem past its time, word for word, once" "made '2 seconds' && log_line '$NOT_MADE' && [ \$(count 'did not make the pool') -eq 1 ]"

echo "== SB29 no script program placed (test build): the session never starts, so the check never reports"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly sbwait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the check made and waiting, nothing reported past its time" "made '2 seconds' && log_lacks 'did not make the pool' && quiet"

echo "== SB30 nothing hooked, no check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): DRY_RUN, a refused job, a hook not made"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
for case in dryrun sbp1jump sba1noexec; do
	fresh_ini; options="sbearly script sbwait"
	if [ "$case" = dryrun ]; then setkey DRY_RUN 1; else options="$options $case"; fi
	out=$(ASI=TweaksV_test.asi game full_game.exe $options)
	check "$case: no check - neither its line nor its test build's line nor its problem past its time" "log_lacks 'static bounds check' && log_lacks 'did not make the pool'"
done

echo "== SB31 a capacity below 0, a script program placed (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2): its problem once, and the check not again"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnegative script sbwait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the capacity reported once, the check made and silent past its time" "[ \$(count 'which is none') -eq 1 ] && made '2 seconds' && log_lacks 'did not make the pool'"

# the line of a check left to a plugin that does the Blips part's job, word for word
LEFT="${TAG}The static bounds check is not made this session: it starts with the session, which TweaksV sees through the script programs. hook, and a plugin that does the Blips part.s job on the script programs is installed (named above) - TweaksV leaves the script programs to it.\$"

echo "== SB32 BlipLimitFix.asi in the game folder, the Blips part on in TweaksV.ini (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): no check, the line says why"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; touch BlipLimitFix.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait); rm -f BlipLimitFix.asi
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the line, word for word; no check, nothing reported past its time; the program placed through the game's own function" "log_line '$LEFT' && log_lacks 'static bounds check comes' && log_lacks 'did not make the pool' && has 'through the game function'"
check "the hooks made all the same, no problem of the part" "hooks_made 16000 && quiet"

echo "== SB33 [BLIPS] ENABLED=0 with ResourceExtensions.asi in the game folder (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): the plugin does the Blips part's job whatever TweaksV.ini says - no hook of the script programs, no check, the line"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ENABLED 0 BLIPS; touch ResourceExtensions.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait); rm -f ResourceExtensions.asi
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the conflict check's line and the part's, word for word; no hook of the script programs, no check, nothing reported past its time; the program placed through the game's own function; the hooks made all the same, no problem of the part" "log_line '\[TweaksV\] Conflict check: .*ResourceExtensions.asi is installed; TweaksV.s Blips part is off in TweaksV.ini already.\$' && log_line '$LEFT' && log_lacks 'natives. resolution call' && log_lacks 'static bounds check comes' && log_lacks 'did not make the pool' && has 'through the game function' && hooks_made 16000 && quiet"

# a check that cannot be made: its problem, word for word but for the reason, once; the start-up summary's box with N problems;
# no check made; no hook of the script programs made by the part
NOT_CHECKED="${TAG}PROBLEM: The check that the game makes the static bounds pool is not done this session: "
not_checked() { log_line "${NOT_CHECKED}$1\$" && [ $(count 'pool is not done this session: ') -eq 1 ]; }
boxed() { log_has "Showing the start-up summary ($1 problem(s))." && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:'; }
unchecked() { log_lacks 'The static bounds check waits' && log_lacks 'did not make the pool'; }
unhooked() { log_lacks 'natives. resolution call at RVA 0x[0-9A-F]* is redirected' && has 'through the game function'; }

echo "== SB34 TEST_BREAK_PATTERN=Y-E-H1 with the Blips part on (test build, ERROR_POPUP=1, a script program placed): the part joins the hook Blips could not make - its problem beside Blips', in the start-up summary's box, the pool raised all the same"
fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN Y-E-H1; out=$(ASI=TweaksV_test.asi game full_game.exe script)
check "the check's problem, word for word, once, beside the Blips part's, with the same reason; the pattern looked for once" "not_checked 'Y-E-H1 was not found.' && log_line '\[Blips\] PROBLEM: blip_controller.s threshold is not raised this session: Y-E-H1 was not found.\$' && [ \$(count 'TEST BUILD: pattern Y-E-H1 is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ]"
check "both in the start-up summary's box" "boxed 2"
check "the hooks made, the pool raised; no check made, the program placed through the game's own function" "hooks_made 16000 && has '$RAISED' && unchecked && unhooked"

echo "== SB35 each of the hook's patterns missing, [BLIPS] ENABLED=0 (test build, ERROR_POPUP=1, a script program placed): the part, the hook's first user, cannot make it - its problem alone, the pool raised all the same"
for label in Y-E-H1 Y-E-N1 Y-E-I1; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe script)
	check "$label: the scanner's test line once, the problem word for word, once, alone in the start-up summary's box; nothing of the Blips part" "[ \$(count 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && not_checked '$label was not found.' && boxed 1 && log_lacks '\[Blips\]'"
	check "$label: the hooks made, the pool raised; no check made, no hook of the script programs, the program placed through the game's own function" "hooks_made 16000 && has '$RAISED' && unchecked && unhooked"
done

echo "== SB36 the natives' resolution call another mod redirected, [BLIPS] ENABLED=0 (the Release build): the hook's own reason, the pool raised all the same"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(game full_game.exe scripthooked)
check "the problem, word for word, once; nothing of the Blips part" "not_checked 'the natives. resolution call at $RVA is not a call into the game (another mod may have hooked it already).' && log_lacks '\[Blips\]'"
check "the hooks made, the pool raised; no check made, the other mod's call left where it led" "hooks_made 16000 && has '$RAISED' && unchecked && has 'script: none placed, call to another mod'"

echo "== SB37 each of the check's events not made (test build, TWEAKSV_TEST_STATICBOUNDS_NO_EVENT, ERROR_POPUP=1, [BLIPS] ENABLED=0, a script program placed): the problem, the pool raised all the same, no hook of the script programs"
for event in made session; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event ASI=TweaksV_test.asi game full_game.exe script)
	check "$event: the test build's line, word for word, once" "log_line '${TAG}TEST BUILD: the static bounds check.s $event event is not made (TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event).\$' && [ \$(count 'event is not made') -eq 1 ]"
	check "$event: the problem, word for word, once, in the start-up summary's box" "not_checked 'its events could not be made.' && boxed 1"
	check "$event: the hooks made, the pool raised, said once; no check made, no hook of the script programs, the program placed through the game's own function" "hooks_made 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && unchecked && unhooked"
done

echo "== SB38 the switch's limits: the Release build, another value, DRY_RUN - no event left out, nothing reported"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=made game full_game.exe)
check "the Release build ignores it: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=other ASI=TweaksV_test.asi game full_game.exe)
check "another value: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; setkey DRY_RUN 1; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=made ASI=TweaksV_test.asi game full_game.exe)
check "DRY_RUN: no event wanted, nothing reported" "quiet && log_lacks 'event is not made' && kept"

echo "== SB39 BlipLimitFix.asi with the made event not made (test build): no check was to be made, logged - not reported"
fresh_ini; touch BlipLimitFix.asi; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=made ASI=TweaksV_test.asi game full_game.exe); rm -f BlipLimitFix.asi
check "the line, word for word, no problem of the part; the pool raised all the same" "log_line '$LEFT' && quiet && hooks_made 16000 && has '$RAISED'"

echo
echo "STATIC BOUNDS SCENARIOS: $pass passed, $fail failed"
