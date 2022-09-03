/**
  *  \file interpreter/unaryoperation.cpp
  *  \brief Enum interpreter::UnaryOperation
  */

#include "interpreter/unaryoperation.hpp"
#include "afl/base/countof.hpp"

namespace {
    static const char* const unary_names[] = {
        "not",
        "bool",
        "neg",
        "pos",
        "sin",
        "cos",
        "tan",
        "zap",
        "abs",
        "exp",
        "log",
        "bitnot",
        "isempty",
        "isnum",
        "isstr",
        "asc",
        "chr",
        "str",
        "sqrt",
        "trunc",
        "round",
        "ltrim",
        "rtrim",
        "lrtrim",
        "length",
        "val",
        "trace",
        "not2",
        "atom",
        "atomstr",
        "keycreate",
        "keylookup",
        "inc",
        "dec",
        "isproc",
        "filenr",
        "isarray",
        "ucase",
        "lcase",
    };
}

// Get name for an unary operation.
const char*
interpreter::getUnaryName(uint8_t op)
{
    // ex int/unary.h:getUnaryName
    if (op < countof(unary_names)) {
        return unary_names[op];
    } else {
        return "?";
    }
}
