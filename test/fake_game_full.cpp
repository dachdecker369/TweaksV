// SPDX-License-Identifier: GPL-3.0-only
// Test harness for TweaksV: pretends to be GTA5_Enhanced.exe and contains copies of the game
// code TweaksV patches: HeapAdjuster's, StraightToStoryMode's, ModkitLimit's, PackfileLimit's,
// BoxStreamer's and Decals' here, with the script programs', the Blips part's, those of the parts of the parts'
// coding round and the sites of the limits measured first, RageOpenV's in fake_game_rov.cpp.
// Prints every patch site after TweaksV.asi has loaded.
// Link it with a version resource (fake_game_version.rc) for the game build to pretend to be.
// Usage: GTA5_Enhanced.exe [TweaksV.asi] [options...]
//   prepatched     StraightToStoryMode's patches are already applied (an old .asi ran first)
//   badsplash      unexpected bytes at the splash site
//   badlegal       unexpected bytes at the legal site
//   jnzlegal       a jnz instead of the game's jz at the legal site
//   hookedlanding  the landing page function starts with a jmp (another mod's hook)
//   twos3          a second copy of the landing page's site (S-E3 matches twice)
//   heapraised     the heap site already holds 2000 MB (another heap adjuster ran first)
//   heaphuge       the heap site already holds 3072 MB, more than HEAP_SIZE allows (another heap adjuster ran first)
//   heapodd        the heap site holds a value that is not a whole number of MB
//   modkitmoved / modkitsplit / modkitmismatch   see main()
//   modkitextra    a seventh lea of the modkit array, which no pattern names (a changed game)
//   packmoved      one packfile array reference points outside the game (another mod moved it)
//   packhooked     the streaming configuration call already leads outside the game
//   packbadsize    the direct size limit holds an implausible 70000
//   packindirect   the indirect size variable holds the array's size itself, 4500, not 4 less (a changed margin)
//   packbigconfig  the streaming configuration the report hands the hooked call holds 20000 archives, not 3000
//   twor31a        a second, identical copy of the 1158.13+ packfile reference (its pattern matches twice)
//   junkr31a       the game's obfuscated code that the old 1158.13+ reference pattern also matched
//   packextra      a 34th reference to the packfile array, a mov no pattern names (a changed game)
//   boxmoved       every reference to the box streamer's traversal list points outside the game (another mod moved it)
//   boxsplit       one reference to the list points somewhere else in the game
//   boxninth       a ninth lea loads the list (a changed game)
//   boxmismatch    one capacity of the list holds 1001 instead of 1000
//   boxbadsize     every capacity of the list holds an implausible 200000
//   boxcallhooked  the second tree-search call of the box streamer already leads outside the game (another mod's hook)
//   boxcallsplit   the third tree-search call leads to the other tree search than the first
//   decalmoved     every reference to the decal definitions array points outside the game (another mod moved it)
//   decalsplit     one reference to the array points somewhere else in the game
//   decalextra     a 32nd lea loads the array (a changed game)
//   decalprefix    one lea of the array has a 49 prefix instead of 48
//   decalmismatch  FindById's second compare of the limit holds 261 instead of 260
//   decaloffbyone  the parser's check of the limit holds 260 instead of 259
//   decalbadsize   the parser's check of the limit holds an implausible 131071
//   decalfull      260 decal definitions are registered (the game's own array is full)
//   decalmany      300 decal definitions are registered
//   dstorage       loads the stand-in dstorage.dll (fake_dstorage.cpp) before TweaksV, calls it after: the
//                  configuration, the factory and its OpenFile for 19 paths, the files it holds closed again
//   dsotherinterface  with dstorage: first asks for a DirectStorage interface TweaksV does not know, then for the known one
//   dsotherlater   with dstorage: asks for an interface TweaksV does not know after the known one and its files
//   dsotheronly    with dstorage: asks only for an interface TweaksV does not know - no factory TweaksV knows, no file;
//                  stays 1.5 s longer after "wait", for the line TweaksV writes 10 s after that request
//   dsnofactory    with dstorage: never asks for a factory
//   dsflood        with dstorage: 20,001 more OpenFile calls, after "wait" or "waitlong"
//   dsshare        with dstorage: other programs' opens of files the stand-in holds open (ShareThroughStandIn)
//   ownshare       other programs' opens next to the fake's own opens in its folder (ShareOwnOpens)
//   dstypes        read-only opens of 17 files of 17 types in its folder that are not there
//   dsopenflood    opens 100,001 files in its folder that are not there, each by another name, after TweaksV has started
//   dsreplace      with dstorage: a file the stand-in holds open written the four ways from inside the game - the
//                  classic replace, a POSIX rename, ReplaceFile, a write in place - and by the classic replace from
//                  another program, renamer.exe (ReplaceHeldFile)
//   dsrenames      with dstorage: the renames TweaksV's NtSetInformationFile hook passes on and turns, most of them
//                  against a file the stand-in holds open (RenameAsked)
//   dsrenameflood  20,001 classic replace-renames in its folder after TweaksV has started
//   fileopens      opens files in its folder (and one outside, in %TEMP%) after TweaksV has started
//   nodyncode      no code may be changed or made once TweaksV loads (Windows' dynamic code policy)
//   lockedlog      while TweaksV starts, holds the TweaksV.log in its folder open as another program
//                  might: for reading, shared for reading only
//   unload         frees TweaksV.asi once it has started (an ASI loader that unloads it) until it is gone or
//                  8 times, says whether it is still loaded, and reports as usual - which calls the hooked
//                  functions
//   wait           stays 11 seconds longer before it reports
//   waitlong       stays 31 seconds longer before it reports (TweaksV's DirectStorage factory check comes 30 s in)
//   script         places the script program blip_controller once, once TweaksV has started, as the game does: on
//                  a thread of its own, through the call at the hook site - TweaksV's stub when the call is
//                  redirected, the fake's own resolution function when it is not - and writes what it placed to
//                  script_placed_<n>.bin, n counting blip_controller's placements, for the suite to compare with
//   scripttwice    places it twice, the second time with a native the fake's resolution function does not know
//   scriptother    places another program, other_script, before it
//   scriptgap      with scriptother: 3 s between the two placements
//   scriptotheralone  places other_script alone
//   scripthash     places a program with blip_controller's hash and another name, blip_imposter, twice instead
//   scriptpage     the second page of its code cannot be read
//   scriptmany     places it 17 times
//   scripthuge     places it twice: its code length says 16 MB and a byte the first time, its native count 65537 the
//                  second
//   scripthooked   the call at the hook site already leads outside the game (another mod redirected it); nothing
//                  is placed
//   blipfused      blip_controller's guard (the Blips part) with the fused ILT_JZ in the place of its comparison and
//                  JZ, two NOPs of room before the push, its body behind the branch and its way out a jump: the
//                  polarity the other way round
//   blippadded     the guard with two NOPs after the count
//   bliproom       the guard with IGE and JZ, no NOT: three bytes before the branch
//   blip240        the guard with the threshold 240
//   blipnative     the guard calling WAIT through native 3, not below the program's native count
//   bliptwice      a second guard at 0x800
//   blipjump       a jump at 0x100 into the guard's span
//   blipcall       a CALL at 0x100 into the guard's span
//   blipswitch     a SWITCH at 0x100 whose one case leads into the guard's span
//   blipcatch      a CATCH, whose target is computed while the script runs, at 0x200
//   blipchanged    the guard's threshold changed to 151 while the game resolves the natives, as another mod could
//   blipbadbyte    the byte 0xA0, no instruction, at 0x200
// The blip pool's two sites are always there, the pool with 162 slots in use below its high-water mark, 170, and
// two beyond it.
// The parts of the parts' coding round (TweaksV's SPECIFICATIONS.md 11.10 to 11.13), RageOpenV's check for packs made for
// GTA V Legacy (11.6) and the limits measured first (11.16), one block each: its options, and what is always there.
// The weapon infos (WeaponLimits): the game's initializer of the weapon info blob array - its lea of the array, then
// W-E-I1: "mov edx, 160", the call to the allocation, the block's store, "mov word [capacity], 160" -, the allocation's
// first bytes, and the loader, a NOP before its start and W-E-I2 0x18 bytes into it, are always there - the loader real
// code, which the fake calls through its start with a data file entry, as the game calls its loader: it counts the file,
// as the game's adds a blob, and returns 1. The array, in the fake's data, is empty while TweaksV starts. Once TweaksV
// has started, the fake sets the array up as the game's initializer does - a block on its heap of as many blobs of 248
// bytes as its site's "mov edx" says, the room its capacity's store says - and its loader's count at 152; then it loads
// the files the options ask for, and when TweaksV's guard refused any, it waits until TweaksV.log holds the guard's
// report and the line after it - 20 and 5 seconds at most.
//   wisetup        the array is set up before TweaksV starts (an initializer that ran before it)
//   wihigh         both immediates hold 2048 before TweaksV starts (another mod's raise, or a later game's own)
//   wiraised       the array gets room for 512, as if another mod had raised both immediates after TweaksV checked
//                  them, and 170 files are loaded
//   wipacks        packs mounted while the game runs: once TweaksV.log holds the first weapon infos summary, the count
//                  becomes 160 (the array full), once it holds the second, 165 (past its end); then 2.5 s of nothing
//                  new (for the test build's TWEAKSV_TEST_SUMMARY_MS=1000)
//   wifill         872 files through the loader: 152 + 872, the part's default room exactly
//   wifew          14 files through the loader after them, as packs bring them: the 9th with a name of 140
//                  characters, the 10th with a byte that is no printable ASCII
//   wiunreadable   with wifew: the 9th file's name runs into memory that cannot be read after its 64th byte
//   wiflood        20,001 files through the loader after the others
//   wilater        once the guard's report is in TweaksV.log, 3 files more, then until the guard's next line is there
//                  and 2.5 s more (for the test build's TWEAKSV_TEST_SUMMARY_MS=1000)
//   wijump         the loader's start a jump already, another mod's hook: to a stub that runs the three pushes it took the
//                  place of and jumps back into the loader
//   wiedx          once TweaksV has started, before the fake's initializer runs, the number of blobs alone rewritten to 512, as
//                  another mod's port could after TweaksV's raise: fewer blobs than the capacity says
//   wiunwind       the loader copied into a function of the fake's own that its unwind data names, at the function's start,
//                  and W-E-I2 in .fakesit broken, so that it matches in the copy alone
//   wiunwindin     the same, the loader copied 8 bytes into that function: its start lies inside it, a CC before it
//   wimismatch     the capacity holds 161, the number allocated 160
//   wizero         both hold 0
//   wihooked       the call to the allocation leads outside the game (another mod's hook)
//   wibytes        the allocation begins with a jump (E9) instead of its own bytes
//   wioutside      the block's store leads outside the game
//   wicapacity     the capacity's store leads to the array + 0xC instead of + 0xA
//   wicount        the loader's read of the count leads to the array + 0x10 instead of + 8
//   wisize         the loader's blob size is 0x100, the allocation's element size 0xF8
//   winotstart     the byte before the loader's start is 48, so that it is no function start
//   wiextra        a third reference to the array, a second store of a block
//   wiinittwice    W-E-I1's bytes a second time in .fakesit: the game holds the pattern twice
//   wiloadertwice  W-E-I2's bytes a second time in .fakesit
// The weapon components' sites are always there (the WeaponLimits part): the rebuild of the list of every weapon
// component definition, its sort and its update of the count, the pool's size call to a stand-in for the game's size
// lookup, and the list's 15 other leas, 17 in all; the list of 470 entries with its count right after it, 412
// definitions from 23 weaponcomponents.meta files, and the files' count beyond it. The report calls what the size call
// leads to twice, as the game makes its pool - TweaksV's stub once it is redirected - unless another mod's hook is there.
//   wcpack         11 seconds after the other actions begin, a pack's weaponcomponents.meta loads: the list is rebuilt
//                  with 430 definitions from 24 files (with "wait", the run then stays 11 seconds more)
//   wcfull         470 definitions from 25 files: the game's own list is full
//   wcmore         556 definitions from 40 files, past the game's own list, as seven weapon packs bring them
//   wcover         the list has overflowed: the 471st definition's pointer lies over its count, which reads its low half
//   wcconfig       the game's configuration names 600 slots for the pool (a gameconfig.xml with the pool's entry)
//   wcconfigbig    the game's configuration names 20000 slots for the pool, more than the part gives it
//   wcputback      once TweaksV.log holds the first weapon components summary, one of the list's loads in the role of the
//                  game's protected code leads to the game's own list again, as if the game had put it back; the run
//                  then waits up to 10 s for the problem (for the test build's TWEAKSV_TEST_SUMMARY_MS)
//   wcmoved        every lea of the list points outside the game (another mod moved it)
//   wcsplit        one lea of the list points elsewhere in the game: only 16 load it
//   wcextra        an 18th lea loads the list (a changed game)
//   wcprefix       one lea of the list has a 49 prefix instead of 4C
//   wccount        the update's store of the count leads 8 bytes past the count: the count's references disagree
//   wcgap          the count's four references lead 4 bytes past it: not a whole number of entries after the list
//   wcbefore       the count's four references lead 8 bytes before the list
//   wcoutside      the count's four references lead outside the game
//   wcfar          the count's four references lead 20000 entries of 8 bytes past the list, inside the fake's image
//   wcdefault      the pool's default holds 0
//   wchooked       the pool size call already leads outside the game (another mod's hook)
//   wcmount        once TweaksV has started, 11 files of packs mounted through the loader's head, 91 definitions in all
//                  (9, 8, 9, 8, 9, 8, 9, 8, 9, 8 and 6); wcbase then clears the blobs and loads the 11 again, as the game's
//                  base loader would; wcmany mounts 60 definitions in one file, then 1,100 files of one definition each
//   wcbadblob      the first blob's count holds 500, past the 470 a blob holds, when the files are mounted
//   wcloaderhooked the loader's head starts with a jump (another mod's hook); the files are not mounted then
//   wcloadernotstart the byte before the loader's head is 48: no function start there; the files are not mounted then
//   wcloaderret   the loader's head returns at once (C3), which MinHook cannot hook; the files are not mounted then
//   wccountnoaccess the store's page - the list and its count - cannot be read while TweaksV starts, readable again after
//   wcloadnoaccess once TweaksV.log holds the first weapon components summary, the page of the list's 15 leas cannot be read
//                  until TweaksV's problem is in the log, up to 10 s (for the test build's TWEAKSV_TEST_SUMMARY_MS)
//   wcearly        the game made the pool before TweaksV started: with wcmount or wcmany it is not made again before the files
//                  are mounted, which wait up to 3 s for the box of TweaksV's problem, or the line that it is not shown
//   wcgrown        the count's four references lead 600 entries of 8 bytes past the list: the game's own list has grown to 600
//   wclarge        they lead 16500 entries past it, inside the fake's image: a size the core takes, past the key's maximum
//   wcfill         like wcmount, 36 files of packs of 470 definitions each, past a room at the key's maximum
//   wcheap         with wcmount: each file's callable on the heap, not inline in its callback, so that destroying it asks for dl true
//   wcvtableout    with wcmount: each callback's vtable on the heap, outside the image
//   wcdestroyout   with wcmount: each callback's vtable in the image, its destroy outside it, a stub that only returns
//   wcbadtype      with wcmount: each file's entry of the data file type 0x4E, not 0x4F
//   wcbadcount     with wcmount: the blob array's capacity 10, below the 23 blobs it counts, when the files are mounted
//   wcnopointers   with wcmount: the blob array's pointer leads to a page that cannot be read, when the files are mounted
//   wcnoblob       with wcmount: the 6th of the blob array's pointers leads to a page that cannot be read, when the files are mounted
//   wcnoarray      with wcmount: the store's page cannot be read until the first file has reached the loader's first call
//   wcpooltwice    W-E-C4's bytes a second time in .fakesit: the game holds the pattern twice
//   wcloadertwice  W-E-C5's bytes a second time in .fakesit
//   wcempty        no blobs and no definitions while TweaksV starts; with wcmount the first file is mounted before the pool is
//                  made, the rest after it
//   wcloadercount  W-E-C5's read of the blobs' count leads elsewhere, not to the count W-E-C1 reads
//   wcloadercapacity W-E-C5's read of the blob array's capacity leads elsewhere, not 2 bytes after its count
// The siren setting IDs (SirenSettings): their 14 sites are always there - the parser registrations of the siren settings
// and the vehicle variations with their tables and the two ID members' data (type 0x01), the XML reader, the base and a
// pack's loops and builders with the four lea of the ID table, the dedupe and its loop, the three constructors, the init's
// last fill of the table and its call of the base loader, the getter - and so are the carcols data with 24 siren settings of
// 0x5E8 bytes and the ID table of 256 entries with their 24 IDs. The part writes into them; the ID's loads and stores, the dedupe
// and its loop and the constructors run as code from their bytes, so that what it writes is proved by what it does, and the
// loaders, C++ of the fake's own, go by the sites as they are - the table the lea leads to, the width of an ID, the limit - and
// the two members' types. A pack's carcols callback (I-E-H1) and the base builder (I-E-H2) are real code around them, a NOP
// before each, and so are a pack's carcols mount, whose call of the game's file opener is I-E-H3's, and a pack's carvariations callback
// (I-E-H4). The options:
//   siload        no settings yet, the table zeros, until the game's init runs after TweaksV has started: its table filled and
//                 the base loader called through the site's call, which reads the base files and their variations
//   sitwice       with siload: the init a second time
//   siclash       with siload: a base setting's ID taken by an earlier one, the next without an ID
//   sipacks       with siload: three packs' carcols through the callback's start - IDs past 255, one taken, one twice, one without
//                 an ID, the low byte 0xFF, none at all - and their variations
//   siflood       with siload: 20,001 packs' carcols of no settings, and 20,001 carvariations of no vehicles
//   sibigpack     with siload: a pack's carcols of 4,097 settings of one ID, one past the IDs the stub of its dedupe keeps
//   sibadfiles    with siload: a pack's file that does not parse, whose callback gets a false byte; one whose read cannot start, whose
//                 callback comes inside the mount's call, the same; and one whose name has a byte past ASCII and runs past 127
//   sinamebad     with siload: a pack's file whose name cannot be read
//   siotherobject with siload: a pack's file whose callback gets another object than the file opener's call did
//   siprofile     with siload: a pack's file whose name holds the user's profile folder
//   sivariations  with siload: a pack's carvariations through its callback's start: five vehicles, two of them naming an ID that finds
//                 no setting, one that names no ID, two whose setting is there
//   sivarmany     with siload: one of 40 vehicles whose ID finds no setting
//   sivarbad      with siload: one that does not parse, its callback given a false byte
//   sivarunreadable with siload: one whose variations lie in memory that cannot be read
//   sivarnames    with siload: one whose vehicles' names cannot be read, run into memory that cannot be read, have a byte past ASCII,
//                 run past 64 or are empty, all five finding no setting
//   sifilehooked  the file opener's call already leads outside the game (another mod's hook); no pack is mounted through it then
//   sihooknoexec  the page of a pack's carcols callback not executable while TweaksV starts, so that MinHook refuses to hook it
//   sibyte0       the byte before the three hooked functions a 00: none where a function starts
//   silimit       a base builder's limit 65537, past the table's fixed size
//   sidisagree    a pack's builder's second limit 512
//   si512         all four limits 512, in agreement (another mod raised them)
//   silimitop     a base builder's limit not a cmp rsi
//   sipack        12 s after TweaksV has loaded, a pack's 6 settings appended and their IDs entered; the fake stays to 15 s
//   sinone        the carcols pointer null - no settings loaded yet - until 12 s after TweaksV has loaded; stays to 15 s
//   simany        300 settings, given their IDs as the game's builder gives them: the table full, the ID 255 to the last 45
//   siunreadable  the settings' array in memory that cannot be read
//   siwidened     the base loop's first ID load a word load already (0F B7), as another mod would leave it
//   sitype        the variations' ID member's type 0x03, a word, as another mod would leave it
//   siregister    the members' offset fields 0 - their structures not registered yet - until TweaksV has started
//   simembers     the variations' registration with an eighth member
//   sioffsets     the siren settings' member 2 at +0x8, not +0x4
//   sidataout     the siren settings' ID member's data pointer leads outside the game
//   simoved       the four lea of the ID table lead outside the game (another mod moved it)
//   sisplit       a pack's builder's lea of the table leads 16 bytes into it
//   sififth       a fifth lea of the table (a changed game)
//   sijz          the dedupe loop's jz leads outside the game
//   sifill        the init's last fill stores 16 bytes below the table's last 16
//   sicallhooked  the base loader call leads outside the game (another mod redirected it)
//   sicarcols     the getter reads another carcols pointer than a pack's builder
//   sicarcolsout  a pack's builder's carcols pointer lies outside the game
//   sitableout    the siren settings' member data table lies outside the game
//   sioffsetsout  the variations' offsets table lies outside the game
//   sioffsetfield the siren settings' ID member's offset field 8, not +0
//   sidatardata   the siren settings' ID member's data lie in the fake's .rdata, not its .data
//   sibadt2, sibadt3, sibadt4, sibadc1, sibadc2, sibadc3  a site of that match, outside its pattern, not as the findings have it
// The siren count (SirenSettings): its sites in .fakesit - the vehicle lights' calls of the siren lights' renderer and of the
// siren settings getter (I-E-B1), a read of a siren's glass bone (I-E-B3) - and the renderer, with its read of a siren's bone
// (I-E-B2), and find-bone-by-name (I-E-B4) as code the fake's unwind data names; ten vehicle models at the game's offsets, of
// which scdraw draws four: the police car (0x79FBB0C5) with 20 sirens and 20 glass bones, one made for 32 (0xA1B2C3D4), one
// whose siren9 and siren10 are bones 128 and 129 (0x0C0FFEE0), past what a signed byte names, one with 8 sirens (0x2C3D4E5F).
//   scdraw         once TweaksV has started, draws their sirens through the call at the site - TweaksV's stub when it is
//                  redirected, the fake's renderer when it is not - 100 passes of each model from each of two threads at once
//   scodd          with scdraw: three more models among them - one whose bone structure cannot be read (0x0BAD0001), one drawn
//                  without a skeleton (0x0BAD0002), one whose bone structure names siren1 at another bone than its name has,
//                  leaves siren2 unnamed though its bone is below 128, and names a glass bone it has no name for (0x0DD0DD00)
//   scfault        with scdraw: 10 passes of a model whose bones by name lie at an address that cannot be read (0x0BAD0003): the
//                  fake's lookup faults, as find-bone-by-name would on what is no skeleton
//   scmore         with scdraw: 11 seconds after the first passes, two more models, 10 passes each - one more made for 32 sirens
//                  (0xB2C3D4E5) and one with 16 (0x16161616) - then 3 seconds more
//   scfull         with scdraw: 1,100 more models after the four, one pass each - more than TweaksV's table of 1,024 holds
//   schooked       the call of the renderer already leads outside the game (another mod redirected it)
//   scgetterhooked the call of the getter already leads outside the game
//   scelsewhere    the call of the renderer leads to another function of the game than the one the bone read lies in
//   scnotstart     find-bone-by-name's pattern lies 16 bytes into its function, not at its start
//   scnounwind     find-bone-by-name's pattern lies in .fakesit, CC before it, where no unwind data names a function
// The sirens per vehicle's lights: in .fakesit the siren state's allocation (I-E-Z2); in the renderer's carrier the renderer's call of the
// getter (I-E-Z1) and its reads (I-E-Z4 to Z6); the array member's tree reader as code the fake's unwind data names, its prologue real and on to
// a reader of the fake's (I-E-Z3), its walk behind it (I-E-Z7); the lights' member data behind I-E-P1's tables, entry 19. Fifteen vehicles in all, each with
// a car at the game's offsets. Once TweaksV has started the fake registers the member, as the game does, and its parser reads through the array member's slot 1.
//   srdraw         the parser reads the settings of the models it draws, with 20, 21, 24, 28, 32 and 40 items - a twin among them, and one with the same first 20 lights
//                  and others past them - and an array of another member, each setting then copied into the game's array at its model's position; then the
//                  vehicles get their states, as big as the allocation's site says, and are drawn, one pass on this thread and 100 from each of two threads at
//                  once: the renderer's own loop over 20 lights, through the getter's call at its site, what each call lights and what it hands on checked against what
//                  the file's items say, and the vehicle lights' own call of the getter after it. A model with no setting among them, and only while the two calls lead to a
//                  TweaksV stub, since the game's getter reads past its array then, one at position 255 and one at a position past the count but below the capacity
//   srbroken       with srdraw: the 32-siren car's sirens 3 and 25 broken
//   srreset        with srdraw: after the first pass every car's first state made anew, its start time another, and one more pass
//   srrace         with srdraw: a thread replaces one setting's lights past 20 again and again while this one draws its model
//   srmany         300 settings of 21 items each, more than TweaksV's table of 255 holds
//   srflood        20,001 twins of one setting of 21 items
//   srfault        one setting's 26th item faults in the reader of an element, and the next setting reads as ever
//   srelement      the array member's element's member object has another member data than the one TweaksV checked
//   srz1out, srz4out, srz5out, srz6out, srz7out, srz3nounwind   that pattern in .fakesit, where no unwind data names a function, and not in its carrier
//   srz3notstart   the tree reader's pattern 16 bytes into its function
//   srz1other      the renderer's call of the getter leads to another function of the game
//   srz1hooked     it leads outside the game
//   src1notstart   a byte before the siren setting's constructor that no function's start has
//   srdataout, srdatardata   the lights' member data outside the game, in the fake's .rdata
//   sroffset, sroffsetfield, srtype, srsubtype, srsize, srcapacity, srelementtype   the lights' member not as the findings have it: the offsets table's
//                  entry 19 0x40, the data's offset field 8, its type 0x0E, its subtype 2, its element's size 0x50, its capacity 32, an element's type 0x0B
//   srnoelement    the lights' member's element's data not there before the game's registration
//   srnocarcols    with srdraw: the carcols pointer null while it draws, which only TweaksV's guard can take
//   sfjoin         with srdraw and sdrun: the two halves of the raise together: a car drawn on the bone structure TweaksV's wrapper filled at its model's
//                  load, then a hit on its siren25 and a wreck by the game's own loops, their bits handed to the car, its report "siren join: ..."
//   srlateoffset   the game's registration writes the lights' offset 0x44, and its parser reads them there
//   srgetterelsewhere   the vehicle lights' call of the getter leads to another function of the game than I-E-M1's
// Ped props (PedProps): the eleven narrowings with the calls in their matches, A-E-G1's two copies among them, the
// per-anchor count's start and loop and the model info store's lookup are always there, with the calls no pattern
// names - the per-anchor count's 7 callers and the metadata lookup's 17 in all - and two calls that lead elsewhere,
// which the scans must not count. So is the store: 60,010 model indices, 16 bytes each, its models at the game's
// offsets - 601 other models, every 100th index; mp_m_freemode_01 with 30 variation files, 212 props on anchor 0, 38 of
// them in its first file, which also holds a second entry of anchor 0 with 99 and one of anchor 5 with none;
// mp_f_freemode_01, its type byte 0x46, with 28 files, 175 props on anchor 0 and 86 on anchor 2; 40 more peds of one
// file each; a ped without variation data; a null model info; a ped whose bit is clear and one past the count, with 500
// and 600 props on anchor 0; a ped at the last index below the count.
//   ppmany         15 more files of mp_m_freemode_01 (302 props on anchor 0) and a 29th of mp_f_freemode_01 with 270
//                  props on anchor 2 (356 there): two anchors and one file past 255
//   ppgrow         10.5 s after the actions once TweaksV has started, about 12 s in, 5 more files of mp_m_freemode_01
//                  (242 props on anchor 0), as the game adds a pack's - after the measurement's first summary
//   ppunknown      a third entry in mp_m_freemode_01's first file that names anchor 13
//   ppflood        10,001 more peds, each with one file of 300 props on anchor 1: 20,002 lines past 255
//   ppbadcount     the store's count holds 2,000,000
//   ppbadstride    the store's stride holds 12
//   ppunreadable   the 5th file of mp_f_freemode_01 lies in memory that cannot be read
//   ppcountcall    an 8th call of the per-anchor count (a changed game)
//   pplookupcall   an 18th call of the metadata lookup (a changed game)
//   ppg1once       A-E-G1's second copy is gone
//   ppg1thrice     a third copy of A-E-G1
//   ppcountsplit   A-E-T3's call leads into the per-anchor count's loop, not to its start
//   pplookupsplit  A-E-G4's call leads to the texture count, not to the metadata lookup
//   pphooked       A-E-T1's call leads outside the game (another mod hooked it)
//   ppstoreout     A-E-S1's reference to the store's array pointer leads outside the game
//   ppwidened      A-E-T3's narrowing holds 0F B7 C0 already (another mod widened it)
//   ppg1widened    A-E-G1's second copy holds 44 0F B7 C6 already: the pattern matches once, its widened twin once
//   pphuge         one more ped with 258 files of 255 props on anchor 0, 65790 in all: past even the part's 65535
//   ppcountshort   one of the per-anchor count's calls no pattern names leads outside the game (another mod redirected it)
//   pplookupshort  one of the metadata lookup's calls no pattern names leads outside the game (another mod redirected it)
//   ppt1twice      a second copy of A-E-T1, a narrowing's pattern that must match once
//   ppc1twice      a second copy of A-E-C1, the per-anchor count's start
//   ppc2twice      a second copy of A-E-C2, the per-anchor count's loop
//   pps1twice      a second copy of A-E-S1, the model info store's lookup
// The texture budget's sites (the Resources part) are always there: the start-up's stores of the most streamed
// objects and of the four budgets and the call after them, the budget's getter, a resource's placement and the update check,
// leading to the counters - textures 2456 MB and buffers 389 MB in 22222 and 1234 objects, the game's high-water mark at
// 2900 MB, the most objects and the budgets 0 until the game's start-up stores them - to the texture quality's index 2, the
// update check's flag down and its switch on with its factors 0.9, 0.8 and 0.95. Once TweaksV has started, the game's start-up
// stores the most, 98304 objects, and the budgets, 1500, 1500, 3000 and 3000 MB, and makes the call after its stores, with four
// arguments the function behind it checks - through TweaksV's stub when TweaksV raises the budgets - and its result checked in
// turn.
//   tbconfigbig    the start-up's gameconfig.xml gives budgets 2 and 3 9000 MB
//   tbnoindex      the texture quality's index is 7, outside 0 to 3
//   tbearly        the game's start-up runs before TweaksV starts, as with a loader that starts TweaksV late: its stores and
//                  the call after them made before TweaksV's DllMain, and not again
//   tbunreadable   the data's page cannot be read while TweaksV starts
//   tbgrow        once TweaksV's first texture budget summary is in TweaksV.log, the textures grow by 200 MB and 800 objects,
//                  past the budget, the game's mark with them, and the update check's flag goes up; then it waits for the
//                  next summary (TWEAKSV_TEST_SUMMARY_MS), 10 seconds at most
//   tbhigh         once TweaksV's first texture budget summary is in TweaksV.log, one new high at a time, each once the summary
//                  before it is there: the textures' bytes by 100 MB, then the game's mark to 2950 MB, then the textures by 400
//                  objects - the update check's flag left down; then it waits for the last one's summary
//   tblate         its data all 0 - the counters, the index, the flag, the update check's switch and factors - as before the
//                  game's start-up reads gameconfig.xml, until TweaksV's first texture budget summary is in TweaksV.log; then
//                  set as the game's start-up sets them, and the next summary waited for, 10 seconds at most
//   tbshift       the shift between the two stores of the budgets is psllq 21, not 20
//   tbcallhooked   the call after the budgets' stores already leads outside the game (another mod's hook)
//   tbindexout     the getter's index lies outside the game
//   tbcounters     the update check's lea of the counters leads 8 bytes past them
//   tbmost         the placement's check loads the most objects from 4 bytes past them
//   tbfactor      the update check's multiplication by its raise factor is an addition (addss)
//   tbmostout      the start-up's store of the most objects leads outside the game
//   tbbudgetsout   the start-up's store of budgets 0 and 1 leads outside the game
//   tbt2moved      the store of budgets 2 and 3 lies elsewhere: Q-E-T2's 41 bytes copied into the update check's NOPs at +0x60,
//                  the first byte of the one right after the store of budgets 0 and 1 changed
//   tbbudgets23    the store of budgets 2 and 3 leads 24 bytes after budgets 0 and 1, not 16
//   tbcountersout  the placement's load of the buffers' bytes, the start of the counters, leads outside the game
//   tbmoststore    the start-up stores the most objects 4 bytes past where the placement's check reads them
//   tbbudgetsoff   both stores of the budgets lead 16 bytes past them, the counters' +0x38
//   tbindexunreadable  the getter's index lies on a page of its own, which cannot be read while TweaksV starts
//   tbhalfraise    the data lie over two pages, budgets 2 and 3 first on the second, which is read-only when the start-up makes
//                  its call: TweaksV's raise stops at its write of budget 2, half done, and a handler of the fake holds it there
//                  for 12 seconds, past the time of TweaksV's first summary, 10 seconds in, then says how many summaries came
//                  meanwhile and lets it go on; then the fake waits for the next summary, 10 seconds at most
//   tbmostodd      the start-up's store of the most objects leads 1 byte past them
//   tbbudgetsodd   its store of budgets 0 and 1 leads 4 bytes past them, on 4 bytes, not 8
//   tbindexodd     the getter's index leads 1 byte past it
//   tbcountersodd  the placement's load of the buffers' bytes, the start of the counters, leads 4 bytes past them, on 4 bytes, not 8
//   tbswitchodd    the update check's switch leads 2 bytes past it
//   tbt1twice, tbt2twice, tbt3twice, tbt4twice, tbt5twice
//                  the game holds Q-E-T1, Q-E-T2, Q-E-T3, Q-E-T4 or Q-E-T5 twice: a copy of the site's bytes in the update check's
//                  NOPs at +0x60
//   tbwritemov, tbwritebyte, tbwritesse, tbwriteavx, tbwritelock, tbwriteimm, tbwriteextract, tbwriteword, tbwriteaddword
//                  a writer of the budgets besides the start-up's two stores, each of another kind: "mov" of 8 bytes from 4
//                  bytes before them, "mov" of their last byte with an imm8, "movups" of budgets 2 and 3, "vmovdqu" of 32 bytes
//                  from 16 bytes before them, "lock add" of budget 1, "mov" of budget 3 with an imm32, "vextractf128", a store with a
//                  C4 VEX prefix and an imm8, of 16 bytes whose last is the budgets' first, "mov" and "add" of their last word with an
//                  imm16, past their end if the 66 goes unread (altImmediate in the scanner); without its option each
//                  leads to a dummy, while the budgets' neighbours and readers, which are no writers, always lead to them
//   tbmisread0f, tbmisreadc5, tbmisreadc4, tbmisreadf3, tbmisread38, tbmisreadf2
//                  a writer of the budgets behind a byte that once hid it from the scanner's search for stores (SPECIFICATIONS 8):
//                  "mov [rip+disp32], eax" of budget 0 after "and eax, 0Fh", "mov [rip+disp32], rax" of budget 1 after "mov al,
//                  0C5h", "movups [rip+disp32], xmm0" with a REX.B from 8 bytes before the budgets after "mov al, 0C4h", "movd
//                  [rip+disp32], xmm0" of budget 3 after "mov al, 0F3h", "mov [rip+disp32], eax" of budget 2 after "mov eax,
//                  380F0000h", "movbe [rip+disp32], eax" of budget 3 after "mov al, 0F2h"; without its option each leads to the dummy
//   tbmisreadload  "movups xmm0, [rip+disp32]" of budget 0, a load whose 10 reads as "adc [rip+disp32], al", so the search counts it
//   tbwritemost    "vmovdqu [rip+disp32], xmm0" of 16 bytes from 8 bytes before the most objects, which end right before the budgets:
//                  a writer of the most, no writer of the budgets; without it it leads to the dummy
// The descriptor heap's sites are there too: the device's start with the two dwords of its size, its lea and the call of its maker,
// and the maker. It holds 196608 descriptors, the first 30000 in use, as the game's start-up has it - with "tblate" all 0 until then.
//   tbconfig140000, tbconfig147456, tbconfig200000
//                  the start-up's gameconfig.xml gives the most objects 140000, 147456 or 200000, not 98304
//   tbheapnone     the heap's two dwords both 0 when the start-up stores the most
//   tbheapsmall, tbheapedge, tbheaptiny
//                  they say 159744, whose 3/4 cut a key of 131072 to 119808; 131072, whose 3/4 are the game's own 98304; 98304, whose
//                  3/4 are 73728, no raise at all
//   tbsizeout, tbsizeodd
//                  the first of the two dwords leads outside the game, or 1 byte past itself
//   tbheapout, tbheapodd
//                  the lea of the heap leads outside the game, or 4 bytes past it
//   tbmakerhooked  the call of its maker leads outside the game (another mod's hook)
//   tbmakermoved   the maker's stores 1 byte further in than the research found
//   tbheadstores   the store of the head past them another offset's
//   tbt6twice, tbt7twice
//                  the game holds Q-E-T6 or Q-E-T7 twice: a copy of the site's bytes in the update check's NOPs at +0x60
//   tbfree16383, tbfree16384
//                  so many descriptors free, the head so near the list's end
//   tblistcycle    the free list's last entry leads back to its head: a cycle
//   tblistpast     an entry 11 steps from the head holds an index past the size
//   tblistunreadable, tblistnull, tblistodd
//                  the free list in memory that cannot be read, its pointer null, or 1 byte past it
//   tbheaphuge     the heap's size past what the game's index fields hold
//   tbhighdesc     once TweaksV's first texture budget summary is in TweaksV.log, 100 descriptors more in use and nothing else
//                  changed; then it waits for the next summary, 10 seconds at most
//   lpmake KIND PATH   the Legacy packs check: a crafted RPF7 archive of KIND written at PATH, its folders made, before TweaksV
//                      starts; repeatable (LpMake has the kinds)
//   lpmany KIND N DIR  the archive of KIND N times in DIR, p00001.rpf on: the first written, the others hard links to it - 1,023,
//                      the most NTFS gives a file - and copies of it past them
//   lplock PATH        PATH held open without sharing from before TweaksV starts until the report, so the check cannot open it
//   lpbadname          makes mods\lpbad<U+D800>.rpf, a legacy archive whose name is not valid UTF-16 (an unpaired surrogate), before
//                      TweaksV starts, and keeps it open with delete on close, so it goes when the fake ends - however it ends
//   lpwait             waits up to 40 s for the check's last line in TweaksV.log, or its problem that it is not done, then 1.5 s
//                      for the box of its problem; reported as "legacy packs: done" or "legacy packs: no
//                      last line in 40 s"
//   lpfree             once lpwait has waited, each pack lpmake and lpmany made, and the folder it lies in, opened without sharing,
//                      which fails while any handle is open on it - the check's on a pack, a walk's on a folder; reported as "lp: all
//                      N free" or "lp: M of N held"
// The ray tracing buffer (RayTracing): its three sites are always there, as code the fake runs - the buffer's set-up (T-E-B1),
// whose call leads to the fake's buffer maker, a function its unwind data names, and whose store sets the buffer object's
// global; the checks of a frame's ranges, the geometry's (T-E-C1) and the ropes' (T-E-C2), each a function that takes a vertex
// count and a record and takes the range from the slot's counter by lock cmpxchg, or jumps to its refusal, which writes every
// register into the record - and a third lea of the counters, the frame's reset. The frame slot, the two counters and the
// global, in the fake's data, are 0 while TweaksV starts. Once TweaksV has started, the fake makes the buffer through the
// set-up, as the game's renderer does, and leaves the counters as a frame does: the last finished frame's, slot 1's, at
// 2811203 vertices, the frame under way's, slot 0's, at 4000000.
//   rtcheck        then the checks run: the ropes' takes 100 vertices, then 600 geometries from each of two threads at once and 10
//                  ropes are refused - each count past what is left, 4400000 and more, the upper half of rcx not 0 - and every
//                  refusal's registers are compared with those the check left
//   rtgrow         once TweaksV's first ray tracing buffer summary is in TweaksV.log, the last finished frame took 3987120; once
//                  the next one is, the frame flips to slot 1, and the finished one, slot 0's, took 4100000; then it waits for
//                  that summary (the test build's TWEAKSV_TEST_SUMMARY_MS), 10 seconds at most each
//   rtmade         the buffer object's global set while TweaksV starts - the buffer made already - and not made again
//   rtraised       the buffer's count and both checks hold 8388608 (another mod raised them)
//   rtmismatch     the ropes' check holds 8388608, the buffer's count and the geometry's check 4194304
//   rtmakerhooked  the set-up's call leads outside the game (another mod's hook)
//   rtmakerinside  the set-up's call leads 16 bytes into the buffer maker, not to its start
//   rtslot         the ropes' check loads another frame slot
//   rtcounters     the ropes' check loads the counters 4 bytes further on
//   rtextra        a fourth lea of the counters (a changed game)
//   rtjumphooked   the geometry's refusal jump leads outside the game (another mod's hook)
//   rtnounwind     the set-up's call leads to the frame's reset, in .fakesit, where no unwind data names a function
//   rtobjectout    the set-up's store of the buffer object leads outside the game
//   rtobjectunread the set-up's store of the buffer object leads into the hole: a page inside the game, no access while TweaksV starts
//   rtslotout      the ropes' check loads its frame slot from outside the game
//   rtslotunread   the geometry's check loads its frame slot from the hole
//   rtcountersout  the geometry's check loads its counters from outside the game
//   rtcountersend  the geometry's check loads its counters from the image's last 4 bytes: the first counter inside the game, the
//                  second past it
//   rtcountersunread  the ropes' check loads its counters from the hole
//   rtjumpunread   T-E-C2 lies at the end of a page, the site's own first byte changed, and its refusal jump's rel32 on the next
//                  page, which cannot be read while TweaksV starts
//   rtropeshooked  the ropes' refusal jump leads outside the game (another mod's hook)
//   rtnoreset      the frame's reset is no lea of the counters: 2 references to them (a changed game)
//   rtobjectodd    the set-up's store of the buffer object leads 4 bytes past its global: not aligned to its 8 bytes
//   rtslotodd      the geometry's check loads its frame slot from 2 bytes past it: not aligned to its 4 bytes
//   rtcountersodd  the ropes' check loads its counters from 2 bytes past them: not aligned to the 4 bytes of a counter
//   rtsetuptwice, rtgeometrytwice, rtropestwice
//                  T-E-B1, T-E-C1 or T-E-C2 a second time: its site's match copied to the twin, a site that holds nothing else
// TEST_DUMP_DATA's window, g_dwWindow, a stretch of this program's own .data:
//   dwrvas         the report gives its RVA, .data's and what a file's header says about the game, for the suite's key
//   dwwindow       the same, and the window filled before TweaksV loads, written to dw_start.bin, then, as soon as TweaksV's
//                  start-up has patched the heap site, changed and written to dw_later.bin, for the suite to compare with the
//                  files; and 2.5 s more at the end, so that a key's 1 s passes well before the program ends
//   dwnoaccess     with dwwindow: the window's second page cannot be read, from before TweaksV loads; zeros there in both files
//   dwnodata       the section table in memory names no .data: that section's name changed before TweaksV loads
// StaticBounds: the store's init (C-E-L1) lies in .fakesit as code the fake runs, a NOP before it, and C-E-D1's lea beside it. Once TweaksV
// has started, the fake makes its four stores through it as the game does: rcx the configuration, edx the store's hash, r8d its default; the
// call of the fake's pool size lookup, whose table gives a hash its value, -1 for the default; what comes back handed to the store's pool, and
// the store built from it at the game's offsets, its name map's free list and its streamer's count and words among them. The static bounds
// store, hash 0x2CDFE406, gets the 12000 the fake's configuration gives it; the others - 500 slots, the default of 7, 40000 - must come through.
//   sbnames N      N names added to the store as the game adds them: a trap, and no more, when its pool or its name map has no room
//   sbbig          the configuration gives the store 24500, more than STATICBOUNDS_SIZE's default
//   sbhuge         the configuration gives it 40000, more than the key's maximum
//   sbnegative     the configuration gives it -2
//   sbzero         the configuration gives it 0
//   sbdefault      the configuration gives it -1, which is the default, 1
//   sbnever        the store's init never run, the others run
//   sbearly        every store made before TweaksV loads, and not again
//   sbfirst        every store made before the script programs are placed, once TweaksV has started, and not again
//   sblate         every store made 4 seconds later than the other actions leave it, after the test build's check of 2 seconds
//   sbwait         the run stays 5 seconds more: past the test build's check of 2 seconds, its box and the read-back
//   sbmodpool      another mod writes the pool's capacity alone, 30000, once the store is made
//   sbstreamer     the streamer's count one less than the store's
//   sbnotmade      the store's made flag never set
//   sbl1twice, sbd1twice
//                  C-E-L1 or C-E-D1 a second time: its match copied to the twin, a site that holds nothing else
//   sbl1hooked     C-E-L1's call leads outside the game (another mod redirected it): no store is made, nothing being there to call
//   sbd1out, sbd1odd
//                  C-E-D1's lea leads outside the game, or 4 bytes past the store
// The boxes after the start-up summary (TweaksVSummary):
//   latefailures N  once everything else has run, the script program placed N times, 2 s apart, with a code length past what
//                   TEST_DUMP_SCRIPT copies: with that key each placement is a problem, far enough from the next for a box of its own
// The lighting capacity (TweaksVLightCapacity): its 22 sites as the patterns match them, add-to-a-job's-list as code the fake runs,
// and its counters in the fake's data at the game's distances, all 0 until an option sets them once TweaksV has started:
//   lcnumbers      the counters as a frame leaves them from before TweaksV loads, so that every sample sees them, the larger of each two
//                  buffers' in the first and the sort list marked below the store's room; the write pointers once TweaksV has started
//   lcsteps        those, and once the first summary is in the log other numbers, some lower, the larger of each two in the second
//                  buffer and the sort list marked with the store at its room, held 3 s
//   lcrunning      the write pointers set before TweaksV loads, as the game's frames leave them once they run
//   lcrunninglights
//                  the lights' write pointer alone set so
//   lcfull         every counter at its room, the write pointers at their stores' ends, the moved stores' if the sites name them
//   lcpast         every counter one past its room, the write pointers one record past their stores' ends: held until the log has
//                  the lights' pointer's line and every number's line, or 10 s
//   lcbelow        the write pointers below their stores: held until the log has the lights' pointer's line, or 10 s
//   lcjobs         two threads add to full lists through add-to-a-job's-list, 100,000 calls each - if its jump leads to its way out
//                  or to TweaksV's stub; each list's capacities are what G-E-C4's movaps loads, run as code
//   lcframe        a frame past the game's numbers, if the sites name moved stores: G-E-A1 run as code, 60 jobs each with its lists full by
//                  those capacities, each ending through G-E-K1 and G-E-K2 run as code, then the drain's walk from the stores G-E-A2 and
//                  G-E-A3 name; the write pointers stay at the stores' ends
//   lcray          the ray traced list's add, G-E-F1 run as code, 2,000 calls on its first list, if its jump leads to its way out or a stub:
//                  1024 fit, the rest take the jump
//   lcends         two threads end jobs through G-E-K1 and G-E-K2 run as code, 20,000 each, on buffers and write pointers of their own, up to
//                  199 coronas and 79 lights on the one and 150 and 60 on the other, if their calls lead to the copy or a stub
//   lcrounds       with lcray and lcends: once the first summary has a job's most in it, 10 more calls of the full list and 1,000 job ends up to
//                  100 coronas and 40 lights, held 1.5 s
//   lcindex        an index of 700 stored by G-E-D1 and read back by G-E-D2, both run as code
//   lcmodded       another mod's lights' bytes in G-E-C3
//   lcsirens       with simany and sinone: the first summary with no carcols data; then 0 settings, 33 and 255, each until the log has
//                  its numbers; then 40 of them with the array taking no reads for 1.5 s, and back
// The checks, each failing on its own, on any rel32 of the sites by the name of the table in LcSetUp:
//   lcout NAME     it leads past the game's end
//   lcend NAME     it leads to the last 16 bytes of the game
//   lcodd NAME     it leads a byte past its target
//   lcunread NAME  it leads to a page of the fake's data that takes no reads
//   lcother NAME   it leads 16 bytes past its target
//   lcedge NAME    with rtjumpunread: the site whose match ends where this rel32 starts copied to the end of g_rtEdge's first page and the
//                  rel32 on the page rtjumpunread takes away, the site's first byte a NOP
//   lcstartvalues  the lists' start values not 0, 0, 200, 80
//   lcfourthcoronas, lcfourthlights
//                  a fourth lea of that store, its first byte 48 only then
//   lcnoctor       the constructor's two leas not as the scan sees one: their first bytes CC
//   lcg1once       G-E-G1 only once
//   lcg1thrice     G-E-G1 a third time, in the twin
// The two model budgets (Population): their sites in .fakesit, none of them run, and their data in this program's own. With no option
// the data are as at the game's start: the tables' first values, level 3, variety 1, the object zeros.
//   popfill        the data as my boot at 180 s: the tables filled, variety 6, level 3; vehicles 428,679,168 bytes, 94 models
//                  for traffic and 8 dropped, VehicleStruct 102 of 200, 57 ambient cars; peds 305,127,424 bytes, 19 models for the
//                  population, one streamed, the most peds of one 9, and 5, 4 and 3 in the other lists
//   poplate        as at the game's start; once TweaksV.log says they are not filled yet, the data of popfill, then it waits for their line
//   poplevel1, poplevel7, popnet
//                  with popfill: both levels 1; both 7; a network game, the top entries of the network tables another than the story's
//   popbind        with popfill: the peds' use 555 MB, past their stop, as the vehicles' is
//   popoverride    with popfill: the same use, and the object's override 4 in place of -1, which moves the peds' stop past that use again
//   popquiet       with popfill: the vehicles' use below their stop, so that no number moves
//   poptunable     with popfill: the vehicles' entry 3 and the peds' entry 1 hold what the configuration does not give
//   popraise       with popfill: VehiclesSP's Level3Kb 98500, its entry 100,000,000, as a raise in gameconfig.xml gives
//   popvariety     with popfill: both varieties 10
//   popoverflow    with popfill: the vehicles' entry 3 400,000,000, which times 6 passes an int
//   popbadmodel    with popfill: two models of the population's list that cannot be read, a null pointer and one into the hole
//   poplists       with popfill: traffic's list ended at its 50th entry by a value not below the model count, the boats' full, with no end
//   popnopool, poppoolunread, popnoconfig
//                  with popfill: VehicleStruct's pointer null; leading into the hole; the configuration's holder null
//   popgrow        with popfill; once the first summary is in the log, a peak of 1.5 s the next summary's highs must show and its in use
//                  not, the fields a line names changed, then level 1 and back to 3, each waiting for its summary (the test build's
//                  TWEAKSV_TEST_SUMMARY_MS), 10 seconds at most
//   popgone        with popfill; from when TweaksV has started the object's first page cannot be read, until the log says so
//   popflip        with popfill and popwait: while the run waits, a thread turns the population's list's 18th entry between its model and
//                  free, as a release of the model would; the free value's slot of the model info table leads to a model of 1065353216 peds
//   popconfiglate  with popfill: the configuration's holder null, until the log says it could not be read
//   popscanmiss    .fakesit, as the section table in memory has it, ends right after the ModRM of O-E-V7's last store, so that the scan
//                  for writers cannot take that store, its disp32 past the code it reads, where the pattern still matches; what lies
//                  behind in .fakesit is not code then, and stays writable
//   popref NAME HOW  the rel32 of a site (the table in PopSetUp) leads: out, outside the game; hole, to a page inside it that cannot be
//                  read, a network table 16 bytes further on in either, behind where its story table's leads; odd, 2 bytes on; byte, 1 byte
//                  on; next, 8 bytes on
//   popwait        stays until the log holds the peds' first line, 25 seconds at most; popstayssec until the 8 s after the first "not filled yet"
//   popstay N      stays N seconds more, after the rest
//   popzeros       with popfill: the lists still zeros, the override -1; once the log says they are not filled yet, the lists laid out, then it
//                  waits for their line
//   poptablelater  with popfill; once the first summary is in the log, VehiclesSP's Level3Kb 98500 and its entry 100,000,000, then it waits for
//                  the vehicles' line that has them
//   popgetterbyte, popgetterbytev, popgatelist, popgatefloor, popgatecall
//                  a byte changed in the ped getter or the vehicle getter; at the gate's +0x7A, its +0xEA or its call's E8
//   popnotstart    the byte before the tables' set-up 48: no function start there
//   popcalltwice   a second call of the tables' set-up
//   popwriter      a store besides the set-up's into each story table
//   popvmoved, poppmoved, popambientmoved
//                  O-E-V7, O-E-P6 or O-E-V5 elsewhere, whole, but for where the others have it
//   popgateedge    with rtjumpunread, whose page it borrows: O-E-P2 so near that page's end that what is read past the match cannot be
// The ped pool (TweaksVPedPool): its sites are in every run, read by nothing but TweaksV; the pool itself only as asked:
//   pprun          the pool made once TweaksV has started, behind the game's obfuscation: 171 peds in use and 165 in the spatial array for 2.5 s, past a
//                  sample of the module's, then 143 and 139 - 12 mission peds, 4 waiting for reuse, 2 that every loop skips, the free slots' stale bytes misleading
//   ppfull         with pprun: 256 in use in those 2.5 s
//   ppspatialover  with pprun: 300 in the spatial array of 300 in those 2.5 s
//   ppmore         with pprun: 11 s later 150 in use and 140 in the spatial array, then 3 s more
//   pplate         with pprun: the pool made 11 s later, then 3 s more past its settling
//   ppnumber       with pprun: the game's number of peds 300, the pool 256
//   ppstride       with pprun: slots of 0x1000 bytes
//   ppcountzero, ppcountbig
//                  with pprun: the pool's count 0, or 20000
//   ppinusebig, ppinusepeak
//                  with pprun: more in use than the pool has slots, always or only until it settles
//   ppnoflags, ppnopeds
//                  with pprun: the flags, or the slots, on a page that cannot be read
//   ppnospatial, ppbadspatial, ppspatialfault, ppspatialbig
//                  with pprun: the spatial array not made, its count past its capacity, its head on a page that cannot be read, or a capacity of 20000 with 300 in it
//   ppfreed, ppgone
//                  with pprun: the pool's head freed when it settles, or the page of the data that holds its pointer no longer readable
// A site or a field not as the research found it, each on its own - no fake code runs any of them but the accessor, which these leave alone or the pool is not made:
//   ppflagout, ppvalueodd, ppkeysno  the accessor's flag outside the image, its value not aligned, its keys on a page that cannot be read
//   ppproxyout     the maker's proxy outside the image
//   pplookupout    the pool's lookup call outside the image
//   ppfilelookup, ppintelligence
//                  the number's, or PedIntelligence's, lookup call to another function of the game
//   ppsetterout, ppsetterno, ppsetterfirst, ppsettersecond, ppsetterbytes, ppnumberodd
//                  the setter's call outside the image, the setter on a page that cannot be read, its first, second or last byte another, the number not aligned
//   ppstore, ppspatialout, ppstoreunread
//                  the spatial array's store holding other bytes, its pointer outside the image, N-E-S1 at the end of a page and the store on the next, which cannot be read
//   ppcountout, ppcapodd
//                  a count outside the image, a cap not aligned
//   ppgetterout, ppgetters
//                  the copy's first getter call outside the image, its two calls to two functions
//   ppresetout, ppfiller, ppfillerout, ppfillerunread
//                  the timeslice list's count outside the image, the reset's call to another function, the filler's call outside the image, N-E-R3 at the end of a page and its
//                  call's rel32 on the next, which cannot be read
//   pprestore, pppointer, pprestoreunread
//                  the priority function's restore holding other bytes, its pointer to another, N-E-R4 at the end of a page and the restore on the next, which cannot be read
// The pool memory budget (HeapAdjuster, H-E2): its set-up runs once TweaksV has started, as the game's does, and hands its constructor what the
// site holds; then the game's load, an 8 MB block and 120 pools of 1 MB, one given back, charged as the game's allocator charges them:
//   pmraised       the site already holds 200 MB (another mod ran first)
//   pmodd          the site holds a value that is no whole number of MB (another mod ran first)
//   pmbefore       the set-up and the load run before TweaksV starts
//   pmnotmade      the set-up never runs
//   pmheavy        one pool more, of 2 MB, before the one given back, which 136 MB cannot hold
//   pmnosum        1 MB more in use than was charged, so that in use and room do not add up to the budget
//   pmmore         11 s in, one more pool, then 3 s more
//   pmoutside      the site's lea of the allocator leads outside the image: the set-up does not run
//   pmunaligned    the site's lea of the allocator leads to an address not aligned to 8 bytes: the set-up does not run
//   pmunreadable   the allocator's page cannot be read while TweaksV starts: the set-up does not run
//   pmheapout, pmcallout
//                  the site's lea of the heap, or its call of the constructor, leads outside the image: the set-up does not run
//   pmcallunread   H-E2 at the end of a page and its call's rel32 on the next, which cannot be read: the set-up does not run
//   pmfield        the allocator's budget holds another value than the site's while TweaksV starts, which the set-up then overwrites
//   pmgone         the allocator's page cannot be read from the moment TweaksV has started: the set-up does not run
// The render queues (TweaksVRenderQueues), whose sites are code the fake runs only as these ask; its report "render queues: ..." and
// its other fields only with an option that starts rq:
//   rqload         as the game does once TweaksV has started: the inits, every pool made through its size lookup, a frame's peak held,
//                  then the numbers fall to the lines of the findings: 14 and 0, 3 and 0, 5 and 0 in use, the most 31 and 9, 8 and 4, 11 and 3;
//                  five frames, so that the fifth flip comes back to the peak's array and its entries let their references go
//   rqmade         the inits and the pools alone, no slot taken and no frame
//   rqfull         rqload's frames, every pool at its capacity
//   rqpast         the peds' array at 130, the props' at 125, and CVehicleStreamRenderGfx named 80 over VehicleStreamRender's 60, so that the
//                  vehicles' array reaches 70: each past its capacity
//   rqmany         rqload's frames but the peds' array at 5,000 entries, 31 different in the first 4,096 and 9 others past them
//   rqfullrefs     rqload's frames with the draw list's references at their capacity
//   rqrs           each pool's rol and add other than the findings' table, its fields encrypted by them; the first pool's add stays 1, since its
//                  code rotates by that alone
//   rqnomake:I     pool I not made: its flag's bit 0 clear, its other bits, its value and its key set (once per option; I 0 to 5, in kRqStrides' order)
//   rqnomanager    the draw list manager not made
//   rqstride       each pool's stride one more than its code hands it
//   rqstridezero   each pool's stride 0
//   rqstridebrief  each pool's stride wrong for 500 ms, four times, right for 300 ms between them: more than 50 samples in all, at 31 ms each
//                  as well as at 20, never 50 in a row
//   rqpoolimage:I  pool I's pointer leads inside the game's .data, to memory of 0
//   rqpoolgone:I   pool I's pointer leads to a page that cannot be read
//   rqmanagergone  the manager's global leads to a page that cannot be read
//   rqentriesgone  the peds' array's pointer leads to a page that cannot be read, a vehicles' array's is null, their counts set by hand, and a
//                  pool's count reads below 0
//   rqnulls        a props' array of 10 entries, the 5 not yet written 0
//   rqgone         once the load is done, the peds' render pool's flag cleared, the props' render pool's pointer and the manager's global
//                  leading to a page that cannot be read: what was read stands
//   rqdatagone     once the load is done, the page of the game's data the module reads cannot be read any more
//   rqgrow         a new most of every family after the first summary, waited for in the log, in the last of the four arrays
//   rqlatest       the latest numbers change after the first summary, and no most, for 5 s
//   rqbad:WHAT:N   the site WHAT, N the pool, the family or the call it asks for, as RqSetUp has them, not as the research found it; no site runs then
//   rqrvas         the report's "rq rvas: ..." field, with every site's RVA, for the suite to compare with the log
// [siren damage]
// The sirens per vehicle's damage (TweaksVSirenDamage): its sites as code the fake runs (SdSetUp), always; what the report says of them, "sd sites: ...", always.
//   sdrun          once TweaksV has started: the pool set up by its site's stride, seven models made through the call at the site - one made for
//                  20 sirens, one for 32 whose names past siren20 are in other cases, one for 24, one with a few here and there, a plane with gear doors in
//                  its first glass slots, one with bones past what a signed byte names, one with 8 sirens - and every loop run on them
//   sdnoholder     with sdrun: one more model, whose model info's +0xB0 is null, as the game's readers allow
//   sdbadholder    with sdrun: the same, its +0xB0 at an address that cannot be read
//   sdfillfault    with sdrun: one more model, on which the game's fill faults when TweaksV calls it
//   sdbadstructure with sdrun: one more model, whose bone structure lies on a page that cannot be written
//   sdmany         with sdrun: 20,001 more models with bones past siren20
//   sdchanged:L    the site of pattern L changed as another mod would, by a byte the pattern pins, the first of three; no loop runs then, nor with the ones below
//   sdfar:L        the site of pattern L away from the loop it belongs to
//   sdfourth:L     one more copy of pattern L
//   sdhooked       the call of the function that makes a bone structure already leads outside the game (another mod redirected it)
//   sdelsewhere    that call leads to another function of the game than the one the allocation lies in
//   sdallocout     the allocation's pattern where no unwind data names a function
//   sdnotstart     the fill's pattern 16 bytes into its function
//   sdnounwind     the fill's pattern where no unwind data names a function
//   sdpoolmade     the bone structures' pool made before TweaksV starts
//   sdpoolshape    the instruction before the allocation not the load of the pool
//   sdpoolout      that load leading outside the game
// [corona store]
// The corona store (TweaksVCoronaStore): its sites as the patterns match them, G-E-I1 in g_ccGeneralAdd and G-E-G1 in the lighting capacity's two checks, and its count
// and records in the lighting capacity's data. The general add, the job's parameters and batch, its own limit and the copy are code the fake runs, only as these ask:
//   ccrun          once TweaksV has started: the job's parameters; two threads through the general add, 3/4 of the room each; a batch from 100 short of the room that
//                  brings 300; the job's own limit at 1087 and three times at 1088; the copy at a full store and at 0
//   cctrap         in place of ccrun: a batch of 100 from a count of 1500, the first past the game's 1088 that its clamp cannot take
//   ccpast         in place of ccrun: the count at 1089 until the log has the part's first summary, then one past the room, until the log has its line
// The checks, each failing on its own, through the lighting capacity's lcout, lcend, lcodd, lcunread and lcother, on any rel32 by the name of the table in CcSetUp:
//   ccedge NAME    with rtjumpunread: j2extras, j3count or g1jump, the site whose match ends where this rel32 starts copied to the end of g_rtEdge's first page and
//                  the rel32 on the page rtjumpunread takes away, the site's first byte a NOP
//   ccmodded NAME  what another mod would have changed, a byte of what a pattern pins: i1nop, i4past, i7count, i8move, i8clamp, i9limit or j1scratch
#include <windows.h>
#include <winternl.h>
#include <cstddef>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <malloc.h>

// RageOpenV's part of the fake game (fake_game_rov.cpp; its options are listed there).
void RovSetup(int argc, wchar_t** argv);
void RovFirstTimeRequest();
void RovReport();
void RovForEachDataSite(void (*fn)(const volatile unsigned char*, size_t));
void RovForEachCallTarget(void (*fn)(const unsigned char*));

// The copies of the game code live in an executable section, as in the game: TweaksV only searches
// the game's code (build.cmd links this section as ERW).
#pragma data_seg(".fakesit")

// HeapAdjuster: mov edx, 648 MiB / xor r8d,r8d / mov r9b,1 / call ... / lea rcx,...
volatile unsigned char g_heapSite[] = { 0xBA, 0x00, 0x00, 0x80, 0x28, 0x45, 0x31, 0xC0, 0x41, 0xB1, 0x01,
	0xE8, 0x11, 0x22, 0x33, 0x44, 0x48, 0x8D, 0x0D };

// StraightToStoryMode splash: "jz +0x185" at match - 0x0D, then "0F 85 A9 00 00 00 48 8D 54"
volatile unsigned char g_splashSite[] = {
	0x0F, 0x84, 0x85, 0x01, 0x00, 0x00,					// jz rel32 - the patch site
	0x48, 0x8B, 0xCB, 0xE8, 0x55, 0x66, 0x77,			// 7 other bytes
	0x0F, 0x85, 0xA9, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x54 };	// the pattern

// legal: "jz rel32" at match - 0x0F (the game's bytes on 1.0.1158.16), then "00 E9 ? 05 00 00 E8 ? 07"
volatile unsigned char g_legalSite[] = {
	0x0F, 0x84, 0x07, 0x05, 0x00, 0x00,					// jz rel32 - the patch site
	0x48, 0x8D, 0x4C, 0x24, 0x20, 0x33, 0xD2, 0x41, 0xB8,	// 9 other bytes
	0x00, 0xE9, 0x3C, 0x05, 0x00, 0x00, 0xE8, 0x9A, 0x07 };	// the pattern

// landing page: call <callee> / mov ecx, [rip+x] / add ecx, imm8 / test al, al
volatile unsigned char g_landingSite[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x0D, 0x10, 0x20, 0x30, 0x40,
	0x83, 0xC1, 0x07, 0x84, 0xC0 };
volatile unsigned char g_landingCallee[] = { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20 };
// a second copy of the landing page's site, its call to the same function: its first byte becomes E8 only
// with the option "twos3"
volatile unsigned char g_landingSiteCopy[] = { 0xCC, 0x00, 0x00, 0x00, 0x00, 0x8B, 0x0D, 0x10, 0x20, 0x30, 0x40,
	0x83, 0xC1, 0x07, 0x84, 0xC0, 0xCC };

// ModkitLimit: the six "lea reg, [rip+disp32]" references to the game's own modkit array (below, outside this
// section) and the eight hard-coded limits.
volatile unsigned char g_modkitRef1[] = { 0x31, 0xC9, 0x48, 0x8D, 0x05, 0, 0, 0, 0, 0xEB };
volatile unsigned char g_modkitRef2[] = { 0x89, 0xC2, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xEB };
volatile unsigned char g_modkitRef3[] = { 0x41, 0x0F, 0xB7, 0xC2, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x0F, 0xB7, 0x04, 0x41 };
volatile unsigned char g_modkitRef4[] = { 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x41, 0xB8 };
volatile unsigned char g_modkitRef5[] = { 0x48, 0x8D, 0x3D, 0, 0, 0, 0, 0x0F, 0xB7, 0x3C, 0x47 };
volatile unsigned char g_modkitRef6[] = { 0x77, 0x10, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x0F, 0xB7, 0x14, 0x42 };
struct Site { volatile unsigned char* bytes; int offset; size_t size; };
#define SITE(a, off) { a, off, sizeof(a) }
static Site g_modkitRefs[] = { SITE(g_modkitRef1, 5), SITE(g_modkitRef2, 5), SITE(g_modkitRef3, 7),
	SITE(g_modkitRef4, 8), SITE(g_modkitRef5, 3), SITE(g_modkitRef6, 5) };
// a seventh lea, which the game does not have: its first byte becomes 48 only with the option "modkitextra"
volatile unsigned char g_modkitExtra[] = { 0xCC, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };	// rel32 at +3
static Site g_modkitExtraSite = SITE(g_modkitExtra, 3);

volatile unsigned char g_modkitLimit1[] = { 0x0F, 0xB7, 0xC0, 0x66, 0xBA, 0xFF, 0xFF, 0x3D, 0xFF, 0x03, 0x00, 0x00, 0x77 };	// size-1
volatile unsigned char g_modkitLimit2[] = { 0x48, 0x83, 0xC5, 0x04, 0x48, 0x83, 0xC0, 0x04, 0x48, 0x83, 0xC7, 0x04, 0x48, 0x81, 0xFF, 0x03, 0x04, 0x00, 0x00 };	// size+3
volatile unsigned char g_modkitLimit3[] = { 0x74, 0x10, 0x48, 0x83, 0xC5, 0x04, 0x48, 0x83, 0xC3, 0x04, 0x48, 0x83, 0xC0, 0x04, 0x48, 0x3D, 0x03, 0x04, 0x00, 0x00 };	// size+3
volatile unsigned char g_modkitLimit4[] = { 0x48, 0x83, 0xC7, 0x04, 0x48, 0x83, 0xC2, 0x04, 0x48, 0x83, 0xC5, 0x04, 0x48, 0x81, 0xFD, 0x03, 0x04, 0x00, 0x00 };	// size+3
volatile unsigned char g_modkitLimit5[] = { 0x48, 0x83, 0xC7, 0x04, 0x48, 0x83, 0xC2, 0x04, 0x48, 0x83, 0xC6, 0x04, 0x48, 0x81, 0xFE, 0x03, 0x04, 0x00, 0x00 };	// size+3
volatile unsigned char g_modkitLimit6[] = { 0x41, 0xB8, 0x00, 0x08, 0x00, 0x00, 0xB2, 0x00, 0xE8, 0x11, 0x22, 0x33, 0x44, 0x66, 0x0F, 0x76, 0xC0 };	// size*2
volatile unsigned char g_modkitLimit7[] = { 0x0F, 0xB7, 0x41, 0x10, 0x66, 0xBF, 0xFF, 0xFF, 0x48, 0x3D, 0xFF, 0x03, 0x00, 0x00 };	// size-1
volatile unsigned char g_modkitLimit8[] = { 0x41, 0x0F, 0xB7, 0xCA, 0x66, 0xB8, 0xFF, 0xFF, 0x81, 0xF9, 0xFF, 0x03, 0x00, 0x00 };	// size-1
static Site g_modkitLimits[] = { SITE(g_modkitLimit1, 8), SITE(g_modkitLimit2, 15), SITE(g_modkitLimit3, 16), SITE(g_modkitLimit4, 15),
	SITE(g_modkitLimit5, 15), SITE(g_modkitLimit6, 2), SITE(g_modkitLimit7, 10), SITE(g_modkitLimit8, 10) };

// PackfileLimit: the two size limits of the game's own packfile array (below, outside this section), the
// call that reads the streaming configuration and the references to the array. Every site ends in CC so no
// pattern can match across two neighbouring arrays.
volatile uint32_t g_packIndirectSize = 4496;	// the variable "cmp eax, [rip+X]" reads: the array's size minus 4, as the game's own
// direct size "cmp rcx, 4500", one per game build
volatile unsigned char g_packD3[] = { 0x48, 0x83, 0xC6, 0x08, 0x48, 0x81, 0xF9, 0x94, 0x11, 0x00, 0x00, 0x75, 0xE0, 0xCC };			// >= 1.0.1158.13, +7
volatile unsigned char g_packD2[] = { 0x49, 0x83, 0xC3, 0x08, 0x48, 0x81, 0xF9, 0x94, 0x11, 0x00, 0x00, 0x75, 0xE0, 0xEB, 0x05, 0xCC };	// >= 1.0.1013.17, +7
volatile unsigned char g_packD1[] = { 0x48, 0x81, 0xF9, 0x94, 0x11, 0x00, 0x00, 0x74, 0x10, 0x48, 0x83, 0x3C, 0xCA, 0x00, 0xCC };		// older, +3
static Site g_packDirect[] = { SITE(g_packD3, 7), SITE(g_packD2, 7), SITE(g_packD1, 3) };
// indirect size: cmp eax, [rip+X] / jge rel32 / mov eax, eax
volatile unsigned char g_packI1[] = { 0x3B, 0x05, 0, 0, 0, 0, 0x0F, 0x8D, 0x20, 0x01, 0x00, 0x00, 0x89, 0xC0, 0xCC };
// the streaming configuration call: mov [rax+8], ecx / lea rcx, [rip+x] / mov [rax], rcx / mov rcx, rax / call rel32
volatile unsigned char g_packH1[] = { 0x89, 0x48, 0x08, 0x48, 0x8D, 0x0D, 0x11, 0x22, 0x33, 0x44, 0x48, 0x89, 0x08, 0x48, 0x89, 0xC1,
	0xE8, 0, 0, 0, 0, 0xCC };
// "data" references (reads of the array)
volatile unsigned char g_packR01[] = { 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x48, 0x8B, 0x3C, 0xCA, 0x48, 0x8B, 0x2F, 0xCC };
volatile unsigned char g_packR02[] = { 0x48, 0x8D, 0x3D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xCF, 0x48, 0x85, 0xC9, 0xCC };
volatile unsigned char g_packR03[] = { 0x48, 0x8D, 0x3D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0x39, 0x48, 0x85, 0xC9, 0xCC };
volatile unsigned char g_packR04[] = { 0x89, 0xC0, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xC1, 0x48, 0x85, 0xC9, 0x74, 0xCC };
volatile unsigned char g_packR05[] = { 0x89, 0xC0, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xC1, 0x48, 0x85, 0xC9, 0x0F, 0x84, 0xCC };
volatile unsigned char g_packR06[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x4A, 0x8B, 0x0C, 0xC9, 0x48, 0x85, 0xC9, 0x0F, 0x84, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x01, 0xCC };
volatile unsigned char g_packR07[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x1C, 0x08, 0x48, 0x8B, 0x03, 0xCC };
volatile unsigned char g_packR08[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x4A, 0x8B, 0x0C, 0xC1, 0x48, 0x85, 0xC9, 0xCC };
volatile unsigned char g_packR09[] = { 0x48, 0x83, 0xEC, 0x20, 0x48, 0x89, 0xCE, 0x48, 0x8D, 0x05, 0x10, 0x20, 0x30, 0x00, 0x48, 0x89, 0x01, 0xC6, 0x81,
	0x00, 0x01, 0x00, 0x00, 0x00,											// 19..23 rest of the mov byte
	0x48, 0xC7, 0x81, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90,	// 24..35
	0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xCC };									// 36 lea, rel32 at +39
volatile unsigned char g_packR10[] = { 0x48, 0xC1, 0xE8, 0x10, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xC1, 0x48, 0x8B, 0x01, 0xCC };
volatile unsigned char g_packR11[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x83, 0x3C, 0xC1, 0x00, 0x40, 0x0F, 0x95, 0xC6, 0xEB, 0xCC };
volatile unsigned char g_packR12[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0x08, 0x48, 0x8B, 0x01, 0xFF, 0x90, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x48, 0xCC };
volatile unsigned char g_packR13[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0x08, 0x48, 0x8B, 0x01, 0x4C, 0x8D, 0x44, 0x24, 0xCC };
volatile unsigned char g_packR14[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x4C, 0x8B, 0x24, 0x08, 0x49, 0x8B, 0x04, 0x24, 0x4C, 0x8D, 0x44, 0x24, 0xCC };
volatile unsigned char g_packR15[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0x08, 0x48, 0x85, 0xC9, 0x75, 0xCC };
volatile unsigned char g_packR16[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xC1, 0x48, 0x85, 0xC9, 0x0F, 0x84, 0x10, 0x01, 0x00, 0x00, 0x48, 0x89, 0xF5, 0xCC };
volatile unsigned char g_packR17[] = { 0x48, 0x83, 0xEC, 0x20, 0x89, 0xD7, 0x48, 0x89, 0xCE, 0x48, 0x8D, 0x05, 0x10, 0x20, 0x30, 0x00, 0x48, 0x89, 0x01, 0xC6, 0x81, 0xC1,
	0x01, 0x00, 0x00, 0x00,													// 22..25 rest of the mov byte
	0x48, 0xC7, 0x81, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90,	// 26..37
	0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xCC };									// 38 lea, rel32 at +41
volatile unsigned char g_packR18[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x3C, 0x08, 0x48, 0x8B, 0x07, 0x48, 0x89, 0xF9, 0xCC };
volatile unsigned char g_packR19[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0x08, 0x48, 0x8B, 0x01, 0xFF, 0x90, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x40, 0xCC };
volatile unsigned char g_packR20[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xD1, 0x48, 0x85, 0xC9, 0x75, 0xCC };
volatile unsigned char g_packR21[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x4C, 0x8B, 0x24, 0xC1, 0x4D, 0x85, 0xE4, 0x0F, 0x84, 0xCC };
volatile unsigned char g_packR22[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xC1, 0x48, 0x8B, 0x01, 0xFF, 0x90, 0x10, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x38, 0xCC };
// "write" references: mov [rip+X], reg / mov ecx, imm32 / mov edx, imm32 / call rel32 / mov ecx, imm32 / ...
#define PACK_WRITE(reg) 0x48, 0x89, reg, 0, 0, 0, 0, 0xB9, 0x01, 0, 0, 0, 0xBA, 0x02, 0, 0, 0, 0xE8, 0x11, 0x22, 0x33, 0x44, 0xB9, 0x03, 0, 0, 0
volatile unsigned char g_packR23[] = { PACK_WRITE(0x2D), 0x48, 0xC7, 0x44, 0x08, 0xCC };
volatile unsigned char g_packR24[] = { PACK_WRITE(0x35), 0x0F, 0x1F, 0x44, 0x00, 0x00, 0xCC };
volatile unsigned char g_packR25[] = { PACK_WRITE(0x3D), 0x66, 0x90, 0xCC };
volatile unsigned char g_packR26[] = { PACK_WRITE(0x35), 0x66, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC };
volatile unsigned char g_packR27[] = { PACK_WRITE(0x2D), 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC };
volatile unsigned char g_packR28[] = { PACK_WRITE(0x3D), 0x66, 0x66, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCC };
volatile unsigned char g_packR29[] = { PACK_WRITE(0x3D), 0x0F, 0x1F, 0x40, 0x00, 0xCC };
volatile unsigned char g_packR30[] = { PACK_WRITE(0x35), 0x48, 0xC7, 0x44, 0x08, 0xCC };
// the two more the game has: the load in the function that hashes a collection's entries, "lea r15" (+8),
// and the one in P-E-R20's function, behind its int3 trap ("lea rcx": the pattern at +3, its rel32 at +9)
volatile unsigned char g_packR32[] = { 0x48, 0x89, 0xCF, 0x31, 0xF6, 0x4C, 0x8D, 0x3D, 0, 0, 0, 0, 0xEB, 0x10, 0xCC };
volatile unsigned char g_packR33[] = { 0x48, 0x85, 0xC9, 0x75, 0x0C, 0xCC, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x0C, 0xD1,
	0x48, 0x8B, 0x19, 0xCC };
// the reference that depends on the build. A game build has only its own: main makes the other build's one
// no 64-bit lea (its 4C becomes 44), which TweaksV's scan for every reference to the array does not count
volatile unsigned char g_packR31a[] = { 0xBF, 0x08, 0x00, 0x00, 0x00, 0xBE, 0x10, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };	// >= 1.0.1158.13, +13
volatile unsigned char g_packR31b[] = { 0x41, 0xB8, 0x00, 0x10, 0x00, 0x00, 0xBA, 0x01, 0x00, 0x00, 0x00, 0x41, 0xBB, 0xFF, 0xFF, 0x00, 0x00,
	0x4C, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };																		// older builds, +20
// a second, identical copy of the 1158.13+ reference: its first byte becomes BF and its lea a 64-bit one
// (44 becomes 4C) only with the option "twor31a"; its reference points at the array too, so either match
// moves the same array. With the option "junkr31a" it holds the game's obfuscated code that the old pattern
// "BF ? ? ? ? BE ? ? ? ? 4C 8D 05" also matched (1.0.1158.16, RVA 0x582A227) instead.
volatile unsigned char g_packR31aCopy[] = { 0xCC, 0x08, 0x00, 0x00, 0x00, 0xBE, 0x10, 0x00, 0x00, 0x00, 0x44, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };
// a 34th reference, which the game does not have, "mov [rip+X], rax": its first byte becomes 48 only with
// the option "packextra"
volatile unsigned char g_packExtra[] = { 0xCC, 0x89, 0x05, 0, 0, 0, 0, 0xCC };	// rel32 at +3
static Site g_packRefs[] = { SITE(g_packR01, 3), SITE(g_packR02, 3), SITE(g_packR03, 3), SITE(g_packR04, 5), SITE(g_packR05, 5),
	SITE(g_packR06, 3), SITE(g_packR07, 3), SITE(g_packR08, 3), SITE(g_packR09, 39), SITE(g_packR10, 7), SITE(g_packR11, 3),
	SITE(g_packR12, 3), SITE(g_packR13, 3), SITE(g_packR14, 3), SITE(g_packR15, 3), SITE(g_packR16, 3), SITE(g_packR17, 41),
	SITE(g_packR18, 3), SITE(g_packR19, 3), SITE(g_packR20, 3), SITE(g_packR21, 3), SITE(g_packR22, 3), SITE(g_packR23, 3),
	SITE(g_packR24, 3), SITE(g_packR25, 3), SITE(g_packR26, 3), SITE(g_packR27, 3), SITE(g_packR28, 3), SITE(g_packR29, 3),
	SITE(g_packR30, 3), SITE(g_packR32, 8), SITE(g_packR33, 9) };
static Site g_packR31aSite = SITE(g_packR31a, 13);
static Site g_packR31bSite = SITE(g_packR31b, 20);
static Site g_packR31aCopySite = SITE(g_packR31aCopy, 13);
static Site g_packExtraSite = SITE(g_packExtra, 3);
static Site g_packIndirectSite = SITE(g_packI1, 2);
static Site g_packCallSite = SITE(g_packH1, 17);	// the rel32 of the E8 at +0x10

// BoxStreamer: the four sites that hand the capacity of the traversal list to the tree search
// ("mov dword ptr [rsp+20h], 1000"), three of them followed by the "lea r13, [rip+disp32]" that loads
// the list, each followed by the "call rel32" to the tree search (DIAGNOSTIC_MODE redirects it), and the
// other five leas that load the list (two "lea r14", three "lea r13"). g_boxRef4 and g_boxRef8 are the
// same bytes, as in the game, where two of the query functions are near-identical copies: only a scan
// for every lea that loads the list finds both. The list itself and the two tree searches are below,
// outside this section, as the game's are.
volatile unsigned char g_boxCap1[] = { 0x48, 0x8B, 0x44, 0x24, 0x40, 0x48, 0x8B, 0x48, 0x08, 0xC7, 0x44, 0x24, 0x20, 0xE8, 0x03, 0x00, 0x00,
	0x48, 0x8D, 0x54, 0x24, 0x60, 0x4C, 0x8D, 0x44, 0x24, 0x70, 0x4D, 0x89, 0xF1, 0xE8, 0, 0, 0, 0, 0xCC };					// imm32 at +13, call at +30
volatile unsigned char g_boxCap2[] = { 0x48, 0x8B, 0x81, 0x08, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x48, 0x08, 0xC7, 0x44, 0x24, 0x20, 0xE8, 0x03, 0x00, 0x00,
	0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x89, 0x6C, 0x24, 0x38, 0x4D, 0x89, 0xE9, 0xE8, 0, 0, 0, 0, 0xCC };					// imm32 at +15, rel32 at +22, call at +34
volatile unsigned char g_boxCap3[] = { 0x0F, 0x29, 0x44, 0x24, 0x40, 0x48, 0x8B, 0x48, 0x08, 0xC7, 0x44, 0x24, 0x20, 0xE8, 0x03, 0x00, 0x00,
	0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x8D, 0x54, 0x24, 0x40, 0x48, 0x89, 0x6C, 0x24, 0x30, 0x4C, 0x8D, 0x44, 0x24, 0x50,
	0x4D, 0x89, 0xE9, 0xE8, 0, 0, 0, 0, 0xCC };																				// imm32 at +13, rel32 at +20, call at +42
volatile unsigned char g_boxCap4[] = { 0x48, 0x8B, 0x01, 0x48, 0x8B, 0x48, 0x08, 0xC7, 0x44, 0x24, 0x20, 0xE8, 0x03, 0x00, 0x00,
	0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x89, 0x6C, 0x24, 0x38, 0x4D, 0x89, 0xE9, 0xE8, 0, 0, 0, 0, 0xCC };					// imm32 at +11, rel32 at +18, call at +30
volatile unsigned char g_boxRef1[] = { 0x31, 0xC9, 0x4C, 0x8D, 0x35, 0, 0, 0, 0, 0x4C, 0x89, 0x64, 0x24, 0x50, 0x4C, 0x89, 0x4C, 0x24, 0x38, 0xEB, 0x14, 0xCC };	// rel32 at +5
volatile unsigned char g_boxRef2[] = { 0x66, 0x89, 0x5F, 0x0A, 0x48, 0x89, 0x2F, 0x0F, 0xB7, 0x77, 0x08, 0x4C, 0x8D, 0x35, 0, 0, 0, 0, 0x8B, 0x54, 0x24, 0x2C, 0xCC };	// rel32 at +14
volatile unsigned char g_boxRef4[] = { 0x44, 0x0F, 0xB6, 0x64, 0x24, 0x37, 0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x8B, 0x5C, 0x24, 0x40, 0x4C, 0x8B, 0x44, 0x24, 0x38,
	0xE9, 0xEB, 0xFD, 0xFF, 0xFF, 0xCC };	// rel32 at +9
volatile unsigned char g_boxRef6[] = { 0x44, 0x0F, 0xB6, 0x64, 0x24, 0x2F, 0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x8B, 0x6C, 0x24, 0x38, 0x4C, 0x8B, 0x44, 0x24, 0x30,
	0xE9, 0xEA, 0xFD, 0xFF, 0xFF, 0xCC };	// rel32 at +9
volatile unsigned char g_boxRef8[] = { 0x44, 0x0F, 0xB6, 0x64, 0x24, 0x37, 0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0x48, 0x8B, 0x5C, 0x24, 0x40, 0x4C, 0x8B, 0x44, 0x24, 0x38,
	0xE9, 0xEB, 0xFD, 0xFF, 0xFF, 0xCC };	// rel32 at +9
// a ninth lea, which the game does not have: its first byte becomes 4C only with the option "boxninth"
volatile unsigned char g_boxRef9[] = { 0xCC, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };	// rel32 at +3
static Site g_boxCaps[] = { SITE(g_boxCap1, 13), SITE(g_boxCap2, 15), SITE(g_boxCap3, 13), SITE(g_boxCap4, 11) };
static Site g_boxRefs[] = { SITE(g_boxRef1, 5), SITE(g_boxRef2, 14), SITE(g_boxCap2, 22), SITE(g_boxRef4, 9), SITE(g_boxCap3, 20),
	SITE(g_boxRef6, 9), SITE(g_boxCap4, 18), SITE(g_boxRef8, 9) };
static Site g_boxRef9Site = SITE(g_boxRef9, 3);
static Site g_boxCalls[] = { SITE(g_boxCap1, 31), SITE(g_boxCap2, 35), SITE(g_boxCap3, 43), SITE(g_boxCap4, 31) };	// the rel32 of each call

// Decals: the game's code that carries the decal definitions limit and loads the array (the array
// itself, with its count 16 bytes before it, is below, outside this section, as the game's is): the
// parser's "full" check, whose mov reads the count; the parser's lea; FindById's head, with three constant
// sites and a lea beyond the pattern; PickVariant's head; the texture override's five sites, two of them
// with a lea; the start-up loop's start (with a lea) and end; the seven identical "add decal" checks and
// the four that differ; and the other 26 leas of the array, which only the scan for its address finds.
// Every site ends in CC.
#define NOP8 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
volatile unsigned char g_decalFull[] = { 0x8B, 0x35, 0, 0, 0, 0, 0x48, 0x81, 0xFE, 0x03, 0x01, 0x00, 0x00, 0x77, 0xAF, 0x48, 0x89, 0xF9, 0xFF, 0x15, 0xCC };	// disp32 at +2 (the count), imm32 at +9
volatile unsigned char g_decalParserLea[] = { 0x4C, 0x8D, 0x2D, 0, 0, 0, 0, 0xF3, 0x44, 0x0F, 0x10, 0x0D, 0, 0, 0, 0, 0xF3, 0x44, 0x0F, 0x10, 0x15, 0xCC };	// rel32 at +3
volatile unsigned char g_decalFindById[] = {
	0x56, 0x85, 0xC9, 0x74, 0x66, 0xC7, 0x02, 0x04, 0x01, 0x00, 0x00, 0x41, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x8B, 0x1D,	// 0: the pattern, imm32 at +7
	0, 0, 0, 0, NOP8,																													// 21: the mov's disp32, then filler
	0x4C, 0x8D, 0x15, 0, 0, 0, 0,																										// 33 (+0x21): lea r10, rel32 at +36
	NOP8, NOP8, NOP8, NOP8, NOP8, 0x90,																									// 40: filler
	0x81, 0x3A, 0x04, 0x01, 0x00, 0x00,																									// 81 (+0x51): cmp dword ptr [rdx], imm32 at +0x53
	NOP8, 0x90,																															// 87: filler
	0x81, 0x3A, 0x04, 0x01, 0x00, 0x00,																									// 96 (+0x60): imm32 at +0x62
	0x0F, 0x92, 0xC0, 0x5E, 0xC3, 0xCC };																								// 102: setc al / pop rsi / ret
volatile unsigned char g_decalPickVariant[] = { 0xB8, 0x04, 0x01, 0x00, 0x00, 0x81, 0xF9, 0x03, 0x01, 0x00, 0x00, 0x77, 0x2E, 0x41, 0x89, 0xD0, 0x8B, 0x05, 0, 0, 0, 0, 0xCC };	// imm32 at +1 (N) and +7 (N-1)
volatile unsigned char g_decalOverrideOne[] = { 0x45, 0x31, 0xC9, 0x41, 0xBB, 0x04, 0x01, 0x00, 0x00, 0x31, 0xC0, 0x45, 0x31, 0xD2, 0xEB, 0x75, 0xCC };	// imm32 at +5
volatile unsigned char g_decalOverrideLoop[] = { 0x41, 0xBB, 0x04, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x3D, 0, 0, 0, 0, 0x31, 0xC0, 0x45, 0x31, 0xD2, 0xCC };	// imm32 at +2, lea rel32 at +9
volatile unsigned char g_decalOverrideCmp1[] = { 0x41, 0x81, 0xFB, 0x04, 0x01, 0x00, 0x00, 0x89, 0xC3, 0x41, 0x0F, 0x44, 0xD9, 0xCC };	// imm32 at +3
volatile unsigned char g_decalOverrideCmpEbp[] = { 0x81, 0xFD, 0x04, 0x01, 0x00, 0x00, 0x89, 0xD8, 0x41, 0x0F, 0x44, 0xC1, 0xCC };	// imm32 at +2
volatile unsigned char g_decalOverrideCmp2[] = { 0x41, 0x81, 0xFB, 0x04, 0x01, 0x00, 0x00, 0x41, 0x0F, 0x44, 0xC1, 0x48, 0x8D, 0x3D, 0, 0, 0, 0, 0xCC };	// imm32 at +3, lea rel32 at +14
volatile unsigned char g_decalLoopStart[] = { 0xB8, 0xD8, 0x07, 0x00, 0x00, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x0F, 0x57, 0xC0, 0xCC };	// the start offset at +1, lea rel32 at +8
volatile unsigned char g_decalLoopEnd[] = { 0x48, 0x05, 0x20, 0x08, 0x00, 0x00, 0x48, 0x3D, 0x18, 0x59, 0x00, 0x00, 0x0F, 0x85, 0xF3, 0xFD, 0xFF, 0xFF, 0xCC };	// the pass at +2, the end at +8
#define DECAL_ADD_SITE { 0xE8, 0, 0, 0, 0, 0x3D, 0x03, 0x01, 0x00, 0x00, 0x0F, 0x8F, 0x58, 0x01, 0x00, 0x00, 0xCC }	// imm32 at +6
volatile unsigned char g_decalAdd[7][17] = { DECAL_ADD_SITE, DECAL_ADD_SITE, DECAL_ADD_SITE, DECAL_ADD_SITE, DECAL_ADD_SITE, DECAL_ADD_SITE, DECAL_ADD_SITE };
volatile unsigned char g_decalAddR8d[] = { 0x44, 0x89, 0xC3, 0x41, 0x81, 0xF8, 0x03, 0x01, 0x00, 0x00, 0x77, 0x95, 0xCC };	// imm32 at +6
volatile unsigned char g_decalAddEsi[] = { 0x89, 0xC6, 0x81, 0xFE, 0x03, 0x01, 0x00, 0x00, 0x0F, 0x87, 0xE6, 0x04, 0x00, 0x00, 0xCC };	// imm32 at +4
volatile unsigned char g_decalAddRel8[] = { 0xE8, 0, 0, 0, 0, 0x3D, 0x03, 0x01, 0x00, 0x00, 0x7F, 0x32, 0xCC };	// imm32 at +6
volatile unsigned char g_decalAddEbx[] = { 0x89, 0xC3, 0x31, 0xC0, 0x81, 0xFB, 0x03, 0x01, 0x00, 0x00, 0x0F, 0x8F, 0x60, 0x01, 0x00, 0x00, 0xCC };	// imm32 at +6
// the other 26 leas of the array, with the registers the game uses (rel32 at +3 of each)
#define DECAL_LEA(rex, modrm) { rex, 0x8D, modrm, 0, 0, 0, 0, 0xCC }
volatile unsigned char g_decalLeas[26][8] = {
	DECAL_LEA(0x48, 0x0D), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x0D), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x1D), DECAL_LEA(0x48, 0x05),
	DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x0D), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x0D),
	DECAL_LEA(0x48, 0x15), DECAL_LEA(0x48, 0x0D), DECAL_LEA(0x48, 0x35), DECAL_LEA(0x48, 0x1D), DECAL_LEA(0x4C, 0x2D), DECAL_LEA(0x48, 0x1D),
	DECAL_LEA(0x48, 0x15), DECAL_LEA(0x48, 0x15), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x48, 0x05), DECAL_LEA(0x4C, 0x3D), DECAL_LEA(0x48, 0x05),
	DECAL_LEA(0x4C, 0x25), DECAL_LEA(0x4C, 0x1D) };
// a 32nd lea, which the game does not have: its first byte becomes 48 only with the option "decalextra"
volatile unsigned char g_decalLeaExtra[] = { 0xCC, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };	// rel32 at +3
static Site g_decalCountSite = SITE(g_decalFull, 2);
// the 31 leas of the array: the five inside the sites above, then the 26 others (filled in main)
static Site g_decalRefs[31] = { SITE(g_decalParserLea, 3), SITE(g_decalFindById, 36), SITE(g_decalOverrideLoop, 9), SITE(g_decalOverrideCmp2, 14),
	SITE(g_decalLoopStart, 8) };
static Site g_decalLeaExtraSite = SITE(g_decalLeaExtra, 3);
// the 23 sites that carry the limit: 'L' holds N-1, 'N' holds N, 'E' the end of the start-up loop
struct DecalLimit { Site site; char holds; };
static DecalLimit g_decalLimits[23] = {
	{ SITE(g_decalFull, 9), 'L' }, { SITE(g_decalFindById, 7), 'N' }, { SITE(g_decalFindById, 0x53), 'N' }, { SITE(g_decalFindById, 0x62), 'N' },
	{ SITE(g_decalPickVariant, 1), 'N' }, { SITE(g_decalPickVariant, 7), 'L' }, { SITE(g_decalOverrideOne, 5), 'N' },
	{ SITE(g_decalOverrideLoop, 2), 'N' }, { SITE(g_decalOverrideCmp1, 3), 'N' }, { SITE(g_decalOverrideCmpEbp, 2), 'N' },
	{ SITE(g_decalOverrideCmp2, 3), 'N' }, { SITE(g_decalLoopEnd, 8), 'E' },
	{ SITE(g_decalAdd[0], 6), 'L' }, { SITE(g_decalAdd[1], 6), 'L' }, { SITE(g_decalAdd[2], 6), 'L' }, { SITE(g_decalAdd[3], 6), 'L' },
	{ SITE(g_decalAdd[4], 6), 'L' }, { SITE(g_decalAdd[5], 6), 'L' }, { SITE(g_decalAdd[6], 6), 'L' },
	{ SITE(g_decalAddR8d, 6), 'L' }, { SITE(g_decalAddEsi, 4), 'L' }, { SITE(g_decalAddRel8, 6), 'L' }, { SITE(g_decalAddEbx, 6), 'L' } };

// The script programs: the call in a program's resource constructor that resolves its natives ("mov rcx, r12",
// the call, then "mov r14d, 0x3FFF" and "mov eax, [r12+0x70]"), the constructor's loop over the native table,
// which reads the code length at +0x1C and the native table at +0x40, and the script interpreter's prologue,
// which reads the code page table at +0x10. The program itself and the fake's resolution function are below,
// outside this section, as the game's are. Every site ends in CC.
volatile unsigned char g_scriptHookSite[] = { 0x4C, 0x89, 0xE1, 0xE8, 0, 0, 0, 0, 0x41, 0xBE, 0xFF, 0x3F, 0x00, 0x00,
	0x41, 0x8B, 0x44, 0x24, 0x70, 0xCC };																	// the call at +3
volatile unsigned char g_scriptNativeLoop[] = { 0x41, 0x8B, 0x4C, 0x24, 0x1C, 0x01, 0xC1, 0x49, 0x8B, 0x54, 0x24, 0x40, 0x48, 0xD3,
	0x04, 0xC2, 0xCC };
volatile unsigned char g_scriptInterpreter[] = { 0x4D, 0x8B, 0x50, 0x10, 0x49, 0x63, 0x41, 0x1C, 0x4C, 0x8D, 0x7C, 0xC1, 0xF8, 0x49,
	0x63, 0x41, 0x18, 0x4C, 0x8D, 0x0C, 0xC1, 0xCC };
static Site g_scriptCallSite = SITE(g_scriptHookSite, 4);	// the rel32 of the E8 at +3

// The blip pool (the Blips part's measurement): its allocator's "lea rbx, [rip+disp32]" of the pool, the rel32 at
// +10, and its update of the high-water mark, "cmp [rip+disp32], esi" with the disp32 at +5 - then "mov [rip+disp32],
// eax" of the mark, its disp32 at +16. The pool and the mark are below, outside this section, as the game's are.
volatile unsigned char g_blipPoolSite[] = { 0xBA, 0x04, 0x00, 0x00, 0x00, 0x31, 0xC0, 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0x31, 0xF6, 0xCC };
volatile unsigned char g_blipMarkSite[] = { 0x48, 0x89, 0x3B, 0x39, 0x35, 0, 0, 0, 0, 0x7F, 0x09, 0x8D, 0x46, 0x01, 0x89, 0x05,
	0, 0, 0, 0, 0xCC };
static Site g_blipPoolRef = SITE(g_blipPoolSite, 10);
static Site g_blipMarkRefs[] = { SITE(g_blipMarkSite, 5), SITE(g_blipMarkSite, 16) };

// The parts of the parts' coding round and the limits measured first: the game code each part or limit's module finds
// and checks, one block each. What its sites reference lies below, outside this section, as the game's does.
// The weapon infos: the game's initializer of the weapon info blob array - its "lea rcx" of the array (0x58 bytes before
// W-E-I1 in the game), then W-E-I1: a field of the manager zeroed, "mov edx, 160", the call to the allocation, the block's
// "mov [rip+disp32], rax", the capacity's "mov word [rip+disp32], 160" - whose disp32 counts from the instruction's end,
// after its imm16 - and the "lea rcx" of the manager's exit destructor; the allocation's first bytes, "push rsi" to
// "imul rcx, rdi, 0xF8", and "mov edx, 8"; the loader, a NOP before its start (padding, as TweaksV's function-start check
// takes it - a CC would add a line to the RageOpenV suite's count of computed starts), its prologue - seven pushes, "sub
// rsp, 180h" - and two moves, then W-E-I2: the count read, "lea ecx, [rax+1]", the count written, "imul r13, rax, 0xF8"
// and "add r13, [block]"; then "mov eax, 1" and the epilogue that undoes the prologue, so that the loader runs whole when
// it is called, through TweaksV's hook once its guard is on; and a third reference to the array, a store of a block once
// "wiextra" makes its first byte 48. Every site ends in CC.
volatile unsigned char g_weaponInfosLea[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_weaponInfosInit[] = { 0xC7, 0x05, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0x00, 0xBA, 0xA0, 0x00, 0x00, 0x00, 0xE8, 0, 0,
	0, 0, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x66, 0xC7, 0x05, 0, 0, 0, 0, 0xA0, 0x00, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_weaponInfosAllocation[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x89, 0xD6, 0x48, 0x63, 0xFA, 0x48, 0x69,
	0xCF, 0xF8, 0x00, 0x00, 0x00, 0xBA, 0x08, 0x00, 0x00, 0x00, 0xCC };
volatile unsigned char g_weaponInfosLoader[] = { 0x90, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x81,
	0xEC, 0x80, 0x01, 0x00, 0x00, 0x48, 0x89, 0xCE, 0x48, 0x89, 0xD7, 0x0F, 0xB7, 0x05, 0, 0, 0, 0, 0x8D, 0x48, 0x01, 0x66, 0x89, 0x0D,
	0, 0, 0, 0, 0x4C, 0x69, 0xE8, 0xF8, 0x00, 0x00, 0x00, 0x4C, 0x03, 0x2D, 0, 0, 0, 0,	// W-E-I2 at +25
	0xB8, 0x01, 0x00, 0x00, 0x00, 0x48, 0x81, 0xC4, 0x80, 0x01, 0x00, 0x00, 0x5B, 0x5F, 0x5E, 0x41, 0x5C, 0x41, 0x5D, 0x41, 0x5E, 0x41,
	0x5F, 0xC3, 0xCC };
volatile unsigned char g_weaponInfosExtra[] = { 0xCC, 0x89, 0x05, 0, 0, 0, 0, 0xCC };
// "wijump": another mod's hook on the loader - the loader's start a jump to this stub, which runs the three pushes the jump
// took the place of and jumps back into the loader, 6 bytes after its start
volatile unsigned char g_weaponInfosOtherHook[] = { 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0xE9, 0, 0, 0, 0, 0xCC };
static Site g_weaponInfosLeaRef = SITE(g_weaponInfosLea, 3);
static Site g_weaponInfosCall = SITE(g_weaponInfosInit, 16);
static Site g_weaponInfosBlockRef = SITE(g_weaponInfosInit, 23);
static Site g_weaponInfosLoaderRefs[] = { SITE(g_weaponInfosLoader, 28), SITE(g_weaponInfosLoader, 38), SITE(g_weaponInfosLoader, 52) };
static Site g_weaponInfosExtraRef = SITE(g_weaponInfosExtra, 3);
static Site g_weaponInfosSites[] = { SITE(g_weaponInfosLea, 0), SITE(g_weaponInfosInit, 0), SITE(g_weaponInfosAllocation, 0),
	SITE(g_weaponInfosLoader, 0), SITE(g_weaponInfosExtra, 0), SITE(g_weaponInfosOtherHook, 0) };

// The weapon components (the WeaponLimits part): the list's rebuild - "mov dword [rip+disp32], 0" of its
// count, the disp32 at +2 and the instruction 10 bytes long, "movzx r8d, word [rip+disp32]" of the files' count, the
// disp32 at +14, and "lea r10, [rip+disp32]" of the list, the rel32 at +34 - its sort - "movsxd rdx, dword [rip+disp32]"
// of the count (+3), "lea rcx, [rip+disp32]" of the list (+14), the comparison's lea (+21) and "mov r8d, 8" - its update
// of the count (+3 and +12), the pool's size - "mov rcx, [rip+disp32]" of the game's configuration (+3), the pool's name,
// its default 470 and the call of the size lookup (+19) - and the list's 15 other leas, with the registers the game's
// have (rel32 at +3 of each), which only the scan finds. Every site ends in CC.
volatile unsigned char g_weaponComponentsRebuild[] = { 0xC7, 0x05, 0, 0, 0, 0, 0x00, 0x00, 0x00, 0x00, 0x44, 0x0F, 0xB7, 0x05, 0, 0, 0, 0,
	0x66, 0x45, 0x85, 0xC0, 0x0F, 0x84, 0x96, 0x00, 0x00, 0x00, 0x45, 0x31, 0xC9, 0x4C, 0x8D, 0x15, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_weaponComponentsSort[] = { 0x48, 0x63, 0x15, 0, 0, 0, 0, 0xEB, 0x02, 0x31, 0xD2, 0x48, 0x8D, 0x0D, 0, 0, 0, 0,
	0x4C, 0x8D, 0x0D, 0, 0, 0, 0, 0x41, 0xB8, 0x08, 0x00, 0x00, 0x00, 0xCC };
volatile unsigned char g_weaponComponentsUpdate[] = { 0x48, 0x63, 0x15, 0, 0, 0, 0, 0x8D, 0x4A, 0x01, 0x89, 0x0D, 0, 0, 0, 0, 0x4D, 0x89,
	0x04, 0xD2, 0xCC };
volatile unsigned char g_weaponComponentsPool[] = { 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0xBA, 0xCF, 0x3C, 0xE3, 0xBA, 0x41, 0xB8, 0xD6, 0x01,
	0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x89, 0xC3, 0xCC };
#define WEAPON_COMPONENTS_LEA(rex, modrm) { rex, 0x8D, modrm, 0, 0, 0, 0, 0xCC }
volatile unsigned char g_weaponComponentsLeas[15][8] = {
	WEAPON_COMPONENTS_LEA(0x4C, 0x05), WEAPON_COMPONENTS_LEA(0x4C, 0x3D), WEAPON_COMPONENTS_LEA(0x4C, 0x2D), WEAPON_COMPONENTS_LEA(0x48, 0x2D),
	WEAPON_COMPONENTS_LEA(0x4C, 0x2D), WEAPON_COMPONENTS_LEA(0x4C, 0x1D), WEAPON_COMPONENTS_LEA(0x4C, 0x0D), WEAPON_COMPONENTS_LEA(0x4C, 0x05),
	WEAPON_COMPONENTS_LEA(0x4C, 0x0D), WEAPON_COMPONENTS_LEA(0x4C, 0x05), WEAPON_COMPONENTS_LEA(0x48, 0x0D), WEAPON_COMPONENTS_LEA(0x4C, 0x05),
	WEAPON_COMPONENTS_LEA(0x48, 0x15), WEAPON_COMPONENTS_LEA(0x4C, 0x0D), WEAPON_COMPONENTS_LEA(0x4C, 0x0D) };
// an 18th lea, which the game does not have: its first byte becomes 48 only with the option "wcextra"
volatile unsigned char g_weaponComponentsLeaExtra[] = { 0xCC, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };
// The loader of one weaponcomponents.meta, after a NOP - .fakesit has no unwind data: its head as the game's begins, W-E-C5's
// 66 bytes at +0x13 - the allocator's and memset's calls, the blob array's count, capacity and pointer - then the fake's own
// tail: its body called with the entry, the callback and the blob, the frame given back.
volatile unsigned char g_weaponComponentsLoader[] = { 0x90,
	0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x55, 0x53, 0x48, 0x81, 0xEC, 0xD8, 0x00, 0x00, 0x00,
	0x49, 0x89, 0xD7, 0x49, 0x89, 0xCE, 0xB9, 0xD8, 0x0E, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x49, 0x89, 0xC4, 0x41, 0xB8, 0xD8, 0x0E, 0x00,
	0x00, 0x48, 0x89, 0xC1, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0x0F, 0xB7, 0x1D, 0, 0, 0, 0, 0x66, 0x3B, 0x1D, 0, 0, 0, 0, 0x75, 0x00, 0x44,
	0x0F, 0xB7, 0xEB, 0x49, 0x8D, 0x6D, 0x10, 0x48, 0x8B, 0x35, 0, 0, 0, 0,
	0x4C, 0x89, 0xF1, 0x4C, 0x89, 0xFA, 0x4D, 0x89, 0xE0, 0xE8, 0, 0, 0, 0,
	0x48, 0x81, 0xC4, 0xD8, 0x00, 0x00, 0x00, 0x5B, 0x5D, 0x5F, 0x5E, 0x41, 0x5C, 0x41, 0x5D, 0x41, 0x5E, 0x41, 0x5F, 0xC3, 0xCC };
static_assert(sizeof(g_weaponComponentsLoader) == 121, "the loader's bytes");
// every site but the 15 leas, whole, for the protection check and the report; the size lookup's call and the pool's default
static Site g_weaponComponentsSites[] = { SITE(g_weaponComponentsRebuild, 0), SITE(g_weaponComponentsSort, 0), SITE(g_weaponComponentsUpdate, 0),
	SITE(g_weaponComponentsPool, 0), SITE(g_weaponComponentsLeaExtra, 0), SITE(g_weaponComponentsLoader, 0) };
static Site g_weaponComponentsSizeCall = SITE(g_weaponComponentsPool, 19);
static Site g_weaponComponentsDefault = SITE(g_weaponComponentsPool, 14);
// a pattern of the WeaponLimits part a second time, copied here by "wiinittwice", "wiloadertwice", "wcpooltwice" or "wcloadertwice"
volatile unsigned char g_weaponLimitsSecondMatch[128] = { 0xCC };

// The siren setting IDs: each array one match of its pattern with the bytes the module checks at their offsets, from
// I-E-P1 to I-E-M1 (the constructors' C2 and C3 from 15 bytes before their match, C1 from its array's second byte, a NOP before it, since the sirens per
// vehicle call it as a function), and a fifth lea of the ID table that is one only with "sififth" - filled with CC and written by SirenIdsSetup before TweaksV loads.
volatile unsigned char g_sirenIdsP1[42] = { 0xCC };
volatile unsigned char g_sirenIdsP2[42] = { 0xCC };
volatile unsigned char g_sirenIdsP3[15] = { 0xCC };
volatile unsigned char g_sirenIdsT1[0xCD] = { 0xCC };
volatile unsigned char g_sirenIdsT2[0xDA] = { 0xCC };
volatile unsigned char g_sirenIdsT3[0xC6] = { 0xCC };
volatile unsigned char g_sirenIdsT4[0xD6] = { 0xCC };
volatile unsigned char g_sirenIdsD1[27] = { 0xCC };
volatile unsigned char g_sirenIdsD2[43] = { 0xCC };
constexpr int kSirenIdsC1At = 1;
volatile unsigned char g_sirenIdsC1[kSirenIdsC1At + 0xD2] = { 0xCC };
volatile unsigned char g_sirenIdsC2[31] = { 0xCC };
volatile unsigned char g_sirenIdsC3[35] = { 0xCC };
volatile unsigned char g_sirenIdsR1[21] = { 0xCC };
volatile unsigned char g_sirenIdsM1[34] = { 0xCC };
volatile unsigned char g_sirenIdsLeaExtra[8] = { 0xCC };
volatile unsigned char g_sirenIdsH1[64] = { 0xCC };
volatile unsigned char g_sirenIdsH2[64] = { 0xCC };
volatile unsigned char g_sirenIdsH3[96] = { 0xCC };
volatile unsigned char g_sirenIdsH4[96] = { 0xCC };
volatile unsigned char g_sirenIdsRun[0x98] = { 0xCC };	// not a site: the fake's way into the sites as code

// The siren count: the vehicle lights' calls of the siren lights' renderer - the E8 at +13 - and of the siren settings getter -
// the E8 at +24, its rel32 past the pattern (I-E-B1); a read of a siren's glass bone, siren_glass1's byte at +4 (I-E-B3). The
// renderer, with its read of a siren's bone, and find-bone-by-name are below, outside this section: code the fake's unwind data
// names, as the game's does.
volatile unsigned char g_sirenCountCallSite[] = { 0x48, 0x89, 0x6C, 0x24, 0x28, 0x0F, 0x95, 0x44, 0x24, 0x20, 0x4C, 0x89, 0xE1, 0xE8, 0, 0, 0, 0,
	0x49, 0x89, 0xEE, 0x48, 0x89, 0xE9, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_sirenCountGlassRead[] = { 0x0F, 0xBE, 0x94, 0x18, 0xAD, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x8E, 0xC0, 0x09, 0x00, 0x00, 0xCC };
static Site g_sirenCountRendererCall = SITE(g_sirenCountCallSite, 14);	// the rel32 of the E8 at +13
static Site g_sirenCountGetterCall = SITE(g_sirenCountCallSite, 25);		// the rel32 of the E8 at +24
static Site g_sirenCountGlassSite = SITE(g_sirenCountGlassRead, 4);		// the disp32 of siren_glass1's byte
// scnounwind: find-bone-by-name's pattern here, where no unwind data names a function - its first byte, at +1, 41 only then
volatile unsigned char g_sirenCountFindBoneCopy[] = { 0xCC, 0xCC, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20,
	0x66, 0x83, 0x79, 0x5E, 0x00, 0xCC };
static Site g_sirenCountFindBoneSite = SITE(g_sirenCountFindBoneCopy, 1);
// The sirens per vehicle's lights: the siren state's allocation, its size at +1 (I-E-Z2). The part's other sites lie in the carriers below, each
// here instead with the option that puts it outside its function.
volatile unsigned char g_sirenCountStateSite[] = { 0xB9, 0x88, 0x01, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0xC3, 0xCC };
static Site g_sirenCountStateSize = SITE(g_sirenCountStateSite, 1);
volatile unsigned char g_sirenCountOutside[0x60] = { 0xCC };
static Site g_sirenCountOutsideSite = SITE(g_sirenCountOutside, 0);

// Ped props: the eleven narrowings to 8 bits, each with the call in its match - an anchor's total in the check the
// set-prop native asks (T1, its call to the texture count) and in the helmet code after its calls of the per-anchor
// count (T2..T5; T5's match ends in the next call, one of the count's three unused ones), a worn prop's index before
// the metadata lookup (G1 twice, G3..G6; G6 starts with a call of its own, its rel32 left 0); a third copy of G1, its
// first byte 48 only with "ppg1thrice". The per-anchor count's start (C1) and its loop (C2), 0x30 bytes in; the model
// info store's lookup and a ped's type (S1, its four disp32 at +2, +15, +39 and +59). The calls no pattern names: the
// per-anchor count's two more unused ones, the metadata lookup's eleven (two more after G3's, five more after G6's, the
// random props' builder, two in their picker, a thunk); two calls that lead elsewhere; one more call of each function,
// its E8 only with "ppcountcall" or "pplookupcall". The stand-ins they lead to and the store are below.
volatile unsigned char g_pedPropsT1[] = { 0x0F, 0xB6, 0xC3, 0x44, 0x39, 0xC0, 0x7E, 0x17, 0xE8, 0, 0, 0, 0, 0x0F, 0xB6, 0xC0, 0x44, 0x39,
	0xF0, 0x0F, 0x9F, 0xC0, 0xCC };
volatile unsigned char g_pedPropsT2[] = { 0x48, 0x8B, 0x0E, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0x0F, 0xB6, 0xC0, 0x39, 0xC5, 0x0F, 0x4C, 0xC5,
	0x83, 0xFD, 0xFF, 0xCC };
volatile unsigned char g_pedPropsT3[] = { 0x48, 0x8B, 0x09, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0x0F, 0xB6, 0xC0, 0x39, 0xC7, 0x0F, 0x4C, 0xC7,
	0x83, 0xFF, 0xFF, 0xCC };
volatile unsigned char g_pedPropsT4[] = { 0x48, 0x8B, 0x09, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0x0F, 0xB6, 0xC0, 0x39, 0xF8, 0x76, 0x06, 0x89,
	0x7E, 0x18, 0x88, 0x5E, 0x20, 0xCC };
volatile unsigned char g_pedPropsT5[] = { 0x48, 0x8B, 0x09, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0x0F, 0xB6, 0xC0, 0x39, 0xC7, 0x7C, 0x0A, 0x48,
	0x8B, 0x0E, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG1a[] = { 0x48, 0x8B, 0x84, 0x24, 0x90, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x40, 0x02, 0x00, 0x00, 0x44,
	0x0F, 0xB6, 0xC6, 0x40, 0x0F, 0xB6, 0xD5, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG1b[] = { 0x48, 0x8B, 0x84, 0x24, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x40, 0x02, 0x00, 0x00, 0x44,
	0x0F, 0xB6, 0xC6, 0x40, 0x0F, 0xB6, 0xD5, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG1c[] = { 0xCC, 0x8B, 0x84, 0x24, 0xB0, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x88, 0x40, 0x02, 0x00, 0x00, 0x44,
	0x0F, 0xB6, 0xC6, 0x40, 0x0F, 0xB6, 0xD5, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG3[] = { 0x41, 0x8B, 0x44, 0x04, 0x08, 0x83, 0xF8, 0xFF, 0x0F, 0x84, 0x20, 0x01, 0x00, 0x00, 0x0F, 0xB6,
	0xE8, 0x48, 0x89, 0xF9, 0x89, 0xF2, 0x41, 0x89, 0xE8, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG4[] = { 0x8B, 0x44, 0x38, 0x08, 0x31, 0xFF, 0x83, 0xF8, 0xFF, 0x74, 0x2A, 0x48, 0x8B, 0x49, 0x20, 0x48,
	0x8B, 0x89, 0x40, 0x02, 0x00, 0x00, 0x44, 0x0F, 0xB6, 0xC0, 0x0F, 0xB6, 0xD2, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG5[] = { 0x41, 0x8B, 0x44, 0x07, 0x08, 0x85, 0xC0, 0x0F, 0x88, 0x40, 0x01, 0x00, 0x00, 0x44, 0x0F, 0xB6,
	0xC0, 0x48, 0x89, 0xF9, 0x44, 0x89, 0xF2, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsG6[] = { 0xE8, 0, 0, 0, 0, 0x44, 0x0F, 0xB6, 0xF7, 0x45, 0x0F, 0xB6, 0xFF, 0x4C, 0x89, 0xE9, 0x44, 0x89,
	0xFA, 0x45, 0x89, 0xF0, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_pedPropsCount[] = { 0x56, 0x57, 0x48, 0x8B, 0x41, 0x20, 0x48, 0x8B, 0x88, 0x40, 0x02, 0x00, 0x00, 0x44, 0x0F, 0xB7,
	0x41, 0x08, 0x31, 0xC0, 0x4D, 0x85, 0xC0, 0x74, 0x48, 0x4C, 0x8B, 0x09, 0x45, 0x31, 0xDB, 0xEB,			// C1, the start
	0x1A, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x40, 0x00,			// to +0x30
	0x44, 0x0F, 0xB7, 0x54, 0x39, 0x08, 0x41, 0x0F, 0xB6, 0xCA, 0x01, 0xC8, 0x49, 0xFF, 0xC3, 0x4D, 0x39, 0xC3,	// C2, the loop
	0x74, 0x2C, 0x4B, 0x8B, 0x0C, 0xD9, 0x0F, 0xB7, 0x71, 0x60, 0x45, 0x31, 0xD2, 0x48, 0x85, 0xF6, 0x74, 0xE0, 0x48, 0x8B,
	0x49, 0x58, 0x48, 0xC1, 0xE6, 0x03, 0x48, 0x8D, 0x34, 0x76, 0x31, 0xFF, 0xCC };
volatile unsigned char g_pedPropsStore[] = { 0x3B, 0x05, 0, 0, 0, 0, 0x0F, 0x8D, 0x50, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x89,
	0xC2, 0xC1, 0xEA, 0x05, 0x8B, 0x0C, 0x91, 0x0F, 0xA3, 0xC1, 0x0F, 0x83, 0x3C, 0x01, 0x00, 0x00, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48,
	0x85, 0xC9, 0x74, 0x30, 0x44, 0x89, 0x7C, 0x24, 0x34, 0x48, 0x98, 0x48, 0x0F, 0xAF, 0x05, 0, 0, 0, 0, 0x4C, 0x8B, 0x3C, 0x01, 0xBB,
	0x02, 0x00, 0x00, 0x00, 0x4D, 0x85, 0xFF, 0x74, 0x1E, 0x41, 0x0F, 0xB6, 0x87, 0x9D, 0x00, 0x00, 0x00, 0x24, 0x1F, 0x3C, 0x06, 0xCC };
volatile unsigned char g_pedPropsCalls[13][6] = { { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC },
	{ 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC },
	{ 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC } };
volatile unsigned char g_pedPropsElsewhere[2][6] = { { 0xE8, 0, 0, 0, 0, 0xCC }, { 0xE8, 0, 0, 0, 0, 0xCC } };
volatile unsigned char g_pedPropsExtra[2][6] = { { 0xCC, 0, 0, 0, 0, 0xCC }, { 0xCC, 0, 0, 0, 0, 0xCC } };
// Room for a second copy of one match, which only "ppt1twice", "ppc1twice", "ppc2twice" or "pps1twice" puts there; S1's is
// the longest.
volatile unsigned char g_pedPropsSecondCopy[sizeof(g_pedPropsStore)] = { 0xCC };
static Site g_pedPropsTextureCall = SITE(g_pedPropsT1, 9);	// the rel32 of each call
static Site g_pedPropsCountCalls[] = { SITE(g_pedPropsT2, 6), SITE(g_pedPropsT3, 6), SITE(g_pedPropsT4, 6), SITE(g_pedPropsT5, 6),
	SITE(g_pedPropsT5, 23), SITE(g_pedPropsCalls[0], 1), SITE(g_pedPropsCalls[1], 1) };
static Site g_pedPropsLookupCalls[] = { SITE(g_pedPropsG1a, 24), SITE(g_pedPropsG1b, 24), SITE(g_pedPropsG3, 26), SITE(g_pedPropsG4, 30),
	SITE(g_pedPropsG5, 24), SITE(g_pedPropsG6, 23), SITE(g_pedPropsCalls[2], 1), SITE(g_pedPropsCalls[3], 1), SITE(g_pedPropsCalls[4], 1),
	SITE(g_pedPropsCalls[5], 1), SITE(g_pedPropsCalls[6], 1), SITE(g_pedPropsCalls[7], 1), SITE(g_pedPropsCalls[8], 1),
	SITE(g_pedPropsCalls[9], 1), SITE(g_pedPropsCalls[10], 1), SITE(g_pedPropsCalls[11], 1), SITE(g_pedPropsCalls[12], 1) };
static Site g_pedPropsElsewhereCalls[] = { SITE(g_pedPropsElsewhere[0], 1), SITE(g_pedPropsElsewhere[1], 1) };
static Site g_pedPropsExtraCalls[] = { SITE(g_pedPropsExtra[0], 1), SITE(g_pedPropsExtra[1], 1) };
static Site g_pedPropsStoreRefs[] = { SITE(g_pedPropsStore, 2), SITE(g_pedPropsStore, 15), SITE(g_pedPropsStore, 39), SITE(g_pedPropsStore, 59) };
static Site g_pedPropsSites[] = { SITE(g_pedPropsT1, 0), SITE(g_pedPropsT2, 0), SITE(g_pedPropsT3, 0), SITE(g_pedPropsT4, 0),
	SITE(g_pedPropsT5, 0), SITE(g_pedPropsG1a, 0), SITE(g_pedPropsG1b, 0), SITE(g_pedPropsG1c, 0), SITE(g_pedPropsG3, 0), SITE(g_pedPropsG4, 0),
	SITE(g_pedPropsG5, 0), SITE(g_pedPropsG6, 0), SITE(g_pedPropsCount, 0), SITE(g_pedPropsStore, 0),
	{ &g_pedPropsCalls[0][0], 0, sizeof(g_pedPropsCalls) }, { &g_pedPropsElsewhere[0][0], 0, sizeof(g_pedPropsElsewhere) },
	{ &g_pedPropsExtra[0][0], 0, sizeof(g_pedPropsExtra) }, SITE(g_pedPropsSecondCopy, 0) };
// The byte of each of the eleven narrowings the part makes B7, its B6 - G1's third copy, which matches only with
// "ppg1thrice", not among them - so that the report tells the sites widened from the sites as placed
static volatile unsigned char* const g_pedPropsWidenable[] = { &g_pedPropsT1[1], &g_pedPropsT2[11], &g_pedPropsT3[11], &g_pedPropsT4[11],
	&g_pedPropsT5[11], &g_pedPropsG1a[17], &g_pedPropsG1b[17], &g_pedPropsG3[15], &g_pedPropsG4[24], &g_pedPropsG5[15], &g_pedPropsG6[7] };

// The texture budget (the Resources part): the start-up's store of the most streamed objects - "mov ecx,
// [rax+0x54C]", "mov [rip+disp32], ecx" with the rel32 at +8 - and its load, shift and store of budgets 0 and 1 (the rel32 at
// +30), then right after them, at +34, those of budgets 2 and 3 (+52), "lea r14", "mov [rip+disp32], r14", "sub rsp, 0x20"
// and the call after the stores (+75); the budget's getter, "movsxd rax, [rip+disp32]" (the index's rel32 at +3) and "mov
// rax, [rcx+rax*8+0x28]"; a resource's placement and its loads of the counters (the rel32 at +2, +8, +16, +25 and +32); the
// update check: the compare of its switch (the disp32 at +2, an imm8 after it), its loads of the counters (+16, +23, +30), the
// getter's call (+35) and, laid in main, NOPs, the compare of its flag at +0x4B (+0x4D) and its multiplications by its lower
// factor at +0x54 (+0x58) and, with a REX, by its raise factor at +0xA0 (+0xA5). What they lead to is below, outside this
// section, as the game's is.
volatile unsigned char g_textureBudgetStores[] = {
	0x8B, 0x88, 0x4C, 0x05, 0x00, 0x00, 0x89, 0x0D, 0, 0, 0, 0, 0x66, 0x0F, 0x38, 0x25, 0x80, 0x50, 0x05, 0x00, 0x00,
	0x66, 0x0F, 0x73, 0xF0, 0x14, 0xF3, 0x0F, 0x7F, 0x05, 0, 0, 0, 0,
	0x66, 0x0F, 0x38, 0x25, 0x80, 0x58, 0x05, 0x00, 0x00, 0x66, 0x0F, 0x73, 0xF0, 0x14, 0xF3, 0x0F, 0x7F, 0x05, 0, 0, 0, 0,
	0x4C, 0x8D, 0x35, 0, 0, 0, 0, 0x4C, 0x89, 0x35, 0, 0, 0, 0, 0x48, 0x83, 0xEC, 0x20, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_textureBudgetGetter[] = { 0x48, 0x63, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x44, 0xC1, 0x28, 0xC3, 0xCC };
volatile unsigned char g_textureBudgetPlacement[] = { 0x8B, 0x05, 0, 0, 0, 0, 0x8B, 0x0D, 0, 0, 0, 0, 0x01, 0xC1, 0x3B, 0x0D, 0, 0, 0, 0,
	0x73, 0x3A, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x35, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_textureBudgetUpdate[0xAA] = { 0x83, 0x3D, 0, 0, 0, 0, 0x00, 0x0F, 0x84, 0, 0, 0, 0, 0x48, 0x8D, 0x0D, 0, 0, 0, 0,
	0x48, 0x8B, 0x3D, 0, 0, 0, 0, 0x48, 0x8B, 0x35, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0 };
static Site g_textureBudgetMostRef = SITE(g_textureBudgetStores, 8);
static Site g_textureBudgetBudgetRefs[] = { SITE(g_textureBudgetStores, 30), SITE(g_textureBudgetStores, 52) };
static Site g_textureBudgetCall = SITE(g_textureBudgetStores, 75);
static Site g_textureBudgetIndexRef = SITE(g_textureBudgetGetter, 3);
// the placement's loads and the field of the counters each leads to
struct TextureBudgetLoad { Site site; size_t field; };
static TextureBudgetLoad g_textureBudgetLoads[] = { { SITE(g_textureBudgetPlacement, 2), 0x10 }, { SITE(g_textureBudgetPlacement, 8), 0x14 },
	{ SITE(g_textureBudgetPlacement, 16), 0x20 }, { SITE(g_textureBudgetPlacement, 25), 0x00 }, { SITE(g_textureBudgetPlacement, 32), 0x08 },
	{ SITE(g_textureBudgetUpdate, 16), 0x00 }, { SITE(g_textureBudgetUpdate, 23), 0x00 }, { SITE(g_textureBudgetUpdate, 30), 0x08 } };
static Site g_textureBudgetSwitchRef = SITE(g_textureBudgetUpdate, 2);
static Site g_textureBudgetGetterCall = SITE(g_textureBudgetUpdate, 35);
static Site g_textureBudgetFlagRef = SITE(g_textureBudgetUpdate, 0x4D);
static Site g_textureBudgetFactorRefs[] = { SITE(g_textureBudgetUpdate, 0x58), SITE(g_textureBudgetUpdate, 0xA5) };	// the lower, the raise
// The budgets' neighbours and readers, which the scan for the budgets' writers must not count: "mov [rip+disp32], eax" 4 bytes
// before them, "mov dword [rip+disp32], imm32" right after them, "vmovdqu [rip+disp32], xmm0" 16 bytes before them only with its
// option (tbwritemost, at +22: those 16 bytes hold the most objects, so it is a writer of the most), "mov rax",
// "movlps xmm0", "cmp [rip+disp32], rax", "cmp byte [rip+disp32], 0" of their last byte, "movdqu xmm0" and "vmovdqu ymm0" reading
// them and "mov [rip+disp32], rax" right after them - the loads ones no reading of their bytes takes for a store, since the scan
// reads the bytes before an operand every way an instruction could start among them (a "movups xmm0", 0F 10, or a "vmovups
// ymm0", C5 FC 10, would read as "adc [rip+disp32], al" behind a 0F or a C5, a "movq xmm0", F3 0F 7E, as "movd" behind an F3);
// then the writers it must count, each leading to the budgets only with its
// option, else to a dummy: "mov [rip+disp32], rax" 4 bytes before them (tbwritemov, at +97), "mov byte [rip+disp32], imm8" of
// their last byte (tbwritebyte, +106), "movups [rip+disp32], xmm1" of budgets 2 and 3 (tbwritesse, +115), "vmovdqu
// [rip+disp32], ymm0" from 16 bytes before them (tbwriteavx, +124), "lock add [rip+disp32], rax" of budget 1 (tbwritelock,
// +134), "mov qword [rip+disp32], imm32" of budget 3 (tbwriteimm, +144), "vextractf128 [rip+disp32], ymm0, 1" of 16 bytes whose
// last is the budgets' first (tbwriteextract, +157: a C4 VEX prefix, and neither the prefix nor the imm8 unread would count it),
// "mov word [rip+disp32], imm16" (tbwriteword, +169) and "add word [rip+disp32], imm16" (tbwriteaddword, +180) of their last word:
// with the immediate read as an imm32, not as the 66 makes it, each would lie past their end. Two NOPs before each, so that no byte of the one
// before it is read as its prefix.
volatile unsigned char g_textureBudgetWriters[] = {
	0x90, 0x90, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xC7, 0x05, 0, 0, 0, 0, 0x11, 0x11, 0x11, 0x11,
	0x90, 0x90, 0xC5, 0xFA, 0x7F, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x48, 0x8B, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x0F, 0x12, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x48, 0x39, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x80, 0x3D, 0, 0, 0, 0, 0x00,
	0x90, 0x90, 0xF3, 0x0F, 0x6F, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xC5, 0xFE, 0x6F, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x48, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x48, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xC6, 0x05, 0, 0, 0, 0, 0x22,
	0x90, 0x90, 0x0F, 0x11, 0x0D, 0, 0, 0, 0,
	0x90, 0x90, 0xC5, 0xFE, 0x7F, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xF0, 0x48, 0x01, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x48, 0xC7, 0x05, 0, 0, 0, 0, 0x33, 0x33, 0x33, 0x33,
	0x90, 0x90, 0xC4, 0xE3, 0x7D, 0x19, 0x05, 0, 0, 0, 0, 0x01,
	0x90, 0x90, 0x66, 0xC7, 0x05, 0, 0, 0, 0, 0x44, 0x44,
	0x90, 0x90, 0x66, 0x81, 0x05, 0, 0, 0, 0, 0x55, 0x55,
	0xCC };
// each one's disp32, the byte of the budgets its store or load starts at, the size of the immediate after the disp32, and
// the option that leads a writer to the budgets
struct TextureBudgetOperand { Site site; int from; int immediate; const wchar_t* option; };
static TextureBudgetOperand g_textureBudgetOperands[] = { { SITE(g_textureBudgetWriters, 4), -4, 0, nullptr },
	{ SITE(g_textureBudgetWriters, 12), 32, 4, nullptr }, { SITE(g_textureBudgetWriters, 26), -16, 0, L"tbwritemost" },
	{ SITE(g_textureBudgetWriters, 35), 0, 0, nullptr }, { SITE(g_textureBudgetWriters, 44), 0, 0, nullptr },
	{ SITE(g_textureBudgetWriters, 53), 0, 0, nullptr }, { SITE(g_textureBudgetWriters, 61), 31, 1, nullptr },
	{ SITE(g_textureBudgetWriters, 72), 0, 0, nullptr }, { SITE(g_textureBudgetWriters, 82), 0, 0, nullptr },
	{ SITE(g_textureBudgetWriters, 91), 32, 0, nullptr },
	{ SITE(g_textureBudgetWriters, 100), -4, 0, L"tbwritemov" }, { SITE(g_textureBudgetWriters, 108), 31, 1, L"tbwritebyte" },
	{ SITE(g_textureBudgetWriters, 118), 16, 0, L"tbwritesse" }, { SITE(g_textureBudgetWriters, 128), -16, 0, L"tbwriteavx" },
	{ SITE(g_textureBudgetWriters, 138), 8, 0, L"tbwritelock" }, { SITE(g_textureBudgetWriters, 147), 24, 4, L"tbwriteimm" },
	{ SITE(g_textureBudgetWriters, 162), -15, 1, L"tbwriteextract" }, { SITE(g_textureBudgetWriters, 172), 30, 2, L"tbwriteword" },
	{ SITE(g_textureBudgetWriters, 183), 30, 2, L"tbwriteaddword" } };
// The scanner's readings of the bytes before an operand (SPECIFICATIONS 8): a writer of the budgets behind each kind of
// byte that once hid it, each leading to the budgets only with its option, else to the dummy - "and eax, 0Fh" then "mov
// [rip+disp32], eax" of budget 0 (tbmisread0f, the store at +5: the 0F read as an escape made it no store), "mov al, 0C5h" then
// "mov [rip+disp32], rax" of budget 1 (tbmisreadc5, +15: the C5 read as a VEX prefix), "mov al, 0C4h" then "movups [rip+disp32],
// xmm0" from 8 bytes before the budgets with a REX.B, which means nothing with [rip+disp32] but is the one kind of byte after a
// C4 that reads as a VEX map (tbmisreadc4, +26: the C4 read as a VEX prefix made it a vmovsd of 8 bytes, short of them), and
// "mov al, 0F3h" then "movd [rip+disp32], xmm0" of budget 3 (tbmisreadf3, +38: the F3 read as a prefix made it movq's load),
// "mov eax, 380F0000h" then "mov [rip+disp32], eax" of budget 2 (tbmisread38, +53: the 0F 38 read as an escape made it no store),
// "mov al, 0F2h" then "movbe [rip+disp32], eax" of budget 3 (tbmisreadf2, +63: the F2 read as a prefix made it crc32's load) and,
// no writer, "movups xmm0, [rip+disp32]" of budget 0 (tbmisreadload, +74: its 10 reads as adc, so the search counts it). Two NOPs
// before each, as above.
volatile unsigned char g_textureBudgetMisread[] = {
	0x90, 0x90, 0x83, 0xE0, 0x0F, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xB0, 0xC5, 0x48, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xB0, 0xC4, 0x41, 0x0F, 0x11, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xB0, 0xF3, 0x66, 0x0F, 0x7E, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xB8, 0x00, 0x00, 0x0F, 0x38, 0x89, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0xB0, 0xF2, 0x0F, 0x38, 0xF1, 0x05, 0, 0, 0, 0,
	0x90, 0x90, 0x0F, 0x10, 0x05, 0, 0, 0, 0,
	0xCC };
static TextureBudgetOperand g_textureBudgetMisreadOperands[] = { { SITE(g_textureBudgetMisread, 7), 0, 0, L"tbmisread0f" },
	{ SITE(g_textureBudgetMisread, 18), 8, 0, L"tbmisreadc5" }, { SITE(g_textureBudgetMisread, 30), -8, 0, L"tbmisreadc4" },
	{ SITE(g_textureBudgetMisread, 42), 24, 0, L"tbmisreadf3" }, { SITE(g_textureBudgetMisread, 55), 16, 0, L"tbmisread38" },
	{ SITE(g_textureBudgetMisread, 67), 24, 0, L"tbmisreadf2" }, { SITE(g_textureBudgetMisread, 76), 0, 0, L"tbmisreadload" } };
// The descriptor heap: in the device's start the loads of its size (the rel32 at +3 and +10), the lea of the heap (+30) and the call
// of its maker (+40); the maker, NOPs laid in main but for its stores of the size and of no free list, 0x36 bytes in, and the store
// of the head 0x9E past them. What they lead to is below, outside this section, as the game's is.
volatile unsigned char g_textureBudgetHeapSite[] = { 0x44, 0x8B, 0x05, 0, 0, 0, 0, 0x44, 0x03, 0x05, 0, 0, 0, 0, 0xC6, 0x44, 0x24, 0x28, 0x00,
	0xC7, 0x44, 0x24, 0x20, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x31, 0xD2, 0x45, 0x31, 0xC9, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_textureBudgetMaker[0xE0] = { 0xCC };
static Site g_textureBudgetSizeRefs[] = { SITE(g_textureBudgetHeapSite, 3), SITE(g_textureBudgetHeapSite, 10) };
static Site g_textureBudgetHeapRef = SITE(g_textureBudgetHeapSite, 30);
static Site g_textureBudgetMakerCall = SITE(g_textureBudgetHeapSite, 40);

// The ray tracing buffer: its sites as code the fake runs. The set-up, a function of its own: "sub rsp, 0x38", then T-E-B1 at +4 -
// the maker's flags, "mov ecx, 0x400000" (the count's imm32 at +13), "mov edx, 20", "mov r8b, 1", "xor r9d, r9d", the maker's
// call (+28, its rel32 at +29) and "mov [rip+disp32], rax" of the buffer object (+33, its disp32 at +36) - "add rsp, 0x38",
// "ret". A check, (count, record): rbx, rbp, rsi, rdi and r12 to r15 pushed, r15 the record, r8, r9 and r11 to r14 from it and
// rsp into it; T-E-C1 or T-E-C2 at +42 - the slot's load (its disp32 at +44), "xor", the counters' lea (its disp32 at +53),
// "mov r10d, 1", the filler, the slot's counter and the range's end, "cmp" with the capacity (its imm32 at +86 or +83) and "ja
// rel32" to the refusal (+90 or +87, the rel32 at +92 or +89) - "mov eax, edi", "lock cmpxchg", "jne" back to the counter's load,
// eax 1; the refusal (+112 or +109): every register but rax into the record, eax 0; the pops, "ret". The frame's reset: "lea
// rax, [rip+disp32]" of the counters (+3), and a fourth lea, its first byte 48 only with "rtextra". Every site ends in CC. The edge,
// two pages: with "rtjumpunread" T-E-C2's 47 bytes end the first, so that its refusal jump's rel32 lies on the second. The twin:
// CC, but with an option "...twice" a second match of a pattern.
#define RT_PROLOGUE 0x53, 0x55, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55, 0x41, 0x56, 0x41, 0x57, 0x49, 0x89, 0xD7, 0x4D, 0x8B, 0x07, \
	0x4D, 0x8B, 0x4F, 0x08, 0x4D, 0x8B, 0x5F, 0x10, 0x4D, 0x8B, 0x67, 0x18, 0x4D, 0x8B, 0x6F, 0x20, 0x4D, 0x8B, 0x77, 0x28, 0x49, 0x89, 0x67, 0x30
#define RT_REFUSAL 0x49, 0x89, 0x5F, 0x38, 0x49, 0x89, 0x4F, 0x40, 0x49, 0x89, 0x57, 0x48, 0x49, 0x89, 0x77, 0x50, 0x49, 0x89, 0x7F, 0x58, \
	0x49, 0x89, 0x6F, 0x60, 0x4D, 0x89, 0x47, 0x68, 0x4D, 0x89, 0x4F, 0x70, 0x4D, 0x89, 0x57, 0x78, 0x4D, 0x89, 0x9F, 0x80, 0x00, 0x00, 0x00, \
	0x4D, 0x89, 0xA7, 0x88, 0x00, 0x00, 0x00, 0x4D, 0x89, 0xAF, 0x90, 0x00, 0x00, 0x00, 0x4D, 0x89, 0xB7, 0x98, 0x00, 0x00, 0x00, \
	0x4D, 0x89, 0xBF, 0xA0, 0x00, 0x00, 0x00, 0x49, 0x89, 0xA7, 0xA8, 0x00, 0x00, 0x00, 0x31, 0xC0, \
	0x41, 0x5F, 0x41, 0x5E, 0x41, 0x5D, 0x41, 0x5C, 0x5F, 0x5E, 0x5D, 0x5B, 0xC3, 0xCC
volatile unsigned char g_rtSetUp[] = { 0x48, 0x83, 0xEC, 0x38, 0xC7, 0x44, 0x24, 0x20, 0x89, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x00, 0x40, 0x00,
	0xBA, 0x14, 0x00, 0x00, 0x00, 0x41, 0xB0, 0x01, 0x45, 0x31, 0xC9, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x38,
	0xC3, 0xCC };
volatile unsigned char g_rtGeometry[] = { RT_PROLOGUE,
	0x8B, 0x2D, 0, 0, 0, 0, 0x31, 0xD2, 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0x41, 0xBA, 0x01, 0x00, 0x00, 0x00,		// +42: T-E-C1
	0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00,					// +63: 15 bytes of filler
	0x8B, 0x3C, 0xAB, 0x8D, 0x34, 0x0F, 0x81, 0xFE, 0x00, 0x00, 0x40, 0x00, 0x0F, 0x87, 0x10, 0x00, 0x00, 0x00,		// +78: the check
	0x89, 0xF8, 0xF0, 0x0F, 0xB1, 0x34, 0xAB, 0x75, 0xE5, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xEB, 0x50,				// +96: the range taken
	RT_REFUSAL };																								// +112: the refusal
volatile unsigned char g_rtRopes[] = { RT_PROLOGUE,
	0x8B, 0x15, 0, 0, 0, 0, 0x31, 0xED, 0x48, 0x8D, 0x35, 0, 0, 0, 0, 0x41, 0xBA, 0x01, 0x00, 0x00, 0x00,		// +42: T-E-C2
	0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00,									// +63: 12 bytes of filler
	0x8B, 0x3C, 0x96, 0x8D, 0x1C, 0x0F, 0x81, 0xFB, 0x00, 0x00, 0x40, 0x00, 0x0F, 0x87, 0x10, 0x00, 0x00, 0x00,		// +75: the check
	0x89, 0xF8, 0xF0, 0x0F, 0xB1, 0x1C, 0x96, 0x75, 0xE5, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xEB, 0x50,				// +93: the range taken
	RT_REFUSAL };																								// +109: the refusal
volatile unsigned char g_rtReset[] = { 0x48, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_rtExtra[] = { 0xCC, 0x8D, 0x05, 0, 0, 0, 0, 0xCC };
constexpr size_t kRtRopesMatch = 47, kRtEdgeMatchAt = 0x1000 - kRtRopesMatch;
alignas(4096) volatile unsigned char g_rtEdge[0x2000] = { 0xCC };
static Site g_rtEdgeRefs[] = { SITE(g_rtEdge, kRtEdgeMatchAt + 2), SITE(g_rtEdge, kRtEdgeMatchAt + 11) };	// T-E-C2's slot and counters there
volatile unsigned char g_rtTwin[64] = { 0xCC };
static Site g_rtCountSite = SITE(g_rtSetUp, 13);		// the buffer's count
static Site g_rtMakerCall = SITE(g_rtSetUp, 29);		// the rel32 of the maker's call
static Site g_rtObjectRef = SITE(g_rtSetUp, 36);		// the disp32 of the object's store
static Site g_rtSlotRefs[] = { SITE(g_rtGeometry, 44), SITE(g_rtRopes, 44) };
static Site g_rtCountersRefs[] = { SITE(g_rtGeometry, 53), SITE(g_rtRopes, 53), SITE(g_rtReset, 3), SITE(g_rtExtra, 3) };
static Site g_rtCapacities[] = { SITE(g_rtGeometry, 86), SITE(g_rtRopes, 83) };
static Site g_rtJumps[] = { SITE(g_rtGeometry, 92), SITE(g_rtRopes, 89) };	// the rel32 of each refusal jump
static Site g_rtSites[] = { SITE(g_rtSetUp, 0), SITE(g_rtGeometry, 0), SITE(g_rtRopes, 0), SITE(g_rtReset, 0), SITE(g_rtExtra, 0),
	{ g_rtEdge + kRtEdgeMatchAt, 0, kRtRopesMatch }, SITE(g_rtTwin, 0) };
constexpr size_t kRtSites = sizeof(g_rtSites) / sizeof(g_rtSites[0]);
// The store's init: a NOP before it, "push r14" and room for its calls, C-E-L1 with both rel32 read and set before TweaksV loads, and out
// again. C-E-D1's lea, never run. The twin: CC, but with an option "...twice" a second match of a pattern.
constexpr size_t kSbInitStart = 1, kSbMatchAt = 7, kSbLookupRel = kSbMatchAt + 9, kSbPoolRel = kSbMatchAt + 23, kSbInitMatch = 27, kSbStoreMatch = 28;
volatile unsigned char g_staticBoundsInit[] = { 0x90, 0x41, 0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x89, 0xD9, 0x89, 0xC2, 0x41, 0x89, 0xF8, 0xE8, 0, 0, 0, 0, 0x41, 0x89, 0xC6,
	0x48, 0x8D, 0x4E, 0x38, 0x89, 0xC2, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x20, 0x41, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_staticBoundsStoreLea[] = { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0xBA, 0x01, 0x00, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0xB9, 0x28, 0x3A, 0x00, 0x00, 0xCC };
volatile unsigned char g_staticBoundsTwin[64] = { 0xCC };
static Site g_staticBoundsSites[] = { SITE(g_staticBoundsInit, 0), SITE(g_staticBoundsStoreLea, 0), SITE(g_staticBoundsTwin, 0) };
// The lighting capacity's sites, in the order of the patterns, a rel32 past a match kept and CC after each, but where the fake runs one. The one a job runs,
// add-to-a-job's-list: its prologue, G-E-C1 at +11 - its jae at +17, its rel32 at +19 to the way out - "inc dword [r14+0x14]" at +33 in
// place of the game's copy of a record, the way out at +37. G-E-A1, run as a frame's reset: "ret" after it. G-E-C4, run through a way in that makes its
// stack: "movaps [rcx], xmm1", the frame undone and "ret" after it. G-E-D1 and G-E-D2, run through ways in that keep rbx: "pop rbx; ret" after the first,
// the second's call to a "ret" and "mov eax, ebx" before them. G-E-F1, as the ray traced list's add: a prologue the way out undoes, the site at +13 - its jg at +31, its rel32 at +33 to the
// way out - "inc dword [r12]" at +37 in place of the game's add, the way out at +41. G-E-K1 and G-E-K2, run through ways in that make the frame their calls want: its undoing and "ret" after
// each call. The twin: CC, but a third match of G-E-G1 with lcg1thrice.
volatile unsigned char g_lcReset[] = { 0x48, 0x8D, 0x05, 0, 0, 0, 0, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x4C, 0x8D, 0x05, 0, 0, 0, 0, 0x48, 0x8D, 0x15, 0, 0, 0, 0,
	0x48, 0x89, 0x15, 0, 0, 0, 0, 0x4C, 0x8D, 0x0D, 0, 0, 0, 0, 0xC3 };
volatile unsigned char g_lcDrainCoronas[] = { 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0x48, 0x39, 0x1D, 0, 0, 0, 0, 0x0F, 0x86, 0xCC };
volatile unsigned char g_lcDrainLights[] = { 0x48, 0x8D, 0x35, 0, 0, 0, 0, 0x48, 0x39, 0x35, 0, 0, 0, 0, 0x76, 0x00, 0x0F, 0x1F, 0x44, 0x00, 0x00, 0x48, 0x89, 0xF1,
	0x31, 0xD2, 0x45, 0x31, 0xC0, 0x45, 0x31, 0xC9, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcJobTest[] = { 0x69, 0x05, 0, 0, 0, 0, 0xCD, 0xCC, 0xCC, 0xCC, 0x05, 0x99, 0x99, 0x99, 0x19, 0x3D, 0x32, 0x33, 0x33, 0x33, 0x77, 0xCC };
volatile unsigned char g_lcKick[] = { 0xC7, 0x44, 0x1A, 0x70, 0x80, 0xB1, 0x00, 0x00, 0xF0, 0xFF, 0x05, 0xCC };
volatile unsigned char g_lcLastKick[] = { 0x42, 0xC7, 0x44, 0x0A, 0x70, 0x80, 0xB1, 0x00, 0x00, 0xCC };
volatile unsigned char g_lcAddToList[] = { 0x41, 0x57, 0x41, 0x56, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20,
	0x8B, 0x41, 0x14, 0x3B, 0x41, 0x1C, 0x0F, 0x83, 0x0E, 0x00, 0x00, 0x00, 0x48, 0x89, 0xD7, 0x49, 0x89, 0xCE, 0x48, 0x8B, 0x59, 0x08,
	0x41, 0xFF, 0x46, 0x14, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5F, 0x5E, 0x41, 0x5E, 0x41, 0x5F, 0xC3, 0xCC };
volatile unsigned char g_lcScratch[] = { 0x4C, 0x8D, 0x48, 0x0F, 0x49, 0x83, 0xE1, 0xF0, 0x4C, 0x89, 0xCA, 0x48, 0x29, 0xC2, 0x49, 0x8D, 0xA9, 0x80, 0x25, 0x00, 0x00,
	0xBB, 0x00, 0x8C, 0x00, 0x00, 0x48, 0x29, 0xD3, 0x48, 0x81, 0xC2, 0xFF, 0x73, 0xFF, 0xFF, 0x45, 0x31, 0xC0, 0x48, 0x81, 0xFA, 0x7F, 0x4E, 0xFF, 0xFF, 0xBE, 0x80, 0xB1, 0x00, 0x00, 0xCC };
volatile unsigned char g_lcScratchLights[] = { 0x48, 0x05, 0x00, 0x8C, 0x00, 0x00, 0x48, 0x81, 0xFA, 0x00, 0x8C, 0x00, 0x00, 0xCC };
volatile unsigned char g_lcStartValuesLoad[] = { 0x48, 0x89, 0x7C, 0x24, 0x68, 0x0F, 0x28, 0x0D, 0, 0, 0, 0, 0x0F, 0x29, 0x09, 0x48, 0x83, 0xC4, 0x78, 0xC3 };
volatile unsigned char g_lcStartValuesWayIn[] = { 0x48, 0x83, 0xEC, 0x78, 0xE9, 0, 0, 0, 0 };	// sub rsp, 0x78; jmp G-E-C4
volatile unsigned char g_lcSceneAdd[] = { 0x48, 0x63, 0x05, 0, 0, 0, 0, 0x48, 0x3D, 0xFF, 0x03, 0x00, 0x00, 0x0F, 0x8F, 0, 0, 0, 0, 0x48, 0x63, 0x0D, 0, 0, 0, 0, 0x48, 0x8D, 0x15, 0, 0, 0, 0,
	0x8D, 0x59, 0x01, 0x89, 0x1D, 0, 0, 0, 0, 0x88, 0x04, 0xCA, 0x5B, 0xC3 };
volatile unsigned char g_lcSceneWayIn[] = { 0x53, 0xE9, 0, 0, 0, 0 };	// push rbx; jmp G-E-D1
volatile unsigned char g_lcByteRead[] = { 0x0F, 0xB6, 0x1C, 0xC1, 0xF3, 0x0F, 0x11, 0x74, 0xC1, 0x04, 0x4A, 0x8D, 0x14, 0xC1, 0xE8, 0x04, 0, 0, 0, 0x89, 0xD8, 0x5B, 0xC3, 0xC3 };
volatile unsigned char g_lcByteWayIn[] = { 0x53, 0x48, 0x89, 0xD0, 0x49, 0x89, 0xD0, 0xE9, 0, 0, 0, 0 };	// push rbx; rax and r8 the entry, as rdx brings it; jmp G-E-D2
volatile unsigned char g_lcRender[] = { 0x48, 0x69, 0xC7, 0x00, 0x00, 0x07, 0x00, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x48, 0x01, 0xC2, 0x48, 0x89, 0x15, 0, 0, 0, 0, 0x48, 0x8D, 0x05, 0, 0, 0, 0,
	0x44, 0x8B, 0x04, 0xB8, 0x44, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x69, 0xC7, 0x0C, 0x9A, 0x02, 0x00, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x01, 0xC1, 0x48, 0x89, 0x0D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcUpdate[] = { 0x48, 0x69, 0xC8, 0x00, 0x00, 0x07, 0x00, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x48, 0x01, 0xCA, 0x48, 0x89, 0x15, 0, 0, 0, 0, 0x48, 0x8D, 0x0D, 0, 0, 0, 0,
	0xC7, 0x04, 0x81, 0x00, 0x00, 0x00, 0x00, 0x48, 0x69, 0xC8, 0x0C, 0x9A, 0x02, 0x00, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcRayLists[] = { 0x48, 0x69, 0xC8, 0x10, 0x20, 0x07, 0x00, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x48, 0x8D, 0x34, 0x0A, 0x48, 0x89, 0x35, 0, 0, 0, 0, 0x8B, 0x04, 0x11, 0xCC };
volatile unsigned char g_lcRayAdd[] = { 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x28,
	0x4C, 0x8B, 0x25, 0, 0, 0, 0, 0x45, 0x8B, 0x34, 0x24, 0x41, 0x81, 0xFE, 0xFF, 0x03, 0x00, 0x00, 0x0F, 0x8F, 0, 0, 0, 0,
	0x41, 0xFF, 0x04, 0x24, 0x48, 0x83, 0xC4, 0x28, 0x5B, 0x5F, 0x5E, 0x41, 0x5C, 0x41, 0x5E, 0x41, 0x5F, 0xC3, 0xCC };
static volatile unsigned char* const g_lcRayCheck = g_lcRayAdd + 13;	// G-E-F1, in it
volatile unsigned char g_lcUpload[] = { 0x44, 0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x81, 0xC1, 0xE0, 0x70, 0x00, 0x00, 0xBA, 0x00, 0x02, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcReflections[] = { 0x4B, 0x63, 0x0C, 0x88, 0x48, 0x81, 0xF9, 0xFF, 0x02, 0x00, 0x00, 0xCC };
volatile unsigned char g_lcCoronaCheck[] = { 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0x81, 0xF9, 0x3F, 0x04, 0x00, 0x00, 0x0F, 0x87, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcCoronaCheckToo[] = { 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0x81, 0xF9, 0x3F, 0x04, 0x00, 0x00, 0x0F, 0x87, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_ccGeneralAdd[] = { 0x66, 0x66, 0x66, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0, 0, 0, 0, 0, 0x44, 0x8B, 0x81, 0x00, 0xCC, 0x00, 0x00, 0x49, 0x81, 0xF8, 0x3F, 0x04, 0x00, 0x00, 0x0F, 0x87, 0x24, 0, 0, 0, 0x41, 0x8D, 0x70, 0x01, 0x44, 0x89, 0xC0, 0xF0, 0x0F, 0xB1, 0xB1, 0x00, 0xCC, 0x00, 0x00, 0x75, 0xDB, 0x4B, 0x8D, 0x04, 0x40, 0x48, 0xC1, 0xE0, 0x04, 0x0F, 0x28, 0x02, 0x0F, 0x29, 0x04, 0x08, 0x44, 0x89, 0x42, 0x10, 0x0F, 0x28, 0x34, 0x24, 0x0F, 0x28, 0x7C, 0x24, 0x10, 0x44, 0x0F, 0x28, 0x44, 0x24, 0x20, 0x48, 0x83, 0xC4, 0x38, 0x5F, 0x5E, 0xC3, 0xCC };	// the corona store's, G-E-I1 and what the fake runs behind it
volatile unsigned char (&g_lcCoronaAdd)[sizeof(g_ccGeneralAdd) - 14] = reinterpret_cast<volatile unsigned char (&)[sizeof(g_ccGeneralAdd) - 14]>(g_ccGeneralAdd[14]);	// G-E-G2, in it
volatile unsigned char g_lcWorkerFrame[] = { 0xB8, 0x58, 0x80, 0x01, 0x00, 0xE8, 0, 0, 0, 0, 0x48, 0x29, 0xC4, 0xCC };
volatile unsigned char g_lcWorkerCall[] = { 0x48, 0x8D, 0x44, 0x24, 0x50, 0x49, 0x89, 0x04, 0x24, 0x4C, 0x89, 0xE1, 0x41, 0xFF, 0x54, 0x24, 0x60, 0xCC };
volatile unsigned char g_lcConstructor[] = { 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0x48, 0x8D, 0x35, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcExtra[] = { 0xCC, 0x8D, 0x1D, 0, 0, 0, 0, 0xCC, 0x8D, 0x35, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_lcTwin[32] = { 0xCC };
volatile unsigned char g_lcCoronaEnd[] = { 0xF0, 0x48, 0x0F, 0xC1, 0x08, 0x48, 0x8B, 0x54, 0x24, 0x40, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x58, 0xC3 };
volatile unsigned char g_lcCoronaEndWayIn[] = { 0x48, 0x83, 0xEC, 0x58, 0x48, 0x89, 0x54, 0x24, 0x40, 0x48, 0x89, 0xC8, 0x4C, 0x89, 0xC1, 0xE9, 0, 0, 0, 0 };	// the frame; [rsp+0x40] the records, rdx brings them; rax the pointer's address, rcx brings it; rcx the bytes, r8 brings them; jmp G-E-K1
volatile unsigned char g_lcLightEnd[] = { 0x4C, 0x69, 0xC6, 0xC0, 0x01, 0x00, 0x00, 0x4C, 0x89, 0xC1, 0xF0, 0x48, 0x0F, 0xC1, 0x08, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x20, 0x5E, 0xC3 };
volatile unsigned char g_lcLightEndWayIn[] = { 0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x89, 0xC8, 0x4C, 0x89, 0xC6, 0xE9, 0, 0, 0, 0 };	// rsi kept, the frame; rax the pointer's address, rcx brings it; rsi the lights, r8 brings them; jmp G-E-K2
static Site g_lcSites[] = { SITE(g_lcCoronaEnd, 0), SITE(g_lcLightEnd, 0), SITE(g_lcReset, 0), SITE(g_lcDrainCoronas, 0), SITE(g_lcDrainLights, 0), SITE(g_lcJobTest, 0), SITE(g_lcKick, 0), SITE(g_lcLastKick, 0),
	SITE(g_lcAddToList, 0), SITE(g_lcScratch, 0), SITE(g_lcScratchLights, 0), SITE(g_lcStartValuesLoad, 0), SITE(g_lcSceneAdd, 0), SITE(g_lcByteRead, 0), SITE(g_lcRender, 0),
	SITE(g_lcUpdate, 0), SITE(g_lcRayLists, 0), SITE(g_lcRayAdd, 0), SITE(g_lcUpload, 0), SITE(g_lcReflections, 0), SITE(g_lcCoronaCheck, 0), SITE(g_lcCoronaCheckToo, 0),
	SITE(g_lcCoronaAdd, 0), SITE(g_lcWorkerFrame, 0), SITE(g_lcWorkerCall, 0), SITE(g_lcConstructor, 0), SITE(g_lcExtra, 0), SITE(g_lcTwin, 0) };
constexpr size_t kLcSites = sizeof(g_lcSites) / sizeof(g_lcSites[0]);
// The measurement's one write: the rel32 of the jump when a job's list is full, of the jump when the ray traced list is full and of a job's two copies.
static Site g_lcJumpRel = SITE(g_lcAddToList, 19);
static const Site kLcRayJumpRel = SITE(g_lcRayAdd, 33), kLcCoronaCallRel = SITE(g_lcCoronaEnd, 11), kLcLightCallRel = SITE(g_lcLightEnd, 16);
// What the part writes (its findings' 8.1), worked out here and not by TweaksV: the five rel32 it leads to its own memory, the stores' in the reset first, then the drain's, then
// the start values'; and the bytes: a job's scratch of 0x17340 = 300 x 0x30 + 180 x 0x1C0 in its kicks and laid out, the index's two instructions.
static const Site kLcLeads[] = { SITE(g_lcReset, 3), SITE(g_lcReset, 24), SITE(g_lcDrainCoronas, 3), SITE(g_lcDrainLights, 3), SITE(g_lcStartValuesLoad, 8) };
constexpr size_t kLcLeadCount = sizeof(kLcLeads) / sizeof(kLcLeads[0]);
struct LcRaisedBytes { Site site; size_t size; unsigned char bytes[4]; };
static const LcRaisedBytes kLcRaisedBytes[] = {
	{ SITE(g_lcKick, 4), 4, { 0x40, 0x73, 0x01, 0x00 } }, { SITE(g_lcLastKick, 5), 4, { 0x40, 0x73, 0x01, 0x00 } },
	{ SITE(g_lcScratch, 17), 4, { 0x40, 0x38, 0x00, 0x00 } },	// 0x3840, the coronas
	{ SITE(g_lcScratch, 22), 4, { 0x00, 0x3B, 0x01, 0x00 } },	// 0x13B00, the lights
	{ SITE(g_lcScratch, 32), 4, { 0xFF, 0xC4, 0xFE, 0xFF } },	// -0x13B01
	{ SITE(g_lcScratch, 42), 4, { 0xBF, 0x8C, 0xFE, 0xFF } },	// -0x17341
	{ SITE(g_lcScratch, 47), 4, { 0x40, 0x73, 0x01, 0x00 } },
	{ SITE(g_lcScratchLights, 2), 4, { 0x00, 0x3B, 0x01, 0x00 } }, { SITE(g_lcScratchLights, 9), 4, { 0x00, 0x3B, 0x01, 0x00 } },
	{ SITE(g_lcSceneAdd, 42), 1, { 0x89 } },					// mov [rdx+rcx*8], eax
	{ SITE(g_lcByteRead, 0), 4, { 0x8B, 0x1C, 0xC1, 0x90 } },	// mov ebx, [rcx+rax*8]; nop
};
// The two model budgets (Population), none of it run: the crash context's read of the ambient cars, O-E-V5, whose last two bytes begin
// its line, O-E-C1; the two getters; the check before a vehicle model's request, O-E-V1 and O-E-V2; the release's test of VehicleStruct,
// O-E-V3; the tables' set-up's call, O-E-V6; the ped memory's sum, O-E-P1; the gate, O-E-P2, the rest of whose bytes PopSetUp places; the
// ped lists, O-E-P3; a model's peds, O-E-P4; the variety's store, O-E-P8. Then what the options use: two stores and a call that are none
// until their first byte is set, and a place for a match moved. Last, for popscanmiss, the tables' set-up, a NOP before it, its head O-E-V4
// behind three pushes and sub rsp, 0x20, and twelve blocks of 0x38 bytes: story peds, O-E-P5 to P7, network peds, story vehicles, O-E-V7,
// network vehicles.
#define POP_GETTER 0x8B, 0x05, 0, 0, 0, 0, 0x80, 0x3D, 0, 0, 0, 0, 0x00, 0x74, 0x11, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x8B, 0x04, 0x81, 0xF3, 0x48, 0x0F, 0x2A, 0xC0, \
	0xEB, 0x17, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x8B, 0x04, 0x81, 0xF3, 0x48, 0x0F, 0x2A, 0xC0, 0xF3, 0x0F, 0x59, 0x05, 0, 0, 0, 0, 0xF3, 0x0F, 0x2C, 0xC0, 0x01, 0xC0, 0xC3, 0xCC
#define POP_BLOCK(low, high, n0, n1, n2, n3) 0x69, 0xBB, low, high, 0x00, 0x00, 0xE8, 0x03, 0x00, 0x00, 0xB9, 0xBD, 0xC5, 0xAF, 0xE3, 0xBA, n0, n1, n2, n3, 0xE8, 0, 0, 0, 0, \
	0x48, 0x89, 0xF1, 0x89, 0xC2, 0x41, 0x89, 0xF8, 0xE8, 0, 0, 0, 0, 0x05, 0x60, 0xE3, 0x16, 0x00, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x35, 0, 0, 0, 0
volatile unsigned char g_popContext[] = { 0x44, 0x8B, 0x35, 0, 0, 0, 0, 0x8B, 0x05, 0, 0, 0, 0, 0x89, 0x84, 0x24, 0, 0, 0, 0, 0x44, 0x8B, 0x3D, 0, 0, 0, 0,
	0x8B, 0x35, 0, 0, 0, 0, 0xC1, 0xFE, 0x14, 0x8B, 0x3D, 0, 0, 0, 0, 0xC1, 0xFF, 0x14, 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0x48, 0x89, 0xD9, 0xE8, 0, 0, 0, 0,	// +27: O-E-C1
	0x89, 0xC5, 0xC1, 0xFD, 0x14, 0x48, 0x89, 0xD9, 0xE8, 0, 0, 0, 0, 0x89, 0xC3, 0xC1, 0xFB, 0x14, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popPedGetter[] = { POP_GETTER };
volatile unsigned char g_popVehicleGetter[] = { POP_GETTER };
volatile unsigned char g_popVehicleTables[] = { 0x8B, 0x05, 0, 0, 0, 0, 0x0F, 0xB6, 0x0D, 0, 0, 0, 0, 0x4C, 0x8D, 0x35, 0, 0, 0, 0, 0x48, 0x8D, 0x1D, 0, 0, 0, 0, 0xEB, 0x00, 0xCC };
volatile unsigned char g_popVehicleCheck[] = { 0x41, 0x8B, 0x14, 0x96, 0x0F, 0x57, 0xC0, 0xF3, 0x48, 0x0F, 0x2A, 0xC2, 0xF3, 0x0F, 0x59, 0x05, 0, 0, 0, 0, 0xF3, 0x0F, 0x2C, 0xD0,
	0x81, 0xC2, 0xA0, 0x1C, 0xE9, 0xFF, 0x39, 0x97, 0x14, 0x62, 0x00, 0x00, 0x7D, 0x05, 0x48, 0x89, 0xF9, 0xE8, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popVehiclePool[] = { 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x8B, 0x68, 0x28, 0xC1, 0xE5, 0x02, 0xC1, 0xFD, 0x02, 0x2B, 0x68, 0x18, 0x83, 0xC5, 0x14, 0x85, 0xED,
	0x0F, 0x8E, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popSetUpCall[] = { 0x41, 0x0F, 0xB7, 0xC6, 0x89, 0x34, 0x87, 0xE8, 0, 0, 0, 0, 0x0F, 0x28, 0x74, 0x24, 0x40, 0x48, 0x83, 0xC4, 0x58, 0xC3, 0xCC };
volatile unsigned char g_popPedSum[] = { 0x8B, 0x86, 0x10, 0x62, 0x00, 0x00, 0x03, 0x86, 0x0C, 0x62, 0x00, 0x00, 0x03, 0x86, 0x2C, 0x62, 0x00, 0x00, 0x03, 0x86, 0x30, 0x62, 0x00, 0x00,
	0x89, 0x05, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popGate[0x108] = { 0x41, 0x8B, 0x8C, 0x24, 0x08, 0x62, 0x00, 0x00, 0x85, 0xC9, 0x79, 0x08, 0x41, 0x8B, 0x8C, 0x24, 0x04, 0x62, 0x00, 0x00,
	0x41, 0x8B, 0xAC, 0x24, 0x0C, 0x62, 0x00, 0x00, 0xD1, 0xF9, 0x69, 0xD9, 0xA0, 0x1C, 0xE9, 0xFF, 0x81, 0xC3, 0xA0, 0x1C, 0xE9, 0xFF, 0x8B, 0x0D };
constexpr size_t kPopGateList = 0x7A, kPopGateFloor = 0xEA, kPopGateCall = 0x102, kPopMatchP2 = 44;
volatile unsigned char g_popPedLists[] = { 0x48, 0x8D, 0x82, 0x04, 0x4E, 0x00, 0x00, 0x4C, 0x8D, 0x8A, 0x04, 0x56, 0x00, 0x00, 0x4C, 0x8D, 0x82, 0x04, 0x52, 0x00, 0x00,
	0x48, 0x81, 0xC2, 0x04, 0x5E, 0x00, 0x00, 0x48, 0x89, 0x44, 0x24, 0x20, 0x48, 0x8D, 0x4C, 0x24, 0x70, 0xE8, 0, 0, 0, 0, 0x0F, 0xB7, 0x05, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popPedsOfModel[] = { 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0x98, 0x48, 0x0F, 0xAF, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x04, 0x01, 0x8B, 0x80, 0x58, 0x01, 0x00, 0x00,
	0x42, 0x89, 0x44, 0xBC, 0x40, 0xCC };
volatile unsigned char g_popVariety[] = { 0xF2, 0x0F, 0x10, 0x86, 0xB4, 0x00, 0x00, 0x00, 0x0F, 0x59, 0x05, 0, 0, 0, 0, 0x0F, 0x58, 0x05, 0, 0, 0, 0, 0xF3, 0x0F, 0x10, 0x0D, 0, 0, 0, 0,
	0x0F, 0x28, 0xD0, 0x0F, 0xC2, 0x15, 0, 0, 0, 0, 0x01, 0x66, 0x0F, 0x38, 0x25, 0xD2, 0x66, 0x0F, 0x50, 0xC2, 0xF3, 0x0F, 0x10, 0x15, 0, 0, 0, 0, 0x0F, 0x28, 0xDA, 0xA8, 0x01,
	0x75, 0x07, 0x0F, 0x28, 0xD9, 0xF3, 0x0F, 0x5D, 0xD8, 0xF3, 0x0F, 0x11, 0x1D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popExtraStores[] = { 0xCC, 0x05, 0, 0, 0, 0, 0xCC, 0x05, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popExtraCall[] = { 0xCC, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_popTwin[256] = { 0xCC };
volatile unsigned char g_popSetUp[] = { 0x90, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x58, 0x18, 0x48, 0x8B, 0x35, 0, 0, 0, 0,	// +1, +8: O-E-V4
	POP_BLOCK(0xF4, 0x04, 0x40, 0x67, 0xD1, 0x1F), POP_BLOCK(0xF8, 0x04, 0xD0, 0x3C, 0x99, 0x0A), POP_BLOCK(0xFC, 0x04, 0x5C, 0x10, 0x5F, 0xF4),	// +26
	POP_BLOCK(0x00, 0x05, 0x11, 0x11, 0x11, 0x11), POP_BLOCK(0x04, 0x05, 0x22, 0x22, 0x22, 0x22), POP_BLOCK(0x08, 0x05, 0x33, 0x33, 0x33, 0x33),
	POP_BLOCK(0x0C, 0x05, 0xD8, 0xC1, 0xCB, 0x18), POP_BLOCK(0x10, 0x05, 0x8A, 0x9B, 0xA4, 0x05), POP_BLOCK(0x14, 0x05, 0x01, 0xFF, 0x5F, 0xF7),
	POP_BLOCK(0x18, 0x05, 0x44, 0x44, 0x44, 0x44), POP_BLOCK(0x1C, 0x05, 0x55, 0x55, 0x55, 0x55), POP_BLOCK(0x20, 0x05, 0x66, 0x66, 0x66, 0x66),
	0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5F, 0x5E, 0xC3, 0xCC };
constexpr size_t kPopStart = 1, kPopHead = 8, kPopBlocks = 26, kPopBlockBytes = 0x38, kPopStoreInBlock = 45, kPopMatchV7 = 157, kPopMatchV5 = 29;
static Site g_popSites[] = { SITE(g_popContext, 0), SITE(g_popPedGetter, 0), SITE(g_popVehicleGetter, 0), SITE(g_popVehicleTables, 0), SITE(g_popVehicleCheck, 0), SITE(g_popVehiclePool, 0),
	SITE(g_popSetUp, 0), SITE(g_popSetUpCall, 0), SITE(g_popPedSum, 0), SITE(g_popGate, 0), SITE(g_popPedLists, 0), SITE(g_popPedsOfModel, 0), SITE(g_popVariety, 0),
	SITE(g_popExtraStores, 0), SITE(g_popExtraCall, 0), SITE(g_popTwin, 0) };
constexpr size_t kPopSites = sizeof(g_popSites) / sizeof(g_popSites[0]);
// The ped pool's sites as the game holds them, N-E-A1 to N-E-D6. The accessor has a tail, mov rax, rdx; ret, so that the fake runs it over its own encoding.
volatile unsigned char g_ppAccessor[] = { 0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0xA8, 0x01, 0x75, 0x03, 0x31, 0xC0, 0xC3, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1E, 0x48, 0x8B, 0x15, 0, 0, 0, 0,
	0x48, 0x31, 0xCA, 0x48, 0xC1, 0xC2, 0x20, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x02, 0x48, 0xD3, 0xC2, 0x48, 0xF7, 0xD2, 0x48, 0x89, 0xD0, 0xC3 };
volatile unsigned char g_ppMaker[] = { 0x48, 0x8D, 0x05, 0, 0, 0, 0, 0x48, 0x89, 0x44, 0x24, 0x28, 0xC7, 0x44, 0x24, 0x20, 0x80, 0x19, 0x00, 0x00 };
volatile unsigned char g_ppPoolLookup[] = { 0xBA, 0x17, 0x21, 0xA1, 0x8D, 0x41, 0x89, 0xD8, 0xE8, 0, 0, 0, 0, 0x89, 0xC3 };
volatile unsigned char g_ppFileValue[] = { 0xBA, 0x17, 0x21, 0xA1, 0x8D, 0x41, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x89, 0xC1, 0xE8, 0, 0, 0, 0 };
volatile unsigned char g_ppSetter[] = { 0x89, 0x0D, 0, 0, 0, 0, 0xC3 };
volatile unsigned char g_ppIntelligence[] = { 0xBA, 0x84, 0xC5, 0x4A, 0x39, 0x41, 0x89, 0xD8, 0xE8, 0, 0, 0, 0, 0x89, 0xC3 };
volatile unsigned char g_ppSpatial[70] = { 0x48, 0x63, 0x72, 0x18, 0x48, 0x83, 0xC6, 0x03, 0x48, 0x83, 0xE6, 0xFC };
volatile unsigned char g_ppCounters[] = { 0x8B, 0x05, 0, 0, 0, 0, 0x8B, 0x0D, 0, 0, 0, 0, 0x8B, 0x35, 0, 0, 0, 0, 0x8B, 0x3D, 0, 0, 0, 0, 0x44, 0x89, 0x74, 0x24, 0x50 };
volatile unsigned char g_ppCaps[] = { 0xF3, 0x0F, 0x2C, 0xC2, 0x89, 0x05, 0, 0, 0, 0, 0xF3, 0x0F, 0x59, 0xC1, 0xF3, 0x0F, 0x2C, 0xC0, 0x89, 0x05, 0, 0, 0, 0, 0x66, 0x0F, 0x7E, 0x3D, 0, 0, 0, 0 };
volatile unsigned char g_ppCopy[] = { 0x8B, 0x3A, 0xE8, 0, 0, 0, 0, 0x39, 0xC7, 0x7D, 0x04, 0x8B, 0x03, 0xEB, 0x05, 0xE8, 0, 0, 0, 0, 0x89, 0x46, 0x10, 0x8B, 0x03, 0x31, 0xFF, 0x85, 0xC0, 0x0F, 0x4F, 0xF8, 0x89, 0x7E, 0x10 };
volatile unsigned char g_ppReset[] = { 0x66, 0xC7, 0x05, 0, 0, 0, 0, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x44, 0x0F, 0xB7, 0x05 };
volatile unsigned char g_ppFiller[] = { 0x53, 0x55, 0x56, 0x57, 0x90, 0x48, 0x81, 0xEC, 0x28, 0x0C, 0x00, 0x00, 0x48, 0x8D, 0x8C, 0x24, 0x20, 0x08, 0x00, 0x00, 0x48, 0x8D, 0x94, 0x24, 0x20, 0x04, 0x00, 0x00,
	0x4C, 0x8D, 0x44, 0x24, 0x20, 0xE8, 0, 0, 0, 0 };
volatile unsigned char g_ppPriority[0x30F] = { 0xB8, 0x48, 0x10, 0x00, 0x00, 0xE8, 0, 0, 0, 0, 0x48, 0x29, 0xC4, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x63, 0x70, 0x5C };
// Two pages, CC: with an option "...unread" a pattern's match ends the first, so that what its check reads past the match lies on the second.
alignas(4096) volatile unsigned char g_ppEdge[0x2000] = { 0xCC };
volatile unsigned char g_ppDirector[] = { 0xBA, 0x10, 0xD8, 0x5E, 0xCA, 0x41, 0xB8, 0x00, 0x04, 0x00, 0x00, 0xE8 };
volatile unsigned char g_ppCreature[] = { 0xBA, 0xCA, 0x6C, 0x77, 0x36, 0x41, 0xB8, 0x00, 0x04, 0x00, 0x00, 0xE8 };
volatile unsigned char g_ppMotionTree[] = { 0xBA, 0x7A, 0xEE, 0xAE, 0xCF, 0x41, 0xB8, 0x55, 0x09, 0x00, 0x00, 0xE8 };
volatile unsigned char g_ppExpressions[] = { 0xBA, 0x46, 0x99, 0xC5, 0x3C, 0x41, 0xB8, 0xAA, 0x02, 0x00, 0x00, 0xE8 };
volatile unsigned char g_ppRagDoll[] = { 0xBA, 0x29, 0xBB, 0x69, 0xBF, 0x41, 0xB8, 0x55, 0x01, 0x00, 0x00, 0xE8 };
volatile unsigned char g_ppFacialRig[] = { 0xBA, 0xBE, 0x10, 0x0E, 0x21, 0x41, 0xB8, 0x55, 0x01, 0x00, 0x00, 0xE8 };
// Two pages, CC: with "pmcallunread" H-E2's match ends the first, so that its call's rel32 lies on the second.
alignas(4096) volatile unsigned char g_poolMemoryEdge[0x2000] = { 0xCC };
// The pool memory budget's set-up in the game's start-up code, run as the game runs it, with room for its call around H-E2's bytes: sub rsp, 28h;
// lea rcx, [the allocator]; lea rdx, [the heap]; mov r8d, 0x10000; mov r9d, 136 MB; call its constructor; add rsp, 28h; ret.
constexpr size_t kPoolMemoryMatch = 4, kPoolMemoryAllocatorAt = kPoolMemoryMatch + 3, kPoolMemoryHeapAt = kPoolMemoryMatch + 10, kPoolMemoryBudgetAt = kPoolMemoryMatch + 22,
	kPoolMemoryCallAt = kPoolMemoryMatch + 27;
volatile unsigned char g_poolMemorySite[] = { 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x41, 0xB8, 0x00, 0x00, 0x01, 0x00,
	0x41, 0xB9, 0x00, 0x00, 0x80, 0x08, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0xC3 };
// The render queues' sites (TweaksVRenderQueues), each with its pattern's bytes as PATTERNS.md has them and around them what makes it
// run as the fake's code: a prologue, a tail, and the rel32 RqSetUp sets. The make of the vehicles' render pool lies in a carrier,
// kRqMakeV2 below, since its jump must lie in a function of the fake's unwind data; g_rqEdge's second page cannot be read with
// "rqbad:edge...", a section of its own, since MakeSitesReadExecute gives .fakesit its protection back after RqSetUp.
#define RQ_INIT_HEAD 0xB9, 0x05, 0, 0, 0, 0x31, 0xD2, 0xE8, 0, 0, 0, 0, 0xB9, 0x05, 0, 0, 0, 0x31, 0xD2, 0xE8, 0, 0, 0, 0
#define RQ_INIT_BLOCK 0xB9, 0xC0, 0x03, 0, 0, 0xBA, 0x08, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x66, 0xC7, 0x05, 0, 0, 0, 0, 0x78, 0x00
#define RQ_LOOKUP(hash0, hash1, hash2, hash3, value) 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0xBA, hash0, hash1, hash2, hash3, 0x41, 0xB8, value, 0, 0, 0, 0xE8, 0, 0, 0, 0, \
	0x48, 0x83, 0xC4, 0x28, 0xC3, 0xCC
#define RQ_VEHICLE_LOOKUP(hash0, hash1, hash2, hash3) 0x53, 0x48, 0x83, 0xEC, 0x20, 0x89, 0xCB, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0xBA, hash0, hash1, hash2, hash3, 0x41, 0x89, 0xD8, 0xE8, 0, 0, 0, 0, \
	0x48, 0x83, 0xC4, 0x20, 0x5B, 0xC3, 0xCC
#define RQ_FLIP(...) 0x48, 0x83, 0xEC, 0x28, __VA_ARGS__, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x8B, 0x05, 0, 0, 0, 0, 0x8D, 0x48, 0x01, 0x8D, 0x50, 0x04, 0x85, 0xC9, 0x0F, 0x49, 0xD1, \
	0x83, 0xE2, 0xFC, 0xF7, 0xDA, 0x44, 0x8D, 0x04, 0x10, 0x41, 0xFF, 0xC0, 0x44, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0xB9, 0, 0, 0, 0, 0xE9, 0, 0, 0, 0, 0xCC
volatile unsigned char g_rqMakeS1[] = { 0x55, 0x48, 0x83, 0xEC, 0x20,
	0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0xC1, 0xC0, 0x1B, 0x48, 0x8B, 0x15, 0, 0, 0, 0, 0x48, 0x31, 0xC2, 0x48, 0xC1, 0xC2, 0x20, 0x24, 0x1F, 0x8D, 0x48, 0x01,
	0x48, 0x89, 0xD5, 0x48, 0xD3, 0xE5, 0x34, 0x3F, 0x89, 0xC1, 0x48, 0xD3, 0xEA, 0x48, 0x09, 0xEA, 0x48, 0xF7, 0xD2, 0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0xA8, 0x01, 0xB9, 0, 0, 0, 0,
	0x48, 0x0F, 0x45, 0xCA, 0xBA, 0x08, 0x05, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x20, 0x5D, 0xC3, 0xCC };
constexpr size_t kRqTrapAt = 7 + 111;	// in g_rqMakeS2: where the full pool's jump leads, the trap
volatile unsigned char g_rqMakeS2[] = { 0x48, 0x83, 0xEC, 0x28, 0x48, 0x89, 0xCA,
	0x8B, 0x42, 0x28, 0xC1, 0xE0, 0x02, 0xC1, 0xF8, 0x02, 0x39, 0x42, 0x18, 0x7E, 0x61, 0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1C, 0x48, 0x8B, 0x05, 0, 0, 0, 0,
	0x48, 0x31, 0xC8, 0x48, 0xC1, 0xC0, 0x20, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x03, 0x48, 0xD3, 0xC0, 0x48, 0xF7, 0xD0, 0x0F, 0xB6, 0x15, 0, 0, 0, 0, 0x31, 0xC9, 0xF6, 0xC2, 0x01,
	0x48, 0x0F, 0x45, 0xC8, 0xBA, 0xE0, 0x02, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0xC3,
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
volatile unsigned char g_rqMakeP1[] = { 0x41, 0x55, 0x41, 0x57, 0x48, 0x83, 0xEC, 0x28,
	0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1B, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x31, 0xC8, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x02, 0x48, 0xD3, 0xC0,
	0x48, 0xC1, 0xC0, 0x20, 0x48, 0xF7, 0xD0, 0x0F, 0xB6, 0x0D, 0, 0, 0, 0, 0x45, 0x31, 0xFF, 0xF6, 0xC1, 0x01, 0x49, 0x0F, 0x44, 0xC7, 0xBA, 0x80, 0, 0, 0, 0x48, 0x89, 0xC1, 0xE8, 0, 0, 0, 0,
	0x49, 0x89, 0xC5, 0xC7, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x66, 0xC7, 0x40, 0x04, 0x00, 0x7F, 0x4C, 0x89, 0xE8, 0x48, 0x83, 0xC4, 0x28, 0x41, 0x5F, 0x41, 0x5D, 0xC3, 0xCC };
volatile unsigned char g_rqMakeP2[] = { 0x57, 0x48, 0x83, 0xEC, 0x20, 0x31, 0xFF,
	0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1D, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x31, 0xC8, 0x48, 0xC1, 0xC0, 0x20, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x05,
	0x48, 0xD3, 0xC0, 0x48, 0xF7, 0xD0, 0x0F, 0xB6, 0x0D, 0, 0, 0, 0, 0xF6, 0xC1, 0x01, 0x48, 0x0F, 0x44, 0xC7, 0xBA, 0x3C, 0, 0, 0, 0x48, 0x89, 0xC1, 0xE8, 0, 0, 0, 0,
	0x48, 0x83, 0xC4, 0x20, 0x5F, 0xC3, 0xCC };
volatile unsigned char g_rqMakeV1[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x28,
	0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1D, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x31, 0xC8, 0x48, 0xC1, 0xC0, 0x20, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x05,
	0x48, 0xD3, 0xC0, 0x48, 0xF7, 0xD0, 0x0F, 0xB6, 0x15, 0, 0, 0, 0, 0x31, 0xC9, 0xF6, 0xC2, 0x01, 0x48, 0x0F, 0x45, 0xC8, 0xBA, 0xC8, 0x01, 0, 0, 0xE8, 0, 0, 0, 0,
	0x48, 0x89, 0xC6, 0x48, 0x89, 0xC7, 0x48, 0x8D, 0x05, 0, 0, 0, 0, 0x48, 0x89, 0xF0, 0x48, 0x83, 0xC4, 0x28, 0x5F, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_rqVehicleSpare[220] = { 0xCC };	// "rqbad:v2nounwind": the make of the vehicles' render pool where no unwind data names a function
volatile unsigned char g_rqAddPed[] = { 0x41, 0x56, 0x55, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x49, 0x89, 0xCE,
	0x66, 0x41, 0xFF, 0x86, 0xD8, 0x02, 0, 0, 0x48, 0x63, 0x05, 0, 0, 0, 0, 0x48, 0xC1, 0xE0, 0x04, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x14, 0x08, 0x0F, 0xB7, 0x6C, 0x08, 0x08, 0x8D, 0x5D, 0x01,
	0x66, 0x89, 0x5C, 0x08, 0x08, 0x4C, 0x89, 0x34, 0xEA, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5D, 0x41, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_rqAddProp[] = { 0x41, 0x57, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x49, 0x89, 0xCF,
	0x66, 0x41, 0xFF, 0x47, 0x38, 0x48, 0x63, 0x05, 0, 0, 0, 0, 0x48, 0xC1, 0xE0, 0x04, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8B, 0x14, 0x08, 0x0F, 0xB7, 0x5C, 0x08, 0x08, 0x8D, 0x7B, 0x01,
	0x66, 0x89, 0x7C, 0x08, 0x08, 0x4C, 0x89, 0x3C, 0xDA, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5F, 0x41, 0x5F, 0xC3, 0xCC };
volatile unsigned char g_rqAddVehicle[] = { 0x41, 0x54, 0x41, 0x56, 0x53, 0x48, 0x83, 0xEC, 0x20,
	0x49, 0x89, 0xCC, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x63, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xE1, 0x04, 0x4C, 0x8D, 0x35, 0, 0, 0, 0, 0x42, 0x0F, 0xB7, 0x44, 0x31, 0x08, 0x48, 0x85, 0xC0,
	0x66, 0x41, 0xFF, 0x84, 0x24, 0xA8, 0x0B, 0, 0, 0x48, 0x63, 0x05, 0, 0, 0, 0, 0x48, 0xC1, 0xE0, 0x04, 0x4A, 0x8B, 0x0C, 0x30, 0x42, 0x0F, 0xB7, 0x54, 0x30, 0x08, 0x8D, 0x5A, 0x01,
	0x66, 0x42, 0x89, 0x5C, 0x30, 0x08, 0x4C, 0x89, 0x24, 0xD1, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x41, 0x5E, 0x41, 0x5C, 0xC3, 0xCC };
volatile unsigned char g_rqInitPed[] = { 0x48, 0x83, 0xEC, 0x28, RQ_INIT_HEAD, RQ_INIT_BLOCK, RQ_INIT_BLOCK, RQ_INIT_BLOCK, RQ_INIT_BLOCK, 0x80, 0x3D, 0, 0, 0, 0, 0x00, 0x48, 0x83, 0xC4, 0x28, 0xC3, 0xCC };
volatile unsigned char g_rqInitProp[] = { 0x48, 0x83, 0xEC, 0x28, RQ_INIT_HEAD, RQ_INIT_BLOCK, RQ_INIT_BLOCK, RQ_INIT_BLOCK, RQ_INIT_BLOCK, 0x48, 0x83, 0xC4, 0x28, 0xC3, 0xCC };
volatile unsigned char g_rqSizes[] = { 0x48, 0x83, 0xEC, 0x28, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0xBA, 0xF4, 0xBF, 0xD4, 0x90, 0x41, 0xB8, 0x01, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x89, 0x05, 0, 0, 0, 0,
	0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0xBA, 0x67, 0x52, 0x1D, 0xE1, 0x41, 0xB8, 0x01, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0xC3, 0xCC };
volatile unsigned char g_rqInitVehicle[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x63, 0x35, 0, 0, 0, 0, 0x48, 0x85, 0xF6, 0x0F, 0x84, 0x27, 0, 0, 0, 0x48, 0x8D, 0x0C, 0xF5, 0, 0, 0, 0,
	0xBA, 0x08, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x63, 0x3D, 0, 0, 0, 0, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x66, 0x89, 0x35, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0x5F, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_rqLookupN1[] = { RQ_LOOKUP(0xF6, 0x68, 0xB9, 0x0A, 0x37) };
volatile unsigned char g_rqLookupN2[] = { RQ_LOOKUP(0x34, 0x0C, 0x37, 0x2D, 0x78) };
volatile unsigned char g_rqLookupN3[] = { RQ_LOOKUP(0x2D, 0x46, 0xFB, 0x85, 0x32) };
volatile unsigned char g_rqLookupN4[] = { RQ_LOOKUP(0xB8, 0x8E, 0x9C, 0x53, 0x78) };
volatile unsigned char g_rqLookupN5[] = { RQ_VEHICLE_LOOKUP(0x86, 0x3C, 0x75, 0x54) };
volatile unsigned char g_rqLookupN6[] = { RQ_VEHICLE_LOOKUP(0x89, 0xF0, 0xB5, 0xC2) };
volatile unsigned char g_rqFlipsCall[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x89, 0xD6, 0x48, 0x89, 0xCF, 0xE8, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0xF9, 0x89, 0xF2,
	0x48, 0x83, 0xC4, 0x28, 0x5F, 0x5E, 0xE9, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_rqFlipPeds[] = { RQ_FLIP(0x90, 0x90) };
volatile unsigned char g_rqFlipProps[] = { RQ_FLIP(0x90, 0x90, 0x90) };
volatile unsigned char g_rqFlipVehicles[] = { RQ_FLIP(0x90, 0x90) };
volatile unsigned char g_rqReferences[] = { 0x56, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x89, 0xCE, 0x48, 0x89, 0xD0,
	0x48, 0x89, 0x86, 0xD0, 0x05, 0, 0, 0x66, 0xC7, 0x86, 0xDA, 0x05, 0, 0, 0x80, 0x28, 0xB9, 0x00, 0x88, 0x02, 0, 0xBA, 0x08, 0, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x20, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_rqRotation[] = { 0x8B, 0x81, 0xA4, 0, 0, 0, 0x8B, 0x91, 0xAC, 0, 0, 0, 0xFF, 0xC0, 0x83, 0xE0, 0x03, 0x89, 0x81, 0xA4, 0, 0, 0, 0xFF, 0xC2, 0x83, 0xE2, 0x03, 0x89, 0x91, 0xAC, 0, 0, 0, 0xC3, 0xCC };
volatile unsigned char g_rqManagerMade[] = { 0x56, 0x48, 0x83, 0xEC, 0x20, 0xB9, 0xA0, 0x4B, 0x01, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0xC6, 0x48, 0x89, 0xC1, 0xE8, 0, 0, 0, 0, 0x48, 0x89, 0x35, 0, 0, 0, 0,
	0x48, 0x83, 0xC4, 0x20, 0x5E, 0xC3, 0xCC };
volatile unsigned char g_rqNotData[0x40] = { 0xCC };		// inside the game and readable, and not its .data
static Site g_rqSites[] = { SITE(g_rqMakeS1, 0), SITE(g_rqMakeS2, 0), SITE(g_rqMakeP1, 0), SITE(g_rqMakeP2, 0), SITE(g_rqMakeV1, 0), SITE(g_rqVehicleSpare, 0), SITE(g_rqAddPed, 0), SITE(g_rqAddProp, 0),
	SITE(g_rqAddVehicle, 0), SITE(g_rqInitPed, 0), SITE(g_rqInitProp, 0), SITE(g_rqSizes, 0), SITE(g_rqInitVehicle, 0), SITE(g_rqLookupN1, 0), SITE(g_rqLookupN2, 0), SITE(g_rqLookupN3, 0), SITE(g_rqLookupN4, 0),
	SITE(g_rqLookupN5, 0), SITE(g_rqLookupN6, 0), SITE(g_rqFlipsCall, 0), SITE(g_rqFlipPeds, 0), SITE(g_rqFlipProps, 0), SITE(g_rqFlipVehicles, 0), SITE(g_rqReferences, 0), SITE(g_rqRotation, 0),
	SITE(g_rqManagerMade, 0), SITE(g_rqNotData, 0) };
#pragma section(".rqedge", read, execute)
__declspec(allocate(".rqedge")) volatile unsigned char g_rqEdge[0x2000] = { 0xCC };
// [siren damage]
// The sirens per vehicle's damage: the loops, the pool's set-up and the caller as code, written by SdSetUp; the stored damage state's loop, which stays at 20;
// room where an option moves a site, far from every other site.
volatile unsigned char g_sdHit[0xB0] = { 0xCC };
volatile unsigned char g_sdImpact[0x80] = { 0xCC };
volatile unsigned char g_sdHide[0x48] = { 0xCC };
volatile unsigned char g_sdWreck[0x40] = { 0xCC };
volatile unsigned char g_sdFlags[0x60] = { 0xCC };
volatile unsigned char g_sdClasses[3][0x60] = { { 0xCC }, { 0xCC }, { 0xCC } };
volatile unsigned char g_sdPoolSite[0x20] = { 0xCC };
volatile unsigned char g_sdCaller[0x28] = { 0xCC };
volatile unsigned char g_sdKept[0x09] = { 0xCC };
volatile unsigned char g_sdSync[0x28] = { 0xCC };
volatile unsigned char g_sdSpare[0x800] = { 0xCC };
static Site g_sdSites[] = { SITE(g_sdHit, 0), SITE(g_sdImpact, 0), SITE(g_sdHide, 0), SITE(g_sdWreck, 0), SITE(g_sdFlags, 0), SITE(g_sdClasses[0], 0), SITE(g_sdClasses[1], 0),
	SITE(g_sdClasses[2], 0), SITE(g_sdPoolSite, 0), SITE(g_sdCaller, 0), SITE(g_sdKept, 0), SITE(g_sdSync, 0), SITE(g_sdSpare, 0) };
// [corona store]
// The corona store's sites. g_ccGeneralAdd, above: G-E-I1, the record's write with the game's movaps, the index it got at [rdx+0x10], the way out at +70; g_ccAddStart its
// prologue, as the way out wants it, and on to it. The inlined adds' stores, never run, and where their jumps lead. The copy: G-E-I4 at kCcI4, G-E-I5 at kCcI5, the call
// to CcCopyBytes, NOPs as far as the game's jle and jz reach. The job's parameters, G-E-I6, and what hands the store out, G-E-I7. The batch, G-E-I8, the placed count at
// [r8] and the store's offset in rax. The job's own limit, G-E-I9, way out at +52. The rest never run.
volatile unsigned char g_ccAddStart[] = { 0x56, 0x57, 0x48, 0x83, 0xEC, 0x38, 0x0F, 0x29, 0x34, 0x24, 0x0F, 0x29, 0x7C, 0x24, 0x10, 0x44, 0x0F, 0x29, 0x44, 0x24, 0x20, 0xE9, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_ccFirstStore[] = { 0x48, 0x8D, 0x04, 0x49, 0x48, 0xC1, 0xE0, 0x04, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x41, 0x0F, 0x28, 0x1E, 0x0F, 0x29, 0x1C, 0x08, 0xCC };
volatile unsigned char g_ccSecondStore[] = { 0x48, 0x8D, 0x04, 0x49, 0x48, 0xC1, 0xE0, 0x04, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x0F, 0x28, 0x44, 0x24, 0x30, 0x0F, 0x29, 0x04, 0x08, 0xCC };
volatile unsigned char g_ccFirstWayOut[] = { 0x0F, 0x28, 0x74, 0x24, 0x20, 0x0F, 0x28, 0x7C, 0x24, 0x30, 0x44, 0x0F, 0x28, 0x44, 0x24, 0x40, 0x44, 0x0F, 0x28, 0x4C, 0x24, 0x50, 0x48, 0x83, 0xC4, 0x68, 0xCC };
volatile unsigned char g_ccSecondWayOut[] = { 0x42, 0xC6, 0x44, 0x2E, 0x6B, 0x00, 0x66, 0x90, 0x48, 0xFF, 0xC3, 0x41, 0x8B, 0x47, 0x04, 0x48, 0x39, 0xC3, 0xCC };
constexpr int kCcAddWayOut = 70, kCcI4 = 12, kCcI5 = 41, kCcI6 = 4, kCcI8 = 17, kCcI9 = 20, kCcLodWayOut = 52;
volatile unsigned char g_ccCopy[] = { 0x57, 0x53, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x89, 0xD7, 0x49, 0x89, 0xC9,
	0x8B, 0x05, 0, 0, 0, 0, 0x89, 0x47, 0x10, 0x85, 0xC0, 0x7E, 0x5C, 0x8D, 0x04, 0x40, 0xC1, 0xE0, 0x04, 0x05, 0x80, 0x01, 0x00, 0x00, 0x89, 0xC3, 0x4C, 0x89, 0xC8,	// +12: G-E-I4; ebx the bytes, rax where they go
	0x48, 0x85, 0xC0, 0x74, 0x1E, 0x48, 0x8D, 0x15, 0, 0, 0, 0, 0x48, 0x89, 0xC1, 0x49, 0x89, 0xD8, 0xE8, 0, 0, 0, 0,	// +41: G-E-I5, its call
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,	// to +76, where the jz leads
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,	// to +117, where the jle leads
	0x48, 0x83, 0xC4, 0x28, 0x5B, 0x5F, 0xC3, 0xCC };
volatile unsigned char g_ccParameters[] = { 0x48, 0x83, 0xEC, 0x68, 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x48, 0x8D, 0x54, 0x24, 0x50, 0x4C, 0x8D, 0x44, 0x24, 0x40, 0xE8, 0, 0, 0, 0, 0x48, 0x8B, 0x44, 0x24, 0x50, 0x48, 0x89, 0x05, 0, 0, 0, 0,
	0x48, 0x8B, 0x44, 0x24, 0x40, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x68, 0xC3, 0xCC };
volatile unsigned char g_ccHandOut[] = { 0x48, 0x89, 0x0A, 0x48, 0x81, 0xC1, 0x00, 0xCC, 0x00, 0x00, 0x49, 0x89, 0x08, 0xC3, 0xCC };
volatile unsigned char g_ccBatch[] = { 0x55, 0x53, 0x56, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x89, 0x4C, 0x24, 0x20, 0x48, 0x89, 0x54, 0x24, 0x28,
	0xBD, 0x40, 0x04, 0x00, 0x00, 0x48, 0x8B, 0x9C, 0x24, 0x20, 0, 0, 0, 0x48, 0x8B, 0xB4, 0x24, 0x28, 0, 0, 0, 0x66, 0x66, 0x66, 0x66, 0x2E, 0x0F, 0x1F, 0x84, 0, 0, 0, 0, 0,	// +17: G-E-I8
	0x8B, 0x0B, 0x8D, 0x14, 0x31, 0x81, 0xFA, 0x40, 0x04, 0x00, 0x00, 0x0F, 0x43, 0xD5, 0x89, 0xC8, 0xF0, 0x0F, 0xB1, 0x13, 0x75, 0xEA, 0x29, 0xCA, 0x48, 0x8D, 0x04, 0x49,
	0x48, 0xC1, 0xE0, 0x04, 0x41, 0x89, 0x10, 0x48, 0x83, 0xC4, 0x30, 0x5E, 0x5B, 0x5D, 0xC3, 0xCC };
volatile unsigned char g_ccLodLimit[] = { 0x48, 0x81, 0xEC, 0xB8, 0, 0, 0, 0x44, 0x0F, 0x29, 0x5C, 0x24, 0x60, 0x89, 0x8C, 0x24, 0x90, 0, 0, 0,
	0x81, 0xBC, 0x24, 0x90, 0, 0, 0, 0x3F, 0x04, 0, 0, 0x0F, 0x87, 0, 0, 0, 0, 0x48, 0x8B, 0x84, 0x24, 0x88, 0, 0, 0, 0xB8, 0x01, 0, 0, 0, 0xEB, 0x19,	// +20: G-E-I9
	0x4C, 0x8B, 0x44, 0x24, 0x58, 0x44, 0x0F, 0x28, 0xDC, 0x0F, 0x28, 0xA4, 0x24, 0xA0, 0, 0, 0, 0x41, 0x0F, 0x50, 0xC1, 0x84, 0xC0, 0x31, 0xC0,	// +52: the way out
	0x44, 0x0F, 0x28, 0x5C, 0x24, 0x60, 0x48, 0x81, 0xC4, 0xB8, 0, 0, 0, 0xC3, 0xCC };
volatile unsigned char g_ccScratch[] = { 0x49, 0x8D, 0x92, 0x00, 0xCC, 0x00, 0x00, 0xBD, 0x00, 0x1C, 0x00, 0x00, 0x4C, 0x29, 0xDD, 0x49, 0x81, 0xC3, 0xFF, 0xE3, 0xFF, 0xFF, 0x49, 0x81, 0xFB, 0xFF, 0x17, 0xFF, 0xFF, 0x48, 0x0F, 0x42, 0xD0, 0x41, 0xBD, 0x00, 0xE8, 0x00, 0x00, 0xCC };
volatile unsigned char g_ccCommand[] = { 0x48, 0x8B, 0x41, 0x08, 0x48, 0x89, 0x05, 0, 0, 0, 0, 0x8B, 0x51, 0x10, 0x89, 0x15, 0, 0, 0, 0, 0x31, 0xD2, 0x48, 0x85, 0xC0, 0x0F, 0x94, 0xC2, 0xC1, 0xE2, 0x03, 0x89, 0x15, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_ccDraw[] = { 0xF0, 0xFF, 0x05, 0, 0, 0, 0, 0x89, 0xD6, 0x49, 0x89, 0xCC, 0x48, 0x63, 0x3D, 0, 0, 0, 0, 0xCC };
volatile unsigned char g_ccStackRecords[] = { 0x48, 0x89, 0xF8, 0x48, 0xC1, 0xE0, 0x05, 0xE8, 0, 0, 0, 0, 0x48, 0x29, 0xC4, 0x49, 0x89, 0xE6, 0x48, 0x83, 0xEC, 0x40, 0xCC };
volatile unsigned char g_ccStackIndices[] = { 0x48, 0x8D, 0x04, 0xBD, 0x0F, 0x00, 0x00, 0x00, 0x48, 0x83, 0xE0, 0xF0, 0xE8, 0, 0, 0, 0, 0x48, 0x29, 0xC4, 0x49, 0x89, 0xE5, 0xCC };
static Site g_ccSites[] = { SITE(g_ccAddStart, 0), SITE(g_ccFirstStore, 0), SITE(g_ccSecondStore, 0), SITE(g_ccFirstWayOut, 0), SITE(g_ccSecondWayOut, 0), SITE(g_ccCopy, 0), SITE(g_ccParameters, 0), SITE(g_ccHandOut, 0),
	SITE(g_ccBatch, 0), SITE(g_ccLodLimit, 0), SITE(g_ccScratch, 0), SITE(g_ccCommand, 0), SITE(g_ccDraw, 0), SITE(g_ccStackRecords, 0), SITE(g_ccStackIndices, 0) };
#pragma data_seg()

// The game's own modkit array (1024 entries of 2 bytes) and packfile array (4500 entries of 8 bytes). The
// harness reaches them only through these pointers, set when the program is loaded: none of its own code
// may load an array with a lea, which TweaksV's scan for every reference to the array would count.
static uint16_t g_oldModkitArrayStorage[1024];
static uint16_t* volatile g_oldModkitArray = g_oldModkitArrayStorage;
static void* g_oldPackfileArrayStorage[4500];
static void** volatile g_oldPackfileArray = g_oldPackfileArrayStorage;

// The game's own traversal list (1000 entries of 8 bytes). The harness reaches it only through this
// pointer, set when the program is loaded: none of its own code may load the list with a lea, which
// TweaksV's scan for the game's loads of the list would count.
static uint64_t g_oldBoxListStorage[1000];
static uint64_t* volatile g_oldBoxList = g_oldBoxListStorage;

// The game's own decal definitions array (260 entries of 80 bytes) with its count 16 bytes before it. The
// harness reaches it only through this pointer, set when the program is loaded: none of its own code may
// load the array with a lea, which TweaksV's scan for the game's loads of it would count.
struct DecalStore { uint32_t count; uint32_t unused[3]; unsigned char entries[260 * 80]; };
alignas(32) static DecalStore g_oldDecalStoreStorage;
static DecalStore* volatile g_oldDecalStore = &g_oldDecalStoreStorage;

// The game's blip pool, 2000 slots of 8 bytes - a blip's pointer, or 0 - and its high-water mark, a dword. The harness
// reaches them only through these pointers, set when the program is loaded, as it does the arrays above.
static uint64_t g_blipPoolStorage[2000];
static uint64_t* volatile g_blipPool = g_blipPoolStorage;
static int32_t g_blipMarkStorage = 0;
static int32_t* volatile g_blipMark = &g_blipMarkStorage;

// The parts of the parts' coding round, RageOpenV's check for packs made for GTA V Legacy and the limits measured first:
// the game's data their sites reference and what stands in for the game's code they call, one block each. The harness
// reaches the data only through pointers, as it does the arrays above.
// The weapon info manager's array of blobs - the block's pointer, the count at +8, the capacity at +0xA - in the fake's
// data, as the game's is in its own: zero while TweaksV starts. The block the fake's initializer allocates, the sites'
// bytes as laid before TweaksV starts, TweaksV.log as the fake reads it to wait for TweaksV's lines, and the files it
// loads through the loader: a data file entry of the game's list as the loader gets it - 0xA8 bytes, the file's name at
// +0, its type at +0x90, 0x4E for a weapons.meta file - and how each load went.
struct WeaponInfosArray { void* block; uint16_t count; uint16_t capacity; uint32_t unused; };
static_assert(offsetof(WeaponInfosArray, count) == 8 && offsetof(WeaponInfosArray, capacity) == 0xA, "the game's array layout");
static WeaponInfosArray g_weaponInfosArrayStorage;
static WeaponInfosArray* volatile g_weaponInfosArray = &g_weaponInfosArrayStorage;
static void* g_weaponInfosBlock = nullptr;
static unsigned char g_weaponInfosLaid[sizeof(g_weaponInfosLea) + sizeof(g_weaponInfosInit) + sizeof(g_weaponInfosAllocation) +
	sizeof(g_weaponInfosLoader) + sizeof(g_weaponInfosExtra) + sizeof(g_weaponInfosOtherHook)];
static char g_weaponInfosLog[4 << 20];
struct WeaponInfosEntry { char name[0x90]; uint32_t type; int32_t next; unsigned char rest[0x10]; };
static_assert(sizeof(WeaponInfosEntry) == 0xA8 && offsetof(WeaponInfosEntry, type) == 0x90, "the game's data file entry");
static unsigned g_weaponInfosCalls = 0, g_weaponInfosPassed = 0, g_weaponInfosRefused = 0, g_weaponInfosAsExpected = 0;
static bool HasOption(int argc, wchar_t** argv, const wchar_t* option);

// The capacity's store: its disp32 counts from the instruction's end, the imm16 after it
static void WeaponInfosPointCapacityAt(const void* target)
{
	const int32_t rel = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(g_weaponInfosInit + 36));
	memcpy(const_cast<unsigned char*>(g_weaponInfosInit + 30), &rel, sizeof(rel));
}

// The sites' bytes as laid, kept before TweaksV starts. The part may write the two immediates and the loader's first 5
// bytes, where its hook's jump goes, and nothing else: the report gives the immediates as they are, the loader's start
// as laid, hooked - a jump in its place - or changed otherwise, and whether every other byte is as laid.
static void WeaponInfosKeepSites()
{
	size_t at = 0;
	for (const Site& site : g_weaponInfosSites)
		for (size_t i = 0; i < site.size; i++)
			g_weaponInfosLaid[at++] = site.bytes[i];
}
static bool WeaponInfosPartsByte(const Site& site, size_t i)
{
	return (site.bytes == g_weaponInfosInit && ((i >= 11 && i < 15) || (i >= 34 && i < 36))) || (site.bytes == g_weaponInfosLoader && i >= 1 && i < 6);
}
static bool WeaponInfosRestAsLaid()
{
	size_t at = 0;
	bool same = true;
	for (const Site& site : g_weaponInfosSites)
		for (size_t i = 0; i < site.size; i++, at++)
			same = same && (WeaponInfosPartsByte(site, i) || g_weaponInfosLaid[at] == site.bytes[i]);
	return same;
}
static const wchar_t* WeaponInfosLoaderState()
{
	const size_t at = sizeof(g_weaponInfosLea) + sizeof(g_weaponInfosInit) + sizeof(g_weaponInfosAllocation);	// the loader's copy
	bool same = true;
	for (size_t i = 1; i < 6; i++)
		same = same && g_weaponInfosLaid[at + i] == g_weaponInfosLoader[i];
	return same ? L"as laid" : g_weaponInfosLoader[1] == 0xE9 ? L"hooked" : L"CHANGED";
}
static unsigned WeaponInfosLimit(size_t at, size_t size)
{
	uint32_t value = 0;
	memcpy(&value, const_cast<const unsigned char*>(g_weaponInfosInit + at), size);
	return value;
}

// The game's initializer of the array, as the fake runs it: a block on its heap of as many blobs of 248 bytes as the
// site's "mov edx" says, the block into the array and the room the site's capacity says - or room for raisedTo, as if
// another mod had raised both immediates ("wiraised") - then the loader's count of the files loaded.
static void WeaponInfosInitialize(unsigned raisedTo, unsigned loaded)
{
	uint32_t number = raisedTo;
	uint16_t capacity = static_cast<uint16_t>(raisedTo);
	if (raisedTo == 0)
	{
		memcpy(&number, const_cast<const unsigned char*>(g_weaponInfosInit + 11), sizeof(number));
		memcpy(&capacity, const_cast<const unsigned char*>(g_weaponInfosInit + 34), sizeof(capacity));
	}
	g_weaponInfosBlock = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, static_cast<SIZE_T>(number) * 248 + 1);
	g_weaponInfosArray->block = g_weaponInfosBlock;
	g_weaponInfosArray->capacity = capacity;
	g_weaponInfosArray->count = static_cast<uint16_t>(loaded);
}

// How often TweaksV.log, read whole and shared as TweaksV shares it while it writes, holds text.
static int WeaponInfosLogHas(const char* text)
{
	const HANDLE file = CreateFileW(L"TweaksV.log", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	DWORD read = 0;
	const BOOL ok = ReadFile(file, g_weaponInfosLog, static_cast<DWORD>(sizeof(g_weaponInfosLog) - 1), &read, nullptr);
	CloseHandle(file);
	if (!ok)
		return 0;
	g_weaponInfosLog[read] = '\0';
	int found = 0;
	for (const char* at = strstr(g_weaponInfosLog, text); at; at = strstr(at + 1, text))
		found++;
	return found;
}

// Waits, 100 ms at a time and ms at most, until TweaksV.log holds text n times.
static bool WeaponInfosAwait(const char* text, int n, DWORD ms)
{
	for (DWORD waited = 0; WeaponInfosLogHas(text) < n; waited += 100)
	{
		if (waited >= ms)
			return false;
		Sleep(100);
	}
	return true;
}

// "wipacks": packs mounted while the game runs, as the loader counts their files - once the first summary is logged, 8
// more, the array full; once the second is, 5 more, past its end, where the game's loader would write without a check
// (the fake writes nothing there); then 2.5 s of nothing new.
static void WeaponInfosMountPacks()
{
	if (!WeaponInfosAwait(" weapons.meta files loaded (", 1, 20000))
		return;
	g_weaponInfosArray->count = 160;
	if (!WeaponInfosAwait(" weapons.meta files loaded (", 2, 10000))
		return;
	g_weaponInfosArray->count = 165;
	if (WeaponInfosAwait(" weapons.meta files loaded (", 3, 10000))
		Sleep(2500);
}

// A file through the loader's start, as the game hands its loader one - to TweaksV's guard once it has hooked the start,
// which passes the file on or refuses it. Passed, the loader counted it and returned 1; refused, it was not counted and 0
// came back; either way with the last error the fake set before the call.
static void WeaponInfosLoad(const WeaponInfosEntry* entry)
{
	using Loader = uintptr_t (*)(const void* entry, void* second, void* third, void* fourth);
	const auto loader = reinterpret_cast<Loader>(const_cast<unsigned char*>(g_weaponInfosLoader + 1));
	const unsigned before = g_weaponInfosArray->count;
	SetLastError(0x5A5A);
	const uintptr_t result = loader(entry, nullptr, nullptr, nullptr);
	const DWORD error = GetLastError();
	const unsigned after = g_weaponInfosArray->count;
	const bool passed = after == before + 1;
	g_weaponInfosCalls++;
	(passed ? g_weaponInfosPassed : g_weaponInfosRefused)++;
	if (result == (passed ? 1u : 0u) && error == 0x5A5A)
		g_weaponInfosAsExpected++;
}
static void WeaponInfosLoadNamed(const char* name)
{
	static WeaponInfosEntry entry;
	memset(&entry, 0, sizeof(entry));
	strncpy_s(entry.name, name, _TRUNCATE);
	entry.type = 0x4E;
	entry.next = -1;
	WeaponInfosLoad(&entry);
}

// "wifew": 14 files as packs bring them - the 9th's name 140 characters long, the 10th's with a byte that is no printable
// ASCII; with "wiunreadable" the 9th's name 64 bytes with no end before a page that cannot be read.
static void WeaponInfosLoadFew(bool unreadable)
{
	char name[sizeof(WeaponInfosEntry::name)];
	for (int i = 1; i <= 14; i++)
	{
		if (i == 9 && unreadable)
		{
			unsigned char* const pages = static_cast<unsigned char*>(VirtualAlloc(nullptr, 0x2000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
			DWORD old = 0;
			if (!pages || !VirtualProtect(pages + 0x1000, 0x1000, PAGE_NOACCESS, &old))
				continue;
			memset(pages + 0x1000 - 64, 'u', 64);
			WeaponInfosLoad(reinterpret_cast<const WeaponInfosEntry*>(pages + 0x1000 - 64));
			continue;
		}
		if (i == 9)
		{
			const int length = sprintf_s(name, "dlc_wi09:/common/data/ai/");
			memset(name + length, 'x', 115);
			name[length + 115] = '\0';
		}
		else if (i == 10)
			strcpy_s(name, "dlc_wi10:/common/data/ai/weap\xE9ons.meta");
		else
			sprintf_s(name, "dlc_wi%02d:/common/data/ai/weapons.meta", i);
		WeaponInfosLoadNamed(name);
	}
}

// The files the options ask for, through the loader - "wifill"'s 872, then "wifew"'s 14, then "wiflood"'s 20,001 - and,
// when the guard refused any, the wait for its report, 10 seconds after the first refusal, and for the line of the start-up
// summary's module after it; then with "wilater" 3 files more and the wait for the guard's next line.
static void WeaponInfosLoadAsAsked(int argc, wchar_t** argv)
{
	char name[sizeof(WeaponInfosEntry::name)];
	if (HasOption(argc, argv, L"wifill"))
		for (int i = 1; i <= 872; i++)
		{
			sprintf_s(name, "dlc_fill%03d:/common/data/ai/weapons.meta", i);
			WeaponInfosLoadNamed(name);
		}
	if (HasOption(argc, argv, L"wifew"))
		WeaponInfosLoadFew(HasOption(argc, argv, L"wiunreadable"));
	if (HasOption(argc, argv, L"wiflood"))
		for (int i = 1; i <= 20001; i++)
		{
			sprintf_s(name, "dlc_flood%05d:/common/data/ai/weapons.meta", i);
			WeaponInfosLoadNamed(name);
		}
	if (g_weaponInfosRefused == 0 || !WeaponInfosAwait(" not loaded: the game's room for weapons.meta files, ", 1, 20000) ||
		!WeaponInfosAwait(" finding(s)", 1, 5000) || !HasOption(argc, argv, L"wilater"))
		return;
	for (int i = 1; i <= 3; i++)
	{
		sprintf_s(name, "dlc_late%d:/common/data/ai/weapons.meta", i);
		WeaponInfosLoadNamed(name);
	}
	if (WeaponInfosAwait(" weapons.meta files in all were not loaded this session: ", 1, 5000))
		Sleep(2500);
}

// "wiunwind" and "wiunwindin": a function of the fake's own with a frame, so that its unwind data names it, and the loader
// copied into it from its start - at the function's start, or 8 bytes into it - its three rel32 leading to the array again;
// false, with nothing copied, where the function has not the room.
static void SetRel32(const Site& site, const void* target);
static volatile int g_weaponInfosSink = 0;
static __declspec(noinline) void WeaponInfosLoaderCarrier()
{
	volatile int frame[32];
	for (int i = 0; i < 32; i++)
		frame[i] = g_weaponInfosSink + 5 * i;
	for (int i = 1; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0x4E4E);
	for (int i = 2; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 2] & 0xF8);
	g_weaponInfosSink = frame[7] + frame[17] + frame[27];
}
static bool WeaponInfosCopyIntoCarrier(size_t at)
{
	unsigned char* const carrier = reinterpret_cast<unsigned char*>(&WeaponInfosLoaderCarrier);
	const size_t size = sizeof(g_weaponInfosLoader) - 1;	// from the loader's start, without the NOP before it
	DWORD64 base = 0;
	const PRUNTIME_FUNCTION entry = RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(carrier), &base, nullptr);
	DWORD old = 0, unused = 0;
	if (!entry || base + entry->BeginAddress != reinterpret_cast<DWORD64>(carrier) || entry->EndAddress - entry->BeginAddress < at + size ||
		!VirtualProtect(carrier, at + size, PAGE_EXECUTE_READWRITE, &old))
		return false;
	memset(carrier, 0xCC, at);
	for (size_t i = 0; i < size; i++)
		carrier[at + i] = g_weaponInfosLoader[1 + i];
	const unsigned char* const array = reinterpret_cast<const unsigned char*>(g_weaponInfosArray);
	SetRel32({ carrier, static_cast<int>(at + 27), size }, array + 8);	// the count's read and write, the block's read
	SetRel32({ carrier, static_cast<int>(at + 37), size }, array + 8);
	SetRel32({ carrier, static_cast<int>(at + 51), size }, array);
	VirtualProtect(carrier, at + size, old, &unused);
	FlushInstructionCache(GetCurrentProcess(), carrier, at + size);
	return true;
}

// The game's weapon component list, 470 entries of 8 bytes - a definition's pointer each - with its count, an int32, right
// after it, then the array of the files' blobs and their count, a uint16, as the game lays them out; what the game's
// configuration pointer leads to; and the game's pool size lookup behind the pool's size call, which gives a pool the size
// the configuration names for it or - as the game's own configuration does for this pool - the default, counts its calls and
// those with the arguments the game's call hands it, and leaves a last error of its own. The harness keeps what it placed, the store and
// the sites' bytes - all but the rel32 of the list's 17 loads and of the size call, which the part writes - and the store as
// TweaksV finds it, whose entries the moved list must hold, to tell afterwards what TweaksV wrote.
struct WeaponComponentsStore { uint64_t list[470]; int32_t count; uint32_t unused; uint64_t blobs; uint16_t files; uint16_t capacity; };
static_assert(offsetof(WeaponComponentsStore, count) == 470 * 8, "the list's count lies right after it");
alignas(4096) static unsigned char g_weaponComponentsStorePage[4096];	// the store alone on its page, for "wccountnoaccess" - reached through
// g_weaponComponentsStore alone: a lea of the page in the fake's code would be an 18th load of the list
static_assert(sizeof(WeaponComponentsStore) <= 4096, "the store fits its page");
static WeaponComponentsStore* volatile g_weaponComponentsStore = reinterpret_cast<WeaponComponentsStore*>(g_weaponComponentsStorePage);
static WeaponComponentsStore g_weaponComponentsPlacedStore;
static WeaponComponentsStore g_weaponComponentsAtStart;
static unsigned char g_weaponComponentsPlacedSites[512];
static uint64_t g_weaponComponentsConfigurationStorage[4];
static void* volatile g_weaponComponentsConfiguration = g_weaponComponentsConfigurationStorage;
static int32_t g_weaponComponentsGameDefault = 470;		// the pool's default at its site (0 with "wcdefault")
static int32_t g_weaponComponentsConfigured = -1;		// the pool's size the configuration names: none, as the game's own
static volatile LONG g_weaponComponentsLookups = 0;		// the lookup's calls, and those with the arguments the game's call hands it
static volatile LONG g_weaponComponentsLookupsAsCalled = 0;
constexpr DWORD kWeaponComponentsLookupError = 0x5B5;	// the lookup's last error, which the call must hand back
static int32_t PoolSizeLookup(const void* configuration, uint32_t hash, int32_t defaultSize)
{
	InterlockedIncrement(&g_weaponComponentsLookups);
	if (configuration == g_weaponComponentsConfiguration && hash == 0xBAE33CCF && defaultSize == g_weaponComponentsGameDefault)
		InterlockedIncrement(&g_weaponComponentsLookupsAsCalled);
	SetLastError(kWeaponComponentsLookupError);
	return configuration && hash == 0xBAE33CCF && g_weaponComponentsConfigured >= 0 ? g_weaponComponentsConfigured : defaultSize;
}

// The blob array, 256 at most, and the files' blobs of 0xED8 bytes on the heap, their definitions' count at +0xEC0
static unsigned char* g_weaponComponentsBlobs[256];
static unsigned char* WeaponComponentsBlob(int definitions)
{
	const auto blob = static_cast<unsigned char*>(calloc(1, 0xED8));
	if (blob)
		memcpy(blob + 0xEC0, &definitions, sizeof(definitions));
	return blob;
}

// The list as the game leaves it after a rebuild: count definitions from files files - their pointers, as on the heap, and the
// files' blobs, as many definitions each, the first the rest - the rest of it empty; kept as placed
static void WeaponComponentsLoad(int count, int files)
{
	WeaponComponentsStore* const store = g_weaponComponentsStore;
	for (int i = 0; i < 470; i++)
		store->list[i] = i < count ? 0x000001D4A3C00000ull + 0xC0ull * static_cast<uint64_t>(i) : 0;
	for (int i = 0; i < files && i < 256; i++)
		g_weaponComponentsBlobs[i] = WeaponComponentsBlob((count < 470 ? count : 470) / files + (i == 0 ? (count < 470 ? count : 470) % files : 0));
	store->count = count;
	store->unused = 0;
	store->blobs = reinterpret_cast<uint64_t>(g_weaponComponentsBlobs);
	store->files = static_cast<uint16_t>(files);
	store->capacity = 256;
	g_weaponComponentsPlacedStore = *store;
}

// The loader's callees and its body: the blob allocated and zeroed as the game's allocator and memset would; then the file
// "parsed" - its definitions, which the fake keeps at +0xA0 of its entry, its blob's count - the blob appended, the list's
// count the sum of the blobs', as the rebuild leaves it, and the callback destroyed as the game's loader does
struct WeaponComponentsEntry { char name[0x90]; int32_t type; int32_t index; unsigned char flags[8]; int32_t definitions; int32_t unused; };
static_assert(sizeof(WeaponComponentsEntry) == 0xA8 && offsetof(WeaponComponentsEntry, definitions) == 0xA0, "a data file entry");
struct WeaponComponentsCallback { const void* const* vtable; unsigned char rest[0x30]; void* callable; };
static_assert(offsetof(WeaponComponentsCallback, callable) == 0x38, "the callable's pointer at +0x38");
static int g_weaponComponentsMounted = 0, g_weaponComponentsLoaded = 0, g_weaponComponentsDestroyed = 0, g_weaponComponentsHeapDestroys = 0;
static int g_weaponComponentsErrorKept = 0;	// the files let in that reached the loader, and the refused ones that came back, with the game's last error as it left it
static int g_weaponComponentsCallablesLeft = 0;		// the callbacks that came back with their callable's pointer still there
static bool g_weaponComponentsStoreLocked = false;	// wcnoarray: the store's page cannot be read until the loader's first call
static void WeaponComponentsDestroy(void* callable, bool heap)
{
	g_weaponComponentsDestroyed++;
	g_weaponComponentsHeapDestroys += heap ? 1 : 0;
	if (heap)
		free(callable);
}
static const void* const g_weaponComponentsCallbackVtable[5] = { nullptr, nullptr, nullptr, nullptr, reinterpret_cast<const void*>(&WeaponComponentsDestroy) };
static void* WeaponComponentsAllocate(size_t size)
{
	g_weaponComponentsErrorKept += GetLastError() == 0x5EED ? 1 : 0;	// the loader's first call: a file let in, its last error as the mounter left it
	if (g_weaponComponentsStoreLocked)	// what the game's loader goes on to read
	{
		DWORD old = 0;
		VirtualProtect(const_cast<WeaponComponentsStore*>(g_weaponComponentsStore), sizeof(g_weaponComponentsStorePage), PAGE_READWRITE, &old);
		g_weaponComponentsStoreLocked = false;
	}
	return malloc(size);
}
static void* WeaponComponentsZero(void* p, int value, size_t size)
{
	return memset(p, value, size);
}
static void WeaponComponentsLoadFile(void* entry, void* callback, unsigned char* blob)
{
	WeaponComponentsStore* const store = g_weaponComponentsStore;
	memcpy(blob + 0xEC0, &static_cast<WeaponComponentsEntry*>(entry)->definitions, sizeof(int32_t));
	if (store->files < store->capacity)
		g_weaponComponentsBlobs[store->files++] = blob;
	int32_t sum = 0;
	for (int i = 0; i < store->files; i++)
	{
		int32_t definitions = 0;
		memcpy(&definitions, g_weaponComponentsBlobs[i] + 0xEC0, sizeof(definitions));
		sum += definitions;
	}
	store->count = sum;
	g_weaponComponentsLoaded++;
	const auto argument = static_cast<WeaponComponentsCallback*>(callback);
	if (argument->callable)
	{
		const void* const* const vtable = *static_cast<const void* const* const*>(argument->callable);
		reinterpret_cast<void (*)(void*, bool)>(const_cast<void*>(vtable[4]))(argument->callable, argument->callable != argument);
		argument->callable = nullptr;
	}
	g_weaponComponentsPlacedStore = *store;
}

// A file mounted as the game's mounter does: its entry - the name, type 0x4F, its definitions - and a callback passed by value,
// its callable stored inline, through the loader's head, as the options ask; the entries kept for the session
static WeaponComponentsEntry g_weaponComponentsEntries[1200];
static int32_t g_weaponComponentsEntryType = 0x4F;
static bool g_weaponComponentsCallableOnHeap = false;
static const void* const* g_weaponComponentsVtable = g_weaponComponentsCallbackVtable;
static void WeaponComponentsMount(const char* name, int definitions)
{
	if (g_weaponComponentsMounted >= 1200)
		return;
	WeaponComponentsEntry& entry = g_weaponComponentsEntries[g_weaponComponentsMounted++];
	memset(&entry, 0, sizeof(entry));
	strncpy_s(entry.name, name, _TRUNCATE);
	entry.type = g_weaponComponentsEntryType;
	entry.index = g_weaponComponentsMounted;
	entry.definitions = definitions;
	WeaponComponentsCallback callback{};
	callback.vtable = g_weaponComponentsVtable;
	callback.callable = &callback;
	if (g_weaponComponentsCallableOnHeap)
	{
		const auto callable = static_cast<const void* const**>(malloc(sizeof(void*)));
		if (!callable)
			return;
		*callable = g_weaponComponentsVtable;
		callback.callable = callable;
	}
	const int loaded = g_weaponComponentsLoaded;
	SetLastError(0x5EED);	// the game's last error, which a refused file must find as it left it
	reinterpret_cast<uint64_t (*)(void*, void*)>(const_cast<unsigned char*>(g_weaponComponentsLoader + 1))(&entry, &callback);
	if (g_weaponComponentsLoaded == loaded)
		g_weaponComponentsErrorKept += GetLastError() == 0x5EED ? 1 : 0;
	g_weaponComponentsCallablesLeft += callback.callable ? 1 : 0;
}

// wcbadblob, wcbadcount, wcnopointers, wcnoblob and wcnoarray: the blobs as the options damage them, once TweaksV has started
static void WeaponComponentsDamageBlobs(int argc, wchar_t** argv)
{
	WeaponComponentsStore* const store = g_weaponComponentsStore;
	if (HasOption(argc, argv, L"wcbadblob"))
	{
		const int32_t past = 500;
		memcpy(g_weaponComponentsBlobs[0] + 0xEC0, &past, sizeof(past));
	}
	if (HasOption(argc, argv, L"wcbadcount"))
		store->capacity = 10;
	if (HasOption(argc, argv, L"wcnopointers"))
		store->blobs = reinterpret_cast<uint64_t>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS));
	if (HasOption(argc, argv, L"wcnoblob"))	// in the copy TweaksV reads alone: the fake's own body goes on with its array
	{
		static unsigned char* asRead[256];
		memcpy(asRead, g_weaponComponentsBlobs, sizeof(asRead));
		asRead[5] = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS));
		store->blobs = reinterpret_cast<uint64_t>(asRead);
	}
	if (HasOption(argc, argv, L"wcnoarray"))
	{
		DWORD old = 0;
		g_weaponComponentsStoreLocked = VirtualProtect(store, sizeof(g_weaponComponentsStorePage), PAGE_NOACCESS, &old) != 0;
	}
}

// wcheap, wcvtableout, wcdestroyout and wcbadtype: what the mounter hands the loader, as the options ask
static void WeaponComponentsMounterAsAsked(int argc, wchar_t** argv)
{
	g_weaponComponentsCallableOnHeap = HasOption(argc, argv, L"wcheap");
	if (HasOption(argc, argv, L"wcbadtype"))
		g_weaponComponentsEntryType = 0x4E;
	if (HasOption(argc, argv, L"wcvtableout"))
	{
		const auto outside = static_cast<const void**>(malloc(sizeof(g_weaponComponentsCallbackVtable)));
		if (outside)
		{
			memcpy(outside, g_weaponComponentsCallbackVtable, sizeof(g_weaponComponentsCallbackVtable));
			g_weaponComponentsVtable = outside;
		}
	}
	if (HasOption(argc, argv, L"wcdestroyout"))
	{
		static const void* inImage[5];
		const auto stub = static_cast<unsigned char*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (stub)
		{
			stub[0] = 0xC3;
			inImage[4] = stub;
			g_weaponComponentsVtable = inImage;
		}
	}
}

// wcmount, wcbase, wcmany and wcfill: the packs' files mounted once TweaksV has started
static int32_t WeaponComponentsMakePool();
static void WeaponComponentsMountPacks(int argc, wchar_t** argv)
{
	static const int definitions[11] = { 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 6 };
	WeaponComponentsDamageBlobs(argc, argv);
	WeaponComponentsMounterAsAsked(argc, argv);
	if (HasOption(argc, argv, L"wcmany"))
	{
		WeaponComponentsMount("dlcpacks:/weaponpack_big/weaponcomponents.meta", 60);
		char name[64];
		for (int i = 0; i < 1100; i++)
		{
			sprintf_s(name, "dlcpacks:/weaponpack_%04d/weaponcomponents.meta", i);
			WeaponComponentsMount(name, 1);
		}
		return;
	}
	char name[64];
	if (HasOption(argc, argv, L"wcfill"))
	{
		for (int i = 0; i < 36; i++)
		{
			sprintf_s(name, "dlcpacks:/weaponpack%02d/common/data/weaponcomponents.meta", i + 1);
			WeaponComponentsMount(name, 470);
		}
		return;
	}
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, "dlcpacks:/weaponpack%02d/common/data/weaponcomponents.meta", i + 1);
		WeaponComponentsMount(name, definitions[i]);
		if (i == 0 && HasOption(argc, argv, L"wcempty"))	// the first file came before the pool
			WeaponComponentsMakePool();
	}
	if (HasOption(argc, argv, L"wcbase"))	// the base loader's run: every blob cleared, the list's count left, the files loaded again
	{
		g_weaponComponentsStore->files = 0;
		g_weaponComponentsPlacedStore = *g_weaponComponentsStore;
		for (int i = 0; i < 11; i++)
		{
			sprintf_s(name, "dlcpacks:/weaponpack%02d/common/data/weaponcomponents.meta", i + 1);
			WeaponComponentsMount(name, definitions[i]);
		}
	}
}

// A rip-relative operand set to lead to target: its disp32 at field, the instruction ending at end
static void WeaponComponentsPoint(volatile unsigned char* bytes, int field, int end, const void* target)
{
	const auto at = const_cast<unsigned char*>(bytes);
	const int32_t rel = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(at + end));
	memcpy(at + field, &rel, sizeof(rel));
}

// Every byte of the sites in a row, the 15 leas after the others, at most room - the rel32 of each of the list's 17 loads
// and of the size call, which the part writes, as 0: the copy before TweaksV loads, and the comparison after it ran
static size_t WeaponComponentsCopySites(unsigned char* to, size_t room)
{
	static constexpr size_t kNone = SIZE_MAX;
	size_t n = 0;
	const auto copy = [&n, to, room](const volatile unsigned char* bytes, size_t size, size_t written) {
		for (size_t i = 0; i < size && n < room; i++)
			to[n++] = written != kNone && i >= written && i < written + 4 ? 0 : bytes[i];
	};
	copy(g_weaponComponentsRebuild, sizeof(g_weaponComponentsRebuild), 34);
	copy(g_weaponComponentsSort, sizeof(g_weaponComponentsSort), 14);
	copy(g_weaponComponentsUpdate, sizeof(g_weaponComponentsUpdate), kNone);
	copy(g_weaponComponentsPool, sizeof(g_weaponComponentsPool), 19);
	copy(g_weaponComponentsLeaExtra, sizeof(g_weaponComponentsLeaExtra), kNone);
	for (auto& row : g_weaponComponentsLeas)
		copy(row, sizeof(row), 3);
	return n;
}

static const unsigned char* Rel32Target(const Site& site);
static bool InImage(const void* p);

// The game makes the pool, as it does before it loads any definition, through what the size call leads to: the lookup, or
// TweaksV's stub, another last error set before the call and the lookup's expected after it; what it gives
static int g_weaponComponentsPoolsMade = 0, g_weaponComponentsPoolErrorsKept = 0;
static int32_t WeaponComponentsMakePool()
{
	using Lookup = int32_t (*)(const void* configuration, uint32_t hash, int32_t defaultSize);
	const auto call = reinterpret_cast<Lookup>(const_cast<unsigned char*>(Rel32Target(g_weaponComponentsSizeCall)));
	SetLastError(0x1234);
	const int32_t slots = call(g_weaponComponentsConfiguration, 0xBAE33CCF, g_weaponComponentsGameDefault);
	g_weaponComponentsPoolErrorsKept += GetLastError() == kWeaponComponentsLookupError ? 1 : 0;
	g_weaponComponentsPoolsMade++;
	return slots;
}

// Where the list's 17 loads lead - the rebuild's, the sort's and the 15 others: to the game's own list, to one list outside
// the game in committed memory - the moved one - or elsewhere
struct WeaponComponentsLoads
{
	int moved = 0, game = 0, other = 0;
	const unsigned char* list = nullptr;
};
static WeaponComponentsLoads WeaponComponentsWhereLoadsLead()
{
	WeaponComponentsLoads loads;
	const auto load = [&loads](const volatile unsigned char* bytes, size_t field) {
		const unsigned char* const at = const_cast<const unsigned char*>(bytes) + field;
		int32_t rel = 0;
		memcpy(&rel, at, sizeof(rel));
		const unsigned char* const target = at + 4 + rel;
		MEMORY_BASIC_INFORMATION info{};
		if (target == reinterpret_cast<const unsigned char*>(g_weaponComponentsStore->list))
			loads.game++;
		else if (!InImage(target) && (!loads.list || target == loads.list) && VirtualQuery(target, &info, sizeof(info)) == sizeof(info) &&
			info.State == MEM_COMMIT)
		{
			loads.list = target;
			loads.moved++;
		}
		else
			loads.other++;
	};
	load(g_weaponComponentsRebuild, 34);
	load(g_weaponComponentsSort, 14);
	for (auto& row : g_weaponComponentsLeas)
		load(row, 3);
	return loads;
}

// The moved list: read/write data of 16854 entries, the game's 470 as TweaksV found them, then zeros
static bool WeaponComponentsMovedAsCopied(const unsigned char* list)
{
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(list, &info, sizeof(info)) != sizeof(info) || info.Protect != PAGE_READWRITE ||
		static_cast<size_t>(static_cast<const unsigned char*>(info.BaseAddress) + info.RegionSize - list) < 16854 * 8)
		return false;
	const auto entries = reinterpret_cast<const uint64_t*>(list);
	for (size_t i = 0; i < 16854; i++)
		if (entries[i] != (i < 470 ? g_weaponComponentsAtStart.list[i] : 0))
			return false;
	return true;
}

// How often TweaksV.log, read whole and shared as TweaksV shares it while it writes, holds text; and a wait, 100 ms at a
// time and ms at most, until it holds text n times
static int WeaponComponentsLogHas(const char* text)
{
	static char log[4 << 20];
	const HANDLE file = CreateFileW(L"TweaksV.log", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	DWORD read = 0;
	if (!ReadFile(file, log, sizeof(log) - 1, &read, nullptr))
		read = 0;
	CloseHandle(file);
	log[read] = '\0';
	int found = 0;
	for (const char* at = strstr(log, text); at; at = strstr(at + 1, text))
		found++;
	return found;
}
static bool WeaponComponentsAwait(const char* text, int n, DWORD ms)
{
	for (DWORD waited = 0; WeaponComponentsLogHas(text) < n; waited += 100)
	{
		if (waited >= ms)
			return false;
		Sleep(100);
	}
	return true;
}

// "wcputback": once TweaksV.log holds the first weapon components summary, the 11th of the 15 leas - the second of the six in
// the role of the game's protected code - leads to the game's own list again, as if the game had put it back; then up to 10 s
// for TweaksV's problem and 5 s more for its box, or its line that the box is not shown
static void WeaponComponentsPutBack()
{
	if (!WeaponComponentsAwait("weapon component definitions from", 1, 20000))
		return;
	const int boxes = WeaponComponentsLogHas("after the start-up summary");
	const auto row = const_cast<unsigned char*>(g_weaponComponentsLeas[10]);
	DWORD old = 0;
	if (!VirtualProtect(row, sizeof(g_weaponComponentsLeas[10]), PAGE_EXECUTE_READWRITE, &old))
		return;
	WeaponComponentsPoint(row, 3, 7, g_weaponComponentsStore->list);
	VirtualProtect(row, sizeof(g_weaponComponentsLeas[10]), old, &old);
	FlushInstructionCache(GetCurrentProcess(), row, sizeof(g_weaponComponentsLeas[10]));
	if (WeaponComponentsAwait("is not kept this session", 1, 10000))
		WeaponComponentsAwait("after the start-up summary", boxes + 1, 5000);
}

// "wcloadnoaccess": once TweaksV.log holds the first weapon components summary, the page of the 15 leas cannot be read until
// TweaksV's problem is in the log, up to 10 s, then its protection back; then up to 5 s for the box, or its line
static void WeaponComponentsLoadsNoAccess()
{
	if (!WeaponComponentsAwait("weapon component definitions from", 1, 20000))
		return;
	const int boxes = WeaponComponentsLogHas("after the start-up summary");
	const auto row = const_cast<unsigned char*>(g_weaponComponentsLeas[0]);
	DWORD old = 0;
	if (!VirtualProtect(row, sizeof(g_weaponComponentsLeas), PAGE_NOACCESS, &old))
		return;
	const bool said = WeaponComponentsAwait("may not be kept this session", 1, 10000);
	VirtualProtect(row, sizeof(g_weaponComponentsLeas), old, &old);
	if (said)
		WeaponComponentsAwait("after the start-up summary", boxes + 1, 5000);
}

// The report's field: where the 17 loads lead and what the moved list holds; the sites' other bytes and the store as placed;
// and what the size call leads to, called twice as the game makes its pool - the lookup itself, or TweaksV's stub, which must
// be read+execute and stays out of the stubs the report counts for the other parts - unless it leads to another mod's hook,
// where nothing is
static void WeaponComponentsReport()
{
	const WeaponComponentsLoads loads = WeaponComponentsWhereLoadsLead();
	unsigned char sites[sizeof(g_weaponComponentsPlacedSites)];
	const size_t size = WeaponComponentsCopySites(sites, sizeof(sites));
	wprintf(L"wc: loads %d moved %d game %d other", loads.moved, loads.game, loads.other);
	if (loads.list)
		wprintf(L", moved list %ls", WeaponComponentsMovedAsCopied(loads.list) ? L"RW, copied then 0" : L"CHANGED");
	wprintf(L", other bytes %ls, data %ls, size call ", memcmp(sites, g_weaponComponentsPlacedSites, size) == 0 ? L"as placed" : L"CHANGED",
		memcmp(g_weaponComponentsStore, &g_weaponComponentsPlacedStore, sizeof(WeaponComponentsStore)) == 0 ? L"as placed" : L"CHANGED");
	const unsigned char* const target = Rel32Target(g_weaponComponentsSizeCall);
	const auto lookup = reinterpret_cast<const unsigned char*>(&PoolSizeLookup);
	MEMORY_BASIC_INFORMATION info{};
	if (target != lookup && (InImage(target) || VirtualQuery(target, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT))
	{
		wprintf(L"elsewhere | wc loader %ls, files mounted %d | ", g_weaponComponentsLoader[1] == 0xE9 ? L"hooked" : L"as placed", g_weaponComponentsMounted);
		return;
	}
	const int32_t first = WeaponComponentsMakePool();
	const int32_t second = WeaponComponentsMakePool();
	wprintf(L"%ls: %d and %d slots, lookups %ld of %ld as called, last error %ls | ", target == lookup ? L"to the lookup" : info.Protect == PAGE_EXECUTE_READ ?
		L"to a stub, RX" : L"to a stub, NOT RX", first, second, g_weaponComponentsLookupsAsCalled, g_weaponComponentsLookups,
		g_weaponComponentsPoolErrorsKept == g_weaponComponentsPoolsMade ? L"kept" : L"LOST");
	wprintf(L"wc loader %ls, files mounted %d, loaded %d, destroyed %d (%d on the heap), list %d, last error kept %d, callables left %d | ", g_weaponComponentsLoader[1] == 0xE9 ?
		L"hooked" : L"as placed", g_weaponComponentsMounted, g_weaponComponentsLoaded, g_weaponComponentsDestroyed, g_weaponComponentsHeapDestroys,
		g_weaponComponentsStore->count, g_weaponComponentsErrorKept, g_weaponComponentsCallablesLeft);
}

// The siren setting IDs: the game's data their sites lead to, at the game's sizes and offsets (TweaksV's PATTERNS.md,
// "SirenSettings") - the members' data the two parser registrations' tables lead to, the ID members of type 0x01 and
// every member's offset at +0x08 as the game writes it when it registers the structure; the members' offsets; the ID
// table, per ID the position of its setting, 0xFF free; the carcols data with the siren settings' array at +0x48, their
// count at +0x50 and capacity at +0x52; the game's pointer to it - then the sites as code, the fake's loaders, the set-up, the
// late actions and the report's fields.
static void SetRel32(const Site& site, const void* target);
static const unsigned char* Rel32Target(const Site& site);
static const unsigned char* ImageEnd();
static bool InImage(const void* p);
static bool HasOption(int argc, wchar_t** argv, const wchar_t* option);
static DWORD ProtectionOf(const volatile void* p);
static void CheckData(const volatile unsigned char* bytes, size_t size);
struct SirenIdsMember { uint64_t nameHash; uint64_t offset; unsigned char type; unsigned char rest[15]; };
static SirenIdsMember* SirenCountLightsEntry(int argc, wchar_t** argv);	// the lights' member, entry 19: the siren count's, the whole member data
static SirenIdsMember g_sirenIdsMembersStorage[28];			// the siren settings' 20, the variations' 7 and an eighth
static SirenIdsMember* volatile g_sirenIdsMembers = g_sirenIdsMembersStorage;
static const SirenIdsMember kSirenIdsMemberInRdata = { 0, 0, 0x01, {} };	// sidatardata
static SirenIdsMember* g_sirenIdsSettingTableStorage[21];	// the siren settings' member data pointers, then null
static SirenIdsMember** volatile g_sirenIdsSettingTable = g_sirenIdsSettingTableStorage;
static SirenIdsMember* g_sirenIdsVariationTableStorage[8];	// the variations', then null
static SirenIdsMember** volatile g_sirenIdsVariationTable = g_sirenIdsVariationTableStorage;
static uint32_t g_sirenIdsSettingOffsetsStorage[20] = { 0x0, 0x5E0, 0x4, 0x8, 0xC, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34,
	0x35, 0x36, 0x37, 0x38, 0x3C };
static uint32_t* volatile g_sirenIdsSettingOffsets = g_sirenIdsSettingOffsetsStorage;
static uint32_t g_sirenIdsVariationOffsetsStorage[7] = { 0x0, 0x8, 0x18, 0x28, 0x38, 0x4A, 0x4B };
static uint32_t* volatile g_sirenIdsVariationOffsets = g_sirenIdsVariationOffsetsStorage;
static unsigned char g_sirenIdsTableStorage[256];
static unsigned char* volatile g_sirenIdsTable = g_sirenIdsTableStorage;
static unsigned char g_sirenIdsCarcolsStorage[0x160];
static unsigned char* volatile g_sirenIdsCarcols = g_sirenIdsCarcolsStorage;
static unsigned char* g_sirenIdsPointerStorage[2];			// the game's pointer to the carcols data, and another one (sicarcols)
static unsigned char** volatile g_sirenIdsPointer = g_sirenIdsPointerStorage;
constexpr size_t kSirenIdsSettingSize = 0x5E8;
static unsigned char* g_sirenIdsSettings = nullptr;			// the siren settings' array, allocated by SirenIdsSetup
static size_t g_sirenIdsCapacity = 0;
static bool g_sirenIdsPack = false, g_sirenIdsNone = false, g_sirenIdsUnreadable = false, g_sirenIdsRegister = false;
static bool g_sirenIdsLoad = false, g_sirenIdsTwice = false, g_sirenIdsClash = false, g_sirenIdsPacks = false, g_sirenIdsFlood = false, g_sirenIdsBigPack = false;

// The sites, each with a hash of its bytes as placed, and one of the data, for the report
struct SirenIdsSite { volatile unsigned char* bytes; size_t size; uint64_t placed; };
static SirenIdsSite g_sirenIdsSites[] = { { g_sirenIdsP1, sizeof(g_sirenIdsP1), 0 }, { g_sirenIdsP2, sizeof(g_sirenIdsP2), 0 },
	{ g_sirenIdsP3, sizeof(g_sirenIdsP3), 0 }, { g_sirenIdsT1, sizeof(g_sirenIdsT1), 0 }, { g_sirenIdsT2, sizeof(g_sirenIdsT2), 0 },
	{ g_sirenIdsT3, sizeof(g_sirenIdsT3), 0 }, { g_sirenIdsT4, sizeof(g_sirenIdsT4), 0 }, { g_sirenIdsD1, sizeof(g_sirenIdsD1), 0 },
	{ g_sirenIdsD2, sizeof(g_sirenIdsD2), 0 }, { g_sirenIdsC1, sizeof(g_sirenIdsC1), 0 }, { g_sirenIdsC2, sizeof(g_sirenIdsC2), 0 },
	{ g_sirenIdsC3, sizeof(g_sirenIdsC3), 0 }, { g_sirenIdsR1, sizeof(g_sirenIdsR1), 0 }, { g_sirenIdsM1, sizeof(g_sirenIdsM1), 0 },
	{ g_sirenIdsLeaExtra, sizeof(g_sirenIdsLeaExtra), 0 }, { g_sirenIdsH1, sizeof(g_sirenIdsH1), 0 }, { g_sirenIdsH2, sizeof(g_sirenIdsH2), 0 } };
constexpr size_t kSirenIdsSites = sizeof(g_sirenIdsSites) / sizeof(g_sirenIdsSites[0]);
static unsigned char g_sirenIdsCopy[kSirenIdsSites][0xE0];	// their bytes as placed
static uint64_t g_sirenIdsDataPlaced = 0;
static unsigned char g_sirenIdsTypesPlaced[2] = {};			// the two ID members' types as placed; the data's hash leaves them out
constexpr uint64_t kSirenIdsHashStart = 0xCBF29CE484222325ull;
static uint64_t SirenIdsHash(uint64_t hash, const volatile unsigned char* bytes, size_t size)
{
	for (size_t i = 0; i < size; i++)
		hash = (hash ^ bytes[i]) * 0x100000001B3ull;
	return hash;
}
static uint64_t SirenIdsData()
{
	uint64_t hash = SirenIdsHash(kSirenIdsHashStart, g_sirenIdsTable, sizeof(g_sirenIdsTableStorage));
	hash = SirenIdsHash(hash, g_sirenIdsCarcols, sizeof(g_sirenIdsCarcolsStorage));
	SirenIdsMember members[28];
	memcpy(members, g_sirenIdsMembers, sizeof(members));
	members[0].type = members[26].type = 0;
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(members), sizeof(members));
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(g_sirenIdsSettingTable), sizeof(g_sirenIdsSettingTableStorage));
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(g_sirenIdsVariationTable), sizeof(g_sirenIdsVariationTableStorage));
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(g_sirenIdsSettingOffsets), sizeof(g_sirenIdsSettingOffsetsStorage));
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(g_sirenIdsVariationOffsets), sizeof(g_sirenIdsVariationOffsetsStorage));
	hash = SirenIdsHash(hash, reinterpret_cast<const unsigned char*>(g_sirenIdsPointer), sizeof(g_sirenIdsPointerStorage));
	return g_sirenIdsUnreadable ? hash : SirenIdsHash(hash, g_sirenIdsSettings, g_sirenIdsCapacity * kSirenIdsSettingSize);
}

// Writes the bytes a text gives ("0F B6 2C 0F") at an offset of a site
static unsigned SirenIdsDigit(char c)
{
	return c <= '9' ? static_cast<unsigned>(c - '0') : static_cast<unsigned>((c | 0x20) - 'a' + 10);
}
static void SirenIdsPut(volatile unsigned char* site, size_t offset, const char* hex)
{
	for (size_t i = 0; hex[i] != 0 && hex[i + 1] != 0; i += hex[i + 2] != 0 ? 3 : 2)
		site[offset++] = static_cast<unsigned char>(SirenIdsDigit(hex[i]) * 16 + SirenIdsDigit(hex[i + 1]));
}
static void SirenIdsRel32(volatile unsigned char* site, int offset, const void* target)
{
	const Site field = { site, offset, 0 };
	SetRel32(field, target);
}

// A site run as code, from a byte of it to its next ret, with the registers it reads and writes as they are here
struct SirenIdsRegs { uint64_t rax, rcx, rdx, rbx, rbp, rsi, rdi, r13, r15; };
static void SirenIdsRunAt(SirenIdsRegs& regs, const volatile unsigned char* code)
{
	using Run = void (*)(SirenIdsRegs*, const void*);
	reinterpret_cast<Run>(const_cast<unsigned char*>(g_sirenIdsRun))(&regs, const_cast<const unsigned char*>(code));
}

// How the game's code stands: the table its lea loads, the width of an ID, the free-ID searches' limit - as placed before
// TweaksV loads and read from the sites once it has started
static unsigned char* g_sirenIdsLoaderTable = g_sirenIdsTableStorage;
static size_t g_sirenIdsWidth = 1;
static uint32_t g_sirenIdsLimit = 256;
static void SirenIdsReadCode()
{
	const Site lea = { g_sirenIdsT4, 16, 0 };
	g_sirenIdsLoaderTable = const_cast<unsigned char*>(Rel32Target(lea));
	g_sirenIdsWidth = g_sirenIdsT4[0x43] == 0xB7 ? 2 : 1;
	memcpy(&g_sirenIdsLimit, const_cast<const unsigned char*>(g_sirenIdsT4 + 0x84), sizeof(g_sirenIdsLimit));
}

// The ID at a setting, as wide as the game's code takes it
static unsigned SirenIdsLoad(const unsigned char* at)
{
	uint16_t id = 0;
	memcpy(&id, at, g_sirenIdsWidth);
	return id;
}

// The game's builder: a setting whose ID is none (0xFF) or taken gets the first free one - none when none is free - and the
// table the setting's position, a byte; then the settings' count
static void SirenIdsGive(size_t position, unsigned wanted)
{
	unsigned id = wanted;
	if (id == 0xFF || g_sirenIdsLoaderTable[id] != 0xFF)
	{
		id = 0xFF;
		for (unsigned candidate = 0; candidate < g_sirenIdsLimit; candidate++)
			if (g_sirenIdsLoaderTable[candidate] == 0xFF)
			{
				id = candidate;
				break;
			}
	}
	memcpy(g_sirenIdsSettings + position * kSirenIdsSettingSize, &id, g_sirenIdsWidth);
	g_sirenIdsLoaderTable[id] = static_cast<unsigned char>(position);
}
static void SirenIdsCount(size_t count)
{
	const uint16_t value = static_cast<uint16_t>(count);
	memcpy(g_sirenIdsCarcols + 0x50, &value, sizeof(value));
}
static size_t SirenIdsCounted()
{
	uint16_t value = 0;
	memcpy(&value, g_sirenIdsCarcols + 0x50, sizeof(value));
	return value;
}
// The game registering the two structures: every member's offset into its data
static void SirenIdsRegister()
{
	for (size_t i = 0; i < 20; i++)
		g_sirenIdsMembers[i].offset = g_sirenIdsSettingOffsets[i];
	for (size_t i = 0; i < 7; i++)
		g_sirenIdsMembers[20 + i].offset = g_sirenIdsVariationOffsets[i];
}

// The parser's reader of a file's value: as wide as the member's type says, the rest of the value cut (types 1 and 2 a byte)
static void SirenIdsParse(unsigned char* at, unsigned value, const SirenIdsMember& member)
{
	memcpy(at, &value, member.type == 0x03 || member.type == 0x04 ? 2 : 1);
}

// The settings' and variations' constructors, run as code
static void SirenIdsMakeSetting(unsigned char* setting)
{
	SirenIdsRegs regs{};
	regs.rcx = reinterpret_cast<uint64_t>(setting);
	SirenIdsRunAt(regs, g_sirenIdsC1 + kSirenIdsC1At);
}
static void SirenIdsMakeVariation(unsigned char* variation, bool inPlace)
{
	SirenIdsRegs regs{};
	regs.rcx = regs.rax = reinterpret_cast<uint64_t>(variation);
	if (inPlace)
		SirenIdsRunAt(regs, g_sirenIdsC2);
	else
		SirenIdsRunAt(regs, g_sirenIdsRun + 0x88);	// sub rsp, 0x28, then on to the allocating constructor's stores, which end with add rsp, 0x28
}

// A variation of a file: the ID its file gives through the parser, then the loaders' lookup of its setting's position; true if
// it leads to the setting that now has that ID
static int g_sirenIdsVariations = 0, g_sirenIdsVariationsRight = 0;
static void SirenIdsVariation(unsigned idInFile, bool inPlace)
{
	unsigned char variation[0x50];
	memset(variation, 0xCC, sizeof(variation));
	SirenIdsMakeVariation(variation, inPlace);
	SirenIdsParse(variation + 0x4B, idInFile, g_sirenIdsMembers[26]);
	const unsigned id = SirenIdsLoad(variation + 0x4B);
	const size_t count = SirenIdsCounted();
	const unsigned char position = g_sirenIdsLoaderTable[id];
	g_sirenIdsVariations++;
	if (id != 0xFF && position < count && SirenIdsLoad(g_sirenIdsSettings + position * kSirenIdsSettingSize) == id)
		g_sirenIdsVariationsRight++;
}

// The base builder's body, behind its prologue in .fakesit: the rule over the settings the base file gave
constexpr uint64_t kSirenIdsResult = 0x5151515151515151ull;
constexpr DWORD kSirenIdsLastError = 0x515;
static uint64_t SirenIdsBaseBuilder(unsigned char* carcols)
{
	unsigned char* array = nullptr;
	memcpy(&array, carcols + 0x48, sizeof(array));
	const size_t count = SirenIdsCounted();
	for (size_t i = 0; i < count; i++)
		SirenIdsGive(i, SirenIdsLoad(array + i * kSirenIdsSettingSize));
	SetLastError(kSirenIdsLastError);
	return kSirenIdsResult;
}

// The base loader: the base file's settings, each constructed and its ID through the parser, the builder through its start,
// the variations; the carcols pointer set. What the call brought and found is kept for the report.
static int g_sirenIdsLoaderCalls = 0, g_sirenIdsTableFree = 0, g_sirenIdsLoaderArguments = 0, g_sirenIdsBuilderKept = 0;
constexpr uint64_t kSirenIdsArguments[4] = { 0x1111111111111111ull, 0x2222222222222222ull, 0x3333333333333333ull, 0x4444444444444444ull };
static uint64_t SirenIdsBaseLoader(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth)
{
	g_sirenIdsLoaderCalls++;
	if (first == kSirenIdsArguments[0] && second == kSirenIdsArguments[1] && third == kSirenIdsArguments[2] && fourth == kSirenIdsArguments[3])
		g_sirenIdsLoaderArguments++;
	SirenIdsReadCode();
	bool isFree = true;
	for (uint32_t id = 0; id < g_sirenIdsLimit; id++)
		isFree = isFree && g_sirenIdsLoaderTable[id] == 0xFF;
	if (isFree)
		g_sirenIdsTableFree++;
	constexpr size_t kBase = 24;
	for (size_t i = 0; i < kBase; i++)
	{
		unsigned char* const setting = g_sirenIdsSettings + i * kSirenIdsSettingSize;
		SirenIdsMakeSetting(setting);
		if (g_sirenIdsClash && i == 11)	// no ID in the file
			continue;
		SirenIdsParse(setting, g_sirenIdsClash && i == 10 ? 7 : static_cast<unsigned>(1 + i), g_sirenIdsMembers[0]);
	}
	SirenIdsCount(kBase);
	using Builder = uint64_t (*)(unsigned char*);
	SetLastError(0);
	const uint64_t result = reinterpret_cast<Builder>(const_cast<unsigned char*>(g_sirenIdsH2 + 1))(g_sirenIdsCarcols);
	if (result == kSirenIdsResult && GetLastError() == kSirenIdsLastError)
		g_sirenIdsBuilderKept++;
	const unsigned variations[] = { 1, 13, 24 };
	for (const unsigned id : variations)
		SirenIdsVariation(id, true);
	g_sirenIdsPointer[0] = g_sirenIdsCarcols;
	SetLastError(kSirenIdsLastError);
	return kSirenIdsResult;
}

// The game's init: its own table filled, then the base loader through the call at the site
static int g_sirenIdsInitKept = 0, g_sirenIdsInits = 0;
static void SirenIdsInit()
{
	memset(g_sirenIdsTable, 0xFF, sizeof(g_sirenIdsTableStorage));
	const Site call = { g_sirenIdsR1, 14, 0 };
	using Loader = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t);
	SetLastError(0);
	const uint64_t result = reinterpret_cast<Loader>(const_cast<unsigned char*>(Rel32Target(call)))(kSirenIdsArguments[0], kSirenIdsArguments[1], kSirenIdsArguments[2],
		kSirenIdsArguments[3]);
	g_sirenIdsInits++;
	if (result == kSirenIdsResult && GetLastError() == kSirenIdsLastError)
		g_sirenIdsInitKept++;
}

// A pack's file: its name as the game's data file entry holds it at +0, the IDs of its settings, -1 for none given, and how its read goes
enum class SirenIdsRead { Parses, Fails, CannotStart };
struct SirenIdsFile { char name[0xA8]; int ids[8]; size_t count; SirenIdsRead read; };
static SirenIdsFile g_sirenIdsFiles[] = {
	{ "dlcpacks:/sipack1/data/carcols.meta", { 300, 1534, 40, 5, -1, 300 }, 6 },
	{ "dlcpacks:/sipack2/data/carcols.meta", { 511, 767, 511 }, 3 },
	{ "dlcpacks:/sipack3/data/carcols.meta", {}, 0 },
};
static SirenIdsFile g_sirenIdsBigFile = { "dlcpacks:/sibig/data/carcols.meta", { 6000 }, 4097 };	// past eight settings a file's all have its first ID
static SirenIdsFile g_sirenIdsBadFiles[] = {
	{ "dlcpacks:/sibad/data/carcols.meta", {}, 0, SirenIdsRead::Fails },
	{ "dlcpacks:/sinoread/data/carcols.meta", {}, 0, SirenIdsRead::CannotStart },
	{ "dlcpacks:/silong/d\xE4ta/0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", {}, 0 },
};
static unsigned char g_sirenIdsFileRoom[8 * kSirenIdsSettingSize];
static unsigned char* g_sirenIdsFileSettings = g_sirenIdsFileRoom;

// The callback's body, behind its prologue in .fakesit: the file read and each setting constructed; the dedupe's load run as
// code, every register it must keep compared, then its compare loop as code; what is not dropped appended raw; then the
// builder's rule over the appended ones
static int g_sirenIdsDedupeLoads = 0, g_sirenIdsDedupeKept = 0;
static uint64_t SirenIdsPackBody(void*, const unsigned char*, const SirenIdsFile* const* object)
{
	const SirenIdsFile* const file = *object;
	for (size_t i = 0; i < file->count; i++)
	{
		unsigned char* const setting = g_sirenIdsFileSettings + i * kSirenIdsSettingSize;
		SirenIdsMakeSetting(setting);
		const int id = file->ids[file->count > 8 ? 0 : i];
		if (id >= 0)
			SirenIdsParse(setting, static_cast<unsigned>(id), g_sirenIdsMembers[0]);
	}
	const size_t first = SirenIdsCounted();
	for (size_t i = 0; i < file->count; i++)
	{
		const size_t count = SirenIdsCounted();
		SirenIdsRegs load{};
		load.rax = 0x0123456789ABCDEFull;
		load.rcx = 0xFEDCBA9876543210ull;
		load.rdx = 0xDDDDDDDDDDDDDDDDull;
		load.rbx = reinterpret_cast<uint64_t>(g_sirenIdsCarcols);
		load.rsi = 0x5555555555555555ull;
		load.r13 = reinterpret_cast<uint64_t>(g_sirenIdsFileSettings);
		load.r15 = i * kSirenIdsSettingSize;
		const SirenIdsRegs before = load;
		SirenIdsRunAt(load, g_sirenIdsD1 + 0x0B);
		g_sirenIdsDedupeLoads++;
		if (load.rax == before.rax && load.rcx == before.rcx && load.rbx == before.rbx && load.rsi == before.rsi && load.r13 == before.r13 && load.r15 == before.r15 &&
			load.rdi == count)
			g_sirenIdsDedupeKept++;
		const unsigned id = static_cast<unsigned>(load.rdx);
		if ((id & 0xFF) != 0xFF && count != 0)
		{
			SirenIdsRegs loop{};
			loop.rdx = id;
			loop.rbp = reinterpret_cast<uint64_t>(g_sirenIdsSettings);
			loop.rbx = count * kSirenIdsSettingSize;
			loop.rsi = 0x5555555555555555ull;
			SirenIdsRunAt(loop, g_sirenIdsD2);
			if (loop.rax == 1)	// the jz: an ID loaded already, the setting dropped
				continue;
		}
		memcpy(g_sirenIdsSettings + count * kSirenIdsSettingSize, g_sirenIdsFileSettings + i * kSirenIdsSettingSize, 2);
		SirenIdsCount(count + 1);
	}
	const size_t count = SirenIdsCounted();
	for (size_t i = first; i < count; i++)
		SirenIdsGive(i, SirenIdsLoad(g_sirenIdsSettings + i * kSirenIdsSettingSize));
	SetLastError(kSirenIdsLastError);
	return kSirenIdsResult;
}

// A pack's callback, through its start at the site: itself, the byte that says the file parsed, where the parsed object's pointer lies
static uint64_t SirenIdsCallBack(const volatile unsigned char* callback, const SirenIdsFile* object, bool parsed)
{
	using Callback = uint64_t (*)(void*, const unsigned char*, const SirenIdsFile* const*);
	const unsigned char ok = parsed ? 1 : 0;
	return reinterpret_cast<Callback>(const_cast<unsigned char*>(callback + 1))(g_sirenIdsCarcols, &ok, &object);
}

// The game's file opener, which the carcols mount's call leads to, as its nine arguments come: the parser, the name, "meta", the parent's
// structure, the parsed object, the callable and three more. A read that cannot start calls the callback at once, with a false byte, inside
// the mount's call. Its result and its last error go back through TweaksV's stub to the mount.
constexpr uint32_t kSirenIdsOpenResult = 0x5A5A5A5A;
static int g_sirenIdsOpened = 0, g_sirenIdsOpenedAsHanded = 0, g_sirenIdsOpenedKept = 0;
static const char* g_sirenIdsNameHanded = nullptr;
static const void* g_sirenIdsObjectHanded = nullptr;
static const char kSirenIdsExtension[] = "meta";
static unsigned char g_sirenIdsStructure;
static unsigned char* g_sirenIdsStructurePointer = &g_sirenIdsStructure;
static uint64_t SirenIdsOpenFile(void* parser, const char* name, const char* extension, void* structure, const SirenIdsFile* object, void* callable, uint64_t seventh,
	uint64_t eighth, uint64_t ninth)
{
	g_sirenIdsOpened++;
	if (parser == g_sirenIdsCarcols && name == g_sirenIdsNameHanded && extension == kSirenIdsExtension && structure == g_sirenIdsStructurePointer && object == g_sirenIdsObjectHanded &&
		callable != nullptr && (seventh & 0xFF) == 1 && (eighth & 0xFF) == 1 && ninth == 0)
		g_sirenIdsOpenedAsHanded++;
	if (object->read == SirenIdsRead::CannotStart)
		SirenIdsCallBack(g_sirenIdsH1, object, false);
	SetLastError(kSirenIdsLastError);
	return kSirenIdsOpenResult;
}

// A pack's carcols through the game's way: the mount, whose call at its site hands the file opener the name in its data file entry and the
// parsed object - with the entry moved right after, as the game's array of entries can, so that a name kept by a pointer shows - then the pump: the
// callback, with the parsed object and whether the file parsed.
static int g_sirenIdsPackCalls = 0, g_sirenIdsPackKept = 0;
static bool g_sirenIdsOtherObject = false;
static unsigned char* g_sirenIdsNoAccess = nullptr;			// a page that cannot be read: "sinamebad"'s name, "sivarunreadable"'s variations
static void SirenIdsMountPack(const SirenIdsFile& file, bool nameUnreadable = false)
{
	char entry[0x80];
	memcpy(entry, file.name, sizeof(entry));
	g_sirenIdsNameHanded = nameUnreadable ? reinterpret_cast<const char*>(g_sirenIdsNoAccess) : entry;
	g_sirenIdsObjectHanded = &file;
	using Mount = uint64_t (*)(void*, const char*, const char*, const SirenIdsFile*);
	SetLastError(0);
	const uint64_t opened = reinterpret_cast<Mount>(const_cast<unsigned char*>(g_sirenIdsH3))(g_sirenIdsCarcols, g_sirenIdsNameHanded, kSirenIdsExtension, &file);
	if (static_cast<uint32_t>(opened) == kSirenIdsOpenResult && GetLastError() == kSirenIdsLastError)
		g_sirenIdsOpenedKept++;
	memset(entry, 'X', sizeof(entry) - 1);
	entry[sizeof(entry) - 1] = '\0';
	if (file.read == SirenIdsRead::CannotStart)
		return;
	SirenIdsFile other = file;
	SetLastError(0);
	const uint64_t result = SirenIdsCallBack(g_sirenIdsH1, g_sirenIdsOtherObject ? &other : &file, file.read != SirenIdsRead::Fails);
	if (file.read == SirenIdsRead::Fails)
		return;
	g_sirenIdsPackCalls++;
	if (result == kSirenIdsResult && GetLastError() == kSirenIdsLastError)
		g_sirenIdsPackKept++;
}

// A pack's carvariations as its callback's start gets them: the parsed object's array of variations, 0x50 bytes each, with its model's name's
// pointer at +0 and its ID through the parser at +0x4B, and their count, a word, at +8; the body, behind its prologue, as the game's gives each
// variation that names an ID the table's position for it, a free entry's 0xFF among them. Whether it ran and came back is kept for the report.
struct SirenIdsVariations { unsigned char* array; uint16_t count; };
static int g_sirenIdsVariationCalls = 0, g_sirenIdsVariationsKept = 0, g_sirenIdsVariationBodies = 0;
static uint64_t SirenIdsVariationsBody(void*, const unsigned char*, const SirenIdsVariations* const* object)
{
	g_sirenIdsVariationBodies++;
	const SirenIdsVariations& file = **object;
	for (size_t i = 0; file.array != g_sirenIdsNoAccess && i < file.count; i++)
	{
		unsigned char* const variation = file.array + i * 0x50;
		const unsigned id = SirenIdsLoad(variation + 0x4B);
		if (id != 0xFF)
			variation[0x49] = g_sirenIdsLoaderTable[id];
	}
	SetLastError(kSirenIdsLastError);
	return kSirenIdsResult;
}
static void SirenIdsVariationsCall(const SirenIdsVariations& file, bool parsed)
{
	using Callback = uint64_t (*)(void*, const unsigned char*, const SirenIdsVariations* const*);
	const unsigned char ok = parsed ? 1 : 0;
	const SirenIdsVariations* const object = &file;
	SetLastError(0);
	const uint64_t result = reinterpret_cast<Callback>(const_cast<unsigned char*>(g_sirenIdsH4 + 1))(g_sirenIdsCarcols, &ok, &object);
	g_sirenIdsVariationCalls++;
	if (!parsed || (result == kSirenIdsResult && GetLastError() == kSirenIdsLastError))
		g_sirenIdsVariationsKept++;
}
// A file of these vehicles, id -1 for none given: each made by the constructor as code, its ID through the parser; its names lie where they are
struct SirenIdsVehicle { const char* name; int id; };
static void SirenIdsLoadVariations(const SirenIdsVehicle* vehicles, size_t count)
{
	static unsigned char array[64][0x50];
	for (size_t i = 0; i < count; i++)
	{
		unsigned char* const variation = array[i];
		memset(variation, 0xCC, 0x50);
		SirenIdsMakeVariation(variation, true);
		memcpy(variation, &vehicles[i].name, sizeof(vehicles[i].name));
		if (vehicles[i].id >= 0)
			SirenIdsParse(variation + 0x4B, static_cast<unsigned>(vehicles[i].id), g_sirenIdsMembers[26]);
	}
	const SirenIdsVariations file = { array[0], static_cast<uint16_t>(count) };
	SirenIdsVariationsCall(file, true);
}

// Before TweaksV loads and as soon as it has started: what the option "sihooknoexec" takes away from the callback's page and puts back, before any code of the page runs
static void SirenIdsHookProtect(int argc, wchar_t** argv, DWORD protection)
{
	DWORD old = 0;
	if (HasOption(argc, argv, L"sihooknoexec"))
		VirtualProtect(const_cast<unsigned char*>(g_sirenIdsH1), sizeof(g_sirenIdsH1), protection, &old);
}

// What the options ask of a pack's files and carvariations once the base files are read, and the two sites' bytes as placed, for the report
static struct { bool badFiles, nameBad, profile, variations, varMany, varBad, varUnreadable, varNames; } g_sirenIdsOptions = {};
static unsigned char g_sirenIdsH3Placed[sizeof(g_sirenIdsH3)], g_sirenIdsH4Placed[sizeof(g_sirenIdsH4)];

// The sites as the game has them, as the options change them, and the data: 24 siren settings with the IDs 1 to 24 (simany: 300,
// the IDs 0 to 254, then none)
static void SirenIdsSetup(int argc, wchar_t** argv)
{
	for (const SirenIdsSite& site : g_sirenIdsSites)
		for (size_t i = 0; i < site.size; i++)
			site.bytes[i] = 0xCC;
	SirenIdsPut(g_sirenIdsP1, 0, "C7 05 00 00 00 00 0D 9B EB 84 0F 57 C0 0F 11 05 00 00 00 00 48 8D 05 00 00 00 00 48 89 05 00 00 00 00 48 8D 05");
	SirenIdsPut(g_sirenIdsP2, 0, "C7 05 00 00 00 00 A9 FE EA 18 0F 57 C0 0F 11 05 00 00 00 00 48 8D 05 00 00 00 00 48 89 05 00 00 00 00 48 8D 05");
	SirenIdsPut(g_sirenIdsP3, 0, "48 8B 41 08 0F B6 48 10 48 83 F9 21 0F 87");
	SirenIdsPut(g_sirenIdsT1, 0, "B9 4B 00 00 00 31 D2 4C 8D 05 00 00 00 00 4C 8D 0D 00 00 00 00");
	SirenIdsPut(g_sirenIdsT1, 0x93, HasOption(argc, argv, L"siwidened") ? "0F B7 2C 0F 48 81 FD FF 00 00 00 C3" : "0F B6 2C 0F 48 81 FD FF 00 00 00 C3");
	SirenIdsPut(g_sirenIdsT1, 0xC4, "0F B6 1C 2B 88 1C 0F C3");
	SirenIdsPut(g_sirenIdsT2, 0, "41 0F B7 46 50 48 85 C0 0F 84 00 00 00 00 49 8B 4E 48 31 D2 48 8D 1D 00 00 00 00");
	SirenIdsPut(g_sirenIdsT2, 0x29, "40 88 31 C3");
	SirenIdsPut(g_sirenIdsT2, 0x30, "0F B6 2C 39 C3");
	SirenIdsPut(g_sirenIdsT2, 0x4B, "0F B6 2C 39 48 81 FD FF 00 00 00 C3");
	const char* const limit = HasOption(argc, argv, L"si512") ? "48 81 FE 00 02 00 00" : "48 81 FE 00 01 00 00";
	SirenIdsPut(g_sirenIdsT2, 0x85, HasOption(argc, argv, L"silimit") ? "48 81 FE 01 00 01 00" : HasOption(argc, argv, L"silimitop") ? "48 81 FF 00 01 00 00" : limit);
	SirenIdsPut(g_sirenIdsT2, 0xCD, limit);
	SirenIdsPut(g_sirenIdsT2, 0xD6, "40 B6 FF");
	SirenIdsPut(g_sirenIdsT3, 0, "B8 4B 00 00 00 31 C9 4C 8D 05 00 00 00 00 48 8D 2D 00 00 00 00");
	SirenIdsPut(g_sirenIdsT3, 0x8D, "0F B6 3C 03 48 81 FF FF 00 00 00 C3");
	SirenIdsPut(g_sirenIdsT3, 0xBE, "0F B6 14 17 88 14 03 C3");
	SirenIdsPut(g_sirenIdsT4, 0, "4C 8B 05 00 00 00 00 49 8B 58 48 89 CA 48 8D 2D 00 00 00 00");
	SirenIdsPut(g_sirenIdsT4, 0x21, "40 88 33 C3");
	SirenIdsPut(g_sirenIdsT4, 0x28, "0F B6 0C 3B C3");
	SirenIdsPut(g_sirenIdsT4, 0x42, "0F B6 34 3B 48 81 FE FF 00 00 00 C3");
	SirenIdsPut(g_sirenIdsT4, 0x81, limit);
	SirenIdsPut(g_sirenIdsT4, 0xC9, HasOption(argc, argv, L"sidisagree") ? "48 81 FE 00 02 00 00" : limit);
	SirenIdsPut(g_sirenIdsT4, 0xD2, "40 B6 FF");
	SirenIdsPut(g_sirenIdsD1, 0, "4D 8B 6F 48 4D 69 FE E8 05 00 00 43 0F B6 54 3D 00 0F B7 7B 50 80 FA FF 74 00 C3");
	SirenIdsPut(g_sirenIdsD2, 0, "31 F6 66 2E 0F 1F 84 00 00 00 00 00 38 54 35 00 0F 84 00 00 00 00 48 81 C6 E8 05 00 00 48 39 F3 75 EA");
	SirenIdsPut(g_sirenIdsD2, 34, "31 C0 C3 B8 01 00 00 00 C3");	// the loop's way out, no ID the same: 0; its jz's, an ID the same: 1
	g_sirenIdsC1[0] = HasOption(argc, argv, L"src1notstart") ? 0x41 : 0x90;
	SirenIdsPut(g_sirenIdsC1, kSirenIdsC1At, "56 48 83 EC 20 48 89 CE C7 41 1C 00 00 00 00 B8 83 00 00 00 0F 57 C0 E9 84 00 00 00");	// then on to the ID's default
	SirenIdsPut(g_sirenIdsC1, kSirenIdsC1At + 0xA0, "C6 06 FF EB 24");
	SirenIdsPut(g_sirenIdsC1, kSirenIdsC1At + 0xC9, "89 46 1C 48 83 C4 20 5E C3");
	SirenIdsPut(g_sirenIdsC2, 0, "C7 41 30 00 00 00 00 48 C7 41 38 00 00 00 00 C7 41 40 00 00 00 00 C7 41 48 00 00 FF FF C3");
	SirenIdsPut(g_sirenIdsC3, 0, "C7 40 30 00 00 00 00 48 C7 40 38 00 00 00 00 C7 40 40 00 00 00 00 C7 40 48 00 00 FF FF 48 83 C4 28 C3");
	if (HasOption(argc, argv, L"sibadt2"))
		g_sirenIdsT2[0x30 + 1] = 0xB7;
	if (HasOption(argc, argv, L"sibadt3"))
		g_sirenIdsT3[0x8D + 1] = 0xB7;
	if (HasOption(argc, argv, L"sibadt4"))
		g_sirenIdsT4[0x28 + 1] = 0xB7;
	if (HasOption(argc, argv, L"sibadc1"))
		g_sirenIdsC1[kSirenIdsC1At + 0xA0 + 2] = 0x00;
	if (HasOption(argc, argv, L"sibadc2"))
		g_sirenIdsC2[2] = 0x34;
	if (HasOption(argc, argv, L"sibadc3"))
		g_sirenIdsC3[2] = 0x34;
	SirenIdsPut(g_sirenIdsR1, 0, "66 0F 7F 05 00 00 00 00 E8 00 00 00 00 E8 00 00 00 00 80 3D");
	SirenIdsPut(g_sirenIdsM1, 0, "0F B6 81 3B 05 00 00 48 85 C0 74 13 48 8B 0D 00 00 00 00 48 69 C0 E8 05 00 00 48 03 41 48 C3 31 C0 C3");
	SirenIdsPut(g_sirenIdsLeaExtra, 0, HasOption(argc, argv, L"sififth") ? "48 8D 05 00 00 00 00" : "CC 8D 05 00 00 00 00");
	// a pack's carcols callback and the base builder: a NOP before each, their prologues, a call of the fake's body, the epilogues
	const bool byte0 = HasOption(argc, argv, L"sibyte0");
	SirenIdsPut(g_sirenIdsH1, 0, byte0 ? "00" : "90");
	SirenIdsPut(g_sirenIdsH1, 1, "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 68 80 3A 00 0F 84 0C 00 00 00 48 B8 00 00 00 00 00 00 00 00 FF D0 48 83 C4 68 5B 5D 5F 5E 41 5C 41 5D 41 5E 41 5F C3");
	const uint64_t packBody = reinterpret_cast<uint64_t>(&SirenIdsPackBody);
	memcpy(const_cast<unsigned char*>(g_sirenIdsH1 + 28), &packBody, sizeof(packBody));
	SirenIdsPut(g_sirenIdsH2, 0, byte0 ? "00" : "90");
	SirenIdsPut(g_sirenIdsH2, 1, "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 28 49 89 CE 0F B7 71 10 89 F0 4C 89 F1 48 B8 00 00 00 00 00 00 00 00 FF D0 48 83 C4 28 5B 5D 5F 5E 41 5C 41 5D 41 5E 41 5F C3");
	const uint64_t builderBody = reinterpret_cast<uint64_t>(&SirenIdsBaseBuilder);
	memcpy(const_cast<unsigned char*>(g_sirenIdsH2 + 31), &builderBody, sizeof(builderBody));
	// a pack's carcols mount: its prologue, taking the parser, the name, "meta" and the object into the registers the game's has them in, I-E-H3
	// at +26 and its epilogue, the file opener's result back
	SirenIdsPut(g_sirenIdsH3, 0, "53 57 41 54 41 56 41 57 48 83 EC 60 49 89 CC 49 89 D7 4D 89 C6 4C 89 CB 31 FF");
	SirenIdsPut(g_sirenIdsH3, 26, "4C 8B 0D 00 00 00 00 48 89 7C 24 40 48 8D 44 24 50 48 89 44 24 28 48 89 5C 24 20 C6 44 24 38 01 C6 44 24 30 01 4C 89 E1 4C 89 FA 4D 89 F0 E8 00 00 00 00 89 C3");
	SirenIdsPut(g_sirenIdsH3, 79, "89 D8 48 83 C4 60 41 5F 41 5E 41 5C 5F 5B C3");
	// a pack's carvariations callback, as the other two
	SirenIdsPut(g_sirenIdsH4, 0, byte0 ? "00" : "90");
	SirenIdsPut(g_sirenIdsH4, 1, "41 57 41 56 41 55 41 54 56 57 55 53 48 81 EC D8 00 00 00 0F 29 B4 24 C0 00 00 00 80 3A 00 0F 84 15 00 00 00 4D 8B 28 66 41 83 7D 08 00 48 B8 00 00 00 00 00 00 00 00 FF D0 "
		"0F 28 B4 24 C0 00 00 00 48 81 C4 D8 00 00 00 5B 5D 5F 5E 41 5C 41 5D 41 5E 41 5F C3");
	const uint64_t variationsBody = reinterpret_cast<uint64_t>(&SirenIdsVariationsBody);
	memcpy(const_cast<unsigned char*>(g_sirenIdsH4 + 48), &variationsBody, sizeof(variationsBody));
	// the way into the sites as code: rcx the registers, rdx the code; rbx, rbp, rsi, rdi, r13, r15 saved around them
	SirenIdsPut(g_sirenIdsRun, 0, "53 55 57 56 41 55 41 57 51 49 89 CA 49 89 D3 49 8B 02 49 8B 4A 08 49 8B 52 10 49 8B 5A 18 49 8B 6A 20 49 8B 72 28 49 8B 7A 30 4D 8B 6A 38 4D 8B 7A 40 "
		"48 83 EC 20 41 FF D3 48 83 C4 20 41 5A 49 89 02 49 89 4A 08 49 89 52 10 49 89 5A 18 49 89 6A 20 49 89 72 28 49 89 7A 30 4D 89 6A 38 4D 89 7A 40 41 5F 41 5D 5E 5F 5D 5B C3");
	// at +0x88, for the allocating constructor, whose stores end with add rsp, 0x28: sub rsp, 0x28, then jmp to them
	SirenIdsPut(g_sirenIdsRun, 0x88, "48 83 EC 28 E9 00 00 00 00");

	// the parser registrations' tables and the members' data, registered - siregister: once TweaksV has started
	for (size_t i = 0; i < 20; i++)
	{
		g_sirenIdsMembers[i].type = i == 0 ? 0x01 : 0x07;
		g_sirenIdsSettingTable[i] = &g_sirenIdsMembers[i];
	}
	g_sirenIdsSettingTable[19] = SirenCountLightsEntry(argc, argv);
	if (HasOption(argc, argv, L"sroffset"))
		g_sirenIdsSettingOffsets[19] = 0x40;
	for (size_t i = 0; i < 7; i++)
	{
		g_sirenIdsMembers[20 + i].type = i >= 5 ? 0x01 : 0x07;	// the light and the siren setting ID, a byte each
		g_sirenIdsVariationTable[i] = &g_sirenIdsMembers[20 + i];
	}
	g_sirenIdsRegister = HasOption(argc, argv, L"siregister");
	if (!g_sirenIdsRegister)
		SirenIdsRegister();
	g_sirenIdsMembers[27].type = 0x01;
	if (HasOption(argc, argv, L"simembers"))
		g_sirenIdsVariationTable[7] = &g_sirenIdsMembers[27];
	if (HasOption(argc, argv, L"sitype"))
		g_sirenIdsMembers[26].type = 0x03;
	if (HasOption(argc, argv, L"sioffsets"))
		g_sirenIdsSettingOffsets[2] = 0x8;
	if (HasOption(argc, argv, L"sidataout"))
		g_sirenIdsSettingTable[0] = reinterpret_cast<SirenIdsMember*>(const_cast<unsigned char*>(ImageEnd() + 0x73000));
	if (HasOption(argc, argv, L"sidatardata"))
		g_sirenIdsSettingTable[0] = const_cast<SirenIdsMember*>(&kSirenIdsMemberInRdata);
	if (HasOption(argc, argv, L"sioffsetfield"))
		g_sirenIdsMembers[0].offset = 8;
	g_sirenIdsTypesPlaced[0] = g_sirenIdsMembers[0].type;
	g_sirenIdsTypesPlaced[1] = g_sirenIdsMembers[26].type;

	// the siren settings, given their IDs as the game gives them
	const bool many = HasOption(argc, argv, L"simany");
	g_sirenIdsLoad = HasOption(argc, argv, L"siload");
	g_sirenIdsTwice = HasOption(argc, argv, L"sitwice");
	g_sirenIdsClash = HasOption(argc, argv, L"siclash");
	g_sirenIdsPacks = HasOption(argc, argv, L"sipacks");
	g_sirenIdsFlood = HasOption(argc, argv, L"siflood");
	g_sirenIdsBigPack = HasOption(argc, argv, L"sibigpack");
	g_sirenIdsOtherObject = HasOption(argc, argv, L"siotherobject");
	g_sirenIdsOptions = { HasOption(argc, argv, L"sibadfiles"), HasOption(argc, argv, L"sinamebad"), HasOption(argc, argv, L"siprofile"), HasOption(argc, argv, L"sivariations"),
		HasOption(argc, argv, L"sivarmany"), HasOption(argc, argv, L"sivarbad"), HasOption(argc, argv, L"sivarunreadable"), HasOption(argc, argv, L"sivarnames") };
	g_sirenIdsNoAccess = static_cast<unsigned char*>(VirtualAlloc(nullptr, 2 * 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	DWORD noAccessOld = 0;
	VirtualProtect(g_sirenIdsNoAccess, 4096, PAGE_NOACCESS, &noAccessOld);
	g_sirenIdsCapacity = many ? 304 : 64;
	g_sirenIdsSettings = static_cast<unsigned char*>(VirtualAlloc(nullptr, g_sirenIdsCapacity * kSirenIdsSettingSize, MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE));
	if (!g_sirenIdsLoad)
	{
		memset(g_sirenIdsTable, 0xFF, sizeof(g_sirenIdsTableStorage));
		const size_t count = many ? 300 : 24;
		for (size_t i = 0; i < count; i++)
			SirenIdsGive(i, many ? (i < 255 ? static_cast<unsigned>(i) : 0xFFu) : static_cast<unsigned>(1 + i));
		SirenIdsCount(count);
	}
	const uint16_t capacity = static_cast<uint16_t>(g_sirenIdsCapacity);
	memcpy(g_sirenIdsCarcols + 0x52, &capacity, sizeof(capacity));
	memcpy(g_sirenIdsCarcols + 0x48, &g_sirenIdsSettings, sizeof(g_sirenIdsSettings));
	g_sirenIdsUnreadable = HasOption(argc, argv, L"siunreadable");
	DWORD old = 0;
	if (g_sirenIdsUnreadable)
		VirtualProtect(g_sirenIdsSettings, g_sirenIdsCapacity * kSirenIdsSettingSize, PAGE_NOACCESS, &old);
	g_sirenIdsPack = HasOption(argc, argv, L"sipack");
	g_sirenIdsNone = HasOption(argc, argv, L"sinone");
	g_sirenIdsPointer[0] = g_sirenIdsNone || g_sirenIdsLoad ? nullptr : g_sirenIdsCarcols;
	g_sirenIdsPointer[1] = g_sirenIdsCarcols;

	// where the sites lead
	SirenIdsRel32(g_sirenIdsP1, 23, HasOption(argc, argv, L"sitableout") ? static_cast<const void*>(ImageEnd() + 0x74000) : g_sirenIdsSettingTable);
	SirenIdsRel32(g_sirenIdsP1, 37, g_sirenIdsSettingOffsets);
	SirenIdsRel32(g_sirenIdsP2, 23, g_sirenIdsVariationTable);
	SirenIdsRel32(g_sirenIdsP2, 37, HasOption(argc, argv, L"sioffsetsout") ? static_cast<const void*>(ImageEnd() + 0x75000) : g_sirenIdsVariationOffsets);
	const unsigned char* const table = HasOption(argc, argv, L"simoved") ? ImageEnd() + 0x70000 : g_sirenIdsTable;
	SirenIdsRel32(g_sirenIdsT1, 10, table);
	SirenIdsRel32(g_sirenIdsT2, 23, table);
	SirenIdsRel32(g_sirenIdsT3, 10, table);
	SirenIdsRel32(g_sirenIdsT4, 16, HasOption(argc, argv, L"sisplit") ? table + 16 : table);
	SirenIdsRel32(g_sirenIdsLeaExtra, 3, table);
	SirenIdsRel32(g_sirenIdsT4, 3, HasOption(argc, argv, L"sicarcolsout") ? static_cast<const void*>(ImageEnd() + 0x76000) : &g_sirenIdsPointer[0]);
	SirenIdsRel32(g_sirenIdsM1, 15, &g_sirenIdsPointer[HasOption(argc, argv, L"sicarcols") ? 1 : 0]);
	SirenIdsRel32(g_sirenIdsD2, 18, HasOption(argc, argv, L"sijz") ? ImageEnd() + 0x71000 : const_cast<const unsigned char*>(g_sirenIdsD2 + 37));
	SirenIdsRel32(g_sirenIdsR1, 4, table + (HasOption(argc, argv, L"sifill") ? 0xE0 : 0xF0));
	SirenIdsRel32(g_sirenIdsR1, 14, HasOption(argc, argv, L"sicallhooked") ? ImageEnd() + 0x72000 : reinterpret_cast<const unsigned char*>(&SirenIdsBaseLoader));
	SirenIdsRel32(g_sirenIdsH1, 22, const_cast<const unsigned char*>(g_sirenIdsH1 + 38));	// the jz of a file that did not parse, to the epilogue
	SirenIdsRel32(g_sirenIdsH3, 26 + 3, &g_sirenIdsStructurePointer);
	SirenIdsRel32(g_sirenIdsH3, 26 + 47, HasOption(argc, argv, L"sifilehooked") ? static_cast<const void*>(ImageEnd() + 0x77000) : reinterpret_cast<const void*>(&SirenIdsOpenFile));
	SirenIdsRel32(g_sirenIdsRun, 0x8D, const_cast<const unsigned char*>(g_sirenIdsC3));

	for (size_t i = 0; i < kSirenIdsSites; i++)
	{
		SirenIdsSite& site = g_sirenIdsSites[i];
		site.placed = SirenIdsHash(kSirenIdsHashStart, site.bytes, site.size);
		for (size_t at = 0; at < site.size; at++)
			g_sirenIdsCopy[i][at] = site.bytes[at];
	}
	g_sirenIdsDataPlaced = SirenIdsData();
	for (size_t i = 0; i < sizeof(g_sirenIdsH3Placed); i++)
		g_sirenIdsH3Placed[i] = g_sirenIdsH3[i];
	for (size_t i = 0; i < sizeof(g_sirenIdsH4Placed); i++)
		g_sirenIdsH4Placed[i] = g_sirenIdsH4[i];
}

// The moved table as TweaksV left it, read before the game's init can fill it
static wchar_t g_sirenIdsMovedTable[96] = L"not moved";
static void SirenIdsNoteTable()
{
	const Site lea = { g_sirenIdsT1, 10, 0 };
	const unsigned char* const moved = Rel32Target(lea);
	if (moved == g_sirenIdsTable || InImage(moved) || ProtectionOf(moved) == 0 || ProtectionOf(moved) == PAGE_NOACCESS)	// not the part's: left alone
		return;
	bool copied = memcmp(moved, g_sirenIdsTable, sizeof(g_sirenIdsTableStorage)) == 0;
	bool rest = true;
	for (size_t i = sizeof(g_sirenIdsTableStorage); i < 65536; i++)
		rest = rest && moved[i] == 0xFF;
	swprintf(g_sirenIdsMovedTable, 96, L"moved, %ls, %ls, the rest %ls", ProtectionOf(moved) == PAGE_READWRITE && ProtectionOf(moved + 65535) == PAGE_READWRITE ? L"RW" : L"NOT RW",
		copied ? L"the game's 256 copied" : L"NOT the game's 256", rest ? L"0xFF" : L"NOT 0xFF");
}

// What the game does with its siren settings once TweaksV has started: the structures registered at once (siregister); the init
// and the base files, the packs (siload, sipacks, siflood); 12 s after TweaksV loaded - after the measurement's first summary - a
// pack's 6 settings appended with the IDs 40 to 45 (sipack), or the base files read (sinone), and the fake stays to 15 s, for the
// summary after it
static void SirenIdsLater()
{
	SirenIdsNoteTable();
	if (g_sirenIdsRegister)
	{
		SirenIdsRegister();
		g_sirenIdsDataPlaced = SirenIdsData();
	}
	SirenIdsReadCode();
	if (g_sirenIdsLoad)
	{
		SirenIdsInit();
		if (g_sirenIdsTwice)
			SirenIdsInit();
		if (g_sirenIdsPacks)
		{
			for (const SirenIdsFile& file : g_sirenIdsFiles)
				SirenIdsMountPack(file);
			const unsigned variations[] = { 300, 1534, 40, 511, 767 };
			for (const unsigned id : variations)
				SirenIdsVariation(id, false);
		}
		if (g_sirenIdsFlood)
		{
			const SirenIdsVariations none = { nullptr, 0 };
			for (int i = 0; i < 20001; i++)
			{
				SirenIdsMountPack(g_sirenIdsFiles[2]);
				SirenIdsVariationsCall(none, true);
			}
		}
		if (g_sirenIdsOptions.badFiles)
			for (const SirenIdsFile& file : g_sirenIdsBadFiles)
				SirenIdsMountPack(file);
		if (g_sirenIdsOptions.nameBad)
			SirenIdsMountPack(g_sirenIdsFiles[2], true);
		if (g_sirenIdsOtherObject)
			SirenIdsMountPack(g_sirenIdsFiles[2]);
		if (g_sirenIdsOptions.profile)
		{
			static SirenIdsFile profile = {};
			const DWORD length = GetEnvironmentVariableA("USERPROFILE", profile.name, 90);
			if (length > 0 && length < 90)
			{
				strcpy_s(profile.name + length, sizeof(profile.name) - length, "\\pack\\carcols.meta");
				SirenIdsMountPack(profile);
			}
		}
		if (g_sirenIdsOptions.variations)
		{
			static const SirenIdsVehicle vehicles[] = { { "sicar0", 1 }, { "sicar1", 777 }, { "sicar2", -1 }, { "sicar3", 13 }, { "sicar4", 2000 } };
			SirenIdsLoadVariations(vehicles, sizeof(vehicles) / sizeof(vehicles[0]));
		}
		if (g_sirenIdsOptions.varMany)
		{
			static char names[40][8];
			static SirenIdsVehicle vehicles[40];
			for (size_t i = 0; i < 40; i++)
			{
				sprintf_s(names[i], "sim%02zu", i);
				vehicles[i] = { names[i], 777 };
			}
			SirenIdsLoadVariations(vehicles, 40);
		}
		if (g_sirenIdsOptions.varBad)
		{
			const SirenIdsVariations none = { nullptr, 0 };
			SirenIdsVariationsCall(none, false);
		}
		if (g_sirenIdsOptions.varUnreadable)
		{
			const SirenIdsVariations unreadable = { g_sirenIdsNoAccess, 1 };
			SirenIdsVariationsCall(unreadable, true);
		}
		if (g_sirenIdsOptions.varNames)
		{
			// the end of a page whose next cannot be read: its last two bytes a name that runs into it
			const auto page = static_cast<unsigned char*>(VirtualAlloc(nullptr, 2 * 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
			DWORD old = 0;
			VirtualProtect(page + 4096, 4096, PAGE_NOACCESS, &old);
			memcpy(page + 4094, "ab", 2);
			const SirenIdsVehicle vehicles[] = { { reinterpret_cast<const char*>(g_sirenIdsNoAccess), 777 }, { reinterpret_cast<const char*>(page + 4094), 777 }, { "si\xE4r", 777 },
				{ "s123456789012345678901234567890123456789012345678901234567890123456789", 777 }, { "", 777 } };
			SirenIdsLoadVariations(vehicles, 5);
		}
		if (g_sirenIdsBigPack)
		{
			g_sirenIdsFileSettings = static_cast<unsigned char*>(VirtualAlloc(nullptr, g_sirenIdsBigFile.count * kSirenIdsSettingSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
			if (g_sirenIdsFileSettings)
				SirenIdsMountPack(g_sirenIdsBigFile);
		}
		g_sirenIdsDataPlaced = SirenIdsData();
	}
	if (!g_sirenIdsPack && !g_sirenIdsNone)
		return;
	Sleep(10500);
	if (g_sirenIdsPack)
	{
		for (size_t i = 24; i < 30; i++)
			SirenIdsGive(i, static_cast<unsigned>(16 + i));
		SirenIdsCount(30);
	}
	if (g_sirenIdsNone)
		g_sirenIdsPointer[0] = g_sirenIdsCarcols;
	g_sirenIdsDataPlaced = SirenIdsData();
	Sleep(3000);
}

// The sites' bytes as the part leaves them: its writes on each site's bytes as placed, as a text of their own here
struct SirenIdsWrite { volatile unsigned char* site; size_t offset; const char* bytes; };
static const SirenIdsWrite g_sirenIdsWrites[] = {
	{ g_sirenIdsT1, 0x93, "0F B7 2C 0F" }, { g_sirenIdsT1, 0xC4, "0F B7 1C 2B" },
	{ g_sirenIdsT2, 0x29, "66 89 31" }, { g_sirenIdsT2, 0x30, "0F B7 2C 39" }, { g_sirenIdsT2, 0x4B, "0F B7 2C 39" },
	{ g_sirenIdsT2, 0x85, "48 81 FE 00 00 01 00" }, { g_sirenIdsT2, 0xCD, "48 81 FE 00 00 01 00" },
	{ g_sirenIdsT3, 0x8D, "0F B7 3C 03" }, { g_sirenIdsT3, 0xBE, "0F B7 14 17" },
	{ g_sirenIdsT4, 0x21, "66 89 33" }, { g_sirenIdsT4, 0x28, "0F B7 0C 3B" }, { g_sirenIdsT4, 0x42, "0F B7 34 3B" },
	{ g_sirenIdsT4, 0x81, "48 81 FE 00 00 01 00" }, { g_sirenIdsT4, 0xC9, "48 81 FE 00 00 01 00" },
	{ g_sirenIdsD1, 0x0B, "43 0F B7 54 3D 00" },
	{ g_sirenIdsD2, 2, "66 39 54 35 00 0F 84 18 00 00 00 48 81 C6 E8 05 00 00 48 39 F3 75 E9 66 0F 1F 84 00 00 00 00 00" },
	{ g_sirenIdsC1, kSirenIdsC1At + 8, "66 C7 01 FF 00 66 90" },
	{ g_sirenIdsC2, 0, "83 61 30 00 48 C7 41 38 00 00 00 00 83 61 40 00 C7 41 48 00 00 FF FF C6 41 4C 00 C3 90 90" },
	{ g_sirenIdsC3, 0, "83 60 30 00 48 C7 40 38 00 00 00 00 83 60 40 00 C7 40 48 00 00 FF FF C6 40 4C 00 48 83 C4 28 C3 90 90" },
};

// "siren writes: 17 of 17 as the part leaves them, a pack's dedupe's load a word's, 0 hooks, ...": each site's bytes against what the
// part must leave, the lea to the moved table, the call's rel32 and a hook's jump apart
static void SirenIdsReportWrites()
{
	const Site firstLea = { g_sirenIdsT1, 10, 0 };
	const unsigned char* const moved = Rel32Target(firstLea);
	int asWritten = 0, hooks = 0;
	const wchar_t* dedupe = L"as placed";
	for (size_t s = 0; s < kSirenIdsSites; s++)
	{
		const SirenIdsSite& site = g_sirenIdsSites[s];
		unsigned char expected[0xE0];
		memcpy(expected, g_sirenIdsCopy[s], site.size);
		const auto is = [&site](const volatile unsigned char* bytes) { return site.bytes == bytes; };
		for (const SirenIdsWrite& write : g_sirenIdsWrites)
			if (write.site == site.bytes)
				SirenIdsPut(expected, write.offset, write.bytes);
		const auto lea = [&](int offset)
		{
			const int32_t rel = static_cast<int32_t>(moved - const_cast<const unsigned char*>(site.bytes + offset + 4));
			memcpy(expected + offset, &rel, sizeof(rel));
		};
		if (is(g_sirenIdsT1) || is(g_sirenIdsT3))
			lea(10);
		if (is(g_sirenIdsT2))
			lea(23);
		if (is(g_sirenIdsT4))
			lea(16);
		if (is(g_sirenIdsR1))
			for (int i = 14; i < 18; i++)
				expected[i] = site.bytes[i];
		if ((is(g_sirenIdsH1) || is(g_sirenIdsH2)) && site.bytes[1] == 0xE9)	// the jump of a hook, in the first instructions
		{
			hooks++;
			for (int i = 1; i < 7; i++)
				expected[i] = site.bytes[i];
		}
		if (is(g_sirenIdsD1))
		{
			if (site.bytes[0x0B] == 0xE9 && site.bytes[0x10] == 0x90)
			{
				for (int i = 0x0B; i < 0x11; i++)
					expected[i] = site.bytes[i];
				const Site toStub = { g_sirenIdsD1, 0x0C, 0 };
				const unsigned char* const stub = Rel32Target(toStub);
				const Site back = { const_cast<unsigned char*>(stub), 42, 0 };
				dedupe = !InImage(stub) && ProtectionOf(stub) == PAGE_EXECUTE_READ && stub[0] == 0x43 && stub[1] == 0x0F && stub[2] == 0xB7 && stub[41] == 0xE9 &&
					Rel32Target(back) == const_cast<const unsigned char*>(g_sirenIdsD1 + 0x11) ? L"through a stub, RX" : L"through a stub NOT as it must be";
			}
			else
				dedupe = site.bytes[0x0D] == 0xB7 ? L"a word's" : L"as placed";
		}
		bool same = true;
		for (size_t i = 0; i < site.size; i++)
			same = same && site.bytes[i] == expected[i];
		if (same)
			asWritten++;
	}
	const Site call = { g_sirenIdsR1, 14, 0 };
	const unsigned char* const target = Rel32Target(call);
	const wchar_t* const leads = target == reinterpret_cast<const unsigned char*>(&SirenIdsBaseLoader) ? L"to the loader"
		: !InImage(target) && ProtectionOf(target) == PAGE_EXECUTE_READ && memcmp(target, "\xFF\x25\x00\x00\x00\x00", 6) == 0 ? L"to a stub, RX" : L"elsewhere";
	wprintf(L"siren writes: %d of %d as the part leaves them, a pack's dedupe's load %ls, %d hooks, types 0x%X and 0x%X, member data %ls, table %ls, base loader call %ls | ", asWritten,
		static_cast<int>(kSirenIdsSites), dedupe, hooks, g_sirenIdsMembers[0].type, g_sirenIdsMembers[26].type,
		ProtectionOf(g_sirenIdsMembers) == PAGE_READWRITE ? L"RW" : L"NOT RW", g_sirenIdsMovedTable, leads);
}

// "siren code: loads and stores 11 words 0 bytes, loop words, constructors 3 words": what the sites do, run as code
static void SirenIdsReportCode()
{
	// a load: the value at an address two registers give, into a third; a store: a register's value at an address
	struct LoadSite { volatile unsigned char* site; size_t offset; uint64_t SirenIdsRegs::*base; uint64_t SirenIdsRegs::*index; uint64_t SirenIdsRegs::*result; };
	const LoadSite loads[] = {
		{ g_sirenIdsT1, 0x93, &SirenIdsRegs::rdi, &SirenIdsRegs::rcx, &SirenIdsRegs::rbp },
		{ g_sirenIdsT1, 0xC4, &SirenIdsRegs::rbx, &SirenIdsRegs::rbp, &SirenIdsRegs::rbx },
		{ g_sirenIdsT2, 0x30, &SirenIdsRegs::rcx, &SirenIdsRegs::rdi, &SirenIdsRegs::rbp },
		{ g_sirenIdsT2, 0x4B, &SirenIdsRegs::rcx, &SirenIdsRegs::rdi, &SirenIdsRegs::rbp },
		{ g_sirenIdsT3, 0x8D, &SirenIdsRegs::rbx, &SirenIdsRegs::rax, &SirenIdsRegs::rdi },
		{ g_sirenIdsT3, 0xBE, &SirenIdsRegs::rdi, &SirenIdsRegs::rdx, &SirenIdsRegs::rdx },
		{ g_sirenIdsT4, 0x28, &SirenIdsRegs::rbx, &SirenIdsRegs::rdi, &SirenIdsRegs::rcx },
		{ g_sirenIdsT4, 0x42, &SirenIdsRegs::rbx, &SirenIdsRegs::rdi, &SirenIdsRegs::rsi },
		{ g_sirenIdsD1, 0x0B, &SirenIdsRegs::r13, &SirenIdsRegs::r15, &SirenIdsRegs::rdx },
	};
	unsigned char value[0x60] = {};
	value[0] = 0xFE;
	value[1] = 0x05;
	unsigned char out[0x60];
	int words = 0, bytes = 0;
	for (const LoadSite& load : loads)
	{
		SirenIdsRegs regs{};
		regs.*load.base = reinterpret_cast<uint64_t>(value);
		regs.*load.index = 0;
		if (load.site == g_sirenIdsT1 && load.offset == 0xC4)	// its byte store into the variation, which stays: its own address
		{
			regs.rdi = reinterpret_cast<uint64_t>(out);
			regs.rcx = 0;
		}
		if (load.site == g_sirenIdsT3 && load.offset == 0xBE)
		{
			regs.rbx = reinterpret_cast<uint64_t>(out);
			regs.rax = 0;
		}
		if (load.site == g_sirenIdsD1)
			regs.rbx = reinterpret_cast<uint64_t>(value);
		SirenIdsRunAt(regs, load.site + load.offset);
		if (regs.*load.result == 0x05FE)
			words++;
		else if (regs.*load.result == 0xFE)
			bytes++;
	}
	struct StoreSite { volatile unsigned char* site; size_t offset; uint64_t SirenIdsRegs::*address; };
	const StoreSite stores[] = { { g_sirenIdsT2, 0x29, &SirenIdsRegs::rcx }, { g_sirenIdsT4, 0x21, &SirenIdsRegs::rbx } };
	for (const StoreSite& store : stores)
	{
		out[0] = out[1] = 0xAA;
		SirenIdsRegs regs{};
		regs.rsi = 0x05FE;
		regs.*store.address = reinterpret_cast<uint64_t>(out);
		SirenIdsRunAt(regs, store.site + store.offset);
		if (out[0] == 0xFE && out[1] == 0x05)
			words++;
		else if (out[0] == 0xFE && out[1] == 0xAA)
			bytes++;
	}
	// the compare loop over three settings: an ID found by its whole word, or by its low byte
	unsigned char settings[3 * kSirenIdsSettingSize] = {};
	settings[0] = 0x05; settings[1] = 0x01;
	settings[kSirenIdsSettingSize] = 0x05; settings[kSirenIdsSettingSize + 1] = 0x02;
	settings[2 * kSirenIdsSettingSize] = 0x05; settings[2 * kSirenIdsSettingSize + 1] = 0x03;
	const auto found = [&settings](uint64_t id)
	{
		SirenIdsRegs regs{};
		regs.rdx = id;
		regs.rbp = reinterpret_cast<uint64_t>(settings);
		regs.rbx = sizeof(settings);
		SirenIdsRunAt(regs, g_sirenIdsD2);
		return regs.rax;
	};
	const Site jz = { g_sirenIdsD2, g_sirenIdsD2[16] == 0x0F ? 18 : 9, 0 };	// a jz that leads out of the game (sijz) is not run
	const bool runs = Rel32Target(jz) == const_cast<const unsigned char*>(g_sirenIdsD2 + 37);
	const bool loopWords = runs && found(0x0305) == 1 && found(0x0205) == 1 && found(0x0105) == 1 && found(0x0405) == 0 && found(0x0005) == 0;
	const bool loopBytes = runs && found(0x0005) == 1 && found(0x0006) == 0;
	// the constructors: the ID's default as a word, or a byte with nothing written after it
	unsigned char setting[kSirenIdsSettingSize];
	memset(setting, 0xCC, sizeof(setting));
	SirenIdsMakeSetting(setting);
	int constructorWords = setting[0] == 0xFF && setting[1] == 0x00 && setting[0x1C] == 0x83 && setting[0x1D] == 0 ? 1 : 0;
	int constructorBytes = setting[0] == 0xFF && setting[1] == 0xCC && setting[0x1C] == 0x83 && setting[0x1D] == 0 ? 1 : 0;
	const bool ways[] = { true, false };
	for (const bool inPlace : ways)
	{
		unsigned char variation[0x50];
		memset(variation, 0xCC, sizeof(variation));
		SirenIdsMakeVariation(variation, inPlace);
		bool others = true;
		for (size_t i = 0x30; i < 0x34; i++)
			others = others && variation[i] == 0;
		for (size_t i = 0x38; i < 0x44; i++)
			others = others && variation[i] == 0;
		others = others && variation[0x34] == 0xCC && variation[0x44] == 0xCC && variation[0x48] == 0 && variation[0x49] == 0 && variation[0x4A] == 0xFF &&
			variation[0x4B] == 0xFF && variation[0x4D] == 0xCC;
		if (others && variation[0x4C] == 0)
			constructorWords++;
		else if (others && variation[0x4C] == 0xCC)
			constructorBytes++;
	}
	wprintf(L"siren code: loads and stores %d words %d bytes, loop %ls, constructors %d words %d bytes | ", words, bytes, !runs ? L"not run" : loopWords ? L"words" : loopBytes ? L"bytes" : L"WRONG",
		constructorWords, constructorBytes);
}

// "siren ids: sites as placed 17 of 17, data as placed | ": TweaksV wrote nothing into the sites or the data; its writes and what
// they do after it; with siload what the game's load found
static void SirenIdsReport()
{
	int asPlaced = 0;
	for (const SirenIdsSite& site : g_sirenIdsSites)
	{
		if (SirenIdsHash(kSirenIdsHashStart, site.bytes, site.size) == site.placed)
			asPlaced++;
		CheckData(site.bytes, site.size);
	}
	const bool typesPlaced = g_sirenIdsMembers[0].type == g_sirenIdsTypesPlaced[0] && g_sirenIdsMembers[26].type == g_sirenIdsTypesPlaced[1];
	const bool typesWidened = g_sirenIdsMembers[0].type == g_sirenIdsTypesPlaced[0] + 2 && g_sirenIdsMembers[26].type == g_sirenIdsTypesPlaced[1] + 2;
	wprintf(L"siren ids: sites as placed %d of %d, data %ls | ", asPlaced, static_cast<int>(kSirenIdsSites),
		SirenIdsData() != g_sirenIdsDataPlaced ? L"NOT as placed" : typesPlaced ? L"as placed" : typesWidened ? L"widened" : L"types CHANGED");
	wprintf(L"siren array: 0x%llX | ", static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(g_sirenIdsSettings)));
	// "siren files: the call as placed, the callback as placed, opened 0, ...": I-E-H3's site but its call's rel32, and I-E-H4's but a hook's jump;
	// where the call leads, a stub's not in the other parts' count; the file opener's calls and the carvariations callback's
	bool h3Same = true, h4Same = true;
	for (size_t i = 0; i < sizeof(g_sirenIdsH3Placed); i++)
		if (g_sirenIdsH3[i] != g_sirenIdsH3Placed[i] && (i < 26 + 47 || i >= 26 + 51))
			h3Same = false;
	const bool h4Hooked = g_sirenIdsH4[1] == 0xE9;
	for (size_t i = h4Hooked ? 7 : 0; i < sizeof(g_sirenIdsH4Placed); i++)
		if (g_sirenIdsH4[i] != g_sirenIdsH4Placed[i])
			h4Same = false;
	const Site fileCall = { g_sirenIdsH3, 26 + 47, 0 };
	const unsigned char* const fileTarget = Rel32Target(fileCall);
	const bool toOpener = fileTarget == reinterpret_cast<const unsigned char*>(&SirenIdsOpenFile);
	const bool toStub = !toOpener && !InImage(fileTarget) && ProtectionOf(fileTarget) == PAGE_EXECUTE_READ && memcmp(fileTarget, "\xFF\x25\x00\x00\x00\x00", 6) == 0;
	CheckData(g_sirenIdsH3, sizeof(g_sirenIdsH3));
	CheckData(g_sirenIdsH4, sizeof(g_sirenIdsH4));
	wprintf(L"siren files: the call %ls, the callback %ls, opened %d, with their arguments %d, back %d; variations back %d of %d, bodies %d | ",
		!h3Same ? L"CHANGED" : toOpener ? L"as placed" : toStub ? L"to a stub, RX" : L"elsewhere", !h4Same ? L"CHANGED" : h4Hooked ? L"hooked" : L"as placed", g_sirenIdsOpened,
		g_sirenIdsOpenedAsHanded, g_sirenIdsOpenedKept, g_sirenIdsVariationsKept, g_sirenIdsVariationCalls, g_sirenIdsVariationBodies);
	SirenIdsReportWrites();
	SirenIdsReportCode();
	if (g_sirenIdsLoad)
		wprintf(L"siren load: %d inits, loader %d calls, %d with its table free, %d with their arguments, %d of them back with result and last error, builder %d back, "
			L"packs %d of %d back, dedupe's registers kept %d of %d, variations %d of %d | ", g_sirenIdsInits, g_sirenIdsLoaderCalls, g_sirenIdsTableFree, g_sirenIdsLoaderArguments, g_sirenIdsInitKept,
			g_sirenIdsBuilderKept, g_sirenIdsPackKept, g_sirenIdsPackCalls, g_sirenIdsDedupeKept, g_sirenIdsDedupeLoads, g_sirenIdsVariationsRight, g_sirenIdsVariations);
}

// The siren count: the game's siren lights' renderer and find-bone-by-name, and the vehicle models the fake draws. The two
// functions are carriers - compiled functions of the fake's own, which its unwind data names - that SirenCountSetup rewrites
// before TweaksV loads: the renderer into a jump on to SirenCountRenderer with its read of a siren's bone (I-E-B2) behind it,
// find-bone-by-name into the game's prologue with its test of the bone count (I-E-B4), undone at once, and a jump on to
// SirenCountFindBone - so that TweaksV checks both by the unwind data, as it does the game's, and neither is on the stack while
// the fake's own code runs. The models lie at the game's offsets: the hash of a model's name at +0x18, the holder of its bone
// structure at +0xB0, the bone structure's signed byte at 0x10 + a hierarchy ID, the skeleton data at the skeleton's +0x00, its
// bone count at +0x5E, and a bone's index at +0x42. Their bone structures have the room the part gives them in the game, filled
// from the start (TweaksVSirenRoom.h), so that the second call's scenarios need no fill, whose own proof is on the merged tree.
static void SetRel32(const Site& site, const void* target);
static const unsigned char* Rel32Target(const Site& site);
static const unsigned char* ImageEnd();
static bool InImage(const void* p);

struct SirenCountModel
{
	unsigned char head[0x18];
	uint32_t hash;							// +0x18: the hash of the model's name
	unsigned char middle[0xB0 - 0x1C];
	const void* const* bones;				// +0xB0: the holder of its bone structure
	unsigned char tail[0x53B - 0xB8];
	unsigned char position;					// +0x53B: its siren setting's position in the game's array, 0 for none
	unsigned char end[4];
};
struct SirenCountBones
{
	signed char ids[0x34C];					// at 0x10 + a hierarchy ID, the index of its bone; -1 for none
	signed char room[2][32];				// +0x34C siren1's bone to siren32's, +0x36C siren_glass1's to siren_glass32's; -1 for none
};
static_assert(offsetof(SirenCountBones, room) == 0x34C && sizeof(SirenCountBones::room[0]) == 0x20 && sizeof(SirenCountBones) == 0x38C,
	"the bone structure with its room, as the part raises it");
struct SirenCountBone
{
	char name[0x42];
	uint16_t index;							// +0x42
};
struct SirenCountSkeletonData
{
	unsigned char head[0x5E];
	uint16_t bones;							// +0x5E: the bone count
	uint32_t named;							// the fake's own from here on: its bones with a name it looks up
	const SirenCountBone* list;
};
struct SirenCountSkeleton
{
	const SirenCountSkeletonData* data;		// +0x00: the skeleton data
	unsigned char rest[0x18];
};
static_assert(offsetof(SirenCountModel, hash) == 0x18 && offsetof(SirenCountModel, bones) == 0xB0 && offsetof(SirenCountModel, position) == 0x53B && offsetof(SirenCountBone, index) == 0x42 &&
	offsetof(SirenCountSkeletonData, bones) == 0x5E && offsetof(SirenCountSkeleton, data) == 0, "the siren count's models lie at the game's offsets");

// A vehicle at the game's offsets: all the renderer reads of it are its +0x48, +0x6C, +0x80, +0xDC, +0x860, +0x986 and the state's pointer at +0x1380.
struct SirenCountCar
{
	unsigned char bytes[0x1388];
};
constexpr size_t kSirenCarStateAt = 0x1380;

// A vehicle model of the fake's, with what TweaksV reaches from it, and a vehicle of it, last, since the game's set-up gives it its state later.
struct SirenCountVehicle
{
	SirenCountModel model;
	const void* holder;
	SirenCountBones structure;
	SirenCountSkeletonData data;
	SirenCountSkeleton skeleton;
	SirenCountBone list[64];
	SirenCountCar car;
};
enum
{
	kSirenCountPolice, kSirenCount32, kSirenCountLost, kSirenCount8,	// what scdraw draws
	kSirenCountNoStructure, kSirenCountNoSkeleton, kSirenCountOdd,		// and with scodd
	kSirenCountMore32, kSirenCountMore16,								// scmore, later
	kSirenCountFault,													// scfault
	kSirenCount21, kSirenCountHoles, kSirenCountShare, kSirenCountReplaced, kSirenCountRace,	// srdraw, with those it draws too
	kSirenCountJoined,													// sfjoin
	kSirenCountNoSetting, kSirenCountPast, kSirenCountSpare,			// and at position 0, 255 and past the count
	kSirenCountVehicles
};
constexpr int kSirenCountMany = 1100;		// scfull: models past what TweaksV's table of 1,024 holds, each the police car's but its hash
static SirenCountVehicle g_sirenCountVehicleStorage[kSirenCountVehicles];
static SirenCountVehicle* volatile g_sirenCountVehicles = g_sirenCountVehicleStorage;
static SirenCountModel g_sirenCountManyStorage[kSirenCountMany];
static SirenCountModel* volatile g_sirenCountMany = g_sirenCountManyStorage;

// A model: the hash of its name, its skeleton's bones, and its siren and glass bones by name - siren1 to siren<sirens> at the
// bones from firstSiren on, siren_glass1 to siren_glass<glass> from firstGlass on. Its bone structure names those of siren1 to
// siren20 - hierarchy IDs 0x89 to 0x9C, their glass 0x9D to 0xB0 - whose index a signed byte holds, as the game's does, and its
// room all of them.
static void SirenCountMake(SirenCountVehicle& v, uint32_t hash, uint16_t bones, int sirens, int firstSiren, int glass, int firstGlass)
{
	memset(&v, 0, sizeof(v));
	memset(&v.structure, 0xFF, sizeof(v.structure));
	v.model.hash = hash;
	v.holder = &v.structure;
	v.model.bones = &v.holder;
	v.data.bones = bones;
	v.data.list = v.list;
	v.skeleton.data = &v.data;
	const struct { const char* name; int count; int first; int id; int room; } kinds[2] = { { "siren", sirens, firstSiren, 0x88, 0 },
		{ "siren_glass", glass, firstGlass, 0x9C, 1 } };
	for (const auto& kind : kinds)
		for (int n = 1; n <= kind.count; n++)
		{
			SirenCountBone& bone = v.list[v.data.named++];
			snprintf(bone.name, sizeof(bone.name), "%s%d", kind.name, n);
			bone.index = static_cast<uint16_t>(kind.first + n - 1);
			if (bone.index >= 128)
				continue;
			v.structure.room[kind.room][n - 1] = static_cast<signed char>(bone.index);
			if (n <= 20)
				v.structure.ids[0x10 + kind.id + n] = static_cast<signed char>(bone.index);
		}
	// its car's fields, each its own by the model's hash
	const uint64_t pointer = 0x1111000000000000ull | hash;
	memcpy(v.car.bytes + 0x48, &pointer, sizeof(pointer));
	const uint16_t kindWord = static_cast<uint16_t>(hash);
	memcpy(v.car.bytes + 0x6C, &kindWord, sizeof(kindWord));
	for (int i = 0; i < 16; i++)
		v.car.bytes[0x80 + i] = static_cast<unsigned char>((hash >> (i % 4 * 8)) + i);
	const uint16_t time = static_cast<uint16_t>(hash >> 16);
	memcpy(v.car.bytes + 0xDC, &time, sizeof(time));
	v.car.bytes[0x986] = static_cast<unsigned char>((hash >> 24) | 1);
}

// What the fake hands the call of the renderer, and the result and last error the renderer leaves; each thread draws on its own,
// so what it handed the call is its own.
using SirenCountLights = uint64_t (*)(void* vehicle, float a, float b, float c, uint64_t flag, const unsigned char* model,
	const unsigned char* skeleton, uint64_t value, void* coronas);
constexpr float kSirenCountFloats[3] = { 1.25f, 2.5f, 3.75f };
constexpr uint64_t kSirenCountFlag = 0xA5A5A5A5A5A5A501ull;		// a flag byte, and the rest of its slot as the game leaves it
constexpr uint64_t kSirenCountValue = 0x0123456789ABCDEFull;
constexpr uint64_t kSirenCountResult = 0x51E7C0DE0000F00Dull;
constexpr DWORD kSirenCountLastError = 0x5157;
static unsigned char g_sirenCountCoronas[0x30];
static thread_local const void* g_sirenCountExpectedVehicle = nullptr;
static thread_local const void* g_sirenCountExpectedModel = nullptr;
static thread_local const void* g_sirenCountExpectedSkeleton = nullptr;
static thread_local const void* g_sirenCountDrawnData = nullptr;	// the skeleton data of the vehicle the renderer drew last
static volatile LONG g_sirenCountDrawn = 0, g_sirenCountKept = 0, g_sirenCountRendered = 0, g_sirenCountArgsBad = 0, g_sirenCountLookups = 0,
	g_sirenCountOutOfTurn = 0;

// The sirens per vehicle's lights in the renderer, once srdraw has set them up: what the file's items say, what the renderer's loop finds, what it counts.
static bool g_sirenRaiseOn = false;
static volatile unsigned char* g_sirenCountRendererGetter = nullptr;	// the rel32 of the renderer's call of the getter (I-E-Z1)
static const unsigned char* g_sirenRaiseSettings[kSirenCountVehicles] = {};	// the setting the parser gave each model, a copy as the game's array holds it
static uint32_t g_sirenRaisePastSeed[kSirenCountVehicles] = {};		// the seed its lights past 20 are made of; 0: any
static volatile LONG g_sirenRaiseLit[kSirenCountVehicles][2] = {};	// the lights of each call of the last pass
static volatile LONG g_sirenRaiseSecond = 0, g_sirenRaiseStandInBad = 0, g_sirenRaiseBlockBad = 0, g_sirenRaiseOwnBad = 0, g_sirenRaiseLightBad = 0, g_sirenRaiseMade = 0,
	g_sirenRaiseDirty = 0, g_sirenRaiseTorn = 0, g_sirenRaiseDefault = 0, g_sirenRaiseGetterBad = 0, g_sirenRaiseEmpty = 0;
constexpr size_t kSirenStateSize = 0x188, kSirenSecondStateAt = 0x190, kSirenStandInAt = 0x320, kSirenStandInSize = sizeof(SirenCountCar);

template <typename T> static T SirenField(const unsigned char* at, size_t offset)
{
	T value{};
	memcpy(&value, at + offset, sizeof(T));
	return value;
}

// Which of the fake's models this is, or -1
static int SirenRaiseIndexOf(const void* model)
{
	const intptr_t at = static_cast<const unsigned char*>(model) - reinterpret_cast<const unsigned char*>(g_sirenCountVehicles);
	if (at < 0 || at >= static_cast<intptr_t>(sizeof(g_sirenCountVehicleStorage)) || at % sizeof(SirenCountVehicle) != 0)
		return -1;
	return static_cast<int>(at / sizeof(SirenCountVehicle));
}

// A file's item: its rotation and flashiness, each with a delta and a start that floats add without rounding, its direction reversed or not, and a colour
// that says which item of which file it was.
static float SirenItemDelta(int item, int sequence) { return (sequence ? 0.5f : 0.25f) * static_cast<float>(item % 4 + 1); }
static float SirenItemStart(uint32_t seed, int item, int sequence) { return static_cast<float>((seed * 3 + item + sequence) % 16); }
static bool SirenItemReversed(int item, int sequence) { return (item + sequence) % 3 == 0; }
static void SirenMakeItem(unsigned char* light, uint32_t seed, int item)
{
	memset(light, 0, 0x48);
	for (int sequence = 0; sequence < 2; sequence++)
	{
		unsigned char* const at = light + (sequence ? 0x14 : 0x00);
		const float delta = SirenItemDelta(item, sequence), start = SirenItemStart(seed, item, sequence);
		memcpy(at, &delta, sizeof(delta));
		memcpy(at + 4, &start, sizeof(start));
		at[0x11] = SirenItemReversed(item, sequence) ? 1 : 0;
	}
	const uint32_t colour = 0xFF000000u | (seed << 8) | static_cast<uint32_t>(item);
	memcpy(light + 0x38, &colour, sizeof(colour));
}

// One light of the renderer's loop, at its place i: the value the game's renderer gets for it must be its file's item's at the item's true index.
static void SirenRaiseCheckLight(const unsigned char* light, int i, bool second, uint32_t pastSeed)
{
	const uint32_t colour = SirenField<uint32_t>(light, 0x38);
	if (colour == 0)	// a light of an own setting no file's item filled, as the constructor leaves it in the fake
	{
		InterlockedIncrement(&g_sirenRaiseDefault);
		return;
	}
	const int item = static_cast<int>(colour & 0xFF);
	const uint32_t seed = (colour >> 8) & 0xFFFF;
	bool right = item == (second ? 20 + i : i) && (!second || !pastSeed || seed == pastSeed);
	for (int sequence = 0; sequence < 2; sequence++)
	{
		const unsigned char* const at = light + (sequence ? 0x14 : 0x00);
		const float delta = SirenField<float>(at, 0), start = SirenField<float>(at, 4);
		const float value = start + delta * static_cast<float>(at[0x11] ? 20 - i : i);
		const float truth = SirenItemStart(seed, item, sequence) + SirenItemDelta(item, sequence) * static_cast<float>(SirenItemReversed(item, sequence) ? 20 - item : item);
		if (value != truth || delta != SirenItemDelta(item, sequence))
			right = false;
	}
	if (!right)
		InterlockedIncrement(&g_sirenRaiseLightBad);
}

// The stand-in: the vehicle's seven fields, its bits past 20 from 0 on, the second state; zero in all else, its matrix 16-aligned.
static bool SirenRaiseStandInOk(const unsigned char* standIn, const unsigned char* car, const unsigned char* state)
{
	if (memcmp(standIn + 0x48, car + 0x48, 8) != 0 || memcmp(standIn + 0x6C, car + 0x6C, 2) != 0 || memcmp(standIn + 0x80, car + 0x80, 16) != 0 ||
		memcmp(standIn + 0xDC, car + 0xDC, 2) != 0 || standIn[0x986] != car[0x986] || SirenField<uint32_t>(standIn, 0x860) != SirenField<uint32_t>(car, 0x860) >> 20 ||
		SirenField<const unsigned char*>(standIn, kSirenCarStateAt) != state + kSirenSecondStateAt || reinterpret_cast<uintptr_t>(standIn + 0x80) % 16 != 0)
		return false;
	for (size_t i = 0; i < kSirenStandInSize; i++)
	{
		const bool field = (i >= 0x48 && i < 0x50) || (i >= 0x6C && i < 0x6E) || (i >= 0x80 && i < 0x90) || (i >= 0xDC && i < 0xDE) || (i >= 0x860 && i < 0x864) || i == 0x986 || i >= kSirenCarStateAt;
		if (!field && standIn[i] != 0)
			return false;
	}
	return true;
}

// The block in the model's place: its bone structure's +0x99 + k the bone of siren(21 + k) in the model's room, all else -1.
static bool SirenRaiseBlockOk(const unsigned char* block, const SirenCountVehicle& v)
{
	const auto holder = SirenField<const signed char* const*>(block, 0xB0);
	const signed char* const bones = holder ? *holder : nullptr;
	if (!bones || bones[0x98] != -1 || bones[0x10] != -1)
		return false;
	for (int k = 0; k < 20; k++)
		if (bones[0x99 + k] != -1 && (k >= 12 || bones[0x99 + k] != v.structure.room[0][20 + k]))
			return false;
	return true;
}

// The own setting the getter gave a block: made by the game's constructor, the real setting's scalars, its head and tail lights zero - the two models
// that share one setting differ in their scalars, the last one's standing.
static bool SirenRaiseOwnOk(const unsigned char* own, const unsigned char* real, bool shared)
{
	static const unsigned char kZero[0x14] = {};
	if (own[0] != 0xFF || memcmp(own + 0x24, kZero, sizeof(kZero)) != 0)
		return false;
	return shared || (memcmp(own + 4, real + 4, 0x20) == 0 && own[0x38] == real[0x38]);
}

// What TweaksV gives a position past the game's settings: read only, zero but for sequencerBpm, 200.
static bool SirenRaiseIsEmpty(const unsigned char* setting)
{
	static const float kBpm = 200.0f;
	for (size_t i = 0; i < 0x5E8; i++)
		if (setting[i] != (i >= 0x20 && i < 0x24 ? reinterpret_cast<const unsigned char*>(&kBpm)[i - 0x20] : 0))
			return false;
	return ProtectionOf(setting) == PAGE_READONLY;
}

// What a getter's call must give a model: none at position 0, the array's entry below the count, the empty setting past it. Past it, it comes only from TweaksV.
static bool SirenRaiseGetterRight(int index, const unsigned char* setting)
{
	const int position = g_sirenCountVehicles[index].model.position;
	if (position == 0)
		return setting == nullptr;
	const unsigned char* const carcols = g_sirenIdsPointer[0];
	if (!carcols || position >= static_cast<int>(SirenField<uint16_t>(carcols, 0x50)))
		return setting && SirenRaiseIsEmpty(setting);
	return setting == SirenField<const unsigned char*>(carcols, 0x48) + position * 0x5E8;
}

// The renderer's loop, as the game's runs: the setting through the getter's call at its site, 20 lights, each that is not broken and has a bone.
static void SirenRaiseLights(const unsigned char* vehicle, const unsigned char* model, bool second)
{
	const int index = SirenRaiseIndexOf(g_sirenCountExpectedModel);
	const auto getter = reinterpret_cast<const unsigned char* (*)(const void*)>(const_cast<unsigned char*>(Rel32Target(Site{ g_sirenCountRendererGetter, 0, 0 })));
	const unsigned char* const setting = getter(model);
	if (!second && index >= 0)
	{
		if (!SirenRaiseGetterRight(index, setting))
			InterlockedIncrement(&g_sirenRaiseGetterBad);
		if (setting && SirenRaiseIsEmpty(setting))	// it lights nothing, whatever the bones say
		{
			InterlockedIncrement(&g_sirenRaiseEmpty);
			return;
		}
	}
	unsigned char* const state = SirenField<unsigned char*>(vehicle, kSirenCarStateAt);
	const auto holder = SirenField<const signed char* const*>(model, 0xB0);
	const signed char* const bones = holder ? *holder : nullptr;
	if (!setting || !state || !bones || index < 0)
		return;
	if (second)
	{
		const SirenCountVehicle& v = g_sirenCountVehicles[index];
		const unsigned char* const first = SirenField<const unsigned char*>(v.car.bytes, kSirenCarStateAt);
		InterlockedIncrement(&g_sirenRaiseSecond);
		if (!SirenRaiseStandInOk(vehicle, v.car.bytes, first))
			InterlockedIncrement(&g_sirenRaiseStandInBad);
		if (!SirenRaiseBlockOk(model, v))
			InterlockedIncrement(&g_sirenRaiseBlockBad);
		if (!SirenRaiseOwnOk(setting, g_sirenRaiseSettings[index], index == kSirenCount32 || index == kSirenCountShare))
			InterlockedIncrement(&g_sirenRaiseOwnBad);
		if (SirenField<int32_t>(state, 8) == -1)	// made anew since the last call: as the game's constructor leaves a state
		{
			InterlockedIncrement(&g_sirenRaiseMade);
			bool clear = SirenField<int32_t>(state, 0) == SirenField<int32_t>(first, 0);
			for (size_t i = 0xC; i < kSirenStateSize; i++)
				if (state[i] != 0)
					clear = false;
			if (!clear)
				InterlockedIncrement(&g_sirenRaiseDirty);
		}
		uint32_t seed = 0;
		for (int i = 0; i < 12; i++)	// a setting published whole has one seed in all its lights
		{
			const uint32_t colour = SirenField<uint32_t>(setting, 0x3C + i * 0x48 + 0x38);
			if (colour && !seed)
				seed = (colour >> 8) & 0xFFFF;
			if (colour && ((colour >> 8) & 0xFFFF) != seed)
			{
				InterlockedIncrement(&g_sirenRaiseTorn);
				break;
			}
		}
	}
	const uint32_t broken = SirenField<uint32_t>(vehicle, 0x860);
	uint32_t lit = 0;
	LONG lights = 0;
	for (int i = 0; i < 20; i++)
	{
		if (((broken >> i) & 1) != 0 || bones[0x99 + i] < 0)
			continue;
		lights++;
		lit |= 1u << i;
		SirenRaiseCheckLight(setting + 0x3C + i * 0x48, i, second, g_sirenRaisePastSeed[index]);
		memcpy(state + 0x0C + i * 4, &lights, sizeof(lights));
	}
	memcpy(state + 0x17C, &lit, sizeof(lit));
	const int32_t beat = 7;
	memcpy(state + 8, &beat, sizeof(beat));
	InterlockedExchange(&g_sirenRaiseLit[index][second ? 1 : 0], lights);
}

// The game's renderer, behind the jump in its carrier: it checks that it got what the fake handed the call, or for the part's second call its stand-ins, notes the
// vehicle it drew for the lookups that follow, runs its loop once srdraw has set it up, and leaves a result and a last error of its own, of each call its own.
static uint64_t SirenCountRenderer(void* vehicle, float a, float b, float c, uint64_t flag, const unsigned char* model,
	const unsigned char* skeleton, uint64_t value, void* coronas)
{
	InterlockedIncrement(&g_sirenCountRendered);
	const unsigned char* const firstState = g_sirenCountExpectedVehicle ? SirenField<const unsigned char*>(static_cast<const unsigned char*>(g_sirenCountExpectedVehicle), kSirenCarStateAt) : nullptr;
	const bool second = g_sirenRaiseOn && firstState && vehicle == firstState + kSirenStandInAt && model != g_sirenCountExpectedModel;
	if ((!second && (vehicle != g_sirenCountExpectedVehicle || model != g_sirenCountExpectedModel)) || a != kSirenCountFloats[0] || b != kSirenCountFloats[1] || c != kSirenCountFloats[2] ||
		flag != kSirenCountFlag || skeleton != g_sirenCountExpectedSkeleton || value != kSirenCountValue || coronas != g_sirenCountCoronas)
		InterlockedIncrement(&g_sirenCountArgsBad);
	g_sirenCountDrawnData = skeleton ? reinterpret_cast<const SirenCountSkeleton*>(skeleton)->data : nullptr;
	if (g_sirenRaiseOn)
		SirenRaiseLights(static_cast<const unsigned char*>(vehicle), model, second);
	SetLastError(second ? kSirenCountLastError + 1 : kSirenCountLastError);
	return second ? ~kSirenCountResult : kSirenCountResult;
}

// find-bone-by-name, behind the jump in its carrier: the bone of that name among the model's, or null - asked, in turn, about the
// vehicle the renderer drew last on this thread.
static const unsigned char* SirenCountFindBone(const SirenCountSkeletonData* data, const char* name)
{
	InterlockedIncrement(&g_sirenCountLookups);
	if (data != g_sirenCountDrawnData)
		InterlockedIncrement(&g_sirenCountOutOfTurn);
	for (uint32_t i = 0; i < data->named; i++)
		if (strcmp(data->list[i].name, name) == 0)
			return reinterpret_cast<const unsigned char*>(&data->list[i]);
	return nullptr;
}

// A function of the game that is not the siren settings getter, for the calls an option leads to it: never called. The getter is I-E-M1's site, real code.
static __declspec(noinline) const void* SirenCountGetter(const void* model)
{
	return model;
}

// The carriers: functions with frames of their own, so that the fake's unwind data names them, and room for their stand-ins.
static volatile int g_sirenCountSink = 0;
static __declspec(noinline) void SirenCountRendererCarrier()
{
	volatile int frame[32];
	for (int i = 0; i < 32; i++)
		frame[i] = g_sirenCountSink + 3 * i;
	for (int i = 1; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0x5157);
	for (int i = 2; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 2] ^ 0x2345);
	for (int i = 3; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 3] ^ 0x5432);
	for (int i = 4; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 4] ^ 0x6543);
	g_sirenCountSink = frame[5] + frame[10] + frame[15] + frame[20];
}
static __declspec(noinline) void SirenCountReaderCarrier()
{
	volatile int frame[32];
	for (int i = 0; i < 32; i++)
		frame[i] = g_sirenCountSink + 9 * i;
	for (int i = 1; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0x1357);
	for (int i = 2; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 2] ^ 0x2468);
	for (int i = 3; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 3] ^ 0x3579);
	g_sirenCountSink = frame[6] + frame[11] + frame[16] + frame[21];
}
static __declspec(noinline) void SirenCountFindBoneCarrier()
{
	volatile int frame[16];
	for (int i = 0; i < 16; i++)
		frame[i] = g_sirenCountSink - 5 * i;
	for (int i = 1; i < 16; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0xB0E5);
	g_sirenCountSink = frame[4] + frame[9] + frame[14];
}
static unsigned char* SirenCountCode(void (*carrier)())
{
	return reinterpret_cast<unsigned char*>(carrier);
}

// The renderer's read of a vehicle's broken sirens, its jump's rel8 the fake's own, then of a siren's bone (I-E-B2, the two together I-E-Z4), its two jumps' rel8 the
// fake's own; find-bone-by-name's prologue and test of the bone count (I-E-B4), then the epilogue that undoes the prologue: add rsp, 20h, pop rbx ... pop r15.
static const unsigned char kSirenCountBrokenRead[] = { 0x41, 0x8B, 0x84, 0x85, 0x60, 0x08, 0x00, 0x00, 0x0F, 0xA3, 0xE8, 0x72, 0x20 };
static const unsigned char kSirenCountBoneRead[] = { 0x48, 0x8B, 0x87, 0xB0, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0, 0x74, 0x1C, 0x48, 0x8B, 0x00,
	0x48, 0x85, 0xC0, 0x74, 0x14, 0x0F, 0xBE, 0x84, 0x28, 0x99, 0x00, 0x00, 0x00 };
// The renderer's call of the getter with its reads of the vehicle around it (I-E-Z1); its reads of a flag byte (I-E-Z5) and for its group lights (I-E-Z6), each
// rel8 and disp32 the fake's own.
static const unsigned char kSirenCountGetterCall[] = { 0x48, 0x8B, 0x8C, 0x24, 0x58, 0x04, 0x00, 0x00, 0x49, 0x8B, 0x45, 0x48, 0x48, 0x8B, 0x40, 0x20, 0x48, 0x89, 0x84, 0x24, 0x58, 0x02, 0x00, 0x00,
	0xE8, 0, 0, 0, 0, 0x45, 0x0F, 0xB7, 0xBD, 0xDC, 0x00, 0x00, 0x00, 0x44, 0x03, 0x3D, 0, 0, 0, 0, 0x49, 0x8B, 0x8D, 0x80, 0x13, 0x00, 0x00 };
static const unsigned char kSirenCountFlagsRead[] = { 0x41, 0x0F, 0xB6, 0x85, 0x86, 0x09, 0x00, 0x00, 0x24, 0x02 };
static const unsigned char kSirenCountGroupRead[] = { 0x41, 0x0F, 0xB7, 0x45, 0x6C, 0x83, 0xE0, 0xFD, 0x83, 0xF8, 0x08, 0x74, 0x10, 0x44, 0x0F, 0x28, 0x3D, 0, 0, 0, 0, 0x83, 0xF8, 0x01, 0x75, 0x05,
	0x45, 0x0F, 0x28, 0xBD, 0x80, 0x00, 0x00, 0x00 };
// The tree reader's prologue (I-E-Z3), then what undoes it, so that the jump on to the fake's reader finds the game's registers: add rsp, 0A8h, pop rbx, rdi, rsi, r12 ... r15, rbp;
// its walk over a node's children (I-E-Z7), never run, each rel8 and rel32 the fake's own.
static const unsigned char kSirenCountReaderStart[] = { 0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x81, 0xEC, 0xA8, 0x00, 0x00, 0x00, 0x48, 0x8D, 0xAC, 0x24, 0x80, 0x00, 0x00, 0x00,
	0x0F, 0x29, 0x75, 0x10, 0x48, 0x89, 0xD7, 0x49, 0x89, 0xCD,
	0x48, 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00, 0x5B, 0x5F, 0x5E, 0x41, 0x5C, 0x41, 0x5D, 0x41, 0x5E, 0x41, 0x5F, 0x5D };
constexpr size_t kSirenCountReaderPattern = 37;	// the pattern's bytes of those
static const unsigned char kSirenCountWalk[] = { 0x48, 0x8B, 0x7F, 0x38, 0x48, 0x85, 0xFF, 0x0F, 0x84, 0, 0, 0, 0, 0x48, 0x8B, 0x75, 0xD0, 0x49, 0x8B, 0x45, 0x08, 0xB9, 0x01, 0x00, 0x00, 0x00,
	0xEB, 0x04, 0x90, 0x90, 0x90, 0x90, 0x8B, 0x48, 0x28, 0x49, 0x39, 0xCE, 0x0F, 0x84, 0, 0, 0, 0, 0x48, 0x8B, 0x7F, 0x30, 0x49, 0x8D, 0x4E, 0x01, 0x48, 0x85, 0xFF, 0x0F, 0x84, 0, 0, 0, 0,
	0x49, 0x89, 0xCE, 0x4C, 0x8D, 0x41, 0xFF, 0x4C, 0x0F, 0xAF, 0x40, 0x20 };
static const unsigned char kSirenCountFindBoneStart[] = { 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41, 0x54, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x20,
	0x66, 0x83, 0x79, 0x5E, 0x00, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0x5F, 0x5E, 0x41, 0x5C, 0x41, 0x5D, 0x41, 0x5E, 0x41, 0x5F };
constexpr size_t kSirenCountGetterCallAt = 0x10;	// in the renderer, after the jump: its call of the getter, the rel32 at +25
constexpr size_t kSirenCountBrokenReadAt = 0x48;	// its read of the broken sirens, and of a siren's bone right behind it
constexpr size_t kSirenCountBoneReadAt = kSirenCountBrokenReadAt + sizeof(kSirenCountBrokenRead);
constexpr size_t kSirenCountFlagsReadAt = 0x78;
constexpr size_t kSirenCountGroupReadAt = 0x88;
constexpr size_t kSirenCountWalkAt = 0x50;			// in the tree reader, after what its start runs
constexpr size_t kSirenCountNotStartAt = 16;	// scnotstart, srz3notstart: find-bone-by-name's and the tree reader's pattern this far into its function
constexpr size_t kSirenCountJumpSize = 6;		// jmp [rip+disp32]
constexpr size_t kSirenCountHookSize = 5;		// the tree reader's first bytes, which a hook takes
static const void* volatile g_sirenCountRendererTarget = nullptr;		// what the carriers' jumps read
static const void* volatile g_sirenCountFindBoneTarget = nullptr;
static const void* volatile g_sirenCountReaderTarget = nullptr;
static size_t g_sirenCountRendererRoom = 0, g_sirenCountFindBoneRoom = 0, g_sirenCountReaderRoom = 0;
static unsigned char g_sirenCountReaderLaid[kSirenCountHookSize] = {};
static bool g_sirenCountBuilt = false;
static uint64_t g_sirenCountPlacedHash = 0;

// The bytes of a carrier's range in the fake's unwind data, if the carrier starts a function there with room for size bytes; else 0.
static size_t SirenCountRoom(const unsigned char* carrier, size_t size)
{
	DWORD64 base = 0;
	const PRUNTIME_FUNCTION entry = RtlLookupFunctionEntry(reinterpret_cast<DWORD64>(carrier), &base, nullptr);
	if (!entry || base + entry->BeginAddress != reinterpret_cast<DWORD64>(carrier) || entry->EndAddress - entry->BeginAddress < size)
		return 0;
	return entry->EndAddress - entry->BeginAddress;
}

// "jmp [rip+disp32]" at code, to the address target holds.
static void SirenCountJump(unsigned char* code, const void* const volatile* target)
{
	code[0] = 0xFF;
	code[1] = 0x25;
	const int32_t disp = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(code + kSirenCountJumpSize));
	memcpy(code + 2, &disp, sizeof(disp));
}

static uint64_t SirenCountHash(uint64_t hash, const volatile void* bytes, size_t size)
{
	const volatile unsigned char* const p = static_cast<const volatile unsigned char*>(bytes);
	for (size_t i = 0; i < size; i++)
		hash = (hash ^ p[i]) * 0x100000001B3ull;
	return hash;
}

// The tree reader as the fake's game has it: a member object's reader of a node into an object, the node with its children; the member data at the game's offsets.
struct SirenParseNode
{
	unsigned char head[0x30];
	const SirenParseNode* next;		// +0x30: the next sibling
	const SirenParseNode* first;	// +0x38: the first child
	unsigned char light[0x48];		// the fake's own from here on: an item of the file
	bool faults;					// its reader faults on this item
};
struct SirenParseData
{
	uint64_t nameHash;
	uint64_t offset;				// +0x08: 0 until the structure is registered
	unsigned char type;				// +0x10
	unsigned char subtype;			// +0x11
	unsigned char rest[14];
	uint64_t elementSize;			// +0x20
	uint32_t capacity;				// +0x28
	uint32_t pad;
	const SirenParseData* element;	// +0x30
};
struct SirenParseMember;
using SirenMemberRead = uint64_t (*)(const SirenParseMember* member, const SirenParseNode* node, unsigned char* object);
struct SirenParseMember
{
	const SirenMemberRead* vtable;	// slot 1 the reader
	const SirenParseData* data;
	const SirenParseMember* element;
};
static_assert(offsetof(SirenParseNode, next) == 0x30 && offsetof(SirenParseNode, first) == 0x38 && offsetof(SirenParseData, offset) == 8 && offsetof(SirenParseData, type) == 0x10 &&
	offsetof(SirenParseData, subtype) == 0x11 && offsetof(SirenParseData, elementSize) == 0x20 && offsetof(SirenParseData, capacity) == 0x28 && offsetof(SirenParseData, element) == 0x30 &&
	offsetof(SirenParseMember, data) == 8 && offsetof(SirenParseMember, element) == 0x10, "the tree and the member data lie at the game's offsets");
static SirenParseData g_sirenLightsDataStorage, g_sirenLightDataStorage, g_sirenOtherLightDataStorage, g_sirenOtherDataStorage;
static SirenParseData* volatile g_sirenLightsData = &g_sirenLightsDataStorage;
static const SirenParseData kSirenLightsDataInRdata = { 0, 0, 0x0D, 0x01, {}, 0x48, 20, 0, nullptr };	// srdatardata
static SirenMemberRead g_sirenArrayVtable[3], g_sirenLightVtable[3];
static SirenParseMember g_sirenLightMember, g_sirenLightsMember, g_sirenOtherLightMember, g_sirenOtherMember;
static volatile LONG g_sirenParseKept = 0, g_sirenParseCalls = 0;
static volatile unsigned char* volatile g_sirenParseNowhere = nullptr;	// what a faulting item writes to
constexpr uint64_t kSirenParseResult = 0x5EADE5F00D000001ull;
constexpr DWORD kSirenParseLastError = 0x5158;

// The reader of an element of the lights, and of the other array's: what the game's reads of a file's item come to
static uint64_t SirenLightRead(const SirenParseMember*, const SirenParseNode* child, unsigned char* element)
{
	if (child->faults)
		*g_sirenParseNowhere = 1;
	memcpy(element, child->light, sizeof(child->light));
	return 1;
}
static uint64_t SirenOtherLightRead(const SirenParseMember*, const SirenParseNode*, unsigned char* element)
{
	memset(element, 0x77, 8);
	return 1;
}

// The game's reader of an array member, behind its carrier's start: the children counted, the count stored behind the elements, each child read by the
// element's reader up to the capacity, and a result and a last error of its own.
static uint64_t SirenArrayRead(const SirenParseMember* member, const SirenParseNode* node, unsigned char* object)
{
	const SirenParseData& data = *member->data;
	size_t children = 0;
	for (const SirenParseNode* child = node->first; child; child = child->next)
		children++;
	const size_t capacity = data.capacity;
	const int32_t count = static_cast<int32_t>(children < capacity ? children : capacity);
	unsigned char* const base = object + data.offset;
	memcpy(base + ((data.elementSize * capacity + 3) & ~size_t{ 3 }), &count, sizeof(count));
	size_t i = 0;
	for (const SirenParseNode* child = node->first; child && i < capacity; child = child->next, i++)
		member->element->vtable[1](member->element, child, base + i * data.elementSize);
	SetLastError(kSirenParseLastError);
	return kSirenParseResult;
}

// What TweaksV may not change: the sites but the rel32 of the vehicle lights' two calls and of the renderer's call of the getter, the allocation's size and the tree
// reader's first bytes, each of which the raise writes, the stand-ins, the models and what they lead to.
static uint64_t SirenCountPlaced()
{
	constexpr size_t kRel32At = 14, kGetterRel32At = 25;
	uint64_t hash = 0xCBF29CE484222325ull;
	hash = SirenCountHash(hash, g_sirenCountCallSite, kRel32At);
	hash = SirenCountHash(hash, g_sirenCountCallSite + kRel32At + 4, kGetterRel32At - kRel32At - 4);
	hash = SirenCountHash(hash, g_sirenCountCallSite + kGetterRel32At + 4, sizeof(g_sirenCountCallSite) - kGetterRel32At - 4);
	hash = SirenCountHash(hash, g_sirenCountGlassRead, sizeof(g_sirenCountGlassRead));
	hash = SirenCountHash(hash, g_sirenCountFindBoneCopy, sizeof(g_sirenCountFindBoneCopy));
	hash = SirenCountHash(hash, g_sirenCountStateSite, 1);
	hash = SirenCountHash(hash, g_sirenCountStateSite + 5, sizeof(g_sirenCountStateSite) - 5);
	const unsigned char* const renderer = SirenCountCode(&SirenCountRendererCarrier);
	const volatile unsigned char* const getter = g_sirenCountRendererGetter;
	if (getter >= renderer && getter < renderer + g_sirenCountRendererRoom)
	{
		hash = SirenCountHash(hash, renderer, static_cast<size_t>(getter - renderer));
		hash = SirenCountHash(hash, getter + 4, g_sirenCountRendererRoom - static_cast<size_t>(getter - renderer) - 4);
	}
	else
		hash = SirenCountHash(hash, renderer, g_sirenCountRendererRoom);
	hash = SirenCountHash(hash, SirenCountCode(&SirenCountFindBoneCarrier), g_sirenCountFindBoneRoom);
	hash = SirenCountHash(hash, SirenCountCode(&SirenCountReaderCarrier) + kSirenCountHookSize, g_sirenCountReaderRoom > kSirenCountHookSize ? g_sirenCountReaderRoom - kSirenCountHookSize : 0);
	for (int i = 0; i < kSirenCountVehicles; i++)	// a car's state is the game's set-up's
		hash = SirenCountHash(hash, &g_sirenCountVehicles[i], offsetof(SirenCountVehicle, car) + kSirenCarStateAt);
	return SirenCountHash(hash, g_sirenCountMany, sizeof(g_sirenCountManyStorage));
}

// The lights' member data: as the game's, its offset 0 until the registration; what an option names otherwise. Entry 19 of the siren settings' member data pointers.
static SirenIdsMember* SirenCountLightsEntry(int argc, wchar_t** argv)
{
	SirenParseData& data = *g_sirenLightsData;
	data.type = HasOption(argc, argv, L"srtype") ? 0x0E : 0x0D;
	data.subtype = HasOption(argc, argv, L"srsubtype") ? 0x02 : 0x01;
	data.elementSize = HasOption(argc, argv, L"srsize") ? 0x50 : 0x48;
	data.capacity = HasOption(argc, argv, L"srcapacity") ? 32 : 20;
	data.offset = HasOption(argc, argv, L"sroffsetfield") ? 8 : 0;
	g_sirenLightDataStorage.type = HasOption(argc, argv, L"srelementtype") ? 0x0B : 0x0C;
	data.element = HasOption(argc, argv, L"srnoelement") ? nullptr : &g_sirenLightDataStorage;
	if (HasOption(argc, argv, L"srdataout"))
		return reinterpret_cast<SirenIdsMember*>(const_cast<unsigned char*>(ImageEnd() + 0x77000));
	if (HasOption(argc, argv, L"srdatardata"))
		return reinterpret_cast<SirenIdsMember*>(const_cast<SirenParseData*>(&kSirenLightsDataInRdata));
	return reinterpret_cast<SirenIdsMember*>(g_sirenLightsData);
}

// Before TweaksV loads: the stand-ins written over their carriers, the vehicle lights' two calls pointed at the renderer and the
// getter - or where an option asks - and the models made. With noUnwind find-bone-by-name's pattern is not written into its
// carrier but completed in .fakesit, where no unwind data names a function; a sr...out option does the same with the part's pattern it names. A carrier's start
// stays runnable with any of them.
static void SirenCountSetup(int argc, wchar_t** argv)
{
	const bool hooked = HasOption(argc, argv, L"schooked"), getterHooked = HasOption(argc, argv, L"scgetterhooked"), elsewhere = HasOption(argc, argv, L"scelsewhere"),
		notStart = HasOption(argc, argv, L"scnotstart"), noUnwind = HasOption(argc, argv, L"scnounwind");
	const bool getterOut = HasOption(argc, argv, L"srz1out"), brokenOut = HasOption(argc, argv, L"srz4out"), flagsOut = HasOption(argc, argv, L"srz5out"), groupOut = HasOption(argc, argv, L"srz6out"),
		walkOut = HasOption(argc, argv, L"srz7out"), readerOut = HasOption(argc, argv, L"srz3nounwind"), readerNotStart = HasOption(argc, argv, L"srz3notstart");
	for (size_t i = 0; i < sizeof(g_sirenCountOutside); i++)
		g_sirenCountOutside[i] = 0xCC;
	const auto outside = [](const unsigned char* bytes, size_t size) {
		for (size_t i = 0; i < size; i++)
			g_sirenCountOutside[i] = bytes[i];
	};
	unsigned char* const renderer = SirenCountCode(&SirenCountRendererCarrier);
	unsigned char* const findBone = SirenCountCode(&SirenCountFindBoneCarrier);
	unsigned char* const reader = SirenCountCode(&SirenCountReaderCarrier);
	const size_t findBoneAt = notStart ? kSirenCountNotStartAt : 0;
	const size_t readerAt = readerNotStart ? kSirenCountNotStartAt : 0;
	g_sirenCountRendererRoom = SirenCountRoom(renderer, kSirenCountGroupReadAt + sizeof(kSirenCountGroupRead) + 1);
	g_sirenCountFindBoneRoom = SirenCountRoom(findBone, findBoneAt + sizeof(kSirenCountFindBoneStart) + kSirenCountJumpSize + 1);
	g_sirenCountReaderRoom = SirenCountRoom(reader, kSirenCountWalkAt + sizeof(kSirenCountWalk) + 1);
	g_sirenCountBuilt = g_sirenCountRendererRoom != 0 && g_sirenCountFindBoneRoom != 0 && g_sirenCountReaderRoom != 0 && readerAt + sizeof(kSirenCountReaderStart) + kSirenCountJumpSize <= kSirenCountWalkAt;
	if (g_sirenCountBuilt)
	{
		g_sirenCountRendererTarget = reinterpret_cast<const void*>(&SirenCountRenderer);
		g_sirenCountFindBoneTarget = reinterpret_cast<const void*>(&SirenCountFindBone);
		g_sirenCountReaderTarget = reinterpret_cast<const void*>(&SirenArrayRead);
		DWORD old = 0, unused = 0;
		VirtualProtect(renderer, g_sirenCountRendererRoom, PAGE_EXECUTE_READWRITE, &old);
		memset(renderer, 0xCC, g_sirenCountRendererRoom);
		SirenCountJump(renderer, &g_sirenCountRendererTarget);
		if (!brokenOut)
			memcpy(renderer + kSirenCountBrokenReadAt, kSirenCountBrokenRead, sizeof(kSirenCountBrokenRead));
		memcpy(renderer + kSirenCountBoneReadAt, kSirenCountBoneRead, sizeof(kSirenCountBoneRead));
		if (!flagsOut)
			memcpy(renderer + kSirenCountFlagsReadAt, kSirenCountFlagsRead, sizeof(kSirenCountFlagsRead));
		if (!groupOut)
			memcpy(renderer + kSirenCountGroupReadAt, kSirenCountGroupRead, sizeof(kSirenCountGroupRead));
		constexpr size_t kGetterRel32At = 25;
		if (getterOut)
			outside(kSirenCountGetterCall, sizeof(kSirenCountGetterCall));
		else
			memcpy(renderer + kSirenCountGetterCallAt, kSirenCountGetterCall, sizeof(kSirenCountGetterCall));
		g_sirenCountRendererGetter = (getterOut ? g_sirenCountOutside : static_cast<volatile unsigned char*>(renderer + kSirenCountGetterCallAt)) + kGetterRel32At;
		SetRel32(Site{ g_sirenCountRendererGetter, 0, 0 }, HasOption(argc, argv, L"srz1hooked") ? static_cast<const void*>(ImageEnd() + 0x78000) :
			HasOption(argc, argv, L"srz1other") ? static_cast<const void*>(findBone) : const_cast<const unsigned char*>(g_sirenIdsM1));
		VirtualProtect(renderer, g_sirenCountRendererRoom, old, &unused);
		FlushInstructionCache(GetCurrentProcess(), renderer, g_sirenCountRendererRoom);
		if (brokenOut)	// the two reads together, I-E-B2 in it a second time, which nothing looks for without DIAGNOSTIC_MODE
		{
			outside(kSirenCountBrokenRead, sizeof(kSirenCountBrokenRead));
			for (size_t i = 0; i < sizeof(kSirenCountBoneRead); i++)
				g_sirenCountOutside[sizeof(kSirenCountBrokenRead) + i] = kSirenCountBoneRead[i];
		}
		if (flagsOut)
			outside(kSirenCountFlagsRead, sizeof(kSirenCountFlagsRead));
		if (groupOut)
			outside(kSirenCountGroupRead, sizeof(kSirenCountGroupRead));
		VirtualProtect(reader, g_sirenCountReaderRoom, PAGE_EXECUTE_READWRITE, &old);
		memset(reader, 0xCC, g_sirenCountReaderRoom);
		if (readerOut || readerNotStart)
			SirenCountJump(reader, &g_sirenCountReaderTarget);
		if (readerOut)
			outside(kSirenCountReaderStart, kSirenCountReaderPattern);
		else
		{
			memcpy(reader + readerAt, kSirenCountReaderStart, sizeof(kSirenCountReaderStart));
			SirenCountJump(reader + readerAt + sizeof(kSirenCountReaderStart), &g_sirenCountReaderTarget);
		}
		if (walkOut)
			outside(kSirenCountWalk, sizeof(kSirenCountWalk));
		else
			memcpy(reader + kSirenCountWalkAt, kSirenCountWalk, sizeof(kSirenCountWalk));
		memcpy(g_sirenCountReaderLaid, reader, kSirenCountHookSize);
		VirtualProtect(reader, g_sirenCountReaderRoom, old, &unused);
		FlushInstructionCache(GetCurrentProcess(), reader, g_sirenCountReaderRoom);
		VirtualProtect(findBone, g_sirenCountFindBoneRoom, PAGE_EXECUTE_READWRITE, &old);
		memset(findBone, 0xCC, g_sirenCountFindBoneRoom);
		if (!noUnwind)
		{
			memcpy(findBone + findBoneAt, kSirenCountFindBoneStart, sizeof(kSirenCountFindBoneStart));
			SirenCountJump(findBone + findBoneAt + sizeof(kSirenCountFindBoneStart), &g_sirenCountFindBoneTarget);
		}
		VirtualProtect(findBone, g_sirenCountFindBoneRoom, old, &unused);
		FlushInstructionCache(GetCurrentProcess(), findBone, g_sirenCountFindBoneRoom);
	}
	SetRel32(g_sirenCountRendererCall, hooked ? static_cast<const void*>(ImageEnd() + 0x71000) :
		elsewhere ? reinterpret_cast<const void*>(&SirenCountGetter) : static_cast<const void*>(renderer));
	SetRel32(g_sirenCountGetterCall, getterHooked ? static_cast<const void*>(ImageEnd() + 0x72000) :
		HasOption(argc, argv, L"srgetterelsewhere") ? reinterpret_cast<const void*>(&SirenCountGetter) : const_cast<const unsigned char*>(g_sirenIdsM1));
	if (noUnwind)
		g_sirenCountFindBoneCopy[1] = 0x41;

	SirenCountVehicle* const v = g_sirenCountVehicles;
	SirenCountMake(v[kSirenCountPolice], 0x79FBB0C5, 90, 20, 40, 20, 60);
	SirenCountMake(v[kSirenCount32], 0xA1B2C3D4, 100, 32, 10, 32, 42);
	SirenCountMake(v[kSirenCountLost], 0x0C0FFEE0, 150, 10, 120, 10, 100);
	SirenCountMake(v[kSirenCount8], 0x2C3D4E5F, 40, 8, 20, 0, 0);
	SirenCountMake(v[kSirenCountNoStructure], 0x0BAD0001, 50, 4, 10, 0, 0);
	v[kSirenCountNoStructure].model.bones = nullptr;
	SirenCountMake(v[kSirenCountNoSkeleton], 0x0BAD0002, 50, 4, 10, 0, 0);
	SirenCountMake(v[kSirenCountOdd], 0x0DD0DD00, 50, 4, 10, 0, 0);
	v[kSirenCountOdd].structure.ids[0x10 + 0x89] = 50;		// siren1 at another bone than its name has
	v[kSirenCountOdd].structure.ids[0x10 + 0x9D] = 20;		// and a glass bone it has no name for
	v[kSirenCountOdd].structure.ids[0x10 + 0x8A] = -1;		// siren2, bone 11, not named, though a signed byte holds 11
	SirenCountMake(v[kSirenCountFault], 0x0BAD0003, 50, 4, 10, 0, 0);
	v[kSirenCountFault].data.list = nullptr;				// its bones by name at an address that cannot be read: the lookup faults
	SirenCountMake(v[kSirenCountMore32], 0xB2C3D4E5, 110, 32, 0, 32, 32);
	SirenCountMake(v[kSirenCountMore16], 0x16161616, 60, 16, 30, 16, 46);
	SirenCountMake(v[kSirenCount21], 0x21212121, 70, 21, 5, 0, 0);
	SirenCountMake(v[kSirenCountHoles], 0x40404040, 80, 32, 40, 0, 0);
	v[kSirenCountHoles].structure.room[0][24] = -1;			// siren25 and siren31, in its room, have no bone
	v[kSirenCountHoles].structure.room[0][30] = -1;
	SirenCountMake(v[kSirenCountShare], 0x50505050, 90, 32, 50, 0, 0);
	SirenCountMake(v[kSirenCountReplaced], 0x60606060, 95, 32, 60, 0, 0);
	SirenCountMake(v[kSirenCountRace], 0x70707070, 96, 32, 70, 0, 0);
	SirenCountMake(v[kSirenCountJoined], 0xB1B1B1B1, 96, 32, 10, 32, 42);
	SirenCountMake(v[kSirenCountNoSetting], 0x80808080, 97, 32, 20, 0, 0);
	SirenCountMake(v[kSirenCountPast], 0x90909090, 98, 32, 30, 0, 0);
	SirenCountMake(v[kSirenCountSpare], 0xA0A0A0A0, 99, 32, 35, 0, 0);
	for (int i = 0; i < kSirenCountVehicles; i++)	// the game's array holds 24 settings and has room for 64
		v[i].model.position = static_cast<unsigned char>(1 + i);
	v[kSirenCountNoSetting].model.position = 0;
	v[kSirenCountPast].model.position = 0xFF;
	v[kSirenCountSpare].model.position = 30;
	for (int i = 0; i < kSirenCountMany; i++)
	{
		g_sirenCountMany[i] = v[kSirenCountPolice].model;
		g_sirenCountMany[i].hash = 0x5C000000u + static_cast<uint32_t>(i);
	}
	g_sirenCountPlacedHash = SirenCountPlaced();
}

// Where the call of the renderer leads now: the fake's renderer, TweaksV's call stub, another mod or another function of the game.
static const wchar_t* SirenCountWhere()
{
	const unsigned char* const target = Rel32Target(g_sirenCountRendererCall);
	if (target == SirenCountCode(&SirenCountRendererCarrier))
		return L"the renderer";
	if (InImage(target))
		return L"another function of the game";
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(target, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT && memcmp(target, "\xFF\x25\x00\x00\x00\x00", 6) == 0)
		return L"a TweaksV stub";
	return L"another mod";
}

// One pass of a vehicle's sirens, as the game's vehicle lights make it: through the call at the site, to wherever it leads, then, once srdraw has set it up and
// while it leads to the getter or to TweaksV's stub, their call of the getter.
static const unsigned char* g_sirenRaiseVehicleGetter = nullptr;
static void SirenCountDraw(const unsigned char* lights, const void* vehicle, const SirenCountModel* model, const SirenCountSkeleton* skeleton)
{
	g_sirenCountExpectedVehicle = vehicle;
	g_sirenCountExpectedModel = model;
	g_sirenCountExpectedSkeleton = skeleton;
	SetLastError(0);
	const uint64_t result = reinterpret_cast<SirenCountLights>(const_cast<unsigned char*>(lights))(const_cast<void*>(vehicle), kSirenCountFloats[0],
		kSirenCountFloats[1], kSirenCountFloats[2], kSirenCountFlag, reinterpret_cast<const unsigned char*>(model),
		reinterpret_cast<const unsigned char*>(skeleton), kSirenCountValue, g_sirenCountCoronas);
	const DWORD error = GetLastError();
	InterlockedIncrement(&g_sirenCountDrawn);
	if (result == kSirenCountResult && error == kSirenCountLastError)
		InterlockedIncrement(&g_sirenCountKept);
	const int index = SirenRaiseIndexOf(model);
	if (g_sirenRaiseVehicleGetter && index >= 0)	// the head and tail lights: its four dwords, a null like zeros
	{
		const unsigned char* const setting = reinterpret_cast<const unsigned char* (*)(const void*)>(const_cast<unsigned char*>(g_sirenRaiseVehicleGetter))(model);
		if (!SirenRaiseGetterRight(index, setting))
			InterlockedIncrement(&g_sirenRaiseGetterBad);
		if (setting && SirenRaiseIsEmpty(setting))
			InterlockedIncrement(&g_sirenRaiseEmpty);
		if (setting && (SirenField<uint32_t>(setting, 0x24) | SirenField<uint32_t>(setting, 0x28) | SirenField<uint32_t>(setting, 0x2C) | SirenField<uint32_t>(setting, 0x30)) == 0 && !SirenRaiseIsEmpty(setting))
			InterlockedIncrement(&g_sirenRaiseGetterBad);
	}
}

// passes passes of the models first to first + count - 1, each the vehicle it is drawn as, the one without a skeleton without one.
struct SirenCountRun
{
	const unsigned char* lights;
	int first, count, passes;
};
static DWORD WINAPI SirenCountRunPasses(LPVOID parameter)
{
	const SirenCountRun& run = *static_cast<const SirenCountRun*>(parameter);
	for (int pass = 0; pass < run.passes; pass++)
		for (int i = run.first; i < run.first + run.count; i++)
		{
			SirenCountVehicle& v = g_sirenCountVehicles[i];
			SirenCountDraw(run.lights, &v.car, &v.model, i == kSirenCountNoSkeleton ? nullptr : &v.skeleton);
		}
	return 0;
}

// Once TweaksV has started (scdraw), when the call leads to the fake's renderer or to TweaksV's stub: 100 passes of the four
// models - seven with scodd - from each of two threads at once; with scfault 10 passes of the model whose lookup faults; with
// scmore 11 seconds later 10 passes of two more, then 3 seconds more; with scfull 1,100 more models, one pass each.
static void SirenCountDrawAll(bool odd, bool more, bool full, bool fault)
{
	const wchar_t* const where = SirenCountWhere();
	if (!g_sirenCountBuilt || (wcscmp(where, L"the renderer") != 0 && wcscmp(where, L"a TweaksV stub") != 0))
		return;
	const unsigned char* const lights = Rel32Target(g_sirenCountRendererCall);
	SirenCountRun runs[2] = { { lights, 0, odd ? kSirenCountMore32 : kSirenCountNoStructure, 100 },
		{ lights, 0, odd ? kSirenCountMore32 : kSirenCountNoStructure, 100 } };
	HANDLE threads[2] = {};
	DWORD started = 0;
	for (SirenCountRun& run : runs)
		if (const HANDLE thread = CreateThread(nullptr, 0, SirenCountRunPasses, &run, 0, nullptr))
			threads[started++] = thread;
		else
			SirenCountRunPasses(&run);
	if (started)
		WaitForMultipleObjects(started, threads, TRUE, INFINITE);
	for (DWORD i = 0; i < started; i++)
		CloseHandle(threads[i]);
	if (fault)
	{
		SirenCountRun faulting = { lights, kSirenCountFault, 1, 10 };
		SirenCountRunPasses(&faulting);
	}
	if (more)
	{
		Sleep(11000);
		SirenCountRun later = { lights, kSirenCountMore32, 2, 10 };
		SirenCountRunPasses(&later);
		Sleep(3000);
	}
	if (full)
		for (int i = 0; i < kSirenCountMany; i++)
			SirenCountDraw(lights, &g_sirenCountVehicles[kSirenCountPolice].car, &g_sirenCountMany[i], &g_sirenCountVehicles[kSirenCountPolice].skeleton);
}

// The sirens per vehicle's lights, once TweaksV has started. The game registers the lights' member, and its parser reads a setting's node through the array member's slot 1
// into an object of its own, which the game's merge then copies into its array, the scalars with it; the vehicles get their states, as big as the allocation's site says.
static const unsigned char* SirenParse(int items, uint32_t seed, uint32_t pastSeed, int faultsAt, uint32_t scalars = 0)
{
	SirenParseNode* const nodes = new SirenParseNode[static_cast<size_t>(items) + 1]();
	SirenParseNode& root = nodes[static_cast<size_t>(items)];
	for (int i = items - 1; i >= 0; i--)
	{
		SirenMakeItem(nodes[i].light, i < 20 ? seed : pastSeed, i);
		nodes[i].next = i + 1 < items ? &nodes[i + 1] : nullptr;
		nodes[i].faults = i == faultsAt;
	}
	root.first = items ? &nodes[0] : nullptr;
	unsigned char* const object = static_cast<unsigned char*>(calloc(1, 0x5F0));
	unsigned char* const copy = static_cast<unsigned char*>(calloc(1, 0x5E8));
	SetLastError(0);
	const uint64_t result = g_sirenLightsMember.vtable[1](&g_sirenLightsMember, &root, object);
	InterlockedIncrement(&g_sirenParseCalls);
	if (result == kSirenParseResult && GetLastError() == kSirenParseLastError)
		InterlockedIncrement(&g_sirenParseKept);
	memcpy(copy + 0x3C, object + g_sirenLightsData->offset, 0x5A0 + 4);	// as the game's merge copies them, to their place in its array, with the count
	for (int i = 0; i < 8; i++)
	{
		const float scalar = static_cast<float>(scalars ? scalars : seed) + static_cast<float>(i) * 0.5f;
		memcpy(copy + 4 + i * 4, &scalar, sizeof(scalar));
	}
	memset(copy + 0x24, 0x5A, 0x14);
	copy[0x38] = 1;
	free(object);
	delete[] nodes;
	return copy;
}

// The array member of another member, which goes through the same reader, 32 items in the room of 8 of its own
static void SirenParseOther()
{
	SirenParseNode* const nodes = new SirenParseNode[33]();
	for (int i = 0; i < 32; i++)
		nodes[i].next = i + 1 < 32 ? &nodes[i + 1] : nullptr;
	nodes[32].first = &nodes[0];
	unsigned char object[0x80] = {};
	g_sirenOtherMember.vtable[1](&g_sirenOtherMember, &nodes[32], object);
	delete[] nodes;
}

static uint32_t SirenRaiseStateSize()
{
	uint32_t size = 0;
	for (int i = 0; i < 4; i++)
		size |= static_cast<uint32_t>(g_sirenCountStateSite[1 + i]) << (8 * i);
	return size;
}

// The game's vehicle set-up: a state as big as the site says, 16-aligned and not cleared as operator new gives it, its first state made by the constructor.
static void SirenRaiseGiveState(SirenCountVehicle& v, int32_t start)
{
	const uint32_t size = SirenRaiseStateSize();
	unsigned char* const state = static_cast<unsigned char*>(_aligned_malloc(size, 16));
	memset(state, 0xCD, size);
	memset(state, 0, kSirenStateSize);
	memcpy(state, &start, sizeof(start));
	const int32_t beat = -1;
	memcpy(state + 8, &beat, sizeof(beat));
	memcpy(v.car.bytes + kSirenCarStateAt, &state, sizeof(state));
}

// What srdraw draws, and from what its settings are made: a model's items, the seed of its first 20 and of those past them, the scalars' if another; the later row of a
// model is the setting it gets.
struct SirenRaiseRow { int vehicle; int items; uint32_t seed; uint32_t pastSeed; uint32_t scalars; };
static const SirenRaiseRow kSirenRaiseRows[] = {
	{ kSirenCountPolice, 20, 1, 1 }, { kSirenCount32, 32, 2, 2 }, { kSirenCountLost, 24, 3, 3 }, { kSirenCount8, 40, 4, 4 }, { kSirenCountNoStructure, 21, 12, 12 }, { kSirenCountMore32, 40, 5, 5 },
	{ kSirenCountMore16, 20, 6, 6 }, { kSirenCount21, 21, 7, 7 }, { kSirenCountHoles, 32, 8, 8 }, { kSirenCountShare, 32, 2, 2, 77 }, { kSirenCountReplaced, 24, 9, 9 }, { kSirenCountReplaced, 28, 9, 10 } };
static const int kSirenRaiseDrawn[] = { kSirenCountPolice, kSirenCount32, kSirenCountLost, kSirenCount8, kSirenCountNoStructure, kSirenCountMore32, kSirenCountMore16, kSirenCount21, kSirenCountHoles,
	kSirenCountShare, kSirenCountReplaced, kSirenCountNoSetting, kSirenCountPast, kSirenCountSpare };
static const wchar_t* const kSirenRaiseNames[] = { L"police", L"32", L"lost", L"8", L"none", L"more32", L"16", L"21", L"holes", L"share", L"replaced", L"nosetting", L"past", L"spare" };
constexpr int kSirenRaiseAll = sizeof(kSirenRaiseDrawn) / sizeof(kSirenRaiseDrawn[0]);
constexpr int kSirenRaiseUnbounded = 2;		// the last two, which the game's own getter would read past its array for: drawn only through TweaksV's
static_assert(kSirenRaiseAll == sizeof(kSirenRaiseNames) / sizeof(kSirenRaiseNames[0]), "a name for each model srdraw draws");
static const unsigned char* g_sirenRaiseLights = nullptr;
static int g_sirenRaiseCount = 0;			// the models this run draws
static volatile LONG g_sirenRaceDone = 0;

static void SirenRaisePass()
{
	for (int n = 0; n < g_sirenRaiseCount; n++)
	{
		SirenCountVehicle& v = g_sirenCountVehicles[kSirenRaiseDrawn[n]];
		SirenCountDraw(g_sirenRaiseLights, &v.car, &v.model, &v.skeleton);
	}
}

static DWORD WINAPI SirenRaiseThread(LPVOID)
{
	for (int pass = 0; pass < 100; pass++)
		SirenRaisePass();
	return 0;
}

// The game's merge: a setting read, copied into the game's array at its model's position.
static void SirenRaiseMerge(int vehicle, const unsigned char* setting)
{
	unsigned char* const entry = g_sirenIdsSettings + g_sirenCountVehicles[vehicle].model.position * kSirenIdsSettingSize;
	memcpy(entry, setting, kSirenIdsSettingSize);
	free(const_cast<unsigned char*>(setting));
	g_sirenRaiseSettings[vehicle] = entry;
}

// srrace: a setting's lights past 20 replaced again and again, each generation of one seed in all of them.
static DWORD WINAPI SirenRaceThread(LPVOID)
{
	for (uint32_t generation = 0; generation < 200; generation++)
	{
		free(const_cast<unsigned char*>(SirenParse(32, 11, 1000 + generation, -1)));
		Sleep(1);
	}
	InterlockedExchange(&g_sirenRaceDone, 1);
	return 0;
}

// Where a call of the getter leads now
static const wchar_t* SirenRaiseLeads(const unsigned char* target)
{
	if (target == const_cast<const unsigned char*>(g_sirenIdsM1))
		return L"the getter";
	if (InImage(target))
		return L"another function of the game";
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(target, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT && memcmp(target, "\xFF\x25\x00\x00\x00\x00", 6) == 0)
		return L"a TweaksV stub";
	return L"another mod";
}

static const wchar_t* SirenRaiseGetterWhere()
{
	return SirenRaiseLeads(Rel32Target(Site{ g_sirenCountRendererGetter, 0, 0 }));
}

static void SirenRaiseActions(int argc, wchar_t** argv)
{
	if (!g_sirenCountBuilt)
		return;
	g_sirenArrayVtable[1] = reinterpret_cast<SirenMemberRead>(SirenCountCode(&SirenCountReaderCarrier));
	g_sirenLightVtable[1] = &SirenLightRead;
	static SirenMemberRead otherLightVtable[3] = { nullptr, &SirenOtherLightRead, nullptr };
	g_sirenLightMember = { g_sirenLightVtable, HasOption(argc, argv, L"srelement") ? &g_sirenOtherLightDataStorage : &g_sirenLightDataStorage, nullptr };
	g_sirenLightsMember = { g_sirenArrayVtable, g_sirenLightsData, &g_sirenLightMember };
	g_sirenOtherLightMember = { otherLightVtable, &g_sirenOtherLightDataStorage, nullptr };
	g_sirenOtherDataStorage.type = 0x0D;
	g_sirenOtherDataStorage.subtype = 0x01;
	g_sirenOtherDataStorage.elementSize = 8;
	g_sirenOtherDataStorage.capacity = 8;
	g_sirenOtherDataStorage.element = &g_sirenOtherLightDataStorage;
	g_sirenOtherMember = { g_sirenArrayVtable, &g_sirenOtherDataStorage, &g_sirenOtherLightMember };

	// the game's registration, after TweaksV's start
	g_sirenLightsData->offset = HasOption(argc, argv, L"srlateoffset") ? 0x44 : g_sirenIdsSettingOffsets[19];
	g_sirenLightsData->element = &g_sirenLightDataStorage;

	if (HasOption(argc, argv, L"srfault") || HasOption(argc, argv, L"srelement"))
	{
		free(const_cast<unsigned char*>(SirenParse(32, 21, 21, 25)));
		free(const_cast<unsigned char*>(SirenParse(32, 22, 22, -1)));
	}
	if (HasOption(argc, argv, L"srmany"))
		for (uint32_t i = 0; i < 300; i++)
			free(const_cast<unsigned char*>(SirenParse(21, 100 + i, 100 + i, -1)));
	if (HasOption(argc, argv, L"srflood"))
		for (int i = 0; i < 20001; i++)
			free(const_cast<unsigned char*>(SirenParse(21, 23, 23, -1)));
	if (!HasOption(argc, argv, L"srdraw"))
		return;

	SirenParseOther();
	for (const SirenRaiseRow& row : kSirenRaiseRows)
	{
		SirenRaiseMerge(row.vehicle, SirenParse(row.items, row.seed, row.pastSeed, -1, row.scalars));
		g_sirenRaisePastSeed[row.vehicle] = row.pastSeed;
	}

	if (HasOption(argc, argv, L"srnocarcols"))
		g_sirenIdsPointer[0] = nullptr;
	const wchar_t* const where = SirenCountWhere();
	const wchar_t* const getterWhere = SirenRaiseGetterWhere();
	if ((wcscmp(where, L"the renderer") != 0 && wcscmp(where, L"a TweaksV stub") != 0) || (wcscmp(getterWhere, L"the getter") != 0 && wcscmp(getterWhere, L"a TweaksV stub") != 0))
		return;	// the options that lead the calls elsewhere: nothing the fake can draw through
	g_sirenRaiseLights = Rel32Target(g_sirenCountRendererCall);
	g_sirenRaiseOn = true;
	const wchar_t* const vehicleWhere = SirenRaiseLeads(Rel32Target(g_sirenCountGetterCall));
	if (wcscmp(vehicleWhere, L"the getter") == 0 || wcscmp(vehicleWhere, L"a TweaksV stub") == 0)
		g_sirenRaiseVehicleGetter = Rel32Target(g_sirenCountGetterCall);
	g_sirenRaiseCount = kSirenRaiseAll - (wcscmp(getterWhere, L"a TweaksV stub") == 0 ? 0 : kSirenRaiseUnbounded);
	int32_t start = 1000;
	for (int n = 0; n < g_sirenRaiseCount; n++)
		SirenRaiseGiveState(g_sirenCountVehicles[kSirenRaiseDrawn[n]], start += 16);
	if (HasOption(argc, argv, L"srbroken"))
	{
		const uint32_t broken = (1u << 2) | (1u << 24);
		memcpy(g_sirenCountVehicles[kSirenCount32].car.bytes + 0x860, &broken, sizeof(broken));
	}
	SirenRaisePass();	// the first pass, on this thread
	if (HasOption(argc, argv, L"srreset"))	// the game's resets touch the first state: made anew, another start time
	{
		for (int n = 0; n < g_sirenRaiseCount; n++)
		{
			unsigned char* const state = SirenField<unsigned char*>(g_sirenCountVehicles[kSirenRaiseDrawn[n]].car.bytes, kSirenCarStateAt);
			start += 16;
			memcpy(state, &start, sizeof(start));
		}
		SirenRaisePass();
	}
	HANDLE threads[2] = {};
	for (HANDLE& thread : threads)
		if ((thread = CreateThread(nullptr, 0, SirenRaiseThread, nullptr, 0, nullptr)) == nullptr)
			SirenRaiseThread(nullptr);
	for (const HANDLE thread : threads)
		if (thread)
		{
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
		}
	if (HasOption(argc, argv, L"srrace"))
	{
		SirenCountVehicle& v = g_sirenCountVehicles[kSirenCountRace];
		SirenRaiseMerge(kSirenCountRace, SirenParse(32, 11, 999, -1));
		SirenRaiseGiveState(v, start += 16);
		SirenCountDraw(g_sirenRaiseLights, &v.car, &v.model, &v.skeleton);	// its first pass, before the setting is replaced
		const HANDLE thread = CreateThread(nullptr, 0, SirenRaceThread, nullptr, 0, nullptr);
		if (!thread)
			SirenRaceThread(nullptr);
		while (!g_sirenRaceDone)
			SirenCountDraw(g_sirenRaiseLights, &v.car, &v.model, &v.skeleton);
		if (thread)
		{
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
		}
	}
}

// "siren count: stand-ins built | siren draws: 800 through a TweaksV stub, the renderer 800 times, args ok, result and last error
// kept 800 of 800, 256 lookups in turn | siren sites and data: as placed | " - "siren draws: none, the call leads to ..." when it
// drew nothing. Then the raise's sites as they stand, and once the parser has read what the renderer's loop found: "siren lights: second calls 1206, stand-ins ok, blocks ok,
// own settings ok, lights right, second states made 6 clear, not torn, 804 default lights, getter calls right, 804 empty settings, parser 12 of 12 back, lit police 20+0, 32 20+12, ... | ".
static void SirenCountReport()
{
	wprintf(L"siren count: stand-ins %ls | ", g_sirenCountBuilt ? L"built" : L"NOT built");
	if (g_sirenCountDrawn == 0)
		wprintf(L"siren draws: none, the call leads to %ls | ", SirenCountWhere());
	else
		wprintf(L"siren draws: %ld through %ls, the renderer %ld times, args %ls, result and last error kept %ld of %ld, %ld lookups %ls | ",
			g_sirenCountDrawn, SirenCountWhere(), g_sirenCountRendered, g_sirenCountArgsBad == 0 ? L"ok" : L"BAD", g_sirenCountKept,
			g_sirenCountDrawn, g_sirenCountLookups, g_sirenCountOutOfTurn == 0 ? L"in turn" : L"OUT OF TURN");
	wprintf(L"siren sites and data: %ls | ", SirenCountPlaced() == g_sirenCountPlacedHash ? L"as placed" : L"CHANGED");
	const unsigned char* const reader = SirenCountCode(&SirenCountReaderCarrier);
	wprintf(L"siren raise sites: state size 0x%X, getter call to %ls, the vehicle lights' to %ls, reader %ls | ", SirenRaiseStateSize(), SirenRaiseGetterWhere(),
		SirenRaiseLeads(Rel32Target(g_sirenCountGetterCall)),
		memcmp(reader, g_sirenCountReaderLaid, kSirenCountHookSize) == 0 ? L"as laid" : reader[0] == 0xE9 ? L"hooked" : L"CHANGED");
	if (g_sirenParseCalls)
	{
		wprintf(L"siren lights: second calls %ld, stand-ins %ls, blocks %ls, own settings %ls, lights %ls, second states made %ld%ls, %ls, %ld default lights, getter calls %ls, %ld empty settings, parser %ld of %ld back, lit",
			g_sirenRaiseSecond, g_sirenRaiseStandInBad == 0 ? L"ok" : L"BAD", g_sirenRaiseBlockBad == 0 ? L"ok" : L"BAD", g_sirenRaiseOwnBad == 0 ? L"ok" : L"BAD",
			g_sirenRaiseLightBad == 0 ? L"right" : L"WRONG", g_sirenRaiseMade, g_sirenRaiseDirty == 0 ? L" clear" : L" DIRTY", g_sirenRaiseTorn == 0 ? L"not torn" : L"TORN",
			g_sirenRaiseDefault, g_sirenRaiseGetterBad == 0 ? L"right" : L"WRONG", g_sirenRaiseEmpty, g_sirenParseKept, g_sirenParseCalls);
		for (size_t i = 0; i < sizeof(kSirenRaiseDrawn) / sizeof(kSirenRaiseDrawn[0]); i++)
			wprintf(L" %ls %ld+%ld,", kSirenRaiseNames[i], g_sirenRaiseLit[kSirenRaiseDrawn[i]][0], g_sirenRaiseLit[kSirenRaiseDrawn[i]][1]);
		wprintf(L" | ");
	}
}

// Ped props: the stand-ins for the game's texture count and metadata lookup, which the sites' calls lead to and nothing
// calls - two different functions, so that the linker cannot fold them into one - and the model info store: its four
// globals, where A-E-S1's references lead, and its data in memory of the fake's own, laid out at the game's offsets as
// the game's heap holds it, a page of no access beside it. The bytes of the sites and the data are summed before
// TweaksV loads, for the report.
static __declspec(noinline) int PedPropsTextureCount(const unsigned char* props, int prop)
{
	return props ? props[prop] : 0;
}
static __declspec(noinline) const unsigned char* PedPropsMetadataLookup(const unsigned char* entries, int count, int prop)
{
	return entries && prop < count ? entries + prop * 0x38 : nullptr;
}
static int32_t g_pedPropsCountStorage = 0;				// the store's count of model indices, a dword
static uint32_t* g_pedPropsBitmapStorage = nullptr;		// its bitmap: a bit per index, set for a model
static unsigned char* g_pedPropsArrayStorage = nullptr;	// its array: an element per index, the model info's pointer its first 8 bytes
static uint64_t g_pedPropsStrideStorage = 0;			// an element's bytes, a qword
static int32_t* volatile g_pedPropsStoreCount = &g_pedPropsCountStorage;
static uint32_t** volatile g_pedPropsStoreBitmap = &g_pedPropsBitmapStorage;
static unsigned char** volatile g_pedPropsStoreArray = &g_pedPropsArrayStorage;
static uint64_t* volatile g_pedPropsStoreStride = &g_pedPropsStrideStorage;
constexpr int32_t kPedPropsIndices = 60010;		// not a multiple of 32: the bitmap's last dword holds bits past the count
constexpr int32_t kPedPropsSlots = 60032;		// the array's elements, up to the bitmap's last bit
constexpr uint64_t kPedPropsStride = 16;
constexpr size_t kPedPropsModelSize = 0x250;	// a model info: its hash at +0x18, its type at +0x9D, its variation collection at +0x240
constexpr size_t kPedPropsFileSize = 0x70;		// a variation file: its anchors at +0x58, their count (a word) at +0x60
constexpr size_t kPedPropsEntrySize = 0x18;		// an anchor's entry: its props (a word) at +8, the anchor (a dword) at +0x10
constexpr size_t kPedPropsHeapSize = 16 << 20;
static unsigned char* g_pedPropsHeap = nullptr;
static size_t g_pedPropsUsed = 0;
static unsigned char* g_pedPropsNoAccess = nullptr;
static unsigned char* g_pedPropsMale = nullptr;	// mp_m_freemode_01's variation collection, which "ppgrow" adds to
static uint64_t g_pedPropsSitesSum = 0, g_pedPropsDataSum = 0;
static uint64_t g_pedPropsWidenedSum = 0;		// the sites' sum with the eleven B6 made B7 and nothing else changed
struct PedPropsEntry { uint32_t anchor; uint16_t props; };

// size bytes of the fake's heap, zeroed, 16-byte aligned
static unsigned char* PedPropsAlloc(size_t size)
{
	size = (size + 15) & ~static_cast<size_t>(15);
	if (!g_pedPropsHeap || g_pedPropsUsed + size > kPedPropsHeapSize)
	{
		wprintf(L"pedprops: the fake's heap is too small\n");
		ExitProcess(3);
	}
	unsigned char* const at = g_pedPropsHeap + g_pedPropsUsed;
	g_pedPropsUsed += size;
	return at;
}

// A model info at a model index, its bit set unless bit is false
static void PedPropsModel(int32_t index, uint32_t hash, uint8_t type, const void* collection, bool bit = true)
{
	unsigned char* const info = PedPropsAlloc(kPedPropsModelSize);
	memcpy(info + 0x18, &hash, sizeof(hash));
	info[0x9D] = type;
	memcpy(info + 0x240, &collection, sizeof(collection));
	memcpy(g_pedPropsArrayStorage + index * kPedPropsStride, &info, sizeof(info));
	if (bit)
		g_pedPropsBitmapStorage[index / 32] |= 1u << (index % 32);
}

// A variation collection with room for room files, none of them yet: the files' pointers at +0, their count at +8
static unsigned char* PedPropsCollection(size_t room)
{
	unsigned char* const collection = PedPropsAlloc(16);
	unsigned char* const files = PedPropsAlloc(room * sizeof(void*));
	memcpy(collection, &files, sizeof(files));
	return collection;
}

// A variation file of these anchor entries added to a collection: the file whole, its pointer, then the count - as the
// game adds a pack's while the measurement may be reading
static void PedPropsAddFile(unsigned char* collection, const PedPropsEntry* entries, size_t count)
{
	unsigned char* const file = PedPropsAlloc(kPedPropsFileSize);
	unsigned char* const anchors = PedPropsAlloc(count * kPedPropsEntrySize);
	for (size_t i = 0; i < count; i++)
	{
		memcpy(anchors + i * kPedPropsEntrySize + 8, &entries[i].props, sizeof(entries[i].props));
		memcpy(anchors + i * kPedPropsEntrySize + 0x10, &entries[i].anchor, sizeof(entries[i].anchor));
	}
	const uint16_t anchorCount = static_cast<uint16_t>(count);
	memcpy(file + 0x58, &anchors, sizeof(anchors));
	memcpy(file + 0x60, &anchorCount, sizeof(anchorCount));
	unsigned char* files = nullptr;
	uint16_t fileCount = 0;
	memcpy(&files, collection, sizeof(files));
	memcpy(&fileCount, collection + 8, sizeof(fileCount));
	memcpy(files + fileCount * sizeof(void*), &file, sizeof(file));
	MemoryBarrier();
	*reinterpret_cast<volatile uint16_t*>(collection + 8) = static_cast<uint16_t>(fileCount + 1);
}

// A ped model of one variation file
static void PedPropsPed(int32_t index, uint32_t hash, const PedPropsEntry* entries, size_t count, bool bit = true)
{
	unsigned char* const collection = PedPropsCollection(1);
	PedPropsAddFile(collection, entries, count);
	PedPropsModel(index, hash, 6, collection, bit);
}

// The store and its models, as the top of this file describes them
static void PedPropsBuild(bool many, bool unknown, bool flood, bool unreadable, bool huge)
{
	g_pedPropsHeap = static_cast<unsigned char*>(VirtualAlloc(nullptr, kPedPropsHeapSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	g_pedPropsNoAccess = static_cast<unsigned char*>(VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS));
	g_pedPropsBitmapStorage = reinterpret_cast<uint32_t*>(PedPropsAlloc(kPedPropsSlots / 8));
	g_pedPropsArrayStorage = PedPropsAlloc(kPedPropsSlots * kPedPropsStride);
	*g_pedPropsStoreCount = kPedPropsIndices;
	*g_pedPropsStoreStride = kPedPropsStride;
	// other models every 100th index, never a ped - their +0x240 the page of no access, which a module that took one for
	// a ped could not read
	static const uint8_t kOtherTypes[] = { 1, 2, 3, 4, 5, 7, 0x25 };
	for (int32_t index = 0; index < kPedPropsIndices; index += 100)
		PedPropsModel(index, 0x3C000000u + index, kOtherTypes[(index / 100) % 7], g_pedPropsNoAccess);
	// mp_m_freemode_01: its first file with 38 props on anchor 0, 20 on anchor 1 and 2 on anchor 6, then a second entry
	// of anchor 0, which the game does not count, one of anchor 5 with none (and with "ppunknown" one of anchor 13); the
	// others 6 on anchor 0, the 2nd to the 10th 3 on anchor 1 too
	g_pedPropsMale = PedPropsCollection(50);
	const PedPropsEntry maleFirst[] = { { 0, 38 }, { 1, 20 }, { 6, 2 }, { 0, 99 }, { 5, 0 }, { 13, 9 } };
	PedPropsAddFile(g_pedPropsMale, maleFirst, unknown ? 6 : 5);
	const PedPropsEntry maleOthers[] = { { 0, 6 }, { 1, 3 } };
	for (int file = 2; file <= (many ? 45 : 30); file++)
		PedPropsAddFile(g_pedPropsMale, maleOthers, file <= 10 ? 2 : 1);
	PedPropsModel(30001, 0x705E61F2, 6, g_pedPropsMale);
	// mp_f_freemode_01, its type byte 0x46 - flags above the low 5 bits: its first file with 40 props on anchor 0 and 5
	// on anchor 2, the others 5 and 3 (with "ppmany" a 29th with 270 on anchor 2; with "ppunreadable" its 5th file's
	// pointer leads to the page of no access)
	unsigned char* const female = PedPropsCollection(30);
	const PedPropsEntry femaleFirst[] = { { 0, 40 }, { 2, 5 } };
	const PedPropsEntry femaleOthers[] = { { 0, 5 }, { 2, 3 } };
	PedPropsAddFile(female, femaleFirst, 2);
	for (int file = 2; file <= 28; file++)
		PedPropsAddFile(female, femaleOthers, 2);
	const PedPropsEntry femaleMany[] = { { 2, 270 } };
	if (many)
		PedPropsAddFile(female, femaleMany, 1);
	if (unreadable)
	{
		unsigned char* files = nullptr;
		memcpy(&files, female, sizeof(files));
		memcpy(files + 4 * sizeof(void*), &g_pedPropsNoAccess, sizeof(g_pedPropsNoAccess));
	}
	PedPropsModel(30002, 0x9C9EFFD8, 0x46, female);
	// 40 more peds of one file, 3 props on anchor 0 and 2 on anchor 1; a ped without variation data; a null model info;
	// a ped whose bit is clear; the last index's ped; one past the count, its bit set
	const PedPropsEntry oneFile[] = { { 0, 3 }, { 1, 2 } };
	for (int32_t k = 1; k <= 40; k++)
		PedPropsPed(30002 + k, 0x2A000000u + k, oneFile, 2);
	PedPropsModel(30043, 0x2A0000FFu, 6, nullptr);
	g_pedPropsBitmapStorage[30044 / 32] |= 1u << (30044 % 32);
	const PedPropsEntry clear[] = { { 0, 500 } };
	PedPropsPed(30045, 0x2A000100u, clear, 1, false);
	const PedPropsEntry last[] = { { 3, 7 } };
	PedPropsPed(kPedPropsIndices - 1, 0x2B000000u, last, 1);
	const PedPropsEntry past[] = { { 0, 600 } };
	PedPropsPed(kPedPropsIndices, 0x2B000001u, past, 1);
	// with "pphuge" one more ped of 258 files with 255 props on anchor 0 each, 65790 in all
	if (huge)
	{
		unsigned char* const collection = PedPropsCollection(258);
		const PedPropsEntry full[] = { { 0, 255 } };
		for (int file = 0; file < 258; file++)
			PedPropsAddFile(collection, full, 1);
		PedPropsModel(30046, 0x2D000000u, 6, collection);
	}
	// with "ppflood" 10,001 more peds, each with one file of 300 props on anchor 1, between the other models
	const PedPropsEntry flooding[] = { { 1, 300 } };
	for (int32_t index = 40001, made = 0; flood && made < 10001; index++)
		if (index % 100 != 0)
			PedPropsPed(index, 0x2C000000u + made++, flooding, 1);
}

// FNV-1a over the ped props' sites, or over the store's data and its globals: the report compares both with the sums
// taken before TweaksV loaded
static uint64_t PedPropsSum(bool sites)
{
	uint64_t hash = 0xCBF29CE484222325ull;
	const auto mix = [&hash](const volatile unsigned char* bytes, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			hash = (hash ^ bytes[i]) * 0x100000001B3ull;
	};
	if (sites)
	{
		for (const Site& site : g_pedPropsSites)
			mix(site.bytes, site.size);
		return hash;
	}
	mix(g_pedPropsHeap, g_pedPropsUsed);
	mix(reinterpret_cast<const volatile unsigned char*>(&g_pedPropsCountStorage), sizeof(g_pedPropsCountStorage));
	mix(reinterpret_cast<const volatile unsigned char*>(&g_pedPropsBitmapStorage), sizeof(g_pedPropsBitmapStorage));
	mix(reinterpret_cast<const volatile unsigned char*>(&g_pedPropsArrayStorage), sizeof(g_pedPropsArrayStorage));
	mix(reinterpret_cast<const volatile unsigned char*>(&g_pedPropsStrideStorage), sizeof(g_pedPropsStrideStorage));
	return hash;
}

// "ppgrow": 5 more files of mp_m_freemode_01, 6 props on anchor 0 each, as the game adds a pack's; the data's sum taken
// anew
static void PedPropsGrow()
{
	const PedPropsEntry entries[] = { { 0, 6 } };
	for (int i = 0; i < 5; i++)
		PedPropsAddFile(g_pedPropsMale, entries, 1);
	g_pedPropsDataSum = PedPropsSum(false);
}

// The texture budget's data, laid out as the game's: the counters - the buffers' and the textures' bytes and numbers, the game's
// high-water mark of both, the most objects allowed and the four budgets in bytes - the index of the budget in use, the update
// check's flag and its switch with its three factors, on a page of their own, so that "tbunreadable" can take the page away
// while TweaksV starts; and the function of the game's the call after the budgets' stores leads to, which the game's start-up
// calls through that call (TextureBudgetStartUp, below). The harness reaches the data only through this pointer, as it does
// the arrays above, and keeps what it set, to tell afterwards that TweaksV wrote nothing but the budgets and the call's rel32,
// with what the sites held before TweaksV loaded.
struct TextureBudgetCounters { uint64_t buffers, textures; uint32_t buffersNumber, texturesNumber; uint64_t highest; uint32_t most, unused;
	uint64_t budgets[4]; };
static_assert(sizeof(TextureBudgetCounters) == 0x48 && offsetof(TextureBudgetCounters, highest) == 0x18 &&
	offsetof(TextureBudgetCounters, most) == 0x20 && offsetof(TextureBudgetCounters, budgets) == 0x28, "the counters as the game lays them out");
struct TextureBudgetSwitch { int32_t on; float raise, lower, windows; };
// The descriptor heap as the game lays it out, with the free list its +0x18 points to: a dword per descriptor, the index of the next free
// one, the first free one's at +0x20, the list's size at +0x2C and in the two dwords of its size, which the game adds up
struct TextureBudgetHeap { void* object; void* cpuStart; void* gpuStart; uint32_t* list; uint32_t head, ringBase, increment, count; uint64_t lock; };
static_assert(offsetof(TextureBudgetHeap, list) == 0x18 && offsetof(TextureBudgetHeap, head) == 0x20 && offsetof(TextureBudgetHeap, count) == 0x2C,
	"the heap as the game lays it out");
struct TextureBudgetData { TextureBudgetCounters counters; int32_t index; uint8_t flag; TextureBudgetSwitch updateCheck; uint32_t heapSizes[2]; TextureBudgetHeap heap; };
struct TextureBudgetPage { TextureBudgetData data; unsigned char rest[4096 - sizeof(TextureBudgetData)]; };
static_assert(sizeof(TextureBudgetPage) == 4096, "the data alone on their page");
alignas(4096) static TextureBudgetPage g_textureBudgetPage;
static TextureBudgetData* volatile g_textureBudgetData = &g_textureBudgetPage.data;
static TextureBudgetData g_textureBudgetDataAsSet;
static uint64_t g_textureBudgetDummy[8];	// where the writers of the budgets lead without their options
alignas(4096) static unsigned char g_textureBudgetIndexPage[4096];	// where the getter's index lies with "tbindexunreadable"
alignas(4096) static unsigned char g_textureBudgetTwoPages[2 * 4096];	// where the data lie with "tbhalfraise"
static bool g_textureBudgetHoldRaise = false;
static uint32_t g_textureBudgetSizes[2] = { 0x10000, 0x20000 };		// the two dwords the game adds up, as the game's start-up has them
static TextureBudgetHeap g_textureBudgetHeapMade = {};				// the heap as the game's device makes it, laid out in main
static unsigned char g_textureBudgetSitesAsPlaced[sizeof(g_textureBudgetStores) + sizeof(g_textureBudgetGetter) +
	sizeof(g_textureBudgetPlacement) + sizeof(g_textureBudgetUpdate) + sizeof(g_textureBudgetWriters) + sizeof(g_textureBudgetHeapSite) + sizeof(g_textureBudgetMaker)];
static unsigned char g_textureBudgetMisreadAsPlaced[sizeof(g_textureBudgetMisread)];	// the readings' block as placed
// The game's function the call after the budgets' stores leads to: it counts its calls, checks the four arguments the start-up
// hands it, keeps the budgets and the most as they are when it runs - after TweaksV's raise when the call is redirected - and gives
// back a result the start-up checks.
constexpr uint64_t kTextureBudgetArguments[] = { 0x1111111111111111ull, 0x2222222222222222ull, 0x3333333333333333ull, 0x4444444444444444ull };
constexpr uint64_t kTextureBudgetResult = 0x5A5A5A5A5A5A5A5Aull;
static volatile int g_textureBudgetNextSteps = 0;
static bool g_textureBudgetArgumentsAsHanded = true;
static uint64_t g_textureBudgetBudgetsThen[4] = {};
static uint32_t g_textureBudgetMostThen = 0;
static __declspec(noinline) uint64_t TextureBudgetNextStep(uint64_t first, uint64_t second, uint64_t third, uint64_t fourth)
{
	g_textureBudgetNextSteps = g_textureBudgetNextSteps + 1;
	if (first != kTextureBudgetArguments[0] || second != kTextureBudgetArguments[1] || third != kTextureBudgetArguments[2] ||
		fourth != kTextureBudgetArguments[3])
		g_textureBudgetArgumentsAsHanded = false;
	for (int i = 0; i < 4; i++)
		g_textureBudgetBudgetsThen[i] = g_textureBudgetData->counters.budgets[i];
	g_textureBudgetMostThen = g_textureBudgetData->counters.most;
	return kTextureBudgetResult;
}

// Where the call after the budgets' stores leads: to the next step, to a TweaksV stub - read+execute memory outside the game -
// or elsewhere, where "another mod" is and nothing is.
static const unsigned char* Rel32Target(const Site& site);
static const unsigned char* ImageBase();
static const unsigned char* ImageEnd();
static const wchar_t* TextureBudgetCallLeads(const unsigned char* target)
{
	if (target == reinterpret_cast<const unsigned char*>(&TextureBudgetNextStep))
		return L"the next step";
	MEMORY_BASIC_INFORMATION info{};
	if ((target < ImageBase() || target >= ImageEnd()) && VirtualQuery(target, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT &&
		info.Protect == PAGE_EXECUTE_READ)
		return L"a TweaksV stub";
	return L"elsewhere";
}

// The game's start-up, after TweaksV's DllMain - with "tbearly" before it: its stores of the most and the four budgets from
// gameconfig.xml - the game's own file's 98304 objects and 1500, 1500, 3000 and 3000 MB, or with "tbconfigbig" a file that gives
// budgets 2 and 3 9000 MB, or with "tbconfig140000" and the like another most - then the call after them, with the four arguments
// the next step checks: through TweaksV's stub when TweaksV redirected it, to the next step when not, and not made when it leads to
// another mod.
static bool g_textureBudgetBigConfig = false;
static uint32_t g_textureBudgetConfigMost = 98304;
static wchar_t g_textureBudgetStartUp[224] = L"start-up: not made";
static void TextureBudgetStartUp()
{
	TextureBudgetData& data = *g_textureBudgetData;
	data.counters.most = g_textureBudgetConfigMost;
	const uint64_t budgetsMb[] = { 1500, 1500, g_textureBudgetBigConfig ? 9000u : 3000u, g_textureBudgetBigConfig ? 9000u : 3000u };
	for (int i = 0; i < 4; i++)
		data.counters.budgets[i] = budgetsMb[i] << 20;
	g_textureBudgetDataAsSet = data;
	const unsigned char* const target = Rel32Target(g_textureBudgetCall);
	const wchar_t* const leads = TextureBudgetCallLeads(target);
	const size_t size = sizeof(g_textureBudgetStartUp) / sizeof(g_textureBudgetStartUp[0]);
	if (wcscmp(leads, L"elsewhere") == 0)
	{
		swprintf(g_textureBudgetStartUp, size, L"start-up: the call leads elsewhere, not made");
		return;
	}
	using NextStep = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t);
	const auto next = reinterpret_cast<NextStep>(const_cast<unsigned char*>(target));
	if (g_textureBudgetHoldRaise)	// a write of budget 2 or 3 now stops in TextureBudgetHoldRaise, below
	{
		DWORD old = 0;
		VirtualProtect(g_textureBudgetTwoPages + 4096, 4096, PAGE_READONLY, &old);
	}
	const uint64_t result = next(kTextureBudgetArguments[0], kTextureBudgetArguments[1], kTextureBudgetArguments[2], kTextureBudgetArguments[3]);
	const int calls = g_textureBudgetNextSteps;
	swprintf(g_textureBudgetStartUp, size, L"start-up: through %ls, the next step called %d time%ls, its arguments %ls, its result %ls, the budgets then "
		L"%llu, %llu, %llu and %llu MB, the most then %u", leads, calls, calls == 1 ? L"" : L"s", g_textureBudgetArgumentsAsHanded ? L"as handed" : L"CHANGED",
		result == kTextureBudgetResult ? L"back" : L"CHANGED", static_cast<unsigned long long>(g_textureBudgetBudgetsThen[0] >> 20),
		static_cast<unsigned long long>(g_textureBudgetBudgetsThen[1] >> 20), static_cast<unsigned long long>(g_textureBudgetBudgetsThen[2] >> 20),
		static_cast<unsigned long long>(g_textureBudgetBudgetsThen[3] >> 20), g_textureBudgetMostThen);
}

// The data as the game has them, at the game's own sizes, but for the most objects and the budgets, which its start-up stores:
// textures 2456 MB and buffers 389 MB in 22222 and 1234 objects, the mark at 2900 MB, the texture quality's index 2, the update
// check's flag down and its switch on with its factors 0.9, 0.8 and 0.95, and the descriptor heap as the device makes it
static void TextureBudgetSetAsTheGame(TextureBudgetData& data)
{
	data.heapSizes[0] = g_textureBudgetSizes[0];
	data.heapSizes[1] = g_textureBudgetSizes[1];
	data.heap = g_textureBudgetHeapMade;
	data.counters.buffers = (389ull << 20) + 12345;
	data.counters.textures = (2456ull << 20) + 54321;
	data.counters.buffersNumber = 1234;
	data.counters.texturesNumber = 22222;
	data.counters.highest = 2900ull << 20;
	data.index = 2;
	data.flag = 0;
	data.updateCheck = { 1, 0.9f, 0.8f, 0.95f };
}

// The seven sites' bytes in a row, to keep them as placed or to compare them with that
static void TextureBudgetSites(unsigned char* to, bool compare, int& asPlaced)
{
	const volatile unsigned char* const sites[] = { g_textureBudgetStores, g_textureBudgetGetter, g_textureBudgetPlacement, g_textureBudgetUpdate,
		g_textureBudgetWriters, g_textureBudgetHeapSite, g_textureBudgetMaker };
	const size_t sizes[] = { sizeof(g_textureBudgetStores), sizeof(g_textureBudgetGetter), sizeof(g_textureBudgetPlacement), sizeof(g_textureBudgetUpdate),
		sizeof(g_textureBudgetWriters), sizeof(g_textureBudgetHeapSite), sizeof(g_textureBudgetMaker) };
	const volatile unsigned char* const call = g_textureBudgetCall.bytes + g_textureBudgetCall.offset;	// its rel32, which the raise redirects
	asPlaced = 0;
	for (int i = 0; i < 7; i++)
	{
		bool same = true;
		for (size_t j = 0; j < sizes[i]; j++, to++)
			if (!compare)
				*to = sites[i][j];
			else if (*to != sites[i][j] && (&sites[i][j] < call || &sites[i][j] >= call + 4))
				same = false;
		asPlaced += same ? 1 : 0;
	}
}

// How many texture budget summaries TweaksV.log holds - read as the game reads its other files, shared
static int TextureBudgetSummaries()
{
	static char text[4 << 20];
	const HANDLE file = CreateFileW(L"TweaksV.log", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	DWORD read = 0;
	if (!ReadFile(file, text, sizeof(text) - 1, &read, nullptr))
		read = 0;
	CloseHandle(file);
	text[read] = '\0';
	int summaries = 0;
	for (const char* at = text; (at = strstr(at, " objects allowed (MaxStreamingResources")) != nullptr; at++)
		summaries++;
	return summaries;
}

// tbgrow, tbhigh, tblate and tbhighdesc: what the game does with the data once TweaksV's first summary is in its log - the textures
// grow by 200 MB and 800 objects, past the budget, the game's mark with them, and the update check's flag goes up, as the game's
// streaming would have it (grow); one new high at a time, each a single store and each once the summary before it is in the log - the
// textures' bytes by 100 MB, then the game's mark to 2950 MB, then the textures by 400 objects - the flag left down (high); the
// data, all 0 so far, set as the game's start-up sets them (late); or 100 descriptors more taken from the heap's free list, nothing
// else changed (descriptors). Each change is followed by the wait for its summary.
enum class TextureBudgetChange { Grow, High, Late, Descriptors };
static wchar_t g_textureBudgetLater[64] = L"";
static bool TextureBudgetWaitFor(int summaries, int ms)
{
	for (int waited = 0; TextureBudgetSummaries() < summaries && waited < ms; waited += 250)
		Sleep(250);
	return TextureBudgetSummaries() >= summaries;
}
static void TextureBudgetLater(TextureBudgetChange change)
{
	const size_t size = sizeof(g_textureBudgetLater) / sizeof(g_textureBudgetLater[0]);
	if (!TextureBudgetWaitFor(1, 25000))
	{
		swprintf(g_textureBudgetLater, size, L", nothing done: no first summary");
		return;
	}
	TextureBudgetData& data = *g_textureBudgetData;
	if (change == TextureBudgetChange::Grow)
	{
		data.counters.textures += 200ull << 20;
		data.counters.texturesNumber += 800;
		data.counters.highest = data.counters.buffers + data.counters.textures;
		data.flag = 1;
	}
	else if (change == TextureBudgetChange::Late)
	{
		TextureBudgetSetAsTheGame(data);
		TextureBudgetStartUp();
	}
	else if (change == TextureBudgetChange::Descriptors)
		data.heap.head += 100;
	else
	{
		data.counters.textures += 100ull << 20;
		g_textureBudgetDataAsSet = data;
		TextureBudgetWaitFor(2, 10000);
		data.counters.highest = 2950ull << 20;
		g_textureBudgetDataAsSet = data;
		TextureBudgetWaitFor(3, 10000);
		data.counters.texturesNumber += 400;
	}
	g_textureBudgetDataAsSet = data;
	TextureBudgetWaitFor(change == TextureBudgetChange::High ? 4 : 2, 10000);
	swprintf(g_textureBudgetLater, size, change == TextureBudgetChange::Grow ? L", grown after the first summary" :
		change == TextureBudgetChange::High ? L", highs raised one at a time after the first summary" :
		change == TextureBudgetChange::Descriptors ? L", descriptors taken after the first summary" : L", set after the first summary");
}

// tbhalfraise: a write to the second page, the raise's of budget 2, is held 12 seconds, past TweaksV's first summary, 10 seconds in,
// and with TWEAKSV_TEST_SUMMARY_MS=500 past a reading whenever the start-up came; then the report gets how many summaries came
// meanwhile and the write goes on. Any other fault is not this handler's.
static int g_textureBudgetSummariesAtHold = 0;
static LONG CALLBACK TextureBudgetHoldRaise(EXCEPTION_POINTERS* pointers)
{
	const EXCEPTION_RECORD& record = *pointers->ExceptionRecord;
	const auto second = reinterpret_cast<ULONG_PTR>(g_textureBudgetTwoPages + 4096);
	if (record.ExceptionCode != EXCEPTION_ACCESS_VIOLATION || record.NumberParameters < 2 || record.ExceptionInformation[0] != 1 ||
		record.ExceptionInformation[1] < second || record.ExceptionInformation[1] >= second + 4096)
		return EXCEPTION_CONTINUE_SEARCH;
	g_textureBudgetSummariesAtHold = TextureBudgetSummaries();
	Sleep(12000);
	swprintf(g_textureBudgetLater, sizeof(g_textureBudgetLater) / sizeof(g_textureBudgetLater[0]), L", the raise held half done, %d summaries meanwhile",
		TextureBudgetSummaries() - g_textureBudgetSummariesAtHold);
	DWORD old = 0;
	VirtualProtect(g_textureBudgetTwoPages + 4096, 4096, PAGE_READWRITE, &old);
	return EXCEPTION_CONTINUE_EXECUTION;
}

// The Legacy packs check (SPECIFICATIONS 11.6): crafted RPF7 archives (PATTERNS.md, "RageOpenV"), written into the mods folder
// before TweaksV starts. An archive is its header of 16 bytes - RPF7, the entry count, the names' length, the encryption - its
// entries of 16 bytes, the root folder first with every other entry in it, the names, each ending in a zero, and then each
// binary's data at its block of 512 bytes; a resource's version is split into the top four bits of its system flags (+8) and
// of its graphics flags (+12).
static const uint32_t kLpOpen = 0x4E45504F;
static const uint32_t kLpNg = 0x0FEFFFFF;
static const uint32_t kLpAes = 0x0FFFFFF9;

struct LpEntry
{
	const char* name;
	int version;					// a resource's, or -1 for a binary
	const unsigned char* data;		// a binary's bytes: a file, or an archive nested in this one
	size_t size;
	uint32_t encryption;			// a binary's own
};

struct LpArchive
{
	unsigned char* bytes;			// from the process heap: LpFree
	size_t size;
};

static void* LpAllocate(size_t size)
{
	void* const memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size ? size : 1);
	if (!memory)
	{
		wprintf(L"lp: out of memory |\n");
		ExitProcess(3);
	}
	return memory;
}

static void LpFree(LpArchive& archive)
{
	if (archive.bytes)
		HeapFree(GetProcessHeap(), 0, archive.bytes);
	archive = {};
}

static void LpPut(unsigned char* at, uint32_t value, int bytes)
{
	for (int i = 0; i < bytes; i++)
		at[i] = static_cast<unsigned char>(value >> (8 * i));
}

static size_t LpBlocks(size_t size)
{
	return (size + 511) / 512 * 512;
}

static unsigned char* LpEntryAt(const LpArchive& archive, size_t index)
{
	return archive.bytes + 16 + index * 16;
}

// An archive of the root folder and the entries, the encryption given in its header, every binary's data after the names.
static LpArchive LpBuild(const LpEntry* entries, size_t count, uint32_t encryption = kLpOpen)
{
	size_t names = 1;	// the root's name, empty
	for (size_t i = 0; i < count; i++)
		names += strlen(entries[i].name) + 1;
	names = (names + 15) / 16 * 16;
	const size_t table = 16 + (count + 1) * 16;
	size_t size = LpBlocks(table + names);
	for (size_t i = 0; i < count; i++)
		if (entries[i].version < 0)
			size += LpBlocks(entries[i].size);
	const LpArchive archive = { static_cast<unsigned char*>(LpAllocate(size)), size };
	unsigned char* const out = archive.bytes;
	LpPut(out, 0x52504637, 4);
	LpPut(out + 4, static_cast<uint32_t>(count + 1), 4);
	LpPut(out + 8, static_cast<uint32_t>(names), 4);
	LpPut(out + 12, encryption, 4);
	LpPut(out + 20, 0x7FFFFF00, 4);		// the root folder: its name at 0, its entries from 1, count of them
	LpPut(out + 24, 1, 4);
	LpPut(out + 28, static_cast<uint32_t>(count), 4);
	size_t name = 1, data = LpBlocks(table + names);
	for (size_t i = 0; i < count; i++)
	{
		const LpEntry& e = entries[i];
		unsigned char* const entry = LpEntryAt(archive, i + 1);
		LpPut(entry, static_cast<uint32_t>(name), 2);
		memcpy(out + table + name, e.name, strlen(e.name));
		name += strlen(e.name) + 1;
		if (e.version >= 0)		// a resource: no size, at block 1 with the resource bit, the version's two halves
		{
			LpPut(entry + 5, 0x800001, 3);
			LpPut(entry + 8, static_cast<uint32_t>(e.version >> 4) << 28, 4);
			LpPut(entry + 12, static_cast<uint32_t>(e.version & 15) << 28, 4);
			continue;
		}
		LpPut(entry + 5, static_cast<uint32_t>(data / 512), 3);	// a binary: stored, at its block, its size and its encryption
		LpPut(entry + 8, static_cast<uint32_t>(e.size), 4);
		LpPut(entry + 12, e.encryption, 4);
		if (e.size)
			memcpy(out + data, e.data, e.size);
		data += LpBlocks(e.size);
	}
	return archive;
}

// The binaries first to last lead to the data of the binary at source: archives nested at one place.
static void LpShare(const LpArchive& archive, size_t source, size_t first, size_t last)
{
	for (size_t i = first; i <= last; i++)
		memcpy(LpEntryAt(archive, i) + 2, LpEntryAt(archive, source) + 2, 10);	// no compressed size, the block, the size
}

// An archive of 1,048,576 entries - the root folder and empty binaries, each named by the root's empty name - and 16 bytes of
// names: 16 MB of entry table, the most an entry table may have.
static LpArchive LpBig()
{
	const size_t count = 0x100000, size = 16 + count * 16 + 16;
	const LpArchive big = { static_cast<unsigned char*>(LpAllocate(size)), size };
	LpPut(big.bytes, 0x52504637, 4);
	LpPut(big.bytes + 4, static_cast<uint32_t>(count), 4);
	LpPut(big.bytes + 8, 16, 4);
	LpPut(big.bytes + 12, kLpOpen, 4);
	LpPut(big.bytes + 20, 0x7FFFFF00, 4);
	LpPut(big.bytes + 24, 1, 4);
	LpPut(big.bytes + 28, static_cast<uint32_t>(count - 1), 4);
	return big;
}

// An archive of n binaries named name, all leading to the archive inner.
static LpArchive LpNestedAtOnePlace(const char* name, const LpArchive& inner, size_t n)
{
	LpEntry* const entries = static_cast<LpEntry*>(LpAllocate(n * sizeof(LpEntry)));
	for (size_t i = 0; i < n; i++)
		entries[i] = { name, -1, i == 0 ? inner.bytes : nullptr, i == 0 ? inner.size : 0, 0 };
	const LpArchive archive = LpBuild(entries, n);
	LpShare(archive, 1, 2, n);
	HeapFree(GetProcessHeap(), 0, entries);
	return archive;
}

static const unsigned char kLpMeta[] = { '<', 'C', 'V', 'e', 'h', 'i', 'c', 'l', 'e', 'M', 'o', 'd', 'e', 'l', 'I', 'n', 'f', 'o', ' ', '/', '>' };
static const unsigned char kLpZip[] = { 0x50, 0x4B, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x21, 0x43, 0x65, 0x87, 0xA9, 0xCB, 0xED, 0x0F };
static const char kLpOddName[] = { 'l', 0x0A, 'p', static_cast<char>(0xC3), static_cast<char>(0xBC), '.', 'y', 'f', 't', 0 };	// l, a line feed, p, u umlaut

// The archive of a kind of lpmake and lpmany: false for a kind not known.
static bool LpMake(const wchar_t* kind, LpArchive& out)
{
	const LpEntry legacy[] = { { "lp.yft", 162 }, { "lp_hi.yft", 162 }, { "lp.ydr", 165 }, { "lp.ydd", 165 }, { "lp.ytd", 13 },
		{ "vehicles.meta", -1, kLpMeta, sizeof(kLpMeta), 0 } };
	const LpEntry enhanced[] = { { "lp.yft", 171 }, { "lp_hi.yft", 171 }, { "lp.ydr", 159 }, { "lp.ydd", 159 }, { "lp.ytd", 5 },
		{ "vehicles.meta", -1, kLpMeta, sizeof(kLpMeta), 0 } };
	const LpEntry enhancedModel[] = { { "lp.yft", 171 } };
	const LpEntry legacyModel[] = { { "lp.yft", 162 } };
	if (wcscmp(kind, L"legacy") == 0)				// models and textures with Legacy's versions, a vehicles.meta
		out = LpBuild(legacy, 6);
	else if (wcscmp(kind, L"enhanced") == 0)		// the same with Enhanced's
		out = LpBuild(enhanced, 6);
	else if (wcscmp(kind, L"textures") == 0)		// an Enhanced model, a texture dictionary with Legacy's version
	{
		const LpEntry textures[] = { { "lp.yft", 171 }, { "lp.ytd", 13 } };
		out = LpBuild(textures, 2);
	}
	else if (wcscmp(kind, L"others") == 0)			// Legacy's numbers on other kinds and names, another number on a ytd, a yft in capitals
	{
		const LpEntry others[] = { { "lp.ybn", 13 }, { "lp.ymap", 162 }, { "lp.yftx", 162 }, { "yft", 162 }, { "LP.YFT", 171 }, { "lp.ytd", 162 } };
		out = LpBuild(others, 6);
	}
	else if (wcscmp(kind, L"nested") == 0)			// a Legacy model in a nested archive; a nested archive not OPEN, one whose bytes are encrypted
	{
		LpArchive vehicles = LpBuild(legacyModel, 1), enc = LpBuild(legacyModel, 1, kLpNg), data = LpBuild(legacyModel, 1);
		const LpEntry outer[] = { { "lp.ytd", 5 }, { "vehicles.rpf", -1, vehicles.bytes, vehicles.size, 0 }, { "enc.rpf", -1, enc.bytes, enc.size, 0 },
			{ "data.rpf", -1, data.bytes, data.size, 1 } };
		out = LpBuild(outer, 4);
		LpFree(vehicles);
		LpFree(enc);
		LpFree(data);
	}
	else if (wcscmp(kind, L"deep8") == 0 || wcscmp(kind, L"deep9") == 0)	// a Legacy model 8 or 9 archives deep: l1.rpf holds l2.rpf, ...
	{
		LpArchive inner = LpBuild(legacyModel, 1);
		for (int level = kind[4] - L'0'; level >= 1; level--)
		{
			char name[8];
			snprintf(name, sizeof(name), "l%d.rpf", level);
			const LpEntry holder[] = { { name, -1, inner.bytes, inner.size, 0 } };
			LpArchive next = LpBuild(holder, 1);
			LpFree(inner);
			inner = next;
		}
		out = inner;
	}
	else if (wcscmp(kind, L"ng") == 0)				// the legacy kind, its header NG-encrypted
		out = LpBuild(legacy, 6, kLpNg);
	else if (wcscmp(kind, L"aes") == 0)				// AES-encrypted
		out = LpBuild(legacy, 6, kLpAes);
	else if (wcscmp(kind, L"plain") == 0)			// its encryption field 0, which names no encryption
		out = LpBuild(legacy, 6, 0);
	else if (wcscmp(kind, L"empty") == 0)			// no byte at all
		out = { static_cast<unsigned char*>(LpAllocate(0)), 0 };
	else if (wcscmp(kind, L"rootonly") == 0)		// the root folder alone
		out = LpBuild(nullptr, 0);
	else if (wcscmp(kind, L"badmagic") == 0)		// a zip's first bytes
	{
		out = LpBuild(legacy, 6);
		memcpy(out.bytes, kLpZip, 4);
	}
	else if (wcscmp(kind, L"nocount") == 0)			// no entries
	{
		out = LpBuild(legacy, 6);
		LpPut(out.bytes + 4, 0, 4);
	}
	else if (wcscmp(kind, L"toomany") == 0)			// a header alone, its count past the bound
	{
		out = LpBuild(nullptr, 0);
		LpPut(out.bytes + 4, 0x100001, 4);
		out.size = 16;
	}
	else if (wcscmp(kind, L"pastend") == 0)			// cut off inside its entry table
	{
		out = LpBuild(legacy, 6);
		out.size = 64;
	}
	else if (wcscmp(kind, L"pastnames") == 0)		// its names' length past its end
	{
		out = LpBuild(legacy, 6);
		LpPut(out.bytes + 8, 0x7FFFFFF0, 4);
	}
	else if (wcscmp(kind, L"noroot") == 0)			// a first entry that is no folder
	{
		out = LpBuild(legacy, 6);
		LpPut(LpEntryAt(out, 0) + 4, 0, 4);
	}
	else if (wcscmp(kind, L"badname") == 0)			// the first file's name past the names
	{
		out = LpBuild(legacy, 6);
		LpPut(LpEntryAt(out, 1), 0xFFFF, 2);
	}
	else if (wcscmp(kind, L"nestedout") == 0)		// a texture dictionary with Legacy's version, an archive nested past this one's end
	{
		LpArchive inner = LpBuild(enhancedModel, 1);
		const LpEntry outer[] = { { "lp.ytd", 13 }, { "out.rpf", -1, inner.bytes, inner.size, 0 } };
		out = LpBuild(outer, 2);
		LpPut(LpEntryAt(out, 2) + 5, 0x7FFFFE, 3);	// the last block a binary can have: 0x7FFFFF with no size makes a folder's mark, the top bit a resource
		LpFree(inner);
	}
	else if (wcscmp(kind, L"nestedself") == 0)		// an archive nested at this one's own header
	{
		LpArchive inner = LpBuild(enhancedModel, 1);
		const LpEntry outer[] = { { "self.rpf", -1, inner.bytes, inner.size, 0 } };
		out = LpBuild(outer, 1);
		LpPut(LpEntryAt(out, 1) + 5, 0, 3);
		LpFree(inner);
	}
	else if (wcscmp(kind, L"compressed") == 0)		// a nested archive with a compressed size
	{
		LpArchive inner = LpBuild(enhancedModel, 1);
		const LpEntry outer[] = { { "packed.rpf", -1, inner.bytes, inner.size, 0 } };
		out = LpBuild(outer, 1);
		LpPut(LpEntryAt(out, 1) + 2, 100, 3);
		LpFree(inner);
	}
	else if (wcscmp(kind, L"mixed") == 0)			// a nested archive whose bytes are encrypted, and a compressed one
	{
		LpArchive inner = LpBuild(enhancedModel, 1);
		const LpEntry outer[] = { { "enc.rpf", -1, inner.bytes, inner.size, 1 }, { "packed.rpf", -1, inner.bytes, inner.size, 0 } };
		out = LpBuild(outer, 2);
		LpPut(LpEntryAt(out, 2) + 2, 100, 3);
		LpFree(inner);
	}
	else if (wcscmp(kind, L"nestedlong") == 0)		// a nested archive inside its holder whose size runs past the holder's end
	{
		LpArchive inner = LpBuild(enhancedModel, 1);
		const LpEntry outer[] = { { "long.rpf", -1, inner.bytes, inner.size, 0 } };
		out = LpBuild(outer, 1);
		LpPut(LpEntryAt(out, 1) + 8, 0x7FFFFFFF, 4);
		LpFree(inner);
	}
	else if (wcscmp(kind, L"longname") == 0)		// a Legacy model whose name has no zero within 256 bytes
	{
		char name[257];
		memset(name, 'x', 252);
		memcpy(name + 252, ".yft", 5);
		const LpEntry model[] = { { name, 162 } };
		out = LpBuild(model, 1);
	}
	else if (wcscmp(kind, L"farname") == 0)			// a Legacy model whose name of 255 bytes starts at 65,535, the last offset a file's
	{												// name can have, in names of 70,000 bytes: only their first 65,792 are read
		const size_t names = 70000, size = LpBlocks(16 + 3 * 16 + names);
		out = { static_cast<unsigned char*>(LpAllocate(size)), size };
		LpPut(out.bytes, 0x52504637, 4);
		LpPut(out.bytes + 4, 3, 4);
		LpPut(out.bytes + 8, static_cast<uint32_t>(names), 4);
		LpPut(out.bytes + 12, kLpOpen, 4);
		LpPut(LpEntryAt(out, 0) + 4, 0x7FFFFF00, 4);	// the root folder, its entries 1 and 2
		LpPut(LpEntryAt(out, 0) + 8, 1, 4);
		LpPut(LpEntryAt(out, 0) + 12, 2, 4);
		LpPut(LpEntryAt(out, 1), 1, 2);					// a binary named "f", no bytes, at the end
		LpPut(LpEntryAt(out, 1) + 5, static_cast<uint32_t>(size / 512), 3);
		LpPut(LpEntryAt(out, 2), 0xFFFF, 2);			// the model
		LpPut(LpEntryAt(out, 2) + 5, 0x800001, 3);
		LpPut(LpEntryAt(out, 2) + 8, 0xAu << 28, 4);
		LpPut(LpEntryAt(out, 2) + 12, 0x2u << 28, 4);
		unsigned char* const nameBytes = out.bytes + 16 + 3 * 16;
		nameBytes[1] = 'f';
		memset(nameBytes + 0xFFFF, 'x', 251);
		memcpy(nameBytes + 0xFFFF + 251, ".yft", 4);	// its zero at 65,790, inside the 65,792 bytes read
	}
	else if (wcscmp(kind, L"notrpf") == 0)			// a nested .rpf that is a zip
	{
		const LpEntry outer[] = { { "junk.rpf", -1, kLpZip, sizeof(kLpZip), 0 } };
		out = LpBuild(outer, 1);
	}
	else if (wcscmp(kind, L"oddname") == 0)			// a Legacy model whose name has a line feed and a byte outside ASCII
	{
		const LpEntry odd[] = { { kLpOddName, 162 } };
		out = LpBuild(odd, 1);
	}
	else if (wcscmp(kind, L"manynested") == 0 || wcscmp(kind, L"manynestedok") == 0)	// 4096 or 4095 archives nested at one place
	{
		const LpEntry textures[] = { { "lp.ytd", 5 } };
		LpArchive inner = LpBuild(textures, 1);
		out = LpNestedAtOnePlace("n.rpf", inner, kind[10] ? 4095 : 4096);
		LpFree(inner);
	}
	else if (wcscmp(kind, L"bigtables") == 0 || wcscmp(kind, L"heavy") == 0)	// 5 or 3 archives of 16 MB of entry table, at one place
	{
		LpArchive big = LpBig();
		out = LpNestedAtOnePlace("big.rpf", big, kind[0] == L'b' ? 5 : 3);
		LpFree(big);
	}
	else
		return false;
	return true;
}

// The folders on the way to path ("\\?\C:\...\mods\a\x.rpf": each folder from the drive on; those that are there stay).
static const size_t kLpLongPath = 32768;
static void LpMakeFolders(const wchar_t* path)
{
	static wchar_t folder[kLpLongPath];
	for (size_t i = 0; path[i] && i < kLpLongPath - 1; i++)
	{
		if (path[i] == L'\\')
		{
			folder[i] = 0;
			CreateDirectoryW(folder, nullptr);
		}
		folder[i] = path[i];
	}
}

// path in full, \\?\ in front, so that a path below mods\ of 254 bytes and more is reached too; null when it cannot be read
static wchar_t* LpInFull(const wchar_t* relative)
{
	static wchar_t path[kLpLongPath];
	memcpy(path, L"\\\\?\\", 4 * sizeof(wchar_t));
	const DWORD length = GetFullPathNameW(relative, static_cast<DWORD>(kLpLongPath - 4), path + 4, nullptr);
	return length == 0 || length >= kLpLongPath - 4 ? nullptr : path;
}

static bool LpWrite(const wchar_t* relative, const LpArchive& archive)
{
	const wchar_t* const path = LpInFull(relative);
	if (!path)
		return false;
	LpMakeFolders(path);
	const HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return false;
	DWORD written = 0;
	const bool ok = archive.size == 0 || (WriteFile(file, archive.bytes, static_cast<DWORD>(archive.size), &written, nullptr) && written == archive.size);
	CloseHandle(file);
	return ok;
}

static int g_lpMade = 0;							// the archives lpmake and lpmany wrote
static HANDLE g_lpLocked = INVALID_HANDLE_VALUE;	// lplock's file
static HANDLE g_lpBadName = INVALID_HANDLE_VALUE;	// lpbadname's, gone when the fake ends
static const wchar_t* g_lpWaited = nullptr;			// how lpwait ended
static int g_lpFree = 0, g_lpHeld = 0;				// what lpfree found: the packs free, each with its folder, and those not

// lpmake, lpmany, lplock and lpbadname, before TweaksV starts.
static void LpSetup(int argc, wchar_t** argv)
{
	for (int i = 2; i < argc; i++)
	{
		if (wcscmp(argv[i], L"lpmake") == 0 && i + 2 < argc)
		{
			LpArchive archive = {};
			if (LpMake(argv[i + 1], archive) && LpWrite(argv[i + 2], archive))
				g_lpMade++;
			else
				wprintf(L"lp: %ls NOT made at %ls | ", argv[i + 1], argv[i + 2]);
			LpFree(archive);
			i += 2;
		}
		else if (wcscmp(argv[i], L"lpmany") == 0 && i + 3 < argc)
		{
			const int count = _wtoi(argv[i + 2]);
			wchar_t first[MAX_PATH], path[MAX_PATH];
			swprintf(first, MAX_PATH, L"%ls/p%05d.rpf", argv[i + 3], 1);
			LpArchive archive = {};
			if (count > 0 && LpMake(argv[i + 1], archive) && LpWrite(first, archive))
			{
				g_lpMade++;
				for (int n = 2; n <= count; n++)
				{
					swprintf(path, MAX_PATH, L"%ls/p%05d.rpf", argv[i + 3], n);
					if (CreateHardLinkW(path, first, nullptr) || LpWrite(path, archive))
						g_lpMade++;
				}
			}
			else
				wprintf(L"lp: %ls NOT made in %ls | ", argv[i + 1], argv[i + 3]);
			LpFree(archive);
			i += 3;
		}
		else if (wcscmp(argv[i], L"lplock") == 0 && i + 1 < argc)	// held without sharing until the report
		{
			g_lpLocked = CreateFileW(argv[i + 1], GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (g_lpLocked == INVALID_HANDLE_VALUE)
				wprintf(L"lp: %ls NOT held | ", argv[i + 1]);
			i += 1;
		}
		else if (wcscmp(argv[i], L"lpbadname") == 0)	// held open with delete on close until the fake ends, then gone: no shell has
		{												// to delete a name it cannot spell
			wchar_t name[] = L"mods\\lpbad_.rpf";
			name[10] = static_cast<wchar_t>(0xD800);	// the _: half of a surrogate pair, alone
			LpArchive archive = {};
			LpMake(L"legacy", archive);
			g_lpBadName = CreateFileW(name, GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
			DWORD written = 0;
			const bool made = g_lpBadName != INVALID_HANDLE_VALUE &&
				WriteFile(g_lpBadName, archive.bytes, static_cast<DWORD>(archive.size), &written, nullptr) && written == archive.size;
			wprintf(L"lp: bad name %ls | ", made ? L"made" : L"NOT made");
			LpFree(archive);
		}
	}
}

// lpwait: the check's last line in TweaksV.log - or its problem that it is not done - read as TweaksV writes it, every 20 ms, on from
// where the last read ended, for 40 s at most; then 1.5 s for the box of its problem, which comes a second after the problem.
static bool LpFound(const char* bytes, size_t size, const char* text)
{
	const size_t length = strlen(text);
	for (size_t i = 0; i + length <= size; i++)
		if (memcmp(bytes + i, text, length) == 0)
			return true;
	return false;
}

// A file or folder opened without sharing, which fails while any handle is open on it, and closed again; a moment's hold of another
// program's, a virus scanner's, waited out, 2 s at most.
static bool LpOpensAlone(const wchar_t* path, DWORD flags)
{
	const ULONGLONG start = GetTickCount64();
	for (;;)
	{
		const HANDLE file = CreateFileW(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, flags, nullptr);
		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
			return true;
		}
		if (GetLastError() != ERROR_SHARING_VIOLATION || GetTickCount64() - start >= 2000)
			return false;
		Sleep(20);
	}
}

static void LpCheckFree(const wchar_t* pack)
{
	wchar_t* const path = LpInFull(pack);
	bool alone = path && LpOpensAlone(path, FILE_ATTRIBUTE_NORMAL);
	if (alone)
	{
		*wcsrchr(path, L'\\') = L'\0';	// the folder it lies in, which a find handle holds
		alone = LpOpensAlone(path, FILE_FLAG_BACKUP_SEMANTICS);
	}
	(alone ? g_lpFree : g_lpHeld)++;
}

// lpfree: every pack lpmake and lpmany made, once the check is done.
static void LpCheckAllFree(int argc, wchar_t** argv)
{
	for (int i = 2; i < argc; i++)
	{
		if (wcscmp(argv[i], L"lpmake") == 0 && i + 2 < argc)
		{
			LpCheckFree(argv[i + 2]);
			i += 2;
		}
		else if (wcscmp(argv[i], L"lpmany") == 0 && i + 3 < argc)
		{
			for (int n = 1, count = _wtoi(argv[i + 2]); n <= count; n++)
			{
				wchar_t path[MAX_PATH];
				swprintf(path, MAX_PATH, L"%ls/p%05d.rpf", argv[i + 3], n);
				LpCheckFree(path);
			}
			i += 3;
		}
	}
}

static void LpWait(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"lpwait"))
		return;
	static const char kLast[] = "] Packs checked for GTA V Legacy versions: ";
	static const char kNotDone[] = "] PROBLEM: The check for packs made for GTA V Legacy is not done this session: ";
	const size_t lastSize = sizeof(kNotDone) - 1;	// the longer of the two
	static char buffer[65536 + sizeof(kNotDone)];
	size_t kept = 0;			// the end of the bytes read before, so that a line split between two reads is found too
	LARGE_INTEGER offset = {};
	const ULONGLONG start = GetTickCount64();
	bool found = false;
	while (!found && GetTickCount64() - start < 40000)
	{
		const HANDLE log = CreateFileW(L"TweaksV.log", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (log != INVALID_HANDLE_VALUE)
		{
			DWORD read = 0;
			while (!found && SetFilePointerEx(log, offset, nullptr, FILE_BEGIN) && ReadFile(log, buffer + kept, 65536, &read, nullptr) && read > 0)
			{
				offset.QuadPart += read;
				const size_t size = kept + read;
				found = LpFound(buffer, size, kLast) || LpFound(buffer, size, kNotDone);
				kept = size < lastSize ? size : lastSize;
				memmove(buffer, buffer + size - kept, kept);
			}
			CloseHandle(log);
		}
		if (!found)
			Sleep(20);
	}
	if (found)
		Sleep(1500);
	g_lpWaited = found ? L"done" : L"no last line in 40 s";
	if (HasOption(argc, argv, L"lpfree"))
		LpCheckAllFree(argc, argv);
}

static void LpReport()
{
	if (g_lpMade)
		wprintf(L"lp: %d made | ", g_lpMade);
	if (g_lpWaited)
		wprintf(L"legacy packs: %ls | ", g_lpWaited);
	if (g_lpHeld)
		wprintf(L"lp: %d of %d held | ", g_lpHeld, g_lpFree + g_lpHeld);
	else if (g_lpFree)
		wprintf(L"lp: all %d free | ", g_lpFree);
	if (g_lpLocked != INVALID_HANDLE_VALUE)
		CloseHandle(g_lpLocked);
}

// The ray tracing buffer's data, in the fake's data as the game's are in its own, reached only through pointers: the frame slot and
// a second one for "rtslot", the two counters - a dword per slot - with room past them for "rtcounters", and the buffer object's
// global; the buffer object the fake's maker hands out, its count at +0x08 and its stride in the low 12 bits of the word at +0x0C;
// the maker; what a check's refusal records; the checks' runs; and the sites' bytes as placed.
static const unsigned char* ImageBase();
static uint32_t Imm32(const Site& site);
static void SetImm32(const Site& site, uint32_t value);
static uint32_t g_rtSlotStorage[2] = {};
static uint32_t* volatile g_rtSlot = g_rtSlotStorage;
alignas(8) static uint32_t g_rtCountersStorage[4] = {};
static uint32_t* volatile g_rtCounters = g_rtCountersStorage;
static uint64_t g_rtObjectStorage = 0;
static uint64_t* volatile g_rtObject = &g_rtObjectStorage;
alignas(4096) static unsigned char g_rtHoleStorage[4096];	// the hole: a page inside the game, no access with an option "...unread"
static unsigned char* volatile g_rtHole = g_rtHoleStorage;
struct RtBufferObject
{
	unsigned char head[0x08];
	uint32_t count;			// +0x08
	uint16_t stride;		// +0x0C: the stride in its low 12 bits, the maker's flags above them
	unsigned char rest[0x32];
};
static_assert(offsetof(RtBufferObject, count) == 0x08 && offsetof(RtBufferObject, stride) == 0x0C, "the buffer object at the game's offsets");
static RtBufferObject g_rtBufferStorage{};
static RtBufferObject* volatile g_rtBuffer = &g_rtBufferStorage;
static volatile LONG g_rtMakerCalls = 0;
static uint32_t g_rtMakerFlags = 0, g_rtMakerView = 0, g_rtMakerZero = 1;

// The game's buffer maker, where the set-up's call leads: (count, stride, its view's flag, 0, flags) - the buffer object with the
// count and the stride as the game's maker keeps them. Its call of GetCurrentThreadId gives it a frame of its own, which the
// fake's unwind data names.
static DWORD g_rtMakerThread = 0;
static __declspec(noinline) const void* RtBufferMaker(uint32_t count, uint32_t stride, uint8_t view, uint32_t zero, uint32_t flags)
{
	RtBufferObject& buffer = *g_rtBuffer;
	buffer.count = count;
	buffer.stride = static_cast<uint16_t>(0x7000 | (stride & 0xFFF));
	g_rtMakerView = view;
	g_rtMakerZero = zero;
	g_rtMakerFlags = flags;
	g_rtMakerThread = GetCurrentThreadId();
	InterlockedIncrement(&g_rtMakerCalls);
	return &buffer;
}

// What a check's refusal finds: every register but rax and the flags as the check left them; the six it does not set are given
// before it, and rsp is noted before it.
struct RtRecord
{
	uint64_t in[6];			// r8, r9, r11, r12, r13 and r14, given
	uint64_t rspBefore;		// +0x30
	uint64_t rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15, rsp;	// +0x38 to +0xA8: at the refusal
};
static_assert(offsetof(RtRecord, rspBefore) == 0x30 && offsetof(RtRecord, rbx) == 0x38 && offsetof(RtRecord, rsp) == 0xA8,
	"the record as the checks fill it");
using RtCheck = int (*)(uint64_t count, RtRecord* record);
constexpr uint32_t kRtFinished = 2811203, kRtUnderWay = 4000000, kRtTaken = 100, kRtRefused = 4400000, kRtGrown = 3987120, kRtFlipped = 4100000;

// The registers a refusal found, as the check left them: the counters and the slot it loaded, rdx or rbp 0, the slot's counter
// and the range's end, r10 1, the count with its upper half, the six given, the record and rsp.
static bool RtRegistersKept(const RtRecord& r, uint64_t rcx, bool ropes)
{
	const uint64_t counters = reinterpret_cast<uintptr_t>(g_rtCounters);
	const uint64_t base = ropes ? r.rsi : r.rbx, slot = ropes ? r.rdx : r.rbp, zero = ropes ? r.rbp : r.rdx, end = ropes ? r.rbx : r.rsi;
	return base == counters && slot == *g_rtSlot && zero == 0 && r.rdi == kRtUnderWay + kRtTaken &&
		end == static_cast<uint32_t>(r.rdi + static_cast<uint32_t>(rcx)) && r.rcx == rcx && r.r10 == 1 && r.r8 == r.in[0] && r.r9 == r.in[1] &&
		r.r11 == r.in[2] && r.r12 == r.in[3] && r.r13 == r.in[4] && r.r14 == r.in[5] && r.r15 == reinterpret_cast<uintptr_t>(&r) && r.rsp == r.rspBefore;
}

// One thread's calls of a check, each count past what is left - the first count, then one more each call - a record of its own
// each, the upper half of rcx not 0.
struct RtRun
{
	const volatile unsigned char* code;
	bool ropes;
	int calls;
	uint32_t count;
	int took, refused, kept;
	uint64_t vertices;
};
static DWORD WINAPI RtRunChecks(LPVOID parameter)
{
	RtRun& run = *static_cast<RtRun*>(parameter);
	for (int i = 0; i < run.calls; i++)
	{
		RtRecord record{};
		for (int k = 0; k < 6; k++)
			record.in[k] = (0x0101010101010101ull * static_cast<uint64_t>(k + 1)) ^ (static_cast<uint64_t>(i) << 24);
		const uint32_t count = run.count + static_cast<uint32_t>(i % 1000);
		const uint64_t rcx = 0xA5A5A5A500000000ull | count;
		if (reinterpret_cast<RtCheck>(const_cast<unsigned char*>(run.code))(rcx, &record))
		{
			run.took++;
			continue;
		}
		run.refused++;
		run.vertices += count;
		if (RtRegistersKept(record, rcx, run.ropes))
			run.kept++;
	}
	return 0;
}

// Where a check's refusal jump leads: the game's refusal, a TweaksV stub - read+execute, its counting instructions first and the
// game's refusal last - or elsewhere.
static const wchar_t* RtJumpWhere(size_t check)
{
	const unsigned char* const target = Rel32Target(g_rtJumps[check]);
	if (InImage(target))
		return L"the game";
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(target, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT || memcmp(target, "\xF0\x48\xFF\x05", 4) != 0)
		return L"elsewhere";
	const unsigned char* refusal = nullptr;
	memcpy(&refusal, target + 24, sizeof(refusal));
	const volatile unsigned char* const own = check == 0 ? g_rtGeometry + 112 : g_rtRopes + 109;
	return info.Protect != PAGE_EXECUTE_READ ? L"a stub NOT RX" : refusal == own ? L"a stub, RX, on to its refusal" : L"a stub elsewhere";
}

// How many times TweaksV.log, read whole and shared as TweaksV shares it while it writes, holds text; and a wait for it, 100 ms
// at a time and ms at most.
static int RtLogHas(const char* text)
{
	static char log[4 << 20];
	const HANDLE file = CreateFileW(L"TweaksV.log", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return 0;
	DWORD read = 0;
	if (!ReadFile(file, log, sizeof(log) - 1, &read, nullptr))
		read = 0;
	CloseHandle(file);
	log[read] = '\0';
	int found = 0;
	for (const char* at = log; (at = strstr(at, text)) != nullptr; at++)
		found++;
	return found;
}
static bool RtWaitFor(const char* text, int ms)
{
	for (int waited = 0; RtLogHas(text) == 0 && waited < ms; waited += 100)
		Sleep(100);
	return RtLogHas(text) != 0;
}

// Before TweaksV loads: the set-up's call to the maker and its store to the global, the checks' loads of the slot and the counters
// and the reset's lea - but where an option asks otherwise; then the sites' bytes as placed.
static unsigned char g_rtPlaced[kRtSites][208];
static void RtSetUp(int argc, wchar_t** argv)
{
	SetRel32(g_rtMakerCall, reinterpret_cast<const void*>(&RtBufferMaker));
	SetRel32(g_rtObjectRef, g_rtObject);
	for (const Site& site : g_rtSlotRefs)
		SetRel32(site, g_rtSlot);
	for (const Site& site : g_rtCountersRefs)
		SetRel32(site, g_rtCounters);
	if (HasOption(argc, argv, L"rtraised"))
	{
		SetImm32(g_rtCountSite, 8388608);
		SetImm32(g_rtCapacities[0], 8388608);
		SetImm32(g_rtCapacities[1], 8388608);
	}
	if (HasOption(argc, argv, L"rtmismatch"))
		SetImm32(g_rtCapacities[1], 8388608);
	if (HasOption(argc, argv, L"rtmakerhooked"))
		SetRel32(g_rtMakerCall, ImageEnd() + 0x90000);
	if (HasOption(argc, argv, L"rtmakerinside"))
		SetRel32(g_rtMakerCall, reinterpret_cast<const unsigned char*>(&RtBufferMaker) + 16);
	if (HasOption(argc, argv, L"rtslot"))
		SetRel32(g_rtSlotRefs[1], g_rtSlot + 1);
	if (HasOption(argc, argv, L"rtcounters"))
		SetRel32(g_rtCountersRefs[1], g_rtCounters + 1);
	if (HasOption(argc, argv, L"rtextra"))
		g_rtExtra[0] = 0x48;
	if (HasOption(argc, argv, L"rtjumphooked"))
		SetRel32(g_rtJumps[0], ImageEnd() + 0xA0000);
	if (HasOption(argc, argv, L"rtnounwind"))
		SetRel32(g_rtMakerCall, const_cast<const unsigned char*>(g_rtReset));
	if (HasOption(argc, argv, L"rtobjectout"))
		SetRel32(g_rtObjectRef, ImageEnd() + 0xB0000);
	if (HasOption(argc, argv, L"rtobjectunread"))
		SetRel32(g_rtObjectRef, g_rtHole);
	if (HasOption(argc, argv, L"rtslotout"))
		SetRel32(g_rtSlotRefs[1], ImageEnd() + 0xB2000);
	if (HasOption(argc, argv, L"rtslotunread"))
		SetRel32(g_rtSlotRefs[0], g_rtHole);
	if (HasOption(argc, argv, L"rtcountersout"))
		SetRel32(g_rtCountersRefs[0], ImageEnd() + 0xB4000);
	if (HasOption(argc, argv, L"rtcountersend"))
		SetRel32(g_rtCountersRefs[0], ImageEnd() - sizeof(uint32_t));
	if (HasOption(argc, argv, L"rtcountersunread"))
		SetRel32(g_rtCountersRefs[1], g_rtHole);
	if (HasOption(argc, argv, L"rtjumpunread"))
	{
		for (size_t i = 0; i < kRtRopesMatch; i++)
			g_rtEdge[kRtEdgeMatchAt + i] = g_rtRopes[42 + i];
		g_rtRopes[42] = 0x90;
		SetRel32(g_rtEdgeRefs[0], g_rtSlot);
		SetRel32(g_rtEdgeRefs[1], g_rtCounters);
	}
	if (HasOption(argc, argv, L"rtropeshooked"))
		SetRel32(g_rtJumps[1], ImageEnd() + 0xB6000);
	if (HasOption(argc, argv, L"rtnoreset"))
		g_rtReset[0] = 0xCC;
	if (HasOption(argc, argv, L"rtobjectodd"))
		SetRel32(g_rtObjectRef, reinterpret_cast<const unsigned char*>(g_rtObject) + 4);
	if (HasOption(argc, argv, L"rtslotodd"))
		SetRel32(g_rtSlotRefs[0], reinterpret_cast<const unsigned char*>(g_rtSlot) + 2);
	if (HasOption(argc, argv, L"rtcountersodd"))
		SetRel32(g_rtCountersRefs[1], reinterpret_cast<const unsigned char*>(g_rtCounters) + 2);
	const auto twin = [](const volatile unsigned char* match, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			g_rtTwin[i] = match[i];
	};
	if (HasOption(argc, argv, L"rtsetuptwice"))
		twin(g_rtSetUp + 4, 36);
	if (HasOption(argc, argv, L"rtgeometrytwice"))
		twin(g_rtGeometry + 42, 50);
	if (HasOption(argc, argv, L"rtropestwice"))
		twin(g_rtRopes + 42, kRtRopesMatch);
	if (HasOption(argc, argv, L"rtmade"))
		*g_rtObject = 0x0000BAD0B1EC7000ull;
	for (size_t i = 0; i < kRtSites; i++)
		for (size_t at = 0; at < g_rtSites[i].size && at < sizeof(g_rtPlaced[i]); at++)
			g_rtPlaced[i][at] = g_rtSites[i].bytes[at];
}

// Once the whole of .fakesit is read+execute, before TweaksV loads: what the options "...unread" take away.
static void RtTakeAway(int argc, wchar_t** argv)
{
	DWORD old = 0;
	if (HasOption(argc, argv, L"rtobjectunread") || HasOption(argc, argv, L"rtslotunread") || HasOption(argc, argv, L"rtcountersunread"))
		VirtualProtect(g_rtHole, sizeof(g_rtHoleStorage), PAGE_NOACCESS, &old);
	if (HasOption(argc, argv, L"rtjumpunread"))
		VirtualProtect(const_cast<unsigned char*>(g_rtEdge) + 0x1000, 0x1000, PAGE_NOACCESS, &old);
}

// Once TweaksV has started, as the game's renderer and its frames would: the buffer made through the set-up - unless "rtmade" made it
// already, or its call or its store leads elsewhere - and the counters as a frame leaves them; then, as asked, the checks and the frames.
static wchar_t g_rtChecks[160] = L"";
static void RtActions(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"rtmade") && Rel32Target(g_rtMakerCall) == reinterpret_cast<const unsigned char*>(&RtBufferMaker) &&
		Rel32Target(g_rtObjectRef) == reinterpret_cast<const unsigned char*>(g_rtObject))
		reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_rtSetUp))();
	uint32_t* const counters = g_rtCounters;
	counters[1] = kRtFinished;	// the last finished frame, slot 1's
	counters[0] = kRtUnderWay;	// the frame under way, slot 0's
	const bool run = wcscmp(RtJumpWhere(0), L"elsewhere") != 0 && wcscmp(RtJumpWhere(1), L"elsewhere") != 0;
	if (HasOption(argc, argv, L"rtcheck") && run)
	{
		RtRun took = { g_rtRopes, true, 1, kRtTaken, 0, 0, 0, 0 };	// 100 vertices fit: the counter at 4000100
		RtRunChecks(&took);
		RtRun runs[3] = { { g_rtGeometry, false, 600, kRtRefused, 0, 0, 0, 0 }, { g_rtGeometry, false, 600, kRtRefused, 0, 0, 0, 0 },
			{ g_rtRopes, true, 10, kRtRefused, 0, 0, 0, 0 } };
		HANDLE threads[2] = {};
		DWORD started = 0;
		for (size_t i = 0; i < 2; i++)
			if (const HANDLE thread = CreateThread(nullptr, 0, RtRunChecks, &runs[i], 0, nullptr))
				threads[started++] = thread;
			else
				RtRunChecks(&runs[i]);
		if (started)
			WaitForMultipleObjects(started, threads, TRUE, INFINITE);
		for (DWORD i = 0; i < started; i++)
			CloseHandle(threads[i]);
		RtRunChecks(&runs[2]);
		const int refused = runs[0].refused + runs[1].refused + runs[2].refused;
		swprintf(g_rtChecks, sizeof(g_rtChecks) / sizeof(g_rtChecks[0]), L", checks took %d, refused %d (%llu vertices, %d ropes), registers kept %d of %d",
			took.took + runs[0].took + runs[1].took + runs[2].took, refused, static_cast<unsigned long long>(runs[0].vertices + runs[1].vertices + runs[2].vertices),
			runs[2].refused, runs[0].kept + runs[1].kept + runs[2].kept, refused);
	}
	if (HasOption(argc, argv, L"rtgrow"))
	{
		const size_t size = sizeof(g_rtChecks) / sizeof(g_rtChecks[0]);
		if (!RtWaitFor("DIAGNOSTIC: ray tracing buffer of moving geometry, ", 25000))
		{
			swprintf(g_rtChecks, size, L", nothing grown: no first summary");
			return;
		}
		counters[1] = kRtGrown;
		RtWaitFor("at most 3987120 so far", 10000);
		counters[0] = kRtFlipped;	// the frame flips to slot 1: slot 0's frame is the finished one
		counters[1] = 50;
		*g_rtSlot = 1;
		RtWaitFor("at most 4100000 so far", 10000);
		swprintf(g_rtChecks, size, L", grown and flipped after the first summary");
	}
}

// "rt buffer: made with 4194304 vertices (stride word 0x7014), 1 maker call, flags 0x89, view 1, r9 0 | rt sites: as placed, count
// 4194304, checks 4194304 and 4194304, jumps to the game and the game | rt rvas: set-up 0x..., ... | " - the buffer as made, the sites
// but the five fields TweaksV may write as placed, those fields, and where the sites and the data lie, for the suite to compare with
// the log's lines.
static void RtReport()
{
	const RtBufferObject& buffer = *g_rtBuffer;
	if (g_rtMakerCalls)
		wprintf(L"rt buffer: made with %u vertices (stride word 0x%04X), %ld maker call%ls, flags 0x%X, view %u, r9 %u%ls | ", buffer.count,
			static_cast<unsigned>(buffer.stride),
			g_rtMakerCalls, g_rtMakerCalls == 1 ? L"" : L"s", g_rtMakerFlags, g_rtMakerView, g_rtMakerZero, g_rtChecks);
	else
		wprintf(L"rt buffer: not made%ls | ", g_rtChecks);
	const size_t mayWrite[kRtSites][2] = { { 13, 0 }, { 86, 92 }, { 83, 89 } };	// the fields TweaksV may write, 4 bytes each
	bool placed = true;
	for (size_t i = 0; i < kRtSites; i++)
		for (size_t at = 0; at < g_rtSites[i].size && at < sizeof(g_rtPlaced[i]); at++)
		{
			const bool field = (mayWrite[i][0] && at - mayWrite[i][0] < 4) || (mayWrite[i][1] && at - mayWrite[i][1] < 4);
			if (!field && g_rtSites[i].bytes[at] != g_rtPlaced[i][at])
				placed = false;
		}
	const auto rva = [](const volatile void* p) {
		return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(const_cast<const void*>(p)) - reinterpret_cast<uintptr_t>(ImageBase()));
	};
	wprintf(L"rt sites: %ls, count %u, checks %u and %u, jumps to %ls and %ls | rt rvas: set-up 0x%llX, maker 0x%llX, object 0x%llX, geometry 0x%llX, "
		L"ropes 0x%llX, slot 0x%llX, counters 0x%llX, reset 0x%llX, hole 0x%llX, edge 0x%llX | ", placed ? L"as placed" : L"CHANGED", Imm32(g_rtCountSite),
		Imm32(g_rtCapacities[0]), Imm32(g_rtCapacities[1]), RtJumpWhere(0), RtJumpWhere(1), rva(g_rtSetUp), rva(reinterpret_cast<const void*>(&RtBufferMaker)),
		rva(g_rtObject), rva(g_rtGeometry), rva(g_rtRopes), rva(g_rtSlot), rva(g_rtCounters), rva(g_rtReset), rva(g_rtHole), rva(g_rtEdge));
}

// TEST_DUMP_DATA's window: larger than the largest window, 64 KB, and on whole pages, since dwnoaccess takes one away.
constexpr size_t kDwBytes = 0x12000;
constexpr size_t kDwPage = 0x1000;			// where the page dwnoaccess takes away starts, and its size
__declspec(align(4096)) static unsigned char g_dwWindow[kDwBytes];
static bool g_dwOn = false, g_dwNoAccess = false, g_dwReport = false;

// Fills the window: every byte differs from the one another salt gives it. file gets what the window then holds,
// zeros in the page that cannot be read: what TweaksV's file must hold.
static void DwFill(unsigned salt, const wchar_t* file)
{
	static unsigned char held[kDwBytes];
	for (size_t i = 0; i < kDwBytes; i++)
	{
		const bool readable = !g_dwNoAccess || i < kDwPage || i >= 2 * kDwPage;
		if (readable)
			g_dwWindow[i] = static_cast<unsigned char>(i * 31 + (i >> 8) + salt);
		held[i] = readable ? g_dwWindow[i] : 0;
	}
	const HANDLE out = CreateFileW(file, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (out == INVALID_HANDLE_VALUE)
		return;
	DWORD written = 0;
	WriteFile(out, held, static_cast<DWORD>(kDwBytes), &written, nullptr);
	CloseHandle(out);
}

// Changes the window as soon as TweaksV has patched the heap site: in its start-up, after the first copies of its windows,
// which come before any part changes the game, and before its own thread can run.
static HANDLE g_dwWatching = nullptr;
static DWORD WINAPI DwWatchHeapSite(void*)
{
	const unsigned char before = g_heapSite[4];		// the top byte of the size
	SetEvent(g_dwWatching);
	for (int i = 0; i < 5000 && g_heapSite[4] == before; i++)
		Sleep(1);
	DwFill(0x55, L"dw_later.bin");
	return 0;
}

static IMAGE_NT_HEADERS* DwHeaders()
{
	const auto base = const_cast<unsigned char*>(ImageBase());
	return reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
}

// The section this program's table in memory names .data
static IMAGE_SECTION_HEADER* DwDataSection()
{
	IMAGE_NT_HEADERS* const nt = DwHeaders();
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
	for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
		if (memcmp(section->Name, ".data\0\0\0", 8) == 0)
			return section;
	return nullptr;
}

static void DwSetUp(int argc, wchar_t** argv)
{
	g_dwOn = HasOption(argc, argv, L"dwwindow");
	g_dwNoAccess = HasOption(argc, argv, L"dwnoaccess");
	g_dwReport = g_dwOn || HasOption(argc, argv, L"dwrvas");
	if (g_dwOn)
	{
		DWORD old = 0;
		if (g_dwNoAccess)
			VirtualProtect(g_dwWindow + kDwPage, kDwPage, PAGE_NOACCESS, &old);
		DwFill(0, L"dw_start.bin");
		// A new thread waits for the loader's lock, which TweaksV's start-up holds: it must be running before that
		g_dwWatching = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		if (const HANDLE thread = CreateThread(nullptr, 0, DwWatchHeapSite, nullptr, 0, nullptr))
		{
			WaitForSingleObject(g_dwWatching, 30000);
			CloseHandle(thread);
		}
	}
	IMAGE_SECTION_HEADER* const section = HasOption(argc, argv, L"dwnodata") ? DwDataSection() : nullptr;
	if (section)
	{
		DWORD old = 0;
		VirtualProtect(section, sizeof(*section), PAGE_READWRITE, &old);
		memcpy(section->Name, ".notdata", sizeof(section->Name));
		VirtualProtect(section, sizeof(*section), old, &old);
	}
}

static void DwReport()
{
	const IMAGE_SECTION_HEADER* const section = g_dwReport ? DwDataSection() : nullptr;
	if (!section)
		return;
	const IMAGE_NT_HEADERS* const nt = DwHeaders();
	const auto base = reinterpret_cast<uintptr_t>(ImageBase());
	wprintf(L"dw: window RVA 0x%llX, .data RVA 0x%lX, 0x%lX bytes, image %lu %lu %lu %lu | ",
		static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(g_dwWindow) - base), section->VirtualAddress, section->Misc.VirtualSize,
		static_cast<unsigned long>(base & 0xFFFFFFFFu), static_cast<unsigned long>(base >> 32), nt->OptionalHeader.SizeOfImage, nt->FileHeader.TimeDateStamp);
}
// StaticBounds: a store as its init leaves it, at the game's offsets, its pool within it. The fake's lookup checks its arguments and leaves a last
// error of its own, which the caller of the init checks: the redirect's lines cannot hide behind it.
struct StaticBoundsPool
{
	void* memory;			// +0x00
	void* block;			// +0x08
	void* flags;			// +0x10
	int32_t capacity;		// +0x18
	int32_t stride;			// +0x1C
	uint64_t used;			// +0x20
	uint32_t bits;			// +0x28, its low 30 the slots in use
	uint32_t padding;
};
struct StaticBoundsStore
{
	void* vtable;			// +0x00
	int32_t base;			// +0x08, its streaming base
	int32_t unused0C;
	int32_t count;			// +0x10, before the init the default
	int32_t unused14;
	const char* name;		// +0x18
	unsigned char unused20[0x18];
	StaticBoundsPool pool;	// +0x38: its capacity at +0x50, its bits at +0x60
	unsigned char unused68[0x10];
	int32_t* entries;		// +0x78, the name map's, here only their next links
	uint32_t* buckets;		// +0x80
	uint32_t bucketCount;	// +0x88
	int32_t freeHead;		// +0x8C, -1 when none is free
	uint32_t names;			// +0x90
	uint32_t unused94;
	uint32_t made;			// +0x98
	unsigned char unused9C[0x3C];
	void* boxes;			// +0xD8, the streamer's
	uint16_t boxWords[2];	// +0xE0
	uint32_t unusedE4;
	void* boxFlags;			// +0xE8
	uint16_t flagWords[2];	// +0xF0
	uint32_t unusedF4;
	uint32_t streamerCount;	// +0xF8
	uint32_t unusedFC;
};
static_assert(offsetof(StaticBoundsStore, count) == 0x10 && offsetof(StaticBoundsStore, pool) == 0x38 && offsetof(StaticBoundsStore, pool) + offsetof(StaticBoundsPool, capacity) == 0x50 &&
	offsetof(StaticBoundsStore, pool) + offsetof(StaticBoundsPool, bits) == 0x60 && offsetof(StaticBoundsStore, entries) == 0x78 && offsetof(StaticBoundsStore, bucketCount) == 0x88 &&
	offsetof(StaticBoundsStore, freeHead) == 0x8C && offsetof(StaticBoundsStore, names) == 0x90 && offsetof(StaticBoundsStore, made) == 0x98 && offsetof(StaticBoundsStore, boxes) == 0xD8 &&
	offsetof(StaticBoundsStore, flagWords) == 0xF0 && offsetof(StaticBoundsStore, streamerCount) == 0xF8, "the fake's store at the game's offsets");
constexpr DWORD kSbCallerError = 0x5B4, kSbLookupError = 0x5B5;
struct StaticBoundsEntry { uint32_t hash; int32_t value; int32_t fallback; };
constexpr size_t kSbStores = 4;
static StaticBoundsEntry g_staticBoundsConfig[kSbStores] = { { 0x2CDFE406, 12000, 1 }, { 0x11111111, 500, 1 }, { 0x22222222, -1, 7 }, { 0x33333333, 40000, 1 } };
static StaticBoundsStore g_staticBoundsStores[kSbStores];
static const char g_staticBoundsConfiguration[8] = "config";
static int g_staticBoundsAsked = 0, g_staticBoundsAskedGood = 0, g_staticBoundsErrorKept = 0, g_staticBoundsMade = 0, g_staticBoundsNamesAsked = 0, g_staticBoundsCarried = 0;
static const wchar_t* g_staticBoundsTrap = L"no trap";

// The game's pool size lookup: the value the configuration gives the hash, -1 meaning the default.
static int32_t StaticBoundsLookup(const void* configuration, uint32_t hash, int32_t fallback)
{
	int32_t value = -1;
	bool known = false;
	for (const StaticBoundsEntry& entry : g_staticBoundsConfig)
		if (entry.hash == hash)
		{
			value = entry.value;
			known = fallback == entry.fallback;
		}
	g_staticBoundsAsked++;
	if (configuration == g_staticBoundsConfiguration && known)
		g_staticBoundsAskedGood++;
	SetLastError(kSbLookupError);
	return value == -1 ? fallback : value;
}

// What the init hands the pool: its size, as 0x15A6C0 takes it.
static void StaticBoundsPoolSize(StaticBoundsPool* pool, int32_t size)
{
	pool->capacity = size;
	pool->stride = 16;
}

// A store's init, as 0x342F0 and the streamer's step after it: everything from what its pool was handed.
static void StaticBoundsInit(StaticBoundsStore& store, const StaticBoundsEntry& entry)
{
	SirenIdsRegs regs{};
	regs.rax = entry.hash;
	regs.rbx = reinterpret_cast<uint64_t>(g_staticBoundsConfiguration);
	regs.rdi = static_cast<uint32_t>(entry.fallback);
	regs.rsi = reinterpret_cast<uint64_t>(&store);
	SetLastError(kSbCallerError);
	SirenIdsRunAt(regs, g_staticBoundsInit + kSbInitStart);
	if (GetLastError() == kSbLookupError)
		g_staticBoundsErrorKept++;
	const int32_t size = store.pool.capacity;
	store.freeHead = -1;
	if (size > 0)
	{
		store.bucketCount = static_cast<uint32_t>(size) / 4 + 1;
		store.entries = static_cast<int32_t*>(HeapAlloc(GetProcessHeap(), 0, static_cast<size_t>(size) * sizeof(int32_t)));
		for (int32_t i = 0; i < size; i++)
			store.entries[i] = i + 1 < size ? i + 1 : -1;
		store.freeHead = 0;
	}
	store.base = 4242;
	store.count = size;
	store.made = 1;
	store.streamerCount = static_cast<uint32_t>(size);
	store.boxWords[0] = store.boxWords[1] = store.flagWords[0] = store.flagWords[1] = static_cast<uint16_t>(size);
}

// One name after another, as 0x93AA40 adds them: a slot from the pool, then the name map's entry from its free list, each a trap when it has none.
static void StaticBoundsAddNames(StaticBoundsStore& store, int names)
{
	g_staticBoundsNamesAsked = names;
	for (int i = 0; i < names; i++)
	{
		const int32_t inUse = static_cast<int32_t>(store.pool.bits & 0x3FFFFFFFu);
		if (inUse >= store.pool.capacity)
		{
			g_staticBoundsTrap = L"the trap of a full pool";
			return;
		}
		if (store.freeHead == -1)
		{
			g_staticBoundsTrap = L"the trap of a full name map";
			return;
		}
		store.pool.bits = static_cast<uint32_t>(inUse + 1);
		store.freeHead = store.entries[store.freeHead];
		store.names++;
		g_staticBoundsCarried++;
	}
}

// Every store, in the game's way, and what the options do to the static bounds store. With its call leading to another mod nothing can be called.
static void StaticBoundsMakeStores(int argc, wchar_t** argv)
{
	if (HasOption(argc, argv, L"sbl1hooked"))
		return;
	for (size_t i = HasOption(argc, argv, L"sbnever") ? 1 : 0; i < kSbStores; i++)
		StaticBoundsInit(g_staticBoundsStores[i], g_staticBoundsConfig[i]);
	if (HasOption(argc, argv, L"sbnever"))
		return;
	g_staticBoundsMade++;
	StaticBoundsStore& store = g_staticBoundsStores[0];
	if (HasOption(argc, argv, L"sbmodpool"))
		store.pool.capacity = 30000;
	if (HasOption(argc, argv, L"sbstreamer"))
		store.streamerCount--;
	if (HasOption(argc, argv, L"sbnotmade"))
		store.made = 0;
	for (int i = 1; i + 1 < argc; i++)
		if (wcscmp(argv[i], L"sbnames") == 0)
			StaticBoundsAddNames(store, static_cast<int>(wcstol(argv[i + 1], nullptr, 10)));
}

// Before TweaksV loads: the sites as the options have them - then their bytes as placed - and with "sbearly" every store.
static unsigned char g_staticBoundsPlaced[sizeof(g_staticBoundsInit) + sizeof(g_staticBoundsStoreLea) + sizeof(g_staticBoundsTwin)];
static const unsigned char* StaticBoundsOtherMod() { return ImageEnd() + 0x64000; }
static void StaticBoundsSetUpSites(int argc, wchar_t** argv)
{
	g_staticBoundsConfig[0].value = HasOption(argc, argv, L"sbbig") ? 24500 : HasOption(argc, argv, L"sbhuge") ? 40000 : HasOption(argc, argv, L"sbnegative") ? -2 :
		HasOption(argc, argv, L"sbzero") ? 0 : HasOption(argc, argv, L"sbdefault") ? -1 : 12000;
	const Site lookupCall = { g_staticBoundsInit, kSbLookupRel, 0 }, poolCall = { g_staticBoundsInit, kSbPoolRel, 0 }, storeLea = { g_staticBoundsStoreLea, 3, 0 };
	if (HasOption(argc, argv, L"sbl1hooked"))
		SetRel32(lookupCall, StaticBoundsOtherMod());
	else
		SetRel32(lookupCall, reinterpret_cast<const void*>(&StaticBoundsLookup));
	SetRel32(poolCall, reinterpret_cast<const void*>(&StaticBoundsPoolSize));
	const auto store = reinterpret_cast<const unsigned char*>(&g_staticBoundsStores[0]);
	SetRel32(storeLea, HasOption(argc, argv, L"sbd1out") ? ImageEnd() + 0x65000 : HasOption(argc, argv, L"sbd1odd") ? store + 4 : store);
	if (HasOption(argc, argv, L"sbl1twice"))
		for (size_t i = 0; i < kSbInitMatch; i++)
			g_staticBoundsTwin[i] = g_staticBoundsInit[kSbMatchAt + i];
	if (HasOption(argc, argv, L"sbd1twice"))
		for (size_t i = 0; i < kSbStoreMatch - 1; i++)
			g_staticBoundsTwin[i] = g_staticBoundsStoreLea[i];
	size_t at = 0;
	for (const Site& site : g_staticBoundsSites)
		for (size_t i = 0; i < site.size; i++)
			g_staticBoundsPlaced[at++] = site.bytes[i];
	if (HasOption(argc, argv, L"sbearly"))
		StaticBoundsMakeStores(argc, argv);
}

// Once TweaksV has started, as the game does - with "sblate" 4 s later, with "sbearly" or "sbfirst" not again.
static void StaticBoundsActions(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"sbearly") && !HasOption(argc, argv, L"sbfirst"))
	{
		if (HasOption(argc, argv, L"sblate"))
			Sleep(4000);
		StaticBoundsMakeStores(argc, argv);
	}
	if (HasOption(argc, argv, L"sbwait"))
		Sleep(5000);
}

// "static bounds: call through a stub, the rest as placed, the store made: 16000 slots, pool 16000, name map 16000 entries, 4001 buckets, streamer 16000,
// word 16000, names 0 of 0, no trap, other stores 500 7 40000, lookups 4 of 4 good, last error kept 4 | ": where the call leads; the sites as placed but
// for its rel32; the store, or "the store not made"; the others' sizes; and the calls as the fake's lookup found them.
static void StaticBoundsReport()
{
	const Site lookupCall = { g_staticBoundsInit, kSbLookupRel, 0 };
	const unsigned char* const target = Rel32Target(lookupCall);
	bool rest = true;
	size_t at = 0;
	for (const Site& site : g_staticBoundsSites)
		for (size_t i = 0; i < site.size; i++)
		{
			const bool rel32 = site.bytes == g_staticBoundsInit && i >= kSbLookupRel && i < kSbLookupRel + 4;
			rest = rest && (rel32 || site.bytes[i] == g_staticBoundsPlaced[at]);
			at++;
		}
	wprintf(L"static bounds: call %ls, the rest %ls, ", target == reinterpret_cast<const unsigned char*>(&StaticBoundsLookup) ? L"to the lookup" : target == StaticBoundsOtherMod() ? L"to another mod" :
		InImage(target) ? L"elsewhere in the game" : L"through a stub", rest ? L"as placed" : L"CHANGED");
	const StaticBoundsStore& store = g_staticBoundsStores[0];
	if (g_staticBoundsMade == 0)
		wprintf(L"the store not made, ");
	else
		wprintf(L"the store made: %d slots, pool %d, name map %u buckets, streamer %u, word %u, names %d of %d, %ls, ", store.count, store.pool.capacity, store.bucketCount, store.streamerCount,
			static_cast<unsigned>(store.flagWords[0]), g_staticBoundsCarried, g_staticBoundsNamesAsked, g_staticBoundsTrap);
	wprintf(L"other stores %d %d %d, lookups %d of %d good, last error kept %d | ", g_staticBoundsStores[1].count, g_staticBoundsStores[2].count, g_staticBoundsStores[3].count,
		g_staticBoundsAskedGood, g_staticBoundsAsked, g_staticBoundsErrorKept);
}

// The lighting capacity's data, in the fake's data at the game's distances and reached only through a pointer: the frame's two stores
// with their write pointers right behind them, the sort list with its count and mark, the counters, what the sites name and nothing
// reads, the lists' start values, the two ray traced lists and the two sets of reflection lists, each with its counts first; a page
// that takes no reads with an option lcunread.
struct LcData
{
	unsigned char coronaStore[0xAE00];		// 928 coronas of 0x30 bytes
	unsigned char lightStore[0x2BC00];		// 400 lights of 0x1C0 bytes
	uint64_t coronaPointer;
	uint64_t lightPointer;
	uint64_t sortList[1024];
	uint32_t sortCount;
	uint8_t sortMark;
	uint32_t vehicles, sceneCount, renderCount, bufferCounts[2];
	uint64_t renderPointer, currentPointer, rayPointer, reflectionPointer;
	alignas(16) uint32_t startValues[4];
	alignas(16) unsigned char rayTraced[2][0x72010];
	unsigned char reflections[2][0x29A0C];
	unsigned char toCoronas[8];
	unsigned char gameCoronas[0xCC00];		// 1088 coronas of 0x30 bytes, 16-aligned, the count right behind them, and what the copy reads past it
	uint32_t coronas;
	unsigned char pastCoronas[0x17C];
};
static_assert(offsetof(LcData, gameCoronas) % 16 == 0 && offsetof(LcData, coronas) == offsetof(LcData, gameCoronas) + 0xCC00, "the corona store 16-aligned, its count right behind it");
static_assert(offsetof(LcData, lightStore) == 0xAE00 && offsetof(LcData, coronaPointer) == 0xAE00 + 0x2BC00 && offsetof(LcData, lightPointer) == offsetof(LcData, coronaPointer) + 8 &&
	offsetof(LcData, sortCount) == offsetof(LcData, sortList) + 1024 * 8 && offsetof(LcData, sortMark) == offsetof(LcData, sortCount) + 4 && offsetof(LcData, reflections) % 4 == 0,
	"the lighting capacity's data at the game's distances");
alignas(16) static LcData g_lcDataStorage;
static LcData* volatile g_lcData = &g_lcDataStorage;
alignas(4096) static unsigned char g_lcHoleStorage[4096];
static unsigned char* volatile g_lcHole = g_lcHoleStorage;
static const unsigned char* volatile g_lcWayOut = nullptr;	// where the jump when full leads in the game: its way out
static const unsigned char* volatile g_lcRayWayOut = nullptr;	// the same of the ray traced list's
static int32_t g_lcJumpPlaced = 0;							// its rel32 as placed
static int32_t g_lcRayJumpPlaced = 0, g_lcCoronaCallPlaced = 0, g_lcLightCallPlaced = 0;	// the measurement's other three as placed
static int32_t g_lcLeadsPlaced[kLcLeadCount];				// the part's rel32 as placed
static unsigned char g_lcPlaced[kLcSites][96];				// the sites' bytes as placed

// A stand-in for the two functions the sites call and nothing runs here.
static __declspec(noinline) void LcCalled()
{
	g_lcHole = g_lcHoleStorage;
}

static bool LcOption(int argc, wchar_t** argv, const wchar_t* option, const wchar_t* name)
{
	for (int i = 2; i + 1 < argc; i++)
		if (wcscmp(argv[i], option) == 0 && wcscmp(argv[i + 1], name) == 0)
			return true;
	return false;
}

// A stand-in for the game's copy at a job's end, called through G-E-K1 and G-E-K2: a real one.
static void* __cdecl LcCopy(void* to, const void* from, size_t bytes)
{
	return memcpy(to, from, bytes);
}

// One rel32 of the sites, by the name an option gives it: where it leads as the game has it, past: how far past its own end it counts from.
struct LcRef
{
	const wchar_t* name;
	Site site;
	const void* target;
	ptrdiff_t past;
};

static const unsigned char* LcTarget(int argc, wchar_t** argv, const LcRef& ref)
{
	const unsigned char* const target = static_cast<const unsigned char*>(ref.target);
	if (LcOption(argc, argv, L"lcout", ref.name))
		return ImageEnd() + 0x90000;
	if (LcOption(argc, argv, L"lcend", ref.name))
		return ImageEnd() - 16;
	if (LcOption(argc, argv, L"lcodd", ref.name))
		return target + 1;
	if (LcOption(argc, argv, L"lcunread", ref.name))
		return g_lcHole;
	if (LcOption(argc, argv, L"lcother", ref.name))
		return target + 16;
	return target;
}

// The counters as asked: each store's number by its write pointer, each list's in its two buffers.
struct LcNumbers
{
	uint32_t vehicles, lights, vehicleCoronas, render, scene, ray[2], reflections[2][3], coronas;
	uint8_t mark;
};
static const LcNumbers kLcFirst = { 87, 143, 512, 811, 700, { 640, 200 }, { { 10, 212, 5 }, { 100, 3, 7 } }, 900, 1 };
static void LcSetCounts(const LcNumbers& numbers)
{
	LcData& data = *g_lcData;
	data.vehicles = numbers.vehicles;
	data.renderCount = numbers.render;
	data.sceneCount = numbers.scene;
	for (size_t i = 0; i < 2; i++)
	{
		memcpy(data.rayTraced[i], &numbers.ray[i], sizeof(uint32_t));
		memcpy(data.reflections[i], numbers.reflections[i], sizeof(numbers.reflections[i]));
	}
	data.coronas = numbers.coronas;
	data.sortMark = numbers.mark;
}
// The write pointers, by the stores the frame's reset names now: the game's, or those the part moved them to.
static void LcSetPointers(const LcNumbers& numbers)
{
	LcData& data = *g_lcData;
	data.lightPointer = reinterpret_cast<uintptr_t>(Rel32Target(kLcLeads[1])) + uint64_t{ numbers.lights } * 0x1C0;
	data.coronaPointer = reinterpret_cast<uintptr_t>(Rel32Target(kLcLeads[0])) + uint64_t{ numbers.vehicleCoronas } * 0x30;
}
static void LcSet(const LcNumbers& numbers)
{
	LcSetCounts(numbers);
	LcSetPointers(numbers);
}

// Before TweaksV loads: every rel32 as the game has it, but where an option asks otherwise, the first numbers but the write pointers, which say the frames run; the sites'
// bytes as placed.
static void LcSetUp(int argc, wchar_t** argv)
{
	LcData& data = *g_lcData;
	const void* const called = reinterpret_cast<const void*>(&LcCalled);
	const void* const copy = reinterpret_cast<const void*>(&LcCopy);
	g_lcWayOut = const_cast<const unsigned char*>(g_lcAddToList) + 37;
	g_lcRayWayOut = const_cast<const unsigned char*>(g_lcRayAdd) + 41;
	const uint32_t start[4] = { 0, 0, 200, HasOption(argc, argv, L"lcstartvalues") ? 81u : 80u };
	memcpy(data.startValues, start, sizeof(start));
	const LcRef refs[] = {
		{ L"a1coronas", SITE(g_lcReset, 3), data.coronaStore, 0 },
		{ L"a1coronap", SITE(g_lcReset, 10), &data.coronaPointer, 0 },
		{ L"a1coronalea", SITE(g_lcReset, 17), &data.coronaPointer, 0 },
		{ L"a1lights", SITE(g_lcReset, 24), data.lightStore, 0 },
		{ L"a1lightp", SITE(g_lcReset, 31), &data.lightPointer, 0 },
		{ L"a1lightlea", SITE(g_lcReset, 38), &data.lightPointer, 0 },
		{ L"a2coronas", SITE(g_lcDrainCoronas, 3), data.coronaStore, 0 },
		{ L"a2coronap", SITE(g_lcDrainCoronas, 10), &data.coronaPointer, 0 },
		{ L"a3lights", SITE(g_lcDrainLights, 3), data.lightStore, 0 },
		{ L"a3lightp", SITE(g_lcDrainLights, 10), &data.lightPointer, 0 },
		{ L"a3call", SITE(g_lcDrainLights, 33), called, 0 },
		{ L"b1count", SITE(g_lcJobTest, 2), &data.vehicles, 4 },
		{ L"c1jump", g_lcJumpRel, const_cast<const unsigned char*>(g_lcWayOut), 0 },
		{ L"c4values", SITE(g_lcStartValuesLoad, 8), data.startValues, 0 },
		{ L"d1count", SITE(g_lcSceneAdd, 3), &data.sceneCount, 0 },
		{ L"d1sortcount", SITE(g_lcSceneAdd, 22), &data.sortCount, 0 },
		{ L"d1sortlist", SITE(g_lcSceneAdd, 29), data.sortList, 0 },
		{ L"d1sortstore", SITE(g_lcSceneAdd, 38), &data.sortCount, 0 },
		{ L"e1store", SITE(g_lcRender, 10), data.rayTraced, 0 },		// the scene store's two buffers, which nothing reads, in the room of the ray traced lists
		{ L"e1renderp", SITE(g_lcRender, 20), &data.renderPointer, 0 },
		{ L"e1counts", SITE(g_lcRender, 27), data.bufferCounts, 0 },
		{ L"e1rendercount", SITE(g_lcRender, 38), &data.renderCount, 0 },
		{ L"e1reflections", SITE(g_lcRender, 52), data.reflections, 0 },
		{ L"e1reflp", SITE(g_lcRender, 62), &data.reflectionPointer, 0 },
		{ L"e2store", SITE(g_lcUpdate, 10), data.rayTraced, 0 },
		{ L"e2current", SITE(g_lcUpdate, 20), &data.currentPointer, 0 },
		{ L"e3lists", SITE(g_lcRayLists, 10), data.rayTraced, 0 },
		{ L"e3pointer", SITE(g_lcRayLists, 21), &data.rayPointer, 0 },
		{ L"f1pointer", SITE(g_lcRayAdd, 16), &data.rayPointer, 0 },
		{ L"f1jump", kLcRayJumpRel, const_cast<const unsigned char*>(g_lcRayWayOut), 0 },
		{ L"k1call", kLcCoronaCallRel, copy, 0 },
		{ L"k2call", kLcLightCallRel, copy, 0 },
		{ L"f2call", SITE(g_lcUpload, 21), called, 0 },
		{ L"g1first", SITE(g_lcCoronaCheck, 2), &data.coronas, 0 },
		{ L"g1second", SITE(g_lcCoronaCheckToo, 2), &data.coronas, 0 },
		{ L"ctorcoronas", SITE(g_lcConstructor, 3), data.coronaStore, 0 },
		{ L"ctorlights", SITE(g_lcConstructor, 10), data.lightStore, 0 },
		{ L"extracoronas", SITE(g_lcExtra, 3), data.coronaStore, 0 },
		{ L"extralights", SITE(g_lcExtra, 10), data.lightStore, 0 },
	};
	for (const LcRef& ref : refs)
		SetRel32(ref.site, LcTarget(argc, argv, ref) - ref.past);
	// a rel32 on the page rtjumpunread takes away: its match copied to end the first page of g_rtEdge, every rel32 of the copy set, the site's first byte a NOP
	const struct { const wchar_t* name; volatile unsigned char* site; size_t start; size_t match; } edges[] = {
		{ L"a3call", g_lcDrainLights, 0, 33 }, { L"e1reflp", g_lcRender, 0, 62 }, { L"f2call", g_lcUpload, 0, 21 }, { L"f1jump", g_lcRayAdd, 13, 20 }, { L"k1call", g_lcCoronaEnd, 0, 11 }, { L"k2call", g_lcLightEnd, 0, 16 } };
	for (const auto& edge : edges)
		if (LcOption(argc, argv, L"lcedge", edge.name))
		{
			const size_t at = 0x1000 - edge.match;
			for (size_t i = 0; i < edge.match; i++)
				g_rtEdge[at + i] = edge.site[edge.start + i];
			for (const LcRef& ref : refs)
				if (ref.site.bytes == edge.site && static_cast<size_t>(ref.site.offset) >= edge.start && static_cast<size_t>(ref.site.offset) < edge.start + edge.match)
					SetRel32({ g_rtEdge, static_cast<int>(at + ref.site.offset - edge.start), sizeof(g_rtEdge) }, LcTarget(argc, argv, ref));
			edge.site[edge.start] = 0x90;
		}
	if (HasOption(argc, argv, L"lcfourthcoronas"))
		g_lcExtra[0] = 0x48;
	if (HasOption(argc, argv, L"lcfourthlights"))
		g_lcExtra[7] = 0x48;
	if (HasOption(argc, argv, L"lcnoctor"))
		g_lcConstructor[0] = g_lcConstructor[7] = 0xCC;
	if (HasOption(argc, argv, L"lcmodded"))
		g_lcScratchLights[3] = 0x90;
	SetRel32(SITE(g_lcStartValuesWayIn, 5), const_cast<const unsigned char*>(g_lcStartValuesLoad));
	SetRel32(SITE(g_lcSceneWayIn, 2), const_cast<const unsigned char*>(g_lcSceneAdd));
	SetRel32(SITE(g_lcByteWayIn, 8), const_cast<const unsigned char*>(g_lcByteRead));
	SetRel32(SITE(g_lcCoronaEndWayIn, 16), const_cast<const unsigned char*>(g_lcCoronaEnd));
	SetRel32(SITE(g_lcLightEndWayIn, 12), const_cast<const unsigned char*>(g_lcLightEnd));
	if (HasOption(argc, argv, L"lcnumbers") || HasOption(argc, argv, L"lcsteps"))
		LcSetCounts(kLcFirst);
	if (HasOption(argc, argv, L"lcrunning"))
		LcSetPointers(kLcFirst);
	if (HasOption(argc, argv, L"lcrunninglights"))
		data.lightPointer = reinterpret_cast<uintptr_t>(Rel32Target(kLcLeads[1])) + uint64_t{ kLcFirst.lights } * 0x1C0;
	if (HasOption(argc, argv, L"lcg1once"))
		g_lcCoronaCheckToo[0] = 0xCC;
	if (HasOption(argc, argv, L"lcg1thrice"))
	{
		for (size_t i = 0; i < sizeof(g_lcCoronaCheck); i++)
			g_lcTwin[i] = g_lcCoronaCheck[i];
		SetRel32(SITE(g_lcTwin, 2), &data.coronas);
	}
	if (HasOption(argc, argv, L"lcunread"))
	{
		DWORD old = 0;
		VirtualProtect(g_lcHole, sizeof(g_lcHoleStorage), PAGE_NOACCESS, &old);
	}
	for (size_t i = 0; i < kLcSites; i++)
		for (size_t at = 0; at < g_lcSites[i].size && at < sizeof(g_lcPlaced[i]); at++)
			g_lcPlaced[i][at] = g_lcSites[i].bytes[at];
	g_lcJumpPlaced = static_cast<int32_t>(Imm32(g_lcJumpRel));
	g_lcRayJumpPlaced = static_cast<int32_t>(Imm32(kLcRayJumpRel));
	g_lcCoronaCallPlaced = static_cast<int32_t>(Imm32(kLcCoronaCallRel));
	g_lcLightCallPlaced = static_cast<int32_t>(Imm32(kLcLightCallRel));
	for (size_t i = 0; i < kLcLeadCount; i++)
		g_lcLeadsPlaced[i] = static_cast<int32_t>(Imm32(kLcLeads[i]));
}

// True if every one of the part's rel32 leads elsewhere than as placed.
static bool LcLed()
{
	for (size_t i = 0; i < kLcLeadCount; i++)
		if (static_cast<int32_t>(Imm32(kLcLeads[i])) == g_lcLeadsPlaced[i])
			return false;
	return true;
}

// The lists' start values as a job loads them: G-E-C4 run as code, whose movaps would fault on 16 bytes not 16-aligned.
static void LcStartValues(uint32_t (&values)[4])
{
	alignas(16) uint32_t loaded[4] = {};
	reinterpret_cast<void (*)(uint32_t*)>(const_cast<unsigned char*>(g_lcStartValuesWayIn))(loaded);
	memcpy(values, loaded, sizeof(loaded));
}

// A job's list as add-to-a-job's-list reads it, and one thread's calls of it on a list of its own: as many fit as its capacity, the rest take the jump when full.
struct LcList
{
	void* coronas;
	void* lights;
	uint32_t coronaCount, lightCount, coronaCapacity, lightCapacity;
};
static_assert(offsetof(LcList, lights) == 0x08 && offsetof(LcList, lightCount) == 0x14 && offsetof(LcList, lightCapacity) == 0x1C, "a job's list at the game's offsets");
constexpr int kLcJobCalls = 100000;
static DWORD WINAPI LcRunJob(LPVOID parameter)
{
	LcList& list = *static_cast<LcList*>(parameter);
	const auto add = reinterpret_cast<void (*)(LcList*, const void*)>(const_cast<unsigned char*>(g_lcAddToList));
	for (int i = 0; i < kLcJobCalls; i++)
		add(&list, &list);
	return 0;
}

// The stub the jump when full leads to, if it is what TweaksV makes: "lock inc qword [rip+disp32]", "jmp qword [rip+0]" and the address it goes on to.
struct LcStub
{
	bool found;
	bool readExecute;
	bool onToWayOut;
	const uint64_t* counter;
};
static LcStub LcFindStub(const Site& rel, const unsigned char* wayOut)
{
	LcStub stub{};
	const unsigned char* const target = Rel32Target(rel);
	MEMORY_BASIC_INFORMATION info{};
	if (InImage(target) || VirtualQuery(target, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT || memcmp(target, "\xF0\x48\xFF\x05", 4) != 0 ||
		memcmp(target + 8, "\xFF\x25\x00\x00\x00\x00", 6) != 0)
		return stub;
	stub.found = true;
	stub.readExecute = info.Protect == PAGE_EXECUTE_READ;
	const unsigned char* onTo = nullptr;
	memcpy(&onTo, target + 14, sizeof(onTo));
	stub.onToWayOut = onTo == wayOut;
	int32_t disp = 0;
	memcpy(&disp, target + 4, sizeof(disp));
	stub.counter = reinterpret_cast<const uint64_t*>(target + 8 + disp);
	return stub;
}

// The stub a job's copy leads to, if it is what TweaksV makes: "mov rax, [rip+disp32]" of the mark, "jmp qword [rip+0]" at +23 and the address it goes on to.
static LcStub LcFindMostStub(const Site& rel)
{
	LcStub stub{};
	const unsigned char* const target = Rel32Target(rel);
	MEMORY_BASIC_INFORMATION info{};
	if (InImage(target) || VirtualQuery(target, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT || memcmp(target, "\x48\x8B\x05", 3) != 0 ||
		memcmp(target + 23, "\xFF\x25\x00\x00\x00\x00", 6) != 0)
		return stub;
	stub.found = true;
	stub.readExecute = info.Protect == PAGE_EXECUTE_READ;
	const unsigned char* onTo = nullptr;
	memcpy(&onTo, target + 29, sizeof(onTo));
	stub.onToWayOut = onTo == reinterpret_cast<const unsigned char*>(&LcCopy);
	int32_t disp = 0;
	memcpy(&disp, target + 3, sizeof(disp));
	stub.counter = reinterpret_cast<const uint64_t*>(target + 7 + disp);
	return stub;
}

// A frame past the game's own numbers, on what the sites name now: the reset, 60 jobs with their lists full, each marking its records and ending as the game's do, and the drain's
// walk from the store it names to the write pointer. The sites must name moved stores: on the game's these jobs would run over the write pointers, as the game's would.
static wchar_t g_lcFrame[120] = L"not run";
constexpr uint32_t kLcCoronaBytes = 0x30, kLcLightBytes = 0x1C0;
static uint32_t LcMark(uint32_t job, uint32_t record)
{
	return job << 16 | record;
}
// What a job holds, in its scratch: each record marked first and last.
static void LcMarkRecords(unsigned char* records, uint32_t job, uint32_t count, uint32_t bytes)
{
	for (uint32_t record = 0; record < count; record++)
	{
		const uint32_t mark = LcMark(job, record);
		memcpy(records + size_t{ record } * bytes, &mark, sizeof(mark));
		memcpy(records + size_t{ record + 1 } * bytes - sizeof(mark), &mark, sizeof(mark));
	}
}
// A job's end, as the game's, G-E-K1 and G-E-K2 run as code: each write pointer moved by what the job holds, then the copy; nothing of a list that holds nothing.
static void LcEndJob(uint64_t& coronaPointer, const void* coronas, uint64_t coronaCount, uint64_t& lightPointer, const void* lights, uint64_t lightCount)
{
	using End = void (*)(uint64_t*, const void*, uint64_t);
	if (coronaCount != 0)
		reinterpret_cast<End>(const_cast<unsigned char*>(g_lcCoronaEndWayIn))(&coronaPointer, coronas, coronaCount * kLcCoronaBytes);
	if (lightCount != 0)
		reinterpret_cast<End>(const_cast<unsigned char*>(g_lcLightEndWayIn))(&lightPointer, lights, lightCount);
}
static void LcWalk(const unsigned char* store, uint64_t end, uint32_t perJob, uint32_t bytes, uint32_t& count, uint32_t& wrong)
{
	count = 0;
	wrong = 0;
	for (const unsigned char* at = store; reinterpret_cast<uintptr_t>(at) < end; at += bytes, count++)
	{
		uint32_t first = 0, last = 0;
		memcpy(&first, at, sizeof(first));
		memcpy(&last, at + bytes - sizeof(last), sizeof(last));
		const uint32_t mark = LcMark(count / perJob, count % perJob);
		if (first != mark || last != mark)
			wrong++;
	}
}
static void LcRunFrame()
{
	LcData& data = *g_lcData;
	if (!LcLed())
		return;
	uint32_t values[4] = {};
	LcStartValues(values);
	const uint32_t coronas = values[2], lights = values[3];
	static unsigned char scratch[0x18008];	// a job's, as much as a worker's frame gives it
	if (size_t{ coronas } * kLcCoronaBytes + size_t{ lights } * kLcLightBytes > sizeof(scratch))
	{
		swprintf(g_lcFrame, sizeof(g_lcFrame) / sizeof(g_lcFrame[0]), L"lists past the scratch: %u coronas and %u lights", coronas, lights);
		return;
	}
	reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_lcReset))();
	const uint64_t coronaStart = data.coronaPointer, lightStart = data.lightPointer;
	for (uint32_t job = 0; job < 60; job++)
	{
		unsigned char* const jobLights = scratch + size_t{ coronas } * kLcCoronaBytes;
		LcMarkRecords(scratch, job, coronas, kLcCoronaBytes);
		LcMarkRecords(jobLights, job, lights, kLcLightBytes);
		LcEndJob(data.coronaPointer, scratch, coronas, data.lightPointer, jobLights, lights);
	}
	const unsigned char* const coronaStore = Rel32Target(kLcLeads[2]);
	const unsigned char* const lightStore = Rel32Target(kLcLeads[3]);
	uint32_t coronasDrained = 0, coronasWrong = 0, lightsDrained = 0, lightsWrong = 0;
	LcWalk(coronaStore, data.coronaPointer, coronas, kLcCoronaBytes, coronasDrained, coronasWrong);
	LcWalk(lightStore, data.lightPointer, lights, kLcLightBytes, lightsDrained, lightsWrong);
	swprintf(g_lcFrame, sizeof(g_lcFrame) / sizeof(g_lcFrame[0]), L"60 jobs of %u coronas and %u lights, the reset and the drain %ls, drained %u and %u, %u and %u wrong", coronas, lights,
		coronaStart == reinterpret_cast<uintptr_t>(coronaStore) && lightStart == reinterpret_cast<uintptr_t>(lightStore) ? L"one" : L"APART", coronasDrained, lightsDrained, coronasWrong, lightsWrong);
}

// An index past a byte through the scene store's two sites, run as code: the sort list's first entry as the store's add leaves it, then read back.
static wchar_t g_lcIndex[32] = L"not run";
static void LcRunIndex()
{
	LcData& data = *g_lcData;
	const uint32_t scene = data.sceneCount;
	data.sceneCount = 700;
	data.sortCount = 0;
	reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_lcSceneWayIn))();
	const uint32_t back = reinterpret_cast<uint32_t (*)(uint64_t*, uint64_t)>(const_cast<unsigned char*>(g_lcByteWayIn))(data.sortList, 0);
	swprintf(g_lcIndex, sizeof(g_lcIndex) / sizeof(g_lcIndex[0]), L"700 read back as %u", back);
	data.sortCount = 0;
	data.sceneCount = scene;
}

// The ray traced list's add as code on its first list, if its jump leads to its way out or a stub: the calls so far, for the report.
static int g_lcRayCalls = 0;
static void LcRunRay(int calls)
{
	LcData& data = *g_lcData;
	if (Rel32Target(kLcRayJumpRel) != g_lcRayWayOut && !LcFindStub(kLcRayJumpRel, g_lcRayWayOut).found)
		return;
	data.rayPointer = reinterpret_cast<uintptr_t>(data.rayTraced[0]);
	for (int i = 0; i < calls; i++)
		reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_lcRayAdd))();
	g_lcRayCalls += calls;
}

// One thread's job ends, the counts walking up to its most again and again, on buffers and write pointers of its own: what wrong counts is a write pointer not moved by what the job
// held, or a first or last byte not where the copy must have put it, so a stub that touched an argument shows.
constexpr size_t kLcEndCoronaBytes = 0x3840, kLcEndLightBytes = 0x13B00;
struct LcEnds
{
	uint32_t coronas, lights;
	int calls, wrong;
	unsigned char* from;
	unsigned char* to;
};
static DWORD WINAPI LcRunEnds(LPVOID parameter)
{
	LcEnds& ends = *static_cast<LcEnds*>(parameter);
	for (int i = 0; i < ends.calls; i++)
	{
		const uint32_t coronas = i % (ends.coronas + 1), lights = i % (ends.lights + 1);
		const size_t coronaBytes = size_t{ coronas } * kLcCoronaBytes, lightBytes = size_t{ lights } * kLcLightBytes;
		const unsigned char mark = static_cast<unsigned char>(i % 251 + 1);
		unsigned char* const fromLights = ends.from + kLcEndCoronaBytes;
		unsigned char* const toLights = ends.to + kLcEndCoronaBytes;
		if (coronas != 0)
			ends.from[0] = ends.from[coronaBytes - 1] = mark;
		if (lights != 0)
			fromLights[0] = fromLights[lightBytes - 1] = mark;
		uint64_t coronaPointer = reinterpret_cast<uintptr_t>(ends.to), lightPointer = reinterpret_cast<uintptr_t>(toLights);
		LcEndJob(coronaPointer, ends.from, coronas, lightPointer, fromLights, lights);
		if (coronaPointer != reinterpret_cast<uintptr_t>(ends.to) + coronaBytes || lightPointer != reinterpret_cast<uintptr_t>(toLights) + lightBytes ||
			(coronas != 0 && (ends.to[0] != mark || ends.to[coronaBytes - 1] != mark)) || (lights != 0 && (toLights[0] != mark || toLights[lightBytes - 1] != mark)))
			ends.wrong++;
	}
	return 0;
}
static int g_lcEnds = 0, g_lcEndsWrong = 0;
static unsigned char g_lcEndsFrom[2][kLcEndCoronaBytes + kLcEndLightBytes], g_lcEndsTo[2][kLcEndCoronaBytes + kLcEndLightBytes];
static void LcRunEndsRound(int threads, uint32_t coronas[2], uint32_t lights[2], int calls)
{
	const bool copyLeads = Rel32Target(kLcCoronaCallRel) == reinterpret_cast<const unsigned char*>(&LcCopy) || LcFindMostStub(kLcCoronaCallRel).found;
	const bool lightLeads = Rel32Target(kLcLightCallRel) == reinterpret_cast<const unsigned char*>(&LcCopy) || LcFindMostStub(kLcLightCallRel).found;
	if (!copyLeads || !lightLeads)
		return;
	LcEnds ends[2] = { { coronas[0], lights[0], calls, 0, g_lcEndsFrom[0], g_lcEndsTo[0] }, { coronas[1], lights[1], calls, 0, g_lcEndsFrom[1], g_lcEndsTo[1] } };
	const HANDLE thread = threads > 1 ? CreateThread(nullptr, 0, LcRunEnds, &ends[1], 0, nullptr) : nullptr;
	if (threads > 1 && !thread)
		LcRunEnds(&ends[1]);
	LcRunEnds(&ends[0]);
	if (thread)
	{
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}
	g_lcEnds += threads * calls;
	g_lcEndsWrong += ends[0].wrong + (threads > 1 ? ends[1].wrong : 0);
}

// Once TweaksV has started, as the game's frames would: the counters and the lists as asked.
static wchar_t g_lcJobs[96] = L"not run";
static wchar_t g_lcSteps[96] = L"";
static void LcActions(int argc, wchar_t** argv)
{
	const size_t jobsSize = sizeof(g_lcJobs) / sizeof(g_lcJobs[0]), stepsSize = sizeof(g_lcSteps) / sizeof(g_lcSteps[0]);
	const LcNumbers later = { 20, 200, 100, 811, 1024, { 200, 700 }, { { 1, 2, 3 }, { 4, 5, 300 } }, 950, 1 };
	const bool led = LcLed();
	const uint32_t lightRoom = led ? 10800 : 400, coronaRoom = led ? 18000 : 928;
	const LcNumbers full = { 300, lightRoom, coronaRoom, 1024, 1024, { 1024, 1024 }, { { 768, 768, 768 }, { 768, 768, 768 } }, 1088, 0 };
	const LcNumbers past = { 301, lightRoom + 1, coronaRoom + 1, 1025, 1, { 1025, 0 }, { { 769, 0, 0 }, { 0, 0, 0 } }, 1089, 0 };
	if (HasOption(argc, argv, L"lcindex"))
		LcRunIndex();
	if (HasOption(argc, argv, L"lcframe"))
		LcRunFrame();
	if (HasOption(argc, argv, L"lcnumbers") || HasOption(argc, argv, L"lcsteps"))
		LcSetPointers(kLcFirst);
	if (HasOption(argc, argv, L"lcfull"))
		LcSet(full);
	if (HasOption(argc, argv, L"lcpast"))
	{
		LcSet(past);
		const char* const lines[] = { "vehicle lights' write pointer", "vehicle coronas' write pointer", "vehicles seen at 301", "scene lights seen at 1025", "ray traced list seen at 1025",
			"reflection lists seen at 769", "coronas seen at 1089" };
		for (const char* const line : lines)
			RtWaitFor(line, 10000);
	}
	if (HasOption(argc, argv, L"lcbelow"))
	{
		LcData& data = *g_lcData;
		data.lightPointer = reinterpret_cast<uintptr_t>(Rel32Target(kLcLeads[1])) - 0x1C0;
		data.coronaPointer = reinterpret_cast<uintptr_t>(Rel32Target(kLcLeads[0])) - 0x30;
		RtWaitFor("vehicle lights' write pointer", 10000);
		RtWaitFor("vehicle coronas' write pointer", 10000);
	}
	if (HasOption(argc, argv, L"lcray"))
		LcRunRay(2000);
	if (HasOption(argc, argv, L"lcends"))
	{
		uint32_t coronas[2] = { 199, 150 }, lights[2] = { 79, 60 };
		LcRunEndsRound(2, coronas, lights, 20000);
	}
	if (HasOption(argc, argv, L"lcrounds"))
	{
		if (!RtWaitFor("a job's most 79 of", 25000))
		{
			swprintf(g_lcSteps, stepsSize, L", no first summary");
			return;
		}
		uint32_t coronas[2] = { 100, 0 }, lights[2] = { 40, 0 };
		LcRunRay(10);
		LcRunEndsRound(1, coronas, lights, 1000);
		Sleep(1500);
		swprintf(g_lcSteps, stepsSize, L", the second round run");
	}
	if (HasOption(argc, argv, L"lcjobs") && (Rel32Target(g_lcJumpRel) == g_lcWayOut || LcFindStub(g_lcJumpRel, g_lcWayOut).found))
	{
		uint32_t values[4] = {};
		LcStartValues(values);
		LcList lists[2] = { { nullptr, nullptr, values[0], values[1], values[2], values[3] }, { nullptr, nullptr, values[0], values[1], values[2], values[3] } };
		const HANDLE thread = CreateThread(nullptr, 0, LcRunJob, &lists[0], 0, nullptr);
		if (!thread)
			LcRunJob(&lists[0]);
		LcRunJob(&lists[1]);
		if (thread)
		{
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
		}
		const int added = static_cast<int>(lists[0].lightCount + lists[1].lightCount);
		swprintf(g_lcJobs, jobsSize, L"added %d, refused %d", added, 2 * kLcJobCalls - added);
	}
	if (HasOption(argc, argv, L"lcsteps"))
	{
		if (!RtWaitFor("DIAGNOSTIC: the most so far: vehicles", 25000))
		{
			swprintf(g_lcSteps, stepsSize, L", no first summary");
			return;
		}
		LcSet(later);
		Sleep(3000);
		swprintf(g_lcSteps, stepsSize, L", the numbers changed after the first summary");
	}
	if (HasOption(argc, argv, L"lcsirens"))
	{
		if (!RtWaitFor("siren settings not read yet.", 25000))
		{
			swprintf(g_lcSteps, stepsSize, L", no first summary");
			return;
		}
		// each setting's byte for real lights on every third, and lights with their flags: i % 13, but 20 on the last that has real lights, past all the others
		for (size_t i = 0; i < 255; i++)
		{
			unsigned char* const setting = g_sirenIdsSettings + i * kSirenIdsSettingSize;
			const bool real = i % 3 == 0;
			setting[0x38] = real ? 1 : 0;
			const size_t lights = i == 252 ? 20 : real ? i % 13 : 20;	// those without real lights all 20, which no count may take for lights
			for (size_t light = 0; light < 20; light++)
				setting[0x3C + light * 0x48 + 0x45] = light < lights ? 1 : 0;
		}
		SirenIdsCount(0);
		g_sirenIdsPointer[0] = g_sirenIdsCarcols;
		const bool none = RtWaitFor("siren settings 0, real lights in 0, up to 0 lights.", 5000);
		SirenIdsCount(33);
		const bool some = RtWaitFor("siren settings 33, real lights in 11, up to 12 lights.", 5000);
		SirenIdsCount(255);
		const bool many = RtWaitFor("siren settings 255, real lights in 85, up to 20 lights.", 5000);
		DWORD old = 0;
		VirtualProtect(g_sirenIdsSettings, g_sirenIdsCapacity * kSirenIdsSettingSize, PAGE_NOACCESS, &old);
		SirenIdsCount(40);
		Sleep(1500);
		const int whileUnread = RtLogHas("siren settings 40,");
		VirtualProtect(g_sirenIdsSettings, g_sirenIdsCapacity * kSirenIdsSettingSize, PAGE_READWRITE, &old);
		const bool back = RtWaitFor("siren settings 40, real lights in 14, up to 12 lights.", 5000);
		swprintf(g_lcSteps, stepsSize, L", settings %ls %ls %ls, %d lines while they took no reads, %ls", none ? L"0" : L"no-0", some ? L"33" : L"no-33", many ? L"255" : L"no-255",
			whileUnread, back ? L"40 after" : L"no-40");
	}
}

// True if the byte lies in the size bytes of that site.
static bool LcIn(const volatile unsigned char* byte, const Site& site, size_t size)
{
	return byte >= site.bytes + site.offset && byte < site.bytes + site.offset + size;
}

// "lc sites: as placed, jump as placed | lc stores: as placed | lc frame: not run | lc index: not run | lc jobs: not run | lc rvas: ... | " - the sites' bytes but the jump's rel32 as placed, or
// "raised": every one the part writes as it must and every other as placed; where the jump leads and, if to a stub, how it is and what its counter holds; the stores and the lists' start values the
// sites name, how they are and what a job loads; the frame's and the index's runs; the lists' calls; and where the sites and the data lie, for the suite to compare with the log's lines.
static void LcReport()
{
	bool placed = true, raised = LcLed();
	for (size_t i = 0; i < kLcSites; i++)
		for (size_t at = 0; at < g_lcSites[i].size && at < sizeof(g_lcPlaced[i]); at++)
		{
			const volatile unsigned char* const byte = g_lcSites[i].bytes + at;
			if (LcIn(byte, g_lcJumpRel, 4) || LcIn(byte, kLcRayJumpRel, 4) || LcIn(byte, kLcCoronaCallRel, 4) || LcIn(byte, kLcLightCallRel, 4))
				continue;
			placed = placed && *byte == g_lcPlaced[i][at];
			bool led = false;
			for (const Site& lead : kLcLeads)
				led = led || LcIn(byte, lead, 4);
			if (led)
				continue;
			unsigned char expected = g_lcPlaced[i][at];
			for (const LcRaisedBytes& write : kLcRaisedBytes)
				if (LcIn(byte, write.site, write.size))
					expected = write.bytes[byte - (write.site.bytes + write.site.offset)];
			raised = raised && *byte == expected;
		}
	wprintf(L"lc sites: %ls, ", placed ? L"as placed" : raised ? L"raised" : L"CHANGED");
	const LcStub stub = LcFindStub(g_lcJumpRel, g_lcWayOut);
	if (stub.found)
	{
		MEMORY_BASIC_INFORMATION info{};
		const bool counterRw = VirtualQuery(stub.counter, &info, sizeof(info)) == sizeof(info) && info.Protect == PAGE_READWRITE;
		wprintf(L"jump to a stub, %ls, on to %ls, its counter %ls, holding %llu", stub.readExecute ? L"RX" : L"NOT RX", stub.onToWayOut ? L"its way out" : L"elsewhere",
			counterRw ? L"RW" : L"NOT RW", counterRw ? static_cast<unsigned long long>(*stub.counter) : 0ull);
	}
	else
		wprintf(L"jump %ls", static_cast<int32_t>(Imm32(g_lcJumpRel)) == g_lcJumpPlaced ? L"as placed" : L"CHANGED");
	const auto rva = [](const volatile void* p) {
		return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(const_cast<const void*>(p)) - reinterpret_cast<uintptr_t>(ImageBase()));
	};
	const LcData& data = *g_lcData;
	const auto counterRw = [](const uint64_t* counter) {
		MEMORY_BASIC_INFORMATION info{};
		return VirtualQuery(counter, &info, sizeof(info)) == sizeof(info) && info.Protect == PAGE_READWRITE;
	};
	const LcStub rayStub = LcFindStub(kLcRayJumpRel, g_lcRayWayOut);
	if (rayStub.found)
		wprintf(L" | lc counts: ray jump to a stub, %ls, on to %ls, its counter %ls, holding %llu", rayStub.readExecute ? L"RX" : L"NOT RX", rayStub.onToWayOut ? L"its way out" : L"elsewhere",
			counterRw(rayStub.counter) ? L"RW" : L"NOT RW", counterRw(rayStub.counter) ? static_cast<unsigned long long>(*rayStub.counter) : 0ull);
	else
		wprintf(L" | lc counts: ray jump %ls", static_cast<int32_t>(Imm32(kLcRayJumpRel)) == g_lcRayJumpPlaced ? L"as placed" : L"CHANGED");
	const struct { const wchar_t* name; const Site* rel; int32_t placed; } copies[] = { { L"corona", &kLcCoronaCallRel, g_lcCoronaCallPlaced }, { L"light", &kLcLightCallRel, g_lcLightCallPlaced } };
	for (const auto& copy : copies)
	{
		const LcStub most = LcFindMostStub(*copy.rel);
		if (most.found)
			wprintf(L", %ls copy to a stub, %ls, on to %ls, its most %ls", copy.name, most.readExecute ? L"RX" : L"NOT RX", most.onToWayOut ? L"the copy" : L"elsewhere", counterRw(most.counter) ? L"RW" : L"NOT RW");
		else
			wprintf(L", %ls copy %ls", copy.name, static_cast<int32_t>(Imm32(*copy.rel)) == copy.placed ? L"as placed" : L"CHANGED");
	}
	if (g_lcRayCalls != 0)
	{
		const uint32_t rayAdded = *reinterpret_cast<const uint32_t*>(data.rayTraced[0]);
		wprintf(L" | lc ray: added %u, refused %d", rayAdded, g_lcRayCalls - static_cast<int>(rayAdded));
	}
	else
		wprintf(L" | lc ray: not run");
	if (g_lcEnds != 0)
		wprintf(L" | lc ends: %d ends, %d wrong", g_lcEnds, g_lcEndsWrong);
	else
		wprintf(L" | lc ends: not run");
	if (raised)
	{
		const auto protection = [](const void* at) {
			MEMORY_BASIC_INFORMATION info{};
			const DWORD protect = VirtualQuery(at, &info, sizeof(info)) == sizeof(info) ? info.Protect : 0;
			return protect == PAGE_READWRITE ? L"RW" : protect == PAGE_READONLY ? L"read-only" : L"NEITHER";
		};
		uint32_t values[4] = {};
		LcStartValues(values);
		wprintf(L" | lc stores: moved, coronas %ls, lights %ls, the drain names %ls, the start values %ls, loaded %u %u %u %u", protection(Rel32Target(kLcLeads[0])), protection(Rel32Target(kLcLeads[1])),
			Rel32Target(kLcLeads[0]) == Rel32Target(kLcLeads[2]) && Rel32Target(kLcLeads[1]) == Rel32Target(kLcLeads[3]) ? L"the same" : L"OTHERS", protection(Rel32Target(kLcLeads[4])), values[0], values[1], values[2], values[3]);
	}
	else
		wprintf(L" | lc stores: %ls", placed ? L"as placed" : L"CHANGED");
	wprintf(L" | lc frame: %ls | lc index: %ls", g_lcFrame, g_lcIndex);
	wprintf(L" | lc jobs: %ls%ls | lc rvas: reset 0x%llX, coronas 0x%llX, corona pointer 0x%llX, lights 0x%llX, light pointer 0x%llX, drain coronas 0x%llX, drain lights 0x%llX, called 0x%llX, "
		L"job test 0x%llX, vehicles 0x%llX, kick 0x%llX, last kick 0x%llX, add 0x%llX, scratch 0x%llX, scratch lights 0x%llX, start values 0x%llX, scene add 0x%llX, scene count 0x%llX, sort list 0x%llX, "
		L"sort count 0x%llX, sort mark 0x%llX, byte read 0x%llX, render 0x%llX, store 0x%llX, render pointer 0x%llX, buffer counts 0x%llX, render count 0x%llX, reflections 0x%llX, reflection pointer 0x%llX, "
		L"update 0x%llX, current pointer 0x%llX, ray lists 0x%llX, ray pointer 0x%llX, ray check 0x%llX, upload 0x%llX, reflection check 0x%llX, corona check 0x%llX, corona check too 0x%llX, corona count 0x%llX, "
		L"corona add 0x%llX, worker frame 0x%llX, worker call 0x%llX, constructor 0x%llX, start load 0x%llX, corona end 0x%llX, light end 0x%llX, copy 0x%llX, ray way out 0x%llX | ",
		g_lcJobs, g_lcSteps, rva(g_lcReset), rva(data.coronaStore), rva(&data.coronaPointer), rva(data.lightStore), rva(&data.lightPointer), rva(g_lcDrainCoronas), rva(g_lcDrainLights),
		rva(reinterpret_cast<const void*>(&LcCalled)), rva(g_lcJobTest), rva(&data.vehicles), rva(g_lcKick), rva(g_lcLastKick), rva(g_lcAddToList), rva(g_lcScratch), rva(g_lcScratchLights),
		rva(data.startValues), rva(g_lcSceneAdd), rva(&data.sceneCount), rva(data.sortList), rva(&data.sortCount), rva(&data.sortMark), rva(g_lcByteRead), rva(g_lcRender), rva(data.rayTraced),
		rva(&data.renderPointer), rva(data.bufferCounts), rva(&data.renderCount), rva(data.reflections), rva(&data.reflectionPointer), rva(g_lcUpdate), rva(&data.currentPointer), rva(g_lcRayLists),
		rva(&data.rayPointer), rva(g_lcRayCheck), rva(g_lcUpload), rva(g_lcReflections), rva(g_lcCoronaCheck), rva(g_lcCoronaCheckToo), rva(&data.coronas), rva(g_lcCoronaAdd), rva(g_lcWorkerFrame),
		rva(g_lcWorkerCall), rva(g_lcConstructor), rva(g_lcStartValuesLoad), rva(g_lcCoronaEnd), rva(g_lcLightEnd), rva(reinterpret_cast<const void*>(&LcCopy)), rva(g_lcRayAdd + 41));
}
// The two model budgets: the data the sites lead to, reached through pointers, so that no store of this program's own code counts as a
// writer of a table; the object on whole pages, since popgone takes one away, and the hole a page of its own.
struct PopTables
{
	uint32_t pedStory[4], pedNetwork[4], vehicleStory[4], vehicleNetwork[4];
	uint32_t vehicleLevel, pedLevel;
	float vehicleVariety, pedVariety;
	float factor, added, most;
	int32_t ambient;
	uint32_t pedDisplay, vehicleDisplay;
	uint16_t modelCount;
	uint8_t beforeFlag;		// what the getters' disp32 leads to, since a cmp's imm8 lies behind it
	uint8_t network;
	const void* pool;
	const void* holder;
	const void* tunables;
	const void* modelTable;
	uint64_t modelStride;
};
struct PopModel { unsigned char bytes[0x290]; };
struct PopEntry { const PopModel* model; uint64_t other; };
constexpr size_t kPopObjectBytes = 0x7000, kPopModels = 40, kPopModelCount = 1000, kPopFirstModel = 100, kPopFlipEntry = 17;
constexpr size_t kPopVehicleUsed = 0x6214, kPopPedUsed = 0x620C, kPopPopulationList = 0x4E04;
static_assert(offsetof(PopTables, pedNetwork) == 0x10 && offsetof(PopTables, vehicleNetwork) == 0x30 && alignof(PopTables) == 8 && kPopModelCount > kPopFirstModel + kPopModels, "the budgets' data");
static PopTables g_popTablesStorage;
static PopTables* volatile g_popTables = &g_popTablesStorage;
alignas(4096) static unsigned char g_popObjectStorage[kPopObjectBytes];
static unsigned char* volatile g_popObject = g_popObjectStorage;
alignas(4096) static unsigned char g_popHoleStorage[4096];
static unsigned char* volatile g_popHole = g_popHoleStorage;
static unsigned char g_popManager[0x20];		// the configuration's holder leads here, its +0x18 to the configuration
static unsigned char g_popConfiguration[0x588];
static unsigned char g_popPool[0x40];			// VehicleStruct: +0x18 its size, +0x28 its used
static PopModel g_popModelsStorage[kPopModels];
static PopEntry g_popModelTable[0x10000];		// up to the free value's slot, which no list's end test lets a reader reach
static PopModel g_popJunkModel;
static unsigned char g_popPlaced[kPopSites][0x300];
static uint64_t g_popDataHash = 0;

template <typename T> static void PopField(size_t at, T value)
{
	memcpy(g_popObject + at, &value, sizeof(T));
}

static void PopList(size_t at, uint32_t first, uint32_t count)
{
	const auto list = reinterpret_cast<uint32_t*>(g_popObject + at);
	for (uint32_t i = 0; i < 256; i++)
		list[i] = i < count ? first + i : 0xFFFF;
}

static void PopPoolField(size_t at, uint32_t value)
{
	memcpy(g_popPool + at, &value, sizeof(value));
}

// What the data hold now, for the report: this program's own changes only, TweaksV writing none.
static uint64_t PopHashOfData()
{
	uint64_t hash = 14695981039346656037ull;
	const auto add = [&hash](const void* bytes, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			hash = (hash ^ static_cast<const volatile unsigned char*>(bytes)[i]) * 1099511628211ull;
	};
	add(g_popTables, sizeof(PopTables));
	add(g_popObject, kPopObjectBytes);
	add(g_popManager, sizeof(g_popManager));
	add(g_popConfiguration, sizeof(g_popConfiguration));
	add(g_popPool, sizeof(g_popPool));
	add(g_popModelsStorage, sizeof(g_popModelsStorage));
	add(g_popModelTable, sizeof(g_popModelTable));
	return hash;
}

static void PopLayOutLists()
{
	PopList(0x3600, 300, 94);
	PopList(0x3A00, 0, 0);
	PopList(0x3E00, 0, 0);
	PopList(0x4200, 0, 0);
	PopList(0x4A00, 400, 8);
	PopList(kPopPopulationList, kPopFirstModel, 19);
	PopList(0x5204, 500, 5);
	PopList(0x5604, 510, 4);
	PopList(0x5A04, 0, 0);
	PopList(0x5E04, 520, 3);
}

// The data as my boot read them at 180 s, then what an option asks of them.
static void PopFill(int argc, wchar_t** argv)
{
	PopTables& t = *g_popTables;
	const uint32_t peds[4] = { 1500000, 55500000, 73500000, 93500000 }, vehicles[4] = { 1500000, 33500000, 57500000, 71500000 };
	for (size_t i = 0; i < 4; i++)
	{
		t.pedStory[i] = t.pedNetwork[i] = peds[i];
		t.vehicleStory[i] = t.vehicleNetwork[i] = vehicles[i];
	}
	t.vehicleVariety = t.pedVariety = 6.0f;
	t.ambient = 57;
	PopField<int32_t>(0x6204, 8);
	PopField<int32_t>(0x6208, -1);
	PopField<int32_t>(kPopPedUsed, 305127424);
	PopField<int32_t>(kPopVehicleUsed, 428679168);
	PopField<int32_t>(0x623C, 10100000);
	if (!HasOption(argc, argv, L"popzeros"))
		PopLayOutLists();
	PopPoolField(0x18, 200);
	PopPoolField(0x28, 102 | 0x40000000);	// a flag in the top two bits
	t.pool = g_popPool;
	if (HasOption(argc, argv, L"poplevel1"))
		t.vehicleLevel = t.pedLevel = 1;
	if (HasOption(argc, argv, L"poplevel7"))
		t.vehicleLevel = t.pedLevel = 7;
	if (HasOption(argc, argv, L"popnet"))
	{
		t.network = 1;
		t.vehicleNetwork[3] = 72000000;
		t.pedNetwork[3] = 94000000;
	}
	if (HasOption(argc, argv, L"popbind") || HasOption(argc, argv, L"popoverride"))
		PopField<int32_t>(kPopPedUsed, 555000000);
	if (HasOption(argc, argv, L"popoverride"))
		PopField<int32_t>(0x6208, 4);
	if (HasOption(argc, argv, L"popquiet"))
		PopField<int32_t>(kPopVehicleUsed, 300000000);
	if (HasOption(argc, argv, L"poptunable"))
	{
		t.vehicleStory[3] = 120000000;
		t.pedStory[1] = 60000000;
	}
	if (HasOption(argc, argv, L"popraise"))
	{
		const uint32_t kb = 98500;
		memcpy(g_popConfiguration + 0x514, &kb, sizeof(kb));
		t.vehicleStory[3] = 100000000;
	}
	if (HasOption(argc, argv, L"popvariety"))
		t.vehicleVariety = t.pedVariety = 10.0f;
	if (HasOption(argc, argv, L"popoverflow"))
		t.vehicleStory[3] = 400000000;
	if (HasOption(argc, argv, L"popbadmodel"))
	{
		g_popModelTable[kPopFirstModel + 1].model = nullptr;
		g_popModelTable[kPopFirstModel + 2].model = reinterpret_cast<const PopModel*>(g_popHole);
	}
	if (HasOption(argc, argv, L"poplists"))
	{
		PopField<uint32_t>(0x3600 + 50 * sizeof(uint32_t), kPopModelCount);
		for (uint32_t i = 0; i < 256; i++)
			PopField<uint32_t>(0x4200 + i * sizeof(uint32_t), 600 + i);
	}
	if (HasOption(argc, argv, L"popnopool"))
		t.pool = nullptr;
	if (HasOption(argc, argv, L"poppoolunread"))
		t.pool = g_popHole;
	if (HasOption(argc, argv, L"popnoconfig") || HasOption(argc, argv, L"popconfiglate"))
		t.holder = nullptr;
}

// Before TweaksV loads: every rel32 to what it leads to, but where an option asks otherwise; the data as at the game's start or, with an
// option, already filled; then the sites as placed.
struct PopRef { const wchar_t* name; Site site; const void* target; };
static void PopSetUp(int argc, wchar_t** argv)
{
	static const float kFactor = 5.0f, kAdded = 1.0f, kMost = 10.0f;
	static const char kText[] = "Ped Memory Use/Budget : %d/%d\n";
	PopTables& t = *g_popTables;
	const auto storeOf = [](size_t block) { return Site{ g_popSetUp, static_cast<int>(kPopBlocks + block * kPopBlockBytes + kPopStoreInBlock), sizeof(g_popSetUp) }; };
	const PopRef refs[] = {
		{ L"ambient", SITE(g_popContext, 3), &t.ambient },
		{ L"peddisplay", SITE(g_popContext, 27 + 2), &t.pedDisplay },
		{ L"vehicledisplay", SITE(g_popContext, 27 + 11), &t.vehicleDisplay },
		{ L"object", SITE(g_popContext, 27 + 21), g_popObject },
		{ L"pedgetter", SITE(g_popContext, 27 + 29), const_cast<const unsigned char*>(g_popPedGetter) },
		{ L"vehiclegetter", SITE(g_popContext, 27 + 42), const_cast<const unsigned char*>(g_popVehicleGetter) },
		{ L"text", SITE(g_popContext, 27 + 54), kText },
		{ L"pgetlevel", SITE(g_popPedGetter, 2), &t.pedLevel },
		{ L"pgetflag", SITE(g_popPedGetter, 8), &t.beforeFlag },
		{ L"pgetnetwork", SITE(g_popPedGetter, 18), t.pedNetwork },
		{ L"pgetstory", SITE(g_popPedGetter, 35), t.pedStory },
		{ L"pgetvariety", SITE(g_popPedGetter, 51), &t.pedVariety },
		{ L"vgetlevel", SITE(g_popVehicleGetter, 2), &t.vehicleLevel },
		{ L"vgetflag", SITE(g_popVehicleGetter, 8), &t.beforeFlag },
		{ L"vgetnetwork", SITE(g_popVehicleGetter, 18), t.vehicleNetwork },
		{ L"vgetstory", SITE(g_popVehicleGetter, 35), t.vehicleStory },
		{ L"vgetvariety", SITE(g_popVehicleGetter, 51), &t.vehicleVariety },
		{ L"vtablevel", SITE(g_popVehicleTables, 2), &t.vehicleLevel },
		{ L"vtabflag", SITE(g_popVehicleTables, 9), &t.network },
		{ L"vtabstory", SITE(g_popVehicleTables, 16), t.vehicleStory },
		{ L"vtabnetwork", SITE(g_popVehicleTables, 23), t.vehicleNetwork },
		{ L"vcheckvariety", SITE(g_popVehicleCheck, 16), &t.vehicleVariety },
		{ L"vrequest", SITE(g_popVehicleCheck, 42), const_cast<const unsigned char*>(g_popExtraCall) },
		{ L"pool", SITE(g_popVehiclePool, 3), &t.pool },
		{ L"holder", SITE(g_popSetUp, kPopHead + 3), &t.holder },
		{ L"tunables", SITE(g_popSetUp, kPopHead + 14), &t.tunables },
		{ L"setupcall", SITE(g_popSetUpCall, 8), const_cast<const unsigned char*>(g_popSetUp + kPopStart) },
		{ L"pstore1", storeOf(0), &t.pedStory[1] },
		{ L"pstore2", storeOf(1), &t.pedStory[2] },
		{ L"pstore3", storeOf(2), &t.pedStory[3] },
		{ L"pnetstore1", storeOf(3), &t.pedNetwork[1] },
		{ L"pnetstore2", storeOf(4), &t.pedNetwork[2] },
		{ L"pnetstore3", storeOf(5), &t.pedNetwork[3] },
		{ L"vstore1", storeOf(6), &t.vehicleStory[1] },
		{ L"vstore2", storeOf(7), &t.vehicleStory[2] },
		{ L"vstore3", storeOf(8), &t.vehicleStory[3] },
		{ L"vnetstore1", storeOf(9), &t.vehicleNetwork[1] },
		{ L"vnetstore2", storeOf(10), &t.vehicleNetwork[2] },
		{ L"vnetstore3", storeOf(11), &t.vehicleNetwork[3] },
		{ L"pedsum", SITE(g_popPedSum, 26), &t.pedDisplay },
		{ L"gatelevel", SITE(g_popGate, kPopMatchP2), &t.pedLevel },
		{ L"gaterequest", SITE(g_popGate, kPopGateCall + 1), const_cast<const unsigned char*>(g_popExtraCall) },
		{ L"modelcount", SITE(g_popPedLists, 46), &t.modelCount },
		{ L"modeltable", SITE(g_popPedsOfModel, 3), &t.modelTable },
		{ L"modelstride", SITE(g_popPedsOfModel, 13), &t.modelStride },
		{ L"varfactor", SITE(g_popVariety, 11), &kFactor },
		{ L"varadded", SITE(g_popVariety, 18), &kAdded },
		{ L"varmost", SITE(g_popVariety, 26), &kMost },
		{ L"varstore", SITE(g_popVariety, 76), &t.pedVariety },
	};

	// the gate's bytes further on, every rel32, and the data as at the game's start
	const unsigned char list[] = { 0x41, 0x8B, 0x9C, 0xAC, 0x04, 0x4E, 0x00, 0x00 }, floor[] = { 0x83, 0xFD, 0x06, 0x0F, 0x9C, 0xC0, 0x83, 0xF9, 0x03, 0x0F, 0x94, 0xC1 };
	for (size_t i = 0; i < sizeof(list); i++)
		g_popGate[kPopGateList + i] = list[i];
	for (size_t i = 0; i < sizeof(floor); i++)
		g_popGate[kPopGateFloor + i] = floor[i];
	g_popGate[kPopGateCall] = 0xE8;
	for (const PopRef& ref : refs)
		SetRel32(ref.site, ref.target);
	const uint32_t peds[4] = { 1500000, 28500000, 37500000, 48300000 }, vehicles[4] = { 1500000, 17500000, 29500000, 36500000 };
	for (size_t i = 0; i < 4; i++)
	{
		t.pedStory[i] = t.pedNetwork[i] = peds[i];
		t.vehicleStory[i] = t.vehicleNetwork[i] = vehicles[i];
	}
	t.vehicleLevel = t.pedLevel = 3;
	t.vehicleVariety = t.pedVariety = 1.0f;
	t.modelCount = kPopModelCount;
	t.modelTable = g_popModelTable;
	t.modelStride = sizeof(PopEntry);
	t.holder = g_popManager;
	const void* const configuration = g_popConfiguration;
	memcpy(g_popManager + 0x18, &configuration, sizeof(configuration));
	const uint32_t kb[4][3] = { { 54000, 72000, 92000 }, { 54000, 72000, 92000 }, { 32000, 56000, 70000 }, { 32000, 56000, 70000 } };
	memcpy(g_popConfiguration + 0x4F4, kb, sizeof(kb));
	for (size_t i = 0; i < kPopModels; i++)
	{
		const int32_t many = i < 19 ? 1 + static_cast<int32_t>(i % 9) : 12;
		memcpy(g_popModelsStorage[i].bytes + 0x158, &many, sizeof(many));
		g_popModelsStorage[i].bytes[0x286] = i == 5 || i == 25 || i == 26 ? 0x02 : 0xFD;
		g_popModelTable[kPopFirstModel + i].model = &g_popModelsStorage[i];
		g_popModelTable[kPopFirstModel + i].other = 0xBAD0BAD0BAD0BAD0ull;
	}
	const int32_t junk = 1065353216;
	memcpy(g_popJunkModel.bytes + 0x158, &junk, sizeof(junk));
	g_popModelTable[0xFFFF].model = &g_popJunkModel;

	// what the options ask
	if (HasOption(argc, argv, L"popfill"))
		PopFill(argc, argv);
	for (int i = 2; i + 2 < argc; i++)
		if (wcscmp(argv[i], L"popref") == 0)
			for (const PopRef& ref : refs)
				if (wcscmp(ref.name, argv[i + 1]) == 0)
				{
					const auto target = static_cast<const unsigned char*>(ref.target);
					const size_t behind = wcsstr(ref.name, L"network") ? 16 : 0;
					SetRel32(ref.site, wcscmp(argv[i + 2], L"out") == 0 ? ImageEnd() + 0x90000 + behind : wcscmp(argv[i + 2], L"hole") == 0 ? g_popHole + behind :
						wcscmp(argv[i + 2], L"odd") == 0 ? target + 2 : wcscmp(argv[i + 2], L"byte") == 0 ? target + 1 : target + 8);
				}
	if (HasOption(argc, argv, L"popgetterbyte"))
		g_popPedGetter[60] = 0x90;
	if (HasOption(argc, argv, L"popgetterbytev"))
		g_popVehicleGetter[60] = 0x90;
	if (HasOption(argc, argv, L"popgatelist"))
		g_popGate[kPopGateList + 2] = 0x90;
	if (HasOption(argc, argv, L"popgatefloor"))
		g_popGate[kPopGateFloor + 2] = 0x08;
	if (HasOption(argc, argv, L"popgatecall"))
		g_popGate[kPopGateCall] = 0x90;
	if (HasOption(argc, argv, L"popnotstart"))
		g_popSetUp[0] = 0x48;
	if (HasOption(argc, argv, L"popcalltwice"))
	{
		g_popExtraCall[0] = 0xE8;
		SetRel32(Site{ g_popExtraCall, 1, sizeof(g_popExtraCall) }, const_cast<const unsigned char*>(g_popSetUp + kPopStart));
	}
	if (HasOption(argc, argv, L"popwriter"))
	{
		g_popExtraStores[0] = g_popExtraStores[6] = 0x89;
		SetRel32(Site{ g_popExtraStores, 2, sizeof(g_popExtraStores) }, &t.vehicleStory[3]);
		SetRel32(Site{ g_popExtraStores, 8, sizeof(g_popExtraStores) }, &t.pedStory[1]);
	}
	const auto moved = [](const volatile unsigned char* match, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			g_popTwin[i] = match[i];
	};
	if (HasOption(argc, argv, L"popvmoved"))
	{
		const size_t at = kPopBlocks + 6 * kPopBlockBytes;
		moved(g_popSetUp + at, kPopMatchV7 + 4);
		g_popSetUp[at] = 0x90;
		for (size_t i = 0; i < 3; i++)
			SetRel32(Site{ g_popTwin, static_cast<int>(i * kPopBlockBytes + kPopStoreInBlock), sizeof(g_popTwin) }, &t.vehicleStory[i + 1]);
	}
	if (HasOption(argc, argv, L"poppmoved"))
	{
		const size_t at = kPopBlocks + kPopBlockBytes;
		moved(g_popSetUp + at, kPopBlockBytes);
		g_popSetUp[at] = 0x90;
		SetRel32(Site{ g_popTwin, kPopStoreInBlock, sizeof(g_popTwin) }, &t.pedStory[2]);
	}
	if (HasOption(argc, argv, L"popambientmoved"))
	{
		moved(g_popContext, kPopMatchV5);
		g_popContext[0] = 0x90;
		SetRel32(Site{ g_popTwin, 3, sizeof(g_popTwin) }, &t.ambient);
	}
	if (HasOption(argc, argv, L"popgateedge"))
	{
		const size_t at = kRtEdgeMatchAt - 2 * kPopMatchP2;
		for (size_t i = 0; i < kPopMatchP2 + 4; i++)
			g_rtEdge[at + i] = g_popGate[i];
		g_popGate[0] = 0x90;
		SetRel32(Site{ g_rtEdge, static_cast<int>(at + kPopMatchP2), sizeof(g_rtEdge) }, &t.pedLevel);
	}
	if (HasOption(argc, argv, L"popscanmiss"))
	{
		const auto base = const_cast<unsigned char*>(ImageBase());
		const auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
			if (memcmp(section->Name, ".fakesit", 8) == 0)
			{
				const auto end = const_cast<const unsigned char*>(g_popSetUp + kPopBlocks + 8 * kPopBlockBytes + kPopStoreInBlock);
				DWORD old = 0;
				VirtualProtect(section, sizeof(*section), PAGE_READWRITE, &old);
				section->Misc.VirtualSize = static_cast<DWORD>(end - base) - section->VirtualAddress;
				VirtualProtect(section, sizeof(*section), old, &old);
			}
	}
	if (HasOption(argc, argv, L"popref") || HasOption(argc, argv, L"popbadmodel") || HasOption(argc, argv, L"poppoolunread"))
	{
		DWORD old = 0;
		VirtualProtect(g_popHole, sizeof(g_popHoleStorage), PAGE_NOACCESS, &old);
	}
	static_assert(sizeof(g_popSetUp) <= sizeof(g_popPlaced[0]) && sizeof(g_popGate) <= sizeof(g_popPlaced[0]), "the placed bytes hold the largest site");
	for (size_t i = 0; i < kPopSites; i++)
		for (size_t at = 0; at < g_popSites[i].size; at++)
			g_popPlaced[i][at] = g_popSites[i].bytes[at];
	g_popDataHash = PopHashOfData();
}

static volatile LONG g_popFlipping = 0;
static DWORD WINAPI PopFlip(LPVOID)
{
	const auto entry = reinterpret_cast<volatile uint32_t*>(g_popObject + kPopPopulationList + kPopFlipEntry * sizeof(uint32_t));
	while (g_popFlipping)
	{
		*entry = 0xFFFF;
		*entry = static_cast<uint32_t>(kPopFirstModel + kPopFlipEntry);
	}
	return 0;
}

// Once TweaksV has started, as the game's streaming would change the data: what the options ask, each waiting for its summary.
static void PopActions(int argc, wchar_t** argv)
{
	HANDLE flip = nullptr;
	if (HasOption(argc, argv, L"popflip"))
	{
		g_popFlipping = 1;
		flip = CreateThread(nullptr, 0, PopFlip, nullptr, 0, nullptr);
	}
	if (HasOption(argc, argv, L"poplate") && RtWaitFor("the model budgets are not filled yet", 25000))
	{
		PopFill(argc, argv);
		g_popDataHash = PopHashOfData();
		RtWaitFor("DIAGNOSTIC: ped model budget: ", 10000);
	}
	if (HasOption(argc, argv, L"popzeros") && RtWaitFor("the model budgets are not filled yet", 25000))
	{
		PopLayOutLists();
		g_popDataHash = PopHashOfData();
		RtWaitFor("DIAGNOSTIC: ped model budget: ", 10000);
	}
	if (HasOption(argc, argv, L"poptablelater") && RtWaitFor("DIAGNOSTIC: ped model budget: ", 25000))
	{
		const uint32_t kb = 98500;
		memcpy(g_popConfiguration + 0x514, &kb, sizeof(kb));
		g_popTables->vehicleStory[3] = 100000000;
		g_popDataHash = PopHashOfData();
		RtWaitFor("428.7 of 600.0 MB", 10000);
	}
	if (HasOption(argc, argv, L"popgone"))
	{
		DWORD old = 0;
		VirtualProtect(g_popObject, 0x1000, PAGE_NOACCESS, &old);
		RtWaitFor("could not be read this time", 25000);
		VirtualProtect(g_popObject, 0x1000, PAGE_READWRITE, &old);
		RtWaitFor("DIAGNOSTIC: ped model budget: ", 10000);
	}
	if (HasOption(argc, argv, L"popgrow") && RtWaitFor("DIAGNOSTIC: ped model budget: ", 25000))
	{
		PopTables& t = *g_popTables;
		PopField<int32_t>(kPopPedUsed, 550000000);
		PopList(kPopPopulationList, kPopFirstModel, 40);
		t.ambient = 64;
		PopPoolField(0x28, 120);
		Sleep(1500);
		PopField<int32_t>(kPopPedUsed, 500000000);
		PopList(kPopPopulationList, kPopFirstModel, 19);
		PopField<int32_t>(0x621C, 1000000);
		PopField<int32_t>(0x6224, 1500000);
		PopField<int32_t>(0x6234, 2000000);
		PopField<int32_t>(0x6238, 3000000);
		PopField<uint32_t>(0x62BC, 2);
		t.ambient = 50;
		PopPoolField(0x28, 100);
		g_popDataHash = PopHashOfData();
		RtWaitFor("at most 550.0 MB so far", 10000);
		t.vehicleLevel = t.pedLevel = 1;
		g_popDataHash = PopHashOfData();
		RtWaitFor("at level 1 (", 10000);
		t.vehicleLevel = t.pedLevel = 3;
		g_popDataHash = PopHashOfData();
		RtWaitFor("at level 1 at the lowest so far", 10000);
	}
	if (HasOption(argc, argv, L"popconfiglate") && RtWaitFor("could not be read this time", 25000))
	{
		g_popTables->holder = g_popManager;
		g_popDataHash = PopHashOfData();
		RtWaitFor("54000, 72000 and 92000, the game's defaults", 10000);
	}
	if (HasOption(argc, argv, L"popwait"))
		RtWaitFor("DIAGNOSTIC: ped model budget: ", 25000);
	if (flip)
	{
		g_popFlipping = 0;
		WaitForSingleObject(flip, INFINITE);
		CloseHandle(flip);
	}
	if (HasOption(argc, argv, L"popquiet"))
		Sleep(3000);
	if (HasOption(argc, argv, L"popstayssec") && RtWaitFor("the model budgets are not filled yet", 25000))
		Sleep(8000);
	for (int i = 2; i + 1 < argc; i++)
		if (wcscmp(argv[i], L"popstay") == 0)
			Sleep(1000 * static_cast<DWORD>(_wtoi(argv[i + 1])));
}

// "pop sites: as placed | pop data: as placed | pop rvas: object 0x..., ... | " - TweaksV writes nothing, and where the data lie, for the
// suite to compare with the log's lines.
static void PopReport()
{
	bool placed = true;
	for (size_t i = 0; i < kPopSites; i++)
		for (size_t at = 0; at < g_popSites[i].size; at++)
			if (g_popSites[i].bytes[at] != g_popPlaced[i][at])
				placed = false;
	const auto rva = [](const volatile void* p) {
		return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(const_cast<const void*>(p)) - reinterpret_cast<uintptr_t>(ImageBase()));
	};
	const PopTables& t = *g_popTables;
	wprintf(L"pop sites: %ls | pop data: %ls | pop rvas: object 0x%llX, vehicle story 0x%llX, ped story 0x%llX, set-up 0x%llX | ", placed ? L"as placed" : L"CHANGED",
		PopHashOfData() == g_popDataHash ? L"as placed" : L"CHANGED", rva(g_popObject), rva(t.vehicleStory), rva(t.pedStory), rva(g_popSetUp + kPopStart));
}
// The ped pool's data, on pages of their own, at the game's layouts, and what the sites' calls lead to, which nothing runs: the pool, its slots and their flag bytes, the spatial
// array; each ped's type, the flag every loop of the game skips it by, the flag of a ped waiting for reuse.
struct PedPoolObject
{
	void* allocator;
	unsigned char* slots;
	unsigned char* flags;
	int32_t count;
	int32_t stride;
	int32_t freeHead;
	int32_t freeTail;
	uint32_t inUse;			// the low 30 bits: the fake's top bits show that they are masked
};
static_assert(offsetof(PedPoolObject, slots) == 0x08 && offsetof(PedPoolObject, flags) == 0x10 && offsetof(PedPoolObject, count) == 0x18 && offsetof(PedPoolObject, stride) == 0x1C &&
	offsetof(PedPoolObject, inUse) == 0x28, "the game's pool layout");
struct PedPoolData
{
	uint64_t value;
	uint64_t keys;
	uint64_t spatial;
	uint8_t flag;
	uint16_t timesliceCount;
	int32_t number;
	int32_t ambientCars, ambientFoot, scenarioCars, scenarioFoot;
	int32_t capCars, capFoot, capScenario;
};
constexpr int32_t kPpSlots = 256, kPpStride = 0x1980;
constexpr uint64_t kPpKeys = 0x0123456789ABCDEF;
alignas(4096) static unsigned char g_ppDataPage[4096];
alignas(4096) static unsigned char g_ppNoAccessPage[4096];
static unsigned char g_ppAllocatorProxy[16];
static const Site g_ppSites[] = { SITE(g_ppAccessor, 0), SITE(g_ppMaker, 0), SITE(g_ppPoolLookup, 0), SITE(g_ppFileValue, 0), SITE(g_ppSetter, 0), SITE(g_ppIntelligence, 0), SITE(g_ppSpatial, 0),
	SITE(g_ppCounters, 0), SITE(g_ppCaps, 0), SITE(g_ppCopy, 0), SITE(g_ppReset, 0), SITE(g_ppFiller, 0), SITE(g_ppPriority, 0), SITE(g_ppDirector, 0), SITE(g_ppCreature, 0), SITE(g_ppMotionTree, 0),
	SITE(g_ppExpressions, 0), SITE(g_ppRagDoll, 0), SITE(g_ppFacialRig, 0), { g_ppEdge, 0, 0x1000 } };
static unsigned char g_ppPlaced[0x2000];
static PedPoolObject* g_ppPool = nullptr;
static unsigned char* g_ppSlots = nullptr;
static unsigned char* g_ppFlags = nullptr;
static unsigned char* g_ppSpatialArray = nullptr;
static uint64_t g_ppDecoded = 0;
static PedPoolData* PedPool()
{
	return reinterpret_cast<PedPoolData*>(g_ppDataPage);
}
static void PedPoolGameFunction()
{
	wprintf(L"the ped pool's sites' calls lead here, and nothing runs them | ");
}
static void PedPoolSetRel32(volatile unsigned char* site, size_t size, size_t at, const void* target)
{
	SetRel32({ site, static_cast<int>(at), size }, target);
}
// Before TweaksV loads: the sites' operands as the options ask, then the sites' bytes as placed, and the page that cannot be read as they ask.
static void PedPoolSetUpSites(int argc, wchar_t** argv)
{
	PedPoolData* const data = PedPool();
	data->number = HasOption(argc, argv, L"ppnumber") ? 300 : kPpSlots;
	const auto option = [argc, argv](const wchar_t* name) { return HasOption(argc, argv, name); };
	const auto bytes = reinterpret_cast<const unsigned char*>(data);
	const unsigned char* const outside = ImageEnd() + 0x77000;
	const auto game = reinterpret_cast<const unsigned char*>(&PedPoolGameFunction);
	const unsigned char* const other = game + 1;
	const void* const flag = &data->flag;
	const void* const value = &data->value;
	const void* const keys = &data->keys;
	const void* const number = &data->number;
	const void* const spatial = &data->spatial;
	const void* const scenarioFoot = &data->scenarioFoot;
	const void* const capScenario = &data->capScenario;
	const void* const count = &data->timesliceCount;

	PedPoolSetRel32(g_ppAccessor, sizeof(g_ppAccessor), 3, option(L"ppflagout") ? outside : flag);
	PedPoolSetRel32(g_ppAccessor, sizeof(g_ppAccessor), 17, option(L"ppkeysno") ? g_ppNoAccessPage : keys);
	PedPoolSetRel32(g_ppAccessor, sizeof(g_ppAccessor), 28, option(L"ppvalueodd") ? bytes + offsetof(PedPoolData, value) + 4 : value);
	PedPoolSetRel32(g_ppMaker, sizeof(g_ppMaker), 3, option(L"ppproxyout") ? outside : g_ppAllocatorProxy);
	PedPoolSetRel32(g_ppPoolLookup, sizeof(g_ppPoolLookup), 9, option(L"pplookupout") ? outside : game);
	PedPoolSetRel32(g_ppFileValue, sizeof(g_ppFileValue), 12, option(L"ppfilelookup") ? other : game);
	PedPoolSetRel32(g_ppFileValue, sizeof(g_ppFileValue), 19, option(L"ppsetterout") ? outside : option(L"ppsetterno") ? g_ppNoAccessPage : const_cast<const unsigned char*>(g_ppSetter));
	PedPoolSetRel32(g_ppSetter, sizeof(g_ppSetter), 2, option(L"ppnumberodd") ? bytes + offsetof(PedPoolData, number) + 2 : number);
	if (option(L"ppsetterfirst"))
		g_ppSetter[0] = 0x8B;
	if (option(L"ppsettersecond"))
		g_ppSetter[1] = 0x15;
	if (option(L"ppsetterbytes"))
		g_ppSetter[sizeof(g_ppSetter) - 1] = 0xCC;
	PedPoolSetRel32(g_ppIntelligence, sizeof(g_ppIntelligence), 9, option(L"ppintelligence") ? other : game);
	for (size_t i = 12; i < 63; i++)
		g_ppSpatial[i] = 0x90;
	g_ppSpatial[63] = option(L"ppstore") ? 0x4C : 0x48;
	g_ppSpatial[64] = 0x89;
	g_ppSpatial[65] = 0x1D;
	PedPoolSetRel32(g_ppSpatial, sizeof(g_ppSpatial), 66, option(L"ppspatialout") ? outside : spatial);
	PedPoolSetRel32(g_ppCounters, sizeof(g_ppCounters), 2, &data->ambientCars);
	PedPoolSetRel32(g_ppCounters, sizeof(g_ppCounters), 8, &data->ambientFoot);
	PedPoolSetRel32(g_ppCounters, sizeof(g_ppCounters), 14, &data->scenarioCars);
	PedPoolSetRel32(g_ppCounters, sizeof(g_ppCounters), 20, option(L"ppcountout") ? outside : scenarioFoot);
	PedPoolSetRel32(g_ppCaps, sizeof(g_ppCaps), 6, &data->capCars);
	PedPoolSetRel32(g_ppCaps, sizeof(g_ppCaps), 20, &data->capFoot);
	PedPoolSetRel32(g_ppCaps, sizeof(g_ppCaps), 28, option(L"ppcapodd") ? bytes + offsetof(PedPoolData, capScenario) + 2 : capScenario);
	PedPoolSetRel32(g_ppCopy, sizeof(g_ppCopy), 3, option(L"ppgetterout") ? outside : game);
	PedPoolSetRel32(g_ppCopy, sizeof(g_ppCopy), 16, option(L"ppgetters") ? other : game);
	// the count's operand ends past its imm16, 2 bytes past the rel32
	PedPoolSetRel32(g_ppReset, sizeof(g_ppReset), 3, (option(L"ppresetout") ? outside : static_cast<const unsigned char*>(count)) - 2);
	PedPoolSetRel32(g_ppReset, sizeof(g_ppReset), 10, option(L"ppfiller") ? game : const_cast<const unsigned char*>(g_ppFiller));
	PedPoolSetRel32(g_ppFiller, sizeof(g_ppFiller), 34, option(L"ppfillerout") ? outside : game);
	for (size_t i = 24; i < 0x308; i++)
		g_ppPriority[i] = 0x90;
	static const unsigned char kRestore[] = { 0x48, 0x81, 0xC4, 0x48, 0x10, 0x00, 0x00 };
	for (size_t i = 0; i < sizeof(kRestore); i++)
		g_ppPriority[0x308 + i] = kRestore[i];
	if (option(L"pprestore"))
		g_ppPriority[0x30B] = 0x49;
	PedPoolSetRel32(g_ppPriority, sizeof(g_ppPriority), 6, game);
	PedPoolSetRel32(g_ppPriority, sizeof(g_ppPriority), 16, option(L"pppointer") ? &data->keys : spatial);
	// a match moved to the first page's end, its first byte changed where it was, so that it is the one match
	const auto toEdge = [](const volatile unsigned char* match, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			g_ppEdge[0x1000 - size + i] = match[i];
	};
	if (option(L"ppstoreunread"))
	{
		toEdge(g_ppSpatial, 12);
		g_ppSpatial[0] = 0x90;
	}
	if (option(L"ppfillerunread"))
	{
		toEdge(g_ppFiller + 5, 29);
		g_ppFiller[5] = 0x90;
		PedPoolSetRel32(g_ppReset, sizeof(g_ppReset), 10, const_cast<const unsigned char*>(g_ppEdge) + 0x1000 - 29 - 5);
	}
	if (option(L"pprestoreunread"))
	{
		toEdge(g_ppPriority, 24);
		g_ppPriority[0] = 0x90;
	}

	size_t at = 0;
	for (const Site& site : g_ppSites)
		for (size_t i = 0; i < site.size; i++)
			g_ppPlaced[at++] = site.bytes[i];
	DWORD old = 0;
	if (option(L"ppkeysno") || option(L"ppnoflags") || option(L"ppnopeds") || option(L"ppsetterno") || option(L"ppspatialfault"))
		VirtualProtect(g_ppNoAccessPage, sizeof(g_ppNoAccessPage), PAGE_NOACCESS, &old);
}
// Once the whole of .fakesit is read+execute, before TweaksV loads: an edge's second page, as the options "...unread" ask.
static void PedPoolTakeAway(int argc, wchar_t** argv)
{
	DWORD old = 0;
	if (HasOption(argc, argv, L"ppstoreunread") || HasOption(argc, argv, L"ppfillerunread") || HasOption(argc, argv, L"pprestoreunread"))
		VirtualProtect(const_cast<unsigned char*>(g_ppEdge) + 0x1000, 0x1000, PAGE_NOACCESS, &old);
	if (HasOption(argc, argv, L"pmcallunread"))
		VirtualProtect(const_cast<unsigned char*>(g_poolMemoryEdge) + 0x1000, 0x1000, PAGE_NOACCESS, &old);
}
// The peds as the game keeps them: the first 12 mission peds, of whom 2 more that every loop skips; 4 waiting for reuse; every free slot's bytes stale, as a mission ped's waiting for reuse.
static void PedPoolSetInUse(uint32_t inUse, int32_t spatial, int argc, wchar_t** argv)
{
	const uint32_t slots = static_cast<uint32_t>(kPpSlots);
	for (uint32_t i = 0; i < slots; i++)
	{
		unsigned char* const ped = g_ppSlots + static_cast<size_t>(i) * kPpStride;
		const bool free = i >= inUse;
		g_ppFlags[i] = free ? 0x8F : 0x0F;
		const bool mission = free || i < 12;
		const bool waiting = free || (i >= 12 && i < 16);
		const bool skipped = !free && (i == 16 || i == 17);
		ped[0xDA] = mission || i == 16 ? 0xF7 : 0xF9;		// 6 of bits 1 to 3 a mission ped's, the other bits set
		ped[0x1091] = skipped ? 0xFF : 0xEF;				// bit 4 what every loop skips
		ped[0x1468] = waiting || i == 17 ? 0xFF : 0xEF;		// bit 4 waiting for reuse
	}
	g_ppPool->inUse = (HasOption(argc, argv, L"ppinusebig") ? 257 : inUse) | 0xC0000000;
	memcpy(g_ppSpatialArray + 0x5C, &spatial, sizeof(spatial));
}
static void PedPoolSetSpatialCapacity(int32_t capacity)
{
	memcpy(g_ppSpatialArray + 0x58, &capacity, sizeof(capacity));
}
// The pool made as the game makes it: its pointer obfuscated in the game's three fields, with the population's counts and caps.
static void PedPoolMake(int argc, wchar_t** argv)
{
	PedPoolData* const data = PedPool();
	g_ppPool = static_cast<PedPoolObject*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	g_ppSlots = static_cast<unsigned char*>(VirtualAlloc(nullptr, static_cast<size_t>(kPpSlots) * kPpStride, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	g_ppFlags = static_cast<unsigned char*>(VirtualAlloc(nullptr, kPpSlots, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	g_ppSpatialArray = static_cast<unsigned char*>(VirtualAlloc(nullptr, 0x60, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	g_ppPool->slots = HasOption(argc, argv, L"ppnopeds") ? g_ppNoAccessPage : g_ppSlots;
	g_ppPool->flags = HasOption(argc, argv, L"ppnoflags") ? g_ppNoAccessPage : g_ppFlags;
	g_ppPool->count = HasOption(argc, argv, L"ppcountzero") ? 0 : HasOption(argc, argv, L"ppcountbig") ? 20000 : kPpSlots;
	g_ppPool->stride = HasOption(argc, argv, L"ppstride") ? 0x1000 : kPpStride;
	PedPoolSetSpatialCapacity(HasOption(argc, argv, L"ppspatialbig") ? 20000 : HasOption(argc, argv, L"ppspatialover") ? 300 : kPpSlots);
	data->scenarioFoot = 40;
	data->scenarioCars = 2;
	data->capScenario = 40;
	data->ambientFoot = 61;
	data->ambientCars = 18;
	data->capFoot = 64;
	data->capCars = 16;
	if (!HasOption(argc, argv, L"ppnospatial"))
		data->spatial = reinterpret_cast<uintptr_t>(HasOption(argc, argv, L"ppspatialfault") ? g_ppNoAccessPage : g_ppSpatialArray);
	PedPoolSetInUse(HasOption(argc, argv, L"ppfull") ? kPpSlots : HasOption(argc, argv, L"ppinusepeak") ? 257 : 171,
		HasOption(argc, argv, L"ppspatialover") || HasOption(argc, argv, L"ppspatialbig") ? 300 : HasOption(argc, argv, L"ppbadspatial") ? 500 : 165, argc, argv);
	const uint64_t turned = _rotl64(kPpKeys, 30);
	const int rotation = 32 + static_cast<int>(turned & 0x1F) + 2;
	data->keys = kPpKeys;
	data->value = _rotr64(~reinterpret_cast<uintptr_t>(g_ppPool), rotation) ^ turned;
	data->flag = 0x81;
	g_ppDecoded = reinterpret_cast<uint64_t (*)()>(const_cast<unsigned char*>(g_ppAccessor))();
}
// Once TweaksV has started, as the game makes the pool when its session starts.
static void PedPoolActions(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"pprun"))
		return;
	if (HasOption(argc, argv, L"pplate"))
		Sleep(11000);
	PedPoolMake(argc, argv);
	Sleep(2500);
	PedPoolSetInUse(143, HasOption(argc, argv, L"ppbadspatial") ? 500 : 139, argc, argv);
	if (HasOption(argc, argv, L"ppfreed"))
		VirtualFree(g_ppPool, 0, MEM_RELEASE);
	if (HasOption(argc, argv, L"ppgone"))
	{
		DWORD old = 0;
		VirtualProtect(g_ppDataPage, sizeof(g_ppDataPage), PAGE_NOACCESS, &old);
	}
	if (HasOption(argc, argv, L"ppmore"))
	{
		Sleep(11000);
		PedPoolSetInUse(150, 140, argc, argv);
	}
	if (HasOption(argc, argv, L"ppmore") || HasOption(argc, argv, L"pplate"))
		Sleep(3000);
}
// "ped pool: sites as placed, made, its accessor decodes it | " - the sites, which TweaksV must not change, and the pool, with what the game's own accessor makes of the fake's encoding.
static void PedPoolReport()
{
	bool placed = true;
	size_t at = 0;
	for (const Site& site : g_ppSites)
		for (size_t i = 0; i < site.size; i++)
			placed = placed && site.bytes[i] == g_ppPlaced[at++];
	wprintf(L"ped pool: sites %ls, ", placed ? L"as placed" : L"CHANGED");
	if (g_ppPool)
		wprintf(L"made, its accessor %ls | ", g_ppDecoded == reinterpret_cast<uintptr_t>(g_ppPool) ? L"decodes it" : L"does NOT decode it");
	else
		wprintf(L"not made | ");
}

// The allocator a pool's memory comes from, on a page of its own, as the constructor and the allocation of the game's lay it out, and the site as placed.
static uint32_t Imm32(const Site& site);
struct PoolMemoryAllocator
{
	void* vtable;
	void* heap;
	void* blockList;
	uint64_t threshold;
	uint64_t budget;
	uint64_t used;
	uint64_t room;
	uint64_t leastRoom;
	uint16_t blocks;
	uint8_t flags;
};
static_assert(offsetof(PoolMemoryAllocator, threshold) == 0x18 && offsetof(PoolMemoryAllocator, budget) == 0x20 && offsetof(PoolMemoryAllocator, used) == 0x28 &&
	offsetof(PoolMemoryAllocator, room) == 0x30 && offsetof(PoolMemoryAllocator, leastRoom) == 0x38 && offsetof(PoolMemoryAllocator, blocks) == 0x40 &&
	offsetof(PoolMemoryAllocator, flags) == 0x42, "the game's allocator layout");
alignas(4096) static unsigned char g_poolMemoryPage[4096];
static unsigned char g_poolMemoryHeap[16];
static unsigned char g_poolMemoryPlaced[sizeof(g_poolMemorySite)];
static uint64_t g_poolMemoryGiven = 0;		// what the constructor was handed
static bool g_poolMemoryMade = false;
static int g_poolMemoryRefused = 0;
static PoolMemoryAllocator* PoolMemory()
{
	return reinterpret_cast<PoolMemoryAllocator*>(g_poolMemoryPage);
}
static void PoolMemoryConstructor(PoolMemoryAllocator* allocator, void* heap, uint64_t threshold, uint64_t budget)
{
	allocator->heap = heap;
	allocator->threshold = threshold;
	allocator->budget = budget;
	allocator->used = 0;
	allocator->room = budget;
	allocator->leastRoom = budget;
	allocator->blocks = 0;
	allocator->flags = 1;
	g_poolMemoryGiven = budget;
	g_poolMemoryMade = true;
}
// A big block's charge: its size rounded up to 64 KB and up to 64 KB more; past the room, null, which the game then writes through.
static uint64_t PoolMemoryCharge(uint64_t size)
{
	return (size + 0x1003F) & ~uint64_t{ 0xFFFF };
}
static void PoolMemoryAllocate(uint64_t size)
{
	PoolMemoryAllocator* const allocator = PoolMemory();
	const uint64_t charge = PoolMemoryCharge(size);
	if (charge > allocator->room)
	{
		g_poolMemoryRefused++;
		return;
	}
	allocator->room -= charge;
	allocator->used += charge;
	if (allocator->room < allocator->leastRoom)
		allocator->leastRoom = allocator->room;
	allocator->blocks++;
}
static void PoolMemoryGiveBack(uint64_t size)
{
	PoolMemoryAllocator* const allocator = PoolMemory();
	const uint64_t charge = PoolMemoryCharge(size);
	allocator->room += charge;
	allocator->used -= charge;
	allocator->blocks--;
}
// The site run as the game runs it, then the load.
static bool PoolMemoryDoesNotRun(int argc, wchar_t** argv)
{
	return HasOption(argc, argv, L"pmnotmade") || HasOption(argc, argv, L"pmoutside") || HasOption(argc, argv, L"pmunaligned") || HasOption(argc, argv, L"pmunreadable") ||
		HasOption(argc, argv, L"pmheapout") || HasOption(argc, argv, L"pmcallout") || HasOption(argc, argv, L"pmcallunread") || HasOption(argc, argv, L"pmgone");
}
static void PoolMemoryRun(int argc, wchar_t** argv)
{
	if (PoolMemoryDoesNotRun(argc, argv))
		return;
	reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_poolMemorySite))();
	PoolMemoryAllocate(8 << 20);
	for (int i = 0; i < 120; i++)
		PoolMemoryAllocate(1 << 20);
	if (HasOption(argc, argv, L"pmheavy"))
		PoolMemoryAllocate(2 << 20);
	PoolMemoryGiveBack(1 << 20);
	if (HasOption(argc, argv, L"pmnosum"))
		PoolMemory()->used += 1 << 20;
}
// Before TweaksV loads: the site as the options ask, then its bytes as placed - and with "pmbefore" the set-up and the load, with "pmunreadable" the allocator's page taken away.
static void PoolMemorySetUpSite(int argc, wchar_t** argv)
{
	const size_t size = sizeof(g_poolMemorySite);
	const void* allocator = g_poolMemoryPage;
	if (HasOption(argc, argv, L"pmoutside"))
		allocator = ImageEnd() + 0x73000;
	else if (HasOption(argc, argv, L"pmunaligned"))
		allocator = g_poolMemoryPage + 4;
	SetRel32({ g_poolMemorySite, static_cast<int>(kPoolMemoryAllocatorAt), size }, allocator);
	SetRel32({ g_poolMemorySite, static_cast<int>(kPoolMemoryHeapAt), size }, HasOption(argc, argv, L"pmheapout") ? ImageEnd() + 0x74000 : g_poolMemoryHeap);
	SetRel32({ g_poolMemorySite, static_cast<int>(kPoolMemoryCallAt), size }, HasOption(argc, argv, L"pmcallout") ? static_cast<const void*>(ImageEnd() + 0x75000) : reinterpret_cast<const void*>(&PoolMemoryConstructor));
	if (HasOption(argc, argv, L"pmcallunread"))
	{
		constexpr size_t kMatch = 27, kAt = 0x1000 - kMatch;	// the match without its rel32, ending the first page; its first byte changed where it was
		for (size_t i = 0; i < kMatch; i++)
			g_poolMemoryEdge[kAt + i] = g_poolMemorySite[kPoolMemoryMatch + i];
		SetRel32({ g_poolMemoryEdge, static_cast<int>(kAt + 3), sizeof(g_poolMemoryEdge) }, g_poolMemoryPage);
		SetRel32({ g_poolMemoryEdge, static_cast<int>(kAt + 10), sizeof(g_poolMemoryEdge) }, g_poolMemoryHeap);
		g_poolMemorySite[kPoolMemoryMatch] = 0x90;
	}
	if (HasOption(argc, argv, L"pmfield"))
		PoolMemory()->budget = 200u << 20;
	if (HasOption(argc, argv, L"pmraised"))
		SetImm32({ g_poolMemorySite, static_cast<int>(kPoolMemoryBudgetAt), size }, 200u << 20);
	if (HasOption(argc, argv, L"pmodd"))
		SetImm32({ g_poolMemorySite, static_cast<int>(kPoolMemoryBudgetAt), size }, (200u << 20) | 1);
	for (size_t i = 0; i < size; i++)
		g_poolMemoryPlaced[i] = g_poolMemorySite[i];
	if (HasOption(argc, argv, L"pmbefore"))
		PoolMemoryRun(argc, argv);
	DWORD old = 0;
	if (HasOption(argc, argv, L"pmunreadable"))
		VirtualProtect(g_poolMemoryPage, sizeof(g_poolMemoryPage), PAGE_NOACCESS, &old);
}
// Once TweaksV has started, as the game does its start-up.
static void PoolMemoryActions(int argc, wchar_t** argv)
{
	DWORD old = 0;
	if (HasOption(argc, argv, L"pmgone"))
		VirtualProtect(g_poolMemoryPage, sizeof(g_poolMemoryPage), PAGE_NOACCESS, &old);
	if (!HasOption(argc, argv, L"pmbefore"))
		PoolMemoryRun(argc, argv);
	if (HasOption(argc, argv, L"pmmore"))
	{
		Sleep(11000);
		PoolMemoryAllocate(1 << 20);
		Sleep(3000);
	}
}
// "pool memory: 256 MB, the rest as placed, its constructor given 256 MB, 120 blocks, 0 refused | " - what the site holds, the rest of the site as
// placed or changed, and what the game's start-up made of it, or "its constructor not run".
static void PoolMemoryReport()
{
	const size_t size = sizeof(g_poolMemorySite);
	bool restPlaced = true;
	for (size_t i = 0; i < size; i++)
		restPlaced = restPlaced && ((i >= kPoolMemoryBudgetAt && i < kPoolMemoryBudgetAt + 4) || g_poolMemorySite[i] == g_poolMemoryPlaced[i]);
	const uint32_t held = Imm32({ g_poolMemorySite, static_cast<int>(kPoolMemoryBudgetAt), size });
	wprintf(L"pool memory: 0x%08X, %u MB, the rest %ls, ", held, held >> 20, restPlaced ? L"as placed" : L"CHANGED");
	if (g_poolMemoryMade)
		wprintf(L"its constructor given %llu MB, %d blocks, %d refused | ", static_cast<unsigned long long>(g_poolMemoryGiven >> 20), PoolMemory()->blocks, g_poolMemoryRefused);
	else
		wprintf(L"its constructor not run | ");
}
// The render queues (TweaksVRenderQueues). What TweaksV reads lies in one page of the fake's .data, and what the pools and the arrays
// hold in memory of the fake's game allocator: the code RqSetUp points the sites at runs on it as the game's, so that the module's
// numbers are what the patterns' own bytes wrote and its decoding of a pool's pointer is what the patterns' own bytes decode.
constexpr int kRqPools = 6, kRqFamilies = 3, kRqArrays = 4;
enum { kRqPedRequests, kRqPedRender, kRqPropRequests, kRqPropRender, kRqVehicleRequests, kRqVehicleRender };
enum { kRqPeds, kRqProps, kRqVehicles };
static const int32_t kRqStrides[kRqPools] = { 0x508, 0x2E0, 0x80, 0x3C, 0x1C8, 0xBB0 };
static const uint32_t kRqRefAt[kRqFamilies] = { 0x2D8, 0x38, 0xBA8 };		// where a family's gfx counts its references, a word
static const int kRqRenderPool[kRqFamilies] = { kRqPedRender, kRqPropRender, kRqVehicleRender };
static const uint64_t kRqKeys[kRqPools] = { 0x0123456789ABCDEFull, 0xFEDCBA9876543210ull, 0x5A5AA5A55A5AA5A7ull, 0x1F2E3D4C5B6A7988ull, 0x8000000000000011ull, 0x7766554433221100ull };
struct RqPool
{
	void* allocator;			// +0x00
	unsigned char* storage;		// +0x08
	unsigned char* slotBytes;	// +0x10
	int32_t capacity;			// +0x18
	int32_t stride;				// +0x1C
	int32_t firstFree;			// +0x20
	int32_t unused;				// +0x24
	uint32_t count;				// +0x28: its low 30 bits, 0x40000000 set
	uint32_t unused2;			// +0x2C
};
static_assert(sizeof(RqPool) == 0x30 && offsetof(RqPool, capacity) == 0x18 && offsetof(RqPool, stride) == 0x1C && offsetof(RqPool, count) == 0x28, "a pool at the game's offsets");
struct RqArray
{
	uint64_t* entries;			// +0x00
	uint16_t count;				// +0x08
	uint16_t capacity;			// +0x0A
	uint32_t unused;
};
struct RqFamily
{
	RqArray arrays[kRqArrays];
	int32_t index;				// +0x40
	int32_t unused;
	uint64_t lock;				// +0x48
	uint64_t unused2[6];
};
static_assert(sizeof(RqArray) == 16 && offsetof(RqFamily, index) == 0x40 && offsetof(RqFamily, lock) == 0x48, "a family at the game's offsets");
struct RqFields
{
	uint64_t flag;				// its low byte
	uint64_t value;
	uint64_t key;
};
struct RqData
{
	RqFields fields[kRqPools];
	RqFamily families[kRqFamilies];
	int32_t sizes[2];			// the vehicles': what VehicleStreamRequest and VehicleStreamRender came to
	unsigned char* manager;		// the draw list manager's global
	void* config;				// the config manager's global
	uint64_t aByte;				// what the end of an init compares
};
static_assert(sizeof(RqData) <= 4096, "the game's data the module reads fits its page");
alignas(4096) static unsigned char g_rqDataPage[4096];
static RqData& g_rqData = *reinterpret_cast<RqData*>(g_rqDataPage);
struct alignas(4096) RqHole { unsigned char bytes[4096]; };
static RqHole g_rqHole;			// inside the game's .data: no access with "...unread" and "...gone"
static unsigned char* g_rqGone = nullptr;	// a page of no access outside the game: a pointer that leads nowhere
static RqPool* g_rqObjects[kRqPools] = {};
static unsigned char g_rqSpare[0x1000];		// what a make that found no room hands back
static unsigned char g_rqVehicleThis[0x50];	// the object the vehicles' render pool's make reads, as the game's its +0x48
static int g_rqVehicleRenderNamed = -1;		// what the fake's configuration names CVehicleStreamRenderGfx
static LONG g_rqMade = 0, g_rqMadeWrong = 0, g_rqNoRoom = 0, g_rqLookupCalls = 0, g_rqLookupsWrong = 0, g_rqLocks = 0, g_rqFrames = 0, g_rqRefsLetGo = 0, g_rqRefsWrong = 0;
static LONG g_rqFaults = 0;		// the first-chance access violations the module's reads have cost, on the pages an option takes away
static wchar_t g_rqBad[24] = L"";
static int g_rqWhich = 0;
static bool g_rqAsked = false, g_rqBuilt = false;
static size_t g_rqV2Room = 0;
static uint64_t g_rqSitesPlaced = 0, g_rqDataLeft = 0;
static bool g_rqEntriesGone = false, g_rqDataGone = false, g_rqDataChanged = false;

static bool RqIs(const wchar_t* what)
{
	return wcscmp(g_rqBad, what) == 0;
}

// True if "NAME:I" is among the options for this I.
static bool RqHasFor(int argc, wchar_t** argv, const wchar_t* name, int i)
{
	wchar_t option[32];
	swprintf(option, sizeof(option) / sizeof(option[0]), L"%ls:%d", name, i);
	return HasOption(argc, argv, option);
}

static uint64_t RqRol(uint64_t value, int by)
{
	by &= 63;
	return by ? (value << by) | (value >> (64 - by)) : value;
}

// The game's stand-ins the sites call. A site's make takes a slot through RqAllocate, which sees the pointer the site's own bytes decoded.
static __declspec(noinline) void RqNothing()
{
	InterlockedIncrement(&g_rqLocks);
}
struct RqBlock
{
	void* at;
	SIZE_T size;
};
static RqBlock g_rqBlocks[64] = {};
static size_t g_rqBlockCount = 0;
static __declspec(noinline) void* RqGameAllocate(uint32_t size, uint32_t)
{
	const SIZE_T bytes = size < 0x10000 ? 0x10000 : size;	// room for an array to be written past
	void* const block = VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (g_rqBlockCount < sizeof(g_rqBlocks) / sizeof(g_rqBlocks[0]))
		g_rqBlocks[g_rqBlockCount++] = { block, bytes };
	return block;
}
// While nothing of the fake's changes them, the game's data and memory are read-only: a write of the module's would fault, whatever it wrote.
static void RqProtectData(DWORD protection)
{
	DWORD old = 0;
	VirtualProtect(g_rqDataPage, sizeof(g_rqDataPage), protection, &old);
	for (size_t i = 0; i < g_rqBlockCount; i++)
		VirtualProtect(g_rqBlocks[i].at, g_rqBlocks[i].size, protection, &old);
}
static __declspec(noinline) int32_t RqLookup(void* config, uint32_t hash, int32_t defaultValue)
{
	InterlockedIncrement(&g_rqLookupCalls);
	if (config != &g_rqData.config)
		InterlockedIncrement(&g_rqLookupsWrong);
	switch (hash)
	{
	case 0x90D4BFF4: return 30;		// VehicleStreamRequest
	case 0xE11D5267: return 60;		// VehicleStreamRender
	case 0xC2B5F089: return g_rqVehicleRenderNamed >= 0 ? g_rqVehicleRenderNamed : defaultValue;	// CVehicleStreamRenderGfx
	default: return defaultValue;
	}
}
static int32_t RqCount(const RqPool& pool)
{
	return static_cast<int32_t>(pool.count << 2) >> 2;
}
static __declspec(noinline) void* RqAllocate(RqPool* pool, uint32_t size)
{
	InterlockedIncrement(&g_rqMade);
	int which = 0;
	while (which < kRqPools && static_cast<uint32_t>(kRqStrides[which]) != size)
		which++;
	if (which == kRqPools || pool != g_rqObjects[which])
	{
		InterlockedIncrement(&g_rqMadeWrong);
		return g_rqSpare;
	}
	const int32_t count = RqCount(*pool);
	if (count >= pool->capacity)
	{
		InterlockedIncrement(&g_rqNoRoom);
		return g_rqSpare;
	}
	pool->count = 0x40000000u | static_cast<uint32_t>(count + 1);
	return pool->storage + static_cast<size_t>(count) * static_cast<size_t>(pool->stride);
}
// What a flip does past its head: each entry of the array it enters loses its reference, and its count goes to 0.
static __declspec(noinline) void RqFlipDone(int family)
{
	RqFamily& f = g_rqData.families[family];
	RqArray& entered = f.arrays[f.index & 3];
	for (uint32_t i = 0; i < entered.count; i++)
	{
		const auto refs = reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(static_cast<uintptr_t>(entered.entries[i])) + kRqRefAt[family]);
		InterlockedIncrement(&g_rqRefsLetGo);
		if (*refs == 0)
			InterlockedIncrement(&g_rqRefsWrong);
		else
			(*refs)--;
	}
	entered.count = 0;
}
static __declspec(noinline) void RqAfterFlips(void*, int)
{
	InterlockedIncrement(&g_rqFrames);
}
// The draw list manager's constructor: its first array of references through its site, the other three as the game goes on.
static __declspec(noinline) void RqManagerMade(unsigned char* manager)
{
	const auto next = reinterpret_cast<void* (*)(void*, void*)>(const_cast<unsigned char*>(g_rqReferences))(manager, RqGameAllocate(0x28800, 8));
	for (int i = 1; i < kRqArrays; i++)
	{
		RqArray& references = reinterpret_cast<RqArray*>(manager + 0x5D0)[i];
		references.entries = static_cast<uint64_t*>(i == 1 ? next : RqGameAllocate(0x28800, 8));
		references.capacity = 0x2880;
	}
}

// The carrier of the make of the vehicles' render pool: a function with a frame of its own, so that the fake's unwind data names it, and
// room for its stand-in: its prologue, its pattern's 78 bytes, its make, and 185 bytes past the match's start the branch of a full pool.
static __declspec(noinline) void RqVehicleCarrier()
{
	volatile int frame[32];
	for (int i = 0; i < 32; i++)
		frame[i] = g_sirenCountSink + 7 * i;
	for (int i = 1; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0x5157);
	for (int i = 2; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 2] ^ 0x1234);
	for (int i = 3; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 3] ^ 0x4321);
	for (int i = 4; i < 32; i++)
		frame[i] = frame[i] - (frame[i - 4] ^ 0x7654);
	for (int i = 5; i < 32; i++)
		frame[i] = frame[i] + (frame[i - 5] ^ 0x3210);
	g_sirenCountSink = frame[5] + frame[10] + frame[15] + frame[20] + frame[25] + frame[30];
}
constexpr size_t kRqV2Match = 9, kRqV2Branch = kRqV2Match + 185;
static const unsigned char kRqMakeV2[] = { 0x53, 0x57, 0x48, 0x83, 0xEC, 0x28, 0x48, 0x89, 0xCF,
	0x0F, 0xB6, 0x05, 0, 0, 0, 0, 0x48, 0x8B, 0x0D, 0, 0, 0, 0, 0x48, 0xC1, 0xC1, 0x1E, 0x48, 0x8B, 0x05, 0, 0, 0, 0, 0x48, 0x31, 0xC8, 0x80, 0xE1, 0x1F, 0x80, 0xC1, 0x03, 0x48, 0xD3, 0xC0,
	0x48, 0xC1, 0xC0, 0x20, 0x48, 0xF7, 0xD0, 0x0F, 0xB6, 0x0D, 0, 0, 0, 0, 0x31, 0xD2, 0xF6, 0xC1, 0x01, 0x48, 0x0F, 0x45, 0xD0, 0x8B, 0x42, 0x28, 0xC1, 0xE0, 0x02, 0xC1, 0xF8, 0x02,
	0x48, 0x8B, 0x5F, 0x48, 0x39, 0x42, 0x18, 0x7E, 0x6B,
	0x48, 0x89, 0xD1, 0xBA, 0xB0, 0x0B, 0, 0, 0xE8, 0, 0, 0, 0, 0x48, 0x83, 0xC4, 0x28, 0x5F, 0x5B, 0xC3 };
static const unsigned char kRqMakeV2Branch[] = { 0x31, 0xC0, 0x48, 0x83, 0xC4, 0x28, 0x5F, 0x5B, 0xC3 };

// Where each pool's make lies, its match in its site, and from the match its fields' rel32, its two bytes and its make's rel32.
struct RqMake
{
	volatile unsigned char* site;
	size_t match, flag, key, rol, value, add, flagAgain, allocate;
};
static RqMake g_rqMakes[kRqPools] = {
	{ g_rqMakeS1, 5, 3, 10, 17, 21, 36, 59, 80 }, { g_rqMakeS2, 7, 17, 24, 31, 35, 51, 61, 80 }, { g_rqMakeP1, 8, 3, 10, 17, 21, 33, 47, 70 },
	{ g_rqMakeP2, 7, 3, 10, 17, 21, 37, 47, 67 }, { g_rqMakeV1, 6, 3, 10, 17, 21, 37, 47, 66 }, { nullptr, kRqV2Match, 3, 10, 17, 21, 33, 47, 87 } };
// Each flip's match, and each family's first bytes of each site that loads its arrays.
static volatile unsigned char* const g_rqFlipSites[kRqFamilies] = { g_rqFlipPeds, g_rqFlipProps, g_rqFlipVehicles };
static const size_t kRqFlipMatch[kRqFamilies] = { 6, 7, 6 };
static const size_t kRqFlipOrder[kRqFamilies] = { kRqVehicles, kRqPeds, kRqProps };	// the frame's calls, in an order no pattern says
static volatile unsigned char* const g_rqLookupSites[kRqPools] = { g_rqLookupN1, g_rqLookupN2, g_rqLookupN3, g_rqLookupN4, g_rqLookupN5, g_rqLookupN6 };
static const size_t kRqLookupMatch[kRqPools] = { 4, 4, 4, 4, 7, 7 }, kRqLookupCall[kRqPools] = { 18, 18, 18, 18, 15, 15 };
static volatile unsigned char* const g_rqInits[2] = { g_rqInitPed, g_rqInitProp };
static volatile unsigned char* const g_rqArrayLoads[kRqFamilies] = { g_rqAddPed + 11 + 22, g_rqAddProp + 11 + 19, g_rqAddVehicle + 9 + 29 };

static void RqRel32(volatile unsigned char* at, const volatile void* target)
{
	const Site site = { at, 0, 0 };
	SetRel32(site, const_cast<const void*>(target));
}

// A pool's pointer as the game keeps it, encrypted by the two bytes its make holds: what DecodePoolPointer and the make's own bytes undo.
static void RqSetPointer(int pool, const void* pointer)
{
	const RqMake& make = g_rqMakes[pool];
	const volatile unsigned char* const match = make.site + make.match;
	RqFields& fields = g_rqData.fields[pool];
	fields.key = kRqKeys[pool];
	const uint64_t turned = RqRol(fields.key, match[make.rol]);
	const int by = 32 + static_cast<int>(turned & 0x1F) + match[make.add];
	fields.value = RqRol(~static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer)), 64 - (by & 63)) ^ turned;
	fields.flag = 0xA5A5A5A5A5A5A501ull;
}

// The end of .data, from the section table.
static const unsigned char* RqDataEnd()
{
	const unsigned char* const base = ImageBase();
	const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
	const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
	for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
		if (memcmp(section->Name, ".data\0\0", 8) == 0)
			return base + section->VirtualAddress + section->Misc.VirtualSize;
	return base;
}

// "rqbad:...": the site not as the research found it, for each of the module's checks, each alone. No site runs in such a run.
static void RqBreakSite()
{
	const unsigned char* const outside = ImageEnd() + 0xC0000;
	RqFields* const fields = g_rqData.fields;
	if (RqIs(L"rol0") || RqIs(L"rol64") || RqIs(L"add64") || RqIs(L"add2") || RqIs(L"value") || RqIs(L"key") || RqIs(L"out") || RqIs(L"end") || RqIs(L"odd") || RqIs(L"unread"))
	{
		const RqMake& make = g_rqMakes[g_rqWhich];
		volatile unsigned char* const match = make.site + make.match;
		const auto setFields = [&](const unsigned char* at)
		{
			RqRel32(match + make.flag, at);
			RqRel32(match + make.value, at + 8);
			RqRel32(match + make.key, at + 16);
		};
		if (RqIs(L"rol0"))
			match[make.rol] = 0;
		if (RqIs(L"rol64"))
			match[make.rol] = 64;
		if (RqIs(L"add64"))
			match[make.add] = 64;
		if (RqIs(L"add2"))
			match[make.add] = 2;
		if (RqIs(L"value"))
			RqRel32(match + make.value, &fields[g_rqWhich].key);
		if (RqIs(L"key"))
			RqRel32(match + make.key, reinterpret_cast<const unsigned char*>(&fields[g_rqWhich].key) + 8);
		if (RqIs(L"out"))
			setFields(const_cast<const unsigned char*>(g_rqNotData));
		if (RqIs(L"end"))
			setFields(RqDataEnd() - 8);
		if (RqIs(L"odd"))
			setFields(reinterpret_cast<const unsigned char*>(&fields[g_rqWhich]) + 1);
		if (RqIs(L"unread"))
			setFields(g_rqHole.bytes);
	}
	if (RqIs(L"trap"))
		g_rqMakeS2[kRqTrapAt] = 0x90;
	if (RqIs(L"edgetrap") || RqIs(L"edgelookup"))
	{
		DWORD old = 0, unused = 0;
		const bool trap = RqIs(L"edgetrap");
		const size_t size = trap ? 80 : kRqLookupCall[g_rqWhich] + 1;
		volatile unsigned char* const from = trap ? g_rqMakeS2 + 7 : g_rqLookupSites[g_rqWhich] + kRqLookupMatch[g_rqWhich];
		volatile unsigned char* const to = g_rqEdge + 0x1000 - (trap ? 100 : size);
		VirtualProtect(const_cast<unsigned char*>(g_rqEdge), 0x1000, PAGE_READWRITE, &old);
		for (size_t i = 0; i < size; i++)
			to[i] = from[i];
		VirtualProtect(const_cast<unsigned char*>(g_rqEdge), 0x1000, old, &unused);
		from[size - 1] = 0x90;		// the pattern's last byte: it matches at the edge alone
		if (trap)					// the site's fields as the one at the edge has them, each rel32 counted from there
		{
			VirtualProtect(const_cast<unsigned char*>(g_rqEdge), 0x1000, PAGE_READWRITE, &old);
			RqRel32(to + 17, &fields[kRqPedRender]);
			RqRel32(to + 24, &fields[kRqPedRender].key);
			RqRel32(to + 35, &fields[kRqPedRender].value);
			VirtualProtect(const_cast<unsigned char*>(g_rqEdge), 0x1000, old, &unused);
		}
		VirtualProtect(const_cast<unsigned char*>(g_rqEdge) + 0x1000, 0x1000, PAGE_NOACCESS, &old);
	}
	if (RqIs(L"arraysother") || RqIs(L"arraysout") || RqIs(L"arraysodd") || RqIs(L"arraysunread"))
	{
		const unsigned char* const arrays = reinterpret_cast<const unsigned char*>(g_rqData.families[g_rqWhich].arrays);
		RqRel32(g_rqArrayLoads[g_rqWhich], RqIs(L"arraysother") ? arrays + 16 : RqIs(L"arraysout") ? const_cast<const unsigned char*>(g_rqNotData) :
			RqIs(L"arraysodd") ? arrays + 1 : g_rqHole.bytes);
	}
	if (RqIs(L"initarray") || RqIs(L"initcapacity"))	// N: the family, ten times, and the array, 1 to 3 or for the capacity 0 to 3
	{
		const int family = g_rqWhich / 10, i = g_rqWhich % 10;
		volatile unsigned char* const block = g_rqInits[family] + 4 + 24 + 31 * i;
		if (RqIs(L"initarray"))
			RqRel32(block + 18, &g_rqData.families[family].arrays[i] + 1);
		else
			RqRel32(block + 25, &g_rqData.families[family].arrays[i].unused);
	}
	if (RqIs(L"vehicleindex"))
		RqRel32(g_rqAddVehicle + 51 + 12, &g_rqData.families[kRqVehicles].lock);
	if (RqIs(L"vehicleinitarray"))
		RqRel32(g_rqInitVehicle + 6 + 44, &g_rqData.families[kRqVehicles].arrays[1]);
	if (RqIs(L"vehiclesize"))
		RqRel32(g_rqSizes + 4 + 54, &g_rqData.sizes[0]);
	if (RqIs(L"sizecall"))	// N: VehicleStreamRequest's or VehicleStreamRender's
		RqRel32(g_rqSizes + 4 + (g_rqWhich == 0 ? 19 : 48), outside);
	if (RqIs(L"lookupout"))
		RqRel32(g_rqLookupSites[g_rqWhich] + kRqLookupMatch[g_rqWhich] + kRqLookupCall[g_rqWhich] + 1, outside);
	if (RqIs(L"lookupother"))
		RqRel32(g_rqLookupSites[g_rqWhich] + kRqLookupMatch[g_rqWhich] + kRqLookupCall[g_rqWhich] + 1, reinterpret_cast<const void*>(&RqNothing));
	if (RqIs(L"flipout"))	// N: the call, 0 to 2
		RqRel32(g_rqFlipsCall + 12 + 5 * g_rqWhich, outside);
	if (RqIs(L"flipunread"))
		RqRel32(g_rqFlipsCall + 12 + 5 * g_rqWhich, g_rqHole.bytes);
	if (RqIs(L"fliphead"))	// N: the family
		g_rqFlipSites[g_rqWhich][kRqFlipMatch[g_rqWhich] + 31] = 0xF8;
	if (RqIs(L"flipindex"))
		RqRel32(g_rqFlipSites[g_rqWhich] + kRqFlipMatch[g_rqWhich] + 14, &g_rqData.sizes[0]);
	if (RqIs(L"fliplock"))
		RqRel32(g_rqFlipSites[g_rqWhich] + kRqFlipMatch[g_rqWhich] + 3, &g_rqData.sizes[1]);
	if (RqIs(L"flipsame"))
		RqRel32(g_rqFlipsCall + 17, g_rqFlipSites[kRqFlipOrder[0]]);
	if (RqIs(L"managerout"))
		RqRel32(g_rqManagerMade + 5 + 24, g_rqNotData);
	if (RqIs(L"managerodd"))
		RqRel32(g_rqManagerMade + 5 + 24, reinterpret_cast<const unsigned char*>(&g_rqData.manager) + 4);
	if (RqIs(L"managerunread"))
		RqRel32(g_rqManagerMade + 5 + 24, g_rqHole.bytes);
}

static uint64_t RqSitesHash()
{
	uint64_t hash = 0xCBF29CE484222325ull;
	for (const Site& site : g_rqSites)
		hash = SirenCountHash(hash, site.bytes, site.size);
	hash = SirenCountHash(hash, g_rqEdge, 0x1000);
	return g_rqBuilt ? SirenCountHash(hash, SirenCountCode(&RqVehicleCarrier), g_rqV2Room) : hash;
}

// All the module reads of the game's: its data, the pools, the arrays' entries and the references.
static uint64_t RqDataHash()
{
	uint64_t hash = SirenCountHash(0xCBF29CE484222325ull, &g_rqData, sizeof(g_rqData));
	for (const RqPool* const pool : g_rqObjects)
		if (pool && !g_rqEntriesGone)
			hash = SirenCountHash(hash, pool, sizeof(RqPool));
	for (const RqFamily& family : g_rqData.families)
		for (const RqArray& array : family.arrays)
			if (array.entries && !g_rqEntriesGone)
				hash = SirenCountHash(hash, array.entries, array.count * sizeof(uint64_t));
	if (g_rqData.manager && reinterpret_cast<unsigned char*>(g_rqData.manager) != g_rqGone)
		hash = SirenCountHash(hash, g_rqData.manager + 0x5D0, kRqArrays * sizeof(RqArray));
	return hash;
}

// A vectored handler, as any in the process: what a first-chance access violation on those pages reaches, each counted.
static LONG CALLBACK RqCountFaults(EXCEPTION_POINTERS* info)
{
	const EXCEPTION_RECORD& record = *info->ExceptionRecord;
	if (record.ExceptionCode == EXCEPTION_ACCESS_VIOLATION && record.NumberParameters >= 2)
	{
		const auto at = reinterpret_cast<const unsigned char*>(record.ExceptionInformation[1]);
		if ((g_rqGone && at >= g_rqGone && at < g_rqGone + 0x10000) || (at >= g_rqDataPage && at < g_rqDataPage + sizeof(g_rqDataPage)))
			InterlockedIncrement(&g_rqFaults);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

// Before TweaksV loads: every site's rel32 pointed at the game's data and the stand-ins, the options' changes, and then the sites as placed.
static void RqSetUp(int argc, wchar_t** argv)
{
	for (int i = 1; i < argc; i++)
		if (wcsncmp(argv[i], L"rq", 2) == 0)
		{
			g_rqAsked = true;
			if (wcsncmp(argv[i], L"rqbad:", 6) == 0)
			{
				const wchar_t* const colon = wcschr(argv[i] + 6, L':');
				const size_t length = colon ? static_cast<size_t>(colon - (argv[i] + 6)) : wcslen(argv[i] + 6);
				if (length < sizeof(g_rqBad) / sizeof(g_rqBad[0]))
				{
					wmemcpy(g_rqBad, argv[i] + 6, length);
					g_rqBad[length] = L'\0';
				}
				g_rqWhich = colon ? _wtoi(colon + 1) : 0;
			}
		}
	if (g_rqAsked)
		AddVectoredExceptionHandler(1, &RqCountFaults);
	unsigned char* const carrier = SirenCountCode(&RqVehicleCarrier);
	g_rqV2Room = SirenCountRoom(carrier, kRqV2Branch + sizeof(kRqMakeV2Branch) + 1);
	g_rqBuilt = g_rqV2Room != 0;
	DWORD carrierProtection = 0, unused = 0;
	if (g_rqBuilt)
	{
		const size_t at = RqIs(L"v2short") ? g_rqV2Room - 100 - kRqV2Match : 0;	// its full pool's jump leads past the function's end
		VirtualProtect(carrier, g_rqV2Room, PAGE_EXECUTE_READWRITE, &carrierProtection);
		memset(carrier, 0xCC, g_rqV2Room);
		volatile unsigned char* const v2 = RqIs(L"v2nounwind") ? g_rqVehicleSpare : carrier + at;
		const size_t room = RqIs(L"v2nounwind") ? sizeof(g_rqVehicleSpare) : g_rqV2Room - at;
		for (size_t i = 0; i < sizeof(kRqMakeV2) && i < room; i++)
			v2[i] = kRqMakeV2[i];
		for (size_t i = 0; i < sizeof(kRqMakeV2Branch) && kRqV2Branch + i < room; i++)
			v2[kRqV2Branch + i] = kRqMakeV2Branch[i];
		g_rqMakes[kRqVehicleRender].site = v2;
	}
	RqData& data = g_rqData;
	for (int i = 0; g_rqBuilt && i < kRqPools; i++)
	{
		const RqMake& make = g_rqMakes[i];
		volatile unsigned char* const match = make.site + make.match;
		RqRel32(match + make.flag, &data.fields[i].flag);
		RqRel32(match + make.key, &data.fields[i].key);
		RqRel32(match + make.value, &data.fields[i].value);
		RqRel32(match + make.flagAgain, &data.fields[i].flag);
		RqRel32(match + make.allocate, reinterpret_cast<const void*>(&RqAllocate));
	}
	RqRel32(g_rqAddPed + 11 + 11, &data.families[kRqPeds].index);
	RqRel32(g_rqAddPed + 11 + 22, data.families[kRqPeds].arrays);
	RqRel32(g_rqAddProp + 11 + 8, &data.families[kRqProps].index);
	RqRel32(g_rqAddProp + 11 + 19, data.families[kRqProps].arrays);
	RqRel32(g_rqAddVehicle + 9 + 6, &data.families[kRqVehicles].lock);
	RqRel32(g_rqAddVehicle + 9 + 11, reinterpret_cast<const void*>(&RqNothing));
	RqRel32(g_rqAddVehicle + 9 + 18, &data.families[kRqVehicles].index);
	RqRel32(g_rqAddVehicle + 9 + 29, data.families[kRqVehicles].arrays);
	RqRel32(g_rqAddVehicle + 51 + 12, &data.families[kRqVehicles].index);
	for (int family = 0; family < 2; family++)
	{
		volatile unsigned char* const init = g_rqInits[family] + 4;
		RqRel32(init + 8, reinterpret_cast<const void*>(&RqNothing));
		RqRel32(init + 20, reinterpret_cast<const void*>(&RqNothing));
		for (int i = 0; i < kRqArrays; i++)
		{
			volatile unsigned char* const block = init + 24 + 31 * i;
			RqRel32(block + 11, reinterpret_cast<const void*>(&RqGameAllocate));
			RqRel32(block + 18, &data.families[family].arrays[i].entries);
			RqRel32(block + 25, reinterpret_cast<const unsigned char*>(&data.families[family].arrays[i].capacity) - 2);	// counted from past its imm16
		}
	}
	RqRel32(g_rqInitPed + 4 + 150, reinterpret_cast<const unsigned char*>(&data.aByte) - 1);	// counted from past its imm8
	volatile unsigned char* const sizes = g_rqSizes + 4;
	RqRel32(sizes + 3, &data.config);
	RqRel32(sizes + 19, reinterpret_cast<const void*>(&RqLookup));
	RqRel32(sizes + 25, &data.sizes[0]);
	RqRel32(sizes + 32, &data.config);
	RqRel32(sizes + 48, reinterpret_cast<const void*>(&RqLookup));
	RqRel32(sizes + 54, &data.sizes[1]);
	volatile unsigned char* const vehicleInit = g_rqInitVehicle + 6;
	RqRel32(vehicleInit + 3, &data.sizes[1]);
	RqRel32(vehicleInit + 30, reinterpret_cast<const void*>(&RqGameAllocate));
	RqRel32(vehicleInit + 37, &data.sizes[0]);
	RqRel32(vehicleInit + 44, &data.families[kRqVehicles].arrays[0].entries);
	RqRel32(vehicleInit + 51, &data.families[kRqVehicles].arrays[0].capacity);
	for (int i = 0; i < kRqPools; i++)
	{
		volatile unsigned char* const lookup = g_rqLookupSites[i] + kRqLookupMatch[i];
		RqRel32(lookup + 3, &data.config);
		RqRel32(lookup + kRqLookupCall[i] + 1, reinterpret_cast<const void*>(&RqLookup));
	}
	for (int i = 0; i < kRqFamilies; i++)
	{
		const size_t family = kRqFlipOrder[i];
		volatile unsigned char* const flip = g_rqFlipSites[family];
		volatile unsigned char* const head = flip + kRqFlipMatch[family];
		RqRel32(g_rqFlipsCall + 12 + 5 * i, flip);
		RqRel32(head + 3, &data.families[family].lock);
		RqRel32(head + 8, reinterpret_cast<const void*>(&RqNothing));
		RqRel32(head + 14, &data.families[family].index);
		RqRel32(head + 44, &data.families[family].index);
		const uint32_t which = static_cast<uint32_t>(family);
		for (size_t k = 0; k < sizeof(which); k++)
			head[53 + k] = reinterpret_cast<const unsigned char*>(&which)[k];
		RqRel32(head + 58, reinterpret_cast<const void*>(&RqFlipDone));
	}
	RqRel32(g_rqFlipsCall + 38, reinterpret_cast<const void*>(&RqAfterFlips));
	RqRel32(g_rqReferences + 11 + 27, reinterpret_cast<const void*>(&RqGameAllocate));
	RqRel32(g_rqManagerMade + 5 + 6, reinterpret_cast<const void*>(&RqGameAllocate));
	RqRel32(g_rqManagerMade + 5 + 17, reinterpret_cast<const void*>(&RqManagerMade));
	RqRel32(g_rqManagerMade + 5 + 24, &data.manager);
	data.config = &data.config;
	if (HasOption(argc, argv, L"rqrs") && g_rqBuilt)
	{
		static const unsigned char rols[kRqPools] = { 0x21, 0x0D, 0x2F, 0x07, 0x3F, 0x01 }, adds[kRqPools] = { 1, 9, 0, 17, 63, 31 };
		for (int i = 0; i < kRqPools; i++)
		{
			const RqMake& make = g_rqMakes[i];
			make.site[make.match + make.rol] = rols[i];
			make.site[make.match + make.add] = adds[i];
		}
	}
	if (HasOption(argc, argv, L"rqpast"))
		g_rqVehicleRenderNamed = 80;
	RqBreakSite();
	if (carrierProtection)
	{
		VirtualProtect(carrier, g_rqV2Room, carrierProtection, &unused);
		FlushInstructionCache(GetCurrentProcess(), carrier, g_rqV2Room);
	}
	g_rqSitesPlaced = RqSitesHash();
	DWORD old = 0;
	if (RqIs(L"unread") || RqIs(L"arraysunread") || RqIs(L"flipunread") || RqIs(L"managerunread"))
		VirtualProtect(g_rqHole.bytes, sizeof(g_rqHole.bytes), PAGE_NOACCESS, &old);
	g_rqDataLeft = RqDataHash();
	RqProtectData(PAGE_READONLY);
}

// Runs a site as a function of one argument: a make, an add.
static void* RqRun(volatile unsigned char* code, void* argument)
{
	return reinterpret_cast<void* (*)(void*)>(const_cast<unsigned char*>(code))(argument);
}
static unsigned char* RqMakeSlot(int pool)
{
	const RqMake& make = g_rqMakes[pool];
	return static_cast<unsigned char*>(RqRun(make.site, pool == kRqPedRender ? static_cast<void*>(g_rqObjects[pool]) : pool == kRqVehicleRender ? static_cast<void*>(g_rqVehicleThis) : nullptr));
}
static void RqMakeSlots(int pool, int count)
{
	for (int i = 0; i < count; i++)
		RqMakeSlot(pool);
}
static unsigned char* RqSlot(int pool, int slot)
{
	return g_rqObjects[pool]->storage + static_cast<size_t>(slot) * static_cast<size_t>(g_rqObjects[pool]->stride);
}
static void RqFreeSlots(int pool, int count)
{
	g_rqObjects[pool]->count = 0x40000000u | static_cast<uint32_t>(RqCount(*g_rqObjects[pool]) - count);
}
// The vehicles' add, with its dedupe in the walk the pattern's head begins: a vehicle enters a frame once.
static void RqAddVehicle(unsigned char* gfx)
{
	const RqFamily& family = g_rqData.families[kRqVehicles];
	const RqArray& array = family.arrays[family.index & 3];
	for (uint32_t i = 0; i < array.count; i++)
		if (array.entries[i] == reinterpret_cast<uintptr_t>(gfx))
			return;
	RqRun(g_rqAddVehicle, gfx);
}
// A frame's flips, the draw list's rotation and the references that frame has.
static void RqNextFrame(uint16_t references)
{
	reinterpret_cast<void (*)(void*, int)>(const_cast<unsigned char*>(g_rqFlipsCall))(g_rqData.manager, 0);
	if (!g_rqData.manager || g_rqData.manager == g_rqGone)
		return;
	reinterpret_cast<void (*)(void*)>(const_cast<unsigned char*>(g_rqRotation))(g_rqData.manager);
	const int index = *reinterpret_cast<const int*>(g_rqData.manager + 0xAC);
	reinterpret_cast<RqArray*>(g_rqData.manager + 0x5D0)[index & 3].count = references;
}
// A frame: so many peds of the render pool, so many entries each; props with one seen twice; vehicles with three seen again.
static void RqDrawFrame(int peds, int perPed, int props, int vehicles, uint16_t references)
{
	RqNextFrame(references);
	for (int ped = 0; ped < peds; ped++)
		for (int entry = 0; entry < perPed; entry++)
			RqRun(g_rqAddPed, RqSlot(kRqPedRender, ped));
	for (int prop = 0; prop < props; prop++)
		RqRun(g_rqAddProp, RqSlot(kRqPropRender, prop));
	if (props)
		RqRun(g_rqAddProp, RqSlot(kRqPropRender, 0));
	for (int vehicle = 0; vehicle < vehicles; vehicle++)
		RqAddVehicle(RqSlot(kRqVehicleRender, vehicle));
	for (int vehicle = 0; vehicle < 3 && vehicle < vehicles; vehicle++)
		RqAddVehicle(RqSlot(kRqVehicleRender, vehicle));
}

// The inits, as the game runs them once TweaksV has started, and every pool made through its size lookup, but as the options ask.
static void RqMakeAll(int argc, wchar_t** argv)
{
	using Run = int32_t (*)(int32_t);
	RqRun(g_rqInitPed, nullptr);
	RqRun(g_rqInitProp, nullptr);
	RqRun(g_rqSizes, nullptr);
	RqRun(g_rqInitVehicle, nullptr);
	RqArray* const vehicles = g_rqData.families[kRqVehicles].arrays;
	for (int i = 1; i < kRqArrays; i++)	// as the game goes on past its pattern
	{
		vehicles[i].entries = static_cast<uint64_t*>(RqGameAllocate(static_cast<uint32_t>(g_rqData.sizes[1]) * 8, 8));
		vehicles[i].capacity = static_cast<uint16_t>(g_rqData.sizes[1]);
	}
	if (HasOption(argc, argv, L"rqmanagergone"))
		g_rqData.manager = g_rqGone;
	else if (!HasOption(argc, argv, L"rqnomanager"))
		RqRun(g_rqManagerMade, nullptr);
	for (int i = 0; i < kRqPools; i++)
	{
		const int32_t capacity = reinterpret_cast<Run>(const_cast<unsigned char*>(g_rqLookupSites[i]))(i == kRqVehicleRequests ? g_rqData.sizes[0] : g_rqData.sizes[1]);
		if (RqHasFor(argc, argv, L"rqnomake", i))
		{
			g_rqData.fields[i] = { 0xA5A5A5A5A5A5A5FEull, 0x1111111111111111ull, 0x2222222222222222ull };
			continue;
		}
		const auto pool = static_cast<RqPool*>(RqGameAllocate(sizeof(RqPool), 8));
		pool->capacity = capacity;
		pool->stride = HasOption(argc, argv, L"rqstride") ? kRqStrides[i] + 1 : HasOption(argc, argv, L"rqstridezero") ? 0 : kRqStrides[i];
		pool->firstFree = 0;
		pool->count = 0x40000000u;
		pool->storage = static_cast<unsigned char*>(RqGameAllocate(static_cast<uint32_t>(capacity) * static_cast<uint32_t>(kRqStrides[i]), 8));
		g_rqObjects[i] = pool;
		RqSetPointer(i, RqHasFor(argc, argv, L"rqpoolgone", i) ? static_cast<const void*>(g_rqGone) : RqHasFor(argc, argv, L"rqpoolimage", i) ? static_cast<const void*>(g_rqSpare) : pool);
	}
}

// Once TweaksV has started: the game's work, as the options ask, held long enough for the module's samples to see it.
static void RqActions(int argc, wchar_t** argv)
{
	if (!g_rqBuilt)
		return;
	if (RqDataHash() != g_rqDataLeft)
		g_rqDataChanged = true;
	RqProtectData(PAGE_READWRITE);
	g_rqGone = static_cast<unsigned char*>(VirtualAlloc(nullptr, 0x10000, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS));
	const bool full = HasOption(argc, argv, L"rqfull"), past = HasOption(argc, argv, L"rqpast"), many = HasOption(argc, argv, L"rqmany"), refsFull = HasOption(argc, argv, L"rqfullrefs");
	const bool load = HasOption(argc, argv, L"rqload") || full || past || many || refsFull;
	if (load || HasOption(argc, argv, L"rqmade"))
		RqMakeAll(argc, argv);
	if (HasOption(argc, argv, L"rqstridebrief"))
		for (int spell = 0; spell < 4; spell++)
		{
			for (int i = 0; i < kRqPools; i++)
				g_rqObjects[i]->stride = kRqStrides[i] + 1;
			Sleep(500);
			for (int i = 0; i < kRqPools; i++)
				g_rqObjects[i]->stride = kRqStrides[i];
			Sleep(300);
		}
	if (HasOption(argc, argv, L"rqentriesgone"))
	{
		g_rqEntriesGone = true;
		g_rqData.families[kRqPeds].arrays[1].entries = reinterpret_cast<uint64_t*>(g_rqGone);
		g_rqData.families[kRqPeds].arrays[1].count = 40;
		g_rqData.families[kRqVehicles].arrays[2].entries = nullptr;
		g_rqData.families[kRqVehicles].arrays[2].count = 7;
		g_rqObjects[kRqPedRequests]->count = 0x7FFFFFFFu;
	}
	else if (HasOption(argc, argv, L"rqnulls"))
	{
		RqArray& array = g_rqData.families[kRqProps].arrays[2];
		for (int i = 0; i < 5; i++)
			array.entries[i] = reinterpret_cast<uintptr_t>(RqSlot(kRqPropRender, i));
		array.count = 10;
	}
	else if (load)
	{
		const bool made[kRqPools] = { !RqHasFor(argc, argv, L"rqnomake", 0) && !RqHasFor(argc, argv, L"rqpoolgone", 0), !RqHasFor(argc, argv, L"rqnomake", 1) && !RqHasFor(argc, argv, L"rqpoolgone", 1),
			!RqHasFor(argc, argv, L"rqnomake", 2) && !RqHasFor(argc, argv, L"rqpoolgone", 2), !RqHasFor(argc, argv, L"rqnomake", 3) && !RqHasFor(argc, argv, L"rqpoolgone", 3),
			!RqHasFor(argc, argv, L"rqnomake", 4) && !RqHasFor(argc, argv, L"rqpoolgone", 4), !RqHasFor(argc, argv, L"rqnomake", 5) && !RqHasFor(argc, argv, L"rqpoolgone", 5) };
		const int peak[kRqPools] = { 9, 31, 4, 8, 3, 11 }, latest[kRqPools] = { 0, 14, 0, 3, 0, 5 };
		for (int i = 0; i < kRqPools; i++)
			if (made[i])
				RqMakeSlots(i, full ? g_rqObjects[i]->capacity : past && (i == kRqPedRender || i == kRqPropRender || i == kRqVehicleRender) ? g_rqObjects[i]->capacity : peak[i]);
		const uint16_t references = refsFull ? 10368 : 4210;
		if (many)
		{
			RqNextFrame(references);
			for (int entry = 0; entry < 5000; entry++)
				RqRun(g_rqAddPed, RqSlot(kRqPedRender, entry < 4096 ? entry % 31 : 31 + entry % 9));
		}
		else if (past)
		{
			RqNextFrame(references);
			for (int entry = 0; entry < 130; entry++)
				RqRun(g_rqAddPed, RqSlot(kRqPedRender, entry % 65));
			for (int entry = 0; entry < 125; entry++)
				RqRun(g_rqAddProp, RqSlot(kRqPropRender, entry % 100));
			for (int entry = 0; entry < 70; entry++)
				RqAddVehicle(RqSlot(kRqVehicleRender, entry));
		}
		else
			RqDrawFrame(made[kRqPedRender] ? 21 : 0, 3, made[kRqPropRender] ? 8 : 0, made[kRqVehicleRender] ? 9 : 0, references);
		Sleep(1000);	// the most: long enough for a sample on a loaded machine, and well inside the first summary's 10 seconds
		if (!full && !past)
		{
			for (int i = 0; i < kRqPools; i++)
				if (made[i])
					RqFreeSlots(i, peak[i] - latest[i]);
			for (int frame = 0; frame < 4; frame++)		// the last of them, the fifth, comes back to the peak's array
			{
				RqDrawFrame(made[kRqPedRender] ? 14 : 0, 2, made[kRqPropRender] ? 3 : 0, made[kRqVehicleRender] ? 5 : 0, static_cast<uint16_t>(2000 - 100 * frame));
				Sleep(100);
			}
		}
	}
	if (HasOption(argc, argv, L"rqgone"))
	{
		g_rqData.fields[kRqPedRender].flag = 0;
		RqSetPointer(kRqPropRender, g_rqGone);
		g_rqData.manager = g_rqGone;
	}
	g_rqDataLeft = RqDataHash();
	if (HasOption(argc, argv, L"rqgrow"))
	{
		if (RtWaitFor("DIAGNOSTIC: the draw list's references: ", 25000))
		{
			RqMakeSlots(kRqPedRender, 18);
			RqMakeSlots(kRqPropRequests, 1);
			RqMakeSlots(kRqVehicleRender, 6);
			RqDrawFrame(1, 1, 1, 1, 100);		// the index on, past the five frames' second array to its third,
			RqDrawFrame(25, 3, 12, 12, 5000);	// and the new most in its fourth, the last the module reads
			RtWaitFor("the draw list's references: at most 5000 of 10368", 10000);
		}
		g_rqDataLeft = RqDataHash();
	}
	if (HasOption(argc, argv, L"rqlatest"))
	{
		if (RtWaitFor("DIAGNOSTIC: the draw list's references: ", 25000))
		{
			RqMakeSlots(kRqPedRender, 2);
			Sleep(5000);
		}
		g_rqDataLeft = RqDataHash();
	}
	RqProtectData(PAGE_READONLY);
	if (HasOption(argc, argv, L"rqdatagone"))
	{
		DWORD old = 0;
		g_rqDataGone = true;
		VirtualProtect(g_rqDataPage, sizeof(g_rqDataPage), PAGE_NOACCESS, &old);
	}
}

// "render queues: built, sites as placed, data as left | render queues load: 66 slots through the decoding of their sites, 0 wrong, no room for 0, 8 size lookups, 0 wrong, 5
// frames, 81 references let go, 0 wrong | render queues faults: 0 | " and with "rqrvas" the RVAs for the suite: "rq rvas: S1=0x... | ".
static void RqReport(int argc, wchar_t** argv)
{
	if (!g_rqAsked)
		return;
	if (g_rqDataGone)
	{
		DWORD old = 0;
		VirtualProtect(g_rqDataPage, sizeof(g_rqDataPage), PAGE_READONLY, &old);
	}
	wprintf(L"render queues: %ls, sites %ls, data %ls | ", g_rqBuilt ? L"built" : L"NOT BUILT", RqSitesHash() == g_rqSitesPlaced ? L"as placed" : L"CHANGED",
		!g_rqDataChanged && RqDataHash() == g_rqDataLeft ? L"as left" : L"CHANGED");
	wprintf(L"render queues load: %ld slots through the decoding of their sites, %ld wrong, no room for %ld, %ld size lookups, %ld wrong, %ld frames, %ld references let go, %ld wrong | render queues faults: %ld | ",
		g_rqMade, g_rqMadeWrong, g_rqNoRoom, g_rqLookupCalls, g_rqLookupsWrong, g_rqFrames, g_rqRefsLetGo, g_rqRefsWrong, g_rqFaults);
	if (!HasOption(argc, argv, L"rqrvas"))
		return;
	const auto rva = [](const volatile void* at) { return static_cast<unsigned long long>(static_cast<const volatile unsigned char*>(at) - ImageBase()); };
	static const wchar_t* const makes[kRqPools] = { L"S1", L"S2", L"P1", L"P2", L"V1", L"V2" };
	wprintf(L"rq rvas:");
	for (int i = 0; i < kRqPools; i++)
		wprintf(L" %ls=0x%llX F%d=0x%llX C%d=0x%llX", makes[i], rva(g_rqMakes[i].site + g_rqMakes[i].match), i, rva(&g_rqData.fields[i]), i, rva(g_rqLookupSites[i] + kRqLookupMatch[i] + kRqLookupCall[i]));
	const RqFamily* const families = g_rqData.families;
	wprintf(L" TJ=0x%llX T=0x%llX VJ=0x%llX VB=0x%llX VF=0x%llX", rva(g_rqMakeS2 + 7 + 12), rva(g_rqMakeS2 + kRqTrapAt), rva(g_rqMakes[5].site + kRqV2Match + 76), rva(g_rqMakes[5].site + kRqV2Branch), rva(g_rqMakes[5].site));
	wprintf(L" S3=0x%llX S4=0x%llX P3=0x%llX P4=0x%llX V3=0x%llX V4=0x%llX V5=0x%llX V6=0x%llX", rva(g_rqAddPed + 11), rva(g_rqInitPed + 4), rva(g_rqAddProp + 11), rva(g_rqInitProp + 4), rva(g_rqAddVehicle + 51),
		rva(g_rqAddVehicle + 9), rva(g_rqSizes + 4), rva(g_rqInitVehicle + 6));
	for (int i = 0; i < kRqFamilies; i++)
		wprintf(L" A%d=0x%llX I%d=0x%llX L%d=0x%llX", i, rva(families[i].arrays), i, rva(&families[i].index), i, rva(&families[i].lock));
	wprintf(L" ZR=0x%llX ZV=0x%llX CA=0x%llX CB=0x%llX LK=0x%llX", rva(&g_rqData.sizes[0]), rva(&g_rqData.sizes[1]), rva(g_rqSizes + 4 + 18), rva(g_rqSizes + 4 + 47), rva(reinterpret_cast<const void*>(&RqLookup)));
	for (int i = 0; i < kRqFamilies; i++)
		wprintf(L" K%d=0x%llX FF%d=0x%llX H%d=0x%llX", i, rva(g_rqFlipsCall + 11 + 5 * i), i, rva(g_rqFlipSites[kRqFlipOrder[i]]), i, rva(g_rqFlipSites[kRqFlipOrder[i]] + kRqFlipMatch[kRqFlipOrder[i]]));
	wprintf(L" D1=0x%llX D2=0x%llX D3=0x%llX MG=0x%llX | ", rva(g_rqReferences + 11), rva(g_rqRotation), rva(g_rqManagerMade + 5), rva(&g_rqData.manager));
}

// [siren damage]
// The sirens per vehicle's damage (TweaksVSirenDamage): the game's damage code as code that runs, around the patterns' bytes, so that
// what TweaksV writes is read by the real instructions. SdSetUp writes each loop into .fakesit, and the function that makes a bone
// structure and the game's fill over carriers its unwind data names, as the siren count does; each loop reads its bones where the game's
// does, so the numbers it leaves are the raise's.
//   the caller (I-E-Y3)    mov rsi, rcx, then the site's two calls: the maker, or TweaksV's stub, and the fake's next function
//   the maker (I-E-Y2)     a jump on to SdMaker, its allocation and constructor behind it, which SdMaker runs with the pool in rcx
//   the fill (I-E-Y4)      the game's prologue and first call, undone at once, and a jump on to SdFill
//   a hit                  rcx the vehicle, rdx the structure, r8d a bone: for that bone's siren the vehicle's +0x860 bit and +0x865, and its glass's child
//   an impact              rcx the vehicle, its +0x408 the structure, rdx the damage object, r8d a bone: the object's +0x440 bit and +0x445, and the glass's child
//   the hiding             rcx the damage object, rdx the structure, r8 a byte per bone: each marked siren's bone hidden
//   a wreck                rcx the vehicle, its +0x408 the structure: the +0x860 bit of each siren whose bone exists
//   the flags, the classes rcx the vehicle, through +0x20 and +0xB0 to the structure: each glass bone's child gets 0x17FFFFE0, in the array at +0x90
struct SdBone
{
	char name[24];
	uint16_t index;
	uint16_t tag;
};
constexpr int kSdBones = 100;
struct SdModel
{
	unsigned char head[0x18];
	uint32_t hash;						// +0x18: the hash of the model's name
	unsigned char a[0xB0 - 0x1C];
	void* const* holder;				// +0xB0: the holder of its bone structure
	unsigned char b[0x340 - 0xB8];
	uint32_t type;						// +0x340: 8 and 1 have a third table, gear doors in the first glass slots
	unsigned char c[0x54B - 0x344];
	unsigned char flags;				// +0x54B: what the caller's site tests
	void* holding;						// the fake's own from here on: the holder's +0x00, the bone structure
	bool faults;						// the game's fill faults on TweaksV's table
	bool unwritable;					// the game's maker leaves a structure that cannot be written
	int named;
	SdBone bones[kSdBones];				// its skeleton's bones with a name
	unsigned char made[0x34C];			// the bone structure as the game made it, before the wrapper
};
static_assert(offsetof(SdModel, hash) == 0x18 && offsetof(SdModel, holder) == 0xB0 && offsetof(SdModel, type) == 0x340 && offsetof(SdModel, flags) == 0x54B,
	"the damage's models lie at the game's offsets");
struct SdEntry
{
	const char* name;
	int32_t id;
};
static_assert(sizeof(SdEntry) == 16, "an entry of the game's tables of bones");
struct SdVehicle
{
	unsigned char head[0x20];
	const SdModel* model;				// +0x20
	unsigned char a[0x90 - 0x28];
	uint32_t* children;					// +0x90: the flags of the fragment's children
	unsigned char b[0x408 - 0x98];
	const void* structure;				// +0x408
	unsigned char c[0x440 - 0x410];
	uint32_t marked;					// +0x440: the damage object's set
	unsigned char d;
	unsigned char markedByte;			// +0x445
	unsigned char e[0x860 - 0x446];
	uint32_t broken;					// +0x860: the vehicle's broken sirens
	unsigned char f;
	unsigned char brokenByte;			// +0x865
	unsigned char g[0x9C0 - 0x866];
	const void* fragment;				// +0x9C0
};
static_assert(offsetof(SdVehicle, model) == 0x20 && offsetof(SdVehicle, children) == 0x90 && offsetof(SdVehicle, structure) == 0x408 && offsetof(SdVehicle, marked) == 0x440 &&
	offsetof(SdVehicle, markedByte) == 0x445 && offsetof(SdVehicle, broken) == 0x860 && offsetof(SdVehicle, brokenByte) == 0x865 && offsetof(SdVehicle, fragment) == 0x9C0,
	"the damage's vehicles lie at the game's offsets");

enum { kSdPolice, kSdFull, kSdSome, kSdSparse, kSdPlane, kSdLost, kSdFew, kSdDrawn, kSdNoHolder = kSdDrawn, kSdFault, kSdBadStructure, kSdManyModel, kSdModels };
static SdModel g_sdModels[kSdModels];
static unsigned char* volatile g_sdPool = nullptr;	// made once TweaksV has started, as the game makes it; the allocation's load reads it
static uint32_t g_sdStride = 0, g_sdAllocation = 0, g_sdElements = 0, g_sdUsed = 0;
static int g_sdTraps = 0, g_sdMade = 0, g_sdArgsBad = 0, g_sdKeptBoth = 0, g_sdOwnFills = 0, g_sdOwnEntries = 0, g_sdFillStarts = 0, g_sdChildBad = 0;
static uint32_t g_sdOwnFlag = 0xFFFFFFFF;
static unsigned char g_sdTrapped[0x400];
static bool g_sdBuilt = false, g_sdStopped = false, g_sdRan = false;
static volatile uint64_t g_sdMakerResult = 0;
static SdEntry g_sdBase[41], g_sdExtra[15];
static uint16_t g_sdBaseTags[40], g_sdExtraTags[14];
static char g_sdNames[54][24];
static const volatile int* volatile g_sdNowhere = nullptr;
static void* g_sdReadOnly = nullptr;	// sdbadstructure: a page that cannot be written
static volatile int g_sdSink = 0;
static const void* volatile g_sdMakerTarget = nullptr;
static const void* volatile g_sdFillTarget = nullptr;
static size_t g_sdMakerRoom = 0, g_sdFillRoom = 0;
constexpr uint64_t kSdSecond = 0x2222222222222222ull, kSdThird = 0x3333333333333333ull, kSdFourth = 0x4444444444444444ull;
constexpr uint64_t kSdResult = 0x5D5D5D5D0000F00Dull;
constexpr DWORD kSdLastError = 0x5158;
constexpr size_t kSdAllocAt = 8;		// in the maker's carrier: the allocation, behind the jump
constexpr int kSdPoolRel32 = 8, kSdAllocRel32 = 18, kSdConstructRel32 = 29, kSdAllocationAt = 12;	// in the allocation: the pool's load's disp32, its two calls' rel32, I-E-Y2
constexpr int kSdMakerRel32 = 12, kSdNextRel32 = 20;		// in the caller: I-E-Y3's two calls' rel32
constexpr int kSdFillStartRel32 = 29, kSdFillJumpAt = 61;	// in the fill: its first call's rel32, the jump on to SdFill

// The tag the game knows a bone by, as the findings have it: letters folded to upper case, an ELF-style hash, modulo 0xFE8F, plus 0x170.
static uint16_t SdTag(const char* name)
{
	uint32_t hash = 0;
	for (; *name; name++)
	{
		hash = (hash << 4) + static_cast<unsigned char>(*name >= 'a' && *name <= 'z' ? *name - 'a' + 'A' : *name);
		const uint32_t top = hash & 0xF0000000u;
		hash = (hash ^ (top >> 24)) & ~top;
	}
	return static_cast<uint16_t>(hash % 0xFE8Fu + 0x170u);
}

// The game's allocator: it traps at a size of 0, a size above the stride or a full pool.
static unsigned char* SdAllocate(unsigned char* pool, uint32_t size)
{
	g_sdAllocation = size;
	if (size == 0 || size > g_sdStride || g_sdUsed >= g_sdElements)
	{
		g_sdTraps++;
		return g_sdTrapped;
	}
	return pool + static_cast<size_t>(g_sdStride) * g_sdUsed++;
}

// The bone structure's constructor: the set at +0x00, the table of bones and to +0x2A8 all 0xFF, then fields to 0x34C. What lies past that,
// a pool's own bytes, it leaves.
static void SdConstruct(unsigned char* structure)
{
	memset(structure, 0xFF, 0x10);
	memset(structure + 0x10, 0xFF, 0x298);
	for (size_t i = 0x2A8; i < 0x34C; i++)
		structure[i] = static_cast<unsigned char>(i);
}

// The game's fill: for each entry until a null name the bone by its tag and, if found, the low byte of its index at 0x10 + the entry's ID, unchecked.
// With a table outside the game it is TweaksV's call; a model that faults faults then, three of the table's bones in.
static void SdFill(const SdModel* model, const SdEntry* table, const uint16_t* tags, uint32_t flag)
{
	const bool own = !InImage(table);
	if (own)
	{
		g_sdOwnFills++;
		g_sdOwnFlag = flag;
	}
	unsigned char* const structure = static_cast<unsigned char*>(model->holding);
	int k = 0;
	for (; table[k].name; k++)
	{
		if (own && model->faults && k == 3)
			g_sdSink = *g_sdNowhere;
		for (int i = 0; i < model->named; i++)
			if (model->bones[i].tag == tags[k])
			{
				structure[0x10 + table[k].id] = static_cast<unsigned char>(model->bones[i].index);
				break;
			}
	}
	if (own)
		g_sdOwnEntries = k;
}
static void SdFillStart(const SdModel*)
{
	g_sdFillStarts++;
}

// The carriers: functions with frames of their own, so that the fake's unwind data names them, and room for their stand-ins.
static __declspec(noinline) void SdMakerCarrier()
{
	volatile int frame[16];
	for (int i = 0; i < 16; i++)
		frame[i] = g_sdSink + 7 * i;
	for (int i = 1; i < 16; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0x5DA7);
	g_sdSink = frame[3] + frame[8] + frame[13];
}
static __declspec(noinline) void SdFillCarrier()
{
	volatile int frame[16];
	for (int i = 0; i < 16; i++)
		frame[i] = g_sdSink - 9 * i;
	for (int i = 1; i < 16; i++)
		frame[i] = frame[i] + (frame[i - 1] ^ 0xF177);
	g_sdSink = frame[2] + frame[7] + frame[12];
}

// The function that makes a structure, behind its jump: the allocation and the constructor as code, the game's fills, what it made kept for
// the suite's comparison. It gives a result and a last error of its own, which the caller's site hands on and looks at.
static uint64_t SdMaker(SdModel* model, uint64_t second, uint64_t third, uint64_t fourth)
{
	g_sdMade++;
	if (second != kSdSecond || third != kSdThird || fourth != kSdFourth)
		g_sdArgsBad++;
	using Allocation = unsigned char* (*)();
	unsigned char* const structure = reinterpret_cast<Allocation>(SirenCountCode(&SdMakerCarrier) + kSdAllocAt)();
	model->holding = structure;
	SdFill(model, g_sdBase, g_sdBaseTags, 0);
	if (model->type == 8 || model->type == 1)
		SdFill(model, g_sdExtra, g_sdExtraTags, 1);
	memcpy(model->made, structure, sizeof(model->made));
	if (model->unwritable)
		model->holding = g_sdReadOnly;
	SetLastError(kSdLastError);
	return kSdResult;
}

// What the fragment answers about a bone: its child, here its index, which the flags' loops index their array by; the hit's and the
// impact's calls of it break the child.
static unsigned char g_sdAsked[128], g_sdHidden[128];
static uint32_t g_sdChildren[128];
static SdVehicle g_sdVehicle;
static uint64_t SdChild(const void* fragment, int bone)
{
	if (fragment != &g_sdVehicle || bone < 0 || bone >= 128)
	{
		g_sdChildBad++;
		return ~uint64_t{ 0 };
	}
	g_sdAsked[bone] = 1;
	return static_cast<uint64_t>(bone);
}

// A place of each pattern in its code: the label, where its bytes start and how many, and the byte it pins that "sdchanged" changes.
struct SdPlace
{
	const wchar_t* label;
	volatile unsigned char* code;
	int at, length, pinned;
};
// An operand TweaksV writes: what the game holds there and what it holds raised, raised + perSiren x N.
struct SdOperand
{
	volatile unsigned char* code;
	int at, size;
	int32_t raised, perSiren;
};
static unsigned char g_sdPlaced[10][0x800];
static int32_t g_sdAllocationPlaced = 0;
static const SdOperand g_sdOperands[] = {
	{ g_sdHit, 36, 4, 0x33C, 0 }, { g_sdHit, 0x38, 4, 0x33C, 1 }, { g_sdHit, 0x44, 4, -0x33C, 0 }, { g_sdHit, 0x7F, 1, 0x30, 0 },
	{ g_sdImpact, 0x18, 1, 0, 1 }, { g_sdImpact, 0x2E, 4, 0x34C, 0 }, { g_sdImpact, 0x5B, 4, 0x36C, 0 },
	{ g_sdHide, 0x13, 1, 0, 1 }, { g_sdHide, 0x2E, 4, 0x34C, 0 },
	{ g_sdWreck, 11, 1, 0, 1 }, { g_sdWreck, 0x1C, 4, 0x34C, 0 },
	{ g_sdFlags, 0x12, 1, 0, 1 }, { g_sdFlags, 0x27, 4, 0x36C, 0 },
	{ g_sdClasses[0], 16, 4, 0, -1 }, { g_sdClasses[1], 16, 4, 0, -1 }, { g_sdClasses[2], 16, 4, 0, -1 },
	{ g_sdClasses[0], 0x26, 4, 0x36C, 1 }, { g_sdClasses[1], 0x26, 4, 0x36C, 1 }, { g_sdClasses[2], 0x26, 4, 0x36C, 1 },
	{ g_sdPoolSite, 8, 4, 0x38C, 0 },
};
constexpr int kSdOperands = 20;	// and the allocation's, in its carrier
static_assert(sizeof(g_sdOperands) / sizeof(g_sdOperands[0]) == kSdOperands, "19 damage writes and the pool's");
static const struct { volatile unsigned char* code; size_t size; } g_sdCode[] = { { g_sdHit, sizeof(g_sdHit) }, { g_sdImpact, sizeof(g_sdImpact) }, { g_sdHide, sizeof(g_sdHide) },
	{ g_sdWreck, sizeof(g_sdWreck) }, { g_sdFlags, sizeof(g_sdFlags) }, { g_sdClasses[0], sizeof(g_sdClasses[0]) }, { g_sdClasses[1], sizeof(g_sdClasses[1]) },
	{ g_sdClasses[2], sizeof(g_sdClasses[2]) }, { g_sdPoolSite, sizeof(g_sdPoolSite) }, { g_sdSync, sizeof(g_sdSync) } };

static int32_t SdValueAt(const volatile unsigned char* at, int size)
{
	if (size == 1)
		return *at;
	uint32_t value = 0;
	for (int i = 0; i < 4; i++)
		value |= static_cast<uint32_t>(at[i]) << (8 * i);
	return static_cast<int32_t>(value);
}

// How the code stands against what was placed: as placed, or the operands raised to one N and nothing else.
static const wchar_t* SdSitesState()
{
	static wchar_t text[64];
	const volatile unsigned char* const allocation = SirenCountCode(&SdMakerCarrier) + kSdAllocAt + kSdAllocationAt;
	const int n = SdValueAt(g_sdHit + 0x38, 4) - 0x33C;
	bool placed = SdValueAt(allocation + 1, 4) == g_sdAllocationPlaced, raised = n > 20 && n <= 32 && SdValueAt(allocation + 1, 4) == 0x38C, other = false;
	for (size_t c = 0; c < sizeof(g_sdCode) / sizeof(g_sdCode[0]); c++)
		for (size_t i = 0; i < g_sdCode[c].size; i++)
		{
			const volatile unsigned char* const at = g_sdCode[c].code + i;
			bool operand = false;
			for (const SdOperand& o : g_sdOperands)
				operand = operand || (at >= o.code + o.at && at < o.code + o.at + o.size);
			if (*at != g_sdPlaced[c][i])
			{
				placed = false;
				other = other || !operand;
			}
		}
	for (const SdOperand& o : g_sdOperands)
		if (SdValueAt(o.code + o.at, o.size) != (o.size == 1 ? (o.raised + o.perSiren * n) & 0xFF : o.raised + o.perSiren * n))
			raised = false;
	if (placed)
		return L"as placed";
	if (!raised || other)
		return L"NOT AS WRITTEN";
	swprintf_s(text, L"raised to %d, nothing else", n);
	return text;
}

// Where the call of the maker leads now: the maker, TweaksV's call stub, another mod or another function of the game.
static const wchar_t* SdWhere()
{
	const unsigned char* const target = Rel32Target(Site{ g_sdCaller, kSdMakerRel32, 0 });
	if (target == SirenCountCode(&SdMakerCarrier))
		return L"the maker";
	if (InImage(target))
		return L"another function of the game";
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(target, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT && memcmp(target, "\xFF\x25\x00\x00\x00\x00", 6) == 0)
		return L"a TweaksV stub";
	return L"another mod";
}

static void SdPut(volatile unsigned char* code, size_t size, const char* hex)
{
	for (size_t i = 0; i < size; i++)
		code[i] = 0xCC;
	SirenIdsPut(code, 0, hex);
}

// Before TweaksV loads: every loop written, its calls pointed; the carriers rewritten; what the options change, then what stands copied for the
// comparison. Any option that changes a site stops the loops from running, each of which then has a site it does not know.
static void SdSetUp(int argc, wchar_t** argv)
{
	SdPut(g_sdHit, sizeof(g_sdHit), "53 55 57 56 41 54 41 56 41 57 48 83 EC 40 44 0F 11 44 24 20 44 0F 11 5C 24 30 48 89 CB 49 89 D7 44 89 C6 BF 89 00 00 00 F3 44 0F 10 05 00 00 00 00 45 "
		"0F 57 DB 48 81 FF 9D 00 00 00 0F 84 4F 00 00 00 8D AF 77 FF FF FF 41 BE 01 00 00 00 4C 89 F8 44 0F B6 64 38 10 41 80 FC FF 74 2F 41 39 F4 75 2A 89 E9 "
		"41 D3 E6 44 09 B3 60 08 00 00 C6 83 65 08 00 00 01 48 89 D8 4C 89 F9 0F BE 54 39 24 48 8B 88 C0 09 00 00 E8 00 00 00 00 48 FF C7 EB A4 44 0F 10 44 24 "
		"20 44 0F 10 5C 24 30 48 83 C4 40 41 5F 41 5E 41 5C 5E 5F 5D 5B C3");
	SdPut(g_sdImpact, sizeof(g_sdImpact), "53 55 57 56 41 54 48 83 EC 20 49 89 CC 48 89 D7 44 89 C6 31 ED 48 83 FD 14 0F 84 51 00 00 00 49 8B 84 24 08 04 00 00 48 89 C1 0F BE 94 29 99 00 00 00 "
		"BB 01 00 00 00 39 F2 75 30 89 E9 D3 E3 09 9F 40 04 00 00 C6 87 45 04 00 00 01 4C 89 E0 49 8B 8C 24 08 04 00 00 0F BE 94 29 AD 00 00 00 48 8B 88 C0 09 "
		"00 00 E8 00 00 00 00 48 FF C5 EB A5 48 83 C4 20 41 5C 5E 5F 5D 5B C3");
	SdPut(g_sdHide, sizeof(g_sdHide), "53 55 56 41 54 48 89 CE 49 89 D1 4D 89 C4 31 DB 48 83 FB 14 74 2B 89 D8 C1 E8 05 8B 84 86 40 04 00 00 0F A3 D8 73 15 4C 89 C8 0F BE AC 18 99 00 00 00 "
		"83 FD FF 74 05 41 C6 04 2C 01 48 FF C3 EB CF 41 5C 5E 5D 5B C3");
	SdPut(g_sdWreck, sizeof(g_sdWreck), "55 41 57 49 89 CF 31 C9 48 83 F9 14 74 2A 49 8B 87 08 04 00 00 48 89 C5 0F BE 94 0D 99 00 00 00 BD 01 00 00 00 83 FA FF 74 09 D3 E5 41 09 AF 60 08 00 "
		"00 48 FF C1 EB D0 41 5F 5D C3");
	SdPut(g_sdFlags, sizeof(g_sdFlags), "53 57 56 48 83 EC 20 48 89 CE 48 89 CF 31 DB 48 83 FB 14 74 3F 48 8B 46 20 48 8B 80 B0 00 00 00 48 8B 00 0F BE 94 18 AD 00 00 00 83 FA FF 74 1F 48 8B "
		"8E C0 09 00 00 E8 00 00 00 00 83 F8 FF 74 0E 48 8B 8F 90 00 00 00 C7 04 81 E0 FF FF 17 48 FF C3 EB BB 48 83 C4 20 5E 5F 5B C3");
	for (auto& code : g_sdClasses)
	{
		SdPut(code, sizeof(code), "53 57 56 48 83 EC 20 48 89 CE 48 89 CF 48 C7 C3 EC FF FF FF 48 8B 46 20 48 8B 80 B0 00 00 00 48 8B 00 0F BE 94 18 C1 00 00 00 83 FA FF 74 1F 48 8B 8E "
			"C0 09 00 00 E8 00 00 00 00 83 F8 FF 74 0E 48 8B 8F 90 00 00 00 C7 04 81 E0 FF FF 17 48 FF C3 75 C1 48 83 C4 20 5E 5F 5B C3");
		SetRel32({ code, 55, 0 }, reinterpret_cast<const void*>(&SdChild));
	}
	SetRel32({ g_sdHit, 136, 0 }, reinterpret_cast<const void*>(&SdChild));
	SetRel32({ g_sdImpact, 103, 0 }, reinterpret_cast<const void*>(&SdChild));
	SetRel32({ g_sdFlags, 56, 0 }, reinterpret_cast<const void*>(&SdChild));
	SdPut(g_sdPoolSite, sizeof(g_sdPoolSite), "48 83 EC 28 C7 44 24 20 4C 03 00 00 4C 8D 05 00 00 00 00 8B 44 24 20 48 83 C4 28 C3");
	SdPut(g_sdCaller, sizeof(g_sdCaller), "56 48 83 EC 20 48 89 CE 48 89 F1 E8 00 00 00 00 48 89 F1 E8 00 00 00 00 F6 86 4B 05 00 00 10 48 83 C4 20 5E C3");
	SdPut(g_sdKept, sizeof(g_sdKept), "48 89 05 00 00 00 00 C3");
	SetRel32({ g_sdKept, 3, 0 }, const_cast<const uint64_t*>(&g_sdMakerResult));
	SetRel32({ g_sdCaller, kSdNextRel32, 0 }, const_cast<const unsigned char*>(g_sdKept));
	// the stored damage state's loop and read, which stay at 20: TweaksV looks for neither
	SdPut(g_sdSync, sizeof(g_sdSync), "48 83 FD 14 0F 84 00 00 00 00 41 0F B6 84 2D F4 00 00 00 CC 0F BE 9C 28 AD 00 00 00 48 8B 06 48 89 F1");
	for (size_t i = 0; i < sizeof(g_sdSpare); i++)
		g_sdSpare[i] = 0xCC;

	// the carriers
	unsigned char* const maker = SirenCountCode(&SdMakerCarrier);
	unsigned char* const fill = SirenCountCode(&SdFillCarrier);
	constexpr size_t kAllocationSize = 42, kFillSize = 67, kNotStartAt = 16;
	g_sdMakerRoom = SirenCountRoom(maker, kSdAllocAt + kAllocationSize + 1);
	g_sdFillRoom = SirenCountRoom(fill, kNotStartAt + kFillSize + 1);
	g_sdBuilt = g_sdMakerRoom != 0 && g_sdFillRoom != 0;
	if (!g_sdBuilt)
		return;
	static const char kFillCode[] = "41 57 41 56 41 55 41 54 56 57 55 53 48 83 EC 28 45 89 CE 4D 89 C5 48 89 D3 49 89 CF E8 00 00 00 00 4C 89 F9 48 89 DA 4D 89 E8 45 89 F1 48 83 C4 28 5B "
		"5D 5F 5E 41 5C 41 5D 41 5E 41 5F";
	static const char kAllocationCode[] = "57 48 83 EC 20 48 8B 0D 00 00 00 00 BA 4C 03 00 00 E8 00 00 00 00 48 89 C7 48 89 C1 E8 00 00 00 00 48 89 F8 48 83 C4 20 5F C3";
	const bool allocationOut = HasOption(argc, argv, L"sdallocout"), notStart = HasOption(argc, argv, L"sdnotstart"), noUnwind = HasOption(argc, argv, L"sdnounwind");
	g_sdMakerTarget = reinterpret_cast<const void*>(&SdMaker);
	g_sdFillTarget = reinterpret_cast<const void*>(&SdFill);
	DWORD makerOld = 0, fillOld = 0, unused = 0;
	VirtualProtect(maker, g_sdMakerRoom, PAGE_EXECUTE_READWRITE, &makerOld);
	VirtualProtect(fill, g_sdFillRoom, PAGE_EXECUTE_READWRITE, &fillOld);
	memset(maker, 0xCC, g_sdMakerRoom);
	memset(fill, 0xCC, g_sdFillRoom);
	SirenCountJump(maker, &g_sdMakerTarget);
	if (allocationOut)
		SirenIdsPut(g_sdSpare, 0x600, kAllocationCode);		// the allocation's pattern where no unwind data names a function
	else
	{
		SirenIdsPut(maker, kSdAllocAt, kAllocationCode);
		SetRel32({ maker + kSdAllocAt, kSdPoolRel32, 0 }, HasOption(argc, argv, L"sdpoolout") ? static_cast<const void*>(ImageEnd() + 0x78000) :
			static_cast<const void*>(const_cast<unsigned char* const*>(&g_sdPool)));
		if (HasOption(argc, argv, L"sdpoolshape"))
			maker[kSdAllocAt + kSdPoolRel32 - 2] = 0x8D;	// a lea, not the load: another shape; no loop runs then
		SetRel32({ maker + kSdAllocAt, kSdAllocRel32, 0 }, reinterpret_cast<const void*>(&SdAllocate));
		SetRel32({ maker + kSdAllocAt, kSdConstructRel32, 0 }, reinterpret_cast<const void*>(&SdConstruct));
	}
	if (noUnwind)
		SirenIdsPut(g_sdSpare, 0x680, kFillCode);			// the fill's pattern where no unwind data names a function
	else
	{
		unsigned char* const code = fill + (notStart ? kNotStartAt : 0);
		SirenIdsPut(code, 0, kFillCode);
		SetRel32({ code, kSdFillStartRel32, 0 }, reinterpret_cast<const void*>(&SdFillStart));
		SirenCountJump(code + kSdFillJumpAt, &g_sdFillTarget);
	}
	SetRel32({ g_sdCaller, kSdMakerRel32, 0 }, HasOption(argc, argv, L"sdhooked") ? static_cast<const void*>(ImageEnd() + 0x76000) :
		HasOption(argc, argv, L"sdelsewhere") ? static_cast<const void*>(fill) : static_cast<const void*>(maker));
	g_sdStopped = allocationOut || notStart || noUnwind || HasOption(argc, argv, L"sdhooked") || HasOption(argc, argv, L"sdelsewhere") ||
		HasOption(argc, argv, L"sdpoolshape") || HasOption(argc, argv, L"sdpoolmade") || HasOption(argc, argv, L"sdpoolout");
	if (HasOption(argc, argv, L"sdpoolmade"))
		g_sdPool = g_sdTrapped;		// the pool made before TweaksV starts

	// sdchanged:LABEL, sdfar:LABEL, sdfourth:LABEL
	const SdPlace places[] = {
		{ L"I-E-X1", g_sdHit, 35, 18, 1 }, { L"I-E-X2", g_sdHit, 0x35, 25, 3 }, { L"I-E-X3", g_sdHit, 0x7B, 13, 4 }, { L"I-E-X4", g_sdHit, 0x51, 10, 5 },
		{ L"I-E-L05", g_sdImpact, 0x15, 18, 3 }, { L"I-E-N04", g_sdImpact, 0x2A, 13, 4 }, { L"I-E-N05", g_sdImpact, 0x57, 15, 4 },
		{ L"I-E-L03", g_sdHide, 0x10, 18, 3 }, { L"I-E-N03", g_sdHide, 0x2A, 13, 4 },
		{ L"I-E-L04", g_sdWreck, 8, 13, 3 }, { L"I-E-N02", g_sdWreck, 0x18, 13, 4 },
		{ L"I-E-L07", g_sdFlags, 0xF, 17, 3 }, { L"I-E-N06", g_sdFlags, 0x23, 37, 4 },
		{ L"I-E-L08", g_sdClasses[0], 13, 7, 3 }, { L"I-E-N09", g_sdClasses[0], 0x22, 12, 4 },
		{ L"I-E-Y1", g_sdPoolSite, 4, 11, 4 }, { L"I-E-Y2", maker + kSdAllocAt, kSdAllocationAt, 17, 1 }, { L"I-E-Y3", g_sdCaller, 8, 23, 0 }, { L"I-E-Y4", fill, 0, 29, 0 } };
	for (int i = 1; i < argc; i++)
		for (const SdPlace& place : places)
		{
			const volatile unsigned char* const bytes = place.code + place.at;
			if (wcsncmp(argv[i], L"sdchanged:", 10) == 0 && wcscmp(argv[i] + 10, place.label) == 0)
			{
				place.code[place.at + place.pinned] ^= 0x01;
				g_sdStopped = true;
			}
			if (wcsncmp(argv[i], L"sdfar:", 6) == 0 && wcscmp(argv[i] + 6, place.label) == 0)
			{
				for (int k = 0; k < place.length; k++)
					g_sdSpare[0x400 + k] = bytes[k];
				place.code[place.at] ^= 0x01;
				g_sdStopped = true;
			}
			if (wcsncmp(argv[i], L"sdfourth:", 9) == 0 && wcscmp(argv[i] + 9, place.label) == 0)
			{
				for (int k = 0; k < place.length; k++)
					g_sdSpare[0x100 + k] = bytes[k];
				g_sdStopped = true;
			}
		}
	VirtualProtect(fill, g_sdFillRoom, fillOld, &unused);		// the carriers may share a page: the later's old protection first
	VirtualProtect(maker, g_sdMakerRoom, makerOld, &unused);
	FlushInstructionCache(GetCurrentProcess(), maker, g_sdMakerRoom);
	FlushInstructionCache(GetCurrentProcess(), fill, g_sdFillRoom);

	for (size_t c = 0; c < sizeof(g_sdCode) / sizeof(g_sdCode[0]); c++)
		for (size_t i = 0; i < g_sdCode[c].size; i++)
			g_sdPlaced[c][i] = g_sdCode[c].code[i];
	g_sdAllocationPlaced = SdValueAt(maker + kSdAllocAt + kSdAllocationAt + 1, 4);
}

// A model: its name's hash, its type, and bones by name.
static SdModel& SdNew(int which, uint32_t hash, uint32_t type)
{
	SdModel& model = g_sdModels[which];
	model.hash = hash;
	model.type = type;
	model.holder = &model.holding;
	return model;
}
static void SdAdd(SdModel& model, const char* format, int first, int last, int firstBone)
{
	for (int n = first; n <= last && model.named < kSdBones; n++)
	{
		SdBone& bone = model.bones[model.named++];
		snprintf(bone.name, sizeof(bone.name), format, n);
		bone.index = static_cast<uint16_t>(firstBone + n - first);
		bone.tag = SdTag(bone.name);
	}
}

// The index of a model's bone "<prefix><n>", in any case, or -1.
static int SdBoneOf(const SdModel& model, const char* prefix, int n)
{
	char name[24];
	snprintf(name, sizeof(name), "%s%d", prefix, n);
	for (int i = 0; i < model.named; i++)
		if (_stricmp(model.bones[i].name, name) == 0)
			return model.bones[i].index;
	return -1;
}

// What a slot of the room must hold: the game's byte for siren 1 to 20 or their glass, a plane's gear doors among them; past them a bone by name
// that a signed byte can name, or -1.
static int SdSlot(const SdModel& model, int kind, int n)
{
	if (n <= 20)
		return static_cast<signed char>(model.made[(kind ? 0xAD : 0x99) + n - 1]);
	if (model.faults)
		return -1;	// whatever the game's fill had written when it faulted
	const int bone = SdBoneOf(model, kind ? "siren_glass" : "siren", n);
	return bone >= 0 && bone < 128 ? bone : -1;
}
static bool SdRoomRight(const SdModel& model)
{
	const unsigned char* const structure = static_cast<const unsigned char*>(model.holding);
	for (int kind = 0; kind < 2; kind++)
		for (int n = 1; n <= 32; n++)
			if (static_cast<signed char>(structure[(kind ? 0x36C : 0x34C) + n - 1]) != SdSlot(model, kind, n))
				return false;
	return true;
}

// The structure's own bytes as the game made them, but for the bits of the bones past siren20 in its set.
static bool SdStructureRight(const SdModel& model)
{
	unsigned char want[sizeof(model.made)];
	memcpy(want, model.made, sizeof(want));
	for (int kind = 0; kind < 2; kind++)
		for (int n = 21; n <= 32; n++)
		{
			const int bone = SdSlot(model, kind, n);
			if (bone >= 0)
				want[bone / 8] &= static_cast<unsigned char>(~(1u << (bone % 8)));
		}
	return memcmp(want, model.holding, sizeof(want)) == 0;
}

// The game's load, once TweaksV has started: the pool set up by its site's stride, then each model made through the caller's site, with
// arguments, a result and a last error to see handed on.
static void SdMake(SdModel& model)
{
	SetLastError(0);
	reinterpret_cast<void (*)(SdModel*, uint64_t, uint64_t, uint64_t)>(const_cast<unsigned char*>(g_sdCaller))(&model, kSdSecond, kSdThird, kSdFourth);
	const DWORD error = GetLastError();
	if (error == kSdLastError && g_sdMakerResult == kSdResult)
		g_sdKeptBoth++;
}
static void SdActions(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"sdrun") || !g_sdBuilt || g_sdStopped)
		return;
	g_sdRan = true;
	g_sdStride = reinterpret_cast<uint32_t (*)()>(const_cast<unsigned char*>(g_sdPoolSite))();
	g_sdElements = 16;
	unsigned char* const pool = static_cast<unsigned char*>(VirtualAlloc(nullptr, static_cast<size_t>(g_sdElements) * g_sdStride, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	memset(pool, 0xAB, static_cast<size_t>(g_sdElements) * g_sdStride);
	g_sdPool = pool;
	for (int n = 1; n <= 20; n++)
	{
		snprintf(g_sdNames[n - 1], sizeof(g_sdNames[0]), "siren%d", n);
		snprintf(g_sdNames[19 + n], sizeof(g_sdNames[0]), "siren_glass%d", n);
		g_sdBase[n - 1] = { g_sdNames[n - 1], 0x88 + n };
		g_sdBase[19 + n] = { g_sdNames[19 + n], 0x9C + n };
		g_sdBaseTags[n - 1] = SdTag(g_sdNames[n - 1]);
		g_sdBaseTags[19 + n] = SdTag(g_sdNames[19 + n]);
	}
	for (int n = 1; n <= 14; n++)
	{
		snprintf(g_sdNames[39 + n], sizeof(g_sdNames[0]), "gear_door_%d", n);
		g_sdExtra[n - 1] = { g_sdNames[39 + n], 0x9C + n };
		g_sdExtraTags[n - 1] = SdTag(g_sdNames[39 + n]);
	}

	SdModel& police = SdNew(kSdPolice, 0x79FBB0C5, 0);
	SdAdd(police, "siren%d", 1, 20, 40);
	SdAdd(police, "siren_glass%d", 1, 20, 60);
	SdModel& full = SdNew(kSdFull, 0xA1B2C3D4, 0);				// past siren20 in other cases: the game's lookup ignores case
	SdAdd(full, "siren%d", 1, 20, 10);
	SdAdd(full, "SIREN%d", 21, 32, 30);
	SdAdd(full, "siren_glass%d", 1, 20, 42);
	SdAdd(full, "Siren_Glass%d", 21, 32, 62);
	SdModel& some = SdNew(kSdSome, 0x24242424, 0);
	SdAdd(some, "siren%d", 1, 24, 10);
	SdAdd(some, "siren_glass%d", 1, 24, 50);
	SdModel& sparse = SdNew(kSdSparse, 0x0DD50A25, 0);
	SdAdd(sparse, "siren%d", 1, 4, 10);
	SdAdd(sparse, "siren%d", 21, 21, 14);
	SdAdd(sparse, "siren%d", 25, 25, 15);
	SdAdd(sparse, "siren_glass%d", 1, 2, 16);
	SdAdd(sparse, "siren_glass%d", 25, 25, 18);
	SdModel& plane = SdNew(kSdPlane, 0xA1A1A1A1, 8);			// its first glass slots gear doors, by the third table
	SdAdd(plane, "siren%d", 1, 2, 5);
	SdAdd(plane, "siren_glass%d", 1, 1, 40);
	SdAdd(plane, "gear_door_%d", 1, 14, 20);
	SdAdd(plane, "siren%d", 21, 21, 34);
	SdAdd(plane, "siren_glass%d", 21, 21, 35);
	SdModel& lost = SdNew(kSdLost, 0x0C0FFEE0, 0);				// bones past what a signed byte names: siren5 as the game has it, siren21 and its glass none
	SdAdd(lost, "siren%d", 5, 5, 130);
	SdAdd(lost, "siren%d", 21, 21, 129);
	SdAdd(lost, "siren%d", 22, 22, 127);
	SdAdd(lost, "siren_glass%d", 21, 21, 131);
	SdModel& few = SdNew(kSdFew, 0x2C3D4E5F, 0);
	SdAdd(few, "siren%d", 1, 8, 20);
	SdAdd(few, "siren%d", 21, 21, 140);							// its one bone past siren20 past what a signed byte names
	for (int i = 0; i < kSdDrawn; i++)
		SdMake(g_sdModels[i]);
	if (HasOption(argc, argv, L"sdnoholder") || HasOption(argc, argv, L"sdbadholder"))
	{
		SdModel& model = SdNew(kSdNoHolder, 0x0BAD0001, 0);
		SdAdd(model, "siren%d", 1, 24, 10);
		model.holder = HasOption(argc, argv, L"sdnoholder") ? nullptr : reinterpret_cast<void* const*>(ImageEnd() + 0x77000);
		SdMake(model);
	}
	if (HasOption(argc, argv, L"sdfillfault"))
	{
		SdModel& model = SdNew(kSdFault, 0x0BAD0003, 0);
		SdAdd(model, "siren%d", 1, 24, 10);
		model.faults = true;
		SdMake(model);
	}
	if (HasOption(argc, argv, L"sdbadstructure"))
	{
		SdModel& model = SdNew(kSdBadStructure, 0x0BAD0004, 0);
		SdAdd(model, "siren%d", 1, 24, 10);
		model.unwritable = true;
		g_sdReadOnly = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY);
		SdMake(model);
	}
	if (HasOption(argc, argv, L"sdmany"))
	{
		SdModel& model = SdNew(kSdManyModel, 0, 0);
		SdAdd(model, "siren%d", 1, 24, 10);
		for (uint32_t i = 0; i < 20001; i++)
		{
			model.hash = 0x5D000000u + i;
			g_sdUsed = g_sdElements - 1;
			SdMake(model);
		}
	}
}

static void SdFresh(const SdModel& model)
{
	memset(&g_sdVehicle, 0, sizeof(g_sdVehicle));
	memset(g_sdAsked, 0, sizeof(g_sdAsked));
	memset(g_sdHidden, 0, sizeof(g_sdHidden));
	memset(g_sdChildren, 0, sizeof(g_sdChildren));
	g_sdVehicle.model = &model;
	g_sdVehicle.structure = model.holding;
	g_sdVehicle.children = g_sdChildren;
	g_sdVehicle.fragment = &g_sdVehicle;
}

// The slots of a kind whose bone a byte per bone marks, siren1's bit 0.
static uint32_t SdMarkedSlots(const SdModel& model, int kind, const unsigned char* marks)
{
	uint32_t slots = 0;
	for (int n = 1; n <= 32; n++)
	{
		const int bone = SdSlot(model, kind, n);
		if (bone >= 0 && bone < 128 && marks[bone])
			slots |= 1u << (n - 1);
	}
	return slots;
}

// The damage, each loop run as code: a hit and an impact on a siren of the model made for 32, a wreck on each model, the flags' four loops on three of them.
static void SdDamageReport()
{
	const SdModel& full = g_sdModels[kSdFull];
	const int sirens[] = { 5, 20, 21, 25, 32 };
	wprintf(L"sd hit:");
	for (const int n : sirens)
	{
		SdFresh(full);
		reinterpret_cast<void (*)(SdVehicle*, const void*, int)>(const_cast<unsigned char*>(g_sdHit))(&g_sdVehicle, full.holding, SdBoneOf(full, "siren", n));
		wprintf(L" %d 0x%08X %u 0x%08X,", n, g_sdVehicle.broken, g_sdVehicle.brokenByte, SdMarkedSlots(full, 1, g_sdAsked));
	}
	wprintf(L" | sd impact:");
	for (const int n : sirens)
	{
		SdFresh(full);
		reinterpret_cast<void (*)(SdVehicle*, SdVehicle*, int)>(const_cast<unsigned char*>(g_sdImpact))(&g_sdVehicle, &g_sdVehicle, SdBoneOf(full, "siren", n));
		reinterpret_cast<void (*)(SdVehicle*, const void*, unsigned char*)>(const_cast<unsigned char*>(g_sdHide))(&g_sdVehicle, full.holding, g_sdHidden);
		wprintf(L" %d 0x%08X %u 0x%08X hidden 0x%08X,", n, g_sdVehicle.marked, g_sdVehicle.markedByte, SdMarkedSlots(full, 1, g_sdAsked), SdMarkedSlots(full, 0, g_sdHidden));
	}
	wprintf(L" | sd wreck:");
	for (int i = 0; i < kSdDrawn; i++)
	{
		SdFresh(g_sdModels[i]);
		reinterpret_cast<void (*)(SdVehicle*)>(const_cast<unsigned char*>(g_sdWreck))(&g_sdVehicle);
		wprintf(L" 0x%08X", g_sdVehicle.broken);
	}
	wprintf(L" | sd flags:");
	const int flagged[] = { kSdPolice, kSdFull, kSdPlane, kSdLost };
	for (const int which : flagged)
	{
		const SdModel& model = g_sdModels[which];
		volatile unsigned char* const loops[] = { g_sdFlags, g_sdClasses[0], g_sdClasses[1], g_sdClasses[2] };
		for (volatile unsigned char* const loop : loops)
		{
			SdFresh(model);
			reinterpret_cast<void (*)(SdVehicle*)>(const_cast<unsigned char*>(loop))(&g_sdVehicle);
			unsigned char set[128] = {};
			int others = 0;
			for (int bone = 0; bone < 128; bone++)
			{
				set[bone] = g_sdChildren[bone] == 0x17FFFFE0 ? 1 : 0;
				others += g_sdChildren[bone] != 0 && !set[bone];
			}
			wprintf(L" 0x%08X%ls", SdMarkedSlots(model, 1, set), others ? L" WRONG FLAGS" : L"");
		}
		wprintf(L",");
	}
	wprintf(L" | ");
}

static void SdReport()
{
	wprintf(L"sd sites: %ls, the maker's call leads to %ls | ", SdSitesState(), SdWhere());
	if (!g_sdRan)
		return;
	int rooms = 0, structures = 0;
	for (int i = 0; i < kSdDrawn; i++)
	{
		rooms += SdRoomRight(g_sdModels[i]);
		structures += SdStructureRight(g_sdModels[i]);
	}
	wprintf(L"sd pool: stride 0x%X, allocation 0x%X, %d traps | ", g_sdStride, g_sdAllocation, g_sdTraps);
	wprintf(L"sd models: %d made, %d with bad arguments, result and last error kept %d, %d through its code, rooms right %d of %d, their structures right %d | ",
		g_sdMade, g_sdArgsBad, g_sdKeptBoth, g_sdFillStarts, rooms, kSdDrawn, structures);
	wprintf(L"sd fill: TweaksV's %d times, flag %u, %d entries | ", g_sdOwnFills, g_sdOwnFlag, g_sdOwnEntries);
	if (g_sdModels[kSdFault].faults)
		wprintf(L"sd fault: room %ls, structure %ls | ", SdRoomRight(g_sdModels[kSdFault]) ? L"right" : L"WRONG", SdStructureRight(g_sdModels[kSdFault]) ? L"right" : L"WRONG");
	if (g_sdModels[kSdNoHolder].hash)
		wprintf(L"sd noholder: room byte 0x%02X | ", static_cast<const unsigned char*>(g_sdModels[kSdNoHolder].holding)[0x34C]);
	SdDamageReport();
	wprintf(L"sd children: %d asked wrongly | ", g_sdChildBad);		// after the loops, the only code that asks
}

// sfjoin, the two halves of the raise on one model: its bone structure the pool's element TweaksV's wrapper filled at the load, the room the second call reads, and the
// bits the game's own loops set on it, which the wrapper's shift hands to that call. What each call lit: at first, after a hit on siren25, after a wreck.
static LONG g_sirenJoinLit[3][2] = {};
static bool g_sirenJoined = false;
static void SirenJoinActions(int argc, wchar_t** argv)
{
	if (!HasOption(argc, argv, L"sfjoin") || !g_sdRan || !g_sirenRaiseOn)
		return;
	const SdModel& full = g_sdModels[kSdFull];
	SirenCountVehicle& v = g_sirenCountVehicles[kSirenCountJoined];
	memcpy(&v.structure, full.holding, sizeof(v.structure));	// what the renderer compares the block with
	v.holder = full.holding;
	SirenRaiseMerge(kSirenCountJoined, SirenParse(32, 14, 14, -1));
	g_sirenRaisePastSeed[kSirenCountJoined] = 14;
	SirenRaiseGiveState(v, 9000);
	const auto draw = [&v](int at)
	{
		SirenCountDraw(g_sirenRaiseLights, &v.car, &v.model, &v.skeleton);
		g_sirenJoinLit[at][0] = g_sirenRaiseLit[kSirenCountJoined][0];
		g_sirenJoinLit[at][1] = g_sirenRaiseLit[kSirenCountJoined][1];
		SdFresh(g_sdModels[kSdFull]);
	};
	draw(0);
	reinterpret_cast<void (*)(SdVehicle*, const void*, int)>(const_cast<unsigned char*>(g_sdHit))(&g_sdVehicle, full.holding, SdBoneOf(full, "siren", 25));
	memcpy(v.car.bytes + 0x860, &g_sdVehicle.broken, sizeof(g_sdVehicle.broken));
	draw(1);
	reinterpret_cast<void (*)(SdVehicle*)>(const_cast<unsigned char*>(g_sdWreck))(&g_sdVehicle);
	memcpy(v.car.bytes + 0x860, &g_sdVehicle.broken, sizeof(g_sdVehicle.broken));
	draw(2);
	g_sirenJoined = true;
}

static void SirenJoinReport()
{
	if (g_sirenJoined)
		wprintf(L"siren join: %ld+%ld, after a hit on siren25 %ld+%ld, after a wreck %ld+%ld | ", g_sirenJoinLit[0][0], g_sirenJoinLit[0][1], g_sirenJoinLit[1][0], g_sirenJoinLit[1][1],
			g_sirenJoinLit[2][0], g_sirenJoinLit[2][1]);
}
// [corona store]
// The corona store's data: what the job's parameters are given and what the drawing side's sites name, the store and its count being the lighting capacity's.
struct CcData
{
	const void* parameterStore;
	const void* parameterCount;
	uint64_t renderPointer;
	uint32_t renderCount, extras, mark;
};
static CcData g_ccDataStorage;
static CcData* volatile g_ccData = &g_ccDataStorage;

// What the report compares: the sites' bytes as placed, the two checks in address order, and each jump's target as placed.
static Site g_ccAllSites[sizeof(g_ccSites) / sizeof(g_ccSites[0]) + 3];
constexpr size_t kCcAllSites = sizeof(g_ccAllSites) / sizeof(g_ccAllSites[0]), kCcJumps = 4;
static unsigned char g_ccPlaced[kCcAllSites][128];
static unsigned char g_ccBatchSix[6];
static volatile unsigned char* g_ccFirstCheck = nullptr;
static volatile unsigned char* g_ccSecondCheck = nullptr;
static Site g_ccJumpRels[kCcJumps];
static const unsigned char* g_ccJumpTargets[kCcJumps];
static size_t g_ccCopied = 0;
static const void* g_ccCopiedFrom = nullptr;

// What the copy's call leads to: the game's memcpy, counted.
static void* CcCopyBytes(void* target, const void* source, size_t bytes)
{
	g_ccCopied = bytes;
	g_ccCopiedFrom = source;
	return memcpy(target, source, bytes);
}

// Before TweaksV loads: every rel32 as the game has it, but where an option asks otherwise; the sites' bytes as placed. It runs after LcSetUp, whose copy of the two checks
// takes their jumps anew.
static void CcSetUp(int argc, wchar_t** argv)
{
	LcData& data = *g_lcData;
	CcData& cc = *g_ccData;
	const bool ordered = reinterpret_cast<uintptr_t>(g_lcCoronaCheck) < reinterpret_cast<uintptr_t>(g_lcCoronaCheckToo);
	g_ccFirstCheck = ordered ? g_lcCoronaCheck : g_lcCoronaCheckToo;
	g_ccSecondCheck = ordered ? g_lcCoronaCheckToo : g_lcCoronaCheck;
	const Site firstJump = { g_ccFirstCheck, 15, sizeof(g_lcCoronaCheck) }, secondJump = { g_ccSecondCheck, 15, sizeof(g_lcCoronaCheck) };
	const auto site = [](const volatile unsigned char* bytes, int at) { return static_cast<const void*>(const_cast<const unsigned char*>(bytes) + at); };
	const LcRef refs[] = {
		{ L"i2store", SITE(g_ccFirstStore, 11), data.gameCoronas, 0 },
		{ L"i3store", SITE(g_ccSecondStore, 11), data.gameCoronas, 0 },
		{ L"i4count", SITE(g_ccCopy, kCcI4 + 2), &data.coronas, 0 },
		{ L"i5store", SITE(g_ccCopy, kCcI5 + 8), data.gameCoronas, 0 },
		{ L"i6store", SITE(g_ccParameters, kCcI6 + 3), data.gameCoronas, 0 },
		{ L"i6call", SITE(g_ccParameters, kCcI6 + 18), site(g_ccHandOut, 0), 0 },
		{ L"j2pointer", SITE(g_ccCommand, 7), &cc.renderPointer, 0 },
		{ L"j2count", SITE(g_ccCommand, 16), &cc.renderCount, 0 },
		{ L"j2extras", SITE(g_ccCommand, 33), &cc.extras, 0 },
		{ L"j3mark", SITE(g_ccDraw, 3), &cc.mark, 0 },
		{ L"j3count", SITE(g_ccDraw, 15), &cc.renderCount, 0 },
		{ L"i1jump", SITE(g_ccGeneralAdd, 30), site(g_ccGeneralAdd, kCcAddWayOut), 0 },
		{ L"g1jump1", firstJump, site(g_ccFirstWayOut, 0), 0 },
		{ L"g1jump2", secondJump, site(g_ccSecondWayOut, 0), 0 },
		{ L"i9jump", SITE(g_ccLodLimit, kCcI9 + 13), site(g_ccLodLimit, kCcLodWayOut), 0 },
	};
	for (const LcRef& ref : refs)
		SetRel32(ref.site, LcTarget(argc, argv, ref));
	SetRel32(SITE(g_ccAddStart, 22), site(g_ccGeneralAdd, 0));
	SetRel32(SITE(g_ccCopy, kCcI5 + 19), reinterpret_cast<const void*>(&CcCopyBytes));
	SetRel32(SITE(g_ccParameters, 34), &cc.parameterStore);
	SetRel32(SITE(g_ccParameters, 46), &cc.parameterCount);

	// a rel32 on the page rtjumpunread takes away: its match copied to end the first page of g_rtEdge, every rel32 of the copy set, the site's first byte a NOP
	const struct { const wchar_t* name; volatile unsigned char* site; size_t match; } edges[] = {
		{ L"j2extras", g_ccCommand, 33 }, { L"j3count", g_ccDraw, 15 }, { L"g1jump", g_ccFirstCheck, 15 } };
	for (const auto& edge : edges)
		if (LcOption(argc, argv, L"ccedge", edge.name))
		{
			const size_t at = 0x1000 - edge.match;
			for (size_t i = 0; i < edge.match; i++)
				g_rtEdge[at + i] = edge.site[i];
			for (const LcRef& ref : refs)
				if (ref.site.bytes == edge.site && static_cast<size_t>(ref.site.offset) < edge.match)
					SetRel32({ g_rtEdge, static_cast<int>(at) + ref.site.offset, sizeof(g_rtEdge) }, LcTarget(argc, argv, ref));
			if (edge.site == g_ccFirstCheck)	// the copy and the other check in address order anew, the other's jump leading where its place wants it
			{
				SetRel32({ g_rtEdge, static_cast<int>(at) + 2, sizeof(g_rtEdge) }, &data.coronas);
				const bool copyFirst = reinterpret_cast<uintptr_t>(g_rtEdge + at) < reinterpret_cast<uintptr_t>(g_ccSecondCheck);
				SetRel32(secondJump, site(copyFirst ? g_ccSecondWayOut : g_ccFirstWayOut, 0));
			}
			edge.site[0] = 0x90;
		}

	const struct { const wchar_t* name; volatile unsigned char* at; unsigned char value; } modded[] = {
		{ L"i1nop", g_ccGeneralAdd, 0xE9 }, { L"i4past", g_ccCopy + kCcI4 + 21, 0x02 }, { L"i7count", g_ccHandOut + 7, 0xCD }, { L"i8move", g_ccBatch + kCcI8 + 2, 0x08 },
		{ L"i8clamp", g_ccBatch + kCcI8 + 42, 0x08 }, { L"i9limit", g_ccLodLimit + kCcI9 + 8, 0x08 }, { L"j1scratch", g_ccScratch + 4, 0xCD } };
	for (const auto& one : modded)
		if (LcOption(argc, argv, L"ccmodded", one.name))
			*one.at = one.value;

	// the jumps of the two checks are the lighting capacity's sites too, set here after its copy of them
	for (size_t i = 0; i < kLcSites; i++)
		for (size_t at = 0; at < g_lcSites[i].size && at < sizeof(g_lcPlaced[i]); at++)
			g_lcPlaced[i][at] = g_lcSites[i].bytes[at];
	size_t all = 0;
	for (const Site& one : g_ccSites)
		g_ccAllSites[all++] = one;
	g_ccAllSites[all++] = SITE(g_ccGeneralAdd, 0);
	g_ccAllSites[all++] = SITE(g_lcCoronaCheck, 0);
	g_ccAllSites[all++] = SITE(g_lcCoronaCheckToo, 0);
	for (size_t i = 0; i < kCcAllSites; i++)
		for (size_t at = 0; at < g_ccAllSites[i].size && at < sizeof(g_ccPlaced[i]); at++)
			g_ccPlaced[i][at] = g_ccAllSites[i].bytes[at];
	for (size_t i = 0; i < sizeof(g_ccBatchSix); i++)
		g_ccBatchSix[i] = g_ccBatch[kCcI8 + 56 + i];
	const Site jumps[kCcJumps] = { SITE(g_ccGeneralAdd, 30), firstJump, secondJump, SITE(g_ccLodLimit, kCcI9 + 13) };
	for (size_t i = 0; i < kCcJumps; i++)
	{
		g_ccJumpRels[i] = jumps[i];
		g_ccJumpTargets[i] = Rel32Target(jumps[i]);
	}
}

static const unsigned char* CcBytes(const volatile unsigned char* bytes)
{
	return const_cast<const unsigned char*>(bytes);
}

// One write of the findings' table.
struct CcPatch
{
	volatile unsigned char* at;
	size_t size;
	unsigned char bytes[14];
};
constexpr size_t kCcWrites = 13;

// The 13 writes, for a new store at store of room coronas.
static void CcWritesOf(const unsigned char* store, uint32_t room, CcPatch (&patches)[kCcWrites])
{
	const unsigned char* const count = reinterpret_cast<const unsigned char*>(&g_lcData->coronas);
	const int32_t toCount = static_cast<int32_t>(count - store);
	const uint32_t limit = room - 1;
	size_t n = 0;
	const auto value = [&patches, &n](volatile unsigned char* at, uint32_t bytes)
	{
		patches[n] = { at, 4, {} };
		memcpy(patches[n].bytes, &bytes, 4);
		n++;
	};
	const auto lead = [&value, store](volatile unsigned char* at) { value(at, static_cast<uint32_t>(store - (CcBytes(at) + 4))); };
	patches[n] = { g_ccGeneralAdd, 14, { 0x48, 0x8D, 0x0D, 0, 0, 0, 0, 0x0F, 0x1F, 0x80, 0, 0, 0, 0 } };
	const int32_t lea = static_cast<int32_t>(store - (CcBytes(g_ccGeneralAdd) + 7));
	memcpy(patches[n].bytes + 3, &lea, 4);
	n++;
	value(g_ccGeneralAdd + 17, static_cast<uint32_t>(toCount));
	value(g_ccGeneralAdd + 24, limit);
	value(g_ccGeneralAdd + 45, static_cast<uint32_t>(toCount));
	lead(g_ccFirstStore + 11);
	value(g_ccFirstCheck + 9, limit);
	lead(g_ccSecondStore + 11);
	value(g_ccSecondCheck + 9, limit);
	lead(g_ccCopy + kCcI5 + 8);
	lead(g_ccParameters + kCcI6 + 3);
	value(g_ccHandOut + 6, static_cast<uint32_t>(toCount));
	value(g_ccBatch + kCcI8 + 1, room);
	value(g_ccBatch + kCcI8 + 41, room);
}

// The first byte of the sites that is neither as placed nor as patches has it, or null; the jumps' rel32 and the batch's six bytes go by their own check.
static const volatile unsigned char* CcFirstChange(const CcPatch* patches, size_t patchCount)
{
	for (size_t i = 0; i < kCcAllSites; i++)
		for (size_t at = 0; at < g_ccAllSites[i].size && at < sizeof(g_ccPlaced[i]); at++)
		{
			const volatile unsigned char* const byte = g_ccAllSites[i].bytes + at;
			bool theirs = byte >= g_ccBatch + kCcI8 + 56 && byte < g_ccBatch + kCcI8 + 62;
			for (const Site& rel : g_ccJumpRels)
				theirs = theirs || (byte >= rel.bytes + rel.offset && byte < rel.bytes + rel.offset + 4);
			if (theirs)
				continue;
			unsigned char expected = g_ccPlaced[i][at];
			for (size_t p = 0; p < patchCount; p++)
				if (byte >= patches[p].at && byte < patches[p].at + patches[p].size)
					expected = patches[p].bytes[byte - patches[p].at];
			if (*byte != expected)
				return byte;
		}
	return nullptr;
}

// The stubs the jumps and the batch lead to, if they are what TweaksV makes: "lock inc qword [rip+disp32]", "jmp qword [rip+0]" and the address it goes on to; and the batch's.
struct CcStubs
{
	bool found;
	bool readExecute;
	bool onwards;
	const uint64_t* refused;
	const uint64_t* lodRefused;
	const uint32_t* lodBrought;
};
static CcStubs CcFindStubs()
{
	CcStubs stubs{};
	stubs.readExecute = stubs.onwards = true;
	for (size_t i = 0; i < kCcJumps; i++)
	{
		const unsigned char* const target = Rel32Target(g_ccJumpRels[i]);
		MEMORY_BASIC_INFORMATION info{};
		if (InImage(target) || VirtualQuery(target, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT || memcmp(target, "\xF0\x48\xFF\x05", 4) != 0 ||
			memcmp(target + 8, "\xFF\x25\x00\x00\x00\x00", 6) != 0)
			return stubs;
		stubs.readExecute = stubs.readExecute && info.Protect == PAGE_EXECUTE_READ;
		const unsigned char* onTo = nullptr;
		memcpy(&onTo, target + 14, sizeof(onTo));
		stubs.onwards = stubs.onwards && onTo == g_ccJumpTargets[i];
		int32_t disp = 0;
		memcpy(&disp, target + 4, sizeof(disp));
		const uint64_t* const counter = reinterpret_cast<const uint64_t*>(target + 8 + disp);
		if (i == kCcJumps - 1)
			stubs.lodRefused = counter;
		else if (i == 0)
			stubs.refused = counter;
		else
			stubs.onwards = stubs.onwards && counter == stubs.refused;
	}
	if (g_ccBatch[kCcI8 + 56] != 0xE9 || g_ccBatch[kCcI8 + 61] != 0x90)
		return stubs;
	const unsigned char* const batch = Rel32Target(SITE(g_ccBatch, kCcI8 + 57));
	MEMORY_BASIC_INFORMATION info{};
	static const unsigned char kHead[] = { 0x89, 0xF0, 0x29, 0xD0, 0x01, 0xC8, 0xF0, 0x48, 0x01, 0x05 };
	static const unsigned char kMoved[] = { 0x89, 0x35 };
	static const unsigned char kTail[] = { 0x29, 0xCA, 0x48, 0x8D, 0x04, 0x49, 0xFF, 0x25, 0, 0, 0, 0 };
	if (VirtualQuery(batch, &info, sizeof(info)) != sizeof(info) || info.Protect != PAGE_EXECUTE_READ || memcmp(batch, kHead, sizeof(kHead)) != 0 || memcmp(batch + 14, kMoved, sizeof(kMoved)) != 0 ||
		memcmp(batch + 20, kTail, sizeof(kTail)) != 0)
		return stubs;
	int32_t refused = 0, brought = 0;
	memcpy(&refused, batch + 10, sizeof(refused));
	memcpy(&brought, batch + 16, sizeof(brought));
	const unsigned char* onTo = nullptr;
	memcpy(&onTo, batch + 32, sizeof(onTo));
	stubs.onwards = stubs.onwards && onTo == CcBytes(g_ccBatch) + kCcI8 + 62 && reinterpret_cast<const uint64_t*>(batch + 14 + refused) == stubs.refused;
	stubs.lodBrought = reinterpret_cast<const uint32_t*>(batch + 20 + brought);
	stubs.found = true;
	return stubs;
}

// What the fake runs, as the game's frames would, and its one line of the report.
static wchar_t g_ccRun[640] = L"not run";
static void CcSay(const wchar_t* format, ...)
{
	static size_t used = 0;
	va_list args;
	va_start(args, format);
	const int written = vswprintf(g_ccRun + used, sizeof(g_ccRun) / sizeof(g_ccRun[0]) - used, format, args);
	va_end(args);
	if (written > 0)
		used += static_cast<size_t>(written);
}

// A record as the general add takes it: its first 16 bytes written to the store, the index it got behind them.
struct alignas(16) CcRecord
{
	uint32_t tag, more[3];
	int32_t index;
	uint32_t room[3];
};
static_assert(offsetof(CcRecord, index) == 0x10, "the general add's tail stores the index at +0x10");
struct CcAdder
{
	const void* store;
	uint32_t thread;
	int calls;
	int added;
};
static DWORD WINAPI CcRunAdds(LPVOID parameter)
{
	CcAdder& adder = *static_cast<CcAdder*>(parameter);
	const auto add = reinterpret_cast<void (*)(const void*, CcRecord*)>(const_cast<unsigned char*>(g_ccAddStart));
	for (int i = 0; i < adder.calls; i++)
	{
		CcRecord record{};
		record.tag = adder.thread << 24 | static_cast<uint32_t>(i + 1);
		record.index = -1;
		add(adder.store, &record);
		if (record.index >= 0)
			adder.added++;
	}
	return 0;
}

// What the copy stores the count in.
struct CcCommand
{
	uint64_t op;
	void* pointer;
	uint32_t count;
};
static_assert(offsetof(CcCommand, count) == 0x10, "the copy stores the count at +0x10");

static void CcActions(int argc, wchar_t** argv)
{
	const bool trap = HasOption(argc, argv, L"cctrap"), past = HasOption(argc, argv, L"ccpast");
	if (!trap && !past && !HasOption(argc, argv, L"ccrun"))
		return;
	LcData& data = *g_lcData;
	CcData& cc = *g_ccData;
	volatile uint32_t* const count = &data.coronas;
	const bool moved = g_ccGeneralAdd[0] == 0x48;
	unsigned char* const store = moved ? const_cast<unsigned char*>(Rel32Target(SITE(g_ccGeneralAdd, 3))) : data.gameCoronas;
	const uint32_t room = Imm32(SITE(g_ccGeneralAdd, 24)) + 1;
	const auto batch = reinterpret_cast<uint64_t (*)(volatile uint32_t*, uint64_t, uint32_t*)>(const_cast<unsigned char*>(g_ccBatch));
	g_ccRun[0] = L'\0';
	if (past)
	{
		*count = 1089;
		const bool summed = RtWaitFor("DIAGNOSTIC: the most so far: vehicles", 25000);
		*count = room + 1;
		const bool said = RtWaitFor("coronas seen at ", 5000);
		*count = 0;
		CcSay(L"held at 1089, %ls; then at %u, %ls", summed ? L"a summary came" : L"NO SUMMARY", room + 1, said ? L"a line came" : L"NO LINE");
		return;
	}
	if (trap)
	{
		*count = 1500;
		uint32_t placed = 0;
		batch(count, 100, &placed);
		CcSay(L"batch at 1500 brought 100 placed %u, count %u, the copy asks %llu bytes", placed, *count, uint64_t{ placed } * 0x30);
		*count = 0;
		return;
	}

	// the job's parameters
	reinterpret_cast<void (*)()>(const_cast<unsigned char*>(g_ccParameters))();
	CcSay(L"parameters name %ls and %ls; ", cc.parameterStore == store ? L"the store" : L"ANOTHER store", cc.parameterCount == const_cast<const uint32_t*>(count) ? L"its count" : L"ANOTHER count");

	// two threads through the general add, as the callers call it, with the game's store
	*count = 0;
	const int calls = static_cast<int>(room / 4 * 3 + (room % 4) * 3 / 4);
	CcAdder adders[2] = { { data.gameCoronas, 1, calls, 0 }, { data.gameCoronas, 2, calls, 0 } };
	const HANDLE thread = CreateThread(nullptr, 0, CcRunAdds, &adders[0], 0, nullptr);
	if (!thread)
		CcRunAdds(&adders[0]);
	CcRunAdds(&adders[1]);
	if (thread)
	{
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}
	int slots[3] = {};
	for (uint32_t i = 0; i < room; i++)
	{
		uint32_t tag = 0;
		memcpy(&tag, store + size_t{ i } * 0x30, sizeof(tag));
		slots[tag >> 24 < 3 ? tag >> 24 : 0]++;
	}
	bool gameStoreEmpty = true;
	for (size_t i = 0; i < sizeof(data.gameCoronas); i++)
		gameStoreEmpty = gameStoreEmpty && data.gameCoronas[i] == 0;
	CcSay(L"adds %d added, %d refused, %ls, the old store %ls, count %u; ", adders[0].added + adders[1].added, 2 * calls - adders[0].added - adders[1].added,
		slots[1] == adders[0].added && slots[2] == adders[1].added && slots[0] == 0 ? L"one slot each" : L"SLOTS LOST", gameStoreEmpty ? L"all 0" : L"in use", *count);

	// a batch that reaches the room, its copy as the game makes it
	static unsigned char batchRecords[300 * 0x30];
	memset(batchRecords, 0xBB, sizeof(batchRecords));
	const uint32_t start = room - 100;
	*count = start;
	uint32_t placed = 0;
	const uint64_t offset = batch(count, 300, &placed);
	const bool placedWell = placed <= 300 && offset == uint64_t{ start } * 0x30;
	if (placedWell)
		memcpy(store + offset, batchRecords, size_t{ placed } * 0x30);
	CcSay(L"batch at %u brought 300 placed %u, count %u, %ls; ", start, placed, *count, placedWell && store[size_t{ room - 1 } * 0x30] == 0xBB ? L"its last record in the last slot" : L"NOT IN PLACE");

	// the job's own limit, which the raise leaves
	const auto lodLimit = reinterpret_cast<int (*)(uint32_t)>(const_cast<unsigned char*>(g_ccLodLimit));
	const int below = lodLimit(0x43F);
	const int at = lodLimit(0x440) + lodLimit(0x441) + lodLimit(0x7FF);
	CcSay(L"its own limit %ls 1087, %ls 1088; ", below == 1 ? L"passes" : L"REFUSES", at == 0 ? L"refuses" : L"PASSES");

	// the copy, at a full store and at 0
	const size_t bytes = size_t{ room } * 0x30 + 0x180;
	void* const target = VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	CcCommand command = {};
	const auto copy = reinterpret_cast<void (*)(void*, void*)>(const_cast<unsigned char*>(g_ccCopy));
	*count = room;
	copy(target, &command);
	CcSay(L"copy of %u asked %llu bytes, %ls; ", command.count, static_cast<unsigned long long>(g_ccCopied), g_ccCopiedFrom == store && target && memcmp(target, store, size_t{ room } * 0x30) == 0 ? L"as the store" : L"NOT AS THE STORE");
	g_ccCopied = 0;
	*count = 0;
	copy(target, &command);
	CcSay(L"copy of 0 asked %llu", static_cast<unsigned long long>(g_ccCopied));
	if (target)
		VirtualFree(target, 0, MEM_RELEASE);
}

// "cc sites: as placed | cc jumps: as placed | cc run: not run | cc now: ... | cc rvas: ... | " - the sites: as placed, or the store moved, N coronas, every write of the findings'
// table and nothing else, its memory RW, 16-aligned and of the size asked; the jumps: as placed, or each through its stub, with the counters; what the fake ran; the accessor, in a test
// build; and where the sites lie, for the suite to compare with the log's lines.
static void CcReport(HMODULE module)
{
	if (!CcFirstChange(nullptr, 0))
		wprintf(L"cc sites: as placed");
	else
	{
		const unsigned char* const store = Rel32Target(SITE(g_ccGeneralAdd, 3));
		const uint32_t room = Imm32(SITE(g_ccGeneralAdd, 24)) + 1;
		const size_t bytes = size_t{ room } * 0x30 + 0x180;
		CcPatch patches[kCcWrites];
		CcWritesOf(store, room, patches);
		MEMORY_BASIC_INFORMATION info{};
		const bool held = VirtualQuery(store, &info, sizeof(info)) == sizeof(info) && info.State == MEM_COMMIT && info.Protect == PAGE_READWRITE && info.RegionSize >= bytes;
		const volatile unsigned char* const changed = CcFirstChange(patches, kCcWrites);
		if (changed)
			wprintf(L"cc sites: CHANGED, at 0x%llX", static_cast<unsigned long long>(CcBytes(changed) - ImageBase()));
		else
			wprintf(L"cc sites: the store moved, %u coronas, the 13 writes and nothing else, its memory %ls, %ls, %llu bytes at least", room, held ? L"RW" : L"NOT AS ASKED",
				reinterpret_cast<uintptr_t>(store) % 16 == 0 ? L"16-aligned" : L"NOT 16-ALIGNED", static_cast<unsigned long long>(bytes));
	}
	bool asPlaced = true;
	for (size_t i = 0; i < kCcJumps; i++)
		asPlaced = asPlaced && Rel32Target(g_ccJumpRels[i]) == g_ccJumpTargets[i];
	for (size_t i = 0; i < sizeof(g_ccBatchSix); i++)
		asPlaced = asPlaced && g_ccBatch[kCcI8 + 56 + i] == g_ccBatchSix[i];
	if (asPlaced)
		wprintf(L" | cc jumps: as placed");
	else
	{
		const CcStubs stubs = CcFindStubs();
		if (!stubs.found)
			wprintf(L" | cc jumps: CHANGED");
		else
		{
			MEMORY_BASIC_INFORMATION info{};
			const bool counters = VirtualQuery(stubs.refused, &info, sizeof(info)) == sizeof(info) && info.Protect == PAGE_READWRITE && stubs.lodRefused && stubs.lodBrought;
			wprintf(L" | cc jumps: 4 and the batch to stubs, %ls, %ls, their counters %ls: refused %llu, lod own %llu, lod lights %u", stubs.readExecute ? L"RX" : L"NOT RX",
				stubs.onwards ? L"on to where each led" : L"ELSEWHERE", counters ? L"RW" : L"NOT RW", counters ? static_cast<unsigned long long>(*stubs.refused) : 0ull,
				counters ? static_cast<unsigned long long>(*stubs.lodRefused) : 0ull, counters ? *stubs.lodBrought : 0u);
		}
	}
	wprintf(L" | cc run: %ls | ", g_ccRun);
	const auto now = reinterpret_cast<void (*)(uint64_t*)>(GetProcAddress(module, "TweaksVTestCoronaStoreNow"));
	if (now)
	{
		uint64_t numbers[5] = {};
		now(numbers);
		wprintf(L"cc now: room %llu, counted %llu, refused %llu, lod own %llu, lod lights %llu | ", static_cast<unsigned long long>(numbers[0]), static_cast<unsigned long long>(numbers[1]),
			static_cast<unsigned long long>(numbers[2]), static_cast<unsigned long long>(numbers[3]), static_cast<unsigned long long>(numbers[4]));
	}
	const auto rva = [](const volatile void* p) { return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(const_cast<const void*>(p)) - reinterpret_cast<uintptr_t>(ImageBase())); };
	const LcData& data = *g_lcData;
	const CcData& cc = *g_ccData;
	wprintf(L"cc rvas: i1 0x%llX, i2 0x%llX, i3 0x%llX, i4 0x%llX, i5 0x%llX, i6 0x%llX, i7 0x%llX, i8 0x%llX, i9 0x%llX, j1 0x%llX, j2 0x%llX, j3 0x%llX, j4 0x%llX, j5 0x%llX, first 0x%llX, second 0x%llX, "
		L"store 0x%llX, count 0x%llX, pointer 0x%llX, render count 0x%llX, extras 0x%llX, mark 0x%llX, add way out 0x%llX, first way out 0x%llX, second way out 0x%llX, lod way out 0x%llX | ",
		rva(g_ccGeneralAdd), rva(g_ccFirstStore), rva(g_ccSecondStore), rva(g_ccCopy + kCcI4), rva(g_ccCopy + kCcI5), rva(g_ccParameters + kCcI6), rva(g_ccHandOut), rva(g_ccBatch + kCcI8), rva(g_ccLodLimit + kCcI9),
		rva(g_ccScratch), rva(g_ccCommand), rva(g_ccDraw), rva(g_ccStackRecords), rva(g_ccStackIndices), rva(g_ccFirstCheck), rva(g_ccSecondCheck), rva(data.gameCoronas), rva(&data.coronas),
		rva(&cc.renderPointer), rva(&cc.renderCount), rva(&cc.extras), rva(&cc.mark), rva(g_ccGeneralAdd + kCcAddWayOut), rva(g_ccFirstWayOut), rva(g_ccSecondWayOut), rva(g_ccLodLimit + kCcLodWayOut));
}

// The game's two tree searches behind the box streamer's four calls: the first and the third call lead
// to the first search, the second and the fourth to the second. Each writes the number of entries the
// harness asks for - at most 16 short of the capacity it is handed, where the game's searches stop - and
// returns it, and checks that it was given the harness's own arguments, the capacity its call site
// holds among them. The two differ, so that the linker cannot fold them into one.
using BoxTreeSearch = int (*)(void* tree, const float* min, const float* max, uint64_t* list, int capacity);
static uint64_t g_boxSearchList[10000];
static int g_boxSearchWanted = 0, g_boxSearchCapacity = 0, g_boxSearchCallsA = 0, g_boxSearchCallsB = 0, g_boxSearchArgsBad = 0;
static int FakeTreeSearch(void* tree, const float* min, const float* max, uint64_t* list, int capacity)
{
	if (tree != nullptr || !min || !max || list != g_boxSearchList || capacity != g_boxSearchCapacity)
		g_boxSearchArgsBad++;
	const int found = g_boxSearchWanted < capacity - 16 ? g_boxSearchWanted : capacity - 16;
	for (int i = 0; i < found; i++)
		list[i] = static_cast<uint64_t>(i);
	return found;
}
static __declspec(noinline) int FakeTreeSearchA(void* tree, const float* min, const float* max, uint64_t* list, int capacity)
{
	g_boxSearchCallsA++;
	return FakeTreeSearch(tree, min, max, list, capacity);
}
static __declspec(noinline) int FakeTreeSearchB(void* tree, const float* min, const float* max, uint64_t* list, int capacity)
{
	g_boxSearchCallsB++;
	return FakeTreeSearch(tree, min, max, list, capacity);
}

// The game's own function behind the streaming configuration call.
static int g_origCalls = 0;
static uint32_t g_origSawCount = 0;
static __declspec(noinline) void OriginalSetStreamingInterface(char* si)
{
	g_origCalls++;
	g_origSawCount = *reinterpret_cast<uint32_t*>(si + 24);
}

// A script program, laid out at the offsets of the game's (PATTERNS.md, "Script programs"); the fields the game
// has there and TweaksV does not read hold recognisable values, so that the copy of the object shows them.
struct FakeScriptProgram
{
	uint64_t vtable;				// +0x00
	uint64_t unknown08;
	unsigned char** codePages;		// +0x10: one pointer per page of 16 KB
	uint32_t unknown18;
	uint32_t codeLength;			// +0x1C
	uint32_t unknown20[3];
	uint32_t nativeCount;			// +0x2C
	uint64_t unknown30[2];
	uint64_t* natives;				// +0x40
	uint64_t unknown48[2];
	uint32_t nameHash;				// +0x58
	uint32_t unknown5C;
	const char* name;				// +0x60
	unsigned char** stringPages;	// +0x68
	uint32_t stringLength;			// +0x70
	uint32_t unknown74[3];
};
static_assert(offsetof(FakeScriptProgram, codePages) == 0x10 && offsetof(FakeScriptProgram, codeLength) == 0x1C &&
	offsetof(FakeScriptProgram, nativeCount) == 0x2C && offsetof(FakeScriptProgram, natives) == 0x40 &&
	offsetof(FakeScriptProgram, nameHash) == 0x58 && offsetof(FakeScriptProgram, name) == 0x60 &&
	offsetof(FakeScriptProgram, stringPages) == 0x68 && offsetof(FakeScriptProgram, stringLength) == 0x70 &&
	sizeof(FakeScriptProgram) == 0x80, "the fake's script program must have the game's layout");

// The natives the fake's programs call: their hashes, and the handler the fake's resolution function puts in
// their place - a function of the fake's own, each a different one so that the linker cannot fold them; a hash
// it does not know gets the "unknown" handler, as the game's does, and the function returns 0.
static int g_nativeCalls = 0;
static __declspec(noinline) void FakeNativeA() { g_nativeCalls += 1; }
static __declspec(noinline) void FakeNativeB() { g_nativeCalls += 2; }
static __declspec(noinline) void FakeNativeC() { g_nativeCalls += 3; }
static __declspec(noinline) void FakeNativeUnknown() { g_nativeCalls += 4; }
struct FakeNative { uint64_t hash; void (*handler)(); };
static const FakeNative g_fakeNatives[] = { { 0x4A5B6C7D8E9F0011ull, &FakeNativeA }, { 0x1122334455667788ull, &FakeNativeB },
	{ 0x99AABBCCDDEEFF00ull, &FakeNativeC } };
constexpr size_t kFakeNatives = sizeof(g_fakeNatives) / sizeof(g_fakeNatives[0]);
constexpr uint64_t kUnknownNativeHash = 0x0123456789ABCDEFull;
constexpr DWORD kResolveLastError = 0x2A1B;	// the last error the fake's resolution function leaves

// The game's own function behind the call at the hook site: the program's native table, hashes until now,
// gets the handlers; 1 when every hash was known. It reads no more entries than the fake's tables have, whatever
// the program's native count says. With the option blipchanged it also changes blip_controller's threshold, at
// 0x403 of its code, to 151 - between the Blips part's look at the program and its write, as another mod could.
static int g_resolveCalls = 0;
static bool g_blipChanged = false;
static __declspec(noinline) unsigned char FakeResolveNatives(unsigned char* program)
{
	g_resolveCalls++;
	const auto p = reinterpret_cast<FakeScriptProgram*>(program);
	if (g_blipChanged && p->name && strcmp(p->name, "blip_controller") == 0)
		p->codePages[0][0x403] = 0x97;
	unsigned char every = 1;
	for (uint32_t i = 0; i < p->nativeCount && i < kFakeNatives; i++)
	{
		uint64_t handler = reinterpret_cast<uint64_t>(&FakeNativeUnknown);
		for (const FakeNative& native : g_fakeNatives)
			if (p->natives[i] == native.hash)
				handler = reinterpret_cast<uint64_t>(native.handler);
		if (handler == reinterpret_cast<uint64_t>(&FakeNativeUnknown))
			every = 0;
		p->natives[i] = handler;
	}
	SetLastError(kResolveLastError);
	return every;
}

static void SetRel32(const Site& site, const void* target)
{
	auto field = const_cast<unsigned char*>(site.bytes + site.offset);
	const int32_t rel = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - (reinterpret_cast<intptr_t>(field) + 4));
	memcpy(field, &rel, 4);
}

static const unsigned char* Rel32Target(const Site& site)
{
	auto field = const_cast<const unsigned char*>(site.bytes + site.offset);
	int32_t rel = 0;
	memcpy(&rel, field, 4);
	return field + 4 + rel;
}

static uint32_t Imm32(const Site& site)
{
	uint32_t value = 0;
	memcpy(&value, const_cast<const unsigned char*>(site.bytes + site.offset), 4);
	return value;
}

static void SetImm32(const Site& site, uint32_t value)
{
	memcpy(const_cast<unsigned char*>(site.bytes + site.offset), &value, 4);
}

static const unsigned char* ImageBase()
{
	return reinterpret_cast<const unsigned char*>(GetModuleHandleW(nullptr));
}

// The game build this program pretends to be, from its own version resource (fake_game_version.rc):
// FILEVERSION 1,0,C,D gives (C << 16) | D; 0 without one.
static uint32_t OwnBuild()
{
	const HRSRC resource = FindResourceW(nullptr, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
	const HGLOBAL loaded = resource ? LoadResource(nullptr, resource) : nullptr;
	const auto data = static_cast<const unsigned char*>(loaded ? LockResource(loaded) : nullptr);
	const DWORD size = resource ? SizeofResource(nullptr, resource) : 0;
	for (DWORD at = 0; data && at + sizeof(VS_FIXEDFILEINFO) <= size; at += 4)
	{
		VS_FIXEDFILEINFO info{};
		memcpy(&info, data + at, sizeof(info));
		if (info.dwSignature == VS_FFI_SIGNATURE)
			return info.dwFileVersionLS;
	}
	return 0;
}

static const unsigned char* ImageEnd()
{
	const auto base = ImageBase();
	const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
	const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
	return base + nt->OptionalHeader.SizeOfImage;
}

static bool InImage(const void* p)
{
	return p >= ImageBase() && p < ImageEnd();
}

// "RW, 128 KB" for a new array outside the game
static void PrintNewArray(const unsigned char* array)
{
	MEMORY_BASIC_INFORMATION info{};
	VirtualQuery(array, &info, sizeof(info));
	const SIZE_T usable = info.RegionSize - (array - static_cast<const unsigned char*>(info.BaseAddress));
	wprintf(L"%ls, %llu KB", info.Protect == PAGE_READWRITE ? L"RW" : info.Protect == PAGE_EXECUTE_READWRITE ? L"RWX" : L"other",
		static_cast<unsigned long long>(usable / 1024));
}

// "game array", "new array (...)" or "mixed: ..."
static void PrintModkitRefs()
{
	const unsigned char* first = Rel32Target(g_modkitRefs[0]);
	bool same = true;
	for (const Site& site : g_modkitRefs)
		same = same && Rel32Target(site) == first;
	if (same && first == reinterpret_cast<const unsigned char*>(g_oldModkitArray))
	{
		wprintf(L"modkit refs: game array | ");
	}
	else if (same && !InImage(first))
	{
		wprintf(L"modkit refs: new array (");
		PrintNewArray(first);
		wprintf(L") | ");
	}
	else
	{
		wprintf(L"modkit refs: mixed | ");
	}
}

static void PrintModkitLimits()
{
	wprintf(L"limits:");
	for (const Site& site : g_modkitLimits)
		wprintf(L" %u", Imm32(site));
	wprintf(L" | ");
}

// "modkit copy: ok" - the new array holds the game's own 1024 entries (main puts a value in the first
// and the last); "game array" when nothing was moved
static void PrintModkitCopy()
{
	const unsigned char* first = Rel32Target(g_modkitRefs[0]);
	if (first == reinterpret_cast<const unsigned char*>(g_oldModkitArray) || InImage(first))
	{
		wprintf(L"modkit copy: game array | ");
		return;
	}
	MEMORY_BASIC_INFORMATION info{};
	if (VirtualQuery(first, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT ||
		static_cast<const unsigned char*>(info.BaseAddress) + info.RegionSize < first + sizeof(g_oldModkitArrayStorage))
	{
		wprintf(L"modkit copy: unreadable | ");
		return;
	}
	wprintf(L"modkit copy: %ls | ", memcmp(first, g_oldModkitArray, sizeof(g_oldModkitArrayStorage)) == 0 ? L"ok" : L"MISSING");
}

// "pack refs: 32 new, R31a new, R31b game (RW, 512 KB)" - "new" is the common target of the
// first reference when that is outside the game
static void PrintPackfile(int argc, wchar_t** argv);

static void SetBytes(volatile unsigned char* target, const char* bytes, size_t size)
{
	for (size_t i = 0; i < size; i++)
		target[i] = static_cast<unsigned char>(bytes[i]);
}

static void PrintBytes(const char* name, const volatile unsigned char* bytes, size_t size)
{
	wprintf(L"%hs:", name);
	for (size_t i = 0; i < size; i++)
		wprintf(L" %02X", bytes[i]);
	wprintf(L" | ");
}

static bool HasOption(int argc, wchar_t** argv, const wchar_t* option)
{
	for (int i = 2; i < argc; i++)
		if (wcscmp(argv[i], option) == 0)
			return true;
	return false;
}

static const char* RefState(const unsigned char* target, const unsigned char* newArray)
{
	if (target == reinterpret_cast<const unsigned char*>(g_oldPackfileArray))
		return "game";
	if (newArray && target == newArray)
		return "new";
	return "other";
}

static void PrintPackfile(int argc, wchar_t** argv)
{
	const unsigned char* first = Rel32Target(g_packRefs[0]);
	const unsigned char* newArray = InImage(first) ? nullptr : first;
	int moved = 0, game = 0, other = 0;
	for (const Site& site : g_packRefs)
	{
		const unsigned char* target = Rel32Target(site);
		if (target == reinterpret_cast<const unsigned char*>(g_oldPackfileArray))
			game++;
		else if (newArray && target == newArray)
			moved++;
		else
			other++;
	}
	wprintf(L"pack refs: %d new %d game %d other, R31a %hs, R31b %hs", moved, game, other,
		RefState(Rel32Target(g_packR31aSite), newArray), RefState(Rel32Target(g_packR31bSite), newArray));
	if (newArray)
	{
		wprintf(L" (");
		PrintNewArray(newArray);
		wprintf(L")");
	}
	wprintf(L" | pack sizes: D3=%u D2=%u D1=%u I=%u | ", Imm32(g_packDirect[0]), Imm32(g_packDirect[1]), Imm32(g_packDirect[2]),
		g_packIndirectSize);

	// The streaming configuration call: still the game's function, another mod's, or TweaksV's stub.
	const unsigned char* callTarget = Rel32Target(g_packCallSite);
	if (callTarget == reinterpret_cast<const unsigned char*>(&OriginalSetStreamingInterface))
	{
		wprintf(L"hook: not set | ");
	}
	else if (HasOption(argc, argv, L"packhooked") && callTarget == ImageEnd() + 0x20000)
	{
		wprintf(L"hook: other mod's | ");
	}
	else
	{
		// Call it the way the game would, with a streaming configuration whose archive count is 3000 - or,
		// with the option "packbigconfig", 20000, more than the array is given.
		alignas(8) char si[64] = {};
		const uint32_t configCount = HasOption(argc, argv, L"packbigconfig") ? 20000 : 3000;
		memcpy(si + 24, &configCount, 4);
		reinterpret_cast<void (*)(char*)>(const_cast<unsigned char*>(callTarget))(si);
		uint32_t after = 0;
		memcpy(&after, si + 24, 4);
		wprintf(L"hook: config %u -> %u, original called %dx with %u | ", configCount, after, g_origCalls, g_origSawCount);
	}
}

// Calls each of the four tree-search calls' targets the way the game would - TweaksV's stub or the
// game's own tree search, with the capacity its site holds; "another mod's" address (option
// "boxcallhooked") not at all - asking for 100, 400, 700 and 1000 entries of a 10000-entry list.
// "box calls: 4 redirected 0 game 0 other, originals 2x+2x, args ok, returns 100 400 700 1000 | "
static wchar_t g_boxCallsResult[160];
static void CallBoxSearches()
{
	const unsigned char* const otherMod = ImageEnd() + 0x40000;
	int redirected = 0, game = 0, other = 0;
	int returns[4] = { -1, -1, -1, -1 };
	const float lower[4] = { -10.0f, -10.0f, -10.0f, 0.0f };
	const float upper[4] = { 10.0f, 10.0f, 10.0f, 0.0f };
	for (int i = 0; i < 4; i++)
	{
		const unsigned char* target = Rel32Target(g_boxCalls[i]);
		if (target == reinterpret_cast<const unsigned char*>(&FakeTreeSearchA) ||
			target == reinterpret_cast<const unsigned char*>(&FakeTreeSearchB))
		{
			game++;
		}
		else if (target == otherMod || InImage(target))
		{
			other++;
			continue;
		}
		else
		{
			redirected++;
		}
		g_boxSearchWanted = 100 + 300 * i;
		g_boxSearchCapacity = static_cast<int>(Imm32(g_boxCaps[i]));
		returns[i] = reinterpret_cast<BoxTreeSearch>(const_cast<unsigned char*>(target))(nullptr, lower, upper, g_boxSearchList,
			g_boxSearchCapacity);
	}
	swprintf(g_boxCallsResult, 160, L"box calls: %d redirected %d game %d other, originals %dx+%dx, args %ls, returns %d %d %d %d | ",
		redirected, game, other, g_boxSearchCallsA, g_boxSearchCallsB, g_boxSearchArgsBad == 0 ? L"ok" : L"BAD",
		returns[0], returns[1], returns[2], returns[3]);
}

// "box refs: 8 new 0 game 0 other (RW, 784 KB) | box caps: 10000 10000 10000 10000 | " - "new" is the
// common target of the first reference when that is outside the game and not the other mod's address
// (option "boxmoved") - then the tree-search calls (above)
static void PrintBoxStreamer()
{
	const unsigned char* const oldList = reinterpret_cast<const unsigned char*>(g_oldBoxList);
	const unsigned char* const otherMod = ImageEnd() + 0x30000;
	const unsigned char* first = Rel32Target(g_boxRefs[0]);
	const unsigned char* newList = InImage(first) || first == otherMod ? nullptr : first;
	int moved = 0, game = 0, other = 0;
	for (const Site& site : g_boxRefs)
	{
		const unsigned char* target = Rel32Target(site);
		if (target == oldList)
			game++;
		else if (newList && target == newList)
			moved++;
		else
			other++;
	}
	wprintf(L"box refs: %d new %d game %d other", moved, game, other);
	if (newList)
	{
		wprintf(L" (");
		PrintNewArray(newList);
		wprintf(L")");
	}
	wprintf(L" | box caps: %u %u %u %u | %ls", Imm32(g_boxCaps[0]), Imm32(g_boxCaps[1]), Imm32(g_boxCaps[2]), Imm32(g_boxCaps[3]),
		g_boxCallsResult);
}

// "decal refs: 31 new 0 game 0 other (RW, 1280 KB) | decal limits: 2047 2048 0x28198, 23 agree | decal count: 0 | " -
// "new" is the common target of the first reference when that is outside the game and not the other
// mod's address (option "decalmoved"); the limits are what the first site of each kind holds (N-1, N,
// the end of the start-up loop) and how many sites differ from the first of their kind
static void PrintDecals()
{
	const unsigned char* const oldArray = g_oldDecalStore->entries;
	const unsigned char* const otherMod = ImageEnd() + 0x50000;
	const unsigned char* first = Rel32Target(g_decalRefs[0]);
	const unsigned char* newArray = InImage(first) || first == otherMod ? nullptr : first;
	int moved = 0, game = 0, other = 0;
	for (const Site& site : g_decalRefs)
	{
		const unsigned char* target = Rel32Target(site);
		if (target == oldArray)
			game++;
		else if (newArray && target == newArray)
			moved++;
		else
			other++;
	}
	wprintf(L"decal refs: %d new %d game %d other", moved, game, other);
	if (newArray)
	{
		wprintf(L" (");
		PrintNewArray(newArray);
		wprintf(L")");
	}
	uint32_t firstOf[3] = {};
	bool seen[3] = {};
	int differ = 0;
	for (const DecalLimit& limit : g_decalLimits)
	{
		const int kind = limit.holds == 'L' ? 0 : limit.holds == 'N' ? 1 : 2;
		const uint32_t value = Imm32(limit.site);
		if (!seen[kind])
		{
			seen[kind] = true;
			firstOf[kind] = value;
		}
		else if (value != firstOf[kind])
			differ++;
	}
	wprintf(L" | decal limits: %u %u 0x%X, ", firstOf[0], firstOf[1], firstOf[2]);
	if (differ == 0)
		wprintf(L"23 agree");
	else
		wprintf(L"%d differ", differ);
	wprintf(L" | decal count: %u | ", g_oldDecalStore->count);
}

// Every site is written once before TweaksV loads, so all their pages are private copies alike; then
// the whole section is made read+execute, as the game's code is (MakeSitesReadExecute). TweaksV must
// make each page it patches writable and give it its own protection back.
static void TouchSite(const Site& site)
{
	for (size_t i = 0; i < site.size; i++)
		site.bytes[i] = site.bytes[i];
}

static void ForEachSite(void (*fn)(const Site&))
{
	static Site singles[] = { SITE(g_heapSite, 0), SITE(g_splashSite, 0), SITE(g_legalSite, 0), SITE(g_landingSite, 0),
		SITE(g_landingCallee, 0), SITE(g_landingSiteCopy, 0), SITE(g_modkitExtra, 0), SITE(g_packI1, 0), SITE(g_packH1, 0),
		SITE(g_packR31a, 0), SITE(g_packR31b, 0), SITE(g_packR31aCopy, 0), SITE(g_packExtra, 0), SITE(g_boxRef9, 0),
		SITE(g_decalFull, 0), SITE(g_decalParserLea, 0), SITE(g_decalFindById, 0), SITE(g_decalPickVariant, 0), SITE(g_decalOverrideOne, 0), SITE(g_decalOverrideLoop, 0), SITE(g_decalOverrideCmp1, 0),
		SITE(g_decalOverrideCmpEbp, 0), SITE(g_decalOverrideCmp2, 0), SITE(g_decalLoopStart, 0), SITE(g_decalLoopEnd, 0),
		SITE(g_decalAddR8d, 0), SITE(g_decalAddEsi, 0), SITE(g_decalAddRel8, 0), SITE(g_decalAddEbx, 0), SITE(g_decalLeaExtra, 0),
		SITE(g_scriptHookSite, 0), SITE(g_scriptNativeLoop, 0), SITE(g_scriptInterpreter, 0), SITE(g_blipPoolSite, 0), SITE(g_blipMarkSite, 0) };
	for (const Site& site : singles) fn(site);
	for (auto& row : g_decalAdd) { Site site = { row, 0, sizeof(row) }; fn(site); }
	for (auto& row : g_decalLeas) { Site site = { row, 0, sizeof(row) }; fn(site); }
	for (const Site& site : g_modkitRefs) fn(site);
	for (const Site& site : g_modkitLimits) fn(site);
	for (const Site& site : g_packDirect) fn(site);
	for (const Site& site : g_packRefs) fn(site);
	for (const Site& site : g_boxCaps) fn(site);
	for (const Site& site : g_boxRefs) fn(site);
	Site indirect = { reinterpret_cast<volatile unsigned char*>(&g_packIndirectSize), 0, sizeof(g_packIndirectSize) };
	fn(indirect);
	// the parts of the parts' coding round and the limits measured first: every site of each
	for (const Site& site : g_weaponInfosSites) fn(site);
	for (const Site& site : g_weaponComponentsSites) fn(site);
	for (auto& row : g_weaponComponentsLeas) { Site site = { row, 0, sizeof(row) }; fn(site); }
	for (const SirenIdsSite& sirenIds : g_sirenIdsSites)
	{
		const Site site = { sirenIds.bytes, 0, sirenIds.size };
		fn(site);
	}
	fn(g_sirenCountRendererCall);
	fn(g_sirenCountGlassSite);
	fn(g_sirenCountFindBoneSite);
	fn(g_sirenCountStateSize);
	fn(g_sirenCountOutsideSite);
	for (const Site& site : g_pedPropsSites) fn(site);
	static Site textureBudgetSites[] = { SITE(g_textureBudgetStores, 0), SITE(g_textureBudgetGetter, 0), SITE(g_textureBudgetPlacement, 0),
		SITE(g_textureBudgetUpdate, 0), SITE(g_textureBudgetWriters, 0), SITE(g_textureBudgetMisread, 0), SITE(g_textureBudgetHeapSite, 0), SITE(g_textureBudgetMaker, 0) };
	for (const Site& site : textureBudgetSites) fn(site);
	for (const Site& site : g_rtSites) fn(site);
	for (const Site& site : g_staticBoundsSites) fn(site);
	for (const Site& site : g_lcSites) fn(site);
	for (const Site& site : g_popSites) fn(site);
	for (const Site& site : g_ppSites) fn(site);
	static Site poolMemorySite = SITE(g_poolMemorySite, 0);
	fn(poolMemorySite);
	for (const Site& site : g_rqSites) fn(site);
	// [siren damage]
	for (const Site& site : g_sdSites) fn(site);
	// [corona store]
	for (const Site& site : g_ccSites) fn(site);
}

static void MakeSitesReadExecute()
{
	const auto base = const_cast<unsigned char*>(ImageBase());
	const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
	const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
	for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
		if (memcmp(section->Name, ".fakesit", 8) == 0)
		{
			DWORD old = 0;
			VirtualProtect(base + section->VirtualAddress, section->Misc.VirtualSize, PAGE_EXECUTE_READ, &old);
		}
}

// The protection every site had before TweaksV loaded (read+execute); TweaksV must put every page
// it changes back to it.
static DWORD g_siteProtect = 0;
static int g_notRestored = 0;
static DWORD ProtectionOf(const volatile void* p)
{
	MEMORY_BASIC_INFORMATION info{};
	VirtualQuery(const_cast<const void*>(p), &info, sizeof(info));
	return info.Protect;
}
static void TouchData(const volatile unsigned char* bytes, size_t size)
{
	auto p = const_cast<volatile unsigned char*>(bytes);
	for (size_t i = 0; i < size; i++)
		p[i] = p[i];
}
static void CheckData(const volatile unsigned char* bytes, size_t size)
{
	if (ProtectionOf(bytes) != g_siteProtect)
		g_notRestored++;
	if (ProtectionOf(bytes + size - 1) != g_siteProtect)
		g_notRestored++;
}
static void CheckSite(const Site& site)
{
	if (ProtectionOf(site.bytes) != g_siteProtect)
		g_notRestored++;
	if (ProtectionOf(site.bytes + site.size - 1) != g_siteProtect)
		g_notRestored++;
}

// TweaksV's call stubs: every call it redirected leads to one, and none may be writable.
static int g_stubs = 0, g_writableStubs = 0;
static void CheckStub(const unsigned char* callTarget)
{
	MEMORY_BASIC_INFORMATION info{};
	if (InImage(callTarget) || VirtualQuery(callTarget, &info, sizeof(info)) != sizeof(info) || info.State != MEM_COMMIT)
		return;	// not redirected, or "another mod's" address, where nothing is
	g_stubs++;
	if (info.Protect != PAGE_EXECUTE_READ)
		g_writableStubs++;
}

// DirectStorage (TweaksV's UNLOCK_FILES and DIAGNOSTIC_MODE): the calls the game makes into the stand-in
// dstorage.dll, which opens its files for real and holds them open, as DirectStorage does, and the factory's
// OpenFile for 19 paths: 4 archives in the game folder (one with a full path, one .CACHE), 7 other files in
// it (one with '/', one with a \\?\ prefix, one named x.rpf.txt, one whose extension is too long for
// TweaksV's table of types), 7 paths outside it (a save, temp files - one an archive - a sibling folder whose
// name starts with the game folder's, a path that leaves the game folder through "..", a network path in its
// \\?\UNC\ form, its prefix in upper and in lower case) and one too long to check. The files in the game
// folder are made first and deleted again afterwards; the others are not there.
enum class OtherInterface { None, First, Later, Only };
static const GUID g_factoryId = { 0x6924EA0C, 0xC3CD, 0x4826, { 0xB1, 0x0A, 0xF6, 0x4F, 0x4E, 0xD9, 0x27, 0xC1 } };
static const GUID g_otherId = { 0x12345678, 0x1234, 0x5678, { 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34, 0x56, 0x78 } };
using OpenFileFn = HRESULT(STDMETHODCALLTYPE*)(void*, const wchar_t*, REFIID, void**);
using FileHandleFn = HANDLE(WINAPI*)(void*);
using CloseFileFn = void(WINAPI*)(void*);
static void* g_factory = nullptr;			// for dsshare and dsflood
static OpenFileFn g_openFile = nullptr;
static FileHandleFn g_fileHandle = nullptr;	// the handle a file the stand-in opened holds
static CloseFileFn g_closeFile = nullptr;	// lets go of a file the stand-in opened
static wchar_t g_dstorageResult[1024];

// A file of the fake's own with text in it (one write open).
static void MakeFile(const wchar_t* path, const char* text = "test")
{
	const HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return;
	DWORD written = 0;
	WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, nullptr);
	CloseHandle(file);
}

static void CallDStorage(HMODULE dstorage, OtherInterface other)
{
	using GetFactory = HRESULT(WINAPI*)(REFIID, void**);
	using SetConfiguration = HRESULT(WINAPI*)(const void*);
	const auto setConfiguration = reinterpret_cast<SetConfiguration>(GetProcAddress(dstorage, "DStorageSetConfiguration"));
	const auto setConfiguration1 = reinterpret_cast<SetConfiguration>(GetProcAddress(dstorage, "DStorageSetConfiguration1"));
	const auto getFactory = reinterpret_cast<GetFactory>(GetProcAddress(dstorage, "DStorageGetFactory"));
	using FakeOpenFileCalls = long(WINAPI*)();
	const auto openFileCalls = reinterpret_cast<FakeOpenFileCalls>(GetProcAddress(dstorage, "FakeOpenFileCalls"));
	g_fileHandle = reinterpret_cast<FileHandleFn>(GetProcAddress(dstorage, "FakeFileHandle"));
	g_closeFile = reinterpret_cast<CloseFileFn>(GetProcAddress(dstorage, "FakeCloseFile"));
	const size_t size = sizeof(g_dstorageResult) / sizeof(g_dstorageResult[0]);
	if (!setConfiguration || !setConfiguration1 || !getFactory || !openFileCalls || !g_fileHandle || !g_closeFile)
	{
		swprintf(g_dstorageResult, size, L"dstorage: exports missing | ");
		return;
	}
	const uint32_t configuration[8] = { 4, 0xFFFFFFFF, 0, 1, 0, 0, 1, 1 };
	const HRESULT a = setConfiguration(configuration);
	const HRESULT b = setConfiguration1(configuration);
	void* otherFactory = nullptr;
	if (other == OtherInterface::Only)	// dsotheronly: an interface TweaksV does not know, and nothing else
	{
		const HRESULT c = getFactory(g_otherId, &otherFactory);
		swprintf(g_dstorageResult, size, L"dstorage: configuration 0x%lX/0x%lX, other interface only 0x%lX | ", static_cast<unsigned long>(a),
			static_cast<unsigned long>(b), static_cast<unsigned long>(c));
		return;
	}
	if (other == OtherInterface::First)	// dsotherinterface: first an interface TweaksV does not know
		getFactory(g_otherId, &otherFactory);
	void* factory = nullptr;
	const HRESULT c = getFactory(g_factoryId, &factory);
	int length = swprintf(g_dstorageResult, size, L"dstorage: configuration 0x%lX/0x%lX, factory 0x%lX %ls | ",
		static_cast<unsigned long>(a), static_cast<unsigned long>(b), static_cast<unsigned long>(c), factory ? L"set" : L"null");
	if (FAILED(c) || !factory || length < 0)
		return;

	const auto openFile = reinterpret_cast<OpenFileFn>((*reinterpret_cast<void***>(factory))[4]);
	g_factory = factory;
	g_openFile = openFile;
	wchar_t folder[MAX_PATH + 1] = {};
	GetCurrentDirectoryW(MAX_PATH, folder);
	wchar_t absolute[MAX_PATH + 64] = {}, prefixed[MAX_PATH + 64] = {}, sibling[MAX_PATH + 64] = {}, dotdot[MAX_PATH + 64] = {};
	swprintf(absolute, MAX_PATH + 64, L"%ls\\ds_update\\update.RPF", folder);
	swprintf(prefixed, MAX_PATH + 64, L"\\\\?\\%ls\\prefixed.txt", folder);		// in the game folder after all
	swprintf(sibling, MAX_PATH + 64, L"%ls2\\sibling.txt", folder);			// "...\TestRun2": not in the game folder
	swprintf(dotdot, MAX_PATH + 64, L"%ls\\..\\outside.txt", folder);		// out of it again
	// outside the game folder, like the game's saves (Documents\...\Profiles\...\SGTA50000) - here in the
	// folder above it, outside %TEMP% too - its cloud files in %TEMP% and a network path in its
	// \\?\UNC\ form: none of them is there
	wchar_t save[MAX_PATH + 64] = {};
	wcscpy_s(save, folder);
	if (wchar_t* const slash = wcsrchr(save, L'\\'))
		*slash = L'\0';
	wcscat_s(save, L"\\tweaksv_profile\\SGTA50000");
	wchar_t tempFolder[MAX_PATH + 1] = {}, temp[MAX_PATH + 64] = {}, tempArchive[MAX_PATH + 64] = {};
	GetTempPathW(MAX_PATH, tempFolder);
	swprintf(temp, MAX_PATH + 64, L"%lscloud_tweaksv.dat", tempFolder);
	swprintf(tempArchive, MAX_PATH + 64, L"%lsoutside.rpf", tempFolder);
	static wchar_t tooLong[3001];	// more than the 2047 characters TweaksV checks
	wmemset(tooLong, L'a', 3000);
	tooLong[3000] = L'\0';
	const struct { const wchar_t* name; const wchar_t* path; } files[] = { { L"rpf", L"x64a.rpf" }, { L"RPF", absolute },
		{ L"cache", L"rpf.cache" }, { L"txt", L"locktest.txt" }, { L"asi", L"TweaksV.asi" }, { L"none", L"noextension" },
		{ L"save", save }, { L"temp", temp }, { L"unc", L"\\\\?\\UNC\\localhost\\tweaksv_share\\cloud.dat" },
		{ L"slash", L"ds_dir/slash.txt" }, { L"prefixed", prefixed }, { L"sibling", sibling }, { L"dotdot", dotdot },
		{ L"rpftxt", L"x.rpf.txt" }, { L"CACHE", L"y.CACHE" }, { L"outrpf", tempArchive },
		{ L"unc2", L"\\\\?\\unc\\localhost\\tweaksv_share\\cloud2.dat" }, { L"long", tooLong }, { L"manifest", L"other.manifest" } };
	// the files in the game folder (TweaksV.asi is there already)
	const wchar_t* const made[] = { L"x64a.rpf", L"ds_update\\update.RPF", L"rpf.cache", L"locktest.txt", L"noextension",
		L"ds_dir\\slash.txt", L"prefixed.txt", L"x.rpf.txt", L"y.CACHE", L"other.manifest" };
	CreateDirectoryW(L"ds_update", nullptr);
	CreateDirectoryW(L"ds_dir", nullptr);
	for (const wchar_t* path : made)
		MakeFile(path);
	void* opened[sizeof(files) / sizeof(files[0])] = {};
	length += swprintf(g_dstorageResult + length, size - length, L"open:");
	for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++)
	{
		const HRESULT result = openFile(factory, files[i].path, g_factoryId, &opened[i]);
		length += swprintf(g_dstorageResult + length, size - length, L" %ls 0x%lX,", files[i].name, static_cast<unsigned long>(result));
	}
	swprintf(g_dstorageResult + length, size - length, L" reached %ld | ", openFileCalls());
	for (void* file : opened)
		g_closeFile(file);	// nullptr for a file not opened
	for (const wchar_t* path : made)
		DeleteFileW(path);
	RemoveDirectoryW(L"ds_update");
	RemoveDirectoryW(L"ds_dir");
	if (other == OtherInterface::Later)	// dsotherlater: an interface TweaksV does not know, after the known one
		getFactory(g_otherId, &otherFactory);
}

// dsflood: 20,001 more OpenFile calls - with DIAGNOSTIC_MODE, more than the 20,000 OpenFile lines TweaksV writes.
static wchar_t g_floodResult[64];
static void FloodOpenFile()
{
	if (!g_openFile)
		return;
	int calls = 0;
	for (int i = 0; i < 20001; i++)
	{
		void* file = nullptr;
		g_openFile(g_factory, L"locktest.txt", g_factoryId, &file);
		g_closeFile(file);
		calls++;
	}
	swprintf(g_floodResult, sizeof(g_floodResult) / sizeof(g_floodResult[0]), L"flood: %d calls | ", calls);
}

// Opens path as another program would and closes it again: 0 if it opened, else Windows' error.
static DWORD TryOpen(const wchar_t* path, DWORD access, DWORD share)
{
	const HANDLE file = CreateFileW(path, access, share, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return GetLastError();
	CloseHandle(file);
	return 0;
}

// Renames from to to and back again, as another program would: 0 if it could be renamed, else Windows' error.
static DWORD TryRename(const wchar_t* from, const wchar_t* to)
{
	if (!MoveFileExW(from, to, 0))
		return GetLastError();
	MoveFileExW(to, from, 0);
	return 0;
}

// dsshare (with dstorage): while the stand-in holds a file open, as DirectStorage holds the game's, other
// programs' opens of it. A text file in the game folder, opened by a relative path and by a full one: a write
// open that shares reading and a rename succeed, a write open that shares nothing fails (the limit: the
// stand-in reads it). An archive: a write open fails. A loose file in a subfolder, the way a mods loader
// serves it (onigiri\common\data\gameconfig.xml): opened, its bytes read through the stand-in's handle,
// shared; the same file by a \\?\ path, shared. A file in %TEMP% and one reached through "..", both outside
// the game folder: a write open fails. Each open's result, then the error of the other program's open (0:
// it succeeded).
static wchar_t g_shareResult[512];
static void ShareThroughStandIn()
{
	const size_t size = sizeof(g_shareResult) / sizeof(g_shareResult[0]);
	if (!g_openFile)
	{
		swprintf(g_shareResult, size, L"share: no stand-in | ");
		return;
	}
	wchar_t folder[MAX_PATH + 1] = {}, path[MAX_PATH * 2] = {};
	GetCurrentDirectoryW(MAX_PATH, folder);
	void* file = nullptr;

	MakeFile(L"ds_lock.txt");
	const HRESULT txt = g_openFile(g_factory, L"ds_lock.txt", g_factoryId, &file);
	const DWORD txtWrite = TryOpen(L"ds_lock.txt", GENERIC_WRITE, FILE_SHARE_READ);
	const DWORD txtRename = TryRename(L"ds_lock.txt", L"ds_lock_renamed.txt");
	const DWORD txtExclusive = TryOpen(L"ds_lock.txt", GENERIC_WRITE, 0);
	g_closeFile(file);
	swprintf(path, MAX_PATH * 2, L"%ls\\ds_lock.txt", folder);
	const HRESULT full = g_openFile(g_factory, path, g_factoryId, &file);
	const DWORD fullWrite = TryOpen(L"ds_lock.txt", GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	DeleteFileW(L"ds_lock.txt");

	MakeFile(L"ds_lock.rpf");
	const HRESULT rpf = g_openFile(g_factory, L"ds_lock.rpf", g_factoryId, &file);
	const DWORD rpfWrite = TryOpen(L"ds_lock.rpf", GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	DeleteFileW(L"ds_lock.rpf");

	const wchar_t* const loosePath = L"onigiri\\common\\data\\gameconfig.xml";
	CreateDirectoryW(L"onigiri", nullptr);
	CreateDirectoryW(L"onigiri\\common", nullptr);
	CreateDirectoryW(L"onigiri\\common\\data", nullptr);
	MakeFile(loosePath, "<gameconfig/>");
	const HRESULT loose = g_openFile(g_factory, loosePath, g_factoryId, &file);
	char bytes[32] = {};
	DWORD read = 0;
	const bool looseBytes = SUCCEEDED(loose) && ReadFile(g_fileHandle(file), bytes, sizeof(bytes) - 1, &read, nullptr) &&
		read == 13 && memcmp(bytes, "<gameconfig/>", 13) == 0;
	const DWORD looseWrite = TryOpen(loosePath, GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	swprintf(path, MAX_PATH * 2, L"\\\\?\\%ls\\%ls", folder, loosePath);
	const HRESULT prefixed = g_openFile(g_factory, path, g_factoryId, &file);
	const DWORD prefixedWrite = TryOpen(loosePath, GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	DeleteFileW(loosePath);
	RemoveDirectoryW(L"onigiri\\common\\data");
	RemoveDirectoryW(L"onigiri\\common");
	RemoveDirectoryW(L"onigiri");

	wchar_t temp[MAX_PATH + 64] = {};
	GetTempPathW(MAX_PATH, temp);
	wcscat_s(temp, L"tweaksv_ds_temp.txt");
	MakeFile(temp);
	const HRESULT tempOpen = g_openFile(g_factory, temp, g_factoryId, &file);
	const DWORD tempWrite = TryOpen(temp, GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	DeleteFileW(temp);

	// out of the game folder through "..", into the harness's %TEMP% next to it (object\TestTemp, common.sh)
	const wchar_t* const upPath = L"..\\TestTemp\\tweaksv_ds_up.txt";
	MakeFile(upPath);
	const HRESULT up = g_openFile(g_factory, upPath, g_factoryId, &file);
	const DWORD upWrite = TryOpen(upPath, GENERIC_WRITE, FILE_SHARE_READ);
	g_closeFile(file);
	DeleteFileW(upPath);

	swprintf(g_shareResult, size, L"share: txt 0x%lX write %lu rename %lu write-exclusive %lu, full 0x%lX write %lu, rpf 0x%lX write %lu, "
		L"loose 0x%lX bytes %ls write %lu, prefixed 0x%lX write %lu, temp 0x%lX write %lu, up 0x%lX write %lu | ",
		static_cast<unsigned long>(txt), txtWrite, txtRename, txtExclusive, static_cast<unsigned long>(full), fullWrite,
		static_cast<unsigned long>(rpf), rpfWrite, static_cast<unsigned long>(loose), looseBytes ? L"ok" : L"WRONG", looseWrite,
		static_cast<unsigned long>(prefixed), prefixedWrite, static_cast<unsigned long>(tempOpen), tempWrite,
		static_cast<unsigned long>(up), upWrite);
}

// NtCreateFile as a program that calls it directly: path relative to root or, without root, made full
// ("\??\C:\..."). The handle, or nullptr.
using NtCreateFileFn = LONG(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER,
	ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
static HANDLE OpenNt(const wchar_t* path, HANDLE root, ACCESS_MASK access, ULONG share, ULONG options)
{
	const auto create = reinterpret_cast<NtCreateFileFn>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateFile"));
	wchar_t name[MAX_PATH * 2] = {};
	if (root)
		wcscpy_s(name, path);
	else
	{
		wchar_t full[MAX_PATH] = {};
		GetFullPathNameW(path, MAX_PATH, full, nullptr);
		swprintf(name, MAX_PATH * 2, L"\\??\\%ls", full);
	}
	UNICODE_STRING string{};
	string.Length = string.MaximumLength = static_cast<USHORT>(wcslen(name) * sizeof(wchar_t));
	string.Buffer = name;
	OBJECT_ATTRIBUTES attributes{};
	InitializeObjectAttributes(&attributes, &string, OBJ_CASE_INSENSITIVE, root, nullptr);
	IO_STATUS_BLOCK io{};
	HANDLE handle = nullptr;
	if (!create || create(&handle, access | SYNCHRONIZE, &attributes, &io, nullptr, FILE_ATTRIBUTE_NORMAL, share, FILE_OPEN,
		options | FILE_SYNCHRONOUS_IO_NONALERT, nullptr, 0) < 0)
		return nullptr;
	return handle;
}

// ownshare: the fake's own opens in its folder, also without the stand-in (dstorage.dll not loaded), and
// other programs' opens next to them. A read-only open that shares reading is given write and delete sharing:
// a write open that shares reading succeeds while it is held. An exclusive read-only open stays exclusive: a
// second open fails. A folder opened as one (FILE_DIRECTORY_FILE) is left as it is: it cannot be renamed while
// held. A name relative to another folder's handle is left as it is: a write open fails. A write open is left
// as it is: a second write open fails, as its sharing leaves writing out. 9999: the fake's own open failed.
static wchar_t g_ownResult[256];
static void ShareOwnOpens()
{
	MakeFile(L"ds_own.txt");
	HANDLE held = CreateFileW(L"ds_own.txt", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	const DWORD widened = held != INVALID_HANDLE_VALUE ? TryOpen(L"ds_own.txt", GENERIC_WRITE, FILE_SHARE_READ) : 9999;
	if (held != INVALID_HANDLE_VALUE)
		CloseHandle(held);
	DeleteFileW(L"ds_own.txt");

	MakeFile(L"ds_excl.txt");
	held = CreateFileW(L"ds_excl.txt", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	const DWORD exclusive = held != INVALID_HANDLE_VALUE ? TryOpen(L"ds_excl.txt", GENERIC_READ, FILE_SHARE_READ) : 9999;
	if (held != INVALID_HANDLE_VALUE)
		CloseHandle(held);
	DeleteFileW(L"ds_excl.txt");

	CreateDirectoryW(L"ds_own_dir", nullptr);
	MakeFile(L"ds_own_dir\\inner.txt");
	const HANDLE folder = OpenNt(L"ds_own_dir", nullptr, FILE_LIST_DIRECTORY, FILE_SHARE_READ, FILE_DIRECTORY_FILE);
	const DWORD folderRename = folder ? TryRename(L"ds_own_dir", L"ds_own_dir2") : 9999;
	if (folder)
		CloseHandle(folder);
	const HANDLE root = OpenNt(L"ds_own_dir", nullptr, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_DIRECTORY_FILE);
	const HANDLE rooted = root ? OpenNt(L"inner.txt", root, GENERIC_READ, FILE_SHARE_READ, FILE_NON_DIRECTORY_FILE) : nullptr;
	const DWORD rootedWrite = rooted ? TryOpen(L"ds_own_dir\\inner.txt", GENERIC_WRITE, FILE_SHARE_READ) : 9999;
	if (rooted)
		CloseHandle(rooted);
	if (root)
		CloseHandle(root);
	DeleteFileW(L"ds_own_dir\\inner.txt");
	RemoveDirectoryW(L"ds_own_dir");

	MakeFile(L"ds_w.txt");
	held = CreateFileW(L"ds_w.txt", GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	const DWORD writeOpen = held != INVALID_HANDLE_VALUE ? TryOpen(L"ds_w.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE) : 9999;
	if (held != INVALID_HANDLE_VALUE)
		CloseHandle(held);
	DeleteFileW(L"ds_w.txt");

	swprintf(g_ownResult, sizeof(g_ownResult) / sizeof(g_ownResult[0]),
		L"own: widened write %lu, exclusive %lu, folder rename %lu, rooted write %lu, write-open %lu | ",
		widened, exclusive, folderRename, rootedWrite, writeOpen);
}

// dstypes: read-only opens of 17 files in its folder that are not there, each of another type - 16 short ones
// and one too long for TweaksV's table of types: more types than its summary lists.
static void OpenTypes()
{
	for (int i = 1; i <= 17; i++)
	{
		wchar_t name[32];
		if (i <= 16)
			swprintf(name, 32, L"ds_type.e%02d", i);
		else
			wcscpy_s(name, L"ds_type.toolongx");
		const HANDLE file = CreateFileW(name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
			CloseHandle(file);
	}
}

// dsopenflood: 100,001 opens in the game folder of files that are not there, each by another name - more than
// the tables of TweaksV's open summary take, and more files than its UNLOCK_FILES summary tells apart.
static void FloodOpens()
{
	for (int i = 0; i < 100001; i++)
	{
		wchar_t name[32];
		swprintf(name, sizeof(name) / sizeof(name[0]), L"dsflood_%06d.txt", i);
		const HANDLE file = CreateFileW(name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
			CloseHandle(file);
	}
}

// The replace-renames (TweaksV's UNLOCK_FILES on NtSetInformationFile): ds_held.txt in the game folder, holding
// "old", held open by the stand-in as DirectStorage holds the game's files, and written while it is held, one of
// the ways a program saves a file at a time; ds_new.txt holds "new". What a way gave - 0 when it went through,
// else Windows' error, or NtSetInformationFile's status - then what the file's name holds afterwards and what
// the stand-in's handle reads: "new/old" is a replace, which the holder sees once it opens the file again,
// "new/new" a write in place, "old/old" nothing done.
static const wchar_t* Content(HANDLE file)
{
	char bytes[8] = {};
	DWORD read = 0;
	if (!ReadFile(file, bytes, sizeof(bytes) - 1, &read, nullptr))
		return L"none";
	return read == 3 && memcmp(bytes, "old", 3) == 0 ? L"old" : read == 3 && memcmp(bytes, "new", 3) == 0 ? L"new" : L"?";
}

static const wchar_t* ContentByName(const wchar_t* path)
{
	const HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return L"none";
	const wchar_t* const content = Content(file);
	CloseHandle(file);
	return content;
}

struct HeldWrite
{
	DWORD result;			// 0: it went through; 9999: the stand-in could not hold the file
	const wchar_t* named;	// what ds_held.txt holds afterwards
	const wchar_t* held;	// what the stand-in's handle reads
};

// ds_held.txt written by write while the stand-in holds it - made read-only first with readOnly.
static HeldWrite WriteHeld(DWORD (*write)(), bool readOnly = false)
{
	HeldWrite outcome = { 9999, L"none", L"none" };
	MakeFile(L"ds_held.txt", "old");
	MakeFile(L"ds_new.txt", "new");
	void* file = nullptr;
	if (g_openFile && SUCCEEDED(g_openFile(g_factory, L"ds_held.txt", g_factoryId, &file)) && file)
	{
		if (readOnly)
			SetFileAttributesW(L"ds_held.txt", FILE_ATTRIBUTE_READONLY);
		outcome.result = write();
		outcome.named = ContentByName(L"ds_held.txt");
		const HANDLE handle = g_fileHandle(file);
		const LARGE_INTEGER start{};
		outcome.held = SetFilePointerEx(handle, start, nullptr, FILE_BEGIN) ? Content(handle) : L"none";
		g_closeFile(file);
	}
	SetFileAttributesW(L"ds_held.txt", FILE_ATTRIBUTE_NORMAL);
	DeleteFileW(L"ds_held.txt");
	DeleteFileW(L"ds_new.txt");
	return outcome;
}

// The classic replace: MoveFileExW with MOVEFILE_REPLACE_EXISTING, the usual safe way to save a file.
static DWORD ClassicReplace()
{
	return MoveFileExW(L"ds_new.txt", L"ds_held.txt", MOVEFILE_REPLACE_EXISTING) ? 0 : GetLastError();
}

// A POSIX rename: SetFileInformationByHandle with FileRenameInfoEx, replace-if-exists and POSIX semantics.
static DWORD PosixRename()
{
	const HANDLE file = CreateFileW(L"ds_new.txt", DELETE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return GetLastError();
	wchar_t target[MAX_PATH + 1] = {};
	const DWORD length = GetFullPathNameW(L"ds_held.txt", MAX_PATH + 1, target, nullptr);
	alignas(FILE_RENAME_INFO) unsigned char buffer[sizeof(FILE_RENAME_INFO) + (MAX_PATH + 1) * sizeof(wchar_t)] = {};
	FILE_RENAME_INFO* const info = reinterpret_cast<FILE_RENAME_INFO*>(buffer);
	info->Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS;
	info->FileNameLength = length * sizeof(wchar_t);
	memcpy(info->FileName, target, (length + 1) * sizeof(wchar_t));
	const DWORD error = SetFileInformationByHandle(file, FileRenameInfoEx, info, sizeof(buffer)) ? 0 : GetLastError();
	CloseHandle(file);
	return error;
}

static DWORD ReplaceHeld()
{
	return ReplaceFileW(L"ds_held.txt", L"ds_new.txt", nullptr, 0, nullptr, nullptr) ? 0 : GetLastError();
}

// A write in place: the file opened for writing, emptied and written, as the C runtime's fopen with "w" does.
static DWORD WriteInPlace()
{
	const HANDLE file = CreateFileW(L"ds_held.txt", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return GetLastError();
	DWORD written = 0;
	const DWORD error = WriteFile(file, "new", 3, &written, nullptr) ? 0 : GetLastError();
	CloseHandle(file);
	return error;
}

// The classic replace from another program, outside the game (renamer.exe, fake_renamer.cpp): its exit code, the
// error it got; 9998 if it cannot be started, 9997 if it does not end within 10 seconds.
static DWORD ClassicReplaceOutside()
{
	wchar_t command[] = L"renamer.exe ds_new.txt ds_held.txt";
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	if (!CreateProcessW(nullptr, command, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process))
		return 9998;
	DWORD code = 9997;
	if (WaitForSingleObject(process.hProcess, 10000) == WAIT_OBJECT_0)
		GetExitCodeProcess(process.hProcess, &code);
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	return code;
}

// dsreplace (with dstorage): the four ways of writing a held file from inside the game - the classic replace, a
// POSIX rename, ReplaceFile, a write in place - and the classic replace from another program.
static wchar_t g_replaceResult[256];
static void ReplaceHeldFile()
{
	const size_t size = sizeof(g_replaceResult) / sizeof(g_replaceResult[0]);
	if (!g_openFile)
	{
		swprintf(g_replaceResult, size, L"replace: no stand-in | ");
		return;
	}
	const HeldWrite classic = WriteHeld(ClassicReplace), posix = WriteHeld(PosixRename), replace = WriteHeld(ReplaceHeld),
		inPlace = WriteHeld(WriteInPlace), other = WriteHeld(ClassicReplaceOutside);
	swprintf(g_replaceResult, size, L"replace: classic %lu %ls/%ls, posix %lu %ls/%ls, replacefile %lu %ls/%ls, inplace %lu %ls/%ls, "
		L"other %lu %ls/%ls | ", classic.result, classic.named, classic.held, posix.result, posix.named, posix.held, replace.result,
		replace.named, replace.held, inPlace.result, inPlace.named, inPlace.held, other.result, other.named, other.held);
}

// NtSetInformationFile as a program that calls it directly - ntdll's, which passes TweaksV's hook as every rename
// in the game does: from, opened for deleting, renamed to name (nameLength characters), relative to root when
// given, with the class and flags asked for (FileRenameInformation's ReplaceIfExists is the first byte of flags).
// The status, 0x9999 if from cannot be opened. The last error set before the call must be there after it.
using NtSetInformationFileFn = LONG(NTAPI*)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, ULONG);
static bool g_lastErrorKept = true;
static LONG RenameNt(const wchar_t* from, const wchar_t* name, size_t nameLength, HANDLE root, ULONG informationClass, ULONG flags)
{
	const auto setInformation = reinterpret_cast<NtSetInformationFileFn>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),
		"NtSetInformationFile"));
	const HANDLE file = CreateFileW(from, DELETE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (!setInformation || file == INVALID_HANDLE_VALUE)
		return 0x9999;
	// FILE_RENAME_INFORMATION: the flags at 0, the folder handle at 8, the name's length in bytes at 16, the name at 20
	alignas(8) static unsigned char buffer[24 + 4096 * sizeof(wchar_t)];
	const ULONG bytes = static_cast<ULONG>(nameLength * sizeof(wchar_t));
	memset(buffer, 0, 24);
	memcpy(buffer, &flags, sizeof(flags));
	memcpy(buffer + 8, &root, sizeof(root));
	memcpy(buffer + 16, &bytes, sizeof(bytes));
	memcpy(buffer + 20, name, bytes);
	IO_STATUS_BLOCK io{};
	SetLastError(0xA5A5);
	const LONG status = setInformation(file, &io, buffer, 20 + bytes < 24 ? 24 : 20 + bytes, informationClass);
	if (GetLastError() != 0xA5A5)
		g_lastErrorKept = false;
	CloseHandle(file);
	return status;
}

constexpr ULONG kRenameInformation = 10, kRenameInformationEx = 65;
static wchar_t g_heldNtName[MAX_PATH * 2];			// \??\<the game folder>\ds_held.txt
static HANDLE g_gameFolderHandle = INVALID_HANDLE_VALUE;

// dsrenames (with dstorage): the renames TweaksV's hook sees, against the held file unless said otherwise - onto a
// file outside the game folder that the fake holds with every sharing (MoveFileExW, refused if passed on: 5), one
// without replace-if-exists (MoveFileExW without MOVEFILE_REPLACE_EXISTING: 183), one that asks for POSIX semantics
// itself, then the classic ones TweaksV turns: by a bare name, by a name relative to the game folder's handle, and
// FileRenameInformationEx with replace-if-exists and FILE_RENAME_IGNORE_READONLY_ATTRIBUTE onto the held file made
// read-only, which only goes through with that flag kept; onto a file in the game folder that nothing holds; and a
// name longer than TweaksV copies (2100 characters in the game folder), passed on as made.
static wchar_t g_renamesResult[384];
static void RenameAsked()
{
	const size_t size = sizeof(g_renamesResult) / sizeof(g_renamesResult[0]);
	if (!g_openFile)
	{
		swprintf(g_renamesResult, size, L"renames: no stand-in | ");
		return;
	}
	wchar_t folder[MAX_PATH + 1] = {};
	GetCurrentDirectoryW(MAX_PATH, folder);
	swprintf(g_heldNtName, MAX_PATH * 2, L"\\??\\%ls\\ds_held.txt", folder);
	g_gameFolderHandle = CreateFileW(L".", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

	wchar_t temp[MAX_PATH + 1] = {}, outsideHeld[MAX_PATH + 64] = {}, outsideNew[MAX_PATH + 64] = {};
	GetTempPathW(MAX_PATH, temp);
	swprintf(outsideHeld, MAX_PATH + 64, L"%lstweaksv_ds_held.txt", temp);
	swprintf(outsideNew, MAX_PATH + 64, L"%lstweaksv_ds_new.txt", temp);
	MakeFile(outsideHeld, "old");
	MakeFile(outsideNew, "new");
	const HANDLE outside = CreateFileW(outsideHeld, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	DWORD outsideResult = 9999;
	if (outside != INVALID_HANDLE_VALUE)
	{
		outsideResult = MoveFileExW(outsideNew, outsideHeld, MOVEFILE_REPLACE_EXISTING) ? 0 : GetLastError();
		CloseHandle(outside);
	}
	DeleteFileW(outsideHeld);
	DeleteFileW(outsideNew);

	const HeldWrite noReplace = WriteHeld([]() -> DWORD { return MoveFileExW(L"ds_new.txt", L"ds_held.txt", 0) ? 0 : GetLastError(); });
	const HeldWrite posix = WriteHeld([]() -> DWORD {
		return static_cast<DWORD>(RenameNt(L"ds_new.txt", g_heldNtName, wcslen(g_heldNtName), nullptr, kRenameInformationEx, 0x3)); });
	const HeldWrite bare = WriteHeld([]() -> DWORD {
		return static_cast<DWORD>(RenameNt(L"ds_new.txt", L"ds_held.txt", 11, nullptr, kRenameInformation, 1)); });
	const HeldWrite relative = WriteHeld([]() -> DWORD {
		return static_cast<DWORD>(RenameNt(L"ds_new.txt", L"ds_held.txt", 11, g_gameFolderHandle, kRenameInformation, 1)); });
	const HeldWrite ex = WriteHeld([]() -> DWORD {
		return static_cast<DWORD>(RenameNt(L"ds_new.txt", g_heldNtName, wcslen(g_heldNtName), nullptr, kRenameInformationEx, 0x41)); },
		true);

	wchar_t freeName[MAX_PATH * 2] = {};
	swprintf(freeName, MAX_PATH * 2, L"\\??\\%ls\\ds_free.txt", folder);
	MakeFile(L"ds_free.txt", "old");
	MakeFile(L"ds_new.txt", "new");
	const LONG freeResult = RenameNt(L"ds_new.txt", freeName, wcslen(freeName), nullptr, kRenameInformation, 1);
	const wchar_t* const freeNamed = ContentByName(L"ds_free.txt");
	DeleteFileW(L"ds_free.txt");
	DeleteFileW(L"ds_new.txt");

	static wchar_t longName[MAX_PATH + 2200];
	const int prefix = swprintf(longName, MAX_PATH + 2200, L"\\??\\%ls\\", folder);
	LONG longResult = 0x9999;
	MakeFile(L"ds_new.txt", "new");
	if (prefix > 0)
	{
		wmemset(longName + prefix, L'a', 2100);
		longName[prefix + 2100] = L'\0';
		longResult = RenameNt(L"ds_new.txt", longName, wcslen(longName), nullptr, kRenameInformation, 1);
	}
	DeleteFileW(L"ds_new.txt");
	if (g_gameFolderHandle != INVALID_HANDLE_VALUE)
		CloseHandle(g_gameFolderHandle);

	swprintf(g_renamesResult, size, L"renames: outside %lu, noreplace %lu %ls/%ls, posix 0x%lX %ls/%ls, bare 0x%lX %ls/%ls, "
		L"relative 0x%lX %ls/%ls, ex 0x%lX %ls/%ls, free 0x%lX %ls, long 0x%lX, last error %ls | ", outsideResult, noReplace.result,
		noReplace.named, noReplace.held, posix.result, posix.named, posix.held, bare.result, bare.named, bare.held, relative.result,
		relative.named, relative.held, ex.result, ex.named, ex.held, static_cast<unsigned long>(freeResult), freeNamed,
		static_cast<unsigned long>(longResult), g_lastErrorKept ? L"kept" : L"CHANGED");
}

// dsrenameflood: 20,001 classic replace-renames in the game folder, ds_flood_a.txt onto ds_flood_b.txt and back -
// with DIAGNOSTIC_MODE, more than the 20,000 rename lines TweaksV writes.
static wchar_t g_renameFloodResult[64];
static void FloodRenames()
{
	MakeFile(L"ds_flood_a.txt");
	MakeFile(L"ds_flood_b.txt");
	int renamed = 0;
	for (int i = 0; i < 20001; i++)
	{
		const bool even = i % 2 == 0;
		if (MoveFileExW(even ? L"ds_flood_a.txt" : L"ds_flood_b.txt", even ? L"ds_flood_b.txt" : L"ds_flood_a.txt",
			MOVEFILE_REPLACE_EXISTING))
			renamed++;
	}
	DeleteFileW(L"ds_flood_a.txt");
	DeleteFileW(L"ds_flood_b.txt");
	swprintf(g_renameFloodResult, sizeof(g_renameFloodResult) / sizeof(g_renameFloodResult[0]), L"rename flood: %d renamed | ", renamed);
}

// File opens in the game folder (TweaksV's DIAGNOSTIC_MODE): one file 6 times with the same sharing,
// a missing file, the folder itself, a file in a subfolder and a file outside the game folder.
static wchar_t g_fileOpensResult[160];
static void OpenFiles()
{
	HANDLE file = CreateFileW(L"ds_test.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	for (int i = 0; i < 5; i++)
	{
		file = CreateFileW(L"ds_test.txt", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
			CloseHandle(file);
	}
	SetLastError(0);
	file = CreateFileW(L"ds_missing.txt", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	const DWORD missingError = file == INVALID_HANDLE_VALUE ? GetLastError() : 0;
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	file = CreateFileW(L".", FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	CreateDirectoryW(L"ds_sub", nullptr);
	file = CreateFileW(L"ds_sub\\inner.rpf", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file != INVALID_HANDLE_VALUE)
		CloseHandle(file);
	wchar_t outside[MAX_PATH + 32];
	const DWORD length = GetTempPathW(MAX_PATH, outside);
	if (length > 0 && length < MAX_PATH)
	{
		wcscat_s(outside, L"tweaksv_ds_outside.txt");
		file = CreateFileW(outside, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file != INVALID_HANDLE_VALUE)
			CloseHandle(file);
		DeleteFileW(outside);
	}
	DeleteFileW(L"ds_test.txt");
	DeleteFileW(L"ds_sub\\inner.rpf");
	RemoveDirectoryW(L"ds_sub");
	swprintf(g_fileOpensResult, 160, L"file opens: missing file error %lu | ", static_cast<unsigned long>(missingError));
}

// The script programs (TweaksVScripts, TEST_DUMP_SCRIPT, the Blips part): blip_controller with its code over two
// pages of 16 KB, the second holding 0x123 bytes, three natives and 0x1F0 bytes of strings in one page; other_script
// and blip_imposter - blip_controller's hash under another name - with code and strings of their own. Every page is
// a VirtualAlloc of its own, so that one can be made unreadable. The placements run on a thread of their own,
// as the game's resource placement thread places programs, each through the call at the hook site.
constexpr uint32_t kScriptPage = 0x4000;
constexpr uint32_t kScriptCodeLength = kScriptPage + 0x123;
constexpr uint32_t kScriptStringLength = 0x1F0;
constexpr uint32_t kBlipControllerHash = 0x80CFE974;	// the game's hash of "blip_controller"
struct ScriptStorage
{
	FakeScriptProgram program{};
	unsigned char* codePages[2] = {};
	unsigned char* stringPages[1] = {};
	uint64_t natives[kFakeNatives] = {};
};
static ScriptStorage g_blipController, g_otherScript, g_blipImposter;
struct ScriptPlacement { ScriptStorage* script; bool unknownNative; uint32_t codeLength; uint32_t nativeCount; };
static ScriptPlacement g_scriptPlan[20];
static int g_scriptPlanned = 0;
static int g_scriptPlaced = 0, g_scriptResolved = 0, g_scriptErrorKept = 0;
static DWORD g_scriptThread = 0;
static bool g_scriptPageUnreadable = false, g_scriptGap = false;
static wchar_t g_scriptResult[256];

static void MakeScript(ScriptStorage& script, const char* name, uint32_t hash, unsigned char seed)
{
	FakeScriptProgram& p = script.program;
	p.vtable = 0x1111111111111111ull;
	p.unknown08 = 0x0808080808080808ull;
	p.unknown18 = 0x18181818;
	for (uint32_t& value : p.unknown20)
		value = 0x20202020;
	p.unknown30[0] = p.unknown30[1] = 0x3030303030303030ull;
	p.unknown48[0] = p.unknown48[1] = 0x4848484848484848ull;
	p.unknown5C = 0x5C5C5C5C;
	for (uint32_t& value : p.unknown74)
		value = 0x74747474;
	for (unsigned char*& page : script.codePages)
		page = static_cast<unsigned char*>(VirtualAlloc(nullptr, kScriptPage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	script.stringPages[0] = static_cast<unsigned char*>(VirtualAlloc(nullptr, kScriptPage, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	for (uint32_t i = 0; i < kScriptPage; i++)
	{
		script.codePages[0][i] = static_cast<unsigned char>(i * 7 + seed);
		script.codePages[1][i] = static_cast<unsigned char>(i * 13 + seed + 1);
		script.stringPages[0][i] = static_cast<unsigned char>('a' + (i + seed) % 26);
	}
	p.codePages = script.codePages;
	p.codeLength = kScriptCodeLength;
	p.nativeCount = static_cast<uint32_t>(kFakeNatives);
	p.natives = script.natives;
	p.nameHash = hash;
	p.name = name;
	p.stringPages = script.stringPages;
	p.stringLength = kScriptStringLength;
}

static void PlanScript(ScriptStorage& script, bool unknownNative = false, uint32_t codeLength = kScriptCodeLength,
	uint32_t nativeCount = static_cast<uint32_t>(kFakeNatives))
{
	if (g_scriptPlanned < static_cast<int>(sizeof(g_scriptPlan) / sizeof(g_scriptPlan[0])))
		g_scriptPlan[g_scriptPlanned++] = { &script, unknownNative, codeLength, nativeCount };
}

// blip_controller's guard (the Blips part), at 0x3FE of its code: its bytes there - the guard, then its way out, a
// NATIVE that makes a blip, and LEAVE - and where its branch lies, for each guard an option asks for. The part may
// change the bytes from the end of the count's NATIVE up to the branch, and a fused branch's opcode; nothing else.
struct BlipGuard
{
	const wchar_t* option;
	unsigned char bytes[40];
	uint32_t size;
	uint32_t branch;
	bool fused;
};
constexpr uint32_t kBlipGuard = 0x3FE;
constexpr uint32_t kGuardSize = 0x15;	// the game's guard: 21 bytes, its way out at 0x413
static const BlipGuard kBlipGuards[] = {
	// as the game has it: the count (native 1), PUSH_CONST_U8 150, ILT, NOT, JZ +10; PUSH_CONST_S16 2000, WAIT (native 2), J -21
	{ L"", { 0x2C, 0x01, 0x00, 0x01, 0x25, 0x96, 0x0C, 0x06, 0x56, 0x0A, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00, 0x02, 0x55, 0xEB, 0xFF,
		0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 28, 0x406, false },
	{ L"blip240", { 0x2C, 0x01, 0x00, 0x01, 0x25, 0xF0, 0x0C, 0x06, 0x56, 0x0A, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00, 0x02, 0x55, 0xEB, 0xFF,
		0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 28, 0x406, false },
	// WAIT through native 3, which the program's table of three does not have
	{ L"blipnative", { 0x2C, 0x01, 0x00, 0x01, 0x25, 0x96, 0x0C, 0x06, 0x56, 0x0A, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00, 0x03, 0x55, 0xEB,
		0xFF, 0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 28, 0x406, false },
	// two NOPs after the count, JZ +10 at 0x408, J -23
	{ L"blippadded", { 0x2C, 0x01, 0x00, 0x01, 0x00, 0x00, 0x25, 0x96, 0x0C, 0x06, 0x56, 0x0A, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00, 0x02,
		0x55, 0xE9, 0xFF, 0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 30, 0x408, false },
	// IGE and JZ +10 at 0x405, no NOT, J -20
	{ L"bliproom", { 0x2C, 0x01, 0x00, 0x01, 0x25, 0x96, 0x0B, 0x56, 0x0A, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00, 0x02, 0x55, 0xEC, 0xFF,
		0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 27, 0x405, false },
	// two NOPs, PUSH_CONST_U8 150, ILT_JZ +3 at 0x406 to the body, J +12 to the way out at 0x418; the body, J -24; two NOPs
	{ L"blipfused", { 0x2C, 0x01, 0x00, 0x01, 0x00, 0x00, 0x25, 0x96, 0x5B, 0x03, 0x00, 0x55, 0x0C, 0x00, 0x43, 0xD0, 0x07, 0x2C, 0x04, 0x00,
		0x02, 0x55, 0xE8, 0xFF, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00 }, 33, 0x406, true },
};
static const BlipGuard* g_blipGuard = &kBlipGuards[0];
static bool g_blipTwice = false, g_blipJump = false, g_blipCall = false, g_blipSwitch = false, g_blipCatch = false, g_blipBadByte = false;
// What TweaksV left of what the fake placed: blip_controller's placements and those whose code is as placed but where
// the part may change it, its guard through the branch after the last one, and the other programs' placements and
// those whose code is as placed
static int g_blipPlaced = 0, g_blipAsPlaced = 0, g_otherPlaced = 0, g_otherAsPlaced = 0;
static unsigned char g_blipGuardAfter[40];
static uint32_t g_blipGuardShown = 0;
static wchar_t g_blipResult[256];

// True if a program's code is as the fake placed it, but the bytes from..to of its first page; its second page only
// while it can be read.
static bool AsPlaced(const ScriptStorage& script, const unsigned char (&placed)[kScriptCodeLength], uint32_t from, uint32_t to)
{
	for (uint32_t i = 0; i < kScriptPage; i++)
		if ((i < from || i >= to) && script.codePages[0][i] != placed[i])
			return false;
	return g_scriptPageUnreadable || memcmp(script.codePages[1], placed + kScriptPage, kScriptCodeLength - kScriptPage) == 0;
}

// blip_controller's code, decodable from its start to its end with the game's opcode lengths, as the game's own is
// (PATTERNS.md, "Script programs"): ENTER and instructions of every kind a decoder must step over - a NATIVE, the
// pushes, a CALL, a jump over three NOPs, a SWITCH whose entries hold bytes above the last opcode - then LEAVE; a
// second function whose ENTER's name holds the game's guard, which only a reader of bytes, not of instructions,
// would take for a second guard; a third function with the guard at 0x3FE - its count native index 1 and WAIT index
// 2, as the fake's program has three natives - and its way out, a NATIVE that makes a blip; NOPs to the end of each
// page, every instruction within its page, and LEAVE last. The options add a second guard at 0x800, a jump, a CALL
// or a SWITCH case at 0x100 into the guard's span, a CATCH or the byte 0xA0 at 0x200. Laid again before each
// placement, as the game makes a new program each time - its second page only while it can be read.
static void LayBlipCode(ScriptStorage& script)
{
	static const unsigned char head[] = {
		0x2D, 0x00, 0x02, 0x00, 0x00,				// ENTER, 2 locals, no name
		0x2C, 0x00, 0x00, 0x00,						// NATIVE, no argument, no result, index 0
		0x25, 0x07, 0x26, 0x01, 0x02, 0x27, 0x01, 0x02, 0x03,		// PUSH_CONST_U8, _U8_U8, _U8_U8_U8
		0x28, 0xE8, 0x03, 0x00, 0x00, 0x29, 0x00, 0x00, 0x80, 0x3F,	// PUSH_CONST_U32 1000, PUSH_CONST_F 1.0
		0x5D, 0x00, 0x00, 0x00,						// CALL 0
		0x64, 0x00, 0x01, 0x00,						// PUSH_CONST_U24 256
		0x55, 0x03, 0x00, 0x00, 0x00, 0x00,			// J +3, over three NOPs
		0x65, 0x02, 0xAA, 0xBB, 0xCC, 0xDD, 0x06, 0x00, 0x99, 0x88, 0x77, 0x66, 0x00, 0x00,	// SWITCH, two entries, both to the LEAVE
		0x2E, 0x00, 0x00,							// LEAVE
		0x2D, 0x00, 0x00, 0x00, 0x15 };				// ENTER, its name of 21 bytes the guard's
	static const unsigned char second[] = { 0x2E, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x00 };	// LEAVE; ENTER, the third function
	static const unsigned char jump[] = { 0x55, 0xFF, 0x02 };									// J +0x2FF, from 0x100 to 0x402
	static const unsigned char call[] = { 0x5D, 0x02, 0x04, 0x00 };								// CALL 0x402, from 0x100
	static const unsigned char cases[] = { 0x65, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x02 };	// SWITCH, one case: 0x108 + 0x2FA = 0x402
	static const unsigned char leave[] = { 0x2E, 0x00, 0x00 };
	unsigned char* const code = script.codePages[0];
	memset(code, 0x00, kScriptPage);
	memcpy(code, head, sizeof(head));
	memcpy(code + sizeof(head), kBlipGuards[0].bytes, kGuardSize);
	memcpy(code + sizeof(head) + kGuardSize, second, sizeof(second));
	memcpy(code + kBlipGuard, g_blipGuard->bytes, g_blipGuard->size);
	if (g_blipTwice)
		memcpy(code + 0x800, kBlipGuards[0].bytes, kBlipGuards[0].size);
	if (g_blipJump)
		memcpy(code + 0x100, jump, sizeof(jump));
	if (g_blipCall)
		memcpy(code + 0x100, call, sizeof(call));
	if (g_blipSwitch)
		memcpy(code + 0x100, cases, sizeof(cases));
	if (g_blipCatch)
		code[0x200] = 0x6D;	// CATCH
	if (g_blipBadByte)
		code[0x200] = 0xA0;
	if (g_scriptPageUnreadable)
		return;
	memset(script.codePages[1], 0x00, kScriptPage);
	memcpy(script.codePages[1] + (kScriptCodeLength - kScriptPage) - sizeof(leave), leave, sizeof(leave));
}

// What a copy of blip_controller's n-th placement must hold after its header: the program object, the code as the
// fake placed it, before the call (the unreadable page as zeros), the natives before and after the game's resolution,
// the strings.
static void WriteExpected(int n, const ScriptStorage& script, const unsigned char (&placed)[kScriptCodeLength],
	const uint64_t (&before)[kFakeNatives])
{
	wchar_t name[48];
	swprintf(name, 48, L"script_placed_%d.bin", n);
	const HANDLE file = CreateFileW(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
		return;
	DWORD written = 0;
	WriteFile(file, &script.program, sizeof(script.program), &written, nullptr);
	WriteFile(file, placed, sizeof(placed), &written, nullptr);
	WriteFile(file, before, sizeof(before), &written, nullptr);
	WriteFile(file, script.natives, sizeof(script.natives), &written, nullptr);
	WriteFile(file, script.stringPages[0], kScriptStringLength, &written, nullptr);
	CloseHandle(file);
}

static DWORD WINAPI PlaceScripts(void*)
{
	g_scriptThread = GetCurrentThreadId();
	using Resolve = unsigned char (*)(unsigned char*);
	const auto resolve = reinterpret_cast<Resolve>(const_cast<unsigned char*>(Rel32Target(g_scriptCallSite)));
	int blipPlacements = 0;
	for (int i = 0; i < g_scriptPlanned; i++)
	{
		if (i > 0 && g_scriptGap)
			Sleep(3000);
		ScriptStorage& script = *g_scriptPlan[i].script;
		FakeScriptProgram& p = script.program;
		// as the game's constructor leaves the program before the call: a new program's code, the natives' hashes in
		// the table - and what it placed, which TweaksV may change in the call
		uint64_t before[kFakeNatives] = {};
		for (size_t k = 0; k < kFakeNatives; k++)
			before[k] = script.natives[k] = g_fakeNatives[k].hash;
		if (g_scriptPlan[i].unknownNative)
			before[1] = script.natives[1] = kUnknownNativeHash;
		p.codeLength = g_scriptPlan[i].codeLength;
		p.nativeCount = g_scriptPlan[i].nativeCount;
		if (&script == &g_blipController)
			LayBlipCode(script);
		static unsigned char placed[kScriptCodeLength];
		memcpy(placed, script.codePages[0], kScriptPage);
		if (g_scriptPageUnreadable)
			memset(placed + kScriptPage, 0, kScriptCodeLength - kScriptPage);
		else
			memcpy(placed + kScriptPage, script.codePages[1], kScriptCodeLength - kScriptPage);
		SetLastError(0);
		const unsigned char resolved = resolve(reinterpret_cast<unsigned char*>(&p));
		if (GetLastError() == kResolveLastError)
			g_scriptErrorKept++;
		g_scriptPlaced++;
		if (resolved == 1)
			g_scriptResolved++;
		if (&script == &g_blipController)
		{
			blipPlacements++;
			if (p.codeLength == kScriptCodeLength && p.nativeCount == kFakeNatives)
				WriteExpected(blipPlacements, script, placed, before);
			g_blipPlaced++;
			if (AsPlaced(script, placed, kBlipGuard + 4, g_blipGuard->branch + (g_blipGuard->fused ? 1 : 0)))
				g_blipAsPlaced++;
			g_blipGuardShown = g_blipGuard->branch + 3 - kBlipGuard;
			memcpy(g_blipGuardAfter, script.codePages[0] + kBlipGuard, g_blipGuardShown);
		}
		else
		{
			g_otherPlaced++;
			if (AsPlaced(script, placed, 0, 0))
				g_otherAsPlaced++;
		}
	}
	return 0;
}

// The placements the options ask for, once TweaksV has started; then a while for a problem reported on the
// placing thread to reach its own message box (TweaksV shows such a box a second after the problem).
// "script: 2 placed on thread 1234 through a TweaksV stub, resolved 1 of 2, last error kept 2 of 2,
// image 140695... 1236992 1700000000 | " - through the game function (the fake's own resolution function), a
// TweaksV stub (outside the game), or nothing placed with a call to another mod (option "scripthooked")
static void PlaceScriptsAsAsked(int argc, wchar_t** argv)
{
	const size_t size = sizeof(g_scriptResult) / sizeof(g_scriptResult[0]);
	const unsigned char* const otherMod = ImageEnd() + 0x60000;
	const unsigned char* const target = Rel32Target(g_scriptCallSite);
	const wchar_t* const through = target == reinterpret_cast<const unsigned char*>(&FakeResolveNatives) ? L"the game function"
		: target == otherMod ? L"another mod" : InImage(target) ? L"elsewhere in the game" : L"a TweaksV stub";
	if (HasOption(argc, argv, L"scripthash"))
	{
		PlanScript(g_blipImposter);
		PlanScript(g_blipImposter);
	}
	else if (HasOption(argc, argv, L"scriptother"))
	{
		PlanScript(g_otherScript);
		PlanScript(g_blipController);
		g_scriptGap = HasOption(argc, argv, L"scriptgap");
	}
	else if (HasOption(argc, argv, L"scriptotheralone"))
		PlanScript(g_otherScript);
	else if (HasOption(argc, argv, L"scripttwice"))
	{
		PlanScript(g_blipController);
		PlanScript(g_blipController, true);
	}
	else if (HasOption(argc, argv, L"scriptmany"))
		for (int i = 0; i < 17; i++)
			PlanScript(g_blipController);
	else if (HasOption(argc, argv, L"scripthuge"))
	{
		PlanScript(g_blipController, false, (16u << 20) + 1);
		PlanScript(g_blipController, false, kScriptCodeLength, 65537);
	}
	else if (HasOption(argc, argv, L"script") || HasOption(argc, argv, L"scriptpage") || HasOption(argc, argv, L"scripthooked"))
		PlanScript(g_blipController);
	if (g_scriptPlanned == 0)
	{
		swprintf(g_scriptResult, size, L"script: none placed, call to %ls | ", through);
		return;
	}
	if (target == otherMod || (InImage(target) && target != reinterpret_cast<const unsigned char*>(&FakeResolveNatives)))
	{
		swprintf(g_scriptResult, size, L"script: none placed, call to %ls | ", through);	// nothing is there to call
		return;
	}
	if (HasOption(argc, argv, L"scriptpage"))
	{
		DWORD old = 0;
		g_scriptPageUnreadable = VirtualProtect(g_blipController.codePages[1], kScriptPage, PAGE_NOACCESS, &old) != FALSE;
	}
	const HANDLE thread = CreateThread(nullptr, 0, PlaceScripts, nullptr, 0, nullptr);
	if (thread)
	{
		WaitForSingleObject(thread, 30000);
		CloseHandle(thread);
	}
	Sleep(1500);
	const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(ImageBase() + reinterpret_cast<const IMAGE_DOS_HEADER*>(ImageBase())->e_lfanew);
	swprintf(g_scriptResult, size, L"script: %d placed on thread %lu through %ls, resolved %d of %d, last error kept %d of %d, "
		L"image %llu %lu %lu | ", g_scriptPlaced, static_cast<unsigned long>(g_scriptThread), through, g_scriptResolved, g_scriptPlaced,
		g_scriptErrorKept, g_scriptPlaced, static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(ImageBase())),
		static_cast<unsigned long>(nt->OptionalHeader.SizeOfImage), static_cast<unsigned long>(nt->FileHeader.TimeDateStamp));

	// "blips: 2 placed, guard 2C 01 00 01 43 6C 07 0B 56 0A 00, the rest as placed 2 of 2, other programs as placed 0 of 0 | " -
	// blip_controller's guard from its count's NATIVE through its branch after the last placement ("-" with none)
	wchar_t guard[160] = L"-";
	size_t at = 0;
	for (uint32_t i = 0; i < g_blipGuardShown; i++)
		at += static_cast<size_t>(swprintf(guard + at, sizeof(guard) / sizeof(guard[0]) - at, i ? L" %02X" : L"%02X", g_blipGuardAfter[i]));
	swprintf(g_blipResult, sizeof(g_blipResult) / sizeof(g_blipResult[0]), L"blips: %d placed, guard %ls, the rest as placed %d of %d, "
		L"other programs as placed %d of %d | ", g_blipPlaced, guard, g_blipAsPlaced, g_blipPlaced, g_otherAsPlaced, g_otherPlaced);
}

int wmain(int argc, wchar_t** argv)
{
	const wchar_t* asi = argc > 1 ? argv[1] : L"TweaksV.asi";

	// The call at the landing site must lead to g_landingCallee.
	const int32_t rel32 = static_cast<int32_t>(reinterpret_cast<intptr_t>(g_landingCallee) -
		(reinterpret_cast<intptr_t>(g_landingSite) + 5));
	memcpy(const_cast<unsigned char*>(g_landingSite) + 1, &rel32, 4);
	if (HasOption(argc, argv, L"twos3"))	// a second site of the same shape, its call to the same function
	{
		const int32_t copyRel32 = static_cast<int32_t>(reinterpret_cast<intptr_t>(g_landingCallee) -
			(reinterpret_cast<intptr_t>(g_landingSiteCopy) + 5));
		g_landingSiteCopy[0] = 0xE8;
		memcpy(const_cast<unsigned char*>(g_landingSiteCopy) + 1, &copyRel32, 4);
	}

	if (HasOption(argc, argv, L"prepatched"))
	{
		SetBytes(g_splashSite, "\xE9\x86\x01\x00\x00\x90", 6);
		SetBytes(g_legalSite, "\x90\x90\x90\x90\x90\x90", 6);
		SetBytes(g_landingCallee, "\x31\xC0\xC3", 3);
	}
	if (HasOption(argc, argv, L"badsplash"))
		SetBytes(g_splashSite, "\x48\x89", 2);
	if (HasOption(argc, argv, L"badlegal"))
		SetBytes(g_legalSite, "\x48\x89", 2);
	if (HasOption(argc, argv, L"jnzlegal"))
		SetBytes(g_legalSite, "\x0F\x85", 2);
	if (HasOption(argc, argv, L"hookedlanding"))
		SetBytes(g_landingCallee, "\xE9\x11\x22\x33\x44", 5);
	if (HasOption(argc, argv, L"heapraised"))	// another heap adjuster ran first
	{
		const Site heap = SITE(g_heapSite, 1);
		SetImm32(heap, 2000u << 20);
	}
	if (HasOption(argc, argv, L"heaphuge"))		// another heap adjuster set more than HEAP_SIZE allows
	{
		const Site heap = SITE(g_heapSite, 1);
		SetImm32(heap, 3072u << 20);
	}
	if (HasOption(argc, argv, L"heapodd"))		// a value that is not a whole number of MB
	{
		const Site heap = SITE(g_heapSite, 1);
		SetImm32(heap, (648u << 20) | 1);
	}

	// The game's own modkit array holds something in its first and last entry, so the report can tell
	// whether TweaksV copied the game's entries into the new array.
	g_oldModkitArray[0] = 0x1234;
	g_oldModkitArray[1023] = 0x5678;

	// Every modkit reference points at the game's own array...
	for (const Site& site : g_modkitRefs)
		SetRel32(site, g_oldModkitArray);
	if (HasOption(argc, argv, L"modkitmoved"))		// ...unless another mod moved it already
		SetRel32(g_modkitRefs[2], ImageEnd() + 0x10000);
	if (HasOption(argc, argv, L"modkitsplit"))		// ...or one points somewhere else in the game
		SetRel32(g_modkitRefs[4], g_oldModkitArray + 8);
	if (HasOption(argc, argv, L"modkitmismatch"))	// one limit decodes to 1025 instead of 1024
		SetImm32(g_modkitLimits[5], 2050);
	SetRel32(g_modkitExtraSite, g_oldModkitArray);
	if (HasOption(argc, argv, L"modkitextra"))		// a seventh lea of the array, which no pattern names
		g_modkitExtra[0] = 0x48;

	// Every packfile reference points at the game's own array, the indirect size at its variable
	// and the streaming configuration call at the game's function...
	for (const Site& site : g_packRefs)
		SetRel32(site, g_oldPackfileArray);
	SetRel32(g_packR31aSite, g_oldPackfileArray);
	SetRel32(g_packR31bSite, g_oldPackfileArray);
	SetRel32(g_packR31aCopySite, g_oldPackfileArray);
	SetRel32(g_packExtraSite, g_oldPackfileArray);
	if (OwnBuild() >= ((1158u << 16) | 13))	// only this build's own reference is one: the other build's lea is a 32-bit one
		g_packR31b[17] = 0x44;
	else
		g_packR31a[10] = 0x44;
	if (HasOption(argc, argv, L"twor31a"))		// ...and the 1158.13+ pattern matches twice
	{
		g_packR31aCopy[0] = 0xBF;
		g_packR31aCopy[10] = 0x4C;
	}
	if (HasOption(argc, argv, L"packextra"))		// ...or a 34th reference no pattern names
		g_packExtra[0] = 0x48;
	if (HasOption(argc, argv, L"junkr31a"))		// ...or the code only the old pattern matched is there
		SetBytes(g_packR31aCopy, "\xBF\x3D\xFC\xE9\xF3\xBE\x1E\x00\x41\x50\x4C\x8D\x05\x47\x06\x55\xFC\xCC", 18);
	SetRel32(g_packIndirectSite, const_cast<uint32_t*>(&g_packIndirectSize));
	SetRel32(g_packCallSite, reinterpret_cast<const void*>(&OriginalSetStreamingInterface));
	if (HasOption(argc, argv, L"packmoved"))		// ...unless another mod moved the array already
		SetRel32(g_packRefs[5], ImageEnd() + 0x10000);
	if (HasOption(argc, argv, L"packhooked"))		// ...or hooked the call
		SetRel32(g_packCallSite, ImageEnd() + 0x20000);
	if (HasOption(argc, argv, L"packbadsize"))		// an implausible direct size
		for (const Site& site : g_packDirect)
			SetImm32(site, 70000);
	if (HasOption(argc, argv, L"packindirect"))	// the variable holds the size itself, not 4 less
		g_packIndirectSize = 4500;

	// Every reference to the box streamer's traversal list points at the game's own list, and every
	// capacity holds its 1000 entries...
	for (const Site& site : g_boxRefs)
		SetRel32(site, g_oldBoxList);
	SetRel32(g_boxRef9Site, g_oldBoxList);
	if (HasOption(argc, argv, L"boxmoved"))		// ...unless another mod moved the list already
		for (const Site& site : g_boxRefs)
			SetRel32(site, ImageEnd() + 0x30000);
	if (HasOption(argc, argv, L"boxsplit"))		// ...or one points somewhere else in the game
		SetRel32(g_boxRefs[1], g_oldBoxList + 8);
	if (HasOption(argc, argv, L"boxninth"))		// ...or a ninth lea loads the list
		g_boxRef9[0] = 0x4C;
	if (HasOption(argc, argv, L"boxmismatch"))	// one capacity holds 1001 instead of 1000
		SetImm32(g_boxCaps[2], 1001);
	if (HasOption(argc, argv, L"boxbadsize"))	// an implausible capacity
		for (const Site& site : g_boxCaps)
			SetImm32(site, 200000);
	// ...and the four tree-search calls lead to the game's two tree searches, in pairs...
	SetRel32(g_boxCalls[0], reinterpret_cast<const void*>(&FakeTreeSearchA));
	SetRel32(g_boxCalls[1], reinterpret_cast<const void*>(&FakeTreeSearchB));
	SetRel32(g_boxCalls[2], reinterpret_cast<const void*>(&FakeTreeSearchA));
	SetRel32(g_boxCalls[3], reinterpret_cast<const void*>(&FakeTreeSearchB));
	if (HasOption(argc, argv, L"boxcallhooked"))	// ...unless another mod hooked one of them already
		SetRel32(g_boxCalls[1], ImageEnd() + 0x40000);
	if (HasOption(argc, argv, L"boxcallsplit"))	// ...or a pair leads to two different functions
		SetRel32(g_boxCalls[2], reinterpret_cast<const void*>(&FakeTreeSearchB));

	// Every reference to the decal definitions array points at the game's own array, 16 bytes after its
	// count, and every site that carries the limit holds the game's 260...
	for (size_t i = 0; i < 26; i++)
	{
		Site lea = { g_decalLeas[i], 3, sizeof(g_decalLeas[i]) };
		g_decalRefs[5 + i] = lea;
	}
	unsigned char* const decalArray = g_oldDecalStore->entries;
	for (const Site& site : g_decalRefs)
		SetRel32(site, decalArray);
	SetRel32(g_decalLeaExtraSite, decalArray);
	SetRel32(g_decalCountSite, &g_oldDecalStore->count);
	if (HasOption(argc, argv, L"decalmoved"))		// ...unless another mod moved the array already
		for (const Site& site : g_decalRefs)
			SetRel32(site, ImageEnd() + 0x50000);
	if (HasOption(argc, argv, L"decalsplit"))		// ...or one lea points somewhere else in the game
		SetRel32(g_decalRefs[6], decalArray + 80);
	if (HasOption(argc, argv, L"decalextra"))		// ...or a 32nd lea loads it
		g_decalLeaExtra[0] = 0x48;
	if (HasOption(argc, argv, L"decalprefix"))		// ...or one lea has a prefix the game's never have
		g_decalLeas[0][0] = 0x49;
	if (HasOption(argc, argv, L"decalmismatch"))	// FindById's second compare holds 261 instead of 260
		SetImm32(g_decalLimits[2].site, 261);
	if (HasOption(argc, argv, L"decaloffbyone"))	// the parser's check holds 260 instead of 259: N would be 261
		SetImm32(g_decalLimits[0].site, 260);
	if (HasOption(argc, argv, L"decalbadsize"))		// the parser's check implies an implausible 131072
		SetImm32(g_decalLimits[0].site, 131071);
	if (HasOption(argc, argv, L"decalfull"))		// the game's own array is full
		g_oldDecalStore->count = 260;
	if (HasOption(argc, argv, L"decalmany"))		// more than the game's own array holds
		g_oldDecalStore->count = 300;

	// The call at the script programs' hook site leads to the fake's own resolution function...
	MakeScript(g_blipController, "blip_controller", kBlipControllerHash, 1);
	MakeScript(g_otherScript, "other_script", 0x0F74EC00, 50);		// the game's hash of "other_script"
	MakeScript(g_blipImposter, "blip_imposter", kBlipControllerHash, 100);
	SetRel32(g_scriptCallSite, reinterpret_cast<const void*>(&FakeResolveNatives));
	if (HasOption(argc, argv, L"scripthooked"))	// ...unless another mod redirected it already
		SetRel32(g_scriptCallSite, ImageEnd() + 0x60000);
	// blip_controller's guard as an option asks for it, and what more its code holds
	for (const BlipGuard& guard : kBlipGuards)
		if (guard.option[0] && HasOption(argc, argv, guard.option))
			g_blipGuard = &guard;
	g_blipTwice = HasOption(argc, argv, L"bliptwice");
	g_blipJump = HasOption(argc, argv, L"blipjump");
	g_blipCall = HasOption(argc, argv, L"blipcall");
	g_blipSwitch = HasOption(argc, argv, L"blipswitch");
	g_blipCatch = HasOption(argc, argv, L"blipcatch");
	g_blipChanged = HasOption(argc, argv, L"blipchanged");
	g_blipBadByte = HasOption(argc, argv, L"blipbadbyte");
	// The blip pool: 162 slots in use below its high-water mark, 170 - eight below it empty - and two in use beyond it,
	// which the part's measurement must not count; its two sites lead to it
	for (int slot = 0; slot < 170; slot++)
		g_blipPool[slot] = slot % 20 == 10 ? 0 : static_cast<uint64_t>(0x1000 + slot);
	g_blipPool[500] = g_blipPool[1999] = 0x5000;
	*g_blipMark = 170;
	SetRel32(g_blipPoolRef, g_blipPool);
	for (const Site& site : g_blipMarkRefs)
		SetRel32(site, g_blipMark);
	// The parts of the parts' coding round, the Legacy packs check and the limits measured first: each one's sites lead to
	// its data, as its options ask for them
	// The weapon infos: the sites lead to the array and to the allocation, but where an option asks otherwise; the array
	// stays empty while TweaksV starts, unless "wisetup" sets it up first
	{
		const auto array = reinterpret_cast<const unsigned char*>(g_weaponInfosArray);
		const auto outside = ImageEnd() + 0x70000;
		SetRel32(g_weaponInfosLeaRef, array);
		SetRel32(g_weaponInfosExtraRef, array);
		SetRel32(g_weaponInfosCall, HasOption(argc, argv, L"wihooked") ? outside : const_cast<const unsigned char*>(g_weaponInfosAllocation));
		SetRel32(g_weaponInfosBlockRef, HasOption(argc, argv, L"wioutside") ? outside : array);
		WeaponInfosPointCapacityAt(array + (HasOption(argc, argv, L"wicapacity") ? 0xC : 0xA));
		SetRel32(g_weaponInfosLoaderRefs[0], array + (HasOption(argc, argv, L"wicount") ? 0x10 : 8));
		SetRel32(g_weaponInfosLoaderRefs[1], array + 8);
		SetRel32(g_weaponInfosLoaderRefs[2], array);
		if (HasOption(argc, argv, L"wimismatch"))	// the capacity 161
			g_weaponInfosInit[34] = 0xA1;
		if (HasOption(argc, argv, L"wizero"))		// both limits 0
		{
			g_weaponInfosInit[11] = 0;
			g_weaponInfosInit[34] = 0;
		}
		if (HasOption(argc, argv, L"wihigh"))		// both limits 2048: 00 08
		{
			g_weaponInfosInit[11] = g_weaponInfosInit[34] = 0x00;
			g_weaponInfosInit[12] = g_weaponInfosInit[35] = 0x08;
		}
		if (HasOption(argc, argv, L"wibytes"))		// a jump at the allocation's start
			g_weaponInfosAllocation[0] = 0xE9;
		if (HasOption(argc, argv, L"wisize"))		// the loader's blob size 0x100
		{
			g_weaponInfosLoader[45] = 0x00;
			g_weaponInfosLoader[46] = 0x01;
		}
		if (HasOption(argc, argv, L"winotstart"))	// no padding before the loader's start
			g_weaponInfosLoader[0] = 0x48;
		const auto copyMatch = [](const volatile unsigned char* bytes, size_t size) {
			for (size_t i = 0; i < size && i < sizeof(g_weaponLimitsSecondMatch); i++)
				g_weaponLimitsSecondMatch[i] = bytes[i];
		};
		if (HasOption(argc, argv, L"wiinittwice"))		// the game holds a pattern twice
			copyMatch(g_weaponInfosInit, sizeof(g_weaponInfosInit));
		if (HasOption(argc, argv, L"wiloadertwice"))
			copyMatch(g_weaponInfosLoader + 25, sizeof(g_weaponInfosLoader) - 25);
		if (HasOption(argc, argv, L"wcpooltwice"))
			copyMatch(g_weaponComponentsPool, sizeof(g_weaponComponentsPool));
		if (HasOption(argc, argv, L"wcloadertwice"))
			copyMatch(g_weaponComponentsLoader + 1 + 0x13, 66);
		if (HasOption(argc, argv, L"wiextra"))		// a second store of a block into the array
			g_weaponInfosExtra[0] = 0x48;
		if (HasOption(argc, argv, L"wisetup"))		// the initializer ran before TweaksV
			WeaponInfosInitialize(0, 152);
		if (HasOption(argc, argv, L"wijump"))		// another mod's hook: the loader's start a jump to its stub, which jumps back
		{
			g_weaponInfosLoader[1] = 0xE9;
			SetRel32({ g_weaponInfosLoader, 2, sizeof(g_weaponInfosLoader) }, const_cast<const unsigned char*>(g_weaponInfosOtherHook));
			SetRel32({ g_weaponInfosOtherHook, 7, sizeof(g_weaponInfosOtherHook) }, const_cast<const unsigned char*>(g_weaponInfosLoader + 7));
		}
		if ((HasOption(argc, argv, L"wiunwind") || HasOption(argc, argv, L"wiunwindin")) &&	// the loader in a function the unwind
			WeaponInfosCopyIntoCarrier(HasOption(argc, argv, L"wiunwindin") ? 8 : 0))			// data names, W-E-I2 in .fakesit broken
			g_weaponInfosLoader[25] = 0xCC;
		WeaponInfosKeepSites();
	}
	// The weapon components: 412 definitions from 23 files in the list - 470 from 25 with "wcfull" and "wcover", 556 from 40
	// with "wcmore" - and every site leading to the list, its count, the files' count, the configuration and the size lookup...
	WeaponComponentsLoad(HasOption(argc, argv, L"wcfull") || HasOption(argc, argv, L"wcover") ? 470 : 412,
		HasOption(argc, argv, L"wcfull") || HasOption(argc, argv, L"wcover") ? 25 : 23);
	if (HasOption(argc, argv, L"wcover"))	// ...the 471st definition stored over the count, which reads its pointer's low half
	{
		const uint64_t past = 0x000001D4A3C00000ull + 0xC0ull * 470;
		g_weaponComponentsStore->count = static_cast<int32_t>(static_cast<uint32_t>(past));
		g_weaponComponentsStore->unused = static_cast<uint32_t>(past >> 32);
		g_weaponComponentsPlacedStore = *g_weaponComponentsStore;
	}
	if (HasOption(argc, argv, L"wcmore"))	// ...past the game's own list, which the fake fills, only its count past it
	{
		WeaponComponentsLoad(470, 40);
		g_weaponComponentsStore->count = 556;
		g_weaponComponentsPlacedStore = *g_weaponComponentsStore;
	}
	if (HasOption(argc, argv, L"wcempty"))	// ...or none yet
		WeaponComponentsLoad(0, 0);
	if (HasOption(argc, argv, L"wcconfig"))		// ...the configuration naming the pool's size
		g_weaponComponentsConfigured = 600;
	if (HasOption(argc, argv, L"wcconfigbig"))
		g_weaponComponentsConfigured = 20000;
	{
		WeaponComponentsStore* const store = g_weaponComponentsStore;
		const void* const list = HasOption(argc, argv, L"wcmoved") ? static_cast<const void*>(ImageEnd() + 0x70000) : store->list;	// ...unless another mod moved it
		// ...and the count where the game has it - or 4 bytes past it, 8 bytes before the list, outside the game, 20000, 600 or 16500 entries on
		const unsigned char* count = reinterpret_cast<unsigned char*>(&store->count) + (HasOption(argc, argv, L"wcgap") ? 4 : 0);
		if (HasOption(argc, argv, L"wcbefore"))
			count = reinterpret_cast<unsigned char*>(store->list) - 8;
		if (HasOption(argc, argv, L"wcoutside"))
			count = ImageEnd() + 0x72000;
		if (HasOption(argc, argv, L"wcfar"))
			count = reinterpret_cast<unsigned char*>(store->list) + 20000 * 8;
		if (HasOption(argc, argv, L"wcgrown"))
			count = reinterpret_cast<unsigned char*>(store->list) + 600 * 8;
		if (HasOption(argc, argv, L"wclarge"))
			count = reinterpret_cast<unsigned char*>(store->list) + 16500 * 8;
		if (HasOption(argc, argv, L"wcdefault"))	// ...or the pool's default 0
		{
			SetImm32(g_weaponComponentsDefault, 0);
			g_weaponComponentsGameDefault = 0;
		}
		WeaponComponentsPoint(g_weaponComponentsRebuild, 2, 10, count);
		WeaponComponentsPoint(g_weaponComponentsRebuild, 14, 18, &store->files);
		WeaponComponentsPoint(g_weaponComponentsRebuild, 34, 38, list);
		WeaponComponentsPoint(g_weaponComponentsSort, 3, 7, count);
		WeaponComponentsPoint(g_weaponComponentsSort, 14, 18, list);
		WeaponComponentsPoint(g_weaponComponentsUpdate, 3, 7, count);
		WeaponComponentsPoint(g_weaponComponentsUpdate, 12, 16, count + (HasOption(argc, argv, L"wccount") ? 8 : 0));	// ...or the store elsewhere
		WeaponComponentsPoint(g_weaponComponentsPool, 3, 7, const_cast<void**>(&g_weaponComponentsConfiguration));
		WeaponComponentsPoint(g_weaponComponentsPool, 19, 23, HasOption(argc, argv, L"wchooked") ? static_cast<const void*>(ImageEnd() + 0x71000)
			: reinterpret_cast<const void*>(&PoolSizeLookup));	// ...or the call another mod's hook
		for (auto& row : g_weaponComponentsLeas)
			WeaponComponentsPoint(row, 3, 7, list);
		WeaponComponentsPoint(g_weaponComponentsLeaExtra, 3, 7, list);
		if (HasOption(argc, argv, L"wcsplit"))	// ...or one lea points elsewhere in the game
			WeaponComponentsPoint(g_weaponComponentsLeas[4], 3, 7, store->list + 1);
		if (HasOption(argc, argv, L"wcextra"))	// ...or an 18th loads it
			g_weaponComponentsLeaExtra[0] = 0x48;
		if (HasOption(argc, argv, L"wcprefix"))	// ...or one has a prefix the game's never have
			g_weaponComponentsLeas[0][0] = 0x49;
		// ...the loader's calls of the allocator and memset, its blob array, its body - or another mod's hook at its head
		WeaponComponentsPoint(g_weaponComponentsLoader, 32, 36, reinterpret_cast<const void*>(&WeaponComponentsAllocate));
		WeaponComponentsPoint(g_weaponComponentsLoader, 51, 55, reinterpret_cast<const void*>(&WeaponComponentsZero));
		WeaponComponentsPoint(g_weaponComponentsLoader, 58, 62, &store->files);
		WeaponComponentsPoint(g_weaponComponentsLoader, 65, 69, &store->capacity);
		WeaponComponentsPoint(g_weaponComponentsLoader, 82, 86, &store->blobs);
		WeaponComponentsPoint(g_weaponComponentsLoader, 96, 100, reinterpret_cast<const void*>(&WeaponComponentsLoadFile));
		if (HasOption(argc, argv, L"wcloadercount"))	// ...its read of the blobs' count elsewhere
			WeaponComponentsPoint(g_weaponComponentsLoader, 58, 62, &store->unused);
		if (HasOption(argc, argv, L"wcloadercapacity"))	// ...or of the capacity
			WeaponComponentsPoint(g_weaponComponentsLoader, 65, 69, &store->unused);
		if (HasOption(argc, argv, L"wcloaderhooked"))
			g_weaponComponentsLoader[1] = 0xE9;
		if (HasOption(argc, argv, L"wcloadernotstart"))
			g_weaponComponentsLoader[0] = 0x48;
		if (HasOption(argc, argv, L"wcloaderret"))
			g_weaponComponentsLoader[1] = 0xC3;
		WeaponComponentsCopySites(g_weaponComponentsPlacedSites, sizeof(g_weaponComponentsPlacedSites));
		g_weaponComponentsAtStart = *store;
		if (HasOption(argc, argv, L"wcearly"))	// ...and the game made the pool before TweaksV started
			WeaponComponentsMakePool();
	}
	DWORD weaponComponentsOld = 0;	// ...and the store's page unreadable while TweaksV starts
	if (HasOption(argc, argv, L"wccountnoaccess"))
		VirtualProtect(const_cast<WeaponComponentsStore*>(g_weaponComponentsStore), sizeof(g_weaponComponentsStorePage), PAGE_NOACCESS, &weaponComponentsOld);
	SirenIdsSetup(argc, argv);
	SirenCountSetup(argc, argv);
	// The ped props' calls lead to the stand-ins: T1's to the texture count, the per-anchor count's 7 callers to its start,
	// the metadata lookup's 17 to it, the two others elsewhere, the two more to them too (their E8 only with an option);
	// A-E-S1's four references to the store's globals; then the store built and the sites' and data's bytes summed
	{
		const auto pedPropsCount = const_cast<const unsigned char*>(g_pedPropsCount);
		const auto pedPropsLookup = reinterpret_cast<const void*>(&PedPropsMetadataLookup);
		const auto pedPropsTexture = reinterpret_cast<const void*>(&PedPropsTextureCount);
		SetRel32(g_pedPropsTextureCall, pedPropsTexture);
		for (const Site& site : g_pedPropsCountCalls)
			SetRel32(site, pedPropsCount);
		for (const Site& site : g_pedPropsLookupCalls)
			SetRel32(site, pedPropsLookup);
		SetRel32(g_pedPropsElsewhereCalls[0], pedPropsCount + 0x30);	// into the count's loop
		SetRel32(g_pedPropsElsewhereCalls[1], pedPropsTexture);
		SetRel32(g_pedPropsExtraCalls[0], pedPropsCount);
		SetRel32(g_pedPropsExtraCalls[1], pedPropsLookup);
		if (HasOption(argc, argv, L"ppcountcall"))		// an 8th call of the per-anchor count
			g_pedPropsExtra[0][0] = 0xE8;
		if (HasOption(argc, argv, L"pplookupcall"))	// an 18th call of the metadata lookup
			g_pedPropsExtra[1][0] = 0xE8;
		if (HasOption(argc, argv, L"ppcountshort"))	// one of the per-anchor count's 7 calls redirected by another mod
			SetRel32(g_pedPropsCountCalls[5], ImageEnd() + 0x70000);
		if (HasOption(argc, argv, L"pplookupshort"))	// one of the metadata lookup's 17 calls redirected by another mod
			SetRel32(g_pedPropsLookupCalls[6], ImageEnd() + 0x70000);
		const auto pedPropsSecondCopy = [](const volatile unsigned char* match, size_t size)
		{
			for (size_t i = 0; i < size; i++)
				g_pedPropsSecondCopy[i] = match[i];
		};
		if (HasOption(argc, argv, L"ppt1twice"))		// a second copy of A-E-T1
			pedPropsSecondCopy(g_pedPropsT1, sizeof(g_pedPropsT1));
		if (HasOption(argc, argv, L"ppc1twice"))		// a second copy of A-E-C1, the 0x20 bytes of the count's start
			pedPropsSecondCopy(g_pedPropsCount, 0x20);
		if (HasOption(argc, argv, L"ppc2twice"))		// a second copy of A-E-C2, the count's loop to its end
			pedPropsSecondCopy(g_pedPropsCount + 0x30, sizeof(g_pedPropsCount) - 0x30);
		if (HasOption(argc, argv, L"pps1twice"))		// a second copy of A-E-S1
			pedPropsSecondCopy(g_pedPropsStore, sizeof(g_pedPropsStore));
		if (HasOption(argc, argv, L"ppg1once"))		// A-E-G1's second copy gone
			g_pedPropsG1b[0] = 0xCC;
		if (HasOption(argc, argv, L"ppg1thrice"))		// a third copy of A-E-G1
			g_pedPropsG1c[0] = 0x48;
		if (HasOption(argc, argv, L"ppcountsplit"))	// A-E-T3's call into the count's loop
			SetRel32(g_pedPropsCountCalls[1], pedPropsCount + 0x30);
		if (HasOption(argc, argv, L"pplookupsplit"))	// A-E-G4's call to the texture count
			SetRel32(g_pedPropsLookupCalls[3], pedPropsTexture);
		if (HasOption(argc, argv, L"pphooked"))		// A-E-T1's call to another mod, outside the game
			SetRel32(g_pedPropsTextureCall, ImageEnd() + 0x70000);
		const void* const pedPropsGlobals[] = { g_pedPropsStoreCount, g_pedPropsStoreBitmap, g_pedPropsStoreArray, g_pedPropsStoreStride };
		for (size_t i = 0; i < 4; i++)
			SetRel32(g_pedPropsStoreRefs[i], pedPropsGlobals[i]);
		if (HasOption(argc, argv, L"ppstoreout"))		// the reference to the array's pointer outside the game
			SetRel32(g_pedPropsStoreRefs[2], ImageEnd() + 0x80000);
		PedPropsBuild(HasOption(argc, argv, L"ppmany"), HasOption(argc, argv, L"ppunknown"), HasOption(argc, argv, L"ppflood"),
			HasOption(argc, argv, L"ppunreadable"), HasOption(argc, argv, L"pphuge"));
		if (HasOption(argc, argv, L"ppbadcount"))
			*g_pedPropsStoreCount = 2000000;
		if (HasOption(argc, argv, L"ppbadstride"))
			*g_pedPropsStoreStride = 12;
		if (HasOption(argc, argv, L"ppwidened"))		// A-E-T3's narrowing widened by another mod
			g_pedPropsT3[11] = 0xB7;
		if (HasOption(argc, argv, L"ppg1widened"))	// A-E-G1's second copy widened by another mod
			g_pedPropsG1b[17] = 0xB7;
		// the sums the report compares with: the sites as placed, and as the part leaves them - each of the eleven B6
		// made B7, nothing else changed
		unsigned char placed[sizeof(g_pedPropsWidenable) / sizeof(g_pedPropsWidenable[0])];
		for (size_t i = 0; i < sizeof(placed); i++)
		{
			placed[i] = *g_pedPropsWidenable[i];
			*g_pedPropsWidenable[i] = 0xB7;
		}
		g_pedPropsWidenedSum = PedPropsSum(true);
		for (size_t i = 0; i < sizeof(placed); i++)
			*g_pedPropsWidenable[i] = placed[i];
		g_pedPropsSitesSum = PedPropsSum(true);
		g_pedPropsDataSum = PedPropsSum(false);
	}
	// The texture budget: the data as the game has them before its start-up - the most and the budgets 0, with "tblate" all of
	// them - and every site leading to them as the game's do, but where an option asks otherwise; with "tbearly" the start-up
	// made before TweaksV loads, and with "tbunreadable" the data's page taken away until TweaksV has started
	{
		g_textureBudgetHoldRaise = HasOption(argc, argv, L"tbhalfraise");
		if (g_textureBudgetHoldRaise)	// the data over two pages, budgets 2 and 3 first on the second
		{
			g_textureBudgetData = reinterpret_cast<TextureBudgetData*>(g_textureBudgetTwoPages + 4096 - offsetof(TextureBudgetData, counters.budgets[2]));
			AddVectoredExceptionHandler(1, &TextureBudgetHoldRaise);
		}
		TextureBudgetData& data = *g_textureBudgetData;
		// the descriptor heap and its free list, the first 30000 in use, as the options ask
		constexpr uint32_t kDescriptors = 0x30000;
		const auto list = static_cast<uint32_t*>(VirtualAlloc(nullptr, kDescriptors * sizeof(uint32_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
		for (uint32_t i = 0; list && i < kDescriptors; i++)
			list[i] = i + 1;
		TextureBudgetHeap& heap = g_textureBudgetHeapMade;
		heap.list = list;
		heap.head = 30000;
		heap.count = kDescriptors;
		if (HasOption(argc, argv, L"tbfree16383"))
			heap.head = kDescriptors - 16383;
		if (HasOption(argc, argv, L"tbfree16384"))
			heap.head = kDescriptors - 16384;
		if (list && HasOption(argc, argv, L"tblistcycle"))
			list[kDescriptors - 1] = heap.head;
		if (list && HasOption(argc, argv, L"tblistpast"))
			list[heap.head + 10] = kDescriptors + 5;
		if (HasOption(argc, argv, L"tblistnull"))
			heap.list = nullptr;
		if (HasOption(argc, argv, L"tblistodd"))
			heap.list = reinterpret_cast<uint32_t*>(reinterpret_cast<unsigned char*>(list) + 1);
		if (HasOption(argc, argv, L"tbheaphuge"))
			heap.count = (1u << 24) + 1;
		if (HasOption(argc, argv, L"tblistunreadable"))
		{
			DWORD old = 0;
			VirtualProtect(list, kDescriptors * sizeof(uint32_t), PAGE_NOACCESS, &old);
		}
		if (HasOption(argc, argv, L"tbheapnone"))
			g_textureBudgetSizes[0] = g_textureBudgetSizes[1] = 0;
		if (HasOption(argc, argv, L"tbheapsmall"))	// 159744
			g_textureBudgetSizes[1] = 0x17000;
		if (HasOption(argc, argv, L"tbheapedge"))	// 131072
			g_textureBudgetSizes[1] = 0x10000;
		if (HasOption(argc, argv, L"tbheaptiny"))	// 98304
			g_textureBudgetSizes[1] = 0x8000;
		g_textureBudgetConfigMost = HasOption(argc, argv, L"tbconfig140000") ? 140000 : HasOption(argc, argv, L"tbconfig147456") ? 147456 :
			HasOption(argc, argv, L"tbconfig200000") ? 200000 : 98304;
		if (!HasOption(argc, argv, L"tblate"))
			TextureBudgetSetAsTheGame(data);
		if (HasOption(argc, argv, L"tbnoindex"))	// a texture quality's index outside 0 to 3
			data.index = 7;
		g_textureBudgetBigConfig = HasOption(argc, argv, L"tbconfigbig");
		const auto counters = reinterpret_cast<const unsigned char*>(&data.counters);
		SetRel32(g_textureBudgetMostRef, counters + 0x20);
		SetRel32(g_textureBudgetBudgetRefs[0], counters + 0x28);
		SetRel32(g_textureBudgetBudgetRefs[1], counters + 0x38);
		SetRel32(g_textureBudgetCall, reinterpret_cast<const void*>(&TextureBudgetNextStep));
		SetRel32(g_textureBudgetIndexRef, &data.index);
		for (const TextureBudgetLoad& load : g_textureBudgetLoads)
			SetRel32(load.site, counters + load.field);
		// the update check's code past its pattern: NOPs, "cmp byte [rip+disp32], 0" of its flag and a "jz" at +0x4B, "mulss xmm0,
		// [rip+disp32]" by its lower factor at +0x54 and "mulss xmm9, [rip+disp32]" by its raise factor at +0xA0; the disp32 of its
		// switch and of its flag each one byte short of what it leads to, since an imm8 ends their instructions
		const auto update = const_cast<unsigned char*>(g_textureBudgetUpdate);
		memset(update + 0x27, 0x90, sizeof(g_textureBudgetUpdate) - 0x27 - 1);
		const unsigned char flagCompare[] = { 0x80, 0x3D, 0, 0, 0, 0, 0x00, 0x74, 0x08 };
		const unsigned char lowerFactor[] = { 0xF3, 0x0F, 0x59, 0x05 };
		const unsigned char raiseFactor[] = { 0xF3, 0x44, 0x0F, 0x59, 0x0D };
		memcpy(update + 0x4B, flagCompare, sizeof(flagCompare));
		memcpy(update + 0x54, lowerFactor, sizeof(lowerFactor));
		memcpy(update + 0xA0, raiseFactor, sizeof(raiseFactor));
		update[sizeof(g_textureBudgetUpdate) - 1] = 0xCC;
		SetRel32(g_textureBudgetSwitchRef, reinterpret_cast<const unsigned char*>(&data.updateCheck.on) - 1);
		SetRel32(g_textureBudgetFlagRef, &data.flag - 1);
		SetRel32(g_textureBudgetFactorRefs[0], &data.updateCheck.lower);
		SetRel32(g_textureBudgetFactorRefs[1], &data.updateCheck.raise);
		SetRel32(g_textureBudgetGetterCall, const_cast<const unsigned char*>(g_textureBudgetGetter));
		// the descriptor heap's maker: NOPs, its stores of the size and of no free list 0x36 bytes in (with "tbmakermoved" 0x37), the store
		// of the head 0x9E past them (with "tbheadstores" another offset's); the device's start leading to all of it
		const auto maker = const_cast<unsigned char*>(g_textureBudgetMaker);
		memset(maker, 0x90, sizeof(g_textureBudgetMaker) - 1);
		maker[sizeof(g_textureBudgetMaker) - 1] = 0xCC;
		const unsigned char heapStores[] = { 0x44, 0x89, 0x41, 0x2C, 0x84, 0xC0, 0x74, 0x05, 0x48, 0xC7, 0x46, 0x18, 0x00, 0x00, 0x00, 0x00, 0xE9 };
		const unsigned char headStores[] = { 0x89, 0x7E, 0x24, 0x89, 0x7E, static_cast<unsigned char>(HasOption(argc, argv, L"tbheadstores") ? 0x28 : 0x20) };
		const size_t heapStoresAt = HasOption(argc, argv, L"tbmakermoved") ? 0x37 : 0x36;
		memcpy(maker + heapStoresAt, heapStores, sizeof(heapStores));
		memcpy(maker + heapStoresAt + 0x9E, headStores, sizeof(headStores));
		const auto heapObject = reinterpret_cast<const unsigned char*>(&data.heap);
		SetRel32(g_textureBudgetSizeRefs[0], HasOption(argc, argv, L"tbsizeout") ? ImageEnd() + 0x7B000 :
			reinterpret_cast<const unsigned char*>(&data.heapSizes[0]) + (HasOption(argc, argv, L"tbsizeodd") ? 1 : 0));
		SetRel32(g_textureBudgetSizeRefs[1], &data.heapSizes[1]);
		SetRel32(g_textureBudgetHeapRef, HasOption(argc, argv, L"tbheapout") ? ImageEnd() + 0x7D000 : heapObject + (HasOption(argc, argv, L"tbheapodd") ? 4 : 0));
		SetRel32(g_textureBudgetMakerCall, HasOption(argc, argv, L"tbmakerhooked") ? ImageEnd() + 0x7F000 : const_cast<const unsigned char*>(g_textureBudgetMaker));
		if (HasOption(argc, argv, L"tbshift"))		// the shift between the two stores of the budgets by 21, not 20
			g_textureBudgetStores[25] = 0x15;
		if (HasOption(argc, argv, L"tbcallhooked"))	// another mod hooked the call after the stores already
			SetRel32(g_textureBudgetCall, ImageEnd() + 0x70000);
		if (HasOption(argc, argv, L"tbindexout"))	// the index outside the game
			SetRel32(g_textureBudgetIndexRef, ImageEnd() + 0x78000);
		if (HasOption(argc, argv, L"tbcounters"))	// the update check's lea of the counters 8 bytes past them
			SetRel32(g_textureBudgetLoads[5].site, counters + 8);
		if (HasOption(argc, argv, L"tbmost"))		// the placement's check loads the most objects from 4 bytes past them
			SetRel32(g_textureBudgetLoads[2].site, counters + 0x24);
		if (HasOption(argc, argv, L"tbfactor"))	// "addss" in the place of the multiplication by the raise factor
			g_textureBudgetUpdate[0xA3] = 0x58;
		if (HasOption(argc, argv, L"tbmostout"))	// the start-up's store of the most objects outside the game
			SetRel32(g_textureBudgetMostRef, ImageEnd() + 0x7A000);
		if (HasOption(argc, argv, L"tbbudgetsout"))	// its store of budgets 0 and 1 outside the game
			SetRel32(g_textureBudgetBudgetRefs[0], ImageEnd() + 0x7C000);
		if (HasOption(argc, argv, L"tbt2moved"))	// the store of budgets 2 and 3 elsewhere: Q-E-T2's 41 bytes in the update check's
		{											// NOPs, and the bytes right after the store of budgets 0 and 1 no longer Q-E-T2
			const auto stores = const_cast<unsigned char*>(g_textureBudgetStores);
			memcpy(update + 0x60, stores + 34, 41);
			stores[34] = 0x90;
		}
		if (HasOption(argc, argv, L"tbbudgets23"))	// budgets 2 and 3 24 bytes after budgets 0 and 1
			SetRel32(g_textureBudgetBudgetRefs[1], counters + 0x40);
		if (HasOption(argc, argv, L"tbcountersout"))	// the placement's load of the buffers' bytes, the counters' start, outside the game
			SetRel32(g_textureBudgetLoads[3].site, ImageEnd() + 0x7E000);
		if (HasOption(argc, argv, L"tbmoststore"))	// the start-up's store of the most 4 bytes past where the placement reads it
			SetRel32(g_textureBudgetMostRef, counters + 0x24);
		if (HasOption(argc, argv, L"tbbudgetsoff"))	// both stores of the budgets 16 bytes past them
		{
			SetRel32(g_textureBudgetBudgetRefs[0], counters + 0x38);
			SetRel32(g_textureBudgetBudgetRefs[1], counters + 0x48);
		}
		if (HasOption(argc, argv, L"tbindexunreadable"))	// the index on a page of its own, taken away below
			SetRel32(g_textureBudgetIndexRef, g_textureBudgetIndexPage);
		if (HasOption(argc, argv, L"tbmostodd"))	// the addresses inside the game, not aligned to the size of their reads
			SetRel32(g_textureBudgetMostRef, counters + 0x21);
		if (HasOption(argc, argv, L"tbbudgetsodd"))
			SetRel32(g_textureBudgetBudgetRefs[0], counters + 0x2C);
		if (HasOption(argc, argv, L"tbindexodd"))
			SetRel32(g_textureBudgetIndexRef, reinterpret_cast<const unsigned char*>(&data.index) + 1);
		if (HasOption(argc, argv, L"tbcountersodd"))
			SetRel32(g_textureBudgetLoads[3].site, counters + 4);
		if (HasOption(argc, argv, L"tbswitchodd"))	// its disp32 a byte short of what it leads to, as above
			SetRel32(g_textureBudgetSwitchRef, reinterpret_cast<const unsigned char*>(&data.updateCheck.on) + 1);
		// a pattern the game holds twice: as many of the site's bytes as the pattern has, copied
		const struct { const wchar_t* option; const volatile unsigned char* site; size_t size; } twins[] = { { L"tbt1twice", g_textureBudgetStores, 21 },
			{ L"tbt2twice", g_textureBudgetStores + 34, 41 }, { L"tbt3twice", g_textureBudgetGetter, 13 }, { L"tbt4twice", g_textureBudgetPlacement, 32 },
			{ L"tbt5twice", g_textureBudgetUpdate, 35 }, { L"tbt6twice", g_textureBudgetHeapSite, 40 }, { L"tbt7twice", g_textureBudgetMaker + 0x36, 17 } };
		for (const auto& twin : twins)
			if (HasOption(argc, argv, twin.option))
				memcpy(update + 0x60, const_cast<const unsigned char*>(twin.site), twin.size);
		// the budgets' neighbours and readers where they lie; a writer at the budgets only with its option, else at the dummy
		for (const TextureBudgetOperand& operand : g_textureBudgetOperands)
			SetRel32(operand.site, !operand.option || HasOption(argc, argv, operand.option) ?
				reinterpret_cast<const void*>(counters + 0x28 + operand.from - operand.immediate) : reinterpret_cast<const void*>(g_textureBudgetDummy));
		// the readings' block alike: each of its writers at the budgets only with its option
		for (const TextureBudgetOperand& operand : g_textureBudgetMisreadOperands)
			SetRel32(operand.site, HasOption(argc, argv, operand.option) ?
				reinterpret_cast<const void*>(counters + 0x28 + operand.from - operand.immediate) : reinterpret_cast<const void*>(g_textureBudgetDummy));
		if (HasOption(argc, argv, L"tbearly"))		// the game's start-up before TweaksV's DllMain: its stores and the call after them
			TextureBudgetStartUp();
		g_textureBudgetDataAsSet = data;
		int asPlaced = 0;
		TextureBudgetSites(g_textureBudgetSitesAsPlaced, false, asPlaced);
		for (size_t i = 0; i < sizeof(g_textureBudgetMisread); i++)
			g_textureBudgetMisreadAsPlaced[i] = g_textureBudgetMisread[i];
		if (HasOption(argc, argv, L"tbunreadable"))	// the data's page cannot be read while TweaksV starts
		{
			DWORD old = 0;
			VirtualProtect(&g_textureBudgetPage, sizeof(g_textureBudgetPage), PAGE_NOACCESS, &old);
		}
		if (HasOption(argc, argv, L"tbindexunreadable"))	// the index's page cannot be read while TweaksV starts
		{
			DWORD old = 0;
			VirtualProtect(g_textureBudgetIndexPage, sizeof(g_textureBudgetIndexPage), PAGE_NOACCESS, &old);
		}
	}
	LpSetup(argc, argv);	// the Legacy packs check's archives, before TweaksV starts
	// The ray tracing buffer: the set-up's call to the maker, the checks' slot and counters, the reset's lea - as the options ask
	RtSetUp(argc, argv);
	// TEST_DUMP_DATA's window, filled before TweaksV takes its first copies - as the options ask
	DwSetUp(argc, argv);
	StaticBoundsSetUpSites(argc, argv);
	LcSetUp(argc, argv);
	PopSetUp(argc, argv);
	PedPoolSetUpSites(argc, argv);
	PoolMemorySetUpSite(argc, argv);
	RqSetUp(argc, argv);
	// [siren damage]
	SdSetUp(argc, argv);
	// [corona store]
	CcSetUp(argc, argv);

	RovSetup(argc, argv);
	ForEachSite(TouchSite);
	RovForEachDataSite(TouchData);
	MakeSitesReadExecute();
	g_siteProtect = ProtectionOf(g_heapSite);
	RtTakeAway(argc, argv);
	SirenIdsHookProtect(argc, argv, PAGE_READONLY);
	PedPoolTakeAway(argc, argv);

	// the stand-in dstorage.dll, loaded before TweaksV as the game's import is
	const HMODULE dstorage = HasOption(argc, argv, L"dstorage") ? LoadLibraryW(L"dstorage.dll") : nullptr;
	if (HasOption(argc, argv, L"nodyncode"))	// from here on no code may be changed or made (Windows' dynamic code policy)
	{
		PROCESS_MITIGATION_DYNAMIC_CODE_POLICY policy{};
		policy.ProhibitDynamicCode = 1;
		if (!SetProcessMitigationPolicy(ProcessDynamicCodePolicy, &policy, sizeof(policy)))
			wprintf(L"dynamic code policy not set: %lu | ", GetLastError());
	}
	const HANDLE lockedLog = HasOption(argc, argv, L"lockedlog") ? CreateFileW(L"TweaksV.log", GENERIC_READ,
		FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr) : INVALID_HANDLE_VALUE;
	if (HasOption(argc, argv, L"lockedlog"))
		wprintf(L"log %ls | ", lockedLog != INVALID_HANDLE_VALUE ? L"held" : L"NOT held");
	HMODULE module = LoadLibraryW(asi);
	if (lockedLog != INVALID_HANDLE_VALUE)
		CloseHandle(lockedLog);
	if (!module)
	{
		wprintf(L"LoadLibrary(%ls) failed: %lu\n", asi, GetLastError());
		return 1;
	}
	SirenIdsHookProtect(argc, argv, PAGE_EXECUTE_READ);
	RovFirstTimeRequest();	// the game asks for the time: RageOpenV's hook runs its one-time start-up
	Sleep(1500); // the start-up summary thread runs
	if (HasOption(argc, argv, L"unload"))	// TweaksV pins itself: freed, it must stay, and its hooks with it. One FreeLibrary
	{										// would not show that: TweaksV's own threads hold references of their own for a while
		int frees = 0;						// (RageOpenV's start-up check, 30 s), so it is freed until it is gone, or 8 times -
		while (frees < 8 && GetModuleHandleW(asi))	// pinned, it stays however often
		{
			FreeLibrary(module);
			frees++;
			Sleep(100);
		}
		wprintf(L"unload: %ls after %d x FreeLibrary | ", GetModuleHandleW(asi) ? L"still loaded" : L"unloaded", frees);
	}
	CallBoxSearches();	// the box streamer searches, as the game would call them
	if (HasOption(argc, argv, L"sbfirst"))
		StaticBoundsMakeStores(argc, argv);
	PlaceScriptsAsAsked(argc, argv);	// the script programs, as the game's resource placement thread places them
	if (dstorage && HasOption(argc, argv, L"dsnofactory"))
		swprintf(g_dstorageResult, sizeof(g_dstorageResult) / sizeof(g_dstorageResult[0]), L"dstorage: not called | ");
	else if (dstorage)
		CallDStorage(dstorage, HasOption(argc, argv, L"dsotherinterface") ? OtherInterface::First
			: HasOption(argc, argv, L"dsotherlater") ? OtherInterface::Later
			: HasOption(argc, argv, L"dsotheronly") ? OtherInterface::Only : OtherInterface::None);
	if (HasOption(argc, argv, L"dsshare"))
		ShareThroughStandIn();
	if (HasOption(argc, argv, L"ownshare"))
		ShareOwnOpens();
	if (HasOption(argc, argv, L"dstypes"))
		OpenTypes();
	if (HasOption(argc, argv, L"fileopens"))
		OpenFiles();
	if (HasOption(argc, argv, L"dsopenflood"))
		FloodOpens();
	if (HasOption(argc, argv, L"dsreplace"))
		ReplaceHeldFile();
	if (HasOption(argc, argv, L"dsrenames"))
		RenameAsked();
	if (HasOption(argc, argv, L"dsrenameflood"))
		FloodRenames();
	// The parts of the parts' coding round, the Legacy packs check and the limits measured first: what the game does with
	// each once TweaksV has started, as its options ask
	if (HasOption(argc, argv, L"wiedx"))	// another mod's port rewrites the number of blobs alone to 512, after TweaksV's raise
	{
		DWORD old = 0, unused = 0;
		const uint32_t blobs = 512;
		VirtualProtect(const_cast<unsigned char*>(g_weaponInfosInit + 11), sizeof(blobs), PAGE_EXECUTE_READWRITE, &old);
		memcpy(const_cast<unsigned char*>(g_weaponInfosInit + 11), &blobs, sizeof(blobs));
		VirtualProtect(const_cast<unsigned char*>(g_weaponInfosInit + 11), sizeof(blobs), old, &unused);
	}
	if (!HasOption(argc, argv, L"wisetup"))	// the game's initializer of the weapon info blob array, then its loader
		WeaponInfosInitialize(HasOption(argc, argv, L"wiraised") ? 512 : 0, HasOption(argc, argv, L"wiraised") ? 170 : 152);
	if (HasOption(argc, argv, L"wipacks"))
		WeaponInfosMountPacks();
	WeaponInfosLoadAsAsked(argc, argv);	// the files the options ask for, through the loader
	if (HasOption(argc, argv, L"wccountnoaccess"))	// the store's page readable again
	{
		DWORD old = 0;
		VirtualProtect(const_cast<WeaponComponentsStore*>(g_weaponComponentsStore), sizeof(g_weaponComponentsStorePage), PAGE_READWRITE, &old);
	}
	if (HasOption(argc, argv, L"wcloadnoaccess"))	// the leas' page unreadable a while
		WeaponComponentsLoadsNoAccess();
	if (HasOption(argc, argv, L"wcpack"))	// a pack's weaponcomponents.meta loads: the list is rebuilt from 24 files
	{
		Sleep(11000);
		WeaponComponentsLoad(430, 24);
	}
	if (HasOption(argc, argv, L"wcputback"))	// the game puts one of the list's loads back
		WeaponComponentsPutBack();
	if ((HasOption(argc, argv, L"wcmount") || HasOption(argc, argv, L"wcmany") || HasOption(argc, argv, L"wcfill")) && !HasOption(argc, argv, L"wcloaderhooked") &&
		!HasOption(argc, argv, L"wcloadernotstart") && !HasOption(argc, argv, L"wcloaderret"))	// the pool made, packs mounted
	{
		if (!HasOption(argc, argv, L"wcearly") && !HasOption(argc, argv, L"wcempty"))
			WeaponComponentsMakePool();
		WeaponComponentsMountPacks(argc, argv);
		if (g_weaponComponentsLoaded < g_weaponComponentsMounted)	// a file refused: up to 3 s for the box, or the line that it is not shown
			WeaponComponentsAwait(" finding(s)", 1, 3000);
		else if (HasOption(argc, argv, L"wcearly"))	// the pool made without the redirect: the same for its problem
			WeaponComponentsAwait("start-up summary", 1, 3000);
	}
	SirenIdsLater();
	SirenRaiseActions(argc, argv);
	if (HasOption(argc, argv, L"scdraw"))
		SirenCountDrawAll(HasOption(argc, argv, L"scodd"), HasOption(argc, argv, L"scmore"), HasOption(argc, argv, L"scfull"),
			HasOption(argc, argv, L"scfault"));
	if (HasOption(argc, argv, L"ppgrow"))	// the game loads another pack, after the ped props' first summary, 10 s in
	{
		Sleep(10500);
		PedPropsGrow();
	}
	// the game's start-up after TweaksV's DllMain: the budgets' stores and the call after them - with "tblate" once TweaksV's
	// first summary is in its log, with "tbearly" not again; with "tbunreadable" the data's page readable again first
	if (HasOption(argc, argv, L"tbunreadable"))
	{
		DWORD old = 0;
		VirtualProtect(&g_textureBudgetPage, sizeof(g_textureBudgetPage), PAGE_READWRITE, &old);
	}
	if (HasOption(argc, argv, L"tbindexunreadable"))	// as with "tbunreadable", the index's page alone
	{
		DWORD old = 0;
		VirtualProtect(g_textureBudgetIndexPage, sizeof(g_textureBudgetIndexPage), PAGE_READWRITE, &old);
	}
	if (!HasOption(argc, argv, L"tblate") && !HasOption(argc, argv, L"tbearly"))
		TextureBudgetStartUp();
	if (g_textureBudgetHoldRaise)
		TextureBudgetWaitFor(g_textureBudgetSummariesAtHold + 1, 10000);
	if (HasOption(argc, argv, L"tbgrow"))
		TextureBudgetLater(TextureBudgetChange::Grow);
	else if (HasOption(argc, argv, L"tbhigh"))
		TextureBudgetLater(TextureBudgetChange::High);
	else if (HasOption(argc, argv, L"tblate"))
		TextureBudgetLater(TextureBudgetChange::Late);
	else if (HasOption(argc, argv, L"tbhighdesc"))
		TextureBudgetLater(TextureBudgetChange::Descriptors);
	LpWait(argc, argv);
	// The ray tracing buffer: the game's renderer makes it and its frames take ranges of it - as the options ask, the checks run
	RtActions(argc, argv);
	// TEST_DUMP_DATA's second copies, a key's 1 s after TweaksV started, are made before the program ends
	if (g_dwOn)
		Sleep(2500);
	StaticBoundsActions(argc, argv);
	for (int i = 2; i + 1 < argc; i++)
		if (wcscmp(argv[i], L"latefailures") == 0)
			for (int n = _wtoi(argv[i + 1]); n > 0; n--)
			{
				using Resolve = unsigned char (*)(unsigned char*);
				for (size_t k = 0; k < kFakeNatives; k++)
					g_blipController.natives[k] = g_fakeNatives[k].hash;
				g_blipController.program.codeLength = (16u << 20) + 1;
				reinterpret_cast<Resolve>(const_cast<unsigned char*>(Rel32Target(g_scriptCallSite)))(reinterpret_cast<unsigned char*>(&g_blipController.program));
				Sleep(2000);
			}
	LcActions(argc, argv);
	PopActions(argc, argv);
	PedPoolActions(argc, argv);
	PoolMemoryActions(argc, argv);
	RqActions(argc, argv);
	// [siren damage]
	SdActions(argc, argv);
	SirenJoinActions(argc, argv);
	// [corona store]
	CcActions(argc, argv);
	if (HasOption(argc, argv, L"wait"))
		Sleep(11000);	// TweaksV's UNLOCK_FILES summary comes 10 s after the first open it counts, the OpenFile summary 10 s
						// after the first OpenFile, the interface check 10 s after the request
	if (HasOption(argc, argv, L"waitlong"))
		Sleep(31000);	// TweaksV's DirectStorage factory check comes 30 s after it started
	if (HasOption(argc, argv, L"dsotheronly"))
		Sleep(1500);	// the interface check's line comes 10 s after the request, a little after "wait" began
	if (HasOption(argc, argv, L"dsflood"))
		FloodOpenFile();

	unsigned int heap = 0;
	for (int i = 0; i < 4; i++)
		heap |= static_cast<unsigned int>(g_heapSite[1 + i]) << (8 * i);
	wprintf(L"heap: %u MB | ", heap >> 20);
	PrintBytes("splash", g_splashSite, 6);
	PrintBytes("legal", g_legalSite, 6);
	PrintBytes("landing", g_landingCallee, 5);
	PrintModkitRefs();
	PrintModkitLimits();
	PrintModkitCopy();
	ForEachSite(CheckSite);
	RovForEachDataSite(CheckData);
	PrintPackfile(argc, argv);
	PrintBoxStreamer();
	PrintDecals();
	RovReport();
	wprintf(L"%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls", g_dstorageResult, g_floodResult, g_shareResult, g_ownResult, g_fileOpensResult,
		g_replaceResult, g_renamesResult, g_renameFloodResult, g_scriptResult, g_blipResult);
	// The parts of the parts' coding round, the Legacy packs check and the limits measured first: each one's field of the
	// report, and the stub of a call it redirected
	wprintf(L"weapon infos: limits %u %u, loader %ls, the rest %ls, array %u of %u%ls, files %u passed %u refused, %u of %u as expected | ",
		WeaponInfosLimit(11, 4), WeaponInfosLimit(34, 2), WeaponInfosLoaderState(), WeaponInfosRestAsLaid() ? L"as laid" : L"CHANGED",
		static_cast<unsigned>(g_weaponInfosArray->count), static_cast<unsigned>(g_weaponInfosArray->capacity),
		g_weaponInfosArray->block == g_weaponInfosBlock ? L"" : L", its block CHANGED", g_weaponInfosPassed, g_weaponInfosRefused,
		g_weaponInfosAsExpected, g_weaponInfosCalls);
	WeaponComponentsReport();
	SirenIdsReport();
	SirenCountReport();
	CheckStub(Rel32Target(g_sirenCountRendererCall));
	CheckStub(Rel32Target(Site{ g_sirenCountRendererGetter, 0, 0 }));
	CheckStub(Rel32Target(g_sirenCountGetterCall));
	{
		// "pedprops: sites widened, data as placed | " - the sites as placed, as the part leaves them (the eleven B6 made
		// B7 and nothing else), or changed otherwise; the store's data as placed
		const uint64_t sites = PedPropsSum(true);
		wprintf(L"pedprops: sites %ls, data %ls | ", sites == g_pedPropsSitesSum ? L"as placed" : sites == g_pedPropsWidenedSum ? L"widened" : L"CHANGED",
			PedPropsSum(false) == g_pedPropsDataSum ? L"as placed" : L"CHANGED");
	}
	{
		// "texture budget: sites as placed 7 of 7, the call to the next step, data as set, its page RW, budgets 1500, 1500, 3000 and
		// 3000 MB, the most 98304; start-up: ...; stores RVA 0x..., ... | " - the sites as they were before TweaksV loaded but the call's
		// rel32 and where the call leads, the data as the harness set it but the budgets and the most and those as they are, the
		// start-up's call, and where each site and datum lies, for the suite to compare with the log's lines
		int asPlaced = 0;
		TextureBudgetSites(g_textureBudgetSitesAsPlaced, true, asPlaced);
		bool misreadAsPlaced = true;
		for (size_t i = 0; i < sizeof(g_textureBudgetMisread); i++)
			if (g_textureBudgetMisreadAsPlaced[i] != g_textureBudgetMisread[i])
				misreadAsPlaced = false;
		const TextureBudgetData& data = *g_textureBudgetData;
		TextureBudgetData is = data, set = g_textureBudgetDataAsSet;
		memset(is.counters.budgets, 0, sizeof(is.counters.budgets));
		memset(set.counters.budgets, 0, sizeof(set.counters.budgets));
		is.counters.most = set.counters.most = 0;
		CheckStub(Rel32Target(g_textureBudgetCall));
		const auto rva = [](const volatile void* p) {
			return static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(const_cast<const void*>(p)) - reinterpret_cast<uintptr_t>(ImageBase()));
		};
		wprintf(L"texture budget: sites as placed %d of 7, the call to %ls, data %ls, its page %ls, budgets %llu, %llu, %llu and %llu MB, the most %u%ls; %ls; the misread block %ls; ", asPlaced,
			TextureBudgetCallLeads(Rel32Target(g_textureBudgetCall)), memcmp(&is, &set, sizeof(is)) == 0 ? L"as set" : L"CHANGED",
			ProtectionOf(&data) == PAGE_READWRITE ? L"RW" : L"not RW", static_cast<unsigned long long>(data.counters.budgets[0] >> 20),
			static_cast<unsigned long long>(data.counters.budgets[1] >> 20), static_cast<unsigned long long>(data.counters.budgets[2] >> 20),
			static_cast<unsigned long long>(data.counters.budgets[3] >> 20), data.counters.most, g_textureBudgetLater, g_textureBudgetStartUp,
			misreadAsPlaced ? L"as placed" : L"CHANGED");
		wprintf(L"stores RVA 0x%llX, getter RVA 0x%llX, placement RVA 0x%llX, "
			L"update RVA 0x%llX, writers RVA 0x%llX, next step RVA 0x%llX, counters RVA 0x%llX, index RVA 0x%llX, flag RVA 0x%llX, switch RVA 0x%llX, misread RVA 0x%llX, "
			L"index page RVA 0x%llX, device start RVA 0x%llX, maker RVA 0x%llX, descriptor heap RVA 0x%llX, heap sizes RVA 0x%llX | ",
			rva(g_textureBudgetStores), rva(g_textureBudgetGetter),
			rva(g_textureBudgetPlacement), rva(g_textureBudgetUpdate), rva(g_textureBudgetWriters), rva(reinterpret_cast<const void*>(&TextureBudgetNextStep)),
			rva(&data.counters), rva(&data.index), rva(&data.flag), rva(&data.updateCheck), rva(g_textureBudgetMisread), rva(g_textureBudgetIndexPage),
			rva(g_textureBudgetHeapSite), rva(g_textureBudgetMaker), rva(&data.heap), rva(&data.heapSizes));
	}
	LpReport();
	RtReport();
	DwReport();
	StaticBoundsReport();
	LcReport();
	PopReport();
	PedPoolReport();
	PoolMemoryReport();
	RqReport(argc, argv);
	// [siren damage]
	SdReport();
	SirenJoinReport();
	CheckStub(Rel32Target(Site{ g_sdCaller, kSdMakerRel32, 0 }));
	CheckData(SirenCountCode(&SdMakerCarrier) + kSdAllocAt + kSdAllocationAt, 5);
	// [corona store]
	CcReport(module);
	CheckStub(Rel32Target(g_packCallSite));
	CheckStub(Rel32Target(g_scriptCallSite));
	CheckStub(Rel32Target(Site{ g_staticBoundsInit, kSbLookupRel, 0 }));
	for (const Site& site : g_boxCalls)
		CheckStub(Rel32Target(site));
	RovForEachCallTarget(CheckStub);
	wprintf(L"stubs: %d, %ls | ", g_stubs, g_writableStubs == 0 ? L"all RX" : L"NOT all RX");
	wprintf(L"protect: %ls %ls\n", g_notRestored == 0 ? L"all restored" : L"NOT restored",
		g_siteProtect == PAGE_EXECUTE_READ ? L"(RX)" : L"(not RX)");
	return 0;
}
