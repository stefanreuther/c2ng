/**
  *  \file u/t_interpreter_expr_logicalnode.cpp
  *  \brief Test for interpreter::expr::LogicalNode
  */

#include "interpreter/expr/logicalnode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/assignmentnode.hpp"
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

void
TestInterpreterExprLogicalNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("a")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("b")));
    interpreter::expr::IdentifierNode rightVariable("X");
    interpreter::expr::AssignmentNode rightExpr(rightVariable, rightValue);
    interpreter::expr::LogicalNode testee(interpreter::Opcode::jIfTrue, interpreter::biOr, leftValue, rightExpr);

    // Compile '"a" or (x:="b")' [the 'x:="b"' would fail if executed]
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: result must be integral '1'
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 1);
}

void
TestInterpreterExprLogicalNode::testEffect()
{
    Environment env("testEffect");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(42)));
    interpreter::expr::IdentifierNode leftVariable("Y");
    interpreter::expr::AssignmentNode leftExpr(leftVariable, leftValue);
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("b")));
    interpreter::expr::IdentifierNode rightVariable("X");
    interpreter::expr::AssignmentNode rightExpr(rightVariable, rightValue);
    interpreter::expr::LogicalNode testee(interpreter::Opcode::jIfTrue, interpreter::biOr, leftExpr, rightExpr);

    // Compile '(y:=42) or (x:="b")', then 'y'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("Y");
    testee.compileEffect(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: result must be 42 (assignment to Y has been executed)
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 42);
}

void
TestInterpreterExprLogicalNode::testCondition()
{
    Environment env("testCondition");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("a")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("b")));
    interpreter::expr::IdentifierNode rightVariable("X");
    interpreter::expr::AssignmentNode rightExpr(rightVariable, rightValue);
    interpreter::expr::LogicalNode testee(interpreter::Opcode::jIfTrue, interpreter::biOr, leftValue, rightExpr);

    // Compile 'if ("a" or (x:="b"),2,3)' [the 'x:="b"' would fail if executed]
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    interpreter::BytecodeObject::Label_t lthen = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lelse = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lend = bco->makeLabel();
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 1);  // Not reached; indicates an error if reached
    bco->addJump(interpreter::Opcode::jAlways, lend);
    bco->addLabel(lthen);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 2);
    bco->addJump(interpreter::Opcode::jAlways, lend);
    bco->addLabel(lelse);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 3);
    bco->addLabel(lend);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: result must be integral '1'
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 1);

}

void
TestInterpreterExprLogicalNode::testOther()
{
    Environment env("testOther");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("a")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("b")));
    interpreter::expr::IdentifierNode rightVariable("X");
    interpreter::expr::AssignmentNode rightExpr(rightVariable, rightValue);
    interpreter::expr::LogicalNode testee(interpreter::Opcode::jIfTrue, interpreter::biOr, leftValue, rightExpr);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), leftValue), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

