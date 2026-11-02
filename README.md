# TweaksV

TweaksV is one ASI plugin for GTA V Enhanced, `TweaksV.asi`, with fifteen parts: a larger memory heap, room for more vehicle modkits and more RPF archives, a larger traversal list for the game's box streamer, more decal definitions, a mods folder with unencrypted archives whose packs are checked for GTA V Legacy, a start straight into Story Mode, the files DirectStorage keeps locked set free, the game's own blips kept on the map next to a mod's, room for more `weapons.meta` files and weapon components, more props per anchor of a ped (experimental, off by default), a larger video memory budget for streamed textures, a larger ray tracing buffer for moving geometry, siren setting IDs past 255 and a larger pool of static collision bounds.
Five of the parts are written anew from plugins by [Chiheb-Bacha](https://github.com/Chiheb-Bacha), which TweaksV 1.x merged as they were, and one, StaticBounds, from the source of Nochala's Static Bounds Limit Adjuster; since 2.0.0 every part is TweaksV's own code, built on the patterns and the research of the originals.
Each part has its own section in `TweaksV.ini` and can be switched on or off there.

## Parts

### HeapAdjuster

`HEAP_SIZE` under `[HEAP_ADJUSTER]` (default 2000 MB, 1 to 2047) sets the size of the game's memory heap, one number in its start-up code; the game's own is 646 MB.
A value below the game's own is raised to it and reported.
A heap of 1024 MB or more is not the game's own: another heap adjuster set it, and TweaksV leaves it as it is and says so in the log.
Replaces HeapAdjusterEnhanced.

`POOL_MEMORY` under `[HEAP_ADJUSTER]` (default 256 MB, 136 to 2047) sets a budget of its own that the game's pools draw on, which `HEAP_SIZE` does not touch; the game's own is 136 MB.
Past it the game crashes at `+0x15A514`, and a large load of mods can come within a MB of it, a raised `gameconfig.xml` or a plugin that extends pools more easily still.
It is a ceiling: raising it takes no memory by itself.
Any other value than 136 MB found there is another mod's or a new game build's, which TweaksV leaves as it is and says so in the log.

### ModkitLimit

`MODKITARRAY_SIZE` under `[MODKIT_LIMIT]` (default 65536, 1 to 65536; a value below the game's own 1024 is raised to it and reported) moves the game's array of vehicle modkit IDs, 1024 entries, to a larger one and raises the eight limits the game's code carries for it, so more add-on vehicles with modkits of their own fit.
Replaces ModkitLimitAdjusterEnhanced.

### PackfileLimit

`PACKFILEARRAY_SIZE` under `[PACKFILE_LIMIT]` (default 13000, 1 to 65535; a value below the game's own 7260 is raised to it and reported) moves the game's array of RPF archives, 7260 entries, to a larger one, raises both limits the game's code carries for it, and replaces the archive count of `gameconfig.xml` with the same number when the game reads its configuration, so more add-on archives load.
The count is replaced exactly, also when it was higher: the game's own configuration holds 8512 on 1.0.1158.16, so a `PACKFILEARRAY_SIZE` below 8512 lowers it.
Replaces PackfileLimitAdjusterEnhanced.

### BoxStreamer

The game's box streamer finds the map boxes to stream for the area around the player and keeps what one search returns in a list of 1000 entries.
With heavy map mods a search returns more: the boxes the list has no room for are never streamed, and their LODs are missing.

`TRAVERSAL_LIST_SIZE` under `[BOXSTREAMER]` (default 10000, 1000 to 100000) moves the list to a larger one.
TweaksV's own part; the idea comes from the CitizenFX Collective's fix for FiveM and 0x-FADED's port of it to GTA V Legacy, and none of their code was read or used.

With `DIAGNOSTIC_MODE=1` the part counts the searches and logs, 10 seconds in and then every minute while the numbers change, how many map boxes the largest search returned and how many searches returned more than the game's own list could hold - so you can see whether your map load needs the larger list at all.
Measure with a larger list: with `TRAVERSAL_LIST_SIZE=1000`, the game's own size, no search can return more than the list holds, and the log says so.

### Decals

The game reads its decal definitions - the kinds of bullet holes, blood, scorch and tyre marks, liquids, graffiti and so on - from the `decals.dat` files into an array of 260 entries.
Once 260 are registered, every further definition is dropped silently; the files are read in order, so it is the definitions of the last files - add-on and mod content - that go missing, and the decals that use them never appear.
This is a limit on the kinds of decal, not on the decals on screen.
Vanilla 1.0.1158.16 already registers 259 of the 260, so a mod that adds two definitions is one too many for the game's own array.

`DECALDEFS_SIZE` under `[DECALS]` (default 2048, 260 to 16384) moves the array to a larger one and raises the limit everywhere the game uses it.
TweaksV's own part, written from TweaksV's own findings about the game; the idea comes from the CitizenFX Collective's fix for FiveM and 0x-FADED's port of it to GTA V Legacy.
The published patches were known as the idea and as a guide to what to look for; no line of their code was read for the build or went into TweaksV.

With `DIAGNOSTIC_MODE=1` the part logs how many decal definitions are registered - 10 seconds in and then every minute while the number changes - and whether the game's own array of 260 would have dropped some, so you can see whether your load needs the larger array at all.
With `DRY_RUN=1` as well, it measures the game as it is, without the mods folder, which a dry run does not serve.

### RageOpenV

Three switches under `[RAGEOPENV]`.
With `MODS_FOLDER=1`, a file that exists under the `mods` folder in the game's current directory, normally the game folder, is served instead of the game's own, by the same path (`mods\update\update.rpf` for `update\update.rpf`); the folder is read once at start-up, so a file added while the game runs is seen at the next start.
A file in the mods folder that cannot be opened - another program holds it, or it was deleted while the game runs - is never replaced by the game's own unnoticed: the game gets the failure, as for any file it cannot open, the log names each such file and a message box the first.
With `OPEN_RPF=1`, RPF archives that are not encrypted - the kind OpenIV writes, marked OPEN - load.
With `SKIP_RPF_CACHE_CHECK=1`, the game's check of its RPF cache is skipped, which modified archives need.
Replaces RageOpenV; OpenIV's `OpenIV.asi` and `OpenRPF.asi` do the same job and must not be installed next to TweaksV.
The experimental newmods folder of the original is not carried over.
With NaturalVision Enhanced's `onigiri.asi` installed, TweaksV switches this part off (see "Other mods").

With `DIAGNOSTIC_MODE=1` the part logs every file the game asks for, every file served from the mods folder and every archive header read, 20,000 lines of each kind at most, with a summary 10 seconds in and then every minute while the numbers change.

With `MODS_FOLDER=1` the part also checks the packs in the mods folder for GTA V Legacy, once at the start and on a thread of its own, so the game never waits for it.
A pack made for Legacy loads in Enhanced, but a vehicle from it stops the game the moment it is created, with no word on screen.
Every `.rpf` file in the mods folder, and every archive nested in it, is read up to its table of contents, where each model and texture dictionary carries the version it was made for: `yft` 162, `ydr` and `ydd` 165 and `ytd` 13 are Legacy's, 171, 159 and 5 Enhanced's.
The log names each pack made for Legacy, with its Legacy models and textures and the first of them (`The pack mods\update\x64\dlcpacks\lspdcvpi08\dlc.rpf was made for GTA V Legacy: 4 yft 162, 12 ytd 13 - the first lspdcvpi08.yft in its nested archive vehicles.rpf.`), and one message box names them all.
The fix is to convert such a pack for Enhanced with CodeWalker - the Legacy vehicles of my test spawned once converted - or to take it out of the mods folder.
A pack the check cannot read, or not in full - damaged, held by another program, with a path too long for Windows to open, past one of the check's limits - is named in the log with the reason and in the box as not checked; a pack that is not OPEN, or holds archives that are not, as the game's own archives are not, is named in the log only, since a pack made with OpenIV or CodeWalker is OPEN.
A folder the check cannot read, or not to its end - a share that drops while it reads, say - is named in the log and in the box too, since the packs in it were not all checked.
The check's last line counts the packs and those folders.
It has no key of its own: it reads the packs and changes nothing.
With `DIAGNOSTIC_MODE=1` it logs a line per pack read, with its models and textures by version.

### StraightToStoryMode

`SKIP_SPLASH_SCREEN`, `SKIP_LEGAL_SCREEN` and `SKIP_LANDING_PAGE` under `[STRAIGHTTOSTORYMODE]` (all on) skip the splash screen, the legal screen and the landing page, so the game goes straight into Story Mode.
Replaces StraightToStoryMode.
With NaturalVision Enhanced's `onigiri.asi` installed, TweaksV switches this part off (see "Other mods").

### DirectStorage

With DirectStorage on, GTA V Enhanced opens the files in its folder at start-up, not just its archives - every file in every subfolder, once its RPF cache check is skipped, which TweaksV's RageOpenV part and every other mods loader do - and keeps them open, letting other programs only read them: while the game runs, mods can't write their own files there and scripts can't be reloaded.

`UNLOCK_FILES=1` under `[DIRECTSTORAGE]` (on by default) lets the game open those files as before, but shared: a file in the game folder that is opened only for reading - by the game, by DirectStorage or by a mods loader such as NaturalVision Enhanced's - can be written, renamed and deleted by other programs while it is open.
The `.rpf` archives and `.cache` files stay locked as before, nothing is refused, so the game and its loaders get every file they ask for, and files outside the game folder, such as your saves, are not touched.
A mod or a script inside the game can also replace such a file by renaming a new file over it, the usual safe way to save settings: Windows refuses that kind of rename over a file somebody holds open, whatever its sharing, so TweaksV makes it a rename with POSIX semantics, which goes through - the game goes on reading the file it opened until it opens it again; on a drive that does not take POSIX semantics, a FAT drive or a network share, the rename is made as the mod asked.
Two limits remain, both Windows' rules for a file somebody holds open: a program that opens a file for writing while letting no one else even read it gets "file in use" while the game holds that file open - one that lets others read while it writes, as most editors and scripts do, and renaming a file are not affected; and a program outside the game that replaces a file by renaming a new file over it gets "access denied" while the game holds that file - such a program has to rename with POSIX semantics, use `ReplaceFile` or write the file in place.
TweaksV's own files are written at the start-up, before the game opens its folder, so they are not affected.
Ten seconds after the first file, the log counts the opens it shared, their files by type, the opens it left as they were and the renames it gave POSIX semantics.

With `DIAGNOSTIC_MODE=1` the part logs the game's calls into DirectStorage, including every file it asks DirectStorage to open - a file outside the game folder by its name only, without its folder - and every file and folder in the game folder that is opened, with its access, the sharing asked for and the sharing given, its result and caller, and every rename it gives POSIX semantics, with its result and caller (see "TweaksV.ini" below).

TweaksV's own part; the idea comes from alloc8or's [DirectStorageFix](https://github.com/alloc8or/DirectStorageFix), and none of its code is used.
With `DirectStorageFix.asi` installed, TweaksV switches its own DirectStorage part off.

### Blips

The game's own blips - the collectibles, the shops, the activities and the rest of the map's markers - are made by one of its scripts, `blip_controller`, which makes them only while fewer than 150 blips are on the map.
The blips a mod adds count too, so a mod that puts many of them on the map pushes the game's own off it: the script waits for room that never comes.
Collectibles on Map and Radar is the case this part was made for.

`ACTIVE_BLIP_LIMIT` under `[BLIPS]` (default 1900, 150 to 1997) raises that threshold in the script each time the game loads it - at the start and again at every save load - so the game's blips stay on the map next to a mod's.
1997 is the most that still holds when the game's blip pool of 2000 is full; 1900 leaves room for the blips of missions and other scripts.
TweaksV changes the script only where it finds the check exactly as it expects: otherwise it leaves the script as it is and says why in a message box, and a threshold another mod has already changed is left as it is and only logged.

With `DIAGNOSTIC_MODE=1` the part logs the two natives the script's check calls, and how many of the blip pool's 2000 slots are in use, 10 seconds in and then every minute while the numbers change.

TweaksV's own part; the design comes from rootBrz's [Blip Limit Fix](https://github.com/rootBrz/gtav-blip-limit-fix), and none of its code was read or used.
With `BlipLimitFix.asi` installed, TweaksV switches its own Blips part off.

### WeaponLimits

**Weapon infos:** `WEAPON_INFO_FILES` under `[WEAPON_LIMITS]` (default 1024, 160 to 16384) is how many `weapons.meta` files the game can load.
It reads each one - add-on weapons bring one each as a rule, packed on their own - into one entry of an array that has room for 160 on 1.0.1158.16, of which the base game alone fills 155, and never checks that room: a file past it is written into memory the array does not own, and the game crashes while loading or later.
TweaksV gives the array the room you set before the game sets it up - 1024 take about 254 KB of the game's memory - and says so in the log: `The weapon info blob limit is raised from 160 to 1024 weapons.meta files.` It guards that room as well (`The weapon info blob limit is guarded: ...`): a `weapons.meta` file past it is not loaded at all, instead of being written past the array's end, the log names the first such file at once, and ten seconds later a message box names it and how many more were left out - raise `WEAPON_INFO_FILES` then and start the game again.
The weapons of a file left out are missing from the game, and the rest of their pack can look wrong: in my test the game stayed playable and did not crash, but a refused pack's weapon spawned without its magazine, its rounds floating under the gun.
Where another mod has raised the limit to your value or more already, TweaksV leaves it as it is (`The weapon info blob limit is 2048 already (WEAPON_INFO_FILES=1024) - nothing to patch.`) and guards that room.

With `DIAGNOSTIC_MODE=1` the log has a line for each `weapons.meta` file the game loads, written before it loads it - so the file that ended a game shows - and, 10 seconds in and then every minute while it changes, how many it has loaded against the room: `N weapons.meta files loaded (the game's own array holds 160, this one 1024)`, the second number the room the array really got, with ` - the game's own array would have overflowed by M of them` once your load passes the game's own 160, and how many files the guard has refused.

TweaksV's own code; the idea comes from alexguirre's [Weapon Limits Adjuster](https://github.com/alexguirre/gtav-WeaponLimitsAdjuster), its MIT half, of which one file was read and none of it used ([CREDITS.md](CREDITS.md)).

**Weapon components** (`WEAPON_COMPONENTS_SIZE`, default 1024): the game keeps the weapon component definitions of all `weaponcomponents.meta` files - each scope, clip, suppressor, flashlight, grip and tint the game's weapons and add-on weapons can take - in one list of 470, and the definitions themselves in a pool of 470 slots; the base game alone uses 465, so weapon packs with more than five definitions between them end the game while it loads.
TweaksV moves the list to one with room for 16,854 and gives the pool `WEAPON_COMPONENTS_SIZE` slots and 470 more - the room, from the game's own 470 up to 16,384, and the most one file brings.
A `gameconfig.xml` that names another size for the pool (`CWeaponComponentInfo`) gets that size in its place, larger or smaller: a pool of more slots would let the game make more definitions than the room and one file, and so than its guard lets in, and one of fewer would end the game short of the room.
Its guard refuses a `weaponcomponents.meta` that comes once the room is used up, before the game makes any of its definitions: a box names the first file refused, the log every one, and their weapon components are missing in the game - raise `WEAPON_COMPONENTS_SIZE` then.
The files come in the order the game mounts them, so a load past the room may refuse a later file of the game's own DLC packs as well as an add-on's.
The pool can only be raised if the game makes it after TweaksV has started: should definitions be loaded while the game has never asked TweaksV for the pool's size, a box says so, since the pool then keeps the game's size and past it the game crashes while it loads, whatever `WEAPON_COMPONENTS_SIZE` says.
The log says `The weapon component list moved and its limit raised from 470 to 1024 entries.` and, once the game makes the pool on its way into story mode, `The pool size the game's configuration gives for the weapon components, 470, is replaced by 1494.` One `weaponcomponents.meta` file still holds at most 470 definitions - the game's own limit, which TweaksV leaves.
With `DIAGNOSTIC_MODE=1` the log says how many definitions your game holds, from how many files, 10 seconds in and then every minute while it changes - `[WeaponLimits] DIAGNOSTIC: 556 weapon component definitions from 40 weaponcomponents.meta files (the game's own list holds 470, this one 1024) - the game's own list would have overflowed by 86.` - and ` - the room is used up` at its end means that the guard refuses every further file: raise `WEAPON_COMPONENTS_SIZE`.
Six of the 17 places in the game's code that use the list lie in its protected code, which could put them back; with `DIAGNOSTIC_MODE=1` TweaksV reads them back at every summary, and a box tells you should one be put back, since the game is then likely to crash.

### PedProps

**Experimental and untested, off by default - use it at your own risk:** this part is in TweaksV, but it has not been tried in the game past 255 props on an anchor - no hat past 255 has been worn yet - so `ENABLED` under `[PEDPROPS]` ships at 0.
Set it to 1 to try it; what is still open is in [NOTES.md](NOTES.md), T-PEDPROPS.

A ped model - above all GTA Online's freemode characters, `mp_m_freemode_01` and `mp_f_freemode_01`, which clothing packs dress - carries its props, the hats, helmets, glasses, earrings and watches, on 13 anchors (anchor 0 is the head), and each pack adds a file of them per model.
The game's checks take the props of one anchor, all files together, as 8 bits: past 255 on an anchor, only the props below (total mod 256) can be put on - with 300 props, the first 44 - the helmet code drops or clamps a head prop past it, and the game takes a worn prop past the 255th for another one.
On 1.0.1158.16 the base game alone holds 255 head props on the male freemode model and 254 on the female, so a single add-on hat or helmet takes the male one past the limit.

The part widens those eleven checks to 16 bits, so that an anchor holds up to 65535 props in all; below 256 they give exactly what the game's own give, so the part changes nothing for a game without clothing packs.
`[PEDPROPS]` has `ENABLED` alone, 0 by default - there is no size to choose.
The log says `The ped props' limit is raised from 255 to 65535 per anchor (11 sites).`; TweaksV changes the game only where it finds all eleven checks, and the calls around them, exactly as it expects, and otherwise leaves the game as it is and says why in a message box - also when another mod has widened one of them already.
A file with more than 255 props on one anchor stays its pack's to split: its own data cannot name them, with or without TweaksV.
In my tests the game crashed at the switch to the male freemode model past 255 head props with a big clothing pack in, at the same place with the part (2.6.0) and without it (2.5.0), while 255 switched fine; whether going past 255 or the big packs' own data causes that crash is not known yet.

With `DIAGNOSTIC_MODE=1` the part logs every check and every change, and 10 seconds in and then every minute while the numbers change - also while it is off, when it looks for nothing else and changes nothing, so you can see whether your packs need it - how many ped models it found and the most props on one anchor - `the most props on one anchor: 212 (mp_m_freemode_01, anchor 0, from 30 files, at most 38 in one)` - and gives every anchor past 255, and every file with more than 255 props on one anchor, a line of its own; an anchor's line says whether its limit is raised this session.
The first summary can come before the game has loaded its peds; the next ones count them.

TweaksV's own part; the idea comes from Zombieguy's [PedProp Limit Adjuster](https://www.gta5-mods.com/scripts/pedprop-limit-adjuster), made for GTA V Legacy, and from FiveM's pull request 2421 by Disquse, which that plugin ports - none of their code was read or used.

### Resources

The game streams the world's textures and buffers into a budget of video memory that its `gameconfig.xml` sets by texture quality - on 1.0.1158.16 1,500 MB at Normal and 3,000 MB at High and Very High, as far as my research tells, whatever your card has - and refuses to place more: what it cannot fit, it drops, and textures go missing even when the card has memory to spare.
`VIDEO_MEMORY_BUDGET` under `[RESOURCES]` raises that budget, in MB: each of the four budgets below it is raised to it as the game's start-up sets them, and none is lowered, so a `gameconfig.xml` that gives more keeps its own.
0, the default, keeps the game's own; the lowest raise is 1,500 - a value below it counts as 1,500 and is reported - and the highest 16,384.
Choose it for your card: the budget is the video memory the game may fill with streamed textures, beside what it needs for everything else - at 3840x2160 with ray tracing that is several GB - so a budget past what the card has left only moves the textures' memory out to system memory, which costs speed, and the game's own check, which evicts against what Windows gives it, steps in anyway.
With 0 the log says `VIDEO_MEMORY_BUDGET=0 - the texture budget is not raised.`; with a raise `The call after the budgets' stores at RVA ... is redirected: the texture budgets below 6000 MB are raised to it when the game's start-up sets them.`, and once the game's start-up has set them `[Resources] The texture budgets of gameconfig.xml (MaxStreamingMemory_0..3_Base: 1500, 1500, 3000 and 3000 MB) are raised to 6000 MB where they are below it: 6000, 6000, 6000 and 6000 MB now; the texture quality's index 2 uses 6000 MB.` - the four the game had, the four now, and the one your texture quality uses.
A budget the game's own code writes besides its start-up - after a game update, say - is not raised, nor one whose call after the stores another mod has hooked already; the start-up summary says so.
The check reads the game's own code alone: a mod that writes the budgets from its own code is not seen.
It would rather count a load whose bytes read as a store than miss a writer, so the box says "possible writer".
TweaksV has to start before the game's start-up sets the budgets, as it does with Script Hook V's loader: if the game has set them already, nothing is raised, and the start-up summary says that too.
With `DIAGNOSTIC_MODE=1` the log also says, 10 seconds in and again every minute while the numbers change, how much of the budget the streamed textures and buffers take (`2845 MB of the 6000 MB budget`), the four budgets of `gameconfig.xml` and the raised ones, how many of the 98,304 objects the game allows are in use, the most so far, and whether the game's own check is dropping textures ahead of time (`the update check's flag is up`); ` - at the budget` at its end means the game is dropping what it cannot place right now.
The same budgets can be raised by data instead: `MaxStreamingMemory_2_Base` and `_3_Base` in `ConfigGraphics` of a `gameconfig.xml` in your mods folder.

### RayTracing

With ray tracing on, the game skins every moving object it ray traces - cars, peds, trees, cables, ropes - into one buffer each frame, before the ray tracing builds what the shadows, reflections and occlusion trace against.
The buffer holds 4,194,304 vertices, 80 MB of video memory; each frame hands out its room in turn, and an object whose vertices no longer fit is left out of that frame's ray tracing: its shadow, reflection and occlusion are missing, and since the objects come in another order every frame, they come and go.
With many high-poly vehicles and peds - modded police cars, add-on vehicles, a crowd - that is the shadow under a car that flickers or goes missing.

`DYNAMIC_BVH_VERTICES` under `[RAYTRACING]` (default 0, the game's own size) raises the buffer and the game's two checks of it together, to 4,194,304 to 26,843,545 vertices; a value below the game's own is raised to it and reported.
The buffer is video memory, made once at the game's start and kept, 20 bytes a vertex whether used or not: 8,388,608, twice the game's own, costs 80 MB more, 16,777,216 costs 240 MB more.
TweaksV's own part, written from TweaksV's own findings about the game; the idea comes from Koukol's Resource Extensions [Enhanced], whose plugin was not read - while `ResourceExtensions.asi` is installed, this part is off (see "Other mods").

With `DIAGNOSTIC_MODE=1` the part logs how much of the buffer the frames take - 10 seconds in, then every minute while the most grows - and counts every object the full buffer leaves out: `geometries refused` in its line means the buffer is too small for your load, and none refused while shadows still flicker means another cause.
It measures with the key at 0 too, so you can see whether your load needs a larger buffer at all.

### SirenSettings

Every siren setting of `carcols.meta` - the lights and pattern of an emergency vehicle's sirens - has an ID that the vehicles name it by, and the game keeps both in one byte: it has 256 IDs and lets vehicles use 255 settings, and it drops or mixes up the settings of packs whose IDs clash, without a word.
An ID above 255 in a pack's file is cut to its last byte - 300 becomes 44 - so packs that use large IDs clash with each other and with the game's own, and a pack meets the limit long before anyone has 255 settings.

`LARGE_SIREN_IDS` under `[SIRENSETTINGS]` (default 1, or 0) makes the IDs 16 bits on the whole way from the file to the vehicle: a pack may use 0 to 65535 but 255, which stays the game's "none" - and keep 0 and 65535 free too, as the plugin that had the idea first asks.
A pack's setting whose ID is taken is still dropped, and a clash within one file still moves the later setting; only now the IDs that clash are the IDs the packs chose.
Packs that relied on the cut - a setting named 372 whose vehicles say 116 - lose their link, as they did with that plugin.
The 255 settings stay the most: the position in the game's array stays a byte.
There is no size to set: the table of IDs always takes 64 KB.

TweaksV's own part, written from TweaksV's own findings about the game; the idea comes from cp702's SirenSetting Limit Adjuster, a plugin for GTA V Legacy, of which no code was read.

With `DIAGNOSTIC_MODE=1` the log says, 10 seconds in and then every minute while the numbers change, how many siren settings the game and your packs loaded, how many of the IDs are in use and how many of them past 255, how many settings lie past position 255, which no vehicle can reach, how many IDs two or more settings were given, and the IDs themselves in their settings' order: `[SirenSettings] DIAGNOSTIC: siren settings loaded: 31 (vehicles can use 255 at most); siren setting IDs in use: 31 of 65536, 4 of them past 255; ...`.
Per `carcols` file it also says what the game did with its settings: `a pack's carcols, dlcpacks:/mypack/data/carcols.meta, brought 6 siren settings: 4 added, 2 dropped, their IDs taken (5, 300), 0 moved.` - a dropped setting's vehicles get the setting that had the ID first, so give it another.
With `LARGE_SIREN_IDS=0` or the part off the same summary goes on over the game's own 256 IDs, nothing changed in the game, so that you can compare a start with it and one without.

Open until my first boot: the game's binary `.ymt` files go through another loader than the text files, and whether it copes with the wider ID shows only by comparing the base game's settings and IDs with the key on and off.

### StaticBounds

The game keeps the static collision bounds of the map - every building, prop and interior it places, a heavy map mod's MLOs, custom interiors and expansions among them - in one pool, which it makes once from `gameconfig.xml`'s size, or from its own default where the file names no size for it.
Once the pool is full, the game ends in an access violation: the crash heavy map mods are known for.

`STATICBOUNDS_SIZE` under `[STATICBOUNDS]` (default 16000, 1 to 100000) is the least the pool gets: TweaksV hooks the two functions the game makes its pools through, tells the pool by its name and the size of its slots, and raises its size before the game allocates it, so a `gameconfig.xml` that gives the pool more keeps its size.
The log says at the start that the part is in place, `The pools' set-up at RVA ... and the pools' allocator at RVA ... are hooked: ...`, and what the pool got once the game has made it: `The size the game gives the static bounds pool, <size>, is raised to 16000 (STATICBOUNDS_SIZE).`, or `... is kept: it is not below STATICBOUNDS_SIZE=16000.` when the game gives more.
Where TweaksV cannot find or hook the two functions, it changes nothing and says why in the start-up message (`The static bounds pool's size is not raised this session: ...`).
The size is raised only if the game makes the pool after TweaksV has started, so TweaksV checks that it did: if the game has not made it a minute after your session starts - it made the pool before TweaksV started, or makes it another way - a message box says so, and the pool has the game's size.
TweaksV sees the session start through its hook on the game's script programs, the Blips part's - or the StaticBounds part's own while `[BLIPS]` is off in `TweaksV.ini`; while `BlipLimitFix.asi` or Resource Extensions, which do the Blips part's job, is installed, whatever `[BLIPS]` says, no check is made, and the log says so.
Where the check cannot be made otherwise - that hook is not there after a game update, say - the start-up message says so (`The check that the game makes the static bounds pool is not done this session: ...`): the hooks stay, but nothing then tells you whether the game made the pool.
With `DIAGNOSTIC_MODE=1` the log names the two functions at their RVAs, and when the session started and the game made the pool.

Rewritten from Nochala's [Static Bounds Limit Adjuster](https://www.gta5-mods.com/scripts/static-bound-limit-adjuster) v1.0.2, GPL-3.0-or-later: its two patterns, its two hooks and its rule, on TweaksV's own code.
With `StaticBoundLimitAdjuster.asi` (or `StaticBoundsLimitAdjuster.asi`) installed, TweaksV switches its own StaticBounds part off.
### Measured first: the sirens per vehicle, the lighting capacity, the model budgets, the ped pool and the render queues

More limits of the game may be lifted by parts to come ([ROADMAP.md](ROADMAP.md)): the sirens per vehicle by the SirenSettings part, and the lighting capacity, the model budgets, the ped pool and the render queues if their numbers ask.
A part goes in only once a mod load reaches its limit, so these limits are measured first: with `DIAGNOSTIC_MODE=1`, TweaksV finds the places in the game's code each limit lies in, checks them as the part will, and logs how close the game and your mods come to it - 10 seconds in and then every minute while the numbers change.
These measurements change two things in the game, each said below: the call that draws a vehicle's sirens and the jump a full list of lights takes, which pass through TweaksV; without `DIAGNOSTIC_MODE`, nothing of them runs.
What its lines tell you:

**Siren count** (SirenSettings): the game lights 20 sirens of a vehicle, `siren1` to `siren20`.
An emergency vehicle made for 32 sirens, as some packs ship them, loads all the same, and its lights past the 20th stay dark without an error; and a siren whose bone lies past the model's 128th bone stays dark too, because the game cannot name it.
With `DIAGNOSTIC_MODE=1` TweaksV looks once at each vehicle model whose sirens the game draws, and 10 seconds after the first logs how many such models it has seen, the most sirens the game names on one of them (20 at most), how many carry siren bones past `siren20` that the game never lights and how many lose siren bones past bone 127 - with a line for each model concerned, named by the hash of its name, saying how many of its siren and glass bones are lost.
The game draws sirens only while they are on: drive an emergency vehicle with its sirens on, or let the police chase you.
This measurement changes something in the game: the call that draws a vehicle's sirens passes through TweaksV, which has the game draw them unchanged first; with `DRY_RUN=1` the call stays as it is and nothing is counted.

**Lighting capacity** (LightCapacity): a frame's lights pass through layers of fixed room, and many vehicles with real siren lights are where a load may reach them.
Past some of them a light simply goes dark; past the frame's store of 400 vehicle lights the game writes over its own memory and the crash is next.
With `DIAGNOSTIC_MODE=1` TweaksV reads how full each layer is about a hundred times a second and logs the most, 10 seconds in and then every minute while it changes, of that minute and of the session: the vehicles, the vehicle lights and coronas, the scene's lights, the ray traced list and how often it passes what its upload takes, the reflection lists, the coronas, and how many lights a job's full list refused; and what your siren settings ask for.
It changes one thing in the game, with `DIAGNOSTIC_MODE=1` only: the jump a job's list takes when it is full passes through TweaksV to be counted, and costs nothing while the list has room; with `DRY_RUN=1` it stays as it is and those lights are not counted.

**The model budgets** (Population): the game holds only so many bytes of vehicle models and of ped models - with the Population Variety slider at its top, 429 MB and 561 MB - and past that it asks for no further model, so that traffic and crowds repeat fewer of them than your mods could give, without an error.
`vehicle model budget:` and `ped model budget:` say how many MB are in use and how many models that holds, and `at the stop in 170 of 171 samples` says how long the game stood at its budget: most of the time means the budget binds on your load, and a raise would give you more variety.
Their `tables:` lines name the values the budgets come from, and are the one proof that a `ConfigPopulationStreaming` block of your own `gameconfig.xml` has taken, since the game ignores a misspelled tag without a word.

**Ped pool** (PedPool): the game keeps its peds in a pool of 256 slots, and a full pool only means that no more peds appear; `Peds` raised above 256 in a `gameconfig.xml`, though, boots well and corrupts memory once more than 256 peds are alive, as three arrays of the game stay at 256.
With `DIAGNOSTIC_MODE=1` TweaksV logs how many of the slots are in use, the most so far, how many of them are mission peds or wait for reuse, the population's counts against their caps and the most in the spatial array the game finds nearby peds with - and says so when the pool has been full or that array has passed 256.
This one writes nothing to the game.

**Render queues** (RenderQueues): what streamed peds, their props and vehicles with mods draw in a frame the game keeps in small per-frame arrays, and past 120 entries of the first two it writes into memory that is not theirs, with no error, which can crash the game much later and elsewhere.
With `DIAGNOSTIC_MODE=1` TweaksV reads these arrays, the pools behind them and the draw list's references, writing nothing, and 10 seconds in, then every minute while a most changes, logs how near each has come; `FULL` and `PAST ITS CAPACITY` are the words to look for.
Plain driving may never touch them: many multiplayer-style peds in view, and mods set on many cars, do.

## TweaksV.ini

TweaksV writes `TweaksV.ini` in the game folder, next to `GTA5_Enhanced.exe`, on the first start - or in `%LOCALAPPDATA%\TweaksV` if the game folder cannot be written to - with every setting at its default, and adds back any setting that goes missing.
`1` is on, `0` is off (`true` and `false` work too).
The file carries a short header and, after each key's value, a short comment from `//` to the end of the line; this is what each key does in full.
A comment after a value - after `//` or `;` - is never part of it, so you can write your own there.
A `TweaksV.ini` from an earlier 2.x version is kept as it is, without the comments: delete it to get the commented one, then set your values again.
One from an earlier version gets the keys it lacks added - those of `[BLIPS]` to a file from before the Blips part, those of `[WEAPON_LIMITS]`, `[PEDPROPS]`, `[RESOURCES]` and `[RAYTRACING]` to one from before 2.6.0, those of `[SIRENSETTINGS]` and `[STATICBOUNDS]` to one from before 2.7.0 - and its `VERSION` raised to 5; its values stay as they are.
The settings are read once, at start-up: TweaksV's parts patch the game before it runs, so a live panel would change nothing - there is none, by design; a changed file takes effect at the next start.

| Section | Key | Default | What it does |
|---|---|---|---|
| `[TWEAKSV]` | `ENABLED` | 1 | the master switch: 0 means TweaksV changes nothing |
| | `LOG` | 1 | writes `TweaksV.log`, in the game folder or in `%LOCALAPPDATA%\TweaksV` when the game folder cannot take it |
| | `DIAGNOSTIC_MODE` | 0 | testing: every part that is on logs every pattern, site and write, and runs its measurement - see "Diagnostics" below |
| | `DRY_RUN` | 0 | testing: everything is looked for and checked - but `blip_controller`'s guard, which the Blips part sees only through its hook; nothing in the game is written or hooked (only RageOpenV's one-shot hook on `GetSystemTimeAsFileTime`, which runs its checks and switches itself off); RageOpenV's hooks are off too, so the mods folder is not served - measuring with that part off alone means its own `ENABLED=0`, not `DRY_RUN` |
| | `CONSOLE` | 0 | a console window that mirrors the log |
| | `ERROR_POPUP` | 1 | a message box when something could not be applied, and one for a part that stops working later and for what the check of the packs or a guard of WeaponLimits found (at most three of each per session) |
| | `CONFLICT_CHECK` | 1 | looks for the old plugins TweaksV replaces and for NaturalVision Enhanced's loader and Resource Extensions, in the game folder and its `scripts` and `plugins` subfolders, and switches the matching part off while they are there (see "Other mods") |
| | `VERSION` | 5 | the layout of this file; TweaksV keeps it, and raises it when it adds the keys of a newer layout - leave the line in |
| `[HEAP_ADJUSTER]` | `ENABLED` | 1 | the part on or off - every part's section has this key, 1 by default but under `[PEDPROPS]` |
| | `HEAP_SIZE` | 2000 | the game's heap in MB, 1 to 2047; a value below the game's own is raised to it and reported |
| | `POOL_MEMORY` | 256 | the memory budget of the game's pools in MB, 136 to 2047 |
| `[MODKIT_LIMIT]` | `MODKITARRAY_SIZE` | 65536 | the number of vehicle modkit IDs, 1 to 65536; a value below the game's own 1024 is raised to it and reported |
| `[PACKFILE_LIMIT]` | `PACKFILEARRAY_SIZE` | 13000 | the number of RPF archives, 1 to 65535; a value below the game's own 7260 is raised to it and reported; it also replaces the archive count of `gameconfig.xml`, exactly |
| `[BOXSTREAMER]` | `TRAVERSAL_LIST_SIZE` | 10000 | map boxes one search of the box streamer can return, 1000 to 100000 (the game's own: 1000) |
| `[DECALS]` | `DECALDEFS_SIZE` | 2048 | decal definitions the game can hold, 260 to 16384 (the game's own: 260) |
| `[RAGEOPENV]` | `MODS_FOLDER` | 1 | replacement files from the `mods` folder in the game's current directory, normally the game folder |
| | `OPEN_RPF` | 1 | unencrypted ("OPEN") RPF archives can be loaded |
| | `SKIP_RPF_CACHE_CHECK` | 1 | skips the game's RPF cache check (needed for modified archives; 0 only for testing) |
| `[STRAIGHTTOSTORYMODE]` | `SKIP_SPLASH_SCREEN` | 1 | skips the splash screen |
| | `SKIP_LEGAL_SCREEN` | 1 | skips the legal screen |
| | `SKIP_LANDING_PAGE` | 1 | goes straight into Story Mode |
| `[DIRECTSTORAGE]` | `UNLOCK_FILES` | 1 | the files in the game folder other than `.rpf` and `.cache` stay free while the game holds them open: other programs can write, rename and delete them, and mods inside the game can also replace them by renaming a new file over them - not a program that writes while letting no one else read, and not a program outside the game that replaces a file that way |
| `[BLIPS]` | `ACTIVE_BLIP_LIMIT` | 1900 | the game's own blips are made while fewer blips than this are on the map, 150 to 1997 (the game's own: 150) |
| `[WEAPON_LIMITS]` | `WEAPON_INFO_FILES` | 1024 | the `weapons.meta` files the game can load, 160 to 16384 (the game's own: 160) |
| | `WEAPON_COMPONENTS_SIZE` | 1024 | the weapon component definitions of all `weaponcomponents.meta` files together, 470 to 16384 (the game's own: 470) |
| `[PEDPROPS]` | `ENABLED` | 0 | experimental and untested, at your own risk: the props per anchor of a ped model: 255 in the game's own checks, 65535 with the part on |
| `[RESOURCES]` | `VIDEO_MEMORY_BUDGET` | 0 | the video memory in MB the game may stream textures and buffers into; 0 keeps the game's own budgets, 1500 to 16384 raises each budget below it to it (a value below 1500 counts as 1500) |
| `[RAYTRACING]` | `DYNAMIC_BVH_VERTICES` | 0 | the ray tracing buffer for moving objects in vertices, 4194304 to 26843545; 0 keeps the game's own 4194304, a value below it is raised to it and reported; 20 bytes of video memory a vertex, used or not |
| `[SIRENSETTINGS]` | `LARGE_SIREN_IDS` | 1 | siren setting IDs up to 65535, on the whole way from the file to the vehicle; 0 keeps the game's 8 bits |
| `[STATICBOUNDS]` | `STATICBOUNDS_SIZE` | 16000 | the game's static collision bounds, 1 to 100000; a larger number in `gameconfig.xml` is kept |

**Coming from TweaksV 1.x:** the 1.x file has another layout and is not migrated.
On the first start, TweaksV renames it to `TweaksV.old.ini`, writes a fresh `TweaksV.ini` with the defaults and says so in the start-up message - set your values again, the old file still holds them.
You can also delete the old file before the first start.
The same happens to any `TweaksV.ini` without `VERSION` under `[TWEAKSV]`, so leave that line in.
What changed: `[HEAP_SETTINGS]` is `[HEAP_ADJUSTER]`, `[MODKIT_SETTINGS]` is `[MODKIT_LIMIT]`, `[PACKFILE_SETTINGS]` is `[PACKFILE_LIMIT]` and `[DECALDEFS_SETTINGS]` is `[DECALS]`; the other four sections lost their `_SETTINGS` ending (`[BOXSTREAMER]`, `[RAGEOPENV]`, `[STRAIGHTTOSTORYMODE]`, `[DIRECTSTORAGE]`), and `[TWEAKSV]` kept its name; `LOG_TO_FILE`, `SHOW_ERROR_POPUPS`, `CHECK_FOR_CONFLICTS` and `INI_VERSION` are `LOG`, `ERROR_POPUP`, `CONFLICT_CHECK` and `VERSION`; the testing switches `LOG_SEARCHES`, `LOG_DECALDEFS`, `LOG_INFO`, `LOG_MODS`, `LOG_CALLS` and `LOG_FILE_OPENS` are all `DIAGNOSTIC_MODE` now; `NEWMODS` and `LOG_DEVICE` are gone with the newmods folder.

**Diagnostics:** `DIAGNOSTIC_MODE=1` makes every part that is on log every pattern it looks for (address, number of matches, bytes), every site it finds and every write, and run its measurement: HeapAdjuster how much of its budget the game's pools take and the least room so far, the box streamer counts its searches, the decals part reads how many definitions are registered, RageOpenV logs every file request, every file served from `mods` and every archive header read, and its check a line per pack read, with its models and textures by version, DirectStorage logs the game's calls into it, every file opened in the game folder and every rename it gives POSIX semantics, the Blips part the natives its script's check calls and how full the game's blip pool is, WeaponLimits every `weapons.meta` file the game loads and how many files and weapon component definitions your game holds against their room, PedProps the most props on one anchor of a ped, Resources how much of the texture budget the streamed textures and buffers take, RayTracing how much of its buffer the frames take and what the full buffer leaves out, SirenSettings its siren settings and IDs, and which settings the game dropped or moved, per file, StaticBounds when the session started and the game made the pool, and, measured first, how close the game and your packs come to the sirens per vehicle, the lighting capacity, the model budgets, the ped pool and the render queues (see "Measured first" above).
A part with `ENABLED=0` does nothing, so it logs nothing - but for PedProps' measurement, which goes on while that part is off and writes nothing.
Two ways to use it: with `DRY_RUN=1` nothing in the game is written or hooked (only RageOpenV's one-shot hook on `GetSystemTimeAsFileTime`, which runs its checks and switches itself off) - the quiet check after a game update, which shows the patterns, the sites and the decal count of the unpatched game - `blip_controller`'s guard only a boot without `DRY_RUN` shows; alone, it is the full measurement of the running game with the mods folder served, and it hooks a little more than a normal boot (the box streamer's four search calls, the siren lights' renderer call, the jump a job's full list of lights takes, RayTracing's two refusal jumps, DirectStorage's configuration calls, its factory and `OpenFile`, and - with `UNLOCK_FILES=0` - the file opens).
Every kind of line that comes per event stops after 20,000 - also DirectStorage's `OpenFile` lines - and the events are then only counted, with a summary every minute where the part has one.

## Design

- **GTA V Enhanced only:** TweaksV does nothing in Legacy.
- **Generated files:** `TweaksV.ini` and `TweaksV.log` are created on the first boot.
  Every part can be switched off or on.
  - They go in the game folder, or in `%LOCALAPPDATA%\TweaksV` if the game folder is read-only.
  - The files in the game folder have priority.
- **Own code:** every part is TweaksV's own, in the `TweaksV*` files and `dllmain.cpp`; the parts carry none of the original plugins' code but StaticBounds, whose patterns, hooks and rule are adapted from the Static Bounds Limit Adjuster's source under its GPL - what remains of the others is the console opener and the scanner's pattern parser, all named in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
  What TweaksV knows about the game - the byte patterns and what each site holds - is in [PATTERNS.md](PATTERNS.md), and that is the one place to work when the game changes.
- **Tables:** each setting is one entry in `TweaksVSettings.h`, each byte pattern one in `TweaksVPatterns.h`, each part one in `TweaksVFeatures.cpp`.
  A new part is an entry in these tables plus its own file.
- **Shared code:** patches to the game's code go through `TweaksVPatch` - one writer for bytes, one for redirected calls, the hooks on a function's start through MinHook - which writes all or nothing and gives every page its own protection back; `TweaksVScanner` finds the patterns in the running game's code; the six parts that move one of the game's arrays describe it in a table and share the relocation core, `TweaksVRelocation`, which finds and checks every site before anything is written and then writes all of it in one go; `TweaksVScripts` holds the one hook where the game places its script programs, for every part that works on one - the Blips part, the StaticBounds part's check, and the test builds' `TEST_DUMP_SCRIPT`.
- **The whole design:** [SPECIFICATIONS.md](SPECIFICATIONS.md) describes every module, format and log line as built, [NOTES.md](NOTES.md) what each part does and measures, [PATTERNS.md](PATTERNS.md) every game fact - with this file, the whole truth about the code: enough to fork it or build on it without reading the source first.

## Building

You need Visual Studio 2026 with the "Desktop development with C++" workload.
Everything else, including MinHook, is in this repo.

Open `source\TweaksV.slnx` in Visual Studio, choose **Release** or **Debug**, and build.
Or use x64 Native Tools Command Prompt for VS:

```
git clone https://github.com/dachdecker369/engine-tweaks-and-limits-adjuster
cd engine-tweaks-and-limits-adjuster
msbuild source\TweaksV.slnx -p:Configuration=Release -p:Platform=x64
```

- **Output:** `x64\Release\TweaksV.asi` and `TweaksV.pdb`.
  Debug builds go to `x64\Debug` instead.
- **Intermediates:** `object\Release` and `object\Debug`.
- **No Redistributables:** C runtime is statically linked so players don't need it.
- **Commits:** When the folder is a git repo, `TweaksV.log` lists the commit the build came from; otherwise it lists "unknown".

Developer checks - not for release:

- **Tests:** `bash test/run_all.sh` in Git Bash, in the repository folder.
  It builds the Release and test builds of TweaksV and small fake-game programs, runs every test suite against them side by side, each in a folder of its own, and deletes what it made in `object\` (`--keep` leaves it, `--no-build` skips building TweaksV, `--serial` runs the suites one after another, and a suite's name, as in `bash test/run_all.sh blips`, runs that suite alone).
  Building Release replaces `x64\Release\TweaksV.asi`.

Add these switches to the `msbuild` line above:

- **Code Analysis:** `-p:TweaksVAnalyze=true` runs the MSVC code analysis (`/analyze`).
  Builds into `object\Analyze`.
- **Test Builds:** `-p:TweaksVTestBuild=true` gives `object\Test\Release\TweaksV.asi`.
  Add `TEST_BREAK_PATTERN=<label>` under `[TWEAKSV]` in `TweaksV.ini`, with the label of a pattern in [PATTERNS.md](PATTERNS.md), such as `S-E2`.
  That pattern then counts as missing, and you can watch its part be skipped cleanly.
  `TEST_TWICE_PATTERN=<label>` makes it count as held once more than the game code holds it, and you can watch its part refuse it.
  `TEST_LOG_LIMIT_KB=<KB>` lowers the 64 MB limit of `TweaksV.log`, so you can watch it being reached.
  `TEST_DUMP_CODE=<seconds>` (1 to 3600) saves the game's code as it is in memory - decrypted, unlike `GTA5_Enhanced.exe` on disk - for looking for patterns: `GameCode-start.bin` before TweaksV patches anything, `GameCode-later.bin` that many seconds later, about 50 MB each, in `%LOCALAPPDATA%\TweaksV`.
  They hold the game's code, constants and function table, never its data; the format is in `source\TweaksVDump.h`.
  Delete them when you are done.
  `TEST_DUMP_DATA=<seconds>,<rva>:<bytes>[,<rva>:<bytes>...]` saves named windows of the game's `.data`, which `TEST_DUMP_CODE` leaves out, for reading static data that lies there: at most 16 windows of at most 64 KB each, the RVAs and the sizes in hex, each wholly inside `.data`, as `GameData-<rva>-start.bin` before TweaksV patches anything and `GameData-<rva>-later.bin` that many seconds later, in `%LOCALAPPDATA%\TweaksV`.
  Anything else is a problem, and nothing is saved; the format is in `source\TweaksVDump.h`, and they go when the others go.
  `TEST_DUMP_SCRIPT=<name>` saves one of the game's script programs - `blip_controller`, say: 1 to 63 letters, digits and `_` - as the game places it in memory, for reading a script's code, which the game's archives store encrypted: each time the game places it, at most 16 times a session, as `Script-<name>-<n>.bin` in `%LOCALAPPDATA%\TweaksV`, with its code, its native table before and after the game resolves it, and its strings; the format is in `source\TweaksVDump.h` too.
  The log header will list `TEST BUILD`.

## Testing

1. Use GTA V Enhanced in Story Mode with BattlEye disabled.
   Never go online with mods.
2. Remove the old plugins TweaksV replaces from the game folder and its `scripts` and `plugins` folders: `HeapAdjuster.asi` (or `ZHeapAdjuster.asi`, the name its author suggests), `ModkitLimitAdjusterEnhanced.asi`, `PackfileLimitAdjusterEnhanced.asi`, `RageOpenV.asi`, `OpenRPF.asi`, `OpenIV.asi`, `StraightToStoryMode.asi`, alloc8or's `DirectStorageFix.asi`, `DecalPatch.asi`, rootBrz's `BlipLimitFix.asi` and Nochala's `StaticBoundLimitAdjuster.asi` (also named `StaticBoundsLimitAdjuster.asi`) - the thirteen old plugins the conflict check knows.
   While one of them is there, TweaksV switches its matching part off and says so in the start-up message.
   Two more files it knows stay: NaturalVision Enhanced's loader, `onigiri.asi`, which NVE needs - while it is there, TweaksV's RageOpenV and StraightToStoryMode parts are off - and Koukol's Resource Extensions, `ResourceExtensions.asi` - while it is there, the HeapAdjuster, PackfileLimit, Blips and RayTracing parts are off (see "Other mods").
3. `TweaksV.asi` goes in the game folder next to `GTA5_Enhanced.exe`.
4. Add the ASI loader file `xinput1_4.dll` from the [Script Hook V](http://www.dev-c.com/gtav/scripthookv/) download.
   It is the only file needed; Script Hook V itself is not.
5. Coming from TweaksV 1.x: your old `TweaksV.ini` is renamed to `TweaksV.old.ini` on the first start and a fresh one is written - see "TweaksV.ini" above.
   You can also delete the old file first.

## Other mods

- **NaturalVision Enhanced** sets the heap size as well.
  TweaksV leaves a heap another mod has already set alone and says so in the log, so with NVE installed its heap size wins, whatever the load order.
  NVE also brings its own loader, `onigiri.asi`, next to `GTA5_Enhanced.exe`: a mods loader that serves the files and archives in its `onigiri` folder, and that skips the start-up screens its INI, `onigiri.ini`, names.
  It patches the same places in the game as TweaksV's RageOpenV and StraightToStoryMode parts, after TweaksV, and crashes the game when TweaksV has changed them first.
  So while `onigiri.asi` is installed, TweaksV switches those two parts off and says so in the start-up message; the other thirteen parts work as usual (with `CONFLICT_CHECK=0` TweaksV does not look for it, and the game crashes while those two parts are on).
  Your mods then go where onigiri serves them, in its `onigiri` folder, not in TweaksV's `mods` folder, and which start-up screens are skipped is onigiri's business, set in its `onigiri.ini` under `[PATCHES]`: with NVE's defaults the game went straight into Story Mode on my boots.
  To stop the message, set `ENABLED=0` under `[RAGEOPENV]` and under `[STRAIGHTTOSTORYMODE]` in `TweaksV.ini`.
- **Static Bounds Limit Adjuster** by Nochala, `StaticBoundLimitAdjuster.asi` (its repository names it `StaticBoundsLimitAdjuster.asi`), is detected as an old plugin: TweaksV switches its StaticBounds part off while it is installed, so the pool's size is not changed twice.
  Delete it - TweaksV replaces it.
  `STATICBOUNDS_SIZE` is the original's own key, with its default and its maximum: set the value you used in its `StaticBoundsLimitAdjuster.ini` under `[STATICBOUNDS]` in `TweaksV.ini`.
  The original reads 0 as its default, 16000; TweaksV takes 1 to 100000, so a 0 carried over is reported in the start-up message and used as 1 - which leaves the game's own size, since the part gives the pool the larger of the two: set 16000 for the original's default.
- **Resource Extensions [Enhanced]** by Koukol, a closed-source plugin that raises many of the game's capacities - the archive count, the story blips' threshold and extra room for the ray tracing of moving geometry among them - asks to remove the heap and packfile adjusters.
  TweaksV leaves those jobs to it: while `ResourceExtensions.asi` is installed, TweaksV's HeapAdjuster, PackfileLimit, Blips and RayTracing parts are off, and the start-up message says so, with the `ENABLED=0` that stops it; the other parts work as usual.
- **gameconfig.xml:** a gameconfig that raises the archive count has no effect while the Packfile part is on - TweaksV replaces that count with `PACKFILEARRAY_SIZE` when the game reads its configuration, exactly, also when the gameconfig's count is higher.
  Its pool size for the weapon components (`CWeaponComponentInfo`) is replaced by the WeaponLimits part's, and with `VIDEO_MEMORY_BUDGET` set its texture budgets below it are raised to it (see the parts above).
  Its static bounds pool's size is raised to `STATICBOUNDS_SIZE` where it is lower.
  Everything else in a gameconfig works as usual.
- **Zolika Patch:** the ModkitLimit Extender's author warned that its modkit patch may not be compatible with Zolika Patch's; the same goes for TweaksV's Modkit part.
- **DecalPatch.asi**, a closed-source plugin that raises the box streamer list and the decal definitions limit for GTA V Enhanced, is detected as an old plugin: TweaksV switches its decal definitions part and its box streamer part off while it is installed, so nothing is patched twice.
  Delete it - TweaksV replaces both halves.
- **Mods that hook the game's file functions:** TweaksV's RageOpenV part hooks on top of another mod's hook on GetFileTime or GetFileSize and chains through it.
  A hook on OpenBulk or GetAttributes changes the bytes TweaksV finds those two functions by: the mods folder is then not used, and the start-up message names the pattern that was not found.
- **Collectibles on Map and Radar** puts so many markers on the map that the blips in use pass the threshold of the game's own script: 243 of them on my boot of 2.3.0, against its 150.
  With the Blips part on, the game's own blips stay next to the mod's markers, and Script Hook V, which the mod needs, runs next to TweaksV without a conflict (my boots of 2026-10-28).
- **Script Hook V, ScriptHookVDotNet, OpenIV:** neither needed nor touched.
  Only the ASI loader file from the Script Hook V download is used, and OpenIV's `OpenIV.asi` and `OpenRPF.asi` must not be installed next to TweaksV (their job is TweaksV's RageOpenV part).

## Reporting a problem

Attach `TweaksV.log` - its first lines name the game build and the files in use - and `TweaksV.ini`; say which store the game comes from (Rockstar, Steam, Epic); list the other `.asi` and `.dll` files in the game folder; and describe what happened against what you expected.
A crash dump helps if you have one.
Security problems go the private way described below.

## Security

Please report a security problem privately, not in a public issue: [SECURITY.md](SECURITY.md) says how.

## Credits

Five of TweaksV's parts are rewritten from the five plugins by Chiheb-Bacha and one, StaticBounds, from Nochala's Static Bounds Limit Adjuster; they build on the research of everyone listed in [CREDITS.md](CREDITS.md), which also credits the ideas behind TweaksV's own nine parts: BoxStreamer, Decals, DirectStorage, Blips, WeaponLimits, PedProps, Resources, RayTracing and SirenSettings.
All credit for the original research and code goes to them.

Third-party notices are in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Engine Tweaks and Limits Adjuster is GPL-3.0-only.

## License

Copyright (C) dachdecker369.
Engine Tweaks and Limits Adjuster is free software under the GNU General Public License, version 3 only; the text is in [LICENSE](LICENSE).
