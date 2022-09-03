/**
  *  \file interpreter/ternaryoperation.cpp
  *  \brief Enum interpreter::TernaryOperation
  */

#include "interpreter/ternaryoperation.hpp"
#include "afl/base/countof.hpp"

namespace {
    static const char*const ternary_names[] = {
        "keyadd",
    };
}

// Get name for a ternary operation.
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
