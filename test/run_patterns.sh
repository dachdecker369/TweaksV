#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# The pattern table: source\TweaksVPatterns.h and PATTERNS.md list the same patterns - the same labels
# with the same bytes - and the code uses every pattern of the table exactly once; and, through the full fake
# game and the test build's TEST_TWICE_PATTERN, each pattern held too often is refused in the same words.
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# "LABEL TEXT" per line, sorted: the table's entries, PATTERNS.md's rows (but the removed ones) and the
# labels the code names with Pattern("...") (the table's own comment does not count)
table=$(sed -nE 's/^\s*\{ "([A-Za-z0-9-]+)", "([^"]+)" \},.*/\1 \2/p' "$R/source/TweaksVPatterns.h" | sort)
docs=$(grep -E '^\| [A-Z]-E[-A-Za-z0-9]* \| `' "$R/PATTERNS.md" | grep -v '| removed |' | sed -E 's/^\| ([^ ]+) \| `([^`]+)` \|.*/\1 \2/' | sort)
uses=$(grep -ohE 'Pattern\("[A-Za-z0-9-]+"\)' $(ls "$R"/source/*.cpp "$R"/source/*.h | grep -v TweaksVPatterns.h) | sed -E 's/Pattern\("(.*)"\)/\1/' | sort)
labels=$(cut -d' ' -f1 <<< "$table")
echo "      $(wc -l <<< "$table") patterns in the table, $(wc -l <<< "$docs") in PATTERNS.md, $(wc -l <<< "$uses") uses in the code"

echo "== the table and PATTERNS.md"
check "PATTERNS.md lists exactly the table's patterns, with the same bytes" '[ -n "$table" ] && [ "$table" = "$docs" ]'
[ "$table" = "$docs" ] || diff <(echo "$table") <(echo "$docs") | sed 's/^/      /'

echo "== the table and the code"
check "every pattern of the table is used, and only once" '[ "$labels" = "$uses" ]'
[ "$labels" = "$uses" ] || diff <(echo "$labels") <(echo "$uses") | sed 's/^/      /'

echo "== each pattern held too often (the test build's TEST_TWICE_PATTERN, a run each): the words of one function, whichever part looks for it"
cd "$B" || exit 1
game_folder=$B
cp "$R/object/Test/Release/TweaksV.asi" TweaksV_test.asi
PEDPROPS_ON=1
# held_too_often LABEL, in the background: its run with every part looking, in a game folder of its own; a fixed 2 seconds
# each, so that the runs go side by side, half as many at a time as the suites, and the suite stays short
held_too_often() {
	local exe=full_game.exe
	case "$1" in
		P-E-R31b|P-E-D2) exe=full_game_1013.exe ;;	# the packfile part's patterns of older game builds
		P-E-D1) exe=full_game_889.exe ;;
	esac
	B=$game_folder/twice/$1
	mkdir -p "$B/mods/test" && cd "$B" || exit 1
	echo hi > mods/test/file.txt
	cp ../../TweaksV_test.asi .
	fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_TWICE_PATTERN "$1"
	[ "$1" != R-E-C1 ] || setkey CONSOLE 1			# looked for with the console alone
	ASI=TweaksV_test.asi game "../../$exe" > /dev/null
}
running=0
for label in $labels; do
	held_too_often "$label" &
	running=$((running + 1))
	if [ "$running" -ge "$(( ${TWEAKSV_TEST_JOBS:-4} / 2 + 1 ))" ]; then wait -n; running=$((running - 1)); fi
done
wait
for label in $labels; do
	expected=1
	case "$label" in
		D-E-L12) expected=7 ;;						# the two that must match more than once
		A-E-G1) expected=2 ;;
	esac
	B=$game_folder/twice/$label
	cd "$B" || exit 1
	check "$label: the test line and the words" "log_has 'TEST BUILD: pattern $label is treated as held once more than the game code holds it (TEST_TWICE_PATTERN)\.' && log_has 'the game code holds $label $((expected + 1)) or more times, not the expected $expected'"
done
B=$game_folder
cd "$B" && rm -rf twice
cp "$R/x64/Release/TweaksV.asi" TweaksV.asi
fresh_ini; setkey DIAGNOSTIC_MODE 1; addkey TEST_TWICE_PATTERN S-E1; out=$(game full_game.exe)
check "the Release build ignores the key: the pattern found once, no test line, not the words" "log_has 'Pattern S-E1 found: .* 1 match,' && log_lacks 'TEST BUILD' && log_lacks 'the game code holds'"

echo
echo "PATTERN TABLE CHECKS: $pass passed, $fail failed"
