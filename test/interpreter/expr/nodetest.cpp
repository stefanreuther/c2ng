/**
  *  \file test/interpreter/expr/nodetest.cpp
  *  \brief Test for interpreter::expr::Node
  */

#include "interpreter/expr/node.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.expr.Node")
{
    using interpreter::BytecodeObject;
    using interpreter::CompilationContext;
    class Tester : public interpreter::expr::Node {
     public:
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
