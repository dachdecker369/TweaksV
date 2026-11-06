<img width="3000" height="1500" alt="TweaksV" src="https://github.com/user-attachments/assets/ba4f95d1-6278-44f9-b1ab-e5c92fe8cab3" />

# TweaksV

TweaksV is one ASI plugin for GTA V Enhanced, `TweaksV.asi`, with sixteen parts that raise the game's limits, serve a mods folder and shorten its start-up.
Each part has its own section in `TweaksV.ini` and can be switched on or off there; the table under "TweaksV.ini" gives every key's range and default.
Since 2.0.0 every part is TweaksV's own code, built on the patterns and the research of the plugins it replaces ([CREDITS.md](CREDITS.md)).

## Parts

### HeapAdjuster

`HEAP_SIZE` sets the size of the game's memory heap, 646 MB in the game.
A heap of 1024 MB or more was set by another adjuster: TweaksV leaves it and says so in the log.

`POOL_MEMORY` sets the budget the game's pools draw on, 136 MB in the game, which `HEAP_SIZE` does not touch.
Past it the game crashes at `+0x15A514`, and a large load of mods can come within a MB of it.
It is a ceiling: raising it takes no memory by itself.
Any other value than 136 MB found there is another mod's or a new game build's, and TweaksV leaves it.

### ModkitLimit

`MODKITARRAY_SIZE` moves the game's array of 1024 vehicle modkit IDs to a larger one and raises the eight limits in the game's code, so more add-on vehicles with modkits of their own fit.

### PackfileLimit

`PACKFILEARRAY_SIZE` moves the game's array of 7260 RPF archives to a larger one, raises both limits in the game's code and replaces the archive count of `gameconfig.xml` with the same number, so more add-on archives load.
The count is replaced exactly: the game's own configuration holds 8512 on 1.0.1158.16, so a value below 8512 lowers it.

### BoxStreamer

The game's box streamer keeps what one search for map boxes returns in a list of 1000 entries.
With heavy map mods a search returns more, the boxes past the list are never streamed, and their LODs are missing.
`TRAVERSAL_LIST_SIZE` moves the list to a larger one.
With `DIAGNOSTIC_MODE=1` the log says how many boxes the largest search returned and how many searches passed the game's 1000; measure with a larger list, since at 1000 no search can return more.

### Decals

The game reads its decal definitions - the kinds of bullet holes, blood, scorch and tyre marks, liquids, graffiti and so on - into an array of 260 entries and silently drops every further one.
The files are read in order, so it is add-on and mod content that goes missing, and vanilla 1.0.1158.16 already registers 259.
This is a limit on the kinds of decal, not on the decals on screen.
`DECALDEFS_SIZE` moves the array to a larger one and raises the limit everywhere the game uses it.
With `DIAGNOSTIC_MODE=1` the log says how many definitions are registered and whether the game's own array would have dropped some; with `DRY_RUN=1` as well it measures the game without the mods folder.

### RageOpenV

With `MODS_FOLDER=1`, a file under the `mods` folder in the game's current directory, normally the game folder, is served instead of the game's own by the same path (`mods\update\update.rpf` for `update\update.rpf`).
The folder is read once at start-up, so a file added while the game runs is seen at the next start.
A file in the mods folder that cannot be opened is never replaced by the game's own unnoticed: the game gets the failure, the log names each such file and a message box the first.
With `OPEN_RPF=1`, RPF archives that are not encrypted - the kind OpenIV writes, marked OPEN - load.
With `SKIP_RPF_CACHE_CHECK=1`, the game's check of its RPF cache is skipped, which modified archives need.

**The check of the packs:** with `MODS_FOLDER=1` TweaksV also checks the packs in the mods folder for GTA V Legacy, once at the start and on a thread of its own, so the game never waits for it.
A pack made for Legacy loads in Enhanced, but a vehicle from it stops the game the moment it is created, with no word on screen.
One message box names the packs made for Legacy and the log their Legacy models and textures: convert such a pack for Enhanced with CodeWalker or take it out of the mods folder.
A pack or a folder the check cannot read in full is named in the log and in the box as not checked.
The check has no key of its own: it reads the packs and changes nothing.

### StraightToStoryMode

`SKIP_SPLASH_SCREEN`, `SKIP_LEGAL_SCREEN` and `SKIP_LANDING_PAGE` skip the splash screen, the legal screen and the landing page, so the game goes straight into Story Mode.

### DirectStorage

With DirectStorage on, GTA V Enhanced opens every file in its folder at start-up, once its RPF cache check is skipped, and keeps them open, letting other programs only read them: while the game runs, mods can't write their own files there and scripts can't be reloaded.
`UNLOCK_FILES=1` lets the game open those files as before, but shared: other programs can write, rename and delete a file in the game folder that is opened only for reading.
The `.rpf` archives and `.cache` files stay locked, nothing is refused, and files outside the game folder, such as your saves, are not touched.
A mod inside the game can also replace such a file by renaming a new file over it: TweaksV makes that a rename with POSIX semantics, and the game goes on reading the file it opened until it opens it again.

Two limits remain, both Windows' rules for a file somebody holds open:

- A program that opens a file for writing while letting no one else even read it gets "file in use".
- A program outside the game that replaces a file by renaming a new file over it gets "access denied": it has to rename with POSIX semantics, use `ReplaceFile` or write the file in place.

### Blips

The game's own blips - the collectibles, the shops, the activities and the rest of the map's markers - are made by its script `blip_controller` only while fewer than 150 blips are on the map.
The blips a mod adds count too, so a mod with many of them pushes the game's own off the map.
`ACTIVE_BLIP_LIMIT` raises that threshold in the script each time the game loads it.
1997 is the most that still holds when the game's blip pool of 2000 is full; 1900 leaves room for the blips of missions and other scripts.
Where TweaksV does not find the check exactly as it expects, it leaves the script as it is and says why in a message box; a threshold another mod has changed is left and only logged.

### WeaponLimits

**Weapon infos:** the game reads each `weapons.meta` file into an array with room for 160 on 1.0.1158.16, of which the base game fills 155, and never checks that room: a file past it is written into memory the array does not own, and the game crashes.
`WEAPON_INFO_FILES` gives the array the room you set, 1024 taking about 254 KB of the game's memory, and guards it: a file past it is not loaded, the log names the first at once, and ten seconds later a message box names it and how many more were left out.
Raise the key then: the weapons of a file left out are missing, and the rest of their pack can look wrong.
Where another mod has raised the limit to your value or more, TweaksV leaves it and guards that room.

**Weapon components:** the game keeps the definitions of all `weaponcomponents.meta` files - each scope, clip, suppressor, flashlight, grip and tint - in a list of 470 and a pool of 470 slots; the base game alone uses 465, so weapon packs with more than five definitions between them end the game while it loads.
`WEAPON_COMPONENTS_SIZE` sets the room: TweaksV moves the list to one with room for 16,854 and gives the pool the room and 470 more, the most one file brings.
A `gameconfig.xml` that names another size for the pool (`CWeaponComponentInfo`) gets that size in its place.
The guard refuses a file once the room is used up: a box names the first file refused, the log every one, and their components are missing in the game, so raise the key then.
The files come in the order the game mounts them, so a load past the room may refuse a later file of the game's own DLC packs as well.
If definitions load while the game has never asked TweaksV for the pool's size, a box says so, since the pool then keeps the game's size.
One file still holds at most 470 definitions, the game's own limit.
Six of the 17 places in the game's code that use the list lie in its protected code; with `DIAGNOSTIC_MODE=1` TweaksV reads them back at every summary, and a box tells you should one be put back.

### PedProps

**Experimental and untested, off by default - use it at your own risk:** no hat past 255 has been worn yet, so `ENABLED` under `[PEDPROPS]` ships at 0; what is still open is in [ISSUES.md](ISSUES.md), T-PEDPROPS.

A ped model carries its props - hats, helmets, glasses, earrings, watches - on 13 anchors, and the game's checks take the props of one anchor, all files together, as 8 bits: past 255, only the props below (total mod 256) can be put on.
On 1.0.1158.16 the base game holds 255 head props on the male freemode model and 254 on the female, so a single add-on hat takes the male one past the limit.
The part widens those eleven checks to 16 bits, so an anchor holds up to 65535 props; below 256 they give exactly what the game's own give.
TweaksV changes the game only where it finds all eleven checks exactly as it expects, and otherwise says why in a message box.
A file with more than 255 props on one anchor stays its pack's to split: its own data cannot name them.
In my tests the game crashed at the switch to the male freemode model past 255 head props with a big clothing pack in, with the part and without it, while 255 switched fine; the cause is not known yet.
With `DIAGNOSTIC_MODE=1` the log says the most props on one anchor, also while the part is off, so you can see whether your packs need it.

### Resources

The game streams textures and buffers into a video memory budget that `gameconfig.xml` sets by texture quality - on 1.0.1158.16 1,500 MB at Normal and 3,000 MB at High and Very High - and drops what it cannot place, so textures go missing even when the card has memory to spare.
`VIDEO_MEMORY_BUDGET` raises each of the four budgets below it as the game's start-up sets them; none is lowered, so a `gameconfig.xml` that gives more keeps its own.
Choose it for your card: past what the card has left beside everything else, at 3840x2160 with ray tracing several GB, the textures only move out to system memory, which costs speed.

`STREAMED_OBJECTS` raises the other limit in the same place: how many streamed textures and buffers the game places at once, 98,304 in its own `gameconfig.xml`, which a load reaches at about 4.7 GB of them.
147,456 is the most, 3/4 of the game's descriptor heap, a top no load has yet proved.
A `gameconfig.xml` above that top is kept with a problem, since the game does not check that heap's end and a load that reaches it crashes without a word.

TweaksV has to start before the game's start-up sets the budgets, as it does with Script Hook V's loader.
If it does not, where another mod has hooked the same call, or where the game's own code writes a budget besides its start-up, that budget is not raised and the start-up summary says so.
With `DIAGNOSTIC_MODE=1` the log says how much of the budget is in use (`2845 MB of the 6000 MB budget`), how many objects and descriptors, and ` - at the budget` when the game is dropping what it cannot place.

### RayTracing

With ray tracing on, the game skins every moving object it ray traces into one buffer of 4,194,304 vertices each frame; an object that no longer fits is left out of that frame's ray tracing, so its shadow, reflection and occlusion come and go.
With many high-poly vehicles and peds that is the shadow under a car that flickers or goes missing.
`DYNAMIC_BVH_VERTICES` raises the buffer and the game's two checks of it together.
The buffer is video memory, made once at the game's start: 8,388,608 vertices cost 80 MB more, 16,777,216 cost 240 MB more.
With `DIAGNOSTIC_MODE=1`, also at 0, `geometries refused` in its line means the buffer is too small for your load, and none refused while shadows still flicker means another cause.

### LightCapacity

A vehicle's lights, siren lights among them, pass through two lists: each group of 5 vehicles gets 80 lights and 200 coronas, and a light past that goes dark; then the frame's stores of 400 lights and 928 coronas, which the game does not check, so past them it writes over its own memory and the crash is next.
This part gives each group 180 lights and 300 coronas, the most the game's working memory allows, and the frame's stores room for all of those, 5.7 MB near the game.
It also fixes a slip in the game: past its 1024 lights a frame the scene keeps the nearer light, but over the wrong slot; now the farthest goes.
`CORONA_STORE` moves the store of a frame's coronas, 1088 in the game, past which it drops the rest without a word, the vehicles' first; 4096 takes 197 KB near the game.
The part raises nothing you see by itself, and many real lights cost frame rate; the sirens per vehicle need it, and refuse with a problem without it.

### SirenSettings

Every siren setting of `carcols.meta` has an ID that vehicles name it by, and the game keeps it in one byte: an ID above 255 is cut to its last byte - 300 becomes 44 - so packs that use large IDs clash with each other and with the game's own, and their settings are dropped or mixed up without a word.
`LARGE_SIREN_IDS` makes the IDs 16 bits the whole way from the file to the vehicle: a pack may use 0 to 65535 but 255, which stays the game's "none" - and keep 0 and 65535 free too, as the plugin that had the idea first asks.
A pack's setting whose ID is taken is still dropped, and a clash within one file still moves the later setting.
Packs that relied on the cut - a setting named 372 whose vehicles say 116 - lose their link.
The 255 settings stay the most: the position in the game's array stays a byte.

`SIRENS_PER_VEHICLE` lights `siren21` to `siren32` of a model made for them, the items past the 20th of its setting in `carcols.meta`, through a second call of the game's own renderer; a vehicle without them costs one lookup per pass.
Damage reaches them as it does the first 20.
A vehicle whose siren setting ID finds no setting lights nothing, where the game would read past its settings.
It needs the light stores raised (`[LIGHTCAPACITY]`), and it runs only on the game build TweaksV was read on: another brings a message, and vehicles keep 20 sirens until a new TweaksV.
Two settings with the same first 20 lights share the lights past them, the later's.

With `DIAGNOSTIC_MODE=1` the log says how many settings and IDs are in use, and per `carcols` file what the game did with its settings: a dropped setting's vehicles get the setting that had the ID first, so give it another.

### StaticBounds

The game keeps the static collision bounds of the map - every building, prop and interior it places, a heavy map mod's MLOs and expansions among them - in one store, whose size it reads once from `gameconfig.xml`, or from its own default.
Once the store is full, the game ends at a trap: the crash heavy map mods are known for.
`STATICBOUNDS_SIZE` is the least the store gets, and a `gameconfig.xml` that gives more keeps it.
32767 is the most whose every slot is sound; the store costs about 73 bytes a slot.
A value above 16000 spends the budget the game's pools draw on: while `POOL_MEMORY` is not raised the start-up message says so, and the store is raised all the same.
If the game has not asked for the size a minute after your session starts, a message box says so, and the store has the game's size; where that check cannot be made, the start-up message or the log says so.
At the session's start TweaksV reads the store back: a store whose numbers differ gets a message box, since the load would end at the smallest, and so does a `gameconfig.xml` above 32767 with more than that in use.

### Measured first: the sirens per vehicle, the lighting capacity, the model budgets, the ped pool and the render queues

A limit gets a raise only once a mod load reaches it ([ROADMAP.md](ROADMAP.md)), so these are measured: with `DIAGNOSTIC_MODE=1` TweaksV logs how close the game and your mods come, 10 seconds in and then every minute while the numbers change.
Two of them change something in the game, with that switch only: the call that draws a vehicle's sirens and the jumps a full list of lights takes pass through TweaksV; with `DRY_RUN=1` they stay as they are.

**Siren count** (SirenSettings): how many models the game draws sirens on, the most sirens it names, and which carry siren bones past `siren20` or past bone 127, which stay dark without an error.
The game draws sirens only while they are on: drive an emergency vehicle with its sirens on, or let the police chase you.

**Lighting capacity** (LightCapacity): how full each layer of a frame's lights gets, about a hundred times a second: the vehicle lights and coronas, the scene's lights, the ray traced list, the reflection lists, the coronas and the most one job held, and what your siren settings ask for.
In `coronas 1088 of 4096, refused 0`, a count of 0 refused says that the store is enough.

**The model budgets** (Population): the game holds only so many bytes of vehicle and ped models - at the Population Variety slider's top, 429 MB and 561 MB - and past that asks for no further model, so traffic and crowds repeat fewer of them than your mods could give.
`vehicle model budget:` and `ped model budget:` say how many MB are in use, and `at the stop in 170 of 171 samples` that the budget binds on your load.
Their `tables:` lines are the one proof that a `ConfigPopulationStreaming` block of your own `gameconfig.xml` has taken, since the game ignores a misspelled tag without a word.

**Ped pool** (PedPool): a full pool of 256 slots only means that no more peds appear; `Peds` raised above 256 in a `gameconfig.xml`, though, boots well and corrupts memory once more than 256 peds are alive, as three arrays of the game stay at 256.
The log says how many slots are in use, and says so when the pool has been full, or at once when the spatial array has passed 256.

**Render queues** (RenderQueues): what streamed peds, their props and vehicles with mods draw in a frame stays in small per-frame arrays, and past 120 entries of the first two the game writes into memory that is not theirs, which can crash it much later and elsewhere.
`FULL` and `PAST ITS CAPACITY` are the words to look for; many multiplayer-style peds in view, and mods set on many cars, touch them.

## TweaksV.ini

TweaksV writes `TweaksV.ini` in the game folder, next to `GTA5_Enhanced.exe`, on the first start - or in `%LOCALAPPDATA%\TweaksV` if the game folder cannot be written to - with every setting at its default, and adds back any setting that goes missing.
`1` is on, `0` is off (`true` and `false` work too), and a comment after a value - after `//` or `;` - is never part of it.
The settings are read once, at start-up, so a changed file takes effect at the next start; there is no live panel, by design.
A file from an earlier version gets the keys it lacks added and its `VERSION` raised to 5; its values stay as they are, without the comments: delete it to get the commented one, then set your values again.

| Section | Key | Default | What it does |
|---|---|---|---|
| `[TWEAKSV]` | `ENABLED` | 1 | the master switch: 0 means TweaksV changes nothing |
| | `LOG` | 1 | writes `TweaksV.log`, in the game folder or in `%LOCALAPPDATA%\TweaksV` when the game folder cannot take it |
| | `DIAGNOSTIC_MODE` | 0 | testing: every part that is on logs every pattern, site and write, and runs its measurement - see "Diagnostics" below |
| | `DRY_RUN` | 0 | testing: everything is looked for and checked, nothing in the game is written or hooked, and the mods folder is not served - see "Diagnostics" below |
| | `CONSOLE` | 0 | a console window that mirrors the log |
| | `ERROR_POPUP` | 1 | a message box when something could not be applied, and one for a part that stops working later and for what the check of the packs or a guard of WeaponLimits found (at most three of each per session) |
| | `CONFLICT_CHECK` | 1 | looks for the old plugins TweaksV replaces and for NaturalVision Enhanced's loader and Resource Extensions, in the game folder and its `scripts` and `plugins` subfolders, and switches the matching part off while they are there (see "Other mods") |
| | `VERSION` | 5 | the layout of this file; TweaksV keeps it, and raises it when it adds the keys of a newer layout - leave the line in |
| `[HEAP_ADJUSTER]` | `ENABLED` | 1 | the part on or off - every part's section has this key, 1 by default but under `[PEDPROPS]` |
| | `HEAP_SIZE` | 2000 | the game's heap in MB, 1 to 2047; a value below the game's own is raised to it and reported |
| | `POOL_MEMORY` | 256 | the memory budget of the game's pools in MB, 136 to 2047 |
| `[MODKIT_LIMIT]` | `MODKITARRAY_SIZE` | 65536 | the number of vehicle modkit IDs, 1 to 65536; a value below the game's own 1024 is raised to it and reported |
| `[PACKFILE_LIMIT]` | `PACKFILEARRAY_SIZE` | 13000 | the number of RPF archives, 1 to 32770, a multiple of 5; a value below the game's own 7260 is raised to it and reported, and another is rounded down and reported; it also replaces the archive count of `gameconfig.xml`, exactly |
| `[BOXSTREAMER]` | `TRAVERSAL_LIST_SIZE` | 10000 | map boxes one search of the box streamer can return, 1000 to 100000 (the game's own: 1000) |
| `[DECALS]` | `DECALDEFS_SIZE` | 2048 | decal definitions the game can hold, 260 to 16384 (the game's own: 260) |
| `[RAGEOPENV]` | `MODS_FOLDER` | 1 | replacement files from the `mods` folder in the game's current directory, normally the game folder |
| | `OPEN_RPF` | 1 | unencrypted ("OPEN") RPF archives can be loaded |
| | `SKIP_RPF_CACHE_CHECK` | 1 | skips the game's RPF cache check (needed for modified archives; 0 only for testing) |
| `[STRAIGHTTOSTORYMODE]` | `SKIP_SPLASH_SCREEN` | 1 | skips the splash screen |
| | `SKIP_LEGAL_SCREEN` | 1 | skips the legal screen |
| | `SKIP_LANDING_PAGE` | 1 | goes straight into Story Mode |
| `[DIRECTSTORAGE]` | `UNLOCK_FILES` | 1 | the files in the game folder other than `.rpf` and `.cache` stay free while the game holds them open, within two limits (see "DirectStorage") |
| `[BLIPS]` | `ACTIVE_BLIP_LIMIT` | 1900 | the game's own blips are made while fewer blips than this are on the map, 150 to 1997 (the game's own: 150) |
| `[WEAPON_LIMITS]` | `WEAPON_INFO_FILES` | 1024 | the `weapons.meta` files the game can load, 160 to 16384 (the game's own: 160) |
| | `WEAPON_COMPONENTS_SIZE` | 1024 | the weapon component definitions of all `weaponcomponents.meta` files together, 470 to 16384 (the game's own: 470) |
| `[PEDPROPS]` | `ENABLED` | 0 | experimental and untested, at your own risk: the props per anchor of a ped model: 255 in the game's own checks, 65535 with the part on |
| `[RESOURCES]` | `VIDEO_MEMORY_BUDGET` | 6000 | the video memory in MB the game may stream textures and buffers into; 0 keeps the game's own budgets, 1500 to 16384 raises each budget below it to it (a value below 1500 counts as 1500) |
| | `STREAMED_OBJECTS` | 131072 | the most streamed textures and buffers; 0 keeps the game's own 98304, 98304 to 147456 raises a lower one to it (a value below 98304 counts as 98304) |
| `[RAYTRACING]` | `DYNAMIC_BVH_VERTICES` | 0 | the ray tracing buffer for moving objects in vertices, 4194304 to 26843545; 0 keeps the game's own 4194304, a value below it is raised to it and reported; 20 bytes of video memory a vertex, used or not |
| `[LIGHTCAPACITY]` | `CORONA_STORE` | 4096 | the coronas a frame holds, 1088 to 8192; 1088 keeps the game's own and looks for nothing of it; 48 bytes a corona, used or not |
| `[SIRENSETTINGS]` | `LARGE_SIREN_IDS` | 1 | siren setting IDs up to 65535, on the whole way from the file to the vehicle; 0 keeps the game's 8 bits |
| | `SIRENS_PER_VEHICLE` | 32 | the sirens a vehicle lights, 20 to 32; 20 keeps the game's own; above 20 it needs `[LIGHTCAPACITY]` on and this game build |
| `[STATICBOUNDS]` | `STATICBOUNDS_SIZE` | 16000 | the game's static collision bounds, 1 to 32767; a larger number in `gameconfig.xml` is kept; above 16000 it wants `POOL_MEMORY` raised, and a problem says so where it is not |

**Coming from TweaksV 1.x:** the 1.x file is not migrated.
TweaksV renames it, or any `TweaksV.ini` without `VERSION` under `[TWEAKSV]`, to `TweaksV.old.ini`, writes a fresh one and says so in the start-up message; set your values again, the old file still holds them.

**Diagnostics:** `DIAGNOSTIC_MODE=1` makes every part that is on log every pattern it looks for (address, number of matches, bytes), every site it finds and every write, and run its measurement.
A part with `ENABLED=0` logs nothing, but for PedProps' measurement and SirenSettings' two.
Two ways to use it:

- With `DRY_RUN=1`, the quiet check after a game update: nothing in the game is written or hooked (only RageOpenV's one-shot hook on `GetSystemTimeAsFileTime`, which runs its checks and switches itself off), and the log shows the patterns and the sites of the unpatched game; `blip_controller`'s guard only a boot without `DRY_RUN` shows.
- Alone, the full measurement of the running game with the mods folder served; it hooks a little more than a normal boot.

Every kind of line that comes per event stops after 20,000, and the events are then only counted, with a summary every minute where the part has one.

## Design

- **GTA V Enhanced only:** TweaksV does nothing in Legacy.
- **Own code:** what remains of the originals' code is named in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
- **The whole design:** [SPECIFICATIONS.md](SPECIFICATIONS.md) describes every module, part, format and rule as built, [PATTERNS.md](PATTERNS.md) every game fact and is the one place to work when the game changes, [ISSUES.md](ISSUES.md) what is still open, [NOTES.md](NOTES.md) the versions and the count of the checks - with this file, enough to fork it or build on it; the source holds the rest, the words of the log and the order of the checks.

## Building

You need Visual Studio 2026 with the "Desktop development with C++" workload.
Everything else, including MinHook, is in this repo.

Open `source\TweaksV.slnx` in Visual Studio, choose **Release** or **Debug**, and build.
Or use x64 Native Tools Command Prompt for VS:

```
git clone https://github.com/dachdecker369/TweaksV
cd TweaksV
msbuild source\TweaksV.slnx -p:Configuration=Release -p:Platform=x64
```

- **Output:** `x64\Release\TweaksV.asi` and `TweaksV.pdb`, for Debug builds `x64\Debug`; intermediates go to `object\Release` and `object\Debug`.
- **No Redistributables:** C runtime is statically linked so players don't need it.
- **Commits:** when the folder is a git repo, `TweaksV.log` lists the commit the build came from; otherwise it lists "unknown".
- **Tests:** `bash test/run_all.sh` in Git Bash, in the repository folder, builds the Release and test builds and small fake-game programs and runs every suite side by side.
  `--keep` leaves what it made in `object\`, `--no-build` skips building TweaksV, `--serial` runs the suites one after another, and a suite's name, as in `bash test/run_all.sh blips`, runs that suite alone.
  Building Release replaces `x64\Release\TweaksV.asi`.
- **Code Analysis:** `-p:TweaksVAnalyze=true` on the `msbuild` line runs the MSVC code analysis (`/analyze`), into `object\Analyze`.
- **Test Builds:** `-p:TweaksVTestBuild=true` on the `msbuild` line gives `object\Test\Release\TweaksV.asi`, whose log header lists `TEST BUILD`; these keys go under `[TWEAKSV]`, and [SPECIFICATIONS.md](SPECIFICATIONS.md), section 13, has them all.
  `TEST_BREAK_PATTERN=<label>` has a pattern of [PATTERNS.md](PATTERNS.md) count as missing, `TEST_TWICE_PATTERN=<label>` as held once more, and `TEST_LOG_LIMIT_KB=<KB>` lowers the 64 MB limit of `TweaksV.log`.
  `TEST_DUMP_CODE=<seconds>` (1 to 3600) saves the game's decrypted code, its constants and function table, never its data, as `GameCode-start.bin` and `GameCode-later.bin`, about 50 MB each, in `%LOCALAPPDATA%\TweaksV`.
  `TEST_DUMP_DATA=<seconds>,<rva>:<bytes>[,<rva>:<bytes>...]` saves windows of `.data` beside them, at most 16 of at most 64 KB each, the numbers in hex, each wholly inside `.data`, as `GameData-<rva>-start.bin` and `-later.bin`.
  `TEST_DUMP_SCRIPT=<name>` saves a script program as the game places it, 1 to 63 letters, digits and `_`, at most 16 times a session, as `Script-<name>-<n>.bin`.
  `TEST_SIREN_PASSES=<n>` (0 to 31) has each pass of a vehicle's sirens run the game's renderer n more times, so that a handful of cars loads the lighting stores as no normal load does.
  Delete the dumps when you are done; their format is in `source\dump.h`.

## Testing

1. Use GTA V Enhanced in Story Mode with BattlEye disabled.
   Never go online with mods.
2. Remove the old plugins TweaksV replaces from the game folder and its `scripts` and `plugins` folders: `HeapAdjuster.asi` (or `ZHeapAdjuster.asi`, the name its author suggests), `ModkitLimitAdjusterEnhanced.asi`, `PackfileLimitAdjusterEnhanced.asi`, `RageOpenV.asi`, `OpenRPF.asi`, `OpenIV.asi`, `StraightToStoryMode.asi`, alloc8or's `DirectStorageFix.asi`, `DecalPatch.asi`, rootBrz's `BlipLimitFix.asi`, `WeaponLimitsAdjusterEnhanced.asi`, `GTAV.ResourceAdjusterEnhanced.asi` and Nochala's `StaticBoundLimitAdjuster.asi` (also named `StaticBoundsLimitAdjuster.asi`) - the fifteen old plugins the conflict check knows.
   While one of them is there, TweaksV switches its matching part off and says so in the start-up message.
   Two more files it knows stay: `onigiri.asi` and `ResourceExtensions.asi` (see "Other mods").
3. `TweaksV.asi` goes in the game folder next to `GTA5_Enhanced.exe`.
4. Add the ASI loader file `xinput1_4.dll` from the [Script Hook V](http://www.dev-c.com/gtav/scripthookv/) download.
   It is the only file needed; Script Hook V itself is not.

## Other mods

- **NaturalVision Enhanced** sets the heap size as well: TweaksV leaves it and says so in the log, so NVE's wins, whatever the load order.
  Its loader, `onigiri.asi`, patches the same places in the game as TweaksV's RageOpenV and StraightToStoryMode parts, after TweaksV, and crashes the game when TweaksV has changed them first.
  So while it is installed, TweaksV switches those two parts off and says so in the start-up message (with `CONFLICT_CHECK=0` TweaksV does not look for it, and the game crashes while those two parts are on).
  Your mods then go in its `onigiri` folder, and which start-up screens are skipped is set in its `onigiri.ini` under `[PATCHES]`.
  To stop the message, set `ENABLED=0` under `[RAGEOPENV]` and under `[STRAIGHTTOSTORYMODE]`.
- **Static Bounds Limit Adjuster** by Nochala: delete it, TweaksV replaces it.
  `STATICBOUNDS_SIZE` is the original's own key, with its default; a value above 32767 is cut to 32767 and reported.
  The original reads 0 as its default, 16000, while TweaksV reports a 0 and uses it as 1, which leaves the game's own size: set 16000 for the original's default.
- **Resource Extensions [Enhanced]** by Koukol, a closed-source plugin that raises many of the game's capacities: while `ResourceExtensions.asi` is installed, TweaksV's HeapAdjuster, PackfileLimit, Blips, RayTracing and LightCapacity parts are off, and the start-up message says so, with the `ENABLED=0` that stops it.
- **gameconfig.xml:** a gameconfig that raises the archive count has no effect while the Packfile part is on - TweaksV replaces that count with `PACKFILEARRAY_SIZE`, exactly, also when the gameconfig's count is higher.
  Its pool size for the weapon components (`CWeaponComponentInfo`) is replaced by the WeaponLimits part's, and its texture budgets, its `MaxStreamingResources` and its static bounds store's size are raised to their keys where they are lower.
  Everything else in a gameconfig works as usual.
- **Zolika Patch:** the ModkitLimit Extender's author warned that its modkit patch may not be compatible with Zolika Patch's; the same goes for TweaksV's Modkit part.
- **DecalPatch.asi**, a closed-source plugin that raises the box streamer list and the decal definitions limit: delete it, TweaksV replaces both halves, and switches those two parts off while it is installed.
- **Mods that hook the game's file functions:** TweaksV's RageOpenV part hooks on top of another mod's hook on GetFileTime or GetFileSize and chains through it.
  A hook on OpenBulk or GetAttributes changes the bytes TweaksV finds those two functions by: the mods folder is then not used, and the start-up message names the pattern that was not found.
- **Collectibles on Map and Radar** puts so many markers on the map that the game's own script stops: with the Blips part on, the game's own blips stay next to them, and Script Hook V, which the mod needs, runs next to TweaksV without a conflict.
- **Script Hook V, ScriptHookVDotNet, OpenIV:** neither needed nor touched.
  Only the ASI loader file from the Script Hook V download is used, and OpenIV's `OpenIV.asi` and `OpenRPF.asi` must not be installed next to TweaksV (their job is TweaksV's RageOpenV part).

## Reporting a problem

Attach `TweaksV.log` - its first lines name the game build and the files in use - and `TweaksV.ini`; say which store the game comes from (Rockstar, Steam, Epic); list the other `.asi` and `.dll` files in the game folder; and describe what happened against what you expected.
A crash dump helps if you have one.

## Security

Please report a security problem privately, not in a public issue: [SECURITY.md](SECURITY.md) says how.

## Credits

Five of TweaksV's parts are rewritten from the five plugins by [Chiheb-Bacha](https://github.com/Chiheb-Bacha), and StaticBounds takes its key, default and rule from Nochala's Static Bounds Limit Adjuster.
[CREDITS.md](CREDITS.md) names everyone whose research they build on and the ideas behind TweaksV's own ten parts, and all credit for the original research and code goes to them.
Third-party notices are in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## License

Copyright (C) dachdecker369.
TweaksV is free software under the GNU General Public License, version 3 only; the text is in [LICENSE](LICENSE).
