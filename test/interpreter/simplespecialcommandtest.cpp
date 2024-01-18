/**
  *  \file test/interpreter/simplespecialcommandtest.cpp
  *  \brief Test for interpreter::SimpleSpecialCommand
  */

#include "interpreter/simplespecialcommand.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/world.hpp"

using interpreter::Opcode;

namespace {
    void commandTester(interpreter::Tokenizer& /*line*/,
                       interpreter::BytecodeObject& bco,
                       const interpreter::StatementCompilationContext& /*scc*/)
    {
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    }
}

/** Really simple test. */
AFL_TEST("interpreter.SimpleSpecialCommand", a)
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::BytecodeObject bco;
    interpreter::Tokenizer tok("x");
    interpreter::World world(log, tx, fs);
    interpreter::DefaultStatementCompilationContext scc(world);

    // Tester
    interpreter::SimpleSpecialCommand testee(commandTester);
    testee.compileCommand(tok, bco, scc);

    // Verify
    a.checkEqual("01. getNumInstructions", bco.getNumInstructions(), 1U);
    a.checkEqual("02. opcode", bco(0).major, Opcode::maSpecial);
}
