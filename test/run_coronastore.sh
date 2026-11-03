#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The corona store (TweaksVCoronaStore), the second half of the LightCapacity part: at the game's own 1088 nothing looked for or written; above it the fake
# game's sites found and checked, the 13 writes and nothing else, the new store 16-aligned and of N records and 0x180 bytes; the general add, the LOD lights'
# job and the copy run through the moved store, up to N and past it, from two threads, the count exact; the clamp's trap shown at the game's own clamp and
# gone with the raise; the counters of the four jumps and the batch with DIAGNOSTIC_MODE, a stub that cannot be made logged and the raise going on; DRY_RUN;
# each pattern missing, each check failing on its own, a site another mod changed and Windows refusing - each the part's problem, nothing written. It pins the
# store's own reasons and lines, not what the part says around them, whichever part that is: the accessor through the test build's export. Fake LOCALAPPDATA:
# the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
CHECKS=111
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata
STORES_ON=1

# The store's lines, word for word (an apostrophe as any character). raise_ini [N]: the part on, CORONA_STORE at N. The fake's report: its sites and jumps, what it ran, the accessor.
raise_ini() { fresh_ini; setkey CORONA_STORE "${1:-4096}" LIGHTCAPACITY; }
cc_line() { log_line "\[LightCapacity\] $1\$"; }
cc_lacks() { log_lacks "\[LightCapacity\] .*$1"; }
PLACED='cc sites: as placed | cc jumps: as placed |'
moved() { echo "cc sites: the store moved, $1 coronas, the 13 writes and nothing else, its memory RW, 16-aligned, $(( $1 * 48 + 384 )) bytes at least |"; }
ran() {		# ROOM WORDS: what ccrun says at a room, the old store's words
	local each=$(( $1 / 4 * 3 + $1 % 4 * 3 / 4 ))
	echo "cc run: parameters name the store and its count; adds $1 added, $(( 2 * each - $1 )) refused, one slot each, the old store $2, count $1; batch at $(( $1 - 100 )) brought 300 placed 100, count $1, its last record in the last slot; its own limit passes 1087, refuses 1088; copy of $1 asked $(( $1 * 48 + 384 )) bytes, as the store; copy of 0 asked 0 |"
}
now() { echo "cc now: room $1, counted $2, refused $3, lod own $4, lod lights $5 |"; }
NOW_GAME=$(now 1088 0 0 0 0)
# An RVA from the fake's report: rva i1; rva_plus NAME N: that RVA plus N
rva() { sed -n 's/.*cc rvas: \([^|]*\) |.*/, \1/p' <<< "$out" | sed -n "s/.*, $1 \(0x[0-9A-F]*\).*/\1/p"; }
rva_plus() { printf '0x%X' $(( $(rva "$1") + $2 )); }
# in_order RVA...: "RVA 0x1, RVA 0x2 and RVA 0x3", in address order, as a scan finds them, since the linker lays the fake's sites out as it likes
in_order() {
	local rvas=($(for a in "$@"; do printf '%d\n' "$a"; done | sort -n)) text="" i
	for ((i = 0; i < ${#rvas[@]}; i++)); do
		[ "$i" = 0 ] || { [ "$i" = $(( ${#rvas[@]} - 1 )) ] && text+=" and " || text+=", "; }
		text+=$(printf 'RVA 0x%X' "${rvas[i]}")
	done
	echo "$text"
}
NOT_RAISED='PROBLEM: The light stores are not raised this session:'
NOT_COUNTED='DIAGNOSTIC: the corona store.s refused coronas are not counted:'
REDIRECTED='DIAGNOSTIC: the four jumps when the corona store is full and the LOD lights. batch at RVA 0x[0-9A-F]* (G-E-I8) are redirected to counting stubs once the part.s one write is done.'
HEX='[0-9A-F][0-9A-F]'
ANY4="$HEX $HEX $HEX $HEX"

echo "== C1 [LIGHTCAPACITY] off, CORONA_STORE at 4096, DIAGNOSTIC_MODE=1: nothing of it looked for, nothing written"
STORES_ON= fresh_ini; setkey CORONA_STORE 4096 LIGHTCAPACITY; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "no pattern of the store looked for, no line of it; its sites and jumps as placed; the accessor says the game's own" "log_lacks 'G-E-[IJ]' && cc_lacks 'new corona store' && has '$PLACED' && has '$NOW_GAME'"

echo "== C2 the part on at the game's own 1088, DIAGNOSTIC_MODE=1: nothing of it looked for or written, and the fake's own code as the game's"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "no pattern of the store looked for, no line of it; its sites and jumps as placed" "log_lacks 'G-E-[IJ]' && cc_lacks 'The corona store moved' && cc_lacks 'new corona store' && cc_lacks 'general add' && has '$PLACED'"
check "the run at the game's room: 1088 added and 544 refused, the job's clamp and the copy of 0x180 bytes past the records, all in the game's own store" "has '$(ran 1088 'in use')'"
check "the accessor says the game's own, with nothing counted" "has '$NOW_GAME'"
check "every page its own protection; no problem" "has 'protect: all restored (RX)' && cc_lacks 'PROBLEM'"

echo "== C2b the trap, at the game's own clamp: a batch from a count of 1500, as the raise would leave it if its clamp stayed"
fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe cctrap); echo "      ${out:0:80}"
check "the count lowered to 1088, the placed count wrapped, a copy of about 200 GB asked: what the raise must not leave" "has 'cc run: batch at 1500 brought 100 placed 4294966884, count 1088, the copy asks 206158410432 bytes |'"

echo "== C3 CORONA_STORE=4096, no DIAGNOSTIC_MODE: the store moved, the jumps as placed, no line of the store but the writes' own"
raise_ini; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "the 13 writes of the findings' table and nothing else, in the sites as placed; the new store read/write, 16-aligned, of 196992 bytes; the part's line of it" "has '$(moved 4096)' && cc_line 'The corona store moved and its limit raised from 1088 to 4096 entries.'"
check "the jumps and the batch's six bytes as placed: no stub without DIAGNOSTIC_MODE" "has 'cc jumps: as placed |'"
check "the run: the job's parameters name the new store and the old count; two threads' adds up to 4096, each in a slot of its own, the old store all 0; the batch up to the room; the job's own limit still 1088; the copy of 4096 and the 0x180 past them, as the store" "has '$(ran 4096 'all 0')'"
check "the accessor says the room, nothing counted" "has '$(now 4096 0 0 0 0)'"
check "every page its own protection; no DIAGNOSTIC line of the store; no problem" "has 'protect: all restored (RX)' && cc_lacks 'DIAGNOSTIC: .*corona store' && cc_lacks 'PROBLEM'"

echo "== C3b the trap gone: the same batch with the raise's clamp"
raise_ini; out=$(ASI=TweaksV_test.asi game full_game.exe cctrap); echo "      ${out:0:80}"
check "the count goes on past 1088, 100 placed, a copy of 100 records asked" "has 'cc run: batch at 1500 brought 100 placed 100, count 1600, the copy asks 4800 bytes |'"

echo "== C4 CORONA_STORE=4096, DIAGNOSTIC_MODE=1: every site logged at the fake's RVAs, every write, the counters through their stubs"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "the 14 patterns each found exactly once" "[ \$(grep -c -E 'Pattern G-E-[IJ][1-9] found: .* 1 match,' TweaksV.log) -eq 14 ]"
check "the store, its count and the inlined adds' stores" "cc_line 'DIAGNOSTIC: the corona store at RVA $(rva store), its count at RVA $(rva count): the inlined adds. stores at RVA $(rva_plus i2 11) (G-E-I2) and RVA $(rva_plus i3 11) (G-E-I3)'"
check "the store's references, by the scan, in address order: the four of the sites" "cc_line \"DIAGNOSTIC: the corona store at RVA $(rva store) is referenced in 4 places of the game.s code: $(in_order $(rva_plus i2 8) $(rva_plus i3 8) $(rva_plus i5 5) $(rva i6))\""
check "the copy, the inlined adds' limits" "cc_line 'DIAGNOSTIC: the copy to the draw list at RVA $(rva i5): its count at RVA $(rva_plus i4 2) (G-E-I4), its store at RVA $(rva_plus i5 8) (G-E-I5)' && cc_line 'DIAGNOSTIC: the inlined adds. limits at RVA $(rva_plus first 9) and RVA $(rva_plus second 9) hold 1087, their counts the same'"
check "the general add" "cc_line 'DIAGNOSTIC: the general add at RVA $(rva i1): its count at 0xCC00 of the store at RVA $(rva_plus i1 17) and RVA $(rva_plus i1 45), its limit at RVA $(rva_plus i1 24) holds 1087 (N-1, G-E-I1) -> game size 1088'"
check "the LOD lights' job: its parameters, its batch's clamp, its own limit left, its scratch" "cc_line 'DIAGNOSTIC: the LOD lights. job.s parameters at RVA $(rva i6) (G-E-I6), their call of what hands the store out at RVA $(rva i7) (G-E-I7)' && cc_line 'DIAGNOSTIC: the LOD lights. batch: its clamp at RVA $(rva_plus i8 1) and RVA $(rva_plus i8 41) holds 1088 (G-E-I8)' && cc_line 'DIAGNOSTIC: the LOD lights. job.s own limit at RVA $(rva_plus i9 7) holds 1087 (N-1, G-E-I9) -> game size 1088: left as it is' && cc_line 'DIAGNOSTIC: the LOD lights. job.s scratch at RVA $(rva j1): 0xCC00 bytes of coronas, 0x1C00 of lists, 0xE800 in all (G-E-J1)'"
check "the drawing side" "cc_line 'DIAGNOSTIC: the draw list.s command at RVA $(rva j2): the render pointer at RVA $(rva pointer), the render count at RVA $(rva 'render count'), the extras. count at RVA $(rva extras) (G-E-J2)' && cc_line 'DIAGNOSTIC: the draw at RVA $(rva j3): its mark at RVA $(rva mark), the render count at RVA $(rva 'render count') (G-E-J3)' && cc_line 'DIAGNOSTIC: the draw.s stack, 0x20 bytes a corona at RVA $(rva j4) (G-E-J4) and 4 bytes a corona at RVA $(rva j5) (G-E-J5)'"
check "the new store, its address and its bytes" "cc_line 'DIAGNOSTIC: the new corona store at 0x[0-9A-F]* (outside the game): 4096 coronas of 0x30 bytes and 0x180 past them, 196992 bytes'"
check "the four jumps, each to where the findings have it, with its bytes" "cc_line 'DIAGNOSTIC: the jump when the corona store is full in its general add at RVA $(rva_plus i1 28) -> RVA $(rva 'add way out'): 0F 28 34 24 0F 28 7C 24 10 44 0F 28 44 24 20 48 83 C4 38 (G-E-I1)' && cc_line 'DIAGNOSTIC: the jump when the corona store is full in the first inlined add at RVA $(rva_plus first 13) -> RVA $(rva 'first way out'): 0F 28 74 24 20 0F 28 7C 24 30 44 0F 28 44 24 40 44 0F 28 4C 24 50 48 83 C4 68 (G-E-G1)' && cc_line 'DIAGNOSTIC: the jump when the corona store is full in the second inlined add at RVA $(rva_plus second 13) -> RVA $(rva 'second way out'): 42 C6 44 2E 6B 00 66 90 48 FF C3 41 8B 47 04 48 39 C3 (G-E-G1)' && cc_line 'DIAGNOSTIC: the jump when the LOD lights. job.s own coronas are full at RVA $(rva_plus i9 11) -> RVA $(rva 'lod way out'): 4C 8B 44 24 58 44 0F 28 DC 0F 28 A4 24 A0 00 00 00 41 0F 50 C1 84 C0 (G-E-I9)'"
check "the store's limits' writes, old and new, each at its site" "cc_line 'DIAGNOSTIC: corona store.s limit in its general add at RVA $(rva_plus i1 24): 3F 04 00 00 -> FF 0F 00 00' && cc_line 'DIAGNOSTIC: corona store.s limit in the first inlined add at RVA $(rva_plus first 9): 3F 04 00 00 -> FF 0F 00 00' && cc_line 'DIAGNOSTIC: corona store.s limit in the second inlined add at RVA $(rva_plus second 9): 3F 04 00 00 -> FF 0F 00 00'"
check "the clamp's two writes" "cc_line 'DIAGNOSTIC: corona store.s clamp in the LOD lights. batch at RVA $(rva_plus i8 1): 40 04 00 00 -> 00 10 00 00' && cc_line 'DIAGNOSTIC: corona store.s comparison in the LOD lights. batch at RVA $(rva_plus i8 41): 40 04 00 00 -> 00 10 00 00'"
check "the lea in place of the nop, old and new" "cc_line 'DIAGNOSTIC: corona store.s lea in its general add at RVA $(rva i1): 66 66 66 66 66 2E 0F 1F 84 00 00 00 00 00 -> 48 8D 0D $ANY4 0F 1F 80 00 00 00 00'"
check "the count's three, each from 0xCC00" "cc_line 'DIAGNOSTIC: corona store.s count in its general add.s load at RVA $(rva_plus i1 17): 00 CC 00 00 -> $ANY4' && cc_line 'DIAGNOSTIC: corona store.s count in its general add.s exchange at RVA $(rva_plus i1 45): 00 CC 00 00 -> $ANY4' && cc_line 'DIAGNOSTIC: corona store.s count in what hands it out at RVA $(rva_plus i7 6): 00 CC 00 00 -> $ANY4'"
check "the store's four rel32" "cc_line 'DIAGNOSTIC: corona store in the first inlined add at RVA $(rva_plus i2 11): $ANY4 -> $ANY4' && cc_line 'DIAGNOSTIC: corona store in the second inlined add at RVA $(rva_plus i3 11): $ANY4 -> $ANY4' && cc_line 'DIAGNOSTIC: corona store in the copy to the draw list at RVA $(rva_plus i5 8): $ANY4 -> $ANY4' && cc_line 'DIAGNOSTIC: corona store in the LOD lights. job.s parameters at RVA $(rva_plus i6 3): $ANY4 -> $ANY4'"
check "the jumps' rel32 and the batch's six bytes; the announcement" "cc_line 'DIAGNOSTIC: jump when the corona store is full in its general add at RVA $(rva_plus i1 30): 24 00 00 00 -> $ANY4' && cc_line 'DIAGNOSTIC: jump when the corona store is full in the first inlined add at RVA $(rva_plus first 15): $ANY4 -> $ANY4' && cc_line 'DIAGNOSTIC: jump when the corona store is full in the second inlined add at RVA $(rva_plus second 15): $ANY4 -> $ANY4' && cc_line 'DIAGNOSTIC: jump when the LOD lights. job.s own coronas are full at RVA $(rva_plus i9 13): 0F 00 00 00 -> $ANY4' && cc_line 'DIAGNOSTIC: LOD lights. batch.s way to its counting stub at RVA $(rva_plus i8 56): 29 CA 48 8D 04 49 -> E9 $ANY4 90' && cc_line '$REDIRECTED'"
check "the writes' lines, each once: 13 of the store, 4 jumps, the batch's" "[ \$(count 'DIAGNOSTIC: corona store') -eq 13 ] && [ \$(count 'DIAGNOSTIC: jump when the \(corona store\|LOD lights\)') -eq 4 ] && [ \$(count 'batch.s way to its counting stub') -eq 1 ]"
check "the 13 writes and nothing else, with the jumps through their stubs" "has '$(moved 4096)'"
check "the stubs read+execute, each jump on to where it led, the batch's back past its six bytes; the counters read/write: both threads' 2048 and the batch's 200, the job's own 3, the batch's 300" "has 'cc jumps: 4 and the batch to stubs, RX, on to where each led, their counters RW: refused 2248, lod own 3, lod lights 300 |'"
check "the run as without them" "has '$(ran 4096 'all 0')'"
check "the accessor says the same numbers" "has '$(now 4096 1 2248 3 300)'"
check "every page its own protection; no problem of the store" "has 'protect: all restored (RX)' && cc_lacks 'PROBLEM'"

echo "== C5 the room at its bounds and past them: each moves, runs and fills to what the setting holds"
raise_ini 1089; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "1089, one above the game's: every write, the run up to 1089" "has '$(moved 1089)' && has '$(ran 1089 'all 0')' && has '$(now 1089 0 0 0 0)'"
raise_ini 8192; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "8192, the most: every write, the run up to 8192" "has '$(moved 8192)' && has '$(ran 8192 'all 0')' && has '$(now 8192 0 0 0 0)'"
raise_ini 9000; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "9000: reported by the settings and clamped to 8192, which moves" "log_line 'PROBLEM: TweaksV.ini: \[LIGHTCAPACITY\] CORONA_STORE=9000 is outside 1088..8192 - using 8192.\$' && has '$(moved 8192)'"
raise_ini 1000; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "1000: reported by the settings and clamped to the game's own: nothing of it looked for or written" "log_line 'PROBLEM: TweaksV.ini: \[LIGHTCAPACITY\] CORONA_STORE=1000 is outside 1088..8192 - using 1088.\$' && log_lacks 'G-E-[IJ]' && has '$PLACED' && has '$NOW_GAME'"

echo "== C6 DRY_RUN=1: found and checked, nothing written, the accessor says the game's own"
raise_ini; setkey DRY_RUN 1; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "every site's line, then the would lines: 13 of the store and the move; the jumps' would line, no stub" "cc_lacks 'The corona store moved' && cc_line 'DIAGNOSTIC: the general add at RVA $(rva i1): .*' && [ \$(count 'DRY_RUN: would patch the corona store') -eq 13 ] && cc_line 'DRY_RUN: would patch the corona store.s limit in its general add: 3F 04 00 00 -> FF 0F 00 00' && cc_line 'DRY_RUN: would move the corona store (5 references) and raise its limit from 1088 to 4096 (5 sites), with 3 offsets of its count.' && cc_line 'DRY_RUN: would redirect the four jumps when the corona store is full and the LOD lights. batch at RVA $(rva_plus i8 56) (G-E-I8) to counting stubs - the refused coronas are not counted.'"
check "no write's line, no jump's would line, no stub's announcement" "cc_lacks 'DIAGNOSTIC: corona store' && cc_lacks 'would patch the jump' && cc_lacks 'the four jumps when the corona store is full .* are redirected'"
check "its sites and jumps as placed; the accessor says the game's own; no problem" "has '$PLACED' && has '$NOW_GAME' && cc_lacks 'PROBLEM'"
raise_ini; setkey DRY_RUN 1; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "without DIAGNOSTIC_MODE: the 13 and the move, no word of the jumps; nothing written" "[ \$(count 'DRY_RUN: would patch the corona store') -eq 13 ] && cc_line 'DRY_RUN: would move the corona store (5 references) and raise its limit from 1088 to 4096 (5 sites), with 3 offsets of its count.' && cc_lacks 'would redirect the four jumps' && has '$PLACED' && has '$NOW_GAME'"

echo "== C7 Windows' dynamic code policy: no stub, no write, nothing changed"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe nodyncode); echo "      ${out:0:80}"
check "the stubs' line, word for word; the write refused; its sites and jumps as placed, the problem the part's" "cc_line '$NOT_COUNTED its stubs could not be made read+execute (.*).' && log_line '\[LightCapacity\] The game code at RVA 0x[0-9A-F]* could not be made writable (.*' && has '$PLACED' && cc_line \"$NOT_RAISED the game code could not be changed (details .*).\" && has '$NOW_GAME'"

echo "== C8 the near memory, the test build's TWEAKSV_TEST_CORONA_MEMORY"
raise_ini; out=$(TWEAKSV_TEST_CORONA_MEMORY='none' ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "none for the store: the switch's line and the reason; nothing written" "cc_line 'TEST BUILD: the near memory for the corona store is taken for none (TWEAKSV_TEST_CORONA_MEMORY).' && cc_line '$NOT_RAISED no memory near the game could be had for the new corona store of 4096 coronas.' && has '$PLACED'"
raise_ini; out=$(TWEAKSV_TEST_CORONA_MEMORY='far' ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "the store out of reach: the switch's line and the reason, naming the count; nothing written" "cc_line 'TEST BUILD: the corona store is given memory out of reach (TWEAKSV_TEST_CORONA_MEMORY).' && cc_line \"$NOT_RAISED the new corona store at 0x[0-9A-F]* (outside the game) is out of reach of its count at RVA $(rva count).\" && has '$PLACED'"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_CORONA_MEMORY='no stubs' ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "none for the stubs: the switch's line and the store's; no problem of it" "cc_line 'TEST BUILD: the near memory for the corona store.s stubs is taken for none (TWEAKSV_TEST_CORONA_MEMORY).' && cc_line '$NOT_COUNTED no memory near the game could be had for its stubs.' && cc_lacks 'PROBLEM'"
check "the raise goes on: every write, the jumps as placed, the run as ever, nothing counted" "has '$(moved 4096)' && has 'cc jumps: as placed |' && has '$(ran 4096 'all 0')' && has '$(now 4096 0 0 0 0)'"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_CORONA_MEMORY='far stubs' ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "the stubs out of reach: the switch's line and the store's; the raise goes on, the jumps as placed, nothing counted, no problem of it" "cc_line 'TEST BUILD: the corona store.s stubs are given memory out of reach (TWEAKSV_TEST_CORONA_MEMORY).' && cc_line '$NOT_COUNTED its stubs are out of reach of the game code.' && has '$(moved 4096)' && has 'cc jumps: as placed |' && has '$(now 4096 0 0 0 0)' && cc_lacks 'PROBLEM'"
raise_ini; out=$(TWEAKSV_TEST_CORONA_MEMORY='none' game full_game.exe); echo "      ${out:0:80}"
check "the Release build ignores the switch: no line of it, no export read, the store moved" "cc_lacks 'TEST BUILD' && has '$(moved 4096)' && ! has 'cc now:'"

echo "== C9 none of the game code present (plain fake game): the part's problem, none of the store's patterns looked for"
raise_ini; out=$(game plain_game.exe)
check "the first of the part's patterns named, whichever it looks for first; no pattern of the store looked for" "log_line '\[LightCapacity\] $NOT_RAISED G-E-[AG]1 was not found.\$' && log_lacks 'Pattern G-E-[IJ]'"

# The cases of each check failing and each pattern missing are queued, then run side by side, each in a game folder of its own, half as many at a time as the suites, and
# checked in order, since each run takes seconds and there are about 50. section TEXT: a heading. refused OPTIONS WHY: the fake's options, and the reason the part reports. broken
# LABEL: the test build with that pattern treated as not found. modded NAME LABEL: a byte of what that pattern pins changed. not_counted OPTIONS LINE: with DIAGNOSTIC_MODE=1, a
# jump not as the findings have it: the store's line, the raise going on.
CASES=()
section() { CASES+=("== $1"); }
refused() { CASES+=("R$1|$2"); }
broken() { CASES+=("!$1"); }
modded() { CASES+=("Mccmodded $1|$2"); }
not_counted() { CASES+=("N$1|$2"); }
OUT='0x[0-9A-F]* (outside the game)'
HERE='RVA 0x[0-9A-F]*'

section "C10 the store's sites, each check failing on its own"
refused 'lcout i2store' "the corona store at $OUT (G-E-I2) is not inside the game (another mod may have moved it already)."
refused 'lcend i2store' "the corona store at $HERE (G-E-I2) is not inside the game (another mod may have moved it already)."
refused 'lcother i3store' "the second inlined add names the corona store at $HERE, the first inlined add the one at $HERE (G-E-I3)."
refused 'lcout i3store' "the second inlined add names the corona store at $OUT, the first inlined add the one at $HERE (G-E-I3)."
refused 'lcother i5store' "the copy to the draw list names the corona store at $HERE, the first inlined add the one at $HERE (G-E-I5)."
refused 'lcother i6store' "the LOD lights. job names the corona store at $HERE, the first inlined add the one at $HERE (G-E-I6)."
refused 'lcother i4count' "the copy to the draw list reads the count at $HERE, not the one right behind the 1088 coronas, at $HERE (G-E-I4)."
refused 'lcother g1first lcother g1second' "the first inlined add reads the count at $HERE, not the one right behind the 1088 coronas, at $HERE (G-E-G1)."
refused 'lcother g1second' "\(the \(first\|second\) inlined add reads the count at $HERE, not the one right behind the 1088 coronas, at $HERE (G-E-G1).\|the corona store.s two checks read different counts: $HERE and $HERE (G-E-G1).\)"
refused 'ccrefs' "the game.s code references the corona store at $HERE in 20 places or more, not 19, as the research found them."
refused 'cccalls' "the game.s code calls what hands the corona store out, at $HERE, in 2 places or more, not 1 (G-E-I7)."
refused 'lcother i6call' "the LOD lights. job.s call at $HERE leads to $HERE, not to what hands the store out, at $HERE (another mod may have hooked it already) (G-E-I6)."
refused 'lcout i6call' "the LOD lights. job.s call at $HERE leads to $OUT, not to what hands the store out, at $HERE (another mod may have hooked it already) (G-E-I6)."
refused 'lcout j2pointer' "the draw list.s render pointer at $OUT (G-E-J2) is not inside the game."
refused 'lcout j2count' "the draw list.s render count at $OUT (G-E-J2) is not inside the game."
refused 'lcout j2extras' "the draw list.s extras. count at $OUT (G-E-J2) is not inside the game."
refused 'lcout j3mark' "the draw.s mark at $OUT (G-E-J3) is not inside the game."
refused 'lcother j3count' "the draw reads the render count at $HERE, the draw list.s command stores it at $HERE (G-E-J3)."
refused 'ccedge j2extras rtjumpunread' "the draw list.s command.s store of the extras. count at $HERE cannot be read (G-E-J2)."
refused 'ccedge j3count rtjumpunread' "the draw.s read of the render count at $HERE cannot be read (G-E-J3)."

section "C11 a site another mod changed: what its pattern pins, so the pattern is not found"
modded i1nop G-E-I1
modded i4past G-E-I4
modded i7count G-E-I7
modded i8move G-E-I8
modded i8clamp G-E-I8
modded i9limit G-E-I9
modded j1scratch G-E-J1

section "C12 TEST_BREAK_PATTERN (test build): each of the 14 patterns missing"
for label in $(sed -nE 's/^\s*\{ "(G-E-[IJ][0-9]+)", .*/\1/p' "$R/source/TweaksVPatterns.h"); do
	broken "$label"
done

section "C13 a jump not as the findings have it, with DIAGNOSTIC_MODE=1: the store's line, no stub, and the raise goes on"
not_counted 'lcout i1jump' "the jump when the corona store is full in its general add at $HERE does not lead into the game (another mod may have hooked it already) (G-E-I1)."
not_counted 'lcunread i1jump' "the jump when the corona store is full in its general add at $HERE leads to $HERE, which cannot be read (G-E-I1)."
not_counted 'lcother i1jump' "the jump when the corona store is full in its general add at $HERE leads to $HERE, which holds [0-9A-F ]*, not what the research found there (G-E-I1)."
not_counted 'lcother g1jump1' "the jump when the corona store is full in the first inlined add at $HERE leads to $HERE, which holds [0-9A-F ]*, not what the research found there (G-E-G1)."
not_counted 'lcother g1jump2' "the jump when the corona store is full in the second inlined add at $HERE leads to $HERE, which holds [0-9A-F ]*, not what the research found there (G-E-G1)."
not_counted 'lcout g1jump2' "the jump when the corona store is full in the second inlined add at $HERE does not lead into the game (another mod may have hooked it already) (G-E-G1)."
not_counted 'lcother i9jump' "the jump when the LOD lights. job.s own coronas are full at $HERE leads to $HERE, which holds [0-9A-F ]*, not what the research found there (G-E-I9)."
not_counted 'ccedge g1jump rtjumpunread' "the jump when the corona store is full in the \(first\|second\) inlined add at $HERE cannot be read (G-E-G1)."

# run_case N CASE, in the background
game_folder=$B
run_case() {
	B=$game_folder/cases/$1
	mkdir -p "$B" && cd "$B" || exit 1
	cp ../../TweaksV.asi ../../TweaksV_test.asi .
	raise_ini
	local kind=${2:0:1} body=${2:1}
	case "$kind" in
		"!") addkey TEST_BREAK_PATTERN "$body"; ASI=TweaksV_test.asi game ../../full_game.exe > /dev/null ;;
		N) setkey DIAGNOSTIC_MODE 1; ASI=TweaksV_test.asi game ../../full_game.exe ${body%%|*} > /dev/null ;;
		*) ASI=TweaksV_test.asi game ../../full_game.exe ${body%%|*} > /dev/null ;;
	esac
}
n=0; running=0
for entry in "${CASES[@]}"; do
	[[ "$entry" == "== "* ]] && continue
	n=$((n + 1))
	run_case "$n" "$entry" &
	running=$((running + 1))
	if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
done
wait
n=0
for entry in "${CASES[@]}"; do
	if [[ "$entry" == "== "* ]]; then echo "$entry"; continue; fi
	n=$((n + 1))
	B=$game_folder/cases/$n
	cd "$B" || exit 1
	out=$(tr -d '\r' < harness.out)
	body=${entry:1}
	case "${entry:0:1}" in
		"!")
			check "$body: the scanner's test line and the reason; nothing written; the accessor says the game's own" "grep -q 'TEST BUILD: pattern $body is treated as not found' TweaksV.log && cc_line '$NOT_RAISED $body was not found.' && has '$PLACED' && has '$NOW_GAME'" ;;
		M)
			check "${body%%|*}: the scanner's line and the reason; nothing written" "log_has 'Pattern ${body#*|} not found: ' && cc_line '$NOT_RAISED ${body#*|} was not found.' && has '$PLACED'" ;;
		N)
			options=${body%%|*}
			if [[ "$options" == ccedge* ]]; then
				check "$options: the line, word for word; no stub; no problem of the store" "cc_line \"$NOT_COUNTED ${body#*|}\" && has 'cc jumps: as placed |' && cc_lacks 'the four jumps when the corona store is full .* are redirected' && cc_lacks 'PROBLEM'"
			else
				check "$options: the line, word for word; no stub, the raise goes on, nothing counted; no problem of the store" "cc_line \"$NOT_COUNTED ${body#*|}\" && has 'cc jumps: as placed |' && has '$(moved 4096)' && has '$(now 4096 0 0 0 0)' && cc_lacks 'the four jumps when the corona store is full .* are redirected' && cc_lacks 'PROBLEM'"
			fi ;;
		*)
			check "${body%%|*}: the reason, word for word; nothing written; the accessor says the game's own" "cc_line \"$NOT_RAISED ${body#*|}\" && has '$PLACED' && has '$NOW_GAME'" ;;
	esac
done
B=$game_folder
cd "$B" && rm -rf cases

echo "== C14 the sirens at 32 behind the store: anything that refuses the store holds them at 20, nothing of the store written"
SIREN_REFUSAL='\[SirenSettings\] PROBLEM: The sirens per vehicle are not raised this session: the light stores are not raised (\[LIGHTCAPACITY\] in TweaksV.ini), and the game.s own do not take 32 sirens a vehicle. Vehicles keep 20 sirens.$'
SECOND_ADD="the second inlined add names the corona store at $HERE, the first inlined add the one at $HERE (G-E-I3)."
SIRENS_ON=1 fresh_ini; addkey TEST_BREAK_PATTERN G-E-I1; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "a pattern of the store missing: the part's problem and the sirens' behind it, word for word; nothing written; the accessor says the game's own" "cc_line '$NOT_RAISED G-E-I1 was not found.' && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$NOW_GAME'"
SIRENS_ON=1 fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe lcother i3store); echo "      ${out:0:80}"
check "a check of the store failing: the same" "cc_line \"$NOT_RAISED $SECOND_ADD\" && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$NOW_GAME'"
SIRENS_ON=1 fresh_ini; out=$(TWEAKSV_TEST_CORONA_MEMORY='none' ASI=TweaksV_test.asi game full_game.exe); echo "      ${out:0:80}"
check "no memory for the store: the same" "cc_line '$NOT_RAISED no memory near the game could be had for the new corona store of 4096 coronas.' && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$NOW_GAME'"
SIRENS_ON=1 fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe nodyncode); echo "      ${out:0:80}"
check "the part's write refused by Windows, after the store said yes: the same" "cc_line \"$NOT_RAISED the game code could not be changed (details .*).\" && log_line '$SIREN_REFUSAL' && has '$PLACED' && has '$NOW_GAME'"
SIRENS_ON=1 fresh_ini; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun); echo "      ${out:0:80}"
check "the store raised: the sirens do not refuse, every write of the store, its run as ever" "log_lacks 'the light stores are not raised' && has '$(moved 4096)' && has '$(ran 4096 'all 0')' && has '$(now 4096 0 0 0 0)'"

echo "== C15 the part's measurement reads the store through the accessor: its room and its counters in the summary, the first with the raise written"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe ccrun wait); echo "      ${out:0:80}"
check "the run's counters in the first summary, in the findings' words" "cc_line 'DIAGNOSTIC: the most so far: .*coronas [0-9]* of 4096, refused 2248 (LOD.s own 1088: 3), LOD lights 300, .*'"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe ccpast); echo "      ${out:0:80}"
check "a count past the game's 1088, inside the room: a number of the summary, no line of its own" "cc_line 'DIAGNOSTIC: the most so far: .*coronas 1089 of 4096, refused 0 (LOD.s own 1088: 0), LOD lights 0, .*' && cc_lacks 'coronas seen at 1089'"
check "a count past the room: its line says the room" "cc_line 'DIAGNOSTIC: coronas seen at 4097, past the 4096 the game has room for.'"
raise_ini; setkey DIAGNOSTIC_MODE 1; out=$(ASI=TweaksV_test.asi game full_game.exe lcother i3store wait); echo "      ${out:0:80}"
check "a refusal of the store: the part's problem, nothing written, and the measurement goes on over the game's own room" "cc_line \"$NOT_RAISED $SECOND_ADD\" && has '$PLACED' && has '$NOW_GAME' && cc_line 'DIAGNOSTIC: the lighting capacity is measured: .*' && cc_line 'DIAGNOSTIC: the most so far: .*coronas 0 of 1088, .*'"
raise_ini; setkey DIAGNOSTIC_MODE 1; setkey DRY_RUN 1; out=$(ASI=TweaksV_test.asi game full_game.exe wait); echo "      ${out:0:80}"
check "a dry run: the store's would lines in the part's, and the measurement over the game's own room" "[ \$(count 'DRY_RUN: would patch the corona store') -eq 13 ] && has '$PLACED' && has '$NOW_GAME' && cc_line 'DIAGNOSTIC: the most so far: .*coronas 0 of 1088, .*'"

echo "== C16 what stays of the new store: the raise's one region, and nothing of a dry run or of a store out of reach"
raise_ini; out=$(ASI=TweaksV_test.asi game full_game.exe ccleft); echo "      ${out:0:80}"
check "the raise: its store is the one region of its size, all 0" "has '$(moved 4096)' && has 'cc left: 1 |'"
raise_ini; setkey DRY_RUN 1; out=$(ASI=TweaksV_test.asi game full_game.exe ccleft); echo "      ${out:0:80}"
check "a dry run: the store it made for its lines is gone" "has '$PLACED' && has 'cc left: 0 |'"
raise_ini; out=$(TWEAKSV_TEST_CORONA_MEMORY='far' ASI=TweaksV_test.asi game full_game.exe ccleft); echo "      ${out:0:80}"
check "a store out of reach: refused, and gone" "has '$PLACED' && has 'cc left: 0 |'"

echo
echo "CORONA STORE SCENARIOS: $pass passed, $fail failed"
