#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# StraightToStoryMode scenarios and HeapAdjuster's: a fake GTA5_Enhanced.exe with copies of the patched
# game code loads the real TweaksV.asi. Fake LOCALAPPDATA: the real one is never touched.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
cd "$B" || exit 1
cp "$R/x64/Release/TweaksV.asi" .
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
mkdir -p appdata

ORIG_SPLASH='splash: 0F 84 85 01 00 00 |'; NEW_SPLASH='splash: E9 86 01 00 00 90 |'
ORIG_LEGAL='legal: 0F 84 07 05 00 00 |';   NEW_LEGAL='legal: 90 90 90 90 90 90 |'
ORIG_LANDING='landing: 48 89 5C 24 08 |';  NEW_LANDING='landing: 31 C0 C3 24 08 |'
# the pool memory budget: what its site holds, the rest of the site as placed, what the game's start-up made of it
GAME_POOLS='pool memory: 0x08800000, 136 MB, the rest as placed, its constructor given 136 MB, 120 blocks, 0 refused |'
RAISED_POOLS='pool memory: 0x10000000, 256 MB, the rest as placed, its constructor given 256 MB, 120 blocks, 0 refused |'

echo "== S1 defaults: all three screens skipped, heap adjusted"
fresh_ini; out=$(game full_game.exe); echo "      $out"
check "splash: jz +0x185 -> jmp +0x186 / nop (computed, same as upstream's bytes)" "[[ \"\$out\" == *\"$NEW_SPLASH\"* ]]"
check "legal: 6 NOPs" "[[ \"\$out\" == *\"$NEW_LEGAL\"* ]]"
check "landing page: xor eax,eax / ret" "[[ \"\$out\" == *\"$NEW_LANDING\"* ]]"
check "heap 1024 MB" "[[ \"\$out\" == *'heap: 1024 MB'* ]]"
check "the pool memory budget raised to the default's 256 MB, the one immediate and nothing else of its site, every page its own protection; its line" "has '$RAISED_POOLS' && has 'protect: all restored (RX)' && log_line '\[HeapAdjuster\] Pool memory budget patched: 136 MB -> 256 MB.\$'"
check "the three patch lines, old -> new bytes" "log_has 'Splash screen patched: 0F 84 85 01 00 00 -> E9 86 01 00 00 90.' && log_has 'Legal screen patched: 0F 84 07 05 00 00 -> 90 90 90 90 90 90.' && log_has 'Landing page patched: 48 89 5C -> 31 C0 C3.'"
check "no problems" "log_lacks 'PROBLEM'"

echo "== S2 the three switches off: nothing changed"
fresh_ini; setkey SKIP_SPLASH_SCREEN 0 STRAIGHTTOSTORYMODE; setkey SKIP_LEGAL_SCREEN 0 STRAIGHTTOSTORYMODE; setkey SKIP_LANDING_PAGE 0 STRAIGHTTOSTORYMODE
out=$(game full_game.exe); echo "      $out"
check "all three sites unchanged" "[[ \"\$out\" == *\"$ORIG_SPLASH\"* && \"\$out\" == *\"$ORIG_LEGAL\"* && \"\$out\" == *\"$ORIG_LANDING\"* ]]"
check "each switch logged in the part's own line" "log_has 'SKIP_SPLASH_SCREEN=0 - the splash screen is not skipped.' && log_has 'SKIP_LEGAL_SCREEN=0 - the legal screen is not skipped.' && log_has 'SKIP_LANDING_PAGE=0 - the landing page is not skipped.'"

echo "== S3 only SKIP_LEGAL_SCREEN=0"
fresh_ini; setkey SKIP_LEGAL_SCREEN 0 STRAIGHTTOSTORYMODE; out=$(game full_game.exe); echo "      $out"
check "splash and landing patched, legal left alone" "[[ \"\$out\" == *\"$NEW_SPLASH\"* && \"\$out\" == *\"$ORIG_LEGAL\"* && \"\$out\" == *\"$NEW_LANDING\"* ]]"

echo "== S4 [STRAIGHTTOSTORYMODE] ENABLED=0"
fresh_ini; setkey ENABLED 0 STRAIGHTTOSTORYMODE; out=$(game full_game.exe); echo "      $out"
check "nothing changed" "[[ \"\$out\" == *\"$ORIG_SPLASH\"* && \"\$out\" == *\"$ORIG_LEGAL\"* && \"\$out\" == *\"$ORIG_LANDING\"* ]]"
check "header: StraightToStoryMode off, the other parts on (the heap); no StraightToStoryMode lines" "log_has 'Features: .*, StraightToStoryMode off' && has 'heap: 1024 MB' && log_lacks '\[StraightToStoryMode\]'"

echo "== S5 DRY_RUN=1: found and checked, nothing written"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe); echo "      $out"
check "nothing changed" "[[ \"\$out\" == *\"$ORIG_SPLASH\"* && \"\$out\" == *\"$ORIG_LEGAL\"* && \"\$out\" == *\"$ORIG_LANDING\"* ]]"
check "eight 'would patch' lines - the three screens, the heap, the pool memory budget, the RPF cache check, the weapon info blob array's size and capacity - with old -> new bytes" "[ \$(count 'DRY_RUN: would patch the') -eq 8 ] && log_has 'would patch the splash screen: 0F 84 85 01 00 00 -> E9 86 01 00 00 90' && log_has 'would patch the RPF cache check: 48 89 -> C3 90' && log_has 'would patch the heap size (648 MB -> 1024 MB): 00 00 80 28 -> 00 00 00 40' && log_has 'would patch the pool memory budget (136 MB -> 256 MB): 00 00 80 08 -> 00 00 00 10'"
check "the pool memory budget as the game holds it, no line of a raise" "has '$GAME_POOLS' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
check "no 'patched' line" "log_lacks ' patched: '"

echo "== S6 DIAGNOSTIC_MODE=1: every pattern and write logged"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game full_game.exe)
check "three story patterns found once each" "log_has 'Pattern S-E1 found: 0F 85 A9 00 00 00 48 8D 54 -> RVA 0x.*, 1 match' && log_has 'Pattern S-E2 found: 00 E9 ? 05 00 00 E8 ? 07 -> RVA 0x.*, 1 match' && log_has 'Pattern S-E3 found: E8 ? ? ? ? 8B 0D ? ? ? ? 83 C1 ? 84 C0 -> RVA 0x.*, 1 match'"
check "three writes logged with old -> new bytes" "log_has 'DIAGNOSTIC: splash screen at RVA 0x.*: 0F 84 85 01 00 00 -> E9 86 01 00 00 90' && log_has 'DIAGNOSTIC: legal screen at RVA' && log_has 'DIAGNOSTIC: landing page at RVA 0x.*: 48 89 5C -> 31 C0 C3'"
check "patterns found once get no per-match lines (only D-E-L12, which matches 7 times, and G-E-G1, twice, get them: A-E-G1, which matches twice, is PedProps', off)" "[ \$(count ' match 1 of ') -eq 2 ] && log_has 'D-E-L12 match 1 of 7 at RVA' && log_has 'G-E-G1 match 1 of 2 at RVA'"
check "the heap's site logged with the size it holds" "log_has 'HeapAdjuster\] DIAGNOSTIC: heap size at RVA 0x[0-9A-F]* holds 648 MB'"
check "the pool memory budget's pattern and site, its allocator, its write, word for word" "log_has 'Pattern H-E2 found: 48 8D 0D ? ? ? ? 48 8D 15 ? ? ? ? 41 B8 00 00 01 00 41 B9 ? ? ? ? E8 -> RVA 0x.*, 1 match' && log_line '\[HeapAdjuster\] DIAGNOSTIC: pool memory budget at RVA 0x[0-9A-F]* holds 136 MB, its allocator at RVA 0x[0-9A-F]*, its heap at RVA 0x[0-9A-F]*, its constructor.s call at RVA 0x[0-9A-F]* -> RVA 0x[0-9A-F]* (H-E2)\$' && log_line '\[HeapAdjuster\] DIAGNOSTIC: pool memory budget (136 MB -> 256 MB) at RVA 0x[0-9A-F]*: 00 00 80 08 -> 00 00 00 10\$'"
check "the three screens' sites logged with the bytes they hold, three lines" "[ \$(count 'StraightToStoryMode\] DIAGNOSTIC: .*\(jz\|function\) at RVA') -eq 3 ] && log_has 'DIAGNOSTIC: splash screen jz at RVA 0x[0-9A-F]*: 0F 84 85 01 00 00' &&log_has 'DIAGNOSTIC: legal screen jz at RVA 0x[0-9A-F]*: 0F 84 07 05 00 00' && log_has 'DIAGNOSTIC: landing page function at RVA 0x[0-9A-F]*: 48 89 5C'"

echo "== S7 already patched (an old StraightToStoryMode ran first under another name): skipped quietly"
fresh_ini; out=$(game full_game.exe prepatched); echo "      $out"
check "bytes stay patched" "[[ \"\$out\" == *\"$NEW_SPLASH\"* && \"\$out\" == *\"$NEW_LEGAL\"* && \"\$out\" == *\"$NEW_LANDING\"* ]]"
check "three 'already patched - skipped' lines, no problems" "[ \$(count 'already patched - skipped') -eq 3 ] && log_lacks 'PROBLEM'"

echo "== S8 unexpected bytes at the splash site: refused, the others still applied"
fresh_ini; out=$(game full_game.exe badsplash); echo "      $out"
check "splash left alone" "[[ \"\$out\" == *'splash: 48 89 85 01 00 00 |'* ]]"
check "refusal logged with the bytes found" "log_has 'The splash screen is not skipped this session: the game code found is not the expected instruction (48 89 85 01 00 00).'"
check "legal and landing still patched" "[[ \"\$out\" == *\"$NEW_LEGAL\"* && \"\$out\" == *\"$NEW_LANDING\"* ]]"

echo "== S9 unexpected bytes at the legal site: refused with the bytes logged"
fresh_ini; out=$(game full_game.exe badlegal); echo "      $out"
check "legal left alone" "[[ \"\$out\" == *'legal: 48 89 07 05 00 00 |'* ]]"
check "refusal logged with the bytes found" "log_has 'The legal screen is not skipped this session: the game code found is not the expected instruction (48 89 07 05 00 00).'"

echo "== S9b a jnz instead of the game's jz at the legal site: refused, the others still applied"
fresh_ini; out=$(game full_game.exe jnzlegal); echo "      $out"
check "legal left alone" "[[ \"\$out\" == *'legal: 0F 85 07 05 00 00 |'* ]]"
check "refusal logged with the bytes found" "log_has 'The legal screen is not skipped this session: the game code found is not the expected instruction (0F 85 07 05 00 00).'"
check "splash and landing still patched, the legal screen not" "[[ \"\$out\" == *\"$NEW_SPLASH\"* && \"\$out\" == *\"$NEW_LANDING\"* ]] && log_lacks 'Legal screen patched'"

echo "== S10 the landing page function is hooked by another mod: left alone"
fresh_ini; out=$(game full_game.exe hookedlanding); echo "      $out"
check "hook left alone" "[[ \"\$out\" == *'landing: E9 11 22 33 44 |'* ]]"
check "reported" "log_has 'another mod has already hooked'"

echo "== S11 an old StraightToStoryMode.asi in the folder: TweaksV's part is skipped"
fresh_ini; touch StraightToStoryMode.asi; out=$(game full_game.exe); echo "      $out"; rm -f StraightToStoryMode.asi
check "nothing changed" "[[ \"\$out\" == *\"$ORIG_SPLASH\"* && \"\$out\" == *\"$ORIG_LEGAL\"* && \"\$out\" == *\"$ORIG_LANDING\"* ]]"
check "old plugin reported" "log_has 'Old plugin found: .*StraightToStoryMode.asi'"

echo "== S12 none of the game code present (plain fake game): each screen reported, no crash"
fresh_ini; out=$(game plain_game.exe); echo "      $out"
check "harness survived" "grep -q 'loaded' harness.out"
check "three 'not found' problems" "log_has 'The splash screen is not skipped this session: S-E1 was not found.' && log_has 'The legal screen is not skipped this session: S-E2 was not found.' && log_has 'The landing page is not skipped this session: S-E3 was not found.'"

echo "== S13 another heap adjuster ran first (the site holds 2000 MB, or 3072 MB): left as it is, logged, not a problem"
fresh_ini; out=$(game full_game.exe heapraised); echo "      $out"
check "the heap stays at the 2000 MB found, HEAP_SIZE=1024 not applied, logged, no problem" "[[ \"\$out\" == *'heap: 2000 MB'* ]] && log_has 'The heap size is not changed this session: another heap adjuster already set it to 2000 MB - left as it is.' && log_lacks 'Heap size patched' && log_lacks 'PROBLEM'"
fresh_ini; setkey HEAP_SIZE 2047; out=$(game full_game.exe heapraised)
check "nor is HEAP_SIZE=2047: what another mod changed is not changed again" "[[ \"\$out\" == *'heap: 2000 MB'* ]] && log_lacks 'Heap size patched' && log_lacks 'PROBLEM'"
fresh_ini; setkey HEAP_SIZE 648; out=$(game full_game.exe)
check "HEAP_SIZE at the game's own 648 MB: nothing to patch, logged, no problem" "[[ \"\$out\" == *'heap: 648 MB'* ]] && log_has 'Heap size is 648 MB already (HEAP_SIZE=648) - nothing to patch.' && log_lacks 'Heap size patched' && log_lacks 'PROBLEM'"
fresh_ini; out=$(game full_game.exe heaphuge); echo "      $out"
check "3072 MB, more than HEAP_SIZE allows (the mov zero-extends its imm32), is another heap adjuster's too: left as it is, logged, no problem" "[[ \"\$out\" == *'heap: 3072 MB'* ]] && log_has 'The heap size is not changed this session: another heap adjuster already set it to 3072 MB - left as it is.' && log_lacks 'unexpected value' && log_lacks 'PROBLEM'"

echo "== S14 the heap site holds a value that is not a whole number of MB: refused with the value, the screens still skipped"
fresh_ini; out=$(game full_game.exe heapodd); echo "      $out"
check "refused, the value in the reason, nothing patched there" "log_has 'The heap size is not changed this session: the game code found holds an unexpected value (0x28800001).' && log_lacks 'Heap size patched' && [[ \"\$out\" == *\"$NEW_SPLASH\"* ]]"

echo "== S15 [HEAP_ADJUSTER] ENABLED=0: the heap is left at the game's own size, the screens are still skipped"
fresh_ini; setkey ENABLED 0 HEAP_ADJUSTER; out=$(game full_game.exe); echo "      $out"
check "the heap stays at the game's 648 MB, the pool memory budget at its 136 MB, the three screens skipped" "[[ \"\$out\" == *'heap: 648 MB'* && \"\$out\" == *\"$NEW_SPLASH\"* && \"\$out\" == *\"$NEW_LEGAL\"* && \"\$out\" == *\"$NEW_LANDING\"* ]] && has '$GAME_POOLS'"
check "header: HeapAdjuster off; no HeapAdjuster lines, no problem" "log_has 'Features: HeapAdjuster off, ' && log_lacks '\[HeapAdjuster\]' && log_lacks 'PROBLEM'"

echo "== S16 S-E3 matches twice (a second copy of the landing page's site): not used, the landing page left alone, the others skipped"
fresh_ini; out=$(game full_game.exe twos3); echo "      $out"
check "the landing page left alone, the splash and legal screens skipped" "[[ \"\$out\" == *\"$ORIG_LANDING\"* && \"\$out\" == *\"$NEW_SPLASH\"* && \"\$out\" == *\"$NEW_LEGAL\"* ]]"
check "the scanner says why; the problem names the pattern" "log_has 'Scanner\] Pattern S-E3 matches 2 or more times - not used (the site must be unique).' && log_has 'PROBLEM: The landing page is not skipped this session: the game code holds S-E3 2 or more times, not the expected 1.'"

echo "== S17 HEAP_SIZE=100, below the game's own 648 MB: raised to it and reported, so the start-up summary names the size used"
fresh_ini; setkey HEAP_SIZE 100; out=$(game full_game.exe); echo "      $out"
check "the heap stays at the game's 648 MB; the clamp reported for the start-up summary, nothing to patch" "[[ \"\$out\" == *'heap: 648 MB'* ]] && log_has 'PROBLEM: TweaksV.ini: HEAP_SIZE=100 is outside 648..2047 - using 648.' && log_has 'Heap size is 648 MB already (HEAP_SIZE=100) - nothing to patch.' && log_lacks 'Heap size patched'"

echo "== S18 POOL_MEMORY: the file's value, the game's own, and past either limit, which the settings clamp and report"
fresh_ini; setkey POOL_MEMORY 300; out=$(game full_game.exe); echo "      $out"
check "POOL_MEMORY=300: 300 MB handed to the constructor, its line" "has 'pool memory: 0x12C00000, 300 MB, the rest as placed, its constructor given 300 MB, 120 blocks, 0 refused |' && log_line '\[HeapAdjuster\] Pool memory budget patched: 136 MB -> 300 MB.\$' && log_has 'HeapAdjuster: HEAP_SIZE=1024 POOL_MEMORY=300'"
fresh_ini; setkey POOL_MEMORY 136; out=$(game full_game.exe)
check "POOL_MEMORY=136, the game's own: nothing to patch, logged, no problem" "has '$GAME_POOLS' && log_line '\[HeapAdjuster\] Pool memory budget is 136 MB already (POOL_MEMORY=136) - nothing to patch.\$' && log_lacks '\[HeapAdjuster\] Pool memory budget patched' && log_lacks '\[HeapAdjuster\] PROBLEM'"
fresh_ini; setkey POOL_MEMORY 100; out=$(game full_game.exe)
check "POOL_MEMORY=100, below the game's own: reported by the settings, 136 used, nothing to patch" "has '$GAME_POOLS' && log_has 'PROBLEM: TweaksV.ini: \[HEAP_ADJUSTER\] POOL_MEMORY=100 is outside 136..2047 - using 136.' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
fresh_ini; setkey POOL_MEMORY 5000; out=$(game full_game.exe)
check "POOL_MEMORY=5000: reported by the settings, 2047 used, the most the imm32 holds as a positive int32" "has 'pool memory: 0x7FF00000, 2047 MB, the rest as placed, its constructor given 2047 MB, 120 blocks, 0 refused |' && log_has 'PROBLEM: TweaksV.ini: \[HEAP_ADJUSTER\] POOL_MEMORY=5000 is outside 136..2047 - using 2047.' && log_line '\[HeapAdjuster\] Pool memory budget patched: 136 MB -> 2047 MB.\$'"

echo "== S19 what the budget is for: a load past the game's 136 MB is refused by its allocator there, and fits the default's 256 MB"
fresh_ini; setkey POOL_MEMORY 136; out=$(game full_game.exe pmheavy); echo "      $out"
check "at 136 MB the pool past the budget is refused, the allocation the game then writes through" "has 'pool memory: 0x08800000, 136 MB, the rest as placed, its constructor given 136 MB, 120 blocks, 1 refused |'"
fresh_ini; out=$(game full_game.exe pmheavy)
check "at the default's 256 MB it fits" "has 'pool memory: 0x10000000, 256 MB, the rest as placed, its constructor given 256 MB, 121 blocks, 0 refused |'"

echo "== S20 another mod or a game update (the site holds 200 MB, or no whole number of MB): left as it is, logged, not a problem"
fresh_ini; setkey POOL_MEMORY 1024; out=$(game full_game.exe pmraised); echo "      $out"
check "the 200 MB found stays, POOL_MEMORY=1024 not applied, logged, no problem; the heap still patched" "has 'pool memory: 0x0C800000, 200 MB, the rest as placed, its constructor given 200 MB, 120 blocks, 0 refused |' && log_line '\[HeapAdjuster\] The pool memory budget is not changed this session: another mod or a game update already set it to 200 MB - left as it is.\$' && log_lacks '\[HeapAdjuster\] Pool memory budget patched' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'heap: 1024 MB'"
fresh_ini; out=$(game full_game.exe pmodd); echo "      $out"
check "a value that is no whole number of MB is one too, its bytes in the line; no problem" "has 'pool memory: 0x0C800001, 200 MB, the rest as placed,' && log_line '\[HeapAdjuster\] The pool memory budget is not changed this session: another mod or a game update already set it to 0x0C800001 bytes - left as it is.\$' && log_lacks '\[HeapAdjuster\] Pool memory budget patched' && log_lacks '\[HeapAdjuster\] PROBLEM'"

echo "== S21 each job goes on without the other: H-E2 missing or held twice, H-E1 missing (the test build's keys); the plain game; no code may be changed"
fresh_ini; addkey TEST_BREAK_PATTERN H-E2; out=$(ASI=TweaksV_test.asi game full_game.exe); echo "      $out"
check "H-E2 missing: nothing changed at its site, the problem names it; the heap still patched" "has '$GAME_POOLS' && log_has 'TEST BUILD: pattern H-E2 is treated as not found' && log_line '\[HeapAdjuster\] PROBLEM: The pool memory budget is not changed this session: H-E2 was not found.\$' && has 'heap: 1024 MB'"
fresh_ini; addkey TEST_TWICE_PATTERN H-E2; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "H-E2 held twice: nothing changed at its site, the problem says so; the heap still patched" "has '$GAME_POOLS' && log_line '\[HeapAdjuster\] PROBLEM: The pool memory budget is not changed this session: the game code holds H-E2 2 or more times, not the expected 1.\$' && has 'heap: 1024 MB'"
fresh_ini; addkey TEST_BREAK_PATTERN H-E1; out=$(ASI=TweaksV_test.asi game full_game.exe)
check "H-E1 missing: the heap as the game holds it, the problem names it; the budget still raised" "has 'heap: 648 MB' && log_has 'PROBLEM: The heap size is not changed this session: H-E1 was not found.' && has '$RAISED_POOLS'"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(game plain_game.exe)
check "the plain game: both jobs' problems, the measurement's line, no crash" "grep -q 'loaded' harness.out && log_has 'PROBLEM: The heap size is not changed this session: H-E1 was not found.' && log_line '\[HeapAdjuster\] PROBLEM: The pool memory budget is not changed this session: H-E2 was not found.\$' && log_line '\[HeapAdjuster\] DIAGNOSTIC: the pool memory is not measured: H-E2 was not found.\$'"
fresh_ini; out=$(game full_game.exe nodyncode); echo "      $out"
check "no code may be changed: nothing written, each job's problem" "has 'pool memory: 0x08800000, 136 MB, the rest as placed, its constructor given 136 MB,' && has 'heap: 648 MB' && log_line '\[HeapAdjuster\] PROBLEM: The pool memory budget is not changed this session: the game code could not be made writable (details in TweaksV.log).\$' && log_has 'PROBLEM: The heap size is not changed this session: the game code could not be made writable' && has 'protect: all restored (RX)'"

echo "== S21b the allocator set up before TweaksV started, without DIAGNOSTIC_MODE: written as the heap's is, and reported, or nothing would say that the raise did not take"
LATE='The pool memory budget is not changed this session: the game set its allocator up before TweaksV started, with 136 MB, so the new value at its site comes too late.'
fresh_ini; out=$(game full_game.exe pmbefore); echo "      $out"
check "the problem word for word, after the write's line, the one problem; the site written, its constructor given 136 MB" "log_line \"\\[HeapAdjuster\\] PROBLEM: \$LATE\\\$\" && log_line '\[HeapAdjuster\] Pool memory budget patched: 136 MB -> 256 MB.\$' && log_has 'Start-up summary not shown (ERROR_POPUP=0): 1 problem' && has 'pool memory: 0x10000000, 256 MB, the rest as placed, its constructor given 136 MB, 120 blocks, 0 refused |'"
fresh_ini; setkey DRY_RUN 1; out=$(game full_game.exe pmbefore)
check "DRY_RUN: nothing written, the same problem" "log_line \"\\[HeapAdjuster\\] PROBLEM: \$LATE\\\$\" && log_lacks '\\[HeapAdjuster\\] Pool memory budget patched' && has '$GAME_POOLS'"
fresh_ini; setkey POOL_MEMORY 136; out=$(game full_game.exe pmbefore)
check "POOL_MEMORY=136: no raise to come too late, no problem" "log_lacks '\\[HeapAdjuster\\] PROBLEM' && has '$GAME_POOLS'"
fresh_ini; out=$(game full_game.exe pmfield)
check "a field of another value says nothing: written, no problem, the game's set-up handed it" "log_lacks '\\[HeapAdjuster\\] PROBLEM' && has '$RAISED_POOLS'"
fresh_ini; out=$(game full_game.exe)
check "a field of 0 says nothing: written, no problem" "log_lacks '\\[HeapAdjuster\\] PROBLEM' && has '$RAISED_POOLS'"

# The measurement's runs, each past its first summary, 10 seconds in, side by side in a folder of its own: pm NAME [INI_CHANGES] -- [OPTION...], in the
# background; pm_in NAME takes the checks there. The first summary comes 10 s after TweaksV's start, the fake's report 11 s later or more.
measured=$B
pm() {
	local name=$1 change=$2; shift 2
	B=$measured/pm/$name
	mkdir -p "$B" && cd "$B" || exit 1
	cp ../../TweaksV.asi ../../TweaksV_test.asi .
	fresh_ini; setkey DIAGNOSTIC_MODE 1; eval "$change"
	game "../../full_game.exe" "$@" > /dev/null
}
pm_in() { B=$measured/pm/$1; cd "$B" || exit 1; out=$(tr -d '\r' < harness.out); }
pm_line() { log_line "\[HeapAdjuster\] $1\$"; }

pm default : wait &
pm nearly 'setkey POOL_MEMORY 136' wait &
pm raised : pmraised wait &
pm odd : pmodd wait &
pm before : pmbefore wait &
pm notmade : pmnotmade wait &
pm nosum : pmnosum wait &
pm beforesum : pmbefore pmnosum wait &
pm outside : pmoutside &
pm unaligned : pmunaligned &
pm unreadable : pmunreadable &
pm gone : pmgone wait &
pm dry 'setkey DRY_RUN 1' wait &
pm off 'setkey ENABLED 0 HEAP_ADJUSTER' wait &
pm heapout : pmheapout &
pm callout : pmcallout &
pm callunread : pmcallunread &
pm refused 'export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 pool memory summary"' &
pm off2 'setkey DIAGNOSTIC_MODE 0; export ASI=TweaksV_test.asi TWEAKSV_TEST_REFUSE_THREAD="1 pool memory summary"' wait &
wait

echo "== S22 DIAGNOSTIC_MODE=1: the pool memory measured from the allocator, 10 seconds in"
pm_in default; echo "      $out"
check "the announcement, word for word; the thread's first summary, word for word: the load's 134.5 MB of the default's 256, the least room kept, one block given back" "pm_line 'DIAGNOSTIC: the pool memory is measured, through its allocator at RVA 0x[0-9A-F]* (H-E2): its use and its least room so far are logged after 10 seconds, then every minute while they change.' && pm_line 'DIAGNOSTIC: pool memory: 134.5 of 256 MB in use, at least 120.4 MB free so far, 120 blocks (900 ends the game).' && [ \$(count '\[HeapAdjuster\] DIAGNOSTIC: pool memory: ') -eq 1 ]"
check "no problem of the part's, the game as the part leaves it" "log_lacks '\[HeapAdjuster\] PROBLEM' && has '$RAISED_POOLS'"
pm_in nearly; echo "      $out"
check "POOL_MEMORY=136: the same load nearly spends it - a line of the log, no problem, no box" "pm_line 'DIAGNOSTIC: pool memory: 134.5 of 136 MB in use, at least 0.4 MB free so far, 120 blocks (900 ends the game) - nearly spent: a pool made now can crash the game at RVA 0x15A514 on 1.0.1158.16.' && log_lacks '\[HeapAdjuster\] PROBLEM'"
pm_in raised
check "another mod's 200 MB: measured all the same, what it holds" "pm_line 'DIAGNOSTIC: pool memory: 134.5 of 200 MB in use, at least 64.4 MB free so far, 120 blocks (900 ends the game).' && has 'pool memory: 0x0C800000, 200 MB,'"
pm_in odd
check "another mod's value of no whole MB: its tenth" "pm_line 'DIAGNOSTIC: pool memory: 134.5 of 200.0 MB in use, at least 64.4 MB free so far, 120 blocks (900 ends the game).'"
pm_in before
check "the allocator set up before TweaksV started: the problem, the budget it holds said to be not the site's, then its numbers; the raise written all the same" "pm_line \"PROBLEM: \$LATE\" && pm_line 'DIAGNOSTIC: pool memory: its allocator at RVA 0x[0-9A-F]* holds a budget of 136 MB, not the 256 MB its site holds: the game set it up before TweaksV started, or its layout is not as the research found it.' && pm_line 'DIAGNOSTIC: pool memory: 134.5 of 136 MB in use, at least 0.4 MB free so far, 120 blocks (900 ends the game) - nearly spent: a pool made now can crash the game at RVA 0x15A514 on 1.0.1158.16.'"
pm_in notmade
check "the allocator not set up: said so, nothing read past it" "pm_line 'DIAGNOSTIC: no pool memory yet - the game has not set its allocator up.' && [ \$(count '\[HeapAdjuster\] DIAGNOSTIC: pool memory: ') -eq 0 ]"
pm_in nosum
check "in use and room that do not add up: said so, then the numbers" "pm_line 'DIAGNOSTIC: pool memory: 135.5 MB in use and 121.5 MB of room do not add up to its budget of 256 MB: the game may have been changing them, or its layout is not as the research found it.' && pm_line 'DIAGNOSTIC: pool memory: 135.5 of 256 MB in use, at least 120.4 MB free so far, 120 blocks (900 ends the game).'"
pm_in beforesum
check "both things a pattern cannot pin, each its line, then the numbers: a budget that is not the site's does not keep the use and room from being checked" "pm_line 'DIAGNOSTIC: pool memory: its allocator at RVA 0x[0-9A-F]* holds a budget of 136 MB, not the 256 MB its site holds: the game set it up before TweaksV started, or its layout is not as the research found it.' && pm_line 'DIAGNOSTIC: pool memory: 135.5 MB in use and 1.5 MB of room do not add up to its budget of 136 MB: the game may have been changing them, or its layout is not as the research found it.' && pm_line 'DIAGNOSTIC: pool memory: 135.5 of 136 MB in use, at least 0.4 MB free so far, 120 blocks (900 ends the game) - nearly spent: a pool made now can crash the game at RVA 0x15A514 on 1.0.1158.16.'"
pm_in outside
check "the allocator outside the game: another mod's, left and logged, no problem, nothing written; not measured, a line" "pm_line 'The pool memory budget is not changed this session: its allocator at 0x[0-9A-F]* (outside the game) is another mod.s (H-E2) - left as it is.' && pm_line 'DIAGNOSTIC: the pool memory is not measured: its allocator at 0x[0-9A-F]* (outside the game) is not inside the game (H-E2).' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x08800000, 136 MB, the rest as placed,' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
pm_in heapout
check "the heap outside the game: another mod's, left and logged, nothing written" "pm_line 'The pool memory budget is not changed this session: its heap at 0x[0-9A-F]* (outside the game) is another mod.s (H-E2) - left as it is.' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x08800000, 136 MB, the rest as placed,' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
pm_in callout
check "the constructor outside the game: another mod's, left and logged, nothing written" "pm_line 'The pool memory budget is not changed this session: its constructor at 0x[0-9A-F]* (outside the game) is another mod.s (H-E2) - left as it is.' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x08800000, 136 MB, the rest as placed,' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
pm_in callunread
check "the constructor's call that cannot be read, past the match on a page of no access: the problem, nothing written" "pm_line 'PROBLEM: The pool memory budget is not changed this session: its constructor.s call at RVA 0x[0-9A-F]* cannot be read (H-E2).' && pm_line 'DIAGNOSTIC: pool memory budget at RVA 0x[0-9A-F]* holds 136 MB, its allocator at RVA 0x[0-9A-F]*, its heap at RVA 0x[0-9A-F]*, its constructor.s call at RVA 0x[0-9A-F]* -> cannot be read (H-E2)' && has 'pool memory: 0x08800000, 136 MB, the rest as placed,' && log_lacks '\[HeapAdjuster\] Pool memory budget patched'"
pm_in refused
check "the summary's thread that cannot start: not measured, a line, no problem; the budget raised (the test build's TWEAKSV_TEST_REFUSE_THREAD)" "pm_line 'DIAGNOSTIC: the pool memory is not measured: its summary thread could not be started.' && log_has 'The pool memory summary thread could not be started (Windows error 8' && log_lacks '\[HeapAdjuster\] DIAGNOSTIC: the pool memory is measured' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x10000000, 256 MB,'"
pm_in unaligned
check "the allocator not aligned: not measured, a line, no problem; the budget raised" "pm_line 'DIAGNOSTIC: the pool memory is not measured: its allocator at RVA 0x[0-9A-F]* is not aligned to 8 bytes (H-E2).' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x10000000, 256 MB,'"
pm_in unreadable
check "the allocator that cannot be read as TweaksV starts: not measured, a line, no problem, its field read for nothing; the budget raised" "pm_line 'DIAGNOSTIC: the pool memory is not measured: its allocator at RVA 0x[0-9A-F]* cannot be read (H-E2).' && log_lacks '\[HeapAdjuster\] PROBLEM' && has 'pool memory: 0x10000000, 256 MB,'"
pm_in gone
check "the allocator that cannot be read later: the announcement, then the summary says so, no crash" "pm_line 'DIAGNOSTIC: the pool memory is measured, through its allocator at RVA 0x[0-9A-F]* (H-E2): its use and its least room so far are logged after 10 seconds, then every minute while they change.' && pm_line 'DIAGNOSTIC: pool memory: its allocator at RVA 0x[0-9A-F]* or its site at RVA 0x[0-9A-F]* cannot be read.' && has 'protect: '"
pm_in dry
check "DRY_RUN: nothing written; measured all the same, of the game's own budget" "has '$GAME_POOLS' && pm_line 'DRY_RUN: would patch the pool memory budget (136 MB -> 256 MB): 00 00 80 08 -> 00 00 00 10' && pm_line 'DIAGNOSTIC: pool memory: 134.5 of 136 MB in use, at least 0.4 MB free so far, 120 blocks (900 ends the game) - nearly spent: a pool made now can crash the game at RVA 0x15A514 on 1.0.1158.16.'"
pm_in off
check "[HEAP_ADJUSTER] ENABLED=0: a part that is off logs nothing and measures nothing" "has '$GAME_POOLS' && log_lacks '\[HeapAdjuster\]' && log_lacks 'Pattern H-E1'"
pm_in off2
check "DIAGNOSTIC_MODE=0: the budget raised, no line of the measurement, and no start of its thread, which the test build would refuse and say so" "has '$RAISED_POOLS' && log_lacks '\[HeapAdjuster\] DIAGNOSTIC' && log_has 'TWEAKSV_TEST_REFUSE_THREAD=1 pool memory summary' && log_lacks 'The pool memory summary thread could not be started'"
B=$measured; cd "$B" && rm -rf pm

echo "== S23 the test build, its summaries every 2 s: again when the numbers change - a pool more 11 s in, then 3 s more - and no more"
fresh_ini; setkey DIAGNOSTIC_MODE 1; out=$(TWEAKSV_TEST_SUMMARY_MS=2000 ASI=TweaksV_test.asi game full_game.exe pmmore); echo "      $out"
check "the test build's line for this measurement, the first summary and one more, no third" "log_line '\[TweaksV\] TEST BUILD: the pool memory summary comes every 2000 ms after its first (TWEAKSV_TEST_SUMMARY_MS=2000).\$' && log_line '\[HeapAdjuster\] DIAGNOSTIC: pool memory: 134.5 of 256 MB in use, at least 120.4 MB free so far, 120 blocks (900 ends the game).\$' && log_line '\[HeapAdjuster\] DIAGNOSTIC: pool memory: 135.5 of 256 MB in use, at least 120.4 MB free so far, 121 blocks (900 ends the game).\$' && [ \$(count '\[HeapAdjuster\] DIAGNOSTIC: pool memory: ') -eq 2 ]"

echo
echo "STORY SCENARIOS: $pass passed, $fail failed"
