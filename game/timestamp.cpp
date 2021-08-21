/**
  *  \file game/timestamp.cpp
  *  \brief Class game::Timestamp
  *
  *  Changes to PCC2 version:
  *  - if two timestamps compare equal in isEarlierThan(), delimiters are used as tie-breakers.
  *    Therefore, if a!=b, we will now always have a.isEarlierThan(b) or b.isEarlierThan(a).
  *    This happens on syntactically-invalid timestamps.
  *  - conversion always uses Latin-1.
  *    Since timestamps are normally ASCII only, this only happens on syntactically-invalid timestamps.
  */

#include <cstring>
#include "game/timestamp.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/base/staticassert.hpp"

namespace {
    /** Index table.
        This defines the order in which we compare a timestamp. */
    static const uint8_t INDEX_TABLE[] = {
        6, 7, 8, 9,                 // year
        0, 1,                       // month
        3, 4,                       // day
        10, 11,                     // hour
        13, 14,                     // minute
        16, 17,                     // second
        2, 5, 12, 15                // delimiters
    };


    /** Decode string. */
    String_t decode(afl::base::ConstBytes_t data)
    {
        return afl::charset::CodepageCharset(afl::charset::g_codepageLatin1).decode(data);
    }

    /** Convert digit to ASCII representation. */
    uint8_t digit(int n)
    {
        return uint8_t('0' + (n % 10));
    }

    const char NULL_TIMESTAMP[] = "00-00-000000:00:00";
}

// Construct from binary representation.
game::Timestamp::Timestamp(ConstData_t data)
{
    std::memcpy(m_data, data, SIZE);
}

// Construct from parts.
game::Timestamp::Timestamp(int year, int month, int day,
                           int hour, int minute, int second)
{
    // mm-dd-yyyyhh:mm:ss
    m_data[0] = digit(month / 10);
    m_data[1] = digit(month);
    m_data[2] = '-';
    m_data[3] = digit(day / 10);
    m_data[4] = digit(day);
    m_data[5] = '-';
    m_data[6] = digit(year / 1000);
    m_data[7] = digit(year / 100);
    m_data[8] = digit(year / 10);
    m_data[9] = digit(year);
    m_data[10] = digit(hour / 10);
    m_data[11] = digit(hour);
    m_data[12] = ':';
    m_data[13] = digit(minute / 10);
    m_data[14] = digit(minute);
    m_data[15] = ':';
    m_data[16] = digit(second / 10);
    m_data[17] = digit(second);
}

// Construct empty timestamp.
game::Timestamp::Timestamp()
{
    static_assert(sizeof(NULL_TIMESTAMP) >= SIZE, "Timestamp size");
    std::memcpy(m_data, NULL_TIMESTAMP, SIZE);
}


// Get whole timestamp (18 characters) as string.
String_t
game::Timestamp::getTimestampAsString() const
{
    return decode(m_data);
}

// Get time (8 characters, hh:mm:ss) as string.
String_t
game::Timestamp::getTimeAsString() const
{
    return decode(afl::base::ConstBytes_t(m_data).subrange(10));
}

// Get date (10 characters, mm-dd-yyyy) as string.
String_t
game::Timestamp::getDateAsString() const
{
    return decode(afl::base::ConstBytes_t(m_data).subrange(0, 10));
}

// Get raw data.
game::Timestamp::ConstData_t
game::Timestamp::getRawData() const
{
    return m_data;
}

// Store raw data in data field (array of 18 chars).
void
game::Timestamp::storeRawData(Data_t data) const
{
    std::memcpy(data, m_data, SIZE);
}

// Compare for equality.
bool
game::Timestamp::operator==(const Timestamp& rhs) const
{
    return std::memcmp(m_data, rhs.m_data, SIZE) == 0;
}

bool
game::Timestamp::operator==(ConstData_t rhs) const
{
    return std::memcmp(m_data, rhs, SIZE) == 0;
}

// Compare for inequality.
bool
game::Timestamp::operator!=(const Timestamp& rhs) const
{
    return std::memcmp(m_data, rhs.m_data, SIZE) != 0;
}

bool
game::Timestamp::operator!=(ConstData_t rhs) const
{
    return std::memcmp(m_data, rhs, SIZE) != 0;
}

// Compare two timestamps.
bool
game::Timestamp::isEarlierThan(const game::Timestamp& other) const
{
    // ex ccload.pas:StampLater
    for (size_t i = 0; i < countof(INDEX_TABLE); ++i) {
        if (m_data[INDEX_TABLE[i]] != other.m_data[INDEX_TABLE[i]]) {
            return m_data[INDEX_TABLE[i]] < other.m_data[INDEX_TABLE[i]];
        }
    }
    return false;
}

bool
game::Timestamp::isValid() const
{
    static_assert(sizeof(NULL_TIMESTAMP) >= SIZE, "Timestamp size");
    return std::memcmp(m_data, NULL_TIMESTAMP, SIZE) != 0;
}
