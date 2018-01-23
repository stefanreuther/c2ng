/**
  *  \file u/t_game_actions_buildship.cpp
  *  \brief Test for game::actions::BuildShip
  */

#include "game/actions/buildship.hpp"

#include "t_game_actions.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/exception.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/map/planetstorage.hpp"

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
              root(*new game::Root(afl::io::InternalDirectory::create("spec dir"),
                                   afl::io::InternalDirectory::create("game dir"),
                                   *new game::test::SpecificationLoader(),
                                   game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 47)),
                                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()))),
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
            hh->setNumEngines(3);
            hh->setMaxBeams(4);
            hh->setMaxLaunchers(5);
            hh->cost() = game::spec::Cost::fromString("10T 15$");
        }
        // - Engine #1-#9
        for (int i = 1; i <= 9; ++i) {
            game::spec::Engine* e = h.shipList->engines().create(i);
            e->setTechLevel(i);
            e->cost() = game::spec::Cost::fromString("1TDM 1$") * i;
        }
        // - Beam #1-#10
        for (int i = 1; i <= 10; ++i) {
            game::spec::Beam* b = h.shipList->beams().create(i);
            b->setTechLevel(i);
            b->cost() = game::spec::Cost::fromString("1M") * i;
        }
        // - Launcher #1-#10
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = h.shipList->launchers().create(i);
            tl->setTechLevel(i);
            tl->cost() = game::spec::Cost::fromString("1M 10S") * i;
        }
        // - Hull association
        h.shipList->hullAssignments().add(OWNER, 12, 9);
    }
}


/** Test failure.
    If the planet has no base, constructing the action must fail. */
void
TestGameActionsBuildShip::testError()
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
    TS_ASSERT_THROWS((game::actions::BuildShip(h.planet, container, *h.shipList, *h.root)), game::Exception);
}

/** Test success, simple case. */
void
TestGameActionsBuildShip::testSuccess()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip:
    // Must have tech 1 components, hull #9 (slot #12).
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), 9);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 5);

    // Verify cost:
    //  Hull            10T          15$
    //  Tech upgrade:               100$
    //  Engines (3):     3T  3D  3M   3$
    //  Beams (4):               4M
    //  Launchers (5):           5M      50S
    // Total:           13T  3D 12M 118$ 50S
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Commit and verify result
    a.commit();
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), 12);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech,   12).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
}

/** Test building a ship with no beams. */
void
TestGameActionsBuildShip::testNoBeams()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Set number of beams to zero
    game::ShipBuildOrder sbo = a.getBuildOrder();
    sbo.setNumBeams(0);
    a.setBuildOrder(sbo);

    // Verify: 4M less
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 8M 50S 118$");

    // Commit and verify result
    a.commit();
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), 12);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 0);      // <- also set to 0 by normalisation
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 0);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);
}

/** Test building with initial tech levels.
    This must select parts other than tech 1. */
void
TestGameActionsBuildShip::testInitialTech()
{
    TestHarness h;
    prepare(h);

    // Set tech levels
    h.planet.setBaseTechLevel(game::HullTech, 2);
    h.planet.setBaseTechLevel(game::EngineTech, 3);
    h.planet.setBaseTechLevel(game::BeamTech, 4);
    h.planet.setBaseTechLevel(game::TorpedoTech, 5);

    // Make action
    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip:
    // Must have selected components according to tech levels
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), 9);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 3);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 5);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 5);

    // Verify cost:
    //  Hull            10T          15$
    //  Engines (3):     9T  9D  9M   9$
    //  Beams (4):              16M
    //  Launchers (5):          25M      250S
    // Total:           19T  9D 50M  24$ 250S
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "19T 9D 50M 250S 24$");
}

/** Test building with included tech upgrade. */
void
TestGameActionsBuildShip::testTechUpgrade()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Set component types
    game::ShipBuildOrder sbo = a.getBuildOrder();
    sbo.setEngineType(2);
    sbo.setBeamType(3);
    sbo.setLauncherType(4);
    a.setBuildOrder(sbo);

    // Verify cost:
    //  Hull            10T           15$
    //    Upgrade:                   100$
    //  Engines (3):     6T  6D  6M    6$
    //    Upgrade:                   100$
    //  Beams (4):              12M
    //    Upgrade:                   300$
    //  Launchers (5):          20M      200S
    //    Upgrade:                   600$
    // Total:           16T  6D 38M 1121$ 200S
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "16T 6D 38M 200S 1121$");

    // Commit and verify result
    a.commit();
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), 12);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech,   12).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  2).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    3).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 4).orElse(0), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::EngineTech).orElse(0), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::BeamTech).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 4);
}

/** Test tech upgrade failure.
    If we attempt to build a component with a disallowed tech level, commit must fail. */
void
TestGameActionsBuildShip::testTechUpgradeFail()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Set component types: try tech 9, but our key only allows tech 5.
    game::ShipBuildOrder sbo = a.getBuildOrder();
    sbo.setEngineType(9);
    a.setBuildOrder(sbo);

    // Cost is valid, but transaction is not
    TS_ASSERT(container.isValid());
    TS_ASSERT(a.costAction().isValid());
    TS_ASSERT_EQUALS(a.getStatus(), a.DisallowedTech);

    // Commit fails
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

/** Test using parts from storage.
    If we have plenty parts, cost must be computed as zero. */
void
TestGameActionsBuildShip::testUseParts()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech,   12, 10);
    h.planet.setBaseStorage(game::EngineTech,  1, 10);
    h.planet.setBaseStorage(game::BeamTech,    1, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 10);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Initial state: do not use parts from storage
    TS_ASSERT_EQUALS(a.isUsePartsFromStorage(), false);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Set to use parts:
    a.setUsePartsFromStorage(true);
    TS_ASSERT(a.costAction().getCost().isZero());
}

/** Test using parts from storage.
    If we have some parts, only the missing ones are built. */
void
TestGameActionsBuildShip::testUsePartsPartial()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech,   12, 1);
    h.planet.setBaseStorage(game::EngineTech,  1, 1);
    h.planet.setBaseStorage(game::BeamTech,    1, 1);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 1);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Initial state: do not use parts from storage
    TS_ASSERT_EQUALS(a.isUsePartsFromStorage(), false);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Set to use parts. New costs:
    //  Engines (2):     2T  2D  2M   2$
    //  Beams (3):               3M
    //  Launchers (4):           4M      40S
    // Total:            2T  2D  9M   2$ 40S
    a.setUsePartsFromStorage(true);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "2T 2D 9M 40S 2$");

    // Commit and verify result
    a.commit();
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), 12);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech,   12).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    // No change to hull tech, we're re-using the hull
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 1);
}

/** Test pre-existing build order.
    That one must be re-used and completed. */
void
TestGameActionsBuildShip::testPreexistingOrder()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech,   12, 10);
    h.planet.setBaseStorage(game::EngineTech,  2, 10);
    h.planet.setBaseStorage(game::BeamTech,    4, 10);

    // Set tech level
    h.planet.setBaseTechLevel(game::TorpedoTech, 7);

    // Set build order
    {
        game::ShipBuildOrder sbo;
        sbo.setHullIndex(12);
        sbo.setEngineType(2);
        sbo.setBeamType(4);
        sbo.setNumBeams(1);
        sbo.setLauncherType(0);
        sbo.setNumLaunchers(0);
        h.planet.setBaseBuildOrder(sbo);
    }

    // Create action
    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root);

    // Verify initial order
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), 9);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 2);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 7);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 0);
    TS_ASSERT(a.isUsePartsFromStorage());
}

