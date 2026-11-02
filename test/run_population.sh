#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The two model budgets, a limit measured first (TweaksVPopulation): with DIAGNOSTIC_MODE=1 the fake game's 16 sites found, checked and
# logged, nothing written, and both budgets sampled and summed up 10 seconds in and again when a high or the samples at the stop move -
# the numbers of my boot at 180 s, word for word; before the game's init; a level a script lowered, one above the game's, a
# network game; a table a tunable changed, a raise in gameconfig.xml, a variety past the slider, a budget past an int; models, a pool
# and a configuration that cannot be read; the object gone for a while; each pattern missing, each site not as the findings have it, a
# thread that cannot start - each logged, never a problem, nothing more measured - and a writer besides the set-up's, or a scan that
# misses one of those, a line alone; a model released while its list is read; how long no high has moved, in each line's last words. The
# runs go side by side, each in a folder of its own, each stopping as soon as its last line is in the log. Fake LOCALAPPDATA: the real
# one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=156
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

A='RVA 0x[0-9A-F]*'
OUT='0x[0-9A-F]* (outside the game)'
LABELS='O-E-C1 O-E-V1 O-E-V2 O-E-V3 O-E-V4 O-E-V5 O-E-V6 O-E-V7 O-E-P1 O-E-P2 O-E-P3 O-E-P4 O-E-P5 O-E-P6 O-E-P7 O-E-P8'
P_LABELS='O-E-P5, O-E-P6, O-E-P7'
# The module's lines, word for word (an apostrophe as any character)
pop_line() { log_line "\[Population\] $1\$"; }
NOT_MEASURED='DIAGNOSTIC: the model budgets are not measured:'
ANNOUNCED="DIAGNOSTIC: the model budgets, in the population object at $A (O-E-C1): sampled every second, nothing written; their tables are logged once the game has filled them and whenever they change, each budget.s line after 10 seconds, then every minute while its highs or its samples at the stop change."
NOT_FILLED='DIAGNOSTIC: the model budgets are not filled yet - the game has not started its population.'
tables_line() {	# tables_line vehicle|ped [STORY_TOP [NETWORK_TOP]]: a budget's tables' line, with those top entries, up to the configuration's three values
	if [ "$1" = vehicle ]; then
		echo "DIAGNOSTIC: vehicle model budget.s tables: story 1500000, 33500000, 57500000 and ${2:-71500000} bytes, network 1500000, 33500000, 57500000 and ${3:-71500000} bytes; the configuration.s story Level1Kb to Level3Kb: "
	else
		echo "DIAGNOSTIC: ped model budget.s tables: story 1500000, 55500000, 73500000 and ${2:-93500000} bytes, network 1500000, 55500000, 73500000 and ${3:-93500000} bytes; the configuration.s story Level1Kb to Level3Kb: "
	fi
}
VEHICLE_TABLES="$(tables_line vehicle)32000, 56000 and 70000, the game.s defaults - the story entries are theirs."
PED_TABLES="$(tables_line ped)54000, 72000 and 92000, the game.s defaults - the story entries are theirs."
# Each budget's line at the boot's 180 s
NO_HIGH='; no new high for [0-9]* s.'
VEHICLE_AT_180="DIAGNOSTIC: vehicle model budget: 428.7 of 429.0 MB in use at level 3 (the level.s 71.5 MB x variety 6.00), the requests stop at 427.5 MB, over the budget by 0.0 MB, at most 428.7 MB so far; vehicle models loaded: 94 for traffic, 0 outside the pop cycle, 0 trains and helicopters, 0 boats, 8 dropped, 0 in flight, at most 102 at once, about 4.1 MB each; at the stop in [0-9]* of [0-9]* samples; VehicleStruct 102 of 200 slots, at most 102 so far, unused models released past 180; of the bytes: weapons 10.1 MB, mods 0.0 and 0.0 MB; ambient cars on the map 57, at most 57 so far$NO_HIGH"
PED_AT_180="DIAGNOSTIC: ped model budget: 305.1 of 561.0 MB in use at level 3 (the level.s 93.5 MB x variety 6.00), the requests stop at 553.5 MB, over the budget by 0.0 MB, at most 305.1 MB so far; ped models loaded: 19 for the population, 1 of them streamed, 5 outside the schedule, 4 in no group, 3 asked for directly, at most 31 at once, about 9.8 MB each; at the stop in 0 of [0-9]* samples; at most 9 peds of one model; at most 1 streamed at once$NO_HIGH"
VEHICLE_MODELS='vehicle models loaded: 94 for traffic, 0 outside the pop cycle, 0 trains and helicopters, 0 boats, 8 dropped, 0 in flight, at most 102 at once, about 4.1 MB each'
PED_MODELS='ped models loaded: 19 for the population, 1 of them streamed, 5 outside the schedule, 4 in no group, 3 asked for directly, at most 31 at once'
no_problem() { log_lacks '\[Population\] PROBLEM' && has 'pop sites: as placed | pop data: as placed |' && has 'protect: all restored (RX)'; }

# Runs go side by side, each in a folder of its own, their checks once all have ended. in_folder NAME: that run's folder, also the
# game folder check goes by. side_by_side NAME INI_LINES OPTION...: one run in the background, with the test build, its INI_LINES
# evaluated in its folder, $ENVS its environment, half as many at a time as the suites. all_ended waits for them. after_run NAME: that
# folder again, and the run's report in $out.
game_folder=$B
in_folder() { B=$game_folder/side/$1; mkdir -p "$B" && cd "$B" || exit 1; }
most=$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))
running=0
side_by_side() {
	local name=$1 lines=$2; shift 2
	(
		in_folder "$name"
		cp "$game_folder/TweaksV_test.asi" .
		fresh_ini; setkey DIAGNOSTIC_MODE 1; eval "$lines"
		eval "${ENVS:-}" ASI=TweaksV_test.asi game "../../full_game.exe" "$@" > /dev/null
	) &
	running=$((running + 1))
	if [ "$running" -ge "$most" ]; then wait -n; running=$((running - 1)); fi
}
all_ended() { wait; running=0; }
after_run() { in_folder "$1"; out=$(tr -d '\r' < harness.out); }

echo "== O1 DIAGNOSTIC_MODE=0: nothing looked for, nothing logged, no thread tried (the test build refuses the summary's), however the data stand"
fresh_ini; out=$(TWEAKSV_TEST_REFUSE_THREAD="1 population summary" ASI=TweaksV_test.asi game full_game.exe popfill)
check "no O pattern looked for, no line of the part, its thread not tried; the sites and data as placed, no call stub of its own, every page its own protection" "log_lacks 'O-E-' && log_lacks '\[Population\]' && log_lacks 'The population summary thread could not be started' && no_problem && has 'stubs: 7, all RX'"

echo "== O2 DIAGNOSTIC_MODE=1, the data as my boot at 180 s: the sites found and logged, the summary 10 s in, nothing written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe popfill popwait); echo "      $out"
check "the 16 patterns each found exactly once" "[ \$(grep -c -E 'Pattern O-E-[CVP][0-9] found: .* 1 match,' TweaksV.log) -eq 16 ]"
check "the crash context's line and its two getters, word for word" "pop_line 'DIAGNOSTIC: crash context.s line at $A: its text at $A, the population object at $A, the memories it prints at $A and $A (O-E-C1)' && pop_line 'DIAGNOSTIC: ped budget.s getter at $A: level at $A, story mode.s table at $A, network.s at $A, variety at $A, network flag at $A (O-E-C1)' && pop_line 'DIAGNOSTIC: vehicle budget.s getter at $A: level at $A, story mode.s table at $A, network.s at $A, variety at $A, network flag at $A (O-E-C1)'"
check "the vehicles' check, VehicleStruct and the ambient cars, word for word" "pop_line 'DIAGNOSTIC: vehicle budget.s check at $A (O-E-V1) and $A: in use at +0x6214 of the object, the margin 1500000, the request at $A (O-E-V2)' && pop_line 'DIAGNOSTIC: VehicleStruct pool.s pointer at $A: its used at +0x28, its size at +0x18, unused models released from 20 below it (O-E-V3)' && pop_line 'DIAGNOSTIC: ambient cars on the map at $A (O-E-V5)'"
check "the set-up: its one call, a function start, its holder and the tunables, word for word" "pop_line 'DIAGNOSTIC: tables. set-up call at $A -> $A (no unwind data, byte before it 90), its only call; the configuration.s holder at $A, the tunables at $A (O-E-V4, O-E-V6)'"
check "the two story tables' stores (W2) and their writers (W3), none besides them, word for word" "pop_line 'DIAGNOSTIC: vehicle story entries. stores at $A, $A and $A -> $A, $A and $A (O-E-V7)' && pop_line 'DIAGNOSTIC: ped story entries. stores at $A, $A and $A -> $A, $A and $A ($P_LABELS)' && pop_line 'DIAGNOSTIC: writers of the vehicle story table at $A: the set-up.s stores at $A, $A and $A, none besides them (O-E-V7)' && pop_line 'DIAGNOSTIC: writers of the ped story table at $A: the set-up.s stores at $A, $A and $A, none besides them ($P_LABELS)'"
check "the ped budget's sum, gate, lists and a model's peds, word for word" "pop_line 'DIAGNOSTIC: ped memory.s sum at $A -> $A, the gate reading its +0x620C alone (O-E-P1)' && pop_line 'DIAGNOSTIC: ped budget.s gate at $A: in use at +0x620C of the object, the reserve from +0x6208 or +0x6204, the request at $A (O-E-P2)' && pop_line 'DIAGNOSTIC: ped lists. read at $A: the lists at +0x4E04, +0x5204, +0x5604 and +0x5E04 of the object, the model count at $A (O-E-P3)' && pop_line 'DIAGNOSTIC: a model.s peds at $A: the model info table at $A, its stride at $A, the peds at +0x158 (O-E-P4)'"
check "the variety's store, its numbers read from the game's constants, word for word" "pop_line 'DIAGNOSTIC: ped variety.s store at $A -> $A: the setting x 5.00 + 1.00, at most 10.00 (O-E-P8)'"
check "the announcement, word for word" "pop_line '$ANNOUNCED'"
object=$(sed -n 's/.*pop rvas: object \(0x[0-9A-F]*\),.*/\1/p' <<< "$out"); vehicle=$(sed -n 's/.*vehicle story \(0x[0-9A-F]*\),.*/\1/p' <<< "$out"); ped=$(sed -n 's/.*ped story \(0x[0-9A-F]*\),.*/\1/p' <<< "$out"); setup=$(sed -n 's/.*set-up \(0x[0-9A-F]*\) |.*/\1/p' <<< "$out")
check "the object, both tables and the set-up at the RVAs the fake placed them" "[ -n '$object' ] && log_has '\[Population\] .*the population object at RVA $object,' && log_has '\[Population\] .*writers of the vehicle story table at RVA $vehicle:' && log_has '\[Population\] .*writers of the ped story table at RVA $ped:' && log_has '\[Population\] .*set-up call at $A -> RVA $setup '"
check "both tables' lines, word for word, each once: the game's defaults" "pop_line '$VEHICLE_TABLES' && pop_line '$PED_TABLES' && [ \$(count 'model budget.s tables:') -eq 2 ]"
check "the vehicles' line and the peds', word for word: 428.7 of 429, past the stop at 427.5; 305.1 of 561, the stop at 553.5 with MaxScenPeds 8" "pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180'"
samples=$(tr -d '\r' < TweaksV.log | sed -n 's/.*vehicle model budget: .*at the stop in \([0-9]*\) of \([0-9]*\) samples.*/\1 \2/p' | head -n 1)
check "the vehicles at the stop in every sample, about one a second over the first 10 s: $samples" "[ \"\${samples% *}\" = \"\${samples#* }\" ] && [ \"\${samples% *}\" -ge 5 ] && [ \"\${samples% *}\" -le 12 ]"
check "no problem, no box, nothing written, no call stub of its own (the 12 are the other parts'), every page its own protection" "no_problem && log_lacks 'Start-up summary' && has 'stubs: 12, all RX'"

echo "== O3 DRY_RUN=1 changes nothing here"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe popfill popwait)
check "the same lines, the same summary, no DRY_RUN line of the part, nothing written" "pop_line '$ANNOUNCED' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180' && log_lacks '\[Population\] DRY_RUN' && no_problem"

echo "== O4 the plain fake game: no pattern there - logged, not a problem"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "each pattern named by the scanner, the first by the module; nothing more, no problem" "[ \$(count 'Pattern O-E-[CVP][0-9] not found:') -eq 16 ] && pop_line '$NOT_MEASURED O-E-C1 was not found.' && [ \$(count '\[Population\]') -eq 1 ] && log_lacks '\[Population\] PROBLEM'"

# ---- the runs that wait for a summary, side by side
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side late "" poplate
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side nofill "" popstayssec
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side zeros "" popfill popzeros
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side tablelater "" popfill poptablelater
ENVS='TWEAKSV_TEST_SUMMARY_MS=6000' side_by_side stillness "" popfill popquiet popwait popstay 11
ENVS='TWEAKSV_TEST_SUMMARY_MS=3000' side_by_side grow "" popfill popgrow
ENVS='TWEAKSV_TEST_SUMMARY_MS=500' side_by_side quiet "" popfill popquiet popwait
side_by_side level1 "" popfill poplevel1 popwait
side_by_side level7 "" popfill poplevel7 popwait
side_by_side network "" popfill popnet popwait
side_by_side bind "" popfill popbind popwait
side_by_side override "" popfill popoverride popwait
side_by_side tunable "" popfill poptunable popwait
side_by_side raise "" popfill popraise popwait
side_by_side variety "" popfill popvariety popwait
side_by_side overflow "" popfill popoverflow popwait
side_by_side badmodel "" popfill popbadmodel popwait
side_by_side lists "" popfill poplists popwait
side_by_side nopool "" popfill popnopool popwait
side_by_side poolunread "" popfill poppoolunread popwait
side_by_side noconfig "" popfill popnoconfig popwait
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side gone "" popfill popgone
side_by_side writer "" popfill popwriter popwait
side_by_side scanmiss "" popfill popscanmiss popwait
side_by_side flip "" popfill popflip popwait
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000' side_by_side configlate "" popfill popquiet popconfiglate
ENVS='TWEAKSV_TEST_SUMMARY_MS=2000 TWEAKSV_TEST_REFUSE_THREAD="1 population sampling"' side_by_side nosampling "" popfill popwait
ENVS='TWEAKSV_TEST_REFUSE_THREAD="1 population summary"' side_by_side nosummary "" popfill
all_ended

echo "== O5 the game's first values, then the tables filled once the first summary has said they are not (poplate): .data's other numbers never read as a change"
after_run late
check "not filled yet at the first summary, then the tables' lines and both budgets" "pop_line '$NOT_FILLED' && [ \$(count 'model budgets are not filled yet') -eq 1 ] && pop_line '$VEHICLE_TABLES' && pop_line '$PED_TABLES' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180'"
check "the tables' first values never logged, no problem" "log_lacks '\[Population\].*28500000' && log_lacks '\[Population\].*17500000' && no_problem"
after_run nofill
check "never filled: the one line, its summaries every 2 s bringing no other" "pop_line '$NOT_FILLED' && [ \$(count 'model budgets are not filled yet') -eq 1 ] && log_lacks '\[Population\].*model budget: ' && log_lacks '\[Population\].*tables:' && no_problem"

after_run zeros
check "the override -1 with the lists still zeros: not filled yet, no entry of model 0 counted; once laid out, the boot's numbers" "pop_line '$NOT_FILLED' && log_lacks '\[Population\].*256 for traffic' && log_lacks '\[Population\].*at most 1280 at once' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180' && no_problem"

echo "== O6 a peak between two summaries, the fields the lines name, a level a script lowered and back (the test build, summaries every 3 s)"
after_run grow
check "the test build's line for this measurement" "log_line '\[TweaksV\] TEST BUILD: the population summary comes every 3000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=3000).\$'"
check "the peak shows in the highs of the next summary, with the use back: 500.0 now, at most 550.0; 31 + 21 models at once, 12 peds of one, 3 streamed" "pop_line 'DIAGNOSTIC: ped model budget: 500.0 of 561.0 MB in use at level 3 (the level.s 93.5 MB x variety 6.00), the requests stop at 553.5 MB, over the budget by 1.0 MB, at most 550.0 MB so far; ped models loaded: 19 for the population, 1 of them streamed, 5 outside the schedule, 4 in no group, 3 asked for directly, at most 52 at once, about 16.1 MB each; at the stop in 0 of [0-9]* samples; at most 12 peds of one model; at most 3 streamed at once$NO_HIGH'"
check "the vehicles' fields read where the findings have them: over by 1.5, 2 in flight, the pool's 100, at most 120, mods 2.0 and 3.0, the ambient cars 50, at most 64" "pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 429.0 MB in use at level 3 (the level.s 71.5 MB x variety 6.00), the requests stop at 427.5 MB, over the budget by 1.5 MB, at most 428.7 MB so far; vehicle models loaded: 94 for traffic, 0 outside the pop cycle, 0 trains and helicopters, 0 boats, 8 dropped, 2 in flight, at most 102 at once, about 4.1 MB each; at the stop in [0-9]* of [0-9]* samples; VehicleStruct 100 of 200 slots, at most 120 so far, unused models released past 180; of the bytes: weapons 10.1 MB, mods 2.0 and 3.0 MB; ambient cars on the map 50, at most 64 so far$NO_HIGH'"
check "level 1: its entry and its budget; back at 3, the lowest level said; the tables' lines still once each" "pop_line 'DIAGNOSTIC: ped model budget: 500.0 of 333.0 MB in use at level 1 (the level.s 55.5 MB x variety 6.00), the requests stop at 325.5 MB,.*' && pop_line 'DIAGNOSTIC: ped model budget: 500.0 of 561.0 MB in use at level 3 .*; at level 1 at the lowest so far; at most 12 peds of one model; at most 3 streamed at once$NO_HIGH' && pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 201.0 MB in use at level 1 .*' && [ \$(count 'model budget.s tables:') -eq 2 ]"
highs=($(tr -d '\r' < TweaksV.log | sed -n 's/.*DIAGNOSTIC: ped model budget: .*no new high for \([0-9]*\) s\.$/\1/p')); last=${highs[*]: -1}
check "each line's last words, in the seconds since a high moved: long at the first summary, short at the one after the peak, then growing with no high moving: ${highs[*]}" "[ ${#highs[@]} -ge 3 ] && [ ${highs[0]:-0} -gt ${highs[1]:-0} ] && [ ${last:-0} -gt ${highs[1]:-0} ]"
check "no problem, nothing written but what the fake changed" "no_problem"

echo "== O7 no number moves (popquiet, summaries every 500 ms, 3 s more): no second summary, however many samples"
after_run quiet
check "the test build's line, one summary of each budget" "log_line 'TWEAKSV_TEST_SUMMARY_MS=500' && [ \$(count 'DIAGNOSTIC: vehicle model budget: 300.0 of 429.0 MB') -eq 1 ] && [ \$(count 'DIAGNOSTIC: ped model budget: ') -eq 1 ] && [ \$(count 'model budget.s tables:') -eq 2 ]"
check "below the stop: at the stop in none of the samples" "pop_line 'DIAGNOSTIC: vehicle model budget: 300.0 of 429.0 MB in use .*; at the stop in 0 of [0-9]* samples; VehicleStruct .*' && no_problem"

after_run stillness
stills=($(tr -d '\r' < TweaksV.log | sed -n 's/.*DIAGNOSTIC: vehicle model budget: .*no new high for \([0-9]*\) s\.$/\1/p'))
check "summaries every 6 s, no number moving: one more line once no high has moved for two periods, 12 s, then none: ${stills[*]}" "[ ${#stills[@]} -eq 2 ] && [ ${stills[0]:-99} -lt 12 ] && [ ${stills[1]:-0} -ge 12 ] && no_problem"

echo "== O8 levels, a network game and the reserve"
after_run level1
check "level 1: its entry x the variety, both budgets, no line for a change" "pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 201.0 MB in use at level 1 (the level.s 33.5 MB x variety 6.00), the requests stop at 199.5 MB, over the budget by 0.0 MB, at most 428.7 MB so far; .*' && pop_line 'DIAGNOSTIC: ped model budget: 305.1 of 333.0 MB in use at level 1 (the level.s 55.5 MB x variety 6.00), the requests stop at 325.5 MB,.*' && no_problem"
after_run level7
check "a level above the game's top makes no sample: the line says so, for each budget" "pop_line 'DIAGNOSTIC: vehicle model budget: no sample yet - its level reads 7, above the game.s 3.' && pop_line 'DIAGNOSTIC: ped model budget: no sample yet - its level reads 7, above the game.s 3.' && log_lacks '\[Population\].*tables:' && no_problem"
after_run network
check "a network game: the network table alone stands, no variety, and the line says so; the tables' line names its other entry" "pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 72.0 MB in use at level 3 (the network table.s 72.0 MB, no variety), the requests stop at 70.5 MB,.*' && pop_line 'DIAGNOSTIC: ped model budget: 305.1 of 94.0 MB in use at level 3 (the network table.s 94.0 MB, no variety), the requests stop at 86.5 MB,.*' && pop_line '$(tables_line vehicle 71500000 72000000)32000, 56000 and 70000, the game.s defaults - the story entries are theirs.' && no_problem"
after_run bind
samples=$(tr -d '\r' < TweaksV.log | sed -n 's/.*ped model budget: .*at the stop in \([0-9]*\) of \([0-9]*\) samples.*/\1 \2/p' | head -n 1)
check "the peds past their stop at 553.5: at it in every sample: $samples" "pop_line 'DIAGNOSTIC: ped model budget: 555.0 of 561.0 MB in use .*' && [ \"\${samples% *}\" = \"\${samples#* }\" ] && [ \"\${samples% *}\" -ge 5 ] && no_problem"
after_run override
check "the object's override of 4 in place of the schedule's 8: the stop at 556.5, past that use" "pop_line 'DIAGNOSTIC: ped model budget: 555.0 of 561.0 MB in use at level 3 (the level.s 93.5 MB x variety 6.00), the requests stop at 556.5 MB,.*; at the stop in 0 of [0-9]* samples; .*' && no_problem"

echo "== O9 what the tables' line is for: a tunable, a raise in gameconfig.xml; a variety past the slider, a budget past an int"
after_run tunable
check "an entry the configuration does not give: named, in each table's line; the budget from it" "pop_line '$(tables_line vehicle 120000000)32000, 56000 and 70000 - level 3.s entry holds 120000000 bytes, not the 71500000 they give: a tunable or another mod has changed it, or the game.s set-up has not run yet.' && pop_line 'DIAGNOSTIC: ped model budget.s tables: story 1500000, 60000000, 73500000 and 93500000 bytes, network 1500000, 55500000, 73500000 and 93500000 bytes; the configuration.s story Level1Kb to Level3Kb: 54000, 72000 and 92000 - level 1.s entry holds 60000000 bytes, not the 55500000 they give: a tunable or another mod has changed it, or the game.s set-up has not run yet.' && pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 720.0 MB in use at level 3 (the level.s 120.0 MB x variety 6.00),.*' && no_problem"
after_run raise
check "the raise of the findings' 7.0, 98500 in gameconfig.xml: the entry 100000000, 600 MB, and the line says the file took" "pop_line '$(tables_line vehicle 100000000)32000, 56000 and 98500, not the defaults of 1.0.1158.16, 32000, 56000 and 70000 - the story entries are theirs: gameconfig.xml, or a game update, has changed them.' && pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 600.0 MB in use at level 3 (the level.s 100.0 MB x variety 6.00), the requests stop at 598.5 MB,.*; at the stop in 0 of [0-9]* samples; .*' && pop_line '$PED_TABLES' && no_problem"
after_run tablelater
check "an entry changed after the first tables' line: that table's next line with its numbers, the other's not again, and the budget from it" "pop_line '$VEHICLE_TABLES' && pop_line '$(tables_line vehicle 100000000)32000, 56000 and 98500, not the defaults of 1.0.1158.16, 32000, 56000 and 70000 - the story entries are theirs: gameconfig.xml, or a game update, has changed them.' && [ \$(count 'vehicle model budget.s tables:') -eq 2 ] && [ \$(count 'ped model budget.s tables:') -eq 1 ] && pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 600.0 MB in use .*' && no_problem"
after_run variety
check "a variety of 10: its line, once for each budget, and the budget by it" "pop_line 'DIAGNOSTIC: vehicle model budget: the variety has been 10.00, above the 6.00 of the game.s slider - settings.xml or another mod has changed it.' && pop_line 'DIAGNOSTIC: ped model budget: the variety has been 10.00, above the 6.00 of the game.s slider - settings.xml or another mod has changed it.' && [ \$(count 'the variety has been') -eq 2 ] && pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 of 715.0 MB in use at level 3 (the level.s 71.5 MB x variety 10.00),.*' && no_problem"
after_run overflow
check "a budget past an int, by the game's own cast: nothing stops its requests, and the samples at the stop say so" "pop_line 'DIAGNOSTIC: vehicle model budget: 428.7 MB in use at level 3 (the level.s 400.0 MB x variety 6.00, which the game.s cast to int overflows: nothing stops its requests), over the budget by 0.0 MB, at most 428.7 MB so far; $VEHICLE_MODELS; at the stop in 0 of [0-9]* samples; .*' && no_problem"

echo "== O10 what cannot be read: each said, the rest of the line standing"
after_run badmodel
check "two models' infos, a null pointer and one into the hole: skipped and counted, the most peds and the streamed from the others" "pop_line 'DIAGNOSTIC: ped model budget: 305.1 of 561.0 MB in use .*; $PED_MODELS, about 9.8 MB each; at the stop in 0 of [0-9]* samples; at most 9 peds of one model; at most 1 streamed at once; 2 model infos could not be read this time$NO_HIGH' && no_problem"
after_run flip
check "a model of the population's list released while it is read, over and over: the free value never read as a model" "pop_line 'DIAGNOSTIC: ped model budget: 305.1 of 561.0 MB in use .*; at most 9 peds of one model; at most 1 streamed at once$NO_HIGH' && no_problem"
after_run lists
check "a list's ends: a value not below the model count, at its 50th entry; the 256 entries of a list with none" "pop_line 'DIAGNOSTIC: vehicle model budget: .*; vehicle models loaded: 50 for traffic, 0 outside the pop cycle, 0 trains and helicopters, 256 boats, 8 dropped, 0 in flight, at most 314 at once, about 1.3 MB each; .*' && no_problem"
after_run nopool
check "VehicleStruct not made yet" "pop_line 'DIAGNOSTIC: vehicle model budget: .*; VehicleStruct not made yet; of the bytes: .*' && no_problem"
after_run poolunread
check "VehicleStruct's pointer into the hole" "pop_line 'DIAGNOSTIC: vehicle model budget: .*; VehicleStruct could not be read this time; of the bytes: .*' && no_problem"
after_run noconfig
check "the configuration: the tables' lines without it, the budgets as they are" "pop_line '$(tables_line vehicle | sed 's/; the configuration.*//'); the configuration at $A could not be read this time.' && pop_line '$(tables_line ped | sed 's/; the configuration.*//'); the configuration at $A could not be read this time.' && pop_line '$VEHICLE_AT_180' && no_problem"
after_run configlate
check "the configuration read at the next summary: the tables' lines come again, whole, each once; then, nothing moving, no summary more" "pop_line '$(tables_line vehicle | sed 's/; the configuration.*//'); the configuration at $A could not be read this time.' && pop_line '$VEHICLE_TABLES' && pop_line '$PED_TABLES' && [ \$(count 'model budget.s tables:') -eq 4 ] && [ \$(count 'DIAGNOSTIC: vehicle model budget: 300.0') -eq 2 ] && no_problem"
after_run gone
check "the object's page gone: the line, no other; back again, the summaries as before" "pop_line 'DIAGNOSTIC: the model budgets at $A could not be read this time.' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180' && no_problem"

echo "== O11 a writer of a story table besides the set-up's (W3): a line of the log, and the measurement goes on"
after_run writer
check "the line of each table, naming the other store" "pop_line 'DIAGNOSTIC: writers of the vehicle story table at $A: the set-up.s stores at $A, $A and $A, 1 besides them: $A (O-E-V7)' && pop_line 'DIAGNOSTIC: writers of the ped story table at $A: the set-up.s stores at $A, $A and $A, 1 besides them: $A ($P_LABELS)'"
check "the measurement goes on: announced, both lines, no problem" "pop_line '$ANNOUNCED' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180' && no_problem"

after_run scanmiss
check "a scan that misses one of the set-up's stores: said, and no proof claimed; the other table's as ever" "pop_line 'DIAGNOSTIC: writers of the vehicle story table at $A: the scan finds 2 of the set-up.s 3 stores, not the one at $A, so it proves nothing about other writers (O-E-V7)' && pop_line 'DIAGNOSTIC: writers of the ped story table at $A: the set-up.s stores at $A, $A and $A, none besides them ($P_LABELS)'"
check "the measurement goes on: announced, both lines; no problem of the part, nothing written" "pop_line '$ANNOUNCED' && pop_line '$VEHICLE_AT_180' && pop_line '$PED_AT_180' && log_lacks '\[Population\] PROBLEM' && has 'pop sites: as placed | pop data: as placed |'"

echo "== O12 a thread that cannot start: logged, never a problem"
after_run nosampling
check "no sampling thread: its line, the announcement's other words, and each summary takes a sample of its own: the first finds the fill, not yet taken from one sample, the next counts" "log_line '\[TweaksV\] The population sampling thread could not be started (Windows error 8).' && pop_line 'DIAGNOSTIC: the model budgets, in the population object at $A (O-E-C1): sampled once a summary alone, their sampling thread not started, nothing written; .*' && log_lacks '\[Population\].*sampled every second' && pop_line '$NOT_FILLED' && pop_line 'DIAGNOSTIC: vehicle model budget: .*; at the stop in 1 of 1 samples; .*' && no_problem"
after_run nosummary
check "no summary thread: its lines, no announcement, no line of a budget" "log_line '\[TweaksV\] The population summary thread could not be started (Windows error 8).' && pop_line '$NOT_MEASURED its summary thread could not be started.' && log_lacks '\[Population\] DIAGNOSTIC: the model budgets, in the population' && log_lacks '\[Population\].*model budget: ' && no_problem"

# ---- each pattern missing and each site not as the findings have it: runs of 2 seconds, side by side
for label in $LABELS; do
	side_by_side "$label" "addkey TEST_BREAK_PATTERN $label"
done
all_ended
echo "== O13 TEST_BREAK_PATTERN (test build): each pattern missing - logged, never a problem, nothing more measured"
for label in $LABELS; do
	after_run "$label"
	check "$label: the test line, the scanner's, the module's; no site logged, no announcement" "log_has 'TEST BUILD: pattern $label is treated as not found' && pop_line '$NOT_MEASURED $label was not found.' && [ \$(count '\[Population\]') -eq 1 ] && no_problem"
done

# A row: its options, then the module's reason, @ and ~ standing for an address inside and outside the game.
rows=()
HOOKED='is not a call into the game (another mod may have hooked it already)'
for budget in ped vehicle; do
	g=${budget:0:1}get
	rows+=(
		"popref ${budget}getter out|the $budget budget.s getter.s call at @ $HOOKED (O-E-C1)."
		"popref ${budget}getter hole|the $budget budget.s getter at @ (O-E-C1) cannot be read."
		"popref ${budget}getter next|the $budget budget.s getter at @ (O-E-C1) is not as found: .*"
		"popref ${g}network next|the $budget budget.s network table at @ (O-E-C1) does not lie right behind its story table at @."
		"popref ${g}level out|the $budget budget.s level at ~ (O-E-C1) is not inside the game."
		"popref ${g}level odd|the $budget budget.s level at @ (O-E-C1) is not aligned to 4 bytes."
		"popref ${g}level hole|the $budget budget.s level at @ (O-E-C1) cannot be read."
		"popref ${g}flag out|the $budget budget.s network flag at ~ (O-E-C1) is not inside the game."
		"popref ${g}flag hole|the $budget budget.s network flag at @ (O-E-C1) cannot be read."
		"popref ${g}story out popref ${g}network out|the $budget budget.s tables at ~ (O-E-C1) is not inside the game."
		"popref ${g}story odd popref ${g}network odd|the $budget budget.s tables at @ (O-E-C1) is not aligned to 4 bytes."
		"popref ${g}story hole popref ${g}network hole|the $budget budget.s tables at @ (O-E-C1) cannot be read."
		"popref ${g}variety out|the $budget budget.s variety at ~ (O-E-C1) is not inside the game."
		"popref ${g}variety odd|the $budget budget.s variety at @ (O-E-C1) is not aligned to 4 bytes."
		"popref ${g}variety hole|the $budget budget.s variety at @ (O-E-C1) cannot be read."
	)
done
for what in 'pool|VehicleStruct pool.s pointer|O-E-V3|8' 'ambient|ambient cars. count|O-E-V5|4' 'holder|configuration.s holder|O-E-V4|8' 'modeltable|model info table|O-E-P4|8' 'modelstride|model info table.s stride|O-E-P4|8' 'object|population object|O-E-C1|4'; do
	IFS='|' read -r name words label bytes <<< "$what"
	rows+=(
		"popref $name out|the $words at ~ ($label) is not inside the game."
		"popref $name odd|the $words at @ ($label) is not aligned to $bytes bytes."
		"popref $name hole|the $words at @ ($label) cannot be read."
	)
done
rows+=(
	"popref text out|the crash context.s text at ~ (O-E-C1) is not inside the game."
	"popref text hole|the crash context.s text at @ (O-E-C1) cannot be read."
	"popref text next|the crash context.s text at @ (O-E-C1) is not \"Ped Memory Use/Budget : %d/%d\"."
	"popgetterbyte|the ped budget.s getter at @ (O-E-C1) is not as found: .*"
	"popgetterbytev|the vehicle budget.s getter at @ (O-E-C1) is not as found: .*"
	"popref vgetflag next|the network flag at @ (O-E-C1) is not the one the ped budget.s getter reads, at @."
	"popref vtablevel next|the level at @ (O-E-V1) is not the one the vehicle budget.s getter reads, at @."
	"popref vtabflag next|the network flag at @ (O-E-V1) is not the one the vehicle budget.s getter reads, at @."
	"popref vtabstory next|the story table at @ (O-E-V1) is not the one the vehicle budget.s getter reads, at @."
	"popref vtabnetwork next|the network table at @ (O-E-V1) is not the one the vehicle budget.s getter reads, at @."
	"popref vcheckvariety next|the variety at @ (O-E-V2) is not the one the vehicle budget.s getter reads, at @."
	"popref vrequest out|the vehicle model.s request at @ $HOOKED (O-E-V2)."
	"popambientmoved|the ambient cars. read at @ (O-E-V5) does not lie right before the crash context.s line at @."
	"popref setupcall out|the tables. set-up call at @ $HOOKED (O-E-V6)."
	"popref setupcall next|the tables. set-up call at @ leads to @, not to the set-up at @ (O-E-V4)."
	"popnotstart|the tables. set-up at @ is not a function start (no unwind data, byte before it 48) (O-E-V4)."
	"popcalltwice|the game code calls the tables. set-up at @ in 2 or more places, not the expected 1 (O-E-V6)."
	"popvmoved|O-E-V7 at @ does not lie in the tables. set-up, where its block 6 is, at @."
	"poppmoved|O-E-P6 at @ does not lie in the tables. set-up, where its block 1 is, at @."
	"popref vstore1 next|the vehicle story table.s store at @ leads to @, not to its entry 1 at @ (O-E-V7)."
	"popref vstore2 next|the vehicle story table.s store at @ leads to @, not to its entry 2 at @ (O-E-V7)."
	"popref vstore3 next|the vehicle story table.s store at @ leads to @, not to its entry 3 at @ (O-E-V7)."
	"popref pstore1 next|the ped story table.s store at @ leads to @, not to its entry 1 at @ ($P_LABELS)."
	"popref pstore2 next|the ped story table.s store at @ leads to @, not to its entry 2 at @ ($P_LABELS)."
	"popref pstore3 next|the ped story table.s store at @ leads to @, not to its entry 3 at @ ($P_LABELS)."
	"popref pedsum next|the ped memory.s display at @ (O-E-P1) is not the one the crash context.s line reads, at @."
	"popref gatelevel next|the ped budget.s level at @ (O-E-P2) is not the one the ped budget.s getter reads, at @."
	"popgatelist|the ped budget.s gate at @ holds 41 8B 90 AC 04 4E 00 00 at +0x7A, not 41 8B 9C AC 04 4E 00 00 (O-E-P2)."
	"popgatefloor|the ped budget.s gate at @ holds 83 FD 08 0F 9C C0 83 F9 03 0F 94 C1 at +0xEA, not 83 FD 06 0F 9C C0 83 F9 03 0F 94 C1 (O-E-P2)."
	"popgatecall|the ped model.s request at @ $HOOKED (O-E-P2)."
	"popref gaterequest out|the ped model.s request at @ $HOOKED (O-E-P2)."
	"popref modelcount out|the model count at ~ (O-E-P3) is not inside the game."
	"popref modelcount byte|the model count at @ (O-E-P3) is not aligned to 2 bytes."
	"popref modelcount hole|the model count at @ (O-E-P3) cannot be read."
	"popref varstore next|the ped budget.s variety at @ (O-E-P8) is not the one the ped budget.s getter reads, at @."
	"popref varfactor out|the ped variety.s factor at ~ (O-E-P8) is not inside the game."
	"popref varadded hole|the ped variety.s sum at @ (O-E-P8) cannot be read."
	"popref varmost odd|the ped variety.s most at @ (O-E-P8) is not aligned to 4 bytes."
	"rtjumpunread popgateedge|the code past O-E-P2.s match at @ cannot be read."
)
n=0
for row in "${rows[@]}"; do
	n=$((n + 1))
	side_by_side "row$n" "" ${row%%|*}
done
all_ended
echo "== O14 each site not as the findings have it: its reason, word for word; logged, never a problem, no announcement"
n=0
for row in "${rows[@]}"; do
	n=$((n + 1))
	after_run "row$n"
	why=${row#*|}; why=${why//@/$A}; why=${why//\~/$OUT}
	check "${row%%|*}: ${row#*|}" "pop_line '$NOT_MEASURED $why' && log_lacks '\[Population\] DIAGNOSTIC: the model budgets, in the population' && no_problem"
done

B=$game_folder
cd "$B" && rm -rf side
echo
echo "POPULATION CHECKS: $pass passed, $fail failed"
