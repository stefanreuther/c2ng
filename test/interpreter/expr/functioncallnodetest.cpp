/**
  *  \file test/interpreter/expr/functioncallnodetest.cpp
  *  \brief Test for interpreter::expr::FunctionCallNode
  */

#include "interpreter/expr/functioncallnode.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/expr/identifiernode.hpp"

AFL_TEST("interpreter.expr.FunctionCallNode", a)
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
    a.checkEqual("01. getNumArgs", testee.getNumArgs(), 0U);

    interpreter::expr::IdentifierNode id1("ONE"), id2("TWO");
    testee.addArgument(id1);
    testee.addArgument(id2);
    a.checkEqual("11. getNumArgs", testee.getNumArgs(), 2U);
    a.checkEqual("12. get", testee.get(1), &id2);
}
