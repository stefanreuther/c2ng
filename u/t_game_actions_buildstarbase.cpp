/**
  *  \file u/t_game_actions_buildstarbase.cpp
  *  \brief Test for game::actions::BuildStarbase
  */

#include "game/actions/buildstarbase.hpp"

#include "t_game_actions.hpp"
#include "game/test/cargocontainer.hpp"
#include "game/exception.hpp"
#include "game/element.hpp"
#include "game/map/planetstorage.hpp"

using game::Element;

namespace {
    const int OWNER = 5;

    game::map::Planet& preparePlanet(game::map::Planet& p)
    {
        p.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(OWNER));
        p.setOwner(OWNER);
        p.setPosition(game::map::Point(1122, 3344));
        p.setCargo(Element::Money, 1000);
        p.setCargo(Element::Tritanium, 1000);
        p.setCargo(Element::Duranium, 1000);
        p.setCargo(Element::Molybdenum, 1000);
        p.setPlayability(game::map::Object::Playable);
        return p;
    }

    struct TestHarness {
        game::config::HostConfiguration config;
        game::map::Planet planet;
        game::map::PlanetStorage container;

        TestHarness();
    };

    TestHarness::TestHarness()
        : config(),
          planet(99),
          container(preparePlanet(planet), config)
    {
        config.setDefaultValues();
    }
}

/** Test error case.
    A BuildStarbase action must reject being constructed on a planet not being played. */
void
TestGameActionsBuildStarbase::testError()
{
    game::map::Planet somePlanet(77);
    game::test::CargoContainer container;
    game::config::HostConfiguration config;

    TS_ASSERT_THROWS((game::actions::BuildStarbase(somePlanet, container, true, config)), game::Exception);
}

/** Test null operation.
    Constructing a BuildStarbase when there's nothing to do must fail. */
void
TestGameActionsBuildStarbase::testErrorNullOp()
{
    TestHarness h;
    TS_ASSERT_THROWS((game::actions::BuildStarbase(h.planet, h.container, false, h.config)), game::Exception);
}

/** Test normal case.
    If the BuildStarbase is used normally, it must convert resources into a starbase flag. */
void
TestGameActionsBuildStarbase::testNormal()
{
    TestHarness h;
    game::actions::BuildStarbase a(h.planet, h.container, true, h.config);

    // Verify cost
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "402T 120D 340M 900$");

    // Commit
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Tritanium).orElse(0),  1000 - 402);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Duranium).orElse(0),   1000 - 120);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Molybdenum).orElse(0), 1000 - 340);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(0),      1000 - 900);
    TS_ASSERT_EQUALS(h.planet.isBuildingBase(), true);
}

/** Test modification during transaction.
    If a parallel action builds a base, the BuildStarbase must not bill again. */
void
TestGameActionsBuildStarbase::testModify()
{
    TestHarness h;
    game::actions::BuildStarbase a(h.planet, h.container, true, h.config);

    // Parallel action
    h.planet.setBuildBaseFlag(true);

    // Commit. Must not deduct cash.
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Tritanium).orElse(0),  1000);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Duranium).orElse(0),   1000);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Molybdenum).orElse(0), 1000);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(0),      1000);
    TS_ASSERT_EQUALS(h.planet.isBuildingBase(), true);
}

/** Test config change during transaction.
    Changed configuration must be taken into account when committing. */
void
TestGameActionsBuildStarbase::testConfigChange()
{
    TestHarness h;
    game::actions::BuildStarbase a(h.planet, h.container, true, h.config);

    // Parallel action
    h.config[game::config::HostConfiguration::StarbaseCost].set("T100 D100 M100");

    // Commit. Must deduct new config value.
    TS_ASSERT_THROWS_NOTHING(a.commit());
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Tritanium).orElse(0),   900);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Duranium).orElse(0),    900);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Molybdenum).orElse(0),  900);
    TS_ASSERT_EQUALS(h.planet.getCargo(Element::Money).orElse(0),      1000);
    TS_ASSERT_EQUALS(h.planet.isBuildingBase(), true);
}

/** Test config change with signal.
    Changed configuration must be taken into account when committing. */
void
TestGameActionsBuildStarbase::testConfigChangeSignal()
{
    TestHarness h;
    game::actions::BuildStarbase a(h.planet, h.container, true, h.config);

    // Parallel action
    h.config[game::config::HostConfiguration::StarbaseCost].set("T100 D100 M100");
    h.config.notifyListeners();

    // Cost must have been updated
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "100TDM");
}

/** Test building with too expensive starbase.
    Construction of the transaction must succeed, but it cannot be committed. */
void
TestGameActionsBuildStarbase::testTooExpensive()
{
    TestHarness h;
    h.config[game::config::HostConfiguration::StarbaseCost].set("T2000 D100 M100");
    game::actions::BuildStarbase a(h.planet, h.container, true, h.config);

    // Verify
    TS_ASSERT_EQUALS(a.costAction().getCost().toCargoSpecString(), "2000T 100D 100M");
    TS_ASSERT(!a.isValid());
    TS_ASSERT_THROWS(a.commit(), game::Exception);
}

