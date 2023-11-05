/**
  *  \file server/play/racenamepacker.cpp
  *  \brief Class server::play::RaceNamePacker
  */

#include "server/play/racenamepacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using game::Player;

server::play::RaceNamePacker::RaceNamePacker(const game::Root& root, int firstSlot, afl::string::Translator& tx)
    : m_root(root), m_firstSlot(firstSlot), m_translator(tx)
{ }

// Packer:
server::Value_t*
server::play::RaceNamePacker::buildValue() const
{
    // Add only real players.
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = m_firstSlot; i <= game::MAX_PLAYERS; ++i) {
        const Player* pl = m_root.playerList().get(i);
        if (pl != 0 && pl->isReal()) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            hv->setNew("RACE",         makeStringValue(pl->getName(Player::LongName,      m_translator)));
            hv->setNew("RACE.ADJ",     makeStringValue(pl->getName(Player::AdjectiveName, m_translator)));
            hv->setNew("RACE.ID",      makeIntegerValue(m_root.hostConfiguration().getPlayerRaceNumber(i)));
            hv->setNew("RACE.MISSION", makeIntegerValue(m_root.hostConfiguration().getPlayerMissionNumber(i)));
            hv->setNew("RACE.SHORT",   makeStringValue(pl->getName(Player::ShortName,     m_translator)));
            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }

    return new afl::data::VectorValue(vv);
}

String_t
server::play::RaceNamePacker::getName() const
{
    return "racename";
}
