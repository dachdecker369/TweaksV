#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Builds TweaksV (Release and the test build) and the fake game programs, runs every suite at the same time, each in
# a folder of its own, and deletes what the run made (object\TestRun and object\Test). From Git Bash, in the
# repository:
#   bash test/run_all.sh [--no-build] [--keep] [--serial] [SUITE...]
# --no-build tests the TweaksV.asi files already built, so it works after a --keep run (every other run
# deletes the test build); --keep leaves the run folders for a look; --serial runs the suites one after another;
# a SUITE of the list below runs that suite alone (bash test/run_all.sh blips), and several run side by side,
# their output in the order given. The run fails when a check fails, when a suite stops early, when a suite makes
# another number of checks than the CHECKS line of its script gives, when a build warns, and in a full run when the
# number of checks is not the one NOTES.md gives.
set -u
T="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R="$(cd "$T/.." && pwd)"
RUN="$R/object/TestRun"		# all a run makes but the builds: programs\, the test programs, and a folder per suite
# the suites, one a line, their output in this order: the tables first, the parts in the order of the parts, the parts
# measured first in the order of SPECIFICATIONS.md 11.16, the whole of TweaksV last
SUITES=(
	patterns
	ini_default
	story
	modkit
	packfile
	boxstreamer
	decals
	rageopenv
	legacypacks
	directstorage
	blips
	weaponinfos
	weaponcomponents
	pedprops
	texturebudget
	raytracing
	lightcapacity
	# [corona store]
	sirenids
	sirencount
	# [siren damage]
	sirendamage
	staticbounds
	population
	pedpool
	renderqueues
	integration
)

build=1; keep=0; serial=0; only=()
for a in "$@"; do
	case "$a" in
		--no-build) build=0 ;;
		--keep) keep=1 ;;
		--serial) serial=1 ;;
		-*) echo "unknown option: $a"; exit 2 ;;
		*) [[ " ${SUITES[*]} " == *" $a "* ]] || { echo "unknown suite: $a"; exit 2; }
			[[ " ${only[*]-} " == *" $a "* ]] || only+=("$a") ;;	# a suite has one folder, so it runs once
	esac
done
[ "${#only[@]}" = 0 ] || SUITES=("${only[@]}")

cleanup() {
	[ "$keep" = 1 ] && { echo "kept: $RUN and $R/object/Test"; return; }
	rm -rf -- "${RUN:?}" "${R:?}/object/Test"
}

# check_warnings LOG: the warnings a build log holds, by their codes - "warning C4100", "warning LNK4075", and "cl : Command
# line warning D9025" has no ": warning " - are printed, with the log's last lines, and counted
warnings=0
check_warnings() {
	local found
	found=$(tr -d '\r' < "$1" | grep -cE 'warning [A-Z]+[0-9]+')
	[ "$found" -gt 0 ] || return 0
	echo "== $found warning(s) in $(basename "$1"):"
	tr -d '\r' < "$1" | grep -E 'warning [A-Z]+[0-9]+' | head -n 20
	echo "   the last lines of $(basename "$1"):"
	tr -d '\r' < "$1" | tail -n 5
	warnings=$((warnings + found))
}

# a stopped run's suites and fakes go on and hold their folders: nothing of this run could work beside them
rm -rf -- "${RUN:?}" || { echo "$RUN cannot be deleted: does a stopped run's fake game still hold it?"; exit 1; }
mkdir -p "$RUN/programs" || exit 1

if [ "$build" = 1 ]; then
	echo "== building TweaksV (Release and test build)"
	cmd //c "$(cygpath -w "$T/build_tweaksv.cmd")" > "$RUN/build_tweaksv.txt" 2>&1 \
		|| { tr -d '\r' < "$RUN/build_tweaksv.txt" | tail -n 30; echo "TweaksV build failed"; cleanup; exit 1; }
	check_warnings "$RUN/build_tweaksv.txt"
fi
for f in "$R/x64/Release/TweaksV.asi" "$R/object/Test/Release/TweaksV.asi"; do
	[ -f "$f" ] || { echo "missing: $f (run without --no-build)"; cleanup; exit 1; }
done

echo "== building the test programs"
OUT_DIR="$(cygpath -w "$RUN/programs")" cmd //c "$(cygpath -w "$T/build.cmd")" > "$RUN/build_tests.txt" 2>&1 \
	|| { tr -d '\r' < "$RUN/build_tests.txt" | tail -n 30; echo "test program build failed"; cleanup; exit 1; }
check_warnings "$RUN/build_tests.txt"

# run_suite SUITE, in the background: its game folder a folder of its own, with its own copy of the programs and, next to
# it, its own temp folder (common.sh), so that what one suite's fakes hold, delete or deny never reaches another's. The
# game folder keeps the name TestRun, which the suites' patterns of a path name.
run_suite() {
	local s=$1 start=$SECONDS
	{
		mkdir -p "$RUN/$s/TestRun" && cp "$RUN"/programs/*.exe "$RUN"/programs/*.dll "$RUN/$s/TestRun/" &&
			TWEAKSV_TEST_DIR="$RUN/$s/TestRun" bash "$T/run_$s.sh"
	} > "$RUN/$s.out" 2>&1
	echo "$((SECONDS - start)) $s" > "$RUN/$s.seconds"
}

# As many suites at a time as the machine has threads, one with --serial; each one's output and result come out whole,
# in the list's order, whichever finishes first. A suite that goes side by side itself asks TWEAKSV_TEST_JOBS how far.
most=${NUMBER_OF_PROCESSORS:-4}
[ "$serial" = 0 ] || most=1
export TWEAKSV_TEST_JOBS=$most
total_pass=0; total_fail=0; total_skipped=0; wrong_counts=0; summary=""; pids=(); started=0; n=${#SUITES[@]}
for ((shown = 0; shown < n; shown++)); do
	while [ "$started" -lt "$n" ] && [ "$((started - shown))" -lt "$most" ]; do
		run_suite "${SUITES[started]}" &
		pids[started]=$!
		started=$((started + 1))
	done
	wait "${pids[shown]}"
	s=${SUITES[shown]}
	echo; echo "######## $s"
	cat "$RUN/$s.out"
	line=$(grep -E '[0-9]+ passed, [0-9]+ failed' "$RUN/$s.out" | tail -n 1)
	p=$(sed -nE 's/.* ([0-9]+) passed, ([0-9]+) failed.*/\1/p' <<< "$line")
	f=$(sed -nE 's/.* ([0-9]+) passed, ([0-9]+) failed.*/\2/p' <<< "$line")
	skipped=$(grep -c '^SKIP' "$RUN/$s.out")	# among the passed (common.sh): said, so that a green run names the checks it did not make
	# the number of checks the CHECKS line of the suite's own script gives, compared in every run, a run of named suites too:
	# a check lost shows in the suite it left, also beside another suite's new ones, and a merge changes one line per suite
	want=$(sed -nE 's/^CHECKS=([0-9]+)$/\1/p' "$T/run_$s.sh")
	note=""
	if [ -z "$p" ]; then p=0; f=1; note=" - no result line (the suite stopped early)"
	elif [ "$((p + f))" != "${want:-none}" ]; then note=" - its CHECKS line gives ${want:-no number}"; wrong_counts=$((wrong_counts + 1)); fi
	total_pass=$((total_pass + p)); total_fail=$((total_fail + f)); total_skipped=$((total_skipped + skipped))
	summary+="  $(printf '%-16s' "$s") $p passed, $f failed, $skipped skipped$note"$'\n'
done

# the number of checks NOTES.md's "Tests:" bullet gives ("(N checks)") - that bullet's, not the first number
# before "checks" anywhere in the file: a check that silently stops running shows
expected=$(grep -E '^- \*\*Tests:\*\*' "$R/NOTES.md" | grep -oE '\([0-9]+ checks\)' | head -n 1 | grep -oE '[0-9]+')
ok=1
[ "$total_fail" = 0 ] && [ "$wrong_counts" = 0 ] || ok=0
echo; echo "======== RESULTS"
printf '%s' "$summary"
echo "  all            $total_pass passed, $total_fail failed, $total_skipped skipped"
echo "  time           $SECONDS s; the slowest: $(cat "$RUN"/*.seconds | sort -rn | head -n 3 | sed -E 's/(.*) (.*)/\2 \1 s/' | paste -sd, | sed 's/,/, /g')"
if [ "${#only[@]}" = 0 ] && [ "$((total_pass + total_fail))" != "${expected:-none}" ]; then
	echo "  the count: NOTES.md gives ${expected:-no number of} checks, this run made $((total_pass + total_fail))"; ok=0
fi
if [ "$warnings" != 0 ]; then echo "  build warnings: $warnings (listed above)"; ok=0; fi
cleanup
[ "$ok" = 1 ]
