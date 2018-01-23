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
