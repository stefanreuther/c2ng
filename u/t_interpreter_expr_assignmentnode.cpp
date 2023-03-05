/**
  *  \file u/t_interpreter_expr_assignmentnode.cpp
  *  \brief Test for interpreter::expr::AssignmentNode
  */

#include "interpreter/expr/assignmentnode.hpp"

#include "t_interpreter_expr.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment(String_t name)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, name, 42)
            { }
    };
}

/** Test compileValue(). */
void
TestInterpreterExprAssignmentNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Compile: 'V := 10', keeping the result on stack. To prove that the result was correctly kept, add variable and value.
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("V");
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);
    bco->addInstruction(interpreter::Opcode::maBinary, interpreter::biAdd, 0);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 20);
}

/** Test compileEffect(). */
void
TestInterpreterExprAssignmentNode::testEffect()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(19)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Compile: 'V := 10', effect only. To prove that it worked, load the variable.
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("V");
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 19);
}

/** Test compileStore, compileRead, compileWrite.
    Those must be rejected without generating code. */
void
TestInterpreterExprAssignmentNode::testOther()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(19)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), testee), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

