/**
  *  \file u/t_interpreter_expr_rvaluefunctioncallnode.cpp
  *  \brief Test for interpreter::expr::RValueFunctionCallNode
  */

#include "interpreter/expr/rvaluefunctioncallnode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/world.hpp"

/** Test that modification operations are rejected.
    Accepted operations are tested through derived classes. */
void
TestInterpreterExprRValueFunctionCallNode::testOther()
{
    // An instance for testing
    using interpreter::BytecodeObject;
    using interpreter::CompilationContext;
    class MyNode : public interpreter::expr::RValueFunctionCallNode {
     public:
        virtual void compileEffect(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
        virtual void compileValue(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { }
        virtual void compileCondition(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, BytecodeObject::Label_t /*ift*/, BytecodeObject::Label_t /*iff*/) const
            { }
    };
    MyNode testee;

    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::expr::IdentifierNode leftValue("A");

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(world), leftValue), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

