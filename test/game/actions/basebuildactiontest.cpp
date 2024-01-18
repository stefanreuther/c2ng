/**
  *  \file test/game/actions/basebuildactiontest.cpp
  *  \brief Test for game::actions::BaseBuildAction
  */

#include "game/actions/basebuildaction.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/actions/basebuildexecutor.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"

namespace {
    class TestHarness {
     public:
        TestHarness()
            : planet(72),
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

        game::map::Planet planet;
        game::test::CargoContainer container;
        game::spec::ShipList shipList;
        game::Root root;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
    };

    void preparePlanet(afl::test::Assert a, TestHarness& h)
    {
        // Define planet with base
        h.planet.setPosition(game::map::Point(1111, 2222));
        h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
        h.planet.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(7));
        h.planet.setOwner(7);
        h.planet.setBaseTechLevel(game::HullTech, 1);
        h.planet.setBaseTechLevel(game::EngineTech, 1);
        h.planet.setBaseTechLevel(game::BeamTech, 1);
        h.planet.setBaseTechLevel(game::TorpedoTech, 1);
        h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, h.log);
        h.planet.setPlayability(game::map::Object::Playable);

        // This must have produced a base
        a.check("planet has base", h.planet.hasBase());
    }

    class TestAction : public game::actions::BaseBuildAction {
     public:
        explicit TestAction(TestHarness& h)
            : BaseBuildAction(h.planet, h.container, h.shipList, h.root),
              m_tech(1)
            { }

        virtual void perform(game::actions::BaseBuildExecutor& exec)
            { exec.setBaseTechLevel(game::BeamTech, m_tech); }

        void setTechLevel(int n)
            { m_tech = n; }

     private:
        int m_tech;
    };
}


/** Test error case: instantiating BaseBuildAction on a planet that does not have a base. */
AFL_TEST("game.actions.BaseBuildAction:error:no-base", a)
{
    TestHarness h;

    // Define planet without base
    h.planet.setPosition(game::map::Point(1111, 2222));
    h.planet.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(7));
    h.planet.setOwner(7);
    h.planet.internalCheck(game::map::Configuration(), game::PlayerSet_t(7), 12, h.tx, h.log);
    h.planet.setPlayability(game::map::Object::Playable);

    AFL_CHECK_THROWS(a(""), (TestAction(h)), game::Exception);
}

/** Test success case: instantiating BaseBuildAction on a planet with base, and working on it. */
AFL_TEST("game.actions.BaseBuildAction:success", a)
{
    TestHarness h;
    preparePlanet(a, h);

    // Make an action.
    TestAction act(h);

    // Set null operation; must have cost zero. The update() is normally in the descendant's method.
    act.setTechLevel(1);
    act.update();
    a.check("01. isValid", act.isValid());
    a.check("02. cost isZero", act.costAction().getCost().isZero());
    a.checkEqual("03. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Set invalid (unregistered)
    act.setTechLevel(6);
    act.update();
    a.check("11. isValid", !act.isValid());
    a.checkEqual("12. cost Money", act.costAction().getCost().get(game::spec::Cost::Money), 1500);
    a.checkEqual("13. getStatus", act.getStatus(), game::actions::BaseBuildAction::DisallowedTech);

    // Set valid tech level
    act.setTechLevel(4);
    act.update();
    a.check("21. isValid", act.isValid());
    a.check("22. cost isValid", !act.costAction().getCost().isZero());
    a.checkEqual("23. cost Money", act.costAction().getCost().get(game::spec::Cost::Money), 600);
    a.checkEqual("24. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Chance price configuration. This automatically updates.
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(270);
    h.root.hostConfiguration().notifyListeners();
    a.check("31. isValid", act.isValid());
    a.checkEqual("32. cost Money", act.costAction().getCost().get(game::spec::Cost::Money), 1620);
    a.checkEqual("33. getChange", h.container.getChange(game::Element::Money), -1620);
    a.checkEqual("34. getStatus", act.getStatus(), game::actions::BaseBuildAction::Success);

    // Change even more; this time exceeding the available money (5000).
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(1000);
    h.root.hostConfiguration().notifyListeners();
    a.check("41. isValid", !act.isValid());
    a.checkEqual("42. cost Money", act.costAction().getCost().get(game::spec::Cost::Money), 6000);
    a.checkEqual("43. getChange", h.container.getChange(game::Element::Money), -6000);
    a.checkEqual("44. getStatus", act.getStatus(), game::actions::BaseBuildAction::MissingResources);

    // Change back, and commit
    h.root.hostConfiguration()[game::config::HostConfiguration::BaseTechCost].set(100);
    h.root.hostConfiguration().notifyListeners();
    a.checkEqual("51. cost Money", act.costAction().getCost().get(game::spec::Cost::Money), 600);
    a.checkEqual("52. getChange", h.container.getChange(game::Element::Money), -600);
    AFL_CHECK_SUCCEEDS(a("53. commit"), act.commit());
    a.checkEqual("54. getBaseTechLevel", h.planet.getBaseTechLevel(game::BeamTech).orElse(0), 4);
}

/** Test getCostSummary().
    A: create action. Call getCostSummary().
    E: correct summary produced */
AFL_TEST("game.actions.BaseBuildAction:getCostSummary", a)
{
    TestHarness h;
    preparePlanet(a, h);
    TestAction act(h);

    // Set valid tech level
    act.setTechLevel(4);
    act.update();
    a.check("01. isValid", act.isValid());
    a.check("02. cost isZero", !act.costAction().getCost().isZero());

    // Retrieve and verify CostSummary
    {
        game::spec::CostSummary result;
        act.getCostSummary(result, h.tx);

        a.checkEqual("11. getNumItems", result.getNumItems(), 1U);
        const game::spec::CostSummary::Item* p = result.get(0);
        a.checkNonNull("12. get", p);
        a.checkEqual("13. multiplier", p->multiplier, 3);
        a.checkEqual("14. name", p->name, "Beam tech upgrade");
        a.checkEqual("15. cost", p->cost.get(game::spec::Cost::Money), 600);
    }

    // Disable tech upgrades; summary must be empty (but action must be invalid)
    act.setUseTechUpgrade(false);
    a.checkEqual("21. isUseTechUpgrade", act.isUseTechUpgrade(), false);
    {
        game::spec::CostSummary result;
        act.getCostSummary(result, h.tx);
        a.checkEqual("22. getNumItems", result.getNumItems(), 0U);
    }
    a.check("23. isValid", !act.isValid());

    // Reduce tech level; summary still empty, but action valid
    act.setTechLevel(1);
    act.update();
    {
        game::spec::CostSummary result;
        act.getCostSummary(result, h.tx);
        a.checkEqual("31. getNumItems", result.getNumItems(), 0U);
    }
    a.check("32. isValid", act.isValid());
}
