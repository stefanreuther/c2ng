/**
  *  \file u/t_interpreter_defaultstatementcompilationcontext.cpp
  *  \brief Test for interpreter::DefaultStatementCompilationContext
  */

#include "interpreter/defaultstatementcompilationcontext.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"

/** Test standalone DefaultStatementCompilationContext. */
void
TestInterpreterDefaultStatementCompilationContext::testStandalone()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::BytecodeObject bco;

    // Create
    interpreter::DefaultStatementCompilationContext testee(world);
    interpreter::StatementCompilationContext& scc = testee;

    TS_ASSERT_EQUALS(&scc.world(), &world);

    // Test
    TS_ASSERT_THROWS(scc.compileBreak(bco), interpreter::Error);
    TS_ASSERT_THROWS(scc.compileContinue(bco), interpreter::Error);
    TS_ASSERT_THROWS_NOTHING(scc.compileCleanup(bco));

    // None of the above generated any code
    TS_ASSERT_EQUALS(bco.getNumInstructions(), 0U);
}

/** Test DefaultStatementCompilationContext with a parent. */
void
TestInterpreterDefaultStatementCompilationContext::testParented()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Parent SCC
    class ParentSCC : public interpreter::StatementCompilationContext {
     public:
        ParentSCC(interpreter::World& w)
            : StatementCompilationContext(w)
            { }
        virtual void compileBreak(interpreter::BytecodeObject& bco) const
            { bco.addInstruction(interpreter::Opcode::maUnary, 1, 1); }
        virtual void compileContinue(interpreter::BytecodeObject& bco) const
            { bco.addInstruction(interpreter::Opcode::maUnary, 2, 2); }
        virtual void compileCleanup(interpreter::BytecodeObject& bco) const
            { bco.addInstruction(interpreter::Opcode::maUnary, 3, 3); }
    };
    ParentSCC parent(world);

    // Create
    interpreter::DefaultStatementCompilationContext testee(parent);
    interpreter::StatementCompilationContext& scc = testee;

    TS_ASSERT_EQUALS(&scc.world(), &world);

    // Test
    {
        interpreter::BytecodeObject bco;
        scc.compileBreak(bco);
        TS_ASSERT_EQUALS(bco.getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(bco(0).major, interpreter::Opcode::maUnary);
        TS_ASSERT_EQUALS(bco(0).minor, 1);
    }
    {
        interpreter::BytecodeObject bco;
        scc.compileContinue(bco);
        TS_ASSERT_EQUALS(bco.getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(bco(0).major, interpreter::Opcode::maUnary);
        TS_ASSERT_EQUALS(bco(0).minor, 2);
    }
        {
        interpreter::BytecodeObject bco;
        scc.compileCleanup(bco);
        TS_ASSERT_EQUALS(bco.getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(bco(0).major, interpreter::Opcode::maUnary);
        TS_ASSERT_EQUALS(bco(0).minor, 3);
    }
}

