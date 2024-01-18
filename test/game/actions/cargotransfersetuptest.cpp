/**
  *  \file test/game/actions/cargotransfersetuptest.cpp
  *  \brief Test for game::actions::CargoTransferSetup
  */

#include "game/actions/cargotransfersetup.hpp"

#include "afl/base/countof.hpp"
#include "afl/test/testrunner.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/exception.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/interpreterinterface.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::actions::CargoTransfer;
using game::actions::CargoTransferSetup;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;


/** Test initial state.
    In initial state, a CargoTransferSetup reports failure. */
AFL_TEST("game.actions.CargoTransferSetup:init", a)
{
    SimpleTurn h;
    CargoTransferSetup testee;

    // Status report
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Impossible);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);

    // Building throws
    CargoTransfer act;
    AFL_CHECK_THROWS(a("11. build"), testee.build(act, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version()), game::Exception);
    AFL_CHECK_THROWS(a("12. buildDirect"), testee.buildDirect(act, h.turn().universe(), h.config(), h.shipList()), game::Exception);
}

/** Test creation from nonexistant objects.
    Construction of the CargoTransferSetup must succeed, but the resulting object must report failure. */
AFL_TEST("game.actions.CargoTransferSetup:error:non-existant", a)
{
    const game::map::Universe univ;
    a.checkEqual("01. fromPlanetShip",   CargoTransferSetup::fromPlanetShip(univ, 11, 22).getStatus(), CargoTransferSetup::Impossible);
    a.checkEqual("02. fromShipShip",     CargoTransferSetup::fromShipShip(univ, 11, 22).getStatus(), CargoTransferSetup::Impossible);
    a.checkEqual("03. fromShipJettison", CargoTransferSetup::fromShipJettison(univ, 11).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of a transfer between two own played ships.
    The result must be a client-side transfer. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-ships", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), true);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo", h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 5);
    a.checkEqual("22. getCargo", h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 15);
}

/** Test creation of a transfer between two own played ships, direct version.
    The result must be a client-side transfer. */
AFL_TEST("game.actions.CargoTransferSetup:buildDirect:own-ships", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), true);
    testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo", h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 5);
    a.checkEqual("22. getCargo", h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 15);
}

/** Test creation of a transfer between two played ships of different owners.
    The result must be a host-side transfer. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-allied-ship", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::Playable);       // note different race, but playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    a.checkEqual("22. getTransporterCargo",    h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    a.checkEqual("23. getTransporterTargetId", h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    a.checkEqual("24. getCargo",               h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between two played ships of different owners, direct version.
    This request must fail. */
AFL_TEST("game.actions.CargoTransferSetup:buildDirect:own-allied-ship", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::Playable);       // note different race, but playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    AFL_CHECK_THROWS(a("01. buildDirect"), testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList()), game::Exception);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner.
    The result must be a host-side transfer. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-foreign-ship", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::NotPlayable);       // note different race and not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    a.checkEqual("22. getTransporterCargo",    h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    a.checkEqual("23. getTransporterTargetId", h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    a.checkEqual("24. getCargo",               h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a scanned ship of a different owner and a played ship.
    The result must be a host-side transfer. */
AFL_TEST("game.actions.CargoTransferSetup:build:foreign-own-ship", a)
{
    SimpleTurn h;
    h.addShip(10, 7, Object::NotPlayable);      // note different owner and not playable
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 4, 0, 1, false, false), 0);  // fails, cannot transfer this direction!
    a.checkEqual("12. move", action.move(Element::Neutronium, 4, 1, 0, false, false), 4);  // note reversed direction
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("22. getCargo",               h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 6);
    a.checkEqual("23. getTransporterCargo",    h.universe().ships().get(20)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 4);
    a.checkEqual("24. getTransporterTargetId", h.universe().ships().get(20)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 10);
}

/** Test creation of transfer between two scanned ships.
    The result must be a failure. */
AFL_TEST("game.actions.CargoTransferSetup:build:foreign-ships", a)
{
    SimpleTurn h;
    h.addShip(10, 7, Object::NotPlayable);      // note not playable
    h.addShip(20, 5, Object::NotPlayable);      // note not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Impossible);
    AFL_CHECK_THROWS(a("02. build"), testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version()), game::Exception);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict case.
    CargoTransferSetup must offer to cancel the conflict, then produce the correct transfer. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-foreign-ship:conflict", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::NotPlayable);       // note different race and not playable
    h.addShip(30, 8, Object::NotPlayable);       // for exposition only

    // Ship 10 starts with a cargo transfer
    h.universe().ships().get(10)->setTransporterTargetId(Ship::TransferTransporter, 30);
    h.universe().ships().get(10)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 8);

    // Build new transfer. We will have a conflict.
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 10);
    a.checkEqual("03. isDirect", testee.isDirect(), false);

    // Solve the conflict.
    testee.cancelConflictingTransfer(h.universe(), 10);
    a.checkEqual("11. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("12. getCargo", h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 18);
    CargoTransfer action;
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("21. move", action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    a.checkEqual("31. getCargo",               h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    a.checkEqual("32. getTransporterCargo",    h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    a.checkEqual("33. getTransporterTargetId", h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    a.checkEqual("34. getCargo",               h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict, auto-cancel.
    CargoTransferSetup must automatically cancel the conflict. */
AFL_TEST("game.actions.CargoTransferSetup:own-foreign-ship:auto-cancel", a)
{
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::NotPlayable);       // note different race and not playable
    h.addShip(30, 8, Object::NotPlayable);       // for exposition only

    // Ship 10 starts with a cargo transfer
    h.universe().ships().get(10)->setTransporterTargetId(Ship::TransferTransporter, 30);
    h.universe().ships().get(10)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 8);

    // Build new transfer. We will have a conflict which we ignore.
    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 10);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    a.checkEqual("22. getTransporterCargo",    h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    a.checkEqual("23. getTransporterTargetId", h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    a.checkEqual("24. getCargo",               h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between mismatching ships.
    Operation must report fail if ships are on different positions. */
AFL_TEST("game.actions.CargoTransferSetup:error:mismatch", a)
{
    SimpleTurn h;
    h.addShip(55, 5, Object::Playable);
    h.addShip(66, 5, Object::Playable);

    // Move ship 66
    {
        game::map::ShipData sd;
        h.universe().ships().get(66)->getCurrentShipData(sd);
        sd.x = 1001;
        sd.y = 2002;
        h.universe().ships().get(66)->addCurrentShipData(sd, game::PlayerSet_t(5));
    }

    // Create various failing actions
    // - different location
    a.checkEqual("01. position mismatch", CargoTransferSetup::fromShipShip(h.universe(), 55, 66).getStatus(), CargoTransferSetup::Impossible);
    // - same Id
    a.checkEqual("02. same id",           CargoTransferSetup::fromShipShip(h.universe(), 55, 55).getStatus(), CargoTransferSetup::Impossible);
    // - first does not exist, second does
    a.checkEqual("03. first missing",     CargoTransferSetup::fromShipShip(h.universe(),  1, 55).getStatus(), CargoTransferSetup::Impossible);
    // - second does not exist, first does
    a.checkEqual("04. second missing",    CargoTransferSetup::fromShipShip(h.universe(), 55,  1).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, normal case.
    Transporter must be used as expected. */
AFL_TEST("game.actions.CargoTransferSetup:jettison", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromShipJettison(h.universe(), 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 7);
    a.checkEqual("22. getTransporterCargo",    h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 3);
    a.checkEqual("23. getTransporterTargetId", h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 0);
}

/** Test creation of Jettison action, failure cases.
    Creation must fail for nonexistant or not played ships. */
AFL_TEST("game.actions.CargoTransferSetup:error:jettison", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::NotPlayable);

    // Failure cases:
    // - nonexistant ship
    a.checkEqual("01. missing ship", CargoTransferSetup::fromShipJettison(h.universe(), 1).getStatus(), CargoTransferSetup::Impossible);
    // - existing but not played
    a.checkEqual("02. ship not played", CargoTransferSetup::fromShipJettison(h.universe(), 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, failure at planet.
    Creation must fail if the ship orbits a planet. */
AFL_TEST("game.actions.CargoTransferSetup:error:jettison-at-planet", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::NotPlayable);
    h.addPlanet(99, 2, Object::NotPlayable);

    a.checkEqual("", CargoTransferSetup::fromShipJettison(h.universe(), 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, direct.
    Must fail because it is not a direct transfer. */
AFL_TEST("game.actions.CargoTransferSetup:buildDirect:jettison", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);

    CargoTransferSetup testee = CargoTransferSetup::fromShipJettison(h.universe(), 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);

    CargoTransfer action;
    AFL_CHECK_THROWS(a("11. buildDirect"), testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList()), game::Exception);
}

/** Test creation of planet/ship transfer, own units.
    The action must be created correctly. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-planet-ship", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), true);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo", h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 15);
    a.checkEqual("22. getCargo", h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, own units, direct version
    The action must be created correctly. */
AFL_TEST("game.actions.CargoTransferSetup:buildDirect:own-planet-ship", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), true);
    testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo", h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 15);
    a.checkEqual("22. getCargo", h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, allied units.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-planet-allied-ship", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 8, Object::Playable);      // note different owner

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    a.checkEqual("12. move", action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    a.checkEqual("22. getTransporterTargetId", h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    a.checkEqual("23. getTransporterCargo",    h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    a.checkEqual("24. getCargo",               h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    The unit we're playing is the ship, so this requires a proxy. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-planet-foreign-ship", a)
{
    SimpleTurn h;
    h.addShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.addPlanet(99, 5, Object::Playable);
    h.addShip(100, 5, Object::Playable);
    h.addShip(200, 8, Object::Playable);
    h.addShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    a.check("11. isValidProxy", !testee.isValidProxy(h.universe(), 300));  // wrong owner and not playable
    a.check("12. isValidProxy", !testee.isValidProxy(h.universe(), 200));  // wrong owner
    a.check("13. isValidProxy", !testee.isValidProxy(h.universe(), 42));
    a.check("14. isValidProxy",  testee.isValidProxy(h.universe(), 100));  // valid

    // Set correct proxy
    CargoTransfer action;
    a.check("21. setProxy", testee.setProxy(h.universe(), 100));
    a.checkEqual("22. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("23. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("24. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("31. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("41. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("42. getCargo",               h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("43. getTransporterTargetId", h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    a.checkEqual("44. getTransporterCargo",    h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    a.checkEqual("45. getCargo",               h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign planet.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship.
    This is technically the same as testOwnPlanetAlliedShip. */
AFL_TEST("game.actions.CargoTransferSetup:build:foreign-planet-own-ship", a)
{
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 8, Object::NotPlayable);      // note different owner and playability

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("02. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("03. isDirect", testee.isDirect(), false);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("11. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    a.checkEqual("12. move", action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    action.commit();

    // Verify result of move
    a.checkEqual("21. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    a.checkEqual("22. getTransporterTargetId", h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    a.checkEqual("23. getTransporterCargo",    h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    a.checkEqual("24. getCargo",               h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    This is the same as testOwnPlanetForeignShip, but with reversed sides. */
AFL_TEST("game.actions.CargoTransferSetup:build:foreign-ship-own-planet", a)
{
    SimpleTurn h;
    h.addShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.addPlanet(99, 5, Object::Playable);
    h.addShip(100, 5, Object::Playable);
    h.addShip(200, 8, Object::Playable);
    h.addShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.turn().universe(), 99, 42);
    testee.swapSides();
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    a.check("11. isValidProxy", !testee.isValidProxy(h.universe(), 300));  // wrong owner and not playable
    a.check("12. isValidProxy", !testee.isValidProxy(h.universe(), 200));  // wrong owner
    a.check("13. isValidProxy", !testee.isValidProxy(h.universe(), 42));
    a.check("14. isValidProxy",  testee.isValidProxy(h.universe(), 100));  // valid

    // Set correct proxy
    a.check("21", testee.setProxy(h.universe(), 100));
    a.checkEqual("22. getStatus", testee.getStatus(), CargoTransferSetup::Ready);
    a.checkEqual("23. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 0);
    a.checkEqual("24. isDirect", testee.isDirect(), false);
    CargoTransfer action;
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("31. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 0);   // fails
    a.checkEqual("32. move", action.move(Element::Neutronium, 5, 1, 0, false, false), 5);   // note reversed direction
    action.commit();

    // Verify result of move
    a.checkEqual("41. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("42. getCargo",               h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("43. getTransporterTargetId", h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    a.checkEqual("44. getTransporterCargo",    h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    a.checkEqual("45. getCargo",               h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign ship, conflict case.
    The unit we're playing is the ship, so this requires a proxy.
    The conflict must be detected. */
AFL_TEST("game.actions.CargoTransferSetup:build:own-planet-foreign-ship:proxy-conflict", a)
{
    SimpleTurn h;
    h.addShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.addPlanet(99, 5, Object::Playable);
    h.addShip(100, 5, Object::Playable);
    h.addShip(200, 8, Object::Playable);

    // Ship 100 starts with a cargo transfer
    h.universe().ships().get(100)->setTransporterTargetId(Ship::TransferTransporter, 200);
    h.universe().ships().get(100)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 20);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    a.checkEqual("01. getStatus", testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Set correct proxy
    a.check("11. setProxy", testee.setProxy(h.universe(), 100));
    a.checkEqual("12. getStatus", testee.getStatus(), CargoTransferSetup::Ready);

    // This produces a conflict. Auto-solve it.
    CargoTransfer action;
    a.checkEqual("21. getConflictingTransferShipId", testee.getConflictingTransferShipId(h.universe()), 100);
    testee.build(action, h.turn(), h.mapConfiguration(), h.config(), h.shipList(), h.version());

    // Move
    a.checkEqual("31. move", action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    a.checkEqual("41. getCargo",               h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    a.checkEqual("42. getCargo",               h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 30);
    a.checkEqual("43. getTransporterTargetId", h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    a.checkEqual("44. getTransporterCargo",    h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    a.checkEqual("45. getCargo",               h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}
