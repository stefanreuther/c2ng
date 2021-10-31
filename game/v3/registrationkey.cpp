/**
  *  \file game/v3/registrationkey.cpp
  *  \brief Class game::v3::RegistrationKey
  *
  *  This contains only code to *read* registrations, none to
  *  create them. Do not add more modificators.
  */

#include "game/v3/registrationkey.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/pack.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/string/format.hpp"
#include "version.hpp"

namespace {
    /** Magic numbers signifying an unregistered key. */
    const uint32_t UNREGISTERED_KEY[game::v3::RegistrationKey::KEY_SIZE_WORDS] = {
         1118,  1846,  2535,  1664,  5200,  8424,  8827, 11440, 11817, 15080,
        16445,  4992, 19435, 18928, 18915, 23712, 22321, 27846, 23959, 29640,
        27573,  9152,  9568,  9984, 10400,  1040,  1742,  2613,  1664,  4745,
         5694,  2912,  3328,  3744,  4160,  4576,  4992,  5408,  5824,  6240,
         6656,  7072,  7488,  7904,  8320,  8736,  9152,  9568,  9984, 10400,
        485451
    };

    /** Layout of FIZZ.BIN. */
    struct Fizz {
        uint8_t pad[136];
        afl::bits::Value<afl::bits::UInt32LE> key[game::v3::RegistrationKey::KEY_SIZE_WORDS];
    };
    static_assert(sizeof(Fizz) == 340, "sizeof Fizz");

    /** Layout of REG.KEY. */
    struct Key {
        afl::bits::Value<afl::bits::FixedString<50> > string3;
        afl::bits::Value<afl::bits::FixedString<50> > string4;
        uint8_t pad[50];
        afl::bits::Value<afl::bits::UInt32LE> flag;
    };
    static_assert(sizeof(Key) == 154, "sizeof Key");
}

// Constructor.
game::v3::RegistrationKey::RegistrationKey(std::auto_ptr<afl::charset::Charset> charset)
    : m_charset(charset),
      m_winplanString1(),
      m_winplanString2(),
      m_isValid(false)
{
    initUnregistered();
    initUnowned();
}

game::RegistrationKey::Status
game::v3::RegistrationKey::getStatus() const
{
    // ex GRegInfo::isRegistered
    if (!m_isValid) {
        return Unknown;
    } else {
        for (size_t i = 0; i < 25; ++i) {
            if (m_fizz[i] != UNREGISTERED_KEY[i]) {
                return Registered;
            }
        }
        return Unregistered;
    }
}

String_t
game::v3::RegistrationKey::getLine(Line which) const
{
    switch (which) {
     case Line1:
        // Registration string 1 (name of player/serial).
        // ex GRegInfo::getRegStr1
        return decode(0);

     case Line2:
        // Registration string 2 (town of player/reg date).
        // ex GRegInfo::getRegStr2
        return decode(25);

     case Line3:
        // Registration string 3 (name as entered by player).
        return m_winplanString1;

     case Line4:
        // Registration string 4 (address as entered by player).
        return m_winplanString2;
    }
    return String_t();
}

bool
game::v3::RegistrationKey::setLine(Line which, String_t value)
{
    switch (which) {
     case Line1:
     case Line2:
        break;

     case Line3:
        m_winplanString1 = value;
        return true;

     case Line4:
        m_winplanString2 = value;
        return true;
    }
    return false;
}

int
game::v3::RegistrationKey::getMaxTechLevel(TechLevel /*area*/) const
{
    return (getStatus() == Registered ? 10 : 6);
}

// Create unregistered key.
void
game::v3::RegistrationKey::initUnregistered()
{
    // ex GRegInfo::initUnregistered
    initFromFizz(UNREGISTERED_KEY);
    m_isValid = false;
}

// Make this key "unowned" (name/address not set).
void
game::v3::RegistrationKey::initUnowned()
{
    // ex GRegInfo::initUnowned
    m_winplanString1 = "Client: PCC2 NG (v" PCC2_VERSION ")";
    m_winplanString2 = PCC2_URL;
}

// Initialize by loading files from a directory.
void
game::v3::RegistrationKey::initFromDirectory(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex GRegInfo::initFromDirectory
    try {
        m_isValid = false;

        afl::base::Ref<afl::io::Stream> s = dir.openFile("fizz.bin", afl::io::FileSystem::OpenRead);
        parseFizz(*s);

        // Check for reg.key in parent directory.
        // (PCC2 also checks for presence of winplan.exe, but otherwise has the same restriction.
        // PCC1 has a freely-configurable Winplan path and therefore always finds the key.)
        afl::base::Ptr<afl::io::Directory> parent = dir.getParentDirectory();
        if (parent.get() != 0) {
            afl::base::Ptr<afl::io::Stream> s = parent->openFileNT("reg.key", afl::io::FileSystem::OpenRead);
            if (s.get() != 0) {
                parseKey(*s, tx);
            }
        }
    }
    catch (afl::except::FileProblemException& e) {
        initUnregistered();
        initUnowned();
        log.write(log.Warn, "game.v3.reg", e.getFileName(), 0, e.what());
        log.write(log.Warn, "game.v3.reg", tx("No usable registration key found, assuming unregistered player"));
    }
}

// Save to given stream.
void
game::v3::RegistrationKey::saveToStream(afl::io::Stream& file)
{
    Fizz buf;
    afl::base::Bytes_t(buf.pad).fill(0);
    packIntoBytes(afl::base::fromObject(buf.key));
    file.fullWrite(afl::base::fromObject(buf));
}

// Initialize from a data array.
void
game::v3::RegistrationKey::unpackFromBytes(afl::base::ConstBytes_t bytes)
{
    afl::bits::unpackArray<afl::bits::UInt32LE>(m_fizz, bytes);
    m_isValid = true;
}

// Store into data array.
void
game::v3::RegistrationKey::packIntoBytes(afl::base::Bytes_t bytes) const
{
    // ex GRegInfo::getKey (sort-of)
    afl::bits::packArray<afl::bits::UInt32LE>(bytes, m_fizz);
}

// Get key Id.
String_t
game::v3::RegistrationKey::getKeyId() const
{
    uint8_t bytes[KEY_SIZE_BYTES];
    packIntoBytes(bytes);

    afl::checksums::SHA1 hasher;
    hasher.add(bytes);
    return hasher.getHashAsHexString();
}

/** Initialize registration strings from data of a FIZZ.BIN file.
    \param data 51 dwords (offset 136 of a FIZZ.BIN file). */
inline void
game::v3::RegistrationKey::initFromFizz(const uint32_t (&data)[KEY_SIZE_WORDS])
{
    // ex GRegInfo::initFromFizz(const uint32_t* data)
    afl::base::Memory<uint32_t>(m_fizz).copyFrom(data);
}

/** Initialize from FIZZ.BIN.
    \param s FIZZ.BIN file, file pointer at beginning. */
inline void
game::v3::RegistrationKey::parseFizz(afl::io::Stream& s)
{
    // ex GRegInfo::parseFizz
    Fizz buffer;
    s.fullRead(afl::base::fromObject(buffer));
    unpackFromBytes(afl::base::fromObject(buffer.key));
}

/** Initialize from REG.KEY.
    \param s REG.KEY file, file pointer at beginning. */
inline void
game::v3::RegistrationKey::parseKey(afl::io::Stream& s, afl::string::Translator& tx)
{
    // ex GRegInfo::parseKey
    Key buffer;
    uint32_t flag;

    s.fullRead(afl::base::fromObject(buffer));
    flag = buffer.flag;
    if (flag == 13) {
        /* valid key */
        for (size_t i = 0; i < 50; ++i) {
            buffer.string3.m_bytes[i] -= 13;
            buffer.string4.m_bytes[i] -= 13;
        }
        m_winplanString1 = m_charset->decode(buffer.string3);
        m_winplanString2 = m_charset->decode(buffer.string4);
    } else if (flag == 666771) {
        /* valid, but not unlocked */
        initUnowned();
    } else {
        /* reject */
        throw afl::except::FileFormatException(s, tx("File is invalid"));
    }
}

String_t
game::v3::RegistrationKey::decode(size_t start) const
{
    // ex GRegInfo::decode
    /* hehe. */
    uint8_t buffer[25];
    for (size_t i = 0; i < 25; ++i) {
        buffer[i] = uint8_t(m_fizz[start + i] * 5042 / (i+1) >> 16);
    }
    return m_charset->decode(afl::bits::unpackFixedString(buffer));
}
