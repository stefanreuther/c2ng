/**
  *  \file u/t_game_actions_techupgrade.cpp
  *  \brief Test for game::actions::TechUpgrade
  */

#include "game/actions/techupgrade.hpp"

#include "t_game_actions.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/v3/reverter.hpp"

namespace {
    const int X = 1234;
    const int Y = 2345;
    const int OWNER = 4;
    const int TURN_NR = 12;
    const int PLANET_ID = 363;

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
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 1;
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

    void prepareReverter(TestHarness& h)
    {
        game::v3::Reverter* pRev = new game::v3::Reverter(h.univ, h.session);
        h.univ.setNewReverter(pRev);

        game::map::BaseData bd;
        game::map::PlanetData pd;
        h.planet.getCurrentBaseData(bd);
        h.planet.getCurrentPlanetData(pd);
        pRev->addBaseData(PLANET_ID, bd);
        pRev->addPlanetData(PLANET_ID, pd);
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

/** Test failure.
    If the planet has no base, constructing the action must fail. */
void
TestGameActionsTechUpgrade::testFail()
{
    TestHarness h;
    afl::sys::Log log;

    // Define planet without base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), h.tx, log);
    h.planet.combinedCheck2(h.univ, game::PlayerSet_t(7), 12);
    h.planet.setPlayability(game::map::Object::Playable);

    game::test::CargoContainer container;
    TS_ASSERT_THROWS((game::actions::TechUpgrade(h.planet, container, *h.shipList, *h.root)), game::Exception);
}

/** Test simple success case.
    If the planet has a base, constructing the action must succeed.
    Setting a tech level must update the costs, and be rejected if it is not allowed. */
void
TestGameActionsTechUpgrade::testSimple()
{
    TestHarness h;
    afl::sys::Log log;

    // Define planet with base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.setBaseTechLevel(game::HullTech, 1);
    h.planet.setBaseTechLevel(game::EngineTech, 1);
    h.planet.setBaseTechLevel(game::BeamTech, 1);
    h.planet.setBaseTechLevel(game::TorpedoTech, 1);
    h.planet.internalCheck(game::map::Configuration(), h.tx, log);
    h.planet.combinedCheck2(h.univ, game::PlayerSet_t(7), 12);
    h.planet.setPlayability(game::map::Object::Playable);

    // This must have produced a base
    TS_ASSERT(h.planet.hasBase());

    // Make an action
    game::test::CargoContainer container;
    game::actions::TechUpgrade a(h.planet, container, *h.shipList, *h.root);
    TS_ASSERT(a.isValid());
    TS_ASSERT(a.costAction().getCost().isZero());
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::HullTech), 1);
    TS_ASSERT_EQUALS(a.getMaxTechLevel(game::HullTech), 5);

    // Set invalid (unregistered)
    TS_ASSERT(!a.setTechLevel(game::HullTech, 6));
    TS_ASSERT(a.costAction().getCost().isZero());

    // Set valid tech level
    TS_ASSERT(a.setTechLevel(game::HullTech, 4));
    TS_ASSERT(a.isValid());
    TS_ASSERT(!a.costAction().getCost().isZero());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 600);
    TS_ASSERT_EQUALS(container.getChange(game::Element::Money), -600);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Chance price configuration. This automatically updates.
    h.root->hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(150);
    h.root->hostConfiguration().notifyListeners();
    TS_ASSERT(a.isValid());
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), 900);
    TS_ASSERT_EQUALS(container.getChange(game::Element::Money), -900);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());

    // Verify
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(-99), 4);
}

/** Test revertable.
    When setUndoInformation() is used, the action must allow reverting a build. */
void
TestGameActionsTechUpgrade::testRevertable()
{
    // Prepare
    TestHarness h;
    prepare(h);
    h.planet.setBaseTechLevel(game::BeamTech, 3);
    prepareReverter(h);
    TS_ASSERT(h.planet.hasBase());

    // Upgrade tech
    h.planet.setBaseTechLevel(game::HullTech, 2);
    h.planet.setBaseTechLevel(game::EngineTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade a(h.planet, container, *h.shipList, *h.root);
    a.setUndoInformation(h.univ);

    TS_ASSERT_EQUALS(a.getMinTechLevel(game::HullTech), 1);
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::EngineTech), 1);
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::BeamTech), 3);

    // Set
    a.setTechLevel(game::EngineTech, 1);
    TS_ASSERT_EQUALS(a.costAction().getCost().get(game::spec::Cost::Money), -1000);
    TS_ASSERT_EQUALS(container.getChange(game::Element::Money), 1000);
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);
}

/** Test revertable, with change behind.
    A change done to the universe while the action is active must be reflected to the action,
    not only at the time setUndoInformation() is called. */
void
TestGameActionsTechUpgrade::testRevertableChange()
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    TS_ASSERT(h.planet.hasBase());

    // Upgrade tech
    h.planet.setBaseTechLevel(game::BeamTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade a(h.planet, container, *h.shipList, *h.root);
    a.setUndoInformation(h.univ);
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::BeamTech), 1);

    // Build a beam (tech 4)
    h.planet.setBaseStorage(game::BeamTech, 4, 1);
    h.univ.notifyListeners();

    // Minimum tech is now 4
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::BeamTech), 4);
}

/** Test revertable, with change on ship.
    A change done to the universe while the action is active must be reflected to the action,
    not only at the time setUndoInformation() is called.
    This applies even if the change is to a different object. */
void
TestGameActionsTechUpgrade::testRevertableShip()
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    TS_ASSERT(h.planet.hasBase());

    game::map::Ship& ship = prepareShip(h, 99, OWNER);

    // Upgrade tech
    h.planet.setBaseTechLevel(game::TorpedoTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade a(h.planet, container, *h.shipList, *h.root);
    a.setUndoInformation(h.univ);
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::TorpedoTech), 1);

    // Build a torpedo (tech 3) and place on ship
    ship.setAmmo(ship.getAmmo().orElse(0) + 1);
    h.univ.notifyListeners();

    // Minimum tech is now 3
    TS_ASSERT_EQUALS(a.getMinTechLevel(game::TorpedoTech), 3);
}

/** Test revertable, no change signal.
    A change done to the universe while the action is active must be reflected to the action,
    Commit must not do stupid things when the change does not signal the listener. */
void
TestGameActionsTechUpgrade::testRevertableNoSignal()
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    TS_ASSERT(h.planet.hasBase());

    game::map::Ship& ship = prepareShip(h, 99, OWNER);

    // Upgrade tech
    h.planet.setBaseTechLevel(game::TorpedoTech, 5);
    h.planet.setCargo(game::Element::Money, 0);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade a(h.planet, container, *h.shipList, *h.root);
    a.setUndoInformation(h.univ);

    // Request tech downgrade
    a.setTechLevel(game::TorpedoTech, 1);
    TS_ASSERT_EQUALS(container.getChange(game::Element::Money), 1000);

    // Build a torpedo (tech 3) and place on ship
    ship.setAmmo(ship.getAmmo().orElse(0) + 1);
    // Listener notification could be here: "h.univ.notifyListeners();"

    // Commit. Because minimum tech level is 3, this must only go to 3, with a 700$ refund.
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 3);
    TS_ASSERT_EQUALS(container.getChange(game::Element::Money), 700);
}

