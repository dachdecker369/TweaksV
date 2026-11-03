# Plan

The order of work for a change to TweaksV, phase by phase, each ending with its proof, a commit and my boot.
What came before phase C is in the git history and in NOTES.md's Versions; the passes' findings and reviews are in my research tools' `output-1158.16\`.
Phase C, planned on 2026-11-01, is two builds (D19): 2.7.0, tagged on 7fa28da, and 2.8.0, the sirens per vehicle (D23) and the raises 2.7.0's numbers ask for, planned on 2026-11-03 from those numbers (D34 to D43); 2.7.0's tracks and items are in the git history up to 81f47ac.

## How the work runs

The work runs as my rules for every project have them (D29): the three levels of tracks, the fastest split, short plans, one proof on the merged tree, small state files, few messages.
This section holds only what is this round's.


  It has the merged diff reviewed once, asks for my boots, reads them, tags 2.8.0 and pushes.
- **The merge** (the merge) makes the scaffold on main, writes one short plan per track and starts the tracks in its track.

- **Three coding tracks at the go,** each on a branch of its own from main at the scaffold's commit.
  The sweep and, if its findings allow it, the corona store start from main once the overview has checked the findings and I have decided on them, the same go if those are in by then.

Every coding track builds from what its item names: the findings file in my research tools' folder, `C:\_DEVELOPMENT\_Tools\TweaksV-research\output-1158.16\<topic>_findings_for_the_clean_room_build.txt`, with the decisions below where they change the design and the review beside it, `<topic>_review.md`.
No track reads code of an original whose license does not allow it (D2) - Resource Extensions' plugin and SirenSetting Limit Adjuster's code least of all.
Only the research passes analyse the dump (D2, D34): a fact a coding track lacks goes up as a question.
No committed file and no commit message names a local folder or a branch.

## What the numbers ask

From my proof's start 5 unless said; starts 4 and 5 read minute by minute, 32 minutes in all, for how often a peak comes.

| Limit | The most | Verdict |
|---|---|---|
| The frame's vehicle lights | 327 of 400, unchecked past it | Moved (the lighting capacity's 8.1 A, D30) |
| The frame's vehicle coronas | 439 of 928, unchecked past it | Moved (8.1 A) |
| A job's lists | 0 refused of 80 lights; coronas not seen | Raised to 180 and 300 (8.1 C, D23) |
| The scene store | 642 of 1024, marked in 0 samples | Not raised (8.2); its byte index fixed (8.1 B, D30) |
| The corona store | 1088 of 1088 in 1 minute of 32, 317 to 917 else | Raised to 4096, the key `CORONA_STORE` (D35) |
| The ray traced list | 1024 of 1024 in 2 minutes of 32, 531 to 945 else | A count of what it refuses, no sort (D36) |
| The ray traced upload | past 512 in nearly every sample | Cannot be raised (8.2) |
| `MaxStreamingResources` | 98,343 of 98,304, in one minute of driving | A key, 131072 by default on my proof's number (D37) |
| The texture budget | about 4,700 of 6,000, the most 5,576, the flag down | 6000 by default (D33) |
| The vehicle model budget | at the stop 95% of the time, 1,224 of 429 | My `gameconfig.xml` (D-V5), once the stores are in |
| The ped model budget | 505.8 of 561, at the stop in 0 of 1716 samples in start 4 | No raise (D43) |
| The ped pool | 97 of 256 | No raise (D43) |
| The render queues | peds 15 of 120, props 5 of 120, vehicles 4 of 100, references 4,571 of 10,368 | No raise (D43) |
| The ray tracing buffer | 3.73 of 4.19 million vertices, none refused | No change; watched in my proof, since D-V5 brings more vehicles, and its key is there |
| The siren count | 3 models past `siren20`, half a lightbar dark | The hybrid (D23) |

## The tracks

The fastest split the work allows (D29): the sirens' design is the longest item by far and falls apart at its one seam, the room of its findings' 8.7, which the second call reads only as offsets; small items together in a sweep.
Each table is in the order of the merge; the lengths are rough.

**The coding, at the go:**

| Track | Track | Length | Owns | Joined by |
|---|---|---|---|---|
| Light stores | | about 2 to 3 hours | `source\TweaksVLightCapacity.cpp` and `.h`; `test\run_lightcapacity.sh` | the part's 8.1 A, B and C, its measurement over the moved stores, a job's most and the ray traced list's count: one module, since a pattern is used once |
| Siren lights | | about 3 hours | `source\TweaksVSirenCount.cpp` and `.h`; the accessor of `I-E-C1` and `I-E-P1` in `source\TweaksVSirenIds.cpp` and `.h`; `test\run_sirencount.sh` | the findings' 8.3 A, B and C, one wrapper over one table; the part's one write; D41's guard, at the two getter calls |
| Siren damage | | about 2 hours | `source\TweaksVSirenDamage.cpp` and `.h`, new; `test\run_sirendamage.sh`, new | the findings' 8.7: the room, its fill and its 19 damage writes, constants and one redirected call, all found and checked before the part's one write |

**The coding, later,** each once the overview has checked its findings and I have decided on them, each:

| Track | Track | Length | Owns | Joined by |
|---|---|---|---|---|
| Sweep | | about 2 hours | `source\TweaksVTextureBudget.cpp` and the rows of `VIDEO_MEMORY_BUDGET` and `STREAMED_OBJECTS` in `source\TweaksVSettings.h`, with their lines of `test\expected_default.ini`; `test\run_texturebudget.sh`; `I-E-H1`'s region of `source\TweaksVSirenIds.cpp` and its block in `test\run_sirenids.sh` | D33's default, the object limit's key and its walk, a pack's file name and D41's count: small items, and together they stay under the sirens' three hours |
| Corona store | | about 2 hours | `source\TweaksVCoronaStore.cpp` and `.h`, stubs on main before it starts; `test\run_coronastore.sh`, new | its own module, since the light stores' track runs then: one call and one accessor between the two, `FindCoronaStore` and `CoronaStoreNow`, which both tracks take from main (D35) |

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
- **D34, 2.8.0's plan and its order** (2026-11-03), 1 to 10 to the ten points, which D34 to D43 carry; its first asked for the research now and the coding after the weekly limit's reset on 2026-11-05.
  So nothing waits for the reset: research and coding run side by side, what needs a pass's findings starting once they are checked and I have decided on them.
  The limit on reverse-engineering the game (D2) is lifted by that word for three passes, `streamedobjects`, `coronastore` and `sirenfacts`, and nothing else.
  The choices kept at that word: the sirens' one track split in two at the room; the corona pass carrying two small facts for the light stores; three passes, since a pass on the ray traced list's gate falls away with D36; `S-E-C0n` and `S-E-D0n` named `I-E-L0n` and `I-E-N0n`, since `I-E-C3` and `I-E-D2` are the IDs' already.

- **D35, the corona store** (2026-11-03, on `coronastore`'s findings): raised in this build to 4096: 1088 of 1088 in 1 of the 32 minutes of my starts 4 and 5, 13 to 16 vehicles at 32 sirens take it past that, and the vehicles' coronas are added last in a frame, so theirs are the ones dropped.
  The key `CORONA_STORE` under `[LIGHTCAPACITY]`, 4096 by default, 1088 to 8192; at 1088 nothing of it is looked for or written.
  The counters under `DIAGNOSTIC_MODE` (the findings' 7.1 and 7.2), beside a job's most and the ray traced list's count; the LOD job's own 1088 left as it is.
  A corona pattern missing refuses the part as a whole, since the job's clamp must move with the store or the game faults, which with D39 holds the sirens at 20, and the line says why.
- **D36, the ray traced list:** no sort in 2.8.0, since it was full in 2 of those 32 minutes and a light past it still lights surfaces directly (the lighting capacity's findings, 3.4 and 8.2); a count of what it refuses under `DIAGNOSTIC_MODE`, by a stub as `G-E-C1`'s, and the sort only if my proof's number asks for it.
- **D37, `MaxStreamingResources`** (2026-11-03, on `streamedobjects`' findings).
the proof's start of the object limit is that test.
  The key `STREAMED_OBJECTS` under `[RESOURCES]`, its default 131072 on my proof's descriptor number, 0 if fewer than 16,384 are free at the highest reading, its maximum 147456, 3/4 of the heap read at run time; a `gameconfig.xml` value above that is left as it is, with one problem line.
  The descriptor walk under `DIAGNOSTIC_MODE`, with `Q-E-T6` and `Q-E-T7`; no raise of the heap; no watch, so the heap's end stays an open issue in NOTES.md.
- **D38, `SIRENS_PER_VEHICLE`'s default** (D23): 32, proven on the tagged build by my proof; a vehicle with no lights past 20 pays one lookup.
- **D39, the light stores a part:** the sixteenth, `[LIGHTCAPACITY]`, `ENABLED` and `CORONA_STORE` (D35), no key for the stores' and the lists' sizes, since they stand at the top of what the worker's scratch allows, off beside Resource Extensions.
  `SIRENS_PER_VEHICLE` above 20 refuses with a problem when the stores are not raised, and the vehicles stay at 20 (the sirens' findings, 8.3).
- **D40, a build check on the sirens' raise:** no pattern proves that the renderer reads nothing new of its stand-ins, and on other people's PCs nobody reads it again after a game update; so the raise runs only on game builds its renderer was read on, today 1.0.1158.16, through `ReadGameVersion`, with `PROBLEM: The sirens per vehicle are not raised this session: ...` on any other.
  A game update costs players the raise until a new TweaksV.
- **D41, a siren setting ID that finds no setting** (SPECIFICATIONS.md, 11.14, risk 3), on `sirenfacts`' findings, at the same word of 2026-11-03: both getter calls guarded while the getter is redirected, `I-E-Z1` +24 and `I-E-B1` +24: a position past the array's count gets the empty setting, zeros with +0x20 = 200, no lights.
  A pack's file name by `I-E-H3`, and the count at load through `I-E-H4` under `DIAGNOSTIC_MODE`, which names each vehicle once, no longer on the measurement's wrapper.
  With `SIRENS_PER_VEHICLE=20` the getter is not redirected and the count stands.
- **D42, the hat of T-PEDPROPS:** out of this round unless I have a small clean single-prop pack by the proof, then one start of its own; no code unless it passes, and then one default.
- **D43, no raise of the ped pool, the render queues or the ped budget:** my proof's numbers do not ask for one (D30, D31).

## The items



- `[LIGHTCAPACITY]` with `ENABLED` in `kSections` before `[SIRENSETTINGS]`, its row of `kFeatures` with Resource Extensions as its peer and a stub; `SIRENS_PER_VEHICLE` under `[SIRENSETTINGS]`, 32 by default, 20 to 32; layout 5 stays, unreleased, as D31 had it, at 45 keys, 47 with `CORONA_STORE` and `STREAMED_OBJECTS`; `test\expected_default.ini`.
- `bool LightStoresRaised()` declared, a stub.
- The part's one write as a skeleton in `TweaksVSirenCount`: D40's check, the question to `LightStoresRaised()`, then `FindSirenLights` and `FindSirenDamage` each adding to one list of writes, then one `WriteCode` and one `RedirectCalls`, so that each siren track runs its suite with the key on, on its own branch.
- The room's constants in one header, 0x34C, 0x36C and 0x38C; the fake game's model with S at 0x38C and the room filled, so that the second call's suite does not wait for the fill, which `integration` proves on the merged tree.
- An anchor where two tracks write in one table: `source\TweaksVPatterns.h`, PATTERNS.md, `source\TweaksV.vcxproj` and `.vcxproj.filters`, `test\fake_game_full.cpp`'s regions, `test\run_all.sh`.

Its checks are a build and the suites that pin what it changed; the tracks start right after it.

**Every research pass** (D34): a work folder of its own, `work-<topic>` in my research tools' folder, begun as a copy of `work-wcloader`'s `ghidra`, `ghidra_quick` and `game\GTA5_Enhanced.exe`, with `TWEAKSV_WORK=work-<topic>`; a findings file with a one-page summary and a State section; each fact with its evidence, a hint marked as one.
What each asks:

- **The streamed objects** (`streamedobjects`, D37): every reader of the most objects, RVA 0x481D0A0, and of the configuration's +0x54C; what the placement does at the limit; whatever else counts or is sized by the same objects, so the first hard limit past 98,304 and a safe top; my `gameconfig.xml` for my load, and whether other people's want a key beside `VIDEO_MEMORY_BUDGET`.
- **The corona store** (`coronastore`, D35): every source that adds to the store at RVA 0x41B4210 and their order in the frame; its readers on to the screen and everything sized by 1088; the smallest raise that puts more coronas on screen, or why not.
  With it, two facts for the light stores: the two copies at a job's end, as the place to see a job's most, and whether the target of `G-E-F1`'s jump reads flags.
- **The siren facts** (`sirenfacts`, D41): where a pack's file name is reachable from `I-E-H1`'s arguments; the getter in full, what it gives for position 0xFF, and whether the renderer takes a setting of all zeros safely.

**Every coding track,** each item its own commit:

1. The module: its part's code - the checks first, all or nothing, then the writes - its patterns in the table and their rows in PATTERNS.md, its keys read through the settings table, its lines in the wording of the specification's 6.3, `DRY_RUN` and `DIAGNOSTIC_MODE` as the other parts have them; its files in the project.
2. The suite: its scenarios in the fake game, its line in `run_all.sh`.
   It pins at least the plain game; the part off; `DRY_RUN`; each check failing on its own, with nothing written; a site another mod changed; the writes and nothing else, every page's protection its own; the status lines.
3. The fragment, in the place my rules give it: the words for the specification's rows in 2 and 14.2 and its part's section, README's paragraph, NOTES' line and check count, ROADMAP's entry, CREDITS.md where an original gave the idea or code.

Before its report it makes the builds it needs without a warning and runs the suites its files touch, and it tries its new log lines against the other suites' patterns (my notes on the project's practice); shared code touches the suites of whatever uses it.

**What each track builds:**

- **Light stores** (D30, D36, D39; the lighting capacity's findings, 8.1 A, B and C and 8.3, and its review): the part looks for the 22 patterns always, and the measurement reads the addresses from it: over the moved stores its numbers say `of 10800` and `of 18000`, with the part off the game's own.
  Its checks are the measurement's, as a writing half's, and they refuse: a write pointer not 0 means the frames have begun, so the part refuses with a problem, never writing a rel32 a thread may be reading.
  `G-E-C4`'s 16 bytes are read by a `movaps`, so they lie 16-aligned.
  `LightStoresRaised()` is true only once its one write is done.
  Last, from `coronastore`'s findings, its review's point 2: `G-E-K1` and `G-E-K2` for a job's most lights and coronas under `DIAGNOSTIC_MODE`, so that my proof can show a job past 80 and past 200, not only that none was refused; `G-E-F1`'s jump for the ray traced list's refused lights, by a stub as `G-E-C1`'s; the corona store's "of N" and "refused" in its line, through `CoronaStoreNow()`.
  PATTERNS.md gets the sites, not an order against the constructor (its review's point 7).
- **Siren lights** (D23, D38, D40, D41; the sirens' findings, 8.3's A, B and C as 8.7 keeps them, `I-E-Z8` out, and its review's three points): the two tables each reported once when full, the own settings published whole with no lock on the hot path.
  `SIRENS_PER_VEHICLE=20` looks for nothing and writes nothing of this, and the measurement goes on as now; with `DIAGNOSTIC_MODE` the lines of the findings' section 6, the one that says which models are lit past 20 being my proof's.
  Last, from `sirenfacts`' findings, its review's point 1: D41's guard at both getter calls with its bound, and the same bound on the part's own first pass of a model.
- **Siren damage** (D23; the sirens' findings, 8.7 and 8.8): the room, its fill and the 19 damage writes; `I-E-L0n` and `I-E-N0n` (D34); a hit, an impact and a wreck on lights past 20 in the fake game, the room's mirror on a plane's IDs, each site failing alone with nothing written.
  The findings' `S-E-C06`, `S-E-D07` and `S-E-D08` stay as they are.
- **Sweep** (D33, D37, D41): `VIDEO_MEMORY_BUDGET` at 6000 with its comment, README and the suites that pin 0; `STREAMED_OBJECTS` and the descriptor walk, `streamedobjects`' sections 4.3 to 4.8, no watch; `I-E-H3` and `I-E-H4` beside `I-E-H1`, a pack's file name and D41's count, with `HookedPackCarcols`' parameters and `FileName`'s comment put right.
  Its fragment adds an open issue to NOTES.md: the game's descriptor heap has no check at its end, so past what my proof shows the raise can end in a crash or a corrupted descriptor without a word; accepted; it goes when a load shows fewer than 16,384 free, and then a watch or a lower default comes back.
- **Corona store** (D35, D39): `coronastore`'s sections 5 to 7, `G-E-I1` to `I9` and `J1` to `J5`, found and checked before the part's one write and its writes added to that list: `InitLightCapacity` calls `FindCoronaStore` with `G-E-G1`'s two matches.
  The new store 16-aligned, N x 0x30 + 0x180 bytes; the suite pins the job's clamp's two writes with the rest; the counters of 7.1 and 7.2 under `DIAGNOSTIC_MODE`.

**The merge** (the merge): the long tracks verified from their branches as they finish, the sweep's diff read itself.
All the tracks' commits cherry-picked in the order of the tables onto main, scripted; each code file checked to hold exactly both sides' changes; the anchors removed; the fragments folded into the documents in one commit; the counts summed; CREDITS.md naming SirenSetting Limit Adjuster for the sirens' idea (D20).

fixes in one or two sweep tracks.
Then 2.8.0 set - the project's version, the specification's header example and its section 15, NOTES' Versions, ROADMAP.md's entries of the shipped parts taken out - the four builds from scratch without a warning, `run_all.sh` green, the build reported.

## The boots

**2.8.0's proof,** on the Release build, my usual mods folder and trainer, a backup of my Profiles folder first, `DIAGNOSTIC_MODE=1` but for the last start; each start holds a load past the game's own number for what it proves:

1. **The sirens, at night:** `hp16fpiu1`, `lspdcvpi08` and `policelv` side by side, sirens on: the whole lightbar lit, lights 21 to 32 flashing as their carcols says, by my eye.
   One shot at a light past 20, one ram, one wreck: those lights out, their glass gone.
   One repair through the trainer: they come back.
   One plane with retractable gear, gear up and down: its doors work as ever.
   The log: the models lit past 20 with their bones found, `real lights in N`, no table full.
2. **The stores, the same start:** about 30 of those cars spawned in one place, sirens on, from outside the group so that all 30 are in view, since a light out of view takes no room (the lighting capacity's findings, 3.2).
   The log: vehicle lights past 400, vehicle coronas past 928, a job's most past 80 lights, how many a job refused, the scene store at its room, the coronas past 1088 with none refused, the ray traced list's refused lights; no crash, and the frame rate as I see it (its 8.4).
   A job past 200 coronas is not sure, as five cars at 37 are 185 (its 3.5): if the log stays at or under 200, the 300 stands with no load past the game's number, its layout proven by the lights past 80 behind it, and I decide then.
3. **The object limit, a start of its own:** the build's defaults with `DIAGNOSTIC_MODE=1`, `STREAMED_OBJECTS` doing the raise, `MaxStreamingResources` left at 98304 in my `gameconfig.xml`, the drive of my start 5: `are raised to 131072 by STREAMED_OBJECTS`, the objects past 100,000, the descriptors' number at its highest with at least 16,384 free, which settles the key's default (D37), no ` - at the most objects`, the bytes past 4,714 MB, my LOD switching by eye against start 5.
4. **The vehicle budget, a start of its own:** that and `Level3Kb` at 98,500 in my `gameconfig.xml` (D-V5), `MaxStreamingResources` left at 98304 there, a drive of 20 minutes through the city at night and on to the tunnel: the vehicle budget's share at the stop against 95%, the ray tracing buffer, the lights.
5. **The build as it ships:** a fresh `TweaksV.ini` from the build's defaults, no `DIAGNOSTIC_MODE`: line 1 names 2.8.0 and the commit without `-dirty`, the one `PROBLEM` `lfpolice`'s Legacy box, every part applied, `VIDEO_MEMORY_BUDGET` at 6000 and `STREAMED_OBJECTS` at its default.
   This is the build that is tagged.

Then the tag 2.8.0, the searches and one atomic push.
The hat of T-PEDPROPS is a start of its own.

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

**State:** 2.7.0, phase C's first build, is tagged on 7fa28da and pushed, my proof passed on it, and 2.2.1 stays the last one published.
2.8.0 is planned from my proof's numbers (D34 to D43) and its round runs now: the three research passes, the five coding tracks and the one review are done and merged on main, every fix in.
2.8.0 is set here, with its proof on the merged tree, 2979 checks; my boots of it come next, then the tag and one atomic push.
It takes the sirens per vehicle as the hybrid at 32 by default (D23, D38), the light stores as a sixteenth part (D30, D39), `VIDEO_MEMORY_BUDGET` at 6000 by default (D33), `STREAMED_OBJECTS` at 131072 on my proof's number (D37), `CORONA_STORE` at 4096 (D35), and the vehicle budget's raise in my `gameconfig.xml` in my proof (D-V5).
The ray traced list gets a count and no sort (D36), and the ped pool, the render queues and the ped budget stay as they are (D43).
The open issues are T-HARDEN, T-DIAG, T-FREEZE, T-PEDPROPS and T-DESCRIPTORS.

**Decisions waiting on me:** none before my proof. The dumps, `GameData-*.bin` and the `work-*` folders stay (D28).
