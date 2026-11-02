# Plan

The order of work for a change to TweaksV, phase by phase, each ending with its proof, a commit and my boot.
What came before phase C is in the git history and in NOTES.md's Versions; the passes' findings and reviews are in my research tools' `output-1158.16\`.
Phase C, planned on 2026-11-01, is two builds (D19): 2.7.0, with six research passes side by side, StaticBounds anew through the original's two hooks, the siren setting IDs, the population's limits and the lighting capacity measured, the open issues the round takes, and the sirens per vehicle if approved to them in time; then 2.8.0, the raises 2.7.0's numbers ask for, planned from those numbers.

## How the work runs

The work runs as my rules for every project have them since 2026-11-01 (D29): the three levels of tracks, the fastest split, short plans, one proof on the merged tree, small state files, few messages.
This section holds only what is this round's.

- **The overview** (the overview) reads the weekly limits and starts the merge.
  It checks each research pass's findings from their summary, on paper, before anything is built on them (D20), and brings me the decisions they ask for.
  It has the merged diff reviewed once, asks for my boots, reads them, tags 2.7.0 and pushes.
- **The merge** (the merge) makes the scaffold and the reflow on main (D27), writes one short plan per track and starts the tracks in its track.

- **Eleven tracks at the go** - five coding tracks and six research passes - each on a branch of its own from main at the reflow's commit.
  The later ones - one measurement for each limit its findings confirm (D25), and the sirens per vehicle if approved (D23) - start from main once the overview has checked the findings and I have decided on them, each.

Every coding track builds from what its item names: the findings file in my research tools' folder, `C:\_DEVELOPMENT\_Tools\TweaksV-research\output-1158.16\<topic>_findings_for_the_clean_room_build.txt`, with the decisions below where they change the design and the review beside it, `<topic>_review.md`; the static bounds track from the original's source (D24); the sweeps from the open issues' entries in NOTES.md.
No track reads code of an original whose license does not allow it (D2) - Resource Extensions' plugin and SirenSetting Limit Adjuster's code least of all.
Only the research passes analyse the dump (D20): a fact a coding track lacks goes up as a question.
No committed file and no commit message names a local folder or a branch.

## The tracks

The fastest split the work allows (D29): research by topic, since it runs no suites and merges nothing; a coding track for about an hour of work or more, small items together in a sweep, the tracks about as long as the longest.
Each table is in the order of the merge.

**The research, at the go** (D20):

| Track | Track | Owns |
|---|---|---|
| The sirens' bones and parser | | `sirencount2_findings_for_the_clean_room_build.txt` in my research tools' `output-1158.16\`, its work folder `work-sirencount2` there |
| The lighting capacity | | `lightcapacity_findings_for_the_clean_room_build.txt`, `work-lightcapacity` |
| The vehicle model budget | | `vehiclemodelbudget_findings_for_the_clean_room_build.txt`, `work-vehiclemodelbudget` |
| The ped model budget | | `pedmodelbudget_findings_for_the_clean_room_build.txt`, `work-pedmodelbudget` |
| The ped pool | | `pedpool_findings_for_the_clean_room_build.txt`, `work-pedpool` |
| The render queues | | `renderqueues_findings_for_the_clean_room_build.txt`, `work-renderqueues` |

**The coding, at the go:**

| Track | Track | Length | Owns | Joined by |
|---|---|---|---|---|
| Data window | | short, on its own since the sirens' pass needs its boot early | `source\TweaksVDump.cpp` and `.h`; its block in `test\run_integration.sh` | the switch and its scenarios |
| Static bounds | | long | `source\TweaksVStaticBounds.cpp` and `.h`; `IsPluginInstalledFor` in `source\TweaksVConflicts.cpp` and `.h`; `test\run_staticbounds.sh` | the part and its suite |
| Siren IDs | | long | `source\TweaksVSirenIds.cpp` and `.h`; the relocation core's fixed size in `source\TweaksVRelocation.cpp` and `.h`; `test\run_sirenids.sh` | the part's IDs and its suite; the core's change, which the move needs first and no other track uses |
| Sweep: the code | | about as long | T-COPIES' points in their files, but for `TweaksVSirenIds.cpp`'s, `TweaksVPatch.h` and `.cpp` for the one copies; `PackEntry` and the Legacy check's last line in `source\TweaksVLegacyPacks.cpp`; `source\TweaksVModsFolder.cpp` and `.h`; `source\TweaksVSummary.cpp` and `.h`; the report's one call in the guards and the Legacy check; their blocks in the suites | T-COPIES, T-LEGACYWALK, T-NAMES, T-LATEBOX and T-SUMMARY, none an hour's work alone, and the join does not lengthen the round |
| Sweep: the tests | | shorter | `test\run_all.sh` and `test\common.sh`; `TEST_TWICE_PATTERN` in `source\TweaksVScanner.cpp`; the test build's line in `source\TweaksVLog.cpp`; a real timer's switch in `source\TweaksVMeasure.cpp`; TB16's block in `test\run_texturebudget.sh` and its two writers in `test\fake_game_full.cpp`; its block in `test\run_legacypacks.sh` | the suites made fast (D29) and T-TESTGAPS' four points, none an hour's work alone |

**The coding, later,** each once the overview has checked its findings and I have decided on them, each; a module's name may change with its findings:

| Track | Track | Owns | Joined by |
|---|---|---|---|
| The sirens per vehicle, if approved | | `source\TweaksVSirenCount.cpp` and `.h`; `test\run_sirencount.sh` | the part's raise and its suite; needs its findings first |
| The lighting capacity, measured | | `source\TweaksVLightCapacity.cpp` and `.h`; `test\run_lightcapacity.sh` | the module and its suite; needs its findings first |
| The two model budgets, measured | | `source\TweaksVPopulation.cpp` and `.h`; `test\run_population.sh` | one module for both, as they share a block and a site (D31) |
| The ped pool, measured, and the pool memory raised | | `source\TweaksVPedPool.cpp` and `.h`; `test\run_pedpool.sh`; `POOL_MEMORY` in `source\TweaksVHeapAdjuster.cpp`, its row of `source\TweaksVSettings.h` and the files that pin the layout | the module, its suite and the raise, since they share the site (D31) |
| The render queues, measured | | `source\TweaksVRenderQueues.cpp` and `.h`; `test\run_renderqueues.sh` | the same |

The scaffold puts an anchor only where two tracks must write in one table, and each track writes right below its own: `source\TweaksVFeatures.cpp`'s `kMeasuredFirst`, `source\TweaksVPatterns.h`, `source\TweaksV.vcxproj` and `.vcxproj.filters`, `test\fake_game_full.cpp`, `test\run_all.sh` and `test\run_integration.sh`'s I3, and PATTERNS.md, whose rows the patterns suite pins to the table.
`source\TweaksVPatch.h` and `.cpp` need none under this split, since only the code sweep writes there.
`source\TweaksVSettings.h` and `.cpp` hold the two test keys, the scaffold's (D27), which the tracks read.
Every other document is the fragments' (D29).
The counts more than one track changes are the merge's, which takes their sum.

## Decisions

D1 to D18, phases A and B, in short; their full text is in the git history up to 098836c.
"Stands" marks what still binds.

- **D1:** the parts' coding round in two phases: a build that only measures, then the writing halves, only where the numbers ask for them.
- **D2, stands, the clean room:** no track reads code of an original whose license does not allow it; every game fact comes from findings, found anew in the Enhanced dump; the limit on reverse-engineering the game is lifted only by me, pass by pass; CREDITS.md names an original when a part takes its idea.
- **D3, stands, a limit measured first:** a module that, with `DIAGNOSTIC_MODE=1` only, finds every pattern of the limit, checks each site as a writing half would, logs every site, measures through `StartMeasurement` and writes nothing (SPECIFICATIONS.md, the parts measured first).
- **D4:** a pattern's label has one letter per part (PATTERNS.md).
- **D5:** phase A's scaffold.
- **D6:** 2.5.0's measurement boot, from which phase B was planned.
- **D7, stands, the versions:** a build for my own use is tagged and pushed, and the next version I publish is the full one, with all of the upgrades.
- **D8:** the siren count measured, through one redirected call of the renderer.
- **D9, stands, the conflict check:** it names only plugins made for Enhanced, Legacy names left out; WeaponLimitsAdjuster Enhanced joins once its page shows its file, the withdrawn Resource Adjuster Enhanced if its file is ever known.
- **D10:** phase B built WeaponLimits, PedProps and the texture budget and dropped the streaming heap; PedProps ships off (NOTES.md, T-PEDPROPS).
- **D11:** phase B's scaffold and `TweaksV.ini`'s layout 4.
- **D12:** phase B's designs, as built in SPECIFICATIONS.md's sections of those parts.
- **D13:** `ResourceExtensions.asi` is a plugin that stays: it switches HeapAdjuster, PackfileLimit, Blips and RayTracing off.
- **D14:** SirenSettings a phase of its own after phase B (D22, D23).
- **D15:** the first StaticBounds, built on the size lookup's call, left 2.6.0 before its tag because that pattern is not in Enhanced (D24).
- **D16:** the ray tracing buffer's research and part, as built in SPECIFICATIONS.md; LuminanceV's documents may be read for a pass.
- **D17:** RageOpenV's check for packs made for GTA V Legacy.
- **D18, stands:** parts built in one round ship together as one version, tagged on the commit I boot and pushed with main.
- **D19, phase C's shape** (2026-11-01): two builds, as phases A and B went.
  **2.7.0:** six research passes side by side (D20); beside them, since they need nothing from the passes, the data window (D21), the siren setting IDs (D22), StaticBounds anew (D24) and the open issues the round takes (D26); then, as each pass's findings are checked and I have decided on them, a measurement for each limit they confirm (D25) and the sirens per vehicle if approved in time (D23), as the ray tracing buffer joined phase B - what is not ready then waits for 2.8.0.
  My boots, the tag on the commit I boot, main and the tag pushed together, for my own use.
  **2.8.0:** the raises of the population's limits and of the lighting capacity that 2.7.0's numbers ask for, planned from those numbers as phase B was planned from 2.5.0's.
  Neither publishes anything: the next version I publish stays the full one (D7).
  A fresh overview and a fresh merge.
- **D20, the research passes:** the limit on reverse-engineering the game (D2) is lifted of D19, for three things: the count of the original's two static bounds patterns in the dump and nothing else, done (D24); the sirens' second pass, with the lighting capacity; the population's limits.
a pass that finds its limit sharing code with a neighbour's may read the neighbour's findings file, never write it.
  The passes take the dumps of 2026-10-29, kept for them (the game is still 1.0.1158.16, PE timestamp 0x6AA45F10, checked on 2026-11-01); each begins from a copy of `work-wcloader`'s analysed Ghidra projects, whose project opened in 15 seconds where a new analysis takes about 30 minutes (tried on 2026-11-01).
  The clean room stays D2's: the lighting capacity and the population's limits come from the page of Resource Extensions [Enhanced] 2.4 by Koukol, its words alone (ROADMAP.md), nothing else of that page recorded, its plugin never read, its name in CREDITS.md once a part takes the idea; SirenSetting Limit Adjuster stays idea only.
  The overview checks each findings file on paper, its review beside it; the game-side check is the measurement's boot.
  Two choices of the overview, kept: the ped pool's pass also asks where a pool's memory comes from and how much room it has - not settled in PATTERNS.md, and my `AnimStore` test of 2026-10-30 ended the game with a failed allocation at RVA 0x15A514, 0x44 bytes past the start of the allocator of D24 - since that pool's raise would be the largest and StaticBounds' maximum depends on the answer too; and the lighting capacity's pass may read LuminanceV's FRAME.md, as D16 allowed the ray tracing buffer's.
- **D21, the data window** (D14's call, mine of D19): a test-build key that saves named windows of the game's `.data`, since the sirens' pass needs the parser's member data, which the dump leaves out.
  `TEST_DUMP_DATA=<seconds>,<rva>:<bytes>[,<rva>:<bytes>...]` under `[TWEAKSV]`, the RVAs and the sizes in hex, read by a test build alone: at most 16 windows of at most 64 KB each, each wholly inside the section the game's section table names `.data` - else a problem and nothing saved - each saved at TweaksV's start and that many seconds later, as `GameData-<rva>-start.bin` and `GameData-<rva>-later.bin`, in `%LOCALAPPDATA%\TweaksV` through the own-file checks, never in the game folder, and deleted with the dumps.
  `.data` as a whole stays out, since it may hold my data; a window is named for the game's static parser data, which holds none.

  The key's form and its caps are the choice, kept.
  One boot of mine, of about three minutes (The boots).
- **D22, the siren setting IDs:** built now, in 2.7.0 of D19, though they do not bind on my load (33 of 255 settings, none colliding): the design has been ready since 2026-10-29, my boots confirmed its patterns, it needs nothing of the raise, and silent collisions grow with bigger collections.
  The part is `SirenSettings`, `[SIRENSETTINGS]` with `ENABLED=1` and `LARGE_SIREN_IDS=1`, no size, as the findings' section 8 has it; no old plugin, since `SirenSetting_Limit_Adjuster.asi` is a Legacy plugin (D9).
  The module `TweaksVSirenIds` becomes the part's, while the siren count's stays in `kMeasuredFirst` until a raise is built.
  Its measurement goes on with `DIAGNOSTIC_MODE` with `LARGE_SIREN_IDS=0` and with the part off, through that table as the ped props' does while that part is off, so that my boot can compare the base game's settings and IDs off and on, which settles the findings' open question, what the PSO loader makes of a wider member: the same numbers, and the design stands; not the same, and the track's fallback comes in, the two members widened only after the base loader has returned, binary packs keeping their 8-bit IDs.
  The default, on or off, follows that comparison; until then the default file says 1, the choice, kept.
- **D23, the sirens per vehicle** (D14): the second pass first.
  It closes what the first left open - where the bone structure is made, the parser's data, a 21st light - and gives a verdict: about 100 sites would make this the largest part TweaksV has, and the largest surface at each game update, so the findings also say what the smallest raise that lights a 32-siren model's lights would take.
  I decide on those findings whether it is built, `SIRENS_PER_VEHICLE`, 20 to 32: with 2.7.0 if approved in time, else with 2.8.0, or not at all.
  The measurement stays as it is until then.
- **D24, StaticBounds anew**: the part through the original's two hooks, the way it goes on Enhanced, which D15 had the wrong way round.
  The overview read the original at 61507bc, still its newest commit on 2026-11-01, and counted its two patterns in both dumps of D19: the named pool's set-up, 37 bytes with a wildcard at +7, once at RVA 0x15A3F0; the pool's allocator, 41 bytes with a wildcard at +5, once at RVA 0x15A4D0.
  What D15 settled stays: GPL-3.0-or-later, the key `STATICBOUNDS_SIZE` under `[STATICBOUNDS]`, 1 to 100,000, 16,000 by default, a larger `gameconfig.xml` value kept, the two old plugins' names in the conflict check, the credit and the notice.
  The part: `C-E-P1` and `C-E-A1` (the old `C-E-H1` meant another site), each looked for once and named when held twice; both hooked in `DllMain` through `HookFunctions`, all or none.
  Each match is its function's start, which its pattern pins - the set-up's down to its `ret`, the allocator's from its pushes on - so what `IsFunctionStart` says of it is a line of `DIAGNOSTIC_MODE`, a fact for my boot, and refuses nothing: the choice, kept, since its count read nothing else of the dump and the original checks no more.
  The set-up's hook gets the pool, its name, a third argument, the stride and the allocator: a pool of stride 16 whose name, copied through `CopyMemorySafely`, is `StaticBounds` is noted.
  The allocator's hook gets the pool: the noted pool's capacity at +0x18, with its stride at +0x1C checked again, is raised to the key before the game's allocator runs, and the sizes are logged once.
  Both hooks stay on, where the original takes them off after the first allocation - the choice, kept: the set-up's hook tests the stride before it touches the name, the allocator's compares one pointer, no thread of the game is frozen in the middle of its start, and a pool made a second time is raised as well.
  Never silent: a pattern missing or a hook not made is a problem with nothing changed; the pool not seen within 60 seconds of the session's start is a problem with the part's box, since it keeps the game's size then - phase B's check, its clock the first script program the game places, with its events, its test switches and `IsPluginInstalledFor`, which come back from the git history at 66cd684.
  The original's other routes - the size lookup's call, whose load `BA 06 E4 DF 2C` is not in the game, and hooks on the lookup - stay out: a part does not try a second way.
  `DRY_RUN` hooks nothing.
  With `DIAGNOSTIC_MODE` the two sites, every making of the pool and its time after TweaksV's start.
  No research in the dump: a fact the original's source lacks goes up as a question.
  The source at 61507bc, about 95 KB, was fetched of D19 into the local runs folder, where the state says, so that the track reads it there and needs no download.
- **D25, the population's limits and the lighting capacity, measured first:** for each limit a pass confirms and I want measured, a module as phase A's (D3) - with `DIAGNOSTIC_MODE=1` only it finds every pattern of the limit, checks each site as a writing half would, logs every site and starts its measurement through `StartMeasurement`, and it writes nothing unless changed on its findings; an entry of `kMeasuredFirst`, no section in `TweaksV.ini`, no entry of `kFeatures`, no old plugin.
  Their letters and module names come with the findings.
  A limit a pass finds absent, or not binding, goes no further then.
  A raise is 2.8.0's, from my boot's numbers.
- **D26, the open issues:** T-LEGACYWALK, T-COPIES, T-NAMES and T-TESTGAPS go as their entries in NOTES.md say.
  T-LATEBOX and T-SUMMARY join them of D19: the guards' and the Legacy check's boxes still say a part stopped working, StaticBounds' check would add another late box, and T-SUMMARY's own entry says it goes with the next change to that file.
  T-PEDPROPS stays out until changed; T-HARDEN, T-DIAG and T-FREEZE stay as they are.
  They go into two sweep tracks (D29), which replace the nine tracks of T-COPIES and the four of T-TESTGAPS this decision first had.
- **D27, the scaffold and the reflow, the two commits before the tracks start.**
  The scaffold holds only what the tracks cannot do alone: `TweaksV.ini`'s layout 5 (`VERSION=5`) with its 42 keys - `[SIRENSETTINGS]` with `ENABLED=1` and `LARGE_SIREN_IDS=1`, then `[STATICBOUNDS]` with `ENABLED=1` and `STATICBOUNDS_SIZE=16000`, both after `[RAYTRACING]` - in the settings table, in `test\expected_default.ini` and in the files that pin the layout; the features' table's two new parts, each start a stub in its track's own files, `InitSirenIds()` and `InitStaticBounds()` in a new `TweaksVStaticBounds.cpp`, which changes nothing until the track fills it; the two old plugins' names on StaticBounds' entry; the specification's sections 11.14 SirenSettings and 11.15 StaticBounds, the parts measured first moved to 11.16, every reference moved; the anchors of "The tracks"; and one command for a single suite in `test\run_all.sh`, a few lines, since every track needs it from its first minute.
  Its checks are the Release and test builds without a warning and the suites that pin what it changed; the full run waits for the round's proof.
  A patch of the scaffold a cancelled merge wrote for the old split, its builds clean and its full run never finished, is kept locally: the state says where, and the merge takes from it what still fits.
  The order of the sections and the stubs' places are the choices, kept, as D11's were.
  The test keys are the scaffold's too, a point the overview put to me on 2026-11-02 and I let stand: `TEST_DUMP_DATA` (the data window) and `TEST_TWICE_PATTERN` (the tests' sweep) each need a row of `kSettings`, a named reference and a place in the test build's header line (SPECIFICATIONS.md 4.1 and 6.2), and no track owned those lines, so two tracks would have edited them; the scaffold adds both rows and the line, as shared tables, and the tracks read them.
  The reflow follows in one mechanical commit on line length (D29): NOTES.md, README.md, PATTERNS.md, SPECIFICATIONS.md, PLAN.md, ROADMAP.md, CREDITS.md, THIRD_PARTY_NOTICES.md and SECURITY.md to one sentence per line, tables and headings as they are, each bullet's first sentence on its first line.
  A script proves the rendered text unchanged, comparing each file with its line breaks taken out, and the patterns suite must still pass and the grep `run_all.sh` reads NOTES.md's count with must still find it.
- **D28, the cleanup:** `work-rtbuffer` went on 2026-11-01 of D19, moved to the Recycle Bin.
  `work-wcloader` and the dumps stay until the last pass of this phase is done and its measurement booted; then they go with the passes' work folders and the data windows, as my research tools' README has it.
- **D29, the workflow** (2026-11-01).
  My rules for every project changed: the fastest split in place of the finest, plans of about ten lines, tracks that test only what they touch and write no shared document, one scripted merge, one review and one proof on the merged tree, state files of about 5 KB, one sentence per line, a message only when a track is done or blocked.
  The full run alone was the clock: 45 minutes at 1,247 checks, the same beside ten tracks, since it runs its 19 suites one after another on 32 threads.
  So `test\run_all.sh` becomes parallel in this round, each suite in a folder of its own, with a table of which suites a changed file needs: the first half of the tests' sweep; the command for a single suite comes earlier, with the scaffold (D27).
  The twenty-five tracks this plan first had at the go became eleven: six for the research, as they were, and five for the coding - the data window, static bounds, the siren IDs and two sweeps.
  That split is the choice of the overview that planned it, kept.
  This round is also the test of the new rules, with a fresh overview and a fresh merge: what they show goes into my rules afterwards.
- **D30, the lighting capacity and the render queues, on their findings** (2026-11-02), 1 to 4 to the four decisions in the lighting capacity's summary, 5 to 8 to the four in the render queues'.
  Both measurements go in 2.7.0.
  The lighting capacity's has one write, D25's rule lifted for this alone: the jump at RVA 0xE15711 through a counting stub, with `DIAGNOSTIC_MODE=1` only, as its review's point 6 has it; the jump's target reads no flags, so the stub's `lock inc` is safe (its findings, 6.5).
  If D23 says yes, the stores' move (its 8 A) goes in the same build; otherwise 8 A and 8 B are 2.8.0's, whatever the numbers say.
  An array seen past its capacity is a line of the log, no box.
  `[PEDPROPS]` stays exactly as it ships, off, its comment word for word, and T-PEDPROPS stands; the proof's new step (The boots) needs nothing of that part.
  `StreamPed render data` and `PedProp render data` are a caution for gameconfig questions, no test.
- **D31, the two model budgets and the ped pool, on their findings** (2026-11-02).
  The two budgets' measurement goes in 2.7.0, one track and one module for both: `TweaksVPopulation.cpp` and `.h`, `MeasurePopulation()`, the tag `Population`, `test\run_population.sh`.
  `VehicleMemoryMultiplier`, `PedMemoryMultiplier` and the `PedVarietyMultiplier` start wait for the boot's numbers, with nothing to build.
  The proof's drive stays about ten minutes, going on only while the budgets' line still grows.
  The ped pool's measurement goes in 2.7.0; the pool is not raised, 2.8.0 only if the numbers ask, and `Peds` stays at 256 in my file, a caution, no test.
  The pool memory budget is raised in 2.7.0, D25's rule lifted for this alone: `POOL_MEMORY` under `[HEAP_ADJUSTER]`, in MB, 136 to 2047, 256 by default, one immediate written as `H-E1`'s is and when, its pattern the part's, `H-E2` (the ped pool's `N-E-B1`'s bytes); another value than 136 found there is another mod's, left and logged.
  Layout 5 goes to 43 keys, unreleased, so no new layout number.
  The pool memory's line is HeapAdjuster's measurement, and "nearly spent" is a line of the log, no box.
  The ped pool's track also takes the raise, since they share the site, and its module gets the address from the part, a pattern being used once.
  One reader of a pool's pointer serves the ped pool and the render queues, which decode alike, in shared code on the merged tree before they start.
  The letters are settled: G the lighting capacity, U the render queues, O the population (the vehicle pass's `V-E-` and the ped pass's `H-E-` rows alike, `O-E-V1`, `O-E-P1`, the shared site once, `H-E-C1`'s longer form), N the ped pool, `H-E2` the budget; the sirens' stay I.
  PATTERNS.md's "Not settled: ... the heap the pool's allocator draws on" goes, with the ped pool's section 3.
  Each pass's review, beside its findings in my research tools' folder, has what its track takes in its "For the coding track".

## The items



**Every research pass** (D20): a work folder of its own, `work-<topic>` in my research tools' folder, begun as a copy of `work-wcloader`'s `ghidra`, `ghidra_quick` and `game\GTA5_Enhanced.exe`, with `TWEAKSV_WORK=work-<topic>`.
The findings file has the sections of the earlier passes, opens with a one-page summary and keeps a State section after every step: the page and the clean room, the limit in Enhanced, the sites with their candidate patterns and complete sets, the facts no pattern protects, the measurement, what a mod load needs, the design answers, the hand-over.
Each fact has its evidence, a hint marked as one.
The tools are those of my research tools' README; a question of mine goes.
What each asks:

- **The sirens' bones and parser** (`sirencount2`, D23), from `sirencount_findings_for_the_clean_room_build.txt`, its sections 3.7, 3.8, 4.3, 4.5 and 8:
  where the game allocates and fills a model's bone structure - the fill may lie in the protected code - and what it would take to give `siren21` to `siren32` and their glass a place the 13 read sites reach, or a table of the part's own;
  the parser's member data of the lights' array, at RVA 0x28EE4C8 on the first pass's hint, and the structure's size as the parser keeps it, through the data window (D21), and what a binary `carcols.ymt` does with a longer array;
  what the parser does today with a 21st light;
  the stored damage state of 20 bytes (RVA 0x1548A80);
  every one of the first pass's 82 sites checked again against those answers;
  a design by layer for `SIRENS_PER_VEHICLE`, 20 to 32, and the smallest raise that lights a 32-siren model's lights, with what each layer left out costs and what a game update does to it;
  a verdict I can decide on.
- **The lighting capacity** (`lightcapacity`): how many lights the game keeps for a frame, in which lists and stores, what becomes of a light past them, and whether many vehicles' siren lights reach them - 20 a vehicle, 32 after a raise - the ray traced path included; its measurement; the design of a raise.
- **The vehicle model budget** (`vehiclemodelbudget`): the budget of vehicle models the game keeps streamed for its traffic - the key `VehicleModelBudget` on the page, 0 for the game's own, raised with care for the video memory: where it is, what sets it, what happens past it, whether add-on vehicles reach it; its measurement; the design of a raise.
- **The ped model budget** (`pedmodelbudget`): the same for ped models - the page's fewer repeated ped models.
- **The ped pool** (`pedpool`): the pool of peds, on the page from 256 to 768 slots with 128 kept for missions, peds and animals sharing them: where its size and the missions' share are set, whether `gameconfig.xml` names one, every array and loop sized by it, what the game does when it is full; its measurement; the design of a raise; and D20's question of a pool's memory.
- **The render queues** (`renderqueues`): the queues that hold streamed peds and vehicles for rendering: where they are, how large, what is dropped past them, whether a load reaches them; their measurement; the design of a raise.

**Every coding track,** each item its own commit:

1. The module: its part's code - the checks first, all or nothing, then the writes - its patterns in the table and their rows in PATTERNS.md, its keys read through the settings table, its lines in the wording of the specification's 6.3, `DRY_RUN` and `DIAGNOSTIC_MODE` as the other parts have them; its files in the project.
2. The suite: its scenarios in the fake game, its line in `run_all.sh`.
   It pins at least the plain game; the part off; `DRY_RUN`; each check failing on its own, with nothing written; a site another mod changed; the writes and nothing else, every page's protection its own; the status lines.
3. The fragment, in the place my rules give it: the words for the specification's rows in 2 and 14.2 and its part's section, README's paragraph, NOTES' line and check count, ROADMAP's entry, CREDITS.md where an original gave the idea or code.

An open issue's track takes the items it needs and asks for its entry in NOTES.md to come out, in its fragment.
Before its report it makes the builds it needs without a warning and runs the suites its files touch, and it tries its new log lines against the other suites' patterns (my notes on the project's practice); shared code touches the suites of whatever uses it.

**What each track builds:**

- **Data window** (D21): the key in `TweaksVDump`, its reader and its thread beside `TEST_DUMP_CODE`'s; the integration suite's scenarios beside I16: a fake `.data` saved at both moments and each file checked against what the fake holds, a window that leaves `.data`, too many, too large, text that is no window, a hard link in a file's place, the Release build ignoring the key.
- **Static bounds** (D24): the part from the original's source and from what the git history keeps at 66cd684 - the check, its switches, the suite's frame and its parts of the fake game.
  The fake game makes its pools through a set-up and an allocator of the two patterns' bytes, the static bounds pool among them, once and in the suite's cases twice.
  The scenarios: a size from the configuration below the key and above it, another pool of stride 16 and the pool named with another stride left alone, a name that cannot be read, a capacity below 0, the pool made before TweaksV started and never, each pattern missing and twice, a hook that cannot be made with the other taken back.
  The conflict check's two names are the scaffold's; CREDITS.md and THIRD_PARTY_NOTICES.md come back through the fragment, the notice saying what is adapted now.
- **Siren IDs** (D22; its findings' sections 4.2, 4.3, 8 and 9, the review's two points, PATTERNS.md's "Siren IDs"): the ID table moved on the relocation core to a fixed 65,536 entries of one byte - the core's small change, with its tests - its four `lea` re-pointed and its four limits raised; the 11 byte loads and stores made word ones, the dedupe's loop rewritten inside its padding, the three constructors' defaults, the two parser members' types made words - TweaksV's first write to the game's data, named in the risks - and the base loader's call redirected, so that the new table is filled whenever the game fills its own; all checked first and written together.
  The index stays a byte, so 255 settings stay the most.
  The hooks of `I-E-H1` and `I-E-H2` for the drops and moves per pack, with `DIAGNOSTIC_MODE`.
  The measurement's words past 256 IDs.
  Its own `<cstdio>` out (T-COPIES).
- **Sweep: the code** (D26): T-COPIES' points, which change no line of a run; T-LEGACYWALK and T-NAMES as their entries say; for T-LATEBOX a heading per late report in `TweaksVSummary`, each saying whether a part stopped working or what it found, the last line fitting it, and the guards' refusals and the Legacy check's finding reported as findings; for T-SUMMARY the summary's thread tried again with the next problem reported.
- **Sweep: the tests** (D26, D29): `test\run_all.sh` parallel first, each suite in a folder of its own, its output and its result as they are, `SUITES` still one suite a line; a table of which suites a changed file needs; a real timer shortened by a test-build switch where a suite waits on one in a file it owns and the switch proves the same; then T-TESTGAPS' four points.
  It runs the full suite, serial and parallel, and gives both times and the same tally of checks; NOTES.md's count is the merge's, so its line is not this track's to pass.
- **The sirens per vehicle,** if approved (D23): the second pass's design.
- **A measurement** (D25): its findings' section 6, the module and its suite as phase A's.

**The merge** (the merge): the long tracks verified from their branches as they finish, a sweep's diff read itself.
All the tracks' commits cherry-picked in the order of the tables onto main, scripted; each code file checked to hold exactly both sides' changes; the anchors removed; the fragments folded into the documents in one commit; the counts summed; T-COPIES' points in `TweaksVSirenIds.cpp` (its `ReadAt` and any left) a small commit of the merge's.

fixes in one or two sweep tracks.
Then 2.7.0 set - the project's version, the specification's header example and its section 15, NOTES' Versions, ROADMAP.md's entries of the shipped parts taken out - the four builds from scratch without a warning, `run_all.sh` green, the build reported.

## The boots

**The data window's boot,** early in the round: the test build of the data window's track, which the track builds on its branch, the merge copies out and the overview hands me with the INI line naming the windows the sirens' pass asks for.
I copy it into the game folder, start the game, stay in it for the seconds the line names, quit, and put the build I use back.
The overview reads `TweaksV.log`; the pass reads the files.

**2.7.0's proof,** on the Release build of the tagged commit, with `DIAGNOSTIC_MODE=1`, my usual mods folder, my converted emergency packs, a save loaded and about ten minutes of driving and shootouts with a wanted level in the city, which is what the population's measurements need.
The drive goes on only while the budgets' line still grows (D31).
A start with `LARGE_SIREN_IDS=0` and one with `LARGE_SIREN_IDS=1` compare the base game's settings and IDs (D22), and a pack's ID past 255 kept whole; its emergency vehicle shows its own lights.
`STATICBOUNDS_SIZE` above what my `gameconfig.xml` gives the pool - 24,500 at my second test, so 30000 - since a larger value there keeps the default's 16,000 from changing anything: the raise's line with both sizes, the two hooks at 0x15A3F0 and 0x15A4D0.
One start at `STATICBOUNDS_SIZE=100000`, the key's maximum, in that load, since my `AnimStore` test ended the game with a failed allocation right behind that allocator's start: if it crashes, the maximum comes down to what boots, or README says so.
That start is safe only with `POOL_MEMORY`'s raise (D31), so the key goes in before it; the key's maximum stands.
The render queues' step (D30): about ten `mp_m_freemode_01` spawned in view with Simple Trainer V, some with hats, mods set on three or four cars, 30 seconds standing still; it needs nothing of `[PEDPROPS]`, which stays off.
The proof: line 1 names 2.7.0 and the commit without `-dirty`; no `PROBLEM` but what a test means; `Features:` naming the two new parts; each new pattern at its RVA; each measurement's numbers; no crash.
Then the tag, the searches, and one atomic push.
The next plan, 2.8.0, comes from those numbers.

**What my boots have shown so far** (the full account is in the git history up to 098836c):

- **2.6.0's proof,** 2026-11-01, with `VIDEO_MEMORY_BUDGET=6000`, `DYNAMIC_BVH_VERTICES=8388608`, the seven weapon packs and one Legacy pack: 166 `weapons.meta` files and 556 component definitions loaded; the budgets at most 4,155 of 6,000 MB; the objects 98,359 of `MaxStreamingResources`' 98,304; the ray tracing buffer at most 2,281,234 of 8,388,608 vertices, none refused.
  The guards' boot passed; a refused pack's weapon spawns without its magazine.
- **My load without packs,** 2.5.0, 2026-10-29: `weapons.meta` files 155 of 160; component definitions 465 of 470; siren settings 21; streamed textures 3,004 of 3,000 MB; the resource heap 10 of its 32 regions.
- **With packs,** 2.5.0, 2026-10-30: siren settings 33, their IDs 33 of 256, none given twice, an ID past 255 keeping its low byte.
  Three models past `siren20`: `lspdcvpi08` with 10 sirens and 6 glass bones never lit, the CHP FPIU with 12, half its lightbar dark, the LVMPD Crown Victoria with 10 and 6.
  The male freemode model holds 255 head props with the base game alone, the female 254; a switch past 255 crashes at `+0x5A0998D` (NOTES.md, T-PEDPROPS).
  The box streamer's largest search 1,067 of 10,000; decals 259 of 260; blips 243 of 2,000.
- **Packs:** every Legacy vehicle traps the game at `+0x127C32F` as it is created; converted with CodeWalker, most spawn.
  Two converted packs crash in the game's texture and buffer code, where TweaksV writes nothing: State Wide after a minute or two of driving, the SFPD collection's police6 seconds after its spawn.
  Without them the game ran seven minutes with the CHP FPIU, the LVMPD Crown Victoria and the CVPIs.
- **`gameconfig.xml`:** `AnimStore` at NaturalVision Enhanced's 21,200 boots, and at 32,000 every load crashed at `+0x15A514`, a pool's failed allocation.
  `MetaDataStore` full at 3,200 of 3,200 traps the game at `+0x1585E0`.

## Next steps

**State:** phase C's round runs since 2026-11-02, the split of D29 kept.
The scaffold (D27) is on main: `TweaksV.ini`'s layout 5 with its 42 keys, the two new parts as stubs, the test keys and the anchors; nothing of the tracks is built.
2.6.0 stands as the last build, tagged on c6c8854 and pushed: thirteen parts, layout 4 with its 38 keys, 143 patterns, 1247 checks.
The reflow is on main too;
the sirens' pass waits for my boot of the data window.
The open issues are T-HARDEN, T-DIAG, T-SUMMARY, T-FREEZE, T-LATEBOX, T-LEGACYWALK, T-COPIES, T-NAMES, T-TESTGAPS and T-PEDPROPS.


