/**
  *  \file server/play/minefieldpacker.cpp
  */

#include "server/play/minefieldpacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/map/minefieldtype.hpp"
#include "game/turn.hpp"

server::play::MinefieldPacker::MinefieldPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::MinefieldPacker::buildValue() const
{
    // ex ServerMinefieldWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Root& r = game::actions::mustHaveRoot(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    game::map::MinefieldType& ty = g.currentTurn().universe().minefields();
    for (game::Id_t i = ty.findNextIndex(0); i != 0; i = ty.findNextIndex(i)) {
        afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
        game::interface::MinefieldContext ctx(i, r, g);
        addValue(*hv, ctx, "ID", "ID");
        addValue(*hv, ctx, "LASTSCAN", "LASTSCAN");
        addValue(*hv, ctx, "LOC.X", "X");
        addValue(*hv, ctx, "LOC.Y", "Y");
        addValue(*hv, ctx, "OWNER$", "OWNER");
        addValue(*hv, ctx, "RADIUS", "RADIUS");
        addValue(*hv, ctx, "SCANNED", "SCANNED");
        addValue(*hv, ctx, "TYPE$", "TYPE");
        addValue(*hv, ctx, "UNITS", "UNITS");
        vv->pushBackNew(new afl::data::HashValue(hv));
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::MinefieldPacker::getName() const
{
    return "zmine";
}

