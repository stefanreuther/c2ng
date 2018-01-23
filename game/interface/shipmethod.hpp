/**
  *  \file game/interface/shipmethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SHIPMETHOD_HPP
#define C2NG_GAME_INTERFACE_SHIPMETHOD_HPP

#include "game/map/ship.hpp"
#include "interpreter/process.hpp"
#include "interpreter/arguments.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    enum ShipMethod {
        ismMark,
        ismUnmark,
        ismSetComment,
        ismSetFCode,
        ismSetEnemy,
        ismSetSpeed,
        ismSetName,
        ismSetMission,
        ismFixShip,
        ismRecycleShip,
        ismSetWaypoint,
        ismCargoTransfer,
        ismCargoUnload,
        ismCargoUpload,
        ismSetFleet
    };

    void callShipMethod(game::map::Ship& sh, ShipMethod ism, interpreter::Arguments& args,
                        interpreter::Process& process,                        // needed for cargo transfer which needs it for CARGO.REMAINDER
                        Session& session,                                     // needed for world()
                        afl::base::Ref<Root> root,                            // needed for setShipProperty() which needs it for config
                        afl::base::Ref<game::spec::ShipList> shipList,        // needed for setShipProperty() which needs it for Engines
                        afl::base::Ref<Turn> turn);                           // needed for setShipProperty() which needs it for universe

} }

#endif
