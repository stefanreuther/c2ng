/**
  *  \file game/v3/fizzfile.cpp
  *  \brief Class game::v3::FizzFile
  *
  *  PCC2 Comment:
  *
  *  This module provides functions to access the checksum part of
  *  FIZZ.BIN. This information mirrors the information stored in the
  *  GEN file (see game/gen.cc) and serves as an additional
  *  consistency check for Tim's software. We only write it, but don't
  *  validate it. The registration part is handled in game/reg.cc, not
  *  here.
  */

#include "game/v3/fizzfile.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/ptr.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/pack.hpp"
#include "afl/io/stream.hpp"

static_assert(game::v3::structures::ShipSection   == 0, "ShipSection");
static_assert(game::v3::structures::PlanetSection == 1, "PlanetSection");
static_assert(game::v3::structures::BaseSection   == 2, "BaseSection");

namespace {
    /** File name. */
    const char*const FILE_NAME = "fizz.bin";

    /** Checksum adjustment.
        The checksums stored in FIZZ.BIN are offset by this amount relative to GENx.DAT checksums. */
    // ex fizz_adjust
    const uint32_t ADJUST[] = { 667, 1667, 1262 };
}


game::v3::FizzFile::FizzFile()
{
    clear();
}

// Reset.
void
game::v3::FizzFile::clear()
{
    m_valid = false;
    afl::base::Memory<uint32_t>(m_data).fill(0);
}

// Load data from directory.
void
game::v3::FizzFile::load(afl::io::Directory& dir)
{
    // ex loadFizz
    // @change Failure to load is not an error
    uint8_t buffer[sizeof(m_data)];
    afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(FILE_NAME, afl::io::FileSystem::OpenRead);
    if (file.get() != 0 && file->read(buffer) == sizeof(buffer)) {
        // Success. Remember checksums.
        afl::bits::unpackArray<afl::bits::UInt32LE>(m_data, buffer);
        m_valid = true;
    } else {
        // Failure. Assume invalid.
        clear();
    }
}

// Save data to directory.
void
game::v3::FizzFile::save(afl::io::Directory& dir)
{
    // ex saveFizz
    if (m_valid) {
        // This opens the file in write-existing mode, and writes just the checksum portion of the file,
        // which is at the beginning of the file. Hence, the registration info will be kept intact.
        afl::base::Ptr<afl::io::Stream> file = dir.openFileNT(FILE_NAME, afl::io::FileSystem::OpenWrite);
        if (file.get() != 0) {
            uint8_t buffer[sizeof(m_data)];
            afl::bits::packArray<afl::bits::UInt32LE>(buffer, m_data);
            file->fullWrite(buffer);
        }
    }
}

// Set checksum.
void
game::v3::FizzFile::set(Section section, int player, uint32_t checksum)
{
    // ex setFizzEntry
    if (section >= 0 && section < 3 && player > 0 && player <= NUM_PLAYERS) {
        m_data[3*(player-1) + section] = checksum + ADJUST[section];
    }
}

bool
game::v3::FizzFile::isValid() const
{
    return m_valid;
}
