/**
  *  \file interpreter/binaryexecution.hpp
  */
#ifndef C2NG_INTERPRETER_BINARYEXECUTION_HPP
#define C2NG_INTERPRETER_BINARYEXECUTION_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class World;

    afl::data::Value* executeBinaryOperation(World& world, uint8_t op, afl::data::Value* a, afl::data::Value* b);
    int executeComparison(uint8_t op, afl::data::Value* a, afl::data::Value* b);

}

#endif
