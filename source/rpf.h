// SPDX-License-Identifier: GPL-3.0-only
// TweaksV - the RPF7 facts that RageOpenV and its check for packs made for GTA V Legacy share (PATTERNS.md, "RageOpenV"):
// each number both rely on in one place, so that the two never disagree.
#pragma once

#include <cstddef>
#include <cstdint>

namespace TweaksV::Rpf
{
	// ---- an RPF7 archive: a header, then the entry table, then the names, each ending in a zero

	constexpr uint32_t kMagic = 0x52504637;			// "RPF7" as a dword: the bytes 37 46 50 52
	constexpr size_t kHeaderSize = 16;				// the magic, the entry count, the names' length, the encryption
	constexpr size_t kHeaderCountField = 4;			// uint32: the entries of the table
	constexpr size_t kHeaderNamesField = 8;			// uint32: the names' length in bytes
	constexpr size_t kHeaderEncryptionField = 12;	// uint32: kOpenTag, the game's encryption, or 0, which names none
	constexpr uint32_t kOpenTag = 0x4E45504F;		// "OPEN": the encryption field of an archive that is not encrypted
	constexpr uint32_t kNgMask = 0x0FF00000;		// the game's FindEncryption answers NG-encrypted when these bits...
	constexpr uint32_t kNgTag = 0x0FE00000;			// ...hold this
	constexpr uint32_t kAesTag = 0x0FFFFFF9;
	constexpr int32_t kMaxEntries = 0x100000;		// an entry table's count: more means the header was not read right
	constexpr size_t kEntrySize = 16;				// every entry of the table
	constexpr size_t kEntryNameField = 0;			// uint16: a file's name offset into the names
	constexpr size_t kEntryStoredField = 2;			// 3 bytes: a binary's compressed size, 0 for one stored
	constexpr size_t kEntryTypeField = 4;			// uint32: kFolderType, or kResourceBit set, else a binary
	constexpr size_t kEntryBlockField = 5;			// 3 bytes: the offset in blocks of kBlockSize - its top bit is kResourceBit
	constexpr size_t kEntryDataSizeField = 8;		// uint32: a binary's size; a resource's system flags
	constexpr size_t kEntryEncryptedField = 12;		// uint32: a binary's encryption; a resource's graphics flags
	constexpr uint32_t kFolderType = 0x7FFFFF00;
	constexpr uint32_t kResourceBit = 0x80000000;
	constexpr uint64_t kBlockSize = 512;			// a binary's offset counts these, from the start of its archive
}
