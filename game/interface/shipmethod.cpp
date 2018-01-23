/**
  *  \file game/interface/shipmethod.cpp
  */

#include "game/interface/shipmethod.hpp"
#include "game/actions/basefixrecycle.hpp"
#include "game/exception.hpp"
#include "game/interface/cargomethod.hpp"
#include "game/interface/objectcommand.hpp"
#include "game/interface/shipproperty.hpp"
#include "game/limits.hpp"
#include "game/map/fleetmember.hpp"
#include "interpreter/values.hpp"

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
                                interpreter::Process& process,
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
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
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
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetFCode
        args.checkArgumentCount(1);
        setShipProperty(sh, ispFCode, args.getNext(), root, shipList, turn);
        break;

     case ismSetEnemy:
        /* @q SetEnemy n:Int (Ship Command)
           Set ship primary enemy. %n is an integer, either 0 (=no enemy) or 1..11 (player).
           @see Enemy$ (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetEnemy
        args.checkArgumentCount(1);
        setShipProperty(sh, ispEnemyId, args.getNext(), root, shipList, turn);
        break;

     case ismSetSpeed:
        /* @q SetSpeed sp:Int (Ship Command)
           Set ship warp speed. %sp is an integer between 0 and 9.
           @see Speed$ (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetSpeed
        args.checkArgumentCount(1);
        setShipProperty(sh, ispSpeedId, args.getNext(), root, shipList, turn);
        break;

     case ismSetName:
        /* @q SetName n:Str (Ship Command)
           Set ship name.
           @see Name (Ship Property)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetName
        args.checkArgumentCount(1);
        setShipProperty(sh, ispName, args.getNext(), root, shipList, turn);
        break;

     case ismSetMission: {
        /* @q SetMission m:Int, Optional i:Int, t:Int (Ship Command)
           Set ship mission.
           %m is the mission number, %i and %t are the Intercept and Tow parameters, respectively.
           @see Mission$ (Ship Property), Mission.Intercept, Mission.Tow
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetMission
        int32_t m = 0, i = 0, t = 0;

        // Evaluate arguments
        args.checkArgumentCount(1, 3);
        if (!interpreter::checkIntegerArg(m, args.getNext(), 0, MAX_NUMBER)) {
            return;
        }
        interpreter::checkIntegerArg(i, args.getNext(), 0, MAX_NUMBER);
        interpreter::checkIntegerArg(t, args.getNext(), 0, MAX_NUMBER);

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
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipFixShip
        args.checkArgumentCount(0);
        setBaseShipyardOrder(sh, turn, FixShipyardAction);
        break;

     case ismRecycleShip:
        /* @q RecycleShip (Ship Command)
           Recycle this ship at the starbase.
           Changes the base's order to recycle this ship.
           @see FixShip (Planet Command)
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipRecycleShip
        args.checkArgumentCount(0);
        setBaseShipyardOrder(sh, turn, RecycleShipyardAction);
        break;

     case ismSetWaypoint: {
        /* @q SetWaypoint x:Int, y:Int (Ship Command)
           Change the ship's waypoint.
           When playing on a wrapped map, this sets the waypoint to move the shortest possible way to the specified coordinates.
           @see MoveTo
           @since PCC 1.0.5, PCC2 1.99.9, PCC2 2.40.1 */
        // ex int/if/shipif.h:IFShipSetWaypoint
        int32_t x, y;
        args.checkArgumentCount(2);
        if (!interpreter::checkIntegerArg(x, args.getNext(), 0, MAX_NUMBER)) {
            return;
        }
        if (!interpreter::checkIntegerArg(y, args.getNext(), 0, MAX_NUMBER)) {
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
     case ismCargoTransfer:
        /* @q CargoTransfer amount:Cargo, target:Int, Optional flags:Any (Ship Command, Planet Command)
           Transfers cargo to a ship.
           %target specifies the target ship Id,
           %cargo is the amount and type to transfer, as a {int:index:type:cargo|cargo set}.
           It may contain negative values to beam cargo back from the target if rules allow.

           The optional third parameter, %options, is a string containing some options:
           - <tt>"O"</tt>: permit overloading the ships.
           - <tt>"S"</tt>: sell supplies on-the-fly (not possible for the ship/ship version of this command).
           - <tt>"N"</tt>: don't generate an error when the cargo can not completely be transferred.
             Instead, sets the variable <tt>{Cargo.Remainder}</tt> accordingly.

           When called from a planet, the third parameter can also contain the Id of a ship to use as a proxy.
           That proxy ship must be owned by you. If the target ship is not owned by you,
           the cargo will first be transferred to the proxy; then to the target, using the ship/ship transporter.

           For example,
           | CargoTransfer 10, "300n"
           transfers 300 kt Neutronium to ship 10.
           It will fail with an error if the current ship doesn't have that much,
           or there is not enough space in ship 10's fuel tank.
           If you use
           | CargoTransfer 10, "300n", "n"
           PCC will transfer as much as possible.
           Assuming that the current ship only has 20 kt fuel (and ship 10 has enough space),
           this will set {Cargo.Remainder} to "280N", because that's the amount that was not transferred.
           To test for a successful transfer, you can then use
           | If Not Cargo.Remainder Then Print "Successful."

           Though you seem to be able to call this command beam cargo off foreign ships,
           this can only be used to cancel pending transfer orders.
           That is, if ship 355 is a foreign one and ship 10 is yours: for
           | With Ship(355) Do CargoTransfer 10, "10n"
           to succeed, you must have transferred 10 kt Neutronium there using the ship/enemy-ship transporter,
           e.g. with the command
           | With Ship(10) Do CargoTransfer 355, "10n"
           The game rules do not allow asking foreign ships for stuff; the other ship must explicitly send it.
           @see CargoUnload, CargoUpload, CargoTransferWait
           @since PCC 1.0.10, PCC2 1.99.12, PCC2 2.40.3
           @diff The "proxy" ability is present in PCC 1.0.10, and PCC2 2.40.3 (PCC2ng), but not in PCC2. */
        doCargoTransfer(sh, process, args, session, *turn, *root);
        break;

     case ismCargoUnload:
        /* @q CargoUnload amount:Cargo, Optional flags:Str (Ship Command)
           Unload cargo to planet (or jettison).
           %cargo is the amount and type to transfer, as a {int:index:type:cargo|cargo set}.

           The optional second parameter, %options, is a string containing some options:
           - <tt>"O"</tt>: permit overloading the ships.
           - <tt>"S"</tt>: sell supplies on-the-fly.
           - <tt>"N"</tt>: don't generate an error when the cargo can not completely be transferred.
             Instead, sets the variable <tt>{Cargo.Remainder}</tt> accordingly.
           - <tt>"J"</tt>: permit jettison. By default, this command will fail if the ship does not orbit a planet.

           This command is equivalent to
           | CargoUpload CMul(amount, -1), flags
           @see CargoUpload, CargoTransfer
           @since PCC 1.0.10, PCC2 1.99.12, PCC2 2.40.3 */
        // ex IFShipCargoUnload
        doCargoUnload(sh, false, process, args, session, *turn, *root);
        break;

     case ismCargoUpload:
        /* @q CargoUpload amount:Cargo, Optional flags:Str (Ship Command)
           Load cargo from planet.
           %amount is the amount and type to transfer, as a {int:index:type:cargo|cargo set}.

           The optional second parameter, %options, is a string containing some options:
           - <tt>"O"</tt>: permit overloading the ships.
           - <tt>"S"</tt>: sell supplies on-the-fly.
           - <tt>"N"</tt>: don't generate an error when the cargo can not completely be transferred.
             Instead, sets the variable <tt>{Cargo.Remainder}</tt> accordingly.
           - <tt>"J"</tt>: permit jettison. By default, this command will fail if the ship does not orbit a planet.
             Use this option if you want to use %CargoUpload to get back cargo you jettisoned earlier this turn.
           @see CargoUnload, CargoTransfer, CargoUploadWait
           @since PCC 1.0.10, PCC2 1.99.12, PCC2 2.40.3 */
        // ex IFShipCargoUpload
        doCargoUnload(sh, true, process, args, session, *turn, *root);
        break;

     case ismSetFleet:
        /* @q SetFleet fid:Int (Ship Command)
           Sets the fleet this ship is member of.
           %fid can be one of the following:
           - 0 - the ship is not member of any fleet.
           - the ship's id - the ship is in a fleet alone.
           - a fleet id - the ship becomes member of the specified fleet.

           If you're putting the ship into a fleet, but it is already member of a different one,
           it leaves its old fleet first (as if you had written <tt>SetFleet 0</tt>).
           This makes a difference when you're putting a fleet leader into another fleet.
           @see Fleet$ (Ship Property)
           @since PCC 1.0.13, PCC2 1.99.17, PCC2 2.40.3 */
        // ex IFShipSetFleet
        args.checkArgumentCount(1);
        setShipProperty(sh, ispFleetId, args.getNext(), root, shipList, turn);
        break;
    }
}
