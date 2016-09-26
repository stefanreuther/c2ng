/**
  *  \file interpreter/ternaryoperation.hpp
  */
#ifndef C2NG_INTERPRETER_TERNARYOPERATION_HPP
#define C2NG_INTERPRETER_TERNARYOPERATION_HPP

#include "afl/base/types.hpp"

namespace interpreter {

    /** Minor opcode for ternary operation.
        These are stored in the "minor" field of an Opcode whose major opcode is Opcode::maTernary. */
    enum TernaryOperation {
        teKeyAdd                    ///< Add key/command to keymap, return keymap.
    };

    const char* getTernaryName(uint8_t op);

}

#endif
