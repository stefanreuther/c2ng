/**
  *  \file interpreter/unaryexecution.hpp
  *  \brief Execution of Unary Operations
  */
#ifndef C2NG_INTERPRETER_UNARYEXECUTION_HPP
#define C2NG_INTERPRETER_UNARYEXECUTION_HPP

#include "afl/base/types.hpp"
#include "afl/data/value.hpp"

namespace interpreter {

    class World;

    /** Execute unary operation.
        \param world World to work in
        \param op Operation (see UnaryOperation; appears typed as uint8_t in bytecode)
        \param arg User-supplied argument taken from value stack
        \return New value to push on value stack */
    afl::data::Value* executeUnaryOperation(World& world, uint8_t op, const afl::data::Value* arg);

}

#endif
