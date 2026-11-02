#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The texture budget, the Resources part (TweaksVTextureBudget): VIDEO_MEMORY_BUDGET at 0, said in one line, nothing looked for;
# at 6000 the four budgets raised as the game's start-up sets them, through the call after its stores redirected to a stub,
# read+execute, and nothing else written - the call's four arguments and its result as they came, the budgets raised before
# the game's function runs; none lowered, a gameconfig.xml's larger budgets kept; none below 1500 left as they are; 1000
# raised to 1500 and reported; an index outside 0 to 3 named; the part off, DRY_RUN, each check failing on its own - those of
# the sites also with DIAGNOSTIC_MODE=1, the measurement's line beside the problem, an address not aligned to the size of its
# reads and a pattern the game holds twice among them - a writer besides the start-up's, also with
# DIAGNOSTIC_MODE=1, named and measured all the same, the game's start-up before TweaksV started, its data or its index alone
# unreadable while TweaksV starts, the plain game and Windows' dynamic code policy - each reported word for word, nothing
# written; and
# the measurement with DIAGNOSTIC_MODE=1, also beside a raise, one held half done past the time of the first summary:
# the five patterns found in the full fake game, every site checked and logged with the addresses the fake reports, and the
# counters summed up 10 seconds in, at the game's own sizes - the budgets 1500, 1500, 3000 and 3000 MB, 98304 objects - and
# again once they grow past the budget (the test build's TWEAKSV_TEST_SUMMARY_MS), not while nothing changes, and a first
# summary before the game's start-up has set the budgets, which it sets after it, the update check's line again, and one for
# each new high alone, the update check's flag unchanged; the budgets' writers found by the scanner's search for stores - the
# start-up's two alone, the stores beside the budgets and the loads and compares of them not counted, and six writers of
# six kinds besides them, each named, and in a block of its own (TB50) a writer behind each byte that once hid it from the
# search - a 0F, a C5, a C4, an F3, a 0F 38 and an F2 the instruction before ends with - counted, named and refusing the
# raise, and a load whose bytes read as a store counted too, a possible writer; a pattern
# missing (TEST_BREAK_PATTERN), a site not as expected, a call another mod hooked, an address outside the game, sites that
# disagree on the counters and a factor's multiplication changed - each logged, no problem, no box, no summary; the plain
# game; DRY_RUN alike; and in every run nothing written: the fake's sites and data as they were, no call redirected, every
# page's protection its own. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# The summaries: the counters as the fake sets them, then grown by 200 MB and 800 textures past the 3000 MB budget, the
# update check's flag up; one new high at a time, the flag down - the bytes by 100 MB, the game's mark at 2950 MB, the objects by
# 400; the update check's switch and factors; and both before the game's start-up has set its data
SUMMARY1='\[Resources\] DIAGNOSTIC: texture budget: 2845 MB of the 3000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) - textures 2456 MB, buffers 389 MB; 23456 of the 98304 objects allowed (MaxStreamingResources) - 22222 textures, 1234 buffers; the most so far 2900 MB by the game.s own mark, 2845 MB and 23456 objects at TweaksV.s readings; the update check.s flag is down.$'
SUMMARY2='\[Resources\] DIAGNOSTIC: texture budget: 3045 MB of the 3000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) - textures 2656 MB, buffers 389 MB; 24256 of the 98304 objects allowed (MaxStreamingResources) - 23022 textures, 1234 buffers; the most so far 3045 MB by the game.s own mark, 3045 MB and 24256 objects at TweaksV.s readings; the update check.s flag is up: it evicts ahead of time - at the budget: the streaming engine evicts what it cannot place (texture loss).$'
HIGH_BYTES='\[Resources\] DIAGNOSTIC: texture budget: 2945 MB of the 3000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) - textures 2556 MB, buffers 389 MB; 23456 of the 98304 objects allowed (MaxStreamingResources) - 22222 textures, 1234 buffers; the most so far 2900 MB by the game.s own mark, 2945 MB and 23456 objects at TweaksV.s readings; the update check.s flag is down.$'
HIGH_MARK='\[Resources\] DIAGNOSTIC: texture budget: 2945 MB of the 3000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) - textures 2556 MB, buffers 389 MB; 23456 of the 98304 objects allowed (MaxStreamingResources) - 22222 textures, 1234 buffers; the most so far 2950 MB by the game.s own mark, 2945 MB and 23456 objects at TweaksV.s readings; the update check.s flag is down.$'
HIGH_OBJECTS='\[Resources\] DIAGNOSTIC: texture budget: 2945 MB of the 3000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) - textures 2556 MB, buffers 389 MB; 23856 of the 98304 objects allowed (MaxStreamingResources) - 22622 textures, 1234 buffers; the most so far 2950 MB by the game.s own mark, 2945 MB and 23856 objects at TweaksV.s readings; the update check.s flag is down.$'
UPDATE_CHECK='\[Resources\] DIAGNOSTIC: texture budget: the update check.s switch holds 1 (on), its factors 0.9 of the budget to start evicting, 0.8 to stop, and 0.95 of the video memory Windows gives the game to start evicting too.$'
UNSET='\[Resources\] DIAGNOSTIC: texture budget: 0 MB of the 0 MB budget for the texture quality.s index 0 (MaxStreamingMemory_0..3_Base: 0, 0, 0 and 0 MB) - textures 0 MB, buffers 0 MB; 0 of the 0 objects allowed (MaxStreamingResources) - 0 textures, 0 buffers; the most so far 0 MB by the game.s own mark, 0 MB and 0 objects at TweaksV.s readings; the update check.s flag is down - the budgets and the most are not set yet (the game sets them when it reads gameconfig.xml).$'
UPDATE_CHECK_UNSET='\[Resources\] DIAGNOSTIC: texture budget: the update check.s switch holds 0 (off: nothing is evicted ahead of time), its factors 0 of the budget to start evicting, 0 to stop, and 0 of the video memory Windows gives the game to start evicting too.$'
NOT_MEASURED='\[Resources\] DIAGNOSTIC: the texture budget is not measured: '
NOT_RAISED='\[Resources\] PROBLEM: The texture budget is not raised this session: '
RAISE='\[Resources\] The texture budgets of gameconfig.xml (MaxStreamingMemory_0..3_Base: '
SUMMARY_RAISED='\[Resources\] DIAGNOSTIC: texture budget: 2845 MB of the 6000 MB budget for the texture quality.s index 2 (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB, raised to 6000, 6000, 6000 and 6000 MB by VIDEO_MEMORY_BUDGET) - textures 2456 MB, buffers 389 MB; 23456 of the 98304 objects allowed (MaxStreamingResources) - 22222 textures, 1234 buffers; the most so far 2900 MB by the game.s own mark, 2845 MB and 23456 objects at TweaksV.s readings; the update check.s flag is down.$'

# rva NAME [PLUS]: "RVA 0x..." of what the fake's report places at NAME (stores, getter, placement, update, next step,
# counters, index, flag, switch, index page), plus PLUS
rva() {
	local at
	at=$(sed -nE "s/.*[ ,;]$1 RVA (0x[0-9A-F]+)[ ,].*/\1/p" <<< "$out" | head -n 1)
	[ -n "$at" ] || { echo "RVA-of-$1-not-reported"; return; }
	printf 'RVA 0x%X' $(( at + ${2:-0} ))
}
# report_is CALL BUDGETS: the fake's report - its five sites as placed but the call's rel32, the call to CALL, its data as set but
# the budgets, its page read/write, the budgets BUDGETS, every page's protection its own
report_is() { has "texture budget: sites as placed 5 of 5, the call to $1, data as set, its page RW, budgets $2 MB" && has 'protect: all restored (RX)'; }
# untouched [CALL]: nothing written - the call to CALL (the next step), the budgets the game's own
untouched() { report_is "${1:-the next step}" '1500, 1500, 3000 and 3000'; }
# startup CALL BUDGETS: the start-up's call, through CALL, made once with its four arguments, its result back, the budgets
# BUDGETS while the game's function ran
startup() { has "start-up: through $1, the next step called 1 time, its arguments as handed, its result back, the budgets then $2 MB"; }
# stubs: the call stubs the fake's report counts
stubs() { sed -nE 's/.*stubs: ([0-9]+), all RX.*/\1/p' <<< "$out"; }
# redirected BUDGET: the part's line for the redirected call; call_written: its DIAGNOSTIC line of the write
redirected() { log_line "\[Resources\] The call after the budgets. stores at $(rva stores 74) is redirected: the texture budgets below $1 MB are raised to it when the game.s start-up sets them.\$"; }
call_written() { log_line "\[Resources\] DIAGNOSTIC: call after the budgets. stores at $(rva stores 74): E8 [0-9A-F ]* -> E8 [0-9A-F ]*\$"; }
dry_run_lines() { log_line "\[Resources\] DRY_RUN: would redirect the call after the budgets. stores at $(rva stores 74).\$" && log_line '\[Resources\] DRY_RUN: the texture budgets below 6000 MB would be raised to it when the game.s start-up sets them.$'; }
# one_problem: the part's one problem, an item of the start-up summary - logged before the summary's line. The other parts run
# beside it in the merged runs, so neither every problem of a run nor the summary's count of them is counted here
one_problem() {
	local problem summary
	problem=$(grep -n -m1 '\[Resources\] PROBLEM' TweaksV.log | cut -d: -f1)
	summary=$(grep -n -m1 'Start-up summary not shown (ERROR_POPUP=0): [1-9][0-9]* problem' TweaksV.log | cut -d: -f1)
	[ "$(count '\[Resources\] PROBLEM')" -eq 1 ] && [ -n "$problem" ] && [ -n "$summary" ] && [ "$problem" -lt "$summary" ]
}
# refused WHY [CALL]: the part's problem WHY, word for word, its only one, in the start-up summary, and nothing written
refused() { log_line "${NOT_RAISED}$1\$" && one_problem && untouched "${2:-the next step}"; }
# both_keys: TweaksV.ini with VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1; refused_both WHY: refused, and the measurement's
# line with the same reason, nothing measured
both_keys() { fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DIAGNOSTIC_MODE 1; }
refused_both() { refused "$1" && log_line "${NOT_MEASURED}$1\$" && nothing_measured; }
# found LABEL NAME [PLUS]: the scanner's line for LABEL, found once where the fake has it
found() { log_has "Pattern $1 found: .* -> $(rva "$2" "${3:-0}"), 1 match"; }
patterns_found() { found Q-E-T1 stores && found Q-E-T2 stores 34 && found Q-E-T3 getter && found Q-E-T4 placement && found Q-E-T5 update; }
# writers_line [BESIDES]: the module's line of the budgets' writers - the start-up's two stores, and BESIDES
writers_line() { log_line "\[Resources\] DIAGNOSTIC: writers of the budgets at $(rva counters 0x28): the start-up.s stores at $(rva stores 26) and $(rva stores 48), ${1:-none besides them} (Q-E-T1, Q-E-T2)\$"; }
# sites_logged: the module's nine lines, word for word, with the addresses of the fake's sites and data
sites_logged() {
	log_line "\[Resources\] DIAGNOSTIC: store of the most objects at $(rva stores 6) -> $(rva counters 0x20), from the configuration.s +0x54C (Q-E-T1)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: store of budgets 0 and 1 at $(rva stores 26) -> $(rva counters 0x28), from the configuration.s +0x550 in MB (Q-E-T1)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: store of budgets 2 and 3 at $(rva stores 48) -> $(rva counters 0x38), from the configuration.s +0x558 in MB (Q-E-T2)\$" &&
	writers_line &&
	log_line "\[Resources\] DIAGNOSTIC: call after the budgets. stores at $(rva stores 74) -> $(rva 'next step') (Q-E-T2)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: budget getter at $(rva getter): the index at $(rva index), the budgets at +0x28 of the counters (Q-E-T3)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: placement check at $(rva placement): the numbers at $(rva counters 0x10) and $(rva counters 0x14), the most at $(rva counters 0x20), the bytes at $(rva counters) and $(rva counters 8) (Q-E-T4)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: update check at $(rva update): its switch at $(rva switch), the counters at $(rva counters), the getter.s call at $(rva update 0x22) -> $(rva getter) (Q-E-T5)\$" &&
	log_line "\[Resources\] DIAGNOSTIC: update check.s flag at $(rva update 0x4B) -> $(rva flag), its factors at $(rva update 0x54) -> $(rva switch 8) and $(rva update 0xA0) -> $(rva switch 4) (Q-E-T5)\$"
}
# still_unset: the raise's line with DIAGNOSTIC_MODE that the most and the budgets are still 0 at TweaksV's start
still_unset() { log_line "\[Resources\] DIAGNOSTIC: the most objects at $(rva counters 0x20) and the budgets at $(rva counters 0x28) are still 0 at TweaksV.s start: the game.s start-up sets them after TweaksV.s DllMain, so the call after its stores is still to come\$"; }
announced() { log_line "\[Resources\] DIAGNOSTIC: the texture budget.s counters at $(rva counters) (Q-E-T4): the streamed textures and buffers are logged after 10 seconds, then every minute while their highs, the update check or the budget in use change.\$"; }
# nothing_measured: no announcement, no summary, no line of the update check
nothing_measured() { log_lacks 'texture budget.s counters at' && log_lacks 'DIAGNOSTIC: texture budget:'; }
# at_ms PATTERN: the time of the first log line that matches, in ms of the day
at_ms() {
	tr -d '\r' < TweaksV.log | grep -m1 -- "$1" | sed -nE 's/^([0-9]{2}):([0-9]{2}):([0-9]{2})\.([0-9]{3}) .*/\1 \2 \3 \4/p' |
		{ read -r h m s f && echo $(( (10#$h * 3600 + 10#$m * 60 + 10#$s) * 1000 + 10#$f )); }
}
# first_after_10s: the first summary 10 seconds after the announcement - 9.9 at least, 20 at most on a busy machine
first_after_10s() {
	local a f d
	a=$(at_ms 'texture budget.s counters at'); f=$(at_ms 'objects allowed (MaxStreamingResources)')
	[ -n "$a" ] && [ -n "$f" ] || return 1
	d=$(( f - a )); (( d < 0 )) && d=$(( d + 86400000 ))
	(( d >= 9900 && d <= 20000 ))
}
summaries() { count 'objects allowed (MaxStreamingResources)'; }

echo "== TB1 defaults, VIDEO_MEMORY_BUDGET=0 and DIAGNOSTIC_MODE=0: nothing looked for, nothing written"
fresh_ini; out=$(game full_game.exe); echo "      $out"; stubs0=$(stubs)
check "no texture budget pattern looked for; the part's one line, word for word" "log_lacks 'Q-E-T' && log_line '\[Resources\] VIDEO_MEMORY_BUDGET=0 - the texture budget is not raised.\$' && [ \$(count '\[Resources\]') -eq 1 ]"
check "the start-up's call made to the game's function itself, its arguments and its result as they came, the budgets the game's own" "startup 'the next step' '1500, 1500, 3000 and 3000' && [ -n '$stubs0' ]"
check "the fake's sites as placed, its data as set, every page's protection its own" "untouched"

echo "== TB2 DIAGNOSTIC_MODE=1: the five patterns found once each, every site logged, the counters summed up 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "Q-E-T1 to Q-E-T5 found once each, where the fake has them - Q-E-T2 34 bytes after Q-E-T1" "patterns_found"
check "the nine lines of the sites, word for word, with the addresses of the fake's sites and data" "sites_logged"
check "the budgets' writers: the start-up's two stores alone - the stores beside the budgets, the loads and the compares of them not counted" "writers_line"
check "the announcement, word for word, with the counters' address" "announced"
check "the update check's switch and factors, logged once: they do not change" "log_line '$UPDATE_CHECK' && [ \$(count 'the update check.s switch holds') -eq 1 ]"
check "the summary, word for word, with the fake's numbers at the game's own sizes, 10 s after the announcement" "log_line '$SUMMARY1' && first_after_10s"
check "one summary, no problem; nothing written, no call redirected" "[ \$(summaries) -eq 1 ] && log_lacks 'PROBLEM' && untouched"

echo "== TB3 the test build, TWEAKSV_TEST_SUMMARY_MS=500: once the first summary is out, the textures grow past the budget and the flag goes up - a summary says so"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe tbgrow)
check "the test build's line: the summaries after the first every 500 ms" "log_line '\[TweaksV\] TEST BUILD: the texture budget summary comes every 500 ms after its first (TWEAKSV_TEST_SUMMARY_MS=500).\$'"
check "the first summary as the Release build's, word for word, then the update check's line once" "log_line '$SUMMARY1' && [ \$(count 'the update check.s switch holds') -eq 1 ]"
check "the fake grew the counters after it; the next summary, word for word: past the budget, the flag up" "has 'grown after the first summary' && log_line '$SUMMARY2'"
check "no problem; the sites as placed, the data as the fake set it" "log_lacks 'PROBLEM' && untouched"

echo "== TB4 the test build, TWEAKSV_TEST_SUMMARY_MS=500, nothing changes: no summary after the first"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe wait)
check "the first summary, then none in the readings every 500 ms after it" "log_line '$SUMMARY1' && [ \$(summaries) -eq 1 ] && log_lacks 'PROBLEM' && untouched"

echo "== TB5 the test build, TWEAKSV_TEST_SUMMARY_MS=500: the first summary before the game's start-up has set its data, which it sets after it"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe tblate)
check "the first summary and the update check's line with the data all 0, word for word: not set yet, nothing at a limit" "log_line '$UNSET' && log_line '$UPDATE_CHECK_UNSET' && log_lacks 'at the budget' && log_lacks 'at the most objects'"
check "the fake set the data after it; the summary as the Release build's and the update check's line again, word for word" "has 'set after the first summary' && log_line '$SUMMARY1' && log_line '$UPDATE_CHECK' && [ \$(count 'the update check.s switch holds') -ge 2 ]"
check "no problem; the sites as placed, the data as the fake set it" "log_lacks 'PROBLEM' && untouched"

echo "== TB6 the test build, TWEAKSV_TEST_SUMMARY_MS=500: one new high at a time with the flag down - the bytes, the game's mark, the objects - a summary each"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe tbhigh)
check "after the first, a summary for each new high alone, word for word, four in all, the update check's flag down in every one" "has 'highs raised one at a time after the first summary' && log_line '$SUMMARY1' && log_line '$HIGH_BYTES' && log_line '$HIGH_MARK' && log_line '$HIGH_OBJECTS' && [ \$(summaries) -eq 4 ] && log_lacks 'flag is up'"
check "no problem; the sites as placed, the data as the fake set it" "log_lacks 'PROBLEM' && untouched"

echo "== TB7 TEST_BREAK_PATTERN=Q-E-T3 (test build, ERROR_POPUP=1): logged, not reported - no problem, no box, no summary"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN Q-E-T3; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "the scanner's line and the module's, word for word; the other four found all the same" "log_has 'TEST BUILD: pattern Q-E-T3 is treated as not found (TEST_BREAK_PATTERN).' && log_line '${NOT_MEASURED}Q-E-T3 was not found.\$' && [ \$(grep -c -E 'Pattern Q-E-T[1245] found: .* 1 match,' TweaksV.log) -eq 4 ]"
check "no problem, no box, no site line, no announcement, no summary; nothing written" "log_lacks 'PROBLEM' && log_lacks 'message box not shown' && log_lacks 'DIAGNOSTIC: store of' && nothing_measured && untouched"

echo "== TB8 a site not as expected: the shift between the budgets' stores by 21, not 20"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbshift wait)
check "the reason with the bytes found, word for word; the store of the most logged before it, nothing after it" "log_line '${NOT_MEASURED}the code after Q-E-T1 at $(rva stores 21) is not the store of budgets 0 and 1: 66 0F 73 F0 15 F3 0F 7F 05 [0-9A-F ]*.\$' && log_has 'DIAGNOSTIC: store of the most objects' && log_lacks 'DIAGNOSTIC: store of budgets'"
check "no problem, no announcement, no summary; nothing written" "log_lacks 'PROBLEM' && nothing_measured && untouched"

echo "== TB9 the call after the budgets' stores already hooked by another mod: logged, nothing measured"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbcallhooked)
check "the call's line and the reason, word for word" "log_line '\[Resources\] DIAGNOSTIC: call after the budgets. stores at $(rva stores 74) -> 0x[0-9A-F]* (outside the game) (Q-E-T2)\$' && log_line '${NOT_MEASURED}the call after the budgets. stores at $(rva stores 74) is not a call into the game (another mod may have hooked it already) (Q-E-T2).\$'"
check "no problem, no announcement; nothing written, the call not made" "log_lacks 'PROBLEM' && nothing_measured && untouched elsewhere && has 'start-up: the call leads elsewhere, not made'"

echo "== TB10 an address outside the game: the getter's index"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbindexout)
check "the reason, word for word" "log_line '${NOT_MEASURED}the index at 0x[0-9A-F]* (outside the game) (Q-E-T3) is not inside the game.\$'"
check "no problem, no announcement; nothing written" "log_lacks 'PROBLEM' && nothing_measured && untouched"

echo "== TB11 the sites disagree on the counters: the update check's lea 8 bytes past them"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbcounters)
check "the reason, word for word, the placement's check logged before it" "log_line '${NOT_MEASURED}the update check at $(rva update) does not read the counters at $(rva counters) (Q-E-T5).\$' && log_has 'DIAGNOSTIC: placement check at'"
check "no problem, no announcement; nothing written" "log_lacks 'PROBLEM' && nothing_measured && untouched"

echo "== TB12 the sites disagree on the counters: the placement's check loads the most from 4 bytes past them"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbmost)
check "the reason, word for word" "log_line '${NOT_MEASURED}the placement.s check loads the most objects from $(rva counters 0x24), not from +0x20 of the counters at $(rva counters) (Q-E-T4).\$'"
check "no problem, no announcement; nothing written" "log_lacks 'PROBLEM' && nothing_measured && untouched"

echo "== TB13 the update check's multiplication by its raise factor turned into an addition"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbfactor)
check "the reason with the bytes found, word for word" "log_line '${NOT_MEASURED}the update check.s code at $(rva update 0xA0) is not the multiplication by its factor at $(rva switch 4): F3 44 0F 58 0D [0-9A-F ]* (Q-E-T5).\$'"
check "no problem, no announcement; nothing written" "log_lacks 'PROBLEM' && nothing_measured && untouched"

echo "== TB14 the plain game, DIAGNOSTIC_MODE=1: every pattern missing - logged, not the module's problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "the scanner's five lines and the module's, word for word" "[ \$(grep -c -E 'Pattern Q-E-T[1-5] not found: ' TweaksV.log) -eq 5 ] && log_line '${NOT_MEASURED}Q-E-T1 was not found.\$'"
check "no problem of the module's, no announcement" "log_lacks '\[Resources\] PROBLEM' && nothing_measured"

echo "== TB15 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the same lines and the same summary; nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe wait)
check "the five patterns, the nine lines of the sites, the announcement and the summary 10 s in, word for word" "patterns_found && sites_logged && announced && log_line '$SUMMARY1' && first_after_10s"
check "no problem; nothing written" "log_lacks 'PROBLEM' && untouched"

echo "== TB16 nine writers of the budgets besides the start-up's stores, each of another kind: each named, measured all the same"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbwritemov tbwritebyte tbwritesse tbwriteavx tbwritelock tbwriteimm tbwriteextract tbwriteword tbwriteaddword)
check "the line, word for word: mov of 8 bytes, of a byte with an imm8, movups, vmovdqu of 32 bytes, lock add, mov with an imm32, vextractf128 behind a C4 prefix with an imm8, mov and add of a word with an imm16" "writers_line 'and 9 besides them: $(rva writers 97), $(rva writers 106), $(rva writers 115), $(rva writers 124), $(rva writers 134), $(rva writers 144), $(rva writers 157), $(rva writers 169), $(rva writers 180)'"
check "the measurement announced all the same; no problem; nothing written" "announced && log_lacks 'PROBLEM' && untouched"

echo "== TB17 VIDEO_MEMORY_BUDGET=6000: the budgets raised as the game's start-up sets them, through the redirected call and nothing else"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe)
check "the redirect's line, word for word" "redirected 6000"
check "the raise's line, word for word: the four budgets the game had, the four now, the index in use" "log_line '${RAISE}1500, 1500, 3000 and 3000 MB) are raised to 6000 MB where they are below it: 6000, 6000, 6000 and 6000 MB now; the texture quality.s index 2 uses 6000 MB.\$'"
check "the start-up's call through TweaksV's stub to the game's function, once, its arguments as handed, its result back, the budgets raised before it ran" "startup 'a TweaksV stub' '6000, 6000, 6000 and 6000'"
check "nothing else written: the sites as placed but the call, the data as set but the budgets, one stub more, read+execute; no DIAGNOSTIC line of the part, Q-E-T5 not looked for, no problem" "report_is 'a TweaksV stub' '6000, 6000, 6000 and 6000' && [ \"\$(stubs)\" = \"\$((stubs0 + 1))\" ] && log_lacks '\[Resources\] DIAGNOSTIC' && log_lacks 'Q-E-T5' && log_lacks 'PROBLEM'"

echo "== TB18 none lowered: a gameconfig.xml that gives budgets 2 and 3 9000 MB, VIDEO_MEMORY_BUDGET=6000"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbconfigbig)
check "the raise's line, word for word: budgets 0 and 1 raised, 2 and 3 kept at 9000 MB, as the game's function saw them" "log_line '${RAISE}1500, 1500, 9000 and 9000 MB) are raised to 6000 MB where they are below it: 6000, 6000, 9000 and 9000 MB now; the texture quality.s index 2 uses 9000 MB.\$' && startup 'a TweaksV stub' '6000, 6000, 9000 and 9000' && report_is 'a TweaksV stub' '6000, 6000, 9000 and 9000'"

echo "== TB19 VIDEO_MEMORY_BUDGET=1500: no budget below it - left as they are"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 1500 RESOURCES; out=$(game full_game.exe)
check "the line, word for word; the call redirected all the same, the budgets as the game set them; no problem" "log_line '${RAISE}1500, 1500, 3000 and 3000 MB) are at least 1500 MB each - left as they are; the texture quality.s index 2 uses 3000 MB.\$' && redirected 1500 && startup 'a TweaksV stub' '1500, 1500, 3000 and 3000' && log_lacks 'PROBLEM'"

echo "== TB20 VIDEO_MEMORY_BUDGET=1000, below the lowest budget the game's own file gives: 1500 used and reported"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 1000 RESOURCES; out=$(game full_game.exe)
check "the problem, word for word, the part's only one, in the start-up summary" "log_line '\[Resources\] PROBLEM: TweaksV.ini: \[RESOURCES\] VIDEO_MEMORY_BUDGET=1000 is outside 0 or 1500..16384 - using 1500.\$' && one_problem"
check "1500 used: the redirect's line with it, the budgets left as they are" "redirected 1500 && log_has 'are at least 1500 MB each - left as they are' && report_is 'a TweaksV stub' '1500, 1500, 3000 and 3000'"

echo "== TB21 the texture quality's index 7, outside 0 to 3, when the start-up sets the budgets"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbnoindex)
check "the raise's line names the index, which has no budget, word for word" "log_line '${RAISE}1500, 1500, 3000 and 3000 MB) are raised to 6000 MB where they are below it: 6000, 6000, 6000 and 6000 MB now; the texture quality.s index 7 has no budget.\$'"

echo "== TB22 [RESOURCES] ENABLED=0 with VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: the part does nothing"
fresh_ini; setkey ENABLED 0 RESOURCES; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "no line of the part, none of its patterns looked for, the header says it is off; nothing written, the call made to the game's function" "log_lacks -i 'texture budget' && log_lacks 'Q-E-T' && log_has 'Features: .*, Resources off, ' && untouched && startup 'the next step' '1500, 1500, 3000 and 3000'"

echo "== TB23 DRY_RUN=1 with VIDEO_MEMORY_BUDGET=6000: everything found and checked, the call not redirected"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the two dry-run lines, word for word, and not the redirect's; no problem" "dry_run_lines && log_lacks 'stores at RVA 0x[0-9A-F]* is redirected' && log_lacks 'PROBLEM'"
check "nothing written: the call made to the game's function, the budgets as the game set them" "untouched && startup 'the next step' '1500, 1500, 3000 and 3000'"

echo "== TB24 each check failing on its own, VIDEO_MEMORY_BUDGET=6000: the problem, word for word, the part's only one, nothing written"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; addkey TEST_BREAK_PATTERN Q-E-T2; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "Q-E-T2 missing (TEST_BREAK_PATTERN, test build)" "refused 'Q-E-T2 was not found.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbshift)
check "the shift between the budgets' stores by 21" "refused 'the code after Q-E-T1 at $(rva stores 21) is not the store of budgets 0 and 1: 66 0F 73 F0 15 F3 0F 7F 05 [0-9A-F ]*.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbcallhooked)
check "the call after the stores hooked by another mod - not made by the start-up either" "refused 'the call after the budgets. stores at $(rva stores 74) is not a call into the game (another mod may have hooked it already) (Q-E-T2).' elsewhere"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbindexout)
check "the index outside the game" "refused 'the index at 0x[0-9A-F]* (outside the game) (Q-E-T3) is not inside the game.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmost)
check "the placement's check loading the most 4 bytes past it" "refused 'the placement.s check loads the most objects from $(rva counters 0x24), not from +0x20 of the counters at $(rva counters) (Q-E-T4).'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbwritemov)
check "a writer of the budgets besides the start-up's stores" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva writers 97), which could undo the raise later.'"

echo "== TB30 the update check's raise factor not multiplied, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: the raise all the same, no measurement"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbfactor)
check "the raise and its lines; the measurement's reason, word for word, no announcement; no problem" "redirected 6000 && log_has 'are raised to 6000 MB where they are below it' && log_line '${NOT_MEASURED}the update check.s code at $(rva update 0xA0) is not the multiplication by its factor at $(rva switch 4): F3 44 0F 58 0D [0-9A-F ]* (Q-E-T5).\$' && log_lacks 'texture budget.s counters at' && log_lacks 'PROBLEM'"

echo "== TB31 the plain game, VIDEO_MEMORY_BUDGET=6000: every pattern missing - reported"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game plain_game.exe)
check "the scanner's four lines - Q-E-T5 not looked for - and the part's problem, word for word" "[ \$(grep -c -E 'Pattern Q-E-T[1-4] not found: ' TweaksV.log) -eq 4 ] && log_lacks 'Q-E-T5' && log_line '${NOT_RAISED}Q-E-T1 was not found.\$'"

echo "== TB32 Windows' dynamic code policy, VIDEO_MEMORY_BUDGET=6000: the call cannot be redirected - reported, nothing written"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe nodyncode)
check "the problem, word for word" "log_line '${NOT_RAISED}the call after the budgets. stores at $(rva stores 74) could not be redirected (details in TweaksV.log).\$'"
check "nothing written: the call made to the game's function, the budgets as the game set them" "untouched && startup 'the next step' '1500, 1500, 3000 and 3000'"

echo "== TB33 DIAGNOSTIC_MODE=1 with VIDEO_MEMORY_BUDGET=6000: the sites, the call's write, the summary 10 s in of the raised budget"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "the nine lines of the sites, the most and the budgets still 0 at TweaksV's start, and the call's write, word for word" "sites_logged && still_unset && call_written && redirected 6000"
check "the summary 10 s in, word for word, of the raised budget, the game's own four beside it" "log_line '$SUMMARY_RAISED' && first_after_10s"
check "no problem; the budgets raised, nothing else written" "log_lacks 'PROBLEM' && report_is 'a TweaksV stub' '6000, 6000, 6000 and 6000'"

echo "== TB34 the test build, TWEAKSV_TEST_SUMMARY_MS=500, VIDEO_MEMORY_BUDGET=6000: the first summary before the start-up, then the raise as it sets the budgets"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe tblate)
check "the first summary with the data all 0, then the raise's line and the summary of the raised budget, word for word" "log_line '$UNSET' && log_has 'are raised to 6000 MB where they are below it: 6000, 6000, 6000 and 6000 MB now' && log_line '$SUMMARY_RAISED'"
check "the start-up's call through the stub after the first summary; no problem" "has 'set after the first summary' && startup 'a TweaksV stub' '6000, 6000, 6000 and 6000' && log_lacks 'PROBLEM'"

echo "== TB35 the game's start-up before TweaksV started (a loader that starts TweaksV late), VIDEO_MEMORY_BUDGET=6000: the raise refused, reported"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbearly)
check "the problem, word for word, with the most and the budgets as the start-up set them, the part's only one, in the start-up summary; nothing written" "refused 'the most objects at $(rva counters 0x20) and the budgets at $(rva counters 0x28) were set before TweaksV started (MaxStreamingResources: 98304, MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB): the game.s start-up ran before TweaksV.s DllMain, and the call after its stores with it.'"
check "the start-up's call made before TweaksV, once, to the game's function itself; no line of a redirect" "startup 'the next step' '1500, 1500, 3000 and 3000' && log_lacks 'budgets. stores at RVA 0x[0-9A-F]* is redirected'"

echo "== TB36 the data's page unreadable while TweaksV starts, VIDEO_MEMORY_BUDGET=6000: the raise refused, reported"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbunreadable)
check "the problem, word for word, the part's only one, in the start-up summary; nothing written, the start-up's call made to the game's function" "refused 'the most objects at $(rva counters 0x20) and the budgets at $(rva counters 0x28) cannot be read.' && startup 'the next step' '1500, 1500, 3000 and 3000'"

echo "== TB37 the checks of the sites TB24 leaves, each failing on its own, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: the problem and the measurement's line, word for word, nothing written"
both_keys; out=$(game full_game.exe tbmostout)
check "the start-up's store of the most objects leading outside the game" "refused_both 'the most objects at 0x[0-9A-F]* (outside the game) (Q-E-T1) are not inside the game.'"
both_keys; out=$(game full_game.exe tbbudgetsout)
check "its store of budgets 0 and 1 leading outside the game" "refused_both 'the budgets at 0x[0-9A-F]* (outside the game) (Q-E-T1) are not inside the game.'"
both_keys; out=$(game full_game.exe tbt2moved)
check "the store of budgets 2 and 3 elsewhere, not right after the store of budgets 0 and 1" "refused_both 'the store of budgets 2 and 3 at $(rva update 0x60) (Q-E-T2) does not follow the store of budgets 0 and 1, which ends at $(rva stores 34).'"
both_keys; out=$(game full_game.exe tbbudgets23)
check "budgets 2 and 3 24 bytes after budgets 0 and 1" "refused_both 'budgets 2 and 3 at $(rva counters 0x40) (Q-E-T2) do not lie 16 bytes after budgets 0 and 1 at $(rva counters 0x28).'"
both_keys; out=$(game full_game.exe tbcountersout)
check "the counters the placement's check loads outside the game" "refused_both 'the counters at 0x[0-9A-F]* (outside the game) (Q-E-T4) are not inside the game.'"
both_keys; out=$(game full_game.exe tbmoststore)
check "the most objects stored 4 bytes past where the placement's check reads them" "refused_both 'the most objects the placement.s check reads at $(rva counters 0x20) are not those Q-E-T1 stores at $(rva counters 0x24).'"
both_keys; out=$(game full_game.exe tbbudgetsoff)
check "the budgets stored 16 bytes past the counters' +0x28" "refused_both 'the budgets at $(rva counters 0x38) (Q-E-T1) are not at +0x28 of the counters at $(rva counters) (Q-E-T4).'"

echo "== TB37b the index's page alone unreadable while TweaksV starts, the counters' readable, VIDEO_MEMORY_BUDGET=6000: the raise refused with its own reason, reported"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbindexunreadable)
check "the problem, word for word, the part's only one, in the start-up summary; nothing written, the start-up's call made to the game's function" "refused 'the index at $(rva 'index page') (Q-E-T3) cannot be read.' && startup 'the next step' '1500, 1500, 3000 and 3000'"
both_keys; out=$(game full_game.exe tbindexunreadable)
check "with DIAGNOSTIC_MODE=1 the same problem, the measurement announced all the same; the most and the budgets not read, no redirect" "refused 'the index at $(rva 'index page') (Q-E-T3) cannot be read.' && announced && log_lacks 'are still 0 at TweaksV' && log_lacks 'budgets. stores at RVA 0x[0-9A-F]* is redirected'"

echo "== TB37c the test build, TWEAKSV_TEST_SUMMARY_MS=500: the raise held half done past the time of the first summary, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: each reading made again for a second, then given up with one line - no summary while the raise is held, a half-raised set never passing as the game's own"
both_keys; out=$(TWEAKSV_TEST_SUMMARY_MS=500 ASI=TweaksV_test.asi game full_game.exe tbhalfraise)
check "the fake held the raise at its write of budget 2 for 12 s, readings due within them, whenever the start-up came: no summary meanwhile" "has 'the raise held half done, 0 summaries meanwhile'"
check "the readings given up after a second: one line, word for word, however many there were" "log_line '\[Resources\] DIAGNOSTIC: texture budget: the raise of the budgets has not ended after 1 second: no summary comes until it has.\$' && [ \$(count 'the raise of the budgets has not ended') -eq 1 ]"
check "the summary once the raise is done, word for word: the game's own four and the four raised" "log_line '$SUMMARY_RAISED'"
check "the half-raised set in no line" "log_lacks '6000, 6000, 3000 and 3000'"
check "the raise's line, the budgets raised before the game's function ran, nothing else written, no problem" "log_has 'are raised to 6000 MB where they are below it: 6000, 6000, 6000 and 6000 MB now' && startup 'a TweaksV stub' '6000, 6000, 6000 and 6000' && report_is 'a TweaksV stub' '6000, 6000, 6000 and 6000' && log_lacks 'PROBLEM'"

echo "== TB37d an address inside the game, not aligned to the size of its reads, each on its own, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: the problem and the measurement's line, word for word, nothing written"
both_keys; out=$(game full_game.exe tbmostodd)
check "the most objects at an odd address" "refused_both 'the most objects at $(rva counters 0x21) (Q-E-T1) are not aligned to 4 bytes.'"
both_keys; out=$(game full_game.exe tbbudgetsodd)
check "the budgets on 4 bytes, not 8" "refused_both 'the budgets at $(rva counters 0x2C) (Q-E-T1) are not aligned to 8 bytes.'"
both_keys; out=$(game full_game.exe tbindexodd)
check "the index at an odd address" "refused_both 'the index at $(rva index 1) (Q-E-T3) is not aligned to 4 bytes.'"
both_keys; out=$(game full_game.exe tbcountersodd)
check "the counters on 4 bytes, not 8" "refused_both 'the counters at $(rva counters 4) (Q-E-T4) are not aligned to 8 bytes.'"
both_keys; out=$(game full_game.exe tbswitchodd)
check "the update check's switch and factors at an odd address: the raise all the same, the measurement's reason, word for word, no announcement, no problem" "redirected 6000 && log_line '${NOT_MEASURED}the update check.s switch and factors at $(rva switch 2) (Q-E-T5) are not aligned to 4 bytes.\$' && log_lacks 'texture budget.s counters at' && log_lacks 'PROBLEM'"

echo "== TB37e a pattern the game holds twice, each on its own, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: named so, never as not found, nothing written"
for n in 1 2 3 4; do
	both_keys; out=$(game full_game.exe tbt${n}twice)
	check "Q-E-T$n twice: the problem and the measurement's line, word for word; the scanner's lines give both matches" "refused_both 'the game code holds Q-E-T$n 2 or more times, not the expected 1.' && log_lacks 'Q-E-T$n was not found' && log_has 'Pattern Q-E-T$n found: .* 2 matches,' && log_has 'Q-E-T$n match 2 of 2 at RVA'"
done
both_keys; out=$(game full_game.exe tbt5twice)
check "Q-E-T5 twice: the raise all the same, the measurement's reason, word for word, no announcement, no problem" "redirected 6000 && log_line '${NOT_MEASURED}the game code holds Q-E-T5 2 or more times, not the expected 1.\$' && log_lacks 'Q-E-T5 was not found' && log_lacks 'texture budget.s counters at' && log_lacks 'PROBLEM'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbt2twice)
check "Q-E-T2 twice without DIAGNOSTIC_MODE: the problem, word for word, the part's only one, in the start-up summary, nothing written" "refused 'the game code holds Q-E-T2 2 or more times, not the expected 1.' && log_lacks 'Q-E-T2 was not found'"

echo "== TB38 a writer besides the start-up's stores, VIDEO_MEMORY_BUDGET=6000 and DIAGNOSTIC_MODE=1: the raise refused, the writer named, the budget measured"
both_keys; out=$(game full_game.exe tbwritemov wait)
check "the problem, word for word, the part's only one, in the start-up summary; nothing written" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva writers 97), which could undo the raise later.'"
check "the writers' line names it; neither the start-up's check nor the redirect made" "writers_line 'and 1 besides them: $(rva writers 97)' && log_lacks 'are still 0 at TweaksV' && log_lacks 'budgets. stores at RVA 0x[0-9A-F]* is redirected'"
check "the measurement all the same: the announcement and the summary 10 s in, word for word, of the game's own budgets" "announced && log_line '$SUMMARY1' && first_after_10s"

echo "== TB50 the scanner's readings of the bytes before an operand (SPECIFICATIONS 8): a writer of the budgets behind a byte that once hid it - a 0F, a C5, a C4, an F3, a 0F 38, an F2 the instruction before ends with - each counted, and a load whose bytes read as a store counted too"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe tbmisread0f tbmisreadc5 tbmisreadc4 tbmisreadf3 tbmisread38 tbmisreadf2)
check "the line, word for word: mov after and eax, 0Fh; mov of 8 bytes after mov al, 0C5h; movups with a REX.B from 8 bytes before the budgets after mov al, 0C4h; movd after mov al, 0F3h; mov after mov eax, 380F0000h; movbe after mov al, 0F2h - each by its own first byte; measured all the same, nothing written" "writers_line 'and 6 besides them: $(rva misread 5), $(rva misread 15), $(rva misread 26), $(rva misread 38), $(rva misread 53), $(rva misread 63)' && announced && log_lacks 'PROBLEM' && untouched && has 'the misread block as placed'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisread0f)
check "the mov behind the 0F alone, VIDEO_MEMORY_BUDGET=6000: the raise refused, the writer named, nothing written" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 5), which could undo the raise later.' && has 'the misread block as placed'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisreadc5)
check "the mov of 8 bytes behind the C5 alone: refused, named" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 15), which could undo the raise later.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisreadc4)
check "the movups behind the C4 alone, 16 bytes from 8 before the budgets, which the VEX reading takes for 8: refused, named by its REX" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 26), which could undo the raise later.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisreadf3)
check "the movd behind the F3 alone, which the F3 as its prefix makes movq's load: refused, named by its 66" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 38), which could undo the raise later.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisread38)
check "the mov behind a 0F 38 alone, which the three-byte reading takes for no store: refused, named by its 89" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 53), which could undo the raise later.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisreadf2)
check "the movbe behind an F2 alone, which the F2 as its prefix makes crc32's load: refused, named by its 0F" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 63), which could undo the raise later.'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; out=$(game full_game.exe tbmisreadload)
check "a load whose bytes read as a store, movups of budget 0, its 10 read as adc: counted all the same, a possible writer named by its 10 - safe, where a missed writer is not" "refused 'the budgets at $(rva counters 0x28) have 1 possible writer besides the start-up.s stores, the first at $(rva misread 74), which could undo the raise later.'"

echo
echo "TEXTURE BUDGET SCENARIOS: $pass passed, $fail failed"
