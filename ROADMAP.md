# Roadmap

What I plan for TweaksV next, before it is planned in [PLAN.md](PLAN.md) and specified in [SPECIFICATIONS.md](SPECIFICATIONS.md); what has shipped is in NOTES.md's Versions.
It holds what phase C leaves open once 2.8.0's raises are proven: the limits measured and not raised, for as long as no mod load's numbers ask for them.
A new part is a part like the sixteen (SPECIFICATIONS.md, section 16): its own file on the shared code - the scanner, the patch module, the relocation core, the three tables - its own section of `TweaksV.ini`, its old plugin in the conflict check, its measurement under `DIAGNOSTIC_MODE`, its suite, its patterns in PATTERNS.md.
One minor version per part, a patch version for its fixes, but parts built in one round ship together as one version, as 2.6.0's did (PLAN.md, D18); a version takes the next free number when it ships.

How a part is taken over depends on the original's license, as [CREDITS.md](CREDITS.md) says of the sixteen.
Code under a license the GPL-3.0 takes in - MIT, BSD, zlib, the GPL-3.0 itself - may be read and adapted: the patterns and the research are credited in CREDITS.md, every line carried over is named in THIRD_PARTY_NOTICES.md, and the part is still built on TweaksV's shared code, as 2.0.0 rebuilt the five plugins, because a plugin merged as it is brings its own scanner, INI and log, what 1.x had.
Code without a license, under all rights reserved or under FiveM's terms is idea only: none of it is read, and every game fact behind the part is found anew in the running Enhanced game with my tooling (a test build's dump, Ghidra; PATTERNS.md, "When Game Updates").
The original left is a Legacy plugin, and no pattern of a Legacy plugin carries over to Enhanced anyway (PATTERNS.md).
So every part starts with a research pass in the dump that settles whether the limit is in Enhanced 1.0.1158.16 at all, where it lies, and whether it still binds - Enhanced may have raised it - and the part goes in only when a mod load hits it; a measurement of the limit under `DIAGNOSTIC_MODE`, as the box streamer and the decals have, comes first and says so.

## Phase C: the population's limits and the lighting capacity

Ideas from the page of [Resource Extensions [Enhanced]](https://www.gta5-mods.com/scripts/resource-extensions) 2.4 by Koukol, read on 2026-11-01: closed source and its redistribution forbidden, so the page's words alone, its plugin never read.
Each has a research pass in the dump (PLAN.md, D20) and a measurement before any raise, in 2.7.0; 2.8.0 raised what the numbers asked for.

- The vehicle model budget and the ped model budget: researched, each a budget in bytes, 429 MB and 561 MB at the Population Variety slider's top, measured first in `TweaksVPopulation` (SPECIFICATIONS.md, section 11.16.4); a raise goes by my own `gameconfig.xml`'s block for my load (the vehicles' in 2.8.0's proof, PLAN.md, D-V5; the peds' not, D43), a key of a part only if other people's loads are to get it, and only from the numbers.
- The ped pool: measured in 2.7.0, not raised: my proof's numbers do not ask for it (PLAN.md, D31, D43).
- The render queues: measured in 2.7.0, not raised: my proof's numbers do not ask for it (PLAN.md, D43); a raise's design is in the findings' section 8.
- The ray traced list: 2.8.0 counts what it refuses, and my proof's start 4 refused 172,943 lights, 165,438 of them in one minute in the tunnel: no sort in 2.8.0, a research pass in a later build (PLAN.md, D36).
