/**
  *  \file test/interpreter/defaultstatementcompilationcontexttest.cpp
  *  \brief Test for interpreter::DefaultStatementCompilationContext
  */

#include "interpreter/defaultstatementcompilationcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"

/** Test standalone DefaultStatementCompilationContext. */
AFL_TEST("interpreter.DefaultStatementCompilationContext:standalone", a)
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

    a.checkEqual("01. world", &scc.world(), &world);

    // Test
    AFL_CHECK_THROWS(a("11. compileBreak"), scc.compileBreak(bco), interpreter::Error);
    AFL_CHECK_THROWS(a("12. compileContinue"), scc.compileContinue(bco), interpreter::Error);
    AFL_CHECK_SUCCEEDS(a("13. compileCleanup"), scc.compileCleanup(bco));

    // None of the above generated any code
    a.checkEqual("21. getNumInstructions", bco.getNumInstructions(), 0U);
}

/** Test DefaultStatementCompilationContext with a parent. */
AFL_TEST("interpreter.DefaultStatementCompilationContext:parented", a)
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

    a.checkEqual("01. world", &scc.world(), &world);

    // Test
    {
        interpreter::BytecodeObject bco;
        scc.compileBreak(bco);
        a.checkEqual("11. getNumInstructions", bco.getNumInstructions(), 1U);
        a.checkEqual("12. major", bco(0).major, interpreter::Opcode::maUnary);
        a.checkEqual("13. minor", bco(0).minor, 1);
    }
    {
        interpreter::BytecodeObject bco;
        scc.compileContinue(bco);
        a.checkEqual("14. getNumInstructions", bco.getNumInstructions(), 1U);
        a.checkEqual("15. major", bco(0).major, interpreter::Opcode::maUnary);
        a.checkEqual("16. minor", bco(0).minor, 2);
    }
        {
        interpreter::BytecodeObject bco;
        scc.compileCleanup(bco);
        a.checkEqual("17. getNumInstructions", bco.getNumInstructions(), 1U);
        a.checkEqual("18. major", bco(0).major, interpreter::Opcode::maUnary);
        a.checkEqual("19. minor", bco(0).minor, 3);
    }
}
