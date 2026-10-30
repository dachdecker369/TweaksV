#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The check for packs made for GTA V Legacy, a job of RageOpenV (SPECIFICATIONS 11.6): the full fake game writes crafted RPF7
# archives into the mods folder before it loads the real TweaksV.asi (its options lpmake, lpmany and lplock), asks for the time
# once, as the game does - RageOpenV hooks the file functions and starts the check - and waits for the check's last line
# (lpwait). Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" .
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata mods/test
echo hi > mods/test/file.txt

FILES_OK='file hooks 8/8, redirect 8/8, pass 8/8,'
DONE='legacy packs: done |'
DLC=mods/update/x64/dlcpacks
# the mods folder back to its one file, test/file.txt
packs_gone() { find mods -mindepth 1 -maxdepth 1 ! -name test -exec rm -rf {} +; }
# the check's one problem, in the start-up summary's box or in the box for problems after it, whichever came first
one_box() { [ "$(grep -c -E '(Start-up summary|Problem after the start-up summary) not shown \(ERROR_POPUP=0\): 1 problem' TweaksV.log)" -eq 1 ]; }
first_line() { tr -d '\r' < TweaksV.log | grep -n -- "$1" | head -n 1 | cut -d: -f1; }
packs_gone	# a run kept with --keep, or a suite stopped half-way, may have left packs

echo "== L1 a pack made for Legacy and one made for Enhanced, each a dlcpack of its own: the Legacy one named in a line and in the one problem"
fresh_ini; out=$(game full_game.exe lpmake legacy $DLC/lpold/dlc.rpf lpmake enhanced $DLC/lpnew/dlc.rpf lpwait); echo "      $out"
check "the archives made, the mods folder used, the check's last line come" "has 'lp: 2 made |' && has '$FILES_OK' && has '$DONE'"
check "the Legacy pack's line: its resources with Legacy's versions and the first of them" "log_line '\[RageOpenV\] The pack mods.update.x64.dlcpacks.lpold.dlc.rpf was made for GTA V Legacy: 2 yft 162, 1 ydr 165, 1 ydd 165, 1 ytd 13 - the first lp.yft\.\$'"
check "no line for the Enhanced pack, none of DIAGNOSTIC_MODE" "log_lacks 'lpnew' && log_lacks 'DIAGNOSTIC: the pack' && log_lacks 'DIAGNOSTIC: the check read'"
check "the one problem names the Legacy pack and says what to do; one box" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, and a Legacy vehicle stops the game as it is created - convert it for Enhanced with CodeWalker, or take it out: mods.update.x64.dlcpacks.lpold.dlc.rpf\.\$' && [ \$(count 'PROBLEM') -eq 1 ] && one_box"
check "the last line counts the packs" "log_line '\[RageOpenV\] Packs checked for GTA V Legacy versions: 2 in the mods folder, 1 made for Legacy\.\$'"

echo "== L2 DIAGNOSTIC_MODE=1: a line per pack read with its resources by version; Legacy by its textures alone; Legacy's numbers on other kinds and names counted for nothing"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake textures $DLC/lptex/dlc.rpf lpmake others $DLC/lpothers/dlc.rpf lpwait)
check "each pack read: its archives, its entries and its resources by version" "log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpnew.dlc.rpf: 1 archive, 7 entries; 2 yft 171, 1 ydr 159, 1 ydd 159, 1 ytd 5\.\$' && log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpold.dlc.rpf: 1 archive, 7 entries; 2 yft 162, 1 ydr 165, 1 ydd 165, 1 ytd 13\.\$'"
check "a texture dictionary with Legacy's version beside an Enhanced model makes the pack one made for Legacy" "log_line 'The pack mods.update.x64.dlcpacks.lptex.dlc.rpf was made for GTA V Legacy: 1 ytd 13 - the first lp.ytd\.\$' && log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lptex.dlc.rpf: 1 archive, 3 entries; 1 yft 171, 1 ytd 13\.\$'"
check "Legacy's numbers on a ybn, a ymap, a yftx and a name that is only yft count for nothing, nor does another number on a ytd; a YFT in capitals is a yft" "log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpothers.dlc.rpf: 1 archive, 7 entries; 1 yft 171, 1 ytd 162\.\$' && log_lacks 'lpothers.dlc.rpf was made'"
check "the one problem names both Legacy packs in the walk's order" "log_line 'PROBLEM: 2 packs in the mods folder were made for GTA V Legacy, and a Legacy vehicle stops the game as it is created - convert them for Enhanced with CodeWalker, or take them out: mods.update.x64.dlcpacks.lpold.dlc.rpf, mods.update.x64.dlcpacks.lptex.dlc.rpf\.\$' && one_box"
check "the check's bytes and time, then its last line" "log_line 'DIAGNOSTIC: the check read 0\.0 MB of entry tables and names from 4 archives in [0-9]*\.[0-9][0-9] s\.\$' && log_line 'Packs checked for GTA V Legacy versions: 4 in the mods folder, 2 made for Legacy\.\$'"

echo "== L3 archives nested in a pack: read as the pack is, 8 levels deep at most; those not OPEN counted, not read"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake nested mods/lpnest.rpf lpmake deep8 mods/lpdeep8.rpf lpmake deep9 mods/lpdeep9.rpf lpwait)
check "a Legacy model in a nested archive: the pack named, with the nested archive that holds it" "log_line 'The pack mods.lpnest.rpf was made for GTA V Legacy: 1 yft 162 - the first lp.yft in its nested archive vehicles.rpf\.\$'"
check "the nested archive read; the one not OPEN and the one whose bytes are encrypted counted, not read" "log_line 'DIAGNOSTIC: the pack mods.lpnest.rpf: 2 archives (1 nested), 2 nested not OPEN, 7 entries; 1 yft 162, 1 ytd 5\.\$'"
check "8 levels deep: read, the Legacy model found" "log_line 'The pack mods.lpdeep8.rpf was made for GTA V Legacy: 1 yft 162 - the first lp.yft in its nested archive l1.rpf/l2.rpf/l3.rpf/l4.rpf/l5.rpf/l6.rpf/l7.rpf/l8.rpf\.\$' && log_line 'DIAGNOSTIC: the pack mods.lpdeep8.rpf: 9 archives (8 nested), 18 entries; 1 yft 162\.\$'"
check "9 levels deep: the ninth not read, the pack not checked in full" "log_line 'The pack mods.lpdeep9.rpf was not checked for GTA V Legacy versions in full: its nested archive l1.rpf/l2.rpf/l3.rpf/l4.rpf/l5.rpf/l6.rpf/l7.rpf/l8.rpf/l9.rpf is nested deeper than 8 levels\.\$' && log_lacks 'lpdeep9.rpf was made'"
check "one problem for all three: the Legacy packs, then the one not checked; the last line" "log_line 'PROBLEM: 2 packs in the mods folder were made for GTA V Legacy, .*: mods.lpdeep8.rpf, mods.lpnest.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.lpdeep9.rpf\.\$' && one_box && log_line 'Packs checked for GTA V Legacy versions: 3 in the mods folder, 2 made for Legacy, 1 not checked\.\$'"

echo "== L4 packs that are not OPEN - an NG and an AES header: named as not checked in their lines, not in a problem"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake ng mods/lpng.rpf lpmake aes mods/lpaes.rpf lpwait)
check "each named with its encryption" "log_line 'The pack mods.lpng.rpf was not checked for GTA V Legacy versions: it is not OPEN (encryption 0x0FEFFFFF, NG) - only OPEN archives are read, and the game.s own archives are encrypted\.\$' && log_line 'The pack mods.lpaes.rpf was not checked for GTA V Legacy versions: it is not OPEN (encryption 0x0FFFFFF9, AES) - only OPEN archives are read, and the game.s own archives are encrypted\.\$'"
check "no problem, no box; the last line counts them" "log_lacks 'PROBLEM' && log_lacks 'not shown (ERROR_POPUP=0)' && log_line 'Packs checked for GTA V Legacy versions: 2 in the mods folder, none made for Legacy, 2 not checked (2 of them not OPEN)\.\$'"

echo "== L5 damaged packs, one of each kind: each named as not checked with its reason; nothing read past the damage"
packs_gone; fresh_ini; args=()
for k in empty badmagic nocount toomany pastend pastnames noroot badname nestedout nestedself compressed notrpf rootonly; do args+=(lpmake $k mods/damaged/$k.rpf); done
out=$(game full_game.exe "${args[@]}" lpwait)
NOT='was not checked for GTA V Legacy versions'
check "13 archives made; the check's last line come" "has 'lp: 13 made |' && has '$DONE'"
check "empty, and a zip's first bytes" "log_line 'The pack mods.damaged.empty.rpf $NOT: it is shorter than an archive.s header (0 bytes)\.\$' && log_line 'The pack mods.damaged.badmagic.rpf $NOT: it is not an RPF7 archive (its first bytes are 50 4B 03 04)\.\$'"
check "no entries, and more than 1,048,576" "log_line 'The pack mods.damaged.nocount.rpf $NOT: it has no entries\.\$' && log_line 'The pack mods.damaged.toomany.rpf $NOT: it has 1048577 entries, more than 1048576\.\$'"
check "an entry table cut off, and names that run past the end" "log_line 'The pack mods.damaged.pastend.rpf $NOT: it has an entry table and names that run past its end (192 of 64 bytes)\.\$' && log_line 'The pack mods.damaged.pastnames.rpf $NOT: it has an entry table and names that run past its end (2147483760 of 1024 bytes)\.\$'"
check "no root folder; a name past the names, the table read up to it" "log_line 'The pack mods.damaged.noroot.rpf $NOT: it has no folder as its first entry\.\$' && log_line 'The pack mods.damaged.badname.rpf $NOT in full: it has a name outside its names (entry 1)\.\$' && log_lacks 'badname.rpf was made'"
check "a nested archive past the end, the Legacy texture dictionary beside it still found" "log_line 'The pack mods.damaged.nestedout.rpf $NOT in full: its nested archive out.rpf lies outside the archive that holds it\.\$' && log_line 'The pack mods.damaged.nestedout.rpf was made for GTA V Legacy: 1 ytd 13 - the first lp.ytd\.\$'"
check "a nested archive at its holder's own header, a compressed one, one that is a zip" "log_line 'The pack mods.damaged.nestedself.rpf $NOT in full: its nested archive self.rpf lies outside the archive that holds it\.\$' && log_line 'The pack mods.damaged.compressed.rpf $NOT in full: its nested archive packed.rpf is compressed\.\$' && log_line 'The pack mods.damaged.notrpf.rpf $NOT in full: its nested archive junk.rpf is not an RPF7 archive (its first bytes are 50 4B 03 04)\.\$'"
check "the root folder alone: read, no line" "log_lacks 'rootonly'"
check "one problem: the Legacy pack, then the first 10 not checked in the walk's order and the 2 more" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: mods.damaged.nestedout.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.damaged.badmagic.rpf, mods.damaged.badname.rpf, mods.damaged.compressed.rpf, mods.damaged.empty.rpf, mods.damaged.nestedout.rpf, mods.damaged.nestedself.rpf, mods.damaged.nocount.rpf, mods.damaged.noroot.rpf, mods.damaged.notrpf.rpf, mods.damaged.pastend.rpf, and 2 more\.\$' && one_box"
check "the last line" "log_line 'Packs checked for GTA V Legacy versions: 13 in the mods folder, 1 made for Legacy, 12 not checked\.\$'"

echo "== L6 the bounds of a pack: 4,096 archives read, 4,097 not; 64 MB of entry tables and names"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake manynestedok mods/bounds/manyok.rpf lpmake manynested mods/bounds/many.rpf lpmake bigtables mods/bounds/big.rpf lpwait)
check "the pack and 4,095 archives nested in it: read" "log_line 'DIAGNOSTIC: the pack mods.bounds.manyok.rpf: 4096 archives (4095 nested), 12286 entries; 4095 ytd 5\.\$' && log_lacks 'manyok.rpf was not'"
check "4,096 nested: the pack not checked in full, none of them read" "log_line 'The pack mods.bounds.many.rpf was not checked for GTA V Legacy versions in full: it holds more than 4096 archives\.\$' && log_line 'DIAGNOSTIC: the pack mods.bounds.many.rpf: 1 archive, 4097 entries; no yft, ydr, ydd or ytd\.\$'"
check "nested archives of 16 MB of entry table: three read, the fourth past 64 MB" "log_line 'The pack mods.bounds.big.rpf was not checked for GTA V Legacy versions in full: its entry tables and names pass 64 MB\.\$' && log_line 'DIAGNOSTIC: the pack mods.bounds.big.rpf: 4 archives (3 nested), 3145734 entries; no yft, ydr, ydd or ytd\.\$'"
check "the one problem names the two" "log_line 'PROBLEM: 2 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.bounds.big.rpf, mods.bounds.many.rpf\.\$' && one_box"

echo "== L7 the whole check's bound, 1 GB of entry tables and names: 23 packs of 48 MB - the 22nd read up to the bound, the 23rd not at all"
packs_gone; fresh_ini; out=$(game full_game.exe lpmany heavy 23 mods/heavy lpwait)
check "the archives made; the check's last line come" "has 'lp: 23 made |' && has '$DONE'"
check "the first 21 read in full, so no line of theirs" "[ \$(count 'mods.heavy.p000') -eq 3 ]"
check "the 22nd read up to the bound, the 23rd not read" "log_line 'The pack mods.heavy.p00022.rpf was not checked for GTA V Legacy versions in full: the check had read 1024 MB of entry tables and names before it\.\$' && log_line 'The pack mods.heavy.p00023.rpf was not checked for GTA V Legacy versions: the check had read 1024 MB of entry tables and names before it\.\$'"
check "the one problem names both; the last line counts the 23" "log_line 'PROBLEM: 2 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.heavy.p00022.rpf, mods.heavy.p00023.rpf\.\$' && one_box && log_line 'Packs checked for GTA V Legacy versions: 23 in the mods folder, none made for Legacy, 2 not checked\.\$'"

echo "== L8 20,001 packs made for Legacy, DIAGNOSTIC_MODE=1: each kind of line stops at 20,000, the rest counted; the start-up does not wait for the check"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey UNLOCK_FILES 0 DIRECTSTORAGE; out=$(game full_game.exe lpmany legacy 20001 mods/many lpwait)
check "the archives made; the check's last line come" "has 'lp: 20001 made |' && has '$DONE'"
check "20000 lines of Legacy packs, then the cap line once" "[ \$(count 'was made for GTA V Legacy: ') -eq 20000 ] && [ \$(count '20000 lines of packs made for GTA V Legacy written - from now on they are only counted\.') -eq 1 ]"
check "20000 lines of packs read, then the cap line once" "[ \$(count 'DIAGNOSTIC: the pack mods') -eq 20000 ] && [ \$(count 'DIAGNOSTIC: 20000 lines of packs read written - from now on they are only counted\.') -eq 1 ]"
check "the problem names the first 10 and counts the rest; the last line counts every pack" "log_line 'PROBLEM: 20001 packs in the mods folder were made for GTA V Legacy, .*: mods.many.p00001.rpf, .*, mods.many.p00010.rpf, and 19991 more\.\$' && log_line 'Packs checked for GTA V Legacy versions: 20001 in the mods folder, 20001 made for Legacy\.\$'"
check "the start-up went on while the check read: the OPEN archives set up before the check's last line, its problem after the start-up summary" "[ \$(first_line 'Unencrypted (OPEN) RPF archives can be loaded') -lt \$(first_line 'Packs checked for GTA V Legacy versions') ] && log_has 'Problem after the start-up summary not shown (ERROR_POPUP=0): 1 problem'"

echo "== L9 20,001 packs that cannot be read: the lines stop at 20,000, the problem names 10"
packs_gone; fresh_ini; out=$(game full_game.exe lpmany empty 20001 mods/many lpwait)
check "20000 lines, then the cap line once; the problem and the last line count every pack" "[ \$(count 'was not checked for GTA V Legacy versions: it is shorter than an archive.s header (0 bytes)\.') -eq 20000 ] && [ \$(count '20000 lines of packs not checked written - from now on they are only counted\.') -eq 1 ] && log_line 'PROBLEM: 20001 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.many.p00001.rpf, .*, and 19991 more\.\$' && log_line 'Packs checked for GTA V Legacy versions: 20001 in the mods folder, none made for Legacy, 20001 not checked\.\$'"

echo "== L10 a pack another program holds without sharing: named as not checked, with the Windows error"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake legacy mods/held/lp.rpf lplock mods/held/lp.rpf lpwait)
check "its line and the problem; one box" "log_line 'The pack mods.held.lp.rpf was not checked for GTA V Legacy versions: it could not be opened (Windows error 32, file in use)\.\$' && log_line 'PROBLEM: A pack in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.held.lp.rpf\.\$' && one_box"

echo "== L11 names: a pack's path outside ASCII, in UTF-8; a name in a pack with a line feed and a byte outside ASCII; a folder named like an archive; .RPF in capitals"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake oddname "mods/Ελληνικά/über.rpf" lpmake legacy mods/folder.rpf/inner.rpf lpmake legacy mods/UPPER.RPF lpwait)
check "the path in UTF-8, the name from the pack printable, on one line" "log_line 'The pack mods.Ελληνικά.über.rpf was made for GTA V Legacy: 1 yft 162 - the first l?p??.yft\.\$'"
check "a folder named .rpf walked, not read as a pack; a file named .RPF read" "log_has 'The pack mods.folder.rpf.inner.rpf was made for GTA V Legacy' && log_lacks 'The pack mods.folder.rpf was' && log_has 'The pack mods.UPPER.RPF was made for GTA V Legacy' && log_line 'Packs checked for GTA V Legacy versions: 3 in the mods folder, 3 made for Legacy\.\$'"

echo "== L12 a pack 32 folders deep read, one 33 deep not, as RageOpenV's list reads the folder"
packs_gone; fresh_ini; d=mods/zdeep; for i in $(seq 1 30); do d="$d/d"; done
out=$(game full_game.exe lpmake legacy "$d/in.rpf" lpmake legacy "$d/d/out.rpf" lpwait)
check "the pack 32 folders deep named, the one below it not walked" "log_has 'The pack mods.zdeep.d.*.d.in.rpf was made for GTA V Legacy' && log_lacks 'out.rpf' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy\.\$'"

echo "== L13 the check reads only what the game is served: no packs, the mods folder unused"
packs_gone; fresh_ini; out=$(game full_game.exe lpwait)
check "no packs: the last line says so, nothing more" "has '$DONE' && log_line 'Packs checked for GTA V Legacy versions: none in the mods folder\.\$' && log_lacks 'PROBLEM' && [ \$(count 'GTA V Legacy') -eq 1 ]"
lp='lpmake legacy mods/lp.rpf'
fresh_ini; setkey MODS_FOLDER 0 RAGEOPENV; out=$(game full_game.exe $lp)
check "MODS_FOLDER=0: the mods folder not used, its packs not checked" "has 'lp: 1 made |' && log_has 'MODS_FOLDER=0 - files in the mods folder are not used.' && log_lacks 'GTA V Legacy'"
fresh_ini; setkey ENABLED 0 RAGEOPENV; out=$(game full_game.exe $lp)
check "RageOpenV off: nothing of it, nothing checked" "log_lacks '\[RageOpenV\]' && log_lacks 'GTA V Legacy'"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe $lp)
check "DRY_RUN=1: the file functions not hooked, so nothing checked" "log_has 'DRY_RUN: would hook the 8 file functions for the mods folder.' && log_lacks 'GTA V Legacy'"
fresh_ini; out=$(game plain_game.exe)
check "the plain game: its file functions not found, so nothing checked" "log_has 'PROBLEM: Files in the mods folder are not used' && log_lacks 'GTA V Legacy'"
fresh_ini; mv mods mods_off; out=$(game full_game.exe); mv mods_off mods
check "no mods folder: nothing checked" "log_has 'No mods folder in the current directory' && log_lacks 'GTA V Legacy'"

echo "== L13b a mods folder the check cannot list - reached through the test build's TWEAKSV_TEST_LEGACY_FOLDER, since RageOpenV stops at such a folder first: a problem, no last line"
packs_gone; fresh_ini; mkdir -p unlisted; deny_listing unlisted
out=$(TWEAKSV_TEST_LEGACY_FOLDER="$(cygpath -w "$B/unlisted")" ASI=TweaksV_test.asi game full_game.exe lpwait); allow_writes unlisted; rmdir unlisted
check "the test build's line names the folder; the check not done, said as a problem with the Windows error; no last line" "has '$DONE' && log_has 'TEST BUILD: the check for packs made for GTA V Legacy reads .*unlisted in place of the mods folder (TWEAKSV_TEST_LEGACY_FOLDER)\.' && log_line 'PROBLEM: The check for packs made for GTA V Legacy is not done this session: the mods folder could not be read (Windows error 5, access denied)\.\$' && log_lacks 'Packs checked for GTA V Legacy versions' && one_box"

echo "== L14 ERROR_POPUP=1 (the test build, which shows no box with TWEAKSV_TEST_NO_BOXES=1): one box for a Legacy pack and one that cannot be read"
packs_gone; fresh_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe lpmake legacy mods/lp.rpf lpmake empty mods/lpempty.rpf lpwait)
check "one problem, one box" "[ \$(count 'PROBLEM') -eq 1 ] && log_has 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: mods.lp.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.lpempty.rpf\.' && [ \$(count 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): ') -eq 1 ] && [ \$(grep -c -E 'Showing (the start-up summary|a problem after the start-up summary) \(1 problem\(s\)\)\.' TweaksV.log) -eq 1 ]"
packs_gone

echo
echo "LEGACY PACKS SCENARIOS: $pass passed, $fail failed"
