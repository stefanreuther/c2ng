/**
  *  \file test/game/proxy/cargotransferproxytest.cpp
  *  \brief Test for game::proxy::CargoTransferProxy
  */

#include "game/proxy/cargotransferproxy.hpp"

#include "afl/test/testrunner.hpp"
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
        Ptr<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,2,0))).asPtr();
        h.session().setRoot(root);
    }

    void addGame(SessionThread& h)
    {
        Ptr<Game> g = new Game();
        h.session().setGame(g);
    }

    Planet& addPlanet(afl::test::Assert a, SessionThread& h, int id)
    {
        Ptr<Game> g = h.session().getGame();
        a.checkNonNull("addPlanet: has game", g.get());

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
        p.internalCheck(g->mapConfiguration(), game::PlayerSet_t(OWNER), 15, h.session().translator(), h.session().log());
        a.check("addPlanet: isVisible", p.isVisible());
        return p;
    }

    Ship& addShip(afl::test::Assert a, SessionThread& h, int id)
    {
        Ptr<Game> g = h.session().getGame();
        a.checkNonNull("addShip: has game", g.get());

        Ship& sh = *g->currentTurn().universe().ships().create(id);
        game::map::ShipData data;
        data.owner                     = OWNER;
        data.x                         = LOC_X;
        data.y                         = LOC_Y;
        data.engineType                = 1;
        data.hullType                  = HULL_NR;
        data.beamType                  = 0;
        data.torpedoType               = 0;
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
        sh.internalCheck(game::PlayerSet_t(OWNER), 15);
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
AFL_TEST("game.proxy.CargoTransferProxy:empty", a)
{
    SessionThread h;
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);

    testee.init(CargoTransferSetup());

    // Check general
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    a.check("01. validTypes",       gen.validTypes.empty());
    a.check("02. allowUnload",     !gen.allowUnload);
    a.check("03. allowSupplySale", !gen.allowSupplySale);

    // Check participant
    CargoTransferProxy::Participant part;
    testee.getParticipantInformation(ind, 0, part);
    a.check("11. name",            part.name.empty());
    a.check("12. isUnloadTarget", !part.isUnloadTarget);
    a.check("13. isTemporary",    !part.isTemporary);
}

/** Test normal behaviour.
    A: create universe with two units. Initialize with correct setup. Move some cargo.
    E: status must be reported correctly. Commit must correctly update participants. */
AFL_TEST("game.proxy.CargoTransferProxy:normal", a)
{
    const int SHIP_ID = 78;
    const int PLANET_ID = 150;

    // Preconditions
    SessionThread h;
    prepare(h);
    Ship& sh = addShip(a, h, SHIP_ID);
    Planet& pl = addPlanet(a, h, PLANET_ID);
    CargoTransferSetup setup = CargoTransferSetup::fromPlanetShip(h.session().getGame()->currentTurn().universe(), PLANET_ID, SHIP_ID);
    a.check("01. isValid", setup.isValid());

    // Testee
    WaitIndicator ind;
    CargoTransferProxy testee(h.gameSender(), ind);
    testee.init(setup);

    // Check general
    CargoTransferProxy::General gen;
    testee.getGeneralInformation(ind, gen);
    a.check("11. Neutronium",      gen.validTypes.contains(Element::Neutronium));
    a.check("12. Money",           gen.validTypes.contains(Element::Money));
    a.check("13. allowUnload",     gen.allowUnload);
    a.check("14. allowSupplySale", gen.allowSupplySale);

    // Check participant. Left is planet.
    CargoTransferProxy::Participant part;
    testee.getParticipantInformation(ind, 0, part);
    a.checkEqual("21. name",      part.name, "Melmac");
    a.check("22. isUnloadTarget", part.isUnloadTarget);
    a.check("23. isTemporary",   !part.isTemporary);

    // Move some cargo
    testee.unload(false);
    testee.move(Element::Tritanium, 20, 0, 1, false);
    testee.commit();
    h.sync();

    // Verify postconditions
    a.checkEqual("31. Neutronium", sh.getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("32. Tritanium",  sh.getCargo(Element::Tritanium).orElse(-1),  20);
    a.checkEqual("33. Duranium",   sh.getCargo(Element::Duranium).orElse(-1),   0);
    a.checkEqual("34. Molybdenum", sh.getCargo(Element::Molybdenum).orElse(-1), 0);

    a.checkEqual("41. Neutronium", pl.getCargo(Element::Neutronium).orElse(-1), 500);
    a.checkEqual("42. Tritanium",  pl.getCargo(Element::Tritanium).orElse(-1),  2000 + 2 - 20);
    a.checkEqual("43. Duranium",   pl.getCargo(Element::Duranium).orElse(-1),   3000 + 4);
    a.checkEqual("44. Molybdenum", pl.getCargo(Element::Molybdenum).orElse(-1), 4000 + 6);
}

/** Test overload behaviour.
    A: create universe with two units. Initialize with correct setup. Move exercising overload.
    E: status must be reported correctly. Commit must correctly update participants. */
AFL_TEST("game.proxy.CargoTransferProxy:overload", a)
{
    const int SHIP_ID = 78;
    const int PLANET_ID = 150;

    // Preconditions
    SessionThread h;
    prepare(h);
    Ship& sh = addShip(a, h, SHIP_ID);
    /*Planet& pl =*/ addPlanet(a, h, PLANET_ID);
    CargoTransferSetup setup = CargoTransferSetup::fromPlanetShip(h.session().getGame()->currentTurn().universe(), PLANET_ID, SHIP_ID);
    a.check("01. isValid", setup.isValid());

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
    a.checkEqual("11. Tritanium", sh.getCargo(Element::Tritanium).orElse(-1),  52);
}

/** Test multi-ship transfer.
    A: create universe with multiple units. Initialize with multi-ship setup.
    E: status reported correctly. */
AFL_TEST("game.proxy.CargoTransferProxy:multi", a)
{
    // Preconditions
    SessionThread h;
    prepare(h);
    addShip(a, h, 1);
    addShip(a, h, 2);
    addShip(a, h, 3);
    addPlanet(a, h, 77);

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
    a.check("01. validTypes",           gen.validTypes.contains(Element::Duranium));
    a.checkEqual("02. numParticipants", gen.numParticipants, 5U);

    // Verify participants
    CargoTransferProxy::Participant part1;
    testee.getParticipantInformation(ind, 0, part1);
    a.checkEqual("11. name",   part1.name, "Hold space");
    a.checkEqual("12. info1",  part1.info1, "");
    a.checkEqual("13. info2",  part1.info2, "");
    a.check("14. isTemporary", part1.isTemporary);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    a.checkEqual("21. name",    part2.name, "Titanic");
    a.checkEqual("22. info1",   part2.info1, "BRUCE");
    a.checkEqual("23. info2",   part2.info2, "FCode: \"joe\", Damage: 12%");
    a.check("24. isTemporary", !part2.isTemporary);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    a.checkEqual("31. name",    part5.name, "Melmac");
    a.checkEqual("32. info1",   part5.info1, "Planet");
    a.checkEqual("33. info2",   part5.info2, "FCode: \"alf\"");
    a.check("34. isTemporary", !part5.isTemporary);
}

/** Test multi-ship transfer, moveExt.
    A: create universe with multiple units. Initialize with multi-ship setup; use moveExt.
    E: status reported correctly. */
AFL_TEST("game.proxy.CargoTransferProxy:multi:moveExt", a)
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(a, h, 1);              // has 20$
    addShip(a, h, 2);              // has 20$
    addShip(a, h, 3);              // has 20$
    addPlanet(a, h, 77);           // has 1000$

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
    a.checkEqual("part1", part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    a.checkEqual("part2", part2.cargo.amount.get(Element::Money), 120);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    a.checkEqual("part5", part5.cargo.amount.get(Element::Money), 900);
}

/** Test multi-ship transfer, moveAll.
    A: create universe with multiple units. Initialize with multi-ship setup; use moveAll.
    E: status reported correctly. */
AFL_TEST("game.proxy.CargoTransferProxy:multi:moveAll", a)
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(a, h, 1);              // has 20$
    addShip(a, h, 2);              // has 20$
    addShip(a, h, 3);              // has 20$
    addPlanet(a, h, 77);           // has 1000$

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
    a.checkEqual("part1", part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    a.checkEqual("part2", part2.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part3;
    testee.getParticipantInformation(ind, 2, part3);
    a.checkEqual("part3", part3.cargo.amount.get(Element::Money), 1040);

    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    a.checkEqual("part4", part4.cargo.amount.get(Element::Money), 20);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    a.checkEqual("part5", part5.cargo.amount.get(Element::Money), 0);
}

/** Test multi-ship transfer, distribute.
    A: create universe with multiple units. Initialize with multi-ship setup; use distribute.
    E: status reported correctly. */
AFL_TEST("game.proxy.CargoTransferProxy:multi:distribute", a)
{
    // Preconditions (same as testMulti)
    SessionThread h;
    prepare(h);
    addShip(a, h, 1);              // has 20$
    addShip(a, h, 2);              // has 20$
    addShip(a, h, 3);              // has 20$
    addPlanet(a, h, 77);           // has 1000$

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
    a.checkEqual("part1", part1.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part2;
    testee.getParticipantInformation(ind, 1, part2);
    a.checkEqual("part2", part2.cargo.amount.get(Element::Money), 30);

    CargoTransferProxy::Participant part3;
    testee.getParticipantInformation(ind, 2, part3);
    a.checkEqual("part3", part3.cargo.amount.get(Element::Money), 0);

    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    a.checkEqual("part4", part4.cargo.amount.get(Element::Money), 30);

    CargoTransferProxy::Participant part5;
    testee.getParticipantInformation(ind, 4, part5);
    a.checkEqual("part5", part5.cargo.amount.get(Element::Money), 1000);
}

/** Test multi-ship transfer, addHoldSpace.
    A: set up a cargo transfer. Use addHoldSpace().
    E: status reported correctly. */
AFL_TEST("game.proxy.CargoTransferProxy:multi:addHoldSpace", a)
{
    // Preconditions (similar to testMulti)
    SessionThread h;
    prepare(h);
    addShip(a, h, 1);              // has 20$
    addShip(a, h, 2);              // has 20$

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
    a.checkEqual("01. numParticipants", gen.numParticipants, 3U);

    // Add a new hold space
    testee.addHoldSpace("Bag");

    // Verify: now 4 participants
    testee.getGeneralInformation(ind, gen);
    a.checkEqual("11. numParticipants", gen.numParticipants, 4U);

    // Verify participants
    CargoTransferProxy::Participant part4;
    testee.getParticipantInformation(ind, 3, part4);
    a.checkEqual("21. name", part4.name, "Bag");
    a.check("22. isTemporary", part4.isTemporary);
}
