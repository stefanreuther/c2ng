/**
  *  \file test/game/actions/buildpartstest.cpp
  *  \brief Test for game::actions::BuildParts
  */

#include "game/actions/buildparts.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/reverter.hpp"
#include "game/map/universe.hpp"
#include "game/registrationkey.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

namespace {
    class TestHarness {
     public:
        TestHarness()
            : univ(),
              planet(*univ.planets().create(72)),
              container(),
              shipList(),
              root(afl::io::InternalDirectory::create("game dir"),
                   *new game::test::SpecificationLoader(),
                   game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 47)),
                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                   game::Root::Actions_t())
            { root.hostConfiguration().setDefaultValues(); }

        game::map::Universe univ;
        game::map::Planet& planet;
        game::test::CargoContainer container;
        game::spec::ShipList shipList;
        game::Root root;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
    };

    class TestReverter : public game::map::Reverter {
     public:
        virtual afl::base::Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return afl::base::Nothing; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 0; }
        virtual afl::base::Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            {
                // This is the only method we need
                return 0;
            }
        virtual int getNumTorpedoesAllowedToSell(int, int) const
            { return 0; }
        virtual int getNumFightersAllowedToSell(int) const
            { return 0; }
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return afl::base::Nothing; }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
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
        }

        // Define planet with base
        h.planet.setPosition(game::map::Point(1111, 2222));
        h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
        h.planet.addCurrentBaseData(bd, game::PlayerSet_t(7));
        h.planet.setOwner(7);
        h.planet.setBaseTechLevel(game::HullTech, 1);
        h.planet.setBaseTechLevel(game::EngineTech, 1);
        h.planet.setBaseTechLevel(game::BeamTech, 1);
        h.planet.setBaseTechLevel(game::TorpedoTech, 1);
        h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, h.log);
        h.planet.setPlayability(game::map::Object::Playable);

        // Define a number of components
        // - Hull #9
        {
            game::spec::Hull* hh = h.shipList.hulls().create(9);
            hh->setTechLevel(2);
            hh->setNumEngines(3);
            hh->cost() = game::spec::Cost::fromString("10T 15$");
        }
        // - Engine #1
        {
            game::spec::Engine* e = h.shipList.engines().create(1);
            e->setTechLevel(1);
            e->cost() = game::spec::Cost::fromString("1TDM 1$");
        }
        // - Beam #4
        {
            game::spec::Beam* b = h.shipList.beams().create(4);
            b->setTechLevel(4);
            b->cost() = game::spec::Cost::fromString("4M");
        }
        // - Launcher #3
        {
            game::spec::TorpedoLauncher* tl = h.shipList.launchers().create(3);
            tl->setTechLevel(3);
            tl->cost() = game::spec::Cost::fromString("4M 30S");
        }
        // - Hull association
        h.shipList.hullAssignments().add(7, 12, 9);
    }
}

/** Basic functionality test.
    If multiple orders are given, they must be billed and executed as a block. */
AFL_TEST("game.actions.BuildParts:simple", a)
{
    TestHarness h;
    prepare(h);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);

    // Add components, verifying each step
    // - 3 hulls = 30T 45$, plus 100$ for tech
    a.checkEqual("01. add", act.add(game::HullTech, 12, 3, false), 3);
    a.checkEqual("02. isValid", act.isValid(), true);
    a.checkEqual("03. getCost", act.costAction().getCost().toCargoSpecString(), "30T 145$");
    a.checkEqual("04. getChange", h.container.getChange(game::Element::Money), -145);

    // - 5 engines = 5TDM$
    a.checkEqual("11. add", act.add(game::EngineTech, 1, 5, false), 5);
    a.checkEqual("12. isValid", act.isValid(), true);
    a.checkEqual("13. getCost", act.costAction().getCost().toCargoSpecString(), "35T 5D 5M 150$");
    a.checkEqual("14. getChange", h.container.getChange(game::Element::Money), -150);

    // - 1 beam = 4M, plus 600$ for tech
    a.checkEqual("21. add", act.add(game::BeamTech, 4, 1, false), 1);
    a.checkEqual("22. isValid", act.isValid(), true);
    a.checkEqual("23. getCost", act.costAction().getCost().toCargoSpecString(), "35T 5D 9M 750$");
    a.checkEqual("24. getChange", h.container.getChange(game::Element::Money), -750);

    // - 4 launchers = 16M 120S, plus 300$ for tech
    a.checkEqual("31. add", act.add(game::TorpedoTech, 3, 4, false), 4);
    a.checkEqual("32. isValid", act.isValid(), true);
    a.checkEqual("33. getCost", act.costAction().getCost().toCargoSpecString(), "35T 5D 25M 120S 1050$");
    a.checkEqual("34. getChange", h.container.getChange(game::Element::Money), -1050);

    // Commit and verify that everything arrived on the planet
    act.commit();
    a.checkEqual("41. hulls", h.planet.getBaseStorage(game::HullTech, 12).orElse(0), 3);
    a.checkEqual("42. engines", h.planet.getBaseStorage(game::EngineTech, 1).orElse(0), 5);
    a.checkEqual("43. beams", h.planet.getBaseStorage(game::BeamTech, 4).orElse(0), 1);
    a.checkEqual("44. torps", h.planet.getBaseStorage(game::TorpedoTech, 3).orElse(0), 4);
}

/** Test adding to present parts.
    If the unit already has some parts, building must add to the storage. */
AFL_TEST("game.actions.BuildParts:add", a)
{
    TestHarness h;
    prepare(h);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    h.planet.setBaseStorage(game::HullTech, 12, 100);

    // Add 5 hulls. Must end with 105.
    a.checkEqual("01. add", act.add(game::HullTech, 12, 5, false), 5);
    act.commit();
    a.checkEqual("02. hulls", h.planet.getBaseStorage(game::HullTech, 12).orElse(0), 105);
}

/** Test modifying a build order.
    Successive changes for the same component must be added.
    The "partial" parameter must be handled correctly. */
AFL_TEST("game.actions.BuildParts:add:sequence", a)
{
    TestHarness h;
    prepare(h);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    h.planet.setBaseStorage(game::HullTech, 12, 100);

    // Modify, verifying each step
    // - 3 hulls = 30T 45$, plus 100$ for tech
    a.checkEqual("01. add", act.add(game::HullTech, 12, 3, false), 3);
    a.checkEqual("02. isValid", act.isValid(), true);
    a.checkEqual("03. getCost", act.costAction().getCost().toCargoSpecString(), "30T 145$");
    a.checkEqual("04. getChange", h.container.getChange(game::Element::Money), -145);

    // - remove 5, must fail
    a.checkEqual("11. add", act.add(game::HullTech, 12, -5, false), 0);
    a.checkEqual("12. isValid", act.isValid(), true);
    a.checkEqual("13. getCost", act.costAction().getCost().toCargoSpecString(), "30T 145$");
    a.checkEqual("14. getChange", h.container.getChange(game::Element::Money), -145);

    // - remove 5, allowing partial remove
    a.checkEqual("21. add", act.add(game::HullTech, 12, -5, true), -3);
    a.checkEqual("22. isValid", act.isValid(), true);
    a.checkEqual("23. getCost", act.costAction().getCost().toCargoSpecString(), "");
    a.checkEqual("24. getChange", h.container.getChange(game::Element::Money), 0);

    // - add 12000, must fail (overflow)
    a.checkEqual("31. add", act.add(game::HullTech, 12, 12000, false), 0);

    // - add 12000, allowing partial add
    a.checkEqual("41. add", act.add(game::HullTech, 12, 12000, true), 9900);
    a.checkEqual("42. isValid", act.isValid(), false);       // not enough cash!
    a.checkEqual("43. getCost", act.costAction().getCost().toCargoSpecString(), "99000T 148600$");
    a.checkEqual("44. getChange", h.container.getChange(game::Element::Money), -148600);
}

/** Test modifying tech behind our back.
    If the configuration or the underlying data changes, and the listeners are called correctly,
    the reported cost must change. */
AFL_TEST("game.actions.BuildParts:parallel-upgrade", a)
{
    TestHarness h;
    prepare(h);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    h.planet.setBaseStorage(game::HullTech, 12, 100);

    // Modify, verifying each step
    // - 10 beams = 40M, plus $600 for tech
    a.checkEqual("01. add", act.add(game::BeamTech, 4, 10, false), 10);
    a.checkEqual("02. isValid", act.isValid(), true);
    a.checkEqual("03. getCost", act.costAction().getCost().toCargoSpecString(), "40M 600$");
    a.checkEqual("04. getChange", h.container.getChange(game::Element::Money), -600);

    // Upgrade to tech 3, this will reduce the tech cost
    h.planet.setBaseTechLevel(game::BeamTech, 2);
    h.univ.notifyListeners();
    a.checkEqual("11. isValid", act.isValid(), true);
    a.checkEqual("12. getCost", act.costAction().getCost().toCargoSpecString(), "40M 500$");
    a.checkEqual("13. getChange", h.container.getChange(game::Element::Money), -500);

    // Change configuration
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(250);
    h.root.hostConfiguration().notifyListeners();
    a.checkEqual("21. getCost", act.costAction().getCost().toCargoSpecString(), "40M 1250$");
    a.checkEqual("22. getChange", h.container.getChange(game::Element::Money), -1250);

    // Upgrade to tech 10, this will drop the tech cost completely
    h.planet.setBaseTechLevel(game::BeamTech, 10);
    h.univ.notifyListeners();
    a.checkEqual("31. isValid", act.isValid(), true);
    a.checkEqual("32. getCost", act.costAction().getCost().toCargoSpecString(), "40M");
    a.checkEqual("33. getChange", h.container.getChange(game::Element::Money), 0);
}

/** Test revertible.
    After setUndoInformation() is called, the action must allow reverting previous builds. */
AFL_TEST("game.actions.BuildParts:revert", a)
{
    TestHarness h;
    prepare(h);
    h.univ.setNewReverter(new TestReverter());
    h.planet.setBaseStorage(game::BeamTech, 4, 10);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);

    // Beam count must not be revertible so far
    a.checkEqual("01. getMinParts", act.getMinParts(game::BeamTech, 4), 10);

    // Add undo information. Beam count must now be revertible
    act.setUndoInformation(h.univ);
    a.checkEqual("11. getMinParts", act.getMinParts(game::BeamTech, 4), 0);

    // OK, scrap some
    a.checkEqual("21. add", act.add(game::BeamTech, 4, -3, false), -3);
    a.checkEqual("22. getNumParts", act.getNumParts(game::BeamTech, 4), 7);
    a.checkEqual("23. isValid", act.isValid(), true);
    a.checkEqual("24. getCost", act.costAction().getCost().toCargoSpecString(), "-12M");
    a.checkEqual("25. getChange", h.container.getChange(game::Element::Molybdenum), 12);

    // Commit
    act.commit();
    a.checkEqual("31. beams", h.planet.getBaseStorage(game::BeamTech, 4).orElse(0), 7);
}

/** Test ship building.
    If a ship is being built while a revert action is active,
    the revert must be adjusted to keep the build order intact. */
AFL_TEST("game.actions.BuildParts:parallel-ship-build", a)
{
    TestHarness h;
    prepare(h);
    h.univ.setNewReverter(new TestReverter());

    // Place some parts on the base
    h.planet.setBaseStorage(game::HullTech,   12,  1);
    h.planet.setBaseStorage(game::EngineTech,  1,  5);
    h.planet.setBaseStorage(game::BeamTech,    4, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 3, 10);

    // Build the action. Everything revertible so far.
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    act.setUndoInformation(h.univ);
    a.checkEqual("01. getMinParts", act.getMinParts(game::HullTech,   12), 0);
    a.checkEqual("02. getMinParts", act.getMinParts(game::EngineTech,  1), 0);
    a.checkEqual("03. getMinParts", act.getMinParts(game::BeamTech,    4), 0);
    a.checkEqual("04. getMinParts", act.getMinParts(game::TorpedoTech, 3), 0);

    // Claim revert.
    a.checkEqual("11. add", act.add(game::EngineTech, 1, -5, true), -5);
    a.checkEqual("12. getNumParts", act.getNumParts(game::EngineTech, 1), 0);

    // Set build order. Do not call listener.
    game::ShipBuildOrder sbo;
    sbo.setHullIndex(12);
    sbo.setEngineType(1);
    sbo.setNumBeams(1);
    sbo.setBeamType(4);
    sbo.setNumLaunchers(1);
    sbo.setTorpedoType(3);
    h.planet.setBaseBuildOrder(sbo);

    // Commit
    AFL_CHECK_SUCCEEDS(a("21. commit"), act.commit());
    a.checkEqual("22. engines", h.planet.getBaseStorage(game::EngineTech, 1).orElse(0), 3);
}

/** Test ship build, check minima.
    If a build order is present, reported minima must correctly protect it. */
AFL_TEST("game.actions.BuildParts:ship-build:same-type", a)
{
    TestHarness h;
    prepare(h);
    h.univ.setNewReverter(new TestReverter());

    // Place some parts on the base
    h.planet.setBaseStorage(game::HullTech,   12,  1);
    h.planet.setBaseStorage(game::EngineTech,  1,  5);
    h.planet.setBaseStorage(game::BeamTech,    4, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 3, 10);

    // Build a ship
    game::ShipBuildOrder sbo;
    sbo.setHullIndex(12);
    sbo.setEngineType(1);
    sbo.setNumBeams(2);
    sbo.setBeamType(4);
    sbo.setNumLaunchers(1);
    sbo.setTorpedoType(3);
    h.planet.setBaseBuildOrder(sbo);

    // Build the action. Check that it protects the ship build order.
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    act.setUndoInformation(h.univ);
    a.checkEqual("01. getMinParts", act.getMinParts(game::HullTech,   12), 1);
    a.checkEqual("02. getMinParts", act.getMinParts(game::EngineTech,  1), 3);
    a.checkEqual("03. getMinParts", act.getMinParts(game::BeamTech,    4), 2);
    a.checkEqual("04. getMinParts", act.getMinParts(game::TorpedoTech, 3), 1);
}

/** Test ship build, check minima, different ship type.
    If a build order is present for different parts, this must not affect reported minima. */
AFL_TEST("game.actions.BuildParts:ship-build:different-type", a)
{
    TestHarness h;
    prepare(h);
    h.univ.setNewReverter(new TestReverter());

    // Place some parts on the base
    h.planet.setBaseStorage(game::HullTech,   12,  1);
    h.planet.setBaseStorage(game::HullTech,   13,  1);
    h.planet.setBaseStorage(game::EngineTech,  1,  5);
    h.planet.setBaseStorage(game::EngineTech,  2,  5);
    h.planet.setBaseStorage(game::BeamTech,    4, 10);
    h.planet.setBaseStorage(game::BeamTech,    5, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 3, 10);
    h.planet.setBaseStorage(game::TorpedoTech, 6, 10);

    // Build a ship
    game::ShipBuildOrder sbo;
    sbo.setHullIndex(13);
    sbo.setEngineType(2);
    sbo.setNumBeams(2);
    sbo.setBeamType(5);
    sbo.setNumLaunchers(1);
    sbo.setTorpedoType(6);
    h.planet.setBaseBuildOrder(sbo);

    // Build the action. Does not match ship being built, so this goes through.
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);
    act.setUndoInformation(h.univ);
    a.checkEqual("01. getMinParts", act.getMinParts(game::HullTech,   12), 0);
    a.checkEqual("02. getMinParts", act.getMinParts(game::EngineTech,  1), 0);
    a.checkEqual("03. getMinParts", act.getMinParts(game::BeamTech,    4), 0);
    a.checkEqual("04. getMinParts", act.getMinParts(game::TorpedoTech, 3), 0);
}

/** Test building with multiple commits.
    It must be possible to call commit() multiple times. */
AFL_TEST("game.actions.BuildParts:commit-repeatedly", a)
{
    TestHarness h;
    prepare(h);

    // Build the action
    game::actions::BuildParts act(h.planet, h.container, h.shipList, h.root);

    // Add components, verifying each step
    // - 3 hulls = 30T 45$, plus 100$ for tech
    a.checkEqual("01. add", act.add(game::HullTech, 12, 3, false), 3);
    a.checkEqual("02. isValid", act.isValid(), true);
    a.checkEqual("03. getCost", act.costAction().getCost().toCargoSpecString(), "30T 145$");

    // Commit
    act.commit();
    a.checkEqual("11. hulls", h.planet.getBaseStorage(game::HullTech, 12).orElse(0), 3);
    a.check("12. getCost", act.costAction().getCost().isZero());

    // Add 2 more
    // - 2 hulls = 20T 30$ (no more tech)
    a.checkEqual("21. add", act.add(game::HullTech, 12, 2, false), 2);
    a.checkEqual("22. isValid", act.isValid(), true);
    a.checkEqual("23. getCost", act.costAction().getCost().toCargoSpecString(), "20T 30$");

    // Commit again
    act.commit();
    a.checkEqual("31. hulls", h.planet.getBaseStorage(game::HullTech, 12).orElse(0), 5);
    a.check("32. getCost", act.costAction().getCost().isZero());
}
