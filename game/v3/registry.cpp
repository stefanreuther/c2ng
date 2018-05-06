/**
  *  \file game/v3/registry.cpp
  *  \brief Winplan Game Registry
  */

#include "game/v3/registry.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/value.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/io/stream.hpp"
#include "util/randomnumbergenerator.hpp"

using afl::base::Ptr;
using afl::base::Ref;

namespace {
    const size_t Num_Slots = 40;
    const size_t Templock_Size = 44;
    struct Data {
        afl::bits::Value<afl::bits::UInt16LE> slot;
        uint8_t timestamps[Num_Slots][18];
        uint8_t templock[Num_Slots][Templock_Size];
    };
    static_assert(sizeof(Data) == 2 + 18*Num_Slots + Templock_Size*Num_Slots, "sizeof Data");
}


void
game::v3::updateGameRegistry(afl::io::Directory& gameDirectory, const Timestamp& time)
{
    // Registry is only relevant for games that are beneath a Winplan installation, so look one up
    Ptr<afl::io::Directory> dir = gameDirectory.getParentDirectory();
    if (dir.get() == 0) {
        return;
    }

    // Only update file if it exists
    Ptr<afl::io::Stream> s = dir->openFileNT("snooker.dat", afl::io::FileSystem::OpenWrite);
    if (s.get() == 0) {
        return;
    }

    // If the file is truncated, we'll happily repair it
    Data fileData;
    afl::base::Bytes_t fileBytes(afl::base::fromObject(fileData));
    fileBytes.fill(0);
    s->read(fileBytes);

    // Look for our entry
    for (size_t i = 0; i < Num_Slots; ++i) {
        if (time == fileData.timestamps[i]) {
            return;
        }
    }

    // Entry not found, so add it
    // To make the generated templock random as specified, but avoid the need to provide an entropy source,
    // seed the RNG with the previous content of the file.
    util::RandomNumberGenerator rng(afl::checksums::ByteSum().add(fileBytes, 0));

    size_t slot = fileData.slot + 1;
    if (slot < 1 || slot > Num_Slots) {
        slot = 1;
    }
    fileData.slot = static_cast<uint16_t>(slot);
    time.storeRawData(fileData.timestamps[slot-1]);
    for (size_t byte = 0; byte < Templock_Size; ++byte) {
        fileData.templock[slot-1][byte] = uint8_t(rng(256));
    }

    // Write back file
    s->setPos(0);
    s->fullWrite(fileBytes);
}
