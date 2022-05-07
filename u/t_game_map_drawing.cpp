/**
  *  \file u/t_game_map_drawing.cpp
  *  \brief Test for game::map::Drawing
  */

#include "game/map/drawing.hpp"

#include "t_game_map.hpp"
#include "game/map/configuration.hpp"

using game::map::Drawing;
using game::map::Point;

/** Test initialisation and setters/getters. */
void
TestGameMapDrawing::testInit()
{
    // Rectangle
    Drawing rect(Point(2000, 2030), Drawing::RectangleDrawing);
    TS_ASSERT_EQUALS(rect.getType(), Drawing::RectangleDrawing);
    TS_ASSERT_EQUALS(rect.getPos(), Point(2000, 2030));
    TS_ASSERT_EQUALS(rect.getPos2(), Point(2000, 2030));
    TS_ASSERT_EQUALS(rect.getTag(), 0U);
    TS_ASSERT_EQUALS(rect.getComment(), "");
    TS_ASSERT_DIFFERS(rect.getColor(), 0);
    TS_ASSERT_EQUALS(rect.getExpire(), -1);

    rect.setPos(Point(1000, 1100));
    rect.setPos2(Point(1200, 1300));
    rect.setColor(7);
    rect.setTag(999);
    rect.setExpire(42);

    TS_ASSERT_EQUALS(rect.getPos(), Point(1000, 1100));
    TS_ASSERT_EQUALS(rect.getPos2(), Point(1200, 1300));
    TS_ASSERT_EQUALS(rect.getTag(), 999U);
    TS_ASSERT_EQUALS(rect.getColor(), 7);
    TS_ASSERT_EQUALS(rect.getExpire(), 42);

    // Circle
    Drawing circle(Point(2000, 2030), Drawing::CircleDrawing);
    circle.setCircleRadius(50);

    TS_ASSERT_EQUALS(circle.getType(), Drawing::CircleDrawing);
    TS_ASSERT_EQUALS(circle.getPos(), Point(2000, 2030));
    TS_ASSERT_EQUALS(circle.getCircleRadius(), 50);

    // Marker
    Drawing marker(Point(1111, 2222), Drawing::MarkerDrawing);
    marker.setMarkerKind(3);
    marker.setComment("m");

    TS_ASSERT_EQUALS(marker.getType(), Drawing::MarkerDrawing);
    TS_ASSERT_EQUALS(marker.getPos(), Point(1111, 2222));
    TS_ASSERT_EQUALS(marker.getMarkerKind(), 3);
    TS_ASSERT_EQUALS(marker.getComment(), "m");

    // Marker from template
    Drawing m2(Point(777, 888), game::config::MarkerOption::Data(8, 5, "hu"));
    TS_ASSERT_EQUALS(m2.getType(), Drawing::MarkerDrawing);
    TS_ASSERT_EQUALS(m2.getPos(), Point(777, 888));
    TS_ASSERT_EQUALS(m2.getMarkerKind(), 8);
    TS_ASSERT_EQUALS(m2.getColor(), 5);
}

/** Test getDistanceTo(). */
void
TestGameMapDrawing::testDistance()
{
    // Rectangle
    {
        Drawing rect(Point(2000, 2100), Drawing::RectangleDrawing);
        rect.setPos2(Point(2200, 2400));

        // - corners
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2000, 2100)), 0);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2200, 2100)), 0);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2000, 2400)), 0);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2200, 2400)), 0);

        // - edges
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2000, 2300)), 0);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2100, 2400)), 0);

        // - near edge
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(1950, 2100)), 50);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2050, 2300)), 50);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2100, 2300)), 100);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(2150, 2300)), 50);
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(1990, 2300)), 10);

        // - far out
        TS_ASSERT_EQUALS(rect.getDistanceTo(Point(1000, 2300)), 1000);
        TS_ASSERT_DELTA(rect.getDistanceTo(Point(1000, 1000)), 1486.6, 0.01);
    }

    // Line
    {
        Drawing line(Point(2000, 2100), Drawing::LineDrawing);
        line.setPos2(Point(2200, 2400));

        // - ends
        TS_ASSERT_EQUALS(line.getDistanceTo(Point(2000, 2100)), 0);
        TS_ASSERT_EQUALS(line.getDistanceTo(Point(2200, 2400)), 0);
    
        // - point on line
        TS_ASSERT_EQUALS(line.getDistanceTo(Point(2100, 2250)), 0);

        // - in bounding rectangle
        TS_ASSERT_DELTA(line.getDistanceTo(Point(2100, 2200)), 27.735, 0.01);

        // - far out
        TS_ASSERT_DELTA(line.getDistanceTo(Point(2000, 1900)), 200, 0.01);
        TS_ASSERT_DELTA(line.getDistanceTo(Point(2400, 2700)), 360.555, 0.01);
    }

    // Circle
    {
        Drawing circle(Point(2500, 2600), Drawing::CircleDrawing);
        circle.setCircleRadius(50);

        // - on circle
        TS_ASSERT_DELTA(circle.getDistanceTo(Point(2500, 2650)), 0, 0.01);
        TS_ASSERT_DELTA(circle.getDistanceTo(Point(2550, 2600)), 0, 0.01);

        // - in center
        TS_ASSERT_DELTA(circle.getDistanceTo(Point(2500, 2600)), 50, 0.01);

        // - outside
        TS_ASSERT_DELTA(circle.getDistanceTo(Point(2400, 2600)), 50, 0.01);
        TS_ASSERT_DELTA(circle.getDistanceTo(Point(2400, 2500)), 91.42, 0.01);
    }

    // Marker
    {
        Drawing marker(Point(3000, 2000), Drawing::MarkerDrawing);

        TS_ASSERT_DELTA(marker.getDistanceTo(Point(3000, 2000)), 0, 0.01);
        TS_ASSERT_DELTA(marker.getDistanceTo(Point(3200, 2000)), 200, 0.01);
        TS_ASSERT_DELTA(marker.getDistanceTo(Point(3300, 1600)), 500, 0.01);
    }
}

/** Test getDistanceToWrap(). */
void
TestGameMapDrawing::testDistanceWrap()
{
    game::map::Configuration config;
    config.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    // Rectangle (over the seam)
    {
        Drawing rect(Point(2500, 2800), Drawing::RectangleDrawing);
        rect.setPos2(Point(2700, 3100));

        // - corners
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2500, 2800), config), 0);
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2700, 2800), config), 0);
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2500, 3100), config), 0);
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2700, 3100), config), 0);

        // - wrapped
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2500, 1100), config), 0);

        // - wrapped distance
        TS_ASSERT_EQUALS(rect.getDistanceToWrap(Point(2700, 1400), config), 300);
    }

    // Marker
    {
        Drawing marker(Point(2900, 2800), Drawing::MarkerDrawing);

        // hypot(110, 220)
        TS_ASSERT_DELTA(marker.getDistanceToWrap(Point(1010, 1020), config), 245.97, 0.01);
    }
}

