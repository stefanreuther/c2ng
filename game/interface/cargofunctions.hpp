/**
  *  \file game/interface/cargofunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_CARGOFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_CARGOFUNCTIONS_HPP

#include "game/cargospec.hpp"
#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    bool checkCargoSpecArg(CargoSpec& out, afl::data::Value* value);

    afl::data::Value* IFCAdd(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCompare(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCDiv(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCExtract(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCMul(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCRemove(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCSub(game::Session& session, interpreter::Arguments& args);

} }

#endif
