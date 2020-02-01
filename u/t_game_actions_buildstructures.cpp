/**
  *  \file u/t_game_actions_buildstructures.cpp
  *  \brief Test for game::actions::BuildStructures
  */

#include "game/actions/buildstructures.hpp"

#include "t_game_actions.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/exception.hpp"
#include "game/map/planetstorage.hpp"
#include "game/test/interpreterinterface.hpp"

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
        game::config::HostConfiguration config;
        game::map::Planet planet;
        game::test::InterpreterInterface iface;
        game::map::PlanetStorage container;

        TestHarness();
    };

    TestHarness::TestHarness()
        : config(),
          planet(99),
          iface(),
          container(preparePlanet(planet), iface, config)
    {
        config.setDefaultValues();
    }
}

/** Test error case: planet not being played.
    A BuildStructures object must refuse being created for a planet we do not play. */
void
TestGameActionsBuildStructures::testError()
{
    game::map::Planet planet(99);
    game::test::CargoContainer container;
    game::config::HostConfiguration config;

    TS_ASSERT_THROWS((game::actions::BuildStructures(planet, container, config)), game::Exception);
}

/** Test standard success case.
    The add() method must correctly add the requested amount, report correct cost, and produce correct result in commit(). */
void
TestGameActionsBuildStructures::testSuccess()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

    // Verify
    // - ranges
    TS_ASSERT_EQUALS(a.getMinBuildings(game::MineBuilding), 10);
    TS_ASSERT_EQUALS(a.getMaxBuildings(game::MineBuilding), 25);
    // - add 10, successfully
    TS_ASSERT_EQUALS(a.add(game::MineBuilding, 10, false), 10);
    // - add 10 more, unsuccessfully
    TS_ASSERT_EQUALS(a.add(game::MineBuilding, 10, false), 0);
    // - add 10, partially
    TS_ASSERT_EQUALS(a.add(game::MineBuilding, 10, true), 5);
    // - cost
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "15S 60$");
    // - reservation
    TS_ASSERT_EQUALS(h.container.getChange(Element::Supplies), -15);
    TS_ASSERT_EQUALS(h.container.getChange(Element::Money), -60);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(0), 5);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(0), 40);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::MineBuilding).orElse(0), 25);
}

/** Test modification in background.
    If the planet is changed in the background, the action must recompute the cost on commit, even when not getting a listener notification. */
void
TestGameActionsBuildStructures::testModify()
{
    TestHarness h;

    // Action: build 15
    game::actions::BuildStructures a(h.planet, h.container, h.config);
    TS_ASSERT_EQUALS(a.add(game::MineBuilding, 15, false), 15);

    // In the background, build 10
    h.planet.setNumBuildings(game::MineBuilding, 20);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());

    // Verify. Must have deduced only 5 (not 15).
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(0), 15);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(0), 80);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::MineBuilding).orElse(0), 25);
}

/** Test multiple builds.
    Multiple builds must be added, cost-wise. */
void
TestGameActionsBuildStructures::testMulti()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

    // Add 3 of each
    TS_ASSERT_EQUALS(a.add(game::MineBuilding,    3, false), 3);
    TS_ASSERT_EQUALS(a.add(game::DefenseBuilding, 3, false), 3);
    TS_ASSERT_EQUALS(a.add(game::FactoryBuilding, 3, false), 3);

    // Verify cost: 3*(4+10+3) = 51$, 9S
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "9S 51$");
}

/** Test building with resource limit.
    The addLimitCash() function must stop adding buildings when resources are exceeded. */
void
TestGameActionsBuildStructures::testResourceLimit()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

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
    a.sig_change.addNewClosure(new Listener(counter));

    // We have 100$ 20S. This is enough to build 10 defenses, leaving 10S.
    TS_ASSERT_EQUALS(a.addLimitCash(game::DefenseBuilding, 100), 10);

    // 10S is enough to build 2 factories, leaving 2S.
    TS_ASSERT_EQUALS(a.addLimitCash(game::FactoryBuilding, 100), 2);

    // There must be exactly two callbacks (one for each addLimitCash call); everything else been consumed by Deferer.
    TS_ASSERT_EQUALS(counter, 2);

    // Commit and verify
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 20);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 12);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(-1), 0);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(-1), 2);
}

/** Test autobuild.
    The doStandardAutoBuild() function must behave as documented in the normal case. */
void
TestGameActionsBuildStructures::testAutoBuild()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

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
    a.doStandardAutoBuild();

    // We have             100$ 20S
    // Build 10 factories:  70$ 10S
    // Build 5 mines:       50$ 5S
    // Build 3 defenses:    20$ 2S
    // Build 2 factories:   14$ 0S
    // (all on top of the 10 we already have.)
    TS_ASSERT_EQUALS(a.getNumBuildings(game::MineBuilding), 15);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::FactoryBuilding), 22);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::DefenseBuilding), 13);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::BaseDefenseBuilding), 0);
    TS_ASSERT_EQUALS(a.costAction().getRemainingAmount(Element::Money), 14);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::MineBuilding).orElse(0), 15);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 22);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 13);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(-1), 14);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(-1), 0);
}

/** Test autobuild.
    The doStandardAutoBuild() function must behave as documented in the normal case.
    This exercises the grouping feature, where equal speeds are built together. */
void
TestGameActionsBuildStructures::testAutoBuildGroup()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

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
    a.doStandardAutoBuild();

    // We have                        100$ 20S
    // Build 5 factories + 5 defense:  35$ 10S
    // Build 3 mines:                  23$  7S
    // Build 2 factories + 2 defense:   0$  0S
    // (all on top of the 10 we already have.)
    TS_ASSERT_EQUALS(a.getNumBuildings(game::MineBuilding), 13);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::FactoryBuilding), 17);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::DefenseBuilding), 17);
    TS_ASSERT_EQUALS(a.getNumBuildings(game::BaseDefenseBuilding), 0);
    TS_ASSERT_EQUALS(a.costAction().getRemainingAmount(Element::Money), 0);

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::MineBuilding).orElse(0), 13);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::FactoryBuilding).orElse(0), 17);
    TS_ASSERT_EQUALS(h.planet.getNumBuildings(game::DefenseBuilding).orElse(0), 17);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(-1), 0);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Supplies).orElse(-1), 0);
}

/** Test build failure (resources exceeded).
    commit() must throw an exception. */
void
TestGameActionsBuildStructures::testBuildFailure()
{
    TestHarness h;
    game::actions::BuildStructures a(h.planet, h.container, h.config);

    // Add 15 defense. These cost 150$ which we do not have
    TS_ASSERT_EQUALS(a.add(game::DefenseBuilding, 15, false), 15);
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "15S 150$");

    // Invalid
    TS_ASSERT_EQUALS(a.isValid(), false);
    TS_ASSERT_THROWS(a.commit(), game::Exception);

    // Reverting must make it valid again
    TS_ASSERT_EQUALS(a.add(game::DefenseBuilding, -100, true), -15);
    TS_ASSERT_EQUALS(a.isValid(), true);
    TS_ASSERT_THROWS_NOTHING(a.commit());
}

