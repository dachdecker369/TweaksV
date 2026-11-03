# Specifications

The design of TweaksV as it is built: what each module and part is for, its mechanism and why it is so, its limits, formats and interfaces, what happens when it fails, the rules, and how to build and extend it.
The words of the log and the order of the checks are in the code.
Five files carry what the code does not say, each with its own job:

| File | Carries |
|---|---|
| [README.md](README.md) | the player's view: what each part does, every key of `TweaksV.ini`, building, testing, other mods |
| SPECIFICATIONS.md, this file | the design |
| [PATTERNS.md](PATTERNS.md) | the game facts: every byte pattern, what the match holds, where it matched on the current build, how the sites are encoded |
| [NOTES.md](NOTES.md) | the versions and the count of the checks |
| [ISSUES.md](ISSUES.md) | the open issues |

Credits and derivations are in [CREDITS.md](CREDITS.md), the notices in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
[SECURITY.md](SECURITY.md) says how to report a security problem; [ROADMAP.md](ROADMAP.md) holds what I plan next, [PLAN.md](PLAN.md) the phases of a change and the decisions that stand.
"I" in these files is the author, dachdecker369.
TweaksV is GPL-3.0-only.

## 1. What TweaksV is

One ASI plugin for GTA V Enhanced, `TweaksV.asi`, loaded by an ASI loader into `GTA5_Enhanced.exe`; it does nothing in any other program.
Sixteen parts change the running game in memory, never a file on disk: they raise the game's limits, serve a mods folder and unencrypted archives, skip the start-up screens and free the files DirectStorage keeps locked (README has each, section 11 its design).
Every part but DirectStorage, which hooks two functions of Windows and needs no pattern, finds the game's code by byte patterns and goes by the rules of section 12; what it could not do it says in the log and in one start-up message box.
With `DIAGNOSTIC_MODE=1` TweaksV also measures how the game uses what the parts change, and the limits of section 11.16.

Everything is driven by three tables - the settings, the patterns, the parts - and by shared services: the log, the start-up summary, the scanner, the patch module, the relocation core.
A new part is an entry in each table plus one file of its own.

## 2. The code at a glance

Every file in `source\` is `TweaksV*` and every symbol is in `namespace TweaksV`, except `dllmain.cpp`, `version.rc` and MinHook in `vendor\MinHook`, compiled unchanged.
In the table a name without an extension stands for its `.h` and its `.cpp`.

| File | Job | Section |
|---|---|---|
| `dllmain.cpp` | the one `DllMain`, the start-up in ten steps | 3 |
| `TweaksVGame` | the game's name and file version | 3 |
| `TweaksVSettings` | the settings table and its readers | 4.1 |
| `TweaksVPatterns.h` | the pattern table | 4.2 |
| `TweaksVFeatures` | the feature table | 4.3 |
| `TweaksVIni` | `TweaksV.ini`, the file | 5 |
| `TweaksVFiles` | the own-file checks | 5.2 |
| `TweaksVLog` | `TweaksV.log`, the console, the text helpers | 6 |
| `TweaksVLock.h` | the lock guard of the log and the summary | 6.1 |
| `TweaksVSummary` | the problems and findings and their boxes | 6.4 |
| `TweaksVMeasure` | the measurements' one loop and summary, and one check of the game's data | 6.6 |
| `TweaksVConflicts` | the conflict check, and the tests for an address inside the game | 7 |
| `TweaksVScanner` | the scanner | 8 |
| `TweaksVPatch` | the patch module | 9 |
| `TweaksVRelocation` | the relocation core, for the six array parts | 10 |
| `TweaksV<Part>` | the part of that name: HeapAdjuster, ModkitLimit, PackfileLimit, BoxStreamer, Decals, RageOpenV, StraightToStoryMode, DirectStorage, Blips, PedProps, RayTracing, StaticBounds | 11 |
| `TweaksVModsFolder` | RageOpenV's one walk of the mods folder, for its list and its check for Legacy packs | 11.6 |
| `TweaksVLegacyPacks` | RageOpenV's check for packs made for GTA V Legacy | 11.6 |
| `TweaksVRpf.h` | the RPF7 facts RageOpenV and that check share | 11.6 |
| `TweaksVWeaponInfos` | WeaponLimits' weapon infos | 11.10.1 |
| `TweaksVWeaponComponents` | WeaponLimits' weapon components | 11.10.2 |
| `TweaksVTextureBudget` | the Resources part | 11.12 |
| `TweaksVSirenIds` | SirenSettings' siren setting IDs, and their measurement | 11.14, 11.16.1 |
| `TweaksVSirenCount` | SirenSettings' sirens per vehicle, their lights and their measurement | 11.14.1, 11.16.2 |
| `TweaksVSirenDamage` | the sirens per vehicle's damage | 11.14 |
| `TweaksVSirenRoom.h` | what those two share | 11.14, 11.14.1 |
| `TweaksVPopulation` | measured first: the two model budgets | 11.16.4 |
| `TweaksVPedPool` | measured first: the ped pool | 11.16.5 |
| `TweaksVRenderQueues` | measured first: the render queues | 11.16.6 |
| `TweaksVLightCapacity` | LightCapacity's job's lists and light stores, and the lighting capacity's measurement | 11.17, 11.16.3 |
| `TweaksVCoronaStore` | LightCapacity's corona store | 11.17.1 |
| `TweaksVScripts` | the script programs' shared hook, and a program's layout | 13.3 |
| `TweaksVDump` | test builds' copies of the game's code, its data windows and a script program | 13.2, 13.3 |
| `version.rc` | the version resource, from `BuildInfo.h` | 15 |

Every `.cpp` and `.h` opens with its SPDX line and what the file is, a part's `.cpp` also with what the part does, every switch it reads and the credit for the idea or the patterns.
A part has one log tag (`kTag`, the feature name of section 4.1) and an entry function, `Init<Part>()` or one per module as its heading in section 11 names, called by the feature table; it keeps its state in file-scope variables inside an anonymous namespace.
A module measured first does the same with `Measure<Limit>()`.
Tabs for indentation; CRLF line endings in the source (git normalises them).

## 3. The start-up

`DllMain` runs these steps on `DLL_PROCESS_ATTACH`, inside a `try` so that no exception leaves it:

1. `IsGtaVEnhanced()`: in any program but `GTA5_Enhanced.exe` nothing more happens.
2. `CreateOrUpdateIni()`: `TweaksV.ini` exists from here on (section 5).
3. `LoadSettings()`: every setting is read once and the file let go of (section 4.1).
4. `[TWEAKSV] ENABLED=0` stops here.
   Otherwise the module pins itself and stays loaded until the game exits, because the game's code calls into it once hooked, and an early `FreeLibrary` would crash the game at the next hooked call; a pin that fails is reported.
5. `CheckForConflicts()` with `CONFLICT_CHECK=1` (section 7), before the log is started, so that the header's `Features:` line says `off` for every part the check switches off.
6. `ClearLogFile()` and `WriteLogHeader()` (section 6): lines logged before this point are written after the header, but the console, opened next, does not show them.
7. `OpenConsole()` with `CONSOLE=1`; a test build then starts its dumps, the code's before anything is patched (sections 13.2 and 13.3).
8. `StartFeatures()`: every part that is on, in table order, then with `DIAGNOSTIC_MODE=1` the parts measured first (section 4.3).
   Each part runs its whole job here, but RageOpenV waits for the game's first time request (section 11.6), DirectStorage works in its hooks of Windows' functions (section 11.8) and Blips each time the game places `blip_controller` (section 11.9).
9. `ShowStartupSummaryLater()` starts the thread that shows the start-up summary (section 6.4), unless a part shows it itself later (RageOpenV, at the first time request).
10. `DllMain` returns `TRUE` whatever happened; an exception that reaches its `try` is reported, the rest of TweaksV skipped this session, and the summary thread started, so the box names it even when the log never started.

On `DLL_PROCESS_DETACH` the log is flushed and closed, without waiting for a lock another thread may have died holding.

The game's file version is read once from the running program's version resource, 0 when unreadable (`GameVersionNumber`).
PackfileLimit chooses patterns by it, the sirens per vehicle are raised only on the build whose renderer I read (section 11.14.1), and the log header prints it.

## 4. The three tables

### 4.1 Settings (`TweaksVSettings.h`)

Every setting is defined once in `kSettings`, in the order of the default file, with its section, key, kind, default, limits and the short comment the default file carries after its value.
The default file, the keys added back, the log header and every reader come from this table, and a name not in it does not compile (`FindSetting`); README has each key's meaning, range and default.
The feature name of a `SettingsSection` is the log tag and the name in the `Features:` line and the start-up summary; `kSections` is in file order, which is also the order the parts start in.

`kIniLayoutVersion` is 5 and `kIniFirstLayout` 2, the first layout with `VERSION` and the oldest one carried forward (section 5.3).
A `static_assert` on `IniKeyCount()` pins the layout at 47 keys: a new key means a new layout number, and so does a key that changes its meaning or moves.

`LoadSettings()` reads every entry once through `ReadIniValue`, a build that is not the test build skipping the seven test keys; nothing reads the file again.
There is no in-game panel and no reload key, by design: the parts patch the game before it runs, so a setting changed live would change nothing.
A value TweaksV cannot use is reported, with the value used instead: the default for one that is no switch or no number, the nearest of the table's limits, the only ones this report knows, for a number outside them.

### 4.2 Patterns (`TweaksVPatterns.h`)

`kPatterns` lists every pattern once, under the label PATTERNS.md gives it and in PATTERNS.md's order, grouped per part, and is the one place in the code to change after a game update.
A malformed pattern, a repeated label or a label not in the table does not compile (`PatternsAreValid()`, and `Pattern(label)`, the only way the code names a pattern).
`PatternByte`, `PatternHolds` and `PatternImm32` check a constant, when TweaksV is compiled, against the bytes its pattern pins it with, one copy for every part.

The table holds 274 patterns; 1.0.1158.16 uses 271, since the packfile part chooses one of three direct-size patterns and one of two references by build.
The check boot (`DIAGNOSTIC_MODE=1` and `CONSOLE=1`, every part on - `[PEDPROPS] ENABLED=1` among them, which ships off - with or without `DRY_RUN=1`) looks for every one.
A part looks on every boot it is on for all of its patterns but these, so a normal boot looks for fewer:

- A job whose key is off or at the game's own value looks for none of its patterns: `R-E-C1` without `CONSOLE=1`, `R-E-F1..F8` without a mods folder in the game's current directory, `Q-E-T1` to `T4` and `T6`, the three `T-E-`, the siren IDs' 14, the sirens' 26 and the corona store's 14.
- `DIAGNOSTIC_MODE=1` looks for those too, but for `R-E-C1`, the sirens' and the corona store's, and for what only a measurement reads: `L-E-P1`, `L-E-P2`, `A-E-C2`, `A-E-S1`, `Q-E-T5`, `Q-E-T7`, `I-E-H1` to `H4` and the 64 of the parts measured first.
- PedProps, off as it ships, looks for none of its thirteen, but for `A-E-C1`, `A-E-C2` and `A-E-S1` with `DIAGNOSTIC_MODE=1`.

### 4.3 Features (`TweaksVFeatures.cpp`)

An old plugin is a stand-alone `.asi` that did a part's job before TweaksV and that TweaksV replaces; a plugin that stays does its job too and stays installed because another mod needs it (a `PeerPlugin` constant, section 7).
`kFeatures` names them per part, with its start function, in the order of `kSections` by a `static_assert`:

| Part | Old plugins | Plugins that stay |
|---|---|---|
| HeapAdjuster | `HeapAdjuster.asi`, `ZHeapAdjuster.asi` | `ResourceExtensions.asi` |
| ModkitLimit | `ModkitLimitAdjusterEnhanced.asi` | |
| PackfileLimit | `PackfileLimitAdjusterEnhanced.asi` | `ResourceExtensions.asi` |
| BoxStreamer, Decals | `DecalPatch.asi` | |
| RageOpenV | `RageOpenV.asi`, `OpenRPF.asi`, `OpenIV.asi` | `onigiri.asi` |
| StraightToStoryMode | `StraightToStoryMode.asi` | `onigiri.asi` |
| DirectStorage | `DirectStorageFix.asi` | |
| Blips | `BlipLimitFix.asi` | `ResourceExtensions.asi` |
| WeaponLimits | `WeaponLimitsAdjusterEnhanced.asi` | |
| Resources | `GTAV.ResourceAdjusterEnhanced.asi` | |
| RayTracing, LightCapacity | | `ResourceExtensions.asi` |
| StaticBounds | `StaticBoundLimitAdjuster.asi`, `StaticBoundsLimitAdjuster.asi` | |

`StartFeatures()` calls each start function whose part is on, each inside its own `try`: an exception ends only that part, and is reported.

`kMeasuredFirst`, beside it, lists the parts measured first, one entry per limit in the order of section 11.16's subsections, and last `MeasurePedPropsWhileOff`, the ped props' measurement while that part is off (section 11.11).
The lighting capacity stays in it beside its part, since its measurement goes on over whatever the part did (section 11.17), and SirenSettings' two since they go on with their job or their part off.
With `DIAGNOSTIC_MODE=1`, `StartFeatures()` starts them after the parts, each inside its own `try`: an exception ends only that measurement and is logged, not reported, since a measurement is an aid (section 12).

## 5. TweaksV.ini

### 5.1 Where it is

The game folder is the running program's, the fallback folder `%LOCALAPPDATA%\TweaksV`, none when `LOCALAPPDATA` is not set.
The file in use is the game folder's `TweaksV.ini` if it exists, else the fallback folder's if that exists, else a new file - in the game folder, or in the fallback folder when the game folder cannot take it.
A file in the game folder always wins; the header says when an unused copy sits in the fallback folder.
A read-only game folder is normal, so it is only logged and shows no message box; any other reason, such as a `TweaksV.log` that another program holds open (section 6.1), is a problem in the start-up summary.

### 5.2 The own-file checks (`TweaksVFiles`)

TweaksV writes only to its own files, `TweaksV.ini`, `TweaksV.log`, `TweaksV.old.ini` and the test-build dumps, and only where each is what its path says: a plain file directly in the folder it belongs in.
A link in its place could otherwise make TweaksV write to another file, which matters when the game runs as administrator.
`Check` refuses a reparse point, a folder, a hard link, a file whose final path (every link followed) is not its folder's final path plus the name, and a file whose attributes, folder or final path cannot be read.
The game folder may itself be a link (`FileFolder::Game`, an install moved to another drive), TweaksV's own folder in `%LOCALAPPDATA%` may not (`Own`).
A file Windows keeps compressed (`compact`, CompactGUI) is not a link and is used as usual.
A refused `TweaksV.ini` is neither read nor written, so every setting keeps its built-in default; a refused `TweaksV.log` is left alone and the log goes to the fallback folder (section 6.1); each is a problem in the start-up summary.

`OpenOwnFile` opens the link itself, never its target, and deletes a file it created itself when the check fails.
`HoldOwnFile` opens without delete sharing: while the handle is open, nobody can rename, delete or replace the file or rename a folder above it, so the Win32 INI functions that open it by its path reach this very file.
`RenameOwnFile` renames through a handle after the same check, and replaces a file already at the new name only when that one passes it too.

### 5.3 Creating, updating, replacing (`TweaksVIni`)

`CreateOrUpdateIni()`:

1. An existing file (game folder first) is held (`HoldOwnFile`) until every value is read; one that fails the check is reported and never read, and the built-in defaults are used.
2. A held file without the key `VERSION` under `[TWEAKSV]` has the layout of TweaksV 1.x, whatever else it holds: an empty or damaged file, or one whose `VERSION` line was deleted.
   Only the key names of that section are read to decide; no value of such a file is ever used.
   It is renamed to `TweaksV.old.ini`, the default file written in its place and held, and the problem reported once.
   A file that cannot be renamed is reported every boot and not read, the built-in defaults used until it is gone; a renamed file whose replacement cannot be written is reported once, and the next boot starts at step 4.
3. Any other held file gets every key it is missing, with its default and without a comment - TweaksV adds none to an existing file - and each is logged; existing values are never changed.
   A file of an older layout - a `VERSION` from `kIniFirstLayout` to below `kIniLayoutVersion` - is then carried forward and logged: this layout's number is written back in the old one's place, a comment after it kept.
   What cannot be written is logged when the file is read-only and reported otherwise.
   Any other `VERSION` is left for `LoadSettings` to report (section 4.1): no number, below 2, or above 5 as a newer TweaksV's file.
4. No file anywhere: the default file is written in the game folder; if that fails, in the fallback folder, which is created, a read-only game folder only logged and any other reason reported; if both fail, the built-in defaults are used and reported.

`ReadIniValue` reads at most 255 characters of a value and cuts it at a comment - `//`, as the default file writes one, or `;` - since Windows' INI functions take everything after the `=` for the value.
`DescribeIniLocation()` gives the header its `TweaksV.ini:` line: the path, and in parentheses a fresh file in place of a 1.x file, no file in the game folder, an unused copy in the fallback folder, or why the file is not used.

### 5.4 The default file

`DefaultIniText()` builds the file from the table, with CRLF line endings and every comment from column 32 on (`kIniCommentColumn`), which a `static_assert` keeps every `KEY=default` short of.
`test\expected_default.ini` is exactly this file, kept with LF endings: the `ini_default` suite gives it CRLF and compares byte for byte.

What each key does is README's table, and the two testing switches are in its "Diagnostics", with the list of what `DIAGNOSTIC_MODE=1` hooks beyond a normal boot.
`DRY_RUN=1` makes every part find and check everything and log what it would change, with nothing in the game written or hooked but RageOpenV's one-shot hook on `GetSystemTimeAsFileTime`, which runs its checks and switches itself off.

## 6. TweaksV.log and the start-up summary

### 6.1 The log file

`WriteLog(tag, text)` appends `hh:mm:ss.mmm [Tag] text` to `TweaksV.log` and to the console when one is open, under one `SRWLOCK` held through the guard of `TweaksVLock.h`, which an exception releases too, so that no failure can leave the next line waiting forever.
A line is cut at 2 KB, in a test build at 64 KB; lines written before the log is started, up to 1024, are kept and written after the header.
Every mention of the Windows user's profile folder is written as `%USERPROFILE%`, in its short form (`C:\Users\NAME~1`) and with forward slashes too, because logs get posted in public and that folder is usually named after the user.
The file grows to 64 MB at most, room for hours of the logs for testing; a last line says when it stops, and the console still gets every line.

`ClearLogFile()` starts the file each session, truncated, in the game folder through `OpenOwnFile`, else in `%LOCALAPPDATA%\TweaksV`.
`LOG=0` writes no file, and the message boxes say so in place of its path.

### 6.2 The header

`WriteLogHeader()` writes, all tagged `[TweaksV]`:

```
TweaksV <version> (git <hash>)
TweaksV.asi: <path>
Game: <path>, file version 1.0.1158.16, PE timestamp 0x6AA45F10 (<UTC date>)
TweaksV.ini: <path> (<notes, section 5.3>)
TweaksV.log: <path>
mods folder: <game folder>\mods (present | not present)
Features: HeapAdjuster on, ... PedProps off, ...
TweaksV: LOG=1 DIAGNOSTIC_MODE=0 ...
HeapAdjuster: HEAP_SIZE=2000 POOL_MEMORY=256 (a line per part)
```

The version and the git hash come from `BuildInfo.h` (section 15): `-dirty` after the hash for a tree with uncommitted changes, `unknown` outside a git repository.
A test build adds a line that says so, with its switches (section 13.1).
A part that is off - in `TweaksV.ini`, or for the session by the conflict check, which runs before the header (section 3) - says `off` in the `Features:` line; its values line still follows, and the part itself logs nothing.

### 6.3 One wording for every part

Every part logs the same things the same way, under its own tag; these are the forms, and the code and the suites hold the words.

| What | Line |
|---|---|
| a patch of bytes | `<What> patched: <old> -> <new>.` (`RPF cache check patched: 41 57 -> C3 90.`) |
| a moved array | `The <array> moved and its limit raised from A to B entries.` |
| a switch that is off | `<KEY>=0 - <what> is not done.` |
| a job not done, for the summary | `PROBLEM: The <job> is not done this session: <why>` (`The modkit limit is not changed this session: M-E-R1 was not found.`) |
| a missing pattern | the scanner's line, then the part's reason above in the words of `Scanner::NotAsOften` (section 8) |
| something another mod changed | left alone and logged, not a problem, `... - left as it is.`, or refused with the reason `(another mod may have hooked it already)` where TweaksV cannot tell what it would be writing over |
| a kind of line per event, past its cap (`kMaxLinesPerKind`, section 12) | `20000 lines of <kind> written - from now on they are only counted.`, ending `...; the summary every minute has the numbers.` where there is one, with `DIAGNOSTIC: ` before it for a kind that comes only with `DIAGNOSTIC_MODE`; the folders of the mods folder that cannot be read have no such line, their number being in RageOpenV's problem (section 11.6) |
| `DRY_RUN` | `DRY_RUN: would patch the <what>: <old> -> <new>`, `DRY_RUN: would move the <array> (N references) and raise its limit from A to B (M sites)<what the part adds>.` (` (M sites)` only where a limit is written), `DRY_RUN: would redirect the <call> at RVA ...`, and a line of its own for a job of its own |
| `DIAGNOSTIC_MODE` | a part's lines start with `DIAGNOSTIC: `: its sites (`DIAGNOSTIC: reference at RVA ... -> RVA ... (M-E-R1)`, `DIAGNOSTIC: limit at RVA ... holds 1023 (N-1, M-E-L1) -> game size 1024`), every write (`DIAGNOSTIC: <what> at RVA ...: <old> -> <new>`) and the measurements; the scanner's pattern lines and DirectStorage's lines carry their own wording (sections 8 and 11.8) |

Addresses inside the game are written as `RVA 0x...`, the offset from the start of `GTA5_Enhanced.exe` in memory, so they can be compared across boots and with PATTERNS.md; an address outside the game as `0x... (outside the game)`.

### 6.4 Problems and the start-up summary (`TweaksVSummary`)

`ReportFailure(tag, what)` logs `PROBLEM: <what>` under the part's tag and keeps the item (`[Tag] what`) for the start-up summary, under a lock held through the same guard.
`ShowStartupSummary()` shows them once in one message box, 40 at most and the rest counted, with the log's path and the switch that stops these messages; with `ERROR_POPUP=0` the log says so in the box's place.
`ShowStartupSummaryLater()` does that from a thread of its own, because a message box must not block `DllMain` or the game's thread that first asks for the time.

A problem reported after the summary was shown - a part that stops working later, such as a file in the mods folder that RageOpenV cannot open - comes a second later in a box of its own, from a thread of its own (`late report`).
A finding is no part failing: the part is doing its job and has found what the player must know.
`ReportFinding(tag, finding, what)` logs it as `PROBLEM: <what>` too, but it is never an item of the start-up summary, whose heading and last line do not fit it: it goes to that same thread, whether the summary has been shown or not.
Each kind has its heading and its last line in a table in `TweaksVSummary`: the part that stopped working, `Finding::LegacyPacks` (section 11.6) and `Finding::RefusedFiles` (section 11.10).
The thread shows a box per kind with something in it, at most three of each per session, so that a part that keeps failing cannot fill the screen nor take a finding's box; past that, and with `ERROR_POPUP=0`, a report is only logged.

`StartModuleThread(thread, what)` starts these threads, RageOpenV's and DirectStorage's checks, the static bounds check and the threads of section 6.6 with an extra reference to the module, which the thread drops when it ends, so that `TweaksV.asi` cannot be unloaded under it.
One that cannot start is logged under `[TweaksV]`, `The <what> thread could not be started (Windows error N).`; the next problem or finding reported tries the summary's or the late reports' again, and with none to come the start-up's problems stand in the log alone.

### 6.5 The console

`CONSOLE=1` opens a console window once the header is written, which mirrors every log line from then on; the RageOpenV part then NOPs the game's call that would hide it (section 11.6).
It is a testing aid: when it cannot be opened or kept, the log says so and nothing is reported.

### 6.6 The measurements (`TweaksVMeasure`)

One loop serves `DIAGNOSTIC_MODE`'s eleven measurements of the parts, the six of the parts measured first (section 11.16), DirectStorage's `UNLOCK_FILES` summary and the weapon infos guard's report (section 11.10.1).
A part describes its measurement as a `Measurement { thread, firstMs, numbers, summary, start }` and starts it with `StartMeasurement`, through `StartModuleThread` (section 6.4); the thread never ends, so it keeps its reference to the module, and nothing leaves it (a `try`).
The first summary always comes, `firstMs` in: `kFirstSummaryMs`, 10 seconds, or `kSummaryEveryMs`, a minute, for DirectStorage's open summary.
Then one comes every minute while `numbers` changes, in a test build every `TWEAKSV_TEST_SUMMARY_MS` milliseconds when that is set (section 13.1).
`start`, if given, is an event the part sets at the first thing it counts, before which `firstMs` does not begin, so that a hook never starts a thread itself; an event that could not be made starts the clock at once.
A measurement whose thread cannot start is logged, not reported: it is an aid that changes nothing in the game (section 12).
The exception is the `weapon infos guard report`, the guard's own: its thread not started, the guard is not set up, a problem of the part (section 11.10.1).

The one data check is `Misplaced(at, size, unit)`, what keeps bytes from being the game's data read `unit` bytes at a time, each read whole, `not inside the game` or `not aligned to 8 bytes`, and `DataFault`, which adds `cannot be read`.
Every part that reads the game's data checks it through them, a caller naming another stretch of the game in place of the whole (the render queues, the game's `.data`) and each its own words around the reason.

## 7. The conflict check (`TweaksVConflicts`)

`CheckForConflicts()` looks in the three places an ASI loader loads from (`kLoaderFolders`: the game folder, its `scripts` and its `plugins`) for every old plugin and every plugin that stays of every feature (section 4.3).
An old plugin found switches its part off for the session and is reported, with the word to delete it.
A plugin that stays does the same while its part is on in `TweaksV.ini` - `IsEnabledInIni`, not `IsFeatureOn`, so that an old plugin found before it, which switched the part off already, does not keep it from being said - and its report names the `ENABLED=0` that stops the message, at every start until then.
A part already off in `TweaksV.ini` gets no item, only a line in the log, which also says when the check found nothing.

The plugins that stay:

- `onigiri.asi`, NaturalVision Enhanced's loader, 1.0.1.0 (PE timestamp 0x68543a76), for RageOpenV and StraightToStoryMode: a mods loader that serves loose files and archives from its `onigiri` folder and skips the splash and legal screens by its `onigiri.ini` (`SkipSplash` and `SkipLegal` on, `SkipIntro` off).
  It loads before TweaksV and patches nothing in its DllMain - at TweaksV's start every site held the game's own bytes - but patches at the game's first request for the time, after TweaksV's changes, and uses what it finds there unchecked.
  It faults with an access violation at its offset 0x1ea45 when RageOpenV has changed the cache check, the five RPF encryption calls or the console call, and at 0x1f2be when StraightToStoryMode has changed the splash and legal `jz` or the landing-page function; Windows' application log names `onigiri.asi` as the faulting module in every such crash, which the game's own handler does not dump.
  That is what a report to NVE's author needs; TweaksV cannot make another mod check its results, it can only leave those sites alone while that mod is there, and nothing of onigiri's code was read.
- `ResourceExtensions.asi`, Resource Extensions [Enhanced] 2.4 by Koukol, for HeapAdjuster, PackfileLimit, Blips, RayTracing and LightCapacity: a closed-source plugin for Enhanced 1.0.1158.16 that raises many of the game's capacities.
  Its page, read on 2026-10-30, names the archive count, the story blips' threshold and, with its key `BvhVertexArena`, extra capacity for dynamic ray-tracing geometry, and asks to remove the heap and packfile adjusters.
  It stays for its other jobs, so those five parts are off while it is there; its redistribution is not allowed, and nothing of its code was read.

`IsPluginInstalledFor(section)` answers from that one search whether a plugin of either kind does a part's job, whatever `TweaksV.ini` says of the part, false when the check did not run: for StaticBounds, which leaves the game's code to such a plugin even while `TweaksV.ini` switches Blips off, since its check would hook the script programs Blips' plugins work on (section 11.15).
`IsInsideGameImage(p)` is the shared test for an address inside `GTA5_Enhanced.exe`'s image, and `IsInsideGameData(p)` the same for the section the game's section table names `.data`, for the one write TweaksV makes outside the game's code (section 11.14).

## 8. The scanner (`TweaksVScanner`)

The scanner finds TweaksV's patterns in the running game's code, logging under `[Scanner]`.
`GTA5_Enhanced.exe` is encrypted on disk, so the code is only readable in memory, and a game update changes it: what the scanner finds is the one thing to maintain after an update (PATTERNS.md, "When Game Updates").

Every search reads the readable parts of the game's executable sections afresh, because TweaksV itself changes protections between two searches and a page the game protects must be skipped, not crash the game.
A pattern's tokens are exactly two hex digits or `?`; any other token, or a text of wildcards only, refuses the pattern as not found, and nothing in the parser can throw.
The search compares a pattern's two rarest fixed bytes, by how often each occurs in the first 256 KB of the game's code, 16 positions at a time with SSE2, and the whole pattern only where both match, about ten times faster than going through the whole image byte by byte; which bytes it takes changes how fast it is, never what it finds.
The pattern parser is an altered version of ScriptHookVDotNet's memory scanner (THIRD_PARTY_NOTICES.md); the search is TweaksV's own.

- `Scanner::FindOnce(pattern, why)` returns the one match or `nullptr`: a pattern that matches twice is not used, because what would be written over cannot be known.
  It leaves the reason in `why` unless `why` holds one already, so that a part looks for all its patterns, the log naming each one missing, and refuses its job with the first, in the words of `Scanner::NotAsOften(pattern, found, expected)`: `<label> was not found`, `the game code holds <label> 2 or more times, not the expected 1`, with no point at the end, since the part's sentence ends with its own.
- `Scanner::FindAll(pattern, limit)` returns every match up to `limit`, for a site the game repeats: one it must have exactly n times is asked for n + 1, so that an n + 1th match shows.
- Both log a miss as `Pattern <label> not found: <bytes>`, and with `DIAGNOSTIC_MODE` the RVA, the number of matches and the bytes of the match; a pattern that matches more than once also gets a line for each of its first ten matches, 16 bytes before and 48 from its start, so that a longer pattern can be chosen that matches only the place it is meant for.
- The searches by address go through the code the same way, return their finds in address order up to `limit` and log nothing.
  `FindRipRelativeLeas(target, limit)` finds every `lea r64, [rip+disp32]` that leads to `target`: how the relocation core finds the loads of an array the game loads in places no pattern can tell apart.
  `FindRipRelativeReferences(target, limit)` finds those and every `mov [rip+disp32], r64`: how the core checks that the references an array's patterns name are all the game has.
  `FindCallsTo(target, limit)` finds every `call rel32` that leads to `target`, compared on the opcode and on the rel32's lowest byte, which over 16 positions in a row counts down by one: how PedProps checks that the game calls a function exactly as often as the research found (section 11.11).
  `FindRipRelativeStores(target, size, limit)` finds every instruction that writes to memory at `[rip+disp32]` and so to any of the `size` bytes from `target`, each given by its first byte, its prefixes counted: how the texture budget checks that the start-up's two stores are the budgets' only writers (section 11.12).

The search for stores takes in moves of 1 to 32 bytes and the integer unit's read-modify-writes; x87 stores and AVX-512's are not looked for.
For every ModRM of mod 00 and r/m 101 whose disp32 leads near the range, within the 4 bytes of an immediate and the 32 of the widest store before it, the bytes before the operand are read every way an instruction could start among them, since any of them may end the instruction before instead.
The readings: as one with a VEX prefix (`C4` with a map of 1 to 3, or `C5`), then with the escapes `0F 38`, `0F 3A` and `0F`, then with a one-byte opcode, the last two each with a REX right before the opcode and up to four legacy prefixes before that, and a reading whose `F3` or `F2` makes a store a load (`movq`'s, `crc32`'s) once more without that prefix and the bytes before it.
The operand counts when any reading is a store that writes a byte of the range, so a byte the instruction before ends with - a `0F` taken for an escape, a `C4` or `C5` for a VEX prefix, an `F3` for a prefix - hides no writer.
A load whose bytes read as a store some way (`movups`'s `0F 10` and `vmovups`'s `C5 FC 10` as `adc r/m8, r8` behind a `0F` or a `C5`, `movq`'s `F3 0F 7E` as `movd` behind an `F3`) is counted as one, given by the byte that reading starts at, which is safe where a missed writer is not.
The immediate after the disp32 counts, since the rip a store's operand is counted from lies past it; a prefix that narrows a store (`66`, `F2`, `F3`) is not counted for its width, since it may belong to the instruction before, so a store is taken for at most as wide as it can be, never narrower.

## 9. The patch module (`TweaksVPatch`)

Every change to the game's code goes through this module, so that every part writes the same way and no part has to know about page protection, `DRY_RUN` or `DIAGNOSTIC_MODE`.

- **`WriteCode(writes, tag)`** is all or nothing: every page any write touches is made writable first, and one that cannot be stops everything before a byte is written, with the protections changed so far put back.
  Every page gets its own protection back through a guard that runs on every exit, an exception included, and nothing that can throw runs while a page is writable.
- **`RedirectCalls(redirects, alsoWrite, tag)`** sends 5-byte relative calls in the game to TweaksV's functions, together with `alsoWrite`, in one `WriteCode`, all or nothing too.
  A call reaches only 2 GB, so each goes through a 16-byte call stub near the game, in a block written once, then read+execute and never changed again: a call stub is never writable and executable at once.
- **`HookFunctions(hooks, tag)`** hooks functions at their start with MinHook: every hook is created and queued, then all are switched on together, the threads frozen once per group, not once per hook; if any step fails, the hooks made so far are removed again.
  `HookExport` hooks an exported function of a loaded DLL at once, and `DisableHook` switches a hook off again, the trampoline kept for a thread that is inside the hook right now.
  MinHook's trampolines are writable and executable, an accepted open issue (ISSUES.md, T-HARDEN).
- **`AllocateNearGame(size)`**, for the moved arrays and the call stubs: read/write, committed and zeroed, within 1 GB below the game's image, where a rel32 in the game's code reaches.
- **The readers.** `IsFunctionStart(address, detail)` asks the game's unwind data of Windows; for code without unwind data the byte before it, read only where it is readable, must be `CC`, `90` or `C3`.
  `IsReadable(address, size)`, over the whole range, comes before any byte outside a pattern's match is read.
  `CopyMemorySafely(target, source, size)` catches an access violation and returns false, nothing else being caught, for memory that may change or be freed between a check and the read: the pointers the game's DirectStorage calls hand the part's hooks, the heap data a measurement or a guard walks while the game changes it.
- **The small helpers** are in `TweaksVPatch.h`; `Printable(name)` makes every byte outside printable ASCII of a name from a file or the game's memory `?`, so that a line stays one line.

## 10. The relocation core (`TweaksVRelocation`)

Six parts move one of the game's static arrays to a larger one and raise the limit the game's code carries for it: the modkit array, the packfile array, the box streamer's traversal list, the decal definitions array, the weapon component list, whose size no instruction carries - it is the distance to the count that follows the list - and the siren setting ID table, whose size no setting chooses.
They describe their array in tables; the core does the work once for all of them.

### 10.1 The tables

`TweaksVRelocation.h` holds the tables' fields and the builders that fill them.

- An `Anchor` is a pattern of the part, how often it must match, and what every match holds: a reference, up to three limits, both, or neither - a site the part reads itself through the plan.
- A `ReferenceSite` is the rel32 of a `lea r64, [rip+disp32]` or of a rip-relative `mov` that addresses the array.
- A `LimitSite` is an `Immediate`, an imm32 that holds `N x mul + add`; a `Variable`, the rel32 of a variable the game compares against, which holds `N + add`; or a `Follower`, the disp32 of an instruction that addresses a variable right after the array, the distance to it being N, in entries, and the variable never written.
- A `RelocationSpec` is an array's names and sizes, its `setting`, its anchors and:
  - `maxEntries`, the new array's size, always allocated in full: the setting's maximum or more (the weapon component list holds one file more), or, with no setting, every value of its index (the siren setting ID table);
  - `scannedReferences`, for an array whose loads no pattern can tell apart, exactly how many `lea` must load it, found by its address, with the REX bytes they may start with (`48` or `4C`; any with REX.W for the box streamer);
  - `expectedReferences`, for an array moved by its anchors' references, how many references the game's code has to it (6 for the modkit array, 33 for the packfile array; 0, not checked, for the others);
  - `emptyEntry`, the byte the new array's entries past the game's own hold: zero, as Windows gives it, or 0xFF, a free ID, for the siren setting ID table.
- A `RelocationPlan` is what was found, the size to use, and every anchor's matches, for the sites a part reads itself.

### 10.2 `FindRelocation(spec, plan)`

Everything is found and checked before anything is written, and the first failure reports `<job> this session: <why>` and leaves the game as it is.
The anchors are worked through in table order, so a missing pattern late in the table is reported only after the sites before it passed:

1. The anchor matches exactly as often as it says.
2. Its reference starts with the opcode when one is given; the first target lies inside the game, else another mod may have moved the array already, and every other points at the same address.
3. Its limits are the expected instruction; a variable lies inside the game; a value decodes to N through its encoding; a follower's variable lies a whole number of entries after the array; the first size decoded is plausible (neither zero nor beyond `maxEntries`), and every other agrees.

Then:

4. The references that are moved are the anchors', or - with `scannedReferences` - every `lea` that loads the array, exactly that many, since another mod may have moved some or the game has changed, each with an allowed prefix.
5. With `expectedReferences`, `FindRipRelativeReferences` must find exactly that many and every one an anchor's rel32, because one no pattern names would keep the game's own, smaller array while the limits are raised.
6. An array without a setting takes `maxEntries`, with nothing to clamp or report.
   Else the size to use is the setting, clamped to the game's own size..the setting's maximum, reported when it lay outside, so that the start-up summary names the size really used.
   Smaller than the game's own would shrink the array; where the game's own size lies past that maximum it is all there is to use, and the part decides whether its array has room for it.

### 10.3 `ApplyRelocation(spec, plan, alsoWrite, redirects, also)`

With `DRY_RUN` it logs what it would do (section 6.3) and nothing else.
Otherwise the new array is allocated in full, near the game, and the game's own entries copied into it, so an array the image initialises keeps its content, the rest holding `emptyEntry`.
Every reference's rel32 is recomputed for it, with the int32 distance check, every limit re-encoded for the new size, a follower staying as it is, and the part's own writes added.
All of it is written in one `WriteCode`, or, when the part redirects calls too, one `RedirectCalls`, all or nothing.

What does not fit the core stays in the part's file and uses the plan (section 11).

## 11. The parts

Each part reads its switches once, at its start-up (RageOpenV's runs inside its one-shot time hook); the hooks that run per call never read them.
Here stands each part's design, beside what sections 9, 10 and 12 say for all of them: its mechanism, why it is so, its limits and what it was measured to do.
The player's view is README's; the game facts - the patterns, what each match holds, the offsets - are PATTERNS.md's.

### 11.1 HeapAdjuster (`InitHeapAdjuster`)

The game sizes its memory heap with one immediate in its start-up code (`H-E1`), and `HEAP_SIZE` sets it in MB, up to 2047, the most whole MB that imm32 holds as a positive int32.
A value of 1024 MB or more - up to 4095 MB, since the `mov` zero-extends its imm32, past what `HEAP_SIZE` allows too - is not the game's own (646 MB on 1.0.1158.16): another heap adjuster set it, and the part leaves it, logs that and reports nothing (README's "Other mods").
Else the setting is clamped to the size found..2047, a value outside reported so that the start-up summary names the size really used.

A pool of the game's does not draw on that heap: it draws on a budget of its own, kept by a big-block allocator (PATTERNS.md, "A pool's memory"), past which an allocation returns null and the game writes through it, the crash at RVA 0x15A514.
`POOL_MEMORY` sets that budget in MB, from the game's own 136 to the same 2047, in an immediate of the same start-up function (`H-E2`), which the part writes at the same moment as the heap's.
It is a ceiling on a counter, so raising it allocates nothing; and it is a job of its own, so that either goes on without the other.
Any value but the game's 136 MB is another mod's or a game update's, and an allocator, heap or constructor outside the game another mod's: each left and logged, no problem.
The write's effect comes later, when the game's start-up hands the immediate to the allocator's constructor, so it is checked at the write: an allocator whose budget field already holds the game's own 136 MB was set up before TweaksV started, and the write, made all the same as the heap's is, comes too late, a problem with `DRY_RUN` too and without `DIAGNOSTIC_MODE`.
A field of 0, or of any other value, says nothing: nothing has proved what an allocator not yet set up holds, so no write is refused on it.
`FindPoolMemoryBudget(why)` gives the imm32's address to the ped pool's module (section 11.16.5), which works with the part off too: `H-E2` is looked for once, whoever asks first.

With `DIAGNOSTIC_MODE` the part measures the budget as it leaves it, with `DRY_RUN` and with another mod's value too (section 6.6): the use, the least room so far and the blocks.
Less than 1 MB of least room adds "nearly spent", a line of the log and no box (D31).
The two things a pattern cannot pin are checked at each summary, each adding a line: that the allocator's budget is what its site holds, and that the use and the room add up to it.

### 11.2 ModkitLimit (`InitModkitLimit`)

The game's array of vehicle modkit IDs: 1024 entries of 2 bytes on 1.0.1158.16, loaded by six `lea` (`M-E-R1..R6`) and sized by eight immediates (`M-E-L1..L8`) that hold N-1, N+3 or 2N.
The part lies entirely on the core, with nothing of its own; the six must be every reference the game's code has to the array (section 10.2).
Modkit IDs are 16-bit, which caps the array at 65536.

### 11.3 PackfileLimit (`InitPackfileLimit`)

The game's array of RPF archives, on the core: 7260 entries of 8 bytes on 1.0.1158.16, which 33 instructions reference, every reference the game's code has (section 10.2), one of them by game build (`P-E-R31a` or `P-E-R31b`).
Two limits size it: an immediate, by game build too (`P-E-D3`, `P-E-D2` or `P-E-D1`), and a variable the game compares an archive index against (`P-E-I1`), which holds four less than the array's size, a margin the core keeps (`Variable(2, -4)`).
Archive indices are 16-bit, which caps the array at 65535.
The anchors are assembled at start-up by the game's file version (`GameVersionNumber`); without one nothing is done, and the start-up summary says so.

The third limit lies in the game's configuration.
The call that reads it (`P-E-H1`) must lead into the game - one another mod has redirected refuses the job - and goes, in the same all-or-nothing write as the references and limits, through a call stub to `SetStreamingInterface`, which replaces the archive count the game read from `gameconfig.xml` with the new size, logs what it replaced and calls the game's own function.
The count is replaced exactly, also one above the new size: the game's own configuration holds 8512 on 1.0.1158.16, more than its array's 7260, so a `PACKFILEARRAY_SIZE` below 8512 lowers the count.

### 11.4 BoxStreamer (`InitBoxStreamer`)

The game's box streamer answers "which map boxes lie in this area" in four query functions that hand one static list of 1000 entries of 8 bytes, and its capacity, 1000, as the fifth argument to one of two tree searches; a search stops 16 entries short of the capacity (`kSearchHeadroom`, also the measurement's threshold).
With heavy map mods the list fills up, and the boxes it has no room for are never streamed, so their LODs are missing.
Eight `lea` load the list, and two of the query functions are near-identical copies that no pattern tells apart (PATTERNS.md), so the core takes the list's address from the `lea` that `B-E-L2..L4` end in and scans the code for every load of it, which must find exactly eight (`scannedReferences`, any REX.W prefix); the four capacities, `B-E-L1..L4`, are its limits.

With `DIAGNOSTIC_MODE` the call to the tree search after each capacity site must also lead into the game, and each pair to the same function.
A call that fails either check refuses the whole job, the list included, one of section 12's exceptions, because the measurement is the reason the switch is on.
The four calls then go, in the same write as the references and capacities, to two wrappers (`CountedSearch0`, `CountedSearch1`) that call the game's search and count the result with atomics, nothing else on the game's streaming thread.
The measurement (section 6.6) logs the number of searches, the most entries one returned and how many returned more than the game's own list could hold.
With `TRAVERSAL_LIST_SIZE` at the game's own size a search stops where the game's own would, so that last number is 0 by construction, and the summary says so.

Measured on 1.0.1158.16 with Forests of San Andreas: Revised and GTA V Remastered: Enhanced loaded: three minutes gave 180353 searches, the largest 1123 entries and 7209 past the game's own list; ten minutes of driving through those forests and the city gave 946209, 1203 and 24244, each search past that list cut off without the larger one.

### 11.5 Decals (`InitDecals`)

The game reads its decal definitions from the `decals.dat` files into one static array of 260 entries of 80 bytes (the entry layout is in PATTERNS.md), with the count registered so far in a dword 16 bytes before it.
Once the count is past 259 the parser refuses every further definition, silently, and add-on content, which comes last, loses its decals.
22 instructions carry the limit as an imm32 (13 hold N-1, 9 hold N), and 31 `lea` load the array, which the core scans for with the prefixes `48` and `4C`.
A definition's index is a uint16 in the game's decal instances, so a larger array would be possible; 16384 entries (1.3 MB) is the limit TweaksV allows.

The part's own checks: the array lies exactly 16 bytes after the count, and the decal manager's start-up loop (`D-E-L10`, `D-E-L11`) loads the array and ends at its start offset + N x 80.
It adds one write to the core's: the loop's end becomes the start offset + 80 x the largest multiple of the entries per pass (26) that is not above the new size, 2028 of the default 2048; the entries it leaves are zero anyway, as the game's own are when the core copies them in, the array's initial state.
The count stays where it is: nothing reaches it through the array, nor the array through it.

The measurement, with `DIAGNOSTIC_MODE` (section 6.6), hooks nothing: it reads the game's count and logs it against the game's own 260 and the array in use.
With `DRY_RUN` it measures the unpatched game.

Measured on 1.0.1158.16: vanilla registers 259 definitions, one short of the game's own 260, so a mod that adds two is already one too many.
With a test `decals.dat` of 659 definitions (400 copies of a definition only a heist scene places, put first, then vanilla's 259) the part held all 659 and every decal type appeared; with the part off and the same file the game's own array filled with the copies and not one bullet hole, blood splatter or tyre mark appeared.

### 11.6 RageOpenV (`InitRageOpenV`)

Three jobs with a switch each, each done all or nothing and left undone alone, plus the console patch and the check for packs made for GTA V Legacy.
The game's file system is not up in `DllMain`, so `InitRageOpenV` only hooks `kernel32`'s `GetSystemTimeAsFileTime`.
The hook runs the start-up once, on whichever thread first asks for the time, the claim an atomic exchanged first because the start-up can ask for the time itself, then shows the start-up summary from a thread of its own and switches itself off.
The start-up check is a thread that waits 30 seconds and, if the start-up has not run, claims it, reports a start-up that never ran, shows the summary, which would otherwise never come for any part, and switches the hook off.
When that thread cannot start, that is reported and `DllMain` shows the summary (section 6.4).

The start-up, in this order:

1. **The current directory** is where the mods folder is looked for, because the game's own file functions resolve `mods/...` there too.
   When it is not the game folder, told by `FinalPathOf` so that a short name or a junction of the game folder is the game folder, with `MODS_FOLDER=1` a `mods` folder in the game folder, which the game then does not use, is a problem.
2. **The console** (`CONSOLE=1` only): the game's call that would hide it (`R-E-C1`) becomes NOPs; bytes not as expected stay, logged, no problem for the summary.
3. **The cache check** (`SKIP_RPF_CACHE_CHECK`): the function `R-E-K1`'s call leads to returns at once (`C3 90`); one another mod has skipped or hooked is left alone, logged, no problem.
4. **The mods folder** (`MODS_FOLDER`): the eight file functions of the game's two file devices (`R-E-F1..F8`) - OpenBulk, GetFileTime, GetFileSize and GetAttributes, twice - are hooked together, so that a path that exists under `mods\` is served from there.
   GetFileTime and GetFileSize are found by a pattern inside them, so another mod's hook on them is hooked on top and chained through; the other four patterns match the function's own first bytes, so another mod's hook there makes the pattern miss and the mods folder is not used.
   Without a mods folder, or with an empty list, nothing is hooked (`DIAGNOSTIC_MODE` still checks the patterns); a mods folder that cannot be listed at all, or none of whose names can be used, is a problem.
5. **OPEN archives** (`OPEN_RPF`): five calls of the archive code (`R-E-E1..E5`) are redirected in one `RedirectCalls`, so that an archive whose header says `OPEN` is read without decryption.
   The encryption tag is kept per thread and used up by the parse it was found for, so that archives opened at the same time never mix (PATTERNS.md, "When Game Updates" item 5, has what that rests on).
   An OPEN archive's entries are marked as the game expects (PATTERNS.md's RageOpenV) only when its table is plausible and can be read, because the table is the game's heap memory and a field moved by a game update would point anywhere; else the archive is left as it is, logged, the first of a session reported once.
6. **The diagnostic lines** (`DIAGNOSTIC_MODE`, README) are counted by the measurement (section 6.6).

**The mods folder's one walk** (`WalkModsFolder`, `TweaksVModsFolder`): RageOpenV's list and the check below go by it, so that the two list the same files the same way and stop at the same name.
Its root is `mods`, relative to the current directory, so that Windows refuses a walk what it refuses the game's functions: in a process that is not long-path aware, a full path past `MAX_PATH` (Windows error 3).
Its limits are 200,000 names handed on and 800,000 looked at, so that a folder of names left out is bounded too and not by the disk alone, the first past either ending the walk, and 32 levels deep, what a folder at that depth holds not read.
Names are spelled in UTF-8 and paths limited to 254 bytes, by what the game's file functions take on every Windows, whatever its ANSI code page (PATTERNS.md, "When Game Updates" item 5).
A name that is not valid Unicode (an unpaired surrogate, which UTF-8 cannot spell) or a path longer than that is left out and counted, since the game cannot ask for it.
A folder that cannot be listed, or not to its end (a share that drops), is handed on as such and the walk goes on; the mods folder itself ends it when no name of it was handed on.

**The list** (`ReadModsFolder`) is read once, before the hooks go on, and never freed, since the hooks stay on until the end of the process.
A walk that stopped, a folder too deep and a folder not read report that only part of the mods folder is used; each folder not read also gets a line, 20,000 at most, with no line of its own at the cap, since the problem has their number.

**The hooks** spell the game's path on the stack and look it up: no file system call, memory allocation or lock on the game's file calls.
`ListSpelling` is one fold for the list and the lookups, as Windows compares file names, all of it checked against NTFS on my PC: the file systems compare upcased names, unit by unit by a table of the Basic Multilingual Plane (the Kelvin sign, U+212A, shares its lower case with `k`, not its upper case; a Deseret letter keeps its case where `LCMapStringEx` would upcase it; U+0250's upper case, U+2C6F, takes a byte more, so a path's length is taken before the fold).
A path that is not relative, not valid UTF-8, longer than 254 bytes or with `..` is never redirected.
A path the list has goes to the game's own function as `mods/` plus the game's own spelling, and what that returns is what the game gets: none of the four falls back to the game's own file, so the player's file is never replaced behind their back.
A mods file OpenBulk cannot open is that failure for the game, logged, the first of a session reported once.

**The check for packs made for GTA V Legacy** (`TweaksVLegacyPacks`, no key of its own).
A pack made for GTA V Legacy loads in Enhanced, but its models and textures keep Legacy's resource versions, and a vehicle made of them traps the game as it is created (PATTERNS.md, "RageOpenV").
Once the eight hooks are on, `StartLegacyPacksCheck()`, the one call, starts the check's thread (`Legacy packs check`), so the game's thread never waits for it; with `DRY_RUN`, and wherever the mods folder is not used, the call is never made.
The check writes nothing, to the game or to a pack.

Every `.rpf` file the walk gives is a pack, named by its path from the current directory.
It is opened by that relative path, so that a pack the game cannot reach gets no verdict and is named as not checked, and with every sharing, so that the game opens it as if the check were not there.
Its archives are read from a queue, no recursion, only their headers, entry tables and names, by the RPF7 facts in PATTERNS.md's RageOpenV, one set in `TweaksVRpf.h` that both files include, so the two never disagree.
An archive that is not OPEN, by its header or by the encryption of its bytes in the archive that holds it, is not read; 0 names no encryption, so such a table is read as it is, and were it encrypted after all, its first entry would not be the root folder and the pack would be named as not checked.
A resource named `.yft`, `.ydr`, `.ydd` or `.ytd` with Legacy's version (PATTERNS.md's RageOpenV) makes the pack one made for Legacy, and a binary named `.rpf` is an archive nested in this one.

Every count and size is bounded, so that no buffer is sized by a file's numbers unchecked: 1,048,576 entries per table; header, table and names inside the archive; 65,792 bytes of names, all that a file's 16-bit name offset can reach, each name ending in a zero within 256 bytes; a nested archive stored, inside the one that holds it and past that one's names, so that it is smaller and the nesting ends; 8 levels of nesting; 4,096 archives and 64 MB of entry tables and names per pack; 1 GB in all.
A damaged archive gives its reason, the pack keeps the first and is named as not checked (in full), and its other archives are still read; past the pack's own bounds the pack is left, and past 1 GB that pack and every pack after it.

A pack made for Legacy gets a line with its Legacy resources and the first of them, a pack not read in full one with its reason, and a folder the walk could not list one too, since this walk comes later, on a thread of its own, and can lose a folder RageOpenV's did not; each kind stops at 20,000 lines.
One problem names them, ten of each kind and `, and N more`, and tells the player to convert a pack made for Legacy with CodeWalker or take it out.
It leaves out the packs not read only for archives that are not OPEN, named in their lines alone, since the game's own archives are encrypted and a pack made with OpenIV or CodeWalker is OPEN.
The problem is a finding (`Finding::LegacyPacks`, section 6.4): a box of its own, whether or not the start-up summary has been shown.
That the check is not done is a problem of a part failing: a thread that cannot start, an exception that ends it, or the mods folder itself, which RageOpenV has just listed, when the check cannot list it, with no last line saying it found nothing.

### 11.7 StraightToStoryMode (`InitStraightToStoryMode`)

Three screens, three small patches, one switch each, each failing alone, so a missing pattern skips only its screen:

| Screen | Site | Patch |
|---|---|---|
| splash screen | the `jz rel32` before `S-E1`'s match | a `jmp rel32` to the same target, computed from the game's own displacement (one byte shorter, so the displacement grows by one), and a NOP |
| legal screen | the 6-byte `jz` before `S-E2`'s match | six NOPs |
| landing page | the function the call at the start of `S-E3`'s match leads to | `xor eax, eax; ret`, so its choice returns 0, Story Mode, at once |

A landing-page function that starts with a jump is another mod's hook, refused and reported; a screen already patched, by an old plugin under another name, is skipped quietly.

### 11.8 DirectStorage (`InitDirectStorage`)

With DirectStorage, the game has it open every file in the game folder at start-up (`IDStorageFactory::OpenFile`), letting others only read them, and keeps them open (PATTERNS.md's Status), so mods cannot write their own files there and scripts cannot be reloaded while the game runs.
The part uses no game pattern and its job needs no DirectStorage: `UNLOCK_FILES` hooks `NtCreateFile` in `ntdll.dll`, deciding every open by its own name, access and sharing, whoever makes it, and `NtSetInformationFile`, deciding every rename that replaces a file by its target; a hook that cannot be made is a problem.

The part frees files and refuses nothing.
A filter on `OpenFile` must never refuse a path outside the game folder: the game reads its saves, `pc_settings.bin` and its cloud files in `%TEMP%` through it too, and one that refused them made the game find no save, start the prologue and rewrite `pc_settings.bin` with defaults, the saves themselves untouched.
Refusing inside the game folder breaks a mods loader that serves loose files through DirectStorage: next to NaturalVision Enhanced's `onigiri.asi` the game died in its own fatal trap 22 ms after the first refusal.

**The opens** (`CheckOpen`) are decided before the call, on the stack, under a guard against a bad pointer.
A full name is compared with the game folder; a name relative to a folder handle, which is what Windows makes of every relative path in the current directory, lies in it when that handle is the game folder's, told by its identity (the volume serial number and the file ID, one query of Windows per such open, two on a file system without file IDs).
An open relative to another folder, by file ID or with a name that cannot be read cannot be examined, and a game folder whose identity cannot be read is a problem, since its relative opens then stay locked.
An open in the game folder is then, in this order: a directory (a folder opened without `FILE_DIRECTORY_FILE` cannot be told from a file before the call), an archive (`.rpf` or `.cache`), a write open, an exclusive open (its sharing leaves out `FILE_SHARE_READ`), and otherwise it is widened: `FILE_SHARE_WRITE` and `FILE_SHARE_DELETE` are added before the call goes on to Windows.

**Windows' rules for a file with an open handle**, which the part stands on and which set its two limits (README):

- Sharing is decided per handle: a new open succeeds only if its access fits the sharing of every handle open on the file and its own sharing allows every such handle's access.
  So a program that opens a widened file for writing while sharing nothing still gets a sharing violation while the game holds it; one that lets others read while it writes, as editors and the C runtime's file functions do, and a rename are not affected.
- Whatever the sharing, such a file cannot be replaced by a classic rename (`FileRenameInformation` with ReplaceIfExists, what `MoveFileEx` with `MOVEFILE_REPLACE_EXISTING` and `std::filesystem::rename` make): access denied.
  That is the usual safe way to save a settings file, and my LuminanceV's panel got access denied for its INI while the game ran.
- A rename with POSIX semantics (`FileRenameInformationEx` with `FILE_RENAME_POSIX_SEMANTICS`), `ReplaceFile`, a delete followed by a new file and a write in place go through.
  A POSIX rename needs every handle on that file to share deleting, which `CheckOpen` gives the game's opens; the holders go on reading the file they opened, and the name is the new file's.

**The replace-renames** (`CheckRename`): a rename made inside the game's process that replaces a file in the game folder goes to Windows with POSIX semantics, from a copy on the stack, so that a mod's safe save goes through while the game holds that file, without a change of its own.
Programs outside the game are not touched: their classic replace still gets access denied.
A target lies in the game folder by a full name or a name relative to its handle, told as for an open, or by a bare name - a rename inside the source's own folder - whose source lies below it, told by its object name (`NtQueryObject`), one query on the stack, against the game folder's read the same way at the start-up.
by design not by its final path (`GetFinalPathNameByHandleW`), which would allocate, take locks and ask the mount manager inside the hook, so a source opened by a short 8.3 path goes on as made.
A game folder whose object name cannot be read is a problem, since its renames by a bare name then stay classic.
A stream's name, a name of more than 1024 characters (counted) and every other rename go on as made.
Where the file system refuses the class or the flag - a FAT drive, a network share, a Windows older than 10 1607 - the call is made again as the caller made it and its result returned.

**The `UNLOCK_FILES` summary** is a measurement (section 6.6), 10 seconds after the first open or rename it counts, then every minute while the numbers change.
Every examined open is in exactly one of six counts and every replace-rename in exactly one of three, so the figures add up, and a new type a game build opens in its folder stands out in an ordinary log.
On 1.0.1158.16, with my mods folder and every part on, a boot's first 10 seconds widened 287 opens of 280 files of 12 types (154 `.cab`, 60 `.txt`, 31 `.dll`, ...) and left 184 opens of archives, none unexamined; a minute in, 835 opens of 282 files.
The save loaded, and a text file in the game folder was renamed while playing.
The game opens every file of its folder tree that way when its RPF cache check is skipped, as RageOpenV's part does: with RageOpenV off, next to NaturalVision Enhanced's loader, it asked DirectStorage for its 194 archives and the loader's loose files only in its first minute.

**The aids** (`DIAGNOSTIC_MODE`): what keeps one from being set up is a log line, never a problem, since nothing depends on the factory (ISSUES.md, T-DIAG).
When the game gets its factory for the interface PATTERNS.md's Status names, the factory's function [4], `OpenFile`, is hooked before the game gets the factory back, so that the hook sees the first `OpenFile` already; another interface, whose functions TweaksV does not know, and a factory not asked for 30 seconds after TweaksV started are log lines.
`HookOpenFile` passes every call on as it is and logs it, with a summary 10 seconds after the first call, naming a path outside the game folder by a category and its file name only (`(outside the game folder) SGTA50000`, `(temp) cloud_tweaksv.dat`, `(network) cloud.dat`), so that no other folder, such as a profile's, reaches the log.
The `NtCreateFile` hook logs every open inside the game folder, with its access, the sharing asked for and given (`share R-- widened to RWD`), its result and its callers, and the measurement (section 6.6) sums them up every minute.
Every hook does its own work after the game's call through one helper, `AfterCall`, so that nothing TweaksV opens there itself (`t_inside`) is examined or logged.

### 11.9 Blips (`InitBlips`)

The game's script `blip_controller` makes the vanilla blips behind a guard: a loop that calls the native that counts the active blips, compares the count with a threshold of its own, 150 on 1.0.1158.16, and while the count is at or above it waits 2000 ms and counts again (PATTERNS.md, "Blips").
A mod's blips count too, so with enough of them the game's own never come.
The part raises the threshold to `ACTIVE_BLIP_LIMIT` in the script's code each time the game places the script, at the start and again at every save load, a new program object each time.
The key stops at 1997, the count's lowest answer with the pool full, since a higher limit could let the script make a blip into a full pool.
`InitBlips` joins the script programs' hook with `Scripts::AddUser` (section 13.3), and a hook that cannot be made is the part's problem.
In a dry run the part checks the hook's sites, but the call is not redirected, so it never sees `blip_controller`.

`before`, on the game's resource placement thread (section 12), costs a program whose hash is not `blip_controller`'s that one read; one with the hash and another name is logged once and left.
The program itself is read within the module's bounds and its whole code decoded with the game's own opcode lengths, constants in the part checked when TweaksV is compiled and against the game's table after an update (PATTERNS.md, "When Game Updates", item 5).
A byte that is no instruction, or an instruction past the code or its page, refuses the program, so the bytes inside operands, names and SWITCH tables are never taken for a match.

The guard is matched by its shape at every instruction start, within a window of 256 bytes: the count's NATIVE (no argument, one result), the threshold's push (1..1997), a comparison and a branch, a body of a push of 2000 and WAIT's NATIVE (one argument, no result) that leads back to the count, and a way out that does not.
NOPs, jumps, a NOT and a fused branch may stand in it.
JZ jumps when what it takes is 0 and a fused jump when its comparison is false, so the branch may leave the guard while the count is below the threshold, as the game's does, or while it is not, a polarity the rewrite keeps.

The program stays as the game placed it when its code holds no guard; a second one, since which one is meant cannot be told; a CATCH, THROW or CALLINDIRECT anywhere, whose target is computed while the script runs and could lead into the guard; a jump, CALL or SWITCH case, read as the interpreter reads them, into the bytes from the end of the count's NATIVE through the branch; or too little room for the new push and comparison.
A threshold that is the limit already is left, and one other than the game's 150 is left and logged, not a problem, as the heap adjuster leaves a heap another adjuster set.
The game's 150 is rewritten within the bytes from the end of the count's NATIVE to the branch, NOPs and the new push and comparison right-aligned against the branch; a fused branch becomes a plain JZ with its target, and nothing after the branch moves (PATTERNS.md, "Blips", the room).

`after`, once the game has resolved the natives, reads the program and matches the window again, which must give the same guard, since another mod may be at work between the placement and the write, and then makes one `WriteCode`, a piece per page the bytes lie in.
With `DIAGNOSTIC_MODE` it logs both natives of every guard found, each with its index, its hash from before the resolution and its handler from after it, to compare after a game update.
Every refusal is a problem the first time a session for each kind of failure, after the start-up summary in a box of its own (section 6.4), and afterwards a counted line, since a save load places the same program again.

The measurement, `DIAGNOSTIC_MODE` only (section 6.6, thread `blip pool summary`), reads the blip pool's slots below its high-water mark (`L-E-P1`, `L-E-P2`) in one pass without a lock and logs the slots in use and the mark against the limit, with what the guard held after the last placement.

### 11.10 WeaponLimits (`InitWeaponInfos`, `InitWeaponComponents`)

Two halves under one tag, each a module of its own with its own key under `[WEAPON_LIMITS]` and its own guard: the weapon infos' (`WEAPON_INFO_FILES`, the `weapons.meta` files) and the weapon components' (`WEAPON_COMPONENTS_SIZE`, the weapon component definitions of all `weaponcomponents.meta` files together).
Its entry of the feature table starts the halves in turn, each inside its own `try`: an exception ends only that half, what it has written or hooked by then staying as it is, its problem names the half, and the other half still starts.

Each guard is a job of its own beside its half's raise: a MinHook hook on the game's loader of one file, which refuses a file once its half's room is used up, before the game reads it, so that a load past the room loses those files instead of damaging or ending the game.
A refused file never reaches the game's loader, and the hook keeps the game's last error.
A guard reads the game's data only after `IsReadable` or through `CopyMemorySafely`, within bounds of its own, at the cost of a bounded check per file the game loads.
The files refused are a finding in a box of its own (section 6.4); what the game does with the rest of a refused pack, as my boot of the guards saw it, stands in PATTERNS.md, "When Game Updates", item 5.

#### 11.10.1 Weapon infos (`InitWeaponInfos`)

The game parses each `weapons.meta` file into one weapon info blob of 248 bytes, in one block on its heap, and a C++ static initializer sets the array up once with two immediates, the number of blobs it allocates and the capacity, 160 on 1.0.1158.16 (PATTERNS.md, "Weapon infos").
The game's loader adds a blob per file without a check, so that a file past the capacity is parsed into the memory after the block.
`WEAPON_INFO_FILES` is the room: 248 bytes of the game's heap a file, the count's 16-bit type allowing 65,535.
`InitWeaponInfos` does two jobs from `DllMain`, each whole or not at all and each left undone alone, each a problem of its own, so that a raise refused keeps the guard and a guard that cannot be set up keeps the raise; a failure of the sites both build on ends both, with one problem.

The two immediates must be equal and not 0, since the game's exit destructor destroys as many blobs as the capacity says, and the raise writes both in one `WriteCode`, before the initializer runs.
The array's address must have exactly 2 references in the game's code, the block's store and the initializer's `lea`, so that a second store of a block would show.
The array must still be empty, no block and capacity 0, which proves on every boot that the initializer runs after TweaksV's `DllMain`, and so with the immediates the part writes.
Immediates that hold as much as the key or more already, another mod's raise or a later game build's own, are left as they are and logged, not reported.

The guard keeps the count below its room: what the raise gave the array, or what the immediates hold where the raise was not done or not needed (`GuardRoom`).
Its hook goes on the loader's start, `W-E-I2`'s match - 0x18, which must be a function start: where the game's unwind data covers the match, the function it names must begin there, so that a shorter prologue after a game update cannot put the hook into the padding between functions.
A start that begins with a jump already is logged and hooked on top, chained through.
The hook and the report's thread, `weapon infos guard report` on the measurements' loop (section 6.6), come together or not at all: a thread that cannot start switches the hook off again, the guard not set up, a problem of the part, and the hook refuses nothing until the thread runs.

The hook, on the game's loading thread, refuses a file, returning 0, once the count has reached the room, the capacity or the blobs the initializer allocated by the imm32 of `mov edx`, whichever is least, so that an array another mod gave less room, or fewer blobs than its capacity, after TweaksV still holds.
The first refusal keeps the file's name, read bounded, and logs it at once, so that a game that dies before the report keeps the line.
Ten seconds later the report's thread reports it once, and every minute while more come it writes a line.
With `DIAGNOSTIC_MODE` the hook writes a line per file before the game's loader runs, so that the file that passed a limit shows even when the game dies on it.

The measurement, with `DIAGNOSTIC_MODE` (section 6.6, thread `weapon infos summary`), reads the count and the capacity back without a lock and logs the files loaded against the game's own number and the capacity read back, the proof that the initializer ran with the number the part wrote.

#### 11.10.2 Weapon components (`InitWeaponComponents`)

The game keeps every weapon component definition of all `weaponcomponents.meta` files in one sorted list, a static array of 470 pointers with its count, an int32, right after it: the list's size is the distance between them, and no instruction carries it (PATTERNS.md, "Weapon components").
It rebuilds the list after every such file without a bound, and every definition also takes a slot of a pool, 470 unless the game's configuration names another size, whose trap ends the game when it is full.
`WEAPON_COMPONENTS_SIZE` is the room the part gives: the list moved on the core to one of 16854 entries - the largest room and 470 more - the pool given the room and 470 more, the most one file brings, and the guard refusing the files that come once the room is used up, so that the last file let in always fits and the pool's trap always comes before the list could be overrun.
The 470 definitions one file may hold are the game's own and stay.

On the core (section 10) the count stays where it is, the follower that gives the list's size, and a scan finds the 17 loads by the list's address.
The room and 470 more must lie within the moved list, so that the pool can never pass the list, whatever size the core takes from the game: the key's maximum fits, checked when TweaksV is compiled, and a game's own list past it is refused.

`ApplyRelocation` takes the pool's size call (`W-E-C4`) as a redirect to `PoolSize`, in the same all-or-nothing write as the 17 references: that call alone, never the game's lookup, which serves every pool of the game.
`PoolSize` calls the game's lookup first, with the game's arguments, so that the configuration is read as it would be, and returns the room and 470 more exactly, also when the configuration names more, since a larger pool would let definitions past the list.

The measurement, with `DIAGNOSTIC_MODE` (section 6.6, thread `weapon components summary`), reads the list's count and the files' count without a lock and logs how far the game's own list, the room and the pool are used.
At every reading the 17 loads are read back, since 6 of them lie in the game's second, protected `.text`, which might put its bytes back.
One that no longer leads to the moved list is a problem once, in a box of its own after the start-up summary, since a search through it no longer reads the moved list and the game is likely to crash; one that cannot be read any more is reported once too.

The guard (D12) hooks the head of the loader of one `weaponcomponents.meta`, which `W-E-C5` finds, so that a file is refused before the game makes any of its definitions (MinHook, its trampoline writable and executable: ISSUES.md, T-HARDEN).
The head must be a function start by the rule of section 11.10.1 and, unlike the weapon infos' loader, must not start with a jump, the sign of another mod's hook.
A guard not made is a problem of its own and leaves the move and the pool.

At each file the hook, on the thread that applies the data file changes, sums the definitions already loaded from the files' blobs, read bounded, the blobs' count at most their capacity and each blob's count 0..470.
A file is refused when the sum has reached the room (`GuardRoom`), or when a file of its name was refused before in the session, so that a base loader's run, which queues every file before any is parsed, refuses again exactly the files refused before.
The names are kept as hashes, 1,024 at most, past which a file is refused by the sum alone, as one whose entry has another type or no name.
A refused file's callback, passed by value, is destroyed as the loader's end would destroy it, and nothing else needs undoing.
Each refusal is a line, and the first the finding of section 11.10.

A read that does not add up lets the file through and switches the guard off for the session, a problem, since past 470 definitions above the room the game then crashes while it loads.
Definitions already loaded at a file - the sum above 0, so the pool exists - while the game has never asked the redirected size call mean that it made the pool without the redirect: before TweaksV started, or by asking for its size another way.
That is one problem, once, since the pool keeps the game's size and past it the game crashes while it loads, and the guard goes on with its room.
What the hook decides stands whatever its lines and problems throw, so that a name kept as refused is always a file refused, and a read that does not add up switches the guard off first.

### 11.11 PedProps (`InitPedProps`)

The part is experimental and ships off: its code is all there and the suites run it switched on, on the fake game, but the raise has never been tried in the game past 255 props on an anchor (ISSUES.md, T-PEDPROPS, holds the open question).

A ped model's props hang on 13 anchors and come in variation files, its own and one per clothing pack (PATTERNS.md, "PedProps").
The game counts an anchor's props over all of a model's files in 32 bits, but narrows that total to 8 bits in the set-prop native's check (`A-E-T1`) and the helmet code (`A-E-T2..T5`), and a worn prop's index before the metadata lookup (`A-E-G1`, twice, and `A-E-G3..G6`), so that from 256 props on one anchor in all the game refuses the props from (total mod 256) on.
The part makes each of the eleven `movzx` of 8 bits one of 16, the same length and the same registers, each narrowed register holding the whole value, so that an anchor holds 65535 props in all, and below 256 the widened reads give the game's own values.
`[PEDPROPS]` holds `ENABLED` alone, the one `ENABLED` that defaults to 0: the width is fixed, there is no size to choose.
Left as they are: the random props' builder (the research's G7), whose list keeps an index in a byte by its own format, and the 41 narrowings of one file's count, a file holding 255 props on an anchor at most by its own data.
No old plugin: the original is Legacy only, and nothing on Enhanced does the part's job.

`InitPedProps()` checks everything first, and the first failure refuses the whole job with nothing written (section 12), a narrowing another mod has widened being named as such through its widened twin, the pattern with that `B6` made `B7`.
`Scanner::FindCallsTo` (section 8) must find exactly 7 calls of the per-anchor count and 17 of the metadata lookup, so that a caller a game update adds, which would narrow again, refuses the job.

The measurement, with `DIAGNOSTIC_MODE` only (section 6.6, thread `ped props summary`), follows the job whether it was done, refused or dry, and runs with the part off too, from the table of the parts measured first (`MeasurePedPropsWhileOff`, section 4.3), so that a load shows whether it needs the part.
It is an aid that needs only its own sites, whose failures are logged, not reported (ISSUES.md, T-DIAG): `A-E-C1` and `A-E-C2` pin the layout it reads, and `A-E-S1` gives the model info store.

A pass walks the store without a lock, the game adding variation files while it loads its packs, and in each file takes only an anchor's first entry, as the game's per-anchor count takes it.
Every read of the heap goes through `CopyMemorySafely` (section 9), within bounds: the count 0..1,000,000, the stride 8..4096 and a multiple of 8, a collection's files and a file's anchors at most their words' 65,535, and at most 4,000,000 files and anchor entries read in one pass, so that data the pass misreads cannot keep the thread busy.

The summary gives the most props on one anchor, the two freemode models by name and every other by its hash, since the release game holds no model names.
Each anchor and each file past 255 has a line of its own, an anchor's saying from which index the game refuses its props this session: from (total mod 65536) past 65535 with the eleven widened, from (total mod 256) without them - a dry run, a job refused, the part off.

### 11.12 Resources (`InitTextureBudget`)

The game counts the video memory of the textures and buffers its streaming engine places in one object of its data, the counters (PATTERNS.md, "Resources"), beside the most objects allowed and four budgets, which its start-up writes once from `gameconfig.xml`, the texture quality picking the budget in use.
A resource is placed only while the objects stay below the most and the bytes below the budget; otherwise the streaming engine evicts what it cannot place, and textures go missing however much memory the card has.
The part, one module, `TweaksVTextureBudget`, raises both under the tag `Resources`, none lowered, and measures them with `DIAGNOSTIC_MODE`.

Windows' own budget for the game's video memory is not read: `VIDEO_MEMORY_BUDGET`'s maximum, 16,384 MB, is a documented cap in place of a query of it, since the game's update check evicts against Windows' budget anyway.
`STREAMED_OBJECTS` sizes nothing and has one reader, the placement's check, but it is all that stands between a load and the game's CPU-side descriptor heap, which has no check at its end (PATTERNS.md, "Resources", ISSUES.md, T-DESCRIPTORS), so its top is 3/4 of that heap as read at run time, 147,456 of 196,608 on this game.

The defaults rest on my boots, and together they ended my LOD switching by my eye:

| The keys | The bytes | The objects | By my eye |
|---|---|---|---|
| the game's 3,000 MB and 98,304 | 3,004 to 3,017 MB, at most 3,076, the update check's flag up | 50,112 | my LOD switching |
| 6000 and 98,304 | about 4,700 MB, at most 5,576, the flag down | 98,343, from 71,727 to 98,283 in one minute's drive | noticeably better, a little left |
| 6000 and 131072 | 5,415 MB | 104,437, descriptors 127,881 | the switching gone, no texture flicker |
| the same, my heaviest load | | 124,634, descriptors 149,438, so 47,170 free | |

6000 is chosen for the 8 GB of video memory or more that most people have by the Steam survey (D33); a load that shows fewer than 16,384 descriptors free reopens 131072.
The `texturebudget` suite runs the sites, the redirect, the raise and the walk on a fake game's numbers; what a raise does for a load, and how far a load stands from the heap's end, only a boot's numbers say.

What is special in the checks (section 12 has the rest), a site both raises share giving each key's own problem, one each:

- The scanner's search for stores of any width (section 8) must find the start-up's two stores the budgets' only writers, and its one store the most's: another could undo the raise later, so it refuses that raise alone, a load whose bytes read as a store counting too, a "possible writer".
- The most or a budget not 0 at TweaksV's start refuses both: the game's start-up has stored them before `DllMain`, as when a loader starts TweaksV late or a game build stores them earlier, and has made the call the raises redirect, so that a redirect would never run; in my starts the first summary, 10 seconds in, has found them still 0.

`RedirectCalls` (section 9) sends the call right after the stores through a call stub to the part's function, on the game's start-up thread before any streaming, which raises each value below its key and calls the game's function with its four registers, its result handed back as it came.
Nothing else is written: of the game's code the call's rel32 alone, of its data the four budgets and the most alone.
A `gameconfig.xml` most above the top is left as it is, with one problem from the raise itself, so usually after the start-up summary, since a load that reaches the heap's end crashes.

The measurement (section 6.6), with `DIAGNOSTIC_MODE=1`, both keys at 0 too, is an aid: logged, not reported (ISSUES.md, T-DIAG), a failed check of `Q-E-T5`, its own pattern, ending it alone.
A reading never passes a half-raised set as the game's own, the raise raising a flag around its writes: only this thread waits for them, for a second at most, and a flag still up after that gives no summary and one line.
`Q-E-T6` and `Q-E-T7` give the walk of the descriptors along the heap's free list at each reading, with no lock, so a race ends it early or long; a failed check ends the walk alone.

### 11.13 RayTracing (`InitRayTracing`)

The game skins every moving object its ray tracing keeps into one vertex buffer each frame, and two checks, every moving object's geometry's (`T-E-C1`) and the ropes' (`T-E-C2`), refuse an object whose vertices no longer fit: it then has no ray traced shadow, reflection or occlusion in that frame, and as the order of the objects changes they come and go, the shadows flickering with many high-poly vehicles (PATTERNS.md, "RayTracing").
`DYNAMIC_BVH_VERTICES` raises the buffer's count (`T-E-B1`) and both checks' capacity, three immediates of one value, in one write, up to 26,843,545: the cap keeps a raw view of the buffer within 2^27 32-bit elements, far below the 26 bits the ray tracing shaders read an object's first vertex in and the game's 32-bit sizes.
Its default is 0, the game's own: my heaviest load has taken 3,734,612 of the 4,194,304 vertices, none refused.
My boot at 8,388,608 took at most 2,281,234, so no load of mine has passed the game's number: a range past it and a refusal have run only on the `raytracing` suite's fake, which shows the sites, the write and the stubs' registers and counts, not what the game's shaders make of such a range.
A buffer object's global not 0 at the start, a build that makes the buffer before `DllMain`, refuses the raise, since checks raised alone would let the game's GPU animation write past the buffer.

The measurement, with `DIAGNOSTIC_MODE` only, also with the key at 0, is an aid, logged and not reported (section 12).
Two counting stubs near the game, read+execute, their counters on a read/write page of their own, take the checks' refusal jumps; they run on the game's worker threads, so they count with lock instructions, and keep every register but rax and the flags, which both refusals overwrite or ignore.
The summary's thread (section 6.6, `ray tracing buffer summary`) and the sampling thread (`ray tracing buffer sampling`) start before both jumps are pointed at their stubs, so that a measurement that cannot sum up redirects nothing.
A buffer the game has made already gets no stubs, since the game's threads may run the checks then and a jump's rel32 can cross a cache line, so that a check could read it half written.
The sampling thread reads three dwords every 50 ms, a frame taking 10 to 20 ms on a fast PC: the slot, the other slot's counter, which holds the last finished frame's total until its own frame starts, and the slot again, the counter taken when both reads agree.

### 11.14 SirenSettings (`InitSirenIds`)

A siren setting of `carcols.meta` and the vehicle variation that names it carry its ID in a byte on the whole load path, and a table of 256 bytes maps an ID to the setting's position, a byte too, in which 0 means no sirens: 256 IDs and 255 settings a vehicle can use, bound silently (PATTERNS.md, "Siren IDs").
`TweaksVSirenIds` widens the ID to 16 bits on that whole path; the position stays a byte, so 255 settings stay the most and more still wrap.
`LARGE_SIREN_IDS` has no size to choose, since the table must cover every 16-bit value, and 255 stays the game's "none".
The limit does not bind on my load, 33 of 255 settings and none colliding; it is there since silent collisions grow with bigger collections.
No old plugin: `SirenSetting_Limit_Adjuster.asi` is a Legacy plugin (D9).
The part's other job, `SIRENS_PER_VEHICLE`, its lights and its damage (`TweaksVSirenDamage`), is section 11.14.1.

The 14 patterns go through the core (section 10.2) as anchors, four limits that agree on another size than 256 being another mod's, and the other sites are checked as section 11.16.1 has them, for the part more strictly: an ID member's type other than a byte's is taken for another mod's widening, and a type byte outside the game's `.data` (`IsInsideGameData`, section 7) is refused.
An offset field's low dword must be the member's offset or 0, since the game may register its structures after TweaksV has started; no boot has shown what its upper dword holds, so it counts for nothing.

The one write, all or nothing (`ApplyRelocation`, with `RedirectCalls`), holds the table, 65,536 entries of a byte, read/write, near the game, the rest 0xFF, free, with its 4 `lea` and 4 limits; the nine loads and two stores of an ID made word ones; a pack's dedupe's compare loop, one byte longer as a word compare, so written over the NOP before it; the three constructors' defaults, the high byte of an ID no file gives; the two parser members' type bytes in the game's `.data`, TweaksV's one write outside the game's code and its own memory; and the base loader's call, `I-E-R1` +13, redirected, so that the moved table is free whenever the game frees its own.
What is checked and left as it is stands in the module's head comment; the trap among it is the dedupe's check for none, on the low byte, so that a pack's ID n*256+255 skips the dedupe, is appended and gets a free ID when its own is taken, the earlier setting keeping the ID, one array slot more.

With `DIAGNOSTIC_MODE`, the part at work, come three aids, logged and not reported (section 12), the part standing when one cannot be set up:

- The measurement over the moved table (section 11.16.1): the summary's numbers are words, it counts the IDs past 255 of the 65,536, and it adds the settings the game dropped and moved so far.
- The settings dropped and moved, per file: hooks on `I-E-H1` and `I-E-H2`, and `I-E-D1`'s load of a pack's ID through a stub, read+execute near the game, its record of 4,096 IDs on a read/write page of its own.
  A pack's file is read inside the callback, so at its start the settings are not in memory and a dropped one never reaches the game's array: the dedupe's load is the one place that sees all of a pack's IDs.
  The callback gets no file name, so `I-E-H3`'s call of the game's file opener is redirected and copies it with the parsed object, and a line takes it only for its own object, so it can never be a wrong file's.
- The vehicles whose siren setting ID finds no setting, named at load, nothing per frame, through `I-E-H4`'s hook, by the bound of D41's guard; the base game's own go through the base loop and are not seen.

`SirenSettingsNow()` (`TweaksVSirenIds.h` says when it works) gives the lighting capacity's measurement where the settings lie this moment; their array grows while packs load, so it is read through `CopyMemorySafely`.

The risks, each with what settles it:

- What the game's PSO loader makes of a member wider than its binary file's no fake can say, though the `sirenids` suite runs the sites as code: my boots compared the base game's settings and IDs with `LARGE_SIREN_IDS=0` and `=1`, the same 27 settings, the base game's IDs the same, the packs' IDs whole, and the default follows that (D22).
  Were it ever not the same, the two members would be widened only after the base loader has returned, in the stub, binary packs keeping their 8-bit IDs.
- A variation that names a setting which is not there gets the position 0xFF, and the game's getter, with no upper bound, returns a place past the array at two calls on each pass while the sirens are on: the game's own hazard for a missing ID.
  With `SIRENS_PER_VEHICLE` above 20 both calls are guarded (section 11.14.1) and such a vehicle gets the empty setting, no lights; at 20 the game's calls stand.
- Other mods that read the IDs or the table as bytes see 16-bit values; no plugin on Enhanced is known to.

#### 11.14.1 The sirens per vehicle's lights (`InitSirensPerVehicle`, `FindSirenLights`)

`SIRENS_PER_VEHICLE` above 20 is a hybrid (D23): every structure of the game stays at 20, the lights past 20 come by a second call of the game's own renderer on stand-ins of the part's own, and the damage by the game's own code raised, glass included.
A second call alone would leave lights 21 and on out of every hit, impact and wreck, a cost I did not accept.
32 is the top, by the 32-bit masks of the siren state, of the vehicle's broken sirens and of a damage object's hidden sirens (PATTERNS.md, "Siren count"), and the default (D38): a vehicle with no lights past 20 pays one lookup.
The `sirencount` and `sirendamage` suites run the sites, the writes and the loops as code on a fake's renderer and structures; that the game's own take them only a boot shows, and my proof did, with 32-siren packs (PATTERNS.md, "Siren damage").

The checks before the write, each failure the part's one problem with the vehicles at 20, are the game's build, 1.0.1158.16 alone (D40), since no pattern proves that the renderer reads nothing new of its stand-ins and on other people's PCs nobody reads it again; `LightStoresRaised()` (D39), since the game's own stores lose lights, and overflow, at more sirens than its 20 (section 11.17); `FindSirenLights`; and `FindSirenDamage`.
The one write: the hook on `I-E-Z3` first, since it can be taken back, and taken back it is when the rest is refused or throws, the problem saying if it stays on; then one `RedirectCalls` inside a `try` with the renderer's call, the getter's two and the state's size, with the damage's writes in it.
An exception from a line after the writes finds them in, by the renderer's call, and they stand.

**The settings.** The hook runs after the game's array reader, which drops item 21 and on: one compare of the member's data per array read, and only the lights go further.
Up to `SIRENS_PER_VEHICLE` - 20 more items are read by the element's own slot 1 into a siren setting made by the game's constructor in zeroed memory, each light's start corrected once, forward start + 20 x delta, reversed start - 20 x delta, so that the renderer's delta x i gives its true index.
The setting's key is a 64-bit hash of the 20 lights the game kept, their members alone, a light's padding left out, so that it is the same at the parser and in the game's array.
A table of 255 keeps them: a twin is dropped, and one with the same 20 lights and others past them replaces the first, the old object staying, since a renderer may be reading it; the renderer's path takes no lock.
A table that is full, a read that faults or memory that cannot be had is reported once, and that setting keeps 20 lights.
A light's group past the renderer's 4 becomes 0, as an index it does not check.

**The second call.** `I-E-Z2`'s allocation holds the state, a second state and a stand-in vehicle, freed whole by the game.
The wrapper calls the game's renderer, then looks the model up in the table of 1,024 (section 11.16.2), and a model that does not fit keeps 20 sirens, reported once.
On a model's first pass a block is made when the room behind its bone structure (`TweaksVSirenRoom.h`) names bones past `siren20` and its setting's 20 lights hash to an own setting, its scalars the last model's where two with the same 20 lights differ, a limit logged; the block stands in for the model info, so that the renderer's light 1 is `siren21`.
The stand-in holds what the renderer reads of the vehicle (PATTERNS.md, "Siren count"), its broken sirens shifted by 20.
The second state is made anew when the first has another start time, or when the 8 free bytes at +0x188 do not name this vehicle, the heap giving a block back as an earlier car left it.

Both calls of the getter lead to one plain function (D41), the game's getter left out: a block, which is no model info, gets its own setting, by one range check; position 0 gets none; a position below the array's count that entry, the count read before the pointer; any other, or no carcols data, the empty setting of TweaksV's, zero but for `sequencerBpm` 200, which lights nothing.
Lights past 20 are per pass, so up to 8 merged lights a vehicle (`useRealLights` off), not 4; a pack's binary file with lights past 20 stays dark if it does not reach the tree reader (HINT: it does).

**The damage** (`FindSirenDamage`, `TweaksVSirenDamage`) is the half of the part's one write behind the lights': 64 bytes of room behind each bone structure, filled at a model's load, and the game's damage loops raised to read them, so that a hit, an impact and a wreck reach lights 21 to 32.
It writes nothing itself, so `DRY_RUN` and `DIAGNOSTIC_MODE` are the patch module's.
Its 19 patterns are each looked for, `I-E-L08` and `I-E-N09` three times each; every operand it writes is the game's own value, pinned by its pattern, so a site another mod changed is a pattern not found.
The bone structures' pool must not be made yet, since TweaksV would have started too late to give its structures their room; no pattern pins its load, so it is read by its shape right before `I-E-Y2`.
It adds 21 writes and one redirected call: the bounds to N, the key, and `I-E-L08`'s -20 to -N, the reads to the room, the hit's counter (PATTERNS.md, "Siren damage"), the pool's stride and the allocation to 0x38C, whatever N is.
The sync loop stays at 20: it reads 20 bytes of sync data, and nothing writes its sites.
The wrapper, `MadeBones`, runs at a model's load, never per frame, and fills the room under an access-violation guard: to 0xFF, the game's 40 bytes copied in as they stand, a plane's gear doors in their slots, then the game's fill once more on a table of TweaksV's own of 24 entries, `siren21` to `siren32` and their glass.
A byte past `siren20` below 0, a bone past index 127, becomes -1, so that no such byte reaches a raised loop.
A structure that cannot be read or written, or a fault in the fill, is said once, the first model named, its bytes past `siren20` all -1.

### 11.15 StaticBounds (`InitStaticBounds`)

The game keeps the static collision bounds of the map in one store, named `StaticBounds`, and a full store ends the game at a trap.
Its size is read once, from one call (PATTERNS.md, "StaticBounds"): the store's init asks the game's pool size lookup, and the pool, the name map, the module's count, the streaming range and the box streamer's arrays all follow what comes back.
So a write to the pool's capacity alone cannot carry a load past the game's size, and nothing has to be written beside the call.
The part gives the store at least `STATICBOUNDS_SIZE`, whose maximum, 32767, is the largest size whose every slot is sound.
The key's name, its default and the rule, the larger of the two, are Nochala's Static Bounds Limit Adjuster v1.0.2's (CREDITS.md; its notice in THIRD_PARTY_NOTICES.md), the route TweaksV's own, in the way of WeaponComponents' `W-E-C4`.

**A proof of this raise needs a load past the game's own number:** a load under it cannot fail on what the raise is for, however many starts pass.
The original's way, its two hooks raising the pool alone, passed three starts at my load's 10,406 slots, under the game's 24,500, and crashed at the first load past the game's size, a pool of 8000 in my `gameconfig.xml` with the key at 16000: the 8001st name, at the save's load, found the name map's free list empty, `int3` at `+0x93AA93`.
The same start passes by this call, and so does 32767 on a pool of 24,500, each with the same 10,406 slots in use, past which no boot has gone.
The `staticbounds` suite's fake holds a store init that runs as code and traps as the game does, so it carries names past the game's size and traps one past the raise; what it cannot show is a slot the game narrows beyond what the research traced, so 32767 rests on the widths found (PATTERNS.md, "StaticBounds").

A mod's jump at the lookup's start is wrapped, the larger winning; `C-E-D1` is only the read-back's, anything wrong with it a line, the raise going on.
The raise spends the pool memory budget, a slot's 17 bytes, which only HeapAdjuster raises, and past it the game writes through a null pointer with nothing on screen (PATTERNS.md, "A pool's memory"), so a size above the key's default while that budget is not raised is one problem of the start-up summary, the raise made all the same; at the default nothing is said, since a box at every start would say little.

**The check,** a thread of its own, `static bounds check`: the raise holds only if the game asks after TweaksV's `DllMain`, about 15.5 s in on 1.0.1158.16, and nothing else would say when it does not.
The clock starts with the session, the game placing `blip_controller`, which the script programs' hook sees (section 13.3), the part making that hook itself while `TweaksV.ini` switches Blips off; the first program of all comes about 24 seconds earlier in the one boot that logged both.
While a plugin that does Blips' job is installed, `BlipLimitFix.asi` or `ResourceExtensions.asi`, whatever `[BLIPS] ENABLED` says (`IsPluginInstalledFor`, section 7), the part makes no hook and no thread, logged, and leaves the script programs to it.
A check that cannot be made otherwise, for want of its two events, the hook or its thread, is reported, the redirect staying.
If the game has not asked within 60 seconds of the session's start the thread reports it, in a box of its own after the start-up summary (section 6.4): it asked before TweaksV started, or makes the store another way, and the store keeps the game's size.
A session that never starts leaves it waiting.

**The read-back,** on every boot with a check, by that thread, a second after the game has asked, since the init runs on: count, pool and streamer differing is a problem, since the load would end at the smallest.
More than 32767 in use is one, which only a `gameconfig.xml` above that can bring, since an interior whose slot lies past them can lose its collision; a `gameconfig.xml` above 32767 is otherwise kept, as any size above the key.

**The redirect** (`StoreSize`) runs on the game's thread for every store of that init, 10 of 0x7740's and three others, and a hash other than 0x2CDFE406 goes back as it came.
For the store's, a size from 0 up to below the key becomes the key, and one below 0 is none, left and reported once.

### 11.16 The parts measured first (`kMeasuredFirst`)

A limit of the game is lifted only once a mod load reaches it, and only from a boot's numbers (D25), so each limit is measured first, by a module of its part: `TweaksV<Limit>.cpp` and `.h`, with its header comment, one entry function, `Measure<Limit>()`, and its part's log tag, or one of its own where no part stands behind it yet.
`kMeasuredFirst` starts them with `DIAGNOSTIC_MODE=1` (section 4.3), in the order of the subsections below, the ped props' measurement last (section 11.11).
A module brings no section in `TweaksV.ini`, no entry of `kFeatures` and no old plugin: those come with a part's writing half, which the siren IDs (section 11.14), the sirens per vehicle (section 11.14.1) and the lighting capacity (section 11.17) have.

A module finds every pattern of its limit, its measurement's and those the writing half will use, so that the log names each one missing and its line the first, checks and logs each site as the writing half will, and starts its measurement through `StartMeasurement` (section 6.6).
It writes nothing to the game, but for two redirects: the siren count's one call (section 11.16.2) and the lighting capacity's two jumps and two calls (D30, D36, section 11.16.3).
Any failure is logged and not reported, a diagnostic aid (section 12; ISSUES.md, T-DIAG), and that module measures nothing more.
Without `DIAGNOSTIC_MODE` nothing is looked for, so a normal boot does what it did without them; `DRY_RUN` changes nothing in them but those two, which then redirect nothing.
Each subsection ends with what my loads showed (more in PLAN.md, "What my boots have shown so far").

#### 11.16.1 Siren IDs (`MeasureSirenIdsWhileOff`)

It measures the settings and IDs a load uses against the game's 256 IDs and 255 settings, bound silently (PATTERNS.md, "Siren IDs"), and writes nothing.
`MeasureSirenIdsWhileOff` does nothing while `InitSirenIds` is at its job: it measures with `LARGE_SIREN_IDS=0` or the part off, over the game's own table and IDs of a byte, as the ped props' measurement does (section 11.11), so that a start with the job off and one with it can be compared.
With `DRY_RUN` and `DIAGNOSTIC_MODE` the part measures in the same way (section 11.14).

It looks for its 14 patterns, `I-E-P1` to `I-E-M1`, and checks every site the part changes or relies on, by PATTERNS.md's rows.
The ID members' offset field and type are logged again at the first summary, since the game may register its structures after TweaksV has started; a type that does not store a byte is a finding that stops nothing here, and the part refuses it (section 11.14).
`FindRipRelativeLeas` must find exactly the 4 `lea`, all to one table of 256 bytes, or another mod has moved some of them.

The measurement (thread `siren settings summary`) reads each setting's ID without a lock, a byte or, once the part has written, a word, then the array and the count again, since the array grows while packs load; a count above the capacity is data the game was changing.
The summary gives the settings loaded, the IDs in use, the settings past position 255, which no vehicle can reach, and the IDs two or more settings carry (with IDs of a byte, 255 when the table is full); with the part at work its line is that of section 11.14.
After each summary the IDs themselves, in their settings' order, the first 1,024 settings at most, 128 a line: what a start with `LARGE_SIREN_IDS=0` and one with `=1` compare, the base game's coming first.

My load: 21 settings without packs, 33 with them, their IDs 33 of 256, none given twice, an ID past 255 keeping its low byte; the comparison held (PATTERNS.md, "Siren IDs").

#### 11.16.2 Siren count (`MeasureSirenCount`)

The game lights `siren1` to `siren20` of a vehicle: its renderer runs over 20 lights, and a model's bone structure names those bones and their glass alone, by signed bytes, so a model made for more sirens keeps the rest dark, without an error, and a bone past index 127 cannot be named at all (PATTERNS.md, "Siren count").
The module measures which models that binds; it looks for `I-E-B1` to `I-E-B4`, `I-E-B4` a function start by the game's unwind data, since the wrapper calls it, and not by the byte before it, which `IsFunctionStart` (section 9) takes for code without unwind data.
It starts its thread (`siren count summary`) and only then redirects `I-E-B1`'s call of the renderer (`RedirectCalls`, through a read+execute stub), so that a measurement that cannot sum up redirects nothing; after a failed redirect, under Windows' dynamic code policy for one, the thread waits for a first model that never comes.

The wrapper, `CountedSirenLights`, takes the renderer's nine arguments, the three floats as floats and the stack's five as 64-bit values, so that each slot reaches the renderer as the game's call left it; it calls the renderer first, then counts on the game's thread, with no lock, no allocation and nothing that can throw.
At each pass it looks the model info up in a fixed table of 1,024 slots, over 32 slots at most, a free one claimed by compare-exchange, so that a new model drawn on two threads at once is counted once.
On a model's first pass, the caller's last error kept around it, it reads the bone structure's bytes and makes 64 calls of the game's find-bone-by-name, `siren1` to `siren32` and `siren_glass1` to `siren_glass32`, each guarded as `CopyMemorySafely` guards a read, since no pattern pins what the renderer's 7th argument holds: an access violation is caught, and the model is one that could not be read.

The summary, 10 seconds after the first model and then every minute while the models change, gives the models seen, the most sirens a bone structure names, those with bones past `siren20`, those losing bones past 127 and those not read.
Then a line, once, for each model the limit binds, that could not be read, or whose bones by name and by bone structure disagree; the table's slots bound those lines, below `kMaxLinesPerKind`, so they need no cap.
With the raise (section 11.14.1) the wrapper is the raise's, `SirenLightsTwice`, the call is redirected once and the measurement the same; the summary then also says which models the second call lights and what the sirens' tables hold and dropped, and a model's line what is lit past `siren20` or why it is not.
A refused raise leaves the measurement as it was.

My packs without the raise: three models past `siren20`, `lspdcvpi08` with 10 sirens and 6 glass bones never lit, the CHP FPIU with 12, half its lightbar dark, the LVMPD Crown Victoria with 10 and 6; with it, three models lit past 20 by my eye and this log.

#### 11.16.3 Lighting capacity (`MeasureLightCapacity`)

It measures how far a load fills the layers of fixed room a frame's lights pass through, each in PATTERNS.md's "LightCapacity" table with what comes past it; many vehicles with real siren lights are where a load may reach them.
Its tag is `LightCapacity`, and it looks for all 22 patterns, unless the part has done so before it and hands over what it found (section 11.17).
The checks, the part's too, the first failure ending it with nothing written, by PATTERNS.md's rows:

- Each store has the three references of the reset, the drain and the constructor (`Scanner::FindRipRelativeReferences`): a fourth ends it, and fewer refuse nothing, a fact for the boot, since the scan sees a `lea` or a store of a pointer and the findings give the constructor's two by their addresses, not their instructions.
- Every other site names what the sampling reads, each value inside the game, aligned to its size and readable when the measurement starts, and read plainly from then on.

Beside those it looks for `G-E-K1` and `G-E-K2`, which nothing else does, and checks them and `G-E-F1`'s jump, the two copies' calls to one function; one not as the findings have it costs its own number and a line, no more.
A write pointer not 0 at the start means the game's frames have begun, as RayTracing's buffer made already does (section 11.13): its threads may run the jumps and a rel32 being written can be read half written, so no stub is made and the sampling goes on.
The same with `DRY_RUN`, and with stubs that cannot be made: no memory near the game, out of reach, or a jump Windows refuses.
The stubs lie on the first page of a block of two near the game (`AllocateNearGame`), read+execute once written, their counters on the second, read/write.
A jump's counts by a `lock inc`, two instructions for each light a full list refuses and none while it has room; a copy's keeps a job's most by a `lock cmpxchg`, a load and a compare at a job's end, at most 120 times a frame; PATTERNS.md gives the flags and registers each may touch.
Its threads start first, the summary's and then the sampling's, and one `WriteCode` then redirects the two jumps and the two calls, in `DllMain`, where no other thread runs them; a summary thread that cannot start redirects nothing, and a sampling thread that cannot start is a line, the rest going on.

`lighting capacity sampling`, every 10 ms, which sees every frame up to 100 fps, reads 15 of the game's values and keeps each number's most since the last reading.
A pointer outside its store or a number past its room is a line at once, once each, no box (D30), and enters no most, since what lies there then is no count of the game's.
The summary, 10 seconds in and then every minute with something new, gives each layer's most against its room, every summary but the first the minute's alone before the session's, which is what a scene reached, then a job's most, the lights refused and what the siren settings ask for.
It counts the samples with the scene store at its 1024 and the sort list's mark set, true on either reading of that mark, which is not settled (PATTERNS.md), so that my boot reads what it shows.
With the part's one write done the numbers go over the moved stores and lists; with the part off, refused or in a `DRY_RUN`, over the game's own.

My load stays under the frame's stores, at most 327 of 400 vehicle lights, 439 of 928 coronas and 642 of 1024 scene lights, but the ray traced list fills, 1024 in 2 of 32 minutes, and one start's refused 172,943 lights, 165,438 of them in one minute in a tunnel (D36).
16 real cars lit gave at most 124 vehicle lights, about 7 a car, under a spawn cap of 17 (D44); raised, 10 cars took a job to 97 of 180 lights, past the game's 80.
Past the game's numbers, with the test switch `TEST_SIREN_PASSES` at 16 and about 10 cars (section 13.1): vehicle lights 1,023 of 10,800, vehicle coronas 2,027 of 18,000, a job's lists at their 180 and 300, 2,252,028 lights refused, the scene store at its 1,024 with the sort list marked in 3,575 of 30,090 samples, no crash, the lights flickering hard and the frame rate about half, the overload's own.

#### 11.16.4 The model budgets (`MeasurePopulation`)

The game keeps a budget in bytes for the vehicle models and one for the ped models it holds in memory, 429 MB and 561 MB at the Population Variety slider's top; short of it by a margin it asks for no further model and releases what nothing uses, so a load with many models repeats those it holds, without an error (PATTERNS.md, "Population").
One module measures both, since they share an object, a set-up and the crash context's line that leads to them, under its own tag, `Population`, and it writes nothing.
It looks for its 16 patterns, `O-E-C1`, `O-E-V1` to `V7` and `O-E-P1` to `P8`, and checks them by their rows in that order, `O-E-C1`'s getters at each call's target against the module's mask, since the two are the same bytes and no pattern can tell them.
Among them are the writing half's W1 to W3; W3, the scanner's search for stores of any width into each story table (section 8), is a line: a writer besides the set-up's stops nothing, since it could undo a raise, not a measurement, and the search skips what it cannot read, so that one which misses a store the patterns pin proves nothing, and says so.

`population sampling` reads both budgets once a second, and each summary (`population summary`) takes a sample of its own, so that it comes without that thread too.
A sample counts for nothing until the game's init has run and its lists are laid out, since `.data`'s other numbers and a list of zeros would stand in highs nothing lowers, and the fill counts from a second sample, since the research gives the order of none of the init's stores.
The budget and the stop come as the game has them (PATTERNS.md), without the getter's doubling, the cast's overflow and the add's wrap included.
Each budget's line ends with `; no new high for N s`, which is what the proof's drive reads: it goes on while N stays small, and N's passing two minutes brings one more line, so that the last line says it.
The tables' line, at the first summary after the fill and whenever they change, is the only proof that a raise in `gameconfig.xml` has taken: it gives both tables and the configuration's `Level1Kb` to `Level3Kb` against the defaults of 1.0.1158.16, 32000, 56000 and 70000, the module's own, which no pattern pins.

My load at the slider's top: the vehicles at the stop about 95% of the time, about 100 models at once, about 4.1 MB each; the peds at most 505.8 of 561 MB, about 9.8 MB each, and never at the stop, so no raise of theirs (D43).
`Level3Kb` at 98,500 in my `gameconfig.xml` made the vehicles' 600 MB, at the stop in 607 of 741 samples, 141 models at once, `VehicleStruct` 141 of 200 (D-V5).

#### 11.16.5 Ped pool (`MeasurePedPool`)

The game keeps its peds in one pool of 256 slots of 6,528 bytes, sized by `Peds` in `gameconfig.xml`, and a full pool only means no more peds; raising the number alone is a trap, since three of the game's arrays stay at 256 with no check (PATTERNS.md, "Ped pool").
The module, tag `PedPool`, writes nothing; it looks for its 18 patterns, `N-E-A1` to `N-E-D6`, and for `H-E2` through its part (section 11.1), and checks them in that order by their rows, a raise's sites among them, the budget's only saying what it holds.
No complete set is scanned: the findings list none, and no read of it depends on one.

`ped pool sampling`, beside `ped pool summary`, keeps the most slots in use and the most in the spatial array, once a second; a sampling thread that cannot start is a line, the most then being the summaries' own.
The summary, for which alone the slots are walked, gives the slots in use, the mission and reuse peds, the scenario and ambient peds against their caps and the spatial array, and says where the pool has been full or the array past the 256 of the game's fixed arrays: lines of the log, no box (D31).
The ambient caps stand in the counts' order by the research's hint of which store is which, so that the line says it is one.
The first time the sampling thread sees the spatial array past 256 it says so at once, as what follows may end the game before the next summary; a reading a second can still miss it.
A pool whose stride is not 0x1980 is not read, since another mod may have made it or the game has changed, and a game's number that is not the pool's slots is a line, since a pool larger than the number overruns two of the game's arrays.

My load: at most 97 of 256 slots, the spatial array the same, so no raise (D43).

#### 11.16.6 Render queues (`MeasureRenderQueues`)

What a streamed ped, its props or a vehicle with mods draws in a frame the game keeps in three families of four arrays, one per frame, the render data behind them in six pools (PATTERNS.md, "Render queues"): past the peds' and the props' 120 entries the game writes past a heap block with nothing said, a full pool stops the game or takes a vehicle's mods, and past the draw list's 10,368 references a streaming reference is never released.
The module, its tag its own, `RenderQueues`, writes nothing; it looks for its 24 patterns, `U-E-S1` to `U-E-D3`, and checks them by their rows.
As for the ped pool, no complete set is scanned.

`render queues sampling` makes about 20 reads every 20 ms on its own thread, inside a `try`, through `CopyMemorySafely` alone, so that no page can fault the game, and nothing on the game's threads.
A read that fails, and any other stride than a pool's code hands it, is believed in nothing, since the game's allocator need not zero and a teardown may read freed memory before the flag clears.
50 of them in a row, about a second, end that reading, said once, since each read that fails is an access violation through every handler in the process.
The first time a family's array is seen past its capacity it is said at once, a line of the log and no box (D30), since what it does may be a crash long before the next summary.
At a family's new most it copies that array's entries, at most 4,096, and counts the different pointers: that gives the entries per ped per frame, the one number the dump cannot give.
The summary (`render queues summary`), 10 seconds in and then every minute while a most, a capacity or a state changes, the latest numbers alone never, gives each family's render data, requests and array against their room, and the references.

My load: streamed peds' render data 15 of 120 and a frame's array 28 of 120, props of peds 5 of 120, streamed vehicles 4 of 100, references 4,571 of 10,368, so no raise (D43).

### 11.17 LightCapacity (`InitLightCapacity`)

The frame's two stores run over their own write pointers with no check, a job's lists drop what does not fit, and past its 1024 the scene store replaces a wrong slot (PATTERNS.md, "LightCapacity"); 32 sirens a vehicle bring the overflow three eighths nearer (D23, D30, D39).
The part is `[LIGHTCAPACITY]`, on by default and off beside `ResourceExtensions.asi`, with no key for these sizes, since they stand at the top of what the worker's scratch allows; its tag is `LightCapacity`.
It looks for the 22 patterns and checks every site as section 11.16.3 has it, with or without `DIAGNOSTIC_MODE`, `G-E-C4`'s 16 bytes 16-aligned for their `movaps`; the measurement reads the addresses from it, looks for nothing again and goes on over whatever the part did.
A failure is a problem, with nothing changed, and the sirens per vehicle then stay at 20; so is a write pointer not 0, since the game's frames have begun and a thread may be reading a rel32.
Then `FindCoronaStore` with `G-E-G1`'s two matches, whose false is the same problem and whose writes join the one write (section 11.17.1).

Its memory is one block near the game (`AllocateNearGame`): the start values 0, 0, 300, 180 on a page of their own, read-only once written, as the game's in its `.rdata`, then the 18,000 coronas of 0x30 bytes and the 10,800 lights of 0x1C0, 5.7 MB, each store on pages of its own, read/write and zero, since the records are copied whole and read by value.
All that the 60 jobs can hand them fits, so no overflow can happen by construction, and the check the game lacks is not needed.
The one write, 16 places in one `WriteCode` with the corona store's, in `DllMain`, leads the stores' four references and `G-E-C4` to that memory, gives a job 180 lights and 300 coronas in a scratch of 0x17340 bytes, inside the worker's 0x18008, and widens the scene store's index, so that past 1024 the farthest light goes (PATTERNS.md, "LightCapacity", has each row's offsets).
No memory, memory out of reach of a site, start values that cannot be made read-only or a write that fails is the same problem.
`DRY_RUN` makes no memory and writes nothing.
`LightStoresRaised()` is true once the one write returned true, a `DRY_RUN`'s would-write included, so that the sirens' dry run goes on; `InitSirensPerVehicle` asks it, after this part's start.

#### 11.17.1 The corona store (`FindCoronaStore`)

The game's store holds 1088 coronas a frame and drops the rest without a word, the vehicles' first, since they come last in a frame (PATTERNS.md, "LightCapacity", its corona store): 1088 of 1088 in 1 of the 32 minutes of my load, and 32 sirens send it more (D35).
`CORONA_STORE` under `[LIGHTCAPACITY]` is the room, 4096 by default, 1088 to 8192; a `static_assert` ties the maximum to what a draw's stack of 0x24 bytes a corona, with 64 KB to spare of the render threads' 0x80000, and its word of an index take.
At 1088 `FindCoronaStore` looks for nothing and writes nothing.
Raised, my load reached 1,588 of 4,096 and the test switch 2,507, none refused.

Above it, called by the part after its own checks and with `G-E-G1`'s two matches, it looks for its 14 patterns, `G-E-I1` to `I9` and `J1` to `J5`, and checks them by their rows, the first failure its reason, which the part reports as its own problem, with nothing written.
Two checks count a complete set, since nothing gates the raise by the game's build: no more than the research's 19 references (`Scanner::FindRipRelativeReferences`), as a 20th may be a new batch with a clamp of its own, fewer refusing nothing, a fact for the boot; and no second caller of `G-E-I7` (`Scanner::FindCallsTo`), which would get its count from the old store.
The new store is N x 0x30 + 0x180 bytes through `AllocateNearGame`, since the copy always reads 8 records past the count, 16-aligned, as `VirtualAlloc` gives memory, since the adds write with `movaps`.
It adds the 13 writes (PATTERNS.md, "LightCapacity") to the part's one write and writes nothing itself; with `DRY_RUN` the store is made for the rel32 of the lines and freed again.

The counters, with `DIAGNOSTIC_MODE` and not with `DRY_RUN`, are an aid (section 12), whose failure is logged while the raise goes on.
The four jumps must lead into the game, to the bytes the research found up to the first instruction that writes the flags, and their stubs lie and count as the lighting capacity's do (section 11.16.3), all three adds' into one counter and the job's own limit's into another.
The batch's, reached by `E9 rel32` and `90` in place of the six bytes at `G-E-I8` +56, adds what the batch could not place, keeps what it brought and runs the six bytes (PATTERNS.md); their rel32 and the six bytes join the 13 in the one write.
`CoronaStoreNow()` gives the part's line and the measurement's, on any thread: the game's own 1088 and nothing counted, on a dry run too, until `LightStoresRaised()` says the write is done; then N and, with the stubs made, the refused coronas, the LOD lights' job's own refused and its last batch.

## 12. The rules every part follows

- **Nothing is written before everything is found and checked.**
  Every site is found by pattern, every instruction's bytes are the expected ones, every address inside the game, every value plausible and consistent; the first thing not as expected ends the part's job with nothing changed.
- **All or nothing.**
  A job's writes go in one `WriteCode` or `RedirectCalls`, and hooks of one group are switched on together or removed again (section 9): a part does its whole job or leaves the game as it was.
  The writes in `DllMain` come while the game is still single-threaded; RageOpenV's start-up writes its few bytes later, on the game's thread, without freezing the others, an accepted window (ISSUES.md, T-FREEZE).
  The Blips part writes into a script program before the game enters it in its registry of programs, where no script thread can run it (section 13.3).
  The one write outside the game's code and TweaksV's own memory is SirenSettings' two parser members' type bytes in the game's `.data` (section 11.14).
- **What another mod has already changed, TweaksV does not change again.**
  A heap another adjuster set, a cache check already skipped, a function already hooked (hooked on top, chained through) are left and logged; an array already moved, a call already redirected, a pattern that matches more often than it must are refused with the reason, because what TweaksV would write over cannot be known.
  The conflict check switches a part off entirely when its old plugin is installed, or a plugin that does its job too and stays (section 7).
- **Never silent.**
  Every job not done is a `PROBLEM` line in the log with the reason and an item of the start-up summary, a message box with `ERROR_POPUP=1`; a part that stops working after the start-up gets a box of its own, and so does what a part finds while it does its job (section 6.4).
  A switch that is off is logged in one line.
  A job whose effect comes later is checked for it: RageOpenV's start-up (section 11.6), HeapAdjuster's pool memory budget (section 11.1), the static bounds' stores (section 11.15) and the weapon components' guard (section 11.10.2).
- **An aid that cannot be set up is logged, not reported** (ISSUES.md, T-DIAG): it is switched on for a test, and the game and every part's job go on without it.
  The aids: the console, DirectStorage's call logging (not its job, the `NtCreateFile` and `NtSetInformationFile` hooks), a measurement whose thread cannot start (section 6.6), the counting stubs of RayTracing, the lighting capacity and the corona store, and SirenSettings' counting of the settings dropped and moved.
  The parts measured first are aids as a whole (section 11.16): a failure of one is logged, not reported.
- **`DRY_RUN` and `DIAGNOSTIC_MODE` work the same for every part,** with three exceptions.
  With `DIAGNOSTIC_MODE=1` the box streamer refuses its whole job when a tree-search call cannot be redirected, because the measurement is the reason the switch is on (section 11.4).
  The ped props' and the siren IDs' measurements run while their job is off too (sections 11.11 and 11.16.1).
  With `DRY_RUN=1` the Blips part checks its hook's sites but never sees `blip_controller`, which reaches it only through the hook a dry run does not make (section 11.9).
- **Everything read is bounded and checked.**
  INI values are limited to 255 characters, parsed and clamped; the mods folder's walk and the check of its packs have limits on every count, size and depth (section 11.6); paths from the game and from DirectStorage, and its rename buffers, are read within a length, DirectStorage's under a guard against unreadable memory (section 11.8); the scanner reads the PE headers only where they lie inside the image (section 8); the log cuts its lines and its file.
  The measurements and the guards of WeaponLimits read the game's data only after `IsReadable` or through `CopyMemorySafely`, within bounds of their own.
- **Nothing throws into the game.**
  Every hook and every thread puts what can throw - a log line, a table walk - inside a `try`, and `DllMain` catches everything on attach and returns `TRUE`.
  A lock is never left taken by an exception (`TweaksVLock.h`), nor a page of the game's code writable (the guard in `WriteCode`).
  A hook that works after the game's function, or in its place, keeps the game's last error around that work; the others call the game's function last, so the game's own error stands.
- **Nothing per frame beyond the game's own work.**
  A hook on a hot path (the file functions, the tree searches, `NtCreateFile`, which every file open of the game passes) does one path spelling and one hash lookup, one count with atomics, or one bounded comparison of the open's name with the game folder: no allocation, no lock of TweaksV's, no file system call but one query of a folder's identity per open relative to a folder handle (section 11.8).
  The measurements read on threads of their own (section 6.6); with `DIAGNOSTIC_MODE=1` a counting stub adds a few instructions when a full store refuses, none while it has room (sections 11.13 and 11.16.3); what else a part adds to a game's call is in its section.
- **Memory TweaksV adds is never writable and executable at once.**
  The moved arrays are read/write data, the light stores' start values read-only (section 11.17).
  Every stub is written once and then read+execute, what it counts or records on a page of its own, read/write.
  MinHook's trampolines are the one exception, accepted (ISSUES.md, T-HARDEN).
- **Per-event output is capped.**
  Every kind of line that can come without bound stops after 20,000 lines of its kind (`kMaxLinesPerKind`; the cap lines are in section 6.3); the events are then only counted, with a summary every minute where `DIAGNOSTIC_MODE` gives one, so no boot fills the 64 MB log.
  The tables behind a summary are bounded too: DirectStorage's open summary takes 100,000 entries per table, its `UNLOCK_FILES` summary 8,192 files and 15 types.
- **One wording** (section 6.3) and one log tag per part, the feature name.

## 13. Test builds

### 13.1 The switches

A test build (`-p:TweaksVTestBuild=true`, which defines `TWEAKSV_TEST_BUILD`) says `TEST BUILD - not for release` in the log's header and reads seven keys under `[TWEAKSV]`, never written to the file, and the environment variables below; one in force says so in a `TEST BUILD:` line.

| Switch | What it does, and for what |
|---|---|
| `TEST_BREAK_PATTERN=<label>` | that pattern counts as not found, to see its part skipped |
| `TEST_TWICE_PATTERN=<label>` | that pattern counts as held once more than the game code holds it, to see its part refuse it (section 8) |
| `TEST_LOG_LIMIT_KB=<KB>` (1..65536) | lowers the log's limit, to reach it |
| `TEST_DUMP_CODE=<seconds>` (1..3600) | saves the game's code (section 13.2) |
| `TEST_DUMP_DATA=<seconds>,<rva>:<bytes>[,...]` | saves named windows of the game's `.data` (section 13.2) |
| `TEST_DUMP_SCRIPT=<name>` | saves that script program each time the game places it (section 13.3) |
| `TEST_SIREN_PASSES=<n>` (0..31) | the sirens' wrapper (section 11.14.1) makes n more of both its renderer calls in each pass, so that real light records and coronas go the game's own way into stores whose numbers no normal load reaches (PLAN.md, D44) |
| `TWEAKSV_TEST_NO_BOXES` | `1` keeps every message box off the screen, `2` logs each box's whole text as well (section 6.4) |
| `TWEAKSV_TEST_FILE_INDEX=1` | DirectStorage reads the game folder's identity as on a file system without file IDs (section 11.8) |
| `TWEAKSV_TEST_NO_RENAME_EX=1` | DirectStorage answers every rename it turns as a file system that refuses POSIX semantics would, so that the fallback runs (section 11.8) |
| `TWEAKSV_TEST_SUMMARY_MS=<ms>` (100..60000) | a measurement's summaries after its first come that often, to see one come again within the minute a fake game runs (section 6.6) |
| `TWEAKSV_TEST_GUARD_ROOM=1` | each guard of WeaponLimits takes the game's own room (160 `weapons.meta` files, 470 weapon component definitions on 1.0.1158.16) in place of the part's, which stays, so that a load past the game's limit reaches the guards while the game stays safe (section 11.10; `GuardRoom`, asked once by each guard) |
| `TWEAKSV_TEST_LEGACY_FOLDER=<folder>` | the check for packs made for GTA V Legacy reads that folder, to reach one it cannot list, which RageOpenV's own reading of the mods folder stops at first (section 11.6) |
| `TWEAKSV_TEST_MODS_ENTRIES=<n>` (1..200000) | lowers the limit of the mods folder's walk, for RageOpenV's list and the check alike, to reach the stop with a handful of files (section 11.6) |
| `TWEAKSV_TEST_MODS_FAIL_LISTING=<folder>` | that folder's listing fails after its first name with Windows error 64, a share that drops, for the list and the check alike, to reach a listing that fails half-way (section 11.6) |
| `TWEAKSV_TEST_WEAPON_HALF_THROWS=<1 or 2>` | the weapon infos' half (1) or the weapon components' (2) throws in place of starting, to see it named in its problem and the other half start (section 11.10) |
| `TWEAKSV_TEST_THROWS=<what>` | what it names throws in place of starting, to reach the catch-all around it: a part by its tag, a measurement as `measurement <tag>`, the start of them all as `start-up`, which only `DllMain`'s `try` catches (sections 3 and 4.3) |
| `TWEAKSV_TEST_STATICBOUNDS_CHECK=<seconds>` (1..60) | the static bounds check comes that many seconds after the session's start, within the minute a fake game runs (section 11.15) |
| `TWEAKSV_TEST_STATICBOUNDS_NO_EVENT=asked` or `session` | the static bounds check does without that one of its two events, to reach a check that cannot be made (section 11.15) |
| `TWEAKSV_TEST_REFUSE_THREAD=<n> <what>` (n 1..9) | Windows refuses the first n starts of the thread `StartModuleThread` names so, with error 8, to reach what comes of a thread that cannot start (section 6.4) |
| `TWEAKSV_TEST_LIGHT_STUB=<value>` | reaches what comes of a stub or jump Windows refuses (section 11.16.3) or of the light stores' memory (section 11.17): `no memory`, `left writable`, `no stores`, `values refused`, `stores out of reach` |
| `TWEAKSV_TEST_CORONA_MEMORY=<value>` | reaches what comes of memory the corona store cannot have (section 11.17.1): `none`, `far`, `no stubs`, `far stubs` |
| `TWEAKSV_TEST_SIREN_WRITE_FAILS=<1 to 6>` | reaches what comes of the sirens' one write failing once the hook is on (section 11.14.1): `1` refused; `2` an exception in its place; `3` refused, with a hook that cannot be switched off; `4` an exception once it is done; `5` the hook refused; `6` the memory of the settings kept taken for none |

A test build also exports `TweaksVTestCoronaStoreNow`, the numbers of `CoronaStoreNow()`, so that the fake's report sees the accessor whichever line of the part reads it.
`WriteLog` leaves a last error of its own, `0x54574B`, after every line of a test build, so that a hook of the game's call that writes a line and does not hand back the game's last error fails the fake's `last error kept` fields; a good write sets none, so that on the Release build those fields pin the outcome and not the restore.

### 13.2 The code dump and the data windows (`TweaksVDump`)

`StartCodeDump()` and `StartDataDump()` take their first copies at once, in `DllMain`, before any part changes the game; a thread saves them and, after the seconds asked for, a second copy, in `%LOCALAPPDATA%\TweaksV` through the own-file checks, never in the game folder (the files: README, "Test Builds"; their layouts: `TweaksVDump.h`).
A code dump holds the PE headers and every section that is not writable, and a writable one only when it is also executable: the plain data sections can hold the player's data and are never saved.
It is how patterns are found after a game update (PATTERNS.md, "When Game Updates", item 3).
A data window is there since a research pass needs static data that lies in `.data`, the parser's member data first (D21); it is named for static data that holds no player's data, and `.data` as a whole stays out.
A value past README's limits is one problem and nothing is saved, the good windows neither, which takes in one RVA named twice, whose two windows would write one file, and a value of more than 254 characters, which a key cannot hold whole.

### 13.3 The script programs (`TweaksVScripts`) and their copy (`TEST_DUMP_SCRIPT`)

A script program is one of the game's compiled scripts as the game holds it in memory.
The game's resource constructor, on its resource placement thread, calls the function that resolves the natives (the call at +3 of `Y-E-H1`) and only afterwards enters the program in its registry of programs, so no script thread can run a program while that call is under way (PATTERNS.md, "Script programs").

`TweaksVScripts` owns the one redirect of that call, for every part that works on a script program.
`Scripts::AddUser`, from `DllMain` only, puts a user in a fixed table of four, so that the game's thread needs no allocation and no lock.
A user's `before` sees every program placed while the native table still holds the hashes, its `after` the program once the game has resolved them, each inside a `try`, with the game's last error kept around their work.
The first user makes the hook, which needs `Y-E-H1`, `Y-E-N1` and `Y-E-I1`, since they pin the layout the users rely on, and a call that leads into the game, or another mod may have hooked it already.
A later user joins a hook that is made, and gets the first user's reason when it could not be made, which each reports as its own problem.
The module holds the layout as constants, the four a pattern pins and the page rounding checked when TweaksV is compiled against the byte the pattern holds there (`PatternByte`, section 4.2).
Its bounds are 16 MB for the code and for the strings, since a call's target in the code is 24 bits, and 65536 natives, since a native's index is 16 bits.

`TEST_DUMP_SCRIPT=<name>` is its first user, at step 7 of the start-up; the Blips part joins at step 8 (section 11.9), so the copy shows a program as the game placed it, before the part writes to it.
The StaticBounds part joins after them, for the session's start (section 11.15).
The first program of all is logged, whatever its name: the line that proves the hook runs.
A program is copied only when its hash and its name are both the one asked for, at most 16 times a session: `before` copies the native table while it holds the hashes, and `after` the rest, writing the file on the placement thread.
A research reader is written against the file (`TweaksVDump.h`), so a change of its layout needs a new format version.

## 14. The tests

### 14.1 The harness

The suites are the arbiter: a change to a part passes the checks that pin its behaviour, or it is not done.

`bash test/run_all.sh`, from Git Bash, builds TweaksV's Release and test builds and the fake programs, runs the suites of its list side by side, as many as the machine has threads, and deletes `object\TestRun` and `object\Test`.
Each suite is one `run_<suite>.sh` with a folder of its own, `object\TestRun\<suite>`, so that what its fakes hold, delete or deny never reaches another's: a game folder `TestRun`, a name the suites' patterns of a path rely on, and `TestTemp` beside it.
The full run takes about ten minutes on my PC, about an hour with `--serial`, and its `time` line names the three slowest suites.
The Release build replaces `x64\Release\TweaksV.asi`, which each suite copies at its start, so nothing else may build it during a run.
A stopped run's suites and fakes go on and hold their folders, so a run that cannot delete the last run's folder stops at once.

`--serial` runs the suites one after another, and `TWEAKSV_TEST_JOBS` tells a suite that goes side by side itself how far.
`--keep` leaves the folders, so that a suite started by hand (`bash test/run_blips.sh`) finds its own again; `TWEAKSV_TEST_DIR` names another game folder.
`--no-build` tests the TweaksV builds already there, so it works after a `--keep` run only.

A run fails when a check fails, when a suite stops early, without its `N passed, M failed` line, when a suite makes another number of checks than its `CHECKS` line gives, when a build log holds a warning, and in a full run when the number of checks is not the one NOTES.md's `**Tests:**` bullet gives.
So the count stands in two places: `CHECKS=N`, the line below a suite's `source` of `common.sh`, compared in every run, so that a check lost shows in the suite it left, also beside another suite's new ones, and the bullet's `(N checks)`, read from that bullet alone, so that a check that silently stops running shows.

A fake game runs as `GTA5_Enhanced.exe`, loads the real `TweaksV.asi` the way an ASI loader does and prints a report whose fields the suites read.
`plain_game.exe` (`fake_game.cpp`) holds no game code, so every part that uses a pattern reports it missing.
`full_game.exe` (`fake_game_full.cpp`, `fake_game_rov.cpp`) holds a copy of every site TweaksV patches in `.fakesit`, an executable section, since TweaksV searches executable sections only, and their data at the game's offsets, which its own code reaches only through a pointer, so that TweaksV's scan counts no `lea` of its own.
Where a part's writes must be proved by what they do, not by their bytes alone, its sites run as code.
A site a part checks as a function start lies in a carrier, a compiled function the fake's unwind data names, or has a NOP before it, never a CC, which would add a line to `rageopenv`'s count of computed starts.
The full fake's options, each listed at the top of its source file, set up the scenarios; a new one brings its sites into `.fakesit` and a field of the report.
Other version resources, and none, make it other game builds.
`dstorage.dll` (`fake_dstorage.cpp`) stands in for DirectStorage, holding the files it opens, and `renamer.exe` (`fake_renamer.cpp`) replaces a file the stand-in holds from outside the game.

A fake shows what TweaksV does with sites laid out as my findings have them, and nothing of the real game: not that the findings hold, nor what a raise does under a real load.
The boot is the one test of the shipped binary in the game (section 15).

`common.sh` has the helpers:

- `fresh_ini [FILE]`: `base.ini` or FILE as `TweaksV.ini`, with `ERROR_POPUP=0`, `PEDPROPS_ON`, `STORES_ON` and `SIRENS_ON` switching on what `base.ini` leaves off; `setkey` and `addkey` edit it.
- `game EXE [OPTION...]`: a fake run as `GTA5_Enhanced.exe` through `run_fake`, with a fake `LOCALAPPDATA`, `TEMP` and `TMP`, for `RUN_TIMEOUT` seconds at most.
- `check DESCRIPTION CONDITION`: a pass or a fail; after a run that was stopped, gave no report or crashed after it, every check fails until the next run, as `TIMEOUT`, `NO REPORT` or `CRASH AT EXIT`.
- `skip DESCRIPTION`: a check the machine cannot make, counted as passed, so that the count is the same everywhere, and named in the results (`K skipped`).
- `has` reads the report; `log_has`, `log_line`, `count` and `log_lacks` read `TweaksV.log`, and `log_lacks` fails on a missing log, so that a negated check cannot pass on a log never written.
- `deny_writes DIR`, `deny_listing DIR` and `allow_writes DIR`: a folder read-only or unlistable for the user, and back.

Scenarios with `ERROR_POPUP=1`, the 1.x-file scenarios and the `TEST_` keys use the test build (`ASI=TweaksV_test.asi`), the only build that reads those keys and keeps the box off the screen (section 13.1).
A new pattern that must match more than once trips `story`'s and `packfile`'s counts of per-match lines.

A new check is a `check` line, with its suite's `CHECKS` line and NOTES.md's count raised by it.
A new suite is a `run_<suite>.sh` that sources `common.sh`, with its `CHECKS` line and a last line ending `N passed, M failed`, its name in `SUITES` and its rows in sections 14.2 and 14.3; section 16 has the rest of a part's.

### 14.2 What each suite pins

A suite's scenarios stand in its own script, whose header comment sums them up; the rows are in the order of `SUITES`.

| Suite | What it is for |
|---|---|
| `patterns` | PATTERNS.md lists exactly the table's patterns with the same bytes, the code uses each one exactly once, each held too often is refused in the same words |
| `ini_default` | the default file byte for byte, keys lost or crafted, older layouts carried forward, 1.x files set aside |
| `story` | StraightToStoryMode's screens, HeapAdjuster's heap and pool memory budget, the budget's measurement |
| `modkit` | ModkitLimit's array moved and its limits raised |
| `packfile` | PackfileLimit's array, sizes and hooked call, by game build |
| `boxstreamer` | BoxStreamer's list moved, the searches' capacities, the measurement |
| `decals` | Decals' array and limits, the measurement |
| `rageopenv` | RageOpenV's hooks end to end: the mods folder's paths and limits, the switches, other mods' hooks, an unload |
| `legacypacks` | the check for packs made for Legacy, on crafted RPF7 archives the fake writes: verdicts, damage, limits, no handle left open |
| `directstorage` | `UNLOCK_FILES` through the stand-in, the replace-renames, the open summaries, interfaces TweaksV does not know |
| `blips` | `blip_controller`'s guard raised at each placement, what refuses it, the blip pool's summary |
| `weaponinfos` | the weapon infos' raise, the guard on their loader, the room full, the measurement |
| `weaponcomponents` | the components' list moved, the pool's size call redirected, the guard on their loader, the measurement |
| `pedprops` | PedProps as it ships, off, and switched on: the narrowings widened, the complete sets of calls, the props per anchor measured |
| `texturebudget` | the Resources part: the budgets raised through the start-up's call, `STREAMED_OBJECTS`, the scan for the budgets' writers, the measurement |
| `raytracing` | the ray tracing buffer and its checks raised together, the refusal jumps' counting stubs, the measurement |
| `lightcapacity` | the light stores' measurement with its counting stubs, and the part: the stores moved, frames past the game's numbers run as code |
| `coronastore` | the corona store moved, adds and the job's copy run through it, the clamp's trap gone, its counters |
| `sirenids` | the siren setting IDs widened on the load path, run as code, packs' IDs past 255, the measurement |
| `sirencount` | the sirens per vehicle's lights: the renderer's second call on the fake's models, their measurement, `TEST_SIREN_PASSES` |
| `sirendamage` | the sirens per vehicle's damage: the rooms behind a bone structure and the game's loops raised, run as code |
| `staticbounds` | the store's size call redirected, names past the game's size carried, the read-back, the check that the game has asked |
| `population` | the two model budgets measured on the numbers of my boot, nothing written |
| `pedpool` | the ped pool measured from behind the game's obfuscation, nothing written |
| `renderqueues` | the render queues measured while the patterns' own bytes run, nothing written, the data read-only |
| `integration` | TweaksV as a whole, every part on: plugins that do a part's job, a broken pattern per part, imports and mitigations, links, held files and read-only folders, the dumps, the boxes, the catch-alls, the harness itself |

### 14.3 Which suites a change needs

One command runs some suites alone, `bash test/run_all.sh <suite>...`, side by side.
A change needs the suites of this table; the full run is a round's one proof.

| A change to | Needs |
|---|---|
| `TweaksVHeapAdjuster` | `story`, `ini_default`, `pedpool`, `integration` |
| `TweaksVStraightToStoryMode` | `story`, `ini_default`, `integration` |
| `TweaksVModkitLimit` | `modkit`, `integration` |
| `TweaksVPackfileLimit` | `packfile`, `ini_default`, `integration` |
| `TweaksVBoxStreamer` | `boxstreamer`, `ini_default`, `integration` |
| `TweaksVDecals` | `decals`, `ini_default`, `integration` |
| `TweaksVRageOpenV`, `TweaksVModsFolder`, `TweaksVLegacyPacks`, `TweaksVRpf.h` | `rageopenv`, `legacypacks`, `integration` |
| `TweaksVDirectStorage`, `test\fake_dstorage.cpp`, `test\fake_renamer.cpp` | `directstorage`, `integration` |
| `TweaksVBlips` | `blips`, `staticbounds`, `integration` |
| `TweaksVScripts` | `blips`, `pedprops`, `staticbounds`, `integration` |
| `TweaksVWeaponInfos` | `weaponinfos`, `integration` |
| `TweaksVWeaponComponents` | `weaponcomponents`, `integration` |
| `TweaksVPedProps` | `pedprops`, `integration` |
| `TweaksVTextureBudget` | `texturebudget`, `integration` |
| `TweaksVRayTracing` | `raytracing`, `integration` |
| `TweaksVSirenIds` | `sirenids`, `sirencount`, `lightcapacity`, `integration` |
| `TweaksVStaticBounds` | `staticbounds`, `blips`, `ini_default`, `integration` |
| `TweaksVSirenCount` | `sirencount`, `sirendamage`, `lightcapacity`, `coronastore`, `integration` |
| `TweaksVSirenRoom.h` | `sirencount`, `sirendamage`, `integration` |
| `TweaksVSirenDamage` | `sirendamage`, `integration` |
| `TweaksVLightCapacity` | `lightcapacity`, `ini_default`, `integration` |
| `TweaksVCoronaStore` | `coronastore`, `lightcapacity`, `ini_default`, `integration` |
| `TweaksVPopulation` | `population`, `integration` |
| `TweaksVPedPool` | `pedpool`, `integration` |
| `TweaksVRenderQueues` | `renderqueues`, `integration` |
| `TweaksVDump` | `blips`, `integration` |
| `TweaksV.vcxproj`, `version.rc` | `integration` |
| `TweaksVRelocation` | `ini_default`, `modkit`, `packfile`, `boxstreamer`, `decals`, `weaponcomponents`, `sirenids`, `integration` |
| `TweaksVMeasure` | `story`, `boxstreamer`, `decals`, `rageopenv`, `legacypacks`, `directstorage`, `blips`, `weaponinfos`, `weaponcomponents`, `pedprops`, `texturebudget`, `raytracing`, `sirenids`, `staticbounds`, `sirencount`, `sirendamage`, `lightcapacity`, `coronastore`, `population`, `pedpool`, `renderqueues`, `integration` |
| `TweaksVPatterns.h`, PATTERNS.md | `patterns`, and the suite of each part whose pattern changed |
| `test\run_<suite>.sh` | that suite, and its `CHECKS` line when its number of checks changes |
| `test\fake_game_full.cpp`, `test\fake_game_rov.cpp` | the suites whose scenarios it changes, and every suite when it changes what each run does or a field the suites read |
| `test\fake_game.cpp` | `story`, `modkit`, `packfile`, `boxstreamer`, `decals`, `rageopenv`, `legacypacks`, `weaponinfos`, `weaponcomponents`, `pedprops`, `texturebudget`, `raytracing`, `sirenids`, `staticbounds`, `sirencount`, `sirendamage`, `lightcapacity`, `coronastore`, `population`, `pedpool`, `renderqueues` |
| `test\fake_game_version.rc` | `packfile`, `patterns`, `sirencount`, `sirendamage` |
| `test\expected_default.ini`, `test\layout1.ini` | `ini_default`, `integration` |
| `test\deps.cmd`, `test\headers.cmd` | `integration` |
| `test\junction.cmd` | `rageopenv`, `integration` |
| the shared code - `dllmain.cpp`, `TweaksVFeatures`, `TweaksVSettings`, `TweaksVIni`, `TweaksVFiles`, `TweaksVGame`, `TweaksVLock.h`, `TweaksVLog`, `TweaksVSummary`, `TweaksVConflicts`, `TweaksVScanner`, `TweaksVPatch` - and `test\run_all.sh`, `test\common.sh`, `test\base.ini`, `test\build.cmd`, `test\build_tweaksv.cmd`, `test\vsenv.cmd` | the full run |

Every part's row needs `patterns` too, whose "every pattern used once" a part's `.cpp` alone can break by dropping a `Pattern("...")`.
A new part's suite adds its row, with `integration`, and its patterns join the patterns suite's run over the table by themselves: a pattern its part looks for only under a key other than `DIAGNOSTIC_MODE`, or a pattern of another game build, gets its line there, as `R-E-C1` and the packfile part's older patterns have.
Two things reach further than the table: a part that redirects a call adds a stub to the fake's `stubs: N, all RX`, which `boxstreamer`, `decals`, `rageopenv`, `blips`, `sirenids`, `sirencount`, `lightcapacity`, `population` and `integration` pin exactly, and a new log line can trip a count or a `log_lacks` in another suite, so it is tried against their patterns first.
`rtjumpunread` in `RtSetUp` serves `lightcapacity`'s `lcedge`, `coronastore`'s `ccedge` and `population`'s `popgateedge` too, each taking a page away after the section goes read+execute.

## 15. Building and releasing

`source\TweaksV.vcxproj` is a plain Visual Studio project, no generated build system: C++20, `/W4` with no warning tolerated, the C runtime linked statically, so that players need no redistributable.
MinHook builds unchanged at `/W3`: at `/W4` its own code warns, and that code is not TweaksV's to change.
`BuildInfo.h` is written before every build with the project's version and `git rev-parse --short HEAD`, `-dirty` appended while the tree has uncommitted changes; `version.rc` fills the file's version resource from it.
New source files get CRLF line endings; git normalizes every text file to LF, and `.sh` files and `test\*.ini` stay LF (`.gitattributes`).

`TweaksV.asi` carries ASLR with high entropy, DEP, Control Flow Guard and CET compatibility, and the integration suite checks all four on the binary.
Control Flow Guard and CET only take effect in a game that turns them on itself, which GTA V Enhanced 1.0.1158.16 does not; TweaksV carries them so that it is not refused by a game that does, and so that its own code leaves no gap in one, which MinHook's trampolines would leave (ISSUES.md, T-HARDEN).

Four builds matter: Release, Debug, the test build and the analysis build (README, "Building"); all four must build from scratch (`-t:Rebuild`: an incremental build shows no warnings for files it does not recompile) without a warning before a build reaches me, and `bash test/run_all.sh` must be green.
`object\Analyze` is deleted afterwards; `run_all.sh` deletes `object\Test` and `object\TestRun` itself.

A release is a tag `major.minor.patch` on the commit I booted, and a zip with exactly five files, for every version: `TweaksV.asi`, `LICENSE`, `THIRD_PARTY_NOTICES.md`, `CREDITS.md` and `README.md`; the `.pdb` stays with the build.
The release notes name the tag and link the repository, which `THIRD_PARTY_NOTICES.md` and `README.md` name too, so the Corresponding Source is one step from the binary wherever the zip goes (GPL-3.0, section 6d); the mod page carries the credits, links the repository and adds no restrictions of its own.
I check the zip on VirusTotal before uploading: a plain build with its version resource and no packer stays clean.
The order: the version in the project file, a local commit, Release rebuilt from it so the header carries that commit, my boots (with `DIAGNOSTIC_MODE=1` when patterns changed, with my map mods for anything that streams), then, with a clean log, an annotated tag named and messaged with the plain version, pushed together with `main`.

## 16. Extending TweaksV

- **A new setting:** one entry in `kSettings` (section 4.1) with its comment for the default file and a named reference, a row in README's table, and `expected_default.ini` with the new default file.
  A new key means a new layout number (`kIniLayoutVersion`; the `static_assert` on `IniKeyCount()` fails until its count is updated), which the suites pin in `run_ini_default.sh`, `base.ini` and `expected_default.ini`, and the keys of one section in `run_directstorage.sh`.
  A file of an older layout is carried forward at its next start (section 5.3), so a new layout needs nothing more in the code.
- **A new pattern:** one entry in `kPatterns` with a new label in its part's group, and the row in PATTERNS.md with the same bytes, the offset and what the match holds; the `patterns` suite fails until both agree.
  After a game update the same; PATTERNS.md's "When Game Updates" has the boot that shows every pattern matching as often as it must.
- **A plugin that does a part's job:** an old one TweaksV replaces is its file name in `oldPlugins` of each part whose job it does; one that stays installed is a `PeerPlugin` constant in `TweaksVFeatures.cpp`, named in `peerPlugins` of each such part, with a comment that says why it stays and what clashes.
  Either kind gets its scenarios in the integration suite, its row in section 4.3 and its name in README's "Testing" step 2.
- **A new part:** a section in `kSections` and `kSettings`, a feature in `kFeatures` in the same order with its old plugins, its plugins that stay and `Init<Part>()`; one file `TweaksV<Part>.cpp` with its header comment, its tag and its credits, listed with its header in `TweaksV.vcxproj` and `.vcxproj.filters`; patterns in the table; a suite `test\run_<part>.sh` with the sites added to the fake game, its name in `run_all.sh`'s list and its `CHECKS` line.
  Its documents: a section 11.x here, its game facts in PATTERNS.md, a section and the key rows in README, the version's entry and the new check count in NOTES.md, and the project it derives from or takes its idea from in CREDITS.md (and in THIRD_PARTY_NOTICES.md where it carries its code).
  It follows section 12, writes through `WriteCode` or `RedirectCalls`, hooks through `HookFunctions`, reports through `ReportFailure` - `ReportFinding`, with a row of its own in `TweaksVSummary`'s table, for what it finds while it does its job -, logs in the wording of section 6.3 and runs a measurement through `StartMeasurement` (section 6.6).
- **A limit measured first,** before its part is built (section 11.16): a module `TweaksV<Limit>.cpp` and `.h` with `Measure<Limit>()`, an entry in `kMeasuredFirst`, and its project files, patterns and suite as for a part.
  Its documents: its subsection of 11.16, its rows in sections 2 and 14.2, its section in PATTERNS.md, its paragraph in README's "Measured first", the version's entry and the new check count in NOTES.md.
  Once its part is built, the limit leaves `kMeasuredFirst` and its module becomes the part's.
- **A new array part:** a `RelocationSpec` with its anchors (section 10.1), `FindRelocation` then `ApplyRelocation`; what the core cannot express stays in the part and reads `RelocationPlan::matches`.
- **A part that works on a script program** (section 13.3): `Scripts::AddUser` from its `Init<Part>()`, the reason it gets back reported as its own problem, its work in its `before` or `after`, by the module's offsets and page rule, every read through `IsReadable`; a pattern of its own goes in a group of its own in the table (the Blips part's `L`).
- **A fork or a pull request** keeps the license (GPL-3.0-only), CREDITS.md and THIRD_PARTY_NOTICES.md, which must accompany every copy, and the pieces of code the notices name credited as they are; keeps the parts their own code, and carries no code of the projects that are idea only (CREDITS.md names them); keeps the suites green with the count updated in NOTES.md; and keeps the documents true: this file for what a part does and how, PATTERNS.md for every game fact, README.md for the player.
