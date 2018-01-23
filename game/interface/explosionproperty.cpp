/**
  *  \file game/interface/explosionproperty.cpp
  */

#include "game/interface/explosionproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

afl::data::Value*
game::interface::getExplosionProperty(const game::map::Explosion& expl,
                                      ExplosionProperty iep,
                                      afl::string::Translator& tx,
                                      InterpreterInterface& iface)
{
    game::map::Point pt;
    switch (iep) {
     case iepId:
        /* @q Id:Int (Explosion Property)
           The internal Id of the explosion.
           Can be zero.
           Some explosions have an Id, which PCC2 uses to merge information from multiple sources.
           @since PCC2 2.40.1 */
        return makeIntegerValue(expl.getId());

     case iepShipId:
        /* @q Id.Ship:Int (Explosion Property)
           The Id of the ship that exploded here.
           0 if not known.
           @since PCC2 2.40.1 */
        return makeIntegerValue(expl.getShipId());

     case iepShipName:
        /* @q Name.Ship:Str (Explosion Property)
           The name of the ship that exploded here.
           Empty string if not known.
           @since PCC2 2.40.1 */
        return makeStringValue(expl.getShipName());

     case iepLocX:
        /* @q Loc.X:Int (Explosion Property)
           X location of explosion.
           @since PCC2 2.40.1 */
        if (expl.getPosition(pt)) {
            return makeIntegerValue(pt.getX());
        } else {
            return 0;
        }

     case iepLocY:
        /* @q Loc.Y:Int (Explosion Property)
           Y location of explosion.
           @since PCC2 2.40.1 */
        if (expl.getPosition(pt)) {
            return makeIntegerValue(pt.getY());
        } else {
            return 0;
        }

     case iepName:
        /* @q Name:Str (Explosion Property)
           User-friendly name of this explosion.
           @since PCC2 2.40.1 */
        return makeStringValue(expl.getName(expl.PlainName, tx, iface));

     case iepTypeStr:
        /* @q Type:Str (Explosion Property)
           Always "Explosion" for explosions.
           @see Type (Ship Property)
           @since PCC2 2.40.1 */
        return makeStringValue("Explosion");

     case iepTypeChar:
        /* @q Type.Short:Str (Explosion Property)
           Always "E" for explosions.
           @see Type.Short (Ship Property)
           @since PCC2 2.40.1 */
        return makeStringValue("E");
    }
    return 0;
}
