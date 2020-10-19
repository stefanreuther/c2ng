/**
  *  \file u/t_interpreter_statementcompilationcontext.cpp
  *  \brief Test for interpreter::StatementCompilationContext
  */

#include "interpreter/statementcompilationcontext.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/error.hpp"
#include "interpreter/world.hpp"

namespace {
    using interpreter::CompilationContext;

    struct TestHarness {
        afl::io::NullFileSystem fs;
        afl::sys::Log log;
        interpreter::World world;

        TestHarness()
            : fs(), log(), world(log, fs)
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
void
TestInterpreterStatementCompilationContext::testConstructor()
{
    TestHarness h;

    // no parent, but world
    ConcreteStatementCompilationContext t1(h.world);
    TS_ASSERT_EQUALS(&t1.world(), &h.world);

    // no world, but parent
    ConcreteStatementCompilationContext t2(t1);
    TS_ASSERT_EQUALS(&t2.world(), &h.world);
}

/** Test default code generation method implementations. */
void
TestInterpreterStatementCompilationContext::testDefaults()
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::BytecodeObject bco;

    // Break/Continue fail, Cleanup succeeds
    TS_ASSERT_THROWS(t.compileBreak(bco), interpreter::Error);
    TS_ASSERT_THROWS(t.compileContinue(bco), interpreter::Error);
    TS_ASSERT_THROWS_NOTHING(t.compileCleanup(bco));

    // No code generated
    TS_ASSERT_EQUALS(bco.getNumInstructions(), 0U);
}

/** Test setOneLineSyntax(). */
void
TestInterpreterStatementCompilationContext::testOneLineSyntax()
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::StatementCompilationContext& t1 = t.setOneLineSyntax();
    TS_ASSERT_EQUALS(&t, &t1);

    TS_ASSERT( t.hasFlag(CompilationContext::RefuseBlocks));
    TS_ASSERT( t.hasFlag(CompilationContext::ExpressionsAreStatements));
    TS_ASSERT(!t.hasFlag(CompilationContext::WantTerminators));
}

/** Test setBlockSyntax(). */
void
TestInterpreterStatementCompilationContext::testBlockSyntax()
{
    TestHarness h;
    ConcreteStatementCompilationContext t(h.world);
    interpreter::StatementCompilationContext& t1 = t.setBlockSyntax();
    TS_ASSERT_EQUALS(&t, &t1);

    TS_ASSERT(!t.hasFlag(CompilationContext::RefuseBlocks));
    TS_ASSERT( t.hasFlag(CompilationContext::ExpressionsAreStatements));
    TS_ASSERT( t.hasFlag(CompilationContext::WantTerminators));
}
