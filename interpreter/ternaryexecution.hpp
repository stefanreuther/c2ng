/**
  *  \file interpreter/ternaryexecution.hpp
  */
#ifndef C2NG_INTERPRETER_TERNARYEXECUTION_HPP
#define C2NG_INTERPRETER_TERNARYEXECUTION_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class World;

    afl::data::Value* executeTernaryOperation(interpreter::World& world, uint8_t op, afl::data::Value* a, afl::data::Value* b, afl::data::Value* c);

}

#endif
