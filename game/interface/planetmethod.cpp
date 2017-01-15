/**
  *  \file game/interface/planetmethod.cpp
  */

#include "game/interface/planetmethod.hpp"
#include "game/actions/basefixrecycle.hpp"
#include "game/exception.hpp"
#include "game/interface/baseproperty.hpp"
#include "game/interface/objectcommand.hpp"
#include "game/interface/planetproperty.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

using game::Exception;

namespace {
    void setBaseShipyardOrder(game::map::Planet& pl,
                              game::ShipyardAction action,
                              interpreter::Arguments& args,
                              game::Turn& turn)
    {
        // Parse args
        int32_t n;
        args.checkArgumentCount(1);
        if (!interpreter::checkIntegerArg(n, args.getNext())) {
            return;
        }

        // Find associated ship
        game::map::Ship* ship;
        if (n == 0) {
            action = game::NoShipyardAction;
            ship = 0;
        } else {
            ship = turn.universe().ships().get(n);
            if (ship == 0) {
                throw interpreter::Error::rangeError();
            }
        }

        // Try it
        if (!game::actions::BaseFixRecycle(pl).set(action, ship)) {
            throw Exception(Exception::ePerm);
        }
    }
}

void
game::interface::callPlanetMethod(game::map::Planet& pl, PlanetMethod ipm, interpreter::Arguments& args,
                                  Session& session,
                                  Turn& turn,
                                  Root& root)
{
    switch (ipm) {
     case ipmMark:
        IFObjMark(pl, args);
        break;

     case ipmUnmark:
        IFObjUnmark(pl, args);
        break;

     case ipmSetComment:
        /* @q SetComment s:Str (Planet Command)
           Set planet comment.
           @see Comment (Planet Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex IFPlanetSetComment
        args.checkArgumentCount(1);
        if (afl::data::Value* value = args.getNext()) {
            if (afl::data::Segment* seg = session.world().planetProperties().create(pl.getId())) {
                seg->setNew(interpreter::World::pp_Comment, interpreter::makeStringValue(interpreter::toString(value, false)));
            }
            pl.markDirty();
        }
        break;

     case ipmFixShip:
        /* @q FixShip sid:Int (Planet Command)
           Fix (repair) a ship. The %sid is a ship Id, or 0 to cancel a pending shipyard order.
           @since PCC2 1.99.9, PCC 1.0.5 */
        // ex IFBaseFixShip
        setBaseShipyardOrder(pl, FixShipyardAction, args, turn);
        break;

     case ipmRecycleShip:
        /* @q RecycleShip sid:Int (Planet Command)
           Recycle a ship. The %sid is a ship Id, or 0 to cancel a pending shipyard order.
           @since PCC2 1.99.9, PCC 1.0.5 */
        // ex IFBaseRecycleShip
        setBaseShipyardOrder(pl, RecycleShipyardAction, args, turn);
        break;

     // FIXME case ipmBuildBase:
     // FIXME case ipmAutoBuild:
     // FIXME case ipmBuildDefense:
     // FIXME case ipmBuildFactories:
     // FIXME case ipmBuildMines:
     case ipmSetColonistTax:
        /* @q SetColonistTax n:Int (Planet Command)
           Set colonist tax.
           @see Colonists.Tax
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex IFPlanetSetColonistTax
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippColonistTax, args.getNext(), root);
        break;

     case ipmSetNativeTax:
        /* @q SetNativeTax n:Int (Planet Command)
           Set native tax.
           @see Natives.Tax
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex IFPlanetSetNativeTax
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippNativeTax, args.getNext(), root);
        break;

     case ipmSetFCode:
        /* @q SetFCode fc:Str (Planet Command)
           Set planet friendly code.
           @see FCode (Planet Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex IFPlanetSetFCode
        args.checkArgumentCount(1);
        setPlanetProperty(pl, ippFCode, args.getNext(), root);
        break;

     case ipmSetMission:
        /* @q SetMission number:Int (Planet Command)
           Set starbase mission.
           @since PCC2 1.99.9, PCC 1.0.5 */
        // ex IFBaseSetMission
        args.checkArgumentCount(1);
        setBaseProperty(pl, ibpMission, args.getNext());
        break;

     // FIXME case ipmBuildBaseDefense:
     // FIXME case ipmSetTech:
     // FIXME case ipmBuildFighters:
     // FIXME case ipmBuildEngines:
     // FIXME case ipmBuildHulls:
     // FIXME case ipmBuildLaunchers:
     // FIXME case ipmBuildBeams:
     // FIXME case ipmBuildTorps:
     // FIXME case ipmSellSupplies:
     // FIXME case ipmBuildShip:
     // FIXME case ipmCargoTransfer:
     // FIXME case ipmAutoTaxColonists:
     // FIXME case ipmAutoTaxNatives:
    }
}
