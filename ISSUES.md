# Issues

Known bugs and open issues in TweaksV, each with what it is, its status and when it goes.
What each part does is in [README.md](README.md), the design as built in [SPECIFICATIONS.md](SPECIFICATIONS.md).

## T-HARDEN: MinHook's trampolines are writable and executable

- **What:** MinHook, the hook library, keeps the trampolines of TweaksV's hooks in memory that is writable and executable at once, and which Control Flow Guard treats as valid call targets - in a game that turned the guard on, the one gap TweaksV would leave in it.
  That only helps an attack that can already write to the game's memory.
  The rest of the code review's hardening is done: links, the user name and the size limit of `TweaksV.log`, problems after the start-up summary, the exploit mitigations.
  The walk of the mods folder follows links: `WalkModsFolder` and `ReadPack` read through a directory symlink in `mods`, so one that points at `\\server\share` makes TweaksV connect there at start-up.
  Making one takes Developer Mode or elevation, so no boundary is crossed, and a junction to another drive is how a player may keep a mods folder, so it stays as it is.
- **Status:** accepted.
  It is MinHook's own design, and `vendor\MinHook` stays unchanged; TweaksV's own stubs are never writable and executable at once.
- **When it goes:** when MinHook keeps its trampolines read and execute only, or TweaksV moves to a hook library that does.

## T-DIAG: a diagnostic aid that cannot be set up is logged, not reported

- **What:** the console (`CONSOLE=1`), DirectStorage's call logging (`DIAGNOSTIC_MODE=1`) and the measurements - `DIAGNOSTIC_MODE`'s and the `UNLOCK_FILES` summary - are aids.
  When one cannot be set up - the console cannot be opened or kept, a DirectStorage function cannot be hooked for the log, the game never asks for the DirectStorage factory those hooks wait on, a measurement's thread cannot start - `TweaksV.log` says so, but it is no `PROBLEM` line and no item of the start-up summary, so nothing on screen tells of it.
- **Status:** accepted.
  An aid is switched on for a test, by someone who reads `TweaksV.log` for what it gives - or misses the console's window - and finds the reason there, and the `UNLOCK_FILES` summary only says what the part did; the game and every part's job go on as without the aid.
- **When it goes:** when a player is meant to rely on one of these switches: then an aid that cannot be set up is reported like a job not done while its switch is on.

## T-FREEZE: RageOpenV's start-up writes without freezing the other threads

- **What:** the writes in `DllMain` happen while the game is still single-threaded.
  RageOpenV's start-up runs later, on the game's thread at its first request for the time, and writes its few bytes there - the cache check's `C3 90`, the console's NOPs, the five `E8 rel32` of the OPEN archives - without freezing the game's other threads, as MinHook does for the hooks.
  A thread that ran one of those instructions while it was written could execute it half old, half new.
- **Status:** accepted: no other thread runs that code at that point of the start-up.
  The Blips part's write after `DllMain` is not of this kind: it goes into a script program on the game's resource placement thread before the game enters the program in its registry, where no script thread can run it (PATTERNS.md, "Script programs").
- **When it goes:** when `WriteCode` freezes the game's other threads around a write made after `DllMain`, as MinHook does - at the latest when a game build runs one of those sites on another thread that early.

## T-DESCRIPTORS: the game's descriptor heap has no check at its end
The game's descriptor heap has no check at its end, so past what my proof shows the raise can end in a crash or a corrupted descriptor without a word; accepted on 2026-11-03; it goes when a load shows fewer than 16,384 free, and then a watch or a lower default comes back.
## T-PEDPROPS: the ped props' raise is untested and experimental, and ships off

- **What:** the part widens the game's eleven narrowings of a ped model's props per anchor, and no boot of mine has shown that this works past 255:
  - My boot of 2.6.0 (2026-11-01, the build of 80fee95) applied the widening at all 11 sites.
  - The switch to `mp_m_freemode_01` past 255, with my converted US Armed Forces pack (`r4usarmyv3`) in, crashed the game: 0xC0000005 at `GTA5_Enhanced.exe+0x5A0998D`, the same offset as in my test of 2.5.0 without the widening.
  - The control, with that pack out, switched fine at 255, with a hat at index 200.
  - Whether the crash comes from going past 255 or from the big packs' own data is not known: no test has gone past 255 without a big pack.
  - A hat past 255 has never been worn.
- **Status:** accepted for 2.6.0, on 2026-11-01: the raise's code stays and the suites run it switched on, on the fake game, and `[PEDPROPS] ENABLED` ships at 0, saying nothing about being off, as any part that is off, the default file's comment on it: `EXPERIMENTAL & UNTESTED - USE AT YOUR OWN RISK!!!`.
  With `DIAGNOSTIC_MODE=1` its measurement goes on while it is off, so that a load shows whether it needs the part.
- **When it goes:** a small clean single-prop pack answers it, later. The base game alone holds 255 head props on the male freemode model, so one add-on hat takes it past 255 without a big pack's data; the switch to that model, and that hat worn, say whether the raise works, and the default follows.
