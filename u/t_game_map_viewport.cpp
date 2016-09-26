/**
  *  \file u/t_game_map_viewport.cpp
  *  \brief Test for game::map::Viewport
  */

#include "game/map/viewport.hpp"

#include "t_game_map.hpp"
#include "game/map/universe.hpp"

/** Test rectangle methods. */
void
TestGameMapViewport::testRectangle()
{
    using game::map::Point;
    game::map::Universe univ;
    game::TeamSettings teams;
    game::map::Viewport t(univ, teams);
    t.setRange(Point(100, 100), Point(200, 300));

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
