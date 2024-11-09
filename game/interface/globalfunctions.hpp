/**
  *  \file game/interface/globalfunctions.hpp
  *  \brief Global Functions
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_GLOBALFUNCTIONS_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /*
     *  These functions are provided to the script world using SimpleFunction.
     */

    afl::data::Value* IFAutoTask(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCfg(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFDistance(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFFormat(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFHasAdvantage(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFIsSpecialFCode(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFMissionDefinitions(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFObjectIsAt(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFPlanetAt(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFPref(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFQuote(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRandom(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRandomFCode(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFTranslate(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFTruehull(game::Session& session, interpreter::Arguments& args);

} }

#endif
