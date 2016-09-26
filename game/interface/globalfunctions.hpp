/**
  *  \file game/interface/globalfunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_GLOBALFUNCTIONS_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    afl::data::Value* IFFormat(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRandom(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFTranslate(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFTruehull(game::Session& session, interpreter::Arguments& args);


} }

#endif
