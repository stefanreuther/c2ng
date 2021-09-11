/**
  *  \file server/play/shipxypacker.cpp
  */

#include "server/play/shipxypacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/turn.hpp"

server::play::ShipXYPacker::ShipXYPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::ShipXYPacker::buildValue() const
{
    // ex ServerShipxyWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Turn& t = g.currentTurn();
    game::Root& r = game::actions::mustHaveRoot(m_session);
    game::spec::ShipList& sl = game::actions::mustHaveShipList(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    game::map::AnyShipType ty(t.universe().ships());
    for (int i = 0, n = t.universe().ships().size(); i <= n; ++i) {
        if (ty.getObjectByIndex(i) != 0) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::ShipContext ctx(i, m_session, r, g, sl);
            addValue(*hv, ctx, "LOC.X", "X");
            addValue(*hv, ctx, "LOC.Y", "Y");
            addValue(*hv, ctx, "MASS", "MASS");
            addValue(*hv, ctx, "NAME", "NAME");
            addValue(*hv, ctx, "OWNER$", "OWNER");
            addValue(*hv, ctx, "PLAYED", "PLAYED");
            vv->pushBackNew(new afl::data::HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }
    return new afl::data::VectorValue(vv);
}

String_t
server::play::ShipXYPacker::getName() const
{
    return "shipxy";
}
