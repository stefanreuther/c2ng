/**
  *  \file interpreter/optimizer.hpp
  *  \brief Interpreter: Optimizer
  */
#ifndef C2NG_INTERPRETER_OPTIMIZER_HPP
#define C2NG_INTERPRETER_OPTIMIZER_HPP

namespace interpreter {

    class BytecodeObject;
    class World;

    void optimize(World& world, BytecodeObject& bco, int level);

}

#endif
