/**
  *  \file test/interpreter/statementcompilationcontexttest.cpp
  *  \brief Test for interpreter::StatementCompilationContext
  */

#include "interpreter/statementcompilationcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"

namespace {
    using interpreter::CompilationContext;

    struct TestHarness {
        afl::io::NullFileSystem fs;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        interpreter::World world;

        TestHarness()
            : fs(), tx(), log(), world(log, tx, fs)
            { }
    };

    class ConcreteStatementCompilationContext : public interpreter::StatementCompilationContext {
     public:
        template<typename T>
        ConcreteStatementCompilationContext(T& t)
            : StatementCompilationContext(t)
            { }
        virtual void compileBreak(interpreter::BytecodeObject& bco) const
            { defaultCompileBreak(bco); }
        virtual void compileContinue(interpreter::BytecodeObject& bco) const
            { defaultCompileContinue(bco); }
        virtual void compileCleanup(interpreter::BytecodeObject& bco) const
            { defaultCompileCleanup(bco); }
    };
}

/** Test constructors. */
AFL_TEST("interpreter.StatementCompilationContext:constructor", a)
{
    TestHarness h;

    // no parent, but world
    ConcreteStatementCompilationContext t1(h.world);
    a.checkEqual("01. world", &t1.world(), &h.world);

    // no world, but parent
    ConcreteStatementCompilationContext t2(t1);
    a.checkEqual("11. world", &t2.world(), &h.world);
}

/** Test default code generation method implementations. */
AFL_TEST("interpreter.StatementCompilationContext:defaults", a)
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::BytecodeObject bco;

    // Break/Continue fail, Cleanup succeeds
    AFL_CHECK_THROWS(a("01. compileBreak"), t.compileBreak(bco), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileContinue"), t.compileContinue(bco), interpreter::Error);
    AFL_CHECK_SUCCEEDS(a("03. compileCleanup"), t.compileCleanup(bco));

    // No code generated
    a.checkEqual("11. getNumInstructions", bco.getNumInstructions(), 0U);
}

/** Test setOneLineSyntax(). */
AFL_TEST("interpreter.StatementCompilationContext:setOneLineSyntax", a)
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::StatementCompilationContext& t1 = t.setOneLineSyntax();
    a.checkEqual("01. return", &t, &t1);

    a.check("11. RefuseBlocks",              t.hasFlag(CompilationContext::RefuseBlocks));
    a.check("12. ExpressionsAreStatements",  t.hasFlag(CompilationContext::ExpressionsAreStatements));
    a.check("13. WantTerminators",          !t.hasFlag(CompilationContext::WantTerminators));
}

/** Test setBlockSyntax(). */
AFL_TEST("interpreter.StatementCompilationContext:setBlockSyntax", a)
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::StatementCompilationContext& t1 = t.setBlockSyntax();
    a.checkEqual("01. return", &t, &t1);

    a.check("11. RefuseBlocks",             !t.hasFlag(CompilationContext::RefuseBlocks));
    a.check("12. ExpressionsAreStatements",  t.hasFlag(CompilationContext::ExpressionsAreStatements));
    a.check("13. WantTerminators",           t.hasFlag(CompilationContext::WantTerminators));
}
