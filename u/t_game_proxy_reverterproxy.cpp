/**
  *  \file u/t_game_proxy_reverterproxy.cpp
  *  \brief Test for game::proxy::ReverterProxy
  */

#include "game/proxy/reverterproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/v3/reverter.hpp"

using afl::base::Ptr;
using game::Game;
using game::map::LocationReverter;
using game::map::Planet;
using game::proxy::ReverterProxy;
using game::test::SessionThread;
using game::test::WaitIndicator;

namespace {
    const int PLANET_ID = 333;

    Planet& prepare(SessionThread& h)
    {
        // Create a game with a planet
        Ptr<Game> g = new Game();
        Planet& p = *g->currentTurn().universe().planets().create(PLANET_ID);

        game::map::PlanetData pd;
        pd.owner = 3;
        pd.friendlyCode = "abc";
        p.setPosition(game::map::Point(1000, 1000));
        p.setName("Jupiter");
        p.addCurrentPlanetData(pd, game::PlayerSet_t(3));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), h.session().translator(), h.session().log());
        p.setPlayability(Planet::Playable);

        // Attach a classic reverter
        std::auto_ptr<game::v3::Reverter> rev(new game::v3::Reverter(g->currentTurn(), h.session()));
        pd.friendlyCode = "xyz";
        rev->addPlanetData(PLANET_ID, pd);
        g->currentTurn().universe().setNewReverter(rev.release());

        // Finish
        h.session().setGame(g);

        return p;
    }
}


/** Test empty universe.
    A: create empty session. Create proxy.
    E: must report nothing to undo. */
void
TestGameProxyReverterProxy::testEmpty()
{
    SessionThread h;
    WaitIndicator ind;

    ReverterProxy testee(h.gameSender());
    ReverterProxy::Status st;
    testee.init(ind, game::map::Point(1000, 1000), st);

    TS_ASSERT(st.modes.empty());
    TS_ASSERT(st.list.empty());
}

/** Test nonempty universe.
    A: create session with an object in it. Create proxy. Call commit().
    E: must report object to undo. Must correctly process undo. */
void
TestGameProxyReverterProxy::testNormal()
{
    SessionThread h;
    WaitIndicator ind;
    Planet& p = prepare(h);

    ReverterProxy testee(h.gameSender());

    // Initialize
    ReverterProxy::Status st;
    testee.init(ind, game::map::Point(1000, 1000), st);

    TS_ASSERT(st.modes.contains(LocationReverter::Cargo));
    TS_ASSERT(st.modes.contains(LocationReverter::Missions));
    TS_ASSERT_EQUALS(st.list.size(), 1U);
    TS_ASSERT_EQUALS(st.list.get(0)->type, game::ref::UserList::ReferenceItem);
    TS_ASSERT_DIFFERS(st.list.get(0)->name.find("Jupiter"), String_t::npos);
    TS_ASSERT_EQUALS(st.list.get(0)->reference, game::Reference(game::Reference::Planet, PLANET_ID));

    // Commit
    testee.commit(st.modes);
    h.sync();

    TS_ASSERT_EQUALS(p.getFriendlyCode().orElse(""), "xyz");
}

