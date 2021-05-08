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
        pHull->setName("BRUCE");

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
        p.setFriendlyCode(String_t("alf"));
        p.internalCheck(g->currentTurn().universe().config(), h.session().translator(), h.session().log());
        TS_ASSERT(p.isVisible());
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
        data.damage                    = 12;
        data.friendlyCode              = "joe";

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
    TS_ASSERT(!part.isTemporary);
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
    TS_ASSERT(!part.isTemporary);

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

/** Test overload behaviour.
    A: create universe with two units. Initialize with correct setup. Move exercising overload.
    E: status must be reported correctly. Commit must correctly update participants. */
void
TestGameProxyCargoTransferProxy::testOverload()
{
    const int SHIP_ID = 78;
    const int PLANET_ID = 150;

    // Preconditions
    SessionThread h;
    prepare(h);
    Ship& sh = addShip(h, SHIP_ID);
    /*Planet& pl =*/ addPlanet(h, PLANET_ID);
    CargoTransferSetup setup = CargoTransferSetup::fromPlanetShip(h.session().getGame()->currentTurn().universe(), PLANET_ID, SHIP_ID);
    TS_ASSERT(setup.isValid());

    // Testee
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    testee.init(setup);

    // Move some cargo: 2000 will only fit with overload,
    // and only then we'll be able to unload 1950.
    testee.setOverload(true);
    testee.move(Element::Tritanium, 2000, 0, 1, false);
    testee.move(Element::Tritanium, 1950, 1, 0, false);
    testee.commit();
    h.sync();

    // Verify postconditions: ship had 2, now should have 52
    TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(-1),  52);
}

/** Test multi-ship transfer.
    A: create universe with multiple units. Initialize with multi-ship setup.
    E: status reported correctly. */
void
TestGameProxyCargoTransferProxy::testMulti()
{
    // Preconditions
    SessionThread h;
    prepare(h);
    addShip(h, 1);
    addShip(h, 2);
    addShip(h, 3);
    addPlanet(h, 77);

    // Testee
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    game::actions::MultiTransferSetup setup;
    setup.setShipId(2);
    setup.setElementType(Element::Duranium);
    testee.init(ind, setup);

    // Verify setup
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    TS_ASSERT(gen.validTypes.contains(Element::Duranium));
    TS_ASSERT_EQUALS(gen.numParticipants, 5U);

    // Verify participants
    CargoTransferProxy::Participant part1;
    testee.getParticipantInformation(ind, 0, part1);
    TS_ASSERT_EQUALS(part1.name, "Hold space");
    TS_ASSERT_EQUALS(part1.info1, "");
    TS_ASSERT_EQUALS(part1.info2, "");
    TS_ASSERT(part1.isTemporary);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    TS_ASSERT_EQUALS(part2.name, "Titanic");
    TS_ASSERT_EQUALS(part2.info1, "BRUCE");
    TS_ASSERT_EQUALS(part2.info2, "FCode: \"joe\", Damage: 12%");
    TS_ASSERT(!part2.isTemporary);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    TS_ASSERT_EQUALS(part5.name, "Melmac");
    TS_ASSERT_EQUALS(part5.info1, "Planet");
    TS_ASSERT_EQUALS(part5.info2, "FCode: \"alf\"");
    TS_ASSERT(!part5.isTemporary);
}

/** Test multi-ship transfer, moveExt.
    A: create universe with multiple units. Initialize with multi-ship setup; use moveExt.
    E: status reported correctly. */
void
TestGameProxyCargoTransferProxy::testMultiMoveExt()
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(h, 1);              // has 20$
    addShip(h, 2);              // has 20$
    addShip(h, 3);              // has 20$
    addPlanet(h, 77);           // has 1000$

    // Testee (same as testMulti)
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    game::actions::MultiTransferSetup setup;
    setup.setShipId(2);
    setup.setElementType(Element::Money);
    testee.init(ind, setup);

    // Move from hold (#0) to #1 (first ship), extension 4 (planet)
    // Hold is empty, so this will consume from 4.
    testee.moveExt(Element::Money, 100, 0, 1, 4, false);

    // Verify participants
    CargoTransferProxy::Participant part1;
    testee.getParticipantInformation(ind, 0, part1);
    TS_ASSERT_EQUALS(part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    TS_ASSERT_EQUALS(part2.cargo.amount.get(Element::Money), 120);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    TS_ASSERT_EQUALS(part5.cargo.amount.get(Element::Money), 900);
}

/** Test multi-ship transfer, moveAll.
    A: create universe with multiple units. Initialize with multi-ship setup; use moveAll.
    E: status reported correctly. */
void
TestGameProxyCargoTransferProxy::testMultiMoveAll()
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(h, 1);              // has 20$
    addShip(h, 2);              // has 20$
    addShip(h, 3);              // has 20$
    addPlanet(h, 77);           // has 1000$

    // Testee (same as testMulti)
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    game::actions::MultiTransferSetup setup;
    setup.setShipId(2);
    setup.setElementType(Element::Money);
    testee.init(ind, setup);

    // Move to #2 (second ship), except 3 (third ship).
    testee.moveAll(Element::Money, 2, 3, false);

    // Verify participants
    CargoTransferProxy::Participant part1;
    testee.getParticipantInformation(ind, 0, part1);
    TS_ASSERT_EQUALS(part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    TS_ASSERT_EQUALS(part2.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part3;
    testee.getParticipantInformation(ind, 2, part3);
    TS_ASSERT_EQUALS(part3.cargo.amount.get(Element::Money), 1040);

    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    TS_ASSERT_EQUALS(part4.cargo.amount.get(Element::Money), 20);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    TS_ASSERT_EQUALS(part5.cargo.amount.get(Element::Money), 0);
}

/** Test multi-ship transfer, distribute.
    A: create universe with multiple units. Initialize with multi-ship setup; use distribute.
    E: status reported correctly. */
void
TestGameProxyCargoTransferProxy::testDistribute()
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(h, 1);              // has 20$
    addShip(h, 2);              // has 20$
    addShip(h, 3);              // has 20$
    addPlanet(h, 77);           // has 1000$

    // Testee (same as testMulti)
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    game::actions::MultiTransferSetup setup;
    setup.setShipId(2);
    setup.setElementType(Element::Money);
    testee.init(ind, setup);

    // Distribute from #2 (second ship), except 4 (planet).
    // This moves 10$ to #1 and #3.
    testee.distribute(Element::Money, 2, 4, game::actions::CargoTransfer::DistributeEqually);

    // Verify participants
    CargoTransferProxy::Participant part1;
    testee.getParticipantInformation(ind, 0, part1);
    TS_ASSERT_EQUALS(part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    TS_ASSERT_EQUALS(part2.cargo.amount.get(Element::Money), 30);

    CargoTransferProxy::Participant part3;
    testee.getParticipantInformation(ind, 2, part3);
    TS_ASSERT_EQUALS(part3.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    TS_ASSERT_EQUALS(part4.cargo.amount.get(Element::Money), 30);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    TS_ASSERT_EQUALS(part5.cargo.amount.get(Element::Money), 1000);
}

/** Test multi-ship transfer, addHoldSpace.
    A: set up a cargo transfer. Use addHoldSpace().
    E: status reported correctly. */
void
TestGameProxyCargoTransferProxy::testAddHoldSpace()
{
    // Preconditions (similar to testMulti)
    SessionThread h;
    prepare(h);
    addShip(h, 1);              // has 20$
    addShip(h, 2);              // has 20$

    // Testee (same as testMulti)
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    game::actions::MultiTransferSetup setup;
    setup.setShipId(2);
    setup.setElementType(Element::Money);
    testee.init(ind, setup);

    // Verify: 3 participants
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    TS_ASSERT_EQUALS(gen.numParticipants, 3U);

    // Add a new hold space
    testee.addHoldSpace("Bag");

    // Verify: now 4 participants
    testee.getGeneralInformation(ind, gen);
    TS_ASSERT_EQUALS(gen.numParticipants, 4U);

    // Verify participants
    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    TS_ASSERT_EQUALS(part4.name, "Bag");
    TS_ASSERT(part4.isTemporary);
}

