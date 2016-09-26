/**
  *  \file game/interface/globalcommands.hpp
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP
#define C2NG_GAME_INTERFACE_GLOBALCOMMANDS_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    void IFHistoryShowTurn(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);

} }


#endif
