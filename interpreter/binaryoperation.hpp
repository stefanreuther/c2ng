/**
  *  \file interpreter/binaryoperation.hpp
  *  \brief Enum interpreter::BinaryOperation
  */
#ifndef C2NG_INTERPRETER_BINARYOPERATION_HPP
#define C2NG_INTERPRETER_BINARYOPERATION_HPP

#include "afl/base/types.hpp"

namespace interpreter {

    /** Minor opcode for binary operations.
        These are stored in the "minor" field of an Opcode whose major opcode is Opcode::maBinary. */
    enum BinaryOperation {
        biAnd,                      ///< Logical And.
        biOr,                       ///< Logical Or.
        biXor,                      ///< Logical Xor.
        biAdd,                      ///< Add.
        biSub,                      ///< Subtract.
        biMult,                     ///< Multiply.
        biDivide,                   ///< Divide.
        biIntegerDivide,            ///< Integer divide.
        biRemainder,                ///< Compute remainder.
        biPow,                      ///< Power.
        biConcat,                   ///< String concat, Empty annihilates result ('#').
        biConcatEmpty,              ///< String concat, Empty interpolates to empty string ('&').
        biCompareEQ,                ///< Comparison.
        biCompareEQ_NC,             ///< Comparison, ignoring case.
        biCompareNE,                ///< Comparison.
        biCompareNE_NC,             ///< Comparison, ignoring case.
        biCompareLE,                ///< Comparison.
        biCompareLE_NC,             ///< Comparison, ignoring case.
        biCompareLT,                ///< Comparison.
        biCompareLT_NC,             ///< Comparison, ignoring case.
        biCompareGE,                ///< Comparison.
        biCompareGE_NC,             ///< Comparison, ignoring case.
        biCompareGT,                ///< Comparison.
        biCompareGT_NC,             ///< Comparison, ignoring case.
        biMin,                      ///< Return minimum.
        biMin_NC,                   ///< Return minimum, ignoring case.
        biMax,                      ///< Return maximum.
        biMax_NC,                   ///< Return maximum, ignoring case.
        biFirstStr,                 ///< First part of string.
        biFirstStr_NC,              ///< First part of string, ignoring case.
        biRestStr,                  ///< Rest of string.
        biRestStr_NC,               ///< Rest of string, ignoring case.
        biFindStr,                  ///< Locate string.
        biFindStr_NC,               ///< Locate string, ignoring case.
        biBitAnd,                   ///< Bitwise and.
        biBitOr,                    ///< Bitwise or.
        biBitXor,                   ///< Bitwise xor.
        biStr,                      ///< Stringify with explicit width given.
        biATan,                     ///< Arctangent.
        biLCut,                     ///< Left cut. Remove everything before string position N.
        biRCut,                     ///< Right cut. Remove everything after string position N.
        biEndCut,                   ///< End cut. Remove everything but last N characters.
        biStrMult,                  ///< String multiply. First arg is count.
        biKeyAddParent,             ///< Add parent to keymap.
        biKeyFind,                  ///< Look up key in keymap.
        biArrayDim                  ///< Get array dimension.
    };

    /** Get name for a binary operation.
        This is used for disassembling.
        @param op Operation, matching a BinaryOperation value
        @return name; never null  */
    const char* getBinaryName(uint8_t op);

}

#endif
