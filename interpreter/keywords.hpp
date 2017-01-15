/**
  *  \file interpreter/keywords.hpp
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

    Keyword lookupKeyword(const String_t& s);

    void enumKeywords(PropertyAcceptor& acceptor);

}

#endif
