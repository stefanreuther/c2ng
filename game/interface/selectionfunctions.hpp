/**
  *  \file game/interface/selectionfunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_SELECTIONFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_SELECTIONFUNCTIONS_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    afl::data::Value* IFCCSelReadHeader(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCSelReadContent(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCSelGetQuestion(game::Session& session, interpreter::Arguments& args);

    void IFSelectionSave(game::Session& session, interpreter::Process& proc, interpreter::Arguments& args);

} }

#endif
