/**
  *  \file u/t_game_interface_drawingproperty.cpp
  *  \brief Test for game::interface::DrawingProperty
  */

#include <memory>
#include <stdexcept>
#include "game/interface/drawingproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"

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

    std::auto_ptr<Value> p;
    p.reset(gi::getDrawingProperty(d, gi::idpColor, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 7);

    p.reset(gi::getDrawingProperty(d, gi::idpComment, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "");

    p.reset(gi::getDrawingProperty(d, gi::idpEndX, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1300);

    p.reset(gi::getDrawingProperty(d, gi::idpEndY, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1400);

    p.reset(gi::getDrawingProperty(d, gi::idpExpire, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 12);

    p.reset(gi::getDrawingProperty(d, gi::idpLocX, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1100);

    p.reset(gi::getDrawingProperty(d, gi::idpLocY, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1200);

    p.reset(gi::getDrawingProperty(d, gi::idpRadius, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpShape, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpTag, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 99);

    p.reset(gi::getDrawingProperty(d, gi::idpTypeString, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "Line");

    p.reset(gi::getDrawingProperty(d, gi::idpTypeCode, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 0);
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

    std::auto_ptr<Value> p;
    p.reset(gi::getDrawingProperty(d, gi::idpColor, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 9);

    p.reset(gi::getDrawingProperty(d, gi::idpComment, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "");

    p.reset(gi::getDrawingProperty(d, gi::idpEndX, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpEndY, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpExpire, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 15);

    p.reset(gi::getDrawingProperty(d, gi::idpLocX, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1500);

    p.reset(gi::getDrawingProperty(d, gi::idpLocY, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 1400);

    p.reset(gi::getDrawingProperty(d, gi::idpRadius, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 220);

    p.reset(gi::getDrawingProperty(d, gi::idpShape, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpTag, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 77);

    p.reset(gi::getDrawingProperty(d, gi::idpTypeString, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "Circle");

    p.reset(gi::getDrawingProperty(d, gi::idpTypeCode, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 2);
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

    std::auto_ptr<Value> p;
    p.reset(gi::getDrawingProperty(d, gi::idpColor, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 11);

    p.reset(gi::getDrawingProperty(d, gi::idpComment, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "note!");

    p.reset(gi::getDrawingProperty(d, gi::idpEndX, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpEndY, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpExpire, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), -1);

    p.reset(gi::getDrawingProperty(d, gi::idpLocX, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 2200);

    p.reset(gi::getDrawingProperty(d, gi::idpLocY, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 2105);

    p.reset(gi::getDrawingProperty(d, gi::idpRadius, cs));
    TS_ASSERT(p.get() == 0);

    p.reset(gi::getDrawingProperty(d, gi::idpShape, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 2);

    p.reset(gi::getDrawingProperty(d, gi::idpTag, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 22);

    p.reset(gi::getDrawingProperty(d, gi::idpTypeString, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "Marker");

    p.reset(gi::getDrawingProperty(d, gi::idpTypeCode, cs));
    TS_ASSERT_EQUALS(Access(p.get()).toInteger(), 3);
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

