/**
  *  \file game/interface/markingfunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MARKINGFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_MARKINGFUNCTIONS_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    afl::data::Value* IFCCSelReadHeader(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCSelReadContent(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCSelGetQuestion(game::Session& session, interpreter::Arguments& args);

    void IFSelectionSave(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);

} }

#endif
