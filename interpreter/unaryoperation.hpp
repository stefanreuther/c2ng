/**
  *  \file interpreter/unaryoperation.hpp
  */
#ifndef C2NG_INTERPRETER_UNARYOPERATION_HPP
#define C2NG_INTERPRETER_UNARYOPERATION_HPP

#include "afl/base/types.hpp"

namespace interpreter {

    /** Minor opcode for unary operations.
        These are stored in the "minor" field of an Opcode whose major opcode is Opcode::maUnary. */
    enum UnaryOperation {
        unNot,                      ///< Logical Not.
        unBool,                     ///< Cast to bool (double Not).
        unNeg,                      ///< Arithmetical negation.
        unPos,                      ///< Arithmetical non-negation (double Neg, Plus).
        unSin,                      ///< Sine.
        unCos,                      ///< Cosine.
        unTan,                      ///< Tangent.
        unZap,                      ///< "Zap" operation ('Z(n)').
        unAbs,                      ///< Absolute value.
        unExp,                      ///< Exponential function.
        unLog,                      ///< Natural logarithm.
        unBitNot,                   ///< Bitwise not.
        unIsEmpty,                  ///< Check for emptiness.
        unIsNum,                    ///< Check for numericness.
        unIsString,                 ///< Check for string.
        unAsc,                      ///< Get ASCII code of stringified arg.
        unChr,                      ///< Make string from ASCII code.
        unStr,                      ///< Convert to string.
        unSqrt,                     ///< Square root.
        unTrunc,                    ///< Truncate to integer.
        unRound,                    ///< Round to integer.
        unLTrim,                    ///< Trim string left.
        unRTrim,                    ///< Trim string right.
        unLRTrim,                   ///< Trim string left and right.
        unLength,                   ///< String length.
        unVal,                      ///< Convert string to number.
        unTrace,                    ///< Generate trace message.
        unNot2,                     ///< Logical Not for two-valued logic, empty counts as false.
        unAtom,                     ///< Convert string to integer.
        unAtomStr,                  ///< Convert previously converted integer back into string.
        unKeyCreate,                ///< Create named keymap.
        unKeyLookup,                ///< Look up named keymap.
        unInc,                      ///< Increment numeric.
        unDec,                      ///< Decrement numeric.
        unIsProcedure,              ///< Check for procedure.
        unFileNr,                   ///< Convert integer into file number.
        unIsArray,                  ///< Check for array (and return number of dimensions).
        unUCase,                    ///< Convert string to upper-case.
        unLCase                     ///< Convert string to lower-case.
    };

    const char* getUnaryName(uint8_t op);

}

#endif
