/**
  *  \file game/interface/cargomethod.hpp
  *  \brief Foundations of cargo-related script commands
  */
#ifndef C2NG_GAME_INTERFACE_CARGOMETHOD_HPP
#define C2NG_GAME_INTERFACE_CARGOMETHOD_HPP

#include "game/map/planet.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Cargo transfer, starting from planet.
        Parses cargospec, target ship Id, flags parameters.
        Executes cargo transfer from planet.
        If requested, sets "CARGO.REMAINDER" variable in process.

        @param pl        Planet
        @param process   Process
        @param args      Parameters
        @param shipList  Ship list
        @param mapConfig Map configuration (for mission update)
        @param turn      Turn (for other units)
        @param root      Root (for host configuration/version)

        @throw interpreter::Error on parser error
        @throw game::Exception on rule violation */
    void doCargoTransfer(game::map::Planet& pl, interpreter::Process& process, interpreter::Arguments& args, const game::spec::ShipList& shipList, const game::map::Configuration& mapConfig, Turn& turn, const Root& root);

    /** Cargo transfer, starting from ship.
        Parses cargospec, target ship Id, flags parameters.
        Executes cargo transfer from ship.
        If requested, sets "CARGO.REMAINDER" variable in process.

        @param sh        Ship
        @param process   Process
        @param args      Parameters
        @param shipList  Ship list
        @param mapConfig Map configuration (for mission update)
        @param turn      Turn (for other units)
        @param root      Root (for host configuration/version)

        @throw interpreter::Error on parser error
        @throw game::Exception on rule violation */
    void doCargoTransfer(game::map::Ship& sh, interpreter::Process& process, interpreter::Arguments& args, const game::spec::ShipList& shipList, const game::map::Configuration& mapConfig, Turn& turn, const Root& root);

    /** Cargo unload from ship.
        Parses cargospec, flags parameters.
        Executes cargo transfer from ship to planet.
        If requested, sets "CARGO.REMAINDER" variable in process.

        @param sh        Ship
        @param reverse   Reverse direction (upload, not unload)
        @param process   Process
        @param args      Parameters
        @param shipList  Ship list
        @param mapConfig Map configuration (for mission update)
        @param turn      Turn (for other units)
        @param root      Root (for host configuration/version)

        @throw interpreter::Error on parser error
        @throw game::Exception on rule violation */
    void doCargoUnload(game::map::Ship& sh, bool reverse, interpreter::Process& process, interpreter::Arguments& args, const game::spec::ShipList& shipList, const game::map::Configuration& mapConfig, Turn& turn, const Root& root);

} }

#endif
