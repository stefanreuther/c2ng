/**
  *  \file interpreter/optimizer.hpp
  *  \brief Interpreter: Optimizer
  */
#ifndef C2NG_INTERPRETER_OPTIMIZER_HPP
#define C2NG_INTERPRETER_OPTIMIZER_HPP

namespace interpreter {

    class BytecodeObject;
    class World;

    /** Optimize the given bytecode object. It must not have been relocated yet.
        @param [in,out] world World
        @param [in,out] bco   Bytecode object to optimize
        @param [in]     level Optimisation level to apply */
    void optimize(World& world, BytecodeObject& bco, int level);

}

#endif
