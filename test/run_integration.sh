#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# TweaksV as a whole, in the fake game: old plugins present, NaturalVision Enhanced's loader present, one
# pattern deliberately broken per plugin with the TEST build, another game exe, no C runtime DLLs, a renamed
# .asi, no code changes allowed, the user name kept out of the log, the log's limit, links, a read-only game
# folder, a held log, the code dump, the copy of a script program, a half of WeaponLimits that throws. Every part
# on, PedProps too, which ships off (PEDPROPS_ON).
# Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=196
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata mods/test scripts plugins
PEDPROPS_ON=1
echo hi > mods/test/file.txt

ALL_ON_HEAP='heap: 1024 MB'
GAME_HEAP='heap: 648 MB'
STORY_ON='splash: E9 86 01 00 00 90 | legal: 90 90 90 90 90 90 | landing: 31 C0 C3 24 08 |'
STORY_OFF='splash: 0F 84 85 01 00 00 | legal: 0F 84 07 05 00 00 | landing: 48 89 5C 24 08 |'
MODKIT_ON='modkit refs: new array (RW, 128 KB) | limits: 65535 65539 '
MODKIT_OFF='modkit refs: game array | limits: 1023 1027 '
PACK_ON='pack refs: 32 new 0 game 0 other, R31a new, R31b game (RW, 512 KB) | pack sizes: D3=12000 '
PACK_OFF='pack refs: 0 new 32 game 0 other, R31a game, R31b game | pack sizes: D3=4500 D2=4500 D1=4500 I=4496 | hook: not set'
BOX_ON='box refs: 8 new 0 game 0 other (RW, 784 KB) | box caps: 10000 10000 10000 10000 |'
BOX_OFF='box refs: 0 new 8 game 0 other | box caps: 1000 1000 1000 1000 |'
DECAL_ON='decal refs: 31 new 0 game 0 other (RW, 1280 KB) | decal limits: 2047 2048 0x28198, 23 agree'
DECAL_OFF='decal refs: 0 new 31 game 0 other | decal limits: 259 260 0x5918, 23 agree'
ROV_ON='file hooks 8/8, redirect 8/8, pass 8/8, spellings ok, folder ok, outside ok | open calls 5/5, behaviour ok'
ROV_OFF='rov api hook: no'
summary_once() { [ "$(count 'Start-up summary not shown (ERROR_POPUP=0)')" -eq 1 ]; }
# nve_item SUB PART SECTION: how many lines of TweaksV.log are, word for word, the start-up summary's item for
# onigiri.asi, NaturalVision Enhanced's loader, in the game folder (SUB "") or in its subfolder SUB ("scripts.")
nve_item() { tr -d '\r' < TweaksV.log | grep -c -- "PROBLEM: .*TestRun.$1onigiri.asi is installed and does this job too (NaturalVision Enhanced.s loader): TweaksV.s $2 part is switched off while it is there, so nothing is patched twice. To stop this message, set ENABLED=0 under \\[$3\\] in TweaksV.ini.\$"; }
# nve_line PART: how many lines of TweaksV.log are, word for word, the conflict check's line for onigiri.asi in the
# game folder next to PART switched off in TweaksV.ini
nve_line() { tr -d '\r' < TweaksV.log | grep -c -- "\\[TweaksV\\] Conflict check: .*TestRun.onigiri.asi is installed; TweaksV.s $1 part is off in TweaksV.ini already.\$"; }
# re_item PART SECTION: how many lines of TweaksV.log are, word for word, the start-up summary's item for
# ResourceExtensions.asi, Resource Extensions [Enhanced], in the game folder
re_item() { tr -d '\r' < TweaksV.log | grep -c -- "PROBLEM: .*TestRun.ResourceExtensions.asi is installed and does this job too (Resource Extensions .Enhanced.): TweaksV.s $1 part is switched off while it is there, so nothing is patched twice. To stop this message, set ENABLED=0 under \\[$2\\] in TweaksV.ini.\$"; }
# features_say [PART...]: the log header's Features: line says off for each PART and on for every other part
features_say() {
	local want="Features:" part sep=" "
	for part in HeapAdjuster ModkitLimit PackfileLimit BoxStreamer Decals RageOpenV StraightToStoryMode DirectStorage Blips WeaponLimits PedProps Resources RayTracing SirenSettings StaticBounds; do
		if [[ " $* " == *" $part "* ]]; then want+="$sep$part off"; else want+="$sep$part on"; fi
		sep=", "
	done
	tr -d '\r' < TweaksV.log | grep -qx -- ".* \[TweaksV\] $want"
}
# after_header PATTERN: the first line of TweaksV.log that matches PATTERN stands after the header, whose last line
# in a release build holds the values of [STATICBOUNDS]
after_header() {
	local header line
	header=$(grep -n -m 1 -- '\[TweaksV\] StaticBounds: STATICBOUNDS_SIZE=' TweaksV.log | cut -d: -f1)
	line=$(grep -n -m 1 -- "$1" TweaksV.log | cut -d: -f1)
	[ -n "$header" ] && [ -n "$line" ] && [ "$line" -gt "$header" ]
}

echo "== I1 each old plugin on its own: only its part is skipped, and the log header says so"
for old in HeapAdjuster.asi ZHeapAdjuster.asi ModkitLimitAdjusterEnhanced.asi PackfileLimitAdjusterEnhanced.asi DecalPatch.asi RageOpenV.asi OpenRPF.asi OpenIV.asi StraightToStoryMode.asi DirectStorageFix.asi StaticBoundLimitAdjuster.asi StaticBoundsLimitAdjuster.asi WeaponLimitsAdjusterEnhanced.asi GTAV.ResourceAdjusterEnhanced.asi; do
	fresh_ini; touch "$old"; out=$(game full_game.exe); rm -f "$old"
	case "$old" in
		*HeapAdjuster.asi) off=HeapAdjuster; ok="has '$GAME_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON'" ;;
		DecalPatch.asi) off="BoxStreamer Decals"; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_OFF' && has '$DECAL_OFF' && has '$ROV_ON' && has '$STORY_ON' && [ \$(count 'Old plugin found: .*DecalPatch.asi') -eq 2 ]" ;;
		Modkit*) off=ModkitLimit; ok="has '$ALL_ON_HEAP' && has '$MODKIT_OFF' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'" ;;
		Packfile*) off=PackfileLimit; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_OFF' && has '$ROV_ON' && has '$STORY_ON'" ;;
		RageOpenV.asi|OpenRPF.asi|OpenIV.asi) off=RageOpenV; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_OFF' && has '$STORY_ON'" ;;
		StraightToStoryMode.asi) off=StraightToStoryMode; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_OFF'" ;;
		DirectStorageFix.asi) off=DirectStorage; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON' && log_lacks '\[DirectStorage\]'" ;;
		StaticBound*) off=StaticBounds; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON' && log_lacks '\[StaticBounds\]'" ;;
		WeaponLimits*) off=WeaponLimits; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON' && log_lacks '\[WeaponLimits\]'" ;;
		GTAV.Resource*) off=Resources; ok="has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON' && log_lacks '\[Resources\]'" ;;
	esac
	check "$old: its part skipped, the rest applied, reported once in the summary" "$ok && log_has 'Old plugin found: .*$old' && summary_once"
	check "$old: the header's Features: line says off for ${off// /, } and on for the rest, the check's line stands after the header" "features_say $off && after_header 'Old plugin found'"
done

echo "== I1b onigiri.asi, NaturalVision Enhanced's loader, alone in the game folder, scripts\\ and plugins\\: RageOpenV and StraightToStoryMode off while it is there"
for dir in "" scripts plugins; do
	fresh_ini; touch "${dir:-.}/onigiri.asi"; out=$(game full_game.exe); rm -f "${dir:-.}/onigiri.asi"
	at=${dir:+$dir\\}; at=${at:-the game folder}; sub=${dir:+$dir.}
	check "in $at: RageOpenV and StraightToStoryMode off, the other six parts applied, DirectStorage on" "has '$ROV_OFF' && has '$STORY_OFF' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && log_has '\[DirectStorage\]'"
	check "in $at: one item per part, word for word, the only two problems of one summary" "[ \$(nve_item '$sub' RageOpenV RAGEOPENV) -eq 1 ] && [ \$(nve_item '$sub' StraightToStoryMode STRAIGHTTOSTORYMODE) -eq 1 ] && log_has 'Start-up summary not shown (ERROR_POPUP=0): 2 problem' && summary_once"
	check "in $at: the header's Features: line says off for RageOpenV and StraightToStoryMode and on for the rest, the check's lines stand after the header" "features_say RageOpenV StraightToStoryMode && after_header 'is installed and does this job too'"
done

echo "== I1c ResourceExtensions.asi, Resource Extensions [Enhanced], in the game folder: HeapAdjuster, PackfileLimit, Blips and RayTracing off while it is there"
fresh_ini; touch ResourceExtensions.asi; out=$(game full_game.exe); rm -f ResourceExtensions.asi
check "HeapAdjuster, PackfileLimit, Blips and RayTracing off, the other parts applied, DirectStorage on" "has '$GAME_HEAP' && has '$PACK_OFF' && log_lacks '\[Blips\]' && log_lacks '\[RayTracing\]' && has '$MODKIT_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON' && log_has '\[DirectStorage\]'"
check "one item per part, word for word, the only four problems of one summary" "[ \$(re_item HeapAdjuster HEAP_ADJUSTER) -eq 1 ] && [ \$(re_item PackfileLimit PACKFILE_LIMIT) -eq 1 ] && [ \$(re_item Blips BLIPS) -eq 1 ] && [ \$(re_item RayTracing RAYTRACING) -eq 1 ] && log_has 'Start-up summary not shown (ERROR_POPUP=0): 4 problem' && summary_once"
check "the header's Features: line says off for HeapAdjuster, PackfileLimit, Blips and RayTracing and on for the rest, the check's lines stand after the header" "features_say HeapAdjuster PackfileLimit Blips RayTracing && after_header 'is installed and does this job too'"

echo "== I2 all old plugins at once, in the game folder, scripts\\ and plugins\\: everything skipped"
fresh_ini
touch HeapAdjuster.asi ModkitLimitAdjusterEnhanced.asi plugins/PackfileLimitAdjusterEnhanced.asi scripts/OpenRPF.asi StraightToStoryMode.asi scripts/DecalPatch.asi WeaponLimitsAdjusterEnhanced.asi plugins/GTAV.ResourceAdjusterEnhanced.asi
out=$(game full_game.exe); echo "      $out"
rm -f HeapAdjuster.asi ModkitLimitAdjusterEnhanced.asi plugins/PackfileLimitAdjusterEnhanced.asi scripts/OpenRPF.asi StraightToStoryMode.asi scripts/DecalPatch.asi WeaponLimitsAdjusterEnhanced.asi plugins/GTAV.ResourceAdjusterEnhanced.asi
check "nothing applied: the eight old plugins switch every part off" "has '$GAME_HEAP' && has '$MODKIT_OFF' && has '$PACK_OFF' && has '$ROV_OFF' && has '$STORY_OFF' && has '$BOX_OFF' && has '$DECAL_OFF'"
check "all eight reported (DecalPatch.asi for two parts), one summary with 9 problems, no crash" "[ \$(count 'PROBLEM: Old plugin found') -eq 9 ] && log_has 'Start-up summary not shown (ERROR_POPUP=0): 9 problem' && summary_once"

echo "== I2b six of them and onigiri.asi in the game folder: everything skipped, every plugin said"
fresh_ini
touch HeapAdjuster.asi ModkitLimitAdjusterEnhanced.asi plugins/PackfileLimitAdjusterEnhanced.asi scripts/OpenRPF.asi StraightToStoryMode.asi scripts/DecalPatch.asi onigiri.asi
out=$(game full_game.exe)
rm -f HeapAdjuster.asi ModkitLimitAdjusterEnhanced.asi plugins/PackfileLimitAdjusterEnhanced.asi scripts/OpenRPF.asi StraightToStoryMode.asi scripts/DecalPatch.asi onigiri.asi
check "nothing applied" "has '$GAME_HEAP' && has '$MODKIT_OFF' && has '$PACK_OFF' && has '$ROV_OFF' && has '$STORY_OFF' && has '$BOX_OFF' && has '$DECAL_OFF'"
check "the 7 old-plugin items, and onigiri.asi's item word for word for each of its two parts, also switched off by OpenRPF.asi and StraightToStoryMode.asi: one summary with 9 problems" "[ \$(count 'PROBLEM: Old plugin found') -eq 7 ] && [ \$(nve_item '' RageOpenV RAGEOPENV) -eq 1 ] && [ \$(nve_item '' StraightToStoryMode STRAIGHTTOSTORYMODE) -eq 1 ] && log_has 'Start-up summary not shown (ERROR_POPUP=0): 9 problem' && summary_once"

echo "== I2c onigiri.asi next to a part TweaksV.ini switches off: no item for that part, a line in the log; with both off, a line each and no problem"
fresh_ini; setkey ENABLED 0 RAGEOPENV; touch onigiri.asi; out=$(game full_game.exe); rm -f onigiri.asi
check "[RAGEOPENV] ENABLED=0: StraightToStoryMode's item alone, the one problem, and RageOpenV's line word for word; both parts off, the other six applied, DirectStorage on" "[ \$(nve_item '' StraightToStoryMode STRAIGHTTOSTORYMODE) -eq 1 ] && [ \$(nve_line RageOpenV) -eq 1 ] && [ \$(nve_line StraightToStoryMode) -eq 0 ] && log_lacks 'RageOpenV part is switched off' && log_has 'Start-up summary not shown (ERROR_POPUP=0): 1 problem' && summary_once && has '$ROV_OFF' && has '$STORY_OFF' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && log_has '\[DirectStorage\]'"
fresh_ini; setkey ENABLED 0 RAGEOPENV; setkey ENABLED 0 STRAIGHTTOSTORYMODE; touch onigiri.asi; out=$(game full_game.exe); rm -f onigiri.asi
check "both sections ENABLED=0: one line per part word for word, no problem, no summary; the other six parts applied, DirectStorage on" "[ \$(nve_line RageOpenV) -eq 1 ] && [ \$(nve_line StraightToStoryMode) -eq 1 ] && log_lacks 'PROBLEM' && log_lacks 'Start-up summary' && has '$ROV_OFF' && has '$STORY_OFF' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && log_has '\[DirectStorage\]'"
check "and not the line that none was found: onigiri.asi is there" "log_lacks 'Conflict check: none'"

echo "== I2d neither kind of plugin installed: the conflict check says so in one line"
fresh_ini; out=$(game full_game.exe)
check "the one line, no problem" "[ \$(count 'Conflict check: none of the plugins that do the job of one of TweaksV.s parts were found.') -eq 1 ] && log_lacks 'PROBLEM'"

echo "== I3 TEST build, one pattern broken per plugin: only that part is skipped, one summary"
breaktest() {	# description, the pattern's label, expected-output check: the label broken and its part's reason, in any case
	fresh_ini; addkey TEST_BREAK_PATTERN "$2"; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$1" "$3 && log_has 'TEST BUILD - not for release' && grep -qi -- 'TEST BUILD: pattern $2 is treated as not found' TweaksV.log && grep -qi -- '$2 was not found[ .]' TweaksV.log && summary_once && has 'protect: all restored'"
}
breaktest "H-E1 (heap): heap unchanged, the rest applied" "H-E1" \
	"has '$GAME_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "M-E-R3 (a modkit reference): modkit untouched, the rest applied" "M-E-R3" \
	"has '$ALL_ON_HEAP' && has '$MODKIT_OFF' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "M-E-L6 (a modkit limit): modkit untouched, the rest applied" "M-E-L6" \
	"has '$ALL_ON_HEAP' && has '$MODKIT_OFF' && has '$PACK_ON' && has '$ROV_ON'"
breaktest "P-E-R12 (a packfile reference): packfile untouched, the rest applied" "P-E-R12" \
	"has '$PACK_OFF' && has '$MODKIT_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "P-E-H1 (the streaming configuration call): packfile untouched, the rest applied" "P-E-H1" \
	"has '$PACK_OFF' && has '$MODKIT_ON' && has '$ROV_ON'"
breaktest "B-E-L2 (a box streamer capacity): list untouched, the rest applied" "B-E-L2" \
	"has '$BOX_OFF' && has '$DECAL_ON' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "D-E-L3 (a decal definitions site): array untouched, the rest applied" "D-E-L3" \
	"has '$DECAL_OFF' && has '$BOX_ON' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "D-E-L12 (the shared 'add decal' check, needed 7 times): array untouched, the rest applied" "D-E-L12" \
	"has '$DECAL_OFF' && has '$BOX_ON' && has '$ROV_ON' && log_has 'The decal definitions limit is not raised this session: D-E-L12 was not found.'"
breaktest "R-E-F5 (a file function): no file hooks, OPEN on" "R-E-F5" \
	"has 'file hooks 0/8 | open calls 5/5, behaviour ok' && has '$PACK_ON'"
breaktest "R-E-E3 (an encryption call): no OPEN calls, files on" "R-E-E3" \
	"has 'file hooks 8/8, redirect 8/8, pass 8/8, spellings ok, folder ok, outside ok | open calls 0/5'"
breaktest "S-E2 (the legal screen): only that screen is left, the others skipped" "S-E2" \
	"has 'splash: E9 86 01 00 00 90 | legal: 0F 84 07 05 00 00 | landing: 31 C0 C3 24 08' && has '$ROV_ON'"
breaktest "W-E-I2 (the weapon info loader): the weapon info limit neither raised nor guarded, the rest applied" "W-E-I2" \
	"has 'weapon infos: limits 160 160, loader as laid, the rest as laid, array 152 of 160,' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "W-E-C2 (the weapon components' sort): list and pool untouched, the rest applied" "W-E-C2" \
	"has 'wc: loads 0 moved 17 game 0 other, other bytes as placed, data as placed, size call to the lookup' && has '$DECAL_ON' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "A-E-G3 (a ped props narrowing): the ped props' sites as placed, the rest applied" "A-E-G3" \
	"has 'pedprops: sites as placed, data as placed |' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
fresh_ini; setkey VIDEO_MEMORY_BUDGET 6000 RESOURCES; addkey TEST_BREAK_PATTERN Q-E-T4; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "Q-E-T4 (the texture budget's placement check), VIDEO_MEMORY_BUDGET=6000: the budgets as the game set them, the rest applied" \
	"has 'the call to the next step' && has 'budgets 1500, 1500, 3000 and 3000 MB' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON' && log_has 'TEST BUILD - not for release' && grep -qi -- 'TEST BUILD: pattern Q-E-T4 is treated as not found' TweaksV.log && grep -qi -- 'Q-E-T4 was not found[ .]' TweaksV.log && summary_once && has 'protect: all restored'"
fresh_ini; setkey DYNAMIC_BVH_VERTICES 8388608; addkey TEST_BREAK_PATTERN T-E-C1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "T-E-C1 (a check of the ray tracing buffer, with DYNAMIC_BVH_VERTICES=8388608): the buffer untouched, the rest applied" "has 'rt sites: as placed, count 4194304, checks 4194304 and 4194304, jumps to the game and the game' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON' && log_has 'TEST BUILD - not for release' && grep -qi -- 'TEST BUILD: pattern T-E-C1 is treated as not found' TweaksV.log && grep -qi -- 'T-E-C1 was not found[ .]' TweaksV.log && summary_once && has 'protect: all restored'"
breaktest "I-E-T3 (a pack's variations' lea of the siren setting ID table): the siren IDs' sites and data as placed, the rest applied" "I-E-T3" \
	"has 'siren ids: sites as placed 17 of 17, data as placed' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON'"
breaktest "C-E-L1 (the static bounds store's size call): the store as the game made it, the rest applied" "C-E-L1" 	"has 'static bounds: call to the lookup, the rest as placed, the store made: 12000 slots' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON' && log_has 'PROBLEM: The static bounds store.s size is not raised this session: C-E-L1 was not found.'"
fresh_ini; addkey TEST_BREAK_PATTERN C-E-D1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "C-E-D1 (the static bounds store's lea, for its read-back): a line and no problem of the part, the store raised all the same, the rest applied" "has 'static bounds: call through a stub, the rest as placed, the store made: 16000 slots' && has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$ROV_ON' && has '$STORY_ON' && log_has 'The static bounds store.s read-back is not done this session: C-E-D1 was not found.' && log_lacks '\[StaticBounds\] PROBLEM' && has 'protect: all restored'"
breaktest "a label in small letters works too" "h-e1" \
	"has '$GAME_HEAP' && has '$MODKIT_ON'"
fresh_ini; addkey TEST_BREAK_PATTERN "H-E1"; out=$(game full_game.exe)
check "the release build ignores TEST_BREAK_PATTERN" "has '$ALL_ON_HEAP' && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM'"

echo "== I4 loaded by another exe (GTA5.exe): TweaksV does nothing and creates nothing"
rm -f TweaksV.ini TweaksV.log; rm -rf appdata/TweaksV
cp full_game.exe GTA5.exe; run_fake GTA5.exe TweaksV.asi; out=$(tr -d '\r' < harness.out); rm -f GTA5.exe
check "nothing applied" "has '$GAME_HEAP' && has '$MODKIT_OFF' && has '$PACK_OFF' && has '$BOX_OFF' && has '$DECAL_OFF' && has '$ROV_OFF' && has '$STORY_OFF'"
check "no TweaksV.ini or TweaksV.log anywhere" "[ ! -e TweaksV.ini ] && [ ! -e TweaksV.log ] && [ ! -e appdata/TweaksV ]"

echo "== I5 the build: no Visual C++ runtime needed (/MT), only system DLLs imported, the exploit mitigations on"
deps=$(cmd //c "$(cygpath -w "$T/deps.cmd")" "$(cygpath -w TweaksV.asi)" 2>/dev/null | tr -d '\r' | grep -i '\.dll' | tr -d ' ' | sort | tr '\n' ' ')
echo "      imports: $deps"
check "KERNEL32, USER32 and VERSION only" "[ \"$deps\" = 'KERNEL32.dll USER32.dll VERSION.dll ' ]"
heads=$(cmd //c "$(cygpath -w "$T/headers.cmd")" "$(cygpath -w TweaksV.asi)" 2>/dev/null | tr -d '\r')
check "ASLR with high entropy, DEP, Control Flow Guard and CET" "grep -q 'High Entropy Virtual Addresses' <<< \"\$heads\" && grep -q 'Dynamic base' <<< \"\$heads\" && grep -q 'NX compatible' <<< \"\$heads\" && grep -q 'Control Flow Guard' <<< \"\$heads\" && grep -q 'CET compatible' <<< \"\$heads\""

echo "== I6 renamed to zTweaksV.asi: the same TweaksV.ini and TweaksV.log, everything works"
fresh_ini; cp TweaksV.asi zTweaksV.asi; out=$(ASI=zTweaksV.asi game full_game.exe); rm -f zTweaksV.asi
check "everything applied" "has '$ALL_ON_HEAP' && has '$MODKIT_ON' && has '$PACK_ON' && has '$BOX_ON' && has '$DECAL_ON' && has '$ROV_ON' && has '$STORY_ON'"
check "TweaksV.log names zTweaksV.asi, no problems" "log_has 'TweaksV.asi: .*zTweaksV.asi' && log_lacks 'PROBLEM'"

echo "== I7 no code may be changed (Windows' dynamic code policy): every write fails whole, each part reported"
fresh_ini; out=$(game full_game.exe nodyncode); echo "      $out"
check "harness survived to its report, the policy was set" "has 'protect: ' && ! has 'dynamic code policy not set'"
check "nothing written: game code, protection and call targets as they were" "has '$GAME_HEAP' && has '$STORY_OFF' && has '$MODKIT_OFF' && has '$PACK_OFF' && has '$BOX_OFF' && has '$DECAL_OFF' && has 'stubs: 0, all RX' && has 'protect: all restored (RX)'"
check "every part reported" "log_has 'PROBLEM: The heap size is not changed this session: the game code could not be made writable' && [ \$(count 'is not skipped this session: the game code could not be made writable') -eq 3 ] && log_has 'PROBLEM: .*modkit' && log_has 'PROBLEM: The packfile limit is not changed' && log_has 'PROBLEM: The box streamer.s traversal list is not enlarged' && log_has 'PROBLEM: The decal definitions limit is not raised' && log_has 'PROBLEM: RageOpenV could not start' && log_has 'PROBLEM: blip_controller.s threshold is not raised this session: the game code could not be changed' && log_has 'PROBLEM: The weapon components limit is not raised this session: the game code could not be changed'"
check "one start-up summary" "summary_once"

echo "== I8 the Windows user name stays out of the log: the profile folder is written as %USERPROFILE%, in both its forms"
bw=$(cygpath -w "$B")	# the game folder, here the profile folder too
fresh_ini; out=$(USERPROFILE="$bw" game full_game.exe)
check "the paths in it start with %USERPROFILE%, the folder's own name is nowhere in the log" "log_has 'TweaksV.asi: %USERPROFILE%.TweaksV.asi' && log_lacks -i -F \"\$bw\""
# A profile folder whose name is only a prefix of the game folder's (the game folder minus its last character): left alone
fresh_ini; out=$(USERPROFILE="${bw%?}" game full_game.exe)
check "a profile folder that is only a prefix of a longer folder name is not replaced" "log_has 'TweaksV.asi: .*TestRun.TweaksV.asi' && log_lacks '%USERPROFILE%'"
# A profile folder with a long name has a short one too (C:\Users\NAME~1), which %TEMP% often has. Here
# %LOCALAPPDATA% has it, and the log header names the TweaksV.ini there that the game folder's wins over.
pl="$B/Some Long User Name"; mkdir -p "$pl/AppData/TweaksV"; cp TweaksV.ini "$pl/AppData/TweaksV/"
pw=$(cygpath -w "$pl"); ps=$(cygpath -d "$pl")
cp full_game.exe GTA5_Enhanced.exe; rm -f TweaksV.log
USERPROFILE="$pw" FAKE_APPDATA="$ps\\AppData" run_fake GTA5_Enhanced.exe TweaksV.asi
if [ "$ps" = "$pw" ]; then
	skip "its short form as well: this volume has no 8.3 names, so there is no short form to check"
else
	check "its short form as well: the log names %USERPROFILE%\\AppData\\TweaksV, neither form is in it" "log_has 'wins over the unused copy in %USERPROFILE%.AppData.TweaksV)' && log_lacks -i -F \"\$ps\" && log_lacks -i -F \"\$pw\""
fi
rm -rf "$pl"

echo "== I9 TweaksV.log stops growing at its limit (16 KB here, set with TEST_LOG_LIMIT_KB in a test build)"
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_LOG_LIMIT_KB 16; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "at most 16 KB, ending with the line that says so" "[ \$(stat -c %s TweaksV.log) -le 16384 ] && tail -n 1 TweaksV.log | grep -q 'TweaksV.log has reached its limit of 16 KB - nothing more is written to it this session.'"
check "the game is patched all the same" "has '$ALL_ON_HEAP' && has '$STORY_ON' && has '$ROV_ON'"

# Where TweaksV's own files belong: a link, a compressed file, a read-only folder, a file in use.
# link_run [OPTION...] runs the fake game without game(), which would delete TweaksV.log.
link_run() { cp full_game.exe GTA5_Enhanced.exe; run_fake GTA5_Enhanced.exe TweaksV.asi "$@"; out=$(tr -d '\r' < harness.out); }
# is_junction DIR: DIR is a junction - a reparse point with the junction's tag, IO_REPARSE_TAG_MOUNT_POINT
is_junction() { MSYS_NO_PATHCONV=1 fsutil reparsepoint query "$(cygpath -w "$1")" 2> /dev/null | grep -qi '0xa0000003'; }

echo "== I10 TweaksV.log in the game folder is a hard link to another file: that file is left alone"
fresh_ini; rm -f TweaksV.log; rm -rf appdata/TweaksV; echo "not the log" > victim.txt; ln victim.txt TweaksV.log
link_run
check "the other file is unchanged" "[ \"\$(cat victim.txt)\" = 'not the log' ]"
check "the log goes to %LOCALAPPDATA%\\TweaksV and says why; the game is patched" "grep -q 'PROBLEM: The TweaksV.log in the game folder is not used: it is a hard link' appdata/TweaksV/TweaksV.log && has '$ALL_ON_HEAP'"
rm -f TweaksV.log victim.txt; rm -rf appdata/TweaksV

echo "== I11 TweaksV.ini in the game folder is a hard link to another file: neither read nor written"
fresh_ini; setkey SKIP_SPLASH_SCREEN 0 STRAIGHTTOSTORYMODE; sed -b -i '/^HEAP_SIZE=/d' TweaksV.ini
mv TweaksV.ini victim.ini; cp victim.ini victim.before; ln victim.ini TweaksV.ini
out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the other file is unchanged - its missing key was not added" "cmp -s victim.ini victim.before"
check "reported; every setting at its built-in default, not the file's (the splash screen is skipped)" "log_has 'PROBLEM: TweaksV.ini (.*) is not used: it is a hard link' && has 'splash: E9 86 01 00 00 90' && has 'heap: 2000 MB'"
check "the log header says so" "log_has 'TweaksV.ini: .*(not used: it is a hard link'"
rm -f TweaksV.ini victim.ini victim.before

echo "== I12 and %LOCALAPPDATA%\\TweaksV is a junction to another folder: nothing is written through it"
fresh_ini; rm -f TweaksV.log; rm -rf appdata/TweaksV elsewhere; mkdir -p elsewhere; echo "not the log" > victim.txt; ln victim.txt TweaksV.log
if ! cmd //c "$(cygpath -w "$T/junction.cmd")" "$(cygpath -w "$B/appdata/TweaksV")" "$(cygpath -w "$B/elsewhere")"; then
	check "the junction could be made (a check of the test itself) - the scenario stops here" "false"
else
	link_run
	check "the junction is there (a reparse point); nothing was left in its target, the other file unchanged" "is_junction appdata/TweaksV && [ -z \"\$(ls -A elsewhere)\" ] && [ \"\$(cat victim.txt)\" = 'not the log' ]"
	check "the game is patched all the same" "has '$ALL_ON_HEAP' && has '$STORY_ON'"
	cmd //c "$(cygpath -w "$T/junction.cmd")" remove "$(cygpath -w "$B/appdata/TweaksV")"
	check "the junction is removed again - the link only, its target is still there" "[ ! -e appdata/TweaksV ] && [ -d elsewhere ]"
fi
rm -f TweaksV.log victim.txt; rm -rf elsewhere

# Windows keeps a compressed file as a reparse point, as it does a link, but shows it as a plain file.
echo "== I13 TweaksV.ini and TweaksV.log compressed by Windows (compact /exe, as CompactGUI does): both used as usual"
fresh_ini; rm -rf appdata/TweaksV
for i in $(seq 100); do printf '; a comment line, so that Windows has something to compress\r\n'; done >> TweaksV.ini
for i in $(seq 400); do printf '12:00:00.000 [TweaksV] a line of an older log\r\n'; done > TweaksV.log
MSYS_NO_PATHCONV=1 compact /c /exe:xpress4k "$(cygpath -w TweaksV.ini)" "$(cygpath -w TweaksV.log)" > /dev/null
compressed=$(MSYS_NO_PATHCONV=1 compact "$(cygpath -w TweaksV.ini)" "$(cygpath -w TweaksV.log)" | tr -d '\r')
link_run
check "both files were compressed" "[[ \"\$compressed\" == *'2 are compressed and 0 are not compressed'* ]]"
check "TweaksV.ini is read (its HEAP_SIZE, not the default), TweaksV.log rewritten in the game folder, no problem" "has '$ALL_ON_HEAP' && log_has '\[TweaksV\] TweaksV [0-9]' && log_lacks 'an older log' && log_lacks 'PROBLEM' && [ ! -e appdata/TweaksV ]"
rm -f TweaksV.log

# Read-only as under C:\Program Files: a deny entry for this user on the folder alone (not on the files
# in it) lets nothing be added to it. The test build, as TweaksV.ini starts out with ERROR_POPUP=1.
echo "== I14 a read-only game folder (as under C:\\Program Files): both files in %LOCALAPPDATA%\\TweaksV, only logged"
rm -rf ro appdata/TweaksV; mkdir -p ro; cp full_game.exe ro/GTA5_Enhanced.exe; cp TweaksV_test.asi ro/TweaksV.asi
deny_writes ro
ro_run() { run_fake -C ro GTA5_Enhanced.exe TweaksV.asi; out=$(tr -d '\r' < harness.out); }
fb=appdata/TweaksV
check "the folder is read-only (a check of the test itself)" "[ -n '$sid' ] && ! touch ro/probe.txt 2>/dev/null"
ro_run
check "TweaksV.ini created there with the defaults, TweaksV.log written there: logged, no problem" "[ -f $fb/TweaksV.ini ] && grep -q 'The game folder is read-only (Windows error 5, access denied), so TweaksV.ini was created with the default settings in ' $fb/TweaksV.log && grep -q 'The game folder is read-only (Windows error 5, access denied), so TweaksV.log is written to ' $fb/TweaksV.log && log_lacks 'PROBLEM' $fb/TweaksV.log"
check "nothing in the game folder; the game is patched with the defaults" "[ ! -e ro/TweaksV.ini ] && [ ! -e ro/TweaksV.log ] && has 'heap: 2000 MB' && has 'splash: E9 86 01 00 00 90 |'"
setkey HEAP_SIZE 1500 "" $fb/TweaksV.ini; setkey ERROR_POPUP 0 "" $fb/TweaksV.ini
ro_run
check "the next start reads that TweaksV.ini (HEAP_SIZE=1500) and says where it is" "has 'heap: 1500 MB' && grep -q 'TweaksV.ini: .*TweaksV.ini (the game folder has no TweaksV.ini)' $fb/TweaksV.log && log_lacks 'PROBLEM' $fb/TweaksV.log"
allow_writes ro
check "the folder is writable again (the test's clean-up)" "touch ro/probe.txt"
rm -rf ro appdata/TweaksV

echo "== I15 another program holds TweaksV.log in the game folder open: the log goes to %LOCALAPPDATA%\\TweaksV, a problem"
fresh_ini; rm -rf appdata/TweaksV; echo "an older log" > TweaksV.log
link_run lockedlog
check "the log was held, and is left as it was" "has 'log held' && [ \"\$(cat TweaksV.log)\" = 'an older log' ]"
check "the log is written to %LOCALAPPDATA%\\TweaksV, and the start-up summary says why; the game is patched" "grep -q 'PROBLEM: The TweaksV.log in the game folder is not used: Windows error 32, file in use. TweaksV.log is written to ' appdata/TweaksV/TweaksV.log && grep -q 'Start-up summary not shown (ERROR_POPUP=0): 1 problem' appdata/TweaksV/TweaksV.log && has '$ALL_ON_HEAP'"
rm -f TweaksV.log; rm -rf appdata/TweaksV

# Test builds only: the game's code as it is in memory, for finding patterns. The fake game's copies of
# the game code lie in .fakesit, its own data in .data - the part that must never be saved.
echo "== I16 TEST_DUMP_CODE in a test build: the game's code saved in %LOCALAPPDATA%\\TweaksV at start and later, never its data"
d=appdata/TweaksV
# dump_sections FILE: the names in a dump's section table, one per line - sectionCount at offset 12 of the
# 40-byte header, then 32 bytes per section with its name in the first 8 (TweaksVDump.h)
dump_sections() {
	local n i
	n=$(od -An -tu4 -j12 -N4 "$1" | tr -d ' ')
	[ -n "$n" ] && [ "$n" -le 96 ] || return 1
	for ((i = 0; i < n; i++)); do dd if="$1" bs=1 skip=$((40 + 32 * i)) count=8 2> /dev/null | tr -d '\0'; echo; done
}
# code_only FILE: the dump's section table holds the headers and the code (.text, .fakesit), never .data
code_only() { local s; s=$(dump_sections "$1") && grep -qxF HEADERS <<< "$s" && grep -qxF .text <<< "$s" && grep -qxF .fakesit <<< "$s" && ! grep -qxF .data <<< "$s"; }
fresh_ini; setkey DRY_RUN 1; addkey TEST_DUMP_CODE 1; rm -rf $d
out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "both copies there, format 1, the second taken 1 s later; nothing in the game folder; the log header lists the switch" "[ \"\$(head -c 8 $d/GameCode-start.bin)\" = TWKVDUMP ] && [ \"\$(head -c 8 $d/GameCode-later.bin)\" = TWKVDUMP ] && [ \"\$(od -An -tu4 -j8 -N4 $d/GameCode-start.bin | tr -d ' ')\" = 1 ] && [ \"\$(od -An -tu4 -j32 -N4 $d/GameCode-later.bin | tr -d ' ')\" = 1 ] && [ ! -e GameCode-start.bin ] && log_has 'TEST BUILD - not for release. .* TEST_DUMP_CODE=1'"
check "the log names the sections saved, the code among them, and leaves out .data" "log_has 'TEST BUILD: GameCode-start.bin saved, the game as it was 0 s after TweaksV started, before anything was patched: .* - sections HEADERS .text .*fakesit' && log_has 'TEST BUILD: GameCode-later.bin saved, the game as it was 1 s after TweaksV started: ' && log_has 'not saved (the game writes to them):.* .data[ .]'"
check "the section tables of both copies: the headers and the code in them, never .data" "code_only $d/GameCode-start.bin && code_only $d/GameCode-later.bin"
check "the copy holds the game's code as TweaksV found it (the heap site), the game is unpatched (DRY_RUN), no problem" "od -An -tx1 -v $d/GameCode-start.bin | tr -d ' \n' | grep -q 'ba000080284531c041b101e811223344488d0d' && has '$GAME_HEAP' && log_lacks 'PROBLEM'"
fresh_ini; addkey TEST_DUMP_CODE 99999; rm -rf $d; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "TEST_DUMP_CODE=99999 is a problem, and nothing is saved" "log_has 'PROBLEM: TweaksV.ini: \[TWEAKSV\] TEST_DUMP_CODE=99999 is not a number of seconds from 1 to 3600 - the game.s code is not saved.' && [ ! -e $d/GameCode-start.bin ]"
fresh_ini; addkey TEST_DUMP_CODE abc; rm -rf $d; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "TEST_DUMP_CODE=abc as well" "log_has 'PROBLEM: TweaksV.ini: \[TWEAKSV\] TEST_DUMP_CODE=abc is not a number of seconds' && [ ! -e $d/GameCode-start.bin ]"
mkdir -p $d; echo "not a dump" > victim.txt; ln victim.txt $d/GameCode-start.bin
fresh_ini; setkey DRY_RUN 1; addkey TEST_DUMP_CODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "GameCode-start.bin a hard link to another file: that file left alone, a problem; the later copy saved" "[ \"\$(cat victim.txt)\" = 'not a dump' ] && log_has 'PROBLEM: TEST_DUMP_CODE: GameCode-start.bin was not saved: it is a hard link' && [ -f $d/GameCode-later.bin ]"
rm -f victim.txt
fresh_ini; addkey TEST_DUMP_CODE 1; rm -rf $d; out=$(game full_game.exe)
check "the release build ignores TEST_DUMP_CODE: nothing saved, no problem" "[ ! -e $d/GameCode-start.bin ] && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM' && has '$ALL_ON_HEAP'"
rm -rf $d

# Test builds only: named windows of the game's .data, the static data the code dump leaves out. The fake game's
# window lies in its own .data, at the RVA its report gives. With dwwindow it holds what dw_start.bin holds until TweaksV
# patches the heap site in its start-up, and what dw_later.bin holds from then on, the files of the two moments the same:
# so the first copies come before any part changes the game.
echo "== I16b TEST_DUMP_DATA in a test build: named windows of the game's .data saved in %LOCALAPPDATA%\\TweaksV at start and later"
d=appdata/TweaksV
hex() { printf '%X' "$1"; }
# dw_read: the last run's report into dw_win, dw_data, dw_bytes (the window's RVA, .data's RVA and its size, in hex)
# and dw_image (the game's base as two numbers, its size, its stamp, as a file's header gives them)
dw_read() { read -r dw_win dw_data dw_bytes dw_image <<< "$(sed -nE 's/.*dw: window RVA 0x([0-9A-F]+), \.data RVA 0x([0-9A-F]+), 0x([0-9A-F]+) bytes, image ([0-9 ]+) \|.*/\1 \2 \3 \4/p' <<< "$out")"; }
# dw_run VALUE [OPTION...]: the fake game with the test build, TEST_DUMP_DATA=VALUE, no window's file there before it
dw_run() { local value=$1; shift; fresh_ini; addkey TEST_DUMP_DATA "$value"; rm -rf $d; out=$(ASI=TweaksV_test.asi game full_game.exe dwrvas "$@"); dw_read; }
# dw_header FILE SECONDS RVA SIZE UNREADABLE: the file's DataDumpHeader (TweaksVDump.h, 48 bytes) for that window of this
# run's game, the RVA and the size in hex, and the file as long as the header and the window
dw_header() {
	[ "$(head -c 8 "$1")" = TWKVDATA ] &&
	[ "$(od -An -tu4 -j8 -N40 "$1" | tr -s ' \n' ' ')" = " 1 48 $dw_image $2 $((0x$3)) $((0x$4)) $5 " ] &&
	[ "$(stat -c %s "$1")" = $((48 + 0x$4)) ]
}
# dw_holds FILE HELD OFFSET SIZE: after its header, the file holds byte for byte the SIZE bytes HELD, a file of the fake
# game's, holds from OFFSET on, both in hex
dw_holds() { cmp -s -i "48:$((0x$3))" -n "$((0x$4))" "$1" "$2"; }
# dw_saved_line NAME SIZE RVA WORDS UNREADABLE: the log's line for a saved file, word for word but the folder
dw_saved_line() { log_line "\[TweaksV\] TEST BUILD: GameData-$1 saved, $2 bytes of the game.s .data at RVA 0x$3 as they were $4, in .*TweaksV; $5.\$"; }
dw_files() { ls $d . 2> /dev/null | grep -c "^GameData-.*$1"; }
# dw_refused DESCRIPTION VALUE REASON: the value is one problem giving REASON, a pattern, and nothing is saved
dw_refused() {
	dw_run "$2"
	dw_want="PROBLEM: TweaksV.ini: \\[TWEAKSV\\] TEST_DUMP_DATA=$2: $3 - the game.s data is not saved.\$"
	check "$1: one problem, nothing saved" 'log_line "$dw_want" && [ "$(count PROBLEM)" = 1 ] && [ "$(dw_files)" = 0 ]'
}
fresh_ini; out=$(game full_game.exe dwrvas); dw_read
check "the fake game's window lies in its own .data" '[ -n "$dw_win" ] && [ $((0x$dw_win)) -ge $((0x$dw_data)) ] && [ $((0x$dw_win + 0x12000)) -le $((0x$dw_data + 0x$dw_bytes)) ]'
dw_a=$dw_win; dw_b=$(hex $((0x$dw_win + 0x1234))); dw_end=$((0x$dw_data + 0x$dw_bytes))

dw_run "1,$dw_a:10000,0x${dw_b,,}:100" dwwindow
check "two windows, one of the largest size, each in two files in %LOCALAPPDATA%\\TweaksV under its RVA in upper-case hex, whatever the key's spelling; nothing in the game folder; the log header lists the switch" "[ -f $d/GameData-$dw_a-start.bin ] && [ -f $d/GameData-$dw_a-later.bin ] && [ -f $d/GameData-$dw_b-start.bin ] && [ -f $d/GameData-$dw_b-later.bin ] && [ \"\$(dw_files)\" = 4 ] && [ ! -e GameData-$dw_a-start.bin ] && log_has 'TEST BUILD - not for release. .* TEST_DUMP_DATA=1,$dw_a:10000,0x${dw_b,,}:100 '"
check "each header: format 1, the game's base, size and stamp, the moment, the RVA, the size, none unreadable; each file's length" "dw_header $d/GameData-$dw_a-start.bin 0 $dw_a 10000 0 && dw_header $d/GameData-$dw_a-later.bin 1 $dw_a 10000 0 && dw_header $d/GameData-$dw_b-start.bin 0 $dw_b 100 0 && dw_header $d/GameData-$dw_b-later.bin 1 $dw_b 100 0"
check "the start files hold what the fake's .data held before TweaksV patched anything, which it no longer held when its thread wrote them" "dw_holds $d/GameData-$dw_a-start.bin dw_start.bin 0 10000 && dw_holds $d/GameData-$dw_b-start.bin dw_start.bin 1234 100"
check "the later files hold what it held after the patch, which is not what it held before" "dw_holds $d/GameData-$dw_a-later.bin dw_later.bin 0 10000 && dw_holds $d/GameData-$dw_b-later.bin dw_later.bin 1234 100 && ! cmp -s -n 256 -i 48:0 $d/GameData-$dw_a-later.bin dw_start.bin"
check "the log says each of them, word for word" "dw_saved_line $dw_a-start.bin 65536 $dw_a '0 s after TweaksV started, before anything was patched' 'none unreadable' && dw_saved_line $dw_b-start.bin 256 $dw_b '0 s after TweaksV started, before anything was patched' 'none unreadable' && dw_saved_line $dw_a-later.bin 65536 $dw_a '1 s after TweaksV started' 'none unreadable' && dw_saved_line $dw_b-later.bin 256 $dw_b '1 s after TweaksV started' 'none unreadable'"
check "no problem, and the parts did their work beside it" "log_lacks 'PROBLEM' && has '$ALL_ON_HEAP'"

# the edges of .data, the most windows and TEST_DUMP_CODE beside it, in one run
dw_most="1,$dw_data:10,$(hex $((dw_end - 0x10))):10"
for i in {0..13}; do dw_most+=",$(hex $((0x$dw_a + i * 0x10))):10"; done
fresh_ini; addkey TEST_DUMP_DATA "$dw_most"; addkey TEST_DUMP_CODE 1; rm -rf $d; out=$(ASI=TweaksV_test.asi game full_game.exe dwwindow); dw_read
check "16 windows, .data's first bytes and its last among them, with TEST_DUMP_CODE beside them: every file saved, no problem" "[ \"\$(dw_files -start.bin)\" = 16 ] && [ \"\$(dw_files -later.bin)\" = 16 ] && [ -f $d/GameCode-later.bin ] && dw_header $d/GameData-$dw_data-start.bin 0 $dw_data 10 0 && dw_header $d/GameData-$(hex $((dw_end - 0x10)))-later.bin 1 $(hex $((dw_end - 0x10))) 10 0 && log_lacks 'PROBLEM'"
# a value of 254 characters, as long as a key is read whole: 14 windows of 8 digits each, the seconds 2 digits
dw_long="01"
for i in {0..13}; do dw_long+=",$(printf '%08X:%08X' $((0x$dw_a + i * 0x10)) 16)"; done
dw_run "$dw_long"
check "a value of ${#dw_long} characters is read whole: its 14 windows saved" "[ ${#dw_long} = 254 ] && [ \"\$(dw_files -start.bin)\" = 14 ] && log_lacks 'PROBLEM'"
# one character more: it may be cut short, and a window cut short would be saved smaller without a word
dw_run "0$dw_long"
check "a value of 255 characters: a problem, nothing saved" "log_has 'PROBLEM: TweaksV.ini: \[TWEAKSV\] TEST_DUMP_DATA=001,.*: it is longer than the 254 characters a key can hold whole - the game.s data is not saved.' && [ \"\$(dw_files)\" = 0 ]"

dw_refused "text that is no number of seconds" "abc,$dw_a:10" 'it does not start with a number of seconds from 1 to 3600'
dw_refused "0 seconds" "0,$dw_a:10" 'it does not start with a number of seconds from 1 to 3600'
dw_refused "3601 seconds" "3601,$dw_a:10" 'it does not start with a number of seconds from 1 to 3600'
dw_refused "the seconds alone" 3 'it names no window'
for window in zz:10 "$dw_a" "$dw_a:" :10 "$dw_a:10:10" "$dw_a:1G" "123456789:10" "$dw_a:0x"; do
	dw_refused "the window $window, text that is no window" "3,$window" "\"$window\" is not a window of <rva>:<bytes>, both in hex"
done
dw_refused "a comma with nothing after it" "3,$dw_a:10," '"" is not a window of <rva>:<bytes>, both in hex'
dw_too_many=3
for i in {0..16}; do dw_too_many+=",$(hex $((0x$dw_a + i * 0x10))):10"; done
dw_refused "17 windows" "$dw_too_many" 'it names more than 16 windows'
dw_refused "a window of no bytes" "3,$dw_a:0" "the window $dw_a:0 is not of 1 byte to 64 KB (10000 in hex)"
dw_refused "a window one byte over 64 KB" "3,$dw_a:10001" "the window $dw_a:10001 is not of 1 byte to 64 KB (10000 in hex)"
dw_refused "one RVA twice, however it is written: its files would have one name" "3,$dw_a:10,0x$dw_a:20" "the window at 0x$dw_a is named twice, and its files have one name"
dw_leaves() { echo "the window of 0x$1 bytes at 0x$2 is not wholly inside the game.s .data, 0x$dw_data to $(printf '0x%X' $dw_end)"; }
dw_refused "a window that runs a byte past .data" "3,$(hex $((dw_end - 0x10))):11" "$(dw_leaves 11 "$(hex $((dw_end - 0x10)))")"
dw_refused "a window that starts a byte before .data" "3,$(hex $((0x$dw_data - 1))):10" "$(dw_leaves 10 "$(hex $((0x$dw_data - 1)))")"
dw_refused "a window in the game's code" "3,1000:10" "$(dw_leaves 10 1000)"
dw_refused "a window whose end does not fit 32 bits" "3,FFFFFFFF:10" "$(dw_leaves 10 FFFFFFFF)"
dw_refused "a good window before one that leaves .data: nothing of either" "3,$dw_a:10,$(hex $((dw_end - 0x10))):11" "$(dw_leaves 11 "$(hex $((dw_end - 0x10)))")"
dw_run "3,$dw_a:10" dwnodata
check "a game whose section table names no .data: a problem, nothing saved" "log_has 'PROBLEM: TweaksV.ini: \[TWEAKSV\] TEST_DUMP_DATA=3,$dw_a:10: the game.s section table names no .data - the game.s data is not saved.' && [ \"\$(dw_files)\" = 0 ]"

mkdir -p $d; echo "not a window" > victim.txt; ln victim.txt $d/GameData-$dw_a-start.bin
fresh_ini; addkey TEST_DUMP_DATA "1,$dw_a:100,$dw_b:100"; out=$(ASI=TweaksV_test.asi game full_game.exe dwwindow)
check "GameData-<rva>-start.bin a hard link to another file: that file left alone, a problem; the other files saved" "[ \"\$(cat victim.txt)\" = 'not a window' ] && log_has 'PROBLEM: TEST_DUMP_DATA: GameData-$dw_a-start.bin was not saved: it is a hard link' && [ -f $d/GameData-$dw_a-later.bin ] && [ -f $d/GameData-$dw_b-start.bin ] && [ -f $d/GameData-$dw_b-later.bin ]"
rm -f victim.txt

dw_run "1,$dw_a:4000" dwwindow dwnoaccess
check "a page of the window that cannot be read: zeros in both files, 4096 bytes counted in each header and log line, the rest as the fake holds it, no problem" "dw_header $d/GameData-$dw_a-start.bin 0 $dw_a 4000 4096 && dw_header $d/GameData-$dw_a-later.bin 1 $dw_a 4000 4096 && dw_holds $d/GameData-$dw_a-start.bin dw_start.bin 0 4000 && dw_holds $d/GameData-$dw_a-later.bin dw_later.bin 0 4000 && dw_saved_line $dw_a-later.bin 16384 $dw_a '1 s after TweaksV started' '4096 bytes unreadable, saved as zeros' && log_lacks 'PROBLEM'"

fresh_ini; addkey TEST_DUMP_DATA "3,$dw_a:100"; rm -rf $d; out=$(game full_game.exe)
check "the release build ignores TEST_DUMP_DATA: nothing saved, no problem" "[ \"\$(dw_files)\" = 0 ] && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM' && has '$ALL_ON_HEAP'"
rm -rf $d; rm -f dw_start.bin dw_later.bin

# Test builds only: a script program as the game places it in memory, for reading a script's code. The fake game
# places blip_controller - and with some options other_script or blip_imposter - through the call at the hook
# site, on a thread of its own, and writes what it placed to script_placed_<n>.bin, n counting blip_controller's
# placements, which the copies must hold byte for byte after their header. The Blips part, which changes
# blip_controller each time it is placed, is off in these runs ([BLIPS] ENABLED=0), and so is the StaticBounds part,
# which would make the hook itself then ([STATICBOUNDS] ENABLED=0), so that they test the switch alone: the switch next
# to the part is the blips suite's scenario.
echo "== I17 TEST_DUMP_SCRIPT in a test build: the script program saved in %LOCALAPPDATA%\\TweaksV each time the game places it"
d=appdata/TweaksV
BLIP_HASH=$((0x80CFE974))
# u32 FILE OFFSET, u64 FILE OFFSET: a little-endian number in a file, in decimal
u32() { od -An -tu4 -j"$2" -N4 "$1" | tr -d ' '; }
u64() { od -An -tu8 -j"$2" -N8 "$1" | tr -d ' '; }
# script_report: the fake game's script field into placed, thread, base, size and stamp - how many programs it placed,
# on which thread, and the game's base, SizeOfImage and PE timestamp
script_report() {
	read -r placed thread base size stamp <<< "$(sed -nE 's/.*script: ([0-9]+) placed on thread ([0-9]+) through [^,]*, resolved [0-9]+ of [0-9]+, last error kept [0-9]+ of [0-9]+, image ([0-9]+) ([0-9]+) ([0-9]+) \|.*/\1 \2 \3 \4 \5/p' <<< "$out")"
}
# header_ok FILE N PLACED RESOLVED UNREADABLE: the header of a copy of blip_controller as the fake game places it
# (TweaksVDump.h's ScriptDumpHeader, 176 bytes: code 16675 bytes, 3 natives, strings 496 bytes), its placement N, the
# programs placed so far, what the resolution returned and the bytes unreadable, the blocks' offsets and the file's size
header_ok() {
	local f=$1 code=16675 natives=3 strings=496
	[ "$(head -c 8 "$f")" = TWKVSCRP ] && [ "$(u32 "$f" 8)" = 1 ] && [ "$(u32 "$f" 12)" = 176 ] &&
	[ "$(head -c 80 "$f" | tail -c 64 | tr -d '\0')" = blip_controller ] && [ "$(head -c 80 "$f" | tail -c 64 | tr -cd '\0' | wc -c)" = 49 ] &&
	[ "$(u32 "$f" 80)" = "$BLIP_HASH" ] && [ "$(u32 "$f" 84)" = $code ] && [ "$(u32 "$f" 88)" = $natives ] && [ "$(u32 "$f" 92)" = $strings ] &&
	[ "$(u32 "$f" 96)" = "$2" ] && [ "$(u32 "$f" 100)" = "$thread" ] && [ "$(u32 "$f" 104)" = "$3" ] &&
	[ "$(od -An -tu1 -j108 -N4 "$f" | tr -s ' ')" = " $4 0 0 0" ] &&
	[ "$(u64 "$f" 112)" = "$base" ] && [ "$(u32 "$f" 120)" = "$size" ] && [ "$(u32 "$f" 124)" = "$stamp" ] &&
	[ "$(u32 "$f" 128)" = "$5" ] && [ "$(u32 "$f" 132)" = 0 ] &&
	[ "$(u64 "$f" 136)" = 176 ] && [ "$(u64 "$f" 144)" = 304 ] && [ "$(u64 "$f" 152)" = $((304 + code)) ] &&
	[ "$(u64 "$f" 160)" = $((304 + code + 8 * natives)) ] && [ "$(u64 "$f" 168)" = $((304 + code + 16 * natives)) ] &&
	[ "$(stat -c %s "$f")" = $((304 + code + 16 * natives + strings)) ]
}
# blocks_ok FILE N: after its header, the copy holds byte for byte what the fake game placed the N-th time
blocks_ok() { tail -c +177 "$1" | cmp -s - "script_placed_$2.bin"; }
# scripts: how many copies of a script program there are; script_run [OPTION...]: the fake game with the test build,
# the Blips and StaticBounds parts off
scripts() { ls $d 2> /dev/null | grep -c '^Script-'; }
script_run() { rm -rf $d; rm -f script_placed_*.bin; setkey ENABLED 0 BLIPS; setkey ENABLED 0 STATICBOUNDS; out=$(ASI=TweaksV_test.asi game full_game.exe "$@"); script_report; }
saved_line() {	# saved_line N PLACED NATIVES UNREADABLE: the log's line for the N-th copy, word for word but the folder
	log_line "\[TweaksV\] TEST BUILD: Script-blip_controller-$1.bin saved in .*TweaksV: blip_controller placed the $(ordinal $1) time, as the game's $(ordinal $2) script program, on thread $thread - code 16675 bytes in 2 pages, 3 natives, $3, strings 496 bytes; $4.\$"
}
ordinal() { case $1 in 1) echo 1st ;; 2) echo 2nd ;; 3) echo 3rd ;; *) echo "$1th" ;; esac; }

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run script
f=$d/Script-blip_controller-1.bin
check "the copy saved in %LOCALAPPDATA%\\TweaksV, and nothing in the game folder; the log header lists the switch, which says what it waits for, and the call is redirected" "[ -f $f ] && [ \$(scripts) = 1 ] && [ ! -e Script-blip_controller-1.bin ] && log_line 'TEST BUILD - not for release. .* TEST_DUMP_SCRIPT=blip_controller\$' && log_line '\[TweaksV\] TEST BUILD: TEST_DUMP_SCRIPT=blip_controller: the script program blip_controller (hash 0x80CFE974) is saved each time the game places it, at most 16 times, as Script-blip_controller-<n>.bin in .*TweaksV.\$' && log_line '\[TweaksV\] The natives. resolution call at RVA 0x[0-9A-F]* is redirected: every script program the game places passes TweaksV, before and after the game resolves its natives.\$'"
check "the header: TWKVSCRP, format 1, 176 bytes, the name zero-padded, the hash and the lengths, placement 1 on the placing thread, the 1st program placed, resolved, the game's base, size and timestamp, none unreadable, the reserved fields 0, the five offsets and the file's size" "[ \"$placed\" = 1 ] && header_ok $f 1 1 1 0"
check "the blocks byte for byte: the program object, the code over its two pages, the native table before (the hashes) and after (the handlers), the strings" "blocks_ok $f 1"
check "the log: the first program the game placed, the line that proves the hook runs, and the copy saved, word for word; no problem" "log_line '\[TweaksV\] TEST BUILD: TEST_DUMP_SCRIPT: the first script program the game placed is blip_controller (hash 0x80CFE974), on thread $thread - the hook runs.\$' && saved_line 1 1 'every one resolved' 'none unreadable' && log_lacks 'PROBLEM'"
check "the placement went through TweaksV's stub, read+execute; the game's resolution ran and its last error came back to the constructor" "has 'script: 1 placed on thread $thread through a TweaksV stub, resolved 1 of 1, last error kept 1 of 1' && has 'stubs: 8, all RX' && has 'protect: all restored (RX)'"

fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_DUMP_SCRIPT BLIP_Controller; script_run scripttwice
check "placed twice, the name asked for in capitals: two copies, the second with a native the game does not know - not every one resolved - each with its blocks and its line" "[ \$(scripts) = 2 ] && header_ok $d/Script-blip_controller-1.bin 1 1 1 0 && header_ok $d/Script-blip_controller-2.bin 2 2 0 0 && blocks_ok $d/Script-blip_controller-1.bin 1 && blocks_ok $d/Script-blip_controller-2.bin 2 && saved_line 1 1 'every one resolved' 'none unreadable' && saved_line 2 2 'not every one resolved' 'none unreadable' && has 'resolved 1 of 2, last error kept 2 of 2'"
check "DIAGNOSTIC_MODE: the three sites under [TweaksV], with what each holds, and the call's write" "log_has '\[TweaksV\] DIAGNOSTIC: natives. resolution call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]*, strings. length offset at RVA 0x[0-9A-F]* holds 0x70 (Y-E-H1)' && log_has '\[TweaksV\] DIAGNOSTIC: code length offset at RVA 0x[0-9A-F]* holds 0x1C, native table offset at RVA 0x[0-9A-F]* holds 0x40 (Y-E-N1)' && log_has '\[TweaksV\] DIAGNOSTIC: code page table offset at RVA 0x[0-9A-F]* holds 0x10 (Y-E-I1)' && log_has '\[TweaksV\] DIAGNOSTIC: natives. resolution call at RVA 0x[0-9A-F]*: E8 .* -> E8 '"

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scriptother
check "another program placed first: other_script is the first program in the log and is not saved; blip_controller is saved as the game's 2nd program" "log_line '\[TweaksV\] TEST BUILD: TEST_DUMP_SCRIPT: the first script program the game placed is other_script (hash 0x0F74EC00), on thread $thread - the hook runs.\$' && [ \$(scripts) = 1 ] && header_ok $d/Script-blip_controller-1.bin 1 2 1 0 && blocks_ok $d/Script-blip_controller-1.bin 1 && saved_line 1 2 'every one resolved' 'none unreadable'"

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scripthash
check "a program with blip_controller's hash and another name, placed twice: logged once, word for word, nothing saved, no problem" "[ \$(count 'TEST BUILD: TEST_DUMP_SCRIPT: a script program has the hash of blip_controller, 0x80CFE974, but is named blip_imposter - it is not saved (logged once).') -eq 1 ] && [ \$(scripts) = 0 ] && log_lacks 'PROBLEM' && has 'script: 2 placed'"

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scriptpage
check "a page of its code that cannot be read: the copy saved all the same, the page's 291 bytes as zeros, counted in the header and the line, the rest byte for byte" "header_ok $f 1 1 1 291 && blocks_ok $f 1 && saved_line 1 1 'every one resolved' '291 bytes unreadable, saved as zeros' && log_lacks 'PROBLEM'"

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scriptmany
check "placed 17 times: 16 copies, the 16th as the game's 16th program, no 17th, and the cap said once" "[ \$(scripts) = 16 ] && header_ok $d/Script-blip_controller-16.bin 16 16 1 0 && blocks_ok $d/Script-blip_controller-16.bin 16 && [ ! -e $d/Script-blip_controller-17.bin ] && [ \$(count 'TEST BUILD: 16 copies of blip_controller saved - from now on its placements are only counted.') -eq 1 ] && has 'script: 17 placed'"

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scripthuge
check "beyond the bounds - a code length of 16 MB and a byte, a native count of 65537: each a problem, word for word, shown after the start-up summary; nothing saved" "log_has 'PROBLEM: TEST_DUMP_SCRIPT: Script-blip_controller-1.bin was not saved: its code length, 16777217 bytes, is beyond 16 MB.' && log_has 'PROBLEM: TEST_DUMP_SCRIPT: Script-blip_controller-2.bin was not saved: its native count, 65537, is beyond 65536.' && log_has 'Problem after the start-up summary not shown (ERROR_POPUP=0): 2 problem(s), see above.' && [ \$(scripts) = 0 ]"

for value in ../x aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa; do
	fresh_ini; addkey TEST_DUMP_SCRIPT "$value"; script_run script
	check "TEST_DUMP_SCRIPT=$value (${#value} characters) is no script's name: a problem, word for word; nothing hooked, nothing saved" "log_has 'PROBLEM: TweaksV.ini: \[TWEAKSV\] TEST_DUMP_SCRIPT=$value is not a script.s name of 1 to 63 letters, digits and _ - no script program is saved.' && has 'through the game function' && [ \$(scripts) = 0 ] && summary_once"
done

for label in Y-E-H1 Y-E-N1 Y-E-I1; do
	fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; addkey TEST_BREAK_PATTERN $label; script_run script
	check "TEST_BREAK_PATTERN=$label: the switch's problem names the pattern; nothing hooked, nothing saved, one summary" "grep -q -- 'TEST BUILD: pattern $label is treated as not found' TweaksV.log && log_has 'PROBLEM: TEST_DUMP_SCRIPT: the script program blip_controller is not saved this session: $label was not found.' && has 'through the game function' && [ \$(scripts) = 0 ] && summary_once"
done

fresh_ini; addkey TEST_DUMP_SCRIPT blip_controller; script_run scripthooked
check "the call another mod redirected already: left alone and a problem, word for word; nothing saved" "log_has 'PROBLEM: TEST_DUMP_SCRIPT: the script program blip_controller is not saved this session: the natives. resolution call at RVA 0x[0-9A-F]* is not a call into the game (another mod may have hooked it already).' && has 'script: none placed, call to another mod' && [ \$(scripts) = 0 ]"

fresh_ini; setkey DRY_RUN 1; addkey TEST_DUMP_SCRIPT blip_controller; script_run script
check "DRY_RUN: the call found and checked, its would line, nothing redirected or saved, no problem" "log_has 'DRY_RUN: would redirect the natives. resolution call at RVA 0x[0-9A-F]*.' && log_has 'TEST BUILD: TEST_DUMP_SCRIPT=blip_controller: DRY_RUN - the call is not redirected, so no script program is saved.' && has 'through the game function' && [ \$(scripts) = 0 ] && log_lacks 'PROBLEM'"

mkdir -p $d; echo "not a copy" > victim.txt
fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; setkey ENABLED 0 STATICBOUNDS; addkey TEST_DUMP_SCRIPT blip_controller; rm -f script_placed_*.bin; ln victim.txt $f
out=$(ASI=TweaksV_test.asi game full_game.exe script)
check "Script-blip_controller-1.bin a hard link to another file: that file left alone; a problem, shown in a box of its own after the start-up summary" "[ \"\$(cat victim.txt)\" = 'not a copy' ] && log_has 'PROBLEM: TEST_DUMP_SCRIPT: Script-blip_controller-1.bin was not saved: it is a hard link' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV: a part stopped working after the start-up:'"
rm -f victim.txt

fresh_ini; setkey ENABLED 0 BLIPS; setkey ENABLED 0 STATICBOUNDS; addkey TEST_DUMP_SCRIPT blip_controller; rm -rf $d; rm -f script_placed_*.bin; out=$(game full_game.exe script)
check "the release build ignores TEST_DUMP_SCRIPT: nothing hooked or saved, no TEST BUILD line, no problem" "has 'script: 1 placed on thread' && has 'through the game function' && [ \$(scripts) = 0 ] && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM'"
rm -rf $d; rm -f script_placed_*.bin

echo "== I18 a half of WeaponLimits throws in place of starting (the test build's TWEAKSV_TEST_WEAPON_HALF_THROWS): only that half ends"
# half_run N: a run of the test build, with ERROR_POPUP=1, in which half N throws. half_ended N NAME: the switch's line and that
# half's problem, word for word and once, the run's only problem, in the start-up summary, and nothing that says the part is skipped.
# parts_after: the parts after WeaponLimits started all the same, every page its own protection.
half_run() { fresh_ini; setkey ERROR_POPUP 1; out=$(TWEAKSV_TEST_WEAPON_HALF_THROWS=$1 ASI=TweaksV_test.asi game full_game.exe); }
half_ended() { [ "$(count "\[WeaponLimits\] TEST BUILD: the $2. half throws in place of starting (TWEAKSV_TEST_WEAPON_HALF_THROWS=$1).")" -eq 1 ] && log_line "\[WeaponLimits\] PROBLEM: The $2. half is not done this session: an unexpected exception ended it.\$" && [ "$(count PROBLEM)" -eq 1 ] && log_has 'Showing the start-up summary (1 problem(s)).' && log_has 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): TweaksV could not apply everything:' && log_lacks 'this part is skipped'; }
parts_after() { has 'pedprops: sites widened' && has 'static bounds: call through a stub, the rest as placed, the store made: 16000 slots' && has 'protect: all restored (RX)'; }

half_run 1
check "the first half, the weapon infos': said once, its problem word for word, the only one, in the start-up summary; nothing says the part is skipped" "half_ended 1 'weapon infos'"
check "that half did nothing, the immediates and the loader as laid; the weapon components' half started all the same - its list moved, its pool's size call redirected, its loader hooked - and the parts after it" "has 'weapon infos: limits 160 160, loader as laid, the rest as laid, array 152 of 160,' && log_lacks 'The weapon info blob limit' && has 'wc: loads 17 moved 0 game 0 other, moved list RW' && has 'size call to a stub, RX: 1494 and 1494 slots' && has 'wc loader hooked' && log_has 'The weapon component list moved and its limit raised from 470 to 1024 entries.' && parts_after"

half_run 2
check "the second half, the weapon components': said once, its problem word for word, the only one, in the start-up summary; nothing says the part is skipped" "half_ended 2 'weapon components'"
check "the weapon infos' half done, the immediates raised and the loader hooked; that half did nothing, its list and its pool's size call the game's and its loader as placed; the parts after it started" "has 'weapon infos: limits 1024 1024, loader hooked, the rest as laid, array 152 of 1024,' && log_has 'The weapon info blob limit is raised from 160 to 1024 weapons.meta files.' && log_has 'The weapon info blob limit is guarded:' && has 'wc: loads 0 moved 17 game 0 other' && has 'size call to the lookup: 470 and 470 slots' && has 'wc loader as placed' && log_lacks 'weapon component list moved' && parts_after"

fresh_ini; out=$(TWEAKSV_TEST_WEAPON_HALF_THROWS=1 game full_game.exe)
check "the release build ignores TWEAKSV_TEST_WEAPON_HALF_THROWS: both halves started, no TEST BUILD line, no problem" "has 'weapon infos: limits 1024 1024, loader hooked' && has 'wc: loads 17 moved 0 game 0 other' && parts_after && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM'"

echo "== I19 the reports after the start-up summary, test build, ERROR_POPUP=1: a heading and a last line per report, each in a box of its own, 3 boxes each at most"
# late_ini: TweaksV.ini for the test build's boxes, with TEST_DUMP_SCRIPT, so that each placement of the fake's latefailures is a problem
late_ini() { fresh_ini; setkey ERROR_POPUP 1; setkey ENABLED 0 BLIPS; addkey TEST_DUMP_SCRIPT blip_controller; }
NO_BOX='TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=1): '
PART_BOX="${NO_BOX}TweaksV: a part stopped working after the start-up: \.\.\. The game continues without the affected parts\."
REFUSED_BOX="${NO_BOX}TweaksV: WeaponLimits refused files the game has no room for: \.\.\. The game continues without the files refused\."
SUMMARY_BOX="${NO_BOX}TweaksV could not apply everything: \.\.\. The game continues without the affected parts\."
NOT_SAVED='PROBLEM: TEST_DUMP_SCRIPT: Script-blip_controller-[0-9]*.bin was not saved: its code length'
late_ini; out=$(TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wcmount latefailures 5)
check "a guard's refusal: a box of its own, with its own heading and last line, its finding not counted as a problem" "[ \$(count '$REFUSED_BOX') -eq 1 ] && [ \$(count 'Showing a finding (1 finding(s))\.') -eq 1 ]"
check "then 5 problems, 2 s apart: 3 boxes all the same, each of a part that stopped working, then lines, none shown" "[ \$(count '$NOT_SAVED') -eq 5 ] && [ \$(count 'Showing a problem after the start-up summary (1 problem(s))\.') -eq 3 ] && [ \$(count '$PART_BOX') -eq 3 ] && [ \$(count 'Problem after the start-up summary not shown: 3 message boxes were shown already this session\.') -eq 2 ]"
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wcmount)
check "the start-up's problem in the summary's box, its heading and last line; the guard's finding not among them" "log_has 'Showing the start-up summary (1 problem(s))\.' && [ \$(count '$SUMMARY_BOX') -eq 1 ] && [ \$(count '$REFUSED_BOX') -eq 1 ]"
fresh_ini; out=$(TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wcmount)
check "ERROR_POPUP=0: the finding logged, no box, its line says so" "log_has 'PROBLEM: Weapon component files are refused this session' && log_line 'Finding not shown (ERROR_POPUP=0): 1 finding(s), see above\.\$' && log_lacks 'message box not shown' && log_lacks 'Start-up summary not shown'"

echo "== I20 Windows refuses a new thread, the test build's TWEAKSV_TEST_REFUSE_THREAD: the start-up summary's thread and the late reports' tried again with the next problem"
SUMMARY_REFUSED='The start-up summary thread could not be started (Windows error 8'
first_line() { tr -d '\r' < TweaksV.log | grep -n -- "$1" | head -n 1 | cut -d: -f1; }
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 start-up summary' ASI=TweaksV_test.asi game full_game.exe latefailures 1)
check "the first start refused, said once; the next problem starts it: one summary of both, in the summary's box, no box of a part that stopped working" "[ \$(count 'TEST BUILD: Windows refuses the first 1 start(s) of the start-up summary thread (TWEAKSV_TEST_REFUSE_THREAD=1 start-up summary)\.') -eq 1 ] && [ \$(count '$SUMMARY_REFUSED') -eq 1 ] && [ \$(count 'Showing the start-up summary (2 problem(s))\.') -eq 1 ] && [ \$(count '$SUMMARY_BOX') -eq 1 ] && log_lacks 'after the start-up summary'"
check "said as it happened: the refusal, then the next problem, then the summary" "[ \$(first_line '$SUMMARY_REFUSED') -lt \$(first_line '$NOT_SAVED') ] && [ \$(first_line '$NOT_SAVED') -lt \$(first_line 'Showing the start-up summary') ]"
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='2 start-up summary' ASI=TweaksV_test.asi game full_game.exe latefailures 2)
check "refused again with the next problem: said again, the one after it starts it, with all three" "[ \$(count '$SUMMARY_REFUSED') -eq 2 ] && [ \$(count 'Showing the start-up summary (3 problem(s))\.') -eq 1 ] && [ \$(count '$SUMMARY_BOX') -eq 1 ]"
late_ini; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 start-up summary' ASI=TweaksV_test.asi game full_game.exe latefailures 1)
check "nothing to report at the start: the next problem alone in the summary's box" "[ \$(count '$SUMMARY_REFUSED') -eq 1 ] && [ \$(count 'Showing the start-up summary (1 problem(s))\.') -eq 1 ] && [ \$(count '$SUMMARY_BOX') -eq 1 ]"
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 start-up summary' TWEAKSV_TEST_GUARD_ROOM=1 ASI=TweaksV_test.asi game full_game.exe wcmount)
check "a finding is a next problem too: the summary's thread tried again for the start-up's problem, the finding in a box of its own" "[ \$(count '$SUMMARY_REFUSED') -eq 1 ] && [ \$(count 'Showing the start-up summary (1 problem(s))\.') -eq 1 ] && [ \$(count '$SUMMARY_BOX') -eq 1 ] && [ \$(count '$REFUSED_BOX') -eq 1 ]"
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 start-up summary' ASI=TweaksV_test.asi game full_game.exe)
check "no next problem: the start-up's problem stands in the log, which says why no box came" "log_has 'PROBLEM: TweaksV.ini: \[HEAP_ADJUSTER\] HEAP_SIZE=5000' && [ \$(count '$SUMMARY_REFUSED') -eq 1 ] && log_lacks 'Showing the start-up summary' && log_lacks 'message box not shown'"
late_ini; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 late report' ASI=TweaksV_test.asi game full_game.exe latefailures 2)
check "the late reports' thread refused: said once; the next problem starts it, with both in its one box" "[ \$(count 'The late report thread could not be started (Windows error 8') -eq 1 ] && [ \$(count 'Showing a problem after the start-up summary (2 problem(s))\.') -eq 1 ] && [ \$(count '$PART_BOX') -eq 1 ]"
late_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='0 start-up summary' ASI=TweaksV_test.asi game full_game.exe)
check "a number outside 1..9 is not taken: nothing refused, no line, the summary at once" "log_lacks 'TWEAKSV_TEST_REFUSE_THREAD' && log_lacks 'could not be started' && log_has 'Showing the start-up summary (1 problem(s))\.'"
fresh_ini; setkey HEAP_SIZE 5000; out=$(TWEAKSV_TEST_REFUSE_THREAD='1 start-up summary' game full_game.exe)
check "the Release build ignores the switch: nothing refused, no line, the summary at once" "log_lacks 'TWEAKSV_TEST_REFUSE_THREAD' && log_lacks 'could not be started' && log_has 'Start-up summary not shown (ERROR_POPUP=0): 1 problem(s), see above\.'"

echo "== I21 the harness itself: a status other than 0 after a fake's report fails the checks after it, as a run without a report does"
# ends.sh STATUS: a fake of three lines that prints a report and ends with STATUS; each check said runs in a subshell, so that it counts for nothing
printf '#!/bin/sh\necho "harness: a report"\nexit "$1"\n' > ends.sh
run_fake ends.sh 5; after_five=$(check 'the check after it' true)
run_fake ends.sh 0; after_none=$(check 'the check after it' true)
rm -f ends.sh "$B/harness.state"
check "status 5: the check after it fails, its line names the crash and the status; status 0: it runs" "[ \"\$after_five\" = 'CRASH AT EXIT  the check after it (exit status 5)' ] && [ \"\$after_none\" = 'PASS  the check after it' ]"

echo "== I22 a message box's whole text, which the test build logs on one line in its place: its heading, its items, ... and N more., its last line and the log's path"
# box_text [FILE]: the text of each box of the last run, a line each, a " | " for each line break. box_end LAST_LINE FOLDER: how a box ends. box_run
# [OPTION...]: a run of the test build that logs the boxes' text, TWEAKSV_TEST_NO_BOXES=2, with a profile folder that hides nothing in the log, so that
# the log's path stands as the box has it.
# wrong_keys N: the first N keys of TweaksV.ini but VERSION take a value no key takes, each a problem, each using its default, ERROR_POPUP's 1 among them.
box_text() { tr -d '\r' < "${1:-TweaksV.log}" | sed -n 's/^.*TEST BUILD: the text of that box: //p'; }
box_end() { echo "$1 Details are in TweaksV.log: | $2\\TweaksV.log | To stop this message, set ERROR_POPUP=0 in TweaksV.ini."; }
box_run() { out=$(TWEAKSV_TEST_NO_BOXES=2 USERPROFILE='C:\NoProfile' ASI=TweaksV_test.asi game full_game.exe "$@"); }
wrong_keys() {
	local last
	last=$(grep -n '^[A-Z_]*=' TweaksV.ini | grep -v ':VERSION=' | sed -n "$1p" | cut -d: -f1)
	sed -b -i "1,$last{/^VERSION=/!s/=.*\r\$/=x\r/}" TweaksV.ini
}
SUMMARY_HEAD='TweaksV could not apply everything: | '
SUMMARY_END=$(box_end 'The game continues without the affected parts.' "$bw")
fresh_ini; setkey ERROR_POPUP 1; setkey HEAP_SIZE 5000; box_run
check "one problem: the start-up summary's heading, the item, its last line, the log's path and how to stop the box, word for word, its bullet's bytes too; beside the line that names heading and last line alone" "[ \"\$(box_text)\" = \"${SUMMARY_HEAD}• [TweaksV] TweaksV.ini: [HEAP_ADJUSTER] HEAP_SIZE=5000 is outside 1..2047 - using 2047. | \$SUMMARY_END\" ] && [ \$(count 'TEST BUILD: message box not shown (TWEAKSV_TEST_NO_BOXES=2): TweaksV could not apply everything: \.\.\. The game continues without the affected parts\.') -eq 1 ]"
fresh_ini; setkey ERROR_POPUP 1; setkey HEAP_SIZE 5000; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "TWEAKSV_TEST_NO_BOXES=1, as every other run has it: that line alone, so that a problem's words stand once in the log, as the suites count them" "[ \$(count '$SUMMARY_BOX') -eq 1 ] && log_lacks 'the text of that box' && [ \$(count 'HEAP_SIZE=5000 is outside') -eq 1 ]"
fresh_ini; wrong_keys 41; box_run
check "41 problems, past the 2 KB of a Release log's line: all 41 in the log and the count, the first 40 items in the box in their order, then ... and 1 more., and the box's end" "[ \$(count 'PROBLEM') -eq 41 ] && log_has 'Showing the start-up summary (41 problem(s))\.' && [ \$(box_text | grep -o '• \[TweaksV\] TweaksV\.ini: ' | wc -l) -eq 40 ] && [ \$(box_text | wc -c) -gt 2048 ] && [[ \"\$(box_text)\" == \"${SUMMARY_HEAD}• [TweaksV] TweaksV.ini: [TWEAKSV] ENABLED=x is not a valid on/off value - using 1. | \"* ]] && [[ \"\$(box_text)\" == *\" | • ... and 1 more. | \$SUMMARY_END\" ]]"
fresh_ini; wrong_keys 40; box_run
check "40 problems: the 40 items, no ... and N more." "[ \$(count 'PROBLEM') -eq 40 ] && log_has 'Showing the start-up summary (40 problem(s))\.' && [ \$(box_text | grep -o '• ' | wc -l) -eq 40 ] && [[ \"\$(box_text)\" != *'... and '* ]] && [[ \"\$(box_text)\" == *\" | \$SUMMARY_END\" ]]"
late_ini; box_run latefailures 1
check "a part that stopped working after the start-up: its box's heading and item, word for word, the summary's last line" "[ \"\$(box_text)\" = \"TweaksV: a part stopped working after the start-up: | • [TweaksV] TEST_DUMP_SCRIPT: Script-blip_controller-1.bin was not saved: its code length, 16777217 bytes, is beyond 16 MB. | \$SUMMARY_END\" ]"
fresh_ini; setkey ERROR_POPUP 1; TWEAKSV_TEST_GUARD_ROOM=1 box_run wcmount
REFUSED_ITEM="• [WeaponLimits] Weapon component files are refused this session: dlcpacks:/weaponpack08/common/data/weaponcomponents.meta, the first, came once 472 definitions were loaded, the room being 470, the test build's (TWEAKSV_TEST_GUARD_ROOM=1) - the weapon components of the files refused are missing in the game. The log names every file refused."
REFUSED_END=$(box_end 'The game continues without the files refused.' "$bw")
check "a guard's finding: its box's heading, item and last line, word for word" "[ \"\$(box_text)\" = \"TweaksV: WeaponLimits refused files the game has no room for: | \$REFUSED_ITEM | \$REFUSED_END\" ]"
fresh_ini; setkey ERROR_POPUP 1; rm -rf appdata/TweaksV; echo "an older log" > TweaksV.log
cp full_game.exe GTA5_Enhanced.exe; TWEAKSV_TEST_NO_BOXES=2 USERPROFILE='C:\NoProfile' run_fake GTA5_Enhanced.exe TweaksV_test.asi lockedlog
HELD_END=$(box_end 'The game continues without the affected parts.' "$AW\\TweaksV")
check "the game folder's log held by another program: the box names the log in use, in %LOCALAPPDATA%\\TweaksV" "[[ \"\$(box_text appdata/TweaksV/TweaksV.log)\" == *\" | \$HELD_END\" ]]"
rm -f TweaksV.log; rm -rf appdata/TweaksV

echo "== I23 an exception nothing nearer catches (the test build's TWEAKSV_TEST_THROWS): each of the three catch-alls, and the game goes on"
fresh_ini; setkey ERROR_POPUP 1; TWEAKSV_TEST_THROWS=RageOpenV box_run
check "a part, the one that would show the summary itself: said once, its problem word for word, the only one, in the summary's box all the same; that part not started, the parts before and after it started" "[ \$(count '\[RageOpenV\] TEST BUILD: RageOpenV throws in place of starting (TWEAKSV_TEST_THROWS=RageOpenV)\.') -eq 1 ] && log_line '\[RageOpenV\] PROBLEM: unexpected exception - this part is skipped for this session\$' && [ \$(count 'PROBLEM') -eq 1 ] && log_has 'Showing the start-up summary (1 problem(s))\.' && [ \"\$(box_text)\" = \"${SUMMARY_HEAD}• [RageOpenV] unexpected exception - this part is skipped for this session | \$SUMMARY_END\" ] && has '$ROV_OFF' && has '$ALL_ON_HEAP' && has '$PACK_ON' && has '$STORY_ON' && parts_after"
fresh_ini; setkey ERROR_POPUP 1; setkey DIAGNOSTIC_MODE 1; TWEAKSV_TEST_THROWS='measurement LightCapacity' box_run
check "a measurement, with DIAGNOSTIC_MODE=1: said once, its line word for word, nothing else under its tag; logged and no problem, no box; the measurements before and after it go on" "[ \$(count '\[LightCapacity\]') -eq 2 ] && log_line '\[LightCapacity\] TEST BUILD: measurement LightCapacity throws in place of starting (TWEAKSV_TEST_THROWS=measurement LightCapacity)\.\$' && log_line '\[LightCapacity\] DIAGNOSTIC: unexpected exception - a measurement of this part is skipped for this session\.\$' && log_lacks 'PROBLEM' && log_lacks 'Showing the start-up summary' && log_lacks 'message box not shown' && log_has '\[SirenSettings\] DIAGNOSTIC: ' && log_has '\[Population\] DIAGNOSTIC: '"
fresh_ini; setkey ERROR_POPUP 1; TWEAKSV_TEST_THROWS=start-up box_run
check "the start of them all, which only DllMain's catches: said once, its problem word for word, the only one, in the summary's box; no part started, the game as it came, its report whole" "[ \$(count '\[TweaksV\] TEST BUILD: start-up throws in place of starting (TWEAKSV_TEST_THROWS=start-up)\.') -eq 1 ] && log_line '\[TweaksV\] PROBLEM: Unexpected exception during the start-up - the rest of TweaksV is skipped this session\.\$' && [ \$(count 'PROBLEM') -eq 1 ] && log_has 'Showing the start-up summary (1 problem(s))\.' && [ \"\$(box_text)\" = \"${SUMMARY_HEAD}• [TweaksV] Unexpected exception during the start-up - the rest of TweaksV is skipped this session. | \$SUMMARY_END\" ] && has '$GAME_HEAP' && has '$MODKIT_OFF' && has '$PACK_OFF' && has '$BOX_OFF' && has '$DECAL_OFF' && has '$ROV_OFF' && has '$STORY_OFF' && has 'protect: all restored (RX)'"
fresh_ini; out=$(TWEAKSV_TEST_THROWS=start-up game full_game.exe)
check "the Release build ignores TWEAKSV_TEST_THROWS: every part started, no TEST BUILD line, no problem" "has '$ALL_ON_HEAP' && has '$ROV_ON' && parts_after && log_lacks 'TEST BUILD' && log_lacks 'PROBLEM'"

echo
echo "INTEGRATION SCENARIOS: $pass passed, $fail failed"
