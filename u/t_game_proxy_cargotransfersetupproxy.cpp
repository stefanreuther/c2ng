/**
  *  \file u/t_game_proxy_cargotransfersetupproxy.cpp
  *  \brief Test for game::proxy::CargoTransferSetupProxy
  */

#include "game/proxy/cargotransfersetupproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
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
    using game::Turn;
    using game::actions::CargoTransferSetup;
    using game::map::Planet;
    using game::map::Ship;
    using game::proxy::CargoTransferSetupProxy;
    using game::spec::ShipList;
    using game::test::SessionThread;
    using game::test::WaitIndicator;

    const int OWNER = 4;
    const int HULL_NR = 12;
    const int LOC_X = 1234;
    const int LOC_Y = 2345;

    void addShipList(SessionThread& h)
    {
        Ptr<ShipList> shipList = new ShipList();

        // A hull
        game::spec::Hull* pHull = shipList->hulls().create(HULL_NR);
        pHull->setMass(1);
        pHull->setMaxCargo(100);
        pHull->setMaxFuel(100);

        // A launcher (just to exercise Element::end())
        shipList->launchers().create(3);

        h.session().setShipList(shipList);
    }

    void addRoot(SessionThread& h)
    {
        Ptr<Root> root = new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(3,2,0)));
        root->hostConfiguration()[game::config::HostConfiguration::AllowBeamUpMultiple].set(1);
        h.session().setRoot(root);
    }

    void addGame(SessionThread& h)
    {
        Ptr<Game> g = new Game();
        h.session().setGame(g);
    }

    Planet& addPlanet(SessionThread& h, int id)
    {
        Ptr<Game> g = h.session().getGame();
        TS_ASSERT(g.get() != 0);

        Planet& p = *g->currentTurn().universe().planets().create(id);
        p.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(OWNER));
        p.setOwner(OWNER);
        p.setPosition(game::map::Point(LOC_X, LOC_Y));
        p.setCargo(Element::Money, 1000);
        p.setCargo(Element::Neutronium, 500);
        p.setCargo(Element::Tritanium, 2000);
        p.setCargo(Element::Duranium, 3000);
        p.setCargo(Element::Molybdenum, 4000);
        p.setCargo(Element::Colonists, 100);
        p.setCargo(Element::Supplies, 500);
        p.setPlayability(Planet::Playable);
        p.setName("Melmac");

        p.internalCheck(game::map::Configuration(), h.session().translator(), h.session().log());

        return p;
    }

    Ship& addShip(SessionThread& h, int id)
    {
        Ptr<Game> g = h.session().getGame();
        TS_ASSERT(g.get() != 0);

        Ship& sh = *g->currentTurn().universe().ships().create(id);
        game::map::ShipData data;
        data.owner                     = OWNER;
        data.x                         = LOC_X;
        data.y                         = LOC_Y;
        data.engineType                = 1;
        data.hullType                  = HULL_NR;
        data.beamType                  = 0;
        data.launcherType              = 0;
        data.mission                   = 0;
        data.missionTowParameter       = 0;
        data.missionInterceptParameter = 0;
        data.warpFactor                = 3;
        data.neutronium                = 10;
        data.tritanium                 = 2;
        data.duranium                  = 4;
        data.molybdenum                = 6;
        data.money                     = 20;
        data.supplies                  = 8;
        data.name                      = "Titanic";

        sh.addCurrentShipData(data, game::PlayerSet_t(OWNER));
        sh.internalCheck();
        sh.setPlayability(game::map::Object::Playable);

        return sh;
    }

    void prepare(SessionThread& h)
    {
        addRoot(h);
        addShipList(h);
        addGame(h);
    }
}

void
TestGameProxyCargoTransferSetupProxy::testIt()
{
    // Preconditions
    SessionThread h;
    prepare(h);
    /*Ship& s1 =*/ addShip(h, 1);
    /*Ship& s2 =*/ addShip(h, 2);
    Ship& s3 = addShip(h, 3);
    s3.setPosition(game::map::Point(LOC_X, LOC_Y + 1));
    /*Planet& p55 =*/ addPlanet(h, 55);

    WaitIndicator ind;
    CargoTransferSetupProxy testee(h.gameSender());

    // Planet/Ship
    // - good case
    TS_ASSERT_EQUALS(testee.createPlanetShip(ind, 55, 1).getStatus(), CargoTransferSetup::Ready);

    // - bad case (wrong position)
    TS_ASSERT_EQUALS(testee.createPlanetShip(ind, 55, 3).getStatus(), CargoTransferSetup::Impossible);

    // - bad case (wrong Id)
    TS_ASSERT_EQUALS(testee.createPlanetShip(ind, 55, 99).getStatus(), CargoTransferSetup::Impossible);

    // Ship/Ship
    // - good case
    TS_ASSERT_EQUALS(testee.createShipShip(ind, 1, 2).getStatus(), CargoTransferSetup::Ready);

    // - bad case (wrong position)
    TS_ASSERT_EQUALS(testee.createShipShip(ind, 1, 3).getStatus(), CargoTransferSetup::Impossible);

    // - bad case (wrong Id)
    TS_ASSERT_EQUALS(testee.createShipShip(ind, 1, 99).getStatus(), CargoTransferSetup::Impossible);

    // Jettison
    // - good case
    TS_ASSERT_EQUALS(testee.createShipJettison(ind, 3).getStatus(), CargoTransferSetup::Ready);

    // - bad case (at planet)
    TS_ASSERT_EQUALS(testee.createShipJettison(ind, 1).getStatus(), CargoTransferSetup::Impossible);

    // - bad case (wrong Id)
    TS_ASSERT_EQUALS(testee.createShipJettison(ind, 99).getStatus(), CargoTransferSetup::Impossible);

    // Beam-up-multiple
    // - good case
    TS_ASSERT_EQUALS(testee.createShipBeamUp(ind, 1).getStatus(), CargoTransferSetup::Ready);

    // - bad case (wrong position)
    TS_ASSERT_EQUALS(testee.createShipBeamUp(ind, 3).getStatus(), CargoTransferSetup::Impossible);

    // - bad case (wrong Id)
    TS_ASSERT_EQUALS(testee.createShipBeamUp(ind, 99).getStatus(), CargoTransferSetup::Impossible);
}
