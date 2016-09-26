/**
  *  \file interpreter/expr/node.hpp
  */
#ifndef C2NG_INTERPRETER_EXPR_NODE_HPP
#define C2NG_INTERPRETER_EXPR_NODE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/uncopyable.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/compilationcontext.hpp"

namespace interpreter { namespace expr {

    /** Basic expression node. */
    class Node : public afl::base::Deletable, private afl::base::Uncopyable {
     public:
        /** Constructor. */
        Node()
            { }

        /** Compile effect of this expression.
            Execution stack will not be changed. Default is to do compileValue and drop result. */
        virtual void compileEffect(BytecodeObject& bco, const CompilationContext& cc) = 0;

        /** Compile value of this expression. Result will be on stack. */
        virtual void compileValue(BytecodeObject& bco, const CompilationContext& cc) = 0;

        /** Store into this expression. Store stack-top into this expression (and leave it there). */
        virtual void compileStore(BytecodeObject& bco, const CompilationContext& cc, Node& rhs) = 0;

        /** Compile as condition. Generates a jump to \c ift if the expression is true,
            to \c iff if the expression is false or empty. Does not change the stack.
            Default is to compile value and generate two jumps. */
        virtual void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) = 0;

        /** Compile read-modify-write cycle, "read" half. Result will be top-of-stack; state information can be below. */
        virtual void compileRead(BytecodeObject& bco, const CompilationContext& cc) = 0;

        /** Compile read-modify-write cycle, "write" half. Updated value
            is on stack; must remain top-of-stack, although state
            information must be removed. */
        virtual void compileWrite(BytecodeObject& bco, const CompilationContext& cc) = 0;

     protected:
        void defaultCompileEffect(BytecodeObject& bco, const CompilationContext& cc);
        void defaultCompileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
    };

} }

#endif
