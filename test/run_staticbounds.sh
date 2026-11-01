#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The StaticBounds part (TweaksVStaticBounds): the fake game's static bounds pool's size call found after the load of the
# pool's name, checked against the pool size lookup the weapon components' pool size call (W-E-C4) leads to - looked for
# once for both parts (TweaksVPools) - and redirected, so that the pool the fake makes twice once TweaksV has started gets the
# larger of its configured size and STATICBOUNDS_SIZE, said once: the configuration below and above the key, the key at its
# limits and beyond; ENABLED=0 and both old plugins; DRY_RUN; DIAGNOSTIC_MODE with every site; each check failing on its own
# - a pattern missing, the weapon components' call hooked, 17 loads of the name, a call after two of them, the call another
# mod redirected, no call stub - each reported with nothing written; the WeaponLimits part off, the pool raised all the same;
# a second load without a call passed over; a call for another pool and a size below 0 passed on and reported; the plain
# game; the check that the game asks, its clock from the session's start, the first script program placed (the test build's
# TWEAKSV_TEST_STATICBOUNDS_CHECK): asked in time, never asked - the pool made before TweaksV loaded - asked after the check,
# the hook made by the part with the Blips part off in TweaksV.ini and the check reporting, no hook and no check with
# BlipLimitFix.asi installed or with Resource Extensions installed and the Blips part off, no report without a session; a
# check that cannot be made, the problem word for word, once, the call redirected and the pool raised all the same, no check
# and no hook made: Y-E-H1 missing with the Blips part on, the part's problem beside Blips' in the start-up summary's box,
# each of the hook's three patterns missing with the Blips part off, the part's problem alone in the box, the natives'
# resolution call another mod redirected, each of the two events not made (the test build's
# TWEAKSV_TEST_STATICBOUNDS_NO_EVENT), in the box, which the Release build, another value and DRY_RUN leave alone, and beside
# BlipLimitFix.asi only the line, no problem. In every run the sites as placed but for the call's rel32, the stub read+execute
# and every page its own protection. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata scripts

RVA='RVA 0x[0-9A-F]*'
TAG='\[StaticBounds\] '
NOT_RAISED="${TAG}PROBLEM: The static bounds pool.s size is not raised this session: "
# the report's field: the pool made twice through TweaksV's stub - or through the game's lookup, nothing written
RAISED='static bounds: sites as placed, size call to a stub (RX), pool sizes 16000 and 16000, the lookup asked 2 times, arguments as handed, last error kept'
UNTOUCHED='static bounds: sites as placed, size call to the lookup, pool sizes 12000 and 12000, the lookup asked 2 times, arguments as handed, last error kept'
# redirected N: the redirect's line, word for word, for STATICBOUNDS_SIZE=N; made N: the check's line, word for word, for its
# time N ("10 seconds"); sizes G WORDS: the sizes' line, word for word
redirected() { log_line "${TAG}The static bounds pool.s size call at $RVA is redirected: the pool gets at least $1 slots (STATICBOUNDS_SIZE) when the game makes it, more where the game.s configuration gives it more.\$"; }
made() { log_line "${TAG}The static bounds check waits for the session.s start - the first script program the game places - and then $1 for the game to ask for the pool.s size.\$"; }
sizes() { log_line "${TAG}The pool size the game.s configuration gives for the static bounds, $1, $2\$"; }
said() { count 'The pool size the game.s configuration gives for the static bounds'; }
# quiet: no problem of the part; kept: the game left as it was - the pool made through its own lookup, every page its own protection
quiet() { log_lacks '\[StaticBounds\] PROBLEM'; }
kept() { has "$UNTOUCHED" && has 'protect: all restored (RX)' && log_lacks 'is redirected: the pool gets'; }
# rva_of SED: what SED's s///p takes from TweaksV.log - the RVA a DIAGNOSTIC line leads to or stands at
rva_of() { tr -d '\r' < TweaksV.log | sed -n "$1"; }

echo "== SB1 the defaults: the call redirected, the pool made twice with 16000 slots in the place of the configuration's 12000, said once"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the redirect's line, word for word, once" "redirected 16000 && [ \$(count 'is redirected: the pool gets') -eq 1 ]"
check "the pool made twice through TweaksV's stub, read+execute: 16000 both times, the lookup asked with the arguments as handed, its last error kept" "has '$RAISED'"
check "the sizes said once, word for word, though the pool is made twice" "sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the redirect and nothing else: the sites as placed but for the call's rel32, every page its own protection, no problem, no DIAGNOSTIC line" "has 'protect: all restored (RX)' && quiet && log_lacks '\[StaticBounds\] DIAGNOSTIC'"

echo "== SB2 the configuration gives the pool 24500, more than STATICBOUNDS_SIZE: kept"
fresh_ini; out=$(game full_game.exe sbbig)
check "the pool keeps 24500, said once, word for word" "has 'size call to a stub (RX), pool sizes 24500 and 24500' && sizes 24500 'is kept: it is not below STATICBOUNDS_SIZE=16000.' && [ \$(said) -eq 1 ]"
check "the call redirected all the same, nothing else written, no problem" "redirected 16000 && has 'static bounds: sites as placed' && has 'protect: all restored (RX)' && quiet"

echo "== SB3 STATICBOUNDS_SIZE=100000, the key's maximum: raised to it"
fresh_ini; setkey STATICBOUNDS_SIZE 100000; out=$(game full_game.exe)
check "100000 both times, each line word for word" "has 'size call to a stub (RX), pool sizes 100000 and 100000' && redirected 100000 && sizes 12000 'is raised to 100000 (STATICBOUNDS_SIZE).' && quiet"

echo "== SB4 STATICBOUNDS_SIZE=1, below the configuration: the configuration's size kept"
fresh_ini; setkey STATICBOUNDS_SIZE 1; out=$(game full_game.exe)
check "12000 both times through the stub, the line word for word" "has 'size call to a stub (RX), pool sizes 12000 and 12000' && sizes 12000 'is kept: it is not below STATICBOUNDS_SIZE=1.' && quiet"

echo "== SB5 STATICBOUNDS_SIZE=200000, beyond the key's range: 100000 used, reported"
fresh_ini; setkey STATICBOUNDS_SIZE 200000; out=$(game full_game.exe)
check "the setting's problem, word for word, and 100000 both times" "log_has 'PROBLEM: TweaksV.ini: \[STATICBOUNDS\] STATICBOUNDS_SIZE=200000 is outside 1..100000 - using 100000.' && has 'pool sizes 100000 and 100000' && quiet"

echo "== SB6 [STATICBOUNDS] ENABLED=0, with DIAGNOSTIC_MODE=1: nothing looked for, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 STATICBOUNDS; out=$(game full_game.exe)
check "the header says off, no line of the part, its own pattern not looked for (W-E-C4 is the weapon components' too)" "log_has 'StaticBounds off' && log_lacks '\[StaticBounds\]' && log_lacks 'C-E-H1'"
check "the pool made through the game's lookup: 12000, nothing written" "kept"

echo "== SB7 the old plugins, StaticBoundLimitAdjuster.asi in the game folder and StaticBoundsLimitAdjuster.asi in scripts\\: each switches the part off"
for old in StaticBoundLimitAdjuster.asi scripts/StaticBoundsLimitAdjuster.asi; do
	fresh_ini; touch "$old"; out=$(game full_game.exe); rm -f "$old"
	name=$(basename "$old")
	check "$name: reported word for word, the header says off" "log_line 'PROBLEM: Old plugin found: .*$name. TweaksV.s StaticBounds part is switched off for this session so nothing is patched twice. Delete the old file - TweaksV replaces it.\$' && log_has 'StaticBounds off'"
	check "$name: no line of the part, nothing written" "log_lacks '\[StaticBounds\]' && kept"
done

echo "== SB8 DRY_RUN=1: nothing redirected, the line says what would be"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the would line, word for word, and not the redirect's" "log_line '${TAG}DRY_RUN: would redirect the static bounds pool.s size call at $RVA.\$' && log_lacks 'is redirected: the pool gets'"
check "the pool made through the game's lookup: 12000, no sizes line, nothing written, no problem" "kept && [ \$(said) -eq 0 ] && quiet"

echo "== SB9 DIAGNOSTIC_MODE=1: both patterns found once - W-E-C4 once, though the weapon components ask for it too - the lookup and the call at their RVAs, the write"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
name=$(rva_of "s/.*Pattern C-E-H1 found: BA 06 E4 DF 2C -> RVA \(0x[0-9A-F]*\), 1 match,.*/\1/p")
lookup=$(rva_of "s/.*${TAG}DIAGNOSTIC: pool size call at $RVA -> RVA \(0x[0-9A-F]*\) for the pool 0xBAE33CCF (W-E-C4): the pool size lookup\$/\1/p")
wclookup=$(rva_of "s/.*\[WeaponLimits\] DIAGNOSTIC: pool size call at $RVA -> RVA \(0x[0-9A-F]*\) for the pool 0xBAE33CCF, its default .*/\1/p")
call=$(rva_of "s/.*${TAG}DIAGNOSTIC: pool size call at RVA \(0x[0-9A-F]*\) -> RVA ${lookup:-none} for the pool 0x2CDFE406, its name loaded at RVA ${name:-none} (C-E-H1)\$/\1/p")
check "C-E-H1 and W-E-C4 found exactly once each, W-E-C4's line the only one of it" "[ \$(grep -c -E 'Pattern (C-E-H1|W-E-C4) found: .* 1 match,' TweaksV.log) -eq 2 ] && [ \$(count 'Pattern W-E-C4 ') -eq 1 ]"
check "the weapon components' pool size call and the lookup it leads to, word for word - the one their own line names" "[ -n '$lookup' ] && [ '$lookup' = '${wclookup:-none}' ]"
check "the static bounds pool's size call, word for word: to the same lookup, 11 bytes into C-E-H1's match" "[ -n '$call' ] && [ \$(( $call - ${name:-0} )) -eq 11 ]"
check "the write's line: the call's E8 kept, its rel32 to the stub" "log_line '${TAG}DIAGNOSTIC: static bounds pool.s size call at RVA ${call:-none}: E8 [0-9A-F ]* -> E8 [0-9A-F ]*\$'"
check "no load of the name without a call; the redirect and the sizes as without DIAGNOSTIC_MODE; no problem" "log_lacks 'is followed by no call' && redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && quiet"

echo "== SB10 TEST_BREAK_PATTERN for each pattern (test build): reported by its label, nothing written - W-E-C4 looked for once, for both parts"
for label in C-E-H1 W-E-C4; do
	fresh_ini; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the scanner's test line once and the problem, word for word" "[ \$(count 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && log_line '${NOT_RAISED}$label was not found.\$'"
	check "$label: nothing written - the pool made through the game's lookup" "kept"
done

echo "== SB11 the weapon components' pool size call another mod hooked: it leads outside the game, so the lookup is not known"
fresh_ini; out=$(game full_game.exe wchooked)
check "the problem, word for word - the reason the weapon components give too; nothing written" "log_line '${NOT_RAISED}the weapon components. pool size call at $RVA is not a call into the game (another mod may have hooked it already) (W-E-C4).\$' && [ \$(count 'is not a call into the game (another mod may have hooked it already) (W-E-C4).') -eq 2 ] && kept"

echo "== SB12 [WEAPON_LIMITS] ENABLED=0, with DIAGNOSTIC_MODE=1: the part finds the lookup through W-E-C4 on its own and raises the pool"
fresh_ini; setkey ENABLED 0 WEAPON_LIMITS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "WeaponLimits off, no line of it; W-E-C4 found once, the lookup's line word for word" "log_has 'WeaponLimits off' && log_lacks '\[WeaponLimits\]' && [ \$(grep -c 'Pattern W-E-C4 found: .* 1 match,' TweaksV.log) -eq 1 ] && log_line '${TAG}DIAGNOSTIC: pool size call at $RVA -> $RVA for the pool 0xBAE33CCF (W-E-C4): the pool size lookup\$'"
check "the call redirected, the pool raised, said once, no problem; the weapon components' size call left to the game's lookup" "redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && quiet && has 'size call to the lookup: 470 and 470 slots, lookups 2 of 2 as called'"

echo "== SB13 17 loads of the pool's name, more than the 16 the part looks through"
fresh_ini; out=$(game full_game.exe sbmany)
check "the problem, word for word; nothing written" "log_line '${NOT_RAISED}the game code holds C-E-H1 17 or more times, more than the 16 the part looks through (the game has changed).\$' && kept"

echo "== SB14 a call of the lookup after two loads of the pool's name: which one sizes the pool cannot be told"
fresh_ini; out=$(game full_game.exe sbtwice)
check "the problem, word for word, with both calls; nothing written" "log_line '${NOT_RAISED}the pool size lookup is called after two loads of the static bounds pool.s name, at $RVA and $RVA - which call sizes the pool cannot be told (C-E-H1).\$' && kept"

echo "== SB15 the call another mod redirected: no call of the lookup after the load of the name"
fresh_ini; out=$(game full_game.exe sbhooked); echo "      $out"
check "the problem, word for word" "log_line '${NOT_RAISED}no call of the pool size lookup at $RVA follows a load of the static bounds pool.s name within 64 bytes (C-E-H1; another mod may have redirected the call already).\$'"
check "the other mod's call left where it led, nothing written, every page its own protection" "has 'static bounds: sites as placed, size call elsewhere, the pool not made' && has 'protect: all restored (RX)'"

echo "== SB16 a second load of the pool's name, no call after it (DIAGNOSTIC_MODE=1): passed over, the call after the first redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sbname)
check "C-E-H1 found twice; the load without a call, its line word for word" "log_has 'Pattern C-E-H1 found: BA 06 E4 DF 2C -> RVA 0x[0-9A-F]*, 2 matches,' && [ \$(count '${TAG}DIAGNOSTIC: the load of the pool.s name at $RVA (C-E-H1) is followed by no call of the pool size lookup within 64 bytes') -eq 1 ]"
check "the call redirected, the pool raised, no problem" "redirected 16000 && has '$RAISED' && quiet"

echo "== SB17 the pool made with the size of another pool: passed on as the game gave it, reported once"
fresh_ini; out=$(game full_game.exe sbother)
check "the problem, word for word, once though the call comes twice" "log_line '${NOT_RAISED}the redirected call at $RVA asks for the size of the pool 0x0BADC0DE, not of the static bounds pool, 0x2CDFE406 - its size, 1000, is passed on as the game gave it.\$' && [ \$(count 'asks for the size of the pool') -eq 1 ]"
check "the other pool's default both times through the stub, no sizes line" "has 'size call to a stub (RX), pool sizes 1000 and 1000, the lookup asked 2 times, arguments as handed, last error kept' && [ \$(said) -eq 0 ]"

echo "== SB18 the lookup gives the pool -1: passed on as the game gave it, reported once"
fresh_ini; out=$(game full_game.exe sbnegative)
check "the problem, word for word, once; -1 both times through the stub" "log_line '${NOT_RAISED}the game.s configuration gives the static bounds pool the size -1, which is none - passed on as the game gave it.\$' && [ \$(count 'which is none') -eq 1 ] && has 'size call to a stub (RX), pool sizes -1 and -1'"

echo "== SB19 Windows' dynamic code policy: no call stub can be made - reported, nothing written"
fresh_ini; out=$(game full_game.exe nodyncode)
check "the stubs' line and the problem, word for word; the pool made through the game's lookup" "log_line '${TAG}The call stubs could not be made read+execute (.*) - nothing is changed.\$' && log_line '${NOT_RAISED}the game code could not be changed (details in TweaksV.log).\$' && kept"

echo "== SB20 none of the game code present (plain fake game): both patterns looked for, the first one missing reported"
fresh_ini; out=$(game plain_game.exe)
check "the harness survived" "grep -q 'harness: .* loaded' harness.out"
check "both looked for and missing, the problem names the first" "[ \$(grep -c -E 'Pattern (C-E-H1|W-E-C4) not found: ' TweaksV.log) -eq 2 ] && log_line '${NOT_RAISED}C-E-H1 was not found.\$'"

NOT_ASKED="${NOT_RAISED}the game did not ask the redirected call at $RVA for the pool.s size within 2 seconds of the session.s start - it made the pool before TweaksV started, or asks for its size another way, and the pool keeps the game.s size.\$"
AFTER_ALL='The game.s ask for the static bounds pool.s size came after all, after the check had reported it missing.'

echo "== SB21 the check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=10, DIAGNOSTIC_MODE=1, a script program placed, past the check's time): the game asks in time - timed, nothing reported"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=10
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe script wait)
asked=$(rva_of "s/.*${TAG}DIAGNOSTIC: the game.s first ask for the static bounds pool.s size came \([0-9]*\) ms after TweaksV.s start.\$/\1/p")
session=$(rva_of "s/.*${TAG}DIAGNOSTIC: the session started \([0-9]*\) ms after TweaksV.s start (the first script program placed): the static bounds check.s clock runs.\$/\1/p")
check "the test build's line and the check's line, once each; the redirect's line" "[ \$(count 'TEST BUILD: the static bounds check comes 10 seconds after the session.s start instead of 60 seconds (TWEAKSV_TEST_STATICBOUNDS_CHECK=10).') -eq 1 ] && made '10 seconds' && [ \$(count 'The static bounds check waits') -eq 1 ] && redirected 16000"
check "the session's start and the first ask timed, once each, the ask within the check's 10 seconds of the start; the sizes said once" "[ '${session:-none}' != none ] && [ '${asked:-none}' != none ] && [ \$(( ${asked:-99999} - ${session:-0} )) -lt 10000 ] && [ \$(said) -eq 1 ]"
check "past the check's time: nothing reported, the pool raised" "log_lacks 'did not ask the redirected call' && quiet && has '$RAISED'"

echo "== SB22 the pool made before TweaksV loaded, never asked (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, ERROR_POPUP=1): reported, with the part's box"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script wait)
check "the problem, word for word, once" "log_line '$NOT_ASKED' && [ \$(count 'did not ask the redirected call') -eq 1 ]"
check "shown after the start-up summary, in a box of its own" "log_has 'Showing a problem after the start-up summary (1 problem(s)).' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"
check "the pool made through the game's lookup before TweaksV, the call redirected all the same, no sizes line" "has 'size call to a stub (RX), pool sizes 12000 and 12000' && [ \$(said) -eq 0 ] && log_lacks 'after all, after the check'"

echo "== SB23 the pool made 4 seconds late, after the check of 2 seconds (test build): reported, then asked after all"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sblate script wait)
check "the problem first, then the line that the game asked after all, then the sizes, each once" "[ \$(count 'did not ask the redirected call') -eq 1 ] && [ \$(count '$AFTER_ALL') -eq 1 ] && [ \$(grep -n 'did not ask the redirected call' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'after all, after the check' TweaksV.log | cut -d: -f1) ] && sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the pool raised all the same" "has '$RAISED'"

echo "== SB24 [BLIPS] ENABLED=0 (test build, the pool made before TweaksV, a script program placed): the part makes the script programs' hook itself, and the check reports"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script wait)
check "the hook made by the part, its line under the part's tag, word for word; the program placed through TweaksV's stub; nothing of the Blips part" "log_line '${TAG}The natives. resolution call at RVA 0x[0-9A-F]* is redirected: every script program the game places passes TweaksV, before and after the game resolves its natives.\$' && has 'script: 1 placed on thread' && has 'through a TweaksV stub' && log_lacks '\[Blips\]'"
check "the check made, and its problem past its time, word for word, once" "made '2 seconds' && log_line '$NOT_ASKED' && [ \$(count 'did not ask the redirected call') -eq 1 ]"

echo "== SB25 no script program placed (test build): the session never starts, so the check never reports"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly wait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the check made and waiting, nothing reported past its time" "made '2 seconds' && log_lacks 'did not ask the redirected call' && quiet"

echo "== SB26 nothing redirected, no check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): DRY_RUN, a refused job, a failed redirect"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
for case in dryrun sbhooked nodyncode; do
	fresh_ini; options="sbearly script wait"
	if [ "$case" = dryrun ]; then setkey DRY_RUN 1; else options="$options $case"; fi
	out=$(ASI=TweaksV_test.asi game full_game.exe $options)
	check "$case: no check - neither its line nor its test build's line nor its problem past its time" "log_lacks 'static bounds check' && log_lacks 'did not ask the redirected call'"
done

echo "== SB27 the pool made with the size of another pool, a script program placed (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2): its problem once, and the check not again"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sbother script wait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the call for another pool reported once, the check made and silent past its time" "[ \$(count 'asks for the size of the pool 0x0BADC0DE') -eq 1 ] && made '2 seconds' && log_lacks 'did not ask the redirected call'"

# the line of a check left to a plugin that does the Blips part's job, word for word
LEFT="${TAG}The static bounds check is not made this session: it starts with the session, which TweaksV sees through the script programs. hook, and a plugin that does the Blips part.s job on the script programs is installed (named above) - TweaksV leaves the script programs to it.\$"

echo "== SB28 BlipLimitFix.asi in the game folder, the Blips part on in TweaksV.ini (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): no check, the line says why"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; touch BlipLimitFix.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script wait); rm -f BlipLimitFix.asi
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the line, word for word; no check, nothing reported past its time; the program placed through the game's own function" "log_line '$LEFT' && log_lacks 'static bounds check comes' && log_lacks 'did not ask the redirected call' && has 'through the game function'"
check "the call redirected all the same, no problem of the part" "redirected 16000 && quiet"

echo "== SB29 [BLIPS] ENABLED=0 with ResourceExtensions.asi in the game folder (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, the pool made before TweaksV, a script program placed): the plugin does the Blips part's job whatever TweaksV.ini says - no hook, no check, the line"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ENABLED 0 BLIPS; touch ResourceExtensions.asi; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly script wait); rm -f ResourceExtensions.asi
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the conflict check's line and the part's, word for word; no hook made, no check, nothing reported past its time; the program placed through the game's own function; the call redirected all the same, no problem of the part" "log_line '\[TweaksV\] Conflict check: .*ResourceExtensions.asi is installed; TweaksV.s Blips part is off in TweaksV.ini already.\$' && log_line '$LEFT' && log_lacks 'natives. resolution call' && log_lacks 'static bounds check comes' && log_lacks 'did not ask the redirected call' && has 'through the game function' && redirected 16000 && quiet"

# a check that cannot be made: its problem, word for word but for the reason, once; the start-up summary's box with N problems;
# no check made; no hook made by the part
NOT_CHECKED="${TAG}PROBLEM: The check that the game asks for the static bounds pool.s size is not done this session: "
not_checked() { log_line "${NOT_CHECKED}$1\$" && [ $(count 'size is not done this session: ') -eq 1 ]; }
boxed() { log_has "Showing the start-up summary ($1 problem(s))." && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:'; }
unchecked() { log_lacks 'The static bounds check waits' && log_lacks 'did not ask the redirected call'; }
unhooked() { log_lacks 'natives. resolution call at RVA 0x[0-9A-F]* is redirected' && has 'through the game function'; }

echo "== SB30 TEST_BREAK_PATTERN=Y-E-H1 with the Blips part on (test build, ERROR_POPUP=1, a script program placed): the part joins the hook Blips could not make - its problem beside Blips', in the start-up summary's box, the pool raised all the same"
fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN Y-E-H1; out=$(ASI=TweaksV_test.asi game full_game.exe script)
check "the check's problem, word for word, once, beside the Blips part's, with the same reason; the pattern looked for once" "not_checked 'Y-E-H1 was not found.' && log_line '\[Blips\] PROBLEM: blip_controller.s threshold is not raised this session: Y-E-H1 was not found.\$' && [ \$(count 'TEST BUILD: pattern Y-E-H1 is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ]"
check "both in the start-up summary's box" "boxed 2"
check "the call redirected, the pool raised; no check made, the program placed through the game's own function" "redirected 16000 && has '$RAISED' && unchecked && unhooked"

echo "== SB31 each of the hook's patterns missing, [BLIPS] ENABLED=0 (test build, ERROR_POPUP=1, a script program placed): the part, the hook's first user, cannot make it - its problem alone, the pool raised all the same"
for label in Y-E-H1 Y-E-N1 Y-E-I1; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe script)
	check "$label: the scanner's test line once, the problem word for word, once, alone in the start-up summary's box; nothing of the Blips part" "[ \$(count 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).') -eq 1 ] && not_checked '$label was not found.' && boxed 1 && log_lacks '\[Blips\]'"
	check "$label: the call redirected, the pool raised; no check made, no hook made, the program placed through the game's own function" "redirected 16000 && has '$RAISED' && unchecked && unhooked"
done

echo "== SB32 the natives' resolution call another mod redirected, [BLIPS] ENABLED=0 (the Release build): the hook's own reason, the pool raised all the same"
fresh_ini; setkey ENABLED 0 BLIPS; out=$(game full_game.exe scripthooked)
check "the problem, word for word, once; nothing of the Blips part" "not_checked 'the natives. resolution call at $RVA is not a call into the game (another mod may have hooked it already).' && log_lacks '\[Blips\]'"
check "the call redirected, the pool raised; no check made, the other mod's call left where it led" "redirected 16000 && has '$RAISED' && unchecked && has 'script: none placed, call to another mod'"

echo "== SB33 each of the check's events not made (test build, TWEAKSV_TEST_STATICBOUNDS_NO_EVENT, ERROR_POPUP=1, [BLIPS] ENABLED=0, a script program placed): the problem, the pool raised all the same, no hook made"
for event in asked session; do
	fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event ASI=TweaksV_test.asi game full_game.exe script)
	check "$event: the test build's line, word for word, once" "log_line '${TAG}TEST BUILD: the static bounds check.s $event event is not made (TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=$event).\$' && [ \$(count 'event is not made') -eq 1 ]"
	check "$event: the problem, word for word, once, in the start-up summary's box" "not_checked 'its events could not be made.' && boxed 1"
	check "$event: the call redirected, the pool raised, said once; no check made, no hook made, the program placed through the game's own function" "redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && unchecked && unhooked"
done

echo "== SB34 the switch's limits: the Release build, another value, DRY_RUN - no event left out, nothing reported"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked game full_game.exe)
check "the Release build ignores it: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=other ASI=TweaksV_test.asi game full_game.exe)
check "another value: the check made, no problem" "made '60 seconds' && quiet && log_lacks 'event is not made'"
fresh_ini; setkey DRY_RUN 1; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked ASI=TweaksV_test.asi game full_game.exe)
check "DRY_RUN: no event wanted, nothing reported" "quiet && log_lacks 'event is not made' && kept"

echo "== SB35 BlipLimitFix.asi with the asked event not made (test build): no check was to be made, logged - not reported"
fresh_ini; touch BlipLimitFix.asi; out=$(TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked ASI=TweaksV_test.asi game full_game.exe); rm -f BlipLimitFix.asi
check "the line, word for word, no problem of the part; the pool raised all the same" "log_line '$LEFT' && quiet && redirected 16000 && has '$RAISED'"

echo
echo "STATIC BOUNDS SCENARIOS: $pass passed, $fail failed"
