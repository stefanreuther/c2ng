/**
  *  \file u/t_interpreter_expr_identifiernode.cpp
  *  \brief Test for interpreter::expr::IdentifierNode
  */

#include "interpreter/expr/identifiernode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/binaryoperation.hpp"
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
TestInterpreterExprIdentifierNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::IdentifierNode testee("AA");

    // Compile: read local variable
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 10);
    bco->addInstruction(interpreter::Opcode::maDim,  interpreter::Opcode::sLocal, bco->addName("AA"));
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 10);
}

/** Test compileStore(). */
void
TestInterpreterExprIdentifierNode::testStore()
{
    Environment env("testStore");
    interpreter::expr::IdentifierNode testee("AA");

    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(55)));

    // Compile: store into local variable: "aa := 55"
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addLocalVariable("AA");
    testee.compileStore(*bco, interpreter::CompilationContext(env.world), value);

    // To prove that value has been stored, add both values
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sNamedVariable, bco->addName("AA"));
    bco->addInstruction(interpreter::Opcode::maBinary, interpreter::biAdd, 0);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 110);    // 55*2
}

/** Test compileCondition(). */
void
TestInterpreterExprIdentifierNode::testCondition()
{
    Environment env("testCondition");
    interpreter::expr::IdentifierNode testee("AA");

    // Compile: basically, "if (testee, 2, 3)".
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    interpreter::BytecodeObject::Label_t lthen = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lelse = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lend = bco->makeLabel();

    // - set testee to 10
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 10);
    bco->addInstruction(interpreter::Opcode::maDim,  interpreter::Opcode::sLocal, bco->addName("AA"));

    // - condition
    testee.compileCondition(*bco, interpreter::CompilationContext(env.world), lthen, lelse);
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

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 2);
}

/** Test compileRead(), compileWrite(). */
void
TestInterpreterExprIdentifierNode::testReadWrite()
{
    Environment env("testReadWrite");
    interpreter::expr::IdentifierNode testee("AA");

    // Compile: basically, 'incr aa', starting with value 10
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 10);
    bco->addInstruction(interpreter::Opcode::maDim,  interpreter::Opcode::sLocal, bco->addName("AA"));
    testee.compileRead(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maUnary, interpreter::unInc, 0);
    testee.compileWrite(*bco, interpreter::CompilationContext(env.world));          // This sets AA to 11

    // To prove that value has been stored, add both values
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sNamedVariable, bco->addName("AA"));
    bco->addInstruction(interpreter::Opcode::maBinary, interpreter::biAdd, 0);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 22);
}

