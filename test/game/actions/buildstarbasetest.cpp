/**
  *  \file test/game/actions/buildstarbasetest.cpp
  *  \brief Test for game::actions::BuildStarbase
  */

#include "game/actions/buildstarbase.hpp"

#include "afl/test/testrunner.hpp"
#include "game/element.hpp"
#include "game/exception.hpp"
#include "game/map/planetstorage.hpp"
#include "game/test/cargocontainer.hpp"

using afl::base::Ref;
using game::Element;
using game::config::HostConfiguration;

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
        Ref<HostConfiguration> config;
        game::map::Planet planet;
        game::map::PlanetStorage container;

        TestHarness();
    };

    TestHarness::TestHarness()
        : config(HostConfiguration::create()),
          planet(99),
          container(preparePlanet(planet), *config)
    {
        config->setDefaultValues();
    }
}

/** Test error case.
    A BuildStarbase action must reject being constructed on a planet not being played. */
AFL_TEST("game.actions.BuildStarbase:error:not-played", a)
{
    game::map::Planet somePlanet(77);
    game::test::CargoContainer container;
    Ref<HostConfiguration> config = HostConfiguration::create();

    AFL_CHECK_THROWS(a, (game::actions::BuildStarbase(somePlanet, container, true, *config)), game::Exception);
}

/** Test null operation.
    Constructing a BuildStarbase when there's nothing to do must fail. */
AFL_TEST("game.actions.BuildStarbase:error:null-op", a)
{
    TestHarness h;
    AFL_CHECK_THROWS(a, (game::actions::BuildStarbase(h.planet, h.container, false, *h.config)), game::Exception);
}

/** Test normal case.
    If the BuildStarbase is used normally, it must convert resources into a starbase flag. */
AFL_TEST("game.actions.BuildStarbase:normal", a)
{
    TestHarness h;
    game::actions::BuildStarbase act(h.planet, h.container, true, *h.config);

    // Verify cost
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "402T 120D 340M 900$");

    // Commit
    AFL_CHECK_SUCCEEDS(a("11. commit"), act.commit());
    a.checkEqual("12. tri", h.planet.getCargo(Element::Tritanium).orElse(0),  1000 - 402);
    a.checkEqual("13. dur", h.planet.getCargo(Element::Duranium).orElse(0),   1000 - 120);
    a.checkEqual("14. mol", h.planet.getCargo(Element::Molybdenum).orElse(0), 1000 - 340);
    a.checkEqual("15. mc",  h.planet.getCargo(Element::Money).orElse(0),      1000 - 900);
    a.checkEqual("16. isBuildingBase", h.planet.isBuildingBase(), true);
}

/** Test modification during transaction.
    If a parallel action builds a base, the BuildStarbase must not bill again. */
AFL_TEST("game.actions.BuildStarbase:parallel-modification", a)
{
    TestHarness h;
    game::actions::BuildStarbase act(h.planet, h.container, true, *h.config);

    // Parallel action
    h.planet.setBuildBaseFlag(true);

    // Commit. Must not deduct cash.
    AFL_CHECK_SUCCEEDS(a("01. commit"), act.commit());
    a.checkEqual("02. tri", h.planet.getCargo(Element::Tritanium).orElse(0),  1000);
    a.checkEqual("03. dur", h.planet.getCargo(Element::Duranium).orElse(0),   1000);
    a.checkEqual("04. mol", h.planet.getCargo(Element::Molybdenum).orElse(0), 1000);
    a.checkEqual("05. mc",  h.planet.getCargo(Element::Money).orElse(0),      1000);
    a.checkEqual("06. isBuildingBase", h.planet.isBuildingBase(), true);
}

/** Test config change during transaction.
    Changed configuration must be taken into account when committing. */
AFL_TEST("game.actions.BuildStarbase:config-change", a)
{
    TestHarness h;
    game::actions::BuildStarbase act(h.planet, h.container, true, *h.config);

    // Parallel action
    (*h.config)[HostConfiguration::StarbaseCost].set("T100 D100 M100");

    // Commit. Must deduct new config value.
    AFL_CHECK_SUCCEEDS(a("01. commit"), act.commit());
    a.checkEqual("02. tri", h.planet.getCargo(Element::Tritanium).orElse(0),   900);
    a.checkEqual("03. dur", h.planet.getCargo(Element::Duranium).orElse(0),    900);
    a.checkEqual("04. mol", h.planet.getCargo(Element::Molybdenum).orElse(0),  900);
    a.checkEqual("05. mc",  h.planet.getCargo(Element::Money).orElse(0),      1000);
    a.checkEqual("06. isBuildingBase", h.planet.isBuildingBase(), true);
}

/** Test config change with signal.
    Changed configuration must be taken into account when committing. */
AFL_TEST("game.actions.BuildStarbase:config-change:signal", a)
{
    TestHarness h;
    game::actions::BuildStarbase act(h.planet, h.container, true, *h.config);

    // Parallel action
    (*h.config)[HostConfiguration::StarbaseCost].set("T100 D100 M100");
    h.config->notifyListeners();

    // Cost must have been updated
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "100TDM");
}

/** Test building with too expensive starbase.
    Construction of the transaction must succeed, but it cannot be committed. */
AFL_TEST("game.actions.BuildStarbase:error:no-resources", a)
{
    TestHarness h;
    (*h.config)[HostConfiguration::StarbaseCost].set("T2000 D100 M100");
    game::actions::BuildStarbase act(h.planet, h.container, true, *h.config);

    // Verify
    a.checkEqual("01. getCost", act.costAction().getCost().toCargoSpecString(), "2000T 100D 100M");
    a.check("02. isValid", !act.isValid());
    AFL_CHECK_THROWS(a("03. commit"), act.commit(), game::Exception);
}
