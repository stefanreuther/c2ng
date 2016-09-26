/**
  *  \file interpreter/unaryexecution.hpp
  */
#ifndef C2NG_INTERPRETER_UNARYEXECUTION_HPP
#define C2NG_INTERPRETER_UNARYEXECUTION_HPP

#include "afl/data/value.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class World;

    afl::data::Value* executeUnaryOperation(World& world, uint8_t op, afl::data::Value* arg);

}

#endif
