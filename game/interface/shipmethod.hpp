/**
  *  \file game/interface/shipmethod.hpp
  *  \brief Enum game::interface::ShipMethod
  */
#ifndef C2NG_GAME_INTERFACE_SHIPMETHOD_HPP
#define C2NG_GAME_INTERFACE_SHIPMETHOD_HPP

#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Ship method identifier. */
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

    /** Call ship method.
        @param sh        Ship
        @param ism       Method to invoke
        @param args      Parameters
        @param process   Process (for cargo transfer return value, CARGO.REMAINDER)
        @param session   Session (for translator, logger, ship properties)
        @param root      Root (for setShipProperty() which needs it for config)
        @param mapConfig Map configuration (for position lookups, e.g. in cargo transfer setup)
        @param shipList  Ship list (for setShipProperty() which needs it for engines)
        @param turn      Turn (for setShipProperty(), cargo transfer [commands])
        @throw interpreter::Error on error
        @throw game::Exception on error */
    void callShipMethod(game::map::Ship& sh, ShipMethod ism, interpreter::Arguments& args,
                        interpreter::Process& process,
                        Session& session,
                        const Root& root,
                        const game::map::Configuration& mapConfig,
                        const game::spec::ShipList& shipList,
                        Turn& turn);

} }

#endif
