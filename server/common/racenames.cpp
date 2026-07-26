/**
  *  \file server/common/racenames.cpp
  *  \brief Class server::common::RaceNames
  */

#include "server/common/racenames.hpp"
#include "game/v3/structures.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"

// Race name storage.
server::common::RaceNames::RaceNames()
    : m_shortNames(),
      m_longNames(),
      m_adjectiveNames()
{ }

// Destructor.
server::common::RaceNames::~RaceNames()
{ }

// Load from array-of-bytes.
void
server::common::RaceNames::load(afl::base::ConstBytes_t data, afl::charset::Charset& cs)
{
    // Parse
    game::v3::structures::RaceNames in;
    if (data.size() < sizeof(in)) {
        throw afl::except::FileTooShortException("<race.nm>");
    }
    afl::base::fromObject(in).copyFrom(data);

    // Convert
    for (int player = 0; player < game::v3::structures::NUM_PLAYERS; ++player) {
        m_longNames.set(player+1, cs.decode(in.longNames[player]));
        m_shortNames.set(player+1, cs.decode(in.shortNames[player]));
        m_adjectiveNames.set(player+1, cs.decode(in.adjectiveNames[player]));
    }
}

// Save to array-of-bytes.
void
server::common::RaceNames::save(afl::base::GrowableBytes_t& data, afl::charset::Charset& cs) const
{
    game::v3::structures::RaceNames out;
    for (int player = 0; player < game::v3::structures::NUM_PLAYERS; ++player) {
        out.longNames[player] = cs.encode(afl::string::toMemory(m_longNames.get(player+1)));
        out.shortNames[player] = cs.encode(afl::string::toMemory(m_shortNames.get(player+1)));
        out.adjectiveNames[player] = cs.encode(afl::string::toMemory(m_adjectiveNames.get(player+1)));
    }
    data.append(afl::base::fromObject(out));
}

// Copy race names.
void
server::common::RaceNames::copy(int toSlot, const RaceNames& from, int fromSlot)
{
    m_longNames.set(toSlot, from.m_longNames.get(fromSlot));
    m_shortNames.set(toSlot, from.m_shortNames.get(fromSlot));
    m_adjectiveNames.set(toSlot, from.m_adjectiveNames.get(fromSlot));
}

// Access short names.
const game::PlayerArray<String_t>&
server::common::RaceNames::shortNames() const
{
    return m_shortNames;
}

// Access long names.
const game::PlayerArray<String_t>&
server::common::RaceNames::longNames() const
{
    return m_longNames;
}

// Access adjectives.
const game::PlayerArray<String_t>&
server::common::RaceNames::adjectiveNames() const
{
    return m_adjectiveNames;
}
