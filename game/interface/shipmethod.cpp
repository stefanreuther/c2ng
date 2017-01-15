/**
  *  \file game/interface/shipmethod.cpp
  */

#include "game/interface/shipmethod.hpp"
#include "game/interface/shipproperty.hpp"
#include "game/interface/objectcommand.hpp"
#include "interpreter/values.hpp"
#include "game/map/fleetmember.hpp"
#include "game/exception.hpp"
#include "game/actions/basefixrecycle.hpp"

using game::Exception;

namespace {

    void setBaseShipyardOrder(game::map::Ship& ship, afl::base::Ref<game::Turn> turn, game::ShipyardAction action)
    {
        // ex int/if/shipif.h:setBaseOrder, game/action/basefix.cc:checkBaseAction

        // Get universe
        game::map::Universe& univ = turn->universe();

        // Find the planet
        game::map::Point shipPosition;
        if (!ship.getPosition(shipPosition)) {
            throw Exception(Exception::ePos);
        }
        game::map::Planet* planet = univ.planets().get(univ.playedBases().findFirstObjectAt(shipPosition));
        if (!planet) {
            throw Exception(Exception::ePos);
        }

        // Execute
        if (!game::actions::BaseFixRecycle(*planet).set(action, &ship)) {
            throw Exception(Exception::ePerm);
        }
    }
}

void
game::interface::callShipMethod(game::map::Ship& sh, ShipMethod ism, interpreter::Arguments& args,
                                Session& session,
                                afl::base::Ref<Root> root,
                                afl::base::Ref<game::spec::ShipList> shipList,
                                afl::base::Ref<Turn> turn)
{
    // ex int/if/shipif.h:callShipMethod
    switch (ism) {
     case ismMark:
        IFObjMark(sh, args);
        break;

     case ismUnmark:
        IFObjUnmark(sh, args);
        break;

     case ismSetComment:
        /* @q SetComment s:Str (Ship Command)
           Set ship comment.
           @see Comment (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetComment
        args.checkArgumentCount(1);
        if (afl::data::Value* value = args.getNext()) {
            if (afl::data::Segment* seg = session.world().shipProperties().create(sh.getId())) {
                seg->setNew(interpreter::World::sp_Comment, interpreter::makeStringValue(interpreter::toString(value, false)));
            }
            sh.markDirty();
        }
        break;

     case ismSetFCode:
        /* @q SetFCode fc:Str (Ship Command)
           Set ship friendly code.
           @see FCode (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetFCode
        args.checkArgumentCount(1);
        setShipProperty(sh, ispFCode, args.getNext(), root, shipList, turn);
        break;

     case ismSetEnemy:
        /* @q SetEnemy n:Int (Ship Command)
           Set ship primary enemy. %n is an integer, either 0 (=no enemy) or 1..11 (player).
           @see Enemy$ (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetEnemy
        args.checkArgumentCount(1);
        setShipProperty(sh, ispEnemyId, args.getNext(), root, shipList, turn);
        break;

     case ismSetSpeed:
        /* @q SetSpeed sp:Int (Ship Command)
           Set ship warp speed. %sp is an integer between 0 and 9.
           @see Speed$ (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetSpeed
        args.checkArgumentCount(1);
        setShipProperty(sh, ispSpeedId, args.getNext(), root, shipList, turn);
        break;

     case ismSetName:
        /* @q SetName n:Str (Ship Command)
           Set ship name.
           @see Name (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetName
        args.checkArgumentCount(1);
        setShipProperty(sh, ispName, args.getNext(), root, shipList, turn);
        break;

     case ismSetMission: {
        /* @q SetMission m:Int, Optional i:Int, t:Int (Ship Command)
           Set ship mission.
           %m is the mission number, %i and %t are the Intercept and Tow parameters, respectively.
           @see Mission$ (Ship Property), Mission.Intercept, Mission.Tow
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetMission
        int32_t m = 0, i = 0, t = 0;

        // Evaluate arguments
        args.checkArgumentCount(1, 3);
        if (!interpreter::checkIntegerArg(m, args.getNext(), 0, 10000)) {
            return;
        }
        interpreter::checkIntegerArg(i, args.getNext(), 0, 10000);
        interpreter::checkIntegerArg(t, args.getNext(), 0, 10000);

        // Set mission on ship
        if (!sh.isPlayable(game::map::Object::Playable)) {
            throw interpreter::Error::notAssignable();
        }
        if (!game::map::FleetMember(turn->universe(), sh).setMission(m, i, t, root->hostConfiguration(), *shipList)) {
            throw Exception(Exception::eFleet);
        }
        break;
     }

     case ismFixShip:
        /* @q FixShip (Ship Command)
           Repair this ship at the starbase.
           Changes the base's order to repair this ship.
           @see FixShip (Planet Command)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipFixShip
        args.checkArgumentCount(0);
        setBaseShipyardOrder(sh, turn, FixShipyardAction);
        break;

     case ismRecycleShip:
        /* @q RecycleShip (Ship Command)
           Recycle this ship at the starbase.
           Changes the base's order to recycle this ship.
           @see FixShip (Planet Command)
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipRecycleShip
        args.checkArgumentCount(0);
        setBaseShipyardOrder(sh, turn, RecycleShipyardAction);
        break;

     case ismSetWaypoint: {
        /* @q SetWaypoint x:Int, y:Int (Ship Command)
           Change the ship's waypoint.
           When playing on a wrapped map, this sets the waypoint to move the shortest possible way to the specified coordinates.
           @see MoveTo
           @since PCC 1.0.5, PCC2 1.99.9 */
        // ex int/if/shipif.h:IFShipSetWaypoint
        int32_t x, y;
        args.checkArgumentCount(2);
        if (!interpreter::checkIntegerArg(x, args.getNext(), 0, 10000)) {
            return;
        }
        if (!interpreter::checkIntegerArg(y, args.getNext(), 0, 10000)) {
            return;
        }
        if (!sh.isPlayable(game::map::Object::Playable)) {
            throw interpreter::Error::notAssignable();
        }
        if (!game::map::FleetMember(turn->universe(), sh).setWaypoint(game::map::Point(x, y), root->hostConfiguration(), *shipList)) {
            throw Exception(Exception::eFleet);
        }
        break;
     }
     // case ismCargoTransfer:
     // case ismCargoUnload:
     // case ismCargoUpload:
     // case ismSetFleet:
     //    break;
    }
}
