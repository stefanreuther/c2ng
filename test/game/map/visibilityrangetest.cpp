/**
  *  \file test/game/map/visibilityrangetest.cpp
  *  \brief Test for game::map::VisibilityRange
  */

#include "game/map/visibilityrange.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/rangeset.hpp"
#include "game/test/simpleturn.hpp"
#include <algorithm>

using afl::base::Ref;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using game::map::Point;

/** Test toString().
    A: call toString() for all values
    E: result must be non-empty for all values */
AFL_TEST("game.map.VisibilityRange:toString", a)
{
    for (int i = 0; i <= game::map::VisModeMax; ++i) {
        afl::string::NullTranslator tx;
        a.checkDifferent("", toString(game::map::VisMode(i), tx), "");
    }
}

/** Test getVisibilityRangeSettings().
    A: set up a configuration with ScanRange=33 for one race.
    E: result must include at least one setting with range=33. All settings must have a name. */
AFL_TEST("game.map.VisibilityRange:getVisibilityRangeSettings", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::ScanRange].set("10,10,10,33,10,10");
    afl::string::NullTranslator tx;

    game::map::VisSettings_t result = game::map::getVisibilityRangeSettings(*config, 4, tx);

    a.checkDifferent("01. size", result.size(), 0U);
    bool found = false;
    for (size_t i = 0; i < result.size(); ++i) {
        a.checkDifferent("02. name", result[i].name, "");
        if (result[i].range == 33) {
            found = true;
        }
    }
    a.check("03. found", found);
}

/** Test buildVisibilityRange().
    A: define some units. Exercise buildVisibilityRange with different options.
    E: correct result */
AFL_TEST("game.map.VisibilityRange:buildVisibilityRange", a)
{
    game::test::SimpleTurn t;
    // One own planet
    t.setPosition(Point(1000, 1000));
    t.addPlanet(1, 1, game::map::Object::ReadOnly);
    // Two allied planets
    for (int i = 0; i < 2; ++i) {
        t.setPosition(Point(2000, 1000+500*i));
        t.addPlanet(10+i, 2, game::map::Object::ReadOnly);
    }
    // Four own ships
    for (int i = 0; i < 4; ++i) {
        t.setPosition(Point(3000, 1000+500*i));
        t.addShip(20+i, 1, game::map::Object::ReadOnly);
    }
    // Eight enemy ships
    for (int i = 0; i < 8; ++i) {
        t.setPosition(Point(4000, 1000+500*i));
        t.addShip(30+i, 3, game::map::Object::ReadOnly);
    }

    game::TeamSettings team;
    team.setPlayerTeam(1, 2);
    team.setViewpointPlayer(1);

    // Own, no allies - 1+4
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeOwn, 100, false), team);
        a.checkEqual("01. own, no allies", std::distance(out.begin(), out.end()), 5U);
    }

    // Own, with allies - 1+2+4
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeOwn, 100, true), team);
        a.checkEqual("11. own, with allies", std::distance(out.begin(), out.end()), 7U);
    }

    // Own ships - 4
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeShips, 100, false), team);
        a.checkEqual("21. own ships", std::distance(out.begin(), out.end()), 4U);
    }

    // Own planets - 1
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModePlanets, 100, false), team);
        a.checkEqual("31. own planets", std::distance(out.begin(), out.end()), 1U);
    }

    // Mark some; test
    t.universe().planets().get(1)->setIsMarked(true);
    t.universe().ships().get(32)->setIsMarked(true);
    t.universe().ships().get(21)->setIsMarked(true);
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeMarked, 100, false), team);
        a.checkEqual("41. marked", std::distance(out.begin(), out.end()), 3U);
    }
}

/** Test load/save.
    A: loadVisibilityConfiguration() from empty config. saveVisibilityConfiguration() with defined settings, reload.
    E: correct defaults loaded; saved values correctly restored */
AFL_TEST("game.map.VisibilityRange:config", a)
{
    // Load
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;
    game::map::VisConfig vc = game::map::loadVisibilityConfiguration(pref);
    a.checkEqual("01. rang",    vc.range, 0);
    a.checkEqual("02. mode",    vc.mode, game::map::VisModeOwn);
    a.checkEqual("03. useTeam", vc.useTeam, false);

    // Save
    game::map::saveVisibilityConfiguration(pref, game::map::VisConfig(game::map::VisModeMarked, 49, true));

    // Re-load
    vc = game::map::loadVisibilityConfiguration(pref);
    a.checkEqual("11. range",   vc.range, 49);
    a.checkEqual("12. mode",    vc.mode, game::map::VisModeMarked);
    a.checkEqual("13. useTeam", vc.useTeam, true);

    // Verify serialisation
    game::config::ConfigurationOption* opt;
    opt = pref.getOptionByName("Chart.Range.Mode");
    a.checkNonNull("21. opt", opt);
    a.checkEqual("22. toString", opt->toString(), "Marked");

    opt = pref.getOptionByName("Chart.Range.Distance");
    a.checkNonNull("31. opt", opt);
    a.checkEqual("32. toString", opt->toString(), "49");

    opt = pref.getOptionByName("Chart.Range.Team");
    a.checkNonNull("41. opt", opt);
    a.checkEqual("42. toString", opt->toString(), "Yes");
}
