/**
  *  \file test/game/actions/buildshiptest.cpp
  *  \brief Test for game::actions::BuildShip
  */

#include "game/actions/buildship.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

namespace {
    const int X = 1234;
    const int Y = 2345;
    const int OWNER = 4;
    const int TURN_NR = 12;
    const int PLANET_ID = 363;

    const int HULL_TYPE = 9;
    const int HULL_SLOT = 12;

    struct TestHarness {
        game::map::Planet planet;
        afl::base::Ref<game::spec::ShipList> shipList;
        afl::base::Ref<game::Root> root;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        game::config::HostConfiguration& config;

        TestHarness()
            : planet(PLANET_ID),
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
        h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(OWNER), TURN_NR, tx, log);
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
AFL_TEST("game.actions.BuildShip:fail", a)
{
    TestHarness h;
    afl::sys::Log log;

    // Define planet without base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), TURN_NR, h.tx, log);
    h.planet.setPlayability(game::map::Object::Playable);

    game::test::CargoContainer container;
    AFL_CHECK_THROWS(a, (game::actions::BuildShip(h.planet, container, *h.shipList, *h.root)), game::Exception);
}

/** Test success, simple case.
    A: create action.
    E: correct initial build order chosen; can be committed correctly. */
AFL_TEST("game.actions.BuildShip:success", a)
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip:
    // Must have tech 1 components, hull #9 (slot #12).
    a.checkEqual("01 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
    a.checkEqual("02. getEngineType", act.getBuildOrder().getEngineType(), 1);
    a.checkEqual("03. getBeamType", act.getBuildOrder().getBeamType(), 1);
    a.checkEqual("04. getNumBeams", act.getBuildOrder().getNumBeams(), 4);
    a.checkEqual("05. getTorpedoType", act.getBuildOrder().getTorpedoType(), 1);
    a.checkEqual("06. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 5);

    // Verify ShipQuery
    a.checkEqual("11. getHullType", act.getQuery().getHullType(), HULL_TYPE);
    a.checkEqual("12. getOwner", act.getQuery().getOwner(), OWNER);

    // Verify cost:
    //  Hull            10T          15$
    //  Tech upgrade:               100$
    //  Engines (3):     3T  3D  3M   3$
    //  Beams (4):               4M
    //  Launchers (5):           5M      50S
    // Total:           13T  3D 12M 118$ 50S
    a.checkEqual("21. getCost", act.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Commit and verify result
    act.commit();
    a.checkEqual("31. getHullIndex", h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    a.checkEqual("32. getEngineType", h.planet.getBaseBuildOrder().getEngineType(), 1);
    a.checkEqual("33. getBeamType", h.planet.getBaseBuildOrder().getBeamType(), 1);
    a.checkEqual("34. getNumBeams", h.planet.getBaseBuildOrder().getNumBeams(), 4);
    a.checkEqual("35. getTorpedoType", h.planet.getBaseBuildOrder().getTorpedoType(), 1);
    a.checkEqual("36. getNumLaunchers", h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    a.checkEqual("41. hulls", h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    a.checkEqual("42. engines", h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    a.checkEqual("43. beams", h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    a.checkEqual("44. torps", h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    a.checkEqual("51. getBaseTechLevel", h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
}

/** Test building a ship with no beams.
    A: create action. Set number of beams to zero. Commit.
    E: beam type set to zero as well. */
AFL_TEST("game.actions.BuildShip:no-beam", a)
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Set number of beams to zero
    game::ShipBuildOrder sbo = act.getBuildOrder();
    sbo.setNumBeams(0);
    act.setBuildOrder(sbo);

    // Verify: 4M less
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "13T 3D 8M 50S 118$");

    // Commit and verify result
    act.commit();
    a.checkEqual("11. getHullIndex", h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    a.checkEqual("12. getEngineType", h.planet.getBaseBuildOrder().getEngineType(), 1);
    a.checkEqual("13. getBeamType", h.planet.getBaseBuildOrder().getBeamType(), 0);      // <- also set to 0 by normalisation
    a.checkEqual("14. getNumBeams", h.planet.getBaseBuildOrder().getNumBeams(), 0);
    a.checkEqual("15. getTorpedoType", h.planet.getBaseBuildOrder().getTorpedoType(), 1);
    a.checkEqual("16. getNumLaunchers", h.planet.getBaseBuildOrder().getNumLaunchers(), 5);
}

/** Test building with initial tech levels.
    A: create action on planet with tech levels other than 1.
    E: initial build order chooses higher-tech components. */
AFL_TEST("game.actions.BuildShip:initial-tech", a)
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
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip:
    // Must have selected components according to tech levels
    a.checkEqual("01 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
    a.checkEqual("02. getEngineType", act.getBuildOrder().getEngineType(), 3);
    a.checkEqual("03. getBeamType", act.getBuildOrder().getBeamType(), 4);
    a.checkEqual("04. getNumBeams", act.getBuildOrder().getNumBeams(), 4);
    a.checkEqual("05. getTorpedoType", act.getBuildOrder().getTorpedoType(), 5);
    a.checkEqual("06. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 5);

    // Verify cost:
    //  Hull            10T          15$
    //  Engines (3):     9T  9D  9M   9$
    //  Beams (4):              16M
    //  Launchers (5):          25M      250S
    // Total:           19T  9D 50M  24$ 250S
    a.checkEqual("11. getCost", act.costAction().getCost().toCargoSpecString(), "19T 9D 50M 250S 24$");
}

/** Test building with included tech upgrade.
    A: select components with tech levels higher than base has.
    E: tech levels included in cost. Committing increases tech. */
AFL_TEST("game.actions.BuildShip:tech-upgrade", a)
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Set component types
    game::ShipBuildOrder sbo = act.getBuildOrder();
    sbo.setEngineType(2);
    sbo.setBeamType(3);
    sbo.setTorpedoType(4);
    act.setBuildOrder(sbo);

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
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "16T 6D 38M 200S 1121$");

    // Commit and verify result
    act.commit();
    a.checkEqual("11. getHullIndex", h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    a.checkEqual("12. getEngineType", h.planet.getBaseBuildOrder().getEngineType(), 2);
    a.checkEqual("13. getBeamType", h.planet.getBaseBuildOrder().getBeamType(), 3);
    a.checkEqual("14. getNumBeams", h.planet.getBaseBuildOrder().getNumBeams(), 4);
    a.checkEqual("15. getTorpedoType", h.planet.getBaseBuildOrder().getTorpedoType(), 4);
    a.checkEqual("16. getNumLaunchers", h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    a.checkEqual("21. hulls", h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    a.checkEqual("22. engines", h.planet.getBaseStorage(game::EngineTech,  2).orElse(0), 3);
    a.checkEqual("23. beams", h.planet.getBaseStorage(game::BeamTech,    3).orElse(0), 4);
    a.checkEqual("24. torps", h.planet.getBaseStorage(game::TorpedoTech, 4).orElse(0), 5);

    a.checkEqual("31. getBaseTechLevel", h.planet.getBaseTechLevel(game::HullTech).orElse(0), 2);
    a.checkEqual("32. getBaseTechLevel", h.planet.getBaseTechLevel(game::EngineTech).orElse(0), 2);
    a.checkEqual("33. getBaseTechLevel", h.planet.getBaseTechLevel(game::BeamTech).orElse(0), 3);
    a.checkEqual("34. getBaseTechLevel", h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 4);
}

/** Test tech upgrade failure.
    A: select component that requires disallowed tech level.
    E: status reported as failure. Commit fails with exception. */
AFL_TEST("game.actions.BuildShip:fail:disallowed-tech", a)
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Set component types: try tech 9, but our key only allows tech 5.
    game::ShipBuildOrder sbo = act.getBuildOrder();
    sbo.setEngineType(9);
    act.setBuildOrder(sbo);

    // Cost is valid, but transaction is not
    a.check("01. isValid", container.isValid());
    a.check("02. isValid", act.costAction().isValid());
    a.checkEqual("03. getStatus", act.getStatus(), act.DisallowedTech);

    // Commit fails
    AFL_CHECK_THROWS(a("11. commit"), act.commit(), game::Exception);
}

/** Test using parts from storage.
    A: place parts in storage. Enable isUsePartsFromStorage.
    E: cost reported as zero. */
AFL_TEST("game.actions.BuildShip:setUsePartsFromStorage", a)
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 10);
    h.planet.setBaseStorage(game::EngineTech,  1, 10);
    h.planet.setBaseStorage(game::BeamTech,    1, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 10);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Initial state: do not use parts from storage
    a.checkEqual("01. isUsePartsFromStorage", act.isUsePartsFromStorage(), false);
    a.checkEqual("02. getCost", act.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Set to use parts:
    act.setUsePartsFromStorage(true);
    a.check("11. getCost", act.costAction().getCost().isZero());
}

/** Test using parts from storage.
    A: place some parts in storage. Enable isUsePartsFromStorage.
    E: cost reports only the missing parts. */
AFL_TEST("game.actions.BuildShip:setUsePartsFromStorage:partial", a)
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 1);
    h.planet.setBaseStorage(game::EngineTech,  1, 1);
    h.planet.setBaseStorage(game::BeamTech,    1, 1);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 1);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Initial state: do not use parts from storage
    a.checkEqual("01. isUsePartsFromStorage", act.isUsePartsFromStorage(), false);
    a.checkEqual("02. getCost", act.costAction().getCost().toCargoSpecString(), "13T 3D 12M 50S 118$");

    // Set to use parts. New costs:
    //  Engines (2):     2T  2D  2M   2$
    //  Beams (3):               3M
    //  Launchers (4):           4M      40S
    // Total:            2T  2D  9M   2$ 40S
    act.setUsePartsFromStorage(true);
    a.checkEqual("11. getCost", act.costAction().getCost().toCargoSpecString(), "2T 2D 9M 40S 2$");

    // Commit and verify result
    act.commit();
    a.checkEqual("21. getHullIndex", h.planet.getBaseBuildOrder().getHullIndex(), HULL_SLOT);
    a.checkEqual("22. getEngineType", h.planet.getBaseBuildOrder().getEngineType(), 1);
    a.checkEqual("23. getBeamType", h.planet.getBaseBuildOrder().getBeamType(), 1);
    a.checkEqual("24. getNumBeams", h.planet.getBaseBuildOrder().getNumBeams(), 4);
    a.checkEqual("25. getTorpedoType", h.planet.getBaseBuildOrder().getTorpedoType(), 1);
    a.checkEqual("26. getNumLaunchers", h.planet.getBaseBuildOrder().getNumLaunchers(), 5);

    a.checkEqual("31. hulls", h.planet.getBaseStorage(game::HullTech, HULL_SLOT).orElse(0), 1);
    a.checkEqual("32. engines", h.planet.getBaseStorage(game::EngineTech,  1).orElse(0), 3);
    a.checkEqual("33. beams", h.planet.getBaseStorage(game::BeamTech,    1).orElse(0), 4);
    a.checkEqual("34. torps", h.planet.getBaseStorage(game::TorpedoTech, 1).orElse(0), 5);

    // No change to hull tech, we're re-using the hull
    a.checkEqual("41. getBaseTechLevel", h.planet.getBaseTechLevel(game::HullTech).orElse(0), 1);
}

/** Test pre-existing build order.
    A: create BuildShip action on planet with pre-existing build order.
    E: build order correctly loaded as default; unused components correctly selected */
AFL_TEST("game.actions.BuildShip:preexisting-order", a)
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
        sbo.setTorpedoType(0);
        sbo.setNumLaunchers(0);
        h.planet.setBaseBuildOrder(sbo);
    }

    // Create action
    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Verify initial order
    a.checkEqual("01 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
    a.checkEqual("02. getEngineType", act.getBuildOrder().getEngineType(), 2);
    a.checkEqual("03. getBeamType", act.getBuildOrder().getBeamType(), 4);
    a.checkEqual("04. getNumBeams", act.getBuildOrder().getNumBeams(), 1);
    a.checkEqual("05. getTorpedoType", act.getBuildOrder().getTorpedoType(), 7);
    a.checkEqual("06. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 0);
    a.check("07. isUsePartsFromStorage", act.isUsePartsFromStorage());
    a.check("08. isChange", !act.isChange());

    // Change must be registered as such
    act.setPart(game::BeamTech, 2);
    a.check("11. isChange", act.isChange());
}

/** Test foreign ship.
    A: attempt to build a ship we cannot build.
    E: building must not succeed. */
AFL_TEST("game.actions.BuildShip:foreign-ship", a)
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
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip: must have hull 9
    game::ShipBuildOrder order = act.getBuildOrder();
    a.checkEqual("01. getHullIndex", order.getHullIndex(), HULL_TYPE);

    // Change to hull 10
    order.setHullIndex(10);
    act.setBuildOrder(order);

    // Verify cost:
    //  Hull           100T         150$
    //  Tech upgrade:               100$
    //  Engines (3):     3T  3D  3M   3$
    //  Beams (4):               4M
    //  Launchers (5):           5M      50S
    // Total:          103T  3D 12M 253$ 50S
    a.checkEqual("11. getCost", act.costAction().getCost().toCargoSpecString(), "103T 3D 12M 50S 253$");

    // Verify cost summary
    game::spec::CostSummary summary;
    act.getCostSummary(summary, h.tx);

    a.checkEqual("21. getNumItems", summary.getNumItems(), 5U);
    const game::spec::CostSummary::Item* p;

    p = summary.get(0);
    a.checkNonNull("31. item 0", p);
    a.checkEqual("32. mult", p->multiplier, 1);
    a.checkEqual("33. name", p->name, "Hull tech upgrade");
    a.checkEqual("34. cost", p->cost.toCargoSpecString(), "100$");

    p = summary.get(1);
    a.checkNonNull("41. item 1", p);
    a.checkEqual("42. mult", p->multiplier, 1);
    a.checkEqual("43. name", p->name, "EX");
    a.checkEqual("44. cost", p->cost.toCargoSpecString(), "100T 150$");

    // Commit must fail
    a.checkEqual("51. getStatus", act.getStatus(), game::actions::BaseBuildAction::ForeignHull);
    AFL_CHECK_THROWS(a("52. commit"), act.commit(), game::Exception);
}

/** Test tech upgrade disabled.
    A: select build order that requires tech upgrade. Disable tech upgrades.
    E: building must not succeed. */
AFL_TEST("game.actions.BuildShip:tech-upgrade:disabled", a)
{
    TestHarness h;
    prepare(h);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Set component types (same as testTechUpgrade)
    game::ShipBuildOrder sbo = act.getBuildOrder();
    sbo.setEngineType(2);
    sbo.setBeamType(3);
    sbo.setTorpedoType(4);
    act.setBuildOrder(sbo);

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
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "16T 6D 38M 200S 1121$");
    a.checkEqual("02. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Disable
    act.setUseTechUpgrade(false);
    a.checkEqual("11. getCost", act.costAction().getCost().toCargoSpecString(), "16T 6D 38M 200S 21$");
    a.checkEqual("12. getStatus", act.getStatus(), game::actions::BaseBuildAction::DisabledTech);

    // Commit must fail
    AFL_CHECK_THROWS(a("21. commit"), act.commit(), game::Exception);
}

/** Test modification of build order.
    A: create BuildShip action. Use partial modifiers (setPart etc.).
    E: modifications correctly executed */
AFL_TEST("game.actions.BuildShip:setPart", a)
{
    TestHarness h;
    prepare(h);
    addExtraHull(h);

    // Make action
    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Check initial build order selected by BuildShip:
    // Must have tech 1 components, hull #9 (slot #12).
    // [same as testSuccess]
    a.checkEqual("01 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
    a.checkEqual("02. getEngineType", act.getBuildOrder().getEngineType(), 1);
    a.checkEqual("03. getBeamType", act.getBuildOrder().getBeamType(), 1);
    a.checkEqual("04. getNumBeams", act.getBuildOrder().getNumBeams(), 4);
    a.checkEqual("05. getTorpedoType", act.getBuildOrder().getTorpedoType(), 1);
    a.checkEqual("06. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 5);

    // Modify components
    act.setPart(game::BeamTech, 4);
    act.setPart(game::TorpedoTech, 5);
    act.setNumParts(game::actions::BuildShip::BeamWeapon, 2);
    act.setNumParts(game::actions::BuildShip::TorpedoWeapon, 1);
    act.setPart(game::EngineTech, 6);

    // Verify
    a.checkEqual("11 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
    a.checkEqual("12. getEngineType", act.getBuildOrder().getEngineType(), 6);
    a.checkEqual("13. getBeamType", act.getBuildOrder().getBeamType(), 4);
    a.checkEqual("14. getNumBeams", act.getBuildOrder().getNumBeams(), 2);
    a.checkEqual("15. getTorpedoType", act.getBuildOrder().getTorpedoType(), 5);
    a.checkEqual("16. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 1);

    // Maximize counts
    act.addParts(game::actions::BuildShip::BeamWeapon, 100);
    act.addParts(game::actions::BuildShip::TorpedoWeapon, 100);
    a.checkEqual("21. getNumBeams", act.getBuildOrder().getNumBeams(), 4);
    a.checkEqual("22. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 5);

    // Change hull
    act.setPart(game::HullTech, 11);
    a.checkEqual("31 .getHullIndex", act.getBuildOrder().getHullIndex(), 11);
    a.checkEqual("32. getEngineType", act.getBuildOrder().getEngineType(), 6);   // unchanged
    a.checkEqual("33. getBeamType", act.getBuildOrder().getBeamType(), 4);     // unchanged
    a.checkEqual("34. getNumBeams", act.getBuildOrder().getNumBeams(), 3);
    a.checkEqual("35. getTorpedoType", act.getBuildOrder().getTorpedoType(), 5); // unchanged
    a.checkEqual("36. getNumLaunchers", act.getBuildOrder().getNumLaunchers(), 10);
}

/** Test use of invalid Ids.
    A: set invalid Id using setPart().
    E: must throw when trying to set an invalid component; must NOT throw when later accessing something unrelated */
AFL_TEST("game.actions.BuildShip:setPart:bad-id", a)
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    {
        game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);
        AFL_CHECK_THROWS(a("01. setPart"), act.setPart(game::HullTech, 77), std::exception);
        AFL_CHECK_SUCCEEDS(a("02. setPart"), act.setPart(game::BeamTech, 9));
    }

    {
        game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);
        AFL_CHECK_THROWS(a("11. setPart"), act.setPart(game::EngineTech, 77), std::exception);
        AFL_CHECK_SUCCEEDS(a("12. setPart"), act.setPart(game::BeamTech, 9));
    }

    {
        game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);
        AFL_CHECK_THROWS(a("21. setPart"), act.setPart(game::BeamTech, 77), std::exception);
        AFL_CHECK_SUCCEEDS(a("22. setPart"), act.setPart(game::EngineTech, 9));
    }

    {
        game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);
        AFL_CHECK_THROWS(a("31. setPart"), act.setPart(game::TorpedoTech, 77), std::exception);
        AFL_CHECK_SUCCEEDS(a("32. setPart"), act.setPart(game::EngineTech, 9));
    }
}

/** Test bad precondition: hull.
    A: create planet with invalid hull slot in its build order. Create BuildShip action.
    E: action created successfully, valid hull chosen */
AFL_TEST("game.actions.BuildShip:fail:bad-hull", a)
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(30);          // Invalid index
    o.setEngineType(9);
    h.planet.setBaseBuildOrder(o);

    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    a.checkEqual("01 .getHullIndex", act.getBuildOrder().getHullIndex(), HULL_TYPE);
}

/** Test bad precondition: engine.
    A: create planet with invalid engine in its build order. Create BuildShip action.
    E: action created successfully, valid engine chosen */
AFL_TEST("game.actions.BuildShip:fail:bad-engine", a)
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(19);       // Invalid type
    h.planet.setBaseBuildOrder(o);

    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    a.checkEqual("01. getEngineType", act.getBuildOrder().getEngineType(), 1);
}

/** Test bad precondition: beam.
    A: create planet with invalid beam in its build order. Create BuildShip action.
    E: action created successfully, valid beam chosen */
AFL_TEST("game.actions.BuildShip:fail:bad-beam", a)
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

    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    a.checkEqual("01. getBeamType", act.getBuildOrder().getBeamType(), 1);
}

/** Test bad precondition: torpedo launcher.
    A: create planet with invalid torpedo launcherin its build order. Create BuildShip action.
    E: action created successfully, valid launcher chosen */
AFL_TEST("game.actions.BuildShip:fail:bad-launcher", a)
{
    TestHarness h;
    prepare(h);
    game::test::CargoContainer container;

    game::ShipBuildOrder o;
    o.setHullIndex(HULL_SLOT);
    o.setEngineType(9);
    o.setNumLaunchers(1);
    o.setTorpedoType(20);        // Invalid type
    h.planet.setBaseBuildOrder(o);

    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    a.checkEqual("01. getTorpedoType", act.getBuildOrder().getTorpedoType(), 1);
}

/** Test cost summary.
    A: create an interesting build order (same as testUsePartsPartial).
    E: verify correct details generated */
AFL_TEST("game.actions.BuildShip:getCostSummary", a)
{
    TestHarness h;
    prepare(h);

    // Put some components into storage
    h.planet.setBaseStorage(game::HullTech, HULL_SLOT, 1);
    h.planet.setBaseStorage(game::EngineTech,  1, 1);
    h.planet.setBaseStorage(game::BeamTech,    1, 1);
    h.planet.setBaseStorage(game::TorpedoTech, 1, 1);

    game::test::CargoContainer container;
    game::actions::BuildShip act(h.planet, container, *h.shipList, *h.root);

    // Initial state: do not use parts from storage
    act.setUsePartsFromStorage(true);
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "2T 2D 9M 40S 2$");

    // Verify cost summary
    //   1x From storage: hull
    //   2x Engine
    //   1x From storage: engine
    //   3x Beam
    //   1x From storage: beam
    //   4x Launcher
    //   1x From storage: launcher
    game::spec::CostSummary summary;
    act.getCostSummary(summary, h.tx);

    a.checkEqual("11. getNumItems", summary.getNumItems(), 7U);

    const game::spec::CostSummary::Item* p;
    p = summary.get(0);
    a.checkNonNull("21. item 0", p);
    a.checkEqual("22. mult", p->multiplier, 1);
    a.checkEqual("23. name", p->name, "From storage: HH");
    a.checkEqual("24. cost", p->cost.isZero(), true);

    p = summary.get(1);
    a.checkNonNull("31. item 1", p);
    a.checkEqual("32. mult", p->multiplier, 2);
    a.checkEqual("33. name", p->name, "E");
    a.checkEqual("34. cost", p->cost.toCargoSpecString(), "2TDM 2$");

    p = summary.get(2);
    a.checkNonNull("41. item 2", p);
    a.checkEqual("42. mult", p->multiplier, 1);
    a.checkEqual("43. name", p->name, "From storage: E");
    a.checkEqual("44. cost", p->cost.isZero(), true);

    p = summary.get(3);
    a.checkNonNull("51. item 3", p);
    a.checkEqual("52. mult", p->multiplier, 3);
    a.checkEqual("53. name", p->name, "B");
    a.checkEqual("54. cost", p->cost.toCargoSpecString(), "3M");

    p = summary.get(4);
    a.checkNonNull("61. item 4", p);
    a.checkEqual("62. mult", p->multiplier, 1);
    a.checkEqual("63. name", p->name, "From storage: B");
    a.checkEqual("64. cost", p->cost.isZero(), true);

    p = summary.get(5);
    a.checkNonNull("71. item 5", p);
    a.checkEqual("72. mult", p->multiplier, 4);
    a.checkEqual("73. name", p->name, "L");
    a.checkEqual("74. cost", p->cost.toCargoSpecString(), "4M 40S");

    p = summary.get(6);
    a.checkNonNull("81. item 6", p);
    a.checkEqual("82. mult", p->multiplier, 1);
    a.checkEqual("83. name", p->name, "From storage: L");
    a.checkEqual("84. cost", p->cost.isZero(), true);
}
