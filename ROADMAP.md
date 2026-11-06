# Roadmap

What is eventually coming to TweaksV next and how long it should take, in hours.
The feature is planned in [PLAN.md](PLAN.md) and specified in [SPECIFICATIONS.md](SPECIFICATIONS.md) before anything is built.
How a feature is added depends on the original's license, as [CREDITS.md](CREDITS.md) states.
Code under a license the GPL-3.0 takes in - MIT, BSD, zlib, the GPL-3.0 itself - may be read and adapted: the patterns and the research are credited in CREDITS.md, every line carried over is named in THIRD_PARTY_NOTICES.md, and the part is still built on TweaksV's shared code, as 2.0.0 rebuilt five plugins, because a plugin merged as it is brings its own scanner, INI and log.
Code without a license, under all rights reserved or under FiveM's terms, is idea only: none of it is read, and every game fact behind the part is found afresh in the running Enhanced game with my tooling (a test build dump, Ghidra; PATTERNS.md etc).
The original left is a Legacy plugin, and no pattern of a Legacy plugin carries over to Enhanced anyway (PATTERNS.md).
So every module starts with a research pass in the dump that settles whether the limit is in Enhanced 1.0.1158.16 at all, where it lies, and whether it still binds - Enhanced may have already raised it - and the part goes in only when a mod load hits it; a measurement of the limit under `DIAGNOSTIC_MODE` comes first and says so.

The below ideas for future features come from the page of [Resource Extensions [Enhanced]](https://www.gta5-mods.com/scripts/resource-extensions) 2.4 by Koukol, read on 2026-11-01.
It is closed source and its redistribution forbidden, so the page's words alone are used, and its plugin is not.
Each has a research pass in the dump and a measurement before any raise.
Each estimate is for a round of that idea alone, from its plan to my boot; about 2 hours of it are the plan, the merge, the review and the proof, which ideas built together share, as they share the boot.

| Feature | Researched | Details | Time to build |
|---|---|---|---|
| The vehicle model budget and the ped model budget | Yes, measured in 2.7.0 | Each a budget in bytes, 429 MB and 561 MB at the Population Variety slider's top, measured first in `population.cpp` (SPECIFICATIONS.md, section 11.16.4); a raise goes by my own `gameconfig.xml`'s block for my load, a key of a part only if other people's loads are to get it, and only from the numbers; the findings design one redirect and one stub for both budgets. | About 5 hours: the coding about 2, my boot with a load past the game's budgets about 1. |
| The ped pool | Yes, measured in 2.7.0; one step needs a research pass of about 1 hour | Not raised: my proof's numbers do not ask for it; the findings design `PED_POOL_SIZE` up to 1,024 in nine steps, one a function of the part in place of one of the game's; the render queues' raise must come first. | About 9 hours once the render queues' raise is in, about 10 for both in one round: the research pass about 1, the coding about 3.5, my boot with more than 256 peds alive about 2. |
| The render queues | Yes, measured in 2.7.0 | Not raised: my proof's numbers do not ask for it; a raise's design is in the findings' section 8. | About 6 hours: the coding about 2.5, my boot with streamed peds past the game's 120 entries about 1.5. |
| The ray traced list | Partly: its room, in the lighting capacity's research; a sort needs a pass of its own | 2.8.0 counts what it refuses, and my proof's start 4 refused 172,943 lights, 165,438 of them in one minute in the tunnel: no sort in 2.8.0, a research pass in a later build. | About 7 hours: a research pass of about 1.5 into what the list feeds and what an add knows of its light, the coding about 2.5, my boot through the tunnel with ray tracing on about 1. |
| Some lights on buildings appearing late | No: to be looked at, no pass planned | By my eye about 50 m from a Gruppe 6 building downtown, a light on the first floor's side turning on and off in sight, with nothing refused in the log and no cutoff near that distance in my `visualsettings.dat`; a hint, not read: the model that holds the light has that LOD distance. | About 2 hours to tell whether it is a limit at all, by reading that model in the packs; a part, if it is one, gets its estimate then. |
