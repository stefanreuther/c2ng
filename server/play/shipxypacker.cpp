/**
  *  \file server/play/shipxypacker.cpp
  *  \brief Class server::play::ShipXYPacker
  */

#include "server/play/shipxypacker.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/turn.hpp"

using afl::base::Ref;
using afl::data::BooleanValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

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

    Ref<Vector> vv = Vector::create();
    game::map::AnyShipType& ty = t.universe().allShips();
    for (int i = 0, n = t.universe().ships().size(); i <= n; ++i) {
        if (ty.getObjectByIndex(i) != 0) {
            Ref<Hash> hv = Hash::create();
            game::interface::ShipContext ctx(i, m_session, r, g, t, sl);
            addValue(*hv, ctx, "LOC.X", "X");
            addValue(*hv, ctx, "LOC.Y", "Y");
            addValue(*hv, ctx, "MASS", "MASS");
            addValue(*hv, ctx, "NAME", "NAME");
            addValue(*hv, ctx, "OWNER$", "OWNER");
            addValue(*hv, ctx, "PLAYED", "PLAYED");
            if (game::map::Ship* sh = ty.getObjectByIndex(i)) {
                if (!sh->isReliablyVisible(0)) {
                    hv->setNew("GUESSED", new BooleanValue(true));
                }
            }
            vv->pushBackNew(new HashValue(hv));
        } else {
            vv->pushBackNew(0);
        }
    }
    return new VectorValue(vv);
}

String_t
server::play::ShipXYPacker::getName() const
{
    return "shipxy";
}
