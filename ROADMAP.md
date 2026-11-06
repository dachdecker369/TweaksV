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

- The vehicle model budget and the ped model budget: researched, each a budget in bytes, 429 MB and 561 MB at the Population Variety slider's top, measured first in `population.cpp` (SPECIFICATIONS.md, section 11.16.4); a raise goes by my own `gameconfig.xml`'s block for my load, a key of a part only if other people's loads are to get it, and only from the numbers.
- The ped pool: measured in 2.7.0, not raised: my proof's numbers do not ask for it.
- The render queues: measured in 2.7.0, not raised: my proof's numbers do not ask for it; a raise's design is in the findings' section 8.
- The ray traced list: 2.8.0 counts what it refuses, and my proof's start 4 refused 172,943 lights, 165,438 of them in one minute in the tunnel: no sort in 2.8.0, a research pass in a later build.
- Some lights on buildings appearing late: by my eye about 50 m from a Gruppe 6 building downtown, a light on the first floor's side turning on and off in sight, with nothing refused in the log and no cutoff near that distance in my `visualsettings.dat`; a hint, not read: the model that holds the light has that LOD distance.
  To be looked at, no pass planned.
