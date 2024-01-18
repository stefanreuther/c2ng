/**
  *  \file test/game/proxy/buildstructuresproxytest.cpp
  *  \brief Test for game::proxy::BuildStructuresProxy
  */

#include "game/proxy/buildstructuresproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "util/simplerequestdispatcher.hpp"

namespace {
    using afl::base::Ptr;
    using game::Element;
    using game::Game;
    using game::HostVersion;
    using game::Root;
    using game::map::Planet;
    using game::proxy::BuildStructuresProxy;
    using game::test::SessionThread;
    using game::test::WaitIndicator;
    using util::SimpleRequestDispatcher;

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
        p.setCargo(Element::Colonists, 100);
        p.setCargo(Element::Supplies, 500);
        p.setNumBuildings(game::MineBuilding, 10);
        p.setNumBuildings(game::FactoryBuilding, 20);
        p.setNumBuildings(game::DefenseBuilding, 15);
        p.setPlayability(Planet::Playable);
        p.setName("Melmac");
        p.setTemperature(33);
        return p;
    }

    struct UpdateReceiver {
        BuildStructuresProxy::Status result;
        bool got;

        void onStatusChange(const BuildStructuresProxy::Status& st)
            { result = st; got = true; }

        UpdateReceiver()
            : result(), got(false)
            { }
    };
}

/** Test behaviour on empty session.
    A: create empty session. Call init().
    E: result reports not ok. */
AFL_TEST("game.proxy.BuildStructuresProxy:empty", a)
{
    SessionThread h;
    WaitIndicator ind;
    BuildStructuresProxy testee(h.gameSender(), ind);

    BuildStructuresProxy::HeaderInfo head;
    testee.init(ind, 99, head);

    a.checkEqual("01. ok", head.ok, false);
    a.checkEqual("02. hasBase", head.hasBase, false);
}

/** Test normal behaviour.
    A: create session containing a planet. Call init(), update().
    E: result reports success, correct status. */
AFL_TEST("game.proxy.BuildStructuresProxy:normal", a)
{
    SessionThread h;
    prepare(h);
    addPlanet(h);

    SimpleRequestDispatcher disp;
    WaitIndicator ind;
    BuildStructuresProxy testee(h.gameSender(), disp);

    // Initialize
    BuildStructuresProxy::HeaderInfo head;
    testee.init(ind, PLANET_ID, head);

    a.checkEqual("01. ok",             head.ok, true);
    a.checkEqual("02. hasBase",        head.hasBase, false);
    a.checkEqual("03. planetName",     head.planetName, "Melmac");
    a.checkDifferent("04. planetInfo", head.planetInfo.find("33"), String_t::npos);

    // Request status and receive it
    UpdateReceiver recv;
    testee.sig_statusChange.add(&recv, &UpdateReceiver::onStatusChange);
    testee.update();
    while (!recv.got) {
        a.check("11. wait", disp.wait(100));
    }

    // Verify status
    a.checkEqual("21. MineBuilding",    recv.result.buildings[game::MineBuilding].have, 10);
    a.checkEqual("22. FactoryBuilding", recv.result.buildings[game::FactoryBuilding].have, 20);
    a.checkEqual("23. DefenseBuilding", recv.result.buildings[game::DefenseBuilding].have, 15);
    a.checkEqual("24. available",       recv.result.available.toCargoSpecString(), "2000T 3000D 4000M 500S 1000$");
}

/** Test building.
    A: create session containing a planet. Call init(), addLimitCash().
    E: reports correct status. */
AFL_TEST("game.proxy.BuildStructuresProxy:addLimitCash", a)
{
    SessionThread h;
    prepare(h);
    addPlanet(h);

    SimpleRequestDispatcher disp;
    WaitIndicator ind;
    BuildStructuresProxy testee(h.gameSender(), disp);

    // Initialize
    BuildStructuresProxy::HeaderInfo head;
    testee.init(ind, PLANET_ID, head);

    // Add buildings and receive status
    UpdateReceiver recv;
    testee.sig_statusChange.add(&recv, &UpdateReceiver::onStatusChange);
    testee.addLimitCash(game::MineBuilding, 12);
    while (!recv.got) {
        a.check("01. wait", disp.wait(100));
    }

    // Verify status
    a.checkEqual("11. MineBuilding",    recv.result.buildings[game::MineBuilding].have, 22);
    a.checkEqual("12. FactoryBuilding", recv.result.buildings[game::FactoryBuilding].have, 20);
    a.checkEqual("13. DefenseBuilding", recv.result.buildings[game::DefenseBuilding].have, 15);
    a.checkEqual("14. available",       recv.result.available.toCargoSpecString(), "2000T 3000D 4000M 500S 1000$");
    a.checkEqual("15. needed",          recv.result.needed.toCargoSpecString(),    "12S 48$");
    a.checkEqual("16. remaining",       recv.result.remaining.toCargoSpecString(), "2000T 3000D 4000M 488S 952$");
}

/** Test auto-build and commit.
    A: create session containing a planet. Call applyAutobuildSettings(), doStandardAutoBuild(), commit().
    E: planet has correct status afterwards. */
AFL_TEST("game.proxy.BuildStructuresProxy:doStandardAutoBuild", a)
{
    SessionThread h;
    prepare(h);
    Planet& p = addPlanet(h);

    WaitIndicator ind;
    BuildStructuresProxy testee(h.gameSender(), ind);

    // Initialize
    BuildStructuresProxy::HeaderInfo head;
    testee.init(ind, PLANET_ID, head);

    // Update autobuild settings
    Planet::AutobuildSettings settings;
    settings.goal[game::MineBuilding] = 25;         // 15 to build
    settings.goal[game::FactoryBuilding] = 24;      //  4 to build
    settings.goal[game::DefenseBuilding] = 23;      //  8 to build
    testee.applyAutobuildSettings(settings);

    // Do autobuild and commit
    testee.doStandardAutoBuild();
    testee.commit();
    h.sync();

    // Verify content of planet
    a.checkEqual("01. MineBuilding",    p.getNumBuildings(game::MineBuilding).orElse(-1), 25);
    a.checkEqual("02. FactoryBuilding", p.getNumBuildings(game::FactoryBuilding).orElse(-1), 24);
    a.checkEqual("03. DefenseBuilding", p.getNumBuildings(game::DefenseBuilding).orElse(-1), 23);

    a.checkEqual("11. Supplies",        p.getCargo(Element::Supplies).orElse(-1), 500 -   (15+  4+   8));
    a.checkEqual("12. Money",           p.getCargo(Element::Money).orElse(-1),   1000 - (4*15+3*4+10*8));
}
