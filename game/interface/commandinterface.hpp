/**
  *  \file game/interface/commandinterface.hpp
  *  \brief Commands and Functions to access a CommandContainer
  */
#ifndef C2NG_GAME_INTERFACE_COMMANDINTERFACE_HPP
#define C2NG_GAME_INTERFACE_COMMANDINTERFACE_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /*
     *  Commands
     */

    void IFAddCommand(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);
    void IFDeleteCommand(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);

    /*
     *  Functions
     */

    afl::data::Value* IFGetCommand(game::Session& session, interpreter::Arguments& args);

} }

#endif
