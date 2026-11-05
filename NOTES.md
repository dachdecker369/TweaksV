# Notes

The versions of TweaksV and the count of its checks.
What each part does is in [README.md](README.md) for the player and in [SPECIFICATIONS.md](SPECIFICATIONS.md), section 11, for the design; the rules every part follows are that file's section 12.
The game facts are in [PATTERNS.md](PATTERNS.md), the open issues in [ISSUES.md](ISSUES.md), the credits and derivations in [CREDITS.md](CREDITS.md).

- **Tests:** `test\` holds fake-game test suites (2995 checks): small programs that pretend to be the game, with copies of the code TweaksV patches and a script program they place as the game does, load the real `TweaksV.asi` and report what it changed.
  One more suite checks the pattern table against `PATTERNS.md` and has each pattern held too often.
  `bash test/run_all.sh` builds and runs them (see README).

## Versions

Newest first; the tags carry the same numbers.
One line per version with what it brought; git has the rest.

- **2.8.1:** `PACKFILEARRAY_SIZE` stops at 32770 and is rounded down to a multiple of 5, since 65531 and up crash the game at start-up: a higher value in an older `TweaksV.ini` now runs at 32770 with the clamp's message on screen; the default stays 13000; 2995 checks.
- **2.8.0:** phase C's second build: the sirens per vehicle, `SIRENS_PER_VEHICLE`, 32 by default, the lights past 20 by a second call of the game's renderer and the damage on them, glass included, by the game's own loops raised; the light stores, a sixteenth part: a job's lists at 180 lights and 300 coronas, the frame's stores moved, the scene store's byte index fixed; the corona store raised to `CORONA_STORE`, 4096 by default; `VIDEO_MEMORY_BUDGET` at 6000 by default and `STREAMED_OBJECTS` at 131072; with `DIAGNOSTIC_MODE=1` a job's most, the ray traced list's refused lights, the coronas the store refuses, the descriptors in use, a pack's carcols lines naming their file, and the vehicles whose siren setting ID finds no setting; `TEST_SIREN_PASSES` in test builds, the load the stores' proof needs (D44); `TweaksV.ini` layout 5 with 47 keys; 2986 checks.
- **2.7.0:** phase C's first build: StaticBounds anew, the store's size raised at its one call, `STATICBOUNDS_SIZE` up to 32767, its read-back on every boot, a check from `blip_controller`'s placement says when the game did not ask, and a size above the default on a pool memory budget not raised is a problem; SirenSettings' IDs, `LARGE_SIREN_IDS`, past 255 on a table of 65,536 entries; `POOL_MEMORY`, the budget of the game's pools raised to 256 MB by default, which ends the crash at `+0x15A514` my load came within a MB of; the lighting capacity, the two model budgets, the ped pool and the render queues measured first with `DIAGNOSTIC_MODE=1`, the lighting capacity through the one jump it redirects, nothing raised; `TEST_DUMP_DATA` in test builds, named windows of the game's `.data` (D21); the open issues T-COPIES, T-LEGACYWALK, T-NAMES, T-LATEBOX, T-SUMMARY and T-TESTGAPS closed, one copy of each helper and one data check, a box per late report with a heading and a last line that fit it; the suites side by side, each in a folder of its own, a full run of about 9 minutes where it was 45, with a table of which suites a change needs (D29), a fake that crashes after its report failing, the checks skipped counted, each suite's number of checks on a line of its own, a message box's whole text and the three catch-alls checked; the conflict check knows `WeaponLimitsAdjusterEnhanced.asi` and `GTAV.ResourceAdjusterEnhanced.asi` (D9); `TweaksV.ini` layout 5 with 43 keys; not published by itself; 2382 checks.
- **2.6.0:** phase B: WeaponLimits (more `weapons.meta` files and component definitions, a guard each), PedProps (off by default, experimental, T-PEDPROPS), Resources (`VIDEO_MEMORY_BUDGET`), RayTracing (`DYNAMIC_BVH_VERTICES`) and a check of the mods folder for packs made for GTA V Legacy; the streaming heap's measurement out, since that heap never ran short, its facts in the documents as tagged 2.5.0; `TweaksV.ini` layout 4; not published by itself; 1247 checks.
- **2.5.0:** the measurement build: seven limits of the parts to come measured with `DIAGNOSTIC_MODE=1`, nothing written but the siren count's redirected call; not published by itself; 901 checks.
- **2.4.0:** the replace-rename: with `UNLOCK_FILES=1` a rename made inside the game over a file the game holds goes through, with POSIX semantics; not published by itself; 641 checks.
- **2.3.0:** the Blips part: the threshold in `blip_controller` raised, so that the game's own blips stay on the map next to a mod's; `TweaksV.ini` layout 3; not published by itself; 609 checks.
- **2.2.3:** RageOpenV's mods folder in UTF-8 on every Windows, paths past 254 bytes left out and counted; 540 checks.
- **2.2.2:** the default `TweaksV.ini` has a comment after each key, and a value is read up to a comment; not published by itself; 525 checks.
- **2.2.1:** NaturalVision Enhanced's loader `onigiri.asi` known to the conflict check, RageOpenV and StraightToStoryMode off beside it, so that the game no longer crashes in it; 521 checks.
- **2.2.0:** the DirectStorage part frees the game folder's files instead of refusing them, so that a loader serving loose files works; 496 checks.
- **2.1.1:** a DirectStorage factory the game never asks for is reported on screen; 486 checks.
- **2.1.0:** the fixes of the 2.0.0 audit; 485 checks.
- **2.0.0:** every part rewritten as TweaksV's own code on shared code; `TweaksV.ini` layout 2; the release zip defined as five files.
- **1.6.0:** the Decals part - the decal definitions array moved and its limit raised; `DecalPatch.asi` as an old plugin.
- **1.5.0:** the box streamer's measurement, and the list's loads found by its address instead of a pattern.
- **1.4.0:** the BoxStreamer part - the traversal list moved and its capacity raised; `TEST_DUMP_CODE` in test builds.
- **1.3.0:** the own-file checks (links, junctions), SECURITY.md, the exploit mitigations checked by the tests.
- **1.2.2:** the pattern table checked against PATTERNS.md by a suite; test builds with `TEST_BREAK_PATTERN`; the integration suite.
- **1.2.1:** the patch module - every write to the game's code through one place that gives the pages their protection back.
- **1.2.0:** the settings table - every setting of `TweaksV.ini` defined once.
- **1.1.1:** the test harness - fake games that load `TweaksV.asi` and report what it changed.
- **1.1.0:** `UNLOCK_FILES` - DirectStorage may open only the `.rpf` and `.cache` files in the game folder, on by default.
- **1.0.2:** the DirectStorage part, logging only at first; the ASI loader file named in the documents.
- **1.0.1:** scanner and pattern fixes.
- **1.0.0:** the five plugins by Chiheb-Bacha merged into one `TweaksV.asi` with one `TweaksV.ini` and one `TweaksV.log`.
