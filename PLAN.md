# Plan

The order of work for a change to TweaksV, phase by phase, each ending with its proof, a commit and my boot.
What came before phase C is in the git history and in NOTES.md's Versions; the passes' findings and reviews are in my research tools' `output-1158.16\`.
Phase C, planned on 2026-11-01, is two builds (D19): 2.7.0, with six research passes side by side, StaticBounds anew through the original's two hooks, the siren setting IDs, the population's limits and the lighting capacity measured, and the open issues the round takes; then 2.8.0, the sirens per vehicle (D23) and the raises 2.7.0's numbers ask for, planned from those numbers.

## How the work runs

The work runs as my rules for every project have them since 2026-11-01 (D29): the three levels of tracks, the fastest split, short plans, one proof on the merged tree, small state files, few messages.
This section holds only what is this round's.

- **The overview** (the overview) reads the weekly limits and starts the merge.
  It checks each research pass's findings from their summary, on paper, before anything is built on them (D20), and brings me the decisions they ask for.
  It has the merged diff reviewed once, asks for my boots, reads them, tags 2.7.0 and pushes.
- **The merge** (the merge) makes the scaffold and the reflow on main (D27), writes one short plan per track and starts the tracks in its track.

- **Eleven tracks at the go** - five coding tracks and six research passes - each on a branch of its own from main at the reflow's commit.
  The later ones - one measurement for each limit its findings confirm (D25) - start from main once the overview has checked the findings and I have decided on them, each.

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
| Sweep: the tests | | shorter | `test\run_all.sh` and `test\common.sh`; `TEST_TWICE_PATTERN` in `source\TweaksVScanner.cpp`; the test build's line in `source\TweaksVLog.cpp`; TB16's block in `test\run_texturebudget.sh` and its two writers in `test\fake_game_full.cpp`; its block in `test\run_legacypacks.sh` | the suites made fast (D29) and T-TESTGAPS' four points, none an hour's work alone |

**The coding, later,** each once the overview has checked its findings and I have decided on them, each; a module's name may change with its findings:

| Track | Track | Owns | Joined by |
|---|---|---|---|
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
- **D9, stands, the conflict check:** it names only plugins made for Enhanced, Legacy names left out; `WeaponLimitsAdjusterEnhanced.asi` is an old plugin of WeaponLimits and `GTAV.ResourceAdjusterEnhanced.asi` one of Resources.
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
  **2.7.0:** six research passes side by side (D20); beside them, since they need nothing from the passes, the data window (D21), the siren setting IDs (D22), StaticBounds anew (D24) and the open issues the round takes (D26); then, as each pass's findings are checked and I have decided on them, a measurement for each limit they confirm (D25), as the ray tracing buffer joined phase B - what is not ready then waits for 2.8.0.
  My boots, the tag on the commit I boot, main and the tag pushed together, for my own use.
  **2.8.0:** the sirens per vehicle (D23), and the raises of the population's limits and of the lighting capacity that 2.7.0's numbers ask for, planned from those numbers as phase B was planned from 2.5.0's.
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
- **D23, the sirens per vehicle** (D14), on the second pass's findings (2026-11-02).
  They are built in 2.8.0, `SIRENS_PER_VEHICLE`, 20 to 32, as the hybrid of the findings' 8.7: the lights by a second call of the game's own renderer, the damage by the game's own code raised, glass included; 29 patterns and 26 writes, which have not run on the game.
  The lighting capacity's 8.1 A and C go in that build, A as D30 has it, C new: without it three fully lit vehicles at 32 sirens in one job lose lights silently.
  So 2.7.0 has no siren track, and the measurement stays as it is until then.
  The key's default is for 2.8.0's plan.
- **D24, StaticBounds anew**: the part went through the original's two hooks, which D32 replaces.
  What D15 settled stays, but for the key's maximum: GPL-3.0-or-later, the key `STATICBOUNDS_SIZE` under `[STATICBOUNDS]`, 16,000 by default, a larger `gameconfig.xml` value kept, the two old plugins' names in the conflict check, the credit and the notice.
  Never silent: a pattern missing or a call that cannot be redirected is a problem with nothing changed; the store's size not asked for within 60 seconds of the session's start is a problem with the part's box, its clock the game placing blip_controller, with its events, its test switches and `IsPluginInstalledFor`.
  `DRY_RUN` redirects nothing.
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
  They stay for now.
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
  8 A, the stores' move, and 8 B are 2.8.0's, whatever the numbers say, with the sirens per vehicle (D23).
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
- **D-V5, the vehicle budget raised with no code** (2026-11-02): `VehiclesSP`'s `Level3Kb` at 98,500 in my `gameconfig.xml`, for 600 MB in place of 429 (the vehicle pass's findings, 7.0), untested.
  It comes after 2.7.0's boot gives its baseline, with the measurement on, and goes out again if the lights' peak nears 400.
  I make the change myself; no session writes in the game folder.
  It waits for 2.8.0's stores, and by its own rule: my proof's start 5, with the texture budget raised, saw the vehicle lights at 327 of 400.
the pass (`staticbounds2_findings_for_the_clean_room_build.txt`) found the size read once, from one call.
  The part is one call redirect at RVA 0x3433E through `C-E-L1`, raising hash 0x2CDFE406 alone, the larger of the game's and the key's, in the way of `W-E-C4`; `C-E-P1`, `C-E-A1` and both hooks go; `CheckBudget` and the track check stay.
  `STATICBOUNDS_SIZE`'s maximum is 32767, past which a slot comes out negative in an interior proxy and, past 65535, two counts wrap; the default stays 16000.
  A read-back through `C-E-D1` runs on every boot, once, on the check's thread: a mismatch of the store's count, its pool and the streamer is one problem, and a missing `C-E-D1` a line, the raise going on.
  A `gameconfig.xml` value above 32767 is kept; it is one problem only when the read-back finds more than 32767 in use.
  CREDITS and the notice stay, reworded: the key's name, default and rule are the original's, the route TweaksV's own.
- **D33, `VIDEO_MEMORY_BUDGET`'s default in 2.8.0** (2026-11-02): 6000, with the light stores' raises, since my proof's start 5 had 327 of 400 lights with it on, the cause not proven.
  Past Windows' budget the game's update check evicts anyway (the texture budget's findings), on paper.
  2.7.0 keeps 0, so that the proof is not disturbed.

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
- **Static bounds** (D24, D32): the second pass's design, its last section the suite.
  The conflict check's two names are the scaffold's; CREDITS.md and THIRD_PARTY_NOTICES.md come back through the fragment, the notice saying what is adapted now.
- **Siren IDs** (D22; its findings' sections 4.2, 4.3, 8 and 9, the review's two points, PATTERNS.md's "Siren IDs"): the ID table moved on the relocation core to a fixed 65,536 entries of one byte - the core's small change, with its tests - its four `lea` re-pointed and its four limits raised; the 11 byte loads and stores made word ones, the dedupe's loop rewritten inside its padding, the three constructors' defaults, the two parser members' types made words - TweaksV's first write to the game's data, named in the risks - and the base loader's call redirected, so that the new table is filled whenever the game fills its own; all checked first and written together.
  The index stays a byte, so 255 settings stay the most.
  The hooks of `I-E-H1` and `I-E-H2` for the drops and moves per pack, with `DIAGNOSTIC_MODE`.
  The measurement's words past 256 IDs.
  Its own `<cstdio>` out (T-COPIES).
- **Sweep: the code** (D26): T-COPIES' points, which change no line of a run; T-LEGACYWALK and T-NAMES as their entries say; for T-LATEBOX a heading per late report in `TweaksVSummary`, each saying whether a part stopped working or what it found, the last line fitting it, and the guards' refusals and the Legacy check's finding reported as findings; for T-SUMMARY the summary's thread tried again with the next problem reported.
- **Sweep: the tests** (D26, D29): `test\run_all.sh` parallel first, each suite in a folder of its own, its output and its result as they are, `SUITES` still one suite a line; a table of which suites a changed file needs; then T-TESTGAPS' four points.
  No real timer's switch: it would save about 20 of 345 seconds and add a way to flake, so it is not built (2026-11-02).
  It runs the full suite, serial and parallel, and gives both times and the same tally of checks; NOTES.md's count is the merge's, so its line is not this track's to pass.
- **A measurement** (D25): its findings' section 6, the module and its suite as phase A's.

**The merge** (the merge): the long tracks verified from their branches as they finish, a sweep's diff read itself.
All the tracks' commits cherry-picked in the order of the tables onto main, scripted; each code file checked to hold exactly both sides' changes; the anchors removed; the fragments folded into the documents in one commit; the counts summed; T-COPIES' points in `TweaksVSirenIds.cpp` (its `ReadAt` and any left) a small commit of the merge's.

fixes in one or two sweep tracks.
Then 2.7.0 set - the project's version, the specification's header example and its section 15, NOTES' Versions, ROADMAP.md's entries of the shipped parts taken out - the four builds from scratch without a warning, `run_all.sh` green, the build reported.

## The boots

**The data window's boot,** early in the round: the test build of the data window's track, which the track builds on its branch, the merge copies out and the overview hands me with the INI line naming the windows the sirens' pass asks for.
I copy it into the game folder, start the game, stay in it for the seconds the line names, quit, and put the build I use back.
The overview reads `TweaksV.log`; the pass reads the files.

**2.7.0's proof,** done on 2026-11-02 (below), on the Release builds of 6042fe7 and, for what D32 rebuilt, of the tagged commit 7fa28da, with `DIAGNOSTIC_MODE=1`, my usual mods folder, my converted emergency packs and a save loaded; `TrainerV.asi` and Collectibles on Map lay beside TweaksV, which only `POOL_MEMORY` makes safe, and the police packs came by the trainer.
A start with `LARGE_SIREN_IDS=0` and one with `LARGE_SIREN_IDS=1` compared the base game's settings and IDs (D22), about half an hour of driving and shootouts with a wanted level gave the measurements' numbers, the render queues' step (D30) stands in starts 4 and 5, and the store's raise (D32) was proved below and above the game's size.
It read line 1 naming 2.7.0 and the commit without `-dirty`, the one `PROBLEM` `lfpolice`'s Legacy box, each new pattern at its RVA, each measurement's numbers, no too-late line of `H-E2`'s write, and no crash but the old build's start 3.
Then came the tag, the searches and one atomic push.
The next plan, 2.8.0, comes from those numbers.

**What my boots have shown so far** (the full account is in the git history up to 098836c):
- **2.7.0's proof,** 2026-11-02, first the build of 6042fe7, each start with `DIAGNOSTIC_MODE=1`, my usual mods folder, `TrainerV.asi` and Collectibles on Map in: line 1 clean in all of them, the one `PROBLEM` `lfpolice`'s Legacy box, all 208 patterns of the check at the RVAs of PATTERNS.md's rows.
  - **Start 1,** `LARGE_SIREN_IDS=0`, `STATICBOUNDS_SIZE=30000`: budget patched 136 to 256 MB, no too-late line, 136.0 MB in use, 119.9 free: the load uses the old budget whole, which is what the crash was. Base IDs 0 to 13; packs' IDs cut to 174, 179, 69, 209, 66, 67.
  - **Start 2,** `LARGE_SIREN_IDS=1`, `STATICBOUNDS_SIZE=100000`: passed, 137.1 MB in use; D22's comparison passes: the same 27 settings, 14 base, 13 from packs, base IDs the same, packs' IDs whole, 5550, 5555, 8005, 2001, 24130, 24131, each the low byte of start 1's.
  - **Start 3,** the pool at 8000, `STATICBOUNDS_SIZE=16000`, `SKIP_LANDING_PAGE=0`, over a minute on the page: no false box, then the crash at the save's load, `int3` at `+0x93AA93`, the store's name map's free list empty on the 8001st name, which D32 answers. The pool is made at 15.3 to 15.9 s whether or not the page is skipped, before the page and well before `blip_controller` at 42.5 to 54.7 s.
  - **Start 4,** the pool at 24,500, the key at its default, `VIDEO_MEMORY_BUDGET=0`, `DYNAMIC_BVH_VERTICES=0`, 28 minutes of bodyguard spawns of `mp_m_freemode_01` with caps, shootouts and the tunnel by the beach at night with a wanted level: no crash.
    Vehicle model budget at most 828.4 of 429 MB, at the stop in 1625 of 1716 samples, 107 models at once, 5 dropped, weapons 375.5 MB of the bytes; ped model budget at most 492.8 of 561 MB, at the stop in 0, 46 models at once, 0 streamed by the population.
    Ped pool at most 97 of 256, spatial array the same; pool memory 135.9 of 256 MB, at least 119.9 free, 213 blocks of 900.
    Lights: vehicles 53 of 300, vehicle lights 159 of 400, vehicle coronas 208 of 928, scene lights 492 of 1024, at its room with the sort list marked in 0 of 163,945 samples, ray traced list 936 of 1024, past 512 in 89,936 samples, by 424 at most, reflection lists 259 of 768, coronas 595 of 1088, a job's refused lights 0.
    Render queues: streamed peds' render data 15 of 120, requests 2 of 55, a frame's array 28 of 120, 14 different; props of peds 5 of 120, requests 3 of 50, array 6 of 120, 3 different; streamed vehicles 1 of 100, no mods set; references 3,813 of 10,368.
    Texture budget 3,017 of 3,000 MB, the most 3,076, the game's own flag up, "texture loss", my LOD switching's first suspect, measured; 50,112 of 98,304 objects; ray tracing buffer at most 3,278,783 of 4,194,304 vertices, none refused; blips 253 of 2,000; `weapons.meta` files 166, component definitions 556; decals 259 of 260; box streamer's largest 1,277 of 10,000; the siren count 6 models with sirens, 20 of 20, none past `siren20`.
    The trainer stopped answering after caps were set on freemode peds; the game ran on, no event, no crash file, the cause not known.
  - **Start 5,** the pool at 24,500, `VIDEO_MEMORY_BUDGET=6000`, `STATICBOUNDS_SIZE=16000`, 16 minutes: `hp16fpiu1` (ID 8005), `lspdcvpi08` (24130, 24131) and `policelv` (2001) side by side, sirens on, each its own pattern, every light lit but half of `hp16fpiu1`'s bar, the 20 sirens' limit, 3 models with bones past `siren20` in the log: D22's check on the game passes; three cars at max mods, no crash, the trainer's stopping not reported this time.
    My LOD switching noticeably better, a little left, by my eye: the texture budget about 4,700 of 6,000 MB, the most 5,576, the game's flag down, so the memory no longer binds; objects 98,343 of `MaxStreamingResources`' 98,304, rising from 71,727 to 98,283 in one minute of driving, so that limit now binds and is the likely rest, not proven; it stands in `ConfigGraphics` in `gameconfig.xml`, no finding on what else reads it, for 2.8.0's plan.
    Lights: vehicles 93 of 300, vehicle lights 327 of 400, vehicle coronas 439 of 928, scene lights 642 of 1024, ray traced list 1024 of 1024, past 512 in 65,887 of 94,076 samples, reflection lists 298 of 768, coronas 1088 of 1088, a job's refused lights 0: two stores at their room, and the 400 past which the game overwrites its memory within 73.
    Vehicle model budget at most 1,223.8 of 429 MB, mods up to 5.3 and 49.9 MB; peds at most 505.8 of 561, 52 models at once; ped pool at most 96; streamed vehicles 4 of 100, requests 1 of 60, array 4 of 100; references 4,571 of 10,368; ray tracing buffer at most 3,734,612 of 4,194,304, none refused.
  - **Start 3 again,** the build of 7fa28da, which is tagged, the pool at 8000 in my `gameconfig.xml`, `STATICBOUNDS_SIZE=16000`, `SKIP_LANDING_PAGE=0`: passed.
    `The size the game gives the static bounds store, 8000, is raised to 16000` at 15.4 s; the read-back: count 16000, pool 16000, name map 4621 buckets with 10406 names, streamer 16000, 10406 slots in use, its streaming base 727662.
    Past the save's load, where the old build died on its 8001st name; driving, into Ammu-Nation and out again by my eye, no falling through, no crash.
  - **The last start,** the build of 7fa28da, the pool back at 24,500, `STATICBOUNDS_SIZE=32767`, `SKIP_LANDING_PAGE=1`: passed.
    24500 raised to 32767 at 15.6 s; the read-back: count 32767, pool 32767, name map 13297 buckets with 10406 names, streamer 32767, 10406 slots in use; no crash.
  - **Both of these:** line 1 names 2.7.0 and 7fa28da, the one `PROBLEM` `lfpolice`'s Legacy box, 208 patterns found, `C-E-L1` at RVA 0x34336 and `C-E-D1` at RVA 0x1046805, each 1 match, no line of a check not made; pool memory at most 136.1 of 256 MB.
- **The data window's boot,** 2026-11-02, the test build of 498a6de on the windows: the nine windows at both moments, 18 files, none unreadable; one `PROBLEM`, the Legacy box for `lfpolice`, which stays Legacy on purpose, never spawned.
  Both budgets at 180 seconds: the variety 6.0, vehicles 71.5 MB x 6 = 429, peds 93.5 MB x 6 = 561, level 3, entry 0 of each table 1.5 MB; vehicles in use 428.7 of 429 MB, past the stop at 427.5, with 94 traffic models and 8 dropped, peds 305.1 MB: the vehicle budget looks to bind at the three-minute mark, one reading only, which the drive will say.
  A start before it, with `TrainerV.asi` and Collectibles on Map in, crashed at `+0x15A514`, a write to 0x170, `rax` 0, a pool of 23 slots of 16 bytes, room under 64 KB, a third site near 0x1AEA7FC, the ninth sample; without the two plugins it passed.

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

**State:** 2.7.0, phase C's first build, is tagged on 7fa28da and pushed, this document with it: StaticBounds anew at its one call, SirenSettings' IDs past 255, `POOL_MEMORY`, and the lighting capacity, the two model budgets, the ped pool and the render queues measured.
It has fifteen parts, `TweaksV.ini`'s layout 5 with its 43 keys, 228 patterns and 2382 checks, the full run about 9 minutes side by side.
My proof passed on it ("The boots"), and 2.2.1 stays the last one published.
All six research passes and the second one on StaticBounds are done and decided (D23, D30, D31, D32).
Next: the plan of 2.8.0, from the numbers of my proof, with what is decided: the sirens per vehicle as the hybrid (D23), the lighting capacity's 8.1 A, B and C (D23, D30), `VIDEO_MEMORY_BUDGET` at 6000 by default (D33), and the vehicle budget's raise in my `gameconfig.xml` once the stores are in (D-V5); `MaxStreamingResources`, which now binds, has no finding yet on what else reads it.
The plan also settles what those decisions do not reach: the corona store, 1088 of 1088 in my proof and in none of 8.1 A, B or C, and the ray traced list, 1024 of 1024, first come with vehicles last, whose gate's sort is not designed (the lighting capacity's findings, 8.2).
Beside them: a count of the vehicles whose siren setting ID finds no setting (SPECIFICATIONS.md, 11.14), a pack's file name, which its line gives as `?` on the game, and, the one hat that answers T-PEDPROPS.
The open issues are T-HARDEN, T-DIAG, T-FREEZE and T-PEDPROPS.

**Decisions waiting on me:** none. The round's tracks are mine to delete, and their branches go then; the dumps, `GameData-*.bin` and the `work-*` folders stay (D28).
