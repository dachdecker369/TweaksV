#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The siren setting IDs, the SirenSettings part's first job (TweaksVSirenIds). First its measurement alone, with
# LARGE_SIREN_IDS=0 or the part off: with DIAGNOSTIC_MODE=1 only, the 14 patterns found in the full fake game, every site
# checked and logged word for word, and the measurement over the fake's carcols data and ID table at the game's sizes - 10
# seconds in, again when the numbers change (the test build's TWEAKSV_TEST_SUMMARY_MS), before the base files are read, with
# the table full and with the settings unreadable; a pattern missing, the plain game, a site another mod changed, the table
# moved, split or loaded a fifth time, a member's count, offset or data, a jump, a fill, a call and a pointer not as the findings
# have them - each logged, with no problem, no box and nothing measured; a member's type a word and the structures registered
# after TweaksV has started, logged at the start and at the first summary and measured all the same; DRY_RUN alike; nothing
# written - the fake's sites and data as placed, every page's protection its own. Then the part: its writes and nothing
# else, every page's protection its own, the sites run as code; the table moved at its fixed size on the relocation core;
# the plain game; the part off; DRY_RUN; each check failing on its own and a site another mod changed, a problem with nothing
# written; the game's init and its load through the redirected call, IDs past 255 kept whole; the drops and moves per file with
# DIAGNOSTIC_MODE; and a start without the part against one with it. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

# the module's lines: under [SirenSettings], with DIAGNOSTIC: in front
L='\[SirenSettings\] DIAGNOSTIC: '
RVA='RVA 0x[0-9A-F]*'
# its 14 patterns, for the scanner's lines
LABELS='I-E-(P[1-3]|T[1-4]|D[12]|C[1-3]|R1|M1)'
# untouched: TweaksV wrote nothing into the fake's siren sites and data, and every page has its own protection back
untouched() { has 'siren ids: sites as placed 17 of 17, data as placed' && has 'table not moved' && has 'protect: all restored (RX)'; }
# measure_ini: the file for the measurement alone, DIAGNOSTIC_MODE on and the part's job off
measure_ini() { fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; }
# lines LINE...: every line is in the log, word for word
lines() { local l; for l in "$@"; do log_line "${L}$l\$" || { echo "      missing: $l"; return 1; }; done; }
# measured: the announcement, word for word; summary LOADED INUSE PAST TWICE: the summary, word for word; summaries: how many
measured() { lines "the siren settings, through the carcols pointer at $RVA (I-E-M1), and the siren setting ID table at $RVA, 256 IDs (I-E-T1 to I-E-T4): the settings loaded and the IDs in use are logged after 10 seconds, then every minute while the numbers change."; }
summary() { lines "siren settings loaded: $1 (vehicles can use 255 at most); siren setting IDs in use: $2 of 256; settings past position 255, which no vehicle can reach: $3; IDs given to more than one setting: $4."; }
summaries() { grep -c -E "${L}(siren settings loaded: |no siren settings loaded yet|the siren settings at .* could not be read)" TweaksV.log; }
# not_measured WHY: the line, word for word up to WHY's end; no problem, no announcement, no summary
not_measured() { lines "the siren settings are not measured: $1" && log_lacks 'PROBLEM' && log_lacks 'the settings loaded and the IDs in use are logged' && [ "$(summaries)" -eq 0 ]; }
# accessor_gives N: the test build's line of SirenSettingsNow, N settings at the array the report names; accessor_nothing: its line when it gives nothing
accessor_gives() { log_line "\[SirenSettings\] TEST BUILD: the siren settings' accessor gives $1 at $(sed -n 's/.*siren array: \(0x[0-9A-F]*\) .*/\1/p' <<< "$out") (outside the game), 0x5E8 bytes apart\.\$"; }
accessor_nothing() { log_line "\[SirenSettings\] TEST BUILD: the siren settings' accessor gives nothing\.\$"; }
# no_box: nothing reported, so no start-up summary and no box (ERROR_POPUP=1, test build)
no_box() { log_lacks 'Showing the start-up summary' && log_lacks 'TEST BUILD: message box not shown'; }

# every site's line, word for word, in the order of the patterns
SITES=(
	"siren settings' parser registration at $RVA: name hash 0x84EB9B0D, member data table at $RVA, offsets table at $RVA (I-E-P1)"
	"siren setting's ID member: member 0 at +0x0, member 2 at +0x4; its data at $RVA, offset field 0x0, type 0x1 (I-E-P1)"
	"vehicle variations' parser registration at $RVA: name hash 0x18EAFEA9, member data table at $RVA, offsets table at $RVA (I-E-P2)"
	"variation's siren setting ID member: member 6 of 7 at +0x4B, member 5 at +0x4A; its data at $RVA, offset field 0x4B, type 0x1 (I-E-P2)"
	"simple member's XML reader at $RVA: the member data at +0x8 of the member, its type at +0x10, types up to 0x21 (I-E-P3)"
	"reference at $RVA -> $RVA (I-E-T1)"
	"siren setting ID load at $RVA: 0F B6 2C 0F (I-E-T1)"
	"siren setting ID check for none at $RVA: 48 81 FD FF 00 00 00 (I-E-T1)"
	"siren setting ID load at $RVA: 0F B6 1C 2B (I-E-T1)"
	"variation's siren setting ID store at $RVA: 88 1C 0F (I-E-T1)"
	"reference at $RVA -> $RVA (I-E-T2)"
	"siren setting ID store at $RVA: 40 88 31 (I-E-T2)"
	"siren setting ID load at $RVA: 0F B6 2C 39 (I-E-T2)"
	"siren setting ID check for none at $RVA: 48 81 FD FF 00 00 00 (I-E-T2)"
	"limit at $RVA holds 256 (N, I-E-T2) -> game size 256"
	"siren setting ID given when none is free at $RVA: 40 B6 FF (I-E-T2)"
	"reference at $RVA -> $RVA (I-E-T3)"
	"siren setting ID load at $RVA: 0F B6 3C 03 (I-E-T3)"
	"siren setting ID check for none at $RVA: 48 81 FF FF 00 00 00 (I-E-T3)"
	"siren setting ID load at $RVA: 0F B6 14 17 (I-E-T3)"
	"variation's siren setting ID store at $RVA: 88 14 03 (I-E-T3)"
	"carcols pointer load at $RVA -> $RVA (I-E-T4)"
	"reference at $RVA -> $RVA (I-E-T4)"
	"siren setting ID store at $RVA: 40 88 33 (I-E-T4)"
	"siren setting ID load at $RVA: 0F B6 0C 3B (I-E-T4)"
	"siren setting ID load at $RVA: 0F B6 34 3B (I-E-T4)"
	"siren setting ID check for none at $RVA: 48 81 FE FF 00 00 00 (I-E-T4)"
	"limit at $RVA holds 256 (N, I-E-T4) -> game size 256"
	"siren setting ID given when none is free at $RVA: 40 B6 FF (I-E-T4)"
	"the siren setting ID table is at $RVA - 256 entries of 1 byte"
	"pack dedupe's siren setting ID load at $RVA: 43 0F B6 54 3D 00 (I-E-D1)"
	"pack dedupe's check for none at $RVA: 80 FA FF (I-E-D1)"
	"pack dedupe's compare loop at $RVA, its jz -> $RVA (I-E-D2)"
	"siren setting constructor's first store at +0x1C at $RVA: C7 41 1C 00 00 00 00 (I-E-C1)"
	"siren setting constructor's ID default at $RVA: C6 06 FF (I-E-C1)"
	"siren setting constructor's second store at +0x1C at $RVA: 89 46 1C (I-E-C1)"
	"variation's in-place constructor's stores at $RVA: C7 41 30 00 00 00 00 48 C7 41 38 00 00 00 00 C7 41 40 00 00 00 00 C7 41 48 00 00 FF FF C3 (I-E-C2)"
	"variation's allocating constructor's stores at $RVA: C7 40 30 00 00 00 00 48 C7 40 38 00 00 00 00 C7 40 40 00 00 00 00 C7 40 48 00 00 FF FF 48 83 C4 28 C3 (I-E-C3)"
	"base loader call at $RVA -> $RVA, after the siren setting ID table's last fill at $RVA -> $RVA (I-E-R1)"
	"siren settings getter at $RVA: a model's siren setting index at +0x53B, the settings at +0x48 of the carcols data, 0x5E8 bytes each; the carcols pointer at $RVA (I-E-M1)"
)
# the table's RVA, as the module names it; how many of the module's lines lead to it
table_rva() { tr -d '\r' < TweaksV.log | sed -n 's/.*the siren setting ID table is at \(RVA 0x[0-9A-F]*\) - 256 entries of 1 byte$/\1/p'; }
to_table() { tr -d '\r' < TweaksV.log | grep -c -E "${L}reference at RVA 0x[0-9A-F]+ -> $(table_rva) \($1\)$"; }

echo "== I1 LARGE_SIREN_IDS=0 without DIAGNOSTIC_MODE: nothing looked for, nothing measured"
fresh_ini; setkey LARGE_SIREN_IDS 0 SIRENSETTINGS; out=$(game full_game.exe); echo "      $out"
check "none of the 14 patterns looked for, the switch's line alone; nothing written" "[ \$(grep -c -E 'Pattern $LABELS' TweaksV.log) -eq 0 ] && log_line \"\[SirenSettings\] LARGE_SIREN_IDS=0 - the siren setting IDs' limit is not raised.\$\" && [ \$(count '\[SirenSettings\]') -eq 1 ] && untouched"

echo "== I2 DIAGNOSTIC_MODE=1, LARGE_SIREN_IDS=0: every pattern found once, every site's line, the announcement, the summary 10 s in"
measure_ini; out=$(game full_game.exe wait)
check "the 14 patterns each found exactly once" "[ \$(grep -c -E 'Pattern $LABELS found: .* 1 match,' TweaksV.log) -eq 14 ]"
check "every site's line, word for word: the registrations, the reader, the loaders, the dedupe, the constructors, the init, the getter" "lines \"\${SITES[@]}\""
check "the 2 limits of each builder and the 4 lea of the table, to one table, and the scan's 4 to the same; the IDs' bytes 8 loads and 2 stores" "[ \$(count '${L}limit at ') -eq 4 ] && [ \$(to_table 'I-E-T[1-4]') -eq 4 ] && [ \$(to_table 'lea scan') -eq 4 ] && [ \$(count '${L}siren setting ID load at ') -eq 8 ] && [ \$(count '${L}siren setting ID store at ') -eq 2 ]"
check "the announcement, word for word" "measured"
check "the summary 10 s in, once, at the game's sizes: 24 settings, 24 of the 256 IDs in use, none past position 255, no ID given twice" "summary 24 24 0 0 && [ \$(summaries) -eq 1 ]"
check "the two ID members once more, right before the first summary: their data, offset fields 0x0 and 0x4B, type 0x1" "lines \"siren setting's ID member, at the first summary: its data at $RVA, offset field 0x0, type 0x1 (I-E-P1)\" \"variation's siren setting ID member, at the first summary: its data at $RVA, offset field 0x4B, type 0x1 (I-E-P2)\" && [ \$(grep -n 'at the first summary' TweaksV.log | tail -n 1 | cut -d: -f1) -lt \$(grep -n 'siren settings loaded: ' TweaksV.log | head -n 1 | cut -d: -f1) ]"
check "no problem; nothing written - the sites and the data as placed, every page's protection its own" "log_lacks 'PROBLEM' && untouched"

echo "== I3 the test build, TWEAKSV_TEST_SUMMARY_MS=500: a pack's 6 settings at 12 s, the summary again when the numbers change"
export TWEAKSV_TEST_SUMMARY_MS=500
measure_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sipack)
check "the test build's line, once for the summary of the siren settings" "[ \$(count 'TEST BUILD: the siren settings summary comes every 500 ms after its first (TWEAKSV_TEST_SUMMARY_MS=500).') -eq 1 ]"
check "two summaries: 24 settings 10 s in, then 30, the pack's IDs in use too - and no more" "summary 24 24 0 0 && summary 30 30 0 0 && [ \$(summaries) -eq 2 ] && grep -m 1 'siren settings loaded: ' TweaksV.log | grep -q 'loaded: 24 '"
check "nothing written; the data as the fake left it after the pack" "log_lacks 'PROBLEM' && untouched"

echo "== I4 the same, the game's base files not read yet at 10 s (the carcols pointer null), read at 12 s"
measure_ini; out=$(ASI=TweaksV_test.asi game full_game.exe sinone)
check "the first summary says so, word for word; the second has the 24 settings - and no more" "lines 'no siren settings loaded yet - the game has not read carcols.ymt.' && summary 24 24 0 0 && [ \$(summaries) -eq 2 ] && grep -m 1 -E 'siren settings loaded' TweaksV.log | grep -q 'no siren settings loaded yet'"
check "nothing written" "log_lacks 'PROBLEM' && untouched"
check "the accessor of the siren settings (test build's line): nothing while the pointer is null, then the 24, read that moment" "accessor_nothing && accessor_gives 24"
unset TWEAKSV_TEST_SUMMARY_MS

echo "== I5 300 settings, given their IDs as the game gives them: the table full, 44 past position 255, the ID 255 to the last 45"
measure_ini; out=$(game full_game.exe simany wait)
check "the summary, word for word: 256 of 256 IDs in use, 44 past position 255, one ID given to more than one setting" "summary 300 256 44 1 && untouched"

echo "== I6 the settings' array in memory that cannot be read: said, not counted"
measure_ini; out=$(game full_game.exe siunreadable wait)
check "the summary says so, word for word; no problem" "lines 'the siren settings at 0x[0-9A-F]* (outside the game) could not be read this time (the game may have been changing them).' && [ \$(summaries) -eq 1 ] && log_lacks 'PROBLEM' && untouched"

echo "== I7 DRY_RUN=1 with DIAGNOSTIC_MODE=1 and LARGE_SIREN_IDS=0: alike - nothing of the module is written either way"
measure_ini; setkey DRY_RUN 1; out=$(game full_game.exe wait)
check "the 14 patterns, every site's line, the announcement and the summary as without DRY_RUN; nothing written" "[ \$(grep -c -E 'Pattern $LABELS found: .* 1 match,' TweaksV.log) -eq 14 ] && lines \"\${SITES[@]}\" && measured && summary 24 24 0 0 && untouched"

echo "== I8 TEST_BREAK_PATTERN=I-E-T3 (test build), ERROR_POPUP=1: logged, no problem, no box, no summary"
measure_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN I-E-T3; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "the pattern treated as not found, every other looked for; the line, word for word" "log_has 'TEST BUILD: pattern I-E-T3 is treated as not found (TEST_BREAK_PATTERN).' && [ \$(grep -c -E 'Pattern $LABELS found: ' TweaksV.log) -eq 13 ] && not_measured 'I-E-T3 was not found.'"
check "no box, nothing written" "no_box && untouched"

echo "== I9 a site another mod changed - the base loop's first ID load a word load already - ERROR_POPUP=1 (test build)"
measure_ini; setkey ERROR_POPUP 1; out=$(ASI=TweaksV_test.asi game full_game.exe siwidened wait)
check "the line, word for word, with the bytes found and those expected" "not_measured 'the siren setting ID load at $RVA holds 0F B7 2C 0F, not 0F B6 2C 0F (I-E-T1).'"
check "no box, nothing written" "no_box && untouched"

echo "== I10 an address outside the game: the four lea of the table lead outside it (another mod moved it)"
measure_ini; out=$(game full_game.exe simoved)
check "the line, word for word; nothing written" "not_measured 'the siren setting ID table is not inside the game: 0x[0-9A-F]* (outside the game) (another mod may have moved it already).' && untouched"

echo "== I11 the plain game: every pattern missing, the first named"
measure_ini; out=$(game plain_game.exe)
check "the 14 not found, the line names I-E-P1; no problem of the module's (the parts report theirs), nothing measured" "[ \$(grep -c -E 'Pattern $LABELS not found: ' TweaksV.log) -eq 14 ] && lines 'the siren settings are not measured: I-E-P1 was not found.' && log_lacks '\[SirenSettings\] PROBLEM' && log_lacks 'the settings loaded and the IDs in use are logged'"

echo "== I12 the parser registrations not as the findings have them, each logged and nothing measured"
measure_ini; out=$(game full_game.exe simembers)
check "an eighth member of the variations" "not_measured \"the vehicle variations' parser registration has 8 or more members, not 7 (I-E-P2).\" && untouched"
measure_ini; out=$(game full_game.exe sioffsets)
check "the siren settings' member 2 at +0x8" "not_measured \"the siren settings' members 0 and 2 are at +0x0 and +0x8, not +0x0 and +0x4 (I-E-P1).\" && untouched"
measure_ini; out=$(game full_game.exe sidataout)
check "the ID member's data outside the game" "not_measured \"the siren setting's ID member's data is not inside the game: 0x[0-9A-F]* (outside the game) (I-E-P1).\" && untouched"

echo "== I13 the ID table split or loaded a fifth time, each logged and nothing measured"
measure_ini; out=$(game full_game.exe sisplit)
check "a pack's builder's lea 16 bytes into the table" "not_measured \"the references to the siren setting ID table do not all point to the game's own table: $RVA and $RVA (I-E-T4).\" && untouched"
measure_ini; out=$(game full_game.exe sififth)
check "a fifth lea: the scan's 5 lines, then the reason" "[ \$(to_table 'lea scan') -eq 5 ] && not_measured 'the game code loads the siren setting ID table in 5 or more places, not the expected 4 (another mod may have moved some of them, or the game has changed).' && untouched"

echo "== I14 a jump, a fill, a call and a pointer not as the findings have them, each logged and nothing measured"
measure_ini; out=$(game full_game.exe sijz)
check "the dedupe loop's jz outside the game" "not_measured \"the jz of the pack dedupe's compare loop at $RVA does not lead into the game (another mod may have changed it) (I-E-D2).\" && untouched"
measure_ini; out=$(game full_game.exe sifill)
check "the init's last fill 16 bytes below the table's last 16" "not_measured \"the store before the base loader call at $RVA fills $RVA, not the siren setting ID table's last 16 bytes at $RVA (I-E-R1).\" && untouched"
measure_ini; out=$(game full_game.exe sicallhooked)
check "the base loader call another mod redirected" "not_measured 'the base loader call at $RVA is not a call into the game (another mod may have hooked it already) (I-E-R1).' && untouched"
measure_ini; out=$(game full_game.exe sicarcols)
check "the getter's carcols pointer not a pack builder's" "not_measured \"the carcols pointer the siren settings getter reads, $RVA, is not the one a pack's builder reads, $RVA (I-E-M1).\" && untouched"

echo "== I15 the variations' ID member's type a word, and the structures registered after TweaksV has started: logged twice, measured"
measure_ini; out=$(game full_game.exe sitype siregister wait)
check "at the start: the offset fields not written yet, the type 0x3; the measurement announced all the same" "lines \"siren setting's ID member: member 0 at +0x0, member 2 at +0x4; its data at $RVA, offset field 0x0, type 0x1 (I-E-P1)\" \"variation's siren setting ID member: member 6 of 7 at +0x4B, member 5 at +0x4A; its data at $RVA, offset field 0x0, type 0x3 (I-E-P2)\" && measured"
check "the type's finding, word for word, at the start and at the first summary; there the offset field 0x4B the game wrote since; then the summary" "[ \$(count \"${L}the variation's siren setting ID member's type at RVA 0x[0-9A-F]* is 0x3, not 0x1 or 0x2, which store a byte - a finding for the part (I-E-P2).\") -eq 2 ] && lines \"variation's siren setting ID member, at the first summary: its data at $RVA, offset field 0x4B, type 0x3 (I-E-P2)\" && summary 24 24 0 0"
check "no problem; nothing written - the data as the fake left it after it registered the structures" "log_lacks 'PROBLEM' && untouched"

# ---- the part

P='\[SirenSettings\] '
JOB="The siren setting IDs' limit is not raised this session:"
# written: every write of the part, the word for word status lines; the sites run as code do what the findings say
written() { has ', data widened' && has 'siren writes: 17 of 17 as the part leaves them' && has 'types 0x3 and 0x3, member data RW' && has 'base loader call to a stub, RX' && has 'siren code: loads and stores 11 words 0 bytes, loop words, constructors 3 words 0 bytes' && has 'protect: all restored (RX)'; }
# a_word: a pack's dedupe's load a word's, without a stub, no hook; the announcement over the moved table; ecount: count, by an extended pattern
a_word() { has "a pack's dedupe's load a word's, 0 hooks"; }
measured_wide() { lines "the siren settings, through the carcols pointer at $RVA (I-E-M1), and the siren setting ID table at 0x[0-9A-F]* (outside the game), 65536 IDs (I-E-T1 to I-E-T4): the settings loaded and the IDs in use are logged after 10 seconds, then every minute while the numbers change."; }
ecount() { grep -c -E -- "$1" TweaksV.log; }
moved_line() { log_line "${P}The siren setting ID table moved and its limit raised from 256 to 65536 entries.\$"; }
widened_line() { log_line "${P}The siren setting IDs' limit is raised to 65535: their 11 loads and stores, a pack's dedupe's loop, the 3 constructors' defaults and the 2 parser members' types are written, and the base loader call at $RVA is redirected, so that the moved table is filled whenever the game fills its own.\$"; }
# refused WHY: the problem, word for word up to WHY's end, nothing written, nothing measured
refused() { log_line "${P}PROBLEM: $JOB $1\$" && [ "$(count 'PROBLEM: The siren setting IDs')" -eq 1 ] && log_lacks 'The siren setting ID table moved' && log_lacks "The siren setting IDs' limit is raised" && log_lacks 'the settings loaded and the IDs in use are logged' && [ "$(summaries)" -eq 0 ] && untouched; }

echo "== W1 the default, LARGE_SIREN_IDS=1: everything written in one go, the sites as the part must leave them and nothing else"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the header: SirenSettings on" "log_has 'Features: .*, SirenSettings on'"
check "the status lines, word for word; no line of the diagnostics; no problem" "moved_line && widened_line && log_lacks '${P}DIAGNOSTIC' && log_lacks 'I-E-H' && log_lacks 'PROBLEM'"
check "the fixed size is no setting's: no line of the settings' clamp, the core's first of its kind" "log_lacks 'TweaksV.ini: LARGE_SIREN_IDS' && log_lacks 'is outside 256'"
check "the 17 sites as the part must leave them, byte for byte: the 4 lea to one table, the 4 limits 65536, the 11 loads and stores, the loop, the 3 constructors; no hook" "written && a_word"
check "the 2 members' types a word's, their page its own, RW; the data's other bytes as placed" "has 'data widened' && has 'types 0x3 and 0x3, member data RW'"
check "the table outside the game, read/write, the game's 256 entries copied, the rest free" "has \"table moved, RW, the game's 256 copied, the rest 0xFF\""
check "the sites run as code: every load a word's, every store too, the loop by words, the constructors' defaults; other bytes the same" "has 'siren code: loads and stores 11 words 0 bytes, loop words, constructors 3 words 0 bytes'"
check "the base loader's call through a stub of its own, read+execute, no stub added to the other parts' count" "has 'base loader call to a stub, RX' && has 'stubs: 7, all RX'"

echo "== W2 the part off, [SIRENSETTINGS] ENABLED=0: off in the header, nothing of it"
fresh_ini; setkey ENABLED 0 SIRENSETTINGS; out=$(game full_game.exe)
check "off in the header; no line, no pattern looked for; nothing written" "log_has 'Features: .*, SirenSettings off' && log_lacks '\[SirenSettings\]' && log_lacks 'I-E-P1' && untouched"

echo "== W3 the part off, DIAGNOSTIC_MODE=1: its measurement goes on, as the table of the parts measured first has it"
fresh_ini; setkey ENABLED 0 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "off in the header; the 14 patterns, the announcement and the summary over the game's own table; no hook looked for; nothing written" "log_has 'Features: .*, SirenSettings off' && [ \$(grep -c -E 'Pattern $LABELS found: .* 1 match,' TweaksV.log) -eq 14 ] && log_lacks 'I-E-H' && measured && summary 24 24 0 0 && untouched"

echo "== W4 DRY_RUN=1: everything found and checked, the writes named in one line, nothing written"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the core's would line, word for word" "log_line \"${P}DRY_RUN: would move the siren setting ID table (4 references) and raise its limit from 256 to 65536 (4 sites) and widen the IDs' 11 loads and stores, a pack's dedupe's loop, the 3 constructors' defaults and the 2 parser members' types, and redirect the base loader call at $RVA.\$\""
check "one line alone, as the core's other parts have it: no line per write" "[ \$(count '${P}DRY_RUN: ') -eq 1 ] && log_lacks '${P}DRY_RUN: would patch'"
check "nothing written, no status line, no problem" "untouched && log_lacks 'The siren setting ID table moved' && log_lacks \"The siren setting IDs' limit is raised\" && log_lacks 'PROBLEM'"

echo "== W5 DRY_RUN=1 with DIAGNOSTIC_MODE=1: the hooks named, not made; the measurement over the game's own table and IDs of a byte"
fresh_ini; setkey DRY_RUN 1; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "the would line of the hooks; every pattern found, H1 and H2 among them; nothing written" "log_line \"${P}DRY_RUN: would hook a pack's carcols callback at $RVA and the base builder at $RVA and take a pack's dedupe load through a stub, to count the settings dropped and moved.\$\" && [ \$(grep -c -E 'Pattern I-E-(P[1-3]|T[1-4]|D[12]|C[1-3]|R1|M1|H1|H2) found: .* 1 match,' TweaksV.log) -eq 16 ] && untouched"
check "the announcement and the summary at the game's sizes" "measured && summary 24 24 0 0"

echo "== W6 the plain game: reported, the pattern named"
fresh_ini; out=$(game plain_game.exe)
check "the problem, word for word, the first pattern named" "log_line \"${P}PROBLEM: $JOB I-E-P1 was not found.\$\""

echo "== W7 each pattern not found (test build), ERROR_POPUP=1: a problem, a box, nothing written"
for label in I-E-P1 I-E-P2 I-E-P3 I-E-T1 I-E-T2 I-E-T3 I-E-T4 I-E-D1 I-E-D2 I-E-C1 I-E-C2 I-E-C3 I-E-R1 I-E-M1; do
	fresh_ini; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN $label; out=$(ASI=TweaksV_test.asi game full_game.exe)
	check "$label: the problem, word for word; the box's line; nothing written" "refused '$label was not found.' && log_has 'TEST BUILD: message box not shown'"
done

echo "== W8 each check failing on its own, a site another mod changed among them: a problem with its reason, nothing written"
try_option() { fresh_ini; out=$(game full_game.exe "$1"); check "$1: $2" "refused \"$3\""; }
try_option siwidened "the base loop's first ID load a word load already" "the siren setting ID load at $RVA holds 0F B7 2C 0F, not 0F B6 2C 0F (I-E-T1)."
try_option sitype "the variations' ID member's type a word's already" "the variation's siren setting ID member's type at $RVA is 0x3, not 0x1 or 0x2, which store a byte (another mod may have widened it already) (I-E-P2)."
try_option simoved "the table's lea lead outside the game" "the siren setting ID table is not inside the game: 0x[0-9A-F]* (outside the game) (another mod may have moved it already)."
try_option sisplit "a lea 16 bytes into the table" "the references to the siren setting ID table do not all point to the game's own array: $RVA and $RVA (I-E-T4)."
try_option sififth "a fifth lea" "the game code loads the siren setting ID table in 5 or more places, not the expected 4 (another mod may have moved some of them, or the game has changed)."
try_option silimit "a limit past the fixed size" "the game's own siren setting ID table size, 65537, is not plausible."
try_option sidisagree "a limit that disagrees" "the limits of the siren setting ID table disagree about the game's own size: 256 and 512 (I-E-T4)."
try_option simembers "an eighth member of the variations" "the vehicle variations' parser registration has 8 or more members, not 7 (I-E-P2)."
try_option sioffsets "the siren settings' member 2 at +0x8" "the siren settings' members 0 and 2 are at +0x0 and +0x8, not +0x0 and +0x4 (I-E-P1)."
try_option sidataout "the ID member's data outside the game" "the siren setting's ID member's data is not inside the game: 0x[0-9A-F]* (outside the game) (I-E-P1)."
try_option sijz "the dedupe loop's jz outside the game" "the jz of the pack dedupe's compare loop at $RVA does not lead into the game (another mod may have changed it) (I-E-D2)."
try_option sifill "the init's last fill 16 bytes below" "the store before the base loader call at $RVA fills $RVA, not the siren setting ID table's last 16 bytes at $RVA (I-E-R1)."
try_option sicallhooked "the base loader call another mod redirected" "the base loader call at $RVA is not a call into the game (another mod may have hooked it already) (I-E-R1)."
try_option sicarcols "the getter's carcols pointer not a pack builder's" "the carcols pointer the siren settings getter reads, $RVA, is not the one a pack's builder reads, $RVA (I-E-M1)."

echo "== W9 the members registered after TweaksV has started: the offset fields 0 at the start, no finding; written"
fresh_ini; out=$(game full_game.exe siregister)
check "written, no problem" "written && moved_line && log_lacks 'PROBLEM'"

echo "== W10 no code may be changed (Windows' dynamic code policy): the part reported, nothing written"
fresh_ini; out=$(game full_game.exe nodyncode)
check "the problem, word for word, the stub's reason in the log; the sites and the data as placed" "log_line \"${P}PROBLEM: $JOB the game code could not be changed (details in TweaksV.log).\$\" && has 'siren ids: sites as placed 17 of 17, data as placed' && has 'table not moved'"

echo "== W11 the game's init after TweaksV has started: the table free when the base loader runs, the base files read through the call"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe siload wait)
check "the call through the stub to the loader, its arguments as handed, its result and last error back; the table free" "has 'siren load: 1 inits, loader 1 calls, 1 with its table free, 1 with their arguments, 1 of them back with result and last error, builder 1 back'"
check "the variations lead to their settings" "has 'variations 3 of 3'"
check "the sites through the hooks as the part must leave them: the stub of the dedupe's load, the 2 hooks" "has \"siren writes: 17 of 17 as the part leaves them, a pack's dedupe's load through a stub, RX, 2 hooks\""
check "the hooks' site lines and their line, word for word" "lines \"a pack's carcols callback at $RVA, no unwind data, byte before it 90 (I-E-H1)\" \"the base builder at $RVA, no unwind data, byte before it 90 (I-E-H2)\" \"a pack's carcols callback at $RVA (I-E-H1) and the base builder at $RVA (I-E-H2) are hooked, and a pack's dedupe load goes through a stub: the settings dropped and moved are logged per file.\""
check "the base file's line through the hook; the summary past 256 IDs and the IDs in their order" "lines 'the base carcols brought 24 siren settings: 0 moved, 0 without an ID given one.' 'siren settings loaded: 24 (vehicles can use 255 at most); siren setting IDs in use: 24 of 65536, 0 of them past 255; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0; settings the game dropped so far: 0, moved: 0.' 'siren setting IDs of settings 0 to 23 of 24: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24'"
check "the write lines with their old and new bytes, the announcement of 65536 IDs; no problem" "lines 'siren setting ID load at $RVA: 0F B6 2C 0F -> 0F B7 2C 0F' 'pack dedupe.s siren setting ID load through a stub at $RVA: 43 0F B6 54 3D 00 -> E9 [0-9A-F ]* 90' \"variation's siren setting ID member's type at $RVA: 01 -> 03\" 'base loader call at $RVA: E8 [0-9A-F ]* -> E8 [0-9A-F ]*' \"the siren settings, through the carcols pointer at $RVA (I-E-M1), and the siren setting ID table at 0x[0-9A-F]* (outside the game), 65536 IDs (I-E-T1 to I-E-T4): the settings loaded and the IDs in use are logged after 10 seconds, then every minute while the numbers change.\" && log_lacks 'PROBLEM'"

echo "== W12 the init twice: the moved table free both times"
fresh_ini; out=$(game full_game.exe siload sitwice)
check "the loader twice, the table free each time" "has 'siren load: 2 inits, loader 2 calls, 2 with its table free, 2 with their arguments, 2 of them back with result and last error, builder 2 back'"

echo "== W13 a clash in the base file: the later setting moved, one without an ID given one, through the builder's hook"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe siload siclash wait)
check "the base file's line, word for word; the summary's count" "lines 'the base carcols brought 24 siren settings: 1 moved (7 to 0), 1 without an ID given one.' 'siren settings loaded: 24 (vehicles can use 255 at most); siren setting IDs in use: 24 of 65536, 0 of them past 255; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0; settings the game dropped so far: 0, moved: 1.'"

echo "== W14 three packs through the callback: IDs past 255 whole, the taken dropped, the low byte 0xFF moved, every number and line per pack"
PACK1='dlcpacks:/sipack1/data/carcols.meta'; PACK2='dlcpacks:/sipack2/data/carcols.meta'; PACK3='dlcpacks:/sipack3/data/carcols.meta'
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe siload sipacks wait)
check "each pack's line, word for word" "lines \"a pack's carcols, $PACK1, brought 6 siren settings: 4 added, 2 dropped, their IDs taken (5, 300), 0 moved.\" \"a pack's carcols, $PACK2, brought 3 siren settings: 3 added, 0 dropped, 1 moved (511 to 25).\" \"a pack's carcols, $PACK3, brought 0 siren settings: 0 added, 0 dropped, 0 moved.\""
check "the summary: 31 settings, 31 of 65536 IDs, 4 past 255, 2 dropped, 1 moved; the IDs whole, in their order" "lines 'siren settings loaded: 31 (vehicles can use 255 at most); siren setting IDs in use: 31 of 65536, 4 of them past 255; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0; settings the game dropped so far: 2, moved: 1.' 'siren setting IDs of settings 0 to 30 of 31: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 300, 1534, 40, 0, 511, 767, 25'"
check "the callback back with its result and last error each time, every register the dedupe's stub must keep kept, the variations at their settings" "has \"packs 3 of 3 back, dedupe's registers kept 9 of 9, variations 8 of 8\""

echo "== W15 the same with LARGE_SIREN_IDS=0: the comparison a start without the part gives - IDs cut to their low byte, no hook, nothing written"
measure_ini; out=$(game full_game.exe siload sipacks wait)
check "the base game's numbers and IDs the same as W11's, the pack's IDs cut: 300 is 44, 1534 is 254, 40 stays; 300's second drop by 44" "lines 'siren settings loaded: 31 (vehicles can use 255 at most); siren setting IDs in use: 31 of 256; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0.' 'siren setting IDs of settings 0 to 30 of 31: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 44, 254, 40, 0, 25, 26, 27'"
check "no hook, nothing written, the code as bytes, the variations' cut IDs lead to theirs, those of 0xFF to none" "log_lacks '\[SirenSettings\] .*are hooked' && log_lacks 'I-E-H1' && has 'siren ids: sites as placed 17 of 17, data as placed' && has 'siren code: loads and stores 0 words 11 bytes, loop bytes, constructors 0 words 3 bytes' && has 'variations 6 of 8'"

echo "== W16 more settings than the lines per kind: 20,001 packs' carcols, the 20,001st line the cap's"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe siload siflood)
check "20,000 lines, the base file's among them; then the cap's, word for word, once" "[ \$(ecount \"${L}(a pack's carcols, |the base carcols brought)\") -eq 20000 ] && [ \$(count \"${L}20000 lines of carcols files written - from now on they are only counted; the summary every minute has the numbers.\") -eq 1 ]"

echo "== W17 the hooks an aid: I-E-H1 not found, or neither function where a function starts - logged, no problem, the part written"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ERROR_POPUP 1; addkey TEST_BREAK_PATTERN I-E-H1; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "not found: the line, word for word; no box; the part written with a word's load, no hook; the measurement announced" "lines 'the siren settings dropped and moved are not counted: I-E-H1 was not found.' && no_box && written && a_word && measured_wide"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe sibyte0)
check "no function start: the line, word for word; the part written, no hook" "lines 'the siren settings dropped and moved are not counted: I-E-H1 at $RVA is not where a function starts: no unwind data, byte before it 00.' && written && a_word"

echo "== W18 the test build's summary again when the numbers change: past 256 IDs, a pack's 6 settings at 12 s"
export TWEAKSV_TEST_SUMMARY_MS=500
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe sipack)
check "two summaries: 24 of 65536 IDs 10 s in, then 30 - and no more" "lines 'siren settings loaded: 24 (vehicles can use 255 at most); siren setting IDs in use: 24 of 65536, 0 of them past 255; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0; settings the game dropped so far: 0, moved: 0.' 'siren settings loaded: 30 (vehicles can use 255 at most); siren setting IDs in use: 30 of 65536, 0 of them past 255; settings past position 255, which no vehicle can reach: 0; IDs given to more than one setting: 0; settings the game dropped so far: 0, moved: 0.' && [ \$(summaries) -eq 2 ]"
unset TWEAKSV_TEST_SUMMARY_MS

echo "== W19 the accessor of the siren settings for another module, whenever the module has looked for its sites (the test build's line)"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the part at work: the 24 at their array, 0x5E8 bytes apart" "written && accessor_gives 24"
fresh_ini; setkey ENABLED 0 SIRENSETTINGS; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the part off, DIAGNOSTIC_MODE=1: the same, nothing written" "accessor_gives 24 && untouched"
measure_ini; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "LARGE_SIREN_IDS=0, DIAGNOSTIC_MODE=1: the same, nothing written" "accessor_gives 24 && untouched"
measure_ini; out=$(ASI=TweaksV_test.asi game full_game.exe siwidened)
check "the measurement refused by another site, I-E-T4 found: the same" "lines 'the siren settings are not measured: the siren setting ID load at $RVA holds 0F B7 2C 0F, not 0F B6 2C 0F (I-E-T1).' && accessor_gives 24"
fresh_ini; addkey TEST_BREAK_PATTERN I-E-D1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the part refused by a pattern after I-E-T4's: the same" "refused 'I-E-D1 was not found.' && accessor_gives 24"
fresh_ini; addkey TEST_BREAK_PATTERN I-E-T1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the part refused by a pattern before I-E-T4's, which it never looks for then: nothing" "refused 'I-E-T1 was not found.' && accessor_nothing"
measure_ini; addkey TEST_BREAK_PATTERN I-E-T4; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "I-E-T4 not found: nothing" "accessor_nothing"
measure_ini; out=$(ASI=TweaksV_test.asi game full_game.exe siload)
check "the game's load after TweaksV has started, the carcols pointer null while the module looks: nothing yet" "accessor_nothing"
measure_ini; out=$(game full_game.exe)
check "the release build has no such line" "log_lacks 'accessor'"

echo
echo "SIREN IDS SCENARIOS: $pass passed, $fail failed"
