#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The StaticBounds part (TweaksVStaticBounds): the fake game's static bounds pool's size call found after the load of the
# pool's name, checked against the pool size lookup the CRoadBlock pool's size call leads to, and redirected, so that the
# pool the fake makes twice once TweaksV has started gets the larger of its configured size and STATICBOUNDS_SIZE, said
# once: the configuration below and above the key, the key at its limits and beyond; ENABLED=0 and both old plugins;
# DRY_RUN; DIAGNOSTIC_MODE with every site; each check failing on its own - a pattern missing, the CRoadBlock call twice or
# hooked, 17 loads of the name, a call after two of them, the call another mod redirected, no call stub - each reported
# with nothing written; a second load without a call passed over; a call for another pool and a size below 0 passed on and
# reported; the plain game; the check that the game asks (the test build's TWEAKSV_TEST_STATICBOUNDS_CHECK): asked in time,
# never asked - the pool made before TweaksV loaded - and asked after the check. In every run the sites as placed but for the
# call's rel32, the stub read+execute and every page its own protection. Fake LOCALAPPDATA: the real one is never touched.
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
# redirected N [CHECK]: the redirect's line, word for word, for STATICBOUNDS_SIZE=N and the check's time CHECK (120 seconds);
# sizes G WORDS: the sizes' line, word for word
redirected() { log_line "${TAG}The static bounds pool.s size call at $RVA is redirected: the pool gets at least $1 slots (STATICBOUNDS_SIZE) when the game makes it, more where the game.s configuration gives it more; a check ${2:-120 seconds} after TweaksV.s start reports it if the game has not asked by then.\$"; }
sizes() { log_line "${TAG}The pool size the game.s configuration gives for the static bounds, $1, $2\$"; }
said() { count 'The pool size the game.s configuration gives for the static bounds'; }
# quiet: no problem of the part; kept: the game left as it was - the pool made through its own lookup, every page its own protection
quiet() { log_lacks '\[StaticBounds\] PROBLEM'; }
kept() { has "$UNTOUCHED" && has 'protect: all restored (RX)' && log_lacks 'is redirected: the pool gets'; }
# lookup_rva KIND: the RVA the module's DIAGNOSTIC line leads to (lookup) or stands at (call), for C-E-L1 or C-E-H1
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
check "the header says off, no line of the part, neither pattern looked for" "log_has 'StaticBounds off' && log_lacks '\[StaticBounds\]' && log_lacks 'C-E-[HL]1'"
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

echo "== SB9 DIAGNOSTIC_MODE=1: both patterns found once, the lookup and the call at their RVAs, the write"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
name=$(rva_of "s/.*Pattern C-E-H1 found: BA 06 E4 DF 2C -> RVA \(0x[0-9A-F]*\), 1 match,.*/\1/p")
lookup=$(rva_of "s/.*${TAG}DIAGNOSTIC: pool size call at $RVA -> RVA \(0x[0-9A-F]*\) for the pool 0xF72CC701 (C-E-L1): the pool size lookup\$/\1/p")
call=$(rva_of "s/.*${TAG}DIAGNOSTIC: pool size call at RVA \(0x[0-9A-F]*\) -> RVA ${lookup:-none} for the pool 0x2CDFE406, its name loaded at RVA ${name:-none} (C-E-H1)\$/\1/p")
check "C-E-H1 and C-E-L1 found exactly once each" "[ \$(grep -c -E 'Pattern C-E-(H1|L1) found: .* 1 match,' TweaksV.log) -eq 2 ]"
check "the CRoadBlock pool's size call and the lookup it leads to, word for word" "[ -n '$lookup' ]"
check "the static bounds pool's size call, word for word: to the same lookup, 11 bytes into C-E-H1's match" "[ -n '$call' ] && [ \$(( $call - ${name:-0} )) -eq 11 ]"
check "the write's line: the call's E8 kept, its rel32 to the stub" "log_line '${TAG}DIAGNOSTIC: static bounds pool.s size call at RVA ${call:-none}: E8 [0-9A-F ]* -> E8 [0-9A-F ]*\$'"
check "no load of the name without a call; the redirect and the sizes as without DIAGNOSTIC_MODE; no problem" "log_lacks 'is followed by no call' && redirected 16000 && has '$RAISED' && [ \$(said) -eq 1 ] && quiet"

echo "== SB10 TEST_BREAK_PATTERN for each pattern (test build): reported by its label, nothing written"
for label in C-E-H1 C-E-L1; do
	fresh_ini; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the scanner's test line and the problem, word for word" "log_has 'TEST BUILD: pattern $label is treated as not found (TEST_BREAK_PATTERN).' && log_line '${NOT_RAISED}$label was not found.\$'"
	check "$label: nothing written - the pool made through the game's lookup" "kept"
done

echo "== SB11 a second CRoadBlock pool's size call: C-E-L1 matches twice, not used"
fresh_ini; out=$(game full_game.exe sbroadtwice)
check "the scanner's line and the problem, word for word; nothing written" "log_has 'Pattern C-E-L1 matches 2 or more times - not used (the site must be unique).' && log_line '${NOT_RAISED}C-E-L1 was not found.\$' && kept"

echo "== SB12 the CRoadBlock pool's size call another mod hooked: it leads outside the game"
fresh_ini; out=$(game full_game.exe sbroadhooked)
check "the problem, word for word; nothing written" "log_line '${NOT_RAISED}the CRoadBlock pool.s size call at $RVA is not a call into the game (another mod may have hooked it already) (C-E-L1).\$' && kept"

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
check "both looked for and missing, the problem names the first" "[ \$(grep -c 'Pattern C-E-[HL]1 not found: ' TweaksV.log) -eq 2 ] && log_line '${NOT_RAISED}C-E-H1 was not found.\$'"

NOT_ASKED="${NOT_RAISED}the game did not ask the redirected call at $RVA for the pool.s size in the first 2 seconds after TweaksV started - it made the pool before TweaksV started, or asks for its size another way, and the pool keeps the game.s size.\$"
AFTER_ALL='The game.s ask for the static bounds pool.s size came after all, after the check had reported it missing.'

echo "== SB21 the check (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=10, DIAGNOSTIC_MODE=1, past the check's time): the game asks in time - timed, nothing reported"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=10
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
asked=$(rva_of "s/.*${TAG}DIAGNOSTIC: the game.s first ask for the static bounds pool.s size came \([0-9]*\) ms after TweaksV.s start.\$/\1/p")
check "the test build's line, once; the redirect's line with the check's 10 seconds" "[ \$(count 'TEST BUILD: the static bounds check comes 10 seconds after TweaksV.s start instead of 120 seconds (TWEAKSV_TEST_STATICBOUNDS_CHECK=10).') -eq 1 ] && redirected 16000 '10 seconds'"
check "the first ask timed, once, within the check's 10 seconds; the sizes said once" "[ \$(count 'first ask for the static bounds pool.s size came') -eq 1 ] && [ '${asked:-none}' != none ] && [ ${asked:-99999} -lt 10000 ] && [ \$(said) -eq 1 ]"
check "past the check's time: nothing reported, the pool raised" "log_lacks 'did not ask the redirected call' && quiet && has '$RAISED'"

echo "== SB22 the pool made before TweaksV loaded, never asked (test build, TWEAKSV_TEST_STATICBOUNDS_CHECK=2, ERROR_POPUP=1): reported, with the part's box"
export TWEAKSV_TEST_STATICBOUNDS_CHECK=2
fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe sbearly wait)
check "the problem, word for word, once" "log_line '$NOT_ASKED' && [ \$(count 'did not ask the redirected call') -eq 1 ]"
check "shown after the start-up summary, in a box of its own" "log_has 'Showing a problem after the start-up summary (1 problem(s)).' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"
check "the pool made through the game's lookup before TweaksV, the call redirected all the same, no sizes line" "has 'size call to a stub (RX), pool sizes 12000 and 12000' && [ \$(said) -eq 0 ] && log_lacks 'after all, after the check'"

echo "== SB23 the pool made 4 seconds late, after the check of 2 seconds (test build): reported, then asked after all"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sblate wait)
unset TWEAKSV_TEST_STATICBOUNDS_CHECK
check "the problem first, then the line that the game asked after all, then the sizes, each once" "[ \$(count 'did not ask the redirected call') -eq 1 ] && [ \$(count '$AFTER_ALL') -eq 1 ] && [ \$(grep -n 'did not ask the redirected call' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'after all, after the check' TweaksV.log | cut -d: -f1) ] && sizes 12000 'is raised to 16000 (STATICBOUNDS_SIZE).' && [ \$(said) -eq 1 ]"
check "the pool raised all the same" "has '$RAISED'"

echo
echo "STATIC BOUNDS SCENARIOS: $pass passed, $fail failed"
