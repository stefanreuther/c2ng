/**
  *  \file game/interface/richtextfunctions.hpp
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP

#include "game/interface/richtextvalue.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    bool checkRichArg(RichTextValue::Ptr_t& out, const afl::data::Value* value);

    afl::data::Value* IFRAdd(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRAlign(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRMid(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRString(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRLen(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRStyle(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRLink(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRXml(game::Session& session, interpreter::Arguments& args);

} }

#endif
