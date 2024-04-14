/**
  *  \file game/interface/selectionfunctions.hpp
  *  \brief Selection I/O Functions
  */
#ifndef C2NG_GAME_INTERFACE_SELECTIONFUNCTIONS_HPP
#define C2NG_GAME_INTERFACE_SELECTIONFUNCTIONS_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Implementation of the "CC$SelReadHeader" function.
        For use with SimpleFunction<Session&>.
        @param session Session
        @param args    Parameters
        @return newly-allocated value */
    afl::data::Value* IFCCSelReadHeader(Session& session, interpreter::Arguments& args);

    /** Implementation of the "CC$SelReadContent" function.
        For use with SimpleFunction<Session&>.
        @param session Session
        @param args    Parameters
        @return newly-allocated value */
    afl::data::Value* IFCCSelReadContent(Session& session, interpreter::Arguments& args);

    /** Implementation of the "CC$SelGetQuestion" function.
        For use with SimpleFunction<Session&>.
        @param session Session
        @param args    Parameters
        @return newly-allocated value */
    afl::data::Value* IFCCSelGetQuestion(Session& session, interpreter::Arguments& args);

    /** Implementation of the "SelectionSave" command.
        For use with SimpleProcedure<Session&>.
        @param session Session
        @param proc    Process
        @param args    Parameters */
    void IFSelectionSave(Session& session, interpreter::Process& proc, interpreter::Arguments& args);

} }

#endif
