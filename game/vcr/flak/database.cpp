/**
  *  \file game/vcr/flak/database.cpp
  *  \brief Class game::vcr::flak::Database
  */

#include <cstring>
#include "game/vcr/flak/database.hpp"
#include "game/vcr/flak/structures.hpp"
#include "afl/except/fileformatexception.hpp"

game::vcr::flak::Database::Database()
    : m_battles()
{
    // ex GFlakVcrDatabase::GFlakVcrDatabase
}

game::vcr::flak::Database::~Database()
{ }

void
game::vcr::flak::Database::load(afl::io::Stream& file, afl::charset::Charset& charset, afl::string::Translator& tx)
{
    // ex GFlakVcrDatabase::loadFromFile

    structures::Header header;
    file.fullRead(afl::base::fromObject(header));

    if (std::memcmp(header.magic, structures::FLAK_MAGIC, sizeof(structures::FLAK_MAGIC)) != 0) {
        throw afl::except::FileFormatException(file, tx("File is missing required signature"));
    }
    if (header.filefmt_version != 0) {
        throw afl::except::FileFormatException(file, tx("Unsupported file format version"));
    }

    m_timestamp = header.timestamp;

    for (int i = 0, n = header.num_battles; i < n; ++i) {
        addNewBattle(readOneBattle(file, charset, tx));
    }
}

game::Timestamp
game::vcr::flak::Database::getTimestamp() const
{
    // ex GFlakVcrDatabase::getTimestamp
    return m_timestamp;
}

game::vcr::flak::Battle*
game::vcr::flak::Database::addNewBattle(Battle* battle)
{
    // ex GFlakVcrDatabase::addBattle
    return m_battles.pushBackNew(battle);
}

// game::vcr::Database methods:
size_t
game::vcr::flak::Database::getNumBattles() const
{
    // ex GFlakVcrDatabase::getNumBattles
    return m_battles.size();
}

game::vcr::flak::Battle*
game::vcr::flak::Database::getBattle(size_t nr)
{
    // ex GFlakVcrDatabase::getBattle
    if (nr < m_battles.size()) {
        return m_battles[nr];
    } else {
        return 0;
    }
}

game::vcr::flak::Battle*
game::vcr::flak::Database::readOneBattle(afl::io::Stream& file, afl::charset::Charset& charset, afl::string::Translator& tx)
{
    // ex GFlakVcrDatabase::readOneBattle
    // First word is size
    afl::base::GrowableBytes_t data;
    data.resize(4);
    file.fullRead(data);

    afl::bits::Value<afl::bits::UInt32LE> rawSize;
    afl::base::fromObject(rawSize).copyFrom(data);
    const uint32_t size = rawSize;

    // DoS protection/avoid unbounded allocation: assume a maximum-size battle with 1000 ships, 1000 fleets
    // - header                                             4 bytes
    // - 1000 fleets x 24 bytes                         24000 bytes
    // - 1000 ships x 56 bytes                          56000 bytes
    // - 1000 x 1000 attack list entries x 4 bytes    4000000 bytes
    // = total                                        4080004 bytes
    const uint32_t MAX_SHIPS = 1000;
    const uint32_t MAX_SIZE
        = sizeof(structures::Fleet) * MAX_SHIPS
        + sizeof(structures::Ship)  * MAX_SHIPS
        + 4 * MAX_SHIPS * MAX_SHIPS
        + 4;
    if (size > MAX_SIZE) {
        throw afl::except::FileFormatException(file, tx("Battle too large"));
    }

    // Read content
    data.resize(size);
    file.fullRead(data.subrange(4));

    // Build the setup (this may throw)
    std::auto_ptr<Setup> result(new Setup());
    result->load(file.getName(), data, charset, tx);

    // Convert to battle
    return new Battle(result);
}
