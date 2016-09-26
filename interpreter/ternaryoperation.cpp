/**
  *  \file interpreter/ternaryoperation.cpp
  */

#include "interpreter/ternaryoperation.hpp"
#include "afl/base/countof.hpp"

namespace {
    static const char*const ternary_names[] = {
        "keyadd",
    };
}

/** Get name for a ternary operation. This is used for disassembling.
    \param op Operation, IntTernaryOperation */
const char*
interpreter::getTernaryName(uint8_t op)
{
    // ex int/ternary.cc:getTernaryName
    if (op < countof(ternary_names)) {
        return ternary_names[op];
    } else {
        return "?";
    }
}
