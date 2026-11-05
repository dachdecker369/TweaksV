#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The sirens per vehicle's damage (sirendamage.cpp), the half of the part's one write that InitSirensPerVehicle asks after the lights'. The
# fake game holds the game's damage code as code that runs, around the patterns' bytes, and makes its bone structures as the game does: with the
# key on, every site found and written and nothing else, each page its own protection; the room behind each structure filled at a model's load,
# a plane's gear doors in their slots, a bone in any case and a bone past 127; a hit, an impact and a wreck on lights past 20 and the glass's
# flags, each loop run; every N from 21 to 32 by a loop's bounds, the room 64 bytes whatever N; DRY_RUN, the part off, the plain game; each site
# failing alone, as another mod would change it or away from its loop, the call hooked or elsewhere, the fill not a function start, with nothing
# written; a model whose room cannot be filled, said once and the game going on; the lines of the rooms, capped.
# Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=86
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata
SIRENS_ON=1

# field NAME: what the report says after "sd NAME: "; is NAME TEXT: that it says just this. line TEXT: a line under the part's tag, whole.
field() { sed -nE "s/.*\| sd $1: ([^|]*[^| ]) \|.*/\1/p" <<< "$out"; }
is() { [ "$(field "$1")" = "$2" ]; }
line() { log_line "\[SirenSettings\] $1\$"; }
RX() { [[ "$out" =~ stubs:\ [0-9]+,\ all\ RX ]] && has 'protect: all restored (RX)'; }
NO_PROBLEM="log_lacks '\[SirenSettings\] PROBLEM'"
NOT_RAISED='PROBLEM: The sirens per vehicle are not raised this session:'
KEEP='Vehicles keep 20 sirens\.'
AS_PLACED="sd sites: as placed, the maker's call leads to the maker |"
LABELS='I-E-[LNXY][0-9]'
RVA='RVA 0x[0-9A-F]*'

# What the loops leave: for each siren hit, its +0x860 bit, +0x865 and the glass whose child broke; for each siren of an impact the damage object's
# +0x440 bit, +0x445, the glass and the bones hidden; the wreck's bits on the seven models: 20 sirens, 32, 24, a few past 20, the plane, the one with
# bones past 127, 8 sirens; the glass bones with the flags, by the flags' loop and the three classes, on the model of 20, of 32, the plane and the one
# whose glass past siren20 lies past bone 127.
HIT_20='5 0x00000010 1 0x00000010, 20 0x00080000 1 0x00080000, 21 0x00000000 0 0x00000000, 25 0x00000000 0 0x00000000, 32 0x00000000 0 0x00000000,'
HIT_32='5 0x00000010 1 0x00000010, 20 0x00080000 1 0x00080000, 21 0x00100000 1 0x00100000, 25 0x01000000 1 0x01000000, 32 0x80000000 1 0x80000000,'
HIT_24='5 0x00000010 1 0x00000010, 20 0x00080000 1 0x00080000, 21 0x00100000 1 0x00100000, 25 0x00000000 0 0x00000000, 32 0x00000000 0 0x00000000,'
IMPACT_20='5 0x00000010 1 0x00000010 hidden 0x00000010, 20 0x00080000 1 0x00080000 hidden 0x00080000, 21 0x00000000 0 0x00000000 hidden 0x00000000, 25 0x00000000 0 0x00000000 hidden 0x00000000, 32 0x00000000 0 0x00000000 hidden 0x00000000,'
IMPACT_32='5 0x00000010 1 0x00000010 hidden 0x00000010, 20 0x00080000 1 0x00080000 hidden 0x00080000, 21 0x00100000 1 0x00100000 hidden 0x00100000, 25 0x01000000 1 0x01000000 hidden 0x01000000, 32 0x80000000 1 0x80000000 hidden 0x80000000,'
IMPACT_24='5 0x00000010 1 0x00000010 hidden 0x00000010, 20 0x00080000 1 0x00080000 hidden 0x00080000, 21 0x00100000 1 0x00100000 hidden 0x00100000, 25 0x00000000 0 0x00000000 hidden 0x00000000, 32 0x00000000 0 0x00000000 hidden 0x00000000,'
WRECK_20='0x000FFFFF 0x000FFFFF 0x000FFFFF 0x0000000F 0x00000003 0x00000010 0x000000FF'
WRECK_32='0x000FFFFF 0xFFFFFFFF 0x00FFFFFF 0x0110000F 0x00100003 0x00200010 0x000000FF'
WRECK_24='0x000FFFFF 0x00FFFFFF 0x00FFFFFF 0x0010000F 0x00100003 0x00200010 0x000000FF'
FLAGS_20='0x000FFFFF 0x000FFFFF 0x000FFFFF 0x000FFFFF, 0x000FFFFF 0x000FFFFF 0x000FFFFF 0x000FFFFF, 0x00003FFF 0x00003FFF 0x00003FFF 0x00003FFF, 0x00000000 0x00000000 0x00000000 0x00000000,'
FLAGS_32='0x000FFFFF 0x000FFFFF 0x000FFFFF 0x000FFFFF, 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF, 0x00103FFF 0x00103FFF 0x00103FFF 0x00103FFF, 0x00000000 0x00000000 0x00000000 0x00000000,'
FLAGS_24='0x000FFFFF 0x000FFFFF 0x000FFFFF 0x000FFFFF, 0x00FFFFFF 0x00FFFFFF 0x00FFFFFF 0x00FFFFFF, 0x00103FFF 0x00103FFF 0x00103FFF 0x00103FFF, 0x00000000 0x00000000 0x00000000 0x00000000,'
MODELS='7 made, 0 with bad arguments, result and last error kept 7, 7 through its code, rooms right 7 of 7, their structures right 7'
CHILDREN='0 asked wrongly'
# the game's own numbers in every loop, whatever was asked for and refused
at_20() { is hit "$HIT_20" && is impact "$IMPACT_20" && is wreck "$WRECK_20" && is flags "$FLAGS_20" && is children "$CHILDREN" && is pool 'stride 0x34C, allocation 0x34C, 0 traps'; }

# Every write, the operand at N=32: what names it, its old and its new bytes (an apostrophe as any character).
WRITES=(
	"siren hit loop.s start (I-E-X1)|89 00 00 00|3C 03 00 00"
	"siren hit loop.s end (I-E-X2)|9D 00 00 00|5C 03 00 00"
	"siren hit loop.s index (I-E-X2)|77 FF FF FF|C4 FC FF FF"
	"siren hit loop.s glass bone (I-E-X3)|24|30"
	"siren impact loop.s bound (I-E-L05)|14|20"
	"siren impact loop.s bone (I-E-N04)|99 00 00 00|4C 03 00 00"
	"siren impact loop.s glass bone (I-E-N05)|AD 00 00 00|6C 03 00 00"
	"hidden sirens. loop.s bound (I-E-L03)|14|20"
	"hidden sirens. loop.s bone (I-E-N03)|99 00 00 00|4C 03 00 00"
	"wrecked sirens. loop.s bound (I-E-L04)|14|20"
	"wrecked sirens. loop.s bone (I-E-N02)|99 00 00 00|4C 03 00 00"
	"siren glass flags. loop.s bound (I-E-L07)|14|20"
	"siren glass flags. loop.s glass bone (I-E-N06)|AD 00 00 00|6C 03 00 00"
	"siren glass flags. class.s start 1 of 3 (I-E-L08)|EC FF FF FF|E0 FF FF FF"
	"siren glass flags. class.s start 2 of 3 (I-E-L08)|EC FF FF FF|E0 FF FF FF"
	"siren glass flags. class.s start 3 of 3 (I-E-L08)|EC FF FF FF|E0 FF FF FF"
	"siren glass flags. class.s glass bone 1 of 3 (I-E-N09)|C1 00 00 00|8C 03 00 00"
	"siren glass flags. class.s glass bone 2 of 3 (I-E-N09)|C1 00 00 00|8C 03 00 00"
	"siren glass flags. class.s glass bone 3 of 3 (I-E-N09)|C1 00 00 00|8C 03 00 00"
	"bone structures. pool.s stride (I-E-Y1)|4C 03 00 00|8C 03 00 00"
	"bone structure.s allocation (I-E-Y2)|4C 03 00 00|8C 03 00 00"
)
# all_written KIND: each write's line of that kind, exactly once.
all_written() {
	local write what old new
	for write in "${WRITES[@]}"; do
		IFS='|' read -r what old new <<< "$write"
		if [ "$1" = dry ]; then
			line "DRY_RUN: would patch the $what: $old -> $new" && [ "$(count "would patch the $what:")" -eq 1 ] || return 1
		else
			line "DIAGNOSTIC: $what at $RVA: $old -> $new" && [ "$(count "DIAGNOSTIC: $what at ")" -eq 1 ] || return 1
		fi
	done
}

echo "== D1 the plain game, with none of the game's code: the part's problem, nothing raised"
fresh_ini; out=$(game plain_game.exe)
check "the problem, no raise" "log_has '$NOT_RAISED' && log_lacks 'The sirens per vehicle are raised'"

echo "== D2 the raise's gates: nothing of the damage looked for, the code as placed, each loop at the game's 20"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey SIRENS_PER_VEHICLE 20 SIRENSETTINGS; out=$(game full_game.exe sdrun)
check "SIRENS_PER_VEHICLE=20: no pattern looked for, no line, no problem; the loops run on the game's own structures" "log_lacks '$LABELS' && has \"$AS_PLACED\" && at_20 && log_lacks 'The sirens per vehicle are' && $NO_PROBLEM"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 SIRENSETTINGS; out=$(game full_game.exe sdrun)
check "the part off: the same" "log_lacks '$LABELS' && has \"$AS_PLACED\" && at_20 && log_lacks 'The sirens per vehicle are' && $NO_PROBLEM"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 LIGHTCAPACITY; out=$(game full_game.exe sdrun)
check "the light stores off: the problem, nothing looked for" "log_has '$NOT_RAISED the light stores are not raised' && log_lacks '$LABELS' && has \"$AS_PLACED\" && at_20"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game_1158_13.exe sdrun)
check "another game build: the problem, nothing looked for" "log_has '$NOT_RAISED this game build is 1.0.1158.13' && log_lacks '$LABELS' && has \"$AS_PLACED\" && at_20"

echo "== D3 SIRENS_ON, 32 sirens: every site written and nothing else, the room filled, damage past 20"
fresh_ini; out=$(game full_game.exe sdrun); echo "      ${out##*sd sites}"
check "the part's line, no problem" "line 'The sirens per vehicle are raised to 32\.' && $NO_PROBLEM"
check "every operand raised, nothing else changed, the call through TweaksV's stub; the stored damage state's loop among what stays; each page its own protection, each stub read+execute" "has \"sd sites: raised to 32, nothing else, the maker's call leads to a TweaksV stub |\" && RX"
check "the pool's stride and the allocation raised together: no trap" "is pool 'stride 0x38C, allocation 0x38C, 0 traps'"
check "7 models through the stub, its arguments, result and last error as the game's; the room as the game's bytes, a plane's gear doors in their slots, the bones by tag in any case, a bone past 127 none; the rest of each structure as made, but for the set's bits past siren20" "is models '$MODELS'"
check "the game's own fill on TweaksV's table, through its prologue, once a model: 24 entries, flag 1" "is fill \"TweaksV's 7 times, flag 1, 24 entries\""
check "a hit: on sirens 21, 25 and 32 the vehicle's bit, +0x865 and the glass's child, as on 5 and 20" "is hit '$HIT_32'"
check "an impact: the damage object's bit, +0x445 and the glass's child; then the marked bone hidden" "is impact '$IMPACT_32'"
check "a wreck: each model's sirens whose bones exist; a bone the game names past 127 as ever, none past siren20" "is wreck '$WRECK_32'"
check "the flags, by their loop and each of the three classes, on every glass bone, a plane's gear doors and its glass past 20; nothing else in the children's array; a glass bone past 127 past siren20 never reaches the game's lookup of its child" "is flags '$FLAGS_32' && is children '$CHILDREN'"

echo "== D4 DIAGNOSTIC_MODE=1, the test build: each pattern's lines, each write's, the sites, a room's line for each model with bones past siren20"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe sdrun)
check "the last error kept across the lines the wrapper writes, in the build whose lines leave one of their own" "is models '$MODELS'"
check "the 17 patterns found once each, 2 three times, word for word, each by its RVA" "[ \$(grep -c -E 'Pattern ${LABELS}+ found: .* 1 match,' TweaksV.log) -eq 17 ] && [ \$(grep -c -E 'Pattern I-E-(L08|N09) found: .* 3 matches,' TweaksV.log) -eq 2 ]"
check "every write: what it is, the site's old bytes and the new, exactly once" "all_written diagnostic"
check "the call redirected, with its old and new bytes" "[ \$(count 'DIAGNOSTIC: bone structure maker.s call at $RVA: E8') -eq 1 ]"
check "the call's target, the allocation and the fill by the unwind data, the pool not made yet, and the hit's read left as it is" "line 'DIAGNOSTIC: bone structure maker.s call at $RVA -> $RVA, its allocation at $RVA, the game.s fill at $RVA, each by the unwind data (I-E-Y3, I-E-Y2, I-E-Y4)' && line 'DIAGNOSTIC: the bone structures. pool at $RVA is not made yet (I-E-Y2)' && line 'DIAGNOSTIC: siren hit loop.s bone read at $RVA holds 0x10 (I-E-X4), left as it is'"
check "the lines of the rooms: the bones past siren20 of each model that has some, those lost past bone 127, and no line for the one that has neither" "line 'DIAGNOSTIC: the room of vehicle model 0xA1B2C3D4 holds 12 siren and 12 glass bones past siren20, up to siren32 and siren_glass32\.' && line 'DIAGNOSTIC: the room of vehicle model 0x24242424 holds 4 siren and 4 glass bones past siren20, up to siren24 and siren_glass24\.' && line 'DIAGNOSTIC: the room of vehicle model 0x0DD50A25 holds 2 siren and 1 glass bones past siren20, up to siren25 and siren_glass25\.' && line 'DIAGNOSTIC: the room of vehicle model 0xA1A1A1A1 holds 1 siren and 1 glass bones past siren20, up to siren21 and siren_glass21\.' && line 'DIAGNOSTIC: the room of vehicle model 0x0C0FFEE0 holds 1 siren and 0 glass bones past siren20, up to siren22 and no siren_glass; lost past bone 127: 1 siren and 1 glass\.' && line 'DIAGNOSTIC: the room of vehicle model 0x2C3D4E5F holds 0 siren and 0 glass bones past siren20; lost past bone 127: 1 siren and 0 glass\.' && [ \$(count 'DIAGNOSTIC: the room of vehicle model') -eq 6 ]"
check "no problem; each page its own protection" "$NO_PROBLEM && RX"

echo "== D5 N takes the loops' bounds alone, and the room stays 64 bytes"
fresh_ini; setkey SIRENS_PER_VEHICLE 24 SIRENSETTINGS; out=$(game full_game.exe sdrun)
check "N=24: each operand raised to 24, nothing else; stride and allocation as at 32; 7 rooms right" "has \"sd sites: raised to 24, nothing else, the maker's call leads to a TweaksV stub |\" && line 'The sirens per vehicle are raised to 24\.' && is pool 'stride 0x38C, allocation 0x38C, 0 traps' && is models '$MODELS'"
check "N=24: a hit and an impact past it reach nothing, the wreck and the flags stop at 24" "is hit '$HIT_24' && is impact '$IMPACT_24' && is wreck '$WRECK_24' && is flags '$FLAGS_24' && is children '$CHILDREN'"
for n in 21 22 23 25 26 27 28 29 30 31; do
	fresh_ini; setkey SIRENS_PER_VEHICLE $n SIRENSETTINGS; out=$(game full_game.exe sdrun)
	check "N=$n: each operand raised to $n, nothing else; the model of 32's wreck its first $n bits" "has \"sd sites: raised to $n, nothing else, the maker's call leads to a TweaksV stub |\" && is models '$MODELS' && [ \"\$(field wreck | cut -d' ' -f2)\" = $(printf '0x%08X' $(( (1 << n) - 1 ))) ]"
done

echo "== D6 DRY_RUN=1: every site found and checked, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe sdrun)
check "the would lines, each write's and the call's, word for word" "all_written dry && line 'DRY_RUN: would redirect the bone structure maker.s call at $RVA\.'"
check "nothing written or redirected; no raise line, no problem" "has \"$AS_PLACED\" && log_lacks 'The sirens per vehicle are raised' && log_lacks 'DIAGNOSTIC: bone structure maker.s call at $RVA: E8' && $NO_PROBLEM"
check "the game's own numbers in every loop; TweaksV's fill not called" "at_20 && is fill \"TweaksV's 0 times, flag 4294967295, 0 entries\""

echo "== D7 a site another mod changed, a byte its pattern pins, each alone: the scanner's line, the one problem, nothing written"
for label in I-E-X1 I-E-X2 I-E-X3 I-E-X4 I-E-L05 I-E-N04 I-E-N05 I-E-L03 I-E-N03 I-E-L04 I-E-N02 I-E-L07 I-E-N06 I-E-Y1 I-E-Y2 I-E-Y3 I-E-Y4; do
	fresh_ini; out=$(game full_game.exe sdrun sdchanged:$label)
	check "$label" "log_has 'Pattern $label not found: ' && line '$NOT_RAISED $label was not found\. $KEEP' && [ \$(count '$NOT_RAISED') -eq 1 ] && log_lacks 'The sirens per vehicle are raised' && has 'sd sites: as placed'"
done
for label in I-E-L08 I-E-N09; do
	fresh_ini; out=$(game full_game.exe sdrun sdchanged:$label)
	check "$label, one of its three: too few" "line '$NOT_RAISED the game code holds $label 2 times, not the expected 3\. $KEEP' && has 'sd sites: as placed' && log_lacks 'The sirens per vehicle are raised'"
	fresh_ini; out=$(game full_game.exe sdrun sdfourth:$label)
	check "$label, a fourth copy: too many" "line '$NOT_RAISED the game code holds $label 4 or more times, not the expected 3\. $KEEP' && has 'sd sites: as placed' && log_lacks 'The sirens per vehicle are raised'"
done
fresh_ini; out=$(game full_game.exe sdrun sdchanged:I-E-L04 sdchanged:I-E-Y1)
check "two at once: the scanner names both, the problem the first, one problem" "log_has 'Pattern I-E-L04 not found: ' && log_has 'Pattern I-E-Y1 not found: ' && line '$NOT_RAISED I-E-L04 was not found\. $KEEP' && [ \$(count '$NOT_RAISED') -eq 1 ]"
fresh_ini; out=$(game full_game.exe sdrun sdchanged:I-E-L04 sdchanged:I-E-L08)
check "a class's count beside a site missing: the problem the first, a line of its own for the count, which the scanner does not say" "line '$NOT_RAISED I-E-L04 was not found\. $KEEP' && line 'the game code holds I-E-L08 2 times, not the expected 3\.' && [ \$(count '$NOT_RAISED') -eq 1 ]"

echo "== D8 a site away from its loop, each alone: the one problem, nothing written"
for pair in I-E-X2:I-E-X1 I-E-X4:I-E-X2 I-E-X3:I-E-X4 I-E-N04:I-E-L05 I-E-N05:I-E-N04 I-E-N03:I-E-L03 I-E-N02:I-E-L04 I-E-N06:I-E-L07 I-E-N09:I-E-L08; do
	label=${pair%:*}; before=${pair#*:}
	fresh_ini; out=$(game full_game.exe sdrun sdfar:$label)
	check "$label, behind $before" "line '$NOT_RAISED $label at $RVA does not lie behind $before at $RVA in one loop\. $KEEP' && has 'sd sites: as placed' && log_lacks 'The sirens per vehicle are raised'"
done
fresh_ini; out=$(game full_game.exe sdfar:I-E-L03)
order=$(tr -d '\r' < TweaksV.log | sed -n 's/.*I-E-N03 at RVA \(0x[0-9A-F]*\) does not lie behind I-E-L03 at RVA \(0x[0-9A-F]*\) in one loop.*/\1 \2/p')
check "the bound moved past its read, the two out of order by their RVAs: the same problem" "[ -n '$order' ] && (( ${order% *} < ${order#* } )) && has 'sd sites: as placed' && log_lacks 'The sirens per vehicle are raised'"

echo "== D9 the call of the function that makes a structure, the allocation and the fill: each not as the findings have it, alone"
fresh_ini; out=$(game full_game.exe sdhooked)
check "the call another mod redirected already: refused, the mod's call kept" "line '$NOT_RAISED the bone structure maker.s call at $RVA is not a call into the game (another mod may have hooked it already) (I-E-Y3)\. $KEEP' && has \"sd sites: as placed, the maker's call leads to another mod |\""
fresh_ini; out=$(game full_game.exe sdelsewhere)
check "the call to another function of the game than the allocation's: refused" "line '$NOT_RAISED I-E-Y2 at $RVA does not lie in the bone structure maker at $RVA: the game.s unwind data puts it in the function at $RVA\. $KEEP' && has \"sd sites: as placed, the maker's call leads to another function of the game |\""
fresh_ini; out=$(game full_game.exe sdallocout)
check "the allocation where no unwind data names a function: refused" "line '$NOT_RAISED I-E-Y2 at $RVA does not lie in the bone structure maker at $RVA: the game.s unwind data names no function there\. $KEEP' && has \"$AS_PLACED\""
fresh_ini; out=$(game full_game.exe sdnotstart)
check "the fill's pattern inside a function: refused" "line '$NOT_RAISED the bone structures. fill at $RVA is not a function start (inside the function at $RVA (unwind data)) (I-E-Y4)\. $KEEP' && has \"$AS_PLACED\""
fresh_ini; out=$(game full_game.exe sdnounwind)
check "the fill's pattern where no unwind data names a function: refused" "line '$NOT_RAISED the bone structures. fill at $RVA is not a function start (no unwind data) (I-E-Y4)\. $KEEP' && has \"$AS_PLACED\""
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sdpoolmade)
check "the pool made before TweaksV starts: refused" "line '$NOT_RAISED the bone structures. pool at $RVA is made already, so TweaksV has started too late to give its structures their room (I-E-Y2)\. $KEEP' && has \"$AS_PLACED\" && log_lacks 'The sirens per vehicle are raised'"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sdpoolshape)
check "another instruction before the allocation: refused, since no pattern pins that load" "line '$NOT_RAISED the bone structures. pool cannot be read: no load of it before the allocation at $RVA (I-E-Y2)\. $KEEP' && has \"$AS_PLACED\" && log_lacks 'The sirens per vehicle are raised'"
fresh_ini; out=$(game full_game.exe sdpoolout)
check "that load leading outside the game: refused" "line '$NOT_RAISED the bone structures. pool cannot be read: no load of it before the allocation at $RVA (I-E-Y2)\. $KEEP' && has \"$AS_PLACED\" && log_lacks 'The sirens per vehicle are raised'"
fresh_ini; out=$(game full_game.exe nodyncode)
check "Windows' dynamic code policy, no call stub can be made: refused, nothing changed" "log_has '$NOT_RAISED' && has \"$AS_PLACED\" && log_lacks 'The sirens per vehicle are raised'"

echo "== D10 a model whose room cannot be filled: said once, the game going on, the other models' rooms right"
PAST='The bones past siren20 are not looked up for every vehicle model this session: vehicle model'
TAIL='so its sirens past 20 stay dark\. Only the first such model is named\.'
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sdrun sdfillfault)
check "the game's fill faults on it: the one problem; the first 20 of its room stand, its structure as made" "line \"PROBLEM: $PAST 0x0BAD0003 faulted while its room was filled (an access violation), $TAIL\" && [ \$(count '$PAST') -eq 1 ] && is fault 'room right, structure right'"
check "the raise stands, the seven rooms right, the last error kept" "line 'The sirens per vehicle are raised to 32\.' && is models '8 made, 0 with bad arguments, result and last error kept 8, 8 through its code, rooms right 7 of 7, their structures right 7'"
fresh_ini; out=$(game full_game.exe sdrun sdnoholder)
check "one with no bone structure, as the game's readers allow: nothing to fill, no problem" "log_lacks '$PAST' && $NO_PROBLEM && is noholder 'room byte 0xAB' && is models '8 made, 0 with bad arguments, result and last error kept 8, 7 through its code, rooms right 7 of 7, their structures right 7'"
fresh_ini; out=$(game full_game.exe sdrun sdbadholder)
check "one whose bone structure cannot be read: the one problem, nothing of it touched" "line \"PROBLEM: $PAST 0x0BAD0001 has a bone structure that cannot be read (the model info.s +0xB0), $TAIL\" && [ \$(count '$PAST') -eq 1 ] && is noholder 'room byte 0xAB'"
fresh_ini; out=$(game full_game.exe sdrun sdbadholder sdfillfault)
check "both: only the first named" "log_has '0x0BAD0001 has a bone structure' && [ \$(count '$PAST') -eq 1 ] && is fault 'room right, structure right'"
fresh_ini; out=$(game full_game.exe sdrun sdbadstructure)
check "one whose bone structure cannot be written: the one problem, the game's fill not called" "line \"PROBLEM: $PAST 0x0BAD0004 faulted while its room was filled (an access violation), $TAIL\" && [ \$(count '$PAST') -eq 1 ] && is models '8 made, 0 with bad arguments, result and last error kept 8, 7 through its code, rooms right 7 of 7, their structures right 7'"

echo "== D11 20,001 more models with bones past siren20, DIAGNOSTIC_MODE=1: the lines of the rooms capped"
fresh_ini; setkey DIAGNOSTIC_MODE 1; RUN_TIMEOUT=120 out=$(game full_game.exe sdrun sdmany)
check "20,000 lines and the cap's line, once" "[ \$(count 'DIAGNOSTIC: the room of vehicle model') -eq 20000 ] && line 'DIAGNOSTIC: 20000 lines of vehicle models. rooms written - from now on they are not written\.' && [ \$(count 'lines of vehicle models') -eq 1 ]"
check "the 20,008 models made through the stub, the last error kept" "[[ \"\$(field models)\" == '20008 made, 0 with bad arguments, result and last error kept 20008,'* ]]"

echo
echo "SIREN DAMAGE SCENARIOS: $pass passed, $fail failed"
