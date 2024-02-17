/**
  *  \file game/interface/richtextfunctions.hpp
  *  \brief Rich-Text Functions
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTFUNCTIONS_HPP

#include "game/interface/richtextvalue.hpp"
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

    afl::data::Value* IFRAdd(interpreter::Arguments& args);
    afl::data::Value* IFRAlign(interpreter::Arguments& args);
    afl::data::Value* IFRMid(interpreter::Arguments& args);
    afl::data::Value* IFRString(interpreter::Arguments& args);
    afl::data::Value* IFRLen(interpreter::Arguments& args);
    afl::data::Value* IFRStyle(interpreter::Arguments& args);
    afl::data::Value* IFRLink(interpreter::Arguments& args);
    afl::data::Value* IFRXml(interpreter::Arguments& args);

} }

#endif
