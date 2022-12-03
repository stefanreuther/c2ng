/**
  *  \file game/interface/ionstormproperty.cpp
  */

#include "game/interface/ionstormproperty.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "game/tables/headingname.hpp"
#include "afl/string/format.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::makeBooleanValue;
using interpreter::makeStringValue;

afl::data::Value*
game::interface::getIonStormProperty(const game::map::IonStorm& ion, IonStormProperty iip, afl::string::Translator& tx, InterpreterInterface& iface)
{
    // ex int/if/ionif.h:getIonProperty
    if (!ion.isActive()) {
        return 0;
    }

    int n;
    game::map::Point pt;
    switch (iip) {
     case iipClass:
        /* @q Class:Int (Storm Property)
           Ion storm's class. */
        return makeOptionalIntegerValue(ion.getClass());
     case iipHeadingInt:
        /* @q Heading$:Int (Storm Property)
           Ion storm's heading, in degrees. */
        return makeOptionalIntegerValue(ion.getHeading());
     case iipHeadingName:
        /* @q Heading:Str (Storm Property)
           Ion storm's heading, as compass point.
           For example, "NE" for north-east. */
        return makeOptionalStringValue(game::tables::HeadingName()(ion.getHeading()));
     case iipId:
        /* @q Id:Int (Storm Property)
           Ion storm's Id. */
        return makeIntegerValue(ion.getId());
     case iipLocX:
        /* @q Loc.X:Int (Storm Property)
           Ion storm center X coordinate. */
        if (ion.getPosition().get(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     case iipLocY:
        /* @q Loc.Y:Int (Storm Property)
           Ion storm center Y coordinate. */
        if (ion.getPosition().get(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     case iipMarked:
        /* @q Marked:Bool (Storm Property)
           True if ion storm is marked. */
        return makeBooleanValue(ion.isMarked());
     case iipName:
        /* @q Name:Str (Storm Property)
           Ion storm name. */
        return makeStringValue(ion.getName(PlainName, tx, iface));
     case iipRadius:
        /* @q Radius:Int (Storm Property)
           Ion storm radius in ly. */
        return makeOptionalIntegerValue(ion.getRadius());
     case iipSpeedInt:
        /* @q Speed$:Int (Storm Property)
           Ion storm speed (warp factor). */
        return makeOptionalIntegerValue(ion.getSpeed());
     case iipSpeedName:
        /* @q Speed:Str (Storm Property)
           Ion storm speed, as human-readable string. */
        if (ion.getSpeed().get(n)) {
            return makeStringValue(afl::string::Format("Warp %d", n));
        } else {
            return 0;
        }
     case iipStatusFlag:
        /* @q Status$:Bool (Storm Property)
           Ion storm status.
           - True if storm is growing
           - False if storm is weakening */
        return makeBooleanValue(ion.isGrowing());
     case iipStatusName:
        /* @q Status:Str (Storm Property)
           Ion storm status, as human-readable string. */
        return makeStringValue(ion.isGrowing() ? "Growing" : "Weakening");
     case iipVoltage:
        /* @q Voltage:Int (Storm Property)
           Ion storm voltage, in MeV. */
        return makeOptionalIntegerValue(ion.getVoltage());
    }
    return 0;
}

void
game::interface::setIonStormProperty(game::map::IonStorm& /*ion*/, IonStormProperty /*iip*/, const afl::data::Value* /*value*/)
{
    // ex int/if/ionif.h:setIonProperty
    throw interpreter::Error::notAssignable();
}
