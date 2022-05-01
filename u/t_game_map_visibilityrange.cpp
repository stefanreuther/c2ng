/**
  *  \file u/t_game_map_visibilityrange.cpp
  *  \brief Test for game::map::VisibilityRange
  */

#include <algorithm>
#include "game/map/visibilityrange.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/rangeset.hpp"
#include "game/test/simpleturn.hpp"

using game::map::Point;

/** Test toString().
    A: call toString() for all values
    E: result must be non-empty for all values */
void
TestGameMapVisibilityRange::testToString()
{
    for (int i = 0; i <= game::map::VisModeMax; ++i) {
        afl::string::NullTranslator tx;
        TS_ASSERT_DIFFERS(toString(game::map::VisMode(i), tx), "");
    }
}

/** Test getVisibilityRangeSettings().
    A: set up a configuration with ScanRange=33 for one race.
    E: result must include at least one setting with range=33. All settings must have a name. */
void
TestGameMapVisibilityRange::testGetVisibilityRangeSettings()
{
    game::config::HostConfiguration config;
    config[game::config::HostConfiguration::ScanRange].set("10,10,10,33,10,10");
    afl::string::NullTranslator tx;

    game::map::VisSettings_t result = game::map::getVisibilityRangeSettings(config, 4, tx);

    TS_ASSERT_DIFFERS(result.size(), 0U);
    bool found = false;
    for (size_t i = 0; i < result.size(); ++i) {
        TS_ASSERT_DIFFERS(result[i].name, "");
        if (result[i].range == 33) {
            found = true;
        }
    }
    TS_ASSERT(found);
}

/** Test buildVisibilityRange().
    A: define some units. Exercise buildVisibilityRange with different options.
    E: correct result */
void
TestGameMapVisibilityRange::testBuildVisibilityRange()
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
        TS_ASSERT_EQUALS(std::distance(out.begin(), out.end()), 5U);
    }

    // Own, with allies - 1+2+4
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeOwn, 100, true), team);
        TS_ASSERT_EQUALS(std::distance(out.begin(), out.end()), 7U);
    }

    // Own ships - 4
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeShips, 100, false), team);
        TS_ASSERT_EQUALS(std::distance(out.begin(), out.end()), 4U);
    }

    // Own planets - 1
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModePlanets, 100, false), team);
        TS_ASSERT_EQUALS(std::distance(out.begin(), out.end()), 1U);
    }

    // Mark some; test
    t.universe().planets().get(1)->setIsMarked(true);
    t.universe().ships().get(32)->setIsMarked(true);
    t.universe().ships().get(21)->setIsMarked(true);
    {
        game::map::RangeSet out;
        buildVisibilityRange(out, t.universe(), game::map::VisConfig(game::map::VisModeMarked, 100, false), team);
        TS_ASSERT_EQUALS(std::distance(out.begin(), out.end()), 3U);
    }
}

/** Test load/save.
    A: loadVisibilityConfiguration() from empty config. saveVisibilityConfiguration() with defined settings, reload.
    E: correct defaults loaded; saved values correctly restored */
void
TestGameMapVisibilityRange::testLoadSave()
{
    // Load
    game::config::UserConfiguration pref;
    game::map::VisConfig vc = game::map::loadVisibilityConfiguration(pref);
    TS_ASSERT_EQUALS(vc.range, 0);
    TS_ASSERT_EQUALS(vc.mode, game::map::VisModeOwn);
    TS_ASSERT_EQUALS(vc.useTeam, false);

    // Save
    game::map::saveVisibilityConfiguration(pref, game::map::VisConfig(game::map::VisModeMarked, 49, true));

    // Re-load
    vc = game::map::loadVisibilityConfiguration(pref);
    TS_ASSERT_EQUALS(vc.range, 49);
    TS_ASSERT_EQUALS(vc.mode, game::map::VisModeMarked);
    TS_ASSERT_EQUALS(vc.useTeam, true);

    // Verify serialisation
    game::config::ConfigurationOption* opt;
    opt = pref.getOptionByName("Chart.Range.Mode");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "Marked");

    opt = pref.getOptionByName("Chart.Range.Distance");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "49");

    opt = pref.getOptionByName("Chart.Range.Team");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "Yes");
}

