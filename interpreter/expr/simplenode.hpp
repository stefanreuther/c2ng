/**
  *  \file interpreter/expr/simplenode.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_SIMPLENODE_HPP
#define C2NG_INTERPRETER_EXPR_SIMPLENODE_HPP

#include "interpreter/expr/simplervaluenode.hpp"
#include "interpreter/opcode.hpp"

namespace interpreter { namespace expr {

    /** Simple expression node. Combines up to three parameters using a single opcode. */
    class SimpleNode : public SimpleRValueNode {
     public:
        SimpleNode(Opcode::Major major, uint8_t minor);
        void compileValue(BytecodeObject& bco, const CompilationContext& cc);
        void compileEffect(BytecodeObject& bco, const interpreter::CompilationContext& cc);
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        bool is(Opcode::Major major, uint8_t minor) const
            { return this->major == major && this->minor == minor; }

     private:
        // FIXME: names
        Opcode::Major major : 8;
        uint8_t minor;
    };

} }

#endif
