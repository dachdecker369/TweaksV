# Roadmap

What I plan for TweaksV next, before it is planned in [PLAN.md](PLAN.md) and specified in [SPECIFICATIONS.md](SPECIFICATIONS.md); what has shipped is in NOTES.md's Versions.
It holds phase C's second build now, 2.8.0: the sirens per vehicle (PLAN.md, D23) and the raises 2.7.0's measurements of the population's limits and the lighting capacity ask for.
A new part is a part like the fifteen (SPECIFICATIONS.md, section 16): its own file on the shared code - the scanner, the patch module, the relocation core, the three tables - its own section of `TweaksV.ini`, its old plugin in the conflict check, its measurement under `DIAGNOSTIC_MODE`, its suite, its patterns in PATTERNS.md.
One minor version per part, a patch version for its fixes, but parts built in one round ship together as one version, as 2.6.0's did (PLAN.md, D18); a version takes the next free number when it ships.

How a part is taken over depends on the original's license, as [CREDITS.md](CREDITS.md) says of the fifteen.
Code under a license the GPL-3.0 takes in - MIT, BSD, zlib, the GPL-3.0 itself - may be read and adapted: the patterns and the research are credited in CREDITS.md, every line carried over is named in THIRD_PARTY_NOTICES.md, and the part is still built on TweaksV's shared code, as 2.0.0 rebuilt the five plugins, because a plugin merged as it is brings its own scanner, INI and log, what 1.x had.
Code without a license, under all rights reserved or under FiveM's terms is idea only: none of it is read, and every game fact behind the part is found anew in the running Enhanced game with my tooling (a test build's dump, Ghidra; PATTERNS.md, "When Game Updates").
The original left is a Legacy plugin, and no pattern of a Legacy plugin carries over to Enhanced anyway (PATTERNS.md).
So every part starts with a research pass in the dump that settles whether the limit is in Enhanced 1.0.1158.16 at all, where it lies, and whether it still binds - Enhanced may have raised it - and the part goes in only when a mod load hits it; a measurement of the limit under `DIAGNOSTIC_MODE`, as the box streamer and the decals have, comes first and says so.

## 2.8.0: the sirens per vehicle

The original: [SirenSetting Limit Adjuster](https://www.gta5-mods.com/scripts/sirensetting-limit-adjuster) 2.0.3 by cp702, 2020 to 2024-07-15, with a 2.0.4 for Legacy build 3788 in its repository, https://github.com/cpast/SirenSetting_Limit_Remover, pushed 2026-05-25.
It raises the sirens a vehicle may have from 20 to 32 (`siren1` to `siren32`, `siren_glass` likewise; the 128 bones per vehicle stay), the siren setting ID of `carcols.meta` from 255 to 65535 (0, 255 and 65535 reserved; above 255 the base game's IDs clash with each other) and the number of unique siren settings, and writes a `SirenSettings.log` of every setting and vehicle loaded and every ID clash; its RagePluginHook integration means nothing on Enhanced, which has no RagePluginHook.
Its license, checked 2026-10-26: none - the repository has no license file, and its README asks to link to the official upload instead of redistributing - so all rights reserved and idea only, none of its code read.
Never ported to Enhanced by its author.

The research's result for the sirens per vehicle (2026-10-29, the dump of 1.0.1158.16): the limit is there and binds silently.
The game lights 20 sirens, `siren1` to `siren20`, in four layers - a siren setting of 0x5E8 bytes holds 20 lights with their count and name behind them, a vehicle's siren state of 0x188 bytes keeps four arrays of 20, its renderer and six more loops run over 20 lights or bones, and the bones' hierarchy has IDs for `siren1` to `siren20` and their glass alone - and the 128 bones per vehicle stay, a bone's index being a signed byte.
Packs make emergency vehicles with 20 sirens or with 32, nothing more; one made for 32 loads, and its lights 21 to 32 stay dark.
So the limit is measured first (SPECIFICATIONS.md, section 11.16.2): with `DIAGNOSTIC_MODE=1` the measurement shows whether a mod load carries siren bones past `siren20`.

My second test (2026-10-30) gave the boot D8 of PLAN.md asked for: three packs' 30- and 32-siren models - the CVPI LSPD pack's, the CHP FPIU and the LVMPD Crown Victoria - with 10 to 12 lights the game never lights, half the CHP FPIU's lightbar dark in the game; the siren IDs had room (33 of 255 settings, none colliding).

It takes 29 patterns and 26 writes, which have not run on the game, and the lighting capacity's 8.1 A and C in the same build.
With it: every pack's line of the siren IDs' log names its file `?`, as `FileName`'s read of the entry's +0 is not the name on the real game (my proof's start 2), log only.

## Phase C: the population's limits and the lighting capacity

Ideas from the page of [Resource Extensions [Enhanced]](https://www.gta5-mods.com/scripts/resource-extensions) 2.4 by Koukol, read on 2026-11-01: closed source and its redistribution forbidden, so the page's words alone, its plugin never read.
Each has a research pass in the dump (PLAN.md, D20) and a measurement before any raise, in 2.7.0; a raise the numbers ask for comes in 2.8.0.

- The vehicle model budget and the ped model budget: researched, each a budget in bytes, 429 MB and 561 MB at the Population Variety slider's top, measured first in `TweaksVPopulation` (SPECIFICATIONS.md, section 11.16.4); a raise goes by my own `gameconfig.xml`'s block for my load (the vehicles' after 2.7.0's boot, PLAN.md, D-V5), a key of a part only if other people's loads are to get it, and only from the numbers.
- The ped pool: measured in 2.7.0, not raised; 2.8.0 only if the numbers ask, with the render queues' raise first (PLAN.md, D31).
- The render queues: measured in 2.7.0; a raise waits for the boot's numbers, its design in the findings' section 8.
- The lighting capacity: measured in 2.7.0; the raises of its findings' 8.1 A, B and C go in 2.8.0, with the sirens per vehicle (PLAN.md, D23, D30).
