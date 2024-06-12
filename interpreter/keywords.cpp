/**
  *  \file interpreter/keywords.cpp
  *  \brief Interpreter: Keywords
  */

#include "interpreter/keywords.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/propertyacceptor.hpp"

namespace {
    const char*const KEYWORD_TABLE[] = {
        "ABORT",
        "BIND",
        "BREAK",
        "CALL",
        "CASE",
        "CONTINUE",
        "CREATEKEYMAP",
        "CREATEPLANETPROPERTY",
        "CREATESHIPPROPERTY",
        "DIM",
        "DO",
        "ELSE",
        "END",
        "ENDFUNCTION",
        "ENDIF",
        "ENDON",
        "ENDSELECT",
        "ENDSTRUCT",
        "ENDSUB",
        "ENDTRY",
        "ENDWITH",
        "EVAL",
        "FOR",
        "FOREACH",
        "FUNCTION",
        "IF",
        "LOAD",
        "LOCAL",
        "LOOP",
        "NEXT",
        "ON",
        "OPTION",
        "PRINT",
        "REDIM",
        "RESTART",
        "RETURN",
        "RUNHOOK",
        "SELECT",
        "SELECTIONEXEC",
        "SHARED",
        "STATIC",
        "STOP",
        "STRUCT",
        "SUB",
        "TRY",
        "TRYLOAD",
        "UNTIL",
        "USEKEYMAP",
        "WHILE",
        "WITH",
    };
}

// Check whether the given string is a keyword.
interpreter::Keyword
interpreter::lookupKeyword(const String_t& s)
{
    // ex lookupKeyword
    for (size_t i = 0; i < countof(KEYWORD_TABLE); ++i) {
        if (KEYWORD_TABLE[i] == s) {
            return Keyword(i+1);
        }
    }
    return kwNone;
}

// Enumerate keywords.
void
interpreter::enumKeywords(PropertyAcceptor& acceptor)
{
    // ex enumKeywords
    for (size_t i = 0; i < countof(KEYWORD_TABLE); ++i) {
        acceptor.addProperty(KEYWORD_TABLE[i], thNone);
    }
}
