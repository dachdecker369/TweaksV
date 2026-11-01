#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The PedProps part (TweaksVPedProps), which ships off (NOTES.md, T-PEDPROPS): as it ships, not a word of it and nothing
# looked for, and with DIAGNOSTIC_MODE=1 the measurement alone. Every other scenario switches it on: the full fake game's
# eleven narrowings widened from 8 to 16 bits, B6 made B7 and nothing else, one line; DRY_RUN; with DIAGNOSTIC_MODE=1
# every pattern, every narrowing with the call in its match, the complete sets through the scanner's FindCallsTo - the per-anchor count called exactly 7 times and
# the metadata lookup exactly 17, the calls that lead elsewhere not counted - and every write, word for word; then the
# measurement: the model info store walked 10 seconds in and again when its numbers change, every ped model's props per
# anchor at the game's offsets, only the first entry of an anchor in a file counted, anchors and files past 255 named -
# raised, not raised in a dry run, past even 65535 - the lines capped, a bound broken and memory that cannot be read ending
# a pass with a line. Each check failing on its own refuses the whole job, a problem word for word, nothing written: each
# pattern missing, A-E-G1 once and three times, A-E-T1 and A-E-C1 twice, a narrowing another mod widened, an 8th and an 18th
# call and one of each redirected away, a call into the count's loop, a lookup call to another function, a call another mod
# hooked, the write refused by Windows' dynamic code policy; a site of the measurement's own not as expected - A-E-C2 and
# A-E-S1 twice among them - only stops the measurement, logged, the part applied; the plain game. Every page's protection
# its own, always.
# Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata
PEDPROPS_ON=1

ASIS='pedprops: sites as placed, data as placed |'
WIDENED='pedprops: sites widened, data as placed |'
RAISED='\[PedProps\] The ped props. limit is raised from 255 to 65535 per anchor (11 sites).$'
DRYLINE='\[PedProps\] DRY_RUN: would widen the 11 narrowings of the ped props from 8 to 16 bits and raise their limit from 255 to 65535 per anchor.$'
NOT_RAISED='\[PedProps\] PROBLEM: The ped props. limit is not raised this session: '
STORE='(the model info store.s 60010 indices, its stride 16)'
SUMMARY="\[PedProps\] DIAGNOSTIC: ped props: 43 ped models with variation data in 99 files $STORE; the most props on one anchor: 212 (mp_m_freemode_01, anchor 0, from 30 files, at most 38 in one); no anchor and no file past 255.\$"
ANNOUNCED='\[PedProps\] DIAGNOSTIC: the model info store at RVA 0x[0-9A-F]* (A-E-S1): the props per anchor of its ped models are logged after 10 seconds, then every minute while they change.$'
# applied: the part's line, word for word, the eleven widened and nothing else, every page's protection its own, no problem
applied() { log_line "$RAISED" && has "$WIDENED" && has 'protect: all restored (RX)' && log_lacks '\[PedProps\] PROBLEM'; }
# refused WHY: the part's problem, word for word, its only one, in the one start-up summary; nothing written - the sites
# as placed, no line of the part's raise, every page's protection its own. Every check here and below that matches or
# counts a line of the log matches the part's own lines alone, so that the other parts' lines change nothing in this suite
refused() { log_line "$NOT_RAISED$1\$" && [ "$(count '\[PedProps\] PROBLEM')" -eq 1 ] && [ "$(count 'Start-up summary not shown (ERROR_POPUP=0): ')" -eq 1 ] && has "$ASIS" && log_lacks 'ped props. limit is raised' && has 'protect: all restored (RX)'; }
# not_measured WHY: the measurement's line, word for word, and nothing more of it: not announced, no summary
not_measured() { log_line "\[PedProps\] DIAGNOSTIC: the ped props are not measured: $1\$" && log_lacks '\[PedProps\] DIAGNOSTIC: the model info store at' && log_lacks '\[PedProps\] DIAGNOSTIC: ped props: '; }
# not_counted WHY: the measurement announced, its pass ended with this line and no summary; the part applied
not_counted() { log_line "\[PedProps\] DIAGNOSTIC: the ped props are not counted: $1\$" && log_line "$ANNOUNCED" && log_lacks 'DIAGNOSTIC: ped props: ' && applied; }
# targets KIND: the targets of the KIND call lines, the patterns' and the scan's, each once; count_start: the per-anchor
# count's own RVA; first_line PATTERN: the number of the first line that matches; rvas PATTERN: the RVAs PATTERN's \(...\)
# takes from the module's DIAGNOSTIC lines, sorted
targets() { tr -d '\r' < TweaksV.log | sed -n "s/.*\] DIAGNOSTIC: $1 call at RVA 0x[0-9A-F]* -> \(RVA 0x[0-9A-F]*\) (.*/\1/p" | sort -u; }
count_start() { tr -d '\r' < TweaksV.log | sed -n 's/.*\] DIAGNOSTIC: per-anchor count at \(RVA 0x[0-9A-F]*\):.*/\1/p'; }
first_line() { tr -d '\r' < TweaksV.log | grep -n -- "$1" | head -n 1 | cut -d: -f1; }
rvas() { tr -d '\r' < TweaksV.log | sed -n "s/.*\[PedProps\] DIAGNOSTIC: $1.*/\1/p" | sort; }
# the sites, each line word for word but its RVA - every narrowing with its bytes, every call; then the scans and the layout
sites_logged() {
	local n
	for n in "0F B6 C3 (an anchor.s total, A-E-T1)" "0F B6 C0 (an anchor.s total, A-E-T2)" "0F B6 C0 (an anchor.s total, A-E-T3)" \
		"0F B6 C0 (an anchor.s total, A-E-T4)" "0F B6 C0 (an anchor.s total, A-E-T5)" "0F B6 E8 (a worn prop.s index, A-E-G3)" \
		"44 0F B6 C0 (a worn prop.s index, A-E-G4)" "44 0F B6 C0 (a worn prop.s index, A-E-G5)" "44 0F B6 F7 (a worn prop.s index, A-E-G6)"; do
		[ "$(count "\[PedProps\] DIAGNOSTIC: narrowing at RVA 0x[0-9A-F]* holds $n")" -eq 1 ] || return 1
	done
	[ "$(count '\[PedProps\] DIAGNOSTIC: narrowing at RVA 0x[0-9A-F]* holds 44 0F B6 C6 (a worn prop.s index, A-E-G1)')" -eq 2 ] &&
	[ "$(count '\[PedProps\] DIAGNOSTIC: texture count call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (A-E-T1)')" -eq 1 ] &&
	[ "$(count '\[PedProps\] DIAGNOSTIC: per-anchor count call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (A-E-T[2-5])')" -eq 4 ] &&
	[ "$(count '\[PedProps\] DIAGNOSTIC: metadata lookup call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (A-E-G[13-6])')" -eq 6 ] &&
	[ "$(count '\[PedProps\] DIAGNOSTIC: per-anchor count call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (call scan)')" -eq 7 ] &&
	[ "$(count '\[PedProps\] DIAGNOSTIC: metadata lookup call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (call scan)')" -eq 17 ] &&
	log_line '\[PedProps\] DIAGNOSTIC: per-anchor count at RVA 0x[0-9A-F]*: a model info.s variation collection at +0x240, its count of variation files at +0x8 (A-E-C1)$' &&
	log_line '\[PedProps\] DIAGNOSTIC: its loop at RVA 0x[0-9A-F]*: a variation file.s anchors at +0x58, their count at +0x60, entries of 0x18 bytes, an entry.s props at +0x8 (A-E-C2)$' &&
	log_line '\[PedProps\] DIAGNOSTIC: model info store at RVA 0x[0-9A-F]*: its count at RVA 0x[0-9A-F]*, its bitmap.s pointer at RVA 0x[0-9A-F]*, its array.s pointer at RVA 0x[0-9A-F]*, its stride at RVA 0x[0-9A-F]*; a model info.s type in the low 5 bits of +0x9D, 6 for a ped (A-E-S1)$'
}
# the eleven writes, each word for word but its RVA, at the RVAs of the eleven narrowings and nowhere else
WRITE='\[PedProps\] DIAGNOSTIC: narrowing of '
writes_logged() {
	local n
	for n in "an anchor.s total (A-E-T1) at RVA 0x[0-9A-F]*: 0F B6 C3 -> 0F B7 C3" "an anchor.s total (A-E-T2) at RVA 0x[0-9A-F]*: 0F B6 C0 -> 0F B7 C0" \
		"an anchor.s total (A-E-T3) at RVA 0x[0-9A-F]*: 0F B6 C0 -> 0F B7 C0" "an anchor.s total (A-E-T4) at RVA 0x[0-9A-F]*: 0F B6 C0 -> 0F B7 C0" \
		"an anchor.s total (A-E-T5) at RVA 0x[0-9A-F]*: 0F B6 C0 -> 0F B7 C0" "a worn prop.s index (A-E-G3) at RVA 0x[0-9A-F]*: 0F B6 E8 -> 0F B7 E8" \
		"a worn prop.s index (A-E-G4) at RVA 0x[0-9A-F]*: 44 0F B6 C0 -> 44 0F B7 C0" "a worn prop.s index (A-E-G5) at RVA 0x[0-9A-F]*: 44 0F B6 C0 -> 44 0F B7 C0" \
		"a worn prop.s index (A-E-G6) at RVA 0x[0-9A-F]*: 44 0F B6 F7 -> 44 0F B7 F7"; do
		[ "$(count "$WRITE$n")" -eq 1 ] || return 1
	done
	[ "$(count "${WRITE}a worn prop.s index (A-E-G1) at RVA 0x[0-9A-F]*: 44 0F B6 C6 -> 44 0F B7 C6")" -eq 2 ] && [ "$(count "$WRITE")" -eq 11 ] &&
	[ -n "$(rvas 'narrowing at RVA \(0x[0-9A-F]*\) holds ')" ] && [ "$(rvas 'narrowing at RVA \(0x[0-9A-F]*\) holds ')" = "$(rvas 'narrowing of .* at RVA \(0x[0-9A-F]*\): ')" ]
}
# each kind of call leads to one function, the per-anchor count's to its start
one_target_each() { [ "$(targets 'per-anchor count' | wc -l)" -eq 1 ] && [ "$(targets 'per-anchor count')" = "$(count_start)" ] && [ "$(targets 'metadata lookup' | wc -l)" -eq 1 ]; }

echo "== PP1 the part on: the eleven narrowings widened, one line"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "the part's line, word for word, its only line" "log_line '$RAISED' && [ \$(count '\[PedProps\]') -eq 1 ]"
check "the eleven narrowings widened, B6 made B7 and nothing else of the sites or the data; every page's protection its own; no problem" "applied"
check "no line of its patterns: found, they say nothing without DIAGNOSTIC_MODE, and the measurement's are not looked for" "log_lacks 'A-E-'"

echo "== PP2 [PEDPROPS] ENABLED=0, with DIAGNOSTIC_MODE=1: the measurement alone, nothing of the job looked for, nothing widened"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey ENABLED 0 PEDPROPS; out=$(game full_game.exe ppmany wait)
check "the header says PedProps off; none of the job's patterns looked for, no line of a narrowing, a write or a raise" "log_has 'Features: .*, PedProps off, Resources' && log_lacks 'A-E-[TG]' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing' && log_lacks 'ped props. limit is raised'"
check "the measurement's three patterns found once each, its sites logged, announced word for word" "[ \$(grep -c -E 'Pattern A-E-(C[12]|S1) found: .* 1 match,' TweaksV.log) -eq 3 ] && log_line '\[PedProps\] DIAGNOSTIC: per-anchor count at RVA 0x[0-9A-F]*: ' && log_line '$ANNOUNCED'"
check "each anchor past 255 refused from (total mod 256) on, word for word: nothing is raised" "log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_m_freemode_01, anchor 0: 302 props from 45 files - past the game.s own limit of 255: the game.s own checks refuse its props from index 46 on.$'"
check "the sites and the data as placed, every page's protection its own, no problem" "has '$ASIS' && has 'protect: all restored (RX)' && log_lacks '\[PedProps\] PROBLEM'"

echo "== PP3 DRY_RUN=1: everything found and checked, nothing written"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe)
check "the dry run's line, word for word; no line of a raise, no write's, no problem" "log_line '$DRYLINE' && log_lacks 'ped props. limit is raised' && log_lacks '\[PedProps\] DRY_RUN: would patch' && log_lacks '\[PedProps\] PROBLEM'"
check "nothing written: the sites as placed, every page's protection its own" "has '$ASIS' && has 'protect: all restored (RX)'"

echo "== PP4 DIAGNOSTIC_MODE=1: every pattern, every site, the complete sets, every write, then the measurement 10 s in"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe wait)
check "12 patterns found once each, A-E-G1 twice with a line per match" "[ \$(grep -c -E 'Pattern A-E-(T[1-5]|G[3-6]|C[12]|S1) found: .* 1 match,' TweaksV.log) -eq 12 ] && log_has 'Pattern A-E-G1 found: .* 2 matches,' && [ \$(count 'A-E-G1 match [12] of 2 at RVA') -eq 2 ]"
check "the eleven narrowings, each with its bytes, and the calls in their matches; the layout's sites and the store's four globals" "sites_logged"
check "the complete sets: exactly 7 calls of the per-anchor count and 17 of the metadata lookup - the calls that lead elsewhere not counted - each kind to one function, the count's to its start" "one_target_each"
check "the eleven writes, word for word with their old and new bytes, at the RVAs of the eleven narrowings" "writes_logged"
check "the part's line after the writes, then the measurement announced, word for word" "log_line '$RAISED' && log_line '$ANNOUNCED' && [ \$(first_line '\[PedProps\] DIAGNOSTIC: narrowing of ') -lt \$(first_line 'ped props. limit is raised') ] && [ \$(first_line 'ped props. limit is raised') -lt \$(first_line '\[PedProps\] DIAGNOSTIC: the model info store at') ]"
check "the summary 10 s in, word for word: 43 ped models of 60010 indices - the other models, the ped without variation data, the null model info, the bit that is clear and the index past the count not counted - the most on one anchor 212, the second entry of anchor 0 in a file not counted" "log_line \"$SUMMARY\" && [ \$(count 'DIAGNOSTIC: ped props: ') -eq 1 ]"
check "no line past 255, no problem; the eleven widened and nothing else, every page's protection its own" "log_lacks '\[PedProps\] DIAGNOSTIC: ped props: .*past the game.s own limit' && applied"

echo "== PP5 two anchors and a file past 255, the limit raised: each named, word for word"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppmany wait)
check "the summary: the most on one anchor 356, two anchors and one file past 255" "log_line \"\[PedProps\] DIAGNOSTIC: ped props: 43 ped models with variation data in 115 files $STORE; the most props on one anchor: 356 (mp_f_freemode_01, anchor 2, from 29 files, at most 270 in one); past 255: 2 anchors and 1 file, each in a line of its own.\\\$\""
check "mp_m_freemode_01's anchor 0: 302 props, the limit raised this session" "log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_m_freemode_01, anchor 0: 302 props from 45 files - past the game.s own limit of 255, raised to 65535 this session.$'"
check "mp_f_freemode_01's file 29 of 29 with 270 props on anchor 2: unusable with or without TweaksV" "log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_f_freemode_01, anchor 2: its file 29 of 29 holds 270 props - a file holds 255 at most, its props past 255 stay unusable with or without TweaksV.$'"
check "mp_f_freemode_01's anchor 2: 356 props, the limit raised this session" "log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_f_freemode_01, anchor 2: 356 props from 29 files - past the game.s own limit of 255, raised to 65535 this session.$'"
check "in that order, after the summary; the part applied" "[ \$(first_line 'DIAGNOSTIC: ped props: 43') -lt \$(first_line 'ped props: mp_m_freemode_01, anchor 0: 302') ] && [ \$(first_line 'ped props: mp_m_freemode_01, anchor 0: 302') -lt \$(first_line 'ped props: mp_f_freemode_01, anchor 2: its file 29 of 29') ] && [ \$(first_line 'ped props: mp_f_freemode_01, anchor 2: its file 29 of 29') -lt \$(first_line 'ped props: mp_f_freemode_01, anchor 2: 356 props') ] && applied"

echo "== PP6 the same with DRY_RUN=1: the game's own checks still refuse"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe ppmany wait)
check "each anchor past 255 refused from (total mod 256) on, word for word" "log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_m_freemode_01, anchor 0: 302 props from 45 files - past the game.s own limit of 255: the game.s own checks refuse its props from index 46 on.$' && log_line '\[PedProps\] DIAGNOSTIC: ped props: mp_f_freemode_01, anchor 2: 356 props from 29 files - past the game.s own limit of 255: the game.s own checks refuse its props from index 100 on.$'"
check "the dry run's line, nothing raised, nothing written, no problem" "log_line '$DRYLINE' && log_lacks '\[PedProps\] .*raised to 65535' && has '$ASIS' && log_lacks '\[PedProps\] PROBLEM'"

echo "== PP7 a ped with 258 files of 255 props on one anchor: past even the 65535 it is raised to"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe pphuge wait)
check "the summary: 65790 on one anchor from 258 files, one anchor past 255 and no file" "log_line \"\[PedProps\] DIAGNOSTIC: ped props: 44 ped models with variation data in 357 files $STORE; the most props on one anchor: 65790 (model 0x2D000000, anchor 0, from 258 files, at most 255 in one); past 255: 1 anchor and 0 files, each in a line of its own.\\\$\""
check "its line: refused from (total mod 65536) on, word for word; the part applied" "log_line '\[PedProps\] DIAGNOSTIC: ped props: model 0x2D000000, anchor 0: 65790 props from 258 files - past the limit of 65535 it is raised to this session: the game.s own checks refuse its props from index 254 on.$' && applied"

echo "== PP8 the test build, TWEAKSV_TEST_SUMMARY_MS=2000: the summary 10 s in and again when the numbers change - 5 more files at 12 s - and not while they stay; an entry of anchor 13, said once"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=2000 ASI=TweaksV_test.asi game full_game.exe ppgrow ppunknown wait)
check "the switch said once for this measurement" "[ \$(count 'TEST BUILD: the ped props summary comes every 2000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=2000).') -eq 1 ]"
check "the first summary, word for word: 212 on anchor 0 from 30 files; the entry of anchor 13 counted apart" "log_line \"\[PedProps\] DIAGNOSTIC: ped props: 43 ped models with variation data in 99 files $STORE; the most props on one anchor: 212 (mp_m_freemode_01, anchor 0, from 30 files, at most 38 in one); 1 anchor entry names no anchor of 0..12; no anchor and no file past 255.\\\$\""
check "the second, once the numbers changed: 242 from 35 files of 104" "log_line \"\[PedProps\] DIAGNOSTIC: ped props: 43 ped models with variation data in 104 files $STORE; the most props on one anchor: 242 (mp_m_freemode_01, anchor 0, from 35 files, at most 38 in one); 1 anchor entry names no anchor of 0..12; no anchor and no file past 255.\\\$\""
check "two summaries, in that order - none while the numbers stayed, over 11 s of passes every 2 s" "[ \$(count 'DIAGNOSTIC: ped props: 43') -eq 2 ] && [ \$(first_line 'ped props: 43 ped models with variation data in 99 files') -lt \$(first_line 'ped props: 43 ped models with variation data in 104 files') ]"
check "the entry of anchor 13, word for word, logged once over both summaries" "[ \$(count 'names anchor 13') -eq 1 ] && log_line '\[PedProps\] DIAGNOSTIC: ped props: an anchor entry of mp_m_freemode_01 names anchor 13, not one of 0..12 - such entries are counted apart (logged once).$'"
check "the part applied; the data as the fake grew it" "applied"

echo "== PP9 10,001 more peds, each with a file of 300 props on one anchor: 20,000 lines past 255, then only counted"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppflood wait)
check "the summary has the numbers: 10001 anchors and 10001 files past 255" "log_line \"\[PedProps\] DIAGNOSTIC: ped props: 10044 ped models with variation data in 10100 files $STORE; the most props on one anchor: 300 (model 0x2C000000, anchor 1, from 1 file, at most 300 in one); past 255: 10001 anchors and 10001 files, each in a line of its own.\\\$\""
check "a model by its hash: its file, then its anchor, word for word" "log_line '\[PedProps\] DIAGNOSTIC: ped props: model 0x2C000000, anchor 1: its file 1 of 1 holds 300 props - a file holds 255 at most, its props past 255 stay unusable with or without TweaksV.$' && log_line '\[PedProps\] DIAGNOSTIC: ped props: model 0x2C000000, anchor 1: 300 props from 1 file - past the game.s own limit of 255, raised to 65535 this session.$'"
check "20000 lines, the 10000th model's the last, then the cap line once" "[ \$(count '\[PedProps\] DIAGNOSTIC: ped props: model 0x2C[0-9A-F]*, anchor 1: ') -eq 20000 ] && log_has 'model 0x2C00270F, anchor 1: 300 props' && log_lacks 'model 0x2C002710' && [ \$(count 'DIAGNOSTIC: 20000 lines of ped props past 255 written - from now on they are only counted; the summary every minute has the numbers.') -eq 1 ]"
check "the part applied" "applied"

echo "== PP10 the store's count holds 2,000,000: the pass ends with a line, no summary"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppbadcount wait)
check "the line, word for word; no summary, the part applied" "not_counted 'the model info store.s count, 2000000, is not plausible.'"

echo "== PP11 the store's stride holds 12: the pass ends with a line, no summary"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppbadstride wait)
check "the line, word for word; no summary, the part applied" "not_counted 'the model info store.s stride, 12, is not plausible.'"

echo "== PP12 a variation file in memory that cannot be read: the access caught, the pass ended with a line, no summary"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppunreadable wait)
check "the line names the model and the file; no summary, the part applied" "not_counted 'a variation file of mp_f_freemode_01 at 0x[0-9A-F]* (outside the game) cannot be read.'"

echo "== PP13 TEST_BREAK_PATTERN=A-E-T1 (the test build), DIAGNOSTIC_MODE=1: refused, the measurement all the same"
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN A-E-T1; out=$(ASI=TweaksV_test.asi game full_game.exe wait)
check "the pattern treated as not found, its widened twin looked for and missing; every other pattern still looked for" "log_has 'TEST BUILD: pattern A-E-T1 is treated as not found (TEST_BREAK_PATTERN).' && log_has 'Pattern A-E-T1 (widened) not found: 0F B7 C3 44 39 C0 7E ? E8 ? ? ? ? 0F B6 C0 44 39 F0 0F 9F C0' && [ \$(grep -c -E 'Pattern A-E-(T[2-5]|G[13-6]|C[12]|S1) found: ' TweaksV.log) -eq 12 ]"
check "the problem, word for word, the only one; no site checked, nothing written" "refused 'A-E-T1 was not found.' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing at'"
check "the measurement all the same: its sites, the announcement and the summary 10 s in" "log_line '$ANNOUNCED' && log_line \"$SUMMARY\""

echo "== PP14 TEST_BREAK_PATTERN=A-E-G1, the pattern that must match twice"
fresh_ini; addkey TEST_BREAK_PATTERN A-E-G1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "refused, word for word; its widened twin looked for and missing" "refused 'A-E-G1 was not found.' && log_has 'Pattern A-E-G1 (widened) not found: '"

echo "== PP15 TEST_BREAK_PATTERN=A-E-C1, the per-anchor count's start, DIAGNOSTIC_MODE=1"
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN A-E-C1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "refused, word for word; no narrowing checked; no widened twin looked for, every narrowing found" "refused 'A-E-C1 was not found.' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing at' && log_lacks 'Pattern A-E-[TG][0-9] (widened)'"
check "nor measured, its layout's first pattern missing" "not_measured 'A-E-C1 was not found.'"

echo "== PP16 TEST_BREAK_PATTERN=A-E-S1, the measurement's own pattern: the part applied, the measurement stopped"
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN A-E-S1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the measurement's line, word for word; the part applied, its writes logged" "not_measured 'A-E-S1 was not found.' && applied && writes_logged"

echo "== PP17 TEST_BREAK_PATTERN=A-E-C2, the measurement's other pattern"
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_BREAK_PATTERN A-E-C2; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "the measurement's line, word for word, A-E-S1 still looked for; the part applied" "not_measured 'A-E-C2 was not found.' && log_has 'Pattern A-E-S1 found: ' && applied"

echo "== PP18 A-E-G1 only once (a changed game)"
fresh_ini; out=$(game full_game.exe ppg1once)
check "refused, word for word; its widened twin looked for and missing" "refused 'the game code holds A-E-G1 1 time, not the expected 2.' && log_has 'Pattern A-E-G1 (widened) not found: '"

echo "== PP19 A-E-G1 three times (a changed game)"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppg1thrice)
check "the scanner's 3 matches; refused, word for word; no twin looked for" "log_has 'Pattern A-E-G1 found: .* 3 matches,' && refused 'the game code holds A-E-G1 3 or more times, not the expected 2.' && log_lacks 'Pattern A-E-[TG][0-9] (widened)'"

echo "== PP20 A-E-T3's narrowing widened already (another mod): its pattern missing, its widened twin found"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppwidened)
t3=$(tr -d '\r' < TweaksV.log | sed -n 's/.*Pattern A-E-T3 (widened) found: .* -> RVA 0x\([0-9A-F]*\), 1 match, .*/\1/p')
site=$( [ -n "$t3" ] && printf 'RVA 0x%X' $((0x$t3 + 10)) )
check "the scanner's miss, then the twin found once, word for word" "log_has 'Pattern A-E-T3 not found: 48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 C7 0F 4C C7 83 FF FF' && log_has 'Pattern A-E-T3 (widened) found: 48 8B 09 31 D2 E8 ? ? ? ? 0F B7 C0 39 C7 0F 4C C7 83 FF FF -> RVA 0x[0-9A-F]*, 1 match, ' && [ -n '$site' ]"
check "refused, the reason naming the site at the twin's +10 and both bytes, word for word; nothing written" "refused 'the narrowing at $site holds 0F B7 C0, not 0F B6 C0 (another mod may have widened it already) (A-E-T3).'"

echo "== PP21 A-E-G1's second copy widened already: the pattern found once, its twin once"
fresh_ini; out=$(game full_game.exe ppg1widened)
check "refused, the reason naming the widened copy, word for word; nothing written" "refused 'the narrowing at RVA 0x[0-9A-F]* holds 44 0F B7 C6, not 44 0F B6 C6 (another mod may have widened it already) (A-E-G1).'"

echo "== PP22 an 8th call of the per-anchor count (a changed game): the scan finds it, refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppcountcall)
check "8 calls logged; refused, word for word" "[ \$(count 'per-anchor count call at RVA .* (call scan)') -eq 8 ] && refused 'the game code calls the per-anchor count at RVA 0x[0-9A-F]* in 8 or more places, not the expected 7 (another mod may have redirected some of them, or the game has changed).'"

echo "== PP23 an 18th call of the metadata lookup (a changed game): the scan finds it, refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe pplookupcall)
check "the count's 7 calls, then 18 of the lookup logged; refused, word for word" "[ \$(count 'per-anchor count call at RVA .* (call scan)') -eq 7 ] && [ \$(count 'metadata lookup call at RVA .* (call scan)') -eq 18 ] && refused 'the game code calls the metadata lookup at RVA 0x[0-9A-F]* in 18 or more places, not the expected 17 (another mod may have redirected some of them, or the game has changed).'"

echo "== PP24 A-E-T3's call leads into the per-anchor count's loop, not to its start: a site not as expected"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppcountsplit)
check "T2's call passed, T3's refusal word for word; T4's never looked at" "log_has 'per-anchor count call at RVA .* (A-E-T2)' && log_lacks 'per-anchor count call at RVA .* (A-E-T4)' && refused 'the per-anchor count call at RVA 0x[0-9A-F]* leads to RVA 0x[0-9A-F]*, not to the per-anchor count at RVA 0x[0-9A-F]* (A-E-C1) (A-E-T3).'"

echo "== PP25 A-E-G4's call leads to another function than the other metadata lookup calls"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe pplookupsplit)
texture=$(tr -d '\r' < TweaksV.log | sed -n 's/.*texture count call at RVA 0x[0-9A-F]* -> \(RVA 0x[0-9A-F]*\) (A-E-T1).*/\1/p')
check "the reason names both functions, the second the texture count's, word for word" "[ -n '$texture' ] && refused 'the metadata lookup calls do not lead to the same function: RVA 0x[0-9A-F]* and $texture (A-E-G4).'"

echo "== PP26 A-E-T1's call leads outside the game (another mod hooked it)"
fresh_ini; out=$(game full_game.exe pphooked)
check "refused, word for word" "refused 'the texture count call at RVA 0x[0-9A-F]* is not a call into the game (another mod may have hooked it already) (A-E-T1).'"

echo "== PP27 A-E-S1's reference to the store's array pointer leads outside the game"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppstoreout)
check "the measurement's line, word for word; the part applied" "not_measured 'the model info store.s array.s pointer at 0x[0-9A-F]* (outside the game) (A-E-S1) is not inside the game.' && applied"

echo "== PP28 the plain game, DIAGNOSTIC_MODE=1: every pattern missing, the part's problem, nothing measured"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "the 13 patterns each named by the scanner, and A-E-T1's widened twin alone" "[ \$(grep -c -E 'Pattern A-E-(T[1-5]|G[13-6]|C[12]|S1) not found: ' TweaksV.log) -eq 13 ] && [ \$(count 'Pattern A-E-[TG][0-9] (widened) not found') -eq 1 ] && log_has 'Pattern A-E-T1 (widened) not found: '"
check "the part's problem, word for word, its only one; the measurement's line names A-E-C1" "log_line '${NOT_RAISED}A-E-T1 was not found.\$' && [ \$(count '\[PedProps\] PROBLEM') -eq 1 ] && not_measured 'A-E-C1 was not found.'"

echo "== PP29 DRY_RUN=1 with DIAGNOSTIC_MODE=1: every site checked and logged, nothing written, the measurement alike"
fresh_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(game full_game.exe wait)
check "every site logged and checked, the complete sets, then the dry run's line instead of the writes" "sites_logged && one_target_each && log_line '$DRYLINE' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing of '"
check "the measurement announced and its summary 10 s in, word for word; nothing written, no problem" "log_line '$ANNOUNCED' && log_line \"$SUMMARY\" && has '$ASIS' && has 'protect: all restored (RX)' && log_lacks '\[PedProps\] PROBLEM'"

echo "== PP30 Windows' dynamic code policy: the eleven writes cannot be made, the job refused, nothing written"
fresh_ini; out=$(game full_game.exe nodyncode)
check "the patch module's line under the part's tag, then its problem, word for word; the sites as placed, every page's protection its own" "log_has '\[PedProps\] The game code at RVA 0x[0-9A-F]* could not be made writable (' && refused 'the game code could not be made writable (details in TweaksV.log).'"

echo "== PP31 one of the per-anchor count's 7 calls redirected by another mod: 6 found, refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppcountshort)
check "6 calls logged; refused, word for word" "[ \$(count 'per-anchor count call at RVA .* (call scan)') -eq 6 ] && refused 'the game code calls the per-anchor count at RVA 0x[0-9A-F]* in 6 places, not the expected 7 (another mod may have redirected some of them, or the game has changed).'"

echo "== PP32 one of the metadata lookup's 17 calls redirected by another mod: 16 found, refused"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe pplookupshort)
check "the count's 7 calls, then 16 of the lookup logged; refused, word for word" "[ \$(count 'per-anchor count call at RVA .* (call scan)') -eq 7 ] && [ \$(count 'metadata lookup call at RVA .* (call scan)') -eq 16 ] && refused 'the game code calls the metadata lookup at RVA 0x[0-9A-F]* in 16 places, not the expected 17 (another mod may have redirected some of them, or the game has changed).'"

echo "== PP33 A-E-T1 twice, a narrowing's pattern that must match once (a changed game)"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppt1twice)
check "the scanner's 2 matches; refused, named so, not as missing, word for word; no twin looked for, no narrowing checked" "log_has 'Pattern A-E-T1 found: .* 2 matches,' && refused 'the game code holds A-E-T1 2 or more times, not the expected 1.' && log_lacks 'A-E-T1 was not found' && log_lacks 'Pattern A-E-[TG][0-9] (widened)' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing at'"
check "the measurement all the same, its own sites found" "log_line '$ANNOUNCED'"

echo "== PP34 A-E-C1 twice, the per-anchor count's start: the job's reason and the measurement's"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppc1twice)
check "the scanner's 2 matches; refused, named so, not as missing, word for word; no twin looked for, no narrowing checked" "log_has 'Pattern A-E-C1 found: .* 2 matches,' && refused 'the game code holds A-E-C1 2 or more times, not the expected 1.' && log_lacks 'A-E-C1 was not found' && log_lacks 'Pattern A-E-[TG][0-9] (widened)' && log_lacks '\[PedProps\] DIAGNOSTIC: narrowing at'"
check "nor measured, its layout's first pattern named in the same words" "not_measured 'the game code holds A-E-C1 2 or more times, not the expected 1.'"

echo "== PP35 A-E-C2 twice, the measurement's own pattern: the part applied, the measurement stopped"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe ppc2twice)
check "the scanner's 2 matches; the measurement's line, word for word, named so, not as missing, A-E-S1 still looked for; the part applied" "log_has 'Pattern A-E-C2 found: .* 2 matches,' && not_measured 'the game code holds A-E-C2 2 or more times, not the expected 1.' && log_lacks 'A-E-C2 was not found' && log_has 'Pattern A-E-S1 found: ' && applied"

echo "== PP36 A-E-S1 twice, the measurement's other pattern"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe pps1twice)
check "the scanner's 2 matches; the measurement's line, word for word, named so, not as missing; the part applied" "log_has 'Pattern A-E-S1 found: .* 2 matches,' && not_measured 'the game code holds A-E-S1 2 or more times, not the expected 1.' && log_lacks 'A-E-S1 was not found' && applied"

echo "== PP37 as the part ships: base.ini's default, off, without DIAGNOSTIC_MODE: not a word of it, nothing looked for"
PEDPROPS_ON=; fresh_ini; out=$(game full_game.exe); PEDPROPS_ON=1
check "the header says PedProps off; no line of the part, none of its patterns looked for" "log_has 'Features: .*, PedProps off, Resources' && log_lacks '\[PedProps\]' && log_lacks 'A-E-'"
check "the sites and the data as placed, every page's protection its own, no problem and so no box" "has '$ASIS' && has 'protect: all restored (RX)' && log_lacks 'PROBLEM' && log_lacks 'Start-up summary'"

echo "== PP38 no TweaksV.ini: the file TweaksV writes has the part off and says it is experimental, and the part does nothing"
rm -f TweaksV.ini; out=$(game full_game.exe)
check "[PEDPROPS] in the new file: ENABLED=0 with its comment, word for word" "[ \"\$(tr -d '\r' < TweaksV.ini | sed -n '/^\[PEDPROPS\]/{n;p}')\" = 'ENABLED=0                   //EXPERIMENTAL & UNTESTED - USE AT YOUR OWN RISK!!!' ]"
check "the part off in the header, no line of it, the sites as placed, no problem" "log_has 'Features: .*, PedProps off, Resources' && log_lacks '\[PedProps\]' && has '$ASIS' && log_lacks 'PROBLEM'"

echo
echo "PED PROPS SCENARIOS: $pass passed, $fail failed"
