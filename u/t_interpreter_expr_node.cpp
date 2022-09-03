/**
  *  \file u/t_interpreter_expr_node.cpp
  *  \brief Test for interpreter::expr::Node
  */

#include "interpreter/expr/node.hpp"

#include "t_interpreter_expr.hpp"

/** Interface test. */
void
TestInterpreterExprNode::testInterface()
{
    using interpreter::BytecodeObject;
    using interpreter::CompilationContext;
    class Tester : public interpreter::expr::Node {
     public:
        virtual void compileEffect(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
        virtual void compileValue(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
        virtual void compileStore(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, const Node& /*rhs*/) const
            { }
        virtual void compileCondition(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, BytecodeObject::Label_t /*ift*/, BytecodeObject::Label_t /*iff*/) const
            { }
        virtual void compileRead(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
        virtual void compileWrite(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
    };
    Tester t;
}

