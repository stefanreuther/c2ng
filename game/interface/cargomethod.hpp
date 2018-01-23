/**
  *  \file game/interface/cargomethod.hpp
  */
#ifndef C2NG_GAME_INTERFACE_CARGOMETHOD_HPP
#define C2NG_GAME_INTERFACE_CARGOMETHOD_HPP

#include "game/map/planet.hpp"
#include "interpreter/process.hpp"
#include "interpreter/arguments.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "game/root.hpp"

namespace game { namespace interface {

    void doCargoTransfer(game::map::Planet& pl, interpreter::Process& process, interpreter::Arguments& args, Session& session, Turn& turn, const Root& root);
    void doCargoTransfer(game::map::Ship& sh, interpreter::Process& process, interpreter::Arguments& args, Session& session, Turn& turn, const Root& root);
    void doCargoUnload(game::map::Ship& sh, bool reverse, interpreter::Process& process, interpreter::Arguments& args, Session& session, Turn& turn, const Root& root);

} }

#endif
