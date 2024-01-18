/**
  *  \file test/game/map/drawingtest.cpp
  *  \brief Test for game::map::Drawing
  */

#include "game/map/drawing.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"

using game::map::Drawing;
using game::map::Point;

/** Test initialisation and setters/getters. */
AFL_TEST("game.map.Drawing:basics", a)
{
    // Rectangle
    Drawing rect(Point(2000, 2030), Drawing::RectangleDrawing);
    a.checkEqual("01. getType",      rect.getType(), Drawing::RectangleDrawing);
    a.checkEqual("02. getPos",       rect.getPos(), Point(2000, 2030));
    a.checkEqual("03. getPos2",      rect.getPos2(), Point(2000, 2030));
    a.checkEqual("04. getTag",       rect.getTag(), 0U);
    a.checkEqual("05. getComment",   rect.getComment(), "");
    a.checkDifferent("06. getColor", rect.getColor(), 0);
    a.checkEqual("07. getExpire",    rect.getExpire(), -1);

    rect.setPos(Point(1000, 1100));
    rect.setPos2(Point(1200, 1300));
    rect.setColor(7);
    rect.setTag(999);
    rect.setExpire(42);

    a.checkEqual("11. getPos",    rect.getPos(), Point(1000, 1100));
    a.checkEqual("12. getPos2",   rect.getPos2(), Point(1200, 1300));
    a.checkEqual("13. getTag",    rect.getTag(), 999U);
    a.checkEqual("14. getColor",  rect.getColor(), 7);
    a.checkEqual("15. getExpire", rect.getExpire(), 42);

    a.check("21. equals", rect.equals(rect));

    // Circle
    Drawing circle(Point(2000, 2030), Drawing::CircleDrawing);
    circle.setCircleRadius(50);

    a.checkEqual("31. getType",         circle.getType(), Drawing::CircleDrawing);
    a.checkEqual("32. getPos",          circle.getPos(), Point(2000, 2030));
    a.checkEqual("33. getCircleRadius", circle.getCircleRadius(), 50);

    a.check("41. equals", !rect.equals(circle));
    a.check("42. equals", circle.equals(circle));

    // Marker
    Drawing marker(Point(1111, 2222), Drawing::MarkerDrawing);
    marker.setMarkerKind(3);
    marker.setComment("m");

    a.checkEqual("51. getType",       marker.getType(), Drawing::MarkerDrawing);
    a.checkEqual("52. getPos",        marker.getPos(), Point(1111, 2222));
    a.checkEqual("53. getMarkerKind", marker.getMarkerKind(), 3);
    a.checkEqual("54. getComment",    marker.getComment(), "m");

    // Marker from template
    Drawing m2(Point(777, 888), game::config::MarkerOption::Data(8, 5, "hu"));
    a.checkEqual("61. getType",       m2.getType(), Drawing::MarkerDrawing);
    a.checkEqual("62. getPos",        m2.getPos(), Point(777, 888));
    a.checkEqual("63. getMarkerKind", m2.getMarkerKind(), 8);
    a.checkEqual("64. getColor",      m2.getColor(), 5);
}

/*
 *  getDistanceTo
 */

// Rectangle
AFL_TEST("game.map.Drawing:getDistanceTo:RectangleDrawing", a)
{
    Drawing rect(Point(2000, 2100), Drawing::RectangleDrawing);
    rect.setPos2(Point(2200, 2400));

    // - corners
    a.checkEqual("01", rect.getDistanceTo(Point(2000, 2100)), 0);
    a.checkEqual("02", rect.getDistanceTo(Point(2200, 2100)), 0);
    a.checkEqual("03", rect.getDistanceTo(Point(2000, 2400)), 0);
    a.checkEqual("04", rect.getDistanceTo(Point(2200, 2400)), 0);

    // - edges
    a.checkEqual("11", rect.getDistanceTo(Point(2000, 2300)), 0);
    a.checkEqual("12", rect.getDistanceTo(Point(2100, 2400)), 0);

    // - near edge
    a.checkEqual("21", rect.getDistanceTo(Point(1950, 2100)), 50);
    a.checkEqual("22", rect.getDistanceTo(Point(2050, 2300)), 50);
    a.checkEqual("23", rect.getDistanceTo(Point(2100, 2300)), 100);
    a.checkEqual("24", rect.getDistanceTo(Point(2150, 2300)), 50);
    a.checkEqual("25", rect.getDistanceTo(Point(1990, 2300)), 10);

    // - far out
    a.checkEqual("31", rect.getDistanceTo(Point(1000, 2300)), 1000);
    a.checkNear("32", rect.getDistanceTo(Point(1000, 1000)), 1486.6, 0.01);
}

// Line
AFL_TEST("game.map.Drawing:getDistanceTo:LineDrawing", a)
{
    Drawing line(Point(2000, 2100), Drawing::LineDrawing);
    line.setPos2(Point(2200, 2400));

    // - ends
    a.checkEqual("01", line.getDistanceTo(Point(2000, 2100)), 0);
    a.checkEqual("02", line.getDistanceTo(Point(2200, 2400)), 0);

    // - point on line
    a.checkEqual("11", line.getDistanceTo(Point(2100, 2250)), 0);

    // - in bounding rectangle
    a.checkNear("21", line.getDistanceTo(Point(2100, 2200)), 27.735, 0.01);

    // - far out
    a.checkNear("31", line.getDistanceTo(Point(2000, 1900)), 200, 0.01);
    a.checkNear("32", line.getDistanceTo(Point(2400, 2700)), 360.555, 0.01);
}

// Line (swapped order of points)
AFL_TEST("game.map.Drawing:getDistanceTo:LineDrawing:swapped", a)
{
    Drawing line(Point(2200, 2400), Drawing::LineDrawing);
    line.setPos2(Point(2000, 2100));

    // - ends
    a.checkEqual("61", line.getDistanceTo(Point(2000, 2100)), 0);
    a.checkEqual("62", line.getDistanceTo(Point(2200, 2400)), 0);

    // - point on line
    a.checkEqual("71", line.getDistanceTo(Point(2100, 2250)), 0);

    // - in bounding rectangle
    a.checkNear("", line.getDistanceTo(Point(2100, 2200)), 27.735, 0.01);

    // - far out
    a.checkNear("", line.getDistanceTo(Point(2000, 1900)), 200, 0.01);
    a.checkNear("", line.getDistanceTo(Point(2400, 2700)), 360.555, 0.01);
}

// Horizontal line
AFL_TEST("game.map.Drawing:getDistanceTo:LineDrawing:horizontal", a)
{
    Drawing line(Point(2000, 2100), Drawing::LineDrawing);
    line.setPos2(Point(2200, 2100));

    // - around first end
    a.checkEqual("01", line.getDistanceTo(Point(2000, 2090)), 10);
    a.checkEqual("02", line.getDistanceTo(Point(1990, 2100)), 10);
    a.checkEqual("03", line.getDistanceTo(Point(2000, 2110)), 10);
    a.checkNear("04", line.getDistanceTo(Point(1990, 2110)), 14.142, 0.01);

    // - around second end
    a.checkEqual("11", line.getDistanceTo(Point(2200, 2090)), 10);
    a.checkEqual("12", line.getDistanceTo(Point(2210, 2100)), 10);
    a.checkEqual("13", line.getDistanceTo(Point(2200, 2110)), 10);
    a.checkNear("14", line.getDistanceTo(Point(2210, 2110)), 14.142, 0.01);

    // - around mid
    a.checkEqual("21", line.getDistanceTo(Point(2100, 2090)), 10);
    a.checkEqual("22", line.getDistanceTo(Point(2100, 2110)), 10);
}

// Circle
AFL_TEST("game.map.Drawing:getDistanceTo:CircleDrawing", a)
{
    Drawing circle(Point(2500, 2600), Drawing::CircleDrawing);
    circle.setCircleRadius(50);

    // - on circle
    a.checkNear("01", circle.getDistanceTo(Point(2500, 2650)), 0, 0.01);
    a.checkNear("02", circle.getDistanceTo(Point(2550, 2600)), 0, 0.01);

    // - in center
    a.checkNear("11", circle.getDistanceTo(Point(2500, 2600)), 50, 0.01);

    // - outside
    a.checkNear("21", circle.getDistanceTo(Point(2400, 2600)), 50, 0.01);
    a.checkNear("22", circle.getDistanceTo(Point(2400, 2500)), 91.42, 0.01);
}

// Marker
AFL_TEST("game.map.Drawing:getDistanceTo:MarkerDrawing", a)
{
    Drawing marker(Point(3000, 2000), Drawing::MarkerDrawing);

    a.checkNear("01", marker.getDistanceTo(Point(3000, 2000)), 0, 0.01);
    a.checkNear("02", marker.getDistanceTo(Point(3200, 2000)), 200, 0.01);
    a.checkNear("03", marker.getDistanceTo(Point(3300, 1600)), 500, 0.01);
}


/*
 *  getDistanceToWrap
 */

// Rectangle (over the seam)
AFL_TEST("game.map.Drawing:getDistanceToWrap:RectangleDrawing", a)
{
    game::map::Configuration config;
    config.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));
    Drawing rect(Point(2500, 2800), Drawing::RectangleDrawing);
    rect.setPos2(Point(2700, 3100));

    // - corners
    a.checkEqual("01", rect.getDistanceToWrap(Point(2500, 2800), config), 0);
    a.checkEqual("02", rect.getDistanceToWrap(Point(2700, 2800), config), 0);
    a.checkEqual("03", rect.getDistanceToWrap(Point(2500, 3100), config), 0);
    a.checkEqual("04", rect.getDistanceToWrap(Point(2700, 3100), config), 0);

    // - wrapped
    a.checkEqual("11", rect.getDistanceToWrap(Point(2500, 1100), config), 0);

    // - wrapped distance
    a.checkEqual("21", rect.getDistanceToWrap(Point(2700, 1400), config), 300);
}

// Marker
AFL_TEST("game.map.Drawing:getDistanceToWrap:MarkerDrawing", a)
{
    game::map::Configuration config;
    config.setConfiguration(game::map::Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));
    Drawing marker(Point(2900, 2800), Drawing::MarkerDrawing);

    // hypot(110, 220)
    a.checkNear("01", marker.getDistanceToWrap(Point(1010, 1020), config), 245.97, 0.01);
}

/*
 *  equals
 */

AFL_TEST("game.map.Drawing:equals", a)
{
    // Two equal lines
    Drawing line(Point(2000, 2100), Drawing::LineDrawing);
    line.setPos2(Point(2200, 2400));

    Drawing line2(Point(2000, 2100), Drawing::LineDrawing);
    line2.setPos2(Point(2200, 2400));

    a.check("01", line.equals(line2));
    a.check("02", line2.equals(line));

    // Modify color
    line.setColor(27);
    a.check("11", !line.equals(line2));
    a.check("12", !line2.equals(line));

    line2.setColor(27);
    a.check("21", line.equals(line2));
    a.check("22", line2.equals(line));

    // Modify tag
    line.setTag(27);
    a.check("31", !line.equals(line2));
    a.check("32", !line2.equals(line));

    line2.setTag(27);
    a.check("41", line.equals(line2));
    a.check("42", line2.equals(line));
}
