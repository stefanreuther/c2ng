/**
  *  \file u/t_game_actions_cargotransfersetup.cpp
  *  \brief Test for game::actions::CargoTransferSetup
  */

#include "game/actions/cargotransfersetup.hpp"

#include "t_game_actions.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/exception.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/interpreterinterface.hpp"
#include "game/test/simpleturn.hpp"

using afl::string::NullTranslator;
using game::Element;
using game::actions::CargoTransfer;
using game::actions::CargoTransferSetup;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;


/** Test initial state.
    In initial state, a CargoTransferSetup reports failure. */
void
TestGameActionsCargoTransferSetup::testInit()
{
    SimpleTurn h;
    NullTranslator tx;
    CargoTransferSetup testee;

    // Status report
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);

    // Building throws
    CargoTransfer a;
    TS_ASSERT_THROWS(testee.build(a, h.turn(), h.config(), h.shipList(), h.version(), tx), game::Exception);
    TS_ASSERT_THROWS(testee.buildDirect(a, h.turn().universe(), h.config(), h.shipList(), tx), game::Exception);
}

/** Test creation from nonexistant objects.
    Construction of the CargoTransferSetup must succeed, but the resulting object must report failure. */
void
TestGameActionsCargoTransferSetup::testCreateNonexistant()
{
    const game::map::Universe univ;
    TS_ASSERT_EQUALS(CargoTransferSetup::fromPlanetShip(univ, 11, 22).getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(univ, 11, 22).getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(univ, 11).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of a transfer between two own played ships.
    The result must be a client-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipOwnShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), true);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 15);
}

/** Test creation of a transfer between two own played ships, direct version.
    The result must be a client-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipOwnShipDirect()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), true);
    testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 15);
}

/** Test creation of a transfer between two played ships of different owners.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipAlliedShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::Playable);       // note different race, but playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between two played ships of different owners, direct version.
    This request must fail. */
void
TestGameActionsCargoTransferSetup::testOwnShipAlliedShipDirect()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::Playable);       // note different race, but playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_THROWS(testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList(), tx), game::Exception);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::NotPlayable);       // note different race and not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a scanned ship of a different owner and a played ship.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testForeignShipOwnShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 7, Object::NotPlayable);      // note different owner and not playable
    h.addShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 4, 0, 1, false, false), 0);  // fails, cannot transfer this direction!
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 4, 1, 0, false, false), 4);  // note reversed direction
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 6);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 4);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 10);
}

/** Test creation of transfer between two scanned ships.
    The result must be a failure. */
void
TestGameActionsCargoTransferSetup::testForeignShipForeignShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 7, Object::NotPlayable);      // note not playable
    h.addShip(20, 5, Object::NotPlayable);      // note not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);

    // Use result
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_THROWS(testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx), game::Exception);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict case.
    CargoTransferSetup must offer to cancel the conflict, then produce the correct transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShipConflict()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(10, 5, Object::Playable);
    h.addShip(20, 7, Object::NotPlayable);       // note different race and not playable
    h.addShip(30, 8, Object::NotPlayable);       // for exposition only

    // Ship 10 starts with a cargo transfer
    h.universe().ships().get(10)->setTransporterTargetId(Ship::TransferTransporter, 30);
    h.universe().ships().get(10)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 8);

    // Build new transfer. We will have a conflict.
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.universe(), 10, 20);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 10);
    TS_ASSERT_EQUALS(testee.isDirect(), false);

    // Solve the conflict.
    testee.cancelConflictingTransfer(h.universe(), 10);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 18);
    CargoTransfer action;
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict, auto-cancel.
    CargoTransferSetup must automatically cancel the conflict. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShipAutoCancel()
{
    NullTranslator tx;
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
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 10);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.universe().ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.universe().ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between mismatching ships.
    Operation must report fail if ships are on different positions. */
void
TestGameActionsCargoTransferSetup::testShipMismatch()
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
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.universe(), 55, 66).getStatus(), CargoTransferSetup::Impossible);
    // - same Id
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.universe(), 55, 55).getStatus(), CargoTransferSetup::Impossible);
    // - first does not exist, second does
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.universe(), 1, 55).getStatus(), CargoTransferSetup::Impossible);
    // - second does not exist, first does
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.universe(), 55, 1).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, normal case.
    Transporter must be used as expected. */
void
TestGameActionsCargoTransferSetup::testJettisonNormal()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromShipJettison(h.universe(), 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 0);
}

/** Test creation of Jettison action, failure cases.
    Creation must fail for nonexistant or not played ships. */
void
TestGameActionsCargoTransferSetup::testJettisonFail()
{
    SimpleTurn h;
    h.addShip(42, 5, Object::NotPlayable);

    // Failure cases:
    // - nonexistant ship
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.universe(), 1).getStatus(), CargoTransferSetup::Impossible);
    // - existing but not played
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.universe(), 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, failure at planet.
    Creation must fail if the ship orbits a planet. */
void
TestGameActionsCargoTransferSetup::testJettisonFailPlanet()
{
    SimpleTurn h;
    h.addShip(42, 5, Object::NotPlayable);
    h.addPlanet(99, 2, Object::NotPlayable);

    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.universe(), 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, direct.
    Must fail because it is not a direct transfer. */
void
TestGameActionsCargoTransferSetup::testJettisonDirect()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);

    CargoTransferSetup testee = CargoTransferSetup::fromShipJettison(h.universe(), 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);

    CargoTransfer action;
    TS_ASSERT_THROWS(testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList(), tx), game::Exception);
}

/** Test creation of planet/ship transfer, own units.
    The action must be created correctly. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetOwnShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), true);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, own units, direct version
    The action must be created correctly. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetOwnShipDirect()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 5, Object::Playable);

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), true);
    testee.buildDirect(action, h.turn().universe(), h.config(), h.shipList(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, allied units.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetAlliedShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 8, Object::Playable);      // note different owner

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    The unit we're playing is the ship, so this requires a proxy. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetForeignShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.addPlanet(99, 5, Object::Playable);
    h.addShip(100, 5, Object::Playable);
    h.addShip(200, 8, Object::Playable);
    h.addShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    TS_ASSERT(!testee.isValidProxy(h.universe(), 300));  // wrong owner and not playable
    TS_ASSERT(!testee.isValidProxy(h.universe(), 200));  // wrong owner
    TS_ASSERT(!testee.isValidProxy(h.universe(), 42));
    TS_ASSERT( testee.isValidProxy(h.universe(), 100));  // valid

    // Set correct proxy
    CargoTransfer action;
    TS_ASSERT(testee.setProxy(h.universe(), 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign planet.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship.
    This is technically the same as testOwnPlanetAlliedShip. */
void
TestGameActionsCargoTransferSetup::testForeignPlanetOwnShip()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 5, Object::Playable);
    h.addPlanet(99, 8, Object::NotPlayable);      // note different owner and playability

    CargoTransfer action;
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.universe(), 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    This is the same as testOwnPlanetForeignShip, but with reversed sides. */
void
TestGameActionsCargoTransferSetup::testForeignShipOwnPlanet()
{
    NullTranslator tx;
    SimpleTurn h;
    h.addShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.addPlanet(99, 5, Object::Playable);
    h.addShip(100, 5, Object::Playable);
    h.addShip(200, 8, Object::Playable);
    h.addShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.turn().universe(), 99, 42);
    testee.swapSides();
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    TS_ASSERT(!testee.isValidProxy(h.universe(), 300));  // wrong owner and not playable
    TS_ASSERT(!testee.isValidProxy(h.universe(), 200));  // wrong owner
    TS_ASSERT(!testee.isValidProxy(h.universe(), 42));
    TS_ASSERT( testee.isValidProxy(h.universe(), 100));  // valid

    // Set correct proxy
    TS_ASSERT(testee.setProxy(h.universe(), 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 0);
    TS_ASSERT_EQUALS(testee.isDirect(), false);
    CargoTransfer action;
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 0);   // fails
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 1, 0, false, false), 5);   // note reversed direction
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign ship, conflict case.
    The unit we're playing is the ship, so this requires a proxy.
    The conflict must be detected. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetForeignShipConflict()
{
    NullTranslator tx;
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
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Set correct proxy
    TS_ASSERT(testee.setProxy(h.universe(), 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);

    // This produces a conflict. Auto-solve it.
    CargoTransfer action;
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.universe()), 100);
    testee.build(action, h.turn(), h.config(), h.shipList(), h.version(), tx);

    // Move
    TS_ASSERT_EQUALS(action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.universe().ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 30);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.universe().ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.universe().planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

