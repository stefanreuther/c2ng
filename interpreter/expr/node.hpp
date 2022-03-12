/**
  *  \file interpreter/expr/node.hpp
  *  \brief Base class interpreter::expr::Node
  */
#ifndef C2NG_INTERPRETER_EXPR_NODE_HPP
#define C2NG_INTERPRETER_EXPR_NODE_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/uncopyable.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/compilationcontext.hpp"

namespace interpreter { namespace expr {

    /** Basic expression node.

        All Nodes involved in an expression are expected to live in a Deleter.
        This allows easy sharing of subexpressions, for example, for transformations, without having to deal with lifetime issues.
        The Deleter (and thus the node tree) is expected to have sufficiently short lifetime,
        so excess long-time memory usage is not an issue. */
    class Node : public afl::base::Deletable, private afl::base::Uncopyable {
     public:
        /** Constructor. */
        Node()
            { }

        /** Compile effect of this expression.
            Execution stack must be unchanged afterwards.
            The default can be invoked as defaultCompileEffect(): do compileValue() and drop the result.

            @param [out] bco Code output
            @param [in]  cc  Compilation context */
        virtual void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const = 0;

        /** Compile value of this expression.
            Must leave result on stack.

            @param [out] bco Code output
            @param [in]  cc  Compilation context */
        virtual void compileValue(BytecodeObject& bco, const CompilationContext& cc) const = 0;

        /** Store into this expression.
            Store value of @c rhs into this expression (and leave it there).

            @param [out] bco Code output
            @param [in]  cc  Compilation context
            @param [in]  rhs Node to */
        virtual void compileStore(BytecodeObject& bco, const CompilationContext& cc, const Node& rhs) const = 0;

        /** Compile as condition.
            Generates a jump to @c ift if the expression is true,
            or to @c iff if the expression is false or empty.
            Does not change the stack.
            The default can be invoked as defaultCompileCondition(): do compileValue() and generate two jumps.

            @param [out] bco Code output
            @param [in]  cc  Compilation context
            @param [in]  ift Jump here on true
            @param [in]  iff Jump here on false/empty */
        virtual void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const = 0;

        /** Compile read-modify-write cycle, "read" half.
            Can push some state information; must push the read result.
            Side effects must be executed only once in a compileRead()/compileWrite() pair,
            whereas compileValue() plus compileStore() would execute them twice.

            @param [out] bco Code output
            @param [in]  cc  Compilation context */
        virtual void compileRead(BytecodeObject& bco, const CompilationContext& cc) const = 0;

        /** Compile read-modify-write cycle, "write" half.
            Updated value is on stack; must remain top-of-stack.
            If compileRead() produced some state information, that must be removed.

            @param [out] bco Code output
            @param [in]  cc  Compilation context */
        virtual void compileWrite(BytecodeObject& bco, const CompilationContext& cc) const = 0;

     protected:
        /** Default implementation for compileEffect(): compileValue() and drop result.

            @param [out] bco Code output
            @param [in]  cc  Compilation context */
        void defaultCompileEffect(BytecodeObject& bco, const CompilationContext& cc) const;

        /** Default implementation for compileCondition(): compileValue() and two jumps.

            @param [out] bco Code output
            @param [in]  cc  Compilation context
            @param [in]  ift Jump here on true
            @param [in]  iff Jump here on false/empty */
        void defaultCompileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const;
    };

} }

#endif
