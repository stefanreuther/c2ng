/**
  *  \file game/interface/drawingproperty.cpp
  */

#include "game/interface/drawingproperty.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using game::map::Drawing;

afl::data::Value*
game::interface::getDrawingProperty(const game::map::Drawing& d, DrawingProperty idp)
{
    // ex int/if/drawingif.h:getDrawingProperty
    switch (idp) {
     case idpColor:
        /* @q Color:Int (Drawing Property)
           Color of this drawing.
           @assignable
           @see SetColor (Drawing Command), NewLine, NewCircle, NewRectangle, NewMarker */
        return makeIntegerValue(d.getColor());
     case idpComment:
        /* @q Comment:Str (Drawing Property)
           Comment of this drawing.
           Displayed for markers.
           @assignable
           @see SetComment (Drawing Command) */
        return makeStringValue(d.getComment());
     case idpEndX:
        /* @q End.X:Int (Drawing Property)
           X location of endpoint.
           Valid for lines and rectangles, EMPTY for others. */
        if (d.getType() == Drawing::LineDrawing || d.getType() == Drawing::RectangleDrawing) {
            return makeIntegerValue(d.getPos2().getX());
        } else {
            return 0;
        }
     case idpEndY:
        /* @q End.Y:Int (Drawing Property)
           Y location of endpoint.
           Valid for lines and rectangles, EMPTY for others. */
        if (d.getType() == Drawing::LineDrawing || d.getType() == Drawing::RectangleDrawing) {
            return makeIntegerValue(d.getPos2().getY());
        } else {
            return 0;
        }
     case idpExpire:
        /* @q Expire:Int (Drawing Property)
           Expiration time.
           @assignable
           @see NewLine, NewCircle, NewMarker, NewRectangle */
        return makeIntegerValue(d.getExpire());
     case idpLocX:
        /* @q Loc.X:Int (Drawing Property)
           X location of starting point/center. */
        return makeIntegerValue(d.getPos().getX());
     case idpLocY:
        /* @q Loc.Y:Int (Drawing Property)
           Y location of starting point/center. */
        return makeIntegerValue(d.getPos().getY());
     case idpRadius:
        /* @q Radius:Int (Drawing Property)
           Radius of drawing.
           Valid for circles, EMPTY for others.
           @assignable */
        if (d.getType() == Drawing::CircleDrawing) {
            return makeIntegerValue(d.getCircleRadius());
        } else {
            return 0;
        }
     case idpShape:
        /* @q Shape:Int (Drawing Property)
           Marker shape.
           Valid for markers, EMPTY for others.
           @assignable */
        if (d.getType() == Drawing::MarkerDrawing) {
            return makeIntegerValue(d.getMarkerKind());
        } else {
            return 0;
        }
     case idpTag:
        /* @q Tag:Int (Drawing Property)
           Marker tag.
           Usually an integer created with {Atom()}.
           @assignable */
        return makeIntegerValue(d.getTag());
     case idpTypeString:
        /* @q Type:Str (Drawing Property)
           Type of drawing.
           @see Type$ (Drawing Property) */
        switch (d.getType()) {
         case Drawing::LineDrawing:
            return makeStringValue("Line");
         case Drawing::RectangleDrawing:
            return makeStringValue("Rectangle");
         case Drawing::CircleDrawing:
            return makeStringValue("Circle");
         case Drawing::MarkerDrawing:
            return makeStringValue("Marker");
        }
        return 0;
     case idpTypeCode:
        /* @q Type$:Int (Drawing Property)
           Type of drawing.
           <table align="left">
            <tr><th width="4">Type$</th><th width="5">Type</th></tr>
            <tr><tn>0</tn>              <td>Line</td></tr>
            <tr><tn>1</tn>              <td>Rectangle</td></tr>
            <tr><tn>2</tn>              <td>Circle</td></tr>
            <tr><tn>3</tn>              <td>Marker</td></tr>
           </table> */
        // GDrawingType intentionally matches the chartX.cc type tag,
        // which is what PCC 1.x exposes to the script side, and so do we.
        // FIXME: isolate representation
        return makeIntegerValue(d.getType());
    }
    return 0;
}

void
game::interface::setDrawingProperty(game::map::Drawing& d, DrawingProperty idp, afl::data::Value* value)
{
    // ex setDrawingProperty
    int32_t i;
    String_t s;
    switch (idp) {
     case idpColor:
        if (checkIntegerArg(i, value, 0, Drawing::NUM_USER_COLORS)) {
            d.setColor(i);
        }
        break;
     case idpComment:
        if (checkStringArg(s, value)) {
            if (d.getType() != Drawing::MarkerDrawing) {
                throw interpreter::Error::notAssignable();
            }
            d.setComment(s);
        }
        break;
        // case idpEndX: - FIXME: could be assignable
        // case idpEndY: - FIXME: could be assignable
     case idpExpire:
        if (checkIntegerArg(i, value, -1, 0x7FFF)) {
            d.setExpire(i);
        }
        break;
        // case idpLocX: - FIXME: could be assignable 
        // case idpLocY: - FIXME: could be assignable
     case idpRadius:
        if (checkIntegerArg(i, value, 1, 5000)) {
            if (d.getType() != Drawing::CircleDrawing) {
                throw interpreter::Error::notAssignable();
            }
            d.setCircleRadius(i);
            // FIXME -> getDisplayedTurn().getChartBBox().addDrawing(d);
        }
        break;
     case idpShape:
        if (checkIntegerArg(i, value, 0, 7 /*FIXME: NUM_USER_MARKERS-1*/)) {
            if (d.getType() != Drawing::MarkerDrawing) {
                throw interpreter::Error::notAssignable();
            }
            d.setMarkerKind(i);
        }
        break;
     case idpTag:
        if (checkIntegerArg(i, value, 0, 0xFFFF)) {
            d.setTag(i);
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
