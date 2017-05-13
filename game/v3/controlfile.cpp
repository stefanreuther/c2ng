/**
  *  \file game/v3/controlfile.cpp
  *  \brief Class game::v3::ControlFile
  */

#include "game/v3/controlfile.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/pack.hpp"

using afl::io::Stream;
using afl::io::FileSystem;
using afl::base::Ptr;
namespace gs = game::v3::structures;

const size_t game::v3::ControlFile::CONTROL_MIN;
const size_t game::v3::ControlFile::CONTROL_MAX;

// Default constructor.
game::v3::ControlFile::ControlFile()
    : m_fileOwner(0)
{
    clear();
}

// Reset.
void
game::v3::ControlFile::clear()
{
    afl::base::fromObject(m_data).fill(0);
    m_fileOwner = -1;
}

// Load data from directory.
void
game::v3::ControlFile::load(afl::io::Directory& dir, int player)
{
    // ex game/checksum.h:loadControl
    clear();

    Ptr<Stream> f = dir.openFileNT("control.dat", FileSystem::OpenRead);
    if (f.get() != 0) {
        // Successfully opened Dosplan file
        m_fileOwner = 0;
    } else {
        // No Dosplan file, try Winplan file
        f = dir.openFileNT(afl::string::Format("contrl%d.dat", player), FileSystem::OpenRead);
        if (f.get() != 0) {
            // Successfully opened Winplan file
            m_fileOwner = player;
        } else {
            // No file.
            // FIXME: console.write(LOG_INFO, _("No control file (checksums) loaded"));
            m_fileOwner = -1;
        }
    }

    // Load the file
    if (f.get() != 0) {
        uint8_t buffer[CONTROL_MAX * 4];
        size_t bytesRead = f->read(buffer);
        if (bytesRead > 0) {
            size_t slotsRead = bytesRead / 4;
            afl::bits::unpackArray<afl::bits::UInt32LE>(m_data, afl::base::ConstBytes_t(buffer).trim(4 * slotsRead));
        }
    }
}

// Save data to directory.
void
game::v3::ControlFile::save(afl::io::Directory& dir)
{
    // ex game/checksum.h:saveControl
    if (m_fileOwner < 0) {
        // We did not load a file, so we do not save one.
        // FIXME: console.write(LOG_INFO, _("Control file (checksums) will not be created"));
    } else {
        // Save it
        Ptr<Stream> f = dir.openFileNT(m_fileOwner == 0
                                       ? String_t("control.dat")
                                       : String_t(afl::string::Format("contrl%d.dat", m_fileOwner)),
                                       FileSystem::Create);
        if (f.get() == 0) {
            // Creating the file failed. This is not fatal for us.
            // FIXME: console.write(LOG_ERROR, _("Control file (checksums) can't be created"));
        } else {
            // Figure out size of file. Normally, this is 6002 bytes (which is 1500.5 longs).
            // In case of a Host999 game, write the full maximum.
            size_t size = 6002;
            uint8_t buffer[CONTROL_MAX * 4];
            buffer[6000] = buffer[6001] = 0;

            for (size_t i = CONTROL_MIN; i < CONTROL_MAX; ++i) {
                if (m_data[i] != 0) {
                    size = CONTROL_MAX*4;
                    break;
                }
            }

            afl::base::Bytes_t actualBuffer(buffer);
            actualBuffer.trim(size);

            afl::bits::packArray<afl::bits::UInt32LE>(actualBuffer, m_data);
            f->fullWrite(actualBuffer);
        }
    }
}

// Set checksum.
void
game::v3::ControlFile::set(Section section, Id_t id, uint32_t checksum)
{
    // ex game/checksum.h:setControlEntry
    if (Value_t* p = getSlot(section, id)) {
        *p = checksum;
    }
}

// Set file owner.
void
game::v3::ControlFile::setFileOwner(int owner)
{
    m_fileOwner = owner;
}

game::v3::ControlFile::Value_t*
game::v3::ControlFile::getSlot(Section section, Id_t id)
{
    // ex game/checksum.cc:getControlIndex
    switch (section) {
     case gs::ShipSection:
        if (id > 0 && id <= 500) {
            return &m_data[id-1];
        } else if (id > 500 && id <= 999) {
            return &m_data[id+1499];
        } else {
            return 0;
        }
     case gs::PlanetSection:
        if (id > 0 && id <= 500) {
            return &m_data[id + 499];
        } else {
            return 0;
        }
     case gs::BaseSection:
        if (id > 0 && id <= 500) {
            return &m_data[id + 999];
        } else {
            return 0;
        }
    }
    return 0;
}
