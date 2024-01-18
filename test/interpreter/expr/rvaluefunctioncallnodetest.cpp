/**
  *  \file test/interpreter/expr/rvaluefunctioncallnodetest.cpp
  *  \brief Test for interpreter::expr::RValueFunctionCallNode
  */

#include "interpreter/expr/rvaluefunctioncallnode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/world.hpp"

/** Test that modification operations are rejected.
    Accepted operations are tested through derived classes. */
AFL_TEST("interpreter.expr.RValueFunctionCallNode", a)
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
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(world), leftValue), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
