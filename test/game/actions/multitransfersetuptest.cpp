/**
  *  \file test/game/actions/multitransfersetuptest.cpp
  *  \brief Test for game::actions::MultiTransferSetup
  */

#include "game/actions/multitransfersetup.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/map/configuration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include <stdexcept>

using game::actions::MultiTransferSetup;

namespace {
    game::map::Ship& addShip(afl::test::Assert a, game::map::Universe& univ, int id, int x, int y, String_t name, int owner, game::map::Object::Playability playability)
    {
        game::map::Ship* sh = univ.ships().create(id);
        a.checkNonNull("create ship", sh);

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

    game::map::Planet& addPlanet(afl::test::Assert a, game::map::Universe& univ, int id, int x, int y, String_t name, int owner, game::map::Object::Playability playability)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::map::Configuration mapConfig;

        game::map::Planet* pl = univ.planets().create(id);
        a.checkNonNull("create planet", pl);

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
AFL_TEST("game.actions.MultiTransferSetup:error:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::map::Universe univ;
    game::actions::CargoTransfer action;

    game::actions::MultiTransferSetup testee;
    AFL_CHECK_THROWS(a, testee.build(action, univ, session), std::exception);
}

/** Test error behaviour: nonexistant unit.
    A: create session with shiplist, root. Call build().
    E: Failure result (this is not 100% contractual). */
AFL_TEST("game.actions.MultiTransferSetup:error:no-unit", a)
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
    a.checkEqual("01. status", r.status, MultiTransferSetup::Failure);
    a.checkEqual("02. getSupportedElementTypes", testee.getSupportedElementTypes(univ, *session.getShipList()), game::ElementTypes_t());
}

/** Test normal behaviour.
    A: create session with shiplist, root. Create universe with units. Call build().
    E: verify correct setup being built. */
AFL_TEST("game.actions.MultiTransferSetup:normal", a)
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
    addShip(a("s10"), univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable);
    addShip(a("s20"), univ, 20, 1000, 1000, "S20", 4, game::map::Object::Playable);
    addShip(a("s30"), univ, 30, 1000, 1000, "S30", 5, game::map::Object::Playable);
    addShip(a("s40"), univ, 40, 1000, 1000, "S40", 4, game::map::Object::NotPlayable);
    addShip(a("s50"), univ, 50, 1000, 1000, "S50", 4, game::map::Object::Playable);
    addShip(a("s60"), univ, 60, 1001, 1000, "S60", 4, game::map::Object::Playable);
    addPlanet(a("p70"), univ, 70, 1000, 1000, "P70", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(50);

    // Verify cargo types
    game::ElementTypes_t ty = testee.getSupportedElementTypes(univ, *shipList);
    a.check("01. getSupportedElementTypes", ty.contains(game::Element::Neutronium));
    a.check("02. getSupportedElementTypes", !ty.contains(game::Element::Fighters));

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::Tritanium);
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    a.checkEqual("11. status", r.status, MultiTransferSetup::Success);

    // Verify action
    a.checkEqual("21. getElementType", testee.getElementType(), game::Element::Tritanium);
    a.checkEqual("22. getShipId",      testee.getShipId(), 50);
    a.checkEqual("23. isFleetOnly",    testee.isFleetOnly(), false);

    // Verify:
    // - Hold Space
    // - S10
    // - S20    // not S30, wrong race; not S40, not playable; not S60, wrong place
    // - S50    // initial ship
    // - P70    // initial extension
    a.checkEqual("31. getNumContainers", action.getNumContainers(), 5U);
    a.checkEqual("32. thisShipIndex", r.thisShipIndex, 3U);
    a.checkEqual("33. extensionIndex", r.extensionIndex, 4U);
    a.checkEqual("34. getName", action.get(0)->getName(tx), "Hold space");
    a.checkEqual("35. getName", action.get(1)->getName(tx), "S10");
    a.checkEqual("36. getName", action.get(2)->getName(tx), "S20");
    a.checkEqual("37. getName", action.get(3)->getName(tx), "S50");
    a.checkEqual("38. getName", action.get(4)->getName(tx), "P70");
}

/** Test normal behaviour, no cargo case.
    A: create session with shiplist, root. Create universe with units that have no Tritanium. Call build().
    E: verify NoCargo result. */
AFL_TEST("game.actions.MultiTransferSetup:error:no-cargo", a)
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
    addShip(a("s10"), univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable)
        .setCargo(game::Element::Tritanium, 0);
    addShip(a("s20"), univ, 20, 1000, 1000, "S20", 4, game::map::Object::Playable)
        .setCargo(game::Element::Tritanium, 0);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(10);
    testee.setElementType(game::Element::Tritanium);

    // Build action
    game::actions::CargoTransfer action;
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    a.checkEqual("01. status", r.status, MultiTransferSetup::NoCargo);
}

/** Test normal behaviour, no peer case.
    A: create session with shiplist, root. Create universe with only one unit. Call build().
    E: verify NoPeer result. */
AFL_TEST("game.actions.MultiTransferSetup:error:no-peer", a)
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
    addShip(a("s10"), univ, 10, 1000, 1000, "S10", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(10);
    testee.setElementType(game::Element::Tritanium);

    // Build action
    game::actions::CargoTransfer action;
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    a.checkEqual("01. status", r.status, MultiTransferSetup::NoPeer);
}

/** Test fleet handling.
    A: create session with shiplist, root. Create universe with ships, some in a fleet. Call build().
    E: verify correct units added. */
AFL_TEST("game.actions.MultiTransferSetup:setFleetOnly", a)
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
    addShip(a("a1"), univ, 1, 1000, 1000, "S1", 4, game::map::Object::Playable)
        .setFleetNumber(3);
    addShip(a("s2"), univ, 2, 1000, 1000, "S2", 4, game::map::Object::Playable);
    addShip(a("s3"), univ, 3, 1000, 1000, "S3", 4, game::map::Object::Playable)
        .setFleetNumber(3);
    addShip(a("s4"), univ, 4, 1000, 1000, "S4", 4, game::map::Object::Playable);
    addPlanet(a("p70"), univ, 70, 1000, 1000, "P70", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(1);

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::Tritanium);
    testee.setFleetOnly(true);
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    a.checkEqual("01. status", r.status, MultiTransferSetup::Success);

    // Verify:
    // - Hold Space
    // - S1
    // - S3
    // - P70
    a.checkEqual("11. getNumContainers", action.getNumContainers(), 4U);
    a.checkEqual("12. thisShipIndex", r.thisShipIndex, 1U);
    a.checkEqual("13. extensionIndex", r.extensionIndex, 3U);
    a.checkEqual("14. getName", action.get(0)->getName(tx), "Hold space");
    a.checkEqual("15. getName", action.get(1)->getName(tx), "S1");
    a.checkEqual("16. getName", action.get(2)->getName(tx), "S3");
    a.checkEqual("17. getName", action.get(3)->getName(tx), "P70");
}

/** Test cargo type handling.
    A: create session with shiplist, root. Create universe with ships, different torpedo types. Call build().
    E: verify correct units added. */
AFL_TEST("game.actions.MultiTransferSetup:torpedo-type-mismatch", a)
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
    addShip(a("s1"), univ, 1, 1000, 1000, "S1", 4, game::map::Object::Playable);
    addShip(a("s2"), univ, 2, 1000, 1000, "S2", 4, game::map::Object::Playable)
        .setTorpedoType(3);
    addShip(a("s3"), univ, 3, 1000, 1000, "S3", 4, game::map::Object::Playable);

    game::actions::MultiTransferSetup testee;
    testee.setShipId(3);

    // Build action
    game::actions::CargoTransfer action;
    testee.setElementType(game::Element::fromTorpedoType(10));
    MultiTransferSetup::Result r = testee.build(action, univ, session);
    a.checkEqual("01. status", r.status, MultiTransferSetup::Success);

    // Verify:
    // - Hold Space
    // - S1
    // - S3
    a.checkEqual("11. getNumContainers", action.getNumContainers(), 3U);
    a.checkEqual("12. thisShipIndex", r.thisShipIndex, 2U);
    a.checkEqual("13. extensionIndex", r.extensionIndex, 0U);
    a.checkEqual("14. getName", action.get(0)->getName(tx), "Hold space");
    a.checkEqual("15. getName", action.get(1)->getName(tx), "S1");
    a.checkEqual("16. getName", action.get(2)->getName(tx), "S3");
}
