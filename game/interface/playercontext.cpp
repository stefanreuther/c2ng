/**
  *  \file game/interface/playercontext.cpp
  */

#include "game/interface/playercontext.hpp"
#include "afl/string/format.hpp"
#include "game/interface/playerproperty.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/typehint.hpp"

namespace {
    enum { PlayerProperty };

    static const interpreter::NameTable player_mapping[] = {
        { "BASES",               game::interface::iplScoreBases,       PlayerProperty, interpreter::thInt },
        { "PBPS",                game::interface::iplPBPs,             PlayerProperty, interpreter::thInt },
        { "PLANETS",             game::interface::iplScorePlanets,     PlayerProperty, interpreter::thInt },
        { "RACE",                game::interface::iplFullName,         PlayerProperty, interpreter::thString },
        { "RACE$",               game::interface::iplId,               PlayerProperty, interpreter::thInt },
        { "RACE.ADJ",            game::interface::iplAdjName,          PlayerProperty, interpreter::thString },
        { "RACE.ID",             game::interface::iplRaceId,           PlayerProperty, interpreter::thInt },
        { "RACE.MISSION",        game::interface::iplMission,          PlayerProperty, interpreter::thInt },
        { "RACE.SHORT",          game::interface::iplShortName,        PlayerProperty, interpreter::thString },
        { "SCORE",               game::interface::iplScore,            PlayerProperty, interpreter::thInt },
        { "SHIPS",               game::interface::iplScoreShips,       PlayerProperty, interpreter::thInt },
        { "SHIPS.CAPITAL",       game::interface::iplScoreCapital,     PlayerProperty, interpreter::thInt },
        { "SHIPS.FREIGHTERS",    game::interface::iplScoreFreighters,  PlayerProperty, interpreter::thInt },
        { "TEAM",                game::interface::iplTeam,             PlayerProperty, interpreter::thInt },
    };
}

game::interface::PlayerContext::PlayerContext(int nr, afl::base::Ref<Game> game, afl::base::Ref<Root> root)
    : m_number(nr),
      m_game(game),
      m_root(root)
{ }

game::interface::PlayerContext::~PlayerContext()
{ }

// Context:
game::interface::PlayerContext*
game::interface::PlayerContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntPlayerContext::lookup
    return lookupName(name, player_mapping, result) ? this : 0;
}

void
game::interface::PlayerContext::set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
{
    // ex IntPlayerContext::set
    throw interpreter::Error::notAssignable();
}

afl::data::Value*
game::interface::PlayerContext::get(PropertyIndex_t index)
{
    // ex IntPlayerContext::get
    return getPlayerProperty(m_number, PlayerProperty(player_mapping[index].index), m_root->playerList(), *m_game, m_root->hostConfiguration());
}

bool
game::interface::PlayerContext::next()
{
    // ex IntPlayerContext::next
    // Find next player until we have a real one.
    Player* p = m_root->playerList().getNextPlayer(m_number);
    while (p != 0 && !p->isReal()) {
        p = m_root->playerList().getNextPlayer(p);
    }

    // Evaluate
    if (p != 0) {
        m_number = p->getId();
        return true;
    } else {
        return false;
    }
}

game::interface::PlayerContext*
game::interface::PlayerContext::clone() const
{
    // ex IntPlayerContext::clone
    return new PlayerContext(m_number, m_game, m_root);
}

game::map::Object*
game::interface::PlayerContext::getObject()
{
    // ex IntPlayerContext::getObject
    return 0;
}

void
game::interface::PlayerContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntPlayerContext::enumProperties
    acceptor.enumTable(player_mapping);
}

// BaseValue:
String_t
game::interface::PlayerContext::toString(bool /*readable*/) const
{
    // ex IntPlayerContext::toString
    return afl::string::Format("Player(%d)", m_number);
}

void
game::interface::PlayerContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntPlayerContext::store
    out.tag = out.Tag_Player;
    out.value = m_number;
}

game::interface::PlayerContext*
game::interface::PlayerContext::create(int nr, Session& session)
{
    // ex values.pas:CreatePlayerContext
    // Valid state?
    Game* g = session.getGame().get();
    Root* r = session.getRoot().get();
    if (g == 0 || r == 0) {
        return 0;
    }

    // Valid player number?
    // \change: This ought to have a "pl->isReal" check which I deliberately omitted.
    // This allows scripts to do "Player(0)" or "Player(12)" to access special slots.
    Player* pl = r->playerList().get(nr);
    if (pl == 0) {
        return 0;
    }

    return new PlayerContext(nr, *g, *r);
}

