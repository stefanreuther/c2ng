/**
  *  \file u/t_game_proxy_cargotransferproxy.cpp
  *  \brief Test for game::proxy::CargoTransferProxy
  */

#include "game/proxy/cargotransferproxy.hpp"

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
    using game::proxy::CargoTransferProxy;
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

/** Test behaviour on empty universe/invalid setup.
    A: create empty universe. Initialize with invalid setup.
    E: status must be reported as empty (not uninitialized) */
void
TestGameProxyCargoTransferProxy::testEmpty()
{
    SessionThread h;
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);

    testee.init(CargoTransferSetup());

    // Check general
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    TS_ASSERT(gen.validTypes.empty());
    TS_ASSERT(!gen.allowUnload);
    TS_ASSERT(!gen.allowSupplySale);

    // Check participant
    CargoTransferProxy::Participant part;
    testee.getParticipantInformation(ind, 0, part);
    TS_ASSERT(part.name.empty());
    TS_ASSERT(!part.isUnloadTarget);
}

/** Test normal behaviour.
    A: create universe with two units. Initialize with correct setup. Move some cargo.
    E: status must be reported correctly. Commit must correctly update participants. */
void
TestGameProxyCargoTransferProxy::testNormal()
{
    const int SHIP_ID = 78;
    const int PLANET_ID = 150;

    // Preconditions
    SessionThread h;
    prepare(h);
    Ship& sh = addShip(h, SHIP_ID);
    Planet& pl = addPlanet(h, PLANET_ID);
    CargoTransferSetup setup = CargoTransferSetup::fromPlanetShip(h.session().getGame()->currentTurn().universe(), PLANET_ID, SHIP_ID);
    TS_ASSERT(setup.isValid());

    // Testee
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    testee.init(setup);

    // Check general
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    TS_ASSERT(gen.validTypes.contains(Element::Neutronium));
    TS_ASSERT(gen.validTypes.contains(Element::Money));
    TS_ASSERT(gen.allowUnload);
    TS_ASSERT(gen.allowSupplySale);

    // Check participant. Left is planet.
    CargoTransferProxy::Participant part;
    testee.getParticipantInformation(ind, 0, part);
    TS_ASSERT_EQUALS(part.name, "Melmac");
    TS_ASSERT(part.isUnloadTarget);

    // Move some cargo
    testee.unload(false);
    testee.move(Element::Tritanium, 20, 0, 1, false);
    testee.commit();
    h.sync();

    // Verify postconditions
    TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1),  20);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Duranium).orElse(-1),   0);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Molybdenum).orElse(-1), 0);

    TS_ASSERT_EQUALS(pl.getCargo(Element::Neutronium).orElse(-1), 500);
    TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(-1),  2000 + 2 - 20);
    TS_ASSERT_EQUALS(pl.getCargo(Element::Duranium).orElse(-1),   3000 + 4);
    TS_ASSERT_EQUALS(pl.getCargo(Element::Molybdenum).orElse(-1), 4000 + 6);
}
