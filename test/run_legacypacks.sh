#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The check for packs made for GTA V Legacy, a job of RageOpenV (SPECIFICATIONS 11.6): the full fake game writes crafted RPF7
# archives into the mods folder before it loads the real TweaksV.asi (its options lpmake, lpmany and lplock), asks for the time
# once, as the game does - RageOpenV hooks the file functions and starts the check - and waits for the check's last line
# (lpwait), then opens what it made without sharing (lpfree). Fake LOCALAPPDATA: the real one is never touched.
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
packs_gone() { find mods -mindepth 1 -maxdepth 1 ! -name test -exec rm -rf {} +; rm -f mods/test/*.rpf; }
# the check's one problem, a finding, in a box of its own; one_box: its problem that it is not done, in the start-up summary's
# box or in the box for problems after it, whichever came first
one_finding() { [ "$(count 'Finding not shown (ERROR_POPUP=0): 1 finding(s), see above\.')" -eq 1 ] && log_lacks 'problem(s), see above'; }
one_box() { [ "$(grep -c -E '(Start-up summary|Problem after the start-up summary) not shown \(ERROR_POPUP=0\): 1 problem' TweaksV.log)" -eq 1 ]; }
first_line() { tr -d '\r' < TweaksV.log | grep -n -- "$1" | head -n 1 | cut -d: -f1; }
packs_gone	# a run kept with --keep, or a suite stopped half-way, may have left packs

echo "== L1 a pack made for Legacy and one made for Enhanced, each a dlcpack of its own: the Legacy one named in a line and in the one problem"
fresh_ini; out=$(game full_game.exe lpmake legacy $DLC/lpold/dlc.rpf lpmake enhanced $DLC/lpnew/dlc.rpf lpwait); echo "      $out"
check "the archives made, the mods folder used, the check's last line come" "has 'lp: 2 made |' && has '$FILES_OK' && has '$DONE'"
check "the Legacy pack's line: its resources with Legacy's versions and the first of them" "log_line '\[RageOpenV\] The pack mods.update.x64.dlcpacks.lpold.dlc.rpf was made for GTA V Legacy: 2 yft 162, 1 ydr 165, 1 ydd 165, 1 ytd 13 - the first lp.yft\.\$'"
check "no line for the Enhanced pack, none of DIAGNOSTIC_MODE" "log_lacks 'lpnew' && log_lacks 'DIAGNOSTIC: the pack' && log_lacks 'DIAGNOSTIC: the check read'"
check "the one problem names the Legacy pack and says what to do; one box" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, and a Legacy vehicle stops the game as it is created - convert it for Enhanced with CodeWalker, or take it out: mods.update.x64.dlcpacks.lpold.dlc.rpf\.\$' && [ \$(count 'PROBLEM') -eq 1 ] && one_finding"
check "the last line counts the packs" "log_line '\[RageOpenV\] Packs checked for GTA V Legacy versions: 2 in the mods folder, 1 made for Legacy\.\$'"

echo "== L2 DIAGNOSTIC_MODE=1: a line per pack read with its resources by version; Legacy by its textures alone; Legacy's numbers on other kinds and names counted for nothing"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake textures $DLC/lptex/dlc.rpf lpmake others $DLC/lpothers/dlc.rpf lpwait)
check "each pack read: its archives, its entries and its resources by version" "log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpnew.dlc.rpf: 1 archive, 7 entries; 2 yft 171, 1 ydr 159, 1 ydd 159, 1 ytd 5\.\$' && log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpold.dlc.rpf: 1 archive, 7 entries; 2 yft 162, 1 ydr 165, 1 ydd 165, 1 ytd 13\.\$'"
check "a texture dictionary with Legacy's version beside an Enhanced model makes the pack one made for Legacy" "log_line 'The pack mods.update.x64.dlcpacks.lptex.dlc.rpf was made for GTA V Legacy: 1 ytd 13 - the first lp.ytd\.\$' && log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lptex.dlc.rpf: 1 archive, 3 entries; 1 yft 171, 1 ytd 13\.\$'"
check "Legacy's numbers on a ybn, a ymap, a yftx and a name that is only yft count for nothing, nor does another number on a ytd; a YFT in capitals is a yft" "log_line 'DIAGNOSTIC: the pack mods.update.x64.dlcpacks.lpothers.dlc.rpf: 1 archive, 7 entries; 1 yft 171, 1 ytd 162\.\$' && log_lacks 'lpothers.dlc.rpf was made'"
check "the one problem names both Legacy packs in the walk's order, RageOpenV's - the last folder listed first" "log_line 'PROBLEM: 2 packs in the mods folder were made for GTA V Legacy, and a Legacy vehicle stops the game as it is created - convert them for Enhanced with CodeWalker, or take them out: mods.update.x64.dlcpacks.lptex.dlc.rpf, mods.update.x64.dlcpacks.lpold.dlc.rpf\.\$' && one_finding"
check "the check's bytes and time, then its last line" "log_line 'DIAGNOSTIC: the check read 0\.0 MB of entry tables and names from 4 archives in [0-9]*\.[0-9][0-9] s\.\$' && log_line 'Packs checked for GTA V Legacy versions: 4 in the mods folder, 2 made for Legacy\.\$'"

echo "== L3 archives nested in a pack: read as the pack is, 8 levels deep at most; those not OPEN not read - the pack named in its line alone, or beside its damage"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake nested mods/lpnest.rpf lpmake deep8 mods/lpdeep8.rpf lpmake deep9 mods/lpdeep9.rpf lpmake mixed mods/lpmixed.rpf lpwait)
check "a Legacy model in a nested archive: the pack named, with the nested archive that holds it" "log_line 'The pack mods.lpnest.rpf was made for GTA V Legacy: 1 yft 162 - the first lp.yft in its nested archive vehicles.rpf\.\$'"
check "the nested archive read; the one not OPEN and the one whose bytes are encrypted counted, not read" "log_line 'DIAGNOSTIC: the pack mods.lpnest.rpf: 2 archives (1 nested), 2 nested not OPEN, 7 entries; 1 yft 162, 1 ytd 5\.\$'"
check "those two named in a line of the pack's own, as not OPEN" "log_line 'The pack mods.lpnest.rpf was not checked for GTA V Legacy versions in full: 2 archives nested in it are not OPEN - only archives that are not encrypted are read, and the game.s own archives are encrypted\.\$'"
check "a nested archive whose bytes are encrypted beside a compressed one: the damage named, the archive not OPEN counted beside it" "log_line 'The pack mods.lpmixed.rpf was not checked for GTA V Legacy versions in full: its nested archive packed.rpf is compressed; 1 archive nested in it is not OPEN\.\$'"
check "8 levels deep: read, the Legacy model found" "log_line 'The pack mods.lpdeep8.rpf was made for GTA V Legacy: 1 yft 162 - the first lp.yft in its nested archive l1.rpf/l2.rpf/l3.rpf/l4.rpf/l5.rpf/l6.rpf/l7.rpf/l8.rpf\.\$' && log_line 'DIAGNOSTIC: the pack mods.lpdeep8.rpf: 9 archives (8 nested), 18 entries; 1 yft 162\.\$'"
check "9 levels deep: the ninth not read, the pack not checked in full" "log_line 'The pack mods.lpdeep9.rpf was not checked for GTA V Legacy versions in full: its nested archive l1.rpf/l2.rpf/l3.rpf/l4.rpf/l5.rpf/l6.rpf/l7.rpf/l8.rpf/l9.rpf is nested deeper than 8 levels\.\$' && log_lacks 'lpdeep9.rpf was made'"
check "one problem: the Legacy packs, then the damaged ones - not the one with archives that are not OPEN alone; the last line counts it" "log_line 'PROBLEM: 2 packs in the mods folder were made for GTA V Legacy, .*: mods.lpdeep8.rpf, mods.lpnest.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.lpdeep9.rpf, mods.lpmixed.rpf\.\$' && one_finding && log_line 'Packs checked for GTA V Legacy versions: 4 in the mods folder, 2 made for Legacy, 3 not checked (1 of them not OPEN)\.\$'"

echo "== L4 packs that are not OPEN - an NG and an AES header: named as not checked in their lines, not in a problem"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake ng mods/lpng.rpf lpmake aes mods/lpaes.rpf lpwait)
check "each named with its encryption" "log_line 'The pack mods.lpng.rpf was not checked for GTA V Legacy versions: it is not OPEN (encryption 0x0FEFFFFF, NG) - only archives that are not encrypted are read, and the game.s own archives are encrypted\.\$' && log_line 'The pack mods.lpaes.rpf was not checked for GTA V Legacy versions: it is not OPEN (encryption 0x0FFFFFF9, AES) - only archives that are not encrypted are read, and the game.s own archives are encrypted\.\$'"
check "no problem, no box; the last line counts them" "log_lacks 'PROBLEM' && log_lacks 'not shown (ERROR_POPUP=0)' && log_line 'Packs checked for GTA V Legacy versions: 2 in the mods folder, none made for Legacy, 2 not checked (2 of them not OPEN)\.\$'"

echo "== L4b a pack whose encryption field holds 0, which names no encryption: its table read as it is"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake plain mods/lpplain.rpf lpwait)
check "read and named as made for Legacy, one box" "log_line 'The pack mods.lpplain.rpf was made for GTA V Legacy: 2 yft 162, 1 ydr 165, 1 ydd 165, 1 ytd 13 - the first lp.yft\.\$' && log_lacks 'not OPEN' && one_finding && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy\.\$'"

echo "== L5 damaged packs, one of each kind: each named as not checked with its reason; nothing read past the damage"
packs_gone; fresh_ini; args=()
for k in empty badmagic nocount toomany pastend pastnames noroot badname nestedout nestedself compressed notrpf rootonly; do args+=(lpmake $k mods/damaged/$k.rpf); done
out=$(game full_game.exe "${args[@]}" lpwait lpfree)
NOT='was not checked for GTA V Legacy versions'
check "13 archives made; the check's last line come" "has 'lp: 13 made |' && has '$DONE'"
check "empty, and a zip's first bytes" "log_line 'The pack mods.damaged.empty.rpf $NOT: it is shorter than an archive.s header (0 bytes)\.\$' && log_line 'The pack mods.damaged.badmagic.rpf $NOT: it is not an RPF7 archive (its first bytes are 50 4B 03 04)\.\$'"
check "no entries, and more than 1,048,576" "log_line 'The pack mods.damaged.nocount.rpf $NOT: it has no entries\.\$' && log_line 'The pack mods.damaged.toomany.rpf $NOT: it has 1048577 entries, more than 1048576\.\$'"
check "an entry table cut off, and names that run past the end" "log_line 'The pack mods.damaged.pastend.rpf $NOT: it has an entry table and names that run past its end (192 of 64 bytes)\.\$' && log_line 'The pack mods.damaged.pastnames.rpf $NOT: it has an entry table and names that run past its end (2147483760 of 1024 bytes)\.\$'"
check "no root folder; a name past the names, the table read up to it" "log_line 'The pack mods.damaged.noroot.rpf $NOT: it has no folder as its first entry\.\$' && log_line 'The pack mods.damaged.badname.rpf $NOT in full: it has a name outside its names (entry 1)\.\$' && log_lacks 'badname.rpf was made'"
check "a nested archive past the end, the Legacy texture dictionary beside it still found" "log_line 'The pack mods.damaged.nestedout.rpf $NOT in full: its nested archive out.rpf lies outside the archive that holds it\.\$' && log_line 'The pack mods.damaged.nestedout.rpf was made for GTA V Legacy: 1 ytd 13 - the first lp.ytd\.\$'"
check "a nested archive at its holder's own header, a compressed one, one that is a zip" "log_line 'The pack mods.damaged.nestedself.rpf $NOT in full: its nested archive self.rpf lies outside the archive that holds it\.\$' && log_line 'The pack mods.damaged.compressed.rpf $NOT in full: its nested archive packed.rpf is compressed\.\$' && log_line 'The pack mods.damaged.notrpf.rpf $NOT in full: its nested archive junk.rpf is not an RPF7 archive (its first bytes are 50 4B 03 04)\.\$'"
check "the root folder alone: read, no line" "log_lacks 'rootonly'"
check "one problem: the Legacy pack, then the first 10 not checked in the walk's order and the 2 more" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: mods.damaged.nestedout.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.damaged.badmagic.rpf, mods.damaged.badname.rpf, mods.damaged.compressed.rpf, mods.damaged.empty.rpf, mods.damaged.nestedout.rpf, mods.damaged.nestedself.rpf, mods.damaged.nocount.rpf, mods.damaged.noroot.rpf, mods.damaged.notrpf.rpf, mods.damaged.pastend.rpf, and 2 more\.\$' && one_finding"
check "the last line" "log_line 'Packs checked for GTA V Legacy versions: 13 in the mods folder, 1 made for Legacy, 12 not checked\.\$'"
check "every pack and its folder free once the check is done, opened without sharing: no handle left open on any kind of damage" "has 'lp: all 13 free |'"

echo "== L6 the bounds of a pack: 4,096 archives read, 4,097 not; 64 MB of entry tables and names"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe lpmake manynestedok mods/bounds/manyok.rpf lpmake manynested mods/bounds/many.rpf lpmake bigtables mods/bounds/big.rpf lpwait lpfree)
check "the pack and 4,095 archives nested in it: read" "log_line 'DIAGNOSTIC: the pack mods.bounds.manyok.rpf: 4096 archives (4095 nested), 12286 entries; 4095 ytd 5\.\$' && log_lacks 'manyok.rpf was not'"
check "4,096 nested: the pack not checked in full, none of them read" "log_line 'The pack mods.bounds.many.rpf was not checked for GTA V Legacy versions in full: it holds more than 4096 archives\.\$' && log_line 'DIAGNOSTIC: the pack mods.bounds.many.rpf: 1 archive, 4097 entries; no yft, ydr, ydd or ytd\.\$'"
check "nested archives of 16 MB of entry table: three read, the fourth past 64 MB" "log_line 'The pack mods.bounds.big.rpf was not checked for GTA V Legacy versions in full: its entry tables and names pass 64 MB\.\$' && log_line 'DIAGNOSTIC: the pack mods.bounds.big.rpf: 4 archives (3 nested), 3145734 entries; no yft, ydr, ydd or ytd\.\$'"
check "the one problem names the two" "log_line 'PROBLEM: 2 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.bounds.big.rpf, mods.bounds.many.rpf\.\$' && one_finding"
check "every pack and its folder free once the check is done: no handle left open past a bound" "has 'lp: all 3 free |'"

echo "== L7 the whole check's bound, 1 GB of entry tables and names: 23 packs of 48 MB - the 22nd read up to the bound, the 23rd not at all"
packs_gone; fresh_ini; out=$(game full_game.exe lpmany heavy 23 mods/heavy lpwait)
check "the archives made; the check's last line come" "has 'lp: 23 made |' && has '$DONE'"
check "the first 21 read in full, so no line of theirs" "[ \$(count 'mods.heavy.p000') -eq 3 ]"
check "the 22nd read up to the bound, the 23rd not read" "log_line 'The pack mods.heavy.p00022.rpf was not checked for GTA V Legacy versions in full: the check had read 1024 MB of entry tables and names before it\.\$' && log_line 'The pack mods.heavy.p00023.rpf was not checked for GTA V Legacy versions: the check had read 1024 MB of entry tables and names before it\.\$'"
check "the one problem names both; the last line counts the 23" "log_line 'PROBLEM: 2 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.heavy.p00022.rpf, mods.heavy.p00023.rpf\.\$' && one_finding && log_line 'Packs checked for GTA V Legacy versions: 23 in the mods folder, none made for Legacy, 2 not checked\.\$'"

echo "== L8 20,001 packs made for Legacy, DIAGNOSTIC_MODE=1: each kind of line stops at 20,000, the rest counted; the start-up does not wait for the check"
packs_gone; fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey UNLOCK_FILES 0 DIRECTSTORAGE; out=$(game full_game.exe lpmany legacy 20001 mods/many lpwait)
check "the archives made; the check's last line come" "has 'lp: 20001 made |' && has '$DONE'"
check "20000 lines of Legacy packs, then the cap line once" "[ \$(count 'was made for GTA V Legacy: ') -eq 20000 ] && [ \$(count '20000 lines of packs made for GTA V Legacy written - from now on they are only counted\.') -eq 1 ]"
check "20000 lines of packs read, then the cap line once" "[ \$(count 'DIAGNOSTIC: the pack mods') -eq 20000 ] && [ \$(count 'DIAGNOSTIC: 20000 lines of packs read written - from now on they are only counted\.') -eq 1 ]"
check "the problem names the first 10 and counts the rest; the last line counts every pack" "log_line 'PROBLEM: 20001 packs in the mods folder were made for GTA V Legacy, .*: mods.many.p00001.rpf, .*, mods.many.p00010.rpf, and 19991 more\.\$' && log_line 'Packs checked for GTA V Legacy versions: 20001 in the mods folder, 20001 made for Legacy\.\$'"
check "the start-up went on while the check read: the OPEN archives set up before the check's last line, its finding in a box of its own" "[ \$(first_line 'Unencrypted (OPEN) RPF archives can be loaded') -lt \$(first_line 'Packs checked for GTA V Legacy versions') ] && one_finding"

echo "== L9 20,001 packs that cannot be read: the lines stop at 20,000, the problem names 10"
packs_gone; fresh_ini; out=$(game full_game.exe lpmany empty 20001 mods/many lpwait)
check "20000 lines, then the cap line once; the problem and the last line count every pack" "[ \$(count 'was not checked for GTA V Legacy versions: it is shorter than an archive.s header (0 bytes)\.') -eq 20000 ] && [ \$(count '20000 lines of packs not checked written - from now on they are only counted\.') -eq 1 ] && log_line 'PROBLEM: 20001 packs in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.many.p00001.rpf, .*, and 19991 more\.\$' && log_line 'Packs checked for GTA V Legacy versions: 20001 in the mods folder, none made for Legacy, 20001 not checked\.\$'"

echo "== L10 a pack another program holds without sharing: named as not checked, with the Windows error"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake legacy mods/held/lp.rpf lplock mods/held/lp.rpf lpwait lpfree)
check "its line and the problem; one box" "log_line 'The pack mods.held.lp.rpf was not checked for GTA V Legacy versions: it could not be opened (Windows error 32, file in use)\.\$' && log_line 'PROBLEM: A pack in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.held.lp.rpf\.\$' && one_finding"
check "the fake's own handle on it shows as held, so 'free' in L5 and L6 means no handle is open" "has 'lp: 1 of 1 held |'"

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
check "one problem, one box" "[ \$(count 'PROBLEM') -eq 1 ] && log_has 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: mods.lp.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.lpempty.rpf\.' && [ \$(count 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): ') -eq 1 ] && log_line 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: its check of the packs in the mods folder found: \.\.\. The game continues with the packs as they are\.\$' && [ \$(count 'Showing a finding (1 finding(s))\.') -eq 1 ] && log_lacks 'Showing a problem' && log_lacks 'Showing the start-up summary'"

echo "== L15 the walk's limits, as RageOpenV's list has them: a path below mods\ of 254 bytes read, one of 255 not; a folder that cannot be listed skipped, the rest read; a name with an unpaired surrogate left out"
# test\ and these and .rpf: 254 and 255 bytes - the first in letters of 3 bytes in UTF-8 and one unit in UTF-16, since 254 of
# ASCII pass MAX_PATH wherever the folder lies, and the check reads only what the game can open (L15b)
a254=aa$(printf '\xe2\x82\xac%.0s' $(seq 1 81)); b255=$(printf 'b%.0s' $(seq 1 246))
packs_gone; fresh_ini; out=$(game full_game.exe lpmake legacy "mods/test/$a254.rpf" lpmake legacy "mods/test/$b255.rpf" lpwait)
check "254 bytes: listed, read and named; 255: left out of the list and not read" "has 'lp: 2 made |' && log_has \"The pack mods.test.$a254.rpf was made for GTA V Legacy\" && log_has 'Names in the mods folder left out of the list: 1 whose path is longer than 254 bytes.' && log_lacks 'bbbbbbbbbb' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy\.\$'"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake legacy mods/locked/in.rpf lpmake legacy mods/open/in.rpf lpwait)
check "two packs in two folders, both named while both can be listed" "log_has 'The pack mods.locked.in.rpf was made for GTA V Legacy' && log_has 'The pack mods.open.in.rpf was made for GTA V Legacy'"
fresh_ini; deny_listing mods/locked; out=$(game full_game.exe lpwait); allow_writes mods/locked
check "the folder that cannot be listed: RageOpenV reports it, the check skips its pack, reads the rest and says how many it could not read" "has '$DONE' && log_has 'PROBLEM: Only part of the mods folder is used this session: 1 folder could not be read (Windows error 5, access denied).' && log_lacks 'mods.locked.in.rpf' && log_has 'The pack mods.open.in.rpf was made for GTA V Legacy' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy, 1 folder could not be read\.\$'"
packs_gone; fresh_ini; out=$(game full_game.exe lpbadname lpwait)
if has 'lp: bad name NOT made'; then
	skip "this file system refuses a name with an unpaired surrogate, so none can be left out"
else
	check "a pack whose name is not valid UTF-16: left out of the list and not read; the name gone with the fake" "has 'lp: bad name made |' && log_has 'Names in the mods folder left out of the list: 1 whose name is not valid Unicode' && log_line 'Packs checked for GTA V Legacy versions: none in the mods folder\.\$' && [ -z \"\$(ls -A mods | grep -v '^test\$')\" ]"
fi

echo "== L15b past MAX_PATH the check reaches what the game reaches, by the same paths: a folder Windows will not list, a pack it will not open"
# test\ and these and .rpf: 254 bytes, past MAX_PATH wherever the folder lies; a name of a folder that takes its path past it there too
a254=$(printf 'a%.0s' $(seq 1 245)); far=$(printf 'f%.0s' $(seq 1 240))
packs_gone; fresh_ini; out=$(game full_game.exe lpmake legacy "mods/test/$a254.rpf" lpmake legacy "mods/$far/in.rpf" lpwait)
check "the folder: RageOpenV logs and reports it, by the Windows error; the check skips it, so its pack is not read" "has 'lp: 2 made |' && has '$DONE' && log_has 'The folder mods.$far could not be read (Windows error [0-9]*.*) - its files are not used\.' && log_has 'PROBLEM: Only part of the mods folder is used this session: 1 folder could not be read (Windows error' && log_lacks 'in\.rpf'"
check "the pack, listed by both: not opened as the game's file functions would not open it, named as not checked; no verdict on a pack the game cannot read" "log_line 'The pack mods.test.$a254.rpf was not checked for GTA V Legacy versions: it could not be opened (Windows error [0-9]*.*)\.\$' && log_lacks 'was made for GTA V Legacy' && log_line 'PROBLEM: A pack in the mods folder could not be checked for GTA V Legacy versions (TweaksV.log says why): mods.test.$a254.rpf\. Folders not read, so their packs not checked (TweaksV.log says why): mods.$far\.\$' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, none made for Legacy, 1 not checked, 1 folder could not be read\.\$'"

echo "== L16 the names of a pack within their bounds: a name with no zero within 256 bytes, a name of 255 bytes at the last offset in names of 70,000 bytes, a nested archive whose size runs past its holder"
packs_gone; fresh_ini; out=$(game full_game.exe lpmake longname mods/names/longname.rpf lpmake farname mods/names/farname.rpf lpmake nestedlong mods/names/nestedlong.rpf lpwait)
check "no zero within 256 bytes: the name refused, nothing of it counted" "log_line 'The pack mods.names.longname.rpf was not checked for GTA V Legacy versions in full: it has a name outside its names (entry 1)\.\$' && log_lacks 'longname.rpf was made'"
check "a name of 255 bytes at offset 65,535, its zero at 65,790 inside the 65,792 bytes read: the pack named by it" "log_line 'The pack mods.names.farname.rpf was made for GTA V Legacy: 1 yft 162 - the first x*\.yft\.\$'"
check "a nested archive whose size runs past its holder's end: refused, as one that starts past it" "log_line 'The pack mods.names.nestedlong.rpf was not checked for GTA V Legacy versions in full: its nested archive long.rpf lies outside the archive that holds it\.\$'"
check "the one problem and the last line" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: mods.names.farname.rpf\. Not checked for Legacy versions (TweaksV.log says why): mods.names.longname.rpf, mods.names.nestedlong.rpf\.\$' && one_finding && log_line 'Packs checked for GTA V Legacy versions: 3 in the mods folder, 1 made for Legacy, 2 not checked\.\$'"

echo "== L17 the walk's limit, lowered by the test build's TWEAKSV_TEST_MODS_ENTRIES: RageOpenV's list and the check stop at the same name"
# alate, stop and test listed in that order, test read first - its file - then stop: a name past 254 bytes, left out and not
# counted, then the packs. 7 names end at p00003.rpf, in the middle of stop's names, and alate is never read.
past=$(printf 'a%.0s' $(seq 1 246))	# stop\ and these and .rpf: 255 bytes
packs_gone; fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=7 ASI=TweaksV_test.asi game full_game.exe lpmake legacy mods/alate/in.rpf lpmake legacy "mods/stop/$past.rpf" lpmany legacy 5 mods/stop rovask stop/p00003.rpf rovask stop/p00004.rpf rovask alate/in.rpf lpwait)
check "the archives made; the test build's line once, for both walks; the check's last line come" "has 'lp: 7 made |' && has '$DONE' && [ \$(count 'TEST BUILD: the walk of the mods folder stops at 7 files and folders instead of 200000 (TWEAKSV_TEST_MODS_ENTRIES=7)\.') -eq 1 ]"
check "RageOpenV's list: 7 names, its problem says where it stopped; the name past 254 bytes left out, not counted" "log_has 'The mods folder has 7 files and folders\.' && log_has 'PROBLEM: Only part of the mods folder is used this session: the list stopped at 7 files and folders\.' && log_has 'Names in the mods folder left out of the list: 1 whose path is longer than 254 bytes\.'"
check "the check: the packs before the stop read, its last line says where it stopped" "log_line 'Packs checked for GTA V Legacy versions: 3 in the mods folder, 3 made for Legacy - the walk stopped at 7 files and folders, where RageOpenV.s list stops\.\$' && [ \$(count 'PROBLEM') -eq 2 ]"
check "the same name: p00003.rpf served and named, p00004.rpf neither" "has 'ask 1 served 8/8 passed 0/8' && log_has 'The pack mods.stop.p00003.rpf was made for GTA V Legacy' && has 'ask 2 served 0/8 passed 8/8' && log_lacks 'p00004'"
check "the folder the walk did not come to: its file not served, its pack not read" "has 'ask 3 served 0/8 passed 8/8' && log_lacks 'mods.alate.in\.rpf'"
fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=0 ASI=TweaksV_test.asi game full_game.exe lpwait)
check "a value outside 1..200000 is not taken: no line, nothing stopped" "has '$DONE' && log_lacks 'TEST BUILD: the walk' && log_lacks 'the walk stopped' && log_line 'Packs checked for GTA V Legacy versions: 6 in the mods folder, 6 made for Legacy\.\$'"
fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=7 game full_game.exe lpwait)
check "the Release build leaves the limit as it is: no line, nothing stopped" "has '$DONE' && log_lacks 'TEST BUILD' && log_lacks 'the list stopped' && log_line 'Packs checked for GTA V Legacy versions: 6 in the mods folder, 6 made for Legacy\.\$'"

echo "== L18 a listing that fails half-way, a share that drops, through the test build's TWEAKSV_TEST_MODS_FAIL_LISTING: its names so far stand, the rest is said not to be used, the walk goes on"
# after, half and test listed in that order, so test and half read before after
packs_gone; fresh_ini; out=$(TWEAKSV_TEST_MODS_FAIL_LISTING='mods\half' ASI=TweaksV_test.asi game full_game.exe lpmany legacy 3 mods/half lpmake legacy mods/after/in.rpf rovask half/p00001.rpf rovask half/p00002.rpf rovask after/in.rpf lpwait)
check "the test build's line once, for both walks; RageOpenV logs and reports the folder, the rest of its files not used" "has 'lp: 4 made |' && has '$DONE' && [ \$(count 'TEST BUILD: the listing of mods.half fails after its first name with Windows error 64 (TWEAKSV_TEST_MODS_FAIL_LISTING=mods.half)\.') -eq 1 ] && log_has 'The folder mods.half could not be read to its end (Windows error 64) - the rest of its files are not used\.' && log_has 'PROBLEM: Only part of the mods folder is used this session: 1 folder could not be read (Windows error 64)\.'"
check "the names so far stand, for both: p00001.rpf served and named, p00002.rpf neither" "has 'ask 1 served 8/8 passed 0/8' && log_has 'The pack mods.half.p00001.rpf was made for GTA V Legacy' && has 'ask 2 served 0/8 passed 8/8' && log_lacks 'p00002'"
check "the walk goes on: the folder read after it served and named; the last line counts what was read and the folder not read to its end" "has 'ask 3 served 8/8 passed 0/8' && log_has 'The pack mods.after.in.rpf was made for GTA V Legacy' && log_line 'Packs checked for GTA V Legacy versions: 2 in the mods folder, 2 made for Legacy, 1 folder could not be read\.\$'"
fresh_ini; out=$(TWEAKSV_TEST_MODS_FAIL_LISTING=mods ASI=TweaksV_test.asi game full_game.exe rovask after/in.rpf rovask half/p00001.rpf lpwait)
check "the mods folder's own listing, after its first name: the same - that folder served and read, the rest not, the check done" "has '$DONE' && log_has 'The folder mods could not be read to its end (Windows error 64) - the rest of its files are not used\.' && log_has 'PROBLEM: Only part of the mods folder is used this session: 1 folder could not be read (Windows error 64)\.' && has 'ask 1 served 8/8 passed 0/8' && has 'ask 2 served 0/8 passed 8/8' && log_lacks 'is not done this session' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy, 1 folder could not be read\.\$'"
first=$(printf 'a%.0s' $(seq 1 255))	# a name of 255 bytes, left out: the first the listing gives, and the only one
packs_gone; touch "mods/$first"; fresh_ini; out=$(TWEAKSV_TEST_MODS_FAIL_LISTING=mods ASI=TweaksV_test.asi game full_game.exe)
check "the mods folder's listing with no name handed on: a mods folder that cannot be read, not called empty; nothing hooked" "log_has 'PROBLEM: Files in the mods folder are not used this session: the mods folder could not be read (Windows error 64)\.' && log_lacks 'is empty' && log_lacks 'could not be read to its end' && has 'file hooks 0/8 |'"
packs_gone; fresh_ini; out=$(TWEAKSV_TEST_MODS_FAIL_LISTING='mods\half' game full_game.exe lpmany legacy 3 mods/half lpmake legacy mods/after/in.rpf rovask half/p00001.rpf rovask half/p00002.rpf rovask after/in.rpf lpwait)
check "the Release build ignores the switch: no line, nothing stopped - all 4 served and named, counted in the last line" "has 'lp: 4 made |' && has '$DONE' && log_lacks 'TEST BUILD' && log_lacks 'could not be read to its end' && log_lacks 'Only part of the mods folder' && has 'ask 1 served 8/8 passed 0/8' && has 'ask 2 served 8/8 passed 0/8' && has 'ask 3 served 8/8 passed 0/8' && log_has 'The pack mods.half.p00002.rpf was made for GTA V Legacy' && log_line 'Packs checked for GTA V Legacy versions: 4 in the mods folder, 4 made for Legacy\.\$'"
packs_gone

echo "== L19 folders only the check's own walk cannot list, through the test build's TWEAKSV_TEST_LEGACY_FOLDER: each a line, the one problem names them, the last line says how many"
OTHER="$(cygpath -w "$B/other")"
other_game() { TWEAKSV_TEST_LEGACY_FOLDER="$OTHER" ASI=TweaksV_test.asi game full_game.exe "$@"; }
packs_read_later='its packs were not checked for GTA V Legacy versions\.'
packs_gone; fresh_ini; out=$(other_game lpmake legacy other/locked/in.rpf lpmake legacy other/open/in.rpf lpwait)
check "made and read while they can be listed: both packs named, no folder" "has 'lp: 2 made |' && log_has 'other.locked.in.rpf was made for GTA V Legacy' && log_has 'other.open.in.rpf was made for GTA V Legacy' && log_lacks 'could not be read'"
rm -rf other/open; fresh_ini; deny_listing other/locked; out=$(other_game lpwait); allow_writes other/locked
check "the folder's line, word for word; RageOpenV lists its own folder and has nothing to report" "has '$DONE' && log_line 'The folder .*other.locked could not be read (Windows error 5, access denied) - $packs_read_later\$' && log_lacks 'Only part of the mods folder'"
check "every pack in such a folder: the one problem names it, and the last line no longer says there is none in the mods folder" "log_line 'PROBLEM: A folder in the mods folder could not be read, so the packs in it were not all checked for GTA V Legacy versions (TweaksV.log says why): .*other.locked\.\$' && [ \$(count 'PROBLEM') -eq 1 ] && log_line 'Packs checked for GTA V Legacy versions: none in the folders read, 1 folder could not be read\.\$' && log_lacks 'in.rpf' && one_finding"
rm -rf other/locked; fresh_ini; out=$(TWEAKSV_TEST_MODS_FAIL_LISTING="$OTHER\\half" other_game lpmany legacy 3 other/half lpwait)
check "a listing that fails half-way for the check alone: the pack before it read and named, the rest said not to be checked; RageOpenV has nothing to report" "has '$DONE' && log_has 'other.half.p00001.rpf was made for GTA V Legacy' && log_lacks 'p00002' && log_line 'The folder .*other.half could not be read to its end (Windows error 64) - the rest of its packs were not checked for GTA V Legacy versions\.\$' && log_lacks 'Only part of the mods folder'"
check "the one problem: the Legacy pack, then the folder; the last line counts both" "log_line 'PROBLEM: A pack in the mods folder was made for GTA V Legacy, .*: .*other.half.p00001.rpf\. Folders not read, so their packs not checked (TweaksV.log says why): .*other.half\.\$' && log_line 'Packs checked for GTA V Legacy versions: 1 in the mods folder, 1 made for Legacy, 1 folder could not be read\.\$' && one_finding"
rm -rf other/half; folders=''; for n in 01 02 03 04 05 06 07 08 09 10 11; do folders+=" lpmake legacy other/d$n/in.rpf"; done
fresh_ini; out=$(other_game $folders lpwait); for n in 01 02 03 04 05 06 07 08 09 10 11; do deny_listing other/d$n; done
fresh_ini; out=$(other_game lpwait); for n in 01 02 03 04 05 06 07 08 09 10 11; do allow_writes other/d$n; done
check "eleven such folders: a line each, the problem names ten and counts the rest, in the plural; the last line counts all" "[ \$(count 'could not be read (Windows error 5, access denied) - $packs_read_later') -eq 11 ] && log_line 'PROBLEM: 11 folders in the mods folder could not be read, so the packs in them were not all checked for GTA V Legacy versions (TweaksV.log says why): .*, and 1 more\.\$' && log_line 'Packs checked for GTA V Legacy versions: none in the folders read, 11 folders could not be read\.\$' && one_finding"
rm -rf other; packs_gone

echo "== L20 names left out count against a second limit, 4 times the walk's, lowered with it by TWEAKSV_TEST_MODS_ENTRIES: 12 names, the 13th ends the walk, for RageOpenV's list and the check alike"
# the mods folder's names first, the long ones and test, then test's one file: with 3 files and folders the walk may look at
# 12 names, so 10 long ones leave room for test/file.txt and 11 do not
long_names() { for n in $(seq -w 1 "$1"); do touch "mods/$n$(printf 'a%.0s' $(seq 1 253))"; done; }
packs_gone; long_names 10; fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=3 ASI=TweaksV_test.asi game full_game.exe rovask test/file.txt lpwait)
check "12 names, 10 of them left out: nothing stopped, the left out counted as before, test/file.txt served" "has '$DONE' && log_has 'The mods folder has 2 files and folders\.' && log_has 'Names in the mods folder left out of the list: 10 whose path is longer than 254 bytes\.' && has 'ask 1 served 8/8 passed 0/8' && log_lacks 'stopped at' && log_line 'Packs checked for GTA V Legacy versions: none in the mods folder\.\$'"
packs_gone; long_names 11; fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=3 ASI=TweaksV_test.asi game full_game.exe rovask test/file.txt lpwait)
check "the 13th name ends the walk: RageOpenV's problem says at what, test/file.txt not served" "has '$DONE' && log_has 'PROBLEM: Only part of the mods folder is used this session: the list stopped at 12 names, most of them too long or not valid Unicode\.' && has 'ask 1 served 0/8 passed 8/8' && log_has 'The mods folder has 1 files and folders\.'"
check "the check stops at the same name, and its last line says so" "log_line 'Packs checked for GTA V Legacy versions: none in the mods folder - the walk stopped at 12 names, most of them too long or not valid Unicode, where RageOpenV.s list stops\.\$'"
packs_gone; long_names 11; fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=3 game full_game.exe rovask test/file.txt lpwait)
check "the Release build keeps the limit of 200000 and 800000 names: nothing stopped" "has '$DONE' && log_lacks 'stopped at' && has 'ask 1 served 8/8 passed 0/8'"
packs_gone; for n in 1 2 3 4 5; do touch "mods/$n$(printf 'a%.0s' $(seq 1 254))"; done; fresh_ini; out=$(TWEAKSV_TEST_MODS_ENTRIES=1 ASI=TweaksV_test.asi game full_game.exe)
check "the walk stopped before a name it could use, its 4 names all too long: one problem, that none can be used, and the line it points to right above it; nothing hooked" "log_has 'The mods folder has 0 files and folders\.' && [ \$(count '\[RageOpenV\] PROBLEM') -eq 1 ] && log_has 'PROBLEM: Files in the mods folder are not used this session: none of the 4 names in the mods folder can be used (see the line above)\.' && tr -d '\r' < TweaksV.log | grep '\[RageOpenV\]' | grep -B 1 'none of the 4 names' | head -n 1 | grep -q 'left out of the list: 4 ' && has 'file hooks 0/8'"
packs_gone

echo
echo "LEGACY PACKS SCENARIOS: $pass passed, $fail failed"
