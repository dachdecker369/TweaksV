#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The siren count, a limit of the SirenSettings part measured first (TweaksVSirenCount): with DIAGNOSTIC_MODE=1 the fake game's
# four sites found, checked and logged, the vehicle lights' call of the siren lights' renderer redirected, and the fake's
# vehicles drawn through it from two threads at once - the game's renderer first, with its arguments, its result and its last
# error, then each model counted once and summed up 10 seconds after the first, and again when the models change, and with
# nothing drawn no summary at all; models that cannot be read, one whose names disagree with its bone structure and one whose
# lookup faults; more models than the table holds; each pattern missing, the two calls another mod redirected, the renderer's
# call to another function, find-bone-by-name not a function start and without unwind data, the plain game, DRY_RUN and
# Windows' dynamic code policy - each logged, never a problem, nothing redirected; without DIAGNOSTIC_MODE nothing looked for.
# And the raise, SIRENS_PER_VEHICLE above 20: the fake's parser reads settings of 20 to 40 items through its hook, its cars get states as big
# as the allocation's site says, and its renderer's own loop, called again on the stand-ins, checks what each call lights against the file's items: a
# twin, a replacement, the tables full, 24 sirens, broken sirens past 20, a state made anew, a setting replaced while drawn; the game's build, the light
# stores and each site failing alone with nothing written, the hook taken back, DRY_RUN, the IDs' job on and off.
# Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=145
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# What the fake reports: its stand-ins, how its vehicles' sirens were drawn - 100 passes of four models from each of two
# threads - and whether its sites and data are as it placed them
BUILT='siren count: stand-ins built |'
COUNTED='siren draws: 800 through a TweaksV stub, the renderer 800 times, args ok, result and last error kept 800 of 800, 256 lookups in turn |'
UNCOUNTED='siren draws: 800 through the renderer, the renderer 800 times, args ok, result and last error kept 800 of 800, 0 lookups in turn |'
PLACED='siren sites and data: as placed |'
# The module's lines, word for word (an apostrophe as any character)
sc_line() { log_line "\[SirenSettings\] $1\$"; }
NOT_MEASURED='DIAGNOSTIC: the siren count is not measured:'
SUMMARY='DIAGNOSTIC: 4 vehicle models with sirens seen in 800 passes of the siren lights. renderer - the most sirens a model.s bone structure names: 20 of 20 (model 0x79FBB0C5); 1 with siren bones past siren20, which the game never lights; 1 losing siren bones past bone 127, which a bone structure cannot name; 0 that could not be read.'
LINE_32='DIAGNOSTIC: vehicle model 0xA1B2C3D4, 100 bones - siren and glass bones by name: 32 and 32, up to siren32 and siren_glass32; named by its bone structure: 20 and 20; past siren20, never lit: 12 and 12.'
LINE_LOST='DIAGNOSTIC: vehicle model 0x0C0FFEE0, 150 bones - siren and glass bones by name: 10 and 10, up to siren10 and siren_glass10; named by its bone structure: 8 and 10; lost past bone 127, found by name but not named by its bone structure: 2 and 0 (the highest bone 129).'
SITES="\[SirenSettings\] DIAGNOSTIC: .* (I-E-B[1-4])"

echo "== C1 DIAGNOSTIC_MODE=0: nothing looked for, the call not redirected, the sirens drawn as the game draws them"
fresh_ini; out=$(game full_game.exe scdraw); echo "      $out"
check "no siren count pattern looked for, no SirenSettings line but the siren IDs' two and SIRENS_PER_VEHICLE's" "log_lacks 'I-E-B' && log_lacks '\[SirenSettings\] DIAGNOSTIC' && [ \$(count '\[SirenSettings\]') -eq 3 ] && sc_line 'SIRENS_PER_VEHICLE=20 - the sirens per vehicle are not raised.'"
check "the stand-ins built; 800 passes through the renderer itself, its arguments, result and last error as the game's, no lookup" "has '$BUILT' && has '$UNCOUNTED'"
check "sites and data as placed, no call stub of its own (the 8 are the other parts'), every page its own protection" "has '$PLACED' && has 'stubs: 8, all RX' && has 'protect: all restored (RX)'"

echo "== C2 DIAGNOSTIC_MODE=1: the four sites found and logged, the call redirected, the models counted and summed up 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw wait); echo "      $out"
renderer=$(tr -d '\r' < TweaksV.log | sed -n 's/.*siren lights. renderer call at RVA 0x[0-9A-F]* -> RVA \(0x[0-9A-F]*\), .*/\1/p')
function=$(tr -d '\r' < TweaksV.log | sed -n 's/.*, in the function at RVA \(0x[0-9A-F]*\) (unwind data) (I-E-B2)$/\1/p')
check "the 4 patterns each found exactly once, none of the raise's looked for" "[ \$(grep -c -E 'Pattern I-E-B[1-4] found: .* 1 match,' TweaksV.log) -eq 4 ] && log_lacks 'Pattern I-E-Z'"
check "the vehicle lights' two calls, word for word" "sc_line 'DIAGNOSTIC: siren lights. renderer call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]*, siren settings getter call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (I-E-B1)'"
check "the renderer's bone read, its two offsets, in the function the renderer's call leads to, by the fake's unwind data" "sc_line 'DIAGNOSTIC: bone structure offset at RVA 0x[0-9A-F]* holds 0xB0, siren1.s bone offset at RVA 0x[0-9A-F]* holds 0x99, in the function at RVA 0x[0-9A-F]* (unwind data) (I-E-B2)' && [ -n '$renderer' ] && [ '$renderer' = '$function' ]"
check "the glass bones' offset and find-bone-by-name, a function start by the unwind data, its bone count offset" "sc_line 'DIAGNOSTIC: siren_glass1.s bone offset at RVA 0x[0-9A-F]* holds 0xAD (I-E-B3)' && sc_line 'DIAGNOSTIC: find-bone-by-name at RVA 0x[0-9A-F]*, a function start (unwind data), bone count offset at RVA 0x[0-9A-F]* holds 0x5E (I-E-B4)'"
check "the one write, the redirected call with its old and new bytes" "[ \$(count 'DIAGNOSTIC: siren lights. renderer call at RVA 0x[0-9A-F]*: E8') -eq 1 ] && sc_line 'DIAGNOSTIC: siren lights. renderer call at RVA 0x[0-9A-F]*: E8\( [0-9A-F][0-9A-F]\)\{4\} -> E8\( [0-9A-F][0-9A-F]\)\{4\}'"
check "the announcement, word for word" "sc_line 'DIAGNOSTIC: the siren lights. renderer call at RVA 0x[0-9A-F]* is redirected (I-E-B1): the vehicle models with sirens on, each counted once with its siren bones looked up by name up to siren32, are logged 10 seconds after the first, then every minute while they change.'"
check "800 passes through TweaksV's stub: the renderer each time, with the game's arguments, its result and last error back; 64 lookups for each of the 4 models, each after its vehicle's renderer" "has '$BUILT' && has '$COUNTED'"
check "the summary 10 s in, word for word: 4 models from two threads at once, each counted once - the police car's 20 sirens the most, one past siren20, one losing bones past 127" "sc_line '$SUMMARY' && [ \$(count 'vehicle models with sirens seen') -eq 1 ]"
check "a line for the model made for 32 sirens and for the one losing bones past 127, word for word, and for no other" "sc_line '$LINE_32' && sc_line '$LINE_LOST' && [ \$(count 'DIAGNOSTIC: vehicle model 0x') -eq 2 ]"
check "the call stub read+execute, 13 in all (the other parts' 12 and this one), every page its own protection" "has 'stubs: 13, all RX' && has 'protect: all restored (RX)'"
check "the sites but the redirected call and the models' data as placed" "has '$PLACED'"
check "no problem, no box" "log_lacks 'PROBLEM' && log_lacks 'Start-up summary'"

echo "== C2b DIAGNOSTIC_MODE=1 and nothing drawn for 11 s: the call redirected, no summary - its 10 seconds run from the first model counted"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "the call redirected and announced, its stub read+execute; no model counted, no summary; no problem" "sc_line 'DIAGNOSTIC: the siren lights. renderer call at RVA 0x[0-9A-F]* is redirected (I-E-B1): the vehicle models with sirens on, each counted once with its siren bones looked up by name up to siren32, are logged 10 seconds after the first, then every minute while they change.' && has 'siren draws: none, the call leads to a TweaksV stub |' && has 'stubs: 13, all RX' && log_lacks 'vehicle models with sirens seen' && log_lacks 'PROBLEM'"

echo "== C3 the test build, its summaries every 2 s: again when the models change - two more models 11 s in, then 3 s more"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=2000 ASI=TweaksV_test.asi game full_game.exe scdraw scmore); echo "      $out"
check "the test build's line for this measurement" "log_line '\[TweaksV\] TEST BUILD: the siren count summary comes every 2000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=2000).\$'"
check "the first summary, then a second with 6 models in 820 passes, 2 of them past siren20 - and no third, the models unchanged" "sc_line '$SUMMARY' && sc_line 'DIAGNOSTIC: 6 vehicle models with sirens seen in 820 passes of the siren lights. renderer - the most sirens a model.s bone structure names: 20 of 20 (model 0x79FBB0C5); 2 with siren bones past siren20, which the game never lights; 1 losing siren bones past bone 127, which a bone structure cannot name; 0 that could not be read.' && [ \$(count 'vehicle models with sirens seen') -eq 2 ]"
check "a line for the new model made for 32 sirens, none for the one with 16; the earlier two lines not written again" "sc_line 'DIAGNOSTIC: vehicle model 0xB2C3D4E5, 110 bones - siren and glass bones by name: 32 and 32, up to siren32 and siren_glass32; named by its bone structure: 20 and 20; past siren20, never lit: 12 and 12.' && [ \$(count 'vehicle model 0xA1B2C3D4,') -eq 1 ] && [ \$(count 'vehicle model 0x0C0FFEE0,') -eq 1 ] && [ \$(count 'DIAGNOSTIC: vehicle model 0x') -eq 3 ]"
check "820 passes through the stub with the renderer's arguments, result and last error; 64 lookups for each of the 6 models" "has 'siren draws: 820 through a TweaksV stub, the renderer 820 times, args ok, result and last error kept 820 of 820, 384 lookups in turn |' && log_lacks 'PROBLEM'"

echo "== C4 four more models: a bone structure that cannot be read, a vehicle drawn without a skeleton, names that disagree with the bone structure, a lookup that faults"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scodd scfault wait); echo "      $out"
check "1410 passes, the one without a skeleton handed on as it was; no lookup for the two whose data cannot be read, one for the one whose lookup faults" "has 'siren draws: 1410 through a TweaksV stub, the renderer 1410 times, args ok, result and last error kept 1410 of 1410, 321 lookups in turn |'"
check "the summary: 8 models, 3 that could not be read" "sc_line 'DIAGNOSTIC: 8 vehicle models with sirens seen in 1410 passes of the siren lights. renderer - the most sirens a model.s bone structure names: 20 of 20 (model 0x79FBB0C5); 1 with siren bones past siren20, which the game never lights; 1 losing siren bones past bone 127, which a bone structure cannot name; 3 that could not be read.'"
check "the two whose data could not be read, each with what could not" "sc_line 'DIAGNOSTIC: vehicle model 0x0BAD0001 could not be read: its bone structure (the model info.s +0xB0).' && sc_line 'DIAGNOSTIC: vehicle model 0x0BAD0002 could not be read: its skeleton data (the renderer.s 7th argument, its first field).'"
check "the lookup that faults: the access violation caught, the model one that could not be read and looked at no more, the game going on" "sc_line 'DIAGNOSTIC: vehicle model 0x0BAD0003 could not be read: find-bone-by-name faulted on its skeleton data (an access violation).' && has '$PLACED'"
check "the one whose names disagree with its bone structure: siren2 below bone 128 not named, a glass bone not found by name, siren1 found at another bone" "sc_line 'DIAGNOSTIC: vehicle model 0x0DD0DD00, 50 bones - siren and glass bones by name: 4 and 0, up to siren4 and no siren_glass; named by its bone structure: 3 and 1; found by name below bone 128 but not named by its bone structure: 1 and 0; named by its bone structure but not found by name: 0 and 1; found by name at another bone than its bone structure names: 1 and 0.' && [ \$(count 'DIAGNOSTIC: vehicle model 0x') -eq 6 ]"
check "no problem, no box" "log_lacks 'PROBLEM' && log_lacks 'Start-up summary'"

echo "== C5 1,100 more models after the four, one pass each: more than the table of 1,024 holds"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scfull wait); echo "      $out"
summary=$(tr -d '\r' < TweaksV.log | grep 'vehicle models with sirens seen')
seen=$(sed -n 's/.*DIAGNOSTIC: \([0-9]*\) vehicle models with sirens seen in 1900 passes .*/\1/p' <<< "$summary")
held=$(sed -n 's/.*; the table full, \([0-9]*\) passes of models it could not hold not counted\.$/\1/p' <<< "$summary")
check "1900 passes through the stub, the renderer's arguments, result and last error each time" "has 'siren draws: 1900 through a TweaksV stub, the renderer 1900 times, args ok, result and last error kept 1900 of 1900, '"
check "the summary: at most 1,024 models held, each of the 1,104 either held or its one pass counted as not held" "[ -n '$seen' ] && [ -n '$held' ] && [ '$seen' -le 1024 ] && [ \$(( $seen + $held )) -eq 1104 ]"
check "no problem" "log_lacks 'PROBLEM'"

echo "== C6 TEST_BREAK_PATTERN (test build): each of the four patterns missing - logged, never a problem, nothing redirected"
for label in I-E-B1 I-E-B2 I-E-B3 I-E-B4; do
	more=""; [ "$label" = I-E-B1 ] && more=wait	# once with 11 s more: no summary comes
	fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe scdraw $more)
	check "$label: the scanner's test line and the module's, word for word; drawn through the renderer itself, nothing counted, no stub of its own; no problem, no box, no summary" "grep -q 'TEST BUILD: pattern $label is treated as not found' TweaksV.log && sc_line '$NOT_MEASURED $label was not found.' && has '$UNCOUNTED' && has 'stubs: 12, all RX' && log_lacks 'PROBLEM' && log_lacks 'Start-up summary' && log_lacks 'vehicle models with sirens seen'"
done

echo "== C7 the call of the renderer another mod redirected already: left alone, logged"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw schooked)
check "the line, word for word; the other mod's call kept and nothing drawn through it here; no stub of its own; no problem" "sc_line '$NOT_MEASURED the siren lights. renderer call at RVA 0x[0-9A-F]* is not a call into the game (another mod may have hooked it already) (I-E-B1).' && has 'siren draws: none, the call leads to another mod |' && has 'stubs: 12, all RX' && log_lacks 'PROBLEM'"

echo "== C8 the call of the getter another mod redirected already: logged, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scgetterhooked)
check "the line, word for word; the renderer's call not redirected; no problem" "sc_line '$NOT_MEASURED the siren settings getter call at RVA 0x[0-9A-F]* is not a call into the game (another mod may have hooked it already) (I-E-B1).' && has '$UNCOUNTED' && has 'stubs: 12, all RX' && log_lacks 'PROBLEM'"

echo "== C9 the renderer's call leads to another function of the game than the one its bone read lies in: logged, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scelsewhere)
check "the line, word for word, naming the function the unwind data gives; the call as it was; no problem" "sc_line '$NOT_MEASURED I-E-B2 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: the game.s unwind data puts it in the function at RVA 0x[0-9A-F]*.' && has 'siren draws: none, the call leads to another function of the game |' && has 'stubs: 12, all RX' && log_lacks 'PROBLEM'"

echo "== C10 find-bone-by-name's pattern 16 bytes into its function: not a function start - logged, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scnotstart)
check "its site line and the module's line, word for word, by the unwind data; the call not redirected; no problem" "sc_line 'DIAGNOSTIC: find-bone-by-name at RVA 0x[0-9A-F]*, inside the function at RVA 0x[0-9A-F]* (unwind data), bone count offset at RVA 0x[0-9A-F]* holds 0x5E (I-E-B4)' && sc_line '$NOT_MEASURED find-bone-by-name at RVA 0x[0-9A-F]* is not a function start (inside the function at RVA 0x[0-9A-F]* (unwind data)) (I-E-B4).' && has '$UNCOUNTED' && log_lacks 'PROBLEM'"

echo "== C10b find-bone-by-name's pattern where no unwind data names a function, CC before it: not a start by the unwind data - logged, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scnounwind)
check "its site line and the module's line, word for word; the call not redirected; no problem" "sc_line 'DIAGNOSTIC: find-bone-by-name at RVA 0x[0-9A-F]*, no unwind data, bone count offset at RVA 0x[0-9A-F]* holds 0x5E (I-E-B4)' && sc_line '$NOT_MEASURED find-bone-by-name at RVA 0x[0-9A-F]* is not a function start (no unwind data) (I-E-B4).' && has '$UNCOUNTED' && has 'stubs: 12, all RX' && log_lacks 'PROBLEM'"

echo "== C11 none of the game code present (plain fake game): logged, not a problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game plain_game.exe)
check "the scanner's four misses and the module's line, word for word; none of it a problem" "[ \$(grep -c -E 'Pattern I-E-B[1-4] not found: ' TweaksV.log) -eq 4 ] && sc_line '$NOT_MEASURED I-E-B1 was not found.' && log_lacks '\[SirenSettings\] PROBLEM'"

echo "== C12 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the sites found and checked, the call not redirected, nothing counted"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe scdraw)
check "the four patterns found once each, the four site lines" "[ \$(grep -c -E 'Pattern I-E-B[1-4] found: .* 1 match,' TweaksV.log) -eq 4 ] && [ \$(count '$SITES') -eq 4 ]"
check "the would line and the module's line, word for word; nothing written, drawn through the renderer itself; no stub; no problem" "sc_line 'DRY_RUN: would redirect the siren lights. renderer call at RVA 0x[0-9A-F]*.' && sc_line '$NOT_MEASURED DRY_RUN - the siren lights. renderer call is not redirected, so nothing is counted.' && log_lacks 'renderer call at RVA 0x[0-9A-F]*: E8' && has '$UNCOUNTED' && has '$PLACED' && has 'stubs: 0,' && log_lacks 'PROBLEM'"

echo "== C13 Windows' dynamic code policy: no call stub can be made - logged under the part, not its problem, nothing redirected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game full_game.exe scdraw nodyncode)
check "the stubs' line and the module's line, word for word; the call as it was; no problem of the part's" "sc_line 'The call stubs could not be made read+execute (.*) - nothing is changed.' && sc_line '$NOT_MEASURED the siren lights. renderer call at RVA 0x[0-9A-F]* could not be redirected (details above).' && has '$UNCOUNTED' && log_lacks '\[SirenSettings\] PROBLEM'"


# ---- the raise: SIRENS_PER_VEHICLE above 20, the lights past 20 by a second call of the game's own renderer

# The call stubs the raise adds to the others': the renderer's call, both calls of the getter and the bone structures' maker's.
RAISE_STUBS=4
RAISED_SITES="siren raise sites: state size 0x16A8, getter call to a TweaksV stub, the vehicle lights' to a TweaksV stub, reader hooked |"
NOTHING_WRITTEN="siren raise sites: state size 0x188, getter call to the getter, the vehicle lights' to the getter, reader as laid |"
# What the loop of the fake's renderer found: 201 passes of the 14 models, 6 of them lit past 20 - 4 of the 12 own lights of the one whose setting has 28 items are default ones
LIGHTS_OK='stand-ins ok, blocks ok, own settings ok, lights right,'
LIT_32='lit police 20+0, 32 20+12, lost 8+0, 8 8+0, none 0+0, more32 20+12, 16 16+0, 21 20+1, holes 20+10, share 20+12, replaced 20+12, nosetting 0+0, past 0+0, spare 0+0, |'
RAISED='The sirens per vehicle are raised to 32.'
NOT_RAISED='PROBLEM: The sirens per vehicle are not raised this session:'
SETTINGS_FULL='PROBLEM: Some siren settings. lights past 20 are not kept this session: the table of 255 siren settings with lights past 20 is full, and the next ones keep 20 lights.'
raised_ini() { SIRENS_ON=1 fresh_ini; }
second_calls() { grep -qE 'siren lights: second calls [1-9]' <<< "$out"; }	# not the 0 that a run with no raise prints beside its other words

echo "== R1 SIRENS_PER_VEHICLE=32, as the default file has it: the settings' lights past 20 read, each vehicle made for them lit by a second call"
raised_ini; out=$(game full_game.exe srdraw); echo "      $out"
check "the state's size, the renderer's call and both calls of the getter written, the tree reader hooked; nothing else of the sites and data changed" "has \"$RAISED_SITES\" && has '$PLACED'"
check "the one line, word for word, no DIAGNOSTIC line of the part's, no problem" "sc_line '$RAISED' && log_lacks '\[SirenSettings\] DIAGNOSTIC' && log_lacks 'PROBLEM' && log_lacks 'Start-up summary'"
check "2814 passes through TweaksV's stub, the renderer 4020 times with what the game hands it and with the stand-ins; the first call's result and last error back; no lookup by name without DIAGNOSTIC_MODE" "has 'siren draws: 2814 through a TweaksV stub, the renderer 4020 times, args ok, result and last error kept 2814 of 2814, 0 lookups in turn |'"
check "the 6 models with bones past siren20 and an own setting called again, 201 times each: each stand-in the vehicle's fields, each block the room's bones, each own setting made by the constructor with the real one's scalars" "has 'siren lights: second calls 1206, $LIGHTS_OK'"
check "each second state made anew once, from memory nobody cleared; a setting's lights never torn; each light its file's item at the item's true index" "has 'second states made 6 clear, not torn, 804 default lights,'"
check "what each model lights by each call: no second call without bones past 20 or an own setting, and only the bones its room has; 21 items lights 1, 32 and 40 items light 12, the setting with 28 items 12, 4 of them default lights, the one with two bones missing 10" "has '$LIT_32'"
check "the parser's 12 readers back with the game's result and last error, an array of another member with 32 items left alone" "has 'parser 12 of 12 back,'"
check "both calls of the getter guarded (D41): position 0 none, each model's entry below the count, position 255 and one past the count the empty setting, read only, zero but for sequencerBpm - 201 passes of 2 models at 2 calls - and neither of those lit" "has 'getter calls right, 804 empty settings,'"
check "the stubs of the raise's three calls read+execute, the others' as ever, every page its own protection" "has \"stubs: \$((8 + RAISE_STUBS)), all RX\" && has 'protect: all restored (RX)'"

echo "== R2 with DIAGNOSTIC_MODE=1: every site logged, the writes, the settings read, and the summary saying which models are lit"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw wait); echo "      $out"
check "the raise's 7 patterns, the measurement's 4, the constructor, the registration and the getter each found exactly once" "[ \$(grep -c -E 'Pattern I-E-(Z[1-7]|B[1-4]|C1|P1|M1) found: .* 1 match,' TweaksV.log) -eq 14 ]"
check "the renderer's sites, word for word: the getter's call in the renderer, the vehicle's offsets each read by it" "sc_line 'DIAGNOSTIC: the renderer.s siren settings getter call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]*, in the function at RVA 0x[0-9A-F]* (unwind data) (I-E-Z1)' && sc_line 'DIAGNOSTIC: the vehicle.s broken sirens offset at RVA 0x[0-9A-F]* holds 0x860 (I-E-Z4)' && sc_line 'DIAGNOSTIC: the vehicle.s flag byte offset at RVA 0x[0-9A-F]* holds 0x986 (I-E-Z5)' && sc_line 'DIAGNOSTIC: the vehicle.s kind offset at RVA 0x[0-9A-F]* holds 0x6C, its matrix.s offset at RVA 0x[0-9A-F]* holds 0x80 (I-E-Z6)'"
check "the state's size, the tree reader, its walk, the constructor and the lights' member, word for word" "sc_line 'DIAGNOSTIC: the siren state.s size at RVA 0x[0-9A-F]* holds 0x188 (I-E-Z2)' && sc_line 'DIAGNOSTIC: the array member.s tree reader at RVA 0x[0-9A-F]*, a function start (unwind data) (I-E-Z3)' && sc_line 'DIAGNOSTIC: a node.s first child offset at RVA 0x[0-9A-F]* holds 0x38, its next sibling offset at RVA 0x[0-9A-F]* holds 0x30, in the function at RVA 0x[0-9A-F]* (unwind data) (I-E-Z7)' && sc_line 'DIAGNOSTIC: the siren setting.s constructor at RVA 0x[0-9A-F]*, no unwind data, byte before it 90 (I-E-C1)' && sc_line 'DIAGNOSTIC: the siren settings. lights member: entry 19 of their parser tables, at +0x3C, its data at RVA 0x[0-9A-F]*, offset field 0x0, type 0xD, subtype 0x1, element size 0x48, capacity 20, its element.s data at RVA 0x[0-9A-F]* (I-E-P1)'"
check "the writes, each once, with its old and new bytes: the state's size, the renderer's call, once, and both calls of the getter" "sc_line 'DIAGNOSTIC: siren state.s size at RVA 0x[0-9A-F]*: 88 01 00 00 -> A8 16 00 00' && [ \$(count 'DIAGNOSTIC: siren lights. renderer call at RVA 0x[0-9A-F]*: E8') -eq 1 ] && [ \$(count 'DIAGNOSTIC: renderer.s siren settings getter call at RVA 0x[0-9A-F]*: E8') -eq 1 ] && [ \$(count 'DIAGNOSTIC: siren settings getter call at RVA 0x[0-9A-F]*: E8') -eq 1 ]"
check "the settings read, word for word: each kept with its count, those past SIRENS_PER_VEHICLE dropped, the twin dropped, the one with others past the same 20 replacing" "[ \$(count 'a siren setting of 32 lights read (key 0x[0-9A-F]\{16\}): 12 past 20 kept in a setting of its own, [0-9]* of 255\.') -eq 2 ] && [ \$(count 'a siren setting of 40 lights read (key 0x[0-9A-F]\{16\}): 12 past 20 kept in a setting of its own, [0-9]* of 255, and 8 more than SIRENS_PER_VEHICLE gives dropped\.') -eq 2 ] && sc_line 'DIAGNOSTIC: a siren setting of 21 lights read (key 0x[0-9A-F]\{16\}): 1 past 20 kept in a setting of its own, 4 of 255\.' && sc_line 'DIAGNOSTIC: a siren setting of 32 lights read (key 0x[0-9A-F]\{16\}): the twin of one read before, dropped\.' && sc_line 'DIAGNOSTIC: a siren setting of 28 lights read (key 0x[0-9A-F]\{16\}): the same 20 lights as one read before and others past them, so it replaces that one\.'"
check "a setting of 20 items no line, the array of another member none, nor anything past the 8 in the table" "log_lacks 'a siren setting of 20 lights' && [ \$(count 'lights read (key') -eq 10 ]"
check "the wrapper the measurement's too, the call redirected once: the models counted, 64 lookups for each of the 13 that can be read, in turn; the announcement" "has 'siren draws: 2814 through a TweaksV stub, the renderer 4020 times, args ok, result and last error kept 2814 of 2814, 832 lookups in turn |' && sc_line 'DIAGNOSTIC: the siren lights. renderer call at RVA 0x[0-9A-F]* is redirected (I-E-B1): the vehicle models with sirens on, each counted once with its siren bones looked up by name up to siren32, are logged 10 seconds after the first, then every minute while they change.'"
check "the summary's first line, the words of a raise" "sc_line 'DIAGNOSTIC: 14 vehicle models with sirens seen in 2814 passes of the siren lights. renderer - the most sirens a model.s bone structure names: 20 of 20 (model 0x21212121); 9 with siren bones past siren20, 6 lit past siren20 by the second call; 1 losing siren bones past bone 127, which a bone structure cannot name; 1 that could not be read.'"
check "its second, what the tables hold: 8 settings, 1 twin, 1 replaced, 1 model sharing a setting's scalars, and the second calls" "sc_line 'DIAGNOSTIC: the sirens per vehicle at 32: 8 siren settings with lights past 20 in the table of 255, 1 twins dropped, 1 replaced by a later one with the same 20 lights, 0 that did not fit or could not be read; 1 models that changed the scalars of a setting another model shares; 1206 second calls of the renderer.'"
check "the line of a model lit past 20, word for word, the proof's; one for each of the 6" "sc_line 'DIAGNOSTIC: vehicle model 0xA1B2C3D4, 100 bones - siren and glass bones by name: 32 and 32, up to siren32 and siren_glass32; named by its bone structure: 20 and 20; past siren20: 12 and 12; lit past siren20 by the second call: 12 bones, the 12 lights past 20 of a setting of its own.' && sc_line 'DIAGNOSTIC: vehicle model 0x21212121, 70 bones - siren and glass bones by name: 21 and 0, up to siren21 and no siren_glass; named by its bone structure: 20 and 0; past siren20: 1 and 0; lit past siren20 by the second call: 1 bones, the 1 lights past 20 of a setting of its own.' && [ \$(count 'lit past siren20 by the second call: ') -eq 6 ]"
check "those not lit, each saying why: no setting, a position past the settings" "sc_line 'DIAGNOSTIC: vehicle model 0x80808080, 97 bones - siren and glass bones by name: 32 and 0, up to siren32 and no siren_glass; named by its bone structure: 20 and 0; past siren20: 12 and 0; not lit past siren20: it has no siren setting.' && sc_line 'DIAGNOSTIC: vehicle model 0x90909090, 98 bones - siren and glass bones by name: 32 and 0, up to siren32 and no siren_glass; named by its bone structure: 20 and 0; past siren20: 12 and 0; not lit past siren20: its siren setting.s position lies past the game.s settings.' && sc_line 'DIAGNOSTIC: vehicle model 0xA0A0A0A0, 99 bones - siren and glass bones by name: 32 and 0, up to siren32 and no siren_glass; named by its bone structure: 20 and 0; past siren20: 12 and 0; not lit past siren20: its siren setting.s position lies past the game.s settings.'"
check "the stubs, the measurement's own none: the others' and the raise's three; no problem" "has \"stubs: \$((12 + RAISE_STUBS)), all RX\" && has 'protect: all restored (RX)' && log_lacks 'PROBLEM'"
check "the lights as without DIAGNOSTIC_MODE" "has 'siren lights: second calls 1206, $LIGHTS_OK' && has '$LIT_32'"

echo "== R3 SIRENS_PER_VEHICLE=24: at most 4 lights past 20, the state and the stubs the same"
raised_ini; setkey SIRENS_PER_VEHICLE 24 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw)
check "raised to 24; 4 of each setting's lights past 20 kept, the rest dropped; each model's second call lights at most 4" "sc_line 'The sirens per vehicle are raised to 24.' && has \"$RAISED_SITES\" && [ \$(count 'a siren setting of 32 lights read (key 0x[0-9A-F]\{16\}): 4 past 20 kept in a setting of its own, [0-9]* of 255, and 8 more than SIRENS_PER_VEHICLE gives dropped\.') -eq 2 ] && has '$LIGHTS_OK' && has 'lit police 20+0, 32 20+4, lost 8+0, 8 8+0, none 0+0, more32 20+4, 16 16+0, 21 20+1, holes 20+4, share 20+4, replaced 20+4, nosetting 0+0, past 0+0, spare 0+0, |'"

echo "== R4 the key's limits: 99 is 32, 19 is 20"
raised_ini; setkey SIRENS_PER_VEHICLE 99 SIRENSETTINGS; out=$(game full_game.exe)
check "99 raised to 32, reported as other sizes are" "sc_line '$RAISED' && log_has 'PROBLEM: TweaksV.ini: \[SIRENSETTINGS\] SIRENS_PER_VEHICLE=99 is outside 20..32 - using 32\.'"
raised_ini; setkey SIRENS_PER_VEHICLE 19 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "19 the game's own 20, reported the same: nothing looked for, nothing written" "log_has 'PROBLEM: TweaksV.ini: \[SIRENSETTINGS\] SIRENS_PER_VEHICLE=19 is outside 20..32 - using 20\.' && sc_line 'SIRENS_PER_VEHICLE=20 - the sirens per vehicle are not raised.' && log_lacks 'Pattern I-E-Z' && has \"$NOTHING_WRITTEN\""

echo "== R5 a car's sirens 3 and 25 broken: the first call skips its light 3, the second, by the bits past 20, its light 5"
raised_ini; out=$(game full_game.exe srdraw srbroken)
check "19 and 11 lights of that car, the others' as ever, each light still its own item" "has 'lit police 20+0, 32 19+11, lost 8+0,' && has '$LIGHTS_OK' && log_lacks 'PROBLEM'"

echo "== R6 every car's first state made anew after the first pass: the second state follows"
raised_ini; out=$(game full_game.exe srdraw srreset)
check "each second state made twice, each time as the game's constructor leaves a state, its start time the first's" "has 'second states made 12 clear,' && has '$LIGHTS_OK' && has 'siren draws: 2828 through a TweaksV stub,'"

echo "== R7 a setting's lights past 20 replaced 200 times while its model is drawn: each published whole, no lock on the renderer's path"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw srrace)
check "never torn, each light of one generation, the scalars carried over; 201 replacing lines" "second_calls && has 'not torn,' && has '$LIGHTS_OK' && [ \$(count 'so it replaces that one\.') -eq 201 ] && log_lacks 'PROBLEM'"

raised_ini; out=$(game full_game.exe srdraw srnofirst)
check "no pass on one thread first: each model's first pass with both threads drawing it, each its own car; no block under way reaches the renderer" "second_calls && has '$LIGHTS_OK' && has '$LIT_32'"

echo "== R7b what a block and a file leave behind them"
raised_ini; out=$(game full_game.exe srdraw srreused)
check "a block the heap gives back from an earlier car, both start times 0: the second state made again for this vehicle, not the earlier car's" "has 'second states made 7 clear,' && has '$LIGHTS_OK' && has '$LIT_32'"
raised_ini; out=$(game full_game.exe srdraw srnostate)
check "a car with no state: no second call for it, no fault" "has 'siren lights: second calls 1005,' && has '$LIGHTS_OK'"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw srpadding)
check "other heap bytes in the lights' padding at each read, and a merge that copies the members alone: the key the same at the parser and in the array, the twin still a twin" "has '$LIT_32' && has '$LIGHTS_OK' && [ \$(count 'the twin of one read before, dropped\.') -eq 1 ] && log_lacks 'PROBLEM'"

echo "== R8 300 settings with lights past 20: more than the table of 255 holds"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srmany)
check "255 kept, 45 no room, each logged; the problem once, word for word" "[ \$(count 'past 20 kept in a setting of its own, [0-9]* of 255\.') -eq 255 ] && [ \$(count 'no room for it in the table\.') -eq 45 ] && sc_line '$SETTINGS_FULL' && [ \$(count 'Some siren settings. lights past 20') -eq 1 ]"

echo "== R9 20,001 twins: the lines of the settings read stop at 20,000, the numbers go on"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srflood)
check "the cap's line, word for word, once; 20,000 lines before it" "sc_line 'DIAGNOSTIC: 20000 lines of siren settings with lights past 20 written - from now on they are only counted; the summary every minute has the numbers.' && [ \$(count 'lights read (key') -eq 20000 ] && log_lacks 'PROBLEM'"

echo "== R10 an item that faults in the reader of an element: the setting keeps 20 lights, the next is read"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srfault)
check "the problem once, word for word; the next setting kept, the first of the table" "sc_line 'PROBLEM: Some siren settings. lights past 20 are not kept this session: reading the lights past 20 of one ended in an access violation, and such a setting keeps 20 lights.' && sc_line 'DIAGNOSTIC: a siren setting of 32 lights read (key 0x[0-9A-F]\{16\}): 12 past 20 kept in a setting of its own, 1 of 255\.' && [ \$(count 'lights read (key') -eq 1 ] && has 'parser 2 of 2 back,'"
raised_ini; out=$(game full_game.exe srbadnode)
check "a node that cannot be read: the problem once, word for word; the next setting read" "sc_line 'PROBLEM: Some siren settings. lights past 20 are not kept this session: a lights array.s node could not be read, and such a setting keeps 20 lights.' && [ \$(count 'Some siren settings. lights past 20') -eq 1 ] && has 'parser 1 of 1 back,'"
raised_ini; out=$(game full_game.exe srbadobject)
check "an object that cannot be read: the problem once, word for word; the next setting read" "sc_line 'PROBLEM: Some siren settings. lights past 20 are not kept this session: the siren setting a lights array is read into could not be read, and such a setting keeps 20 lights.' && [ \$(count 'Some siren settings. lights past 20') -eq 1 ] && has 'parser 1 of 1 back,'"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=6 ASI=TweaksV_test.asi game full_game.exe srdraw wait)
check "no memory for the settings kept (the test build's switch): the problem once, word for word; no second call; each setting that could not be kept counted in the summary" "sc_line 'PROBLEM: The lights past 20 of the siren settings are not kept this session: the memory for them could not be had (Windows error 8), and the vehicles keep 20 sirens.' && has 'siren lights: second calls 0,' && log_has '0 siren settings with lights past 20 in the table of 255, 0 twins dropped, 0 replaced by a later one with the same 20 lights, 10 that did not fit or could not be read;'"
raised_ini; out=$(game full_game.exe srelement)
check "an element's member object with another member data: nothing read, the problem once" "sc_line 'PROBLEM: Some siren settings. lights past 20 are not kept this session: a lights array.s element is not the one its member data names, and such a setting keeps 20 lights.' && [ \$(count 'Some siren settings. lights past 20') -eq 1 ] && has 'parser 2 of 2 back,'"

echo "== R11 the models past the table of 1,024, with the raise: they keep 20 sirens, reported once"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scfull wait)
check "the problem once, word for word; the summary's numbers as the measurement's" "sc_line 'PROBLEM: Some vehicle models. lights past 20 are not lit this session: the table of 1024 vehicle models is full, and the next ones keep 20 sirens.' && [ \$(count 'Some vehicle models. lights past 20') -eq 1 ] && log_has '; the table full, [0-9]* passes of models it could not hold not counted' && has 'siren draws: 1900 through a TweaksV stub, the renderer 1900 times, args ok, result and last error kept 1900 of 1900, '"

echo "== R12 the models drawn with the raise and no setting read: none lit, each saying why"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw scodd scfault wait)
check "no second call; the model made for 32 sirens: not lit past siren20, its reason; the others as the measurement has them" "sc_line 'DIAGNOSTIC: vehicle model 0xA1B2C3D4, 100 bones - siren and glass bones by name: 32 and 32, up to siren32 and siren_glass32; named by its bone structure: 20 and 20; past siren20: 12 and 12; not lit past siren20: its siren setting has no lights past 20.' && has 'siren draws: 1410 through a TweaksV stub, the renderer 1410 times, args ok, result and last error kept 1410 of 1410, 321 lookups in turn |' && sc_line 'DIAGNOSTIC: vehicle model 0x0BAD0001 could not be read: its bone structure (the model info.s +0xB0).' && log_lacks 'PROBLEM'"

echo "== R13 the raise stands without the measurement: a measurement's pattern missing is the measurement's alone"
raised_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN I-E-B2; out=$(ASI=TweaksV_test.asi game full_game.exe srdraw)
check "not measured, no lookup, no problem; the lights as ever" "sc_line '$NOT_MEASURED I-E-B2 was not found.' && sc_line '$RAISED' && has '$LIT_32' && has ', 0 lookups in turn |' && log_lacks 'PROBLEM'"

echo "== R14 the IDs' job: its writes change what I-E-C1 matches, and the accessor keeps it; with the IDs off the accessor looks once, and the IDs' measurement after it not again"
raised_ini; out=$(game full_game.exe srdraw)
check "LARGE_SIREN_IDS=1: the constructor with the IDs' write in it, every own setting's ID 0xFF" "sc_line 'The siren setting IDs. limit is raised to 65535: .*' && has 'own settings ok,' && has '$LIT_32'"
raised_ini; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw)
check "LARGE_SIREN_IDS=0: raised all the same, I-E-C1 and I-E-P1 each looked for once" "sc_line '$RAISED' && has 'own settings ok,' && has '$LIT_32' && [ \$(grep -c 'Pattern I-E-C1 found' TweaksV.log) -eq 1 ] && [ \$(grep -c 'Pattern I-E-P1 found' TweaksV.log) -eq 1 ]"

echo "== R15 DRY_RUN=1: every site found and checked, the would lines, nothing written"
raised_ini; setkey DRY_RUN 1; out=$(game full_game.exe srdraw)
check "the hook's, the two calls' and the size's would lines, word for word, each once; no line that it is raised" "sc_line 'DRY_RUN: would hook the array member.s tree reader at RVA 0x[0-9A-F]*\.' && sc_line 'DRY_RUN: would redirect the siren lights. renderer call at RVA 0x[0-9A-F]*\.' && sc_line 'DRY_RUN: would redirect the renderer.s siren settings getter call at RVA 0x[0-9A-F]*\.' && sc_line 'DRY_RUN: would redirect the siren settings getter call at RVA 0x[0-9A-F]*\.' && sc_line 'DRY_RUN: would patch the siren state.s size: 88 01 00 00 -> A8 16 00 00' && log_lacks '$RAISED' && log_lacks 'PROBLEM'"
check "nothing written, drawn through the game's own: no second call, no stub, the sites as placed; both calls of the getter its own, its two models past the array not drawn" "has \"$NOTHING_WRITTEN\" && has '$PLACED' && has 'siren lights: second calls 0,' && has 'lit police 20+0, 32 20+0,' && has 'getter calls right, 0 empty settings,' && has 'siren draws: 2412 through the renderer,' && has 'stubs: 0,'"
raised_ini; setkey DRY_RUN 1; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "with DIAGNOSTIC_MODE: the call's would line once, the measurement's own words, nothing counted" "[ \$(count 'DRY_RUN: would redirect the siren lights. renderer call') -eq 1 ] && sc_line '$NOT_MEASURED DRY_RUN - the siren lights. renderer call is not redirected, so nothing is counted.' && log_lacks '$RAISED' && has \"$NOTHING_WRITTEN\""

echo "== R16 the game's build (D40) and the light stores (D39), each the part's problem; the part off"
raised_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game_1013.exe)
check "another build: the problem word for word, no pattern of the raise's looked for, nothing written" "sc_line '$NOT_RAISED this game build is 1.0.1013.17, and TweaksV has read the siren lights. renderer of 1.0.1158.16 only. Vehicles keep 20 sirens until a new TweaksV.' && log_lacks 'Pattern I-E-Z' && has \"$NOTHING_WRITTEN\""
raised_ini; out=$(game full_game_nover.exe)
check "no version resource: the same problem" "sc_line '$NOT_RAISED this game build is not known, and TweaksV has read the siren lights. renderer of 1.0.1158.16 only. Vehicles keep 20 sirens until a new TweaksV.' && has \"$NOTHING_WRITTEN\""
raised_ini; setkey ENABLED 0 LIGHTCAPACITY; out=$(game full_game.exe srdraw)
check "the light stores off: the problem word for word, nothing written, the vehicles at 20" "sc_line '$NOT_RAISED the light stores are not raised (\[LIGHTCAPACITY\] in TweaksV.ini), and the game.s own do not take 32 sirens a vehicle. Vehicles keep 20 sirens.' && has \"$NOTHING_WRITTEN\" && has 'siren lights: second calls 0,' && has 'lit police 20+0, 32 20+0,'"
raised_ini; setkey ENABLED 0 LIGHTCAPACITY; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe scdraw wait)
check "and with DIAGNOSTIC_MODE the measurement goes on as without the raise, its call redirected, its words" "sc_line '$SUMMARY' && sc_line '$LINE_32' && has '$COUNTED'"
raised_ini; setkey ENABLED 0 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe srdraw)
check "[SIRENSETTINGS] off: no line of the part, none of its raise's patterns looked for, nothing written" "log_lacks 'Pattern I-E-Z' && log_lacks '$NOT_RAISED' && has \"$NOTHING_WRITTEN\""

echo "== R17 TEST_BREAK_PATTERN (test build): each of the raise's patterns missing - the part's problem, nothing written"
for label in I-E-B1 I-E-Z1 I-E-Z2 I-E-Z3 I-E-Z4 I-E-Z5 I-E-Z6 I-E-Z7 I-E-C1 I-E-P1 I-E-M1; do
	raised_ini; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the problem, word for word; nothing written" "sc_line '$NOT_RAISED $label was not found.' && has \"$NOTHING_WRITTEN\" && has '$PLACED'"
done

echo "== R18 the plain game: the light stores find nothing, so the part's problem is theirs"
raised_ini; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game plain_game.exe)
check "the problem, word for word" "sc_line '$NOT_RAISED the light stores are not raised (.LIGHTCAPACITY. in TweaksV.ini), and the game.s own do not take 32 sirens a vehicle. Vehicles keep 20 sirens.'"

# refused WORDS [OPTION...]: full_game with the raise on, its problem word for word, nothing written of the raise's
refused() {
	local words=$1; shift
	raised_ini; out=$(game full_game.exe "$@")
	check "$* : the problem, word for word; nothing written" "sc_line '$NOT_RAISED $words' && has 'state size 0x188,' && has 'reader as laid |' && has '$PLACED' && has 'stubs: 8,' && [ \$(count '$NOT_RAISED') -eq 1 ] && log_lacks '$RAISED'"
}
echo "== R19 each site not as the findings have it, on its own: the part's problem, nothing written"
ANOTHER_MOD='(another mod may have hooked it already)'
NO_FUNCTION='the game.s unwind data names no function there\.'
refused "the siren lights. renderer call at RVA 0x[0-9A-F]* is not a call into the game $ANOTHER_MOD (I-E-B1)." schooked
refused "the siren settings getter call at RVA 0x[0-9A-F]* is not a call into the game $ANOTHER_MOD (I-E-B1)." scgetterhooked
refused "I-E-Z1 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: the game.s unwind data puts it in the function at RVA 0x[0-9A-F]*\." scelsewhere
refused "I-E-Z1 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: $NO_FUNCTION" srz1out
refused "I-E-Z4 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: $NO_FUNCTION" srz4out
refused "I-E-Z5 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: $NO_FUNCTION" srz5out
refused "I-E-Z6 at RVA 0x[0-9A-F]* does not lie in the siren lights. renderer at RVA 0x[0-9A-F]*: $NO_FUNCTION" srz6out
refused "the siren settings getter call at RVA 0x[0-9A-F]* leads to RVA 0x[0-9A-F]*, not to the getter at RVA 0x[0-9A-F]* $ANOTHER_MOD (I-E-M1)." srgetterelsewhere
refused "the renderer.s siren settings getter call at RVA 0x[0-9A-F]* leads to RVA 0x[0-9A-F]*, not to the getter at RVA 0x[0-9A-F]* $ANOTHER_MOD (I-E-Z1)." srz1other
refused "the renderer.s siren settings getter call at RVA 0x[0-9A-F]* leads to 0x[0-9A-F]* (outside the game), not to the getter at RVA 0x[0-9A-F]* $ANOTHER_MOD (I-E-Z1)." srz1hooked
refused "the array member.s tree reader at RVA 0x[0-9A-F]* is not a function start (inside the function at RVA 0x[0-9A-F]* (unwind data)) (I-E-Z3)." srz3notstart
refused "the array member.s tree reader at RVA 0x[0-9A-F]* is not a function start (no unwind data) (I-E-Z3)." srz3nounwind
refused "I-E-Z7 at RVA 0x[0-9A-F]* does not lie in the array member.s tree reader at RVA 0x[0-9A-F]*: $NO_FUNCTION" srz7out
refused "the siren setting.s constructor at RVA 0x[0-9A-F]* is not a function start (no unwind data, byte before it 41) (I-E-C1)." src1notstart
refused "the siren settings. lights member lies at +0x40, not +0x3C (I-E-P1)." sroffset
refused "the siren settings. lights member.s data at 0x[0-9A-F]* (outside the game) do not lie in the game.s .data, or cannot be read (I-E-P1)." srdataout
refused "the siren settings. lights member.s data at RVA 0x[0-9A-F]* do not lie in the game.s .data, or cannot be read (I-E-P1)." srdatardata
refused "the siren settings. lights member.s offset field at RVA 0x[0-9A-F]* holds 0x8, not +0x3C (I-E-P1)." sroffsetfield
refused "the siren settings. lights member.s type at RVA 0x[0-9A-F]* is 0xE and 0x1, not 0xD and 0x1, a fixed array (I-E-P1)." srtype
refused "the siren settings. lights member.s type at RVA 0x[0-9A-F]* is 0xD and 0x2, not 0xD and 0x1, a fixed array (I-E-P1)." srsubtype
refused "the siren settings. lights member holds 20 elements of 0x50 bytes, not 20 of 0x48 (another mod may have raised them already) (I-E-P1)." srsize
refused "the siren settings. lights member holds 32 elements of 0x48 bytes, not 20 of 0x48 (another mod may have raised them already) (I-E-P1)." srcapacity
refused "the siren settings. lights member.s element.s data at RVA 0x[0-9A-F]* are not a structure.s in the game.s .data (I-E-P1)." srelementtype
raised_ini; out=$(game full_game.exe srdraw srnoelement)
check "srnoelement: its element's data not there yet at the start, as the offset is not: raised, the hook asks it again" "sc_line '$RAISED' && has '$LIT_32' && has '$LIGHTS_OK' && log_lacks 'PROBLEM'"
raised_ini; out=$(game full_game.exe srdraw srlateoffset)
check "srlateoffset: the lights' offset read from the member data at the read, not from the table at the start: a setting's key the same at the parser's offset and in the game's array" "sc_line '$RAISED' && has '$LIT_32' && has '$LIGHTS_OK'"
raised_ini; out=$(game full_game.exe srdraw srnocarcols)
check "srnocarcols: no carcols data: every position the empty setting, no second call, no block, no read of a null pointer" "sc_line '$RAISED' && has 'siren lights: second calls 0,' && has 'getter calls right,' && has 'lit police 0+0, 32 0+0,' && has 'nosetting 0+0, past 0+0, spare 0+0, |'"

echo "== R20 the one write refused: by Windows' dynamic code policy at the stores, and by the test build's switch at the hook and at the writes, the hook taken back"
raised_ini; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game full_game.exe srdraw nodyncode)
check "Windows' dynamic code policy stops the stores first: the problem, word for word; nothing written, the vehicles at 20" "sc_line '$NOT_RAISED the light stores are not raised (.LIGHTCAPACITY. in TweaksV.ini), and the game.s own do not take 32 sirens a vehicle. Vehicles keep 20 sirens.' && has \"$NOTHING_WRITTEN\" && has 'siren lights: second calls 0,' && has '$PLACED'"
raised_ini; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=5 ASI=TweaksV_test.asi game full_game.exe srdraw)
check "no hook can be made: the test line, the problem, word for word; nothing written, the vehicles at 20" "sc_line 'TEST BUILD: the sirens. one write has its hook refused (TWEAKSV_TEST_SIREN_WRITE_FAILS=5).' && sc_line '$NOT_RAISED the functions could not be hooked (details above). Nothing is changed.' && has \"$NOTHING_WRITTEN\" && has 'siren lights: second calls 0,' && has '$PLACED'"
raised_ini; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=1 ASI=TweaksV_test.asi game full_game.exe srdraw)
check "the writes refused once the hook is on: the test line, the problem, the hook taken back, nothing written, the vehicles at 20" "sc_line 'TEST BUILD: the sirens. one write is refused (TWEAKSV_TEST_SIREN_WRITE_FAILS=1).' && sc_line '$NOT_RAISED its writes could not be made (details above). Nothing is changed.' && has \"$NOTHING_WRITTEN\" && has 'siren lights: second calls 0,' && has '$PLACED' && log_lacks '$RAISED'"
raised_ini; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=2 ASI=TweaksV_test.asi game full_game.exe srdraw)
check "an exception in place of the writes, the hook on, which the catch-all would call a part skipped: the problem, the hook taken back, nothing written, the vehicles at 20" "sc_line 'TEST BUILD: the sirens. one write throws in its place (TWEAKSV_TEST_SIREN_WRITE_FAILS=2).' && sc_line '$NOT_RAISED its writes ended in an unexpected exception. Nothing is changed.' && has \"$NOTHING_WRITTEN\" && has 'siren lights: second calls 0,' && has '$PLACED' && log_lacks 'this part is skipped'"
raised_ini; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=3 ASI=TweaksV_test.asi game full_game.exe srdraw)
check "a hook that cannot be switched off: the patch module's line, and the problem says that it stays, not that nothing is changed" "log_has 'could not be switched off (MinHook: ' && sc_line '$NOT_RAISED its writes could not be made (details above). Its hook stays on and passes the calls on (details above); nothing else is changed.' && has 'state size 0x188,' && has 'siren lights: second calls 0,'"
raised_ini; out=$(TWEAKSV_TEST_SIREN_WRITE_FAILS=4 ASI=TweaksV_test.asi game full_game.exe srdraw)
check "an exception once the writes are in, as a line after them can throw: they stand with their hook, the raise with them, no problem" "sc_line '$RAISED' && has \"$RAISED_SITES\" && has '$LIT_32' && has '$LIGHTS_OK' && log_lacks 'PROBLEM'"

echo "== R21 TEST_SIREN_PASSES (test build): so many more of both calls in each pass, the load my proof of the stores needs; no effect at 0, none in the Release build"
PASSES_LINE() { echo "TEST BUILD: each pass of the siren lights. renderer runs $1 more times, the second call with it (TEST_SIREN_PASSES=$1, up to 31)\."; }
raised_ini; addkey TEST_SIREN_PASSES 3; out=$(ASI=TweaksV_test.asi game full_game.exe srdraw); echo "      $out"
check "3 more of each: the renderer 1 + 3 times, the second call likewise, with the same arguments; the first call's result and last error back; each model lit as ever" "has 'siren draws: 2814 through a TweaksV stub, the renderer 16080 times, args ok, result and last error kept 2814 of 2814,' && has 'siren lights: second calls 4824, $LIGHTS_OK' && has '$LIT_32'"
check "its one line, word for word, and the header's; no problem" "sc_line \"\$(PASSES_LINE 3)\" && [ \$(count 'TEST_SIREN_PASSES=3, up to 31)') -eq 1 ] && log_has 'TEST BUILD - not for release\..* TEST_SIREN_PASSES=3\$' && log_lacks 'PROBLEM'"
raised_ini; addkey TEST_SIREN_PASSES 31; out=$(ASI=TweaksV_test.asi game full_game.exe srdraw)
check "its top, 31: the renderer 32 times, the second call likewise" "has 'the renderer 128640 times, args ok, result and last error kept 2814 of 2814,' && has 'siren lights: second calls 38592, $LIGHTS_OK' && sc_line \"\$(PASSES_LINE 31)\" && log_lacks 'PROBLEM'"
raised_ini; addkey TEST_SIREN_PASSES 0; out=$(ASI=TweaksV_test.asi game full_game.exe srdraw)
check "0: as without it, no line" "has 'the renderer 4020 times, args ok,' && has 'siren lights: second calls 1206, $LIGHTS_OK' && [ \$(count 'TEST BUILD: each pass') -eq 0 ] && log_lacks 'PROBLEM'"
for value in 32 x; do
	raised_ini; addkey TEST_SIREN_PASSES $value; out=$(ASI=TweaksV_test.asi game full_game.exe srdraw)
	check "$value is no number from 0 to 31: the problem word for word, as without it, the raise standing" "sc_line \"PROBLEM: TweaksV.ini: \\\\[TWEAKSV\\\\] TEST_SIREN_PASSES=$value is not a number from 0 to 31, so it is ignored\\\\.\" && has 'the renderer 4020 times, args ok,' && sc_line '$RAISED'"
done
raised_ini; addkey TEST_SIREN_PASSES 3; out=$(game full_game.exe srdraw)
check "the Release build asks for no such key: as without it, no TEST BUILD line, the header without the key" "has 'the renderer 4020 times, args ok,' && has 'siren lights: second calls 1206, $LIGHTS_OK' && log_lacks 'TEST BUILD' && log_lacks 'TEST_SIREN_PASSES' && log_lacks 'PROBLEM'"

echo
echo "SIREN COUNT SCENARIOS: $pass passed, $fail failed"
