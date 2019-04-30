/**
  *  \file server/play/ufopacker.cpp
  */

#include "server/play/ufopacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/ufocontext.hpp"
#include "game/map/ufotype.hpp"
#include "game/turn.hpp"

server::play::UfoPacker::UfoPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::UfoPacker::buildValue() const
{
    // ex ServerUfoWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    game::map::UfoType& ty = g.currentTurn().universe().ufos();
    for (game::Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
        game::interface::UfoContext ctx(i, g.currentTurn(), m_session);

        addValue(*hv, ctx, "COLOR.EGA", "COLOR");
        addValue(*hv, ctx, "HEADING$", "HEADING");
        addValue(*hv, ctx, "ID", "ID");
        addValue(*hv, ctx, "INFO1", "INFO1");
        addValue(*hv, ctx, "INFO2", "INFO2");
        addValue(*hv, ctx, "KEEP", "KEEP");
        addValue(*hv, ctx, "LASTSCAN", "LASTSCAN");
        addValue(*hv, ctx, "LOC.X", "X");
        addValue(*hv, ctx, "LOC.Y", "Y");
        addValue(*hv, ctx, "MOVE.DX", "MOVE.DX");
        addValue(*hv, ctx, "MOVE.DY", "MOVE.DY");
        addValue(*hv, ctx, "NAME", "NAME");
        addValue(*hv, ctx, "RADIUS", "RADIUS");
        addValue(*hv, ctx, "SPEED$", "SPEED");
        addValue(*hv, ctx, "TYPE", "TYPE");
        addValue(*hv, ctx, "VISIBLE.PLANET", "VISIBLE.PLANET");
        addValue(*hv, ctx, "VISIBLE.SHIP", "VISIBLE.SHIP");

        vv->pushBackNew(new afl::data::HashValue(hv));
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::UfoPacker::getName() const
{
    return "zufo";
}
