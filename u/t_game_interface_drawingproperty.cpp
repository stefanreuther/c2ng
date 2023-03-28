/**
  *  \file u/t_game_interface_drawingproperty.cpp
  *  \brief Test for game::interface::DrawingProperty
  */

#include <stdexcept>
#include "game/interface/drawingproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/test/valueverifier.hpp"

namespace gi = game::interface;
using afl::data::Value;
using afl::data::Access;
using game::map::Drawing;
using game::map::Point;

/** Test getDrawingProperty() for a LineDrawing. */
void
TestGameInterfaceDrawingProperty::testGetLine()
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1100, 1200), Drawing::LineDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    interpreter::test::verifyNewInteger("idpColor",      gi::getDrawingProperty(d, gi::idpColor,      cs), 7);
    interpreter::test::verifyNewString ("idpComment",    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewInteger("idpEndX",       gi::getDrawingProperty(d, gi::idpEndX,       cs), 1300);
    interpreter::test::verifyNewInteger("idpEndY",       gi::getDrawingProperty(d, gi::idpEndY,       cs), 1400);
    interpreter::test::verifyNewInteger("idpExpire",     gi::getDrawingProperty(d, gi::idpExpire,     cs), 12);
    interpreter::test::verifyNewInteger("idpLocX",       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1100);
    interpreter::test::verifyNewInteger("idpLocY",       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1200);
    interpreter::test::verifyNewNull   ("idpRadius",     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewNull   ("idpShape",      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger("idpTag",        gi::getDrawingProperty(d, gi::idpTag,        cs), 99);
    interpreter::test::verifyNewString ("idpTypeString", gi::getDrawingProperty(d, gi::idpTypeString, cs), "Line");
    interpreter::test::verifyNewInteger("idpTypeCode",   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 0);

    interpreter::test::verifyNewString ("idpEncodedMessage", gi::getDrawingProperty(d, gi::idpEncodedMessage, cs),
                                        "<<< VPA Data Transmission >>>\n"
                                        "\n"
                                        "OBJECT: Marker\n"
                                        "DATA: -1321271283\n"
                                        "iajbmeeaaleaaaaaimaaimaaaa\n");
}

/** Test getDrawingProperty() for a RectangleDrawing. */
void
TestGameInterfaceDrawingProperty::testGetRectangle()
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1100, 1200), Drawing::RectangleDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    interpreter::test::verifyNewInteger("idpColor",      gi::getDrawingProperty(d, gi::idpColor,      cs), 7);
    interpreter::test::verifyNewString ("idpComment",    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewInteger("idpEndX",       gi::getDrawingProperty(d, gi::idpEndX,       cs), 1300);
    interpreter::test::verifyNewInteger("idpEndY",       gi::getDrawingProperty(d, gi::idpEndY,       cs), 1400);
    interpreter::test::verifyNewInteger("idpExpire",     gi::getDrawingProperty(d, gi::idpExpire,     cs), 12);
    interpreter::test::verifyNewInteger("idpLocX",       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1100);
    interpreter::test::verifyNewInteger("idpLocY",       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1200);
    interpreter::test::verifyNewNull   ("idpRadius",     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewNull   ("idpShape",      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger("idpTag",        gi::getDrawingProperty(d, gi::idpTag,        cs), 99);
    interpreter::test::verifyNewString ("idpTypeString", gi::getDrawingProperty(d, gi::idpTypeString, cs), "Rectangle");
    interpreter::test::verifyNewInteger("idpTypeCode",   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 1);
}

/** Test getDrawingProperty() for a CircleDrawing. */
void
TestGameInterfaceDrawingProperty::testGetCircle()
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(1500, 1400), Drawing::CircleDrawing);
    d.setColor(9);
    d.setTag(77);
    d.setCircleRadius(220);
    d.setExpire(15);

    interpreter::test::verifyNewInteger("idpColor",      gi::getDrawingProperty(d, gi::idpColor,      cs), 9);
    interpreter::test::verifyNewString ("idpComment",    gi::getDrawingProperty(d, gi::idpComment,    cs), "");
    interpreter::test::verifyNewNull   ("idpEndX",       gi::getDrawingProperty(d, gi::idpEndX,       cs));
    interpreter::test::verifyNewNull   ("idpEndY",       gi::getDrawingProperty(d, gi::idpEndY,       cs));
    interpreter::test::verifyNewInteger("idpExpire",     gi::getDrawingProperty(d, gi::idpExpire,     cs), 15);
    interpreter::test::verifyNewInteger("idpLocX",       gi::getDrawingProperty(d, gi::idpLocX,       cs), 1500);
    interpreter::test::verifyNewInteger("idpLocY",       gi::getDrawingProperty(d, gi::idpLocY,       cs), 1400);
    interpreter::test::verifyNewInteger("idpRadius",     gi::getDrawingProperty(d, gi::idpRadius,     cs), 220);
    interpreter::test::verifyNewNull   ("idpShape",      gi::getDrawingProperty(d, gi::idpShape,      cs));
    interpreter::test::verifyNewInteger("idpTag",        gi::getDrawingProperty(d, gi::idpTag,        cs), 77);
    interpreter::test::verifyNewString ("idpTypeString", gi::getDrawingProperty(d, gi::idpTypeString, cs), "Circle");
    interpreter::test::verifyNewInteger("idpTypeCode",   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 2);
}

/** Test getDrawingProperty() for a MarkerDrawing. */
void
TestGameInterfaceDrawingProperty::testGetMarker()
{
    afl::charset::Utf8Charset cs;
    Drawing d(Point(2200, 2105), Drawing::MarkerDrawing);
    d.setColor(11);
    d.setTag(22);
    d.setMarkerKind(2);
    d.setExpire(-1);
    d.setComment("note!");

    interpreter::test::verifyNewInteger("idpColor",      gi::getDrawingProperty(d, gi::idpColor,      cs), 11);
    interpreter::test::verifyNewString ("idpComment",    gi::getDrawingProperty(d, gi::idpComment,    cs), "note!");
    interpreter::test::verifyNewNull   ("idpEndX",       gi::getDrawingProperty(d, gi::idpEndX,       cs));
    interpreter::test::verifyNewNull   ("idpEndY",       gi::getDrawingProperty(d, gi::idpEndY,       cs));
    interpreter::test::verifyNewInteger("idpExpire",     gi::getDrawingProperty(d, gi::idpExpire,     cs), -1);
    interpreter::test::verifyNewInteger("idpLocX",       gi::getDrawingProperty(d, gi::idpLocX,       cs), 2200);
    interpreter::test::verifyNewInteger("idpLocY",       gi::getDrawingProperty(d, gi::idpLocY,       cs), 2105);
    interpreter::test::verifyNewNull   ("idpRadius",     gi::getDrawingProperty(d, gi::idpRadius,     cs));
    interpreter::test::verifyNewInteger("idpShape",      gi::getDrawingProperty(d, gi::idpShape,      cs), 2);
    interpreter::test::verifyNewInteger("idpTag",        gi::getDrawingProperty(d, gi::idpTag,        cs), 22);
    interpreter::test::verifyNewString ("idpTypeString", gi::getDrawingProperty(d, gi::idpTypeString, cs), "Marker");
    interpreter::test::verifyNewInteger("idpTypeCode",   gi::getDrawingProperty(d, gi::idpTypeCode,   cs), 3);
}

/** Test setDrawingProperty() for a LineDrawing. */
void
TestGameInterfaceDrawingProperty::testSetLine()
{
    Drawing d(Point(1100, 1200), Drawing::LineDrawing);
    d.setColor(7);
    d.setTag(99);
    d.setPos2(Point(1300, 1400));
    d.setExpire(12);

    // Set the color
    {
        afl::data::IntegerValue iv(12);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpColor, &iv));
        TS_ASSERT_EQUALS(d.getColor(), 12);
    }

    // Failure to set color
    {
        afl::data::IntegerValue iv(9999);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpColor, &iv), std::exception);
        TS_ASSERT_EQUALS(d.getColor(), 12);
    }

    // Set color to null
    {
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpColor, 0));
        TS_ASSERT_EQUALS(d.getColor(), 12);
    }

    // Set comment - fails for Line
    {
        afl::data::StringValue sv("hi");
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpComment, &sv), std::exception);
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(77);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpExpire, &iv));
        TS_ASSERT_EQUALS(d.getExpire(), 77);
    }

    // Set radius - fails for Line
    {
        afl::data::IntegerValue iv(90);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpRadius, &iv), std::exception);
    }

    // Set shape - fails for Line
    {
        afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpShape, &iv), std::exception);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(7777);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpTag, &iv));
        TS_ASSERT_EQUALS(d.getTag(), 7777U);
    }

    // Set type code (failure)
    {
        afl::data::IntegerValue iv(7777);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpTypeCode, &iv), std::exception);
    }
}

/** Test setDrawingProperty() for a CircleDrawing. */
void
TestGameInterfaceDrawingProperty::testSetCircle()
{
    Drawing d(Point(1500, 1400), Drawing::CircleDrawing);
    d.setColor(9);
    d.setTag(77);
    d.setCircleRadius(220);
    d.setExpire(15);

    // Set the color
    {
        afl::data::IntegerValue iv(3);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpColor, &iv));
        TS_ASSERT_EQUALS(d.getColor(), 3);
    }

    // Set comment - fails for circle
    {
        afl::data::StringValue sv("hi");
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpComment, &sv), std::exception);
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(55);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpExpire, &iv));
        TS_ASSERT_EQUALS(d.getExpire(), 55);
    }

    // Set radius
    {
        afl::data::IntegerValue iv(90);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpRadius, &iv));
        TS_ASSERT_EQUALS(d.getCircleRadius(), 90);
    }

    // Set shape - fails for circle
    {
        afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpShape, &iv), std::exception);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(666);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpTag, &iv));
        TS_ASSERT_EQUALS(d.getTag(), 666U);
    }
}

/** Test setDrawingProperty() for a MarkerDrawing. */
void
TestGameInterfaceDrawingProperty::testSetMarker()
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
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpColor, &iv));
        TS_ASSERT_EQUALS(d.getColor(), 3);
    }

    // Set comment
    {
        afl::data::StringValue sv("hi");
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpComment, &sv));
        TS_ASSERT_EQUALS(d.getComment(), "hi");
    }

    // Set expiration date
    {
        afl::data::IntegerValue iv(66);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpExpire, &iv));
        TS_ASSERT_EQUALS(d.getExpire(), 66);
    }

    // Set radius - fails for Marker
    {
        afl::data::IntegerValue iv(90);
        TS_ASSERT_THROWS(gi::setDrawingProperty(d, gi::idpRadius, &iv), std::exception);
    }

    // Set shape
    {
        afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpShape, &iv));
        TS_ASSERT_EQUALS(d.getMarkerKind(), 1);
    }

    // Set tag
    {
        afl::data::IntegerValue iv(1234);
        TS_ASSERT_THROWS_NOTHING(gi::setDrawingProperty(d, gi::idpTag, &iv));
        TS_ASSERT_EQUALS(d.getTag(), 1234U);
    }
}

