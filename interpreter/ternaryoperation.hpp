/**
  *  \file interpreter/ternaryoperation.hpp
  *  \brief Enum interpreter::TernaryOperation
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

    /** Get name for a ternary operation.
        This is used for disassembling.
        @param op Operation, matching a TernaryOperation value
        @return name; never null  */
    const char* getTernaryName(uint8_t op);

}

#endif
