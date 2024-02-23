/**
  *  \file test/game/proxy/buildstarbaseproxytest.cpp
  *  \brief Test for game::proxy::BuildStarbaseProxy
  */

#include "game/proxy/buildstarbaseproxy.hpp"

#include "afl/test/testrunner.hpp"
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
        Ptr<Root> r = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))).asPtr();
        s.session().setRoot(r);

        Ptr<Game> g = new Game();
        s.session().setGame(g);
    }

    Planet& addPlanet(SessionThread& s)
    {
        Ptr<Game> g = s.session().getGame();
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
AFL_TEST("game.proxy.BuildStarbaseProxy:empty", a)
{
    SessionThread h;
    BuildStarbaseProxy testee(h.gameSender());

    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, 99, st);
    a.checkEqual("01. mode", st.mode, BuildStarbaseProxy::Error);
    a.check("02. errorMessage", !st.errorMessage.empty());
}

/** Test normal behaviour.
    A: create session containing a planet. Call init().
    E: result reports CanBuild. */
AFL_TEST("game.proxy.BuildStarbaseProxy:normal", a)
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
    a.checkEqual("01. mode",      st.mode, BuildStarbaseProxy::CanBuild);
    a.checkEqual("02. available", st.available.toCargoSpecString(), "2000T 3000D 4000M 1000$");
    a.checkEqual("03. cost",      st.cost.toCargoSpecString(),      "402T 120D 340M 900$");
    a.checkEqual("04. remaining", st.remaining.toCargoSpecString(), "1598T 2880D 3660M 100$");
    a.checkEqual("05. missing",   st.missing.isZero(), true);

    // Commit
    testee.commit(ind);

    // Verify
    a.checkEqual("11. Tritanium", p.getCargo(Element::Tritanium).orElse(1000), 1598);
    a.checkEqual("12. isBuildingBase", p.isBuildingBase(), true);
}

/** Test lifetime behaviour.
    A: create session containing a planet. Call init(). Destroy session content. Call commit.
    E: Call must succeed (not segfault). */
AFL_TEST_NOARG("game.proxy.BuildStarbaseProxy:lifetime")
{
    SessionThread h;
    prepare(h);
    Planet& p = addPlanet(h);

    BuildStarbaseProxy testee(h.gameSender());

    // Prepare
    WaitIndicator ind;
    BuildStarbaseProxy::Status st;
    testee.init(ind, PLANET_ID, st);

    // Clear session
    h.session().setGame(0);
    h.session().setRoot(0);

    // Commit
    testee.commit(ind);
}

/** Test cancellation behaviour.
    A: create session containing a planet that is building a starbase. Call init().
    E: result reports CanCancel. */
AFL_TEST("game.proxy.BuildStarbaseProxy:cancel", a)
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
    a.checkEqual("01. mode", st.mode, BuildStarbaseProxy::CanCancel);
}

/** Test missing resources behaviour.
    A: create session containing a planet with too little resources. Call init().
    E: result reports CannotBuild. */
AFL_TEST("game.proxy.BuildStarbaseProxy:missing-resources", a)
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
    a.checkEqual("01. mode",      st.mode, BuildStarbaseProxy::CannotBuild);
    a.checkEqual("02. available", st.available.toCargoSpecString(), "100T 3000D 4000M 1000$");
    a.checkEqual("03. cost",      st.cost.toCargoSpecString(),      "402T 120D 340M 900$");
    a.checkEqual("04. remaining", st.remaining.toCargoSpecString(), "-302T 2880D 3660M 100$");
    a.checkEqual("05. missing",   st.missing.toCargoSpecString(),   "302T");
}
