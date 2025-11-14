/**
  *  \file test/game/actions/buildstructurestest.cpp
  *  \brief Test for game::actions::BuildStructures
  */

#include "game/actions/buildstructures.hpp"

#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/map/planetstorage.hpp"
#include "game/test/cargocontainer.hpp"

using game::Element;

namespace {
    const int OWNER = 5;

    game::map::Planet& preparePlanet(game::map::Planet& p)
    {
        p.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(OWNER));
        p.setOwner(OWNER);
        p.setPosition(game::map::Point(1122, 3344));
        p.setCargo(Element::Money, 100);
        p.setCargo(Element::Supplies, 20);
        p.setCargo(Element::Colonists, 25);
        p.setNumBuildings(game::MineBuilding, 10);
        p.setNumBuildings(game::DefenseBuilding, 10);
        p.setNumBuildings(game::FactoryBuilding, 10);
        p.setPlayability(game::map::Object::Playable);
        return p;
    }

    struct TestHarness {
        afl::base::Ref<game::config::HostConfiguration> config;
        game::map::Planet planet;
        game::map::PlanetStorage container;

        TestHarness();
    };

    TestHarness::TestHarness()
        : config(game::config::HostConfiguration::create()),
          planet(99),
          container(preparePlanet(planet), *config)
    {
        config->setDefaultValues();
    }
}

/** Test error case: planet not being played.
    A BuildStructures object must refuse being created for a planet we do not play. */
AFL_TEST("game.actions.BuildStructures:error:not-played", a)
{
    game::map::Planet planet(99);
    game::test::CargoContainer container;
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();

    AFL_CHECK_THROWS(a, (game::actions::BuildStructures(planet, container, *config)), game::Exception);
}

/** Test standard success case.
    The add() method must correctly add the requested amount, report correct cost, and produce correct result in commit(). */
AFL_TEST("game.actions.BuildStructures:success", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Verify
    // - ranges
    a.checkEqual("01. getMinBuildings", act.getMinBuildings(game::MineBuilding), 10);
    a.checkEqual("02. getMaxBuildings", act.getMaxBuildings(game::MineBuilding), 25);
    // - add 10, successfully
    a.checkEqual("03. add", act.add(game::MineBuilding, 10, false), 10);
    // - add 10 more, unsuccessfully
    a.checkEqual("04. add", act.add(game::MineBuilding, 10, false), 0);
    // - add 10, partially
    a.checkEqual("05. add", act.add(game::MineBuilding, 10, true), 5);
    // - cost
    a.checkEqual("06. getCost", act.costAction().getCost().toCargoSpecString(), "15S 60$");
    // - reservation
    a.checkEqual("07. getChange", h.container.getChange(Element::Supplies), -15);
    a.checkEqual("08. getChange", h.container.getChange(Element::Money), -60);

    // Commit
    AFL_CHECK_SUCCEEDS(a("11. commit"), act.commit());
    a.checkEqual("12. getCargo", h.planet.getCargo(Element::Supplies).orElse(0), 5);
    a.checkEqual("13. getCargo", h.planet.getCargo(Element::Money).orElse(0), 40);
    a.checkEqual("14. getNumBuildings", h.planet.getNumBuildings(game::MineBuilding).orElse(0), 25);
}

/** Test modification in background.
    If the planet is changed in the background, the action must recompute the cost on commit, even when not getting a listener notification. */
AFL_TEST("game.actions.BuildStructures:parallel-modification", a)
{
    TestHarness h;

    // Action: build 15
    game::actions::BuildStructures act(h.planet, h.container, *h.config);
    a.checkEqual("01. add", act.add(game::MineBuilding, 15, false), 15);

    // In the background, build 10
    h.planet.setNumBuildings(game::MineBuilding, 20);

    // Commit
    AFL_CHECK_SUCCEEDS(a("11. commit"), act.commit());

    // Verify. Must have deduced only 5 (not 15).
    a.checkEqual("21. getCargo", h.planet.getCargo(Element::Supplies).orElse(0), 15);
    a.checkEqual("22. getCargo", h.planet.getCargo(Element::Money).orElse(0), 80);
    a.checkEqual("23. getNumBuildings", h.planet.getNumBuildings(game::MineBuilding).orElse(0), 25);
}

/** Test modification in background.
    If the planet is changed in the background, the action must recompute the cost on commit, even when not getting a listener notification. */
AFL_TEST("game.actions.BuildStructures:parallel-modification:notify", a)
{
    TestHarness h;

    // Action: build 15
    game::actions::BuildStructures act(h.planet, h.container, *h.config);
    a.checkEqual("01. add", act.add(game::MineBuilding, 15, false), 15);
    a.checkEqual("02. cost", act.costAction().getCost().toCargoSpecString(), "15S 60$");

    // In the background, build 10
    h.planet.setNumBuildings(game::MineBuilding, 20);
    h.planet.notifyListeners();

    // Cost has updated
    a.checkEqual("02. cost", act.costAction().getCost().toCargoSpecString(), "5S 20$");
}

/** Test multiple builds.
    Multiple builds must be added, cost-wise. */
AFL_TEST("game.actions.BuildStructures:multiple", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Add 3 of each
    a.checkEqual("01. add", act.add(game::MineBuilding,    3, false), 3);
    a.checkEqual("02. add", act.add(game::DefenseBuilding, 3, false), 3);
    a.checkEqual("03. add", act.add(game::FactoryBuilding, 3, false), 3);

    // Verify cost: 3*(4+10+3) = 51$, 9S
    a.checkEqual("11. getCost", act.costAction().getCost().toCargoSpecString(), "9S 51$");
}

/** Test building with resource limit.
    The addLimitCash() function must stop adding buildings when resources are exceeded. */
AFL_TEST("game.actions.BuildStructures:addLimitCash", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Check how callbacks are suppressed
    int counter = 0;
    class Listener : public afl::base::Closure<void()> {
     public:
        Listener(int& counter)
            : m_counter(counter)
            { }
        virtual void call()
            { ++m_counter; }
        virtual Listener* clone() const
            { return new Listener(m_counter); }
     private:
        int& m_counter;
    };
    act.sig_change.addNewClosure(new Listener(counter));

    // We have 100$ 20S. This is enough to build 10 defenses, leaving 10S.
    a.checkEqual("01. addLimitCash", act.addLimitCash(game::DefenseBuilding, 100), 10);

    // 10S is enough to build 2 factories, leaving 2S.
    a.checkEqual("11. addLimitCash", act.addLimitCash(game::FactoryBuilding, 100), 2);

    // There must be exactly two callbacks (one for each addLimitCash call); everything else been consumed by Deferer.
    a.checkEqual("21. num listener invocations", counter, 2);

    // Commit and verify
    AFL_CHECK_SUCCEEDS(a("31. commit"), act.commit());
    a.checkEqual("32. getNumBuildings", h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 20);
    a.checkEqual("33. getNumBuildings", h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 12);
    a.checkEqual("34. getCargo", h.planet.getCargo(Element::Money).orElse(-1), 0);
    a.checkEqual("35. getCargo", h.planet.getCargo(Element::Supplies).orElse(-1), 2);
}

/** Test autobuild.
    The doStandardAutoBuild() function must behave as documented in the normal case. */
AFL_TEST("game.actions.BuildStructures:doStandardAutoBuild", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Set autobuild goals. (These are defaults.)
    h.planet.setAutobuildGoal(game::MineBuilding,        1000);
    h.planet.setAutobuildGoal(game::FactoryBuilding,     1000);
    h.planet.setAutobuildGoal(game::DefenseBuilding,     1000);
    h.planet.setAutobuildGoal(game::BaseDefenseBuilding, 1000);
    h.planet.setAutobuildSpeed(game::MineBuilding,        5);
    h.planet.setAutobuildSpeed(game::FactoryBuilding,    10);
    h.planet.setAutobuildSpeed(game::DefenseBuilding,     3);
    h.planet.setAutobuildSpeed(game::BaseDefenseBuilding, 2);

    // Do it
    act.doStandardAutoBuild();

    // We have             100$ 20S
    // Build 10 factories:  70$ 10S
    // Build 5 mines:       50$ 5S
    // Build 3 defenses:    20$ 2S
    // Build 2 factories:   14$ 0S
    // (all on top of the 10 we already have.)
    a.checkEqual("01. getNumBuildings", act.getNumBuildings(game::MineBuilding), 15);
    a.checkEqual("02. getNumBuildings", act.getNumBuildings(game::FactoryBuilding), 22);
    a.checkEqual("03. getNumBuildings", act.getNumBuildings(game::DefenseBuilding), 13);
    a.checkEqual("04. getNumBuildings", act.getNumBuildings(game::BaseDefenseBuilding), 0);
    a.checkEqual("05. getRemainingAmount", act.costAction().getRemainingAmount(Element::Money), 14);

    // Commit
    AFL_CHECK_SUCCEEDS(a("11. commit"), act.commit());
    a.checkEqual("12. getNumBuildings", h.planet.getNumBuildings(game::MineBuilding).orElse(0), 15);
    a.checkEqual("13. getNumBuildings", h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 22);
    a.checkEqual("14. getNumBuildings", h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 13);
    a.checkEqual("15. getCargo", h.planet.getCargo(Element::Money).orElse(-1), 14);
    a.checkEqual("16. getCargo", h.planet.getCargo(Element::Supplies).orElse(-1), 0);
}

/** Test autobuild.
    The doStandardAutoBuild() function must behave as documented in the normal case.
    This exercises the grouping feature, where equal speeds are built together. */
AFL_TEST("game.actions.BuildStructures:doStandardAutoBuild:grouping", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Set autobuild goals. Factories and defense form a group.
    h.planet.setAutobuildGoal(game::MineBuilding,        1000);
    h.planet.setAutobuildGoal(game::FactoryBuilding,     1000);
    h.planet.setAutobuildGoal(game::DefenseBuilding,     1000);
    h.planet.setAutobuildGoal(game::BaseDefenseBuilding, 1000);
    h.planet.setAutobuildSpeed(game::MineBuilding,        3);
    h.planet.setAutobuildSpeed(game::FactoryBuilding,     5);
    h.planet.setAutobuildSpeed(game::DefenseBuilding,     5);
    h.planet.setAutobuildSpeed(game::BaseDefenseBuilding, 2);

    // Do it
    act.doStandardAutoBuild();

    // We have                        100$ 20S
    // Build 5 factories + 5 defense:  35$ 10S
    // Build 3 mines:                  23$  7S
    // Build 2 factories + 2 defense:   0$  0S
    // (all on top of the 10 we already have.)
    a.checkEqual("01. getNumBuildings", act.getNumBuildings(game::MineBuilding), 13);
    a.checkEqual("02. getNumBuildings", act.getNumBuildings(game::FactoryBuilding), 17);
    a.checkEqual("03. getNumBuildings", act.getNumBuildings(game::DefenseBuilding), 17);
    a.checkEqual("04. getNumBuildings", act.getNumBuildings(game::BaseDefenseBuilding), 0);
    a.checkEqual("05. getRemainingAmount", act.costAction().getRemainingAmount(Element::Money), 0);

    // Commit
    AFL_CHECK_SUCCEEDS(a("11. commit"), act.commit());
    a.checkEqual("12. getNumBuildings", h.planet.getNumBuildings(game::MineBuilding).orElse(0), 13);
    a.checkEqual("13. getNumBuildings", h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 17);
    a.checkEqual("14. getNumBuildings", h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 17);
    a.checkEqual("15. getCargo", h.planet.getCargo(Element::Money).orElse(-1), 0);
    a.checkEqual("16. getCargo", h.planet.getCargo(Element::Supplies).orElse(-1), 0);
}

/** Test build failure (resources exceeded).
    commit() must throw an exception. */
AFL_TEST("game.actions.BuildStructures:error:no-resources", a)
{
    TestHarness h;
    game::actions::BuildStructures act(h.planet, h.container, *h.config);

    // Add 15 defense. These cost 150$ which we do not have
    a.checkEqual("01. add", act.add(game::DefenseBuilding, 15, false), 15);
    a.checkEqual("02. getCost", act.costAction().getCost().toCargoSpecString(), "15S 150$");

    // Invalid
    a.checkEqual("11. isValid", act.isValid(), false);
    AFL_CHECK_THROWS(a("12. commit"), act.commit(), game::Exception);

    // Reverting must make it valid again
    a.checkEqual("21. add", act.add(game::DefenseBuilding, -100, true), -15);
    a.checkEqual("22. isValid", act.isValid(), true);
    AFL_CHECK_SUCCEEDS(a("23. commit"), act.commit());
}
