/**
  *  \file test/game/proxy/reverterproxytest.cpp
  *  \brief Test for game::proxy::ReverterProxy
  */

#include "game/proxy/reverterproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
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
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(3), 15, h.session().translator(), h.session().log());
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
AFL_TEST("game.proxy.ReverterProxy:empty", a)
{
    SessionThread h;
    WaitIndicator ind;

    ReverterProxy testee(h.gameSender());
    ReverterProxy::Status st;
    testee.init(ind, game::map::Point(1000, 1000), st);

    a.check("01. modes", st.modes.empty());
    a.check("02. list", st.list.empty());
}

/** Test nonempty universe.
    A: create session with an object in it. Create proxy. Call commit().
    E: must report object to undo. Must correctly process undo. */
AFL_TEST("game.proxy.ReverterProxy:normal", a)
{
    SessionThread h;
    WaitIndicator ind;
    Planet& p = prepare(h);

    ReverterProxy testee(h.gameSender());

    // Initialize
    ReverterProxy::Status st;
    testee.init(ind, game::map::Point(1000, 1000), st);

    a.check("01. Cargo", st.modes.contains(LocationReverter::Cargo));
    a.check("02 . Missions", st.modes.contains(LocationReverter::Missions));
    a.checkEqual("03. size", st.list.size(), 1U);
    a.checkEqual("04. type", st.list.get(0)->type, game::ref::UserList::ReferenceItem);
    a.checkDifferent("05. name", st.list.get(0)->name.find("Jupiter"), String_t::npos);
    a.checkEqual("06. reference", st.list.get(0)->reference, game::Reference(game::Reference::Planet, PLANET_ID));

    // Commit
    testee.commit(st.modes);
    h.sync();

    a.checkEqual("11. getFriendlyCode", p.getFriendlyCode().orElse(""), "xyz");
}
