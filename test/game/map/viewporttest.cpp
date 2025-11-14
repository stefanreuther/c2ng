/**
  *  \file test/game/map/viewporttest.cpp
  *  \brief Test for game::map::Viewport
  */

#include "game/map/viewport.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

using game::map::Point;
using game::map::Viewport;

/** Test rectangle methods. */
AFL_TEST("game.map.Viewport:rectangle", a)
{
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    game::TeamSettings teams;
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::UnitScoreDefinitionList shipScores;
    game::spec::ShipList shipList;
    Viewport t(univ, 7, teams, 0, 0, shipScores, shipList, mapConfig, *config, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    t.setRange(Point(100, 100), Point(200, 300));

    // Verify settings
    a.checkEqual("01. getMin", t.getMin(), Point(100, 100));
    a.checkEqual("02. getMax", t.getMax(), Point(200, 300));
    a.checkEqual("03. teamSettings", &t.teamSettings(), &teams);
    a.checkNull("04. labels", t.labels());
    a.checkEqual("05. getTurnNumber", t.getTurnNumber(), 7);
    a.checkEqual("06. shipScores", &t.shipScores(), &shipScores);
    a.checkEqual("07. shipList", &t.shipList(), &shipList);
    a.checkEqual("08. mapConfig", &t.mapConfiguration(), &mapConfig);
    a.checkEqual("09. hostVersion", t.hostVersion().getKind(), game::HostVersion::PHost);

    // Borders as lines
    a.check("11. containsRectangle", t.containsRectangle(Point(100, 100), Point(200, 100)));
    a.check("12. containsRectangle", t.containsRectangle(Point(100, 300), Point(200, 300)));
    a.check("13. containsRectangle", t.containsRectangle(Point(200, 100), Point(200, 300)));
    a.check("14. containsRectangle", t.containsRectangle(Point(200, 100), Point(200, 300)));

    // Whole area normally and crosswise
    a.check("21. containsRectangle", t.containsRectangle(Point(100, 100), Point(200, 300)));
    a.check("22. containsRectangle", t.containsRectangle(Point(100, 300), Point(200, 100)));
    a.check("23. containsRectangle", t.containsRectangle(Point(200, 300), Point(100, 100)));

    // Just outside
    a.check("31. containsRectangle", !t.containsRectangle(Point(0, 0), Point(99, 99)));
    a.check("32. containsRectangle", !t.containsRectangle(Point(0, 0), Point(99, 200)));
    a.check("33. containsRectangle", !t.containsRectangle(Point(0, 0), Point(200, 99)));

    // Just touching
    a.check("41. containsRectangle", t.containsRectangle(Point(0, 0), Point(100, 100)));

    // Inside-out
    a.check("51. containsRectangle", t.containsRectangle(Point(90, 110), Point(130, 120)));

    // Single dot
    a.check("61. containsRectangle", t.containsRectangle(Point(150, 250), Point(150, 250)));
}

/** Test option handling. */
AFL_TEST("game.map.Viewport:options", a)
{
    game::map::Universe univ;
    game::map::Configuration mapConfig;
    game::TeamSettings teams;
    afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
    game::UnitScoreDefinitionList shipScores;
    game::spec::ShipList shipList;
    Viewport t(univ, 7, teams, 0, 0, shipScores, shipList, mapConfig, *config, game::HostVersion());

    // Set an option
    t.setOption(Viewport::ShowMessages, true);
    a.check("01. hasOption", t.hasOption(Viewport::ShowMessages));
    a.check("02. getOptions", t.getOptions().contains(Viewport::ShowMessages));

    // Clear an option
    t.setOption(Viewport::ShowMessages, false);
    a.check("11. hasOption", !t.hasOption(Viewport::ShowMessages));
    a.check("12. getOptions", !t.getOptions().contains(Viewport::ShowMessages));

    // Drawing tag filter
    t.setDrawingTagFilter(99);
    a.check("21. isDrawingTagVisible", !t.isDrawingTagVisible(77));
    a.check("22. isDrawingTagVisible", t.isDrawingTagVisible(99));

    // Clear drawing tag filter
    t.clearDrawingTagFilter();
    a.check("31. isDrawingTagVisible", t.isDrawingTagVisible(77));
    a.check("32. isDrawingTagVisible", t.isDrawingTagVisible(99));

    // Ship trail Id
    a.checkEqual("41. getShipTrailId", t.getShipTrailId(), 0);
    t.setShipTrailId(77);
    a.checkEqual("42. getShipTrailId", t.getShipTrailId(), 77);

    // Set ignore-ship Id
    a.checkEqual("51. getShipIgnoreTaskId", t.getShipIgnoreTaskId(), 0);
    t.setShipIgnoreTaskId(33);
    a.checkEqual("52. getShipIgnoreTaskId", t.getShipIgnoreTaskId(), 33);
}
