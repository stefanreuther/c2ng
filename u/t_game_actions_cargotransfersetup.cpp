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

using game::actions::CargoTransferSetup;
using game::map::Ship;
using game::map::Planet;
using game::map::Object;
using game::Element;

namespace {
    const int LOC_X = 1112;
    const int LOC_Y = 2223;

    class TestHarness {
     public:
        TestHarness()
            : action(),
              univ(),
              interface(),
              config(),
              shipList(),
              version(game::HostVersion::PHost, MKVERSION(3,5,0))
            { config.setDefaultValues(); }
        game::actions::CargoTransfer action;
        game::map::Universe univ;
        game::test::InterpreterInterface interface;
        game::config::HostConfiguration config;
        game::spec::ShipList shipList;
        game::HostVersion version;

        void prepareShip(int shipId, int owner, Object::Playability playability);
        void preparePlanet(int planetId, int owner, Object::Playability playability);
    };

    void TestHarness::prepareShip(int shipId, int owner, Object::Playability playability)
    {
        // Make sure there is a hull, so querying the ship's hull properties works.
        const int HULL_ID = 17;
        if (shipList.hulls().get(HULL_ID) == 0) {
            game::spec::Hull* pHull = shipList.hulls().create(HULL_ID);
            TS_ASSERT(pHull != 0);
            pHull->setMass(1);
            pHull->setMaxCargo(100);
            pHull->setMaxFuel(100);
        }

        // Create ship
        Ship* pShip = univ.ships().create(shipId);

        // - seed the ship to make it visible
        game::map::ShipData sd;
        sd.x = LOC_X;
        sd.y = LOC_Y;
        sd.owner = owner;
        pShip->addCurrentShipData(sd, game::PlayerSet_t(owner));
        pShip->internalCheck();
        pShip->setPlayability(playability);

        // - set some nice properties
        pShip->setHull(HULL_ID);
        static const Element::Type elems[] = { Element::Neutronium, Element::Tritanium, Element::Duranium,
                                               Element::Molybdenum, Element::Supplies, Element::Colonists,
                                               Element::Money };
        for (size_t i = 0; i < countof(elems); ++i) {
            pShip->setCargo(elems[i], 10);
            pShip->setTransporterCargo(Ship::TransferTransporter, elems[i], 0);
            pShip->setTransporterCargo(Ship::UnloadTransporter, elems[i], 0);
        }
        pShip->setTransporterTargetId(Ship::TransferTransporter, 0);
        pShip->setTransporterTargetId(Ship::UnloadTransporter, 0);
    }

    void TestHarness::preparePlanet(int planetId, int owner, Object::Playability playability)
    {
        // Create planet
        Planet* pPlanet = univ.planets().create(planetId);

        pPlanet->setPosition(game::map::Point(LOC_X, LOC_Y));

        game::map::PlanetData pd;
        pd.owner = owner;
        pd.minedNeutronium = 1000;
        pd.minedTritanium = 1000;
        pd.minedDuranium = 1000;
        pd.minedMolybdenum = 1000;
        pd.colonistClans = 1000;
        pd.money = 1000;
        pd.supplies = 1000;
        pPlanet->addCurrentPlanetData(pd, game::PlayerSet_t(owner));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        pPlanet->internalCheck(game::map::Configuration(), tx, log);
        pPlanet->setPlayability(playability);
    }
}


/** Test initial state.
    In initial state, a CargoTransferSetup reports failure. */
void
TestGameActionsCargoTransferSetup::testInit()
{
    TestHarness h;
    CargoTransferSetup testee;

    // Status report
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);

    // Building throws
    TS_ASSERT_THROWS(testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version), game::Exception);
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
    TestHarness h;
    h.prepareShip(10, 5, Object::Playable);
    h.prepareShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);

    // Use result
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 15);
}

/** Test creation of a transfer between two played ships of different owners.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipAlliedShip()
{
    TestHarness h;
    h.prepareShip(10, 5, Object::Playable);
    h.prepareShip(20, 7, Object::Playable);       // note different race, but playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);

    // Use result
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShip()
{
    TestHarness h;
    h.prepareShip(10, 5, Object::Playable);
    h.prepareShip(20, 7, Object::NotPlayable);       // note different race and not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);

    // Use result
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a scanned ship of a different owner and a played ship.
    The result must be a host-side transfer. */
void
TestGameActionsCargoTransferSetup::testForeignShipOwnShip()
{
    TestHarness h;
    h.prepareShip(10, 7, Object::NotPlayable);      // note different owner and not playable
    h.prepareShip(20, 5, Object::Playable);
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);

    // Use result
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 4, 0, 1, false, false), 0);  // fails, cannot transfer this direction!
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 4, 1, 0, false, false), 4);  // note reversed direction
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 6);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 4);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 10);
}

/** Test creation of transfer between two scanned ships.
    The result must be a failure. */
void
TestGameActionsCargoTransferSetup::testForeignShipForeignShip()
{
    TestHarness h;
    h.prepareShip(10, 7, Object::NotPlayable);      // note not playable
    h.prepareShip(20, 5, Object::NotPlayable);      // note not playable
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);

    // Use result
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Impossible);
    TS_ASSERT_THROWS(testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version), game::Exception);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict case.
    CargoTransferSetup must offer to cancel the conflict, then produce the correct transfer. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShipConflict()
{
    TestHarness h;
    h.prepareShip(10, 5, Object::Playable);
    h.prepareShip(20, 7, Object::NotPlayable);       // note different race and not playable
    h.prepareShip(30, 8, Object::NotPlayable);       // for exposition only

    // Ship 10 starts with a cargo transfer
    h.univ.ships().get(10)->setTransporterTargetId(Ship::TransferTransporter, 30);
    h.univ.ships().get(10)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 8);

    // Build new transfer. We will have a conflict.
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 10);

    // Solve the conflict.
    testee.cancelConflictingTransfer(h.univ, 10);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 18);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between a played ship and a scanned ship of a different owner, conflict, auto-cancel.
    CargoTransferSetup must automatically cancel the conflict. */
void
TestGameActionsCargoTransferSetup::testOwnShipForeignShipAutoCancel()
{
    TestHarness h;
    h.prepareShip(10, 5, Object::Playable);
    h.prepareShip(20, 7, Object::NotPlayable);       // note different race and not playable
    h.prepareShip(30, 8, Object::NotPlayable);       // for exposition only

    // Ship 10 starts with a cargo transfer
    h.univ.ships().get(10)->setTransporterTargetId(Ship::TransferTransporter, 30);
    h.univ.ships().get(10)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 8);

    // Build new transfer. We will have a conflict which we ignore.
    CargoTransferSetup testee = CargoTransferSetup::fromShipShip(h.univ, 10, 20);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 10);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.univ.ships().get(10)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 20);
    TS_ASSERT_EQUALS(h.univ.ships().get(20)->getCargo(Element::Neutronium).orElse(-1), 10);
}

/** Test creation of a transfer between mismatching ships.
    Operation must report fail if ships are on different positions. */
void
TestGameActionsCargoTransferSetup::testShipMismatch()
{
    TestHarness h;
    h.prepareShip(55, 5, Object::Playable);
    h.prepareShip(66, 5, Object::Playable);

    // Move ship 66
    {
        game::map::ShipData sd;
        h.univ.ships().get(66)->getCurrentShipData(sd);
        sd.x = LOC_X + 1;
        sd.y = LOC_Y;
        h.univ.ships().get(66)->addCurrentShipData(sd, game::PlayerSet_t(5));
    }

    // Create various failing actions
    // - different location
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.univ, 55, 66).getStatus(), CargoTransferSetup::Impossible);
    // - same Id
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.univ, 55, 55).getStatus(), CargoTransferSetup::Impossible);
    // - first does not exist, second does
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.univ, 1, 55).getStatus(), CargoTransferSetup::Impossible);
    // - second does not exist, first does
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipShip(h.univ, 55, 1).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, normal case.
    Transporter must be used as expected. */
void
TestGameActionsCargoTransferSetup::testJettisonNormal()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::Playable);

    CargoTransferSetup testee = CargoTransferSetup::fromShipJettison(h.univ, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 3, 0, 1, false, false), 3);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 7);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 3);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 0);
}

/** Test creation of Jettison action, failure cases.
    Creation must fail for nonexistant or not played ships. */
void
TestGameActionsCargoTransferSetup::testJettisonFail()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::NotPlayable);

    // Failure cases:
    // - nonexistant ship
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.univ, 1).getStatus(), CargoTransferSetup::Impossible);
    // - existing but not played
    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.univ, 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of Jettison action, failure at planet.
    Creation must fail if the ship orbits a planet. */
void
TestGameActionsCargoTransferSetup::testJettisonFailPlanet()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::NotPlayable);
    h.preparePlanet(99, 2, Object::NotPlayable);

    TS_ASSERT_EQUALS(CargoTransferSetup::fromShipJettison(h.univ, 42).getStatus(), CargoTransferSetup::Impossible);
}

/** Test creation of planet/ship transfer, own units.
    The action must be created correctly. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetOwnShip()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::Playable);
    h.preparePlanet(99, 5, Object::Playable);

    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 15);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, allied units.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetAlliedShip()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::Playable);
    h.preparePlanet(99, 8, Object::Playable);      // note different owner

    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    The unit we're playing is the ship, so this requires a proxy. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetForeignShip()
{
    TestHarness h;
    h.prepareShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.preparePlanet(99, 5, Object::Playable);
    h.prepareShip(100, 5, Object::Playable);
    h.prepareShip(200, 8, Object::Playable);
    h.prepareShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    TS_ASSERT(!testee.isValidProxy(h.univ, 300));  // wrong owner and not playable
    TS_ASSERT(!testee.isValidProxy(h.univ, 200));  // wrong owner
    TS_ASSERT(!testee.isValidProxy(h.univ, 42));
    TS_ASSERT( testee.isValidProxy(h.univ, 100));  // valid

    // Set correct proxy
    TS_ASSERT(testee.setProxy(h.univ, 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign planet.
    Since a direct transfer is not possible, this will produce a ship/planet transfer from the ship.
    This is technically the same as testOwnPlanetAlliedShip. */
void
TestGameActionsCargoTransferSetup::testForeignPlanetOwnShip()
{
    TestHarness h;
    h.prepareShip(42, 5, Object::Playable);
    h.preparePlanet(99, 8, Object::NotPlayable);      // note different owner and playability

    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 0);     // planet->ship fails
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 1, 0, false, false), 5);     // note reversed direction
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterTargetId(Ship::UnloadTransporter).orElse(-1), 99);
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getTransporterCargo(Ship::UnloadTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 1000);
}

/** Test creation of planet/ship transfer, foreign ship.
    This is the same as testOwnPlanetForeignShip, but with reversed sides. */
void
TestGameActionsCargoTransferSetup::testForeignShipOwnPlanet()
{
    TestHarness h;
    h.prepareShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.preparePlanet(99, 5, Object::Playable);
    h.prepareShip(100, 5, Object::Playable);
    h.prepareShip(200, 8, Object::Playable);
    h.prepareShip(300, 8, Object::NotPlayable);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    testee.swapSides();
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Try proxies
    TS_ASSERT(!testee.isValidProxy(h.univ, 300));  // wrong owner and not playable
    TS_ASSERT(!testee.isValidProxy(h.univ, 200));  // wrong owner
    TS_ASSERT(!testee.isValidProxy(h.univ, 42));
    TS_ASSERT( testee.isValidProxy(h.univ, 100));  // valid

    // Set correct proxy
    TS_ASSERT(testee.setProxy(h.univ, 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 0);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 0);   // fails
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 1, 0, false, false), 5);   // note reversed direction
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

/** Test creation of planet/ship transfer, foreign ship, conflict case.
    The unit we're playing is the ship, so this requires a proxy.
    The conflict must be detected. */
void
TestGameActionsCargoTransferSetup::testOwnPlanetForeignShipConflict()
{
    TestHarness h;
    h.prepareShip(42, 8, Object::NotPlayable);      // note different owner and not playable
    h.preparePlanet(99, 5, Object::Playable);
    h.prepareShip(100, 5, Object::Playable);
    h.prepareShip(200, 8, Object::Playable);

    // Ship 100 starts with a cargo transfer
    h.univ.ships().get(100)->setTransporterTargetId(Ship::TransferTransporter, 200);
    h.univ.ships().get(100)->setTransporterCargo(Ship::TransferTransporter, Element::Neutronium, 20);

    // Create transfer.
    CargoTransferSetup testee = CargoTransferSetup::fromPlanetShip(h.univ, 99, 42);
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::NeedProxy);

    // Set correct proxy
    TS_ASSERT(testee.setProxy(h.univ, 100));
    TS_ASSERT_EQUALS(testee.getStatus(), CargoTransferSetup::Ready);

    // This produces a conflict. Auto-solve it.
    TS_ASSERT_EQUALS(testee.getConflictingTransferShipId(h.univ), 100);
    testee.build(h.action, h.univ, h.interface, h.config, h.shipList, h.version);

    // Move
    TS_ASSERT_EQUALS(h.action.move(Element::Neutronium, 5, 0, 1, false, false), 5);
    h.action.commit();

    // Verify result of move
    TS_ASSERT_EQUALS(h.univ.ships().get(42)->getCargo(Element::Neutronium).orElse(-1), 10);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getCargo(Element::Neutronium).orElse(-1), 30);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterTargetId(Ship::TransferTransporter).orElse(-1), 42);
    TS_ASSERT_EQUALS(h.univ.ships().get(100)->getTransporterCargo(Ship::TransferTransporter, Element::Neutronium).orElse(-1), 5);
    TS_ASSERT_EQUALS(h.univ.planets().get(99)->getCargo(Element::Neutronium).orElse(-1), 995);
}

