/**
  *  \file game/interface/minefieldproperty.cpp
  *  \brief Enum game::interface::MinefieldProperty
  */

#include "game/interface/minefieldproperty.hpp"
#include "game/parser/binarytransfer.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeStringValue;

afl::data::Value*
game::interface::getMinefieldProperty(const game::map::Minefield& mf, MinefieldProperty imp)
{
    // ex int/if/mineif.h:getMinefieldProperty
    // Fend off invalid minefields
    if (!mf.isValid()) {
        return 0;
    }

    // Regular minefield case
    game::map::Point pt;
    switch (imp) {
     case impId:
        /* @q Id:Int (Minefield Property)
           Id of this minefield. */
        return makeIntegerValue(mf.getId());
     case impEncodedMessage:
        /* @q Message.Encoded:Str (Minefield Property)
           Minefield data, encoded in "VPA Data Transmission" format.
           @since PCC2 2.41 */
        return makeStringValue(game::parser::packBinaryMinefield(mf));
     case impLastScan:
        /* @q LastScan:Int (Minefield Property)
           Turn when minefield was last scanned. */
        return makeIntegerValue(mf.getTurnLastSeen());
     case impLocX:
        /* @q Loc.X:Int (Minefield Property)
           X location of center of minefield. */
        if (mf.getPosition().get(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }
     case impLocY:
        /* @q Loc.Y:Int (Minefield Property)
           Y location of center of minefield. */
        if (mf.getPosition().get(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }
     case impMarked:
        /* @q Marked:Bool (Minefield Property)
           True if minefield is marked. */
        return makeBooleanValue(mf.isMarked());
     case impRadius:
        /* @q Radius:Int (Minefield Property)
           Minefield radius in ly. */
        return makeOptionalIntegerValue(mf.getRadius());
     case impScanType:
        /* @q Scanned:Int (Minefield Property)
           Last reported action on this minefield.
           <table>
            <tr><td width="1">0</td><td width="20">Not scanned this turn</td></tr>
            <tr><td width="1">1</td><td width="20">Laid this turn</td></tr>
            <tr><td width="1">2</td><td width="20">Swept this turn</td></tr>
            <tr><td width="1">3</td><td width="20">Scanned this turn</td></tr>
           </table> */
        return makeIntegerValue(mf.getReason());
     case impTypeCode:
        /* @q Type$:Bool (Minefield Property)
           True if this is a web mine field. */
        return makeBooleanValue(mf.isWeb());
     case impTypeStr:
        /* @q Type:Str (Minefield Property)
           Minefield type, human-readable.
           One of "Web Mines" or "Mines". */
        return makeStringValue(mf.isWeb() ? "Web Mines" : "Mines");
     case impUnits:
        /* @q Units:Int (Minefield Property)
           Number of mine units. */
        return makeIntegerValue(mf.getUnits());
    }
    return 0;
}

void
game::interface::setMinefieldProperty(game::map::Minefield& /*mf*/, MinefieldProperty /*imp*/, const afl::data::Value* /*value*/)
{
    // ex int/if/mineif.h:setMinefieldProperty
    throw interpreter::Error::notAssignable();
}
