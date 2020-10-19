/**
  *  \file interpreter/binaryexecution.hpp
  *  \brief Execution of Binary Operations
  */
#ifndef C2NG_INTERPRETER_BINARYEXECUTION_HPP
#define C2NG_INTERPRETER_BINARYEXECUTION_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class World;

    /** Execute binary operation.
        \param world World to work in
        \param op Operation (see BinaryOperation; appears typed as uint8_t in bytecode)
        \param a,b User-supplied arguments taken from value stack
        \return New value to push on value stack */
    afl::data::Value* executeBinaryOperation(World& world, uint8_t op, const afl::data::Value* a, const afl::data::Value* b);

    /** Execute a comparison operation.
        \param op Operation (see BinaryOperation; appears typed as uint8_t in bytecode)
        \param a,b User-supplied arguments taken from value stack
        \return Comparison result, possible input to makeBooleanValue. */
    int executeComparison(uint8_t op, const afl::data::Value* a, const afl::data::Value* b);

}

#endif
