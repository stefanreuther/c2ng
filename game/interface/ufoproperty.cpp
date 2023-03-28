/**
  *  \file game/interface/ufoproperty.cpp
  *  \brief Enum game::interface::UfoProperty
  */

#include "game/interface/ufoproperty.hpp"
#include "afl/string/format.hpp"
#include "game/tables/headingname.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::makeStringValue;

using interpreter::checkIntegerArg;
using interpreter::checkBooleanArg;

namespace {
    // ex values.pas:UfoColor
    const uint8_t UFO_COLORS[16] = { 0, 11, 12, 13, 14, 15, 27, 2, 1, 21, 22, 23, 24, 25, 26, 15 };
}

afl::data::Value*
game::interface::getUfoProperty(const game::map::Ufo& ufo, UfoProperty iup,
                                afl::string::Translator& tx,
                                InterpreterInterface& iface)
{
    // ex int/if/ufoif.h:getUfoProperty
    game::map::Point pt;
    int r;
    switch (iup) {
     case iupColorEGA:
        /* @q Color.EGA:Int (Ufo Property)
           Ufo color code.
           This is the value reported by the host, as a value from the standard MS-DOS EGA palette.
           @see Color (Ufo Property) */
        return makeIntegerValue(ufo.getColorCode());
     case iupColorPCC:
        /* @q Color:Int (Ufo Property)
           Ufo color.
           This color is compatible to the {NewLine}, {NewMarker} etc. commands.
           @see Color (Ufo Property) */
        return makeIntegerValue(UFO_COLORS[ufo.getColorCode() & 15]);
     case iupHeadingInt:
        /* @q Heading$:Int (Ufo Property)
           Heading, in degrees. EMPTY if Ufo does not move or heading is not known. */
        return makeOptionalIntegerValue(ufo.getHeading());
     case iupHeadingName:
        /* @q Heading:Str (Ufo Property)
           Heading, as compass direction. */
        // \change we do NOT pass this through NLS like PCC2 does
        return makeOptionalStringValue(game::tables::HeadingName()(ufo.getHeading()));
     case iupId:
        /* @q Id:Int (Ufo Property)
           Ufo Id. */
        return makeIntegerValue(ufo.getId());
     case iupId2:
        /* @q Id2:Int (Ufo Property)
           Real Id number, or 0.
           Some add-ons send their own objects, which may have different Id numbers,
           to Ufos to make them visible to players.
           This field is to support these add-ons.
           Currently, this field is used with PHost's wormholes,
           where it holds the real Id number of the wormhole,
           using the usual rules (even Id = entry, odd Id = exit).
           @assignable */
        return makeIntegerValue(ufo.getRealId());
     case iupInfo1:
        /* @q Info1:Str (Ufo Property), Info2:Str (Ufo Property)
           Description of this Ufo. */
        return makeStringValue(ufo.getInfo1());
     case iupInfo2:
        return makeStringValue(ufo.getInfo2());
     case iupKeepFlag:
        /* @q Keep:Bool (Ufo Property)
           True to keep this Ufo in the database.
           Defaults to False, i.e. the Ufo is only displayed when sent by the host.
           @assignable */
        return makeBooleanValue(ufo.isStoredInHistory());
     case iupLastScan:
        /* @q LastScan:Int (Ufo Property)
           Turn when Ufo was last scanned. */
        return makeIntegerValue(ufo.getLastTurn());
     case iupLocX:
        /* @q Loc.X:Int (Ufo Property)
           X location of Ufo center. */
        if (ufo.getPosition().get(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     case iupLocY:
        /* @q Loc.Y:Int (Ufo Property)
           Y location of Ufo center. */
        if (ufo.getPosition().get(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     case iupMarked:
        /* @q Marked:Bool (Ufo Property)
           True if Ufo is marked. */
        return makeBooleanValue(ufo.isMarked());
     case iupMoveDX:
        /* @q Move.DX:Int (Ufo Property)
           Estimated/average per-turn movement in X direction.
           Used to update guessed positions when the Ufo is not scanned.
           @assignable */
        return makeIntegerValue(ufo.getMovementVector().getX());
     case iupMoveDY:
        /* @q Move.DY:Int (Ufo Property)
           Estimated/average per-turn movement in Y direction.
           Used to update guessed positions when the Ufo is not scanned.
           @assignable */
        return makeIntegerValue(ufo.getMovementVector().getY());
     case iupName:
        /* @q Name:Str (Ufo Property)
           Name of Ufo. */
        return makeStringValue(ufo.getName(PlainName, tx, iface));
     case iupRadius:
        /* @q Radius:Int (Ufo Property)
           Radius of Ufo in ly. */
        return makeOptionalIntegerValue(ufo.getRadius());
     case iupSpeedInt:
        /* @q Speed$:Int (Ufo Property)
           Speed (warp factor). */
        return makeOptionalIntegerValue(ufo.getWarpFactor());
     case iupSpeedName:
        /* @q Speed:Str (Ufo Property)
           Speed, as human-readable string. */
        if (ufo.getWarpFactor().get(r)) {
            return makeStringValue(afl::string::Format(tx.translateString("Warp %d").c_str(), r));
        } else {
            return 0;
        }
     case iupType:
        /* @q Type:Int (Ufo Property)
           Type of Ufo.
           This is an integer reported by the add-on providing the Ufo,
           identifying the Ufo type. */
        return makeOptionalIntegerValue(ufo.getTypeCode());
     case iupVisiblePlanet:
        /* @q Visible.Planet:Int (Ufo Property)
           Distance from which Ufo can be seen from a planet, in ly. */
        return makeOptionalIntegerValue(ufo.getPlanetRange());
     case iupVisibleShip:
        /* @q Visible.Ship:Int (Ufo Property)
           Distance from which Ufo can be seen from a ship, in ly. */
        return makeOptionalIntegerValue(ufo.getShipRange());
    }
    return 0;
}

void
game::interface::setUfoProperty(game::map::Ufo& ufo, UfoProperty iup, const afl::data::Value* value)
{
    // ex int/if/ufoif.h:setUfoProperty
    int32_t iv;
    bool bv;
    switch (iup) {
     case iupId2:
        if (checkIntegerArg(iv, value)) {
            ufo.setRealId(iv);
        }
        break;

     case iupKeepFlag:
        if (checkBooleanArg(bv, value)) {
            ufo.setIsStoredInHistory(bv);
        }
        break;

     case iupMoveDX:
     case iupMoveDY:
        if (checkIntegerArg(iv, value, -500, 500)) {
            game::map::Point pt = ufo.getMovementVector();
            if (iup == iupMoveDX) {
                pt.setX(iv);
            } else {
                pt.setY(iv);
            }
            ufo.setMovementVector(pt);
        }
        break;

     default:
        throw interpreter::Error::notAssignable();
    }
}
