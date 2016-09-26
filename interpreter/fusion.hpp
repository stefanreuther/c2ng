/**
  *  \file interpreter/fusion.hpp
  */
#ifndef C2NG_INTERPRETER_FUSION_HPP
#define C2NG_INTERPRETER_FUSION_HPP

namespace interpreter {

    class BytecodeObject;

    void fuseInstructions(BytecodeObject& bco);
    void unfuseInstructions(BytecodeObject& bco);

}

#endif
