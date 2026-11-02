#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The weapon infos, the WeaponLimits part's half for the weapons.meta files (TweaksVWeaponInfos): the full fake game's
# initializer of the weapon info blob array, its allocation and its loader - real code, called through its start with a
# data file entry as the game calls its loader - the array in the fake's data empty while TweaksV starts and set up by
# the fake afterwards, as the game's initializer does, with as many blobs as the immediates say. With DIAGNOSTIC_MODE
# every site's line at its place, both immediates written in one go and the loader hooked, a line per file, the summary
# 10 seconds in; the defaults with the raise's line and the guard's alone; the room full - the files past it refused, the
# game's loader not called for them, reported once in a box of its own and later ones in a line; the guards' test switch
# of a test build, which gives the guard the game's own room, the report naming it, and the report at the key's maximum; a name that cannot be read; the lines per file past their
# cap; DRY_RUN, with the would lines and nothing written or hooked; packs mounted while the game runs; an array set up
# before TweaksV starts, immediates another mod raised before TweaksV or an array another mod gave less room after it;
# Windows' dynamic code policy; the part off; a pattern missing, a pattern the game holds twice, each site not as expected and
# an address outside the game, each a problem of the start-up summary with nothing written; the plain game. And in every run nothing written but
# the two immediates and the loader's first bytes, every page its own protection. Fake LOCALAPPDATA: the real one is
# never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=74
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

W='\[WeaponLimits\]'
D="$W DIAGNOSTIC:"
# a summary line, not the announcement (. for an apostrophe, as in every pattern below)
SUMMARY='weapons.meta files loaded (the game.s own array holds'
RAISED="$W The weapon info blob limit is raised from 160 to 1024 weapons.meta files.\$"
# the 9th file of "wifew", 140 characters long, as the guard reads it: its first 127
LONG="dlc_wi09:/common/data/ai/$(printf 'x%.0s' $(seq 102))"
# the summaries of the scenarios, word for word
S_FEW="166 weapons.meta files loaded (the game's own array holds 160, this one 1024) - the game's own array would have overflowed by 6 of them."
S_SWITCH="160 weapons.meta files loaded (the game's own array holds 160, this one 1024) - the guard's room, 160, is full: 6 files were refused.
160 weapons.meta files loaded (the game's own array holds 160, this one 1024) - the guard's room, 160, is full: 9 files were refused."
S_FLOOD="1024 weapons.meta files loaded (the game's own array holds 160, this one 1024) - the game's own array would have overflowed by 864 of them - the guard's room, 1024, is full: 20001 files were refused."
S_FULL="1024 weapons.meta files loaded (the game's own array holds 160, this one 1024) - the game's own array would have overflowed by 864 of them - the guard's room, 1024, is full: the next file is refused."
S_DRY="166 weapons.meta files loaded (the game's own array holds 160, this one 160) - past its end: the game's memory is already damaged."
S_PACKS="152 weapons.meta files loaded (the game's own array holds 160, this one 160).
160 weapons.meta files loaded (the game's own array holds 160, this one 160) - it is full: the next file would be written past its end.
165 weapons.meta files loaded (the game's own array holds 160, this one 160) - past its end: the game's memory is already damaged."

# site TEXT: the half's line "DIAGNOSTIC: TEXT", whole (a regex); problem TEXT: its problem, whole; file N TEXT: the guard's
# line for a file
site() { log_line "$D $1\$"; }
problem() { log_line "$W PROBLEM: $1\$"; }
file() { log_line "$D weapons.meta file $1: $2\$"; }
# rva LABEL: the RVA of a pattern's one match, from the scanner's line; hex N: N as the log writes an RVA
rva() { tr -d '\r' < TweaksV.log | sed -nE "s/.*Pattern $1 found: .* -> RVA (0x[0-9A-F]+), 1 match,.*/\1/p" | head -n 1; }
hex() { printf '0x%X' "$1"; }
# summaries: the summary lines, without their time and tag
summaries() { tr -d '\r' < TweaksV.log | grep -- "$SUMMARY" | sed 's/.*\] DIAGNOSTIC: //'; }
# guarded ROOM [RVA]: the guard's line, word for word - the loader at W-E-I2 - 0x18 (or RVA) hooked, the room it keeps
guarded() { log_line "$W The weapon info blob limit is guarded: the weapon info loader at RVA ${2:-$loader} is hooked, and a weapons.meta file past the room, $1, is refused instead of being written past the array.s end.\$"; }
# field LIMITS LOADER ARRAY [FILES]: the fake's report - the two immediates as they are, the loader's start (as laid,
# hooked or CHANGED), every other byte of the sites as laid, the array's count and room, and the files through the
# loader, those it counted and those refused, with how many came back as the loader returns and with the last error
# kept (none by default)
field() { has "weapon infos: limits $1, loader $2, the rest as laid, array $3, files ${4:-0 passed 0 refused, 0 of 0 as expected} |"; }
# The weapon components' half writes and reports under the same tag, so the checks below match this half's lines and
# problems by their words, never every line of the tag, a box's count of problems or every problem of a run.
# boxed: the start-up summary shown, its box kept off the screen by the test build; late: the guard's finding, one box of
# its own; quiet: no problem of this half - its two jobs' and its report's
boxed() { log_has 'Showing the start-up summary (' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:'; }
late() { [ "$(count 'Showing a finding (')" -eq 1 ] && log_line 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: WeaponLimits refused files the game has no room for: \.\.\. The game continues without the files refused\.$'; }
quiet() { log_lacks "$W PROBLEM: The weapon info blob limit" && log_lacks "$W PROBLEM: The weapons.meta file"; }
# failing: TweaksV.ini for a run of the test build with DIAGNOSTIC_MODE=1 and ERROR_POPUP=1; run_test OPTION...: that run
failing() { fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; }
run_test() { out=$(ASI=TweaksV_test.asi game full_game.exe "$@"); }
# undone REASON LIMITS ARRAY: both jobs undone for one reason, word for word, in the start-up summary; nothing written,
# nothing measured
undone() { problem "The weapon info blob limit is neither raised nor guarded this session: $1" && boxed && field "$2" 'as laid' "$3" && log_lacks 'and its room are logged after'; }

echo "== WI1 DIAGNOSTIC_MODE=1, 14 files of packs, 11 seconds: every site's line at its place, the writes, the hook, a line per file, the summary 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wifew wait); echo "      $out"
i1=$(rva W-E-I1); i2=$(rva W-E-I2); loader=$(hex $((i2 - 0x18)))
arr=$(tr -d '\r' < TweaksV.log | sed -nE "s/.* reference at RVA $(hex $((i1 + 23))) -> RVA (0x[0-9A-F]+) \(W-E-I1\): the block.s store - the weapon info blob array$/\1/p")
alloc=$(tr -d '\r' < TweaksV.log | sed -nE "s/.* allocation call at RVA $(hex $((i1 + 15))) -> RVA (0x[0-9A-F]+) \(W-E-I1\): .*/\1/p")
check "W-E-I1 and W-E-I2 found, 1 match each; the array and the allocation where their lines say" "[ -n '$i1' ] && [ -n '$i2' ] && [ -n '$arr' ] && [ -n '$alloc' ]"
check "the number allocated at W-E-I1 + 11 and the capacity at + 34, 160 each" "site 'limit at RVA $(hex $((i1 + 11))) holds 160 (W-E-I1): the blobs the game allocates' && site 'limit at RVA $(hex $((i1 + 34))) holds 160 (W-E-I1): the array.s capacity'"
check "the call at + 15 to the allocation, its first 14 bytes and its element size 0xF8" "site 'allocation call at RVA $(hex $((i1 + 15))) -> RVA $alloc (W-E-I1): 56 57 48 83 EC 28 89 D6 48 63 FA 48 69 CF F8 00 00 00'"
check "the block's store at + 23 giving the array, the capacity's store at + 30 leading to the array + 0xA" "site 'reference at RVA $(hex $((i1 + 23))) -> RVA $arr (W-E-I1): the block.s store - the weapon info blob array' && site 'reference at RVA $(hex $((i1 + 30))) -> RVA $(hex $((arr + 0xA))) (W-E-I1): the capacity.s store'"
check "the loader's count read at W-E-I2 + 3 and written at + 13, both at the array + 8, its block read at + 27, at the array, its blob size at + 20, 0xF8" "site 'reference at RVA $(hex $((i2 + 3))) -> RVA $(hex $((arr + 8))) (W-E-I2): the loader.s read of the count' && site 'reference at RVA $(hex $((i2 + 13))) -> RVA $(hex $((arr + 8))) (W-E-I2): the loader.s write of the count' && site 'reference at RVA $(hex $((i2 + 27))) -> RVA $arr (W-E-I2): the loader.s read of the block' && site 'blob size at RVA $(hex $((i2 + 20))) holds 0xF8 (W-E-I2)'"
check "the loader's start, 0x18 before the match, a function start (no unwind data in the fake, a NOP before it)" "site 'the loader at RVA $loader (W-E-I2 - 0x18): no unwind data, byte before it 90'"
check "the scan: exactly the 2 references to the array, the block's store one of them; the array still empty at TweaksV's start" "[ \$(count '$D reference at RVA 0x[0-9A-F]* -> RVA $arr (scan)') -eq 2 ] && site 'reference at RVA $(hex $((i1 + 23))) -> RVA $arr (scan)' && site 'the weapon info blob array at RVA $arr is still empty at TweaksV.s start (no block, 0 loaded, capacity 0): its initializer runs after TweaksV.s DllMain, so its limit can be raised in time.'"
check "both immediates written in one go, each write's line word for word, then the raise's line" "site 'weapon info blob array size (160 -> 1024) at RVA $(hex $((i1 + 11))): A0 00 00 00 -> 00 04 00 00' && site 'weapon info blob array capacity (160 -> 1024) at RVA $(hex $((i1 + 34))): A0 00 -> 00 04' && log_line '$RAISED'"
check "the guard's line, word for word: the loader hooked, the room 1024" "guarded 1024"
check "a line per file before the game's loader runs: 153 to 160, then 161 to 166 past the game's own 160 - the 140-character name cut at 127, the byte that is no printable ASCII a ?" "[ \$(count '$D weapons.meta file ') -eq 14 ] && file 153 'dlc_wi01:/common/data/ai/weapons.meta' && file 160 'dlc_wi08:/common/data/ai/weapons.meta' && file '161 (past the game.s own 160)' '$LONG' && file '162 (past the game.s own 160)' 'dlc_wi10:/common/data/ai/weap?ons.meta' && file '166 (past the game.s own 160)' 'dlc_wi14:/common/data/ai/weapons.meta'"
check "the announcement, and the summary 10 s in, word for word, once: 166 files of the game's 160, the capacity read back 1024" "site 'the weapon info blob array at RVA $arr (W-E-I1), blobs of 248 bytes: the weapons.meta files loaded and its room are logged after 10 seconds, then every minute while the numbers change.' && [ \"\$(summaries)\" = \"\$S_FEW\" ]"
check "the writes and nothing else: the immediates 1024, the loader hooked, every other byte as laid, every page its own protection; the 14 files counted by the loader, each with its result and last error; no problem" "field '1024 1024' hooked '166 of 1024' '14 passed 0 refused, 14 of 14 as expected' && has 'protect: all restored (RX)' && quiet"

echo "== WI2 the defaults, DIAGNOSTIC_MODE=0: the raise's line and the guard's, nothing more of the half"
fresh_ini; out=$(game full_game.exe)
check "the raise's line and the guard's, word for word, the half's only lines; no pattern's line, no DIAGNOSTIC line" "log_line '$RAISED' && guarded 1024 && [ \$(count '$W The weapon info blob limit') -eq 2 ] && log_lacks 'W-E-I' && log_lacks '$D .*weapon info' && log_lacks '$D .*weapons.meta'"
check "the writes and nothing else: the immediates 1024, the loader hooked, every page its own protection; no problem" "field '1024 1024' hooked '152 of 1024' && has 'protect: all restored (RX)' && quiet"

echo "== WI3 the room full - 872 files to it, then 14 more (test build, ERROR_POPUP=1): the 14 refused, reported once in a box of its own"
fresh_ini; setkey ERROR_POPUP 1; run_test wifill wifew
check "the count at the room, 1024; the 14 refused, the game's loader not called for them, each given 0 with its last error kept" "field '1024 1024' hooked '1024 of 1024' '872 passed 14 refused, 886 of 886 as expected'"
check "the report 10 s after the first refusal, once, word for word: the first file's name, 13 more, the room and what to set" "problem 'The weapons.meta file dlc_wi01:/common/data/ai/weapons.meta and 13 more were not loaded: the game.s room for weapons.meta files, 1024, is full - raise WEAPON_INFO_FILES in TweaksV.ini above 1024 (at most 16384).' && [ \$(count 'PROBLEM: The weapons.meta file') -eq 1 ]"
check "the first refusal's line at once, word for word, once" "[ \$(count 'is refused: the game.s room for weapons.meta files') -eq 1 ] && log_line '$W The weapons.meta file dlc_wi01:/common/data/ai/weapons.meta is refused: the game.s room for weapons.meta files, 1024, is full - the files refused are reported 10 seconds after this first one.\$'"
check "in a box of its own, its heading and last line those of a guard's refusal; no line per file without DIAGNOSTIC_MODE" "late && log_lacks 'weapons.meta file 1'"

echo "== WI4 the guards' test switch (test build, TWEAKSV_TEST_GUARD_ROOM=1, TWEAKSV_TEST_SUMMARY_MS=1000, DIAGNOSTIC_MODE=1): the game's own room, 14 files, then 3 more"
failing; out=$(TWEAKSV_TEST_GUARD_ROOM=1 TWEAKSV_TEST_SUMMARY_MS=1000 ASI=TweaksV_test.asi game full_game.exe wifew wilater)
check "the switch said once, the game's own room in the place of the part's, which the raise still gives the array" "[ \$(count 'TEST BUILD: the weapon infos guard.s room is the game.s own 160 instead of 1024 (TWEAKSV_TEST_GUARD_ROOM=1).') -eq 1 ] && guarded 160 && log_line '$RAISED'"
check "8 files to the guard's room, 6 refused, then 3 more - each refused given 0, every last error kept" "field '1024 1024' hooked '160 of 1024' '8 passed 9 refused, 17 of 17 as expected'"
check "a line per file: 160 the last loaded, then 161 refused 9 times - the 140-character name cut at 127, the byte that is no printable ASCII a ?" "file 160 'dlc_wi08:/common/data/ai/weapons.meta' && file '161 refused (the room, 160, is full)' '$LONG' && file '161 refused (the room, 160, is full)' 'dlc_wi10:/common/data/ai/weap?ons.meta' && [ \$(count '$D weapons.meta file 161 refused (the room, 160, is full): ') -eq 9 ]"
check "the report once, word for word: the first refused file's name cut at 127, 5 more, the guard's room, named as the test build's - no key to raise" "problem 'The weapons.meta file $LONG and 5 more were not loaded: the game.s room for weapons.meta files, 160, is full - the test build.s room (TWEAKSV_TEST_GUARD_ROOM=1).'"
check "one box of its own; the 3 later a line, not a second problem: 9 in all" "late && [ \$(count 'PROBLEM: The weapons.meta file') -eq 1 ] && log_line '$W 9 weapons.meta files in all were not loaded this session: the game.s room for weapons.meta files, 160, is full.\$'"
check "the summaries, word for word: the guard's room full with 6 refused, then with 9, and no third while nothing changes" '[ "$(summaries)" = "$S_SWITCH" ]'
check "the test build's line said once for each of the half's two threads" "[ \$(count 'TEST BUILD: the weapon infos summary comes every 1000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=1000).') -eq 1 ] && [ \$(count 'TEST BUILD: the weapon infos guard report comes every 1000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=1000).') -eq 1 ]"

echo "== WI5 TWEAKSV_TEST_GUARD_ROOM=1 with the release build: not read - the part's room"
fresh_ini; out=$(TWEAKSV_TEST_GUARD_ROOM=1 game full_game.exe wifew)
check "no test line, the room 1024: all 14 files loaded" "log_lacks 'TEST BUILD' && guarded 1024 && field '1024 1024' hooked '166 of 1024' '14 passed 0 refused, 14 of 14 as expected'"
check "the release build's file names neither the switch nor the test build's room, in either half; the test build's names both" "! grep -a -q 'TWEAKSV_TEST_GUARD_ROOM' TweaksV.asi && ! grep -a -q 'the test build' TweaksV.asi && grep -a -q 'the test build.s room (TWEAKSV_TEST_GUARD_ROOM=1)' TweaksV_test.asi && grep -a -q 'the test build.s (TWEAKSV_TEST_GUARD_ROOM=1)' TweaksV_test.asi"

echo "== WI6 the 9th file's name runs into memory that cannot be read (test build, TWEAKSV_TEST_GUARD_ROOM=1): kept as such"
failing; out=$(TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wifew wiunreadable)
check "its line and the report, word for word, the name that cannot be read said in its place" "file '161 refused (the room, 160, is full)' '(a name that cannot be read)' && problem 'The weapons.meta file (a name that cannot be read) and 5 more were not loaded: the game.s room for weapons.meta files, 160, is full - the test build.s room (TWEAKSV_TEST_GUARD_ROOM=1).'"
check "the game runs on: 8 passed, 6 refused; one box of its own" "field '1024 1024' hooked '160 of 1024' '8 passed 6 refused, 14 of 14 as expected' && late"

echo "== WI7 DIAGNOSTIC_MODE=1, 872 files and then 20,001 more: the lines per file past their cap"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wifill wiflood)
check "20,000 lines per file, then the cap's line once, word for word; the rest only counted" "[ \$(count '$D weapons.meta file [0-9]') -eq 20000 ] && [ \$(count '$D 20000 lines of weapons.meta files written - from now on they are only counted; the summary every minute has the numbers.') -eq 1 ]"
check "the summary 10 s in, word for word: 1024 loaded, all 20,001 refused counted" '[ "$(summaries)" = "$S_FLOOD" ]'
check "the report once, word for word; the game runs on" "problem 'The weapons.meta file dlc_flood00001:/common/data/ai/weapons.meta and 20000 more were not loaded: the game.s room for weapons.meta files, 1024, is full - raise WEAPON_INFO_FILES in TweaksV.ini above 1024 (at most 16384).' && field '1024 1024' hooked '1024 of 1024' '872 passed 20001 refused, 20873 of 20873 as expected'"

echo "== WI8 DRY_RUN=1 with DIAGNOSTIC_MODE=1, 14 files, 11 seconds: the would lines, nothing written, nothing hooked"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe wifew wait)
check "every site checked as without DRY_RUN; the would lines, word for word - both immediates, then the hook with the part's room" "site 'limit at RVA $(hex $((i1 + 11))) holds 160 (W-E-I1): the blobs the game allocates' && site 'the loader at RVA $loader (W-E-I2 - 0x18): no unwind data, byte before it 90' && log_line '$W DRY_RUN: would patch the weapon info blob array size (160 -> 1024): A0 00 00 00 -> 00 04 00 00\$' && log_line '$W DRY_RUN: would patch the weapon info blob array capacity (160 -> 1024): A0 00 -> 00 04\$' && log_line '$W DRY_RUN: would hook the weapon info loader at RVA $loader so that a weapons.meta file past the room, 1024, is refused.\$'"
check "nothing written, nothing hooked: the loader counts all 14, past the game's own array; no raise or guard line, no line per file, no problem" "field '160 160' 'as laid' '166 of 160' '14 passed 0 refused, 14 of 14 as expected' && log_lacks 'is raised from' && log_lacks 'is guarded' && log_lacks 'weapons.meta file 1' && quiet"
check "the summary 10 s in, word for word: past its end" '[ "$(summaries)" = "$S_DRY" ]'

echo "== WI9 DRY_RUN=1, packs mounted while the game runs (test build, TWEAKSV_TEST_SUMMARY_MS=1000): the summary again each time the numbers change - the game's own array full, then past its end"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(TWEAKSV_TEST_SUMMARY_MS=1000 ASI=TweaksV_test.asi game full_game.exe wipacks)
check "three summaries, word for word, in order: 152, then 160 - full -, then 165 - past its end -, and no fourth while nothing changes" '[ "$(summaries)" = "$S_PACKS" ]'
check "the switch said once for this measurement; nothing written; no problem" "[ \$(count 'TEST BUILD: the weapon infos summary comes every 1000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=1000).') -eq 1 ] && field '160 160' 'as laid' '165 of 160' && quiet"

echo "== WI10 the array set up before TweaksV starts (an initializer that ran first), 14 files (test build, ERROR_POPUP=1): the raise refused, the guard on with the game's own room"
fresh_ini; setkey ERROR_POPUP 1; run_test wisetup wifew
check "the raise refused, word for word, in the start-up summary" "problem 'The weapon info blob limit is not raised this session: the weapon info blob array at RVA $arr was set up before TweaksV started (a block, 152 loaded, capacity 160): its initializer ran before TweaksV.s DllMain.' && boxed"
check "the guard on all the same, with the game's own room: 8 files to it, 6 refused" "guarded 160 && field '160 160' hooked '160 of 160' '8 passed 6 refused, 14 of 14 as expected'"
check "the report, word for word, in a box of its own: the limit was not raised" "problem 'The weapons.meta file $LONG and 5 more were not loaded: the game.s room for weapons.meta files, 160, is full: its limit was not raised this session (see above).' && late"

echo "== WI11 both immediates hold 2048 before TweaksV starts (another mod's raise, or a later game's own): left as they are, the guard with that room"
fresh_ini; out=$(game full_game.exe wihigh)
check "the line, word for word, not a problem; the guard with the room the immediates give" "log_line '$W The weapon info blob limit is 2048 already (WEAPON_INFO_FILES=1024) - nothing to patch.\$' && guarded 2048 && quiet && field '2048 2048' hooked '152 of 2048'"
fresh_ini; setkey WEAPON_INFO_FILES 4096; out=$(game full_game.exe wihigh)
check "WEAPON_INFO_FILES=4096, above them: raised from 2048 to 4096" "log_line '$W The weapon info blob limit is raised from 2048 to 4096 weapons.meta files.\$' && guarded 4096 && field '4096 4096' hooked '152 of 4096'"

echo "== WI12 the array gets room for 512 after TweaksV's raise (another mod), 170 files and 872 more: the guard keeps to the array's own room"
fresh_ini; out=$(game full_game.exe wiraised wifill)
check "342 files to the array's 512, 530 refused - the array not overflowed" "field '1024 1024' hooked '512 of 512' '342 passed 530 refused, 872 of 872 as expected'"
check "the report, word for word: the array's room, less than the part gave it; with ERROR_POPUP=0 not shown" "problem 'The weapons.meta file dlc_fill343:/common/data/ai/weapons.meta and 529 more were not loaded: the game.s room for weapons.meta files, 512, is full - the array has less room than the 1024 TweaksV gave it (another mod may have changed its limit after TweaksV).' && log_has 'Finding not shown (ERROR_POPUP=0): '"

echo "== WI13 no code may be changed (Windows' dynamic code policy; test build, ERROR_POPUP=1): both jobs reported, nothing written"
fresh_ini; setkey ERROR_POPUP 1; run_test nodyncode
check "the raise and the guard reported, word for word" "problem 'The weapon info blob limit is not raised this session: the game code could not be made writable (details in TweaksV.log).' && problem 'The weapon info blob limit is not guarded this session: the weapon info loader at RVA $loader could not be hooked (details in TweaksV.log).'"
check "nothing written" "field '160 160' 'as laid' '152 of 160'"

echo "== WI14 [WEAPON_LIMITS] ENABLED=0 with DIAGNOSTIC_MODE=1: the part off - nothing looked for, nothing written"
fresh_ini; setkey ENABLED 0 WEAPON_LIMITS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "neither pattern looked for, no line of the half; the header says WeaponLimits off" "log_lacks 'W-E-I' && log_lacks 'weapon info' && log_lacks 'weapons.meta' && log_has 'Features: .*WeaponLimits off'"
check "nothing written" "field '160 160' 'as laid' '152 of 160'"

echo "== WI15 TEST_BREAK_PATTERN=W-E-I1 (test build, ERROR_POPUP=1): both jobs undone, one problem, nothing written"
failing; addkey TEST_BREAK_PATTERN W-E-I1; run_test
check "the scanner's test line, W-E-I2 looked for all the same" "log_has 'TEST BUILD: pattern W-E-I1 is treated as not found (TEST_BREAK_PATTERN).' && log_has 'Pattern W-E-I2 found: .* 1 match'"
check "the problem, word for word, in the start-up summary; nothing written, nothing measured" "undone 'W-E-I1 was not found.' '160 160' '152 of 160'"

echo "== WI16 TEST_BREAK_PATTERN=W-E-I2 (test build, ERROR_POPUP=1)"
failing; addkey TEST_BREAK_PATTERN W-E-I2; run_test
check "the problem, word for word, W-E-I1 found; nothing written" "undone 'W-E-I2 was not found.' '160 160' '152 of 160' && log_has 'Pattern W-E-I1 found: .* 1 match'"

echo "== WI17 the two limits disagree, 160 and 161 (another mod changed one)"
failing; run_test wimismatch
check "the problem, word for word; nothing written" "undone 'the two limits of the weapon info blob array disagree: 160 at RVA $(hex $((i1 + 11))) and 161 at RVA $(hex $((i1 + 34))) (W-E-I1; another mod may have changed one).' '160 161' '152 of 161'"

echo "== WI18 both limits 0"
failing; run_test wizero
check "the problem, word for word; nothing written" "undone 'the limits of the weapon info blob array hold 0 (W-E-I1).' '0 0' '152 of 0'"

echo "== WI19 the call to the allocation leads outside the game (another mod's hook)"
failing; run_test wihooked
check "the call's line and the problem, word for word; nothing written" "site 'allocation call at RVA $(hex $((i1 + 15))) -> 0x[0-9A-F]* (outside the game) (W-E-I1)' && undone 'the allocation call at RVA $(hex $((i1 + 15))) is not a call into the game (another mod may have hooked it already) (W-E-I1).' '160 160' '152 of 160'"

echo "== WI20 the allocation begins with a jump, not with its own bytes"
failing; run_test wibytes
check "the call's line with the bytes found, the problem, word for word; nothing written" "site 'allocation call at RVA $(hex $((i1 + 15))) -> RVA $alloc (W-E-I1): E9 57 48 83 EC 28 89 D6 48 63 FA 48 69 CF F8 00 00 00' && undone 'the allocation at RVA $alloc does not begin with the bytes it is expected to (W-E-I1).' '160 160' '152 of 160'"

echo "== WI21 the block's store leads outside the game: an address outside the game"
failing; run_test wioutside
check "the problem, word for word; nothing written" "undone 'the weapon info blob array at 0x[0-9A-F]* (outside the game) (W-E-I1) is not inside the game.' '160 160' '152 of 160'"

echo "== WI22 the capacity's store leads to the array + 0xC, not to its capacity"
failing; run_test wicapacity
check "the problem, word for word; nothing written" "undone 'the capacity.s store at RVA $(hex $((i1 + 30))) leads to RVA $(hex $((arr + 0xC))), not to the array.s capacity at RVA $(hex $((arr + 0xA))) (W-E-I1).' '160 160' '152 of 160'"

echo "== WI23 the loader's read of the count leads to the array + 0x10, not to its count"
failing; run_test wicount
check "the problem, word for word; nothing written" "undone 'the loader.s read of the count at RVA $(hex $((i2 + 3))) leads to RVA $(hex $((arr + 0x10))), not to RVA $(hex $((arr + 8))) (W-E-I2).' '160 160' '152 of 160'"

echo "== WI24 the loader's blob size 0x100, not the allocation's element size 0xF8"
failing; run_test wisize
check "the problem, word for word; nothing written" "undone 'the loader.s blob size, 0x100 at RVA $(hex $((i2 + 20))), is not the allocation.s element size, 0xF8 (W-E-I2).' '160 160' '152 of 160'"

echo "== WI25 the loader's start is no function start (48 before it, no unwind data): the guard refused, the raise done"
failing; run_test winotstart
check "the guard's problem, word for word, in the start-up summary; the raise done, the loader not hooked" "problem 'The weapon info blob limit is not guarded this session: the loader at RVA $loader (W-E-I2 - 0x18) is not a function start (no unwind data, byte before it 48).' && boxed && log_line '$RAISED' && field '1024 1024' 'as laid' '152 of 1024'"

echo "== WI26 a third reference to the array, a second store of a block (another mod, or a changed game): the raise refused, the guard on"
failing; run_test wiextra
check "the three references logged, the raise's problem word for word; the guard on with the game's own room, the immediates as laid" "[ \$(count '$D reference at RVA 0x[0-9A-F]* -> RVA $arr (scan)') -eq 3 ] && problem 'The weapon info blob limit is not raised this session: the game code holds 3 or more references to the weapon info blob array, not the expected 2 (another mod may have changed one, or the game has changed).' && boxed && guarded 160 && field '160 160' hooked '152 of 160'"

echo "== WI27 the plain game, no game code, DIAGNOSTIC_MODE=1: both patterns missing, one problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "both looked for and missing, the problem names the first, word for word" "log_has 'Pattern W-E-I1 not found' && log_has 'Pattern W-E-I2 not found' && problem 'The weapon info blob limit is neither raised nor guarded this session: W-E-I1 was not found.'"

echo "== WI28 the loader copied into a function the fake's unwind data names, at its start (wiunwind; test build): its start by unwind data"
failing; run_test wiunwind
c2=$(rva W-E-I2)
check "the loader's start a function start by unwind data, word for word; the guard on at that start, the raise done, no problem of the half" "[ -n '$c2' ] && site 'the loader at RVA $(hex $((c2 - 0x18))) (W-E-I2 - 0x18): a function start (unwind data)' && guarded 1024 $(hex $((c2 - 0x18))) && log_line '$RAISED' && quiet"

echo "== WI29 the loader copied 8 bytes into that function (wiunwindin): its start inside the function by unwind data - refused, though a CC stands before it"
failing; run_test wiunwindin
c2=$(rva W-E-I2)
check "the guard's problem, word for word: W-E-I2 lies in the function that begins 8 bytes before the loader's start; the raise done" "[ -n '$c2' ] && problem 'The weapon info blob limit is not guarded this session: the loader at RVA $(hex $((c2 - 0x18))) (W-E-I2 - 0x18) is not a function start: W-E-I2 lies in the function at RVA $(hex $((c2 - 0x20))) (unwind data).' && boxed && log_line '$RAISED'"

echo "== WI30 another mod's port rewrites the number of blobs alone to 512 after TweaksV's raise (wiedx), 872 files: the guard keeps to the blobs"
fresh_ini; out=$(game full_game.exe wiedx wifill)
check "360 files to the 512 blobs, 512 refused - none past the blobs, though the capacity says 1024" "field '512 1024' hooked '512 of 1024' '360 passed 512 refused, 872 of 872 as expected'"
check "the report, word for word: the array's room, less than the part gave it" "problem 'The weapons.meta file dlc_fill361:/common/data/ai/weapons.meta and 511 more were not loaded: the game.s room for weapons.meta files, 512, is full - the array has less room than the 1024 TweaksV gave it (another mod may have changed its limit after TweaksV).'"

echo "== WI31 the loader's start a jump already - another mod's hook (wijump) - and 14 files: the guard on top of it, every file through both"
fresh_ini; out=$(game full_game.exe wijump wifew)
check "the line, word for word, not a problem; the guard on, every file through it and the other mod's hook to the loader" "log_line '$W The weapon info loader at RVA $loader already starts with a jump - another mod may have hooked it; the guard goes on top and chains through.\$' && guarded 1024 && quiet && field '1024 1024' hooked '166 of 1024' '14 passed 0 refused, 14 of 14 as expected'"

echo "== WI32 DIAGNOSTIC_MODE=1, 872 files, 11 seconds: the room reached, no file refused yet"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wifill wait)
check "the summary 10 s in, word for word: the guard's room full, the next file refused" '[ "$(summaries)" = "$S_FULL" ]'

echo "== WI33 WEAPON_INFO_FILES=16384, its maximum, and 20,001 files: the room full, no key to raise"
fresh_ini; setkey WEAPON_INFO_FILES 16384; out=$(game full_game.exe wiflood)
check "the report, word for word: the most the key allows, nothing to raise" "problem 'The weapons.meta file dlc_flood16233:/common/data/ai/weapons.meta and 3768 more were not loaded: the game.s room for weapons.meta files, 16384, is full, the most WEAPON_INFO_FILES allows.'"
check "16,232 files to the room, 3,769 refused, each given 0 with its last error kept" "guarded 16384 && field '16384 16384' hooked '16384 of 16384' '16232 passed 3769 refused, 20001 of 20001 as expected'"

echo "== WI34 the raise refused with the guards' test switch (test build, TWEAKSV_TEST_GUARD_ROOM=1, an array set up before TweaksV): why the room is the game's own is the raise's, not the switch's"
failing; out=$(TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wisetup wifew)
check "the report, word for word: the limit not raised, not the test build's room" "problem 'The weapons.meta file $LONG and 5 more were not loaded: the game.s room for weapons.meta files, 160, is full: its limit was not raised this session (see above).'"

echo "== WI35 W-E-I1 twice in the game's code (wiinittwice; test build, ERROR_POPUP=1): both jobs undone, named as twice, not as missing"
failing; run_test wiinittwice
check "the problem, word for word, in the start-up summary; nothing written, nothing measured" "undone 'the game code holds W-E-I1 2 or more times, not the expected 1.' '160 160' '152 of 160' && log_lacks 'W-E-I1 was not found'"
check "the scanner's line for the matches; W-E-I2 looked for all the same" "log_has 'Pattern W-E-I1 found: .* 2 matches,' && log_has 'Pattern W-E-I2 found: .* 1 match,'"

echo "== WI36 W-E-I2 twice in the game's code (wiloadertwice; test build, ERROR_POPUP=1)"
failing; run_test wiloadertwice
check "the problem, word for word, in the start-up summary; nothing written, nothing measured" "undone 'the game code holds W-E-I2 2 or more times, not the expected 1.' '160 160' '152 of 160' && log_lacks 'W-E-I2 was not found' && log_has 'Pattern W-E-I1 found: .* 1 match,'"

echo
echo "WEAPON INFOS SCENARIOS: $pass passed, $fail failed"
