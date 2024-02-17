/**
  *  \file server/play/ionstormpacker.cpp
  */

#include "server/play/ionstormpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/ionstormcontext.hpp"
#include "game/map/ionstormtype.hpp"
#include "game/turn.hpp"

server::play::IonStormPacker::IonStormPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::IonStormPacker::buildValue() const
{
    // ex ServerStormWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    game::map::IonStormType& ty = g.currentTurn().universe().ionStormType();
    for (game::Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
        game::interface::IonStormContext ctx(i, m_session, g.currentTurn());
        addValue(*hv, ctx, "HEADING$", "HEADING");
        addValue(*hv, ctx, "ID", "ID");
        addValue(*hv, ctx, "LOC.X", "X");
        addValue(*hv, ctx, "LOC.Y", "Y");
        addValue(*hv, ctx, "NAME", "NAME");
        addValue(*hv, ctx, "RADIUS", "RADIUS");
        addValue(*hv, ctx, "SPEED$", "SPEED");
        addValue(*hv, ctx, "STATUS$", "STATUS");
        addValue(*hv, ctx, "VOLTAGE", "VOLTAGE");
        vv->pushBackNew(new afl::data::HashValue(hv));
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::IonStormPacker::getName() const
{
    return "zstorm";
}

