/**
  *  \file interpreter/keywords.hpp
  *  \brief Interpreter: Keywords
  */
#ifndef C2NG_INTERPRETER_KEYWORDS_HPP
#define C2NG_INTERPRETER_KEYWORDS_HPP

#include "afl/string/string.hpp"

namespace interpreter {

    class PropertyAcceptor;

    enum Keyword {
        kwNone,
        kwAbort,
        kwBind,
        kwBreak,
        kwCall,
        kwCase,
        kwContinue,
        kwCreateKeymap,
        kwCreatePlanetProperty,
        kwCreateShipProperty,
        kwDim,
        kwDo,
        kwElse,
        kwEnd,
        kwEndFunction,
        kwEndIf,
        kwEndOn,
        kwEndSelect,
        kwEndStruct,
        kwEndSub,
        kwEndTry,
        kwEndWith,
        kwEval,
        kwFor,
        kwForEach,
        kwFunction,
        kwIf,
        kwLoad,
        kwLocal,
        kwLoop,
        kwNext,
        kwOn,
        kwOption,
        kwPrint,
        kwReDim,
        kwRestart,
        kwReturn,
        kwRunHook,
        kwSelect,
        kwSelectionExec,
        kwShared,
        kwStatic,
        kwStop,
        kwStruct,
        kwSub,
        kwTry,
        kwTryLoad,
        kwUntil,
        kwUseKeymap,
        kwWhile,
        kwWith
    };

    /** Check whether the given string is a keyword.
        @param s String, upper-case
        @return keyword; kwNone if s does not match any keyword */
    Keyword lookupKeyword(const String_t& s);

    /** Enumerate keywords.
        @param [out] acceptor  Will receive the list of keywords */
    void enumKeywords(PropertyAcceptor& acceptor);

}

#endif
