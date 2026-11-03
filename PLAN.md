# Plan

The order of work for a change to TweaksV, phase by phase, each ending with its proof, a commit and my boot.
What came before is in the git history and in NOTES.md's Versions; the passes' findings and reviews are in my research tools' `output-1158.16\`.
Phase C is done: 2.7.0, tagged on 7fa28da, for my own use (D19), and 2.8.0, tagged on b3a7a1b, the full version I publish (D7). Its tracks, items and the decisions' full text are in the git history up to b3a7a1b.
No phase is planned now.

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
- **D10:** phase B built WeaponLimits, PedProps and the texture budget and dropped the streaming heap; PedProps ships off (ISSUES.md, T-PEDPROPS).
- **D11:** phase B's scaffold and `TweaksV.ini`'s layout 4.
- **D12:** phase B's designs, as built in SPECIFICATIONS.md's sections of those parts.
- **D13:** `ResourceExtensions.asi` is a plugin that stays: it switches HeapAdjuster, PackfileLimit, Blips and RayTracing off.
- **D14:** SirenSettings a phase of its own after phase B (D22, D23).
- **D15:** the first StaticBounds, built on the size lookup's call, left 2.6.0 before its tag because that pattern is not in Enhanced (D24).
- **D16:** the ray tracing buffer's research and part, as built in SPECIFICATIONS.md; LuminanceV's documents may be read for a pass.
- **D17:** RageOpenV's check for packs made for GTA V Legacy.
- **D18, stands:** parts built in one round ship together as one version, tagged on the commit I boot and pushed with main.
- **D19:** phase C's shape: two builds, each with my boots; the next version I publish stays the full one (D7), which 2.8.0 is.
- **D20:** the research passes, the limit on reverse-engineering lifted pass by pass; the clean room stays D2's.
- **D21, stands:** the data window, `TEST_DUMP_DATA`, a test build's key; `.data` as a whole stays out, since it may hold my data.
- **D22:** the siren setting IDs, `LARGE_SIREN_IDS`, on by default after my comparison boot; the position stays a byte, so 255 settings stay the most.
- **D23, stands:** the sirens per vehicle as the hybrid: the lights by a second call of the game's own renderer, the damage by the game's own code raised, glass included.
- **D24 and D32, stand:** StaticBounds anew: one call redirect at the store's one size lookup, `STATICBOUNDS_SIZE` up to 32767, a read-back on every boot.
- **D25, stands:** a limit measured first; a raise only from a boot's numbers.
- **D26 and D27:** 2.7.0's open issues taken in two sweeps; its scaffold, the reflow to one sentence a line and `TweaksV.ini`'s layout 5.
- **D28, stands:** the dumps, `GameData-*.bin` and the `work-*` folders stay for now, since they cost nothing.
- **D29, stands:** the workflow, as my rules for every project have it since 2026-11-01.
- **D30:** the lighting capacity and the render queues measured; `[PEDPROPS]` ships off.
- **D31:** the two model budgets and the ped pool measured; `POOL_MEMORY` raised to 256 MB.
- **D-V5, stands:** the vehicle budget by no code: `Level3Kb` at 98,500 in my `gameconfig.xml`, 600 MB in place of 429, which stays after my proof's start 4.
- **D33, stands:** `VIDEO_MEMORY_BUDGET` at 6000 by default.
- **D34:** 2.8.0's order: research and coding side by side, with no wait for the weekly limit's reset.
- **D35, stands:** the corona store moved and raised, `CORONA_STORE`, 4096 by default, 1088 to 8192; the LOD lights' job keeps its own 1088.
- **D36, stands:** no sort of the ray traced list in 2.8.0, a count of what it refuses under `DIAGNOSTIC_MODE`: my start 4 refused 172,943 lights, 165,438 of them in one minute in the tunnel, and vehicles come last. A research pass in a later build.
- **D37, stands:** `STREAMED_OBJECTS`, 131072 by default, at most 147456, 3/4 of the game's descriptor heap of 196,608; no raise of the heap and no watch at its end (T-DESCRIPTORS). My proof: at most 149,438 descriptors in use at 124,634 objects, so the default stands.
- **D38, stands:** `SIRENS_PER_VEHICLE` at 32 by default.
- **D39, stands:** the light stores a part, `[LIGHTCAPACITY]`, off beside Resource Extensions; sirens above 20 refuse with a problem when the stores are not raised.
- **D40, stands:** the sirens' raise runs only on game builds its renderer was read on, today 1.0.1158.16; a game update costs players the raise until a new TweaksV.
- **D41, stands:** a siren setting ID that finds no setting: both getter calls guarded, an empty setting with no lights; the count at load under `DIAGNOSTIC_MODE`.
- **D42, stands:** the hat of T-PEDPROPS, a small clean single-prop pack, a start of its own.
- **D43, stands:** no raise of the ped pool, the render queues or the ped budget: my proof's numbers do not ask for one.
- **D44, stands:** `TEST_SIREN_PASSES`, a test build's switch that repeats the sirens' renderer per pass, since no normal load reaches the stores' numbers: 16 cars lit gave at most 124 vehicle lights, about 7 a car, under a spawn cap of 17.

## What my boots have shown so far

The full account is in the git history up to b3a7a1b.

- **2.8.0's proof,** 2026-11-03: every raise past the game's own number, no crash but my own Legacy car's.
  - **Starts 1 and 2, the sirens and the stores,** 19 minutes: three models lit past 20 by my eye and the log; a shot, a ram and a wreck put those lights out and their glass, a repair brings them back, a plane's gear works as ever.
    Past the game's numbers: coronas 1,588 of 4,096, none refused; a job's 97 of 180 lights; objects 117,269, descriptors 140,933 of 196,608.
    Not past them: vehicle lights 217, vehicle coronas 270, scene lights 616, a job's coronas 72, with 10 cars.
  - **Start 3, the object limit,** 9 minutes: objects 104,437, descriptors 127,881, no ` - at the most objects`, the bytes at 5,415 MB, my LOD switching fixed completely by my eye and no texture flicker, so D37's 131072 stands.
  - **Start 4, the vehicle budget,** 13 minutes, `Level3Kb` at 98,500: 600 MB read, at the stop in 607 of 741 samples, 141 models at once, `VehicleStruct` 141 of 200; objects 124,634, descriptors 149,438, 47,170 free; ray tracing buffer at most 3.39 million vertices, none refused; the ray traced list as in D36.
    Five stars and the drive gave no load: vehicles 48, vehicle lights 193, scene lights 557, coronas 705.
  - **The airport, three attempts with real cars:** no load, which D44 answers: at most 143 vehicle lights, 225 vehicle coronas, scene lights 511, a job's 82 coronas.
    The first ended in the game's trap at `+0x127C32F`, a Legacy `lfpolice` car I spawned ("Packs", below), at none of the raises' sites.
  - **Start 5, the load,** 6 minutes, the test build of 4226c6d (D44), `TEST_SIREN_PASSES=16` for one start's margin, about 10 cars of `hp16fpiu1`, sirens on, all in view: no crash, the one `PROBLEM` `lfpolice`'s Legacy box, my folder's name gone from the log.
    Past the game's numbers, at last: vehicle lights 1,023 of 10,800, vehicle coronas 2,027 of 18,000, coronas 2,507 of 4,096, none refused, a job's lists at their room, 180 and 300, 2,252,028 lights refused as the load asks, and the scene store at its 1,024 with the sort list marked in 3,575 of 30,090 samples.
    By my eye the lights flicker hard and the frame rate is about half, the overload's own.
  - **Start 6, the build as it ships,** 5 minutes, the Release build of 4226c6d, no `TweaksV.ini`: `Created TweaksV.ini with the default settings.`, the one `PROBLEM` `lfpolice`'s Legacy box, every part applied, no `TEST BUILD` line, no user name.
  - **The last boot,** the Release build of b3a7a1b with the new wording of the default file and the popups, no `TweaksV.ini`: the file made equal to the default, the one `PROBLEM` `lfpolice`'s Legacy box, every part applied: the build that is tagged.
- **2.7.0's proof,** 2026-11-02, the numbers 2.8.0 was planned from: with `VIDEO_MEMORY_BUDGET=6000`, vehicle lights 327 of 400, vehicle coronas 439 of 928, scene lights 642 of 1024, the ray traced list 1024 of 1024, coronas 1088 of 1088, objects 98,343 of 98,304; ped pool 97 of 256, ped budget 505.8 of 561, the render queues far under their room.
  The pool at 8000 with `STATICBOUNDS_SIZE=16000` crashed the first build at the save's load, `int3` at `+0x93AA93`, and passed on the tagged one; 32767 passed at the pool of 24,500.
  The trainer stopped answering once after caps were set on freemode peds, no event, no crash file.
- **2.6.0's proof,** 2026-11-01, with `VIDEO_MEMORY_BUDGET=6000`, `DYNAMIC_BVH_VERTICES=8388608`, the seven weapon packs and one Legacy pack: 166 `weapons.meta` files and 556 component definitions loaded; the budgets at most 4,155 of 6,000 MB; the objects 98,359 of `MaxStreamingResources`' 98,304; the ray tracing buffer at most 2,281,234 of 8,388,608 vertices, none refused.
  The guards' boot passed; a refused pack's weapon spawns without its magazine.
- **My load without packs,** 2.5.0, 2026-10-29: `weapons.meta` files 155 of 160; component definitions 465 of 470; siren settings 21; streamed textures 3,004 of 3,000 MB; the resource heap 10 of its 32 regions.
- **With packs,** 2.5.0, 2026-10-30: siren settings 33, their IDs 33 of 256, none given twice, an ID past 255 keeping its low byte.
  Three models past `siren20`: `lspdcvpi08` with 10 sirens and 6 glass bones never lit, the CHP FPIU with 12, half its lightbar dark, the LVMPD Crown Victoria with 10 and 6.
  The male freemode model holds 255 head props with the base game alone, the female 254; a switch past 255 crashes at `+0x5A0998D` (ISSUES.md, T-PEDPROPS).
  The box streamer's largest search 1,067 of 10,000; decals 259 of 260; blips 243 of 2,000.
- **Packs:** every Legacy vehicle traps the game at `+0x127C32F` as it is created; converted with CodeWalker, most spawn.
  Two converted packs crash in the game's texture and buffer code, where TweaksV writes nothing: State Wide after a minute or two of driving, the SFPD collection's police6 seconds after its spawn.
  Without them the game ran seven minutes with the CHP FPIU, the LVMPD Crown Victoria and the CVPIs.
- **`gameconfig.xml`:** `AnimStore` at NaturalVision Enhanced's 21,200 boots, and at 32,000 every load crashed at `+0x15A514`, a pool's failed allocation.
  `MetaDataStore` full at 3,200 of 3,200 traps the game at `+0x1585E0`.

## Next steps

**State:** 2.8.0, phase C's second and last build, is tagged on b3a7a1b and pushed, this document with it, and my proof passed on it; it is the full version I publish (D7), after 2.2.1.
It has sixteen parts, `TweaksV.ini`'s layout 5 with its 47 keys, 274 patterns and 2986 checks, the full run about 10 minutes side by side.
It takes the sirens per vehicle (D23), the light stores and the corona store as a part (D35, D39), `VIDEO_MEMORY_BUDGET` at 6000 (D33), and `STREAMED_OBJECTS` at 131072 (D37), which ended my LOD switching by my eye.
Open for a later plan, none asked for now: the ray traced list's sort (D36); some lights on buildings that come on late, about 50 m from a Gruppe 6 building downtown by my eye, with nothing refused in the log (ROADMAP.md); the hat of T-PEDPROPS (D42).
The open issues, in ISSUES.md, are T-HARDEN, T-DIAG, T-FREEZE, T-PEDPROPS and T-DESCRIPTORS.

**Decisions waiting on me:** none. The dumps, `GameData-*.bin` and the `work-*` folders stay (D28).
