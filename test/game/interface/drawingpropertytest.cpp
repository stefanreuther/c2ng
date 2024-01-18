/**
  *  \file test/game/interface/drawingpropertytest.cpp
  *  \brief Test for game::interface::DrawingProperty
  */

#include "game/interface/drawingproperty.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <stdexcept>

namespace gi = game::interface;
using afl::data::Value;
using afl::data::Access;
using game::map::Drawing;
using game::map::Point;

/** Test getDrawingProperty() for a LineDrawing. */
AFL_TEST("game.interface.DrawingProperty:get:LineDrawing", a)
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1100, 1200), Drawing::LineDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    interpreter::test::verifyNewInteger(a("idpColor"),      gi::getDrawingProperty(d, gi::idpColor,      cs), 7);
    interpreter::test::verifyNewString (a("idpComment"),    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewInteger(a("idpEndX"),       gi::getDrawingProperty(d, gi::idpEndX,       cs), 1300);
    interpreter::test::verifyNewInteger(a("idpEndY"),       gi::getDrawingProperty(d, gi::idpEndY,       cs), 1400);
    interpreter::test::verifyNewInteger(a("idpExpire"),     gi::getDrawingProperty(d, gi::idpExpire,     cs), 12);
    interpreter::test::verifyNewInteger(a("idpLocX"),       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1100);
    interpreter::test::verifyNewInteger(a("idpLocY"),       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1200);
    interpreter::test::verifyNewNull   (a("idpRadius"),     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewNull   (a("idpShape"),      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger(a("idpTag"),        gi::getDrawingProperty(d, gi::idpTag,        cs), 99);
    interpreter::test::verifyNewString (a("idpTypeString"), gi::getDrawingProperty(d, gi::idpTypeString, cs), "Line");
    interpreter::test::verifyNewInteger(a("idpTypeCode"),   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 0);

    interpreter::test::verifyNewString (a("idpEncodedMessage"), gi::getDrawingProperty(d, gi::idpEncodedMessage, cs),
                                        "<<< VPA Data Transmission >>>\n"
                                        "\n"
                                        "OBJECT: Marker\n"
                                        "DATA: -1321271283\n"
                                        "iajbmeeaaleaaaaaimaaimaaaa\n");
}

/** Test getDrawingProperty() for a RectangleDrawing. */
AFL_TEST("game.interface.DrawingProperty:get:RectangleDrawing", a)
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1100, 1200), Drawing::RectangleDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    interpreter::test::verifyNewInteger(a("idpColor"),      gi::getDrawingProperty(d, gi::idpColor,      cs), 7);
    interpreter::test::verifyNewString (a("idpComment"),    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewInteger(a("idpEndX"),       gi::getDrawingProperty(d, gi::idpEndX,       cs), 1300);
    interpreter::test::verifyNewInteger(a("idpEndY"),       gi::getDrawingProperty(d, gi::idpEndY,       cs), 1400);
    interpreter::test::verifyNewInteger(a("idpExpire"),     gi::getDrawingProperty(d, gi::idpExpire,     cs), 12);
    interpreter::test::verifyNewInteger(a("idpLocX"),       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1100);
    interpreter::test::verifyNewInteger(a("idpLocY"),       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1200);
    interpreter::test::verifyNewNull   (a("idpRadius"),     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewNull   (a("idpShape"),      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger(a("idpTag"),        gi::getDrawingProperty(d, gi::idpTag,        cs), 99);
    interpreter::test::verifyNewString (a("idpTypeString"), gi::getDrawingProperty(d, gi::idpTypeString, cs), "Rectangle");
    interpreter::test::verifyNewInteger(a("idpTypeCode"),   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 1);
}

/** Test getDrawingProperty() for a CircleDrawing. */
AFL_TEST("game.interface.DrawingProperty:get:CircleDrawing", a)
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1500, 1400), Drawing::CircleDrawing);
    d.setColor(9);
    d.setTag(77);
    d.setCircleRadius(220);
    d.setExpire(15);

    interpreter::test::verifyNewInteger(a("idpColor"),      gi::getDrawingProperty(d, gi::idpColor,      cs), 9);
    interpreter::test::verifyNewString (a("idpComment"),    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewNull   (a("idpEndX"),       gi::getDrawingProperty(d, gi::idpEndX,       cs));
    interpreter::test::verifyNewNull   (a("idpEndY"),       gi::getDrawingProperty(d, gi::idpEndY,       cs));
    interpreter::test::verifyNewInteger(a("idpExpire"),     gi::getDrawingProperty(d, gi::idpExpire,     cs), 15);
    interpreter::test::verifyNewInteger(a("idpLocX"),       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1500);
    interpreter::test::verifyNewInteger(a("idpLocY"),       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1400);
    interpreter::test::verifyNewInteger(a("idpRadius"),     gi::getDrawingProperty(d, gi::idpRadius,     cs), 220);
    interpreter::test::verifyNewNull   (a("idpShape"),      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger(a("idpTag"),        gi::getDrawingProperty(d, gi::idpTag,        cs), 77);
    interpreter::test::verifyNewString (a("idpTypeString"), gi::getDrawingProperty(d, gi::idpTypeString, cs), "Circle");
    interpreter::test::verifyNewInteger(a("idpTypeCode"),   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 2);
}

/** Test getDrawingProperty() for a MarkerDrawing. */
AFL_TEST("game.interface.DrawingProperty:get:MarkerDrawing", a)
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(2200, 2105), Drawing::MarkerDrawing);
    d.setColor(11);
    d.setTag(22);
    d.setMarkerKind(2);
    d.setExpire(-1);
    d.setComment("note!");

    interpreter::test::verifyNewInteger(a("idpColor"),      gi::getDrawingProperty(d, gi::idpColor,      cs), 11);
    interpreter::test::verifyNewString (a("idpComment"),    gi::getDrawingProperty(d, gi::idpComment,    cs), "note!");
    interpreter::test::verifyNewNull   (a("idpEndX"),       gi::getDrawingProperty(d, gi::idpEndX,       cs));
    interpreter::test::verifyNewNull   (a("idpEndY"),       gi::getDrawingProperty(d, gi::idpEndY,       cs));
    interpreter::test::verifyNewInteger(a("idpExpire"),     gi::getDrawingProperty(d, gi::idpExpire,     cs), -1);
    interpreter::test::verifyNewInteger(a("idpLocX"),       gi::getDrawingProperty(d, gi::idpLocX,       cs), 2200);
    interpreter::test::verifyNewInteger(a("idpLocY"),       gi::getDrawingProperty(d, gi::idpLocY,       cs), 2105);
    interpreter::test::verifyNewNull   (a("idpRadius"),     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewInteger(a("idpShape"),      gi::getDrawingProperty(d, gi::idpShape,      cs), 2);
    interpreter::test::verifyNewInteger(a("idpTag"),        gi::getDrawingProperty(d, gi::idpTag,        cs), 22);
    interpreter::test::verifyNewString (a("idpTypeString"), gi::getDrawingProperty(d, gi::idpTypeString, cs), "Marker");
    interpreter::test::verifyNewInteger(a("idpTypeCode"),   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 3);
}

/** Test setDrawingProperty() for a LineDrawing. */
AFL_TEST("game.interface.DrawingProperty:set:LineDrawing", a)
{
    Drawing d(Point(1100, 1200), Drawing::LineDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    // Set the color
    {
        afl::data::IntegerValue iv(12);
        AFL_CHECK_SUCCEEDS(a("01. idpColor"), gi::setDrawingProperty(d, gi::idpColor, &iv));
        a.checkEqual("02. getColor", d.getColor(), 12);
    }

    // Failure to set color
    {
        afl::data::IntegerValue iv(9999);
        AFL_CHECK_THROWS(a("11. idpColor"), gi::setDrawingProperty(d, gi::idpColor, &iv), std::exception);
        a.checkEqual("12. getColor", d.getColor(), 12);
    }

    // Set color to null
    {
        AFL_CHECK_SUCCEEDS(a("21. idpColor"), gi::setDrawingProperty(d, gi::idpColor, 0));
        a.checkEqual("22. getColor", d.getColor(), 12);
    }

    // Set comment - fails for Line
    {
        afl::data::StringValue sv("hi");
        AFL_CHECK_THROWS(a("31. idpComment"), gi::setDrawingProperty(d, gi::idpComment, &sv), std::exception);
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(77);
        AFL_CHECK_SUCCEEDS(a("41. idpExpire"), gi::setDrawingProperty(d, gi::idpExpire, &iv));
        a.checkEqual("42. getExpire", d.getExpire(), 77);
    }

    // Set radius - fails for Line
    {
        afl::data::IntegerValue iv(90);
        AFL_CHECK_THROWS(a("51. idpRadius"), gi::setDrawingProperty(d, gi::idpRadius, &iv), std::exception);
    }

    // Set shape - fails for Line
    {
        afl::data::IntegerValue iv(1);
        AFL_CHECK_THROWS(a("61. idpShape"), gi::setDrawingProperty(d, gi::idpShape, &iv), std::exception);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(7777);
        AFL_CHECK_SUCCEEDS(a("71. idpTag"), gi::setDrawingProperty(d, gi::idpTag, &iv));
        a.checkEqual("72. getTag", d.getTag(), 7777U);
    }

    // Set type code (failure)
    {
        afl::data::IntegerValue iv(7777);
        AFL_CHECK_THROWS(a("81. idpTypeCode"), gi::setDrawingProperty(d, gi::idpTypeCode, &iv), std::exception);
    }
}

/** Test setDrawingProperty() for a CircleDrawing. */
AFL_TEST("game.interface.DrawingProperty:set:CircleDrawing", a)
{
    Drawing d(Point(1500, 1400), Drawing::CircleDrawing);
    d.setColor(9);
    d.setTag(77);
    d.setCircleRadius(220);
    d.setExpire(15);

    // Set the color
    {
        afl::data::IntegerValue iv(3);
        AFL_CHECK_SUCCEEDS(a("01. idpColor"), gi::setDrawingProperty(d, gi::idpColor, &iv));
        a.checkEqual("02. getColor", d.getColor(), 3);
    }

    // Set comment - fails for circle
    {
        afl::data::StringValue sv("hi");
        AFL_CHECK_THROWS(a("11. idpComment"), gi::setDrawingProperty(d, gi::idpComment, &sv), std::exception);
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(55);
        AFL_CHECK_SUCCEEDS(a("21. idpExpire"), gi::setDrawingProperty(d, gi::idpExpire, &iv));
        a.checkEqual("22. getExpire", d.getExpire(), 55);
    }

    // Set radius
    {
        afl::data::IntegerValue iv(90);
        AFL_CHECK_SUCCEEDS(a("31. idpRadius"), gi::setDrawingProperty(d, gi::idpRadius, &iv));
        a.checkEqual("32. getCircleRadius", d.getCircleRadius(), 90);
    }

    // Set shape - fails for circle
    {
        afl::data::IntegerValue iv(1);
        AFL_CHECK_THROWS(a("41. idpShape"), gi::setDrawingProperty(d, gi::idpShape, &iv), std::exception);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(666);
        AFL_CHECK_SUCCEEDS(a("51. idpTag"), gi::setDrawingProperty(d, gi::idpTag, &iv));
        a.checkEqual("52. getTag", d.getTag(), 666U);
    }
}

/** Test setDrawingProperty() for a MarkerDrawing. */
AFL_TEST("game.interface.DrawingProperty:set:MarkerDrawing", a)
{
    Drawing d(Point(2200, 2105), Drawing::MarkerDrawing);
    d.setColor(11);
    d.setTag(22);
    d.setMarkerKind(2);
    d.setExpire(-1);
    d.setComment("note!");

    // Set the color
    {
        afl::data::IntegerValue iv(3);
        AFL_CHECK_SUCCEEDS(a("01. idpColor"), gi::setDrawingProperty(d, gi::idpColor, &iv));
        a.checkEqual("02. getColor", d.getColor(), 3);
    }

    // Set comment
    {
        afl::data::StringValue sv("hi");
        AFL_CHECK_SUCCEEDS(a("11. idpComment"), gi::setDrawingProperty(d, gi::idpComment, &sv));
        a.checkEqual("12. getComment", d.getComment(), "hi");
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(66);
        AFL_CHECK_SUCCEEDS(a("21. idpExpire"), gi::setDrawingProperty(d, gi::idpExpire, &iv));
        a.checkEqual("22. getExpire", d.getExpire(), 66);
    }

    // Set radius - fails for Marker
    {
        afl::data::IntegerValue iv(90);
        AFL_CHECK_THROWS(a("31. idpRadius"), gi::setDrawingProperty(d, gi::idpRadius, &iv), std::exception);
    }

    // Set shape
    {
        afl::data::IntegerValue iv(1);
        AFL_CHECK_SUCCEEDS(a("41. idpShape"), gi::setDrawingProperty(d, gi::idpShape, &iv));
        a.checkEqual("42. getMarkerKind", d.getMarkerKind(), 1);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(1234);
        AFL_CHECK_SUCCEEDS(a("51. idpTag"), gi::setDrawingProperty(d, gi::idpTag, &iv));
        a.checkEqual("52. getTag", d.getTag(), 1234U);
    }
}
