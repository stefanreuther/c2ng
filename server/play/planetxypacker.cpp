/**
  *  \file server/play/planetxypacker.cpp
  */

#include "server/play/planetxypacker.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/turn.hpp"

server::play::PlanetXYPacker::PlanetXYPacker(game::Session& session)
    : m_session(session)
{ }

server::Value_t*
server::play::PlanetXYPacker::buildValue() const
{
    // ex ServerPlanetxyWriter::write
    game::Game& g = game::actions::mustHaveGame(m_session);
    game::Turn& t = g.currentTurn();
    game::Root& r = game::actions::mustHaveRoot(m_session);

    afl::base::Ref<afl::data::Vector> vv(afl::data::Vector::create());
    game::map::AnyPlanetType& ty(t.universe().allPlanets());

    // Note: iteration starts at 0 so JSON can be indexed with planet Ids
    for (int i = 0, n = t.universe().planets().size(); i <= n; ++i) {
        if (ty.getObjectByIndex(i) != 0) {
            afl::base::Ref<afl::data::Hash> hv(afl::data::Hash::create());
            game::interface::PlanetContext ctx(i, m_session, r, g);
            addValue(*hv, ctx, "BASE.YESNO", "BASE");
            addValue(*hv, ctx, "LOC.X", "X");
            addValue(*hv, ctx, "LOC.Y", "Y");
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
server::play::PlanetXYPacker::getName() const
{
    return "planetxy";
}
