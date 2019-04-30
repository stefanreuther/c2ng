/**
  *  \file server/play/playerpacker.cpp
  */

#include "server/play/playerpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/playercontext.hpp"
#include "game/player.hpp"
#include "game/playerlist.hpp"
#include "game/root.hpp"

server::play::PlayerPacker::PlayerPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::PlayerPacker::buildValue() const
{
    // ex ServerPlayerWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Root& r = game::actions::mustHaveRoot(m_session);

    // Start at 0, and add only real players.
    // This means the 0=none and 12=aliens slot remain empty.
    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    for (int i = 0; i <= game::MAX_PLAYERS; ++i) {
        game::Player* pl = r.playerList().get(i);
        if (pl != 0 && pl->isReal()) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::PlayerContext ctx(i, g, r);
            addValue(*hv, ctx, "BASES", "BASES");
            addValue(*hv, ctx, "PLANETS", "PLANETS");
            addValue(*hv, ctx, "RACE", "RACE");
            addValue(*hv, ctx, "RACE$", "RACE$");
            addValue(*hv, ctx, "RACE.ADJ", "RACE.ADJ");
            addValue(*hv, ctx, "RACE.ID", "RACE.ID");
            addValue(*hv, ctx, "RACE.MISSION", "RACE.MISSION");
            addValue(*hv, ctx, "RACE.SHORT", "RACE.SHORT");
            addValue(*hv, ctx, "SCORE", "SCORE");
            addValue(*hv, ctx, "SHIPS", "SHIPS");
            addValue(*hv, ctx, "SHIPS.CAPITAL", "SHIPS.CAPITAL");
            addValue(*hv, ctx, "SHIPS.FREIGHTERS", "SHIPS.FREIGHTERS");
            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }

    return new afl::data::VectorValue(vv);
}

String_t
server::play::PlayerPacker::getName() const
{
    return "player";
}
