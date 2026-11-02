#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The StaticBounds part (TweaksVStaticBounds): the fake game's store init (C-E-L1) run as code, its call of the pool size lookup redirected,
# so that the static bounds store gets the larger of its size and STATICBOUNDS_SIZE, everything built from what comes back, said once, and the
# fake's three other stores stay as they are: the size below and above the key, the key at its limits and beyond; the regression of start 3
# of 2.7.0's proof, names past the game's size carried and names past the raise trapped, and the old way's write to the pool alone trapped at
# the name map; ENABLED=0 and both old plugins; DRY_RUN; DIAGNOSTIC_MODE's lines; C-E-L1 and C-E-D1 each missing and held twice, the call
# another mod's, the redirect not made, each reported with nothing written or, for C-E-D1, a line and the raise going on; the read-back with
# its numbers, its store differing or not made, more than 32767 in use; the plain game; the check that the game asks, its clock from the
# session's start, the game placing blip_controller, other programs before it (the test build's TWEAKSV_TEST_STATICBOUNDS_CHECK): asked in
# time, before it, before TweaksV loaded, never, after the check; the hook made by the part with the Blips part off, no hook and no check beside
# a plugin that does its job, no report without a session; a check that cannot be made, the problem word for word, once, the redirect on and
# the store raised all the same: Y-E-H1 missing, each of the hook's patterns missing, the natives' call another mod's, each event not made
# (TWEAKSV_TEST_STATICBOUNDS_NO_EVENT), its thread refused (TWEAKSV_TEST_REFUSE_THREAD), and the switches' limits; a size above the key's
# default on a pool memory budget not raised, one problem, and nothing at the default, on a raised budget, in DRY_RUN or with nothing redirected.
# Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=112
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata scripts

RVA='RVA 0x[0-9A-F]*'
TAG='\[StaticBounds\] '
NOT_RAISED="${TAG}PROBLEM: The static bounds store.s size is not raised this session: "
OTHERS='other stores 500 7 40000, lookups 4 of 4 good, last error kept 4'
STUB='static bounds: call through a stub, the rest as placed, '
GAME='static bounds: call to the lookup, the rest as placed, '
# store_is N: the report's words of a store of N slots, everything from what came back; RAISED, UNTOUCHED: the store raised or as the game made it
store_is() { echo "the store made: $1 slots, pool $1, name map $(( $1 / 4 + 1 )) buckets, streamer $1, word $(( $1 & 65535 ))"; }
RAISED="${STUB}$(store_is 16000), names 0 of 0, no trap, $OTHERS"
UNTOUCHED="${GAME}$(store_is 12000), names 0 of 0, no trap, $OTHERS"
# redirected N: the redirect's line, word for word, for STATICBOUNDS_SIZE=N; made N: the check's line, for its time N ("10 seconds"); sizes G WORDS: the sizes' line
redirected() { log_line "${TAG}The static bounds store.s size call at $RVA is redirected: the store gets at least $1 slots (STATICBOUNDS_SIZE), more where the game gives it more.\$"; }
made() { log_line "${TAG}The static bounds check waits for the session.s start - the game placing blip_controller - and then $1 for the game to ask for the store.s size.\$"; }
sizes() { log_line "${TAG}The size the game gives the static bounds store, $1, $2\$"; }
said() { count 'The size the game gives the static bounds store'; }
# quiet: no problem of the part; kept: the game left as it was, nothing redirected, every page its own protection
quiet() { log_lacks '\[StaticBounds\] PROBLEM'; }
kept() { has "$UNTOUCHED" && has 'protect: all restored (RX)' && log_lacks "${TAG}.*is redirected: the store gets"; }
# time_of SED: what SED's s///p takes from TweaksV.log; session_ms, asked_ms: the session's start and the game's asking, timed
time_of() { tr -d '\r' < TweaksV.log | sed -n "$1"; }
session_ms() { time_of "s/.*${TAG}DIAGNOSTIC: the session started \([0-9]*\) ms after the part.s start (blip_controller placed): the static bounds check.s clock runs.\$/\1/p"; }
asked_ms() { time_of "s/.*${TAG}DIAGNOSTIC: the game.s lookup of the static bounds store.s size came \([0-9]*\) ms after the part.s start.\$/\1/p"; }
# back_is COUNT POOL BUCKETS NAMES STREAMER WORD USED: the read-back's line, word for word, once
back_is() { log_line "${TAG}The static bounds store.s read-back: count $1, pool $2, name map $3 buckets with $4 names, streamer $5 (its word $6), $7 slots in use, its streaming base 4242.\$" && [ "$(count 'read-back: count')" -eq 1 ]; }
no_back() { log_lacks "${TAG}.*read-back: count"; }

echo "== SB1 the defaults: the call redirected, the store made with 16000 slots in the place of the game's 12000, everything from that, said once, the three other stores as the game gave them"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the redirect's line, word for word, once" "redirected 16000 && [ \$(count 'is redirected: the store gets') -eq 1 ]"
check "the store's count, pool, name map, streamer and word all 16000's; the others left alone; every lookup's arguments and last error the game's" "has '$RAISED'"
check "the sizes said once, word for word" "sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the redirect and nothing else: every page its own protection, no problem, no DIAGNOSTIC line" "has 'protect: all restored (RX)' && quiet && log_lacks '${TAG}DIAGNOSTIC'"

echo "== SB2 the configuration gives the store 24500, more than STATICBOUNDS_SIZE: kept"
fresh_ini; out=$(game full_game.exe sbbig)
check "the store keeps 24500, said once, word for word; the redirect made all the same, no problem" "has '${STUB}$(store_is 24500)' && sizes 24500 'is kept: it is not below STATICBOUNDS_SIZE=16000.' && [ \$(said) -eq 1 ] && redirected 16000 && quiet"

echo "== SB3 STATICBOUNDS_SIZE=32767, the key's maximum: raised to it"
fresh_ini; setkey STATICBOUNDS_SIZE 32767; out=$(game full_game.exe)
check "32767 slots, each line word for word" "has '${STUB}$(store_is 32767)' && redirected 32767 && sizes 12000 'is raised to 32767 (STATICBOUNDS_SIZE).' && quiet"

echo "== SB4 STATICBOUNDS_SIZE=1, below the game's size: the game's size kept"
fresh_ini; setkey STATICBOUNDS_SIZE 1; out=$(game full_game.exe)
check "12000 slots through the stub, the line word for word" "has '${STUB}$(store_is 12000)' && sizes 12000 'is kept: it is not below STATICBOUNDS_SIZE=1.' && quiet"

echo "== SB5 STATICBOUNDS_SIZE=32768 and 100000, past the key's range: 32767 used, reported"
for past in 32768 100000; do
	fresh_ini; setkey STATICBOUNDS_SIZE $past; out=$(game full_game.exe)
	check "$past: the setting's problem, word for word, and 32767 slots" "log_has 'PROBLEM: TweaksV.ini: \[STATICBOUNDS\] STATICBOUNDS_SIZE=$past is outside 1..32767 - using 32767.' && has '$(store_is 32767)' && quiet"
done

echo "== SB6 the game gives the store 0, or -1, which is the default, 1: raised as any size below the key; -2: left as the game gave it, reported once"
fresh_ini; out=$(game full_game.exe sbzero)
check "0: 16000 slots, said; no problem" "has '$RAISED' && sizes 0 'is raised to 16000 (STATICBOUNDS_SIZE).' && quiet"
fresh_ini; out=$(game full_game.exe sbdefault)
check "-1: the default, 1, raised to 16000, said; no problem" "has '$RAISED' && sizes 1 'is raised to 16000 (STATICBOUNDS_SIZE).' && quiet"
fresh_ini; out=$(game full_game.exe sbnegative)
check "-2: the problem, word for word, once; left as it came; no sizes line" "log_line '${NOT_RAISED}the game gives the store the size -2, which is none - left as the game gave it.\$' && [ \$(count 'which is none') -eq 1 ] && has 'the store made: -2 slots' && [ \$(said) -eq 0 ]"

echo "== SB7 the regression of start 3: names past the game's size carried to the end of the raise, past the raise trapped, and the old way's write to the pool alone trapped at the name map"
fresh_ini; out=$(game full_game.exe sbnames 16000)
check "the game's 12000 passed: 16000 of 16000 names carried, no trap" "has '$(store_is 16000), names 16000 of 16000, no trap'"
fresh_ini; out=$(game full_game.exe sbnames 16001)
check "one past the raise: the game's own trap, at the pool, since the pool and the map are equal" "has 'names 16000 of 16001, the trap of a full pool'"
fresh_ini; setkey ENABLED 0 STATICBOUNDS; out=$(game full_game.exe sbnames 12001)
check "the part off: one past the game's size, its own trap" "has '$(store_is 12000), names 12000 of 12001, the trap of a full pool'"
fresh_ini; setkey ENABLED 0 STATICBOUNDS; out=$(game full_game.exe sbmodpool sbnames 12001)
check "the part off and a mod writing the pool alone, the way the old hooks raised it: the crash of start 3, the name map's trap at its 12001st name" "has 'names 12000 of 12001, the trap of a full name map'"

echo "== SB8 [STATICBOUNDS] ENABLED=0, with DIAGNOSTIC_MODE=1: nothing looked for, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 STATICBOUNDS; out=$(game full_game.exe)
check "the header says off, no line of the part, its patterns not looked for; nothing written" "log_has 'StaticBounds off' && log_lacks '\[StaticBounds\]' && log_lacks 'C-E-' && kept"

echo "== SB9 the old plugins, StaticBoundLimitAdjuster.asi in the game folder and StaticBoundsLimitAdjuster.asi in scripts\\: each switches the part off"
for old in StaticBoundLimitAdjuster.asi scripts/StaticBoundsLimitAdjuster.asi; do
	fresh_ini; touch "$old"; out=$(game full_game.exe); rm -f "$old"
	name=$(basename "$old")
	check "$name: reported word for word, the header says off; no line of the part, nothing written" "log_line 'PROBLEM: Old plugin found: .*$name. TweaksV.s StaticBounds part is switched off for this session so nothing is patched twice. Delete the old file - TweaksV replaces it.\$' && log_has 'StaticBounds off' && log_lacks '\[StaticBounds\]' && kept"
done

echo "== SB10 DRY_RUN=1: nothing written, the line says what would be"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the would line, word for word; the store made through the game's lookup, no sizes line, no problem" "log_line '${TAG}DRY_RUN: would redirect the static bounds store.s size call at $RVA\.\$' && kept && [ \$(said) -eq 0 ] && quiet"

echo "== SB11 DIAGNOSTIC_MODE=1: both patterns found once, the sites at their RVAs, the game's asking timed"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "C-E-L1 and C-E-D1 found exactly once each" "[ \$(grep -c -E 'Pattern C-E-(L1|D1) found: .* 1 match,' TweaksV.log) -eq 2 ]"
check "the call and the store, word for word; the asking, once" "log_line '${TAG}DIAGNOSTIC: the static bounds store.s size call at $RVA -> $RVA (C-E-L1)\$' && log_line '${TAG}DIAGNOSTIC: the static bounds store at $RVA (C-E-D1)\$' && [ \$(count 'DIAGNOSTIC: the game.s lookup of the static bounds') -eq 1 ] && [ -n \"\$(asked_ms)\" ]"
check "the redirect and the sizes as without DIAGNOSTIC_MODE, no problem" "redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && quiet"

echo "== SB12 C-E-L1 missing or held twice, its call another mod's, the redirect not made: reported, nothing written"
fresh_ini; addkey TEST_BREAK_PATTERN C-E-L1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "not found: the scanner's test line, the problem word for word, once; nothing written" "[ \$(count 'TEST BUILD: pattern C-E-L1 is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && log_line '${NOT_RAISED}C-E-L1 was not found.\$' && [ \$(count 'is not raised this session') -eq 1 ] && kept"
fresh_ini; out=$(game full_game.exe sbl1twice)
check "twice: named as twice, not as missing; nothing written" "log_line '${NOT_RAISED}the game code holds C-E-L1 2 or more times, not the expected 1.\$' && log_has 'Pattern C-E-L1 matches 2 or more times - not used (the site must be unique).' && kept"
fresh_ini; out=$(game full_game.exe sbl1hooked)
check "another mod's call: the problem, word for word, left where it led" "log_line '${NOT_RAISED}the static bounds store.s size call at $RVA is not a call into the game (another mod may have hooked it already) (C-E-L1).\$' && has 'static bounds: call to another mod, the rest as placed, the store not made'"
fresh_ini; out=$(game full_game.exe nodyncode)
check "Windows' dynamic code policy: the problem, word for word; nothing written" "log_line '${NOT_RAISED}the game code could not be changed (details in TweaksV.log).\$' && kept && log_lacks 'static bounds check waits'"

echo "== SB13 C-E-D1 missing, held twice, or its lea no store's: a line, no problem, the raise going on, no read-back"
NO_BACK="${TAG}The static bounds store.s read-back is not done this session: "
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN C-E-D1; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst script sbwait)
check "not found: the line, word for word; raised; no problem, no box of the part, no read-back" "log_line '${NO_BACK}C-E-D1 was not found.\$' && has '$RAISED' && quiet && no_back"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbd1twice sbfirst script sbwait)
check "twice: the line, word for word; raised; no problem, no read-back" "log_line '${NO_BACK}the game code holds C-E-D1 2 or more times, not the expected 1.\$' && has '$RAISED' && quiet && no_back"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbd1out sbfirst script sbwait)
check "its lea outside the game: the line, word for word; raised; no problem, no read-back" "log_line '${NO_BACK}the store at 0x[0-9A-F]* (outside the game) is not inside the game (C-E-D1).\$' && has '$RAISED' && quiet && no_back"
fresh_ini; out=$(game full_game.exe sbd1odd)
check "its lea 4 bytes past the store: the line, word for word; raised, no problem" "log_line '${NO_BACK}the store at $RVA is not aligned to 8 bytes (C-E-D1).\$' && has '$RAISED' && quiet"

echo "== SB14 the read-back: once, by the check's thread, after the session's start, every field the game's"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbnames 13000 script sbwait)
check "a load past the game's 12000: the line, word for word; no problem; every name carried" "back_is 16000 16000 4001 13000 16000 16000 13000 && quiet && has 'names 13000 of 13000, no trap'"
fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbmodpool script sbwait)
check "a mod writing the pool alone: the problem, word for word, once, in a box after the start-up summary" "back_is 16000 30000 4001 0 16000 16000 0 && log_line '${NOT_RAISED}the store.s count 16000, its pool 30000 and its streamer 16000 differ, and the load would end at the smallest (another mod may have changed one).\$' && [ \$(count 'differ, and the load') -eq 1 ] && log_has 'Showing a problem after the start-up summary (1 problem(s)).'"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbstreamer script sbwait)
check "the streamer's count another: the problem, word for word, once" "log_line '${NOT_RAISED}the store.s count 16000, its pool 16000 and its streamer 15999 differ, and the load would end at the smallest (another mod may have changed one).\$' && [ \$(count 'differ, and the load') -eq 1 ]"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbnotmade script sbwait)
check "the store not finished: a line, no read-back, no problem" "log_line '${TAG}The static bounds store.s read-back is not done: the game has not finished making the store.\$' && no_back && quiet"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbhuge sbnames 32767 script sbwait)
check "gameconfig.xml's 40000, kept, 32767 in use: no problem" "sizes 40000 'is kept: it is not below STATICBOUNDS_SIZE=16000.' && back_is 40000 40000 10001 32767 40000 40000 32767 && quiet"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst sbhuge sbnames 32768 script sbwait)
check "gameconfig.xml's 40000, 32768 in use: the problem, word for word, once" "log_line '${TAG}PROBLEM: The static bounds store holds 32768 slots in use this session, past the 32767 whose every slot is sound: gameconfig.xml gives it 40000, and an interior whose slot lies past them can lose its collision.\$' && [ \$(count 'past the 32767') -eq 1 ]"
fresh_ini; out=$(game full_game.exe sbfirst script)
check "the Release build reads back too, every boot; no wait for it, no test build's line" "log_lacks '${TAG}TEST BUILD' && quiet"

NOT_ASKED="${NOT_RAISED}the game did not ask for the store.s size within 2 seconds of the session.s start - it asked before TweaksV started, or makes the store another way, and the store keeps the game.s size.\$"
AFTER_ALL='The game.s lookup of the static bounds store.s size came after all, after the check had reported it missing.'

echo "== SB15 the check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=10, DIAGNOSTIC_MODE=1, a script program placed, past the check's time): the game asks in time - timed, nothing reported"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=10
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe script wait)
session=$(session_ms); asked=$(asked_ms)
check "the test build's line and the check's line, once each; the redirect's line" "[ \$(count 'TEST BUILD: the static bounds check comes 10 seconds after the session.s start instead of 60 seconds (TWEAKSV_TEST_STATICBOUNDS_CHECK=10).') -eq 1 ] && made '10 seconds' && [ \$(count 'The static bounds check waits') -eq 1 ] && redirected 16000"
check "the session's start and the asking timed, the asking within the check's 10 seconds; the sizes said once" "[ '${session:-none}' != none ] && [ '${asked:-none}' != none ] && [ \$(( ${asked:-99999} - ${session:-0} )) -lt 10000 ] && [ \$(said) -eq 1 ]"
check "past the check's time: nothing reported, the store raised, read back" "log_lacks 'did not ask' && quiet && has '$RAISED' && back_is 16000 16000 4001 0 16000 16000 0"

echo "== SB16 the store made before TweaksV loaded, never again (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, ERROR_POPUP=1): reported, with the part's box, no read-back"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait)
check "the problem, word for word, once" "log_line '$NOT_ASKED' && [ \$(count 'did not ask') -eq 1 ]"
check "shown after the start-up summary, in a box of its own" "log_has 'Showing a problem after the start-up summary (1 problem(s)).' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"
check "the store through the game's lookup before TweaksV, the redirect made all the same, no sizes line, no read-back" "has '${STUB}$(store_is 12000)' && [ \$(said) -eq 0 ] && log_lacks 'after all, after the check' && no_back"

echo "== SB17 the store never made, the others made (test build): reported"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnever script sbwait)
check "the problem, word for word, once; the others through the stub" "log_line '$NOT_ASKED' && [ \$(count 'did not ask') -eq 1 ] && has '${STUB}the store not made, other stores 500 7 40000, lookups 3 of 3 good, last error kept 3'"

echo "== SB18 the store made 4 seconds late, after the check of 2 seconds (test build): reported, then asked after all"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sblate script)
check "the problem first, then the line that the game asked after all, then the sizes, each once; raised all the same" "[ \$(count 'did not ask') -eq 1 ] && [ \$(count '$AFTER_ALL') -eq 1 ] && [ \$(grep -n 'did not ask' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'after all, after the check' TweaksV.log | cut -d: -f1) ] && sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ] && has '$RAISED'"

echo "== SB19 [BLIPS] ENABLED=0 (test build, the store made before TweaksV, a script program placed): the part makes the script programs' hook itself, and the check reports"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait)
check "the hook made by the part, its line under the part's tag, word for word; the program through TweaksV's stub; nothing of the Blips part" "log_line '${TAG}The natives. resolution call at $RVA is redirected: every script program the game places passes TweaksV, before and after the game resolves its natives.\$' && has 'script: 1 placed' && has 'through a TweaksV stub' && log_lacks '\[Blips\]'"
check "the check made, and its problem past its time, word for word, once" "made '2 seconds' && log_line '$NOT_ASKED' && [ \$(count 'did not ask') -eq 1 ]"

echo "== SB20 no script program placed (test build): the session never starts, so the check never reports"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly sbwait)
check "the check made and waiting, nothing reported past its time" "made '2 seconds' && log_lacks 'did not ask' && quiet"

echo "== SB21 other programs before blip_controller (test build): the session starts with blip_controller, not with the first program of all, which comes perhaps before the landing page"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly scriptotheralone sbwait)
check "another program alone, the store made before TweaksV: the session not started, nothing reported past the check's time" "has 'script: 1 placed' && has 'blips: 0 placed' && made '2 seconds' && log_lacks '${TAG}.*did not ask' && quiet"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly scriptother scriptgap sbwait)
check "another program, blip_controller 3 seconds later, the store made before TweaksV: the problem past the check's time from blip_controller, once" "has 'script: 2 placed' && log_line '$NOT_ASKED' && [ \$(count 'did not ask') -eq 1 ] && [ \$(grep -n 'the threshold 150 -> 1900 (placement 1)' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'did not ask' TweaksV.log | cut -d: -f1) ]"
check "the session's start timed once, blip_controller's, the program before it and its 3 seconds gone by" "[ \$(count 'DIAGNOSTIC: the session started') -eq 1 ] && [ \$(session_ms) -ge 2900 ]"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbfirst script sbwait)
check "the game asks before blip_controller is placed: that ends the check, the session's start timed after it, nothing reported past the check's time; raised, read back" "[ \$(asked_ms) -le \$(session_ms) ] && log_lacks '${TAG}.*did not ask' && quiet && has '$RAISED' && back_is 16000 16000 4001 0 16000 16000 0"

echo "== SB22 nothing redirected, no check (test build, the store made before TweaksV, a script program placed): DRY_RUN, a refused job"
for case in dryrun sbl1hooked; do
	fresh_ini; options="sbearly script sbwait"
	if [ "$case" = dryrun ]; then setkey DRY_RUN 1; else options="$options $case"; fi
	out=$(ASI=TweaksV_test.asi game full_game.exe $options)
	check "$case: no check - neither its line nor its test build's line nor its problem past its time" "log_lacks 'static bounds check' && log_lacks 'did not ask'"
done

echo "== SB23 a size below 0, a script program placed (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=4, past the fake's making): its problem once, and the check not again"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=4
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbnegative script sbwait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the size reported once, the check made and silent past its time" "[ \$(count 'which is none') -eq 1 ] && made '4 seconds' && log_lacks 'did not ask'"

# the line of a check left to a plugin that does the Blips part's job, word for word
LEFT="${TAG}The static bounds check and its read-back are not made this session: they start with the session, which TweaksV sees through the script programs. hook, and a plugin that does the Blips part.s job on the script programs is installed (named above) - TweaksV leaves the script programs to it.\$"

echo "== SB24 BlipLimitFix.asi in the game folder, the Blips part on in TweaksV.ini (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the store made before TweaksV, a script program placed): no check, the line says why"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; touch BlipLimitFix.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait); rm -f BlipLimitFix.asi
check "the line, word for word; no check, nothing reported past its time; the program placed through the game's own function; the redirect made all the same, no problem of the part" "log_line '$LEFT' && log_lacks 'static bounds check comes' && log_lacks 'did not ask' && has 'through the game function' && redirected 16000 && quiet"

echo "== SB25 [BLIPS] ENABLED=0 with ResourceExtensions.asi in the game folder (test build): the plugin does the Blips part's job whatever TweaksV.ini says - no hook of the script programs, no check, the line"
fresh_ini; setkey ENABLED 0 BLIPS; touch ResourceExtensions.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script sbwait); rm -f ResourceExtensions.asi
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the conflict check's line and the part's, word for word; no hook of the script programs, no check; the redirect made all the same, no problem of the part" "log_line '\[TweaksV\] Conflict check: .*ResourceExtensions.asi is installed; TweaksV.s Blips part is off in TweaksV.ini already.\$' && log_line '$LEFT' && log_lacks 'natives. resolution call' && log_lacks 'static bounds check comes' && log_lacks 'did not ask' && has 'through the game function' && redirected 16000 && quiet"

# a check that cannot be made: its problem, word for word but for the reason, once; the start-up summary's box with N problems; no check made; no hook of the script programs made by the part
NOT_CHECKED="${TAG}PROBLEM: The check that the game asks for the static bounds store.s size is not done this session: "
not_checked() { log_line "${NOT_CHECKED}$1\$" && [ $(count 'size is not done this session: ') -eq 1 ]; }
boxed() { log_has "Showing the start-up summary ($1 problem(s))." && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:'; }
unchecked() { log_lacks 'The static bounds check waits' && log_lacks 'did not ask'; }
unhooked() { log_lacks 'natives. resolution call at RVA 0x[0-9A-F]* is redirected' && has 'through the game function'; }

echo "== SB26 TEST_BREAK_PATTERN=Y-E-H1 with the Blips part on (test build, ERROR_POPUP=1, a script program placed): the part joins the hook Blips could not make - its problem beside Blips', in the box, the store raised all the same"
fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN Y-E-H1; out=$(ASI=TweaksV_test.asi game full_game.exe script)
check "the check's problem, word for word, once, beside the Blips part's; the pattern looked for once; both in the box" "not_checked 'Y-E-H1 was not found.' && log_line '\[Blips\] PROBLEM: blip_controller.s threshold is not raised this session: Y-E-H1 was not found.\$' && [ \$(count 'TEST BUILD: pattern Y-E-H1 is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && boxed 2"
check "the redirect made, the store raised; no check made, the program placed through the game's own function" "redirected 16000 && has '$RAISED' && unchecked && unhooked"

echo "== SB27 each of the hook's patterns missing, [BLIPS] ENABLED=0 (test build, ERROR_POPUP=1, a script program placed): the part, the hook's first user, cannot make it"
for label in Y-E-H1 Y-E-N1 Y-E-I1; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe script)
	check "$label: the problem word for word, once, alone in the box; raised all the same; no check, no hook" "not_checked '$label was not found.' && boxed 1 && log_lacks '\[Blips\]' && redirected 16000 && has '$RAISED' && unchecked && unhooked"
done

echo "== SB28 the natives' resolution call another mod redirected, [BLIPS] ENABLED=0 (the Release build): the hook's own reason, the store raised all the same"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(game full_game.exe scripthooked)
check "the problem, word for word, once; the store raised; no check made, the other mod's call left where it led" "not_checked 'the natives. resolution call at $RVA is not a call into the game (another mod may have hooked it already).' && log_lacks '\[Blips\]' && redirected 16000 && has '$RAISED' && unchecked && has 'script: none placed, call to another mod'"

echo "== SB29 each of the check's events not made (test build, TWEAKSV_TEST_STATICBOUNDS_NO_EVENT, ERROR_POPUP=1, [BLIPS] ENABLED=0, a script program placed): the problem, the store raised all the same"
for event in asked session; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event ASI=TweaksV_test.asi game full_game.exe script)
	check "$event: the test build's line, once; the problem, word for word, once, in the box; raised, said once; no check, no hook" "log_line '${TAG}TEST BUILD: the static bounds check.s $event event is not made (TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event).\$' && [ \$(count 'event is not made') -eq 1 ] && not_checked 'its events could not be made.' && boxed 1 && redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && unchecked && unhooked"
done

echo "== SB30 the switch's limits: the Release build, another value, DRY_RUN - no event left out, nothing reported"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked game full_game.exe)
check "the Release build ignores it: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=other ASI=TweaksV_test.asi game full_game.exe)
check "another value: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; setkey DRY_RUN 1; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked ASI=TweaksV_test.asi game full_game.exe)
check "DRY_RUN: no event wanted, nothing reported" "quiet && log_lacks 'event is not made' && kept"

echo "== SB31 Windows refuses the check's thread (test build's TWEAKSV_TEST_REFUSE_THREAD, ERROR_POPUP=1, a script program placed): the problem, the store raised all the same"
fresh_ini; setkey ERROR_POPUP 1; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 static bounds check' ASI=TweaksV_test.asi game full_game.exe script)
check "the thread's line and the problem, word for word, once, in the box; raised; no check made, the program through the hook Blips made" "log_line '\[TweaksV\] The static bounds check thread could not be started (Windows error [0-9]*.*)\.\$' && not_checked 'its thread could not be started (details in TweaksV.log).' && boxed 1 && redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && unchecked && has 'through a TweaksV stub'"

echo "== SB32 the Release build given TWEAKSV_TEST_STATICBOUNDS_CHECK=2 (the store made before TweaksV, a script program placed, past those 2 seconds): the check stays 60 seconds, nothing said, nothing reported"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_CHECK=2 game full_game.exe sbearly script sbwait)
check "the check's line says 60 seconds, the test build's line is not there, nothing reported past the 2 seconds" "made '60 seconds' && log_lacks '${TAG}TEST BUILD' && log_lacks '${TAG}.*did not ask' && quiet"

echo "== SB33 BlipLimitFix.asi with the asked event not made (test build): no check was to be made, logged - not reported"
fresh_ini; touch BlipLimitFix.asi; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked ASI=TweaksV_test.asi game full_game.exe); rm -f BlipLimitFix.asi
check "the line, word for word, no problem of the part; the store raised all the same" "log_line '$LEFT' && quiet && redirected 16000 && has '$RAISED'"

# the problem of a size above the key's default on a budget not raised, word for word, once: may_not_fit SIZE WHY
PART_OFF='the HeapAdjuster part, which raises that budget, is off. A pool past the budget crashes the game: switch the part on, or lower STATICBOUNDS_SIZE.'
AT_136='POOL_MEMORY=136 leaves that budget at the game.s own. A pool past the budget crashes the game: raise POOL_MEMORY, or lower STATICBOUNDS_SIZE.'
may_not_fit() { log_line "${TAG}PROBLEM: The static bounds pool may not fit the pool memory budget this session: STATICBOUNDS_SIZE=$1 is above its default of 16000, and $2\$" && [ $(count 'may not fit the pool memory budget') -eq 1 ]; }

echo "== SB34 STATICBOUNDS_SIZE above its default on a pool memory budget not raised (test build, ERROR_POPUP=1): one problem of the start-up summary, the store raised all the same"
fresh_ini; setkey ERROR_POPUP 1; setkey STATICBOUNDS_SIZE 16001; setkey ENABLED 0 HEAP_ADJUSTER; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "[HEAP_ADJUSTER] ENABLED=0, one above the default: the problem, in the box; the redirect made, the store raised" "may_not_fit 16001 '$PART_OFF' && boxed 1 && redirected 16001 && has '${STUB}$(store_is 16001)'"
fresh_ini; setkey ERROR_POPUP 1; setkey STATICBOUNDS_SIZE 32767; setkey POOL_MEMORY 136; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "POOL_MEMORY=136, the key's maximum: the problem, in the box; the store raised" "may_not_fit 32767 '$AT_136' && boxed 1 && has '${STUB}$(store_is 32767)'"
for plugin in HeapAdjuster.asi ResourceExtensions.asi; do
	fresh_ini; setkey STATICBOUNDS_SIZE 32767; touch "$plugin"; out=$(game full_game.exe); rm -f "$plugin"
	check "$plugin, which switches the HeapAdjuster part off: the conflict check's problem and this one; the store raised" "log_has 'TweaksV.s HeapAdjuster part is switched off' && may_not_fit 32767 '$PART_OFF' && has '${STUB}$(store_is 32767)'"
done

echo "== SB35 what says nothing of the budget: the default on a budget not raised, a raised budget, nothing raised"
fresh_ini; setkey ENABLED 0 HEAP_ADJUSTER; out=$(game full_game.exe)
check "[HEAP_ADJUSTER] ENABLED=0 at the default, where a slot costs 17 bytes: no problem, the store raised" "quiet && has '$RAISED'"
fresh_ini; setkey POOL_MEMORY 136; out=$(game full_game.exe)
check "POOL_MEMORY=136 at the default: no problem, the store raised" "quiet && has '$RAISED'"
fresh_ini; setkey STATICBOUNDS_SIZE 32767; setkey POOL_MEMORY 137; out=$(game full_game.exe)
check "POOL_MEMORY=137, the least raise, at the key's maximum: no problem, the store raised" "quiet && has '$(store_is 32767)'"
fresh_ini; setkey STATICBOUNDS_SIZE 32767; setkey ENABLED 0 HEAP_ADJUSTER; setkey DRY_RUN 1; out=$(game full_game.exe)
check "DRY_RUN, on a budget not raised: nothing written, nothing said" "quiet && kept"
fresh_ini; setkey STATICBOUNDS_SIZE 32767; setkey ENABLED 0 HEAP_ADJUSTER; out=$(game full_game.exe nodyncode)
check "the redirect not made, on a budget not raised: its problem alone, nothing raised" "[ \$(count '${TAG}PROBLEM') -eq 1 ] && log_has '${NOT_RAISED}' && kept"

echo "== SB36 none of the game code present (plain fake game): both patterns looked for, the one missing that matters reported"
fresh_ini; out=$(game plain_game.exe)
check "the harness survived; both looked for and missing; C-E-D1's line, C-E-L1's problem" "grep -q 'harness: .* loaded' harness.out && [ \$(grep -c -E 'Pattern C-E-(L1|D1) not found: ' TweaksV.log) -eq 2 ] && log_line '${NOT_RAISED}C-E-L1 was not found.\$' && log_line '${TAG}The static bounds store.s read-back is not done this session: C-E-D1 was not found.\$'"

echo
echo "STATIC BOUNDS SCENARIOS: $pass passed, $fail failed"
