/**
  *  \file interpreter/binaryoperation.cpp
  *  \brief Enum interpreter::BinaryOperation
  */

#include "interpreter/binaryoperation.hpp"
#include "afl/base/countof.hpp"

namespace {
    const char*const binary_names[] = {
        "and",
        "or",
        "xor",
        "add",
        "sub",
        "mul",
        "div",
        "idiv",
        "rem",
        "pow",
        "concat",
        "concempty",
        "cmpeq",
        "cmpeqNC",
        "cmpne",
        "cmpneNC",
        "cmple",
        "cmpleNC",
        "cmplt",
        "cmpltNC",
        "cmpge",
        "cmpgeNC",
        "cmpgt",
        "cmpgtNC",
        "min",
        "minNC",
        "max",
        "maxNC",
        "firststr",
        "firststrNC",
        "reststr",
        "reststrNC",
        "findstr",
        "findstrNC",
        "bitand",
        "bitor",
        "bitxor",
        "str",
        "atan",
        "lcut",
        "rcut",
        "endcut",
        "strmult",
        "keyaddparent",
        "keyfind",
        "arraydim",
    };
}


// Get name for a binary operation.
const char*
interpreter::getBinaryName(uint8_t op)
{
    // ex int/binary.h:getBinaryName
    if (op < countof(binary_names)) {
        return binary_names[op];
    } else {
        return "?";
    }
}
