# Patterns

Every byte pattern TweaksV searches for in `GTA5_Enhanced.exe`, and the game build each one was last verified on.
TweaksV searches only the game's code (its executable sections).
Each pattern is defined once, in the table in `source\TweaksVPatterns.h`, under the label it has here (`H-E1`, `P-E-R31a`, ...), and is written here exactly as there; `?` matches any byte.
The tests check that this file and the table list the same patterns, and `TweaksV.log` names each pattern by its label.
RVA is where each pattern matched on 1.0.1158.16, counted from the start of `GTA5_Enhanced.exe` in memory.
Every game fact found is here, what each match holds - the reference, limit, call or function - among them; what each part does with them is in [SPECIFICATIONS.md](SPECIFICATIONS.md).

## Status

| | |
|---|---|
| **Game Build** | 1.0.1158.16 (PE timestamp 0x6AA45F10) |
| **Found** | 1.0.1158.16 uses 271 of the table's 274 patterns: Packfile picks one of three direct size patterns and one of two references by game build, so three stay unused. Which part looks for which pattern when is in SPECIFICATIONS.md, section 4.2. |
| **Tested** | My boots of 1.0.1158.16 with `DIAGNOSTIC_MODE=1` have found all 271 at the RVAs their rows give, the last of them on 2026-11-03. |
| **Matches** | All match exactly once but five, as their rows say: `D-E-L12` 7 times, `A-E-G1` and `G-E-G1` twice, `I-E-L08` and `I-E-N09` 3 times. To check: boot once with `DIAGNOSTIC_MODE=1` and `CONSOLE=1`, every part on, `[PEDPROPS] ENABLED=1` among them; the log lists each pattern's address and number of matches, and the bytes around each match of a pattern that matches more than once. |

The DirectStorage part uses no patterns.
Its hooks are on `NtCreateFile` in `ntdll.dll`, which every file open of the game passes, and on `NtSetInformationFile`, which every rename passes.
The first relies on how the game folder's files are locked: at start-up the game asks DirectStorage to open every file in its folder, subfolders included (`Redistributables\*.cab`, `BattlEye\*`, `update\`, `x64\`; on a boot of mine 463 files in the first 10 seconds, 184 of them `.rpf` and `.cache`, 279 others), and DirectStorage opens each with `CreateFileW` for reading, letting others only read it - `NtCreateFile` with access 0x80100080 (`GENERIC_READ`, `SYNCHRONIZE`, `FILE_READ_ATTRIBUTES`), share `R--`, options 0x48 - on the thread that called `OpenFile`, right before it returns, and keeps the handle.
A relative path reaches `NtCreateFile` as a name relative to the current directory's handle, which the hook tells by the folder's identity.
The diagnostic aids hook `DStorageGetFactory` in `dstorage.dll` and, on the factory the game gets for the interface `{6924EA0C-C3CD-4826-B10A-F64F4ED927C1}`, its function [4], `OpenFile` (`DStorageCore.dll+0x3F9D0` with DirectStorage 1.2.2311.1405 on 1.0.1158.16): nothing depends on them.
The game reads more than its folder's files through that `OpenFile`: its saves and `pc_settings.bin` (in `Documents\Rockstar Games\GTAV Enhanced\Profiles`, the saves named `SGTA500xx`) and its cloud files in `%TEMP%` (`cloud_*.dat`), and a mods loader hands it loose files of its own folder: NaturalVision Enhanced's `onigiri.asi` 91 of them, `onigiri\common\data\gameconfig.xml` the first.
The second hook relies on Windows, not on the game, as the suite's runs on my PC showed (Windows 11): a classic replace-rename reaches it as `FileRenameInformation` with ReplaceIfExists and a full name, what `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` makes, or as `FileRenameInformationEx` with replace-if-exists and without POSIX semantics: `ReplaceFileW` renames the replaced file to a `~RF....TMP` name beside it and the replacement onto its name that way, flags 0x5, replace-if-exists and `FILE_RENAME_SUPPRESS_PIN_STATE_INHERITANCE`.
NTFS takes `FILE_RENAME_POSIX_SEMANTICS` from Windows 10 1607 on.

### When Game Updates

1. Start the game with `DIAGNOSTIC_MODE=1`, `DRY_RUN=1` and `CONSOLE=1`, every part on, `[PEDPROPS] ENABLED=1` among them, which ships off, so that every pattern is looked for (SPECIFICATIONS.md, section 4.2).
   The game is not patched then, and `TweaksV.log` lists every pattern not found, by its label.
   A dry run does not redirect the script programs' call, so the Blips part sees no script program then: whether it still finds `blip_controller`'s guard shows only on a boot without `DRY_RUN`, in its line or its problem.
2. Check whether the project the pattern came from has changed it (`CREDITS.md` names them).
3. Update the pattern in `source\TweaksVPatterns.h` and in this file - the tests check that both agree - and confirm that it matches exactly once.
   `GTA5_Enhanced.exe` is encrypted on disk and its code decrypted only in memory, so the file shows random bytes where the patterns match: look for the code in a copy of the running game, which a test build with `TEST_DUMP_CODE=60` saves (README, Test Builds).
   The game is built with clang, not MSVC: its code writes `31 C9` and `45 31 C0` for xor where MSVC writes `33 C9`, and the image has a `.retplne` section, two `.text` sections and a `.pdata` function table, so a pattern from a GTA V Legacy mod never carries over and is found anew here.
   To analyse a dump, rebuild it as a PE (the headers and the saved sections at their offsets, `.data` as zeros) for Ghidra's headless analyser; the `.pdata` table gives the functions.
   A fact in `.data`, which a dump holds as zeros, is read with `TEST_DUMP_DATA` (README, Test Builds): windows of it at the RVAs the code names, at the start and later.
   Re-find each site from its anchor pattern rather than trusting its old RVA, and treat a site that moved as new findings, never a guess.
4. DirectStorage: with `DIAGNOSTIC_MODE=1` the log names the factory's functions and says why `OpenFile` cannot be logged (another interface, an `OpenFile` outside DirectStorage's DLLs); the part's job depends on how the files are opened (item 5).
5. The game facts no pattern protects, to check in the dump even when every pattern still matches, since a change in them goes wrong silently; what an entry only names stands in full in the section of its name below, or in its label's row.
   - **RageOpenV:** `fiPackfile`'s fields, from the object's start, Legacy's in brackets: +0x28 [+0x20] the entry table, +0x30 [+0x28] its count, +0xBC [+0xB4] the current file offset; entries of 16 bytes, +0 the name offset, +4 the type, +12 the encryption; the fixed distances of `R-E-F2`, `F3`, `F6`, `F7`, `R-E-E5` and `R-E-C1`; every ParseHeader that returns true, only for RPF7, passing the FindEncryption call (`R-E-E1`) first, before any branch on the readHeader flag or on a header handed in, so that the tag it sees, which the hooks rely on, is the parsed archive's.
   - **The eight file functions' paths** (`R-E-F1..F8`): each turns its char string into UTF-16 with code page 65001, UTF-8, flags 0, before any Win32 call - `B9 E9 FD 00 00` (`mov ecx, 0xFDE9`), `31 D2`, `49 89 D8` or `49 89 F8`, then `FF 15` through the import slot at RVA 0x51215F4, with MultiByteToWideChar's six arguments - at RVA 0x10B158, 0x10B973, 0x10B823, 0x10BC55, 0x108028, 0x108781, 0x108971 and 0x108CA0, in the labels' order; converts at most 259 bytes, into 260 wide characters; makes every run of `/` or `\` one `\`; and opens by the UTF-16 path, which Windows compares with the files' names by their upper case.
     So RageOpenV's paths take at most 254 bytes, its hooks handing the game `mods/` and the path, and its walk meets the same `MAX_PATH` in a process that is not long-path aware.
     Functions that took their paths in the ANSI code page would leave every name outside ASCII unserved, without an error: check the conversion at the eight sites and the 259 bytes.
   - **PackfileLimit:** the archive count at offset 24 of the streaming configuration that `P-E-H1`'s call reads; that call's single argument.
   - **BoxStreamer:** the tree-search calls in `B-E-L1..L4`, their fifth argument on the stack; the 16 entries a search stops short (`kSearchHeadroom`, by which the measurement counts the searches the game's own list could not hold).
   - **Decals:** the decal manager's loop in `D-E-L10` and `D-E-L11`.
   - **The arrays' references,** exactly the ones listed or the scan refuses: the modkit array's 6 leas, the packfile array's 33 (25 leas and the 8 stores of `P-E-R23..R30`), the box streamer list's 8, the decal definitions array's 31 leas.
   - **HeapAdjuster:** a heap of 1024 MB or more counting as another adjuster's: a build's own heap that large is left alone, with only a log line.
   - **DirectStorage's opens:** the game still locking its folder's files through opens that read only and share reading, which `UNLOCK_FILES` widens: every file of its folder tree, 280 in the first 10 seconds on 1.0.1158.16 with its RPF cache check skipped as RageOpenV's part does, its archives alone when the check runs; opened for writing or exclusively they stay locked without an error, so check the 10-second summary: the game folder's files among the opens given write and delete sharing, by type, few left as they were (`DIAGNOSTIC_MODE=1` names each open's access and sharing).
   - **Script programs:** the fields at +0x2C, +0x58, +0x60 and +0x68; the page size and its rule; the opcodes' values and lengths, against the game's length table; the native table's rotation and what it holds before and after `Y-E-H1`'s call; the function behind that call; the resource placement thread the call runs on at every placement, a save load included; the registry the program enters only after it; the name's hash.
   - **Blips:** the pool's 2,000 slots (`L-E-P1` and `L-E-P2` give addresses, not the number, which the measurement takes as 2,000) and the three the count leaves out; the count's answers with the pool full; the guard's shape in `blip_controller`, which a script update can move or change in its natives' indices, its form or its threshold: the threshold 150, the room it needs and 1,997, the highest limit, are this build's.
   - **Weapon infos:** the initializer running after `DllMain`; 0x1893AC0's blobs from `edx` alone, its loop unchecked; the capacity the only other carrier; no compare of the count and no other array sized by it (60 instructions in 13 functions address its fields); the loader the only adder, the block never reallocated after the initializer; the data file entry in the loader's `rcx`, 0xA8 bytes: its name read bounded within the first 0x80 bytes, which the loader copies itself, its type at +0x90 (0xA3 a re-parse of the blob of that name, which goes to 0x1863E40).
     Not settled: what the loader returns: the guard returns 0 for a file it refuses, the load handler of later packs returns true either way (`B0 01` at 0x188E4E1), the manager's init is not read.
     Seen by my boot of the guards, at the rooms 160 and 470: 3 component files and 6 `weapons.meta` files refused, the game playable, no crash; a refused pack's weapon spawned without its magazine, its rounds floating under the gun.
   - **Weapon components:** the list's size, the distance to its count right behind it (`W-E-C1`'s +2), in no instruction: for a compare added, or something put between them, look in the dump for an immediate of the size and of the size minus one, and at the shape of each load the scan finds; the 17 loads as the list's whole use (no pointer in the dumped sections, `.data`'s initialised part never dumped), 6 in the protected `.text`, which could put them back: only a boot with more than 470 definitions, played a while, shows whether it does; the rebuild after every file, so that the count is all loaded files' definitions and `W-E-C1`'s +14 the files; the entries' sort; the pool's slot count, the lookup's result as it stands, handed to the constructor (0x15A490) at 0x1856022 as `mov edx, ebx`; the lookup's rule; the slot's size; the blob's definitions and their uint16 at +0x08.
     For the guard: the loader's two callers; the parse coming after it, so that the blobs' sum is true at its head for every mounted file, the base loader's list count stale until its rebuild; its callback's vtable's +0x20, with dl the callable not stored inline; the mounter's unload, and its load true whatever the loader did; the blob's count, which `W-E-C5` does not pin: the pool's margin of 470 rests on the parser making no definition past the blob's array, not settled; when the weapon init, and the base loader with it, runs, in the protected code (the names kept cover a run after the packs).
   - **Siren IDs:** the parser's types and member data; the room after the IDs' members (+0x01..+0x03 of a setting, +0x4C..+0x4F of a variation); the settings' and the carcols data's layout; the ID table's place and its fill before the base loader's call; 255 as "no ID", position 0 as no sirens; the loaders' rules for a clash; nothing reading an ID after the load; the constructors' stores `I-E-C1` to `C3` rely on; the protection's reads of the gap bytes; the dedupe's load and what follows it, for the stub; the init calling the base loader right after another call, setting up no argument; the merge and the resize (RVA 0x1378210, 0x137A3D0) copying a setting's +0x00..+0x38 whole, a wide ID's high byte at +0x01 with it; the PSO loader with a member wider than its file's.
   - **Siren count:** the renderer's nine arguments, the 7th, the skeleton, pinned by nothing; its call once per pass of a vehicle whose sirens are on (a HINT from the data flow, not verified); the bone structure's layout and IDs; the skeleton data and its bone count; find-bone-by-name's arguments and result; the bones' names; the model info's +0x18; first sign of a change: the measurement's line per model that says a bone its structure names is not found by name, or found at another bone.
   - **Siren damage:** the fill's arguments, its table's shape, no ID checked; the tag's rule; the 32-bit sets at the vehicle's +0x860 and the damage object's +0x440; every reader not raised reading the game's own table, the raised loops nothing sized by 20; the load of the pool right before `I-E-Y2`; a loop added or missed stays at 20, cosmetic, no crash; the raise stays off any build these were not read on.
   - **PedProps:** the narrowed registers holding the whole 32-bit value, or the widening has no effect: `A-E-T1`'s `ebx`, the anchor's total, by `add ebx, eax` at RVA 0x10F4E19 before the pattern; for `A-E-T2..T5` the per-anchor count's sum in `eax` (`add eax, ecx`, inside `A-E-C2`); `A-E-G1`'s `esi` by `mov esi, [rdx+rdi+8]` at RVA 0x10F1DE9, `A-E-G6`'s `edi` by `mov edi, [rax+rsi*4+0x1c8]` at RVA 0x10F883C, both outside their patterns, `A-E-G3..G5`'s inside; the eleven as all the narrowings of an anchor's total and of a worn prop's index: the call counts (7 and 17) guard the calls, not a walk written inline anew, so repeat the region's scan, the per-file narrowings and every call of the prop functions read for a narrowed index; one file's own bound, 255 props on an anchor; the layout the measurement reads beyond its patterns: an anchor entry's anchor (compared at RVA 0x10F4F00, `39 54 39 10`, after the loop's alignment), a model info's hash, the 13 anchors, the per-anchor count taking an anchor's first entry in each file; the natives' bodies in the protected code, out of the part's reach: on 1.0.1158.16 the set-prop native's path checks the anchor and the index's sign, nothing narrower.
   - **Texture budget:** the counters' layout and what the game counts there; the configuration's offsets: `Q-E-T1` and `Q-E-T2` pin 0x54C, 0x550 and 0x558, +0x554 and +0x55C follow from `pmovsxdq` reading two int32 each; the three stores as the only writers: the part's scan sees only stores through `[rip+disp32]`, not a write through a register, as the getter reads them; no full value beside each `_Base`, as `ConfigPopulation` has; the index and its override; the update check's switch, factors and flag, and its two calls into the device's vtable; a placement failing while the budget is at most the bytes of both (`48 39 C6`, then `73 1C`, at RVA 0x4DBCDF); Windows' budget in the adapter and `availablevidmem`'s `int3`; the game's adapter, DXGI's choice by high performance with software adapters and Microsoft's skipped; the call after the stores (`Q-E-T2`'s +40), its function's four registers at most and nothing on the stack, as my boot at 6,000 MB bore out; the check before the read and the counting at the place; a counted object's one descriptor; the heap's sum taken in code, its free list plain and unchecked (a list of another shape shows as the walk ending at once, "all in use"); 0x509BF0 and 0x1C124E0 not setting the settings' +0x128 and +0x12C, so that the raise's read takes the same constants.
   - **Legacy packs:** the resource versions the check for packs made for GTA V Legacy names a pack by, from the packs' files, and such a vehicle's trap (section "RageOpenV", with the RPF7 layout): a boot with a pack made for Legacy and the same pack converted for Enhanced shows whether the trap and the versions still hold.
   - **RayTracing:** the buffer object's fields, +0x0C the stride in the low 12 bits of a word, and the views' size from them (the maker with its flags' bit 3), which `T-E-B1` does not pin; the 20-byte vertex (the skinning shader 0x4CDF58A2 writes it, the reflection lighting reads it); the buffer made once, after `DllMain`, its global 0 until then: made anew later (a hint: ray tracing switched on in the menu) a second store; the two frame slots and their adjacent counters; the capacity held only by the two checks and the count (a check added elsewhere refuses earlier, which is safe; a write by a size of its own would not show); the shaders' 26-bit first vertex; what a refusal does, its callers handing their job lists back, rax and the flags free for the counting stubs; the skinning job's +0x48; the neighbouring limits of the same frame.
   - **StaticBounds:** the one lookup of the store's size, which `C-E-L1` pins from the call to the pool's lea and no more (the read-back reports a second reader, which splits the store); the name's hash (a store renamed stays at the game's size with the redirect in place, which only the size's line shows); the store's offsets, in no pattern but the pool's; the game asking after `DllMain`; `blip_controller`'s placement after the load's names, which the read-back's numbers rest on, by the hash at +0x58 of a program.
   - **LightCapacity:** the records' sizes, which the stores' distances rest on; 5 vehicles a job and 60 jobs, in the code's shape and no one constant (`lea edi, [rax+rax*4]` at RVA 0xDE692E and 0xDE6931, the same in 0xF0104E); 300 vehicles a frame, the length of four arrays with their counts right behind them, in no instruction; the frame's order, the vehicles' lights last; the sort list's mark, the byte at +4 of its count, read by no pattern's instruction; the siren setting's byte at +0x38 and its lights' flag at +0x45, through `I-E-T4`'s pointer; a job's scratch handed from the worker's frame to a job whose +0x6E has bit 0 set, the only such call found; the shaders' 512 and 128, in the game's shader files; the ray traced upload's score, with its 196 m; the functions behind `G-E-C1`'s and `G-E-F1`'s jumps, pinned only by the 12 and the 14 bytes of their ways out that the module compares at run time.
   - **Population:** the object's fields no pattern pins (the vehicles' excess, lists, requests in flight and shares, the peds' excess) and the list's format, only logged, a wrong one a wrong number, never harm; the model info's +0x286 bit 1, the streamed flag (a hint of the render queues' pass), and its +0x9D's kinds; the variety's formula (`O-E-P8`'s line logs the 5, the 1 and the 10, not the 0.05; a raise's cap rests on the 10); the block of `gameconfig.xml`, but the story entries' dwords, which `O-E-V7` and `O-E-P5` to `P7` pin: the parser ignores a renamed member in silence, so read the names again from its descriptors with a data window; the constructor's defaults (a change reads in the tables' line as `not the defaults of 1.0.1158.16`, and the module's two arrays are read again); entry 0 of each table and the level's first value, in `.data` (the first tables' line and summary show them); the floor of 9 vehicle models, the frame gates and the 64 frames, behaviour TweaksV does not read; the floor of 6 ped models, only as the bytes `O-E-P2`'s check compares at +0xEA, where a change stops the measurement.
   - **Ped pool:** the pool's layout and the high 2 bits of its +0x28; the spatial array's offsets, +0x28, +0x30 and +0x38 x, y and z, +0x40 and +0x48 the node's address's parts, +0x50 flags, +0x58 capacity, +0x5C count, set in 0x11682D0; no other function holding a list by that count, so every read of +0x5C after a load of its pointer; the fixed arrays' sizes, the singleton's offsets in 0x1D90620, +0x20 and +0x1020, the filler's three arrays, 1,024 in the six methods and 0x1164D40; the ped's offsets the measurement reads; the pool's size never passing the number.
   - **A pool's memory:** the allocator's offsets, which the pool memory's line reads and only its budget and the sum of its use and room check; the pools' proxy leading to that allocator, slot 5 of the master allocator (0x36D90, and 0x13CB90 with slot 5 in the start-up).
   - **Render queues:** the pool object's layout, protected by the stride read at +0x1C at run time; the array's capacity at +0xA, which only the inits pin, the adds the rest of the array; three of a family's four arrays holding whole frames (reset on a flip, the most would read low, not wrong); the game's own `gameconfig.xml` naming the vehicles' two sizes (if not, one vehicle streams, a capacity of 1 in the measurement); the pools' names, labels only, since the code matches hashes; whether streamed models exist in a given load, which is data, not code.

## HeapAdjuster

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| H-E1 | `BA ? ? ? ? 45 31 C0 41 B1 01 E8 ? ? ? ? 48 8D 0D` | +1: imm32 of `mov edx` - heap size in bytes; 646 MB on current build | found | 0x13F7 |
| H-E2 | `48 8D 0D ? ? ? ? 48 8D 15 ? ? ? ? 41 B8 00 00 01 00 41 B9 ? ? ? ? E8` | the set-up of the allocator a pool's memory comes from: +3 rel32 the allocator (ends +7, 0x29B1508); +10 rel32 the heap (ends +14, 0x29A8590); +22 imm32 of `mov r9d` - its budget in bytes, 136 MB on current build; +26 E8 to its constructor | found | 0x143D |

## ModkitLimit

The array: RVA 0x471E610, 1024 entries of 2 bytes; modkit IDs are 16-bit, which caps it at 65536.
The six `lea` below, each with its rel32's offset, are every reference the game's code has to it: nothing else reaches it or into it, and no stored pointer holds its address.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| M-E-R1 | `31 c9 48 8d 05 ? ? ? ? eb` | +5 | found | 0x134A56F |
| M-E-R2 | `89 c2 48 8d 0d ? ? ? ? eb` | +5 | found | 0x13785B2 |
| M-E-R3 | `41 0f b7 c2 48 8d 0d ? ? ? ? 0f b7 04 41` | +7 | found | 0x7A6F4B |
| M-E-R4 | `57 48 83 ec ? 48 8d 0d ? ? ? ? 41 b8` | +8 | found | 0x1349BD1 |
| M-E-R5 | `48 8d 3d ? ? ? ? 0f b7 3c 47` | +3 | found | 0xD225CE |
| M-E-R6 | `77 ? 48 8d 15 ? ? ? ? 0f b7 14 42` | +5 | found | 0x1E50467 |

Limits: imm32 = size x mul + add, each decoding to the same size, 1024.

| Label | Pattern | Offset, mul, add | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| M-E-L1 | `0f b7 c0 66 ba ? ? 3d ? ? ? ? 77` | 8, 1, -1 | found | 0x1E5045B |
| M-E-L2 | `48 83 c5 ? 48 83 c0 ? 48 83 c7` | 15, 1, 3 (the imm32 of `cmp rdi, imm32`, `48 81 FF` at 12) | found | 0x137863F |
| M-E-L3 | `74 ? 48 83 c5 ? 48 83 c3 ? 48 83 c0 ? 48 3d` | 16, 1, 3 | found | 0x137868D |
| M-E-L4 | `48 83 c7 ? 48 83 c2 ? 48 83 c5 ? 48 81 fd` | 15, 1, 3 | found | 0x134A603 |
| M-E-L5 | `48 83 c7 ? 48 83 c2 ? 48 83 c6 ? 48 81 fe` | 15, 1, 3 | found | 0x134A657 |
| M-E-L6 | `41 b8 ? ? ? ? b2 ? e8 ? ? ? ? 66 0f 76 c0` | 2, 2, 0 | found | 0x1349BDD |
| M-E-L7 | `0f b7 41 ? 66 bf ? ? 48 3d` | 10, 1, -1 | found | 0xD225BE |
| M-E-L8 | `41 0f b7 ca 66 b8 ? ? 81 f9` | 10, 1, -1 | found | 0x7A6F3B |

## PackfileLimit

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| P-E-D3 | `48 83 C6 ? 48 81 F9` | +7: direct size, the imm32 of `cmp rcx, imm32` (`48 81 F9`), as in `P-E-D2` and `P-E-D1` (builds 1.0.1158.13 and later); 7260 on current build | found | 0x10F1BF |
| P-E-D2 | `49 83 c3 ? 48 81 f9 ? ? ? ? 75 ? eb` | +7: direct size (builds 1.0.1013.17 to 1158.12) | not used on current build | - |
| P-E-D1 | `48 81 f9 ? ? ? ? 74 ? 48 83 3c ca 00` | +3: direct size (older builds) | not used on current build | - |
| P-E-I1 | `3b 05 ? ? ? ? 0f 8d ? ? ? ? 89 c0` | +2: rel32 of `cmp eax, [rip+disp32]` to the indirect size variable, which holds N-4 (7256 on the current build) | found | 0x1279156 |
| P-E-H1 | `89 48 ? 48 8d 0d ? ? ? ? 48 89 08 48 89 c1 e8` | +0x10: the call handed the streaming configuration read from `gameconfig.xml`, its archive count at offset 24, 8512 on current build | found | 0x5093D5 |

The array: RVA 0x2C075B0, 7260 entries of 8 bytes; archive indices are 16-bit, which caps it at 65535.
The rows below, each with its rel32's offset and with the build's `R31a` or `R31b`, are every reference the game's code has to it: 33 on 1.0.1158.16, 25 `lea` and the 8 `mov [rip+disp32], r64` of `P-E-R23..R30`.
Nothing else reaches it or into it, and no stored pointer holds its address in the sections the dump holds.
`P-E-R32` and `P-E-R33` were found in my code dump.

| Label | Pattern | Offset | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| P-E-R01 | `48 8d 15 ? ? ? ? 48 8b 3c ca 48 8b 2f` | 3 | found | 0x7EED5 |
| P-E-R02 | `48 8d 3d ? ? ? ? 48 8b 0c cf 48 85 c9` | 3 | found | 0xCE966C |
| P-E-R03 | `48 8d 3d ? ? ? ? 48 8b 0c 39 48 85 c9` | 3 | found | 0xCE9613 |
| P-E-R04 | `89 c0 48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 74` | 5 | found | 0xCEC43A |
| P-E-R05 | `89 c0 48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 0f 84` | 5 | found | 0xCE9CD3 |
| P-E-R06 | `48 8d 0d ? ? ? ? 4a 8b 0c c9 48 85 c9 0f 84 ? ? ? ? 48 8b 01` | 3 | found | 0xCEE169 |
| P-E-R07 | `48 8d 0d ? ? ? ? 48 8b 1c 08 48 8b 03` | 3 | found | 0xCF0D65 |
| P-E-R08 | `48 8d 0d ? ? ? ? 4a 8b 0c c1 48 85 c9` | 3 | found | 0x127A36E |
| P-E-R09 | `48 83 ec ? 48 89 ce 48 8d 05 ? ? ? ? 48 89 01 c6 81` | 39 (the rel32 of `lea rcx`, `48 8D 0D` at 36) | found | 0x10F781 |
| P-E-R10 | `48 c1 e8 ? 48 8d 0d ? ? ? ? 48 8b 0c c1 48 8b 01` | 7 | found | 0x1276047 |
| P-E-R11 | `48 8d 0d ? ? ? ? 48 83 3c c1 ? 40 0f 95 c6 eb` | 3 | found | 0x1276145 |
| P-E-R12 | `48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 ff 90 ? ? ? ? 48 8b 48` | 3 | found | 0xCF0EDB |
| P-E-R13 | `48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 4c 8d 44 24` | 3 | found | 0x12775CE |
| P-E-R14 | `48 8d 0d ? ? ? ? 4c 8b 24 08 49 8b 04 24 4c 8d 44 24` | 3 | found | 0x1277E25 |
| P-E-R15 | `48 8d 0d ? ? ? ? 48 8b 0c 08 48 85 c9 75` | 3 | found | 0xCEC3E0 |
| P-E-R16 | `48 8d 0d ? ? ? ? 48 8b 0c c1 48 85 c9 0f 84 ? ? ? ? 48 89 f5` | 3 | found | 0xCE3431 |
| P-E-R17 | `48 83 ec ? 89 d7 48 89 ce 48 8d 05 ? ? ? ? 48 89 01 c6 81 c1` | 41 (the rel32 of `lea rcx`, `48 8D 0D` at 38) | found | 0x113FB2 |
| P-E-R18 | `48 8d 0d ? ? ? ? 48 8b 3c 08 48 8b 07 48 89 f9` | 3 | found | 0x127AFB5 |
| P-E-R19 | `48 8d 0d ? ? ? ? 48 8b 0c 08 48 8b 01 ff 90 ? ? ? ? 48 8b 40` | 3 | found | 0xCF1384 |
| P-E-R20 | `48 8d 0d ? ? ? ? 48 8b 0c d1 48 85 c9 75` | 3 | found | 0x1275F75 |
| P-E-R21 | `48 8d 0d ? ? ? ? 4c 8b 24 c1 4d 85 e4 0f 84` | 3 | found | 0x1279164 |
| P-E-R22 | `48 8d 0d ? ? ? ? 48 8b 0c c1 48 8b 01 ff 90 ? ? ? ? 48 8b 38` | 3 | found | 0xCEC4F2 |
| P-E-R23 | `48 89 2d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 48 c7 44 08` | 3 | found | 0x127AAB5 |
| P-E-R24 | `48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 44 00 00` | 3 | found | 0x1275B50 |
| P-E-R25 | `48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 90` | 3 | found | 0x1279C53 |
| P-E-R26 | `48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 66 66 2e 0f 1f 84 00 00 00 00 00` | 3 | found | 0x12799D9 |
| P-E-R27 | `48 89 2d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 84 00 00 00 00 00` | 3 | found | 0x127AD0D |
| P-E-R28 | `48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 66 66 66 66 2e 0f 1f 84 00 00 00 00 00` | 3 | found | 0x1279B18 |
| P-E-R29 | `48 89 3d ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 0f 1f 40 00` | 3 | found | 0x127ABC1 |
| P-E-R30 | `48 89 35 ? ? ? ? b9 ? ? ? ? ba ? ? ? ? e8 ? ? ? ? b9 ? ? ? ? 48 c7 44 08` | 3 | found | 0x1276ED5 |
| P-E-R31a | `BF 08 00 00 00 BE 10 00 00 00 4C 8D 05` | 13 (builds 1.0.1158.13 and later). The two constants are TweaksV's: the original's `BF ? ? ? ? BE ? ? ? ? 4C 8D 05` also matched obfuscated code at 0x582A227 | found | 0x10F163 |
| P-E-R31b | `41 ? ? ? ? ? ? ? ? ? ? 41 bb ? ? ? ? ? 8d` | 20 (older builds) | not used on current build | - |
| P-E-R32 | `48 89 cf 31 f6 4c 8d 3d ? ? ? ? eb 10` | 8 (RVA 0x13EA794): the rel32 of `lea r15` (`4C 8D 3D` at 5), the load in the function 0x13EA760..0x13EA9A4, which walks a collection's entries, looks each one's packfile up as array[handle >> 16], tolerates a null entry and folds a value per entry into a hash it returns | found | 0x13EA78C |
| P-E-R33 | `75 0c cc 48 8d 0d ? ? ? ? 48 8b 0c d1` | 6 (RVA 0x1275F89): the rel32 of `lea rcx` (`48 8D 0D` at 3), in `P-E-R20`'s function 0x1275F10..0x1275FF4, in the bytes after an `int3` trap - reached only when the index is at or past the N-4 variable or the entry is null, so never run | found | 0x1275F83 |

## BoxStreamer

`B-E-L1..L4` hand the traversal list's capacity, 1000, to the tree search (`mov dword ptr [rsp+20h], imm32`).
The `call rel32` after each (the call column) leads to the search: `B-E-L1` and `B-E-L3` to RVA 0x1EBFB20, `B-E-L2` and `B-E-L4` to 0x1EC0140.
A search stops 16 entries short of its capacity, so the game's own list takes at most 984 entries from one search.
The list: RVA 0x3ED6B40, 1000 entries of 8 bytes, with no pattern of its own: two of the four query functions (RVA 0x92C470, 0x92C7A0, 0x92DD70, 0x92E040) are near-identical copies, so the second of the two `lea` that load it in each cannot be told apart by the bytes around it.
`B-E-L2..L4` end in the `lea r13, [rip+disp32]` that loads it; a scan for every `lea r64, [rip+disp32]` that does finds exactly 8: RVA 0x92C4AB, 0x92C764, 0x92C7D3, 0x92CA12, 0x92DDCD, 0x92E013, 0x92E089, 0x92E2D2.

| Label | Pattern | Offset: capacity, lea rel32, call | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| B-E-L1 | `48 8B 44 24 40 48 8B 48 08 C7 44 24 20 ? ? ? ? 48 8D 54 24 60 4C 8D 44 24 70 4D 89 F1` | 13, -, 30 | found | 0x92C527 |
| B-E-L2 | `48 8B 81 08 01 00 00 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D` | 15, 22, 34 | found | 0x92C7C0 |
| B-E-L3 | `0F 29 44 24 40 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D` | 13, 20, 42 | found | 0x92DDBC |
| B-E-L4 | `48 8B 01 48 8B 48 08 C7 44 24 20 ? ? ? ? 4C 8D 2D` | 11, 18, 30 | found | 0x92E07A |

## Decals

The array: RVA 0x42EC8A0, N = 260 entries of 80 bytes.
The count of registered definitions: a dword 16 bytes before it (0x42EC890), addressed only by its own instructions, never through the array, nor the array through it; vanilla registers 259.
The pair of constants 0x100 and 0x200 that `DecalPatch.asi` patches is not this build's limit.
A scan for every `lea r64, [rip+disp32]` that loads the array finds exactly 31: RVA 0xB195A8, 0xB3187E, 0xB31AD6, 0xB31FA7, 0xB324CA, 0xB326EF, 0xB32C71, 0xB32F2B, 0xB33423, 0xB338F6, 0xB33EF5, 0xB34892, 0xB34CF7, 0xB34F96, 0xCA0A1F, 0xCA11CE, 0xCA323B, 0xCA6DD5, 0xCA756C, 0xCA960E, 0xCAA2F7, 0xCAD859, 0xCADE17, 0xCB30C9, 0xCB386D, 0xCB3A4F, 0xCBEAE2, 0xCBF181, 0xCBF251, 0xCBF2D0, 0xCD2B60.
An entry, as the parser fills it from the `decals.dat` columns: +0x00 int32 decal id (the first column); +0x08, +0x10 and +0x18 pointers to the diffuse, normal and specular textures (0 until loaded - the only pointers in an entry, and the ones the start-up loop zeroes); +0x20 float size; +0x24 specular multiplier, +0x28 specular falloff, +0x2C specular intensity, +0x30 fresnel, +0x34 steep, +0x38 parallax scale and +0x3C emissive value, all floats; +0x40 byte texture rows, +0x41 columns, +0x42 id A, +0x43 id B, +0x44 texture wrap (0 or 1; the game sets 4 later for "rotate to camera"), +0x45 anisotropic, +0x46 washable, +0x47 underwater create, +0x48 underwater fade; +0x49 to +0x4F unused.
No entry holds a pointer to another entry or into the array, and nothing else in the game is sized by the number of definitions.
Entries are read with scalar loads and cleared with unaligned stores, so a page-aligned buffer needs no more alignment.
A definition's index is a uint16 in the game's decal instances.
Findings: `decals_findings_for_the_clean_room_build.txt`.

| Label | Pattern | Offset: what | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| D-E-L1 | `8B 35 ? ? ? ? 48 81 FE ? ? ? ? 77 ? 48 89 F9 FF 15` | 2: the count's disp32, in `mov esi, [rip+disp32]`; 9: N-1, in `cmp rsi, imm32` (the parser's "full" check) | found | 0xCBEB8A |
| D-E-L2 | `4C 8D 2D ? ? ? ? F3 44 0F 10 0D ? ? ? ? F3 44 0F 10 15` | 3: the array's rel32 (the parser's lea) | found | 0xCBEAE2 |
| D-E-L3 | `56 85 C9 74 ? C7 02 04 01 00 00 41 C7 00 00 00 00 00 44 8B 1D` | 7, 0x53, 0x62: N (FindById; the last two past the pattern's end); 0x21: a lea of the array | found | 0xCBF160 |
| D-E-L4 | `B8 04 01 00 00 81 F9 03 01 00 00 77 ? 41 89 D0 8B 05` | 1: N, 7: N-1 (PickVariant) | found | 0xCBF1D0 |
| D-E-L5 | `45 31 C9 41 BB 04 01 00 00 31 C0 45 31 D2 EB` | 5: N (the texture override, one definition) | found | 0xCBF232 |
| D-E-L6 | `41 BB 04 01 00 00 48 8D 3D ? ? ? ? 31 C0 45 31 D2` | 2: N (the texture override, its loop); 6: a lea of the array | found | 0xCBF24B |
| D-E-L7 | `41 81 FB 04 01 00 00 89 C3 41 0F 44 D9` | 3: N (the texture override) | found | 0xCBF260 |
| D-E-L8 | `81 FD 04 01 00 00 89 D8 41 0F 44 C1` | 2: N (the texture override) | found | 0xCBF283 |
| D-E-L9 | `41 81 FB 04 01 00 00 41 0F 44 C1 48 8D 3D` | 3: N (the texture override, its odd tail); 11: a lea of the array | found | 0xCBF2C5 |
| D-E-L10 | `B8 D8 07 00 00 48 8D 0D ? ? ? ? 0F 57 C0` | 1: the start offset (0x7D8) of the decal manager's start-up loop, which zeroes the texture pointers of 26 entries a pass; 5: a lea of the array | found | 0xCD2B5B |
| D-E-L11 | `48 05 20 08 00 00 48 3D 18 59 00 00 0F 85` | 2: the bytes one pass clears (0x820 = 26 x 80); 8: the loop's end, the start offset + N x 80 (0x5918) | found | 0xCD2D6B |
| D-E-L12 | `E8 ? ? ? ? 3D 03 01 00 00 0F 8F` | 6: N-1 (the check after the call to PickVariant in 7 decal-placing functions - matches 7 times) | found, 7 matches | 0xB31856, 0xB31F7D, 0xB326C5, 0xB32C49, 0xB32F03, 0xB333FB, 0xB34880 |
| D-E-L13 | `44 89 C3 41 81 F8 03 01 00 00 77` | 6: N-1 (the decal-placing function that takes the index as a parameter) | found | 0xB338A8 |
| D-E-L14 | `89 C6 81 FE 03 01 00 00 0F 87` | 4: N-1 (a decal-placing function) | found | 0xB33ED1 |
| D-E-L15 | `E8 ? ? ? ? 3D 03 01 00 00 7F` | 6: N-1 (the decal-placing function with a short jump) | found | 0xB34CD5 |
| D-E-L16 | `89 C3 31 C0 81 FB 03 01 00 00 0F 8F` | 6: N-1 (a decal-placing function) | found | 0xB34F70 |

## RageOpenV

The RPF7 layout, which no pattern holds:

- **Header,** 16 bytes: the magic `RPF7` (the dword 0x52504637, the bytes `37 46 50 52`), the entry count, the names' length, the encryption; then the entry table, 16 bytes an entry (its fields in "When Game Updates" item 5), and the names, each ending in a zero, both plain in an OPEN archive.
- **Encryption:** `OPEN` (0x4E45504F) not encrypted; NG (`(encryption & 0x0FF00000) == 0x0FE00000`, by which the game's FindEncryption, called at `R-E-E1`, answers yes) or AES (0x0FFFFFF9) for the game's own; 0 names none at all.
- **An entry's kind,** by the dword at +4: 0x7FFFFF00 a folder, the first entry being the root; bit 31 (0x80000000), the top bit of its 3-byte block at +5, a resource; any other a binary.
- **Name offset:** the word at +0, into the names, so a file's name lies in their first 64 KB.
- **Resource:** version = (system >> 28) << 4 | (graphics >> 28), the top four bits of its system flags (the dword at +8) and of its graphics flags (+12).
- **Binary:** +2 the compressed size, 3 bytes (0 for one stored); +5 the offset, 3 bytes, in blocks of 512 bytes from the start of the archive that holds it; +8 the size, a dword; +12 the encryption of its bytes, a dword (0 for none).
  One whose block is 0x7FFFFF and whose compressed size's top byte is 0 would read as a folder, and one whose block has its top bit set as a resource.
- **Nested archive:** such a binary, named `.rpf`, stored, its bytes not encrypted, inside the archive that holds it, past that one's names.
- **Unencrypted archive:** the game expects 0x0FEFFFFF in the encryption field (+12) of every binary entry whose name offset and encryption field are not zero, and in the packfile's current file offset (+0xBC from the `fiPackfile` object's start, item 5).
- **Versions,** from the archives' tables: Enhanced's own files and my map mods `yft` 171, `ydr` and `ydd` 159, `ytd` 5; packs made for Legacy `yft` 162, `ydr` and `ydd` 165, `ytd` 13; both `ybn` 43, `ymap`, `ytyp` and `ymt` 2.
  Each Legacy vehicle trapped the game as it was created (0x80000003 at `GTA5_Enhanced.exe`+0x127C32F); converted with CodeWalker, they spawned.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| R-E-C1 | `ff 15 ? ? ? ? 48 85 c0 74 ? 48 89 c1 31 d2` | +16: the six-byte `call [rip+disp32]` (`FF 15`) that hides the console, which the console patch makes six NOPs (`90`), only with `CONSOLE=1` | found | 0x1F04 |
| R-E-K1 | `e8 ? ? ? ? 48 8d 0d ? ? ? ? 48 8d 15 ? ? ? ? 48 8d 35` | the RPF cache check: its callee starts with `41 57`, the two bytes `C3 90` replaces | found | 0x4F73AE |
| R-E-F1 | `41 56 56 57 53 48 81 ec ? ? ? ? 4c 89 c7 66 c7 ? ? ? ? ? 4c 8d 74 24` | OpenBulk 1 (function start) | found | 0x10B110 |
| R-E-F2 | `66 c7 01 ? ? 29 d9 d1 e9 66 89 4c 24 ? 31 ff 4c 8d 44 ? ? 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0` | GetFileTime 1 = match - 0x10D | found | 0x10BA3D |
| R-E-F3 | `66 c7 01 ? ? 29 d9 d1 e9 66 89 4c 24 ? 31 ff 4c 8d 44 24 ? 48 89 d9 31 d2 ff 15 ? ? ? ? 8b 4c 24` | GetFileSize 1 = match - 0x10D | found | 0x10B8ED |
| R-E-F4 | `41 56 56 57 53 48 81 ec ? ? ? ? 66 c7 ? ? ? ? ? 4c 8d 74 24` | GetAttributes 1 (function start) | found | 0x10BC10 |
| R-E-F5 | `41 57 41 56 56 57 53 48 81 ec 60 02 00 00 48 89 d3 48 89 cf` | OpenBulk 2 (function start) | found | 0x107F60 |
| R-E-F6 | `66 c7 01 00 00 29 d9 d1 e9 66 89 4c 24 70 4c 8d 44 24 48 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0 74 14 8b 44 24 64 8b 4c 24 68 48 c1 e0 20 48 09 c1 48 89 4c 24 38` | GetFileTime 2 = match - 0x15D | found | 0x10887D |
| R-E-F7 | `66 c7 01 00 00 29 d9 d1 e9 66 89 4c 24 70 4c 8d 44 24 48 48 89 d9 31 d2 ff 15 ? ? ? ? 85 c0 74 0a 48 8b 44 24 5c 48 89 44 24 38` | GetFileSize 2 = match - 0x15D | found | 0x108A6D |
| R-E-F8 | `56 57 53 48 81 ec 60 02 00 00 48 89 d7 48 89 ce c7 44 24 3c ff ff ff ff 48 89 d1 ba 3a 00 00 00` | GetAttributes 2 (function start) | found | 0x108C40 |
| R-E-E1 | `e8 ? ? ? ? 80 7c 24 2e ? 74 ? 48 8b 56` | FindEncryption call | found | 0x10FEF2 |
| R-E-E2 | `e8 ? ? ? ? 8b 46 ? 85 c0 74 ? b9` | DecryptHeader call | found | 0x10FF08 |
| R-E-E3 | `e8 ? ? ? ? 48 8b 46 ? 66 c7 00 00 ? eb` | first DecryptHeader2 call | found | 0x10FFAE |
| R-E-E4 | `e8 ? ? ? ? 48 8b 46 ? 8b 4c 24 ? 48 8d 14 08` | second DecryptHeader2 call (same callee as R-E-E3) | found | 0x11001C |
| R-E-E5 | `c6 86 ? ? ? ? ? 48 89 f1 4c 89 fa 45 89 f0 45 31 c9` | +19: ParseHeader call | found | 0x10F5B0 |

## StraightToStoryMode

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| S-E1 | `0F 85 A9 00 00 00 48 8D 54` | splash screen: `jz` at match - 0x0D becomes `jmp` | found | 0x52DC67 |
| S-E2 | `00 E9 ? 05 00 00 E8 ? 07` | legal screen: the 6-byte `jz` (`0F 84`) at match - 0x0F is NOPed | found | 0x52F040 |
| S-E3 | `E8 ? ? ? ? 8B 0D ? ? ? ? 83 C1 ? 84 C0` | landing page: its callee returns 0 | found | 0x1166F |

## Script programs

A script program is one of the game's compiled scripts, which its archives store encrypted, as the game holds it in memory.
The first program of all comes about 24 seconds before `blip_controller` in the one boot that logged both.
Findings, here and for "Blips": `blips_findings_for_the_clean_room_build.txt`.

- **`Y-E-H1`'s callee** takes the program alone and returns a byte, which the constructor ignores: 1 when every native resolved, 0 when one did not.
  Before the call the native table holds the natives' 64-bit hashes, after it each entry its handler's address; a handler can serve more than one hash, and a hash the game does not know gets the handler at RVA 0x50CB60, `CC C3` (an `int3`, then a `ret`).
- **Native table,** in a script file: each hash rotated right by (code length + index) mod 64; the constructor rotates them back before the call (`Y-E-N1`).
- **Thread:** the constructor runs on the game's one resource placement thread whenever a script program is streamed in; loading a save places `blip_controller` again, a new program object, its code and tables the same (the 48th program at the start and the 137th after Load Game in one boot).
  The program enters the game's registry of programs only after the call, and every script thread looks its program up there each time it runs (a thread whose program is not there faults rather than runs it), so none can run it during the call: the window of T-FREEZE (ISSUES.md) does not arise.
- **Fields,** from the program's start: +0x10 the code page table (8-byte pointers, one per page), +0x1C the code length in bytes (dword), +0x2C the native count (dword), +0x40 the native table (count x 8 bytes), +0x58 the name's hash (dword), +0x60 the name (a pointer to a zero-terminated string), +0x68 the string page table, +0x70 the strings' length in bytes (dword); the object is at least 0x80 bytes.
- **The constructor's loop over the strings' pages:** `mov r14d, 0x3FFF` at RVA 0x91BBB0, `mov eax, [r12+0x70]` at 0x91BBB6, `add eax, r14d; cmp eax, 0x4000` at 0x91BBBB; the loop at 0x91BD1B to 0x91BD37, `shr eax, 14` and the page table read at +0x68.
- **Pages:** 0x4000 bytes each, the strings' too; a length gives (length + 0x3FFF) >> 14 pages, every one full but the last, which holds length & 0x3FFF bytes (0x4000 when that is 0).
- **Name's hash,** the game's own function (RVA 0x91B6A0): over the name's bytes - a leading double quote skipped, the name then ending at the next one; A to Z lowered and `\` taken as `/`; each byte sign-extended - h = (h + c) * 0x401 and h ^= h >> 6, all in 32 bits; then h *= 9, h ^= h >> 11, h *= 0x8001; a result below 2 gets 2 added.
  `blip_controller` gives 0x80CFE974.
- **Interpreter** (RVA 0x91DD30): fetches the byte at ip through the code page table, ip >> 14 the page, ip & 0x3FFF the offset in it, and dispatches through the table at .rdata 0x2610450 over 131 opcodes, 0x00 to 0x82; a byte above 0x82 is a fatal trap.
  Every instruction lies within one page, except SWITCH, whose entry table it reads from the page of ip + 2, so a page's end may lie between a SWITCH's count byte and its table.
- **Lengths,** the game's own table at .rdata 0x2610C50, the dispatch table + 0x800, one byte per opcode: 0x00..0x24 1; 0x25 2; 0x26 3; 0x27 4; 0x28..0x29 5; 0x2A..0x2B 1; 0x2C 4; 0x2D 0; 0x2E 3; 0x2F..0x33 1; 0x34..0x3E 2; 0x3F 1; 0x40..0x42 2; 0x43..0x5C 3; 0x5D..0x64 4; 0x65 0; 0x66..0x67 1; 0x68..0x6B 2; 0x6C..0x82 1 - 0 for the two of variable length, ENTER (5 bytes and its name's, whose length is the byte at +4) and SWITCH (2 bytes and 6 for each entry, whose count is the byte at +1).
- **Opcodes a matcher needs,** every operand of more than one byte little-endian but NATIVE's index: NOP 0x00; NOT 0x06; the signed 32-bit compares IEQ 0x08, INE 0x09, IGT 0x0A, IGE 0x0B, ILT 0x0C and ILE 0x0D; PUSH_CONST_U8 0x25, PUSH_CONST_U8_U8 0x26, PUSH_CONST_U8_U8_U8 0x27, PUSH_CONST_U32 0x28 and PUSH_CONST_F 0x29; NATIVE 0x2C (+1 the argument count << 2 | the result count, +2 the native's index, big-endian); ENTER 0x2D and LEAVE 0x2E; PUSH_CONST_S16 0x43 (signed); J 0x55 and JZ 0x56 (a signed 16-bit offset from ip + 3), and the fused IEQ_JZ to ILE_JZ 0x57 to 0x5C, which jump when their comparison is false; CALL 0x5D (a 24-bit address in the code, unsigned); PUSH_CONST_U24 0x64; SWITCH 0x65 (n entries of a 32-bit value and an unsigned 16-bit offset from the entry's end); CATCH 0x6D, THROW 0x6E and CALLINDIRECT 0x6F, whose targets are computed while the script runs; PUSH_CONST_M1 0x70 and PUSH_CONST_0 to PUSH_CONST_7 0x71 to 0x78.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| Y-E-H1 | `4C 89 E1 E8 ? ? ? ? 41 BE FF 3F 00 00 41 8B 44 24 70` | +3: the `E8 rel32` (RVA 0x91BBAB) to the function that resolves a script program's natives (RVA 0x91BF20), in the program's resource constructor (RVA 0x91B9F0), rcx = the program (`mov rcx, r12` at +0); +18: 0x70, the offset of the strings' length (`mov eax, [r12+0x70]`), and +10: 0x3FFF, the page rounding (`mov r14d, 0x3FFF`) - the constructor's loop over the strings' pages after the call | found | 0x91BBA8 |
| Y-E-N1 | `41 8B 4C 24 1C 01 C1 49 8B 54 24 40 48 D3 04 C2` | +4: 0x1C, the offset of a program's code length (`mov ecx, [r12+0x1C]`); +11: 0x40, the offset of its native table (`mov rdx, [r12+0x40]`) - the constructor's loop that rotates the natives' hashes back | found | 0x91BAA0 |
| Y-E-I1 | `4D 8B 50 10 49 63 41 1C 4C 8D 7C C1 F8 49 63 41 18 4C 8D 0C C1` | +3: 0x10, the offset of a program's code page table (`mov r10, [r8+0x10]`) - the script interpreter's prologue | found | 0x91DD50 |

## Blips

- **Pool** (`L-E-P1`, its mark `L-E-P2`): a blip's handle is (serial << 16) | index, and the pool's code bounds an index with 0x7CF.
- **Count native** (hash 0xC802478C8862BDAD, handler at RVA 0x1D51F40, which starts with a jump into the game's protected code): counts the slots below the mark that hold a blip, except three: the slot whose index the dword at RVA 0x288FEF0 holds, and those of the handles at RVA 0x3DEB7F8 and 0x3DEB7F4, these two only while the byte at RVA 0x3DEB7F0 is set.
  The pool's two further sites, for those three, have no pattern.
  With all 2,000 slots taken it answers 1,997 while the three name live blips, and 1,998, 1,999 or 2,000 when one or more of them names none.
  So 1,997 is the highest limit that holds with the pool full: above it the script can pass the guard then, where making a blip fails.
- **WAIT:** hash 0x4EDE34FBADD967A6, handler at RVA 0x920EA0.
- **Guard** in `blip_controller`, read from the copy `TEST_DUMP_SCRIPT` saves (11,394 bytes of code in one page, 55 natives, 1,469 bytes of strings): 21 bytes at 0x3FE of its code, the count native its index 10, WAIT its index 47:

  | Offset | Bytes | Instruction |
  |---|---|---|
  | 0x3FE | `2C 01 00 0A` | NATIVE, 0 arguments, 1 result, index 10: the count |
  | 0x402 | `25 96` | PUSH_CONST_U8 150: the threshold |
  | 0x404 | `0C` | ILT |
  | 0x405 | `06` | NOT |
  | 0x406 | `56 0A 00` | JZ +10, to 0x413: the exit |
  | 0x409 | `43 D0 07` | PUSH_CONST_S16 2000 |
  | 0x40C | `2C 04 00 2F` | NATIVE, 1 argument, no result, index 47: WAIT |
  | 0x410 | `55 EB FF` | J -21, back to the count at 0x3FE |
  | 0x413 | `38 04` | after the guard: its way out, where the JZ leads - a 2-byte instruction, neither a NOP nor a jump |

  It holds the script back while count >= 150 and lets it on to make a blip as soon as count < 150.
  The count native is called nowhere else in the program, 150 is pushed nowhere else, no jump, call or SWITCH case lands in 0x402..0x408, and no target is computed (the program holds no CATCH, THROW or CALLINDIRECT).
  Told apart by their argument and result counts alone, the guard's NATIVE is the only one with 0 arguments and 1 result that a constant push and a comparison follow.
- **Threshold:** the game's own is 150; the program holds no 240, the value some setups show, and where that comes from is open.
- **Room:** the 4 bytes 0x402..0x405 between the count's NATIVE and the JZ, `25 96 0C 06`.
  PUSH_CONST_S16 and IGE fill them for any limit up to 32,767 (`43 lo hi 0B`, for 1900 `43 6C 07 0B`), and NOP, PUSH_CONST_U8 and IGE for one up to 255 (`00 25 nn 0B`, for 200 `00 25 C8 0B`): IGE in the place of ILT and NOT keeps the polarity, and the JZ keeps its target.
- **Not settled:** which of the game's two copies of the script it runs, update2.rpf's, resource version 12, or x64b.rpf's, 10; the part reads either as placed.
  A lead: the script store is built with 12 at its +0x2C (RVA 0x13FA29D and 0x30E40).

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| L-E-P1 | `BA 04 00 00 00 31 C0 48 8D 1D ? ? ? ? 31 F6` | +10: the rel32 of `lea rbx, [rip+disp32]` (`48 8D 1D` at +7) in the pool's allocator - the blip pool, 2,000 slots of 8 bytes (RVA 0x3DEBD70) | found | 0x55D897 |
| L-E-P2 | `48 89 3B 39 35 ? ? ? ? 7F 09 8D 46 01 89 05` | +5: the disp32 of `cmp [rip+disp32], esi` (`39 35` at +3) in the pool's update of its high-water mark - the mark, a dword (RVA 0x3DEFBF0) | found | 0x55DA0E |

## WeaponLimits

### Weapon infos

A `weapons.meta` file is a data file of the type 0x4E (the type's name is not in the dump), parsed into one weapon info blob of 0xF8 = 248 bytes, its vtable 0x26D0940 at +0.
Findings: `weaponinfos_findings_for_the_clean_room_build.txt`.

- **Array:** RVA 0x47CE480 in `.data`: +0 the blobs' one block on the game's heap, +8 the count loaded, +0xA the capacity, both 16-bit, so 65,535 at most.
  It lies in the weapon info manager, a global at 0x47CE430..0x47CE4C8 with seven more arrays of that shape, all sized at run time from what the blobs hold.
- **Limit:** two immediates, 160 (Legacy had 90 at v1365, by the original's page; a boot of mine counted 155 `weapons.meta` files), in the C++ static initializer 0x18956A0, index 1,103 of the CRT's table of C++ initializers at `.rdata` 0x2838700, which no call reaches: 0x1897A80 `mov edx, 160`; `call 0x1893AC0`, which allocates and constructs that many blobs, n x 0xF8 bytes, alignment 8, through the game's allocator 0x18B0, the number from `edx` alone; 0x1897A8A the block's store; 0x1897A91 `mov word [capacity], 160`.
  The manager's exit destructor (0x1863370, registered at 0x1897AA1) destroys `capacity` blobs and frees the block.
- **Loader** 0x1863B40 (761 bytes), a function start by its unwind data (`41 57 41 56 41 55 41 54 56 57 53 48 81 EC 80 01 00 00`, seven pushes and `sub rsp, 0x180`): adds a blob with no check, the count + 1, the blob at the block + the old count x 248, the file parsed into it (the parser's call at 0x1863DF7).
  Past the capacity it writes into memory the array never had: a heap corruption, the game's crash while loading or later.
  It copies the data file entry's first 0x80 bytes and more onto its stack.
- **Callers:** the manager's init 0x18634A0 (every type-0x4E file of the data file list, its call at 0x1863552) and the load handler 0x188E4A0 (packs mounted later: `48 89 D1 48 89 C2 E8 ? ? ? ? EB 08 48 89 D1 E8 ? ? ? ? B0 01` at 0x188E4CC, its call of the loader at +6 for a type-0x4E file, of 0x1863E40 at +16 for a type-0xA3 one, then `mov al, 1` whatever the loader returned); the unload handler 0x188E4F0 takes a pack's blobs back again (0x18641A0, the count - 1).
- **Order:** the initializer runs after TweaksV's `DllMain`: in the table the heap's initializer, `H-E1`'s 0x1000 -> 0x1020, is index 3 and the heap adjusters work at `DllMain`, and my boot read the array's pointer and capacity still 0 at TweaksV's start.
- **References:** exactly 2 instructions of the kind `FindRipRelativeReferences` finds load or store the array's address, 0x1897A1E, the `lea rcx` of the allocation's unused argument, and 0x1897A8A, the block's store.
- **Traps:** shorter forms of `W-E-I1` match the allocate-and-store idiom of many reserved arrays (10 and more); with the number fixed (`BA A0 00 00 00 ...`) it is unique but would break when Rockstar raises it; `W-E-I2` without its last 11 bytes also matches once.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| W-E-I1 | `C7 05 ? ? ? ? 00 00 00 00 BA ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? ? ? 48 8D 0D` | in the initializer: +11 the imm32 of `mov edx` (`BA` at +10) - the blobs allocated, 160; +16 the rel32 of the call to the allocation 0x1893AC0 (`E8` at +15), which begins `56 57 48 83 EC 28 89 D6 48 63 FA 48 69 CF`, the element size, 0xF8, after it; +23 the disp32 of `mov [rip+disp32], rax` (`48 89 05` at +20) - the array (match + 27 + disp); +30 the disp32 of `mov word [rip+disp32], imm16` (`66 C7 05` at +27) - the capacity's address, the array + 0xA (match + 36 + disp: the imm16 follows the disp32); +34 the imm16 - the capacity, 160, equal to +11's | found | 0x1897A76 |
| W-E-I2 | `0F B7 05 ? ? ? ? 8D 48 01 66 89 0D ? ? ? ? 4C 69 E8 ? ? ? ? 4C 03 2D ? ? ? ?` | in the loader: +3 and +13 the disp32 of the count's `movzx eax, word [rip+disp32]` and `mov word [rip+disp32], cx` - the array + 8 (match + 7 + disp, match + 17 + disp); +20 the imm32 of `imul r13, rax, imm32` - the blob's size, 0xF8, equal to the allocation's; +27 the disp32 of `add r13, [rip+disp32]` - the array (match + 31 + disp); the match - 0x18 is the loader's start 0x1863B40, a function start by its unwind data | found | 0x1863B58 |

On 1.0.1158.16, beyond the rows: the immediates at 0x1897A81 (the blobs allocated) and 0x1897A98 (the capacity); the allocation call at 0x1897A85 to 0x1893AC0; the block's store at 0x1897A8D to the array 0x47CE480, the capacity's store at 0x1897A94 to 0x47CE48A; the initializer's `lea` at 0x1897A21; the loader's count read at 0x1863B5B to 0x47CE488; the blob size at 0x1863B6C, holding 0xF8.

### Weapon components

Findings: `weaponcomponents_findings_for_the_clean_room_build.txt`; for the loader `wcloader_findings_for_the_clean_room_build.txt`.

- **List** (`W-E-C1`): every weapon component definition of all `weaponcomponents.meta` files together, sorted, a static array of pointers in `.data` with its count, an int32, right after it.
  Its size is that distance, (0x47CD3E8 - 0x47CC538) / 8 = 470, and no instruction carries it.
  15 binary searches, inlined and bounded by the count, read it.
- **Rebuild** (RVA 0x188E320, the call operator of the callbacks the data file mounter, 0x188E2E0, and the base loader, 0x1856210, hand the parser; their vtables 0x26D0860 and 0x26CE390, entry [2]): runs after every `weaponcomponents.meta` loaded, sets the count to 0, copies every file's definitions in without a bound and sorts with qsort, 8-byte entries by the uint32 hash at +0x10 of each definition (the comparison at 0x1838F50).
  The 471st definition is stored over the count itself, and the copy then writes through the garbage: memory corruption while the game loads.
- **Loads:** exactly 17 `lea r64, [rip+disp32]` with REX 48 or 4C, the rebuild's two and the searches' 15; nothing else references the list or into it.
  In the game's `.text` RVA 0x797B97, 0x12AEE53, 0x155F107, 0x185018F, 0x1867509, 0x1868340, 0x1874229, 0x188E33F (`W-E-C1`), 0x188E3BD (`W-E-C2`), 0x18EA02B, 0x1BE9282; in its second, protected `.text` 0x53855A5, 0x53E8524, 0x53F44AF, 0x572C229, 0x59392B1, 0x596C91F.
- **Pool:** each definition is an object in one slot of 192 bytes: nine classes, made by nine factory functions (0x1857110 to 0x1857D80), the largest 0xC0 bytes.
  Its maker (0x1855FB0) asks the game's configuration for its size by the hash of its name, "cweaponcomponentinfo", 470 by default (`W-E-C4`): `gameconfig.xml`'s value when the file has the entry - or the larger of it and the default while the byte at 0x2E96FA8 is set, which no code in the dump writes - else the default.
  The game's own `gameconfig.xml` has no entry for this pool, so it keeps 470.
  Its address is kept obfuscated at 0x47CD890; its memory comes from the allocator of "A pool's memory".
  Its fields: +0x08 the slots, +0x10 one flag byte a slot, +0x18 the slot count, +0x1C the slot size, +0x20 and +0x24 the free list, +0x28 the slots in use in its low 30 bits.
  A full pool ends the game in an `int3` (the pool's New, 0x15A7C0) while the definition is created: while the pool is no larger than the list, the game traps at the 471st definition before the list could overflow, and a `gameconfig.xml` that gives the pool more turns that trap into the list's silent overflow.
- **Files:** each `weaponcomponents.meta`, of the data file type 0x4F, has its blob of 0xED8 bytes in the array of blobs (`W-E-C5`; its count and capacity uint16); a blob's definitions lie in a fixed array of 470 pointers (+0x10 to +0xEC0, the count at +0xEC0), so one file holds at most 470.
- **Loader** of one file (`W-E-C5`): takes rcx = the file's entry in the data file manager's array (0xA8 bytes: the file's name first, NUL-terminated before +0x90; +0x90 the data file type; +0x94 its index; +0x9B 0 loaded, 1 unloaded) and rdx = a callback passed by value (its callable's pointer at +0x38), which it destroys before it returns.
  It allocates the file's blob, appends it to the array, queues the file's parse with the parser's load 0xAC6B30 and makes no definition itself.
  Its callers, no other reference: the mounter's load (the call at 0x188E2FD) and the base loader (0x18562C2).
  The parse runs in the parser's request runner 0xAC30E0, which the change-set apply 0xC2E0B0 runs after every action, so a mounted file is parsed, and the list rebuilt, before the next file comes.
  The base loader clears every blob, leaves the list's count, queues all its files and the rebuild last.
  The game never unloads a weapon components file (the mounter's unload, vtable 0x26D0840 [2], is a plain return).
- **Not settled:** what the parser does with more than 470 definitions in one file; the use of the definitions' uint16 at +0x08, 0xFFFF when made.
- **Trap:** `W-E-C5` without its last 17 bytes matches once too.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| W-E-C1 | `C7 05 ? ? ? ? 00 00 00 00 44 0F B7 05 ? ? ? ? 66 45 85 C0 0F 84 ? ? ? ? 45 31 C9 4C 8D 15 ? ? ? ?` | the rebuild: +34 the rel32 of `lea r10, [rip+disp32]` (`4C 8D 15` at +31) - the weapon component list (RVA 0x47CC538: 470 entries of 8 bytes); +2 the disp32 of `mov dword [rip+disp32], 0` (the instruction ends at +10) - the list's count (0x47CD3E8); +14 the disp32 of `movzx r8d, word [rip+disp32]` - the number of `weaponcomponents.meta` files loaded (0x47CD3F8) | found | 0x188E320 |
| W-E-C2 | `48 63 15 ? ? ? ? EB 02 31 D2 48 8D 0D ? ? ? ? 4C 8D 0D ? ? ? ? 41 B8 08 00 00 00` | the sort: +14 the rel32 of `lea rcx, [rip+disp32]` (`48 8D 0D` at +11) - the list; +3 the disp32 of `movsxd rdx, dword [rip+disp32]` - the count; +27 the imm32 8 - the bytes an entry (`41 B8` at +25); the `jmp [rip+disp32]` to qsort follows at +31 | found | 0x188E3B2 |
| W-E-C3 | `48 63 15 ? ? ? ? 8D 4A 01 89 0D ? ? ? ? 4D 89 04 D2` | the rebuild's update of the count: +3 and +12 the disp32 of the count's load and store; +16 the unbounded store into the list (`mov [r10+rdx*8], r8`) | found | 0x188E385 |
| W-E-C4 | `48 8B 0D ? ? ? ? BA CF 3C E3 BA 41 B8 ? ? ? ? E8 ? ? ? ? 89 C3` | the pool's size: +18 the `E8 rel32` of the size lookup's call (RVA 0x1855FD8, to 0x149060); +8 0xBAE33CCF, the hash of the pool's name; +14 the imm32 of `mov r8d, imm32` - the default, 470; +3 the disp32 of the configuration's pointer (0x2E96FB0); +23 `mov ebx, eax`, the result kept as the slot count | found | 0x1855FC6 |
| W-E-C5 | `49 89 D7 49 89 CE B9 D8 0E 00 00 E8 ? ? ? ? 49 89 C4 41 B8 D8 0E 00 00 48 89 C1 31 D2 E8 ? ? ? ? 0F B7 1D ? ? ? ? 66 3B 1D ? ? ? ? 75 ? 44 0F B7 EB 49 8D 6D 10 48 8B 35 ? ? ? ?` | the loader of a `weaponcomponents.meta`: the match - 0x13 is its start, 0x1856740, a function start by its unwind data; +0x07 and +0x15 the imm32 of `mov ecx, imm32` (`B9` at +0x06) and `mov r8d, imm32` (`41 B8` at +0x13) - the blob's size, 0xED8, both; +0x26 the disp32 of `movzx ebx, word [rip+disp32]` (the instruction ends at +0x2A) - the blobs' count (0x47CD3F8), the one `W-E-C1` gives at +14; +0x2D the disp32 of `cmp bx, word [rip+disp32]` (ends at +0x31) - the blob array's capacity (0x47CD3FA); +0x3E the disp32 of `mov rsi, [rip+disp32]` (ends at +0x42) - the blob array's pointer (0x47CD3F0) | found | 0x1856753 |

On 1.0.1158.16 the default the pool size call is given lies at 0x1855FD4 and holds 470.

## SirenSettings

Its letter is `I`, for both limits (`S` is StraightToStoryMode's).
Findings, in my research tools' output folder, `<topic>_findings_for_the_clean_room_build.txt`: `sirenids`, `sirenfacts`, `sirencount`, `sirencount2`.

### Siren IDs

- **The limit:** an ID is 8 bits on the whole load path: a byte at +0x00 of a siren setting (`carcols`, 0x5E8 bytes) and at +0x4B of a vehicle variation (`carvariations`, 0x50 bytes).
  The ID table, 256 bytes at `.data` RVA 0x471E510, between the light ID table (RVA 0x471E410) and ModkitLimit's modkit array (RVA 0x471E610), maps an ID to its setting's position, a byte, 0xFF for a free ID.
  The position: the variation's +0x49, the model info's +0x53B (the light's +0x48 and +0x53A).
  255 means "no ID"; position 0 no sirens.
  So 256 IDs and 255 settings, positions 1 to 255: a setting placed at 256 or later is reached as another one.
- **The parser** cuts an XML value to its member's width: the simple member's reader (`I-E-P3`, jump table at RVA 0xAE4C58) stores the types 0x01 and 0x02 as a byte, 0x03 and 0x04 as a word, so an ID above 255 keeps its low byte: 300 is 44, 1534 is 254.
  The member data: +0x08 the member's offset, a qword, 0 at TweaksV's start, written from the structure's offsets table when the game registers it; +0x10 the type, a byte (0x0D an array, 0x0C a structure); +0x11 the subtype; +0x20 the element's size; +0x28 a fixed array's capacity; +0x30 the element's member data.
  Beside the IDs: a setting's member 2, next in memory, at +0x04; a variation's member 5, the light ID, at +0x4A, +0x4C..+0x4F unused.
  The member data lie in `.data`, their pointers and offsets (`I-E-P1`, `I-E-P2`) in `.rdata`, the pointers ending in a null (20 and 7 members).
  The registrations' hashes, in `.data`: `sirenSettings`' at RVA 0x471F310, `CVehicleVariationData`'s at 0x471F400, the neighbouring `sirenLight`'s at 0x471F2A0.
- **The PSO loader** (binary `.ymt` files, the base game's among them) loads member by member by name hash, each value read by the file's type and stored by the member's, so a member wider than its file's keeps the base game's numbers: my comparison boot, the same 27 settings, the base game's IDs the same, the packs' IDs whole.
  Through the base loader a binary `carcols.ymt`'s array is read by the file's count with no limit (RVA 0xACF6F0): past the runtime capacity it writes over the count, the name and the next setting, the game's own hazard.
- **The base load:** the vehicle metadata's init (`I-E-R1`) fills the light and siren ID tables with 0xFF, 32 `movdqa` of 16 bytes, the siren table's 16 from RVA 0x1349C6E to 0x1349CE6, then calls the base loader.
  That loads `carcols.ymt`, runs the builder (`I-E-H2`: kits, lights, sirens), loads `carvariations.ymt`, turns each variation's IDs into positions (`I-E-T1`), copies the positions into the model infos and frees the variations.
- **A pack's load:** its carcols through the data-file callback (`I-E-H1`; `.rdata` 0x266F060, slot 2), the call slot of the callable the carcols mount hands the parser: `rcx` that callable, `r8` a pointer to the parsed object's pointer; nothing is done with a false byte in `rdx`.
  A setting whose ID is loaded already is dropped (`I-E-D1`, bytes compared in `I-E-D2`), the others appended, the array grown by 16, and entered in the table (`I-E-T4`).
  The settings are not in memory as the callback starts: the dedupe's load (`I-E-D1` +0x0B) is the one place that sees all of them, and what the pattern pins after it, a `movzx` and a compare, leaves the flags free for a stub there.
  Its carvariations through `I-E-H4` (`.rdata` 0x266F030, slot 2; `I-E-T3`).
  Within one file a setting whose ID is taken moves to the first free ID.
  All of it silent.
- **A pack's file's name:** inline at +0 of its data file entry, 128 bytes with its NUL (the manager at .data 0x29CAFC0, +0 the array, stride 0xA8), which moves as the array grows and swap-removes.
  The dispatcher (RVA 0xC39440) hands it to the carcols mount (RVA 0x13730D0), through RVA 0x134AFE0 into the file opener (RVA 0xAC6B30), which keeps no name.
  Then the pump (RVA 0xAC30E0) calls the callback, one file at a time, each with exactly one such call before it for the same object; a read that cannot start calls the callback inside the opener.
  A hint: the text is the path the pack's content file gives, a RAGE device path, no user folder.
- **The carcols data** (its pointer in `.data`, `I-E-T4`): +0x38 and +0x40 the light settings; +0x48 the siren settings' array, +0x50 their count, +0x52 their capacity, words; +0x58 and +0x60 the kits.
  The array grows by 16 (RVA 0x137A3D0), the spare entries constructed; the pointer is stored first and the count after (RVA 0x1378210), so a thread reading the count before the pointer never pairs a new count with an old array.
  A position is valid when given and the count never falls: below the count is the one right bound.
- **After the load** a setting is reached only by its position and its ID is not read again: every multiplication by 0x5E8 in the code lies in the load path, the getter and the array's growth.
- **The getter** (`I-E-M1`, 34 bytes, no unwind data): position 0 gives null; else array + position x 0x5E8, no upper bound, so 0xFF, an ID that found no setting, reads array + 0x5E218, past the array unless its capacity is 256 or more.
  It touches rax and rcx alone; neither caller holds anything in a volatile register across it.
  Its only calls, with no other reference and no absolute pointer: RVA 0xE9BD4E in the renderer (`I-E-Z1` +24) and RVA 0xE16FA7 right after the renderer returns (`I-E-B1` +24), which uses the setting's +0x24, +0x28, +0x2C and +0x30, each behind a test for null, null and zero taking the same branches.
  A position comes from [variation+0x49] = table[ID] alone (RVA 0x13780D8 and 0x134A233), so a variation naming a setting not loaded yet keeps the free entry's 0xFF.
  A hint, not walked: the light settings' getter (RVA 0x135D200) has the same unbounded shape, at one call (RVA 0xE161C2).
- **The "has sirens" bit:** bit 1 of the vehicle's +0x96B, set from the model info's +0x53B at RVA 0xDCA9DD, computed at RVA 0xDCA9F5; only RVA 0xE15B20 goes on from it to the setting, by the getter's two calls, while the sirens are on.
- **The constructors** (`I-E-C1` to `C3`): the setting's default ID at RVA 0x13635D0, its +0x1C written at 0x1363538 and again at 0x13635F9; the variation's two leave +0x44..+0x47 and +0x4C..+0x4F unwritten.
- **Code the protection reads:** two functions of the protected `.text` section read the 4 gap bytes at RVA 0x135D21A, right before the getter (a check of the bytes between functions, it seems); no site of the part writes a gap byte.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| I-E-P1 | `C7 05 ? ? ? ? 0D 9B EB 84 0F 57 C0 0F 11 05 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05` | the siren settings' parser registration (no unwind data): +6 the name hash 0x84EB9B0D, `sirenSettings`; +23 the rel32 of `lea rax` - the member data pointers (RVA 0x266EBB0); +37, past the match, the next `lea rax`'s - the members' offsets (RVA 0x266EB50); member 0 is the ID at +0x00, its data at RVA 0x28EE130 | found | 0x1363240 |
| I-E-P2 | `C7 05 ? ? ? ? A9 FE EA 18 0F 57 C0 0F 11 05 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05` | the vehicle variations' parser registration: +6 the hash 0x18EAFEA9, `CVehicleVariationData`; +23 the member data pointers (RVA 0x266ECB0); +37 the offsets (RVA 0x266EC90); 7 members, member 6 the siren setting ID at +0x4B, its data at RVA 0x28EE7B8 | found | 0x1363850 |
| I-E-P3 | `48 8B 41 08 0F B6 48 10 48 83 F9 21 0F 87` | the simple member's XML reader (RVA 0xAE4710): +3 0x08, the member data in the member; +7 0x10, the type in the member data; +11 0x21, the highest type | found | 0xAE4723 |
| I-E-T1 | `B9 4B 00 00 00 31 D2 4C 8D 05 ? ? ? ? 4C 8D 0D` | the base loader's loop over the variations: +10 the rel32 of `lea r8` - the ID table (RVA 0x471E510); +0x93 `0F B6 2C 0F` and +0xC4 `0F B6 1C 2B` the ID's byte loads, +0x97 `48 81 FD FF 00 00 00` its check for none, +0xC8 `88 1C 0F` the ID stored into the variation | found | 0x1349F1D |
| I-E-T2 | `41 0F B7 46 50 48 85 C0 0F 84 ? ? ? ? 49 8B 4E 48 31 D2 48 8D 1D` | the base builder's siren part: +4 0x50, the settings' count; +17 0x48, their array; +23, past the match, the rel32 of `lea rbx` - the ID table; +0x29 `40 88 31` a new ID's byte store, +0x30 and +0x4B `0F B6 2C 39` the loads, +0x4F `48 81 FD FF 00 00 00` the check for none, +0x85 and +0xCD `48 81 FE 00 01 00 00` the free-ID searches' limit, 256, +0xD6 `40 B6 FF` none when none is free | found | 0x134A79A |
| I-E-T3 | `B8 4B 00 00 00 31 C9 4C 8D 05 ? ? ? ? 48 8D 2D` | a pack's loop over its variations (RVA 0x1377E00): +10 the rel32 of `lea r8` - the ID table; +0x8D `0F B6 3C 03` and +0xBE `0F B6 14 17` the loads, +0x91 `48 81 FF FF 00 00 00` the check for none, +0xC2 `88 14 03` the store into the variation | found | 0x1377E33 |
| I-E-T4 | `4C 8B 05 ? ? ? ? 49 8B 58 48 89 CA 48 8D 2D` | a pack's builder: +3 the rel32 of `mov r8, [rip+disp32]` - the carcols pointer (RVA 0x471D920); +10 0x48; +16, past the match, the rel32 of `lea rbp` - the ID table; +0x21 `40 88 33` the store, +0x28 `0F B6 0C 3B` and +0x42 `0F B6 34 3B` the loads, +0x46 `48 81 FE FF 00 00 00` the check for none, +0x81 and +0xC9 `48 81 FE 00 01 00 00` the limit, +0xD2 `40 B6 FF` | found | 0x137880E |
| I-E-D1 | `4D 8B 6F 48 4D 69 FE E8 05 00 00 43 0F B6 54 3D 00 0F B7 7B 50 80 FA FF 74` | a pack's dedupe in its carcols callback (RVA 0x1378210): +0x0B `43 0F B6 54 3D 00` the pack's ID, a byte; +0x15 `80 FA FF` its check for none, on the low byte | found | 0x1378428 |
| I-E-D2 | `31 F6 66 2E 0F 1F 84 00 00 00 00 00 38 54 35 00 0F 84 ? ? ? ? 48 81 C6 E8 05 00 00 48 39 F3 75 EA` | the dedupe's compare loop at +12, `cmp [rbp+rsi], dl`, after a 10-byte NOP at +2: +18 the rel32 of its `jz` (to RVA 0x137857F); its `jnz` back to +12 | found | 0x1378454 |
| I-E-C1 | `56 48 83 EC 20 48 89 CE C7 41 1C 00 00 00 00 B8 83 00 00 00 0F 57 C0` | the siren setting's constructor: +8 `C7 41 1C 00 00 00 00`, a store at +0x1C that +0xC9 (`89 46 1C`) makes again; +0xA0 `C6 06 FF`, the ID's default | found | 0x1363530 |
| I-E-C2 | `C7 41 40 00 00 00 00 C7 41 48 00 00 FF FF C3` | the variation's in-place constructor (RVA 0x13639C0, no unwind data): its stores from -0x0F, `C7 41 30 00 00 00 00 48 C7 41 38 00 00 00 00`, then +0x40 zero and at +0x48 the indices 0 and the IDs 0xFF, `ret` | found | 0x13639FF |
| I-E-C3 | `C7 40 40 00 00 00 00 C7 40 48 00 00 FF FF 48 83 C4 28 C3` | the variation's allocating constructor (RVA 0x1363960): the same stores through `rax` from -0x0F, then `add rsp, 0x28` and `ret` | found | 0x13639AD |
| I-E-R1 | `66 0F 7F 05 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 80 3D` | the vehicle metadata's init (RVA 0x1349BD0): +4 the disp32 of the last `movdqa` that fills the ID table, its last 16 bytes (RVA 0x471E600); +13 the call of the base loader (RVA 0x1349E40) | found | 0x1349CE6 |
| I-E-M1 | `0F B6 81 3B 05 00 00 48 85 C0 74 13 48 8B 0D ? ? ? ? 48 69 C0 E8 05 00 00 48 03 41 48 C3` | the getter of a model's siren setting (no unwind data): +3 0x53B, the model info's siren index, a byte; +15 the rel32 of `mov rcx, [rip+disp32]` - the carcols pointer (RVA 0x471D920); +22 0x5E8, a setting's size; +29 0x48, the settings' array | found | 0x135D220 |
| I-E-H1 | `41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 68 80 3A 00 0F 84 ? ? ? ?` | a pack's carcols callback, hooked at its start with `DIAGNOSTIC_MODE` and the part at work: `rdx` the byte that says the file parsed, `*r8` the parsed object; `I-E-D1`'s and `I-E-T4`'s code lies inside it | found | 0x1378210 |
| I-E-H2 | `41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 28 49 89 CE 0F B7 71 10 89` | the base builder, hooked at its start the same: `rcx` the carcols data, the file's IDs in its settings as it starts; `I-E-T2`'s code lies inside it | found | 0x134A3A0 |
| I-E-H3 | `4C 8B 0D ? ? ? ? 48 89 7C 24 40 48 8D 44 24 50 48 89 44 24 28 48 89 5C 24 20 C6 44 24 38 01 C6 44 24 30 01 4C 89 E1 4C 89 FA 4D 89 F0 E8 ? ? ? ? 89 C3` | in a pack's carcols mount (RVA 0x134AFE0), redirected with `I-E-H1`: +46 the call of the game's file opener (RVA 0xAC6B30), its nine arguments the parser in `rcx`, the file's name in `rdx`, "meta" in `r8`, the parent's structure in `r9`, the parsed object at `[rsp+0x20]`, a callable at `[rsp+0x28]`, the bytes 1 and 1 at `[rsp+0x30]` and `[rsp+0x38]`, 0 at `[rsp+0x40]`, the result in `eax`; +3 the rel32 of the parent's structure pointer (RVA 0x471E160) | found, its RVA to check | 0x134B088 |
| I-E-H4 | `41 57 41 56 41 55 41 54 56 57 55 53 48 81 EC D8 00 00 00 0F 29 B4 24 C0 00 00 00 80 3A 00 0F 84 ? ? ? ? 4D 8B 28 66 41 83 7D 08 00` | a pack's carvariations callback, hooked at its start with `DIAGNOSTIC_MODE` and the part at work, after the game's function: `rdx` the byte that says the file parsed, `*r8` the parsed object, its variations at +0, 0x50 bytes each, their count a word at +8, a variation's model's name's pointer at +0 and its setting's position at +0x49 | found, its RVA to check | 0x1377E00 |

### Siren count

The game lights at most 20 sirens of a vehicle, `siren1` to `siren20`, in four layers: a setting's 20 lights, a vehicle's siren state of 20, its renderer and six more loops over 20, and the bones' hierarchy's IDs for 20 sirens and their glass.
A vehicle made for more loads, and its bones past `siren20` are never looked up: those lights stay dark, without an error.
The raise: SPECIFICATIONS.md, section 11.14.1, at length `sirencount2_findings_for_the_clean_room_build.txt`, section 8.7.
The 32-bit masks of the siren state, of the vehicle's broken sirens and of a damage object's hidden sirens make 32 its top.

- **The renderer** (RVA 0xE9BCB0, read in full): its one caller, RVA 0xE16F9C in the vehicle lights' function (RVA 0xE15B20), run from the job at RVA 0xE15900 on worker threads, calls it once per pass of a vehicle whose sirens are on (HINT, from the data flow, not verified: while the vehicle's +0x96B has bit 1 set and its +0x985 & 0xC0 is 0x40).
  Nine arguments: rcx the vehicle; xmm1 to xmm3 three floats; 5th, [rsp+0x20], a flag byte; 6th, [rsp+0x28], the model info; 7th, [rsp+0x30], the skeleton; a dword; 9th the job's list, of coronas and lights, each with its capacity check.
  Its loop runs over 20 lights by its own constant, whatever a setting's count says, and skips a light whose bone is missing or that the vehicle's +0x860 marks broken.
  Of the vehicle, never passed on to a call: +0x48 (its +0x20 a pointer kept for one float's store at the end), +0x6C and +0x80 (the matrix for the group lights, 16 bytes by a `movaps`), +0xDC, +0x860, +0x986, +0x1380, and its address, shifted, as the lights' shadow ID (low 32 bits + 5 + i).
  Of the model info: the getter's argument at RVA 0xE9BD4E and +0xB0, nothing else.
  Per light i: bit i of +0x860, the bone structure's +0x99 + i, the light, the state's 4 arrays and 2 masks, the skeleton by the bone's index.
  A down-counter from 20 only scales a reversed light's delta: delta x (20 - i), as a signed float; forward delta x i.
  A light's group, +0x40, indexes 4 stack entries unchecked, the game's own.
- **The renderer with a setting of zeros** (`sequencerBpm` 0 or 200; its listing read in full, and emulated) lights nothing and cannot fault.
  `sequencerBpm`, a dword it turns into a float (`cvtsi2ss`), the constructor's 200: at 0 it gives +inf by an SSE `divss`, the exceptions masked (the game's only `LDMXCSR` masks more), as a pack's own 0 does.
  No value of a setting is an index, a pointer or an integer denominator but a light's group, reached only with `light` set and `useRealLights` 0: with bytes from past the array a way to a smashed stack (from group 9 it reaches the return address).
  It never reads a setting's count or its name.
- **The bone structure:** from the model info's +0xB0 through two pointers: +0x00 128 bits, one per bone index; +0x10 one signed byte per hierarchy ID, 644 of them (0 to 0x283), that ID's bone's index, below 0 for none.
  Siren n is ID 0x88 + n, its byte at +0x98 + n (`siren1` at +0x99); its glass, `siren_glass`n, is ID 0x9C + n, at +0xAC + n (`siren_glass1` at +0xAD); n runs from 1 to 20, the next group of IDs begins at 0xB1: no ID and no name past `siren20` and `siren_glass20`.
  A bone index is a signed byte: a model names at most 128 bones to the game, 0 to 127.
- **Find-bone-by-name** (`I-E-B4`): rcx the skeleton data, the skeleton's first field; rdx a name; returns the bone, its index a uint16 at +0x42, or 0; whether it ignores case is not settled.
  The bone names' table, `.data` RVA 0x28CFA10, read with a data window: `siren1` to `siren20`, IDs 0x89 to 0x9C; `siren_glass1` to `siren_glass20`, IDs 0x9D to 0xB0.
- **The model info's +0x18** holds the hash of its name: the game's code compares it with `police`'s, 0x79FBB0C5; not settled beyond that, the measurement's boot confirms it.
- **The parser's member objects:** +0x00 the vtable, whose slot 1 reads a node of the file's tree; +0x08 the member data; for an array +0x10 its element's member object.
  A node: +0x38 its first child, +0x30 its next sibling.
  An array's slot 1: rcx the member object, rdx the node, r8 the structure that holds the array, not the array's base (`mov rbx, r8` at RVA 0xAE66A5), and no more: r9's first mention is a write, at RVA 0xAE66F8, and no stack argument is read.
  The array's base is r8 plus the member data's offset, read at the call.
  The element's slot 1, called at RVA 0xAE6A00 to 0xAE6A1A: rcx the element's member object, rdx the child, r8 the element's address, no r9 set, its result not read.
- **The lights' member data** (RVA 0x28EE4C8, entry 19 of `I-E-P1`'s table): hash 0x7A07C7F6, `sirens`, type 0x0D, subtype 1, flags 0x0009, element size 0x48, capacity 20, offset 0x3C once registered; type, subtype, size and capacity stand from the start, read with a data window.
  Their element's (RVA 0x28EE488): hash 0x063FA3F2, `Item`, type 0x0C.
  The array of settings' (RVA 0x28EC470): hash 0x9AF814F5, `Sirens`, type 0x0D, subtype 0, a growing array, its own element size 0x5E8.
- **A siren setting's members,** each matched by its hash: `id` +0x00, `name` +0x5E0, `timeMultiplier` +0x04, `lightFalloffMax` +0x08, `lightFalloffExponent` +0x0C, `lightInnerConeAngle` +0x10, `lightOuterConeAngle` +0x14, `lightOffset` +0x18, `textureName` +0x1C, `sequencerBpm` +0x20, `leftHeadLight` +0x24, `rightHeadLight` +0x28, `leftTailLight` +0x2C, `rightTailLight` +0x30, `leftHeadLightMultiples` +0x34, right +0x35, `leftTailLightMultiples` +0x36, right +0x37, `useRealLights` +0x38, `sirens` +0x3C, 20 of 0x48 bytes, their count +0x5DC.
  A light's: `rotation` +0x00, `flashiness` +0x14, `corona` +0x28, `color` +0x38, `intensity` +0x3C, `lightGroup` +0x40, `scaleFactor` +0x41, `scale` +0x42, `rotate` +0x43, `flash` +0x44, `light` +0x45, `spotLight` +0x46, `castShadows` +0x47.
  Its rotation and its flashiness, one layout: +0x00 delta, a float; +0x04 start, a float; +0x08 speed; +0x0C sequencer, a dword; +0x10 multiples, a byte; +0x11 direction, a byte, not 0 for reversed; +0x12 syncToBpm, a byte.
  A light's padding, +0x13, +0x27 and +0x35 to +0x37, holds no member, and what a copy does with it no pattern says.
  Delta, start and direction are read from what the code does with them, the other names by the XML's order and their uses.
- **The array's tree reader** (`I-E-Z3`, slot 1 of the array class, vtable at RVA 0x261E520) counts the node's children, calls RVA 0xACFCA0 with the member, the object, the count and 7, then reads each child into the array's base + i x the element's size by the element's slot 1; for subtypes 1 to 4 and 9 it stops when i reaches the capacity.
  RVA 0xACFCA0, for subtype 1, stores min(children, capacity) at ((size x capacity + 3) & ~3) + offset: a fixed array's count lies behind its elements by rule.
  So item 21 and on of an XML file's setting are dropped, the count 20, no message.
  Every file through RVA 0xAC6B30 and 0xB05060 is first made a tree, so the same reader runs; a pack's binary file too (HINT: through the tree's own format check).
- **The siren state:** one allocation, `I-E-Z2`, and one store of its pointer, at RVA 0xDCAA32; nothing copies it; the constructor RVA 0x1363630 leaves it zero but for the beat at +0x08, -1, and the start time at +0x00; its resets touch that state alone; the game frees it whole; `operator new` gives 16-aligned memory.
- **What no pattern protects:** that the renderer reads only those 7 fields of the vehicle and 2 of the model, so after a game update it is read again before these patterns are trusted; that a setting's 20 lights are never changed after their load and are copied whole with their count 20 (RVA 0x1378210, 0x137A3D0), so a hash of their 1,440 bytes is the same at the parser and at the renderer; that the lights' order is the bones', item 20 + k being siren 21 + k; the tree's two fields and the element's call through slot 1.

`I-E-Z1` to `Z7`, each once in both dumps, are the raise's lights', with `I-E-B1`, `I-E-C1` and `I-E-P1`.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| I-E-B1 | `48 89 6C 24 28 0F 95 44 24 20 4C 89 E1 E8 ? ? ? ? 49 89 EE 48 89 E9 E8` | +13: the call of the siren lights' renderer (RVA 0xE9BCB0) in the vehicle lights' function (RVA 0xE15B20), its 6th argument the model info (`mov [rsp+28h], rbp` at +0) - the call the measurement redirects; +24: the call of the siren settings getter (RVA 0x135D220) with the same rbp (`mov rcx, rbp` at +21), its rel32 past the pattern | found | 0xE16F8F |
| I-E-B2 | `48 8B 87 B0 00 00 00 48 85 C0 74 ? 48 8B 00 48 85 C0 74 ? 0F BE 84 28 99 00 00 00` | +3: 0xB0, the model info's pointer to its bone structure's holder (`mov rax, [rdi+0xB0]`); +24: 0x99, `siren1`'s byte in the bone structure (`movsx eax, byte [rax+rbp+0x99]`) - the renderer's read of a siren's bone, in the renderer (RVA 0xE9BCB0) | found | 0xE9C0B9 |
| I-E-B3 | `0F BE 94 18 AD 00 00 00 48 8B 8E C0 09 00 00` | +4: 0xAD, `siren_glass1`'s byte in the bone structure (`movsx edx, byte [rax+rbx+0xAD]`) - a read of a siren's glass bone, in the function that breaks a siren (RVA 0x1DE80A0) | found | 0x1DE8298 |
| I-E-B4 | `41 57 41 56 41 55 41 54 56 57 53 48 83 EC 20 66 83 79 5E 00` | +0: the start of find-bone-by-name (RVA 0x1C4BFF0), a function start by the unwind data; +18: 0x5E, the skeleton data's bone count (`cmp word [rcx+0x5E], 0`), tested first | found | 0x1C4BFF0 |
| I-E-Z1 | `48 8B 8C 24 58 04 00 00 49 8B 45 48 48 8B 40 20 48 89 84 24 58 02 00 00 E8 ? ? ? ? 45 0F B7 BD DC 00 00 00 44 03 3D ? ? ? ? 49 8B 8D 80 13 00 00` | +24: the renderer's call of the siren settings getter, redirected, its target the one `I-E-B1` +24 leads to; +11 0x48, +15 0x20, +33 0xDC and +47 0x1380 pin the renderer's reads of the vehicle's fields; in the renderer by the unwind data | found | 0xE9BD36 |
| I-E-Z2 | `B9 88 01 00 00 E8 ? ? ? ? 48 89 C3` | +1: 0x188, the siren state's allocation, written 0x16A8: the state, a second state at +0x190, a stand-in vehicle of 0x1388 bytes at +0x320 | found | 0xDCAA1B |
| I-E-Z3 | `55 41 57 41 56 41 55 41 54 56 57 53 48 81 EC A8 00 00 00 48 8D AC 24 80 00 00 00 0F 29 75 10 48 89 D7 49 89 CD` | +0: the array member's tree reader, hooked, a function start by the unwind data; it keeps rcx, the member object, in r13 and rdx, the node, in rdi | found | 0xAE6630 |
| I-E-Z4 | `41 8B 84 85 60 08 00 00 0F A3 E8 72 ? 48 8B 87 B0 00 00 00 48 85 C0 74 ? 48 8B 00 48 85 C0 74 ? 0F BE 84 28 99 00 00 00` | checked only: +4 0x860, the vehicle's broken sirens, +16 0xB0 and +37 0x99, `I-E-B2` with what stands before it; in the renderer by the unwind data | found | 0xE9C0AC |
| I-E-Z5 | `41 0F B6 85 86 09 00 00 24 02` | checked only: +4 0x986, the vehicle's flag byte; in the renderer by the unwind data | found | 0xE9C80F |
| I-E-Z6 | `41 0F B7 45 6C 83 E0 FD 83 F8 08 74 ? 44 0F 28 3D ? ? ? ? 83 F8 01 75 ? 45 0F 28 BD 80 00 00 00` | checked only: +4 0x6C and +30 0x80, the vehicle's fields for the group lights; in the renderer by the unwind data | found | 0xE9D05B |
| I-E-Z7 | `48 8B 7F 38 48 85 FF 0F 84 ? ? ? ? 48 8B 75 D0 49 8B 45 08 B9 01 00 00 00 EB ? ? ? ? ? 8B 48 28 49 39 CE 0F 84 ? ? ? ? 48 8B 7F 30 49 8D 4E 01 48 85 FF 0F 84 ? ? ? ? 49 89 CE 4C 8D 41 FF 4C 0F AF 40 20` | checked only: +3 0x38, +20 the member data, +34 0x28, +47 0x30 and +72 0x20 pin the reader's node and member fields; in the tree reader by the unwind data | found | 0xAE69C0 |

### Siren damage

My proof ran the raise on the game, with 32-siren packs: the lights, a shot, a ram, a wreck, a repair and a plane's gear.

- **The bone structure:** a pool element of 0x34C bytes, made by RVA 0x134DD00, whose one caller is RVA 0x134D680 (slot 25 of the vtable at `.rdata` RVA 0x2672270), at RVA 0x134D709, the model info in rcx.
  The maker allocates (RVA 0x15A7C0, which traps at a size of 0, a size above the stride or a full pool) from the pool at `.data` RVA 0x471D908, loaded right before `I-E-Y2` by `48 8B 0D EB FB 3C 03` at RVA 0x134DD16, runs the constructor RVA 0x13497B0 (+0x00 four dwords of 0xFFFFFFFF, `memset` of 0xFF over +0x10 to +0x2A8, fields to +0x348), stores it at the holder's +0x00, and fills it up to 3 times.
  The pool, `VehicleStruct`: set up by RVA 0x1349BD0 with the stride (`I-E-Y1`) and its size from the configuration by the name's hash; its pointer's one store at RVA 0x1349DFC, about 30 seconds after TweaksV's start; the one free at RVA 0x1350623; it takes the stride from its own +0x1C, and nothing copies a structure.
- **The fill** (`I-E-Y4`, 995 bytes) takes the model info, a table, its tags and a flag: for each entry until a null name the bone by `tags[k]`, 0 for `chassis`'s tag, and, if found, the low byte of the bone's index at the structure's 0x10 + the entry's ID, unchecked.
  A table's entry is 16 bytes, a name and an ID; the game's tables come from the vehicle factory's virtuals by the model's type at +0x340, flag 0 for the first and 1 for the others, the third for types 8 and 1, whose first glass slots are gear doors.
- **A tag** (RVA 0xAA0260): letters folded to upper case, an ELF-style hash, modulo 0xFE8F, plus 0x170, so the game's lookup ignores case.
  Read with a data window: `chassis` 0xCD54, `siren1` 0x61A7, `siren20` 0x1642, `siren_glass1` 0xF688; by the same rule `siren21` 0x1643, `siren32` 0x1674, `siren_glass21` 0x4055, `siren_glass32` 0x4026.
- **The set at +0x00:** RVA 0x1350360, called last in RVA 0x134D680, clears the bit of each bone whose ID lies in 7..0x2F, 0x38..0x7F, 0x82..0xB0 or 0x1CE..0x1D1 or is below 3, then sets some for two vehicle types.
  HINT, no reader was found: it does not change how a siren is drawn.
- **The damage, read in full:** each loop touches only the structure's bytes, the 32-bit sets (the vehicle's +0x860 and the damage object's +0x440, both `i >> 5` = 0 up to 31), the bone through the game's functions, and the fragment child through RVA 0xFF1B70; none reads a setting, a state or anything else sized by 20.
  +0x860's other readers: the renderer, by its bound; RVA 0x1543DA0, 20 bits; RVA 0xECE980 copies the dword whole; RVA 0x1DE80A0 its first 3; nothing compares the whole, and +0x865 is only written and copied.
  +0x440's: RVA 0xECB990, RVA 0xE7D590 and two resets of the whole dword (RVA 0xE77820, RVA 0xE77A30).
- **What stays at 20:** the stored damage state, RVA 0x1548A80 (slot 28 of three vtables) and its twin RVA 0x1543DA0: its loop reads 20 bytes of sync data at +0xF4 (`48 83 FD 14 0F 84 ? ? ? ? 41 0F B6 84 2D F4 00 00 00` at RVA 0x15490F3, its glass `0F BE 9C 28 AD 00 00 00 48 8B 06 48 89 F1` at RVA 0x154915A), and past them it would apply other fields as broken sirens; HINT, the network objects' sync data, no path found in story mode.
  RVA 0x1DE80A0 reads 3 sirens only (`I-E-B3`).
  The glass loops over 9 and 8 bones (RVA 0xE29321, RVA 0xE4C9E7).
- **A loop of another shape** would stay at 20, cosmetic: the search took `cmp r64, 0x14`, the three displacements, a start or a bound at the IDs 0x89, 0x9D and 0xB1, a subtraction of those with a compare and lists of those IDs in the read-only data, and found these.
  After a game update these loops too are read again before the patterns are trusted: no pattern proves that no loop was added.

The rows' N is `SIRENS_PER_VEHICLE`; the room stays 64 bytes whatever N.
The hit's counter counts in hierarchy IDs: a siren's byte at +0x10 + the counter, its glass's at +0x24 + it, both 1-byte displacements.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| I-E-X1 | `BF 89 00 00 00 F3 44 0F 10 05 ? ? ? ? 45 0F 57 DB` | a hit near a siren (RVA 0xE7A820), which sets the vehicle's +0x860 bit and +0x865 and breaks the glass's child: +1: 0x89, the counter's start, to 0x33C | found | 0xE7AE6B |
| I-E-X2 | `48 81 FF 9D 00 00 00 0F 84 ? ? ? ? 8D AF 77 FF FF FF 41 BE 01 00 00 00` | its end and index: +3: 0x9D to 0x33C + N; +15: -0x89 to -0x33C, the `lea` that makes i | found | 0xE7AE93 |
| I-E-X3 | `0F BE 54 39 24 48 8B 88 C0 09 00 00 E8` | its glass: +4: 0x24 to 0x30, the glass 0x20 behind the sirens in the room | found | 0xE7B04F |
| I-E-X4 | `44 0F B6 64 38 10 41 80 FC FF` | its read of a siren's bone: +5: 0x10 stays, checked | found | 0xE7AEDD |
| I-E-L05 | `48 83 FD 14 0F 84 ? ? ? ? 49 8B 84 24 08 04 00 00` | an impact (RVA 0xECB990), which sets the damage object's +0x440 bit and +0x445 and breaks the glass's child: +3: 20 to N | found | 0xECC1C3 |
| I-E-N04 | `0F BE 94 29 99 00 00 00 BB 01 00 00 00` | its siren: +4: 0x99 to 0x34C | found | 0xECC1F2 |
| I-E-N05 | `0F BE 94 29 AD 00 00 00 48 8B 88 C0 09 00 00` | its glass: +4: 0xAD to 0x36C | found | 0xECC2A2 |
| I-E-L03 | `48 83 FB 14 74 ? 89 D8 C1 E8 05 8B 84 86 40 04 00 00` | the bones the impact marked, hidden by a zero matrix (RVA 0xE7D590): +3: 20 to N; +14: 0x440, the damage object's set | found | 0xE7D623 |
| I-E-N03 | `0F BE AC 18 99 00 00 00 83 FD FF 74 ?` | its siren: +4: 0x99 to 0x34C | found | 0xE7D659 |
| I-E-L04 | `48 83 F9 14 74 ? 49 8B 87 08 04 00 00` | a wreck, which breaks every siren whose bone exists (RVA 0xDEA730): +3: 20 to N | found | 0xDEA893 |
| I-E-N02 | `0F BE 94 0D 99 00 00 00 BD 01 00 00 00` | its siren: +4: 0x99 to 0x34C | found | 0xDEA8BE |
| I-E-L07 | `48 83 FB 14 74 ? 48 8B 46 20 48 8B 80 B0 00 00 00` | the glass's fragment children's flags, 0x17FFFFE0 (RVA 0xDDF580): +3: 20 to N; +13: 0xB0, the model info's holder | found | 0xDDF5B3 |
| I-E-N06 | `0F BE 94 18 AD 00 00 00 83 FA FF 74 ? 48 8B 8E C0 09 00 00 E8 ? ? ? ? 83 F8 FF 74 ? 48 8B 8F 90 00 00 00` | its glass: +4: 0xAD to 0x36C | found | 0xDDF5D1 |
| I-E-L08 | `48 C7 C3 EC FF FF FF` | the same in three classes (RVA 0xE20B20, RVA 0xE4DE30, RVA 0xE5E510): +3: -20 to -N; matches 3 times by design | found 3 times | 0xE20B48, 0xE4DE5A, 0xE5E538 |
| I-E-N09 | `0F BE 94 18 C1 00 00 00 83 FA FF 74` | their glass, read from -20: +4: 0xC1 to 0x36C + N; matches 3 times by design | found 3 times | 0xE20B7D, 0xE4DE8D, 0xE5E56D |
| I-E-Y1 | `C7 44 24 20 4C 03 00 00 4C 8D 05` | the bone structures' pool: +4: the stride 0x34C to 0x38C | found | 0x1349DE0 |
| I-E-Y2 | `BA 4C 03 00 00 E8 ? ? ? ? 48 89 C7 48 89 C1 E8` | the allocation, in RVA 0x134DD00 by the unwind data: +1: 0x34C to 0x38C | found | 0x134DD1D |
| I-E-Y3 | `48 89 F1 E8 ? ? ? ? 48 89 F1 E8 ? ? ? ? F6 86 4B 05 00 00 10` | +3: the call of RVA 0x134DD00, redirected | found | 0x134D706 |
| I-E-Y4 | `41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 28 45 89 CE 4D 89 C5 48 89 D3 49 89 CF E8` | +0: the fill, called once more by the wrapper, a function start by the unwind data | found | 0x1353E40 |

## PedProps

Its letter is `A`.
Findings: `pedprops_findings_for_the_clean_room_build.txt`, the ped variation code, RVA 0x10C0000..0x1110000, 582 functions, read whole.

- **The model info:** +0x9D its type in the low 5 bits, 6 for a ped; +0x18 the game's hash of the model's name (`mp_m_freemode_01` 0x705E61F2, `mp_f_freemode_01` 0x9C9EFFD8); +0x240 a ped model's variation collection, its first field an array of 8-byte pointers to the model's variation files, their count a word at +8.
- **A variation file,** the model's own and one per pack: +0x58 its anchors, +0x60 their count, a word, entries of 0x18 bytes (+0 a pointer to one byte per prop, its texture count; +8 the props' count, a word; +0x10 the anchor, a dword); +0x48 its prop metadata, +0x50 their count, a word, entries of 0x38 bytes (+0x28 a byte whose low 3 bits the game sets and clears; +0x2C flags, a dword, a word at +0x30 when that is 0; +0x32 the anchor, a byte; +0x33 the prop's local index, a byte); +0x68 its hash, the name of the file or pack it came from.
- **A ped's worn props:** its +0x48, then +0x1C0, 6 slots of 0x50 bytes (+4 the anchor, a dword, -1 empty; +8 the prop's index, a dword; +0xC the texture, a dword).
  The anchors: 13, 0..12, 0 the head's.
- **Global and local indices:** a ped's slot and the natives hold a prop's global index, the props of an anchor counted through all of the model's files in their order; a file's own, local index names its files (`%s_%03d`, `%s_diff_%03d_%c`) and is what its metadata keeps, in a byte.
  The per-anchor count (`A-E-C1`) sums, file by file, the props of the first entry whose anchor is the one asked for.
- **The limit, three kinds of narrowing to 8 bits.**
  Each file's count of props on an anchor, at every walk of its anchors, 41 sites in 16 functions, which the part leaves: one file holds at most 255 props on an anchor, the bound of its own data too (the metadata's byte, the packed slot's 8 bits).
  An anchor's total over a model's files, counted in 32 bits, by the validity check that the set-prop native and the preload native ask (RVA 0x10F4DD0, `A-E-T1`) and by the helmet code after each of its four calls of the per-anchor count (`A-E-T2..T5`).
  A worn prop's index, before 13 calls of the metadata lookup (RVA 0x10F9880) at six places (`A-E-G1` twice, `A-E-G3..G6`), and in the random props' builder, which the part leaves, its list keeping an index as a byte by its format.
  From 256 props on one anchor in all, the set-prop native refuses every prop from (total mod 256) on, the helmet code clamps or drops a head prop past it, and a worn prop at index 256 or more gets the metadata of (index mod 256): `A-E-G6` writes flags into that other prop's entry.
  Below 256 nothing narrowed matters, and a `movzx` of 16 bits gives the values the game's of 8 gives: each narrowed register holds the whole value ("When Game Updates", item 5), and none is AH..BH.
  Not narrowed: the count native's result, the set-prop function's checks and its dwords, the conversions' results, the shop natives' metadata lookups.
- **The complete sets:** the per-anchor count has exactly 7 calls, all in the helmet code: `A-E-T2..T5`'s four, and three whose result is unused, at RVA 0xD614DA, 0xD621EA (at `A-E-T5`'s +22, the last byte of its match) and 0xD622D1.
  The metadata lookup has exactly 17: 13 after the narrowings (`A-E-G1`'s two, `A-E-G3`'s three, `A-E-G4`'s, `A-E-G5`'s and `A-E-G6`'s six), the random props' builder's at 0x10F8483, two in their picker at 0x10F6373 and 0x10F6787, and a thunk's at 0x10F9A64, whose callers pass 32-bit indices.
  The validity check has two callers, the set-prop native's and the preload native's.
- **The model info store,** in `.data`: a dword count of model indices (RVA 0x444AF98), a bitmap with one bit per index (its pointer at 0x444AFC0; index >> 5 the dword, index & 31 the bit), an array of elements of the stride's size (its pointer at 0x444AF90, the stride a qword at 0x444AFA8), each beginning with the model info's pointer.
  The lookup stands inline at about 200 places in the code; `A-E-S1` takes the one in the function at RVA 0x1DFC0, followed by the ped type's check.
- **Measured,** the base game's own head props: 255 on `mp_m_freemode_01`, from 39 files, and 254 on `mp_f_freemode_01`, from 38, so a single add-on head prop takes the male model past the limit.
  With clothing packs its head anchor reached 272 and 279, the props from index 16 and 23 on refused, and every switch to the male model past 255 crashed the game (0xC0000005 at RVA 0x5A0998D, a read of 0x274, in the natives' code), with packs made for Legacy and converted ones alike, while 255 switched fine.
  With the eleven widened the switch past 255, with `r4usarmyv3` in, crashed at the same RVA; with that pack out the male model, at 255, switched fine with a hat at index 200.
  Not settled: whether the crash comes from going past 255 or from the big packs' own data, since no test has gone past 255 without a big pack, and no hat past 255 has been worn (ISSUES.md, T-PEDPROPS).

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| A-E-T1 | `0F B6 C3 44 39 C0 7E ? E8 ? ? ? ? 0F B6 C0 44 39 F0 0F 9F C0` | +0: `movzx eax, bl`, an anchor's total in the validity check (its B6 at +1); +8: the call of the texture count (RVA 0x10E55F0) | found | 0x10F4E63 |
| A-E-T2 | `48 8B 0E 31 D2 E8 ? ? ? ? 0F B6 C0 39 C5 0F 4C C5 83 FD FF` | +10: `movzx eax, al`, the head's total (its B6 at +11), after +5: the call of the per-anchor count, and +3: `xor edx, edx`, the anchor 0 - the helmet code | found | 0xD617EC |
| A-E-T3 | `48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 C7 0F 4C C7 83 FF FF` | the same | found | 0xD61AEF |
| A-E-T4 | `48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 F8 76 ? 89 7E 18 88 5E 20` | the same | found | 0xD61D9A |
| A-E-T5 | `48 8B 09 31 D2 E8 ? ? ? ? 0F B6 C0 39 C7 7C ? 48 8B 0E 31 D2 E8` | the same; +22: the next call of the per-anchor count, one of its three unused | found | 0xD621D4 |
| A-E-G1 | `48 8B 84 24 ? ? ? ? 48 8B 88 40 02 00 00 44 0F B6 C6 40 0F B6 D5 E8` | +15: `movzx r8d, sil`, a worn prop's index (its B6 at +17); +19: `movzx edx, bpl`, the anchor, left; +23: the call of the metadata lookup (RVA 0x10F9880) - matches 2 times | found, 2 matches | 0x10F1EBB, 0x10F1F1D |
| A-E-G3 | `41 8B 44 04 08 83 F8 FF 0F 84 ? ? ? ? 0F B6 E8 48 89 F9 89 F2 41 89 E8 E8` | +0: the slot's index loaded; +14: `movzx ebp, al` (its B6 at +15), for three calls of the metadata lookup, the first at +25 | found | 0x10F4548 |
| A-E-G4 | `8B 44 38 08 31 FF 83 F8 FF 74 ? 48 8B 49 20 48 8B 89 40 02 00 00 44 0F B6 C0 0F B6 D2 E8` | +0: the slot's index loaded; +22: `movzx r8d, al` (its B6 at +24); +29: the call of the metadata lookup | found | 0x10F5A3D |
| A-E-G5 | `41 8B 44 07 08 85 C0 0F 88 ? ? ? ? 44 0F B6 C0 48 89 F9 44 89 F2 E8` | +0: the slot's index loaded; +13: `movzx r8d, al` (its B6 at +15); +23: the call of the metadata lookup | found | 0x10F5CDB |
| A-E-G6 | `E8 ? ? ? ? 44 0F B6 F7 45 0F B6 FF 4C 89 E9 44 89 FA 45 89 F0 E8` | +5: `movzx r14d, dil` (its B6 at +7), for six calls of the metadata lookup, the first at +22; +9: `movzx r15d, r15b`, the anchor, left | found | 0x10F8866 |
| A-E-C1 | `56 57 48 8B 41 20 48 8B 88 40 02 00 00 44 0F B7 41 08 31 C0 4D 85 C0 74 ? 4C 8B 09 45 31 DB EB` | the per-anchor count's start: +5: 0x20, a ped's model info; +9: 0x240 (a disp32), the model info's variation collection; +17: 8, the count of its files (a word) | found | 0x10F4E90 |
| A-E-C2 | `44 0F B7 54 39 08 41 0F B6 CA 01 C8 49 FF C3 4D 39 C3 74 ? 4B 8B 0C D9 0F B7 71 60 45 31 D2 48 85 F6 74 ? 48 8B 49 58 48 C1 E6 03 48 8D 34 76 31 FF` | its loop: +5: 8, an anchor entry's props (a word); +27: 0x60 and +39: 0x58, a file's anchors' count and its anchors; +40 and +44: `shl rsi, 3` and `lea rsi, [rsi+rsi*2]`, an entry's 0x18 bytes; +6: the per-file narrowing, left | found | 0x10F4EC0 |
| A-E-S1 | `3B 05 ? ? ? ? 0F 8D ? ? ? ? 48 8B 0D ? ? ? ? 89 C2 C1 EA 05 8B 0C 91 0F A3 C1 0F 83 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 ? 44 89 7C 24 34 48 98 48 0F AF 05 ? ? ? ? 4C 8B 3C 01 BB 02 00 00 00 4D 85 FF 74 ? 41 0F B6 87 9D 00 00 00 24 1F 3C 06` | the model info store's lookup: +2: its count's disp32; +15: its bitmap pointer's; +39: its array pointer's; +59: its stride's; +81: 0x9D (a disp32), a model info's type byte; +86: 0x1F, its mask; +88: 6, a ped | found | 0x1E0DD |

## Resources

Its letter is `Q`.
The streaming heap's patterns, `Q-E-H1`, `Q-E-A1`, `Q-E-S1` and `Q-E-R1`, went with their measurement, which never found the heap near full: this file as tagged 2.5.0 and `streamingmemory_findings_for_the_clean_room_build.txt` keep them and their facts.

### Texture budget

The game counts the video memory of the textures and buffers its streaming engine places and keeps it below a budget from `gameconfig.xml`: the Legacy budget Resource Adjuster raised, which Enhanced keeps as data.
`texturebudget_findings_for_the_clean_room_build.txt` names `Q-E-T1` to `Q-E-T5` TB-4a, TB-4b, TB-5, TB-6 and TB-7; `streamedobjects_findings_for_the_clean_room_build.txt` holds `Q-E-T6`, `Q-E-T7` and the descriptor heap.

- **The counters,** one object at RVA 0x481D080 (`Q-E-T4`): +0x00 the streamed buffers' video memory in bytes; +0x08 the streamed textures'; +0x10 and +0x14 their numbers (dwords); +0x18 the most of both so far, the game's own high-water mark, raised by the buffers' function with `lock cmpxchg`; +0x20 the most objects allowed (a dword); +0x28 four budgets in bytes, one per index 0 to 3 (to RVA 0x481D0C7).
  The game counts a texture (RVA 0x1E89640) or a buffer (0x1E88030) as it makes it while the thread's dword at TLS +0x15C38, raised by 0x1E9A890 and lowered by 0x1E9AB50, is not 0, and uncounts it as it frees it (0x1E8B0A0, 0x1E89510).
  The device's slots +0x350 and +0x358 raise and lower that dword around a place in 22 functions; the other 33 of the 55 that build a datResource do not, among them the "defrag" re-placements, viseme data and direct loads, whose objects are not counted but take descriptors.
  A buffer counts (word [obj+0xC] & 0xFFF) x dword [obj+8] bytes, a texture the size its creation computes.
- **The sizes:** the start-up (RVA 0x509BF0) loads `common:/data/gameconfig.xml` into the configuration (its data [[RVA 0x2E96FB0] + 0x18]) and writes the most from its +0x54C and the four budgets from its +0x550 to +0x55C, int32 in MB, sign-extended and shifted left by 20.
  By my export of the x64 override of the game's own file: `ConfigGraphics`'s `MaxStreamingResources`, 98,304, and `MaxStreamingMemory_0_Base` to `_3_Base`, 1,500, 1,500, 3,000 and 3,000 MB.
  +0x548 is `MaxWritableResources` (20,480): 0x50A12C puts it at +0x120 of the device settings, a word at 0x44D06FA in the end, which sizes a slot table for writable objects at 0x1191EF0, so it is a u16 and gives this limit no room.
  No other code writes the most and the budgets than the three stores of `Q-E-T1` and `Q-E-T2` (a boot's scan of every `[rip+disp32]` store of any width into their 32 bytes, the bytes read every way: `none besides them`); nothing adds to a `_Base` value on this build: each is its budget as it stands.
- **The call after the stores** (`Q-E-T2` +40, at RVA 0x509F9F) comes once on the start-up thread before any streaming, so every reader of the budgets takes them after it.
  That function's arguments were not read: the start-up sets none up between its stores and the call (`lea r14` and `mov [rip+disp32], r14` stand between them) and gives it 32 bytes of stack, `sub rsp, 0x20`, right before it, so it takes four registers at most and nothing on the stack.
  The start-up comes after TweaksV's `DllMain`: at each of my starts with `DIAGNOSTIC_MODE=1` the first summary, 10 seconds after TweaksV's start, found the most and the budgets still 0.
- **The index** (`Q-E-T3`): min(3, q + (q < 2)) of the texture quality q (the graphics settings' +0x4C, `GFX_TextureQuality`), written at RVA 0x500757; the function at RVA 0x2A3820 sets it to 2 for a while and puts it back.
  Which menu entry gives which q is not seen in the dump; a hint has Normal, High and Very High give 0, 1 and 2, so the index is 2 and the budget 3,000 MB at High and at Very High, whatever the card.
- **Where it binds:** a resource's placement (`Q-E-T4`) needs the objects below the most and the bytes of both below the budget of the index in use (after the getter, `cmp rsi, rax` and `jae` at RVA 0x4DBCDF and 0x4DBCE2) and the physical streaming budget, which never refuses; otherwise it evicts, tries again and at last sets the out-of-memory byte at RVA 0x435F57D.
  The update check (`Q-E-T5`, every update) evicts ahead of time while its switch, a dword, is not 0: it raises its flag, a byte, once the bytes pass the budget times the raise factor, or the game's video memory Windows' budget times the float at RVA 0x28DB5BC, and lowers it below the budget times the lower factor and that ratio.
  The four values lie in `.data`, and no code is known to write them.
- **Windows' budget** for the game's video memory: the adapter object (a static array at RVA 0x2E90270, 0x1490 bytes each) caches DXGI's `QueryVideoMemoryInfo` (RVA 0x1ECCE00) for the local segment group at +0x1428 (Budget) and +0x1430 (CurrentUsage), under its lock at +0x18 (RVA 0x1ECCC80).
  The graphics device reads it through its vtable's +0x948 and +0x960 (the vtable at RVA 0x2717350, a hint on its base), and the settings' video memory bar (RVA 0x5D1410) shows it.
  The command-line parameter `availablevidmem` lowers the cached budget in MB; a value above the real budget runs into an `int3` (RVA 0x1ECCCE0).
- **The most** has one store (RVA 0x509F5B) and one reader, the placement's compare (RVA 0x4DBCBE); the configuration's +0x54C has one reader, the store's load.
  The check comes at the memory's reservation, before the read, and the objects are counted when the file is placed, so up to 32 reserved files stand uncounted: 98,359, 98,343 and 98,326 in my boots.
  At the limit the placement refuses and evicts by the LRU list, then, a hint, the lowest of what is in view and population models, and last sets the out-of-memory byte; nothing is said anywhere but the game's debug dump.
- **The descriptor heap,** CPU-side (`Q-E-T6`, `Q-E-T7`): 196,608 descriptors, the sum of its two dwords, 0x10000 and 0x20000, the initializer's constants at .rdata 0x2662600 and 0x2662610; +0x18 its free list, u32 per descriptor; +0x20 the head; +0x2C the size, which no code reads.
  Taking is head = list[head] under a lock, LIFO; all 11 takers read with plain movs, so at the 196,609th live descriptor the list is read past its end, the view written past the heap and the first descriptor of the visible heap's ring overwritten.
  A streamed texture takes one, a streamed buffer one if it has a view; render targets, code-made textures and buffers, UAV views, the TLAS views and direct loads are not counted and take from it too.
  The first hard limit past 98,304 is objects + overshoot + what is not counted <= 196,608; 3/4 of it is 147,456, and 131,072 leaves 65,536.

Each unchanged 60 seconds later:

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| Q-E-T1 | `8B 88 4C 05 00 00 89 0D ? ? ? ? 66 0F 38 25 80 50 05 00 00` | in the start-up: +2 the configuration's offset of the most, 0x54C; +8 the rel32 of `mov [rip+disp32], ecx` (`89 0D` at +6) - the most objects, a dword (RVA 0x481D0A0); +17 the offset of budgets 0 and 1, 0x550. Past the match: `psllq xmm0, 20` (+21) and `movdqu [rip+disp32], xmm0` (+26, its rel32 at +30) - budgets 0 and 1 (RVA 0x481D0A8) - then `Q-E-T2` at +34 | found | 0x509F55 |
| Q-E-T2 | `66 0F 38 25 80 58 05 00 00 66 0F 73 F0 14 F3 0F 7F 05 ? ? ? ? 4C 8D 35 ? ? ? ? 4C 89 35 ? ? ? ? 48 83 EC 20 E8` | +5 the offset of budgets 2 and 3, 0x558; +9 `psllq xmm0, 20`: the budgets are MB; +18 the rel32 of `movdqu [rip+disp32], xmm0` - budgets 2 and 3 (RVA 0x481D0B8, 16 bytes after budgets 0 and 1); +40 the call after the stores (`E8`, its rel32 past the match, to RVA 0x781C90), on the start-up thread before any streaming - the call the part redirects | found | 0x509F77 |
| Q-E-T3 | `48 63 05 ? ? ? ? 48 8B 44 C1 28 C3` | the budget's getter, a leaf without unwind data: +3 the rel32 of `movsxd rax, [rip+disp32]` - the index (RVA 0x44C3B68); +11 0x28, the budgets' offset in the counters it is handed in `rcx` | found | 0x1C38DB0 |
| Q-E-T4 | `8B 05 ? ? ? ? 8B 0D ? ? ? ? 01 C1 3B 0D ? ? ? ? 73 ? 48 8B 05 ? ? ? ? 48 8B 35` | in a resource's placement (RVA 0x4DBBA0), the rel32 of its loads of the counters: +2 the buffers' number (RVA 0x481D090), +8 the textures' (0x481D094), +16 the most (`cmp ecx, [rip+disp32]`, 0x481D0A0), +25 the buffers' bytes, the counters' start (0x481D080), and +32, past the match, the textures' bytes (0x481D088) | found | 0x4DBCB0 |
| Q-E-T5 | `83 3D ? ? ? ? 00 0F 84 ? ? ? ? 48 8D 0D ? ? ? ? 48 8B 3D ? ? ? ? 48 8B 35 ? ? ? ? E8` | in the update check (RVA 0xCEE8D0): +2 the disp32 of `cmp dword [rip+disp32], 0`, an imm8 after it - the switch (RVA 0x28DB5B0); +16 the counters, handed to the getter (`lea rcx`); +23 and +30 the buffers' and the textures' bytes again; +34 the getter's call, its rel32 past the match. At fixed distances past the match: `cmp byte [rip+disp32], 0` of the flag at +0x4B (RVA 0x432AD60), `mulss` by the lower factor at +0x54 (RVA 0x28DB5B8) and by the raise factor at +0xA0 (RVA 0x28DB5B4) | found | 0xCEE8D6 |
| Q-E-T6 | `44 8B 05 ? ? ? ? 44 03 05 ? ? ? ? C6 44 24 28 00 C7 44 24 20 00 00 00 00 48 8D 0D ? ? ? ? 31 D2 45 31 C9 E8` | in the device's start (RVA 0x1E7FC20): +3 and +10 the rel32 of `mov r8d` and `add r8d`, the two dwords whose sum is the CPU-side descriptor heap's size (RVA 0x44C3A9C and 0x44C3AA0); +30 the rel32 of `lea rcx`, the heap (RVA 0x4E8D220); +39 the call of its maker (RVA 0x1E7C4C0), its rel32 past the match | found, its RVA to check | 0x1E8095D |
| Q-E-T7 | `44 89 41 2C 84 C0 74 ? 48 C7 46 18 00 00 00 00 E9` | 0x36 bytes into that maker: +3 0x2C, the heap's size; +11 0x18, its free list. At the fixed distance +0x9E past the match, read after `IsReadable`: `89 7E 24 89 7E 20`, its ring base at +0x24 and its head at +0x20 | found, its RVA to check | 0x1E7C4F6 |

On 1.0.1158.16, beyond the rows: the stores of budgets 0 and 1 at 0x509F6F to 0x481D0A8, of budgets 2 and 3 at 0x509F85 to 0x481D0B8; the call after them at 0x509F9F, `E8 EC 7C 27 00`; the getter's call at 0xCEE8F8 to 0x1C38DB0; the update check's flag at 0xCEE921 to 0x432AD60, its factors at 0xCEE92A to 0x28DB5B8 and at 0xCEE976 to 0x28DB5B4.

## RayTracing

Its letter is `T`.
Findings: `rtbuffer_findings_for_the_clean_room_build.txt`, from the dump and from the shaders LuminanceV's run saved.

- **The effect** `bvh_skinning`, compute, skins the moving geometry the ray tracing keeps into one global vertex buffer, which the BLAS refits read before the TLASes are built; its seven programs are named skinning, trees, grass, tyre, micromovement, cable and rope.
  Its set-up, once: RVA 0x31E470, called from 0x31C320, which the protected start-up code registers (a hint: the renderer's start); it returns at once when 0x118CFE0 says no (a hint: ray tracing not available).
- **The buffer** (`T-E-B1`): 0x400000 = 4,194,304 vertices of 20 bytes (a float3 position, the normal packed 10:10:10, the tangent 10:10:10:2), 80 MB; the maker takes count, stride, ..., flags; its object's +0x08 the count, +0x0C the stride, +0x28 its GPU address; its view at 0x3852920.
  Neither the maker nor the view maker 0x486440 holds a size of its own: the views take theirs from the object's count and stride.
- **The ranges:** each frame's BVH build (RVA 0x396710, the task named "Build BVH") flips its frame slot, a dword, 0 or 1, and sets the new slot's counter to 0 (the counters: one dword per slot) before the frame's objects take their ranges.
  Every moving object's geometry (`T-E-C1`) and every rope (`T-E-C2`) takes its vertex count from the slot's counter by `lock cmpxchg`, refused when the range's end would pass 0x400000.
  A range's start is the skinning job's +0x48 (the shader's job +72) and the geometry record's vertex address, the buffer's +0x28 plus 20 times it.
  So every frame's ranges start at 0, and the other slot's counter holds the last finished frame's total until its own frame starts.
- **When it is full:** a refused geometry gets no skinning job, and its object no BLAS refit and no instance in that frame's TLASes (0x396710 skips it at 0x39868D): no ray traced shadow, reflection, occlusion or GI of it in that frame, and as the order of the objects changes from frame to frame they come and go, the flicker.
  The refusal comes before the counter moves, so a later, smaller object can still fit.
  The game logs nothing.
  Both refusals overwrite or ignore rax and the flags: 0x38CCC2 is `xor eax, eax` and the epilogue, 0x488736 the epilogue of a function without a result.
- **The bounds of a raise:** the ray tracing shaders read an object's first vertex in 26 bits of its material record (CS_RaytraceReflectionLightPassWave32 0x82EE00F4: word 3 >> 16 and word 7 >> 22), so at most 67,108,864; the game's sizes are 32-bit (count x 20), so at most 214,748,364; D3D12's guaranteed view size, a hint, not verified, 2^27 32-bit elements, 26,843,545 vertices, is the part's cap.
- **The neighbouring limits** of the same frame, per slot, not raised: 0x2600 job lists (the count at RVA 0x383F740 + slot x 0x13008), 0x4000 BLAS build entries (0x307C2D0 + slot x 0xC0008), 0x1000 rope records (0x34FC690 + slot x 0x28008, the count at +0x88038).
- **The complete set:** the counters are referenced by three `lea` in the game's code, `T-E-C1`'s, `T-E-C2`'s and the frame's reset at 0x396E22, and by no other load or store; the immediate 0x400000 addresses the buffer only in the set-up and the two compares.
  The slot's flip (`8B 05 ? ? ? ? F7 D0 FF 05 ? ? ? ? 83 E0 01 89 05` at 0x396BBD) is no pattern of the part.

Each unchanged 60 seconds later:

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| T-E-B1 | `C7 44 24 20 89 00 00 00 B9 ? ? ? ? BA 14 00 00 00 41 B0 01 45 31 C9 E8 ? ? ? ? 48 89 05 ? ? ? ?` | in the set-up 0x31E470: +4 the maker's flags 0x89; +9 the buffer's count, imm32 0x400000 - raised by the part; +14 the stride, 20; +24 the call of the buffer maker 0x11927D0, a function start by the unwind data, its rel32 at +25; +32 the disp32 of `mov [rip+disp32], rax` (`48 89 05` at +29) - the buffer object's global 0x3852918, 0 until the set-up runs | found, its RVA to check | 0x31FB55 |
| T-E-C1 | `8B 2D ? ? ? ? 31 D2 48 8D 1D ? ? ? ? 41 BA 01 00 00 00 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 8B 3C AB 8D 34 0F 81 FE ? ? ? ? 0F 87` | in the geometry records' maker 0x38CA30: +2 the disp32 of `mov ebp, [rip+disp32]` - the frame slot 0x38527D8; +11 the rel32 of `lea rbx, [rip+disp32]` - the counters 0x3852B90; +21 to +35, 15 bytes of alignment filler, wildcarded; +39 `lea esi, [rdi+rcx]`, the slot's counter plus the vertex count in ecx, which a counting stub adds (the SIB `0F` naming rcx at +41); +44 the capacity, imm32 0x400000, compared with esi - raised by the part; +48 `ja rel32` to the refusal, its rel32 at +50, past the match (to 0x38CCC2) - with `DIAGNOSTIC_MODE=1` pointed at a counting stub | found, its RVA to check | 0x38CA8C |
| T-E-C2 | `8B 15 ? ? ? ? 31 ED 48 8D 35 ? ? ? ? 41 BA 01 00 00 00 ? ? ? ? ? ? ? ? ? ? ? ? 8B 3C 96 8D 1C 0F 81 FB ? ? ? ? 0F 87` | in the ropes' 0x4886B0: +2 the frame slot, the same as `T-E-C1`'s; +11 the counters (`lea rsi`), the same; +21 to +32, 12 bytes of filler, wildcarded; +36 `lea ebx, [rdi+rcx]`, alike (the SIB `0F` at +38); +41 the capacity, imm32 0x400000, compared with ebx - raised by the part; +45 `ja rel32` to the refusal, its rel32 at +47 (to 0x488736) - with `DIAGNOSTIC_MODE=1` pointed at a counting stub | found, its RVA to check | 0x4887CF |

## StaticBounds

The game reads the static bounds store's size once, from one call.
Its letter is `C`.
Findings: `staticbounds2_findings_for_the_clean_room_build.txt`.
The key's name, its default and its rule are those of Nochala's Static Bounds Limit Adjuster v1.0.2 (commit 61507bc), whose two patterns and hooks raise the pool's +0x18 alone and leave the name map at the game's size.

- **The way there:** the start-up (0x509BF0) loads `gameconfig.xml` into the configuration [0x2E96FB0], hash to value; state 1 of 0x23F0 reaches 0x7740, which for each of 10 stores calls the store's init 0x342F0 (StaticBounds at 0x77D9) and registers it.
  About 15.5 s after `DllMain`, before the landing page, on one thread.
- **The init:** done if +0x98 is set; the default from +0x10, which the constructor set to 1; the name's hash from +0x18, joaat, `StaticBounds` 0x2CDFE406, computed at run time, nowhere in the code as an immediate; the lookup 0x149060(configuration, hash, default) at 0x3433E, -1 in the map meaning the default; the size kept in `r14d`, only `eax` read after the call.
  Then the pool (0x15A6C0: its capacity, then its allocation), the name map's buckets (a prime from a table, +0x88, array +0x80), entries of 12 bytes (+0x78), free head +0x8C, in use +0x90, and +0x10 = size, +0x98 = 1.
  My boots' read-backs: 16000 slots make a name map of 4,621 buckets and 32767 slots one of 13,297, each with 10,406 names and 10,406 slots in use.
- **What follows that size:** the pool's capacity +0x50 (the pool's +0x18), the module's count +0x10, the next modules' streaming base, the streaming array and total, and, later (0x10460E0 -> 0x93A190 -> 0x92C170), the box streamer: AABBs +0xD8, flags +0xE8, its count +0xF8, its u16 counts +0xE0, +0xE2, +0xF0, +0xF2.
  A name comes in at 0x93AA40, a slot from the pool, then the map's entry from its free list.
- **The traps:** a full name map traps at 0x93AA93, a full pool at 0x15A84C.
  With the redirect the two are equal and the pool's comes first; `crashcontext.log` then says `StaticBoundsStore, N, N`.
- **The store** (`C-E-D1`): +0x08 its streaming base, +0x10 its count, +0x18 its name, +0x38 its pool, +0x50 the pool's capacity, +0x60 a word whose low 30 bits are the slots in use, +0x88, +0x90, +0x98 as above, +0xF0 a u16 and +0xF8 the streamer's count.
- **Why 32767:** a size above 65535 wraps two u16 counts of the box streamer (0x92C19C, 0x92C1DC), 65536 being an empty tree; a slot at 65536 or above is cut to 16 bits in an entity's tag (0x105537D); a slot at 32768 or above comes out negative in an interior proxy (`movsx` at 0xC2EAEB), whose readers skip it or add the base.
  Slots go lowest first, so that last bites only with more in use.
- **A slot's cost:** about 73 bytes, 17 from the pool memory budget (below, "A pool's memory") and about 56 from the general allocator, the heap `HEAP_SIZE` sizes [inferred]: 16000 slots 1.10 MB in all, 32767 slots 2.27 MB.
- **Not known:** a 16-bit slot narrower than those found, outside what was traced; no boot has passed about 10,406 slots.

Each once in both dumps:

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| C-E-L1 | `48 89 D9 89 C2 41 89 F8 E8 ? ? ? ? 41 89 C6 48 8D 4E 38 89 C2 E8` | the store's size call in the store's init 0x342F0: +0 `mov rcx, rbx`, the configuration; +3 `mov edx, eax`, the hash of the store's name; +5 `mov r8d, edi`, the default; +8 `call`, the pool size lookup 0x149060 - the part's redirect lies there; +13 `mov r14d, eax`; +16 `lea rcx, [rsi+0x38]`, the store's pool; +20 `mov edx, eax`; +22 `call` 0x15A6C0, what comes back handed to the pool. Its shape without +16 on matches three times, so +16 to +22 stay in | found | 0x34336 |
| C-E-D1 | `48 8D 0D ? ? ? ? E8 ? ? ? ? BA 01 00 00 00 E8 ? ? ? ? B9 28 3A 00 00` | the store's streamer init's call: +0 `lea rcx, [rip+disp32]`, the store, RVA 0x3ED8D80, which the part's read-back reads; +7 `call` 0x93A190 | found | 0x1046805 |

## LightCapacity

Addresses are RVAs.
The design is in SPECIFICATIONS.md, sections 11.16.3 and 11.17, the research in `lightcapacity_findings_for_the_clean_room_build.txt` and `coronastore_findings_for_the_clean_room_build.txt`.
A frame's lights pass through these layers, sirens' among them:

| Layer | Where (RVA) | Holds | Past it |
|---|---|---|---|
| A job's list | 0xE15700, one list per 5 vehicles | 80 real lights, 200 coronas | silently dropped: the job's later vehicles lose lights |
| The frame's vehicle store | 0x4371B00, 0x4366D00 | 400 real lights, 928 coronas | no check: the jobs write on, over the stores' own write pointers |
| Vehicles a frame | 0x4362160 and three more arrays, 60 jobs | 300 | no check: the 301st overwrites the count itself |
| The scene store | 0x3AB0250, two of 1024 x 0x1C0 | 1024 | a farther light is dropped; a nearer one replaces slot (index AND 0xFF), not the farthest: a bug of the game's |
| The ray traced list | 0x3C53670, two of 1024 | 1024 within 196 m, first come, vehicles last | silently dropped |
| The ray traced upload | 0x492E13, object +0x70E0 | 512, the largest of range / distance | not in the ray traced passes |
| The shaders (LuminanceV's FRAME.md) | a culling mask of 16 words | 512 a wave, 128 a pixel | shader-bound, no patch reaches it |
| Reflection lists | 0x3C00250, three of 768 | 768, used 128 | not fed by vehicles' lights |
| The corona store | 0x41B4210 | 1088, `CORONA_STORE` by the part | silently dropped; past 1088 the LOD lights' batch's clamp must move with it (below) |

- **A siren's light:** the vehicle's lights function 0xE15B20 calls the siren lights 0xE9BCB0 when sirens are on and its other lights through six more functions; each ends in add-to-a-job's-list 0xE15700, seven call sites in six functions, which returns when full, count +0x14 against capacity +0x1C.
  A job's list: +0x00 coronas, +0x08 lights, +0x10 and +0x14 counts, +0x18 and +0x1C capacities, loaded together from `G-E-C4`'s start values; the arrays lie in the job's scratch of 0xB180 bytes, 200 x 0x30 and 80 x 0x1C0.
  A corona is added inline, with its own check.
  One job holds 5 vehicles, run in index order: 0xDE6870 registers a vehicle and kicks a job at every fifth, 0xF01040 kicks the last.
- **The frame's stores** (`G-E-A1`), reset once a frame: at a job's end `xadd` moves a write pointer by what it has and `memcpy` copies, no capacity anywhere; the 401st light's record lands on the pointers.
  Reasoned, not seen: the next job's `xadd` or `memcpy`, or the drain's walk, faults: an access violation in the drain from 0xF01130, in `memcpy` 0x2479F10 called from 0xE15AE2 or 0xE15ABA, or garbage in 0x3EF140.
- **The drain** 0xF010B0: every corona through 0xB3A850 into the corona store; every light through add-scene-light 0x3EF140 with a null mask.
  A frame runs the reset, the vehicles' registration, the other lights, then the drain, so vehicles' lights come last.
- **The scene store** (0x3EF140, then 0x3FA7B0): two arrays swapped each frame (0x3F30A0); the render count set once a frame by 0x3FB200, stable while that frame draws.
  Below 1024 a light is appended, its index and distance into the sort list, 8 bytes an entry, the list's count right behind it, its mark the byte 0x3D4239C.
  Past 1024 the list is sorted if marked; a light not nearer than the farthest is dropped, else it takes that entry's slot; but the index is stored (0x3FA7F3) and read (0x3FAA40) as a byte, so one of the frame's first 256 lights goes, and the farthest stays.
  The sort moves whole entries; no other code reads the list.
- **The ray traced list's add** 0x3FAEF0, while a ray traced effect is on: types 1, 2 and 4; two lists of 0x72010 bytes, their counts first.
  The upload (0x492DC0, then 0x3FCEC0) scores up to 1024 records and copies the first 512, the room its object has.
- **The reflection lists' add** 0x3FACE0: two sets 0x29A0C apart; a null mask never feeds them.
- **What a siren adds** (0xE9BCB0), for each of the 20 lights with a bone and not broken: a corona; and if the light's flag +0x45 is set and it is lit, with the setting's byte +0x38 at 0 it joins one of four groups, merged into at most 4 lights a vehicle, else it is a real light of its own.
  So the byte is what costs: `useRealLights` by the element order of `carcols.meta` (HINT for the name, the behaviour is the code's).
- **The complete sets,** by scan and references: the lights' store's base at 0xF00F85, 0xF011BB and the constructor's 0xF3E27A, which walks it to `cmp rsi, 0x2BC00` at 0xF3E35E; the coronas' at 0xF00F70, 0xF01109 and 0xF3E1B2; both bases also through 0xF00FE0 and 0xF00FE7 into the jobs' parameters +0x30 and +0x38, from the same registers, which the job does not read.
  The write pointers at 0xF00F77, 0xF00F7E, 0xF00F8C, 0xF00F93, 0xF01110, 0xF011AE, 0xF011C2, 0xF011E7, no other reader.
  The vehicles' count 0x4362AC0, referenced in 0xDE6870, 0xF01040, 0xF010B0 and 0xF3C140 alone, none comparing it with 300; the arrays behind it 0x4362160 (8 bytes), 0x4362AC8 (4), 0x4362F80 (16), 0x4364250 (4), their counts right behind them, shared by all jobs through the parameters 0x4366CA0.
  A job's scratch: 0x8C00 at 0xE1592B, 0xE15969, 0xE1596F, in 0xE15933 and 0xE1593D as -0x8C01 and -0xB181; 0x2580 at 0xE15924; 0xB180 at 0xE15944, and at a job's +0x70 at 0xDE6957 and 0xF01090, which the worker does not read; the sixty jobs at 0x4364710, 0xA0 bytes each.
  The 1024: 0x3FF at 0x3EF6EE, 0x3FA7D0, 0x3FAF08, 0x3FD134, 0x3FD139; 0x70000 at 0x3F335F, 0x3FB230 and the static loops 0x4B6D4F, 0x4B7785, 0x4B788A; 0x72010 at 0x3A4947, 0x3F33AE, 0x3FCFA6; 0x29A0C at 0x3F3385, 0x3FB25A.
  It also sizes, in `.data`, 0x3D37690 (count 0x3D38690), 0x3D3F0B0, 0x3D400C0, 0x3D401C0, 0x3D40398, 0x3D423A8, 0x3D463B0; on the heap 0x3D40378, capacity written at 0x3F1329; and the stacks of 0x3F2780 and 0x3FCEC0, so it is no constant to raise.
- **Not settled:** who sets the sort list's mark, and when: the code sorts the list past 1024 if it is set, so it may mark a list sorted or one that needs a sort; the instructions of the constructor's two references to the stores, known by their addresses alone; the value of the pointer 0x2888630; bit 0 of 0x289C098 (HINT: the view and occlusion tests); the name `useRealLights`; whether a job's +0x70 is read anywhere; the size of the vehicle pool; which code adds what to the scene store, 25 callers of 0x3EF140.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| G-E-A1 | `48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 89 15 ? ? ? ? 4C 8D 0D ? ? ? ?` | the frame's reset (RVA 0xF00F70): +3 the coronas' store (RVA 0x4366D00), +10 their write pointer (RVA 0x439D700), +24 the lights' store (RVA 0x4371B00), +31 their write pointer (RVA 0x439D708); the lights' store 0xAE00 past the coronas', 928 x 0x30, the coronas' pointer 0x2BC00 past the lights' store, 400 x 0x1C0, the lights' pointer right behind it | found | 0xF00F70 |
| G-E-A2 | `48 8D 1D ? ? ? ? 48 39 1D ? ? ? ? 0F 86` | the drain: +3 the coronas' store, +10 its write pointer, `G-E-A1`'s | found | 0xF01109 |
| G-E-A3 | `48 8D 35 ? ? ? ? 48 39 35 ? ? ? ? 76 ? 0F 1F 44 00 00 48 89 F1 31 D2 45 31 C0 45 31 C9 E8` | the drain: +3 the lights' store, +10 its write pointer, `G-E-A1`'s; +32 the call of add-scene-light (RVA 0x3EF140) with a null mask, its rel32 past the match | found | 0xF011BB |
| G-E-B1 | `69 05 ? ? ? ? CD CC CC CC 05 99 99 99 19 3D 32 33 33 33 77` | 5 vehicles a job: `imul eax, [rip+disp32], imm32`, +2 the disp32 of the vehicles' count (RVA 0x4362AC0), counted from past the imm32 at +6; then the test of a multiple of 5 | found | 0xDE68FE |
| G-E-B2 | `C7 44 1A 70 80 B1 00 00 F0 FF 05` | +4 a job's scratch size, 0xB180, at its +0x70, in the kick; the part writes 0x17340 | found | 0xDE6957 |
| G-E-B3 | `42 C7 44 0A 70 80 B1 00 00` | +5 the same in the last kick; the part writes 0x17340 | found | 0xF01090 |
| G-E-C1 | `8B 41 14 3B 41 1C 0F 83 ? ? ? ? 48 89 D7 49 89 CE 48 8B 59 08` | add-to-a-job's-list: 11 bytes past the function's start (RVA 0xE15700); +6 the jump when full, `jae rel32`, its rel32 at +8 to the function's way out (RVA 0xE158E7) | found | 0xE1570B |
| G-E-C2 | `4C 8D 48 0F 49 83 E1 F0 4C 89 CA 48 29 C2 49 8D A9 80 25 00 00 BB 00 8C 00 00 48 29 D3 48 81 C2 FF 73 FF FF 45 31 C0 48 81 FA 7F 4E FF FF BE 80 B1 00 00` | a job's scratch laid out: +17 the coronas' bytes, 0x2580; +22 the lights' bytes, 0x8C00; +32 -0x8C01; +42 -0xB181; +47 the sum, 0xB180; the part writes 0x3840, 0x13B00, -0x13B01, -0x17341 and 0x17340 | found | 0xE15916 |
| G-E-C3 | `48 05 00 8C 00 00 48 81 FA 00 8C 00 00` | +2 and +9 the lights' bytes; the part writes 0x13B00 to both | found | 0xE15969 |
| G-E-C4 | `48 89 7C 24 68 0F 28 0D ? ? ? ?` | +8 the rel32 of `movaps` to the lists' start values at .rdata 0x26399A0: 0, 0, 200, 80, 16-aligned for it | found | 0xE159B1 |
| G-E-D1 | `48 63 05 ? ? ? ? 48 3D FF 03 00 00 0F 8F ? ? ? ? 48 63 0D ? ? ? ? 48 8D 15 ? ? ? ? 8D 59 01 89 1D ? ? ? ? 88 04 CA` | the scene store's add (in RVA 0x3FA7B0): +3 its count (RVA 0x3D39ECC), +9 0x3FF, +22 the sort list's count (RVA 0x3D42398), +29 the sort list (RVA 0x3D40398), +38 the count's store, +42 the index stored as a byte; the part writes `89 04 CA`, `mov [rdx+rcx*8], eax` | found | 0x3FA7C9 |
| G-E-D2 | `0F B6 1C C1 F3 0F 11 74 C1 04 4A 8D 14 C1 E8` | +0 the index read back as a byte; the part writes `8B 1C C1 90`, `mov ebx, [rcx+rax*8]` and a `nop` | found | 0x3FAA40 |
| G-E-E1 | `48 69 C7 00 00 07 00 48 8D 15 ? ? ? ? 48 01 C2 48 89 15 ? ? ? ? 48 8D 05 ? ? ? ? 44 8B 04 B8 44 89 05 ? ? ? ? 48 69 C7 0C 9A 02 00 48 8D 0D ? ? ? ? 48 01 C1 48 89 0D` | the scene store's render side (in RVA 0x3FB200): +3 0x70000; +10 the store (RVA 0x3AB0250); +20 the render pointer (RVA 0x2888640); +27 the buffers' counts (RVA 0x3D4A520); +38 the render count (RVA 0x3D39ED0); +45 0x29A0C; +52 the reflection lists (RVA 0x3C00250); +62, past the match, their render pointer | found | 0x3FB230 |
| G-E-E2 | `48 69 C8 00 00 07 00 48 8D 15 ? ? ? ? 48 01 CA 48 89 15 ? ? ? ? 48 8D 0D ? ? ? ? C7 04 81 00 00 00 00 48 69 C8 0C 9A 02 00 48 8D 15` | its update side (in RVA 0x3F30A0): +3 0x70000; +10 the store, `G-E-E1`'s; +20 the current pointer (RVA 0x2888638); +41 0x29A0C | found | 0x3F335F |
| G-E-E3 | `48 69 C8 10 20 07 00 48 8D 15 ? ? ? ? 48 8D 34 0A 48 89 35 ? ? ? ? 8B 04 11` | +3 0x72010; +10 the ray traced lists (RVA 0x3C53670), a list's count first; +21 their current pointer (RVA 0x28886C0) | found | 0x3F33AE |
| G-E-F1 | `4C 8B 25 ? ? ? ? 45 8B 34 24 41 81 FE FF 03 00 00 0F 8F` | the ray traced list's add: +3 the same pointer as `G-E-E3`'s; +14 0x3FF; +18 the jump when full, `jg rel32`, its rel32 past the match to the function's way out (RVA 0x3FB148) | found | 0x3FAEFD |
| G-E-F2 | `44 0F B6 05 ? ? ? ? 48 81 C1 E0 70 00 00 BA 00 02 00 00 E8` | the upload: +11 0x70E0, the records in its object; +16 512; +20 the call of the builder (RVA 0x3FCEC0), its rel32 past the match | found | 0x492E04 |
| G-E-F3 | `4B 63 0C 88 48 81 F9 FF 02 00 00` | the reflection lists' add: +7 0x2FF | found | 0x3FAD63 |
| G-E-G1 | `8B 0D ? ? ? ? 48 81 F9 3F 04 00 00 0F 87` | the corona store, twice, one count: +2 its count (RVA 0x41C0E10); +9 0x43F; the second match at RVA 0xB6ED70 | found | 0xB3A780 |
| G-E-G2 | `44 8B 81 00 CC 00 00 49 81 F8 3F 04 00 00 0F 87` | the drain's add to it (in RVA 0xB3A850): +3 0xCC00; +10 0x43F | found | 0xB3A8D0 |
| G-E-H1 | `B8 58 80 01 00 E8 ? ? ? ? 48 29 C4` | the worker's frame (RVA 0x142760): +1 0x18058 | found | 0x14276C |
| G-E-H2 | `48 8D 44 24 50 49 89 04 24 4C 89 E1 41 FF 54 24 60` | a job's scratch, rsp+0x50: +4 0x50, right before `call [job+0x60]`; so 0x18008 bytes for it, which the part's 0x17340 fits with 0xCC8 to spare | found | 0x14284B |
| G-E-K1 | `F0 48 0F C1 08 48 8B 54 24 40 E8` | a job's coronas at its end: `lock xadd` of the write pointer, its coronas into rdx; +10 the call of the copy, its rel32 past the match | found | 0xE15AB0 |
| G-E-K2 | `4C 69 C6 C0 01 00 00 4C 89 C1 F0 48 0F C1 08 E8` | a job's lights at its end: +3 0x1C0, the bytes of its n, `lock xadd` of the write pointer; +15 the call of the copy, its rel32 past the match | found | 0xE15AD3 |
| G-E-I1 | `66 66 66 66 66 2E 0F 1F 84 00 00 00 00 00 44 8B 81 00 CC 00 00 49 81 F8 3F 04 00 00 0F 87 ? ? ? ? 41 8D 70 01 44 89 C0 F0 0F B1 B1 00 CC 00 00 75 DB` | the general add (in RVA 0xB3A850): +0 a 14-byte nop before its loop, reached only by falling through; +17 and +45 the count's disp, 0xCC00; +24 0x43F; +30 the `ja`'s rel32 to RVA 0xB3A9AF; `G-E-G2` whole at +14 | found | 0xB3A8C2 |
| G-E-I2 | `48 8D 04 49 48 C1 E0 04 48 8D 0D ? ? ? ? 41 0F 28 1E 0F 29 1C 08` | the first inlined add (in RVA 0xB3A4D0): +11 the store (RVA 0x41B4210) | found | 0xB3A7E2 |
| G-E-I3 | `48 8D 04 49 48 C1 E0 04 48 8D 0D ? ? ? ? 0F 28 44 24 30 0F 29 04 08` | the second (in RVA 0xB6E6F0): +11 the store | found | 0xB6EDD2 |
| G-E-I4 | `8B 05 ? ? ? ? 89 47 10 85 C0 7E 5C 8D 04 40 C1 E0 04 05 80 01 00 00` | the copy to the draw list (in RVA 0xB1C510): +2 the count (RVA 0x41C0E10), +20 0x180 | found | 0xB1C587 |
| G-E-I5 | `48 85 C0 74 1E 48 8D 15 ? ? ? ? 48 89 C1 49 89 D8 E8` | the copy: +8 the store, memcpy's source | found | 0xB1C5D7 |
| G-E-I6 | `48 8D 0D ? ? ? ? 48 8D 54 24 50 4C 8D 44 24 40 E8 ? ? ? ? 48 8B 44 24 50 48 89 05` | the LOD lights' job's parameters (in RVA 0x314200): +3 the store, +18 the call of `G-E-I7` | found | 0x3145AA |
| G-E-I7 | `48 89 0A 48 81 C1 00 CC 00 00 49 89 08 C3` | hands the store and its count out as pointers, called once: +6 0xCC00 | found | 0xB3CAA0 |
| G-E-I8 | `BD 40 04 00 00 48 8B 9C 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 66 66 66 66 2E 0F 1F 84 00 00 00 00 00 8B 0B 8D 14 31 81 FA 40 04 00 00 0F 43 D5 89 C8 F0 0F B1 13 75 EA 29 CA 48 8D 04 49` | the job's batch (in RVA 0x2C15C0): +1 and +41 0x440, its clamp; +56 `29 CA 48 8D 04 49`, with rsi what the job brought, edx the new count, ecx the old | found | 0x2C28AE |
| G-E-I9 | `81 BC 24 90 00 00 00 3F 04 00 00 0F 87 ? ? ? ? 48 8B 84 24` | the job's own limit: +7 0x43F; +13 the `ja`'s rel32 to RVA 0x2C211C | found | 0x2C1EA4 |
| G-E-J1 | `49 8D 92 00 CC 00 00 BD 00 1C 00 00 4C 29 DD 49 81 C3 FF E3 FF FF 49 81 FB FF 17 FF FF 48 0F 42 D0 41 BD 00 E8 00 00` | the job's scratch, a check: +3 0xCC00 for coronas, +8 0x1C00 for four lists, +35 0xE800 in all | found | 0x2C1384 |
| G-E-J2 | `48 8B 41 08 48 89 05 ? ? ? ? 8B 51 10 89 15 ? ? ? ? 31 D2 48 85 C0 0F 94 C2 C1 E2 03 89 15` | command 0xDF's handler, a check: +7 the render pointer (RVA 0x42D5C20), +16 the render count (RVA 0x42D5C28), +33 past the match the extras' count (RVA 0x42D5C2C) | found | 0xB3B050 |
| G-E-J3 | `F0 FF 05 ? ? ? ? 89 D6 49 89 CC 48 63 3D` | the draw (in RVA 0xB22640), a check: +3 its mark (RVA 0x42D5C38), +15 past the match the render count's read | found | 0xB22681 |
| G-E-J4 | `48 89 F8 48 C1 E0 05 E8 ? ? ? ? 48 29 C4 49 89 E6 48 83 EC 40` | the draw: `alloca(count x 0x20)`, a check | found | 0xB22811 |
| G-E-J5 | `48 8D 04 BD 0F 00 00 00 48 83 E0 F0 E8 ? ? ? ? 48 29 C4 49 89 E5` | the draw: `alloca(count x 4)`, a check | found | 0xB2285B |

The raise's 16 writes go through these rows, for a job's 180 lights and 300 coronas; the top, with 0x20 to spare, is 187 lights beside the 300 coronas; 5 vehicles of 32 lit sirens take 160.
The moved stores need no constructor: the records are copied whole and read by value; the game's own, which its constructor keeps, nothing reads then.

The stubs' sites:

- **`G-E-C1`'s jump** at 0xE15711, `0F 83 D0 01 00 00`, to the way out: `48 83 C4 20` `add rsp, 0x20`, `5B` `pop rbx`, `5F` `pop rdi`, `5E` `pop rsi`, `41 5E` `pop r14`, `41 5F` `pop r15`, `C3` `ret`.
  None of the seven reads a flag and the first writes them anew, so a `lock inc` before them changes nothing the game sees; the way that adds a light falls through to the same address from `inc dword [r14+0x14]`, so it rests on no flags from the jump's `cmp`.
- **`G-E-F1`'s jump** at 0x3FAF0F, `0F 8F 33 02 00 00`, to the way out: `48 83 C4 28` `add rsp, 0x28`, `5B` `pop rbx`, `5F` `pop rdi`, `5E` `pop rsi`, `41 5C` `pop r12`, `41 5E` `pop r14`, `41 5F` `pop r15`, `C3` `ret`, the flags as above.
  The way out is also reached from 0x3FAF1E, 0x3FAF2C, 0x3FB136 and 0x3FB140 and falls through from 0x3FB146, so this rel32 alone counts the full list and nothing else.
- **A job's two copies at its end:** 0xE15ABA `E8 51 44 66 01` and 0xE15AE2 `E8 29 44 66 01`, to the game's copy 0x2479F10: rcx the place in the frame's store, the write pointer's old value by `lock xadd`; rdx what the job holds in its scratch; r8 the bytes, n x 0x30 and n x 0x1C0; rax the pointer's address, read by nothing past the call, so a stub may use it.
  Each is skipped when n is 0 and runs once a job, with the job's whole count, after every vehicle of that job.
  The jobs run side by side; a job's list lies on a worker's stack.

**The corona store** (SPECIFICATIONS.md, section 11.17.1):

- **The object 0x41B4210:** 1088 records of 0x30 bytes; +0xCC00 the count; +0xCC08 and +0xCC10 two textures; +0xCC18 to +0xCCB8 the visual settings `coronas.*`.
  A record: +0x00 position; +0x0C flags in the low 16 bits, two angles above them; +0x10 direction; +0x1C a float; +0x20 size; +0x24 color; +0x28 intensity; +0x2C zBias.
- **Who adds, in a frame's order:** an effect list in the update (0xB6E6F0, inlined); the map's light entities (0x400100); the LOD lights' job, up to 1088 by itself; small sources (0x1B72530, 0x1840500, 0x233B0, 0x1853C70, 0x1B70EB0, 0xEE3E10, and 0xB3A4D0's list, inlined); some vehicle classes directly in the +0x2E0 loop; last the drain, every job's corona, sirens' among them.
  So the vehicles' coronas are the ones dropped: nothing after the drain reaches an add by a direct call.
  The LOD job runs beside it on a worker, so it may land later and lose its coronas instead (HINT: rarely).
- **The general add** 0xB3A850: eleven callers, lock-free, refused silently at 0xB3A8DE; nothing if the size is 0, the intensity not above 0 or the byte 0x2FA2860 set.
  The two inlined adds do the same by rip, each with a `lea` of the store of its own.
- **The LOD lights' job** 0x2C15C0, kicked in 0x314200, its scratch laid out by 0x2C1360: each LOD corona goes into its scratch while its own limit passes, so a job holds 1088 and the later buckets lose theirs, whatever their distance.
  At its end one batch: `mov ebp, 0x440`, `mov ecx, [rbx]`, `lea edx, [rcx+rsi]`, `cmp edx, 0x440`, `cmovnc edx, ebp`, `lock cmpxchg [rbx], edx`, `sub edx, ecx`, a memcpy of edx x 0x30 to the store + ecx x 0x30; the store and the count's address are its parameters, handed out by 0xB3CAA0.
- **From the store to the screen:** 0xB1C510 allocates count x 0x30 + 0x180 bytes from the draw list and copies the records; command 0xDF hands the copy on; 0xB6EEB0 adds up to 8 records past the count, the 0x180; 0xB22640 draws: `alloca(count x 0x20)`, `alloca(count x 4)`, quads in batches of 341 of immediate-mode memory, no constant buffer or shader counting coronas.
  A failed allocation: no coronas that frame, no crash.
  Every record past the copy is read through the copy, so nothing behind the store knows its size.
  Growing with the count: the stack, 0x24 bytes a corona a draw, of 0x80000 committed on the render threads; the copy; the time.
  A corona's index is a word, 65535 at most.
- **The complete set sized by 1088:** 0x43F at 0xB3A789, 0xB6ED79, 0xB3A8DA, 0x2C1EAB; 0x440 at 0x2C28AF, 0x2C28D7; 0xCC00 at 0xB3A8D3, 0xB3A8EF, 0xB3CAA6, 0xB3B8BF, 0x2C1387; 0xE800, -0xE801, 0x1C00 and -0x1C01 at 0x2C13A7, 0x2C139D, 0x2C14E6, 0x2C138C and 0x2C1396, and +0x70 in 0x3146C8; 0xCCB4 at 0xBD9CAA, the constructor's loop.
  Unrelated, read one by one: 0x43F at 0x763EA5 and the other 156 of 0x440.
  The store's base is named at 19 sites, each a `lea`, by a count of rip-relative references: 0x23FA8, 0x3145AA, 0x4004B8, 0xB0DFC1, 0xB1C4E5, 0xB1C5DC, 0xB22625, 0xB3A7EA, 0xB6EDDA, 0xBD9C6C, 0xDF07AA, 0xDF0CC4, 0xE09637, 0xEE4602, 0xF01122, 0x1840AAC, 0x185512B, 0x1B71D6C, 0x1B72E51.
  What hands the store out is called once, at 0x3145BB.
  The count is read by 6 rip reads, and by `[reg+0xCC00]` at 0xB3A8D0, 0xB3A8EB, 0xB3B8BD.
- **The raise, 13 writes:** the nop at `G-E-I1` +0 becomes `lea rcx, [new store]` and a nop of 7, turning the old store the callers hand it into the new one before the loop; the count's disp at +17 and +45, and `G-E-I7`'s +6, become the count less the new store; the limits at `G-E-I1` +24 and each `G-E-G1` +9 become N - 1, `G-E-I8`'s +1 and +41 N; the rel32 at `G-E-I2` +11, `G-E-I3` +11, `G-E-I5` +8 and `G-E-I6` +3 lead to the new store.
  The count and everything past +0xCC00 stay, so the other fifteen references to the old base, the reset at 0xB3B8BD and the constructor stay true; the job's own 1088 stays: it keeps a city at night from taking more.
- **The trap:** at a count past 1088 the old clamp gives edx = 0x440 below ecx, the `cmpxchg` lowers the count and `sub edx, ecx` wraps, so memcpy gets about 200 GB and faults (reasoned, not seen: an access violation in memcpy 0x2479F10, called from 0x2C290B).
  The adds write with `movaps`, so the new store lies 16-aligned.
- **The counters:** the four jumps, `G-E-I1` +30, each `G-E-G1` +15 and `G-E-I9` +13, and where each leads, up to the first instruction that writes the flags, none before it reading one: 0xB3A9AF `0F 28 34 24 0F 28 7C 24 10 44 0F 28 44 24 20 48 83 C4 38`, 0xB3A826 `0F 28 74 24 20 0F 28 7C 24 30 44 0F 28 44 24 40 44 0F 28 4C 24 50 48 83 C4 68`, 0xB6E798 `42 C6 44 2E 6B 00 66 90 48 FF C3 41 8B 47 04 48 39 C3`, 0x2C211C `4C 8B 44 24 58 44 0F 28 DC 0F 28 A4 24 A0 00 00 00 41 0F 50 C1 84 C0`.
  The batch's six bytes at `G-E-I8` +56: rax is free, since the moved `lea` writes it, and what follows them, `shl rax, 4`, reads no flag.
- **No pattern protects:** that the LOD job has no dependency on the drain; that nothing after the general add's record reads rcx (to 0xB3A9C4); that the render threads have 0x80000 of stack committed, by the PE's reserve and `CreateThread`'s flags.
- **Not settled:** how many each source brings; how often the LOD job lands after the drain; the stages of the sources reached through tables; the time and the GPU's cost at a full store.

## Population

The design is in SPECIFICATIONS.md, section 11.16.4, a raise's in `vehiclemodelbudget_findings_for_the_clean_room_build.txt`, section 7, and `pedmodelbudget_findings_for_the_clean_room_build.txt`, section 8.
A number in MB counts 1,000,000 bytes, as the game's defaults do; HINT marks what is inferred and not shown.

- **The budget** in bytes, inlined at each use: a network game in progress ? the network table[level] : (int)((float)the story table[level] x the variety).
  Vehicles: level 0x2888D10, story table 0x2888CF0, network table 0x2888D00, 4 dwords each, variety 0x28CACE0, a float.
  Peds: level 0x2888D14, story table 0x2888CD0, network table 0x2888CE0, variety 0x2888B80.
  The flag: 0x29CB211 ("Game In Progress").
  Two getters (`O-E-C1`) return it doubled, for the crash context's line alone, which prints it in a network game only (0x4600, its print at 0x6350).
  Their 62 bytes are the same, so no pattern of the table, which wants one match, tells them; a mask at the calls' targets: `8B 05 ? ? ? ? 80 3D ? ? ? ? 00 74 11 48 8D 0D ? ? ? ? 8B 04 81 F3 48 0F 2A C0 EB 17 48 8D 0D ? ? ? ? 8B 04 81 F3 48 0F 2A C0 F3 0F 59 05 ? ? ? ? F3 0F 2C C0 01 C0 C3`: +2 the level; +8 the flag, its `cmp`'s imm8 behind the disp32; +18 the network table; +35 the story table; +51 the variety.
  The cast is `cvttss2si`: a product at 2^31 bytes gives 0x80000000, the margin's add wraps that to a large positive number, the requests never stop, the excess wraps negative and stays at 0, and the budget is gone altogether.
- **The tables:** entries 1 to 3 come from the set-up 0x4BD7A0, called once (0x4BD5C8) at the end of the population streaming's init 0x4BB450, so at each run of it, the game's start and a session's: entry = a tunable of `CD_GLOBAL` (0xE3AFC5BD) by a name's hash, its default the configuration's value x 1000, + 1,500,000.
  Twelve blocks of 0x38 bytes, story peds, network peds, story vehicles, network vehicles, three each; their stores, the tables' only writers by a scan of the code: 0x4BD7E4, 0x4BD81C, 0x4BD854; 0x4BD88C, 0x4BD8C4, 0x4BD8FC; 0x4BD934, 0x4BD96C, 0x4BD9A4; 0x4BD9DC, 0x4BDA14, 0x4BDA4C.
  Entry 0 has no writer: `.data`'s 1,500,000 for each table.
  At TweaksV's start the tables hold `.data`'s other numbers - vehicles 17.5, 29.5, 36.5 MB, peds 28.5, 37.5, 48.3 MB - the level 3, the variety 1.0 and the object zeros; the object's +0x6208 is -1 after the init.
- **`gameconfig.xml`:** the configuration is [[0x2E96FB0]+0x18]; CGameConfig's member at +0x4F4 is the block `ConfigPopulationStreaming`, which the game's own file leaves out, of `PedsSP` +0x4F4, `PedsMP` +0x500, `VehiclesSP` +0x50C, `VehiclesMP` +0x518, each of `Level1Kb`, `Level2Kb`, `Level3Kb`, dwords.
  The names by the parser's hashes in the window `28E0700:900`, a descriptor holding the case-sensitive joaat at +0, the offset at +8, the type at +0x10: 0xB90C36F2; 0x5734E391, 0x63432F09, 0x754AC6B3, 0x281C138B; 0x4D3F3EC4, 0x4ED77A3E, 0x9EBE88D3.
  The defaults, from the constructor 0x12BC2C0: peds 54,000, 72,000, 92,000, vehicles 32,000, 56,000, 70,000, in story mode and a network game alike, which gave the tables at 180 s of a boot exactly.
  So the story budget in MB at the slider's top is (`Level3Kb` / 1000 + 1.5) x 6, "Kb" being a thousand; the merge 0x12BC780 copies a value only when it is not negative, so a partial block holds.
  The form, for a budget of 600 MB: `<ConfigPopulationStreaming><VehiclesSP><Level3Kb value="98500"/></VehiclesSP></ConfigPopulationStreaming>` directly inside a `<Config type="CGameConfig">` that applies on PC.
  `PedMemoryMultiplier` and `VehicleMemoryMultiplier` (cfg+0x38, +0x54) have no reader among the 359 loads of the holder, and the tables stood at the defaults with 2,500 and 4,500 in the file: neither is in the budget.
- **The variety:** the settings' apply 0x5003A0 stores it, at 0x5004F5 for the peds and 0x50050C for the vehicles, the only stores: the setting (+0xB4 `PedVarietyMultiplier`, +0xB8 `VehicleVarietyMultiplier`, the menu's Population Variety, 0 to 1) x 5 + 1, clamped to 0.05 to 10, so 1 to 6.
  The check 0x503380 puts a setting outside 0 to 1 back to its default, so the clamp's 10 cannot be reached by `settings.xml` (HINT until a boot shows it).
- **The level:** 3 from `.data` on; 0x4D5030 (the peds' 0x4D5080) stores a level up to 3 and level / 3 at 0x28CACD4 (0x28C23BC), which also scales how many cars and peds the game wants, and is reached only by a hop of the protected code (HINT: a native's handler); 0x4D5010 and 0x4D5060 set 3 and 1.0 again (HINT: a script's clean-up).
- **The object** 0x3D73F80, in `.data`; vehicles: +0x6214 bytes in use, a signed dword; +0x6224 what is over the budget, at least 0; the lists +0x3600 traffic of the current pop cycle, +0x3A00 loaded but outside it, +0x3E00 trains, helicopters and two named models (HINT on the types), +0x4200 boats, +0x4A00 dropped but still loaded; +0x62BC requests in flight; +0x623C the weapons' share, in use too; +0x6234 and +0x6238 the mods' pieces.
  Peds: +0x620C bytes in use; +0x621C what is over; +0x6204 the schedule's MaxScenPeds, +0x6208 its override; the lists +0x4E04 the population's, +0x5204 in a group the current schedule does not weigh, +0x5604 in no group (HINT: and the cops), +0x5A04 a network game's chosen models, +0x5E04 asked for directly until loaded.
  Each list is 256 dwords, ended by 0xFFFF or by a value not below the model count, `O-E-P3`'s word; one adder stops silently at 256.
  At +0, 0x240 records of 24 bytes for vehicles, peds and weapons.
  The displays (`O-E-C1`), the update's sums (0x4BEB02, 0x4BEB38) that only the crash context reads, count the mods' pieces and a streamed ped's parts twice; every compare reads +0x620C and +0x6214 alone.
- **What counts:** a model's load callback 0x4CE740 (from 0x1334A70, by the model info's +0x9D & 0x1F: 5 a vehicle, 6 a ped, 4 a weapon) adds through 0x4CDF10 the two sizes of the model and of its dependencies, shared ones once, whoever asked for it; high detail assets come and go on top (0x4C2860), and the common assets are in at the init.
  So a budget counts bytes, not models, and is no video memory budget, though a car's bytes are mostly video.
- **Past it,** nothing ends the game and nothing is logged or shown.
  Vehicles: the update's model step 0x4BF990 and the ambient step 0xF06C10 ask for one more model (0x4C5A30) while the use is below the budget less 1.5 MB, the stop, and at level 3 below a floor of 9 models in +0x3600 whatever the use.
  Peds: the gate in 0x4BF990, every 64 frames, about once a second, asks (0x4C7930) while the use is below the budget less 1.5 MB less (n >> 1) x 1.5 MB, n the override or else the schedule's number, and at level 3 below a floor of 6 models in +0x4E04.
  0x4C1FC0 releases what nothing holds, every frame while over the budget, else every 32nd.
  A new ambient ped takes the loaded model of its group with the fewest peds, the model info's +0x158 (0x4D24F0), so repeats are about peds divided by models; the number of cars wanted reads no budget, but a model takes only so many instances (0x4D2D10), so too few models can hold traffic under it.
- **`VehicleStruct`:** the pool behind `O-E-V3`'s pointer, stride 0x34C, 200 slots in the files seen, one per loaded vehicle model.
  A full pool's New ends the game in an `int3`; 0x4C5A30 stops traffic's requests at the size, and 0x4BEB50 releases unused models from 20 below it.
  The ped models' request 0x4C7930 has no such check, and no pool of `gameconfig.xml` is named for a loaded ped model (HINT: none exists).

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| O-E-C1 | `8B 35 ? ? ? ? C1 FE 14 8B 3D ? ? ? ? C1 FF 14 48 8D 1D ? ? ? ? 48 89 D9 E8 ? ? ? ? 89 C5 C1 FD 14 48 89 D9 E8 ? ? ? ? 89 C3 C1 FB 14 48 8D 0D` | the crash context's line of both memories: +2 the peds' display 0x3D7A240; +11 the vehicles' 0x3D7A244; +21 the object 0x3D73F80; +28 the ped getter's call, to 0x4BDAA0; +41 the vehicle getter's, to 0x4BDA60; +54, past the match, the rel32 of its text, 0x2828A4E, which must read `Ped Memory Use/Budget : %d/%d\n` | found | 0x631D |
| O-E-V1 | `8B 05 ? ? ? ? 0F B6 0D ? ? ? ? 4C 8D 35 ? ? ? ? 48 8D 1D ? ? ? ? EB` | in the check before a vehicle model's request 0x4D4F60: +2 the level; +9 the flag; +16 the story table; +23 the network table, each what the getter reads | found | 0x4D4F76 |
| O-E-V2 | `41 8B 14 96 0F 57 C0 F3 48 0F 2A C2 F3 0F 59 05 ? ? ? ? F3 0F 2C D0 81 C2 A0 1C E9 FF 39 97 14 62 00 00 7D ? 48 89 F9 E8` | the same function: +16 the variety; +26 the margin, -1,500,000; +32 the use's offset, 0x6214; +42, past the match, the rel32 of the request, 0x4C5A30 | found | 0x4D4FC0 |
| O-E-V3 | `48 8B 05 ? ? ? ? 8B 68 28 C1 E5 02 C1 FD 02 2B 68 18 83 C5 14 85 ED 0F 8E` | in the release 0x4BEB50: +3 `VehicleStruct`'s pointer 0x471D908; +9 its used at +0x28, the top two bits off by the shifts; +18 its size at +0x18; +21 the margin 20 | found | 0x4BEB63 |
| O-E-V4 | `48 8B 05 ? ? ? ? 48 8B 58 18 48 8B 35 ? ? ? ? 69 BB F4 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 40 67 D1 1F` | the set-up's head, 7 bytes behind its start, three pushes and `sub rsp, 0x20`: +3 the configuration's holder 0x2E96FB0; +10 its +0x18; +14 the tunables 0x29C76F0; +18 the first block, `O-E-P5`'s: +20 its offset, +24 the factor 1000, +29 `CD_GLOBAL`, +34 its name's hash | found | 0x4BD7A7 |
| O-E-V5 | `44 8B 35 ? ? ? ? 8B 05 ? ? ? ? 89 84 24 ? ? ? ? 44 8B 3D ? ? ? ? 8B 35` | the crash context, right before `O-E-C1`, whose first two bytes end it: +3 the ambient cars on the map 0x439D758, "Total Ambient Cars On Map"; +9 0x47968D0 and +23 0x4796B60, the local vehicles' target and total, a network game's, not read | found | 0x6302 |
| O-E-V6 | `41 0F B7 C6 89 34 87 E8 ? ? ? ? 0F 28 74 24 40 48 83 C4 58` | a writing half's: at the end of the init 0x4BB450, +7 the set-up's one call, to `O-E-V4` less 7, a function start | found | 0x4BD5C1 |
| O-E-V7 | `69 BB 0C 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05 ? ? ? ? 48 8B 35 ? ? ? ? 69 BB 10 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05 ? ? ? ? 48 8B 35 ? ? ? ? 69 BB 14 05 00 00 E8 03 00 00 B9 ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05` | a writing half's: the vehicles' three story blocks, the set-up's seventh to ninth: +2, +58, +114 the offsets 0x50C, 0x510, 0x514; +39, +95, +151 the margin 1,500,000; +45, +101 and, past the match, +157 the rel32 of the stores, which must give the story table + 4, + 8, + 12 | found | 0x4BD909 |
| O-E-P1 | `8B 86 10 62 00 00 03 86 0C 62 00 00 03 86 2C 62 00 00 03 86 30 62 00 00 89 05` | the update's sum of the peds' display: the object's +0x6210, +0x620C, +0x622C, +0x6230; +26, past the match, the rel32 of its store, which must be `O-E-C1`'s display | found | 0x4BEB02 |
| O-E-P2 | `41 8B 8C 24 08 62 00 00 85 C9 79 08 41 8B 8C 24 04 62 00 00 41 8B AC 24 0C 62 00 00 D1 F9 69 D9 A0 1C E9 FF 81 C3 A0 1C E9 FF 8B 0D` | the gate in 0x4BF990: the object's +0x6208, +0x6204, +0x620C and the two margins; +44, past the match, the rel32 of the level's load, which must be the getter's. Further on, read after `IsReadable`: +0x7A `41 8B 9C AC 04 4E 00 00`, the population's list; +0xEA `83 FD 06 0F 9C C0 83 F9 03 0F 94 C1`, fewer than 6 and level 3; +0x102 the call of the request 0x4C7930 | found | 0x4BFF56 |
| O-E-P3 | `48 8D 82 04 4E 00 00 4C 8D 8A 04 56 00 00 4C 8D 82 04 52 00 00 48 81 C2 04 5E 00 00 48 89 44 24 20 48 8D 4C 24 70 E8 ? ? ? ? 0F B7 05` | in the release 0x4C1FC0: the object's +0x4E04, +0x5604, +0x5204, +0x5E04; +46, past the match, the rel32 of the model count, the word at 0x28D2DE0 | found | 0x4C263E |
| O-E-P4 | `48 8B 0D ? ? ? ? 48 98 48 0F AF 05 ? ? ? ? 48 8B 04 01 8B 80 58 01 00 00 42 89 44 BC 40` | the read of the least used model, in 0x4D24F0: +3 the model info table 0x444AF90; +13 its stride, a qword, 0x444AFA8; +23 a model's peds at +0x158. The module reads the streamed flag beside it, +0x286 bit 1 | found | 0x4D2631 |
| O-E-P5 | `69 BB F4 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 40 67 D1 1F E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05` | a writing half's: the peds' story block for level 1, right behind `O-E-V4`'s head's pointers: +6 the factor 1000; +39 the margin 1,500,000; +45, past the match, the rel32 of its store, which must give the story table + 4 | found | 0x4BD7B9 |
| O-E-P6 | `69 BB F8 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA D0 3C 99 0A E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05` | the same for level 2, the story table + 8 | found | 0x4BD7F1 |
| O-E-P7 | `69 BB FC 04 00 00 E8 03 00 00 B9 BD C5 AF E3 BA 5C 10 5F F4 E8 ? ? ? ? 48 89 F1 89 C2 41 89 F8 E8 ? ? ? ? 05 60 E3 16 00 89 05` | the same for level 3, the story table + 12 | found | 0x4BD829 |
| O-E-P8 | `F2 0F 10 86 B4 00 00 00 0F 59 05 ? ? ? ? 0F 58 05 ? ? ? ? F3 0F 10 0D ? ? ? ? 0F 28 D0 0F C2 15 ? ? ? ? 01 66 0F 38 25 D2 66 0F 50 C2 F3 0F 10 15 ? ? ? ? 0F 28 DA A8 01 75 07 0F 28 D9 F3 0F 5D D8 F3 0F 11 1D` | the settings' store of the peds' variety, only logged: +11 and +18 the vectors 5.0 and 1.0; +26 the 10.0; +76, past the match, the rel32 of the store, which must be the getter's variety | found | 0x5004AD |

The population object's fields lie within its first 0x62C0 bytes.

## Ped pool

The design is in SPECIFICATIONS.md, section 11.16.5, each fixed array and a raise's design in `pedpool_findings_for_the_clean_room_build.txt`, sections 4 and 9.

- **The size:** `<PoolName>Peds</PoolName><PoolSize value="256"/>` in `gameconfig.xml`, the hash 0x8DA12117, which the game's own crash log gives as `Peds: 0/256`.
  The start-up 0x509BF0 looks it up with a default of 1 and hands it to the setter (`N-E-C1`), which stores the game's number of peds, read by the getter 0xD4EA00; the pool's maker 0xD4E7A0, called once, from the factory's constructor 0xD4EF80, looks it up again with that number as the default (`N-E-M2`), and makes the pool (`N-E-M1`).
- **The pointer,** kept obfuscated, its fields in `N-E-A1`: bit 0 of the flag clear, no pool; else r = rol64(keys, 0x1E), x = rol64(value ^ r, 0x20), x = rol64(x, (r & 0x1F) + 2), the pool = ~x.
- **A pool:** +0x08 the slots, +0x10 a flag byte per slot with bit 7 set when free, +0x18 their number, an int, +0x1C the stride, an int, +0x20 the first free slot, -1 when full, +0x28 the slots in use in its low 30 bits, with 0x40000000 set.
  A ped's handle is (index << 8) | the slot's flag byte.
- **The ped,** as the crash log's loop at 0x4F6B reads it: +0xDA & 0xE == 6 a mission ped, +0x1091 bit 4 what every loop skips, +0x1468 bit 4 in the reuse pool, bit 5 reused.
- **What caps the population,** each update, 0xD8CD20: scenario peds = the schedule's byte x `ScenarioPedsMultiplier` / 100 x factors, ambient peds likewise with `AmbientPedsMultiplier`, both scaled down to `MaxTotalPeds` if their sum passes it, 200 at the population slider's top in my file; then `N-E-K2`'s stores: +28 scenario, +6 ambient x the schedule's percent, +20 the rest of ambient.
  The reuse pool evicts one when its count 0x434F520 is 15, so it holds at most 15; mission peds are counted against neither, which leaves 41 slots or more on my file.
- **A full pool** is graceful: the one allocation of a ped, 0xD515F0 in 0xD514C0, returns null when the slots are all in use, before the pool's New 0x15A7C0, which would trap, and its three callers pass the null on.
  The free-room tests of its users are 2, 3, 5, 10, 11 and 15; "128 kept for missions" is the number of Resource Extensions' page, not the game's.
- **What the number sizes,** so a raise reaches it by itself: through the getter PedIntelligence (0xDB0 a slot), CMovePed (0x4C0), fragInstNMGta (0x460), two arrays of the population, new'd by the number at 0xD6A73C and filled by 0xD6D1F0 for every live ped of the pool, so the pool must never be larger than the number; through the pool's +0x18 the spatial array, of capacity (size + 3) & ~3, 24 bytes each, and more.
- **What it does not size,** unchecked, so a raised `Peds` boots well and past 256 peds alive corrupts memory: a memcpy into a fixed array in `.data` (`N-E-R1`, 0x1D90620), the filler's three stack arrays (`N-E-R2`, `N-E-R3`) and the priority function's (`N-E-R4`, 0x12FD0D0); a cap of 1,024 in the spatial array's six search methods and 0x1164D40; and the anim director's family, whose defaults `N-E-D1` to `N-E-D6` hold and whose pools trap when full.
- **Complete sets:** the pointer and the pool's fields, 608 references in 97 functions and 26 fragments; the getter's 7 calls in 0x10460E0, 0x1167CD0 (3), 0x1175470 and 0x1D90620 (2); the spatial array's pointer, 31 references; a ped's allocation, one `mov edx, 0x1980`.

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| N-E-A1 | `0F B6 05 ? ? ? ? A8 01 75 03 31 C0 C3 48 8B 0D ? ? ? ? 48 C1 C1 1E 48 8B 15 ? ? ? ? 48 31 CA 48 C1 C2 20 80 E1 1F 80 C1 02 48 D3 C2 48 F7 D2` | the pool's accessor: +3 rel32 the flag (ends +7, 0x434A920); +17 rel32 the keys (ends +21, 0x434A930); +28 rel32 the value (ends +32, 0x434A928); the decode pinned | found | 0xD516A0 |
| N-E-M1 | `48 8D 05 ? ? ? ? 48 89 44 24 28 C7 44 24 20 80 19 00 00` | the maker's call of the pool's constructor: +3 rel32 the pools' proxy (ends +7, 0x286A310); +16 the stride, 0x1980 | found | 0xD4E7F4 |
| N-E-M2 | `BA 17 21 A1 8D 41 89 D8 E8 ? ? ? ? 89 C3` | the pool's size looked up: +1 the hash; +8 E8 to the lookup, 0x149060 (rel32 at +9, ends +13); its default the number in ebx | found | 0xD4E7C0 |
| N-E-C1 | `BA 17 21 A1 8D 41 B8 01 00 00 00 E8 ? ? ? ? 89 C1 E8 ? ? ? ?` | the file's value: +11 E8 to the lookup (rel32 at +12, ends +16); +18 E8 to the setter 0xD516F0 (rel32 at +19, ends +23), whose `89 0D rel32 C3` gives the number, 0x434AB4C | found | 0x509F0A |
| N-E-I1 | `BA 84 C5 4A 39 41 89 D8 E8 ? ? ? ? 89 C3` | PedIntelligence's size looked up, which follows the number: +8 E8 to the lookup | found | 0xD4E8F0 |
| N-E-S1 | `48 63 72 18 48 83 C6 03 48 83 E6 FC` | the spatial array's capacity from the pool's +0x18; at a fixed +63 `48 89 1D` with its rel32 at +66 (ends +70): the array's pointer, 0x4816998 | found | 0x1B3BB75 |
| N-E-K1 | `8B 05 ? ? ? ? 8B 0D ? ? ? ? 8B 35 ? ? ? ? 8B 3D ? ? ? ? 44 89 74 24 50` | the crash log's counts: +2 ambient in cars (ends +6, 0x434D19C); +8 ambient on foot (ends +12, 0x434D184); +14 scenario in cars (ends +18, 0x434D1A0); +20 scenario on foot (ends +24, 0x434D188) | found | 0x4FBF |
| N-E-K2 | `F3 0F 2C C2 89 05 ? ? ? ? F3 0F 59 C1 F3 0F 2C C0 89 05 ? ? ? ? 66 0F 7E 3D ? ? ? ?` | the caps' stores: +6 ambient, in vehicles by a hint (ends +10, 0x4352900); +20 ambient, on foot by a hint (ends +24, 0x4352904); +28 scenario (ends +32, 0x4352908) | found | 0xD8D0F1 |
| N-E-R1 | `8B 3A E8 ? ? ? ? 39 C7 7D 04 8B 03 EB 05 E8 ? ? ? ? 89 46 10 8B 03 31 FF 85 C0 0F 4F F8 89 7E 10` | a raise's: the copy into a fixed array of 256: +2 and +15 E8 to the number's getter, 0xD4EA00; +23 the 12 bytes that undo the clamp | found | 0x1D90643 |
| N-E-R2 | `66 C7 05 ? ? ? ? 00 00 E8 ? ? ? ? 44 0F B7 05` | a raise's: the timeslice list's reset: its count, disp at +3, ends +9, 0x4353AE8; +9 E8 to the filler 0xD94C20 (rel32 at +10, ends +14) | found | 0xD93C99 |
| N-E-R3 | `48 81 EC 28 0C 00 00 48 8D 8C 24 20 08 00 00 48 8D 94 24 20 04 00 00 4C 8D 44 24 20 E8` | a raise's: the filler's frame and three fixed arrays, the function starting 5 bytes before; +28 E8 to 0xD95420 (rel32 at +29, ends +33) | found | 0xD94C25 |
| N-E-R4 | `B8 48 10 00 00 E8 ? ? ? ? 48 29 C4 48 8B 05 ? ? ? ? 48 63 70 5C` | a raise's: the priority function's frame of a fixed array: +1 the size, 0x1048; +16 rel32 the spatial array's pointer (ends +20); at a fixed +0x308 `48 81 C4 48 10 00 00` | found | 0x12FD0DC |
| N-E-D1 | `BA 10 D8 5E CA 41 B8 00 04 00 00 E8` | a raise's: fwAnimDirector's default, the imm32 at +7 | found | 0x15CCC2D |
| N-E-D2 | `BA CA 6C 77 36 41 B8 00 04 00 00 E8` | a raise's: the Creature's | found | 0x15D915D |
| N-E-D3 | `BA 7A EE AE CF 41 B8 55 09 00 00 E8` | a raise's: the MotionTree's | found | 0x15D88BD |
| N-E-D4 | `BA 46 99 C5 3C 41 B8 AA 02 00 00 E8` | a raise's: the Expressions' | found | 0x15DA2BD |
| N-E-D5 | `BA 29 BB 69 BF 41 B8 55 01 00 00 E8` | a raise's: the RagDoll's | found | 0x15DA78D |
| N-E-D6 | `BA BE 10 0E 21 41 B8 55 01 00 00 E8` | a raise's: the FacialRig's | found | 0x15DF90D |

## A pool's memory

A pool of the game's does not draw on the heap `H-E1` sizes but on a budget of its own, kept by a big-block allocator that `H-E2` sets up, 0x5A bytes after `H-E1` in the same start-up function (SPECIFICATIONS.md, section 11.1).
The samples are in `pedpool_findings_for_the_clean_room_build.txt`, section 3.

- **The route:** a pool's allocator 0x15A4D0 asks the allocator at the pool's +0x00, through its vtable's slot 1, for its room, and writes the slots' flags at the result plus the slots' size with no test of the result: `or byte [rcx+rax], 0x80` at 0x15A514.
  In the pools' makers +0x00 is the proxy 0x286A310, whose vtable lies in `.data` (a hint: the table 0x247CF90).
  0x36D90 takes the master allocator from TLS+0x5C8, its slot 5, and jumps to that allocator's vtable +0x40 with the size and alignment 16; 0x36E00 frees the same way.
  The start-up 0x1020 fills the master allocator 0x29B1580: slots 0 and 4 the heap 0x29A8590, slots 1, 2, 3 and 6 another 0x29B15D8, slot 5 the big-block allocator 0x29B1508.
- **The allocator,** from its constructor 0x141560, called once, at 0x1457, its allocation 0x141640 and its free 0x141800: +0x08 the heap, +0x10 the list of blocks, +0x18 the threshold 0x10000, +0x20 the budget, +0x28 in use, +0x30 room, +0x38 the least room so far, +0x40 the number of blocks, a word, +0x42 bit 0 the fatal flag, set by its constructor, +0x48 a critical section.
  Its getters: vtable slot 19 the room, slot 22 the least room, slot 17 the use.
- **An allocation of n bytes,** under its lock: rounded = (n + 0x1003F) & ~0xFFFF; rounded above the room returns null, which is the limit; the fatal flag and 900 blocks, int3; rounded above the threshold, `VirtualAlloc` of rounded + 0x1000 with a guard page at its end, else n + 0x40 from the heap; room and in use change by what was charged, the least room is kept, the block is linked.
  Free gives the charge back.
- **The budget:** 0x8800000, 136 MB, `H-E2`'s immediate: a ceiling on a counter, so raising it allocates nothing, and the blocks still come from Windows when asked for.
  `HEAP_SIZE` changes the heap, not this.
  At once 0x10F0 takes 8 MB of it, kept at 0x29C76C0 (a hint: the block stays; nothing read gives it back).
- **A pool's cost:** of n slots s = (stride x n + 7 & ~7) + n, then (s + 0x1003F) & ~0xFFFF above 64 KB and s + 0x40 below it; a store's pool costs stride + 1 bytes a slot.
  My sum over the 248 references that load the proxy, with my `gameconfig.xml`, comes to 132 to 136 MB of the 136: `InstanceBuffer` 50.7, `Building` 12.0, `GrassBatch` 8.8, the asset stores' pools 18.9, `HandlingData` 6.8, the 8 MB block, the ped pool's 1.6 among them.
  Script Hook V extends two pools (a hint for who: its log says so, and the same crash's log has their numbers), about 0.4 MB more; whatever raises a pool pays here.
- **The crash:** past the budget the allocator returns null, and the pool's allocator writes through it at 0x15A514, with `rax` 0 and the address the slots' size.
  Nine of my dumps fail there: six at 0x1030DA0's pool of 512 slots (a write to 0x26000), two at `HandlingData`'s 8700 x 816 (0x6C5340), the ninth at a pool of 23 slots of 16 bytes (0x170), with room under 64 KB and a third site near 0x1AEA7FC, with `TrainerV.asi` and Collectibles on Map in; without the two plugins the same load passed.
  In one the dead stack holds the allocator and its lock, and the jump of 0x36D90 is in `r10`.
  That the budget ran out and not `VirtualAlloc` is by elimination: that log has 46 GB of memory free, and `VirtualAlloc`'s failure makes a call the stack does not show.
- **Not settled:** why the same load passes one time and fails another: at that crash no other thread stands in an allocator of it or in a pool's maker.

## Render queues

The design is in SPECIFICATIONS.md, section 11.16.6, a raise's in `renderqueues_findings_for_the_clean_room_build.txt`, section 8.
A pool's name is a hint from memory of the game's `gameconfig.xml`, each backed by its hash at the lookup.

- **The size lookup** 0x149060: rcx the config manager [0x2E96FB0], edx the joaat of the name, r8d the default; it gives the file's value if the file has the entry, else the default, as for an entry's value -1.
  A flag [0x2E96FA8] would pick the larger of the default and the value; no code writes it, so the value alone stands: `gameconfig.xml` moves a pool both ways (the flag's value a hint: the dump has no `.data`).
- **A pool** (its layout in "Ped pool"), made by the constructor 0x15A490 and the allocator 0x15A4D0, these six with an empty name.
  A slot is taken at 0x15A7C0 (pool, size): full, or a size above the stride, falls through to the game's fatal trap, INT3 at 0x15A84C, with the stride in edx, which names the pool in a dump.
  A slot is freed at 0x15A930.
- **A pool's pointer:** three fields, a flag byte at F, a value at F+8, a key at F+0x10, and two constants of its code, r and s: `t = rol64(key, r); x = value ^ t; c = (t & 0x1F) + s; pointer = ~rol64(x, 32 + c)`, null unless the flag's bit 0 is set.
  0x1363CB0 and 0x1363E60 encrypt it, the same formula backwards.
- **The six pools:**

| Family | Name (joaat), its lookup | Default | Stride | F | r, s | Where a slot is taken | Full |
|---|---|---|---|---|---|---|---|
| Streamed peds, requests | `StreamPed req data` 0x0AB968F6, 0x10E5D58 | 55 | 0x508 | 0x4476148 | 0x1B, 1 | 0x10E888B in 0x10E87C0 | INT3 0x15A84C |
| Streamed peds, render | `StreamPed render data` 0x2D370C34, 0x10E5E98 | 120 | 0x2E0 | 0x4476160 | 0x1C, 3 | 0x10EAC00 in 0x10EAB60 | INT3 0x10EAC20, tested at 0x10EABBA |
| Ped props, requests | `PedProp req data` 0x85FB462D, 0x10EFAC8 | 50 | 0x80 | 0x44766A8 | 0x1B, 2 | 0x10F3E52, 0x10F4AE1, 0x10F56CA | INT3 0x15A84C |
| Ped props, render | `PedProp render data` 0x539C8EB8, 0x10EFBF8 | 120 | 0x3C | 0x44766C0 | 0x1D, 5 | 0x10F8A56 in 0x10F8990 | INT3 0x15A84C |
| Vehicles, requests | `CVehicleStreamRequestGfx` 0x54753C86, 0x1363CD8; its default `VehicleStreamRequest` 0x90D4BFF4, 0x1364B16 | 1 | 0x1C8 | 0x471F4B8 | 0x1D, 5 | 0x13650BA in 0x1365050 | INT3 0x15A84C |
| Vehicles, render | `CVehicleStreamRenderGfx` 0xC2B5F089, 0x1363E88; its default `VehicleStreamRender` 0xE11D5267, 0x1364B33 | 1 | 0xBB0 | 0x471F4D0 | 0x1E, 3 | 0x1364676 in 0x13645D0 | the vehicle's mods wiped, no trap |

  The vehicles' values land at [0x471F5D0] and [0x471F5D4].
- **The arrays,** three families of four, one per frame, each an atArray of 16 bytes - a pointer, the count, a word at +8, the capacity, a word at +0xA:

| Family | Arrays | Capacity | Index | Lock | Add | Check | Dedupe | Its reference at | Flip |
|---|---|---|---|---|---|---|---|---|---|
| Streamed peds | 0x4476630, 640, 650, 660 | a constant 0x78, 0x3C0 bytes (0x10E67F0) | 0x4476670 | 0x4476678 | 0x10EF3C0 | none | none | +0x2D8 | 0x10EF930 |
| Ped props | 0x44766F0, 700, 710, 720 | a constant 0x78, 0x3C0 bytes (0x10EFEE0) | 0x4476730 | 0x4476738 | 0x10F9D90 | none | none | +0x38 | 0x10F9FE0 |
| Vehicles | 0x471F560, 570, 580, 590 | [0x471F5D4] entries (0x1364B70) | 0x471F5A0 | 0x471F5A8 | 0x1368EE0 | none | a walk of the current array | +0xBA8 | 0x1369130 |

- **A flip:** the index goes to (index + 1) mod 4, each entry of the array it enters loses its reference, the count goes to 0, then a pool walk frees render gfx with no references and its "in use" byte clear (+0x2DA for peds).
  So a render gfx outlives its ped by up to four flips, and three of the four arrays always hold a whole past frame's entries.
  All three flips run from 0x3CD400, slot 0x48 of the draw list manager's vtable 0x25EAC60, called in 0x13FCE60, which counts a frame number at [0x475B740]; that this is once per frame is a hint from its place.
  The inits run after `DllMain`: the vehicles' from 0xE7778F in 0xE776D0, called at 0xC4EF1B, the peds' and the props' from 0x1B3BB2F and 0x1B3BB34 in 0x1B3BB00, called at 0xC4EF71.
- **Past the peds' and the props' arrays** nothing is dropped: the add writes the entry at the count with no compare, so entry 121 lies past the block of 0x3C0 bytes, in whatever the heap put next; the count is 16 bits, so at 65,536 it wraps to 0.
  Each array has one writer of entries, its add, and one reader, its flip.
- **Past the vehicles' arrays:** a render gfx enters a frame once, so entries stay within the pool's capacity, which is the arrays' as long as the two names agree; a `gameconfig.xml` naming `CVehicleStreamRenderGfx` above `VehicleStreamRender` makes the pool larger than its arrays, and the same overflow.
- **`CVehicleStreamRenderGfx` full:** the else branch of 0x13645D0 from 0x1364699 lets the vehicle's current render gfx go and sets its mod state to 0xFF (+0x4F2 to +0x522, +0x533): it draws without its mods, silently.
  Only two tests of these pools' free room exist: 0x7A6730 returns 0 when the vehicle request pool is full, and the ped clone 0xD4F950 returns 0 for a streamed ped when the free render slots do not exceed the requests in flight.
- **The draw list's references,** the neighbour on the same path: the manager [0x475B750], 0x14BA0 bytes, made at 0x509330, keeps four arrays at +0x5D0, +0x5E0, +0x5F0, +0x600 of 16-byte entries (module, index), 0x2880 = 10,368 each, with the index at +0xAC and the lock at +0x14AB0.
  Each add above takes a streaming reference for each asset of the gfx and queues its release there: up to 134 for a ped's render gfx (12 x 7 + 50 slots, those not -1), 52 for a vehicle's; peds pay this per draw, vehicles per frame, and every model drawn pays one too (0x1402640, deduped per frame), so the array is shared with the whole scene.
  The push is checked, but the reference is taken before the check: past 10,368 the release is never queued, and that asset can never unload.
- **Who is streamed,** hints: a ped by its model info's byte +0x286 bit 1, set at 0x1343BA1 from the ped definition's bool at +0xC0 (0x13438F0); a vehicle's render gfx when mods are applied through 0x134B600.
- **The callers of the adds:** peds 0x3A0FC6 in 0x3A0CB0, 0x3C2C49 and 0x3C2CC0 in 0x3C2830, which has 13 callers, the draw commands of 0x3EC490; props 0x3A11CD, 0x3D0511, 0x3D28D5; vehicles 0x399AA8, 0x3A2DE9, 0x3C3F37, 0x3C5766.
  0x3EC490 allocates one draw command per call and nothing asks whether this ped has been added this frame, so a ped's entries per frame come from the render phases that see it.

Each pool's make holds, at the offsets its row gives, the rel32 of its flag, its key and its value, rip-relative with the next instruction 4 bytes on, and r and s, a byte each, wildcards since a build changes them:

| Label | Pattern | Use | 1.0.1158.16 | RVA |
|---|---|---|---|---|
| U-E-S1 | `0F B6 05 ? ? ? ? 48 8B 05 ? ? ? ? 48 C1 C0 ? 48 8B 15 ? ? ? ? 48 31 C2 48 C1 C2 20 24 1F 8D 48 ? 48 89 D5 48 D3 E5 34 3F 89 C1 48 D3 EA 48 09 EA 48 F7 D2 0F B6 05 ? ? ? ? A8 01 B9 00 00 00 00 48 0F 45 CA BA 08 05 00 00 E8` | a slot of `StreamPed req data` taken, 0x508 its stride, the call last: F at +3, the key at +10, r at +17, the value at +21, s at +36, which its code rotates by at 1 alone, `34 3F` pairing c ^ 63 with c + s | found | 0x10E883C |
| U-E-S2 | `8B 42 28 C1 E0 02 C1 F8 02 39 42 18 7E 61 0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 15 ? ? ? ? 31 C9 F6 C2 01 48 0F 45 C8 BA E0 02 00 00 E8` | a slot of `StreamPed render data` taken, 0x2E0 its stride, the call last, after the test of its room: +0 and +9 a pool's count at +0x28 and capacity at +0x18, +12 the jump to the trap, a CC past the match; F at +17, the key at +24, r at +31, the value at +35, s at +51 | found | 0x10EABB1 |
| U-E-S3 | `66 41 FF 86 D8 02 00 00 48 63 05 ? ? ? ? 48 C1 E0 04 48 8D 0D ? ? ? ? 48 8B 14 08 0F B7 6C 08 08 8D 5D 01 66 89 5C 08 08 4C 89 34 EA` | the streamed peds' add, from its reference on, which pins that nothing lies between reading the count and writing the entry: the index at +11, the arrays at +22 | found | 0x10EF8B3 |
| U-E-S4 | `B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 80 3D` | the streamed peds' four arrays made: blocks of 31 bytes at +24, +55, +86, +117, each with the size at +1, the array's rel32 at +18, the capacity's rel32 at +25, counted from +31, past its imm16 at +29 | found | 0x10E67F4 |
| U-E-P1 | `0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 80 E1 1F 80 C1 ? 48 D3 C0 48 C1 C0 20 48 F7 D0 0F B6 0D ? ? ? ? 45 31 FF F6 C1 01 49 0F 44 C7 BA 80 00 00 00 48 89 C1 E8 ? ? ? ? 49 89 C5 C7 00 FF FF FF FF 66 C7 40 04 00 7F` | a slot of `PedProp req data` taken, 0x80 its stride, one of its three makes: F at +3, the key at +10, r at +17, the value at +21, s at +33 | found | 0x10F3E0D |
| U-E-P2 | `0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 0D ? ? ? ? F6 C1 01 48 0F 44 C7 BA 3C 00 00 00 48 89 C1 E8` | a slot of `PedProp render data` taken, 0x3C its stride: F at +3, the key at +10, r at +17, the value at +21, s at +37 | found | 0x10F8A14 |
| U-E-P3 | `66 41 FF 47 38 48 63 05 ? ? ? ? 48 C1 E0 04 48 8D 0D ? ? ? ? 48 8B 14 08 0F B7 5C 08 08 8D 7B 01 66 89 7C 08 08 4C 89 3C DA` | the ped props' add: the index at +8, the arrays at +19 | found | 0x10F9F28 |
| U-E-P4 | `B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 05 00 00 00 31 D2 E8 ? ? ? ? B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 B9 C0 03 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 89 05 ? ? ? ? 66 C7 05 ? ? ? ? 78 00 48 83 C4 28 C3` | the ped props' four arrays made, as `U-E-S4`'s, to the function's end | found | 0x10EFEE4 |
| U-E-V1 | `0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 48 C1 C0 20 80 E1 1F 80 C1 ? 48 D3 C0 48 F7 D0 0F B6 15 ? ? ? ? 31 C9 F6 C2 01 48 0F 45 C8 BA C8 01 00 00 E8 ? ? ? ? 48 89 C6 48 89 C7 48 8D 05` | a slot of `CVehicleStreamRequestGfx` taken, 0x1C8 its stride: F at +3, the key at +10, r at +17, the value at +21, s at +37 | found | 0x1365079 |
| U-E-V2 | `0F B6 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 C1 C1 ? 48 8B 05 ? ? ? ? 48 31 C8 80 E1 1F 80 C1 ? 48 D3 C0 48 C1 C0 20 48 F7 D0 0F B6 0D ? ? ? ? 31 D2 F6 C1 01 48 0F 45 D0 8B 42 28 C1 E0 02 C1 F8 02 48 8B 5F 48 39 42 18 7E 6B` | the test of `CVehicleStreamRenderGfx`'s room before a slot is taken, its stride 0xBB0 known from the make past the match: F at +3, the key at +10, r at +17, the value at +21, s at +33, +76 the jump a full pool takes, to the branch that wipes the vehicle's mods, inside its function | found | 0x13645E0 |
| U-E-V3 | `66 41 FF 84 24 A8 0B 00 00 48 63 05 ? ? ? ? 48 C1 E0 04 4A 8B 0C 30 42 0F B7 54 30 08 8D 5A 01 66 42 89 5C 30 08 4C 89 24 D1` | the streamed vehicles' add: the index at +12, the arrays in r14 from `U-E-V4`'s `lea` | found | 0x13690E2 |
| U-E-V4 | `49 89 CC 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 63 0D ? ? ? ? 48 C1 E1 04 4C 8D 35 ? ? ? ? 42 0F B7 44 31 08 48 85 C0` | the head of that add's walk of the current array, its dedupe: the lock at +6, the index at +18, the arrays at +29 | found | 0x1368EEF |
| U-E-V5 | `48 8B 0D ? ? ? ? BA F4 BF D4 90 41 B8 01 00 00 00 E8 ? ? ? ? 89 05 ? ? ? ? 48 8B 0D ? ? ? ? BA 67 52 1D E1 41 B8 01 00 00 00 E8 ? ? ? ? 89 05 ? ? ? ?` | the vehicles' two names looked up: the calls at +18 and +47, the stores' rel32 at +25, `VehicleStreamRequest`, and +54, `VehicleStreamRender` | found | 0x1364B04 |
| U-E-V6 | `48 63 35 ? ? ? ? 48 85 F6 0F 84 ? ? ? ? 48 8D 0C F5 00 00 00 00 BA 08 00 00 00 E8 ? ? ? ? 48 63 3D ? ? ? ? 48 89 05 ? ? ? ? 66 89 35` | the vehicles' arrays made from that size: the size's rel32 at +3, the first array's store's at +44 | found | 0x1364B77 |
| U-E-N1 | `48 8B 0D ? ? ? ? BA F6 68 B9 0A 41 B8 37 00 00 00 E8` | the lookup of `StreamPed req data`, which a raise redirects, the call at +18, its rel32 past the match | found | 0x10E5D46 |
| U-E-N2 | `48 8B 0D ? ? ? ? BA 34 0C 37 2D 41 B8 78 00 00 00 E8` | the lookup of `StreamPed render data`, the same | found | 0x10E5E86 |
| U-E-N3 | `48 8B 0D ? ? ? ? BA 2D 46 FB 85 41 B8 32 00 00 00 E8` | the lookup of `PedProp req data`, the same | found | 0x10EFAB6 |
| U-E-N4 | `48 8B 0D ? ? ? ? BA B8 8E 9C 53 41 B8 78 00 00 00 E8` | the lookup of `PedProp render data`, the same | found | 0x10EFBE6 |
| U-E-N5 | `48 8B 0D ? ? ? ? BA 86 3C 75 54 41 89 D8 E8` | the lookup of `CVehicleStreamRequestGfx`, the call at +15, its rel32 past the match | found | 0x1363CC9 |
| U-E-N6 | `48 8B 0D ? ? ? ? BA 89 F0 B5 C2 41 89 D8 E8` | the lookup of `CVehicleStreamRenderGfx`, the same | found | 0x1363E79 |
| U-E-F1 | `56 57 48 83 EC 28 89 D6 48 89 CF E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 89 F9 89 F2 48 83 C4 28 5F 5E E9` | the frame's three calls of the flips, at +11, +16, +21; each flip does its work 6 or 7 bytes on from its start, `48 8D 0D lock E8 rel 8B 05 index 8D 48 01 8D 50 04 85 C9 0F 49 D1 83 E2 FC F7 DA 44 8D 04 10 41 FF C0`, the mod 4 pinned there (0x10EF936, 0x10F9FE7, 0x1369136) | found | 0x3CD400 |
| U-E-D1 | `48 89 86 D0 05 00 00 66 C7 86 DA 05 00 00 80 28 B9 00 88 02 00 BA 08 00 00 00 E8` | the draw list's first array of references at the manager's +0x5D0, its capacity at +0x5DA, the imm16 at +14 | found | 0x13FF56F |
| U-E-D2 | `8B 81 A4 00 00 00 8B 91 AC 00 00 00 FF C0 83 E0 03 89 81 A4 00 00 00 FF C2 83 E2 03 89 91 AC 00 00 00 C3` | the draw list's arrays rotated, the whole function: the references' index at the manager's +0xAC | found | 0x1402F80 |
| U-E-D3 | `B9 A0 4B 01 00 E8 ? ? ? ? 48 89 C6 48 89 C1 E8 ? ? ? ? 48 89 35 ? ? ? ?` | the draw list manager made, 0x14BA0 bytes at +1, its global's rel32 at +24 | found | 0x5093EF |
