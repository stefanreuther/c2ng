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
#include "afl/charset/utf8charset.hpp"

namespace {
    const int X = 1234;
    const int Y = 2345;
    const int OWNER = 4;
    const int TURN_NR = 12;
    const int PLANET_ID = 363;

    const int HULL_TYPE = 9;
    const int HULL_SLOT = 12;

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
        h.planet.internalCheck(game::map::Configuration(), tx, log);
        h.planet.combinedCheck2(h.univ, game::PlayerSet_t(OWNER), TURN_NR);
        h.planet.setPlayability(game::map::Object::Playable);

        // Define a number of components
        // - Hull #9
        {
            game::spec::Hull* hh = h.shipList->hulls().create(HULL_TYPE);
            hh->setTechLevel(2);
            hh->setNumEngines(3);
            hh->setMaxBeams(4);
            hh->setMaxLaunchers(5);
            hh->setName("HH");
            hh->cost() = game::spec::Cost::fromString("10T 15$");
        }
        // - Engine #1-#9
        for (int i = 1; i <= 9; ++i) {
            game::spec::Engine* e = h.shipList->engines().create(i);
            e->setTechLevel(i);
            e->setName("E");
            e->cost() = game::spec::Cost::fromString("1TDM 1$") * i;
        }
        // - Beam #1-#10
        for (int i = 1; i <= 10; ++i) {
            game::spec::Beam* b = h.shipList->beams().create(i);
            b->setTechLevel(i);
            b->setName("B");
            b->cost() = game::spec::Cost::fromString("1M") * i;
        }
        // - Launcher #1-#10
        for (int i = 1; i <= 10; ++i) {
            game::spec::TorpedoLauncher* tl = h.shipList->launchers().create(i);
            tl->setTechLevel(i);
            tl->setName("L");
            tl->cost() = game::spec::Cost::fromString("1M 10S") * i;
        }
        // - Hull association
        h.shipList->hullAssignments().add(OWNER, HULL_SLOT, HULL_TYPE);
    }

    void addExtraHull(TestHarness& h)
    {
        // - Hull #11
        {
            game::spec::Hull* hh = h.shipList->hulls().create(11);
            hh->setTechLevel(5);
            hh->setNumEngines(2);
            hh->setMaxBeams(3);
            hh->setMaxLaunchers(10);
            hh->cost() = game::spec::Cost::fromString("20T");
        }
        h.shipList->hullAssignments().add(OWNER, 13, 11);
    }
}


/** Test failure.
    A: create planet with no base.
    E: creation of BuildShip action fails with exception. */
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
    h.planet.combinedCheck2(h.univ, game::PlayerSet_t(7), TURN_NR);
    h.planet.setPlayability(game::map::Object::Playable);

    game::test::CargoContainer container;
    TS_ASSERT_THROWS((game::actions::BuildShip(h.planet, container, *h.shipList, *h.root, h.tx)), game::Exception);
}

/** Test success, simple case.
    A: create action.
    E: correct initial build order chosen; can be committed correctly. */
void
TestGameActionsBuildShip::testSuccess()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Check initial build order selected by BuildShip:
    // Must have tech 1 components, hull #9 (slot #12).
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
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
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
}

/** Test building a ship with no beams.
    A: create action. Set number of beams to zero. Commit.
    E: beam type set to zero as well. */
void
TestGameActionsBuildShip::testNoBeams()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Set number of beams to zero
    game::ShipBuildOrder sbo = a.getBuildOrder();
    sbo.setNumBeams(0);
    a.setBuildOrder(sbo);

    // Verify: 4M less
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 8M 50S 118$");

    // Commit and verify result
    a.commit();
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 0);      // <- also set to 0 by normalisation
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 0);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);
}

/** Test building with initial tech levels.
    A: create action on planet with tech levels other than 1.
    E: initial build order chooses higher-tech components. */
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
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Check initial build order selected by BuildShip:
    // Must have selected components according to tech levels
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
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

/** Test building with included tech upgrade.
    A: select components with tech levels higher than base has.
    E: tech levels included in cost. Committing increases tech. */
void
TestGameActionsBuildShip::testTechUpgrade()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

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
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  2).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    3).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 4).orElse(0), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::EngineTech).orElse(0), 2);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::BeamTech).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 4);
}

/** Test tech upgrade failure.
    A: select component that requires disallowed tech level.
    E: status reported as failure. Commit fails with exception. */
void
TestGameActionsBuildShip::testTechUpgradeFail()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

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
    A: place parts in storage. Enable isUsePartsFromStorage.
    E: cost reported as zero. */
void
TestGameActionsBuildShip::testUseParts()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 10);
    h.planet.setBaseStorage(game::EngineTech,  1, 10);
    h.planet.setBaseStorage(game::BeamTech,    1, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 10);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Initial state: do not use parts from storage
    TS_ASSERT_EQUALS(a.isUsePartsFromStorage(), false);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Set to use parts:
    a.setUsePartsFromStorage(true);
    TS_ASSERT(a.costAction().getCost().isZero());
}

/** Test using parts from storage.
    A: place some parts in storage. Enable isUsePartsFromStorage.
    E: cost reports only the missing parts. */
void
TestGameActionsBuildShip::testUsePartsPartial()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 1);
    h.planet.setBaseStorage(game::EngineTech,  1, 1);
    h.planet.setBaseStorage(game::BeamTech,    1, 1);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 1);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

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
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    TS_ASSERT_EQUALS(h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    // No change to hull tech, we're re-using the hull
    TS_ASSERT_EQUALS(h.planet.getBaseTechLevel(game::HullTech).orElse(0), 1);
}

/** Test pre-existing build order.
    A: create BuildShip action on planet with pre-existing build order.
    E: build order correctly loaded as default; unused components correctly selected */
void
TestGameActionsBuildShip::testPreexistingOrder()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 10);
    h.planet.setBaseStorage(game::EngineTech,  2, 10);
    h.planet.setBaseStorage(game::BeamTech,    4, 10);

    // Set tech level
    h.planet.setBaseTechLevel(game::TorpedoTech, 7);

    // Set build order
    {
        game::ShipBuildOrder sbo;
        sbo.setHullIndex(HULL_SLOT);
        sbo.setEngineType(2);
        sbo.setBeamType(4);
        sbo.setNumBeams(1);
        sbo.setLauncherType(0);
        sbo.setNumLaunchers(0);
        h.planet.setBaseBuildOrder(sbo);
    }

    // Create action
    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Verify initial order
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 2);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 7);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 0);
    TS_ASSERT(a.isUsePartsFromStorage());
    TS_ASSERT(!a.isChange());

    // Change must be registered as such
    a.setPart(game::BeamTech, 2);
    TS_ASSERT(a.isChange());
}

/** Test foreign ship.
    A: attempt to build a ship we cannot build.
    E: building must not succeed. */
void
TestGameActionsBuildShip::testForeignShip()
{
    TestHarness h;
    prepare(h);

    // Create another hull that is not linked in hullAssignments
    game::spec::Hull* hh = h.shipList->hulls().create(10);
    hh->setName("EX");
    hh->setTechLevel(2);
    hh->setNumEngines(3);
    hh->setMaxBeams(4);
    hh->setMaxLaunchers(5);
    hh->cost() = game::spec::Cost::fromString("100T 150$");

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Check initial build order selected by BuildShip: must have hull 9
    game::ShipBuildOrder order = a.getBuildOrder();
    TS_ASSERT_EQUALS(order.getHullIndex(), HULL_TYPE);

    // Change to hull 10
    order.setHullIndex(10);
    a.setBuildOrder(order);

    // Verify cost:
    //  Hull           100T         150$
    //  Tech upgrade:               100$
    //  Engines (3):     3T  3D  3M   3$
    //  Beams (4):               4M
    //  Launchers (5):           5M      50S
    // Total:          103T  3D 12M 253$ 50S
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "103T 3D 12M 50S 253$");

    // Verify cost summary
    game::spec::CostSummary summary;
    a.getCostSummary(summary);

    TS_ASSERT_EQUALS(summary.getNumItems(), 5U);
    const game::spec::CostSummary::Item* p;

    p = summary.get(0);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "Hull tech upgrade");
    TS_ASSERT_EQUALS(p->cost.toCargoSpecString(), "100$");

    p = summary.get(1);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "EX");
    TS_ASSERT_EQUALS(p->cost.toCargoSpecString(), "100T 150$");

    // Commit must fail
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::ForeignHull);
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

/** Test tech upgrade disabled.
    A: select build order that requires tech upgrade. Disable tech upgrades.
    E: building must not succeed. */
void
TestGameActionsBuildShip::testTechDisabled()
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Set component types (same as testTechUpgrade)
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
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::Success);

    // Disable
    a.setUseTechUpgrade(false);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "16T 6D 38M 200S 21$");
    TS_ASSERT_EQUALS(a.getStatus(), game::actions::BaseBuildAction::DisabledTech);

    // Commit must fail
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

/** Test modification of build order.
    A: create BuildShip action. Use partial modifiers (setPart etc.).
    E: modifications correctly executed */
void
TestGameActionsBuildShip::testModify()
{
    TestHarness h;
    prepare(h);
    addExtraHull(h);

    // Make action
    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Check initial build order selected by BuildShip:
    // Must have tech 1 components, hull #9 (slot #12).
    // [same as testSuccess]
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 1);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 5);

    // Modify components
    a.setPart(game::BeamTech, 4);
    a.setPart(game::TorpedoTech, 5);
    a.setNumParts(game::actions::BuildShip::BeamWeapon, 2);
    a.setNumParts(game::actions::BuildShip::TorpedoWeapon, 1);
    a.setPart(game::EngineTech, 6);

    // Verify
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 6);
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 2);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 5);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 1);

    // Maximize counts
    a.addParts(game::actions::BuildShip::BeamWeapon, 100);
    a.addParts(game::actions::BuildShip::TorpedoWeapon, 100);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 4);
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 5);

    // Change hull
    a.setPart(game::HullTech, 11);
    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), 11);
    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 6);   // unchanged
    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 4);     // unchanged
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumBeams(), 3);
    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 5); // unchanged
    TS_ASSERT_EQUALS(a.getBuildOrder().getNumLaunchers(), 10);
}

/** Test use of invalid Ids.
    A: set invalid Id using setPart().
    E: must throw when trying to set an invalid component; must NOT throw when later accessing something unrelated */
void
TestGameActionsBuildShip::testBadId()
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    {
        game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);
        TS_ASSERT_THROWS(a.setPart(game::HullTech, 77), std::exception);
        TS_ASSERT_THROWS_NOTHING(a.setPart(game::BeamTech, 9));
    }

    {
        game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);
        TS_ASSERT_THROWS(a.setPart(game::EngineTech, 77), std::exception);
        TS_ASSERT_THROWS_NOTHING(a.setPart(game::BeamTech, 9));
    }

    {
        game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);
        TS_ASSERT_THROWS(a.setPart(game::BeamTech, 77), std::exception);
        TS_ASSERT_THROWS_NOTHING(a.setPart(game::EngineTech, 9));
    }

    {
        game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);
        TS_ASSERT_THROWS(a.setPart(game::TorpedoTech, 77), std::exception);
        TS_ASSERT_THROWS_NOTHING(a.setPart(game::EngineTech, 9));
    }
}

/** Test bad precondition: hull.
    A: create planet with invalid hull slot in its build order. Create BuildShip action.
    E: action created successfully, valid hull chosen */
void
TestGameActionsBuildShip::testBadHull()
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(30);          // Invalid index
    o.setEngineType(9);
    h.planet.setBaseBuildOrder(o);
    
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    TS_ASSERT_EQUALS(a.getBuildOrder().getHullIndex(), HULL_TYPE);
}

/** Test bad precondition: engine.
    A: create planet with invalid engine in its build order. Create BuildShip action.
    E: action created successfully, valid engine chosen */
void
TestGameActionsBuildShip::testBadEngine()
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(19);       // Invalid type
    h.planet.setBaseBuildOrder(o);
    
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    TS_ASSERT_EQUALS(a.getBuildOrder().getEngineType(), 1);
}

/** Test bad precondition: beam.
    A: create planet with invalid beam in its build order. Create BuildShip action.
    E: action created successfully, valid beam chosen */
void
TestGameActionsBuildShip::testBadBeam()
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(9);
    o.setNumBeams(1);
    o.setBeamType(20);        // Invalid type
    h.planet.setBaseBuildOrder(o);
    
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    TS_ASSERT_EQUALS(a.getBuildOrder().getBeamType(), 1);
}

/** Test bad precondition: torpedo launcher.
    A: create planet with invalid torpedo launcherin its build order. Create BuildShip action.
    E: action created successfully, valid launcher chosen */
void
TestGameActionsBuildShip::testBadLauncher()
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(9);
    o.setNumLaunchers(1);
    o.setLauncherType(20);        // Invalid type
    h.planet.setBaseBuildOrder(o);
    
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    TS_ASSERT_EQUALS(a.getBuildOrder().getLauncherType(), 1);
}

/** Test cost summary.
    A: create an interesting build order (same as testUsePartsPartial).
    E: verify correct details generated */
void
TestGameActionsBuildShip::testCostSummary()
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 1);
    h.planet.setBaseStorage(game::EngineTech,  1, 1);
    h.planet.setBaseStorage(game::BeamTech,    1, 1);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 1);

    game::test::CargoContainer container;
    game::actions::BuildShip a(h.planet, container, *h.shipList, *h.root, h.tx);

    // Initial state: do not use parts from storage
    a.setUsePartsFromStorage(true);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "2T 2D 9M 40S 2$");

    // Verify cost summary
    //   1x From storage: hull
    //   2x Engine
    //   1x From storage: engine
    //   3x Beam
    //   1x From storage: beam
    //   4x Launcher
    //   1x From storage: launcher
    game::spec::CostSummary summary;
    a.getCostSummary(summary);

    TS_ASSERT_EQUALS(summary.getNumItems(), 7U);

    const game::spec::CostSummary::Item* p;
    p = summary.get(0);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "From storage: HH");
    TS_ASSERT_EQUALS(p->cost.isZero(), true);

    p = summary.get(1);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 2);
    TS_ASSERT_EQUALS(p->name, "E");
    TS_ASSERT_EQUALS(p->cost.toCargoSpecString(), "2TDM 2$");

    p = summary.get(2);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "From storage: E");
    TS_ASSERT_EQUALS(p->cost.isZero(), true);

    p = summary.get(3);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 3);
    TS_ASSERT_EQUALS(p->name, "B");
    TS_ASSERT_EQUALS(p->cost.toCargoSpecString(), "3M");

    p = summary.get(4);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "From storage: B");
    TS_ASSERT_EQUALS(p->cost.isZero(), true);

    p = summary.get(5);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 4);
    TS_ASSERT_EQUALS(p->name, "L");
    TS_ASSERT_EQUALS(p->cost.toCargoSpecString(), "4M 40S");

    p = summary.get(6);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->multiplier, 1);
    TS_ASSERT_EQUALS(p->name, "From storage: L");
    TS_ASSERT_EQUALS(p->cost.isZero(), true);
}
