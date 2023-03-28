/**
  *  \file game/interface/richtextfunctions.hpp
  *  \brief Rich-Text Functions
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP

#include "game/interface/richtextvalue.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Check for rich-text argument.
        For now, any non-null argument is valid.
        If an argument is specified that is not rich-text, it is stringified.

        @param [out] out   Result
        @param [in]  value User-supplied parameter
        @return true if argument was supplied; false if value was null */
    bool checkRichArg(RichTextValue::Ptr_t& out, const afl::data::Value* value);

    /*
     *  Function Implementations
     */

    afl::data::Value* IFRAdd(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRAlign(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRMid(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRString(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRLen(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRStyle(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRLink(Session& session, interpreter::Arguments& args);
    afl::data::Value* IFRXml(Session& session, interpreter::Arguments& args);

} }

#endif
