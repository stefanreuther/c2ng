/**
  *  \file u/t_game_map_viewport.cpp
  *  \brief Test for game::map::Viewport
  */

#include "game/map/viewport.hpp"

#include "t_game_map.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

using game::map::Point;
using game::map::Viewport;

/** Test rectangle methods. */
void
TestGameMapViewport::testRectangle()
{
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    game::TeamSettings teams;
    game::config::HostConfiguration config;
    game::UnitScoreDefinitionList shipScores;
    game::spec::ShipList shipList;
    Viewport t(univ, 7, teams, 0, shipScores, shipList, mapConfig, config, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    t.setRange(Point(100, 100), Point(200, 300));

    // Verify settings
    TS_ASSERT_EQUALS(t.getMin(), Point(100, 100));
    TS_ASSERT_EQUALS(t.getMax(), Point(200, 300));
    TS_ASSERT_EQUALS(&t.teamSettings(), &teams);
    TS_ASSERT(t.labels() == 0);
    TS_ASSERT_EQUALS(t.getTurnNumber(), 7);
    TS_ASSERT_EQUALS(&t.shipScores(), &shipScores);
    TS_ASSERT_EQUALS(&t.shipList(), &shipList);
    TS_ASSERT_EQUALS(&t.mapConfiguration(), &mapConfig);
    TS_ASSERT_EQUALS(t.hostVersion().getKind(), game::HostVersion::PHost);

    // Borders as lines
    TS_ASSERT(t.containsRectangle(Point(100, 100), Point(200, 100)));
    TS_ASSERT(t.containsRectangle(Point(100, 300), Point(200, 300)));
    TS_ASSERT(t.containsRectangle(Point(200, 100), Point(200, 300)));
    TS_ASSERT(t.containsRectangle(Point(200, 100), Point(200, 300)));

    // Whole area normally and crosswise
    TS_ASSERT(t.containsRectangle(Point(100, 100), Point(200, 300)));
    TS_ASSERT(t.containsRectangle(Point(100, 300), Point(200, 100)));
    TS_ASSERT(t.containsRectangle(Point(200, 300), Point(100, 100)));

    // Just outside
    TS_ASSERT(!t.containsRectangle(Point(0, 0), Point(99, 99)));
    TS_ASSERT(!t.containsRectangle(Point(0, 0), Point(99, 200)));
    TS_ASSERT(!t.containsRectangle(Point(0, 0), Point(200, 99)));

    // Just touching
    TS_ASSERT(t.containsRectangle(Point(0, 0), Point(100, 100)));

    // Inside-out
    TS_ASSERT(t.containsRectangle(Point(90, 110), Point(130, 120)));

    // Single dot
    TS_ASSERT(t.containsRectangle(Point(150, 250), Point(150, 250)));
}

/** Test option handling. */
void
TestGameMapViewport::testOptions()
{
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    game::TeamSettings teams;
    game::config::HostConfiguration config;
    game::UnitScoreDefinitionList shipScores;
    game::spec::ShipList shipList;
    Viewport t(univ, 7, teams, 0, shipScores, shipList, mapConfig, config, game::HostVersion());

    // Set an option
    t.setOption(Viewport::ShowMessages, true);
    TS_ASSERT(t.hasOption(Viewport::ShowMessages));
    TS_ASSERT(t.getOptions().contains(Viewport::ShowMessages));

    // Clear an option
    t.setOption(Viewport::ShowMessages, false);
    TS_ASSERT(!t.hasOption(Viewport::ShowMessages));
    TS_ASSERT(!t.getOptions().contains(Viewport::ShowMessages));

    // Drawing tag filter
    t.setDrawingTagFilter(99);
    TS_ASSERT(!t.isDrawingTagVisible(77));
    TS_ASSERT(t.isDrawingTagVisible(99));

    // Clear drawing tag filter
    t.clearDrawingTagFilter();
    TS_ASSERT(t.isDrawingTagVisible(77));
    TS_ASSERT(t.isDrawingTagVisible(99));

    // Ship trail Id
    t.setShipTrailId(77);
    TS_ASSERT_EQUALS(t.getShipTrailId(), 77);
}

