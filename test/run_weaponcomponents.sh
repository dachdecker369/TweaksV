#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The weapon components (TweaksVWeaponComponents, the WeaponLimits part's second half), on the fake game's list of every
# weapon component definition - its rebuild, its sort, its update of the count, the pool's size call and the list's 15
# other leas: by default the list moved - its 17 loads to a list of 16854 entries that holds the game's own entries, the
# count left where it is - and the pool's size call redirected to a read+execute stub that gives the pool the room and
# 470, the game's lookup called with the game's arguments, its line once, every other byte as placed and every page its
# own protection; the part off; DRY_RUN; WEAPON_COMPONENTS_SIZE raised and past its range; the game's configuration naming
# the pool's size, below the list's and above it; DIAGNOSTIC_MODE with every site, write and status line word for word, the
# measurement announced and its summary 10 seconds in, again once a pack's file loads, past the game's own 470, the list
# full, the list overflowed, the game's own list in use with DRY_RUN; a load the game puts back, reported once with its
# box; each check failing on its own - TEST_BREAK_PATTERN, the list another mod moved, a lea fewer, one more, one with
# another prefix, the count's references disagreeing, the count not a whole number of entries after the list, before it,
# outside the game, too far for a plausible size, the pool's default 0 and the size call another mod hooked - each a
# problem word for word with nothing written; the plain game; and the guard on the loader of a weaponcomponents.meta - by default
# hooked, its line; with the test build's TWEAKSV_TEST_GUARD_ROOM=1 or a smaller room, the files that come once the room is used
# up refused - each a line, the first a problem - and their callbacks destroyed; the names refused kept through a base loader's
# run, 1,024 of them; a blob that does not add up switching it off; W-E-C5 missing and the loader another mod hooked, the guard
# not made and the rest applied. The weapon infos half logs and reports under the same tag,
# so every check matches the weapon components' lines and problems by their wording. Fake LOCALAPPDATA: the real one is
# never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

RVA='RVA 0x[0-9A-F]*'
TAG='\[WeaponLimits\] '
DIAG="${TAG}DIAGNOSTIC: "
NOT_RAISED="${TAG}PROBLEM: The weapon components limit is not raised this session: "
# The report's field: the part's writes - the 17 loads to the moved list, which holds the game's entries and zeros up to
# 16854, read/write; every other byte and the game's data as placed; the size call to TweaksV's read+execute stub - or none
# of them, the game as it was, the size call to the game's lookup; the report calls what the size call leads to twice
APPLIED='wc: loads 17 moved 0 game 0 other, moved list RW, copied then 0, other bytes as placed, data as placed, size call to a stub, RX: '
GAME='wc: loads 0 moved 17 game 0 other, other bytes as placed, data as placed, size call to the lookup: 470 and 470 slots, lookups 2 of 2 as called'
MOVED_LINE="${TAG}The weapon component list moved and its limit raised from 470 to 1024 entries.\$"
REDIRECTED="${TAG}The pool size call at $RVA is redirected: the weapon components. pool gets 1494 slots, the room of 1024 and 470 for the last file let in.\$"
HOOKED="${TAG}The weapon components. loader at $RVA is hooked: a weaponcomponents.meta file that comes once"
GUARD_PROBLEM="${TAG}PROBLEM: Weapon component files are refused this session: dlcpacks:/weaponpack"
# file N VERDICT: the guard's line of pack N's file (two digits), its verdict ("let in", "refused" or "refused, as before"), and how many
file() { count "eapon components file dlcpacks:/weaponpack$1/common/data/weaponcomponents.meta: [0-9]* definitions loaded, the room [0-9]* - $2\.\$"; }
ANNOUNCED="${DIAG}the weapon component definitions in the list and the weaponcomponents.meta files are logged after 10 seconds, then every minute while the numbers change, and the list.s 17 loads are checked at each reading to lead to the moved list still.\$"
# replaced G N: the pool's line, word for word; summary N F REST: the summary of N definitions from F files, word for word
# (REST: what follows the game's 470, from ", this one" or " and is in use" on); summaries: how many
replaced() { log_line "${TAG}The pool size the game.s configuration gives for the weapon components, $1, is replaced by $2.\$"; }
summary() { log_line "${DIAG}$1 weapon component definitions from $2 weaponcomponents.meta files (the game.s own list holds 470$3\$"; }
summaries() { count 'weapon component definitions from'; }
# target KIND LABEL: the RVA the part's line of that kind ("reference", "count reference", ...) leads to, for LABEL;
# follower: the RVA of the count, from the core's line of the variable that follows the list
target() { tr -d '\r' < TweaksV.log | sed -n "s/.*${DIAG}$1 at RVA 0x[0-9A-F]* -> RVA \(0x[0-9A-F]*\) ($2)\$/\1/p"; }
follower() { tr -d '\r' < TweaksV.log | sed -n "s/.*${DIAG}limit variable at RVA \(0x[0-9A-F]*\) lies 3760 bytes after the weapon component list (W-E-C1) -> game size 470\$/\1/p"; }
# quiet: no problem of the weapon components - neither the part's nor its setting's; untouched: nothing of the part written -
# none of the 17 loads moved, every other byte and the data as placed, every page its own protection
quiet() { log_lacks 'PROBLEM: The weapon components limit' && log_lacks 'PROBLEM: TweaksV.ini: \[WEAPON_LIMITS\] WEAPON_COMPONENTS_SIZE' && log_lacks 'PROBLEM: The weapon components. guard' && log_lacks 'PROBLEM: Weapon component files are refused'; }
untouched() { has 'wc: loads 0 moved' && has 'other bytes as placed, data as placed' && has 'protect: all restored (RX)' && log_lacks 'weapon component list moved'; }
# refused REASON: the weapon components' one problem, word for word - its reason after the job's words - and nothing written
refused() { [ "$(count "${TAG}PROBLEM: The weapon components limit")" -eq 1 ] && log_line "${NOT_RAISED}$1\$" && untouched; }

echo "== W1 the defaults: the list moved, the pool's size call redirected, nothing else written"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the 17 loads lead to the moved list, read/write, which holds the game's entries and zeros up to 16854 entries; every other byte and the game's data as placed" "has '$APPLIED'"
check "the size call leads to TweaksV's read+execute stub: the room and 470 slots at each call, the game's lookup called each time with the game's arguments" "has 'size call to a stub, RX: 1494 and 1494 slots, lookups 2 of 2 as called'"
check "the status lines word for word, the pool's line once for its two calls" "log_line '$MOVED_LINE' && log_line '$REDIRECTED' && replaced 470 1494 && [ \$(count 'for the weapon components, 470, is replaced by 1494.') -eq 1 ]"
check "the guard: the loader hooked at its head, its line word for word" "has 'wc loader hooked, files mounted 0' && log_line '$HOOKED 1024 definitions are loaded is refused.\$'"
check "every page back to its own protection, no line of DIAGNOSTIC_MODE, no problem" "has 'protect: all restored (RX)' && log_lacks '${TAG}DIAGNOSTIC' && quiet"

echo "== W2 [WEAPON_LIMITS] ENABLED=0, with DIAGNOSTIC_MODE=1 and [STATICBOUNDS] ENABLED=0, which looks for W-E-C4 too: nothing looked for, nothing written"
fresh_ini; setkey ENABLED 0 WEAPON_LIMITS; setkey ENABLED 0 STATICBOUNDS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "the part off in the header, none of its patterns looked for, no line of the weapon components" "log_has 'WeaponLimits off' && log_lacks 'W-E-C' && log_lacks -i 'weapon component'"
check "the game as it was - the loads, the bytes, the data, the size call to the game's lookup - every page its own protection, no problem" "has '$GAME' && has 'protect: all restored (RX)' && quiet"

echo "== W3 DRY_RUN=1: everything found and checked, the line of what it would do, nothing written"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the dry run's line, word for word" "log_line '${TAG}DRY_RUN: would move the weapon component list (17 references) and raise its limit from 470 to 1024 and redirect the pool size call at $RVA\.\$'"
check "the game as it was, no status line, no problem" "has '$GAME' && untouched && log_lacks 'is redirected: the weapon components' && quiet"
check "the guard's dry run, word for word: the loader as placed" "log_line '${TAG}DRY_RUN: would hook the weapon components. loader at $RVA to refuse the files that come once 1024 definitions are loaded.\$' && has 'wc loader as placed' && log_lacks 'weapon components. loader at $RVA is hooked'"

echo "== W4 DIAGNOSTIC_MODE=1 for 12 seconds: every site and write logged, the measurement announced, its summary 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
list=$(target reference W-E-C1); counted=$(follower); files=$(target "files' count reference" W-E-C1)
check "W-E-C1 to W-E-C4 found exactly once each" "[ \$(grep -c -E 'Pattern W-E-C[1-4] found: .* 1 match,' TweaksV.log) -eq 4 ]"
check "the rebuild's and the sort's leas lead to one list, 470 entries of 8 bytes" "[ -n '$list' ] && [ \"\$(target reference W-E-C2)\" = '$list' ] && log_line '${DIAG}the weapon component list is at RVA $list - 470 entries of 8 bytes\$'"
check "the count the rebuild resets follows the list by 3760 bytes, 470 entries (the core's follower); the sort's read and the update's read and write lead to it" "[ -n '$counted' ] && [ \$(( $counted - $list )) -eq 3760 ] && [ \$(count '${DIAG}count reference at ') -eq 3 ] && [ \"\$(target 'count reference' 'W-E-C[23]' | sort -u)\" = '$counted' ]"
check "the files' count 16 bytes after the count, as the game's lies; the entry size 8" "[ \$(( $files - $counted )) -eq 16 ] && log_line '${DIAG}entry size at $RVA holds 8 (W-E-C2)\$'"
check "the pool's size call, its name and its default, word for word" "log_line '${DIAG}pool size call at $RVA -> $RVA for the pool 0xBAE33CCF, its default at $RVA holds 470, the game.s configuration at $RVA (W-E-C4)\$'"
check "every lea of the list logged: 17, each leading to it" "[ \$(grep -c '${DIAG}reference at $RVA -> RVA $list (lea scan)' TweaksV.log) -eq 17 ]"
check "the 18 writes logged: the 17 loads' rel32 and the size call" "[ \$(count '${DIAG}weapon component list reference at $RVA: ') -eq 17 ] && [ \$(count '${DIAG}pool size call at $RVA: E8 ') -eq 1 ]"
check "the status lines and the announcement, word for word, in that order" "log_line '$MOVED_LINE' && log_line '$REDIRECTED' && log_line '$ANNOUNCED' && [ \$(grep -n 'is redirected: the weapon components' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'loads are checked at each reading' TweaksV.log | cut -d: -f1) ]"
check "the summary 10 s in, word for word: 412 definitions from 23 files against the game's own 470 and the part's 1024, once" "summary 412 23 ', this one 1024).' && [ \$(summaries) -eq 1 ]"
check "the part applied, every page its own protection, no problem" "has '$APPLIED' && has 'protect: all restored (RX)' && quiet"

echo "== W5 the summary again when the numbers change: a pack's file loads after the first (test build, TWEAKSV_TEST_SUMMARY_MS=1000)"
export TWEAKSV_TEST_SUMMARY_MS=1000
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe wcpack wait)
unset TWEAKSV_TEST_SUMMARY_MS
check "the test build's line, once" "[ \$(count 'TEST BUILD: the weapon components summary comes every 1000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=1000).') -eq 1 ]"
check "412 from 23 first, then 430 from 24, and nothing more while the numbers stay" "summary 412 23 ', this one 1024).' && summary 430 24 ', this one 1024).' && [ \$(summaries) -eq 2 ] && [ \$(grep -n 'from 23 weaponcomponents' TweaksV.log | cut -d: -f1) -lt \$(grep -n 'from 24 weaponcomponents' TweaksV.log | cut -d: -f1) ]"
check "the part applied, no problem" "has '$APPLIED' && quiet"

echo "== W6 556 definitions from 40 files, past the game's own 470, as seven weapon packs bring them"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wcmore wait)
check "the summary, word for word: the game's own list would have overflowed" "summary 556 40 ', this one 1024) - the game.s own list would have overflowed by 86.'"
check "the part applied, no problem" "has '$APPLIED' && quiet"

echo "== W7 WEAPON_COMPONENTS_SIZE=556 with those 556 definitions: the list and the pool full"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey WEAPON_COMPONENTS_SIZE 556; out=$(game full_game.exe wcmore wait)
check "raised to 556, the pool given 556 and 470 slots, word for word" "log_line '${TAG}The weapon component list moved and its limit raised from 470 to 556 entries.\$' && log_line '${TAG}The pool size call at $RVA is redirected: the weapon components. pool gets 1026 slots, the room of 556 and 470 for the last file let in.\$' && replaced 470 1026 && has 'size call to a stub, RX: 1026 and 1026 slots'"
check "the summary, word for word: past the game's own, and the room used up" "summary 556 40 ', this one 556) - the game.s own list would have overflowed by 86 - the room is used up: the guard refuses every further file.'"
check "nothing else, no problem" "has '$APPLIED' && quiet"

echo "== W8 DRY_RUN=1 with DIAGNOSTIC_MODE=1, the game's own list full (470 from 25): measured in place"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe wcfull wait)
check "every site, load and line as without DRY_RUN, the dry run's line in the place of the writes" "[ \$(grep -c -E 'Pattern W-E-C[1-4] found: .* 1 match,' TweaksV.log) -eq 4 ] && [ \$(count '${DIAG}count reference at ') -eq 3 ] && log_has 'DRY_RUN: would move the weapon component list (17 references)' && [ \$(count '${DIAG}weapon component list reference at $RVA: ') -eq 0 ]"
check "the announcement and the summary, word for word: the game's own list in use, full" "log_line '${DIAG}the weapon component definitions in the list and the weaponcomponents.meta files are logged after 10 seconds, then every minute while the numbers change (DRY_RUN: the game.s own list is in use).\$' && summary 470 25 ' and is in use) - it is full: one more definition crashes the game while it loads.'"
check "the game as it was, no problem" "has '$GAME' && quiet"

echo "== W9 the list overflowed - the 471st definition's pointer over the count: with DRY_RUN in the game's own list, without it past the moved one"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe wcover wait)
check "DRY_RUN: the summary, word for word - no count within the game's own 470; nothing written" "log_line '${DIAG}the weapon component list.s count reads -1547607936, from 25 weaponcomponents.meta files - outside the game.s own list of 470: the list has overflowed, a definition past its end written over the count and the memory after it, and the game is likely to crash.\$' && has '$GAME' && quiet"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wcover wait)
check "the part on: the summary, word for word - no count within the moved list's 16854; the part applied" "log_line '${DIAG}the weapon component list.s count reads -1547607936, from 25 weaponcomponents.meta files - outside the moved list of 16854: the game.s data is damaged, and the game is likely to crash.\$' && has '$APPLIED' && quiet"

echo "== W10 the game's configuration names the pool's size: 600, then 20000 - replaced by the list's size either way"
fresh_ini; out=$(game full_game.exe wcconfig)
check "600 replaced by 1494, word for word; the stub's 1494 slots, the lookup called each time" "replaced 600 1494 && has 'size call to a stub, RX: 1494 and 1494 slots, lookups 2 of 2 as called' && quiet"
fresh_ini; out=$(game full_game.exe wcconfigbig)
check "20000, more than the room and 470, replaced by 1494 exactly, word for word" "replaced 20000 1494 && has 'size call to a stub, RX: 1494 and 1494 slots, lookups 2 of 2 as called' && quiet"

echo "== W11 WEAPON_COMPONENTS_SIZE=20000, past its range: 16384, reported"
fresh_ini; setkey WEAPON_COMPONENTS_SIZE 20000; out=$(game full_game.exe)
check "the setting's problem, word for word, once; none of the part's" "log_line '\[TweaksV\] PROBLEM: TweaksV.ini: \[WEAPON_LIMITS\] WEAPON_COMPONENTS_SIZE=20000 is outside 470..16384 - using 16384.\$' && [ \$(count 'PROBLEM: TweaksV.ini') -eq 1 ] && log_lacks 'PROBLEM: The weapon components limit'"
check "raised to 16384, the pool given 16384 and 470 slots, the moved list's 16854 entries" "log_line '${TAG}The weapon component list moved and its limit raised from 470 to 16384 entries.\$' && replaced 470 16854 && has 'size call to a stub, RX: 16854 and 16854 slots' && has '$APPLIED'"

echo "== W12 a load the game puts back, after the first summary (test build, ERROR_POPUP=1, TWEAKSV_TEST_SUMMARY_MS=1000): reported once, in a box"
export TWEAKSV_TEST_SUMMARY_MS=1000
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe wcputback)
unset TWEAKSV_TEST_SUMMARY_MS
check "the load leads to the game's own list again: 16 loads to the moved list, 1 to the game's" "has 'wc: loads 16 moved 1 game 0 other, moved list RW, copied then 0, other bytes as placed'"
check "the problem, word for word, once" "[ \$(count 'is not kept this session') -eq 1 ] && log_line '${TAG}PROBLEM: The weapon components limit is not kept this session: the load of the weapon component list at $RVA leads to the game.s own list at RVA 0x[0-9A-F]* again, not to the moved list at 0x[0-9A-F]* (outside the game) - the game has put it back, and a search through it reads the game.s own list, empty since the move: the game is likely to crash.\$'"
check "the summary again, with the load put back, word for word" "summary 412 23 ', this one 1024).' && summary 412 23 ', this one 1024) - a load of the list has changed, reported above.' && [ \$(summaries) -eq 2 ]"
check "the box of a part that stopped working after the start-up" "log_has 'Showing a problem after the start-up summary (' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"

echo "== W13 TEST_BREAK_PATTERN=W-E-C1 (test build, ERROR_POPUP=1): the problem in the start-up summary, nothing written"
fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN W-E-C1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the scanner's line and the problem, word for word" "log_has 'TEST BUILD: pattern W-E-C1 is treated as not found (TEST_BREAK_PATTERN).' && refused 'W-E-C1 was not found.'"
check "the start-up summary's box" "log_has 'Showing the start-up summary (' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:'"
check "the size call to the game's lookup" "has '$GAME'"

echo "== W14 TEST_BREAK_PATTERN=W-E-C4 (test build): the last of the four, refused by its label"
fresh_ini; addkey TEST_BREAK_PATTERN W-E-C4; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the problem, word for word; the other three found; nothing written" "refused 'W-E-C4 was not found.' && has '$GAME'"

echo "== W15 the list another mod moved: its leas outside the game"
fresh_ini; out=$(game full_game.exe wcmoved)
check "the problem, word for word" "refused 'the weapon component list is not inside the game: 0x[0-9A-F]* (outside the game) (another mod may have moved it already).'"
check "the loads where the other mod put them, the size call to the game's lookup" "has 'wc: loads 0 moved 0 game 17 other' && has 'size call to the lookup: 470 and 470 slots'"

echo "== W16 one lea of the list leads elsewhere in the game: only 16 load it"
fresh_ini; out=$(game full_game.exe wcsplit)
check "the problem, word for word; nothing written" "refused 'the game code loads the weapon component list in 16 places, not the expected 17 (another mod may have moved some of them, or the game has changed).' && has 'wc: loads 0 moved 16 game 1 other' && has 'size call to the lookup'"

echo "== W17 an 18th lea loads the list (a changed game)"
fresh_ini; out=$(game full_game.exe wcextra)
check "the problem, word for word; nothing written" "refused 'the game code loads the weapon component list in 18 or more places, not the expected 17 (another mod may have moved some of them, or the game has changed).' && has '$GAME'"

echo "== W18 one lea of the list with a 49 prefix, which the game's never have"
fresh_ini; out=$(game full_game.exe wcprefix)
check "the problem, word for word, with the lea's bytes; nothing written" "refused 'the load of the weapon component list at $RVA is not a \"lea r64, \[rip+disp32\]\" with a 48 or 4C prefix: 49 8D 05 [0-9A-F ]*\.' && has '$GAME'"

echo "== W19 the update's store of the count leads elsewhere: the count's references disagree"
fresh_ini; out=$(game full_game.exe wccount)
check "the problem, word for word, naming the update; nothing written" "refused 'the references to the weapon component list.s count do not all point to one count: $RVA and $RVA (W-E-C3).' && has '$GAME'"

echo "== W20 the core's follower: the count 4 bytes further - not a whole number of 8-byte entries after the list"
fresh_ini; out=$(game full_game.exe wcgap)
check "the problem, word for word; nothing written" "refused 'the variable at $RVA does not lie a whole number of 8-byte entries after the weapon component list at $RVA (W-E-C1).' && has '$GAME'"

echo "== W21 the core's follower: the count 8 bytes before the list"
fresh_ini; out=$(game full_game.exe wcbefore)
check "the problem, word for word; nothing written" "refused 'the variable at $RVA does not lie a whole number of 8-byte entries after the weapon component list at $RVA (W-E-C1).' && has '$GAME'"

echo "== W22 the core's follower: the count outside the game"
fresh_ini; out=$(game full_game.exe wcoutside)
check "the problem, word for word; nothing written" "refused 'the variable that follows the weapon component list is not inside the game: 0x[0-9A-F]* (outside the game) (W-E-C1).' && has '$GAME'"

echo "== W23 the core's follower: the count 20000 entries after the list, no plausible size"
fresh_ini; out=$(game full_game.exe wcfar)
check "the problem, word for word; nothing written" "refused 'the game.s own weapon component list size, 20000, is not plausible.' && has '$GAME'"

echo "== W24 the pool's default holds 0"
fresh_ini; out=$(game full_game.exe wcdefault)
check "the problem, word for word; nothing written, the size call to the game's lookup" "refused 'the pool.s default size at $RVA holds 0, which is no plausible size (W-E-C4).' && has 'size call to the lookup: 0 and 0 slots'"

echo "== W25 the pool size call another mod hooked: it leads outside the game"
fresh_ini; out=$(game full_game.exe wchooked); echo "      $out"
check "the problem, word for word" "refused 'the weapon components. pool size call at $RVA is not a call into the game (another mod may have hooked it already) (W-E-C4).'"
check "the call left where the other mod's hook sent it, not called" "has 'size call elsewhere'"

echo "== W26 none of the game code present (plain fake game): reported, the first pattern named"
fresh_ini; out=$(game plain_game.exe)
check "the harness survived" "grep -q 'harness: .* loaded' harness.out"
check "W-E-C1 looked for and missing, the problem word for word" "log_has 'Pattern W-E-C1 not found: ' && log_line '${NOT_RAISED}W-E-C1 was not found.\$'"

echo "== W27 the guard, test build with TWEAKSV_TEST_GUARD_ROOM=1 and DIAGNOSTIC_MODE=1: 11 files of packs, 91 definitions, past the game's own 470"
export TWEAKSV_TEST_GUARD_ROOM=1
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe wcmount)
unset TWEAKSV_TEST_GUARD_ROOM
check "the test build's line once, the guard's room the game's own 470, word for word" "[ \$(count 'TEST BUILD: the weapon component guard.s room is the game.s own 470 instead of 1024 (TWEAKSV_TEST_GUARD_ROOM=1).') -eq 1 ] && log_line '$HOOKED 470 definitions are loaded is refused.\$'"
check "the loader's head, word for word" "log_line '${DIAG}the weapon components. loader at $RVA (W-E-C5 - 0x13): no unwind data, byte before it 90\$'"
check "files 01 to 07 let in while fewer than 470 are loaded, 08 to 11 refused, a line each" "[ \$(file 0[1-7] 'let in') -eq 7 ] && [ \$(file '\(08\|09\|10\|11\)' refused) -eq 4 ] && [ \$(count 'definitions loaded, the room') -eq 11 ]"
check "the first refused file's problem, word for word, once" "[ \$(count 'Weapon component files are refused this session') -eq 1 ] && log_line '${GUARD_PROBLEM}08/common/data/weaponcomponents.meta, the first, came once 472 definitions were loaded, the room being 470 - the weapon components of the files refused are missing in the game. Raise WEAPON_COMPONENTS_SIZE in TweaksV.ini (at most 16384); the log names every file refused.\$'"
check "the part's box for the first file refused (the test build's line in its place)" "log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV'"
check "7 files loaded, every callback destroyed once - the refused ones by the guard - the list's count 472; the game's last error kept by each refusal" "has 'files mounted 11, loaded 7, destroyed 11 (0 on the heap), list 472, last error kept 4'"

echo "== W28 WEAPON_COMPONENTS_SIZE=480: the room the key gives, the 9th file the first refused"
fresh_ini; setkey WEAPON_COMPONENTS_SIZE 480; out=$(game full_game.exe wcmount)
check "08 let in at 472, 09 to 11 refused, each a line; the problem names 09" "[ \$(file '\(09\|10\|11\)' refused) -eq 3 ] && log_line '${GUARD_PROBLEM}09/common/data/weaponcomponents.meta, the first, came once 480 definitions were loaded, the room being 480 - ' && has 'files mounted 11, loaded 8, destroyed 11 (0 on the heap), list 480'"
check "no line for the files let in without DIAGNOSTIC_MODE" "[ \$(count 'definitions loaded, the room') -eq 3 ]"

echo "== W29 a base loader's run after the packs (test build, TWEAKSV_TEST_GUARD_ROOM=1): the refused names refused again, below the room"
export TWEAKSV_TEST_GUARD_ROOM=1
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe wcmount wcbase)
unset TWEAKSV_TEST_GUARD_ROOM
check "the 4 refused before, refused again by their names while fewer than 470 are loaded; the 7 let in again" "[ \$(file '\(08\|09\|10\|11\)' 'refused, as before') -eq 4 ] && [ \$(count 'definitions loaded, the room 470 - refused, as before') -eq 4 ] && has 'files mounted 22, loaded 14, destroyed 22 (0 on the heap), list 60'"
check "the problem once only" "[ \$(count 'Weapon component files are refused this session') -eq 1 ]"

echo "== W30 1,100 files refused (test build, TWEAKSV_TEST_GUARD_ROOM=1): 1,024 names kept, then the count alone"
export TWEAKSV_TEST_GUARD_ROOM=1
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe wcmany)
unset TWEAKSV_TEST_GUARD_ROOM
check "every one refused, a line each; the names' line once, word for word" "[ \$(count 'definitions loaded, the room 470 - refused\.') -eq 1100 ] && [ \$(count 'keeps 1024 refused names at most') -eq 1 ] && log_line '${TAG}The weapon components. guard keeps 1024 refused names at most: from now on a file is refused by the definitions loaded alone.\$'"
check "the big file let in, the 1,100 not loaded and their callbacks destroyed; the problem once" "has 'files mounted 1101, loaded 1, destroyed 1101 (0 on the heap), list 472' && [ \$(count 'Weapon component files are refused this session') -eq 1 ]"

echo "== W31 a blob whose count does not add up: the guard off, the files let through"
fresh_ini; out=$(game full_game.exe wcmount wcbadblob)
check "the problem, word for word, once" "[ \$(count 'guard is off this session') -eq 1 ] && log_line '${TAG}PROBLEM: The weapon components. guard is off this session: blob 0 holds 500 definitions, outside 0..470 - dlcpacks:/weaponpack01/common/data/weaponcomponents.meta and the files after it go to the game unchecked, and past 470 definitions above the room the game crashes while it loads.\$'"
check "all 11 loaded, none refused" "has 'files mounted 11, loaded 11, destroyed 11 (0 on the heap)' && log_lacks 'definitions loaded, the room'"

echo "== W32 TEST_BREAK_PATTERN=W-E-C5 (test build): the guard not made, the list moved and the pool's size all the same"
fresh_ini; addkey TEST_BREAK_PATTERN W-E-C5; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the problem, word for word; the rest applied" "log_line '${TAG}PROBLEM: The weapon components. guard is not made this session: W-E-C5 was not found.\$' && has '$APPLIED' && log_line '$REDIRECTED' && has 'wc loader as placed'"

echo "== W33 the loader another mod hooked: the guard not made, the rest applied"
fresh_ini; out=$(game full_game.exe wcloaderhooked)
check "the problem, word for word; the rest applied" "log_line '${TAG}PROBLEM: The weapon components. guard is not made this session: the loader at $RVA starts with a jump (another mod may have hooked it already).\$' && has '$APPLIED' && log_lacks 'weapon components. loader at $RVA is hooked'"

echo "== W34 the store's page - the list and its count - unreadable while TweaksV starts: the count cannot be read"
fresh_ini; out=$(game full_game.exe wccountnoaccess)
check "the problem, word for word; nothing written" "refused 'the weapon component list.s count at $RVA cannot be read.' && has '$GAME'"

echo "== W35 the leas' page unreadable a while after the first summary (test build, ERROR_POPUP=1, TWEAKSV_TEST_SUMMARY_MS=1000)"
export TWEAKSV_TEST_SUMMARY_MS=1000
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe wcloadnoaccess)
unset TWEAKSV_TEST_SUMMARY_MS
check "the problem, word for word, once, and its box" "[ \$(count 'may not be kept this session') -eq 1 ] && log_line '${TAG}PROBLEM: The weapon components limit may not be kept this session: the load of the weapon component list at $RVA cannot be read any more, so whether it still leads to the moved list at 0x[0-9A-F]* (outside the game) cannot be told.\$' && log_has 'Showing a problem after the start-up summary ('"
check "the page readable again, the 17 loads to the moved list" "has '$APPLIED' && has 'protect: all restored (RX)'"

echo "== W36 the byte before the loader's head 48: no function start, the guard not made, the rest applied"
fresh_ini; out=$(game full_game.exe wcloadernotstart)
check "the problem, word for word; the rest applied" "log_line '${TAG}PROBLEM: The weapon components. guard is not made this session: the loader at $RVA (W-E-C5 - 0x13) is not a function start (no unwind data, byte before it 48).\$' && has '$APPLIED' && log_lacks 'weapon components. loader at $RVA is hooked'"

echo "== W37 the loader's head returns at once (C3): MinHook cannot hook it, the guard not made, the rest applied"
fresh_ini; out=$(game full_game.exe wcloaderret)
check "the problem, word for word; the rest applied" "log_line '${TAG}PROBLEM: The weapon components. guard is not made this session: the loader at $RVA could not be hooked (details in TweaksV.log).\$' && has '$APPLIED'"

echo
echo "WEAPON COMPONENTS SCENARIOS: $pass passed, $fail failed"
