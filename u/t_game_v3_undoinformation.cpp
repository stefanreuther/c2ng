/**
  *  \file u/t_game_v3_undoinformation.cpp
  *  \brief Test for game::v3::UndoInformation
  */

#include "game/v3/undoinformation.hpp"

#include "t_game_v3.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/v3/reverter.hpp"
#include "afl/charset/utf8charset.hpp"

namespace {
    const int PLANET_ID = 92;
    const int OWNER = 3;
    const int TURN_NR = 92;
    const int X = 1111;
    const int Y = 2222;

    struct TestHarness {
        game::map::Universe univ;
        game::map::Planet& planet;
        afl::base::Ref<game::spec::ShipList> shipList;
        afl::base::Ref<game::Root> root;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::config::HostConfiguration& config;

        TestHarness()
            : univ(),
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
        h.planet.internalCheck(game::map::Configuration(), tx, log);
        h.planet.combinedCheck2(h.univ, game::PlayerSet_t(OWNER), TURN_NR);
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
        game::v3::Reverter* pRev = new game::v3::Reverter(h.univ, h.session);
        h.univ.setNewReverter(pRev);

        game::map::BaseData bd;
        game::map::PlanetData pd;
        h.planet.getCurrentBaseData(bd);
        h.planet.getCurrentPlanetData(pd);
        pRev->addBaseData(PLANET_ID, bd);
        pRev->addPlanetData(PLANET_ID, pd);

        return *pRev;
    }

    game::map::Ship& prepareShip(TestHarness& h, int id, int owner)
    {
        game::map::ShipData sd(id);
        sd.owner = owner;
        sd.x = X;
        sd.y = Y;
        sd.engineType = 1;
        sd.beamType = 4;
        sd.numBeams = 5;
        sd.numBays = 0;
        sd.launcherType = 3;
        sd.ammo = 100;
        sd.numLaunchers = 8;
        sd.supplies = 1000;
        sd.money = 1000;

        game::map::Ship* pShip = h.univ.ships().create(id);
        TS_ASSERT(pShip != 0);
        pShip->addCurrentShipData(sd, game::PlayerSet_t(OWNER));

        game::v3::Reverter* pRev = dynamic_cast<game::v3::Reverter*>(h.univ.getReverter());
        TS_ASSERT(pRev != 0);
        pRev->addShipData(id, sd);

        pShip->internalCheck();
        pShip->combinedCheck1(h.univ, TURN_NR);
        pShip->setPlayability(game::map::Object::Playable);

        return *pShip;
    }
}

/** Test empty (uninitialized) case. */
void
TestGameV3UndoInformation::testEmpty()
{
    game::v3::UndoInformation testee;
    TS_ASSERT_EQUALS(testee.getNumTorpedoesAllowedToSell(0), 0);
    TS_ASSERT_EQUALS(testee.getNumFightersAllowedToSell(), 0);
    TS_ASSERT_EQUALS(testee.getSuppliesAllowedToBuy(), 0);
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::HullTech), 1);
}

/** Test no-reverter case. */
void
TestGameV3UndoInformation::testNoPlanet()
{
    TestHarness h;
    prepare(h);
    game::v3::Reverter& rev = prepareReverter(h);

    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID + 1 /* nonexistant planet */);

    TS_ASSERT_EQUALS(testee.getNumTorpedoesAllowedToSell(0), 0);
    TS_ASSERT_EQUALS(testee.getNumFightersAllowedToSell(), 0);
    TS_ASSERT_EQUALS(testee.getSuppliesAllowedToBuy(), 0);
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::HullTech), 1);
}

/** Test initialized (but unchanged) case. */
void
TestGameV3UndoInformation::testInit()
{
    TestHarness h;
    prepare(h);
    game::v3::Reverter& rev = prepareReverter(h);

    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    TS_ASSERT_EQUALS(testee.getNumTorpedoesAllowedToSell(0), 0);
    TS_ASSERT_EQUALS(testee.getNumFightersAllowedToSell(), 0);
    TS_ASSERT_EQUALS(testee.getSuppliesAllowedToBuy(), 0);
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::HullTech), 1);
}

/** Test standard supply sale case. */
void
TestGameV3UndoInformation::testSupplySale()
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
    TS_ASSERT_EQUALS(testee.getSuppliesAllowedToBuy(), 100);
}

/** Test torpedo upgrade. */
void
TestGameV3UndoInformation::testTorpedoUpgrade()
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
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::TorpedoTech), 3);

    // We can downgrade beam tech up to 1, nothing has been built
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::BeamTech), 1);

    // We can sell 5 torpedoes
    TS_ASSERT_EQUALS(testee.getNumTorpedoesAllowedToSell(3), 5);
}

/** Test torpedo upgrade with a ship. */
void
TestGameV3UndoInformation::testTorpedoShip()
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
    game::map::Ship& ship = prepareShip(h, 100, OWNER);

    // Buy 3 torpedoes.
    ship.setAmmo(ship.getAmmo().orElse(0) + 3);
    h.planet.setBaseTechLevel(game::TorpedoTech, 4);
    h.planet.setBaseTechLevel(game::BeamTech, 4);

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    // We can downgrade torpedo tech up to 3, that's what we built on the ship
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::TorpedoTech), 3);

    // We can downgrade beam tech up to 1, nothing has been built
    TS_ASSERT_EQUALS(testee.getMinTechLevel(game::BeamTech), 1);

    // We can sell 3 torpedoes (those on the ship)
    TS_ASSERT_EQUALS(testee.getNumTorpedoesAllowedToSell(3), 3);
}

/** Test supply sale, ship transfer (#362). */
void
TestGameV3UndoInformation::testSupplyShip()
{
    TestHarness h;
    prepare(h);

    // Give planet 200S, 0$ and save as starting state
    h.planet.setCargo(game::Element::Supplies, 200);
    h.planet.setCargo(game::Element::Money, 0);
    game::v3::Reverter& rev = prepareReverter(h);

    // Create two ships
    game::map::Ship& myShip = prepareShip(h, 100, OWNER);
    game::map::Ship& theirShip = prepareShip(h, 300, OWNER+1);

    // Move supplies into cargo transporter
    myShip.setTransporterTargetId(myShip.TransferTransporter, 300);
    myShip.setTransporterCargo(myShip.TransferTransporter, game::Element::Supplies, 200);
    myShip.setCargo(game::Element::Money, myShip.getCargo(game::Element::Money).orElse(0) - 200);
    h.planet.setCargo(game::Element::Money, 200);
    h.planet.setCargo(game::Element::Supplies, 0);
    TS_ASSERT(myShip.isTransporterActive(myShip.TransferTransporter));

    // Test
    game::v3::UndoInformation testee;
    testee.set(h.univ, *h.shipList, h.config, rev, PLANET_ID);

    // We did not sell any supplies, so we cannot buy any!
    TS_ASSERT_EQUALS(testee.getSuppliesAllowedToBuy(), 0);
}

