/**
  *  \file u/t_interpreter_expr_functioncallnode.cpp
  *  \brief Test for interpreter::expr::FunctionCallNode
  */

#include "interpreter/expr/functioncallnode.hpp"

#include "t_interpreter_expr.hpp"
#include "interpreter/expr/identifiernode.hpp"

void
TestInterpreterExprFunctionCallNode::testIt()
{
    using interpreter::BytecodeObject;
    using interpreter::CompilationContext;
    class MyNode : public interpreter::expr::FunctionCallNode {
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

        const interpreter::expr::Node* get(size_t i) const
            { return args[i]; }
    };

    MyNode testee;
    TS_ASSERT_EQUALS(testee.getNumArgs(), 0U);

    interpreter::expr::IdentifierNode id1("ONE"), id2("TWO");
    testee.addArgument(id1);
    testee.addArgument(id2);
    TS_ASSERT_EQUALS(testee.getNumArgs(), 2U);
    TS_ASSERT_EQUALS(testee.get(1), &id2);
}

