/**
  *  \file game/interface/commandinterface.hpp
  */
#ifndef C2NG_GAME_INTERFACE_COMMANDINTERFACE_HPP
#define C2NG_GAME_INTERFACE_COMMANDINTERFACE_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    void IFAddCommand(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFDeleteCommand(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);

    afl::data::Value* IFGetCommand(game::Session& session, interpreter::Arguments& args);
    
} }

#endif
