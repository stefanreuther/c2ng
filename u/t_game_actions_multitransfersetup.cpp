/**
  *  \file u/t_game_actions_multitransfersetup.cpp
  *  \brief Test for game::actions::MultiTransferSetup
  */

#include <stdexcept>
#include "game/actions/multitransfersetup.hpp"

#include "t_game_actions.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/map/configuration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

using game::actions::MultiTransferSetup;

namespace {
    game::map::Ship& addShip(game::map::Universe& univ, int id, int x, int y, String_t name, int owner, game::map::Object::Playability playability)
    {
        game::map::Ship* sh = univ.ships().create(id);
        TS_ASSERT(sh != 0);

        game::map::ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.name = name;
        sd.owner = owner;
        sd.crew = 10;
        sd.hullType = game::test::ANNIHILATION_HULL_ID;
        sd.engineType = 9;
        sd.beamType = 5;
        sd.numBeams = 3;
        sd.torpedoType = 10;
        sd.numLaunchers = 5;
        sd.ammo = 50;
        sd.tritanium = 100;
        sd.duranium = 100;
        sd.molybdenum = 100;
        sd.neutronium = 100;
        sd.colonists = 100;
        sd.money = 100;
        sd.supplies = 100;
        sh->addCurrentShipData(sd, game::PlayerSet_t(owner));
        sh->internalCheck(game::PlayerSet_t(owner), 10);
        sh->setPlayability(playability);
        return *sh;
    }

    game::map::Planet& addPlanet(game::map::Universe& univ, int id, int x, int y, String_t name, int owner, game::map::Object::Playability playability)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::map::Configuration mapConfig;

        game::map::Planet* pl = univ.planets().create(id);
        TS_ASSERT(pl != 0);

        pl->setPosition(game::map::Point(x, y));
        pl->setName(name);

        game::map::PlanetData pd;
        pd.owner = owner;
        pd.colonistClans = 100;
        pd.minedNeutronium = 100;
        pd.minedTritanium = 100;
        pd.minedDuranium = 100;
        pd.minedMolybdenum = 100;
        pd.supplies = 100;
        pd.money = 100;
        pl->addCurrentPlanetData(pd, game::PlayerSet_t(owner));
        pl->internalCheck(mapConfig, game::PlayerSet_t(owner), 10, tx, log);
        pl->setPlayability(playability);
        return *pl;
    }
}


/** Test error behaviour: empty session.
    A: create empty session. Call build().
    E: exception (this is not 100% contractual). */
void
TestGameActionsMultiTransferSetup::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::map::Universe univ;
    game::actions::CargoTransfer action;

    game::actions::MultiTransferSetup testee;
    TS_ASSERT_THROWS(testee.build(action, univ, session), std::exception);
}

/** Test error behaviour: nonexistant unit.
    A: create session with shiplist, root. Call build().
    E: Failure result (this is not 100% contractual). */
void
TestGameActionsMultiTransferSetup::testError()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    game::map::Universe univ;
    game::actions::CargoTransfer action;

    game::actions::MultiTransferSetup testee;
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::Failure);
    TS_ASSERT_EQUALS(testee.getSupportedElementTypes(univ, *session.getShipList()), game::ElementTypes_t());
}

/** Test normal behaviour.
    A: create session with shiplist, root. Create universe with units. Call build().
    E: verify correct setup being built. */
void
TestGameActionsMultiTransferSetup::testNormal()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    session.setShipList(shipList);
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::map::Universe univ;
    addShip(univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable);
    addShip(univ, 20, 1000, 1000, "S20", 4, game::map::Object::Playable);
    addShip(univ, 30, 1000, 1000, "S30", 5, game::map::Object::Playable);
    addShip(univ, 40, 1000, 1000, "S40", 4, game::map::Object::NotPlayable);
    addShip(univ, 50, 1000, 1000, "S50", 4, game::map::Object::Playable);
    addShip(univ, 60, 1001, 1000, "S60", 4, game::map::Object::Playable);
    addPlanet(univ, 70, 1000, 1000, "P70", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(50);

    // Verify cargo types
    game::ElementTypes_t ty = testee.getSupportedElementTypes(univ, *shipList);
    TS_ASSERT(ty.contains(game::Element::Neutronium));
    TS_ASSERT(!ty.contains(game::Element::Fighters));

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::Tritanium);
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::Success);

    // Verify action
    TS_ASSERT_EQUALS(testee.getElementType(), game::Element::Tritanium);
    TS_ASSERT_EQUALS(testee.getShipId(), 50);
    TS_ASSERT_EQUALS(testee.isFleetOnly(), false);

    // Verify:
    // - Hold Space
    // - S10
    // - S20    // not S30, wrong race; not S40, not playable; not S60, wrong place
    // - S50    // initial ship
    // - P70    // initial extension
    TS_ASSERT_EQUALS(action.getNumContainers(), 5U);
    TS_ASSERT_EQUALS(r.thisShipIndex, 3U);
    TS_ASSERT_EQUALS(r.extensionIndex, 4U);
    TS_ASSERT_EQUALS(action.get(0)->getName(tx), "Hold space");
    TS_ASSERT_EQUALS(action.get(1)->getName(tx), "S10");
    TS_ASSERT_EQUALS(action.get(2)->getName(tx), "S20");
    TS_ASSERT_EQUALS(action.get(3)->getName(tx), "S50");
    TS_ASSERT_EQUALS(action.get(4)->getName(tx), "P70");
}

/** Test normal behaviour, no cargo case.
    A: create session with shiplist, root. Create universe with units that have no Tritanium. Call build().
    E: verify NoCargo result. */
void
TestGameActionsMultiTransferSetup::testNoCargo()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    session.setShipList(shipList);
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::map::Universe univ;
    addShip(univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable)
        .setCargo(game::Element::Tritanium, 0);
    addShip(univ, 20, 1000, 1000, "S20", 4, game::map::Object::Playable)
        .setCargo(game::Element::Tritanium, 0);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(10);
    testee.setElementType(game::Element::Tritanium);

    // Build action
    game::actions::CargoTransfer action;
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::NoCargo);
}

/** Test normal behaviour, no peer case.
    A: create session with shiplist, root. Create universe with only one unit. Call build().
    E: verify NoPeer result. */
void
TestGameActionsMultiTransferSetup::testNoPeer()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    session.setShipList(shipList);
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::map::Universe univ;
    addShip(univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(10);
    testee.setElementType(game::Element::Tritanium);

    // Build action
    game::actions::CargoTransfer action;
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::NoPeer);
}

/** Test fleet handling.
    A: create session with shiplist, root. Create universe with ships, some in a fleet. Call build().
    E: verify correct units added. */
void
TestGameActionsMultiTransferSetup::testFleet()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    session.setShipList(shipList);
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::map::Universe univ;
    addShip(univ, 1, 1000, 1000, "S1", 4, game::map::Object::Playable)
        .setFleetNumber(3);
    addShip(univ, 2, 1000, 1000, "S2", 4, game::map::Object::Playable);
    addShip(univ, 3, 1000, 1000, "S3", 4, game::map::Object::Playable)
        .setFleetNumber(3);
    addShip(univ, 4, 1000, 1000, "S4", 4, game::map::Object::Playable);
    addPlanet(univ, 70, 1000, 1000, "P70", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(1);

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::Tritanium);
    testee.setFleetOnly(true);
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::Success);

    // Verify:
    // - Hold Space
    // - S1
    // - S3
    // - P70
    TS_ASSERT_EQUALS(action.getNumContainers(), 4U);
    TS_ASSERT_EQUALS(r.thisShipIndex, 1U);
    TS_ASSERT_EQUALS(r.extensionIndex, 3U);
    TS_ASSERT_EQUALS(action.get(0)->getName(tx), "Hold space");
    TS_ASSERT_EQUALS(action.get(1)->getName(tx), "S1");
    TS_ASSERT_EQUALS(action.get(2)->getName(tx), "S3");
    TS_ASSERT_EQUALS(action.get(3)->getName(tx), "P70");
}

/** Test cargo type handling.
    A: create session with shiplist, root. Create universe with ships, different torpedo types. Call build().
    E: verify correct units added. */
void
TestGameActionsMultiTransferSetup::testTypeMismatch()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
    session.setShipList(shipList);
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    game::map::Universe univ;
    addShip(univ, 1, 1000, 1000, "S1", 4, game::map::Object::Playable);
    addShip(univ, 2, 1000, 1000, "S2", 4, game::map::Object::Playable)
        .setTorpedoType(3);
    addShip(univ, 3, 1000, 1000, "S3", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(3);

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::fromTorpedoType(10));
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    TS_ASSERT_EQUALS(r.status, MultiTransferSetup::Success);

    // Verify:
    // - Hold Space
    // - S1
    // - S3
    TS_ASSERT_EQUALS(action.getNumContainers(), 3U);
    TS_ASSERT_EQUALS(r.thisShipIndex, 2U);
    TS_ASSERT_EQUALS(r.extensionIndex, 0U);
    TS_ASSERT_EQUALS(action.get(0)->getName(tx), "Hold space");
    TS_ASSERT_EQUALS(action.get(1)->getName(tx), "S1");
    TS_ASSERT_EQUALS(action.get(2)->getName(tx), "S3");
}

