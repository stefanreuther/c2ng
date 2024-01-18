/**
  *  \file test/game/v3/undoinformationtest.cpp
  *  \brief Test for game::v3::UndoInformation
  */

#include "game/v3/undoinformation.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "game/v3/reverter.hpp"

namespace {
    const int PLANET_ID = 92;
    const int OWNER = 3;
    const int TURN_NR = 92;
    const int X = 1111;
    const int Y = 2222;

    struct TestHarness {
        game::Turn turn;
        game::map::Universe& univ;
        game::map::Planet& planet;
        afl::base::Ref<game::spec::ShipList> shipList;
        afl::base::Ref<game::Root> root;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::config::HostConfiguration& config;

        TestHarness()
            : turn(),
              univ(turn.universe()),
              planet(*univ.planets().create(PLANET_ID)),
              shipList(*new game::spec::ShipList()),
              root(*new game::Root(afl::io::InternalDirectory::create("game dir"),
                                   *new game::test::SpecificationLoader(),
                                   game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 47)),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                   game::Root::Actions_t())),
              tx(),
              fs(),
              session(tx, fs),
              config(root->hostConfiguration())
            {
                root->hostConfiguration().setDefaultValues();
                session.setShipList(shipList.asPtr());
                session.setRoot(root.asPtr());
            }
    };

    void prepare(TestHarness& h)
    {
        // Define base storage. This is the only way to reserve memory for base storage.
        // Planet::setBaseStorage only accesses present slots and never creates new ones.
        game::map::BaseData bd;
        for (int i = 0; i < 20; ++i) {
            bd.hullStorage.set(i, 0);
            bd.engineStorage.set(i, 0);
            bd.beamStorage.set(i, 0);
            bd.launcherStorage.set(i, 0);
            bd.torpedoStorage.set(i, 0);
        }

        afl::sys::Log log;
        afl::string::NullTranslator tx;

        // Define planet with base
        h.planet.setPosition(game::map::Point(X, Y));
        h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(OWNER));
        h.planet.addCurrentBaseData(bd, game::PlayerSet_t(OWNER));
        h.planet.setOwner(OWNER);
        h.planet.setBaseTechLevel(game::HullTech, 1);
        h.planet.setBaseTechLevel(game::EngineTech, 1);
        h.planet.setBaseTechLevel(game::BeamTech, 1);
        h.planet.setBaseTechLevel(game::TorpedoTech, 1);
        h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(OWNER), TURN_NR, tx, log);
        h.planet.setPlayability(game::map::Object::Playable);

        // Define a number of components
        // - Hull #9
        {
            game::spec::Hull* hh = h.shipList->hulls().create(9);
            hh->setTechLevel(2);
            hh->cost() = game::spec::Cost::fromString("10T 15$");
        }
        // - Engine #1
        {
            game::spec::Engine* e = h.shipList->engines().create(1);
            e->setTechLevel(1);
            e->cost() = game::spec::Cost::fromString("1TDM 1$");
        }
        // - Beam #4
        {
            game::spec::Beam* b = h.shipList->beams().create(4);
            b->setTechLevel(4);
            b->cost() = game::spec::Cost::fromString("4M");
        }
        // - Launcher #3
        {
            game::spec::TorpedoLauncher* tl = h.shipList->launchers().create(3);
            tl->setTechLevel(3);
            tl->cost() = game::spec::Cost::fromString("4M 30S");
        }
        // - Hull association
        h.shipList->hullAssignments().add(OWNER, 12, 9);
    }

    game::v3::Reverter& prepareReverter(TestHarness& h)
    {
        game::v3::Reverter* pRev = new game::v3::Reverter(h.turn, h.session);
        h.univ.setNewReverter(pRev);

        game::map::BaseData bd;
        game::map::PlanetData pd;
        h.planet.getCurrentBaseData(bd);
        h.planet.getCurrentPlanetData(pd);
        pRev->addBaseData(PLANET_ID, bd);
        pRev->addPlanetData(PLANET_ID, pd);

        return *pRev;
    }

    game::map::Ship& prepareShip(afl::test::Assert a, TestHarness& h, int id, int owner)
    {
        game::map::ShipData sd(id);
        sd.owner = owner;
        sd.x = X;
        sd.y = Y;
        sd.engineType = 1;
        sd.beamType = 4;
        sd.numBeams = 5;
        sd.numBays = 0;
        sd.torpedoType = 3;
        sd.ammo = 100;
        sd.numLaunchers = 8;
        sd.supplies = 1000;
        sd.money = 1000;

        game::map::Ship* pShip = h.univ.ships().create(id);
        a.checkNonNull("prepareShip > ship created", pShip);
        pShip->addCurrentShipData(sd, game::PlayerSet_t(OWNER));

        game::v3::Reverter* pRev = dynamic_cast<game::v3::Reverter*>(h.univ.getReverter());
        a.checkNonNull("prepareShip > reverter", pRev);
        pRev->addShipData(id, sd);

        pShip->internalCheck(game::PlayerSet_t(owner), TURN_NR);
        pShip->setPlayability(game::map::Object::Playable);

        return *pShip;
    }
}

/** Test empty (uninitialized) case. */
AFL_TEST("game.v3.UndoInformation:empty", a)
{
    game::v3::UndoInformation testee;
    a.checkEqual("01. getNumTorpedoesAllowedToSell", testee.getNumTorpedoesAllowedToSell(0), 0);
    a.checkEqual("02. getNumFightersAllowedToSell",  testee.getNumFightersAllowedToSell(), 0);
    a.checkEqual("03. getSuppliesAllowedToBuy",      testee.getSuppliesAllowedToBuy(), 0);
    a.checkEqual("04. min HullTech",                 testee.getMinTechLevel(game::HullTech), 1);
}

/** Test no-reverter case. */
AFL_TEST("game.v3.UndoInformation:no-planet", a)
{
    TestHarness h;
    prepare(h);
    game::v3::Reverter& rev = prepareReverter(h);

    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID + 1 /* nonexistant planet */);

    a.checkEqual("01. getNumTorpedoesAllowedToSell", testee.getNumTorpedoesAllowedToSell(0), 0);
    a.checkEqual("02. getNumFightersAllowedToSell",  testee.getNumFightersAllowedToSell(), 0);
    a.checkEqual("03. getSuppliesAllowedToBuy",      testee.getSuppliesAllowedToBuy(), 0);
    a.checkEqual("04. min HullTech",                 testee.getMinTechLevel(game::HullTech), 1);
}

/** Test initialized (but unchanged) case. */
AFL_TEST("game.v3.UndoInformation:unchanged", a)
{
    TestHarness h;
    prepare(h);
    game::v3::Reverter& rev = prepareReverter(h);

    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    a.checkEqual("01. getNumTorpedoesAllowedToSell", testee.getNumTorpedoesAllowedToSell(0), 0);
    a.checkEqual("02. getNumFightersAllowedToSell",  testee.getNumFightersAllowedToSell(), 0);
    a.checkEqual("03. getSuppliesAllowedToBuy",      testee.getSuppliesAllowedToBuy(), 0);
    a.checkEqual("04. min HullTech",                 testee.getMinTechLevel(game::HullTech), 1);
}

/** Test standard supply sale case. */
AFL_TEST("game.v3.UndoInformation:supply-sale", a)
{
    TestHarness h;
    prepare(h);

    // Give planet 200S, 500$
    h.planet.setCargo(game::Element::Supplies, 200);
    h.planet.setCargo(game::Element::Money, 500);

    // Save that as starting state and sell 100 supplies
    game::v3::Reverter& rev = prepareReverter(h);
    h.planet.setCargo(game::Element::Supplies, 100);
    h.planet.setCargo(game::Element::Money, 600);

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);
    a.checkEqual("01. getSuppliesAllowedToBuy", testee.getSuppliesAllowedToBuy(), 100);
    a.checkEqual("02. getSuppliesAllowedToBuy", rev.getSuppliesAllowedToBuy(PLANET_ID), 100);
}

/** Test torpedo upgrade. */
AFL_TEST("game.v3.UndoInformation:torp-tech-upgrade", a)
{
    TestHarness h;
    prepare(h);

    // Give planet 200S, 500$
    h.planet.setCargo(game::Element::Supplies, 200);
    h.planet.setCargo(game::Element::Money, 500);
    h.planet.setCargo(game::Element::fromTorpedoType(3), 5);

    // Save that as starting state. Buy 10 torps, thereby upgrading tech.
    // The torpedo is tech 3.
    game::v3::Reverter& rev = prepareReverter(h);
    h.planet.setBaseTechLevel(game::TorpedoTech, 4);
    h.planet.setBaseTechLevel(game::BeamTech, 4);
    h.planet.setCargo(game::Element::fromTorpedoType(3), 10);

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    // We can downgrade torpedo tech up to 3, that's what we built
    a.checkEqual("01. min TorpedoTech", testee.getMinTechLevel(game::TorpedoTech), 3);
    a.checkEqual("02. min TorpedoTech", rev.getMinTechLevel(PLANET_ID, game::TorpedoTech).orElse(-1), 3);

    // We can downgrade beam tech up to 1, nothing has been built
    a.checkEqual("11. min BeamTech", testee.getMinTechLevel(game::BeamTech), 1);
    a.checkEqual("12. min BeamTech", rev.getMinTechLevel(PLANET_ID, game::BeamTech).orElse(-1), 1);

    // We can sell 5 torpedoes
    a.checkEqual("21. getNumTorpedoesAllowedToSell", testee.getNumTorpedoesAllowedToSell(3), 5);
    a.checkEqual("22. getNumTorpedoesAllowedToSell", rev.getNumTorpedoesAllowedToSell(PLANET_ID, 3), 5);
}

/** Test torpedo upgrade with a ship. */
AFL_TEST("game.v3.UndoInformation:torp-tech-upgrade:ship", a)
{
    TestHarness h;
    prepare(h);

    // Give planet 200S, 500$
    h.planet.setCargo(game::Element::Supplies, 200);
    h.planet.setCargo(game::Element::Money, 500);
    h.planet.setCargo(game::Element::fromTorpedoType(3), 5);

    // Save that as starting state.
    game::v3::Reverter& rev = prepareReverter(h);

    // Add a ship
    game::map::Ship& ship = prepareShip(a, h, 100, OWNER);

    // Buy 3 torpedoes.
    ship.setAmmo(ship.getAmmo().orElse(0) + 3);
    h.planet.setBaseTechLevel(game::TorpedoTech, 4);
    h.planet.setBaseTechLevel(game::BeamTech, 4);

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    // We can downgrade torpedo tech up to 3, that's what we built on the ship
    a.checkEqual("01. min TorpedoTech", testee.getMinTechLevel(game::TorpedoTech), 3);
    a.checkEqual("02. min TorpedoTech", rev.getMinTechLevel(PLANET_ID, game::TorpedoTech).orElse(-1), 3);

    // We can downgrade beam tech up to 1, nothing has been built
    a.checkEqual("11. min BeamTech", testee.getMinTechLevel(game::BeamTech), 1);
    a.checkEqual("12. min BeamTech", rev.getMinTechLevel(PLANET_ID, game::BeamTech).orElse(-1), 1);

    // We can sell 3 torpedoes (those on the ship)
    a.checkEqual("21. getNumTorpedoesAllowedToSell", testee.getNumTorpedoesAllowedToSell(3), 3);
    a.checkEqual("22. getNumTorpedoesAllowedToSell", rev.getNumTorpedoesAllowedToSell(PLANET_ID, 3), 3);
}

/** Test supply sale, ship transfer (#362). */
AFL_TEST("game.v3.UndoInformation:supply-sale-transfer", a)
{
    TestHarness h;
    prepare(h);

    // Give planet 200S, 0$ and save as starting state
    h.planet.setCargo(game::Element::Supplies, 200);
    h.planet.setCargo(game::Element::Money, 0);
    game::v3::Reverter& rev = prepareReverter(h);

    // Create two ships
    game::map::Ship& myShip = prepareShip(a, h, 100, OWNER);
    /*game::map::Ship& theirShip =*/ prepareShip(a, h, 300, OWNER+1);

    // Move supplies into cargo transporter
    myShip.setTransporterTargetId(myShip.TransferTransporter, 300);
    myShip.setTransporterCargo(myShip.TransferTransporter, game::Element::Supplies, 200);
    myShip.setCargo(game::Element::Money, myShip.getCargo(game::Element::Money).orElse(0) - 200);
    h.planet.setCargo(game::Element::Money, 200);
    h.planet.setCargo(game::Element::Supplies, 0);
    a.check("01. isTransporterActive", myShip.isTransporterActive(myShip.TransferTransporter));

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    // We did not sell any supplies, so we cannot buy any!
    a.checkEqual("11. getSuppliesAllowedToBuy", testee.getSuppliesAllowedToBuy(), 0);
    a.checkEqual("12. getSuppliesAllowedToBuy", rev.getSuppliesAllowedToBuy(PLANET_ID), 0);
}
