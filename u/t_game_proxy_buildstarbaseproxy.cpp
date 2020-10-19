/**
  *  \file u/t_game_proxy_buildstarbaseproxy.cpp
  *  \brief Test for game::proxy::BuildStarbaseProxy
  */

#include "game/proxy/buildstarbaseproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/element.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    using afl::base::Ptr;
    using game::Element;
    using game::Game;
    using game::HostVersion;
    using game::Root;
    using game::map::Planet;
    using game::proxy::BuildStarbaseProxy;
    using game::test::SessionThread;
    using game::test::WaitIndicator;

    const int OWNER = 8;
    const int PLANET_ID = 77;

    void prepare(SessionThread& s)
    {
        Ptr<Root> r = new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)));
        s.session().setRoot(r);

        Ptr<Game> g = new Game();
        s.session().setGame(g);
    }

    Planet& addPlanet(SessionThread& s)
    {
        Ptr<Game> g = s.session().getGame();
        TS_ASSERT(g.get() != 0);

        Planet& p = *g->currentTurn().universe().planets().create(PLANET_ID);
        p.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(OWNER));
        p.setOwner(OWNER);
        p.setPosition(game::map::Point(1122, 3344));
        p.setCargo(Element::Money, 1000);
        p.setCargo(Element::Tritanium, 2000);
        p.setCargo(Element::Duranium, 3000);
        p.setCargo(Element::Molybdenum, 4000);
        p.setPlayability(Planet::Playable);
        return p;
    }
}

/** Test behaviour on empty session.
    A: create empty session. Call init().
    E: result reports Error with a nonempty message. */
void
TestGameProxyBuildStarbaseProxy::testEmpty()
{
    SessionThread h;
    BuildStarbaseProxy testee(h.gameSender());

    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, 99, st);
    TS_ASSERT_EQUALS(st.mode, BuildStarbaseProxy::Error);
    TS_ASSERT(!st.errorMessage.empty());
}

/** Test normal behaviour.
    A: create session containing a planet. Call init().
    E: result reports CanBuild. */
void
TestGameProxyBuildStarbaseProxy::testNormal()
{
    SessionThread h;
    prepare(h);
    Planet& p = addPlanet(h);

    BuildStarbaseProxy testee(h.gameSender());

    // Prepare
    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, PLANET_ID, st);

    // Verify
    TS_ASSERT_EQUALS(st.mode, BuildStarbaseProxy::CanBuild);
    TS_ASSERT_EQUALS(st.available.toCargoSpecString(), "2000T 3000D 4000M 1000$");
    TS_ASSERT_EQUALS(st.cost.toCargoSpecString(),      "402T 120D 340M 900$");
    TS_ASSERT_EQUALS(st.remaining.toCargoSpecString(), "1598T 2880D 3660M 100$");
    TS_ASSERT_EQUALS(st.missing.isZero(), true);

    // Commit
    testee.commit(ind);

    // Verify
    TS_ASSERT_EQUALS(p.getCargo(Element::Tritanium).orElse(1000), 1598);
    TS_ASSERT_EQUALS(p.isBuildingBase(), true);
}

/** Test normal behaviour.
    A: create session containing a planet that is building a starbase. Call init().
    E: result reports CanCancel. */
void
TestGameProxyBuildStarbaseProxy::testCancel()
{
    SessionThread h;
    prepare(h);
    Planet& p = addPlanet(h);
    p.setBuildBaseFlag(true);

    BuildStarbaseProxy testee(h.gameSender());

    // Prepare
    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, PLANET_ID, st);

    // Verify
    // Note: as of 20200814, costs not filled in in this situation!
    TS_ASSERT_EQUALS(st.mode, BuildStarbaseProxy::CanCancel);
}

/** Test missing resources behaviour.
    A: create session containing a planet with too little resources. Call init().
    E: result reports CannotBuild. */
void
TestGameProxyBuildStarbaseProxy::testMissing()
{
    SessionThread h;
    prepare(h);
    Planet& p = addPlanet(h);
    p.setCargo(Element::Tritanium, 100);

    BuildStarbaseProxy testee(h.gameSender());

    // Prepare
    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, PLANET_ID, st);

    // Verify
    TS_ASSERT_EQUALS(st.mode, BuildStarbaseProxy::CannotBuild);
    TS_ASSERT_EQUALS(st.available.toCargoSpecString(), "100T 3000D 4000M 1000$");
    TS_ASSERT_EQUALS(st.cost.toCargoSpecString(),      "402T 120D 340M 900$");
    TS_ASSERT_EQUALS(st.remaining.toCargoSpecString(), "-302T 2880D 3660M 100$");
    TS_ASSERT_EQUALS(st.missing.toCargoSpecString(),   "302T");
}

