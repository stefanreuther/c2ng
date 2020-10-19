/**
  *  \file interpreter/ternaryexecution.hpp
  *  \brief Execution of Ternary Operations
  */
#ifndef C2NG_INTERPRETER_TERNARYEXECUTION_HPP
#define C2NG_INTERPRETER_TERNARYEXECUTION_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class World;

    /** Execute ternary operation.
        \param world World to work in
        \param op Operation (see TernaryOperation; appears typed as uint8_t in bytecode)
        \param a,b,c User-supplied arguments taken from value stack
        \return New value to push on value stack */
    afl::data::Value* executeTernaryOperation(interpreter::World& world, uint8_t op, const afl::data::Value* a, const afl::data::Value* b, const afl::data::Value* c);

}

#endif
