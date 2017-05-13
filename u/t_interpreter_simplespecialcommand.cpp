/**
  *  \file u/t_interpreter_simplespecialcommand.cpp
  *  \brief Test for interpreter::SimpleSpecialCommand
  */

#include "interpreter/simplespecialcommand.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
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
void
TestInterpreterSimpleSpecialCommand::testIt()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::BytecodeObject bco;
    interpreter::Tokenizer tok("x");
    interpreter::World world(log, fs);
    interpreter::DefaultStatementCompilationContext scc(world);

    // Tester
    interpreter::SimpleSpecialCommand testee(commandTester);
    testee.compileCommand(tok, bco, scc);

    // Verify
    TS_ASSERT_EQUALS(bco.getNumInstructions(), 1U);
    TS_ASSERT_EQUALS(bco(0).major, Opcode::maSpecial);
}

