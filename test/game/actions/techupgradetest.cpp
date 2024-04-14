/**
  *  \file test/game/actions/techupgradetest.cpp
  *  \brief Test for game::actions::TechUpgrade
  */

#include "game/actions/techupgrade.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "game/v3/reverter.hpp"

namespace {
    const int X = 1234;
    const int Y = 2345;
    const int OWNER = 4;
    const int TURN_NR = 12;
    const int PLANET_ID = 363;

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

    void prepareReverter(TestHarness& h)
    {
        game::v3::Reverter* pRev = new game::v3::Reverter(h.turn, h.session);
        h.univ.setNewReverter(pRev);

        game::map::BaseData bd;
        game::map::PlanetData pd;
        h.planet.getCurrentBaseData(bd);
        h.planet.getCurrentPlanetData(pd);
        pRev->addBaseData(PLANET_ID, bd);
        pRev->addPlanetData(PLANET_ID, pd);
    }

    game::map::Ship& prepareShip(TestHarness& h, afl::test::Assert a, int id, int owner)
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
        a.checkNonNull("ship created", pShip);
        pShip->addCurrentShipData(sd, game::PlayerSet_t(OWNER));

        game::v3::Reverter* pRev = dynamic_cast<game::v3::Reverter*>(h.univ.getReverter());
        a.checkNonNull("reverter exists", pRev);
        pRev->addShipData(id, sd);

        pShip->internalCheck(game::PlayerSet_t(owner), TURN_NR);
        pShip->setPlayability(game::map::Object::Playable);

        return *pShip;
    }
}

/** Test failure.
    If the planet has no base, constructing the action must fail. */
AFL_TEST("game.actions.TechUpgrade:error:no-base", a)
{
    TestHarness h;
    afl::sys::Log log;

    // Define planet without base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, log);
    h.planet.setPlayability(game::map::Object::Playable);

    game::test::CargoContainer container;
    AFL_CHECK_THROWS(a, (game::actions::TechUpgrade(h.planet, container, *h.shipList, *h.root)), game::Exception);
}

/** Test simple success case.
    If the planet has a base, constructing the action must succeed.
    Setting a tech level must update the costs, and be rejected if it is not allowed. */
AFL_TEST("game.actions.TechUpgrade:simple", a)
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
    h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, log);
    h.planet.setPlayability(game::map::Object::Playable);

    // This must have produced a base
    a.check("01. hasBase", h.planet.hasBase());

    // Make an action
    game::test::CargoContainer container;
    game::actions::TechUpgrade act(h.planet, container, *h.shipList, *h.root);
    a.check("11. isValid", act.isValid());
    a.check("12. getCost", act.costAction().getCost().isZero());
    a.checkEqual("13. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);
    a.checkEqual("14. getMinTechLevel", act.getMinTechLevel(game::HullTech), 1);
    a.checkEqual("15. getMaxTechLevel", act.getMaxTechLevel(game::HullTech), 5);

    // Set invalid (unregistered)
    a.check("21. setTechLevel", !act.setTechLevel(game::HullTech, 6));
    a.check("22. getCost", act.costAction().getCost().isZero());

    // Set valid tech level
    a.check("31. setTechLevel", act.setTechLevel(game::HullTech, 4));
    a.check("32. isValid", act.isValid());
    a.check("33. getCost", !act.costAction().getCost().isZero());
    a.checkEqual("34. getCost", act.costAction().getCost().get(game::spec::Cost::Money), 600);
    a.checkEqual("35. getChange", container.getChange(game::Element::Money), -600);
    a.checkEqual("36. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Test upgrade
    // - 5 is ok
    a.check("41. upgradeTechLevel", act.upgradeTechLevel(game::HullTech, 5));
    a.checkEqual("42. getTechLevel", act.getTechLevel(game::HullTech), 5);
    // - 6 fails, remains at 5
    a.check("43. upgradeTechLevel", !act.upgradeTechLevel(game::HullTech, 6));
    a.checkEqual("44. getTechLevel", act.getTechLevel(game::HullTech), 5);
    // - 3 succeeds, but still 5
    a.check("45. upgradeTechLevel", act.upgradeTechLevel(game::HullTech, 3));
    a.checkEqual("46. getTechLevel", act.getTechLevel(game::HullTech), 5);
    // - revert
    a.check("47. setTechLevel", act.setTechLevel(game::HullTech, 4));
    // - no-op for completeness
    a.check("48. setTechLevel", act.setTechLevel(game::HullTech, 4));

    // Chance price configuration. This automatically updates.
    h.root->hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(150);
    h.root->hostConfiguration().notifyListeners();
    a.check("51. isValid", act.isValid());
    a.checkEqual("52. getCost", act.costAction().getCost().get(game::spec::Cost::Money), 900);
    a.checkEqual("53. getChange", container.getChange(game::Element::Money), -900);
    a.checkEqual("54. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Commit
    AFL_CHECK_SUCCEEDS(a, act.commit());

    // Verify
    a.checkEqual("71. getBaseTechLevel", h.planet.getBaseTechLevel(game::HullTech).orElse(-99), 4);
}

/** Test revertable.
    When setUndoInformation() is used, the action must allow reverting a build. */
AFL_TEST("game.actions.TechUpgrade:setUndoInformation", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    h.planet.setBaseTechLevel(game::BeamTech, 3);
    prepareReverter(h);
    a.check("01. hasBase", h.planet.hasBase());

    // Upgrade tech
    h.planet.setBaseTechLevel(game::HullTech, 2);
    h.planet.setBaseTechLevel(game::EngineTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade act(h.planet, container, *h.shipList, *h.root);
    act.setUndoInformation(h.univ);

    a.checkEqual("11. getMinTechLevel", act.getMinTechLevel(game::HullTech), 1);
    a.checkEqual("12. getMinTechLevel", act.getMinTechLevel(game::EngineTech), 1);
    a.checkEqual("13. getMinTechLevel", act.getMinTechLevel(game::BeamTech), 3);

    // Set
    act.setTechLevel(game::EngineTech, 1);
    a.checkEqual("21. getCost", act.costAction().getCost().get(game::spec::Cost::Money), -1000);
    a.checkEqual("22. getChange", container.getChange(game::Element::Money), 1000);
    a.checkEqual("23. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);
}

/** Test revertable, with change behind.
    A change done to the universe while the action is active must be reflected to the action,
    not only at the time setUndoInformation() is called. */
AFL_TEST("game.actions.TechUpgrade:setUndoInformation:parallel-modification:base", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    a.check("01. hasBase", h.planet.hasBase());

    // Upgrade tech
    h.planet.setBaseTechLevel(game::BeamTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade act(h.planet, container, *h.shipList, *h.root);
    act.setUndoInformation(h.univ);
    a.checkEqual("11. getMinTechLevel", act.getMinTechLevel(game::BeamTech), 1);

    // Build a beam (tech 4)
    h.planet.setBaseStorage(game::BeamTech, 4, 1);
    h.univ.notifyListeners();

    // Minimum tech is now 4
    a.checkEqual("21. getMinTechLevel", act.getMinTechLevel(game::BeamTech), 4);
}

/** Test revertable, with change on ship.
    A change done to the universe while the action is active must be reflected to the action,
    not only at the time setUndoInformation() is called.
    This applies even if the change is to a different object. */
AFL_TEST("game.actions.TechUpgrade:setUndoInformation:parallel-modification:ship", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    a.check("01. hasBase", h.planet.hasBase());

    game::map::Ship& ship = prepareShip(h, a("s99"), 99, OWNER);

    // Upgrade tech
    h.planet.setBaseTechLevel(game::TorpedoTech, 5);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade act(h.planet, container, *h.shipList, *h.root);
    act.setUndoInformation(h.univ);
    a.checkEqual("11. getMinTechLevel", act.getMinTechLevel(game::TorpedoTech), 1);

    // Build a torpedo (tech 3) and place on ship
    ship.setAmmo(ship.getAmmo().orElse(0) + 1);
    h.univ.notifyListeners();

    // Minimum tech is now 3
    a.checkEqual("21. getMinTechLevel", act.getMinTechLevel(game::TorpedoTech), 3);
}

/** Test revertable, no change signal.
    A change done to the universe while the action is active must be reflected to the action,
    Commit must not do stupid things when the change does not signal the listener. */
AFL_TEST("game.actions.TechUpgrade:setUndoInformation:parallel-modification:no-notification", a)
{
    // Prepare
    TestHarness h;
    prepare(h);
    prepareReverter(h);
    a.check("01. hasBase", h.planet.hasBase());

    game::map::Ship& ship = prepareShip(h, a("s99"), 99, OWNER);

    // Upgrade tech
    h.planet.setBaseTechLevel(game::TorpedoTech, 5);
    h.planet.setCargo(game::Element::Money, 0);

    // Test
    game::test::CargoContainer container;
    game::actions::TechUpgrade act(h.planet, container, *h.shipList, *h.root);
    act.setUndoInformation(h.univ);

    // Request tech downgrade
    act.setTechLevel(game::TorpedoTech, 1);
    a.checkEqual("11. getChange", container.getChange(game::Element::Money), 1000);

    // Build a torpedo (tech 3) and place on ship
    ship.setAmmo(ship.getAmmo().orElse(0) + 1);
    // Listener notification could be here: "h.univ.notifyListeners();"

    // Commit. Because minimum tech level is 3, this must only go to 3, with a 700$ refund.
    AFL_CHECK_SUCCEEDS(a("21. commit"), act.commit());
    a.checkEqual("22. getBaseTechLevel", h.planet.getBaseTechLevel(game::TorpedoTech).orElse(0), 3);
    // Since BaseBuildAction::commit now recomputes costs,
    // we end up with 0 here and cannot test the 700$ refund.
    // a.checkEqual("23. getChange", container.getChange(game::Element::Money), 700);
}
