/**
  *  \file test/interpreter/statementcompilertest.cpp
  *  \brief Test for interpreter::StatementCompiler
  */

#include "interpreter/statementcompiler.hpp"

#include "afl/base/ref.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/world.hpp"

// #define DEBUG_DISASSEMBLY
#ifdef DEBUG_DISASSEMBLY
#  include <stdio.h>
namespace {
    void dumpBCO(const interpreter::BytecodeObject& bco, const interpreter::World& world, int level)
    {
        for (size_t i = 0; i < bco.getNumInstructions(); ++i) {
            printf("%5d: %s\n", int(i), bco.getDisassembly(i, world).c_str());
        }
        if (level < 5) {
            const afl::data::Segment& literals = bco.literals();
            for (size_t i = 0; i < literals.size(); ++i) {
                if (const interpreter::SubroutineValue* sub = dynamic_cast<const interpreter::SubroutineValue*>(literals.get(i))) {
                    printf("=== [Nested sub %d] ===\n", int(i));
                    dumpBCO(*sub->getBytecodeObject(), world, level+1);
                }
            }
        }
    }
}
#endif

namespace {
    using afl::base::Ref;
    using afl::data::NameMap;
    using afl::io::ConstMemoryStream;
    using afl::io::InternalDirectory;
    using afl::io::Stream;
    using interpreter::Error;
    using interpreter::Process;
    using interpreter::StatementCompiler;

    class MinGlobalContext : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        explicit MinGlobalContext(interpreter::World& world)
            : m_world(world)
            { }
        ~MinGlobalContext()
            { }

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                NameMap::Index_t i = m_world.globalPropertyNames().getIndexByName(name);
                if (i != NameMap::nil) {
                    result = i;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            { m_world.globalValues().set(index, value); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return afl::data::Value::cloneOf(m_world.globalValues().get(index)); }
        virtual MinGlobalContext* clone() const
            { return new MinGlobalContext(m_world); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<min-global>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        interpreter::World& m_world;
    };

    int toScalar(const afl::data::Value* v)
    {
        const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(v);
        if (iv == 0) {
            throw Error::typeError(Error::ExpectInteger);
        }
        return iv->getValue();
    }

    String_t toString(const afl::data::Value* v)
    {
        const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(v);
        if (sv == 0) {
            throw Error::typeError(Error::ExpectString);
        }
        return sv->getValue();
    }

    class TestHarness {
     public:
        TestHarness(afl::test::Assert a)
            : m_assert(a),
              m_log(),
              m_translator(),
              m_fileSystem(),
              m_world(m_log, m_translator, m_fileSystem),
              m_pProcess()
            {
                setGlobalInt("A", 0);
                setGlobalInt("B", 0);
                setGlobalInt("C", 0);
            }

        interpreter::World& world()
            { return m_world; }

        afl::sys::Log& log()
            { return m_log; }

        Process& process()
            {
                m_assert.checkNonNull("m_pProcess", m_pProcess.get());
                return *m_pProcess;
            }

        void checkCompile(const char* stmt)
            {
                m_assert.checkEqual("compile", compile(stmt), StatementCompiler::EndOfInput);
            }

        void checkCompileExpression(const char* stmt)
            {
                m_assert.checkEqual("compileExpression", compileExpression(stmt), StatementCompiler::CompiledExpression);
            }

        void checkFailCompile(const char* stmt)
            {
                AFL_CHECK_THROWS(m_assert("compile"), compile(stmt), interpreter::Error);
            }

        void checkFailCompileExpression(const char* stmt)
            {
                AFL_CHECK_THROWS(m_assert("compileExpression"), compileExpression(stmt), interpreter::Error);
            }

        void checkRun(const char* stmt, bool ignoreState = false)
            {
                checkCompile(stmt);
                m_assert.checkNonNull("checkRun: m_pProcess", m_pProcess.get());
                m_pProcess->run(0);
                if (!ignoreState) {
                    m_assert.checkEqual("getState", m_pProcess->getState(), Process::Ended);
                }
            }

        void checkRunExpression(const char* stmt)
            {
                checkCompileExpression(stmt);
                m_assert.checkNonNull("checkRunExpression: m_pProcess", m_pProcess.get());
                m_pProcess->run(0);
            }

        void checkIntegerExpressionStatement(const char* stmt, int expectedResult)
            {
                checkRunExpression(stmt);
                m_assert.checkEqual("getState", m_pProcess->getState(), Process::Ended);
                m_assert.checkEqual("result",   toScalar(m_pProcess->getResult()), expectedResult);
            }

        const afl::data::Value* globalValue(const char* name)
            {
                NameMap::Index_t i = m_world.globalPropertyNames().getIndexByName(name);
                m_assert.check(name, i != NameMap::nil);
                return m_world.globalValues().get(i);
            }

        void setGlobalInt(const char* name, int32_t value)
            {
                NameMap::Index_t i = m_world.globalPropertyNames().getIndexByName(name);
                m_assert.check(name, i != NameMap::nil);

                afl::data::IntegerValue iv(value);
                m_world.globalValues().set(i, &iv);
            }

     private:
        afl::test::Assert m_assert;
        afl::sys::Log m_log;
        afl::string::NullTranslator m_translator;
        afl::io::NullFileSystem m_fileSystem;
        interpreter::World m_world;
        std::auto_ptr<Process> m_pProcess;

        StatementCompiler::Result compile(const char* stmt)
        {
            // Build a command source
            interpreter::MemoryCommandSource mcs;
            mcs.addLines(afl::string::toMemory(stmt));

            // Build compilation environment
            m_pProcess.reset(new Process(m_world, "checkCompile", 9));
            m_pProcess->pushNewContext(new MinGlobalContext(m_world));

            interpreter::DefaultStatementCompilationContext scc(m_world);
            // No StaticContext, we're in multiline mode
            scc.withFlag(scc.LinearExecution);
            scc.withFlag(scc.ExpressionsAreStatements);

            // Push frame into process.
            // Normally the BCO should be complete before this, but there's no reason we cannot push an incomplete BCO.
            interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
            m_pProcess->pushFrame(bco, false);

#ifdef DEBUG_DISASSEMBLY
            StatementCompiler::Result r = StatementCompiler(mcs).compileList(*bco, scc);
            printf("=== [Code] ===\n%s\n=== [Assembly] ===\n", stmt);
            dumpBCO(*bco, m_world, 0);
            return r;
#else
            return StatementCompiler(mcs).compileList(*bco, scc);
#endif
        }

        StatementCompiler::Result compileExpression(const char* stmt)
        {
            // Build a command source
            interpreter::MemoryCommandSource mcs;
            mcs.addLines(afl::string::toMemory(stmt));

            // Build compilation environment
            m_pProcess.reset(new Process(m_world, "checkCompile", 9));
            m_pProcess->pushNewContext(new MinGlobalContext(m_world));

            interpreter::DefaultStatementCompilationContext scc(m_world);
            scc.withStaticContext(&*m_pProcess);
            scc.withFlag(scc.LinearExecution);

            // Push frame into process.
            // Normally the BCO should be complete before this, but there's no reason we cannot push an incomplete BCO.
            interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
            m_pProcess->pushFrame(bco, true);

#ifdef DEBUG_DISASSEMBLY
            StatementCompiler::Result r = StatementCompiler(mcs).compile(*bco, scc);
            printf("=== [ %s ] ===\n", stmt);
            dumpBCO(*bco, m_world, 0);
            return r;
#else
            return StatementCompiler(mcs).compile(*bco, scc);
#endif
        }
    };
}


/** Test expression statements.
    This tests just the parser.
    The expression interpreter is tested in detail elsewhere,
    therefore the expressions can be simple,
    and we limit ourselves to testing expressions yielding integers.
    The idea is to simply make sure that we correctly compile syntactically ambiguous statements. */
AFL_TEST("interpreter.StatementCompiler:expression-statement", a)
{
    // ex IntStatementTestSuite::testExprStatement
    TestHarness h(a);

    // Operators: ";"
    h.checkIntegerExpressionStatement("a;97", 97);

    // Operators: ":="
    h.checkIntegerExpressionStatement("a:=3", 3);
    h.checkIntegerExpressionStatement("b:=c:=0", 0);
    a.checkEqual("01. global A", toScalar(h.globalValue("A")), 3);
    a.checkEqual("02. global B", toScalar(h.globalValue("B")), 0);
    a.checkEqual("03. global C", toScalar(h.globalValue("C")), 0);

    // Operators: "Or", "Xor"
    h.checkIntegerExpressionStatement("a or b", 1);
    h.checkIntegerExpressionStatement("a xor a", 0);

    // Operators: "And"
    h.checkIntegerExpressionStatement("a and a", 1);

    // Operators: "Not"
    h.checkIntegerExpressionStatement("not a", 0);

    // Operators: comparisons
    h.checkIntegerExpressionStatement("a>0", 1);
    h.checkIntegerExpressionStatement("a<10", 1);
    h.checkIntegerExpressionStatement("a<3", 0);
    h.checkIntegerExpressionStatement("a>=0", 1);
    h.checkIntegerExpressionStatement("a<=3", 1);
    h.checkIntegerExpressionStatement("a<>99", 1);
    h.checkIntegerExpressionStatement("a=7", 7);               // assignment
    a.checkEqual("11. global A", toScalar(h.globalValue("A")), 7);
    h.checkIntegerExpressionStatement("a=3 or 2", 1);          // comparison

    // Operators: "#", "&"
    h.checkIntegerExpressionStatement("a&b;9", 9);
    h.checkIntegerExpressionStatement("a#b;9", 9);

    // Operators: "+", "-"
    h.checkIntegerExpressionStatement("a+3", 10);
    h.checkIntegerExpressionStatement("a-3", 4);

    // Operators: "*", "/", "\", "Mod"
    h.checkIntegerExpressionStatement("a*3", 21);
    h.checkIntegerExpressionStatement("a/1;12", 12);
    h.checkIntegerExpressionStatement("a\\2", 3);
    h.checkIntegerExpressionStatement("a mod 2", 1);

    // Operators: unary "+", "-"
    h.checkIntegerExpressionStatement("-3", -3);
    h.checkIntegerExpressionStatement("+3", +3);

    // Operators: "^"
    h.checkIntegerExpressionStatement("a^2", 49);

    // Operators: "(...)"
    h.checkIntegerExpressionStatement("(9)", 9);
    h.checkIntegerExpressionStatement("(9)*2", 18);

    // Operators: function call
    h.checkIntegerExpressionStatement("isempty(z(0))", 1);

    // Firsts: identifiers
    h.checkIntegerExpressionStatement("a", 7);

    // Firsts: numbers
    h.checkIntegerExpressionStatement("1+1", 2);
    h.checkIntegerExpressionStatement("1.3*99;5", 5);

    // Firsts: strings
    h.checkIntegerExpressionStatement("'a';99", 99);
    h.checkIntegerExpressionStatement("'a'+'b';98", 98);

    // Unknown identifier fails compile because we have a StaticContext, so it needs to be known.
    h.checkFailCompileExpression("unk");
}

/** Test misplaced keywords. */
AFL_TEST("interpreter.StatementCompiler:misplaced-keywords", a)
{
    // Static failures (fail always, or depend on compilation flags)
    TestHarness h(a);
    h.checkFailCompile("case");         // only within 'Select Case'
    h.checkFailCompile("else");         // only within 'If', 'Try'
    h.checkFailCompile("endif");        // only after 'If'
    h.checkFailCompile("endon");        // only after 'On'
    h.checkFailCompile("endselect");    // only after 'Select Case'
    h.checkFailCompile("endsub");       // only after 'Sub'
    h.checkFailCompile("endfunction");  // only after 'Function'
    h.checkFailCompile("endtry");       // only after 'Try'
    h.checkFailCompile("endwith");      // only after 'With'
    h.checkFailCompile("loop");         // only after 'Do'
    h.checkFailCompile("next");         // only after 'For', 'ForEach'
    h.checkFailCompile("endstruct");    // only after 'Struct'
    h.checkFailCompile("restart");      // reserved for auto-tasks
    h.checkFailCompile("until");        // only within 'Do', 'Loop'
    h.checkFailCompile("while");        // only within 'Do', 'Loop'
}

/** Test failures of Break, Continue. */
AFL_TEST("interpreter.StatementCompiler:misplaced-break", a)
{
    // Dynamic failures (fail depending on StatementCompilationContext)
    TestHarness h(a);
    h.checkFailCompile("Break");        // only within a loop that implements it
    h.checkFailCompile("Continue");     // only within a loop that implements it

    // Syntactic failures
    h.checkFailCompile("foreach a do Break 1");
    h.checkFailCompile("foreach a do continue 1");
}

/*
 *  Test various flavours of "If", "Else", "Else If".
 */

// Generic success sequence
AFL_TEST("interpreter.StatementCompiler:If", a)
{
    TestHarness h(a);
    h.checkRun("a:=3");
    a.checkEqual("01. global A", toScalar(h.globalValue("A")), 3);

    h.checkRun("if a=4 then\n"
               "  a:=5\n"
               "else\n"
               "  a:=6\n"
               "endif");
    a.checkEqual("11. global A", toScalar(h.globalValue("A")), 6);

    h.checkRun("if a=5 then\n"
               "  a:=6\n"
               "else if a=6 then\n"
               "  a=7\n"
               "endif");
    a.checkEqual("21. global A", toScalar(h.globalValue("A")), 7);

    h.checkRun("if a=5 then\n"
               "  a:=6\n"
               "else if a=6 then\n"
               "  a=7\n"
               "else if a=7 then\n"
               "  a=8\n"
               "else\n"
               "  a:=9\n"
               "endif");
    a.checkEqual("31. global A", toScalar(h.globalValue("A")), 8);

    h.checkRun("if a=5 then %second\n"
               "  a:=6\n"
               "else if a=6 then\n"
               "  a=7\n"
               "else if a=7 then\n"
               "  a=8\n"
               "else\n"
               "  a:=9\n"
               "endif");
    a.checkEqual("41. global A", toScalar(h.globalValue("A")), 9);

    h.checkRun("if a=9 then a:=10");
    a.checkEqual("51. global A", toScalar(h.globalValue("A")), 10);
}

// Syntax error - missing "Then"
AFL_TEST("interpreter.StatementCompiler:If:error:missing-then", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a print 1\n");
}

// Syntax error - multiline in one-liner
AFL_TEST("interpreter.StatementCompiler:If:error:multiline-in-oneliner", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then if b\n"
                       "endif");
}

// Syntax error - eof in block
AFL_TEST("interpreter.StatementCompiler:If:error:eof", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then\n"
                       "print b");
}

// Syntax error - duplicate else
AFL_TEST("interpreter.StatementCompiler:If:error:duplicate-else", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then\n"
                       "  a:=1\n"
                       "else\n"
                       "  a:=2\n"
                       "else\n"
                       "  a:=3\n"
                       "endif");
}

// Syntax error - garbage after else
AFL_TEST("interpreter.StatementCompiler:If:error:garbage-after-else", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then\n"
                       "  a:=1\n"
                       "else what\n"
                       "  a:=2\n"
                       "endif");
}

// Syntax error - garbage after endif
AFL_TEST("interpreter.StatementCompiler:If:error:garbage-after-endif", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then\n"
                       "  a:=2\n"
                       "endif a");
}

// Syntax error - mismatching separator
AFL_TEST("interpreter.StatementCompiler:If:error:bad-endIf", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then\n"
                       "  a:=1\n"
                       "endsub\n");
}

/*
 *  Test 'For' statement.
 */

// Basic iteration
AFL_TEST("interpreter.StatementCompiler:For", a)
{
    TestHarness h(a);
    h.checkRun("for b:=1 to 10 do a:=a+b");
    a.checkEqual("01. global A", toScalar(h.globalValue("A")), 55);
}

// Backward iteration: body must not be entered
AFL_TEST("interpreter.StatementCompiler:For:backward-range", a)
{
    TestHarness h(a);
    h.checkRun("for b:=10 to 1 do abort 1");
}

// Body must be entered once
AFL_TEST("interpreter.StatementCompiler:For:unit-range", a)
{
    TestHarness h(a);
    h.checkRun("for b:=20 to 20 do a:=a+99");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 99);
}

AFL_TEST("interpreter.StatementCompiler:For:negative-unit-range", a)
{
    TestHarness h(a);
    h.checkRun("for b:=-20 to -20 do a:=a+b");
    a.checkEqual("global A", toScalar(h.globalValue("A")), -20);
}

// Basic iteration, multi-line
AFL_TEST("interpreter.StatementCompiler:For:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("for b:=1 to 10\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 55);
}

// Basic iteration, multi-line, optional 'do' keyword
AFL_TEST("interpreter.StatementCompiler:For:multi-line-do", a)
{
    TestHarness h(a);
    h.checkRun("a:=10\n"
               "for b:=1 to 10 do\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 65);
}

// Continue
AFL_TEST("interpreter.StatementCompiler:For:Continue", a)
{
    TestHarness h(a);
    h.checkRun("for b:=1 to 10 do\n"
               "  if b mod 2 = 0 then continue\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 25);
}

// Break
AFL_TEST("interpreter.StatementCompiler:For:Break", a)
{
    TestHarness h(a);
    h.checkRun("for b:=1 to 10 do\n"
               "  if b mod 2 = 0 then break\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
}

// Return (=cleanup)
AFL_TEST("interpreter.StatementCompiler:For:Return", a)
{
    TestHarness h(a);
    h.checkRun("for b:=1 to 10 do\n"
               "  if b=4 then return\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 6);
}

// Varying limit (must not affect loop)
AFL_TEST("interpreter.StatementCompiler:For:variable-limit", a)
{
    TestHarness h(a);
    h.checkRun("c:=10\n"
               "for b:=1 to c do\n"
               "  c:=3\n"
               "  a:=a+1\n"
               "next");
    a.checkEqual("global C", toScalar(h.globalValue("C")), 3);
    a.checkEqual("global A", toScalar(h.globalValue("A")), 10);
}

// Varying limit with Continue
AFL_TEST("interpreter.StatementCompiler:For:variable-limit:Continue", a)
{
    TestHarness h(a);
    h.checkRun("c:=10\n"
               "for b:=1 to c do\n"
               "  if b mod 2 = 0 then continue\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 25);
}

// Varying limit with Break
AFL_TEST("interpreter.StatementCompiler:For:variable-limit:Break", a)
{
    TestHarness h(a);
    h.checkRun("c:=10\n"
               "for b:=1 to c do\n"
               "  if b mod 2 = 0 then break\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
}

// Varying limit with Return (=cleanup)
AFL_TEST("interpreter.StatementCompiler:For:variable-limit:Return", a)
{
    TestHarness h(a);
    h.checkRun("c:=10\n"
               "for b:=1 to c do\n"
               "  if b=4 then return\n"
               "  a:=a+b\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 6);
}

// Syntax error
AFL_TEST("interpreter.StatementCompiler:For:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("for a+1 to 5 do b:=c");
    h.checkFailCompile("for 1 to 5 do b:=c");
    h.checkFailCompile("for i:=1, 5 do b:=c");
    h.checkFailCompile("for i:=1 to 10\nnext i\n");
}

/*
 *  Test "Do"/"Loop" statements.
 */
// Basic Do/While loop
AFL_TEST("interpreter.StatementCompiler:Do-While", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do while a<10\n"
               "  a:=a+1\n"
               "  b:=b+1\n"
               "loop");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 10);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 9);
}

// Basic Do/Until loop
AFL_TEST("interpreter.StatementCompiler:Do-Until", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do until a>10\n"
               "  a:=a+1\n"
               "  b:=b+1\n"
               "loop");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 11);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 10);
}

// Basic Do/Loop/While loop
AFL_TEST("interpreter.StatementCompiler:Loop-While", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  a:=a+1\n"
               "  b:=b+1\n"
               "loop while a<10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 10);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 9);
}

// Basic Do/Loop/Until loop
AFL_TEST("interpreter.StatementCompiler:Loop-Until", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  a:=a+1\n"
               "  b:=b+1\n"
               "loop until a>10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 11);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 10);
}

// Do/While entered with wrong condition
AFL_TEST("interpreter.StatementCompiler:Do-While:skip", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do while a<1\n"
               "  b:=99\n"
               "loop");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 0);
}

// Do/Loop/While entered with wrong condition
AFL_TEST("interpreter.StatementCompiler:Loop-While:skip", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  b:=b+99\n"
               "loop while a<1");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 99);
}

// Condition with side-effect
AFL_TEST("interpreter.StatementCompiler:Loop-While:side-effect", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  b:=b+1\n"
               "loop while (a:=a+1)<10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 10);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 9);
}

// Continue
AFL_TEST("interpreter.StatementCompiler:Loop-While:Continue", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  continue\n"
               "  b:=b+1\n"
               "loop while (a:=a+1)<10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 10);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 0);
}

// Break
AFL_TEST("interpreter.StatementCompiler:Loop-While:Break", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  break\n"
               "  b:=b+1\n"
               "loop while (a:=a+1)<10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 0);
}

// Return
AFL_TEST("interpreter.StatementCompiler:Loop-While:Return", a)
{
    TestHarness h(a);
    h.checkRun("a:=1; b:=0\n"
               "do\n"
               "  b:=b+4\n"
               "  return\n"
               "loop while (a:=a+1)<10");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 4);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Do-Loop:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("if 1 then do\n"
                       "loop while false\n");
    h.checkFailCompile("do\n"
                       "next\n");
}

/*
 *  Test "Select Case" statements.
 */

// Basic Select Case
AFL_TEST("interpreter.StatementCompiler:Select", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 1);
    h.checkRun("select case a\n"
               "  case 0\n"
               "    b:=9\n"
               "  case 1\n"
               "    b:=8\n"
               "  case 2\n"
               "    b:=7\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 8);
}

// Basic Select Case - formatting variant
AFL_TEST("interpreter.StatementCompiler:Select:variant", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 1);
    h.checkRun("select case a\n"
               "  % we can have comments here\n"
               "\n"
               "  % and blank lines\n"
               "  case 0\n"
               "    b:=9\n"
               "  case 1\n"
               "    b:=8\n"
               "  case 2\n"
               "    b:=7\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 8);
}

// No matching case
AFL_TEST("interpreter.StatementCompiler:Select:no-match", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 1);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 10\n"
               "    b:=9\n"
               "  case 11\n"
               "    b:=8\n"
               "  case 12\n"
               "    b:=7\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 0);
}

// No matching case, matching else
AFL_TEST("interpreter.StatementCompiler:Select:Else", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 1);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 10\n"
               "    b:=9\n"
               "  case 11\n"
               "    b:=8\n"
               "  case 12\n"
               "    b:=7\n"
               "  case else\n"
               "    b:=6\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 6);
}

// Ranges
AFL_TEST("interpreter.StatementCompiler:Select:ranges", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 5);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 1,2,3\n"
               "    b:=1\n"
               "  case 4,5,6\n"
               "    b:=2\n"
               "  case 7,8,9\n"
               "    b:=3\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 2);
}

// Match first in range
AFL_TEST("interpreter.StatementCompiler:Select:ranges:match-first", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 1);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 1,2,3\n"
               "    b:=1\n"
               "  case 4,5,6\n"
               "    b:=2\n"
               "  case 7,8,9\n"
               "    b:=3\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 1);
}

// Match last in range
AFL_TEST("interpreter.StatementCompiler:Select:ranges:match-last", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 3);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 1,2,3\n"
               "    b:=1\n"
               "  case 4,5,6\n"
               "    b:=2\n"
               "  case 7,8,9\n"
               "    b:=3\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 1);
}

// Match last item
AFL_TEST("interpreter.StatementCompiler:Select:match-last-case", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 9);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case 1,2,3\n"
               "    b:=1\n"
               "  case 4,5,6\n"
               "    b:=2\n"
               "  case 7,8,9\n"
               "    b:=3\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 3);
}

// Relations
AFL_TEST("interpreter.StatementCompiler:Select:relations", a)
{
    TestHarness h(a);
    h.setGlobalInt("A", 5);
    h.setGlobalInt("B", 0);
    h.checkRun("select case a\n"
               "  case is <5\n"
               "    b:=1\n"
               "  case is >=5\n"
               "    b:=2\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 2);
}

// Empty
AFL_TEST("interpreter.StatementCompiler:Select:empty", a)
{
    TestHarness h(a);
    h.checkRun("select case a\n"
               "endselect");
}

// Empty with Else
AFL_TEST("interpreter.StatementCompiler:Select:empty-with-else", a)
{
    TestHarness h(a);
    h.checkRun("select case a\n"
               "  case else\n"
               "    b:=3\n"
               "endselect");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 3);
}

// Break from switch (interesting because both for and select place stuff on the stack)
AFL_TEST("interpreter.StatementCompiler:Select:Break", a)
{
    TestHarness h(a);
    h.checkRun("for c:=1 to 10 do\n"
               "  select case c\n"
               "    case 1,3,5,7,9\n"
               "      b:=b+c\n"            // 1,3,5,7
               "    case is <5\n"
               "      b:=b+2*c\n"          // 2*2, 2*4
               "    case is =8\n"
               "      break\n"
               "    case else\n"           // 3*6
               "      b:=b+3*c\n"
               "  endselect\n"
               "next");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 46);
}

// Continue from switch
AFL_TEST("interpreter.StatementCompiler:Select:Continue", a)
{
    TestHarness h(a);
    h.checkRun("for c:=1 to 10 do\n"
               "  select case c\n"
               "    case 1,3,5,7,9\n"
               "      b:=b+c\n"            // 1,3,5,7,9
               "    case is =8\n"
               "      continue\n"
               "  endselect\n"
               "  b:=b+1\n"
               "next");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 34);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Select:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("select from students");     // it's "select case"
    h.checkFailCompile("if a then select case c\n"  // within single-line
                       "endselect\n");
    h.checkFailCompile("select case a\n"            // statement instead of clause
                       "  if b then print c\n"
                       "endselect");
    h.checkFailCompile("select case a\n");          // eof before first clause
    h.checkFailCompile("select case a\n"            // multiple else
                       "case else\n"
                       "else\n"
                       "endselect\n");
    h.checkFailCompile("select case a\n"            // wrong terminator
                       "case 1\n"
                       "endif\n");
    h.checkFailCompile("select case a\n"            // trailing comma
                       "case 1,\n"
                       "endselect\n");
    h.checkFailCompile("select case a\n"            // wrong separator
                       "case 1)\n"
                       "endselect\n");
    h.checkFailCompile("select case a\n"            // missing operator
                       "case is 1\n"
                       "endselect\n");
    h.checkFailCompile("select case a\n"            // arg at terminator
                       "endselect a\n");
}

/*
 *  Eval statement.
 */

// Single statement
AFL_TEST("interpreter.StatementCompiler:Eval:single", a)
{
    TestHarness h(a);
    h.checkRun("Eval 'a:=1'");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
}

// Multiple statements
AFL_TEST("interpreter.StatementCompiler:Eval:multiple", a)
{
    TestHarness h(a);
    h.checkRun("Eval 'a:=2', 'b:=a+3'");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 2);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 5);
}

// Multiline statement
AFL_TEST("interpreter.StatementCompiler:Eval:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("Eval 'for a:=1 to 10', 'b:=a+5', 'next'");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 15);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Eval:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("Eval");
    h.checkFailCompile("Eval 'a:=1',");
    h.checkFailCompile("Eval)");
}

/*
 *  End statement
 */

// Good case
AFL_TEST("interpreter.StatementCompiler:End", a)
{
    TestHarness h(a);
    h.checkRun("End", true);
    a.checkEqual("getState", h.process().getState(), Process::Terminated);
}

// Bad case
AFL_TEST("interpreter.StatementCompiler:End:error", a)
{
    TestHarness h(a);
    h.checkFailCompile("End 1");
}

/*
 *  Stop statement.
 */

// Good case
AFL_TEST("interpreter.StatementCompiler:Stop", a)
{
    TestHarness h(a);
    h.checkRun("Stop", true);
    a.checkEqual("01. getState", h.process().getState(), Process::Suspended);
}

// Bad case
AFL_TEST("interpreter.StatementCompiler:Stop:error", a)
{
    TestHarness h(a);
    h.checkFailCompile("Stop 1");
}

/*
 *  Abort statement
 */

// Good case: nullary
AFL_TEST("interpreter.StatementCompiler:Abort:nullary", a)
{
    TestHarness h(a);
    h.checkRun("Abort", true);
    a.checkEqual("01. getState", h.process().getState(), Process::Failed);
    a.check("02. getError", !String_t(h.process().getError().what()).empty());
}

// Good case: unary
AFL_TEST("interpreter.StatementCompiler:Abort:unary", a)
{
    TestHarness h(a);
    h.checkRun("Abort 'boom'", true);
    a.checkEqual("11. getState", h.process().getState(), Process::Failed);
    a.checkEqual("12. getError", String_t(h.process().getError().what()), "boom");
}

// Bad case
AFL_TEST("interpreter.StatementCompiler:Abort:error", a)
{
    TestHarness h(a);
    h.checkFailCompile("Abort)");
    h.checkFailCompile("Abort 1)");
    h.checkFailCompile("Abort 1,2,3");
}

/*
 *  Test subroutine definition and calling (Sub, Call).
 */

// Regular call
AFL_TEST("interpreter.StatementCompiler:Sub", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x,y)\n"
               "  a:=x+y\n"
               "endsub\n"
               "tt 3,4\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Regular call, syntactic variant of definition
AFL_TEST("interpreter.StatementCompiler:Sub:variant", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x,y)\n"
               "  a:=x+y\n"
               "endsub tt\n"
               "tt 3,4\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Regular call with Call
AFL_TEST("interpreter.StatementCompiler:Sub:Call", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x,y)\n"
               "  a:=x+y\n"
               "endsub\n"
               "call tt 3,4\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Successful disambiguation
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:static", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x,y)\n"
               "  a:=x+y\n"
               "endsub\n"
               "tt +5,6\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 11);
}

// Runtime disambiguation
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:runtime", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x)\n"
               "  a:=3*x\n"
               "endsub\n"
               "tt +5\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 15);
}

// Explicit disambiguation
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:explicit", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x)\n"
               "  a:=3*x\n"
               "endsub\n"
               "call tt, +6\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 18);
}

// Disambiguarion of builtin name, undefined
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:builtin:undef", a)
{
    TestHarness h(a);
    h.checkRun("sin(3)");
}

// Disambiguarion of builtin name, defined
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:builtin:defined", a)
{
    TestHarness h(a);
    h.checkRun("sub sin(x)\n"
               "  a:=x\n"
               "endsub\n"
               "sin(3)");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 3);
}

// Disambiguation fails both ways
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:fails", a)
{
    TestHarness h(a);
    h.checkFailCompile("a+1*(2:=3)");
}

// Nullary
AFL_TEST("interpreter.StatementCompiler:Sub:nullary", a)
{
    TestHarness h(a);
    h.checkRun("sub tt\n"
               "  a:=12\n"
               "endsub\n"
               "tt\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Call nullary
AFL_TEST("interpreter.StatementCompiler:Sub:nullary:Call", a)
{
    TestHarness h(a);
    h.checkRun("sub tt\n"
               "  a:=12\n"
               "endsub\n"
               "call (tt)\n");
    a.checkEqual("81. global A", toScalar(h.globalValue("A")), 12);
}

// Runtime error: failed disambiguation
AFL_TEST("interpreter.StatementCompiler:Sub:Call:failed-disambiguation", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x)\n"
               "  a:=3*x\n"
               "endsub\n"
               "call tt +6\n", true);
    a.checkEqual("getState", h.process().getState(), Process::Failed);
}

// Runtime error: failed disambiguation - should have created warning ahead
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:warning", a)
{
    TestHarness h(a);
    afl::test::LogListener ll;
    h.log().addListener(ll);
    h.checkCompile("sub tt(x)\n"
                   "  a:=3*x\n"
                   "endsub\n"
                   "call tt +6\n");
    a.checkEqual("getNumMessages", ll.getNumMessages(), 1U);
}

// Same thing, for "#" operator
AFL_TEST("interpreter.StatementCompiler:Sub:disambiguation:warning:hash", a)
{
    TestHarness h(a);
    afl::test::LogListener ll;
    h.log().addListener(ll);
    h.checkCompile("sub tt(x)\n"
                   "  a:=3*x\n"
                   "endsub\n"
                   "call tt #6\n");
    a.checkEqual("getNumMessages", ll.getNumMessages(), 1U);
}

// Optional args (missing)
AFL_TEST("interpreter.StatementCompiler:Sub:optional-args:not-given", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x, optional y)\n"
               "  a:=if(y,y,x)\n"
               "endsub\n"
               "tt 4");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 4);
}

// Optional args (given)
AFL_TEST("interpreter.StatementCompiler:Sub:optional-args:given", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x, optional y)\n"
               "  a:=if(y,y,x)\n"
               "endsub\n"
               "tt 4,5");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 5);
}

// Varargs
AFL_TEST("interpreter.StatementCompiler:Sub:varargs", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x, y())\n"
               "  a:=x + dim(y)\n"
               "endsub\n"
               "tt 4,8,8,8");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Local definition still produces global sub
AFL_TEST("interpreter.StatementCompiler:Sub:nested", a)
{
    TestHarness h(a);
    h.checkRun("sub foo\n"
               " function bar\n"
               "  return 7\n"
               " endfunction\n"
               "endsub\n"
               "foo\n"
               "x := bar()\n");
    a.checkNonNull("global BAR", h.globalValue("BAR"));
    a.checkEqual("global X", toScalar(h.globalValue("X")), 7);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Sub:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then sub foo\nendsub\n");               // multiline after single
    h.checkFailCompile("sub(x)\nendsub\n");                          // missing name
    h.checkFailCompile("sub a(optional b, optional c)\nendsub\n");   // duplicate optional
    h.checkFailCompile("sub a(optional, mandatory)\nendsub\n");      // missing parameter name
    h.checkFailCompile("sub a(b()\nendsub\n");                       // missing closen paren
    h.checkFailCompile("sub a(b\nendsub\n");                         // missing closen paren
    h.checkFailCompile("sub a(x y)\nendsub\n");                      // missing comma
    h.checkFailCompile("sub a\nendfunction\n");                      // mismatching keyword
    h.checkFailCompile("sub a\nreturn 1\nendsub");                   // return with value in sub
    h.checkFailCompile("sub a(x)\nendsub\na,1");                     // comma in call (fails all disambiguation)
    h.checkFailCompile("sub a\nendsub q\n");                         // wrong arg at terminator
}

// Error - sub called as function
AFL_TEST("interpreter.StatementCompiler:Sub:called-as-function", a)
{
    TestHarness h(a);
    h.checkRun("sub tt(x,y)\n"
               "  a:=x+y\n"
               "endsub\n"
               "b:=tt(3,4)\n", true);
    a.checkEqual("161. getState", h.process().getState(), Process::Failed);
}

/*
 *  Test function definition and calling.
 */

// Regular call
AFL_TEST("interpreter.StatementCompiler:Function", a)
{
    TestHarness h(a);
    h.checkRun("function ff(x,y)\n"
               "  return x+y\n"
               "endfunction\n"
               "a := ff(3,4)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Regular call, syntactic variant of definition
AFL_TEST("interpreter.StatementCompiler:Function:variant", a)
{
    TestHarness h(a);
    h.checkRun("function ff(x,y)\n"
               "  return x+y\n"
               "endfunction ff\n"
               "a := ff(3,4)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Runtime disambiguation, valid as both expression or call
AFL_TEST("interpreter.StatementCompiler:Function:disambiguation:runtime", a)
{
    TestHarness h(a);
    h.checkRun("function ff(x)\n"
               "  a:=x\n"
               "endfunction\n"
               "ff(3)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 3);
}

// Nullary called without parens, this is a no-op!
AFL_TEST("interpreter.StatementCompiler:Function:no-op", a)
{
    TestHarness h(a);
    h.checkRun("function tt\n"
               "  a:=12\n"
               "endfunction\n"
               "tt\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);   // unchanged!
}

// Recursion
AFL_TEST("interpreter.StatementCompiler:Function:recursion", a)
{
    TestHarness h(a);
    h.checkRun("function fib(n)\n"
               "  return if(n>1, fib(n-1) + fib(n-2), 1)\n"
               "endfunction\n"
               "a := fib(5)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 8);
}

// Syntax errors (same as for Sub)
AFL_TEST("interpreter.StatementCompiler:Function:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then function foo\nendsub\n");          // multiline after single
    h.checkFailCompile("function(x)\nendfunction\n");                // missing name
    h.checkFailCompile("function a(optional b, optional c)\nendfunction\n");   // duplicate optional
    h.checkFailCompile("function a(optional, mandatory)\nendfunction\n");      // missing parameter name
    h.checkFailCompile("function a(b()\nendfunction\n");             // missing closen paren
    h.checkFailCompile("function a(b\nendfunction\n");               // missing closen paren
    h.checkFailCompile("function a(x y)\nendfunction\n");            // missing comma
    h.checkFailCompile("function a\nendsub\n");                      // mismatching keyword
    h.checkFailCompile("function a\nreturn\nendfunction");           // return without value
    h.checkFailCompile("function a\nreturn 1,2\nendfunction");       // return with 2 values
    h.checkFailCompile("function a\nreturn 1\nendfunction q\n");     // wrong arg at terminator
}

// Error - function called as sub
AFL_TEST("interpreter.StatementCompiler:Function:called-as-sub", a)
{
    TestHarness h(a);
    h.checkRun("function ff(x,y)\n"
               "  return x+y\n"
               "endfunction\n"
               "ff 3,4\n", true);
    a.checkEqual("getState", h.process().getState(), Process::Failed);
}

// Error - function called with Call
AFL_TEST("interpreter.StatementCompiler:Function:Call", a)
{
    TestHarness h(a);
    h.checkRun("function ff(x,y)\n"
               "  return x+y\n"
               "endfunction\n"
               "call ff, 3, 4\n", true);
    a.checkEqual("getState", h.process().getState(), Process::Failed);
}

/*
 *  Test CreateShipProperty, CreatePlanetProperty.
 */

// CreateShipProperty, good case
AFL_TEST("interpreter.StatementCompiler:CreateShipProperty", a)
{
    TestHarness h(a);
    h.checkRun("createshipproperty sp1, ship.sp2");
    a.check("SP1", h.world().shipPropertyNames().getIndexByName("SP1") != NameMap::nil);
    a.check("SP2", h.world().shipPropertyNames().getIndexByName("SP2") != NameMap::nil);
}

// CreatePlanetProperty, good case
AFL_TEST("interpreter.StatementCompiler:CreatePlanetProperty", a)
{
    TestHarness h(a);
    h.checkRun("createplanetproperty pp1, planet.pp2");
    a.check("PP1", h.world().planetPropertyNames().getIndexByName("PP1") != NameMap::nil);
    a.check("PP2", h.world().planetPropertyNames().getIndexByName("PP2") != NameMap::nil);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:CreateShipProperty:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("createshipproperty");             // no name
    h.checkFailCompile("createshipproperty ship.");       // all prefix
    h.checkFailCompile("createshipproperty a,");          // trailing comma
    h.checkFailCompile("createshipproperty a b");         // missing comma
    h.checkFailCompile("createshipproperty a:=1");        // no initializer allowed
    h.checkFailCompile("createshipproperty a()");         // no initializer allowed
    h.checkFailCompile("createshipproperty(a)");          // no parens allowed
}

/*
 *  Test creating local variables (Dim, Local).
 */

// Create and use variable; observe shadowing
AFL_TEST("interpreter.StatementCompiler:Dim", a)
{
    TestHarness h(a);
    h.checkRun("dim a:=9\n"     // shadows the global one
               "b:=a\n");       // set global one
    a.checkEqual("global B", toScalar(h.globalValue("B")), 9);
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);
}

// Variant
AFL_TEST("interpreter.StatementCompiler:Local", a)
{
    TestHarness h(a);
    h.checkRun("local a:=10\n"
               "b:=a\n");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 10);
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);
}

// Variant
AFL_TEST("interpreter.StatementCompiler:Dim-Local", a)
{
    TestHarness h(a);
    h.checkRun("dim local a:=11\n"
               "b:=a\n");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 11);
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);
}

// Multiple initialisations
AFL_TEST("interpreter.StatementCompiler:Dim:repeated", a)
{
    TestHarness h(a);
    h.checkRun("dim a:=7\n"
               "dim a:=8\n"
               "b:=a\n");
    a.checkEqual("global B", toScalar(h.globalValue("B")), 7);
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);
}

// Typed initialisation - integer
AFL_TEST("interpreter.StatementCompiler:Dim:init:int", a)
{
    TestHarness h(a);
    h.checkRun("dim zz as integer\n"
               "a:='<'#zz#'>'\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "<0>");
}

// Typed initialisation - string
AFL_TEST("interpreter.StatementCompiler:Dim:init:str", a)
{
    TestHarness h(a);
    h.checkRun("dim zz as string\n"
               "a:='<'#zz#'>'\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "<>");
}

// Uninitialized array
AFL_TEST("interpreter.StatementCompiler:Dim:array", a)
{
    TestHarness h(a);
    h.checkRun("dim zz(20)\n"
               "a:='<'#zz(1)#','#zz(19)#'>'\n");
    a.checkNull("global A", h.globalValue("A"));
}

// Array
AFL_TEST("interpreter.StatementCompiler:Dim:array:int", a)
{
    TestHarness h(a);
    h.checkRun("dim zz(20) as integer\n"
               "a:='<'&zz(1)&','&zz(19)&'>'\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "<0,0>");
}

// 2D Array
AFL_TEST("interpreter.StatementCompiler:Dim:array:int-2d", a)
{
    TestHarness h(a);
    h.checkRun("dim zz(20,10) as integer\n"
               "a:='<'&zz(1,1)&','&zz(19,9)&'>'\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "<0,0>");
}

// Within a function (pre-allocation)
AFL_TEST("interpreter.StatementCompiler:Dim:pre-alloc", a)
{
    TestHarness h(a);
    h.checkRun("function ff\n"
               "  local v = 9\n"
               "  return v\n"
               "endfunction\n"
               "a:=ff()\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 9);
}

// Within a function (pre-allocation, shadowed)
AFL_TEST("interpreter.StatementCompiler:Dim:pre-alloc:shadowed", a)
{
    TestHarness h(a);
    h.checkRun("function ff(v)\n"
               "  local v = 9\n"
               "  return v\n"
               "endfunction\n"
               "a:=ff(7)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// Within a Do/Loop
AFL_TEST("interpreter.StatementCompiler:Dim:in-loop", a)
{
    TestHarness h(a);
    h.checkRun("do while (a:=a+1) < 5\n"
               "  local zz = a\n"
               "loop\n"
               "b := zz\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 5);
    a.checkEqual("global B", toScalar(h.globalValue("B")), 1);
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Dim:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("dim");
    h.checkFailCompile("dim a,");
    h.checkFailCompile("dim(a)");
    h.checkFailCompile("dim local shared a");
    h.checkFailCompile("dim local shared a");
    h.checkFailCompile("dim for");
    h.checkFailCompile("dim a()");
    h.checkFailCompile("dim a(1 2)");
    h.checkFailCompile("dim a(1,)");
    h.checkFailCompile("dim a as");
    h.checkFailCompile("dim a(10) as");
}

// Error - unknown type
AFL_TEST("interpreter.StatementCompiler:Dim:invalid-type", a)
{
    TestHarness h(a);
    h.checkRun("dim a as yellow_submarine\n", true);
    a.checkEqual("getState", h.process().getState(), Process::Failed);
}

/*
 *  Test creating static variables (Dim, Static)
 */

// Create and use variable
AFL_TEST("interpreter.StatementCompiler:Dim-Static", a)
{
    TestHarness h(a);
    h.checkRun("sub ss\n"
               "  dim static aa:=9\n"     // shadows the global one
               "endsub\n"
               "ss\n"
               "a:=aa\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 9);
}

// Syntax variant
AFL_TEST("interpreter.StatementCompiler:Static", a)
{
    TestHarness h(a);
    h.checkRun("sub ss\n"
               "  static aa:=9\n"         // shadows the global one
               "endsub\n"
               "ss\n"
               "a:=aa\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 9);
}

// Immediately invisible
AFL_TEST("interpreter.StatementCompiler:Static:immediately-shadowed", a)
{
    TestHarness h(a);
    h.checkRun("function ff(xx)\n"
               "  static xx:=9\n"         // shadows the global one
               "  return xx\n"
               "endfunction\n"
               "a:=ff(12)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

/*
 *  Test creating shared variables (Dim, Shared).
 */

// Create variable. Value needs to be globally visible.
AFL_TEST("interpreter.StatementCompiler:Dim-Shared", a)
{
    TestHarness h(a);
    h.checkRun("dim shared vv = 7");
    a.checkEqual("global VV", toScalar(h.globalValue("VV")), 7);
}

// Create variable. Syntax variant.
AFL_TEST("interpreter.StatementCompiler:Shared", a)
{
    TestHarness h(a);
    h.checkRun("shared vv = 7");
    a.checkEqual("global VV", toScalar(h.globalValue("VV")), 7);
}

// Create variable that already exists.
AFL_TEST("interpreter.StatementCompiler:Shared:existing", a)
{
    TestHarness h(a);
    h.checkRun("shared a = 7");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 0);
}

/*
 *  Test Bind command.
 */

// Bind one key
AFL_TEST("interpreter.StatementCompiler:Bind", a)
{
    TestHarness h(a);
    util::KeymapRef_t k = h.world().keymaps().createKeymap("K");
    h.checkRun("bind k 'a' := 'cmda'");
    a.checkEqual("getStringFromAtom", h.world().atomTable().getStringFromAtom(k->lookupCommand('a')), "cmda");
}

// Bind multiple keys
AFL_TEST("interpreter.StatementCompiler:Bind:multiple", a)
{
    TestHarness h(a);
    util::KeymapRef_t k = h.world().keymaps().createKeymap("K");
    h.checkRun("bind k 'x' := 'cmdx', 'y' := 9999");
    a.checkEqual("lookupCommand x", h.world().atomTable().getStringFromAtom(k->lookupCommand('x')), "cmdx");
    a.checkEqual("lookupCommand y", k->lookupCommand('y'), util::Atom_t(9999));
}

// Using ByName()
AFL_TEST("interpreter.StatementCompiler:Bind:ByName", a)
{
    TestHarness h(a);
    util::KeymapRef_t k = h.world().keymaps().createKeymap("K");
    h.checkRun("n := 'k'\n"
               "bind byname(n) 'c' := 'cmdc'");
    a.checkEqual("lookupCommand c", h.world().atomTable().getStringFromAtom(k->lookupCommand('c')), "cmdc");
}

// Syntax errors
AFL_TEST("interpreter.StatementCompiler:Bind:syntax-errors", a)
{
    TestHarness h(a);
    h.world().keymaps().createKeymap("K");
    h.checkFailCompile("bind k");
    h.checkFailCompile("bind k 'c'");
    h.checkFailCompile("bind k 'c' = 'cmdc'");   // fails because entire expression treaded as key
    h.checkFailCompile("bind k 'a':='cmda',");
    h.checkFailCompile("bind k 'a':='cmda')");
}

/*
 *  Test CreateKeymap command.
 */

// Good case: single keymap
AFL_TEST("interpreter.StatementCompiler:CreateKeymap", a)
{
    TestHarness h(a);
    h.checkRun("createkeymap kk");
    a.checkNonNull("getKeymapByName", h.world().keymaps().getKeymapByName("KK"));
}

// Good case: multiple keymaps, with parents
AFL_TEST("interpreter.StatementCompiler:CreateKeymap:multiple", a)
{
    TestHarness h(a);
    h.checkRun("createkeymap kk, mm, nn(kk, mm)");

    util::KeymapRef_t kk = h.world().keymaps().getKeymapByName("KK");
    util::KeymapRef_t mm = h.world().keymaps().getKeymapByName("MM");
    util::KeymapRef_t nn = h.world().keymaps().getKeymapByName("NN");
    a.checkNonNull("01. getKeymapByName KK", kk);
    a.checkNonNull("02. getKeymapByName MM", mm);
    a.checkNonNull("03. getKeymapByName NN", nn);
    a.check("04. hasParent", nn->hasParent(*kk));
    a.check("05. hasParent", nn->hasParent(*mm));
}

// Byname
AFL_TEST("interpreter.StatementCompiler:CreateKeymap:ByName", a)
{
    TestHarness h(a);
    h.checkRun("a := 'kk'\n"
               "b := 'mm'\n"
               "createkeymap byname(a), byname(b)(byname(a))");
    util::KeymapRef_t kk = h.world().keymaps().getKeymapByName("KK");
    util::KeymapRef_t mm = h.world().keymaps().getKeymapByName("MM");
    a.checkNonNull("01. getKeymapByName KK", kk);
    a.checkNonNull("02. getKeymapByName MM", mm);
    a.check("03. hasParent", mm->hasParent(*kk));
}

// Errors
AFL_TEST("interpreter.StatementCompiler:CreateKeymap:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("createkeymap");
    h.checkFailCompile("createkeymap a,");
    h.checkFailCompile("createkeymap a+");
    h.checkFailCompile("createkeymap a(");
    h.checkFailCompile("createkeymap b, a(b");
    h.checkFailCompile("createkeymap b, a(b,");
    h.checkFailCompile("createkeymap b, a(b+");
}

/*
 *  ForEach
 */


// Basic iteration, single-line
AFL_TEST("interpreter.StatementCompiler:ForEach", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h do a:=10*a + value\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Basic iteration, multi-line
AFL_TEST("interpreter.StatementCompiler:ForEach:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h do\n"
               " a:=10*a + value\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Iteration with named iterator, single-line
AFL_TEST("interpreter.StatementCompiler:ForEach:named", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h as q do a:=10*a + q->value\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Iteration with named iterator, multi-line
AFL_TEST("interpreter.StatementCompiler:ForEach:named:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h as q do\n"
               " a:=10*a + q->value\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Shadowing and Break
AFL_TEST("interpreter.StatementCompiler:ForEach:Break", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "local value=77\n"
               "foreach h\n"
               " a:=value\n"
               " break\n"
               "next\n"
               "a:=value");          // local value in scope again
    a.checkEqual("global A", toScalar(h.globalValue("A")), 77);
}

// Named iterator, break
// Undocumented feature: this leaves the induction variable set. This compiles into
//   pushvar h / sfirst / popvar q
AFL_TEST("interpreter.StatementCompiler:ForEach:named:Break", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h as q do break\n"
               "a := q->value\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 1);
}

// Same thing, empty iterable
AFL_TEST("interpreter.StatementCompiler:ForEach:named:empty", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash()\n"
               "foreach h as q do break\n"
               "a := q->value\n");
    a.checkNull("global A", h.globalValue("A"));
}

// Named iteration, continue
AFL_TEST("interpreter.StatementCompiler:ForEach:named:Continue", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h as q do\n"
               " a:=10*a + q->value\n"
               " continue\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Normal iteration, empty iterable
AFL_TEST("interpreter.StatementCompiler:ForEach:empty", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash()\n"
               "foreach h\n"
               "  abort\n"
               "next\n");
}

// Normal iteration, continue
AFL_TEST("interpreter.StatementCompiler:ForEach:Continue", a)
{
    TestHarness h(a);
    h.checkRun("h := newhash(); h('a') := 1; h('b') := 2\n"
               "foreach h do\n"
               " a:=10*a + value\n"
               " continue\n"
               "next");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Normal iteration, return (also exercises 'local as hash')
AFL_TEST("interpreter.StatementCompiler:ForEach:Return", a)
{
    TestHarness h(a);
    h.checkRun("function ff\n"
               " local h as hash\n"
               " h('a') := 1; h('b') := 2\n"
               " foreach h do\n"
               "  if key='b' then return value\n"
               " next\n"
               "endfunction\n"
               "a:=ff()\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 2);
}

// Named iterator, return
AFL_TEST("interpreter.StatementCompiler:ForEach:named:Return", a)
{
    TestHarness h(a);
    h.checkRun("function ff\n"
               " local h as hash\n"
               " h('a') := 1; h('b') := 2\n"
               " foreach h as e do\n"
               "  if e->key='b' then return e->value\n"
               " next\n"
               "endfunction\n"
               "a:=ff()\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 2);
}

// Errors
AFL_TEST("interpreter.StatementCompiler:ForEach:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("if a then foreach b\nnext\n");    // multi-line in single-line
    h.checkFailCompile("foreach b print a\n");            // missing 'Do'
    h.checkFailCompile("foreach b\nendif\n");             // wrong terminator
    h.checkFailCompile("foreach b as\nnext\n");           // missing name after 'As'
    h.checkFailCompile("foreach b\nnext b\n");            // arg at terminator
}

/*
 *   On/RunHook.
 */

// Running an undefined hook succeeds
AFL_TEST("interpreter.StatementCompiler:RunHook", a)
{
    TestHarness h(a);
    h.checkRun("runhook hoho");
}

// Adding to a hook, commands are executed in sequence
AFL_TEST("interpreter.StatementCompiler:On:sequence", a)
{
    TestHarness h(a);
    h.checkRun("on hoho do a:=10*a+1\n"
               "on hoho do a:=10*a+2\n"
               "runhook hoho");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Multi-line syntax
AFL_TEST("interpreter.StatementCompiler:On:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("on hoho do\n"
               " a:=10*a+1\n"
               " a:=10*a+2\n"
               "endon\n"
               "runhook hoho");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Can add the same thing multiple times
AFL_TEST("interpreter.StatementCompiler:On:repeated", a)
{
    TestHarness h(a);
    h.checkRun("for i:=1 to 5 do on hoho do a:=10*a+1\n"
               "runhook hoho");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 11111);
}

// Hook adding to itself (not contractual)
AFL_TEST("interpreter.StatementCompiler:On:add-to-self", a)
{
    TestHarness h(a);
    h.checkRun("on hoho do a:=10*a+1\n"
               "on hoho do on hoho do a:=10*a+2\n"
               "runhook hoho\n"
               "runhook hoho");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12122);
}

// On ByName
AFL_TEST("interpreter.StatementCompiler:On:ByName", a)
{
    TestHarness h(a);
    h.checkRun("k := 'h1'\n"
               "on byname(k) do a:=7\n"
               "runhook h1\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// RunHook ByName
AFL_TEST("interpreter.StatementCompiler:RunHook:ByName", a)
{
    TestHarness h(a);
    h.checkRun("k := 'h1'\n"
               "on h1 do a:=7\n"
               "runhook byname(k)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 7);
}

// ByName is a valid name and has its special meaning only if followed by '('
AFL_TEST("interpreter.StatementCompiler:RunHook:ByName-as-name", a)
{
    TestHarness h(a);
    h.checkRun("on byname do a:=8\n"
               "runhook byname");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 8);
}

// Errors
AFL_TEST("interpreter.StatementCompiler:RunHook:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("on");                                // missing name
    h.checkFailCompile("on hoho do return");                 // Return not supported in On
    h.checkFailCompile("foreach a do on hoho do break");     // Break not supported
    h.checkFailCompile("foreach a do on hoho do continue");  // Continue not supported
    h.checkFailCompile("if a then on hoho\nendon\n");        // multi-line in one-line
    h.checkFailCompile("on hoho a:=1\n");                    // missing Do
    h.checkFailCompile("if a\non hoho do\nendif\n");         // mismatching delimiter
    h.checkFailCompile("on h(a) do x:=1");                   // possible future syntax

    h.checkFailCompile("runhook");                           // missing name
    h.checkFailCompile("runhook h(1)");                      // possible future syntax
    h.checkFailCompile("runhook byname(x");                  // missing )
    h.checkFailCompile("runhook foo)");                      // expecting EOF
}

/*
 *  Test UseKeymap.
 */


// Normal case. 'UseKeymap X' compiles into 'CC$UseKeymap "X", UI.Prefix'.
AFL_TEST("interpreter.StatementCompiler:UseKeymap", a)
{
    TestHarness h(a);
    h.checkRun("local ui.prefix = 99\n"
               "createkeymap kk\n"
               "sub cc$usekeymap(k,p)\n"
               " a := k & p\n"
               "endsub\n"
               "usekeymap kk");
    a.checkEqual("global A", toString(h.globalValue("A")), "#<keymap:KK>99");
}

// ByName version.
AFL_TEST("interpreter.StatementCompiler:UseKeymap:ByName", a)
{
    TestHarness h(a);
    h.checkRun("local ui.prefix = 99\n"
               "createkeymap q\n"
               "sub cc$usekeymap(k,p)\n"
               " a := k & p\n"
               "endsub\n"
               "usekeymap byname('q') ");
    a.checkEqual("global A", toString(h.globalValue("A")), "#<keymap:Q>99");
}

// Errors
AFL_TEST("interpreter.StatementCompiler:UseKeymap:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("usekeymap");
    h.checkFailCompile("usekeymap a(x)");
    h.checkFailCompile("usekeymap a, b");
}

/*
 *  Test SelectionExec.
 */


// Implicit assignment to current
AFL_TEST("interpreter.StatementCompiler:SelectionExec", a)
{
    TestHarness h(a);
    h.checkRun("sub cc$selectionexec(t,x)\n"
               " a := t & '-' & x\n"
               "endsub\n"
               "selectionexec a+b\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "0-AB|");
}

// Explicit assignment to current
AFL_TEST("interpreter.StatementCompiler:SelectionExec:explicit-current", a)
{
    TestHarness h(a);
    h.checkRun("sub cc$selectionexec(t,x)\n"
               " a := t & '-' & x\n"
               "endsub\n"
               "selectionexec current = a+b\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "0-AB|");
}

// Explicit assignment E
AFL_TEST("interpreter.StatementCompiler:SelectionExec:explicit-target", a)
{
    TestHarness h(a);
    h.checkRun("sub cc$selectionexec(t,x)\n"
               " a := t & '-' & x\n"
               "endsub\n"
               "selectionexec e := a+b\n");
    a.checkEqual("global A", toString(h.globalValue("A")), "5-AB|");
}

// Errors
AFL_TEST("interpreter.StatementCompiler:SelectionExec:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("selectionexec");
    h.checkFailCompile("selectionexec a:=");
    h.checkFailCompile("selectionexec a+b:=c");
    h.checkFailCompile("selectionexec s:=c");
    h.checkFailCompile("selectionexec a=b+");
}

/** Test Struct, With. */

// Basic structure test (also tests Dim...As)
AFL_TEST("interpreter.StatementCompiler:Struct", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\n"
               " e1,e2\n"
               " i1 as integer\n"
               " f1 as float\n"
               " s1 as string\n"
               " a1(10) as integer\n"
               " h1 as hash\n"
               " y1 as any\n"
               "endstruct\n"
               "dim sv as foo\n"
               "e:=sv->e1\n"
               "i:=sv->i1\n"
               "f:=sv->f1\n"
               "s:=sv->s1\n"
               "a:=sv->a1(5)\n"
               "h:=sv->h1\n"
               "y:=sv->y1\n");
    a.checkNull   ("global E", h.globalValue("E"));
    a.checkEqual  ("global I", toScalar(h.globalValue("I")), 0);
    a.checkNonNull("global F", dynamic_cast<const afl::data::FloatValue*>(h.globalValue("F")));
    a.checkEqual  ("global S", toString(h.globalValue("S")), "");
    a.checkEqual  ("global A", toScalar(h.globalValue("A")), 0);
    a.checkNonNull("global H", dynamic_cast<const interpreter::HashValue*>(h.globalValue("H")));
    a.checkNull   ("global Y", h.globalValue("Y"));
}

// Syntactic variant
AFL_TEST("interpreter.StatementCompiler:Struct:variant", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\n"
               " x, y as integer\n"          // only Y will be of type integer
               "endstruct\n"
               "dim sv as foo\n"
               "x:=sv->x\n"
               "y:=sv->y\n");
    a.checkNull ("global X", h.globalValue("X"));
    a.checkEqual("global Y", toScalar(h.globalValue("Y")), 0);
}

// Empty struct
AFL_TEST("interpreter.StatementCompiler:Struct:empty", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\n"
               "endstruct\n"
               "dim shared sv as foo\n");
    a.checkNonNull("global SV", h.globalValue("SV"));
}

// Struct with With
AFL_TEST("interpreter.StatementCompiler:With", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "x := foo()\nx->a := 1\nx->b := 10\n" // constructor is a function
               "with x do\n"
               " c:=a+b\n"
               "endwith\n");
    a.checkEqual("global C", toScalar(h.globalValue("C")), 11);
}

// Struct with With, syntactic variant
AFL_TEST("interpreter.StatementCompiler:With:variant", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "x := foo()\nx->a := 1\nx->b := 10\n" // constructor is a function
               "with x\n"
               " c:=a+b\n"
               "endwith\n");
    a.checkEqual("global C", toScalar(h.globalValue("C")), 11);
}

// Struct with With, single-line version
AFL_TEST("interpreter.StatementCompiler:With:single-line", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "x := foo()\nx->a := 1\nx->b := 10\n" // constructor is a function
               "with x do c:=a+b\n");
    a.checkEqual("global C", toScalar(h.globalValue("C")), 11);
}

// With in loop with break
AFL_TEST("interpreter.StatementCompiler:With:Break", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "for i:=1 to 10 do with foo() do break\n");
    a.checkEqual("getStackSize", h.process().getStackSize(), 0U);
}

// With in loop with continue
AFL_TEST("interpreter.StatementCompiler:With:Continue", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "for i:=1 to 10 do with foo() do continue\n");
    a.checkEqual("getStackSize", h.process().getStackSize(), 0U);
}

// With with Return
AFL_TEST("interpreter.StatementCompiler:With:Return", a)
{
    TestHarness h(a);
    h.checkRun("struct foo\na, b\nendstruct\n"
               "function fn(s)\n"
               " with s do return a\n"
               "endfunction\n"
               "x := foo()\n"
               "x->a := 12\n"
               "a := fn(x)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Local definition still produces global type
AFL_TEST("interpreter.StatementCompiler:Struct:nested", a)
{
    TestHarness h(a);
    h.checkRun("sub foo\n"
               " struct bar\n"
               " endstruct\n"
               "endsub\n"
               "foo\n"
               "x := bar()\n");
    a.checkNonNull("global X",   h.globalValue("X"));
    a.checkNonNull("global BAR", h.globalValue("BAR"));
}

// Errors
AFL_TEST("interpreter.StatementCompiler:Struct:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("struct\nendstruct\n");                 // missing name
    h.checkFailCompile("struct qq\nendstruct rr\n");           // stuff after terminator
    h.checkFailCompile("struct hash\nenstruct\n");             // reserved name
    h.checkFailCompile("struct hash\na,\nendstruct\n");        // trailing comma
    h.checkFailCompile("struct hash\na as\nendstruct\n");      // trailing 'As'
    h.checkFailCompile("struct hash\na\nendif\nendstruct\n");  // wrong terminator
    h.checkFailCompile("if a then struct x\nendstruct\n");     // multiline in single-line
    h.checkFailCompile("struct s\n");                          // EOF
    h.checkFailCompile("struct s\n1\nendstruct\n");            // not a name
    h.checkFailCompile("struct s\na,1\nendstruct\n");          // not a name
    h.checkFailCompile("struct s\na,a\nendstruct\n");          // duplicate name
    h.checkFailCompile("struct s\na+b\nendstruct\n");          // wrong separator
}

AFL_TEST("interpreter.StatementCompiler:With:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("with s print a\n");                    // missing Do
    h.checkFailCompile("if a then with b do\nendwith");        // multiline in single-line
    h.checkFailCompile("with a\nendif\nendwith\n");            // wrong terminator
    h.checkFailCompile("with a\nendwith 1\n");                 // arg at terminator
}

/*
 *  Try
 */

// Try/Abort, simple case
AFL_TEST("interpreter.StatementCompiler:Try", a)
{
    TestHarness h(a);
    h.checkRun("try abort 'hi'");
    a.checkEqual("global SYSTEM.ERR", toString(h.globalValue("SYSTEM.ERR")), "hi");
}

// Try/Abort, multi-line
AFL_TEST("interpreter.StatementCompiler:Try:multi-line", a)
{
    TestHarness h(a);
    h.checkRun("try\n"
               "  abort 'hi'\n"
               "endtry\n");
    a.checkEqual("global SYSTEM.ERR", toString(h.globalValue("SYSTEM.ERR")), "hi");
}

// Try/Abort, with handler
AFL_TEST("interpreter.StatementCompiler:Try:Else", a)
{
    TestHarness h(a);
    h.checkRun("try\n"
               "  abort 'hi'\n"
               "else\n"
               "  a := system.err\n"
               "endtry\n");
    a.checkEqual("global SYSTEM.ERR", toString(h.globalValue("SYSTEM.ERR")), "hi");
    a.checkEqual("global A", toString(h.globalValue("A")), "hi");
}

// Try with Break
AFL_TEST("interpreter.StatementCompiler:Try:Break", a)
{
    TestHarness h(a);
    h.checkRun("for i:=1 to 5\n"
               "  try if i=3 then break\n"
               "next\n");
    a.checkEqual("global I", toScalar(h.globalValue("I")), 3);
}

// Try with Break, Abort case
AFL_TEST("interpreter.StatementCompiler:Try:abort-after-break", a)
{
    TestHarness h(a);
    h.checkRun("for i:=1 to 5\n"
               "  try if i=3 then break\n"
               "next\n"
               "abort", true);
    a.checkEqual("getState", h.process().getState(), Process::Failed);
}

// Try with Continue
AFL_TEST("interpreter.StatementCompiler:Try:Continue", a)
{
    TestHarness h(a);
    h.checkRun("for i:=1 to 5\n"
               "  try if i=3 then continue\n"
               "  a:=a+i\n"
               "next\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 12);
}

// Try with Return
AFL_TEST("interpreter.StatementCompiler:Try:Return", a)
{
    TestHarness h(a);
    h.checkRun("function f(i)\n"
               "  try return i\n"
               "endfunction\n"
               "a:=f(99)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 99);
}

// Nullary Abort
AFL_TEST("interpreter.StatementCompiler:Try:nullary", a)
{
    TestHarness h(a);
    h.checkRun("try abort");
    a.checkDifferent("global SYSTEM.ERR", toString(h.globalValue("SYSTEM.ERR")), "");
}

// Errors
AFL_TEST("interpreter.StatementCompiler:Try:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("abort 'hi')");              // specimen for error in argument list
    h.checkFailCompile("if a then try\nendtry\n");  // multiline in single-line
    h.checkFailCompile("try\nendif\nendtry\n");     // wrong terminator
    h.checkFailCompile("try\nendtry 1\n");          // arg at terminator
}

/*
 *  ReDim
 */

// Normal case
AFL_TEST("interpreter.StatementCompiler:ReDim", a)
{
    TestHarness h(a);
    h.checkRun("dim shared ar(1,2)\n"
               "redim ar(4,5)\n");
    const interpreter::ArrayValue* av = dynamic_cast<const interpreter::ArrayValue*>(h.globalValue("AR"));
    a.check("01. ArrayValue", av);
    a.checkEqual("02. getDimension 0", av->getDimension(0), 2U);
    a.checkEqual("03. getDimension 1", av->getDimension(1), 4U);
    a.checkEqual("04. getDimension 2", av->getDimension(2), 5U);
}

// Multiple re-dims at once
AFL_TEST("interpreter.StatementCompiler:ReDim:multiple", a)
{
    TestHarness h(a);
    h.checkRun("dim shared a1(1,2), a2(3,4,5)\n"
               "redim a1(4,5), a2(6,7,8)\n");
    const interpreter::ArrayValue* a1 = dynamic_cast<const interpreter::ArrayValue*>(h.globalValue("A1"));
    a.check("01. ArrayValue", a1);
    a.checkEqual("02. getDimension 0", a1->getDimension(0), 2U);
    a.checkEqual("03. getDimension 1", a1->getDimension(1), 4U);
    a.checkEqual("04. getDimension 2", a1->getDimension(2), 5U);

    const interpreter::ArrayValue* a2 = dynamic_cast<const interpreter::ArrayValue*>(h.globalValue("A2"));
    a.check("11. ArrayValue", a2);
    a.checkEqual("12. getDimension 0", a2->getDimension(0), 3U);
    a.checkEqual("13. getDimension 1", a2->getDimension(1), 6U);
    a.checkEqual("14. getDimension 2", a2->getDimension(2), 7U);
    a.checkEqual("15. getDimension 3", a2->getDimension(3), 8U);
}

// Errors
AFL_TEST("interpreter.StatementCompiler:ReDim:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("redim a\n");              // no dimensions
    h.checkFailCompile("redim a()\n");            // no dimensions
    h.checkFailCompile("redim a(1 2)\n");         // missing ,
    h.checkFailCompile("redim a(1");              // unexpected eof
    h.checkFailCompile("redim a(1) + b(1)");      // wrong separator
    h.checkFailCompile("redim 7");                // missing name
}

/*
 *  Load/TryLoad.
 */

AFL_TEST("interpreter.StatementCompiler:Load", a)
{
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("loaded.q", *new ConstMemoryStream(afl::string::toBytes("a:=1\n")));
    dir->addStream("bad.q", *new ConstMemoryStream(afl::string::toBytes("a b c\n")));

    // Standard case, Load
    {
        TestHarness h(a);
        h.world().setSystemLoadDirectory(dir.asPtr());
        h.checkRun("load 'loaded.q'");
        a.checkEqual("01. global A", toScalar(h.globalValue("A")), 1);
    }

    // Standard case, TryLoad
    {
        TestHarness h(a);
        h.world().setSystemLoadDirectory(dir.asPtr());
        h.checkRun("tryload 'loaded.q'");
        a.checkEqual("11. global A", toScalar(h.globalValue("A")), 1);
    }

    // Syntax error in script, Load
    {
        TestHarness h(a);
        h.world().setSystemLoadDirectory(dir.asPtr());
        h.checkRun("load 'bad.q'", true);
        a.checkEqual("21. getState", h.process().getState(), Process::Failed);
    }

    // Syntax error in script, TryLoad
    {
        TestHarness h(a);
        h.world().setSystemLoadDirectory(dir.asPtr());
        h.checkRun("tryload 'bad.q'", true);
        a.checkEqual("31. getState", h.process().getState(), Process::Failed);
    }

    // Not found, Load
    {
        TestHarness h(a);
        h.checkRun("load 'loaded.q'", true);
        a.checkEqual("41. getState", h.process().getState(), Process::Failed);
    }

    // Not found, TryLoad
    {
        TestHarness h(a);
        h.checkRun("tryload 'loaded.q'");
    }
}

/** Test Load with pre-execute option. */
AFL_TEST("interpreter.StatementCompiler:Load:pre-exec", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Set load directory
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("loaded.q", *new ConstMemoryStream(afl::string::toBytes("a:=1\n")));
    world.setSystemLoadDirectory(dir.asPtr());

    // Create compiler
    interpreter::MemoryCommandSource mcs;
    mcs.addLine("load 'loaded.q'");

    // Build compilation environment and compile
    interpreter::Process p(world, "testPreexecLoad", 32);
    p.pushNewContext(new MinGlobalContext(world));

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    interpreter::DefaultStatementCompilationContext scc(world);
    scc.withFlag(scc.LinearExecution);
    scc.withFlag(scc.ExpressionsAreStatements);
    scc.withFlag(scc.PreexecuteLoad);
    StatementCompiler::Result result = StatementCompiler(mcs).compileList(*bco, scc);
    a.checkEqual("01. compileList", result, StatementCompiler::EndOfInput);

    // Execute, but without load directory
    world.setSystemLoadDirectory(0);
    p.pushFrame(bco, false);
    p.run(0);
    a.checkEqual("11. getState", p.getState(), Process::Ended);

    NameMap::Index_t i = world.globalPropertyNames().getIndexByName("A");
    a.check("21. global A", i != NameMap::nil);

    const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(world.globalValues().get(i));
    a.check("31. IntegerValue", iv);
    a.checkEqual("32. getValue", iv->getValue(), 1);
}

/*
 *  Print
 */

AFL_TEST("interpreter.StatementCompiler:Print", a)
{
    // Base case
    size_t numMessages;
    {
        TestHarness h(a);
        afl::test::LogListener ll;
        h.log().addListener(ll);
        h.checkRun("a:=1");
        numMessages = ll.getNumMessages();
    }

    // Standard case, multiple args
    {
        TestHarness h(a);
        afl::test::LogListener ll;
        h.log().addListener(ll);
        h.checkRun("print 'a', 3, 'b'");
        a.checkEqual("01. getNumMessages", ll.getNumMessages(), numMessages+1);
    }

    // Standard case, one arg
    {
        TestHarness h(a);
        afl::test::LogListener ll;
        h.log().addListener(ll);
        h.checkRun("print 'a'");
        a.checkEqual("11. getNumMessages", ll.getNumMessages(), numMessages+1);
    }

    // Nullary still produces a line
    {
        TestHarness h(a);
        afl::test::LogListener ll;
        h.log().addListener(ll);
        h.checkRun("print");
        a.checkEqual("21. getNumMessages", ll.getNumMessages(), numMessages+1);
    }

    // Print to file
    {
        TestHarness h(a);
        h.checkRun("sub cc$print(fd,text)\n t:=text\nendsub\n"
                   "print #4, 'hi', 9\n");
        a.checkEqual("31", toString(h.globalValue("T")), "hi9");
    }

    // Print to file, empty line
    {
        TestHarness h(a);
        h.checkRun("sub cc$print(fd,text)\n t:=text\nendsub\n"
                   "print #4\n");
        a.checkEqual("41", toString(h.globalValue("T")), "");
    }
}

/*
 *  Option
 */

// Ranges
AFL_TEST("interpreter.StatementCompiler:Option:ranges", a)
{
    TestHarness h(a);
    h.checkRun("option localsubs(0)\n"
               "option localsubs(1)\n"
               "option localsubs(false)\n"
               "option localsubs(true)\n"
               "option localtypes(0)\n"
               "option localtypes(1)\n"
               "option optimize(-1)\n"
               "option optimize(3)\n"
               "option optimize(+3)\n"
               "option localsubs(0), localtypes(1)\n");
}

// Ignored
AFL_TEST("interpreter.StatementCompiler:Option:unknown", a)
{
    TestHarness h(a);
    h.checkRun("option unknown\n"
               "option unknown(arg)\n"
               "option unknown(arg(arg(arg)),i)\n");
}

// LocalSubs, with a sub
AFL_TEST("interpreter.StatementCompiler:Option:LocalSubs", a)
{
    TestHarness h(a);
    h.checkRun("sub foo\n"
               " option localsubs(1)\n"
               " local sub foo\n"
               "  a:=a+1\n"
               " endsub\n"
               " a:=a+10\n"
               " foo\n"
               "endsub\n"
               "foo\n"
               "foo\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 22);
}

// LocalSubs, with a function
AFL_TEST("interpreter.StatementCompiler:Option:LocalSubs:function", a)
{
    TestHarness h(a);
    h.checkRun("function foo(w)\n"
               " option localsubs(1)\n"
               " local function foo(v)\n"
               "  return v+1\n"
               " endfunction\n"
               " return foo(w)\n"
               "endfunction\n"
               "a:=foo(10)+foo(20)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 32);
}

// LocalSubs, unknown context
AFL_TEST("interpreter.StatementCompiler:Option:LocalSubs:With", a)
{
    TestHarness h(a);
    h.checkRun("struct ty\nendstruct\n"
               "dim tyvar as ty\n"
               "function foo(w)\n"
               " option localsubs(1)\n"
               " with tyvar do\n"
               "  local function foo(v)\n"
               "   return v+1\n"
               "  endfunction\n"
               " endwith\n"
               " return foo(w)\n"
               "endfunction\n"
               "a:=foo(10)+foo(20)\n");
    a.checkEqual("global A", toScalar(h.globalValue("A")), 32);
}

// LocalTypes
AFL_TEST("interpreter.StatementCompiler:Option:LocalTypes", a)
{
    TestHarness h(a);
    h.checkRun("function foo()\n"
               " option localtypes(1)\n"
               " local struct ty\n"
               "  x, y\n"
               " endstruct\n"
               " return ty()\n"
               "endfunction\n"
               "a:=foo()\n");

    const interpreter::StructureValue* sv = dynamic_cast<const interpreter::StructureValue*>(h.globalValue("A"));
    a.checkNonNull("StructureValue", sv);
    a.checkEqual("getIndexByName TY", h.world().globalPropertyNames().getIndexByName("TY"), NameMap::nil);
}

// LocalTypes, syntactic variant
AFL_TEST("interpreter.StatementCompiler:Option:LocalTypes:variant", a)
{
    TestHarness h(a);
    h.checkRun("function foo()\n"
               " option localtypes\n"
               " local struct ty\n"
               "  x, y\n"
               " endstruct\n"
               " dim rv as ty\n"
               " return rv\n"
               "endfunction\n"
               "a:=foo()\n");

    const interpreter::StructureValue* sv = dynamic_cast<const interpreter::StructureValue*>(h.globalValue("A"));
    a.checkNonNull("StructureValue", sv);
    a.checkEqual("getIndexByName TY", h.world().globalPropertyNames().getIndexByName("TY"), NameMap::nil);
}

// Encoding
AFL_TEST("interpreter.StatementCompiler:Option:Encoding", a)
{
    Ref<InternalDirectory> dir = InternalDirectory::create("dir");
    dir->addStream("enc.q", *new ConstMemoryStream(afl::string::toBytes("option encoding(\"cp437\")\n"
                                                                        "a:=\"\x8E\"")));
    TestHarness h(a);
    h.world().setSystemLoadDirectory(dir.asPtr());
    h.checkRun("load 'enc.q'");
    a.checkEqual("global A", toString(h.globalValue("A")), "\xC3\x84");
}

// Errors
AFL_TEST("interpreter.StatementCompiler:Option:syntax-errors", a)
{
    TestHarness h(a);
    h.checkFailCompile("option");                              // missing name
    h.checkFailCompile("option localsubs(2)");                 // out of range
    h.checkFailCompile("option localtypes(2)");                // out of range
    h.checkFailCompile("option optimize(99)");                 // out of range
    h.checkFailCompile("option encoding('sga')");              // unknown encoding name
    h.checkFailCompile("option encoding('latin-1')");          // cannot set encoding on MemoryCommandSource
    h.checkFailCompile("option localsubs(++0)");               // syntax
    h.checkFailCompile("option localsubs(--1)");               // syntax
    h.checkFailCompile("option localsubs(+-1)");               // syntax
    h.checkFailCompile("option localsubs(1");                  // missing )
    h.checkFailCompile("if a then option localsubs");          // not in single-line
    h.checkFailCompile("option encoding");                     // needs to have paren
    h.checkFailCompile("option encoding('X'");                 // needs to have paren
    h.checkFailCompile("option encoding(X)");                  // needs to have string literal
    h.checkFailCompile("option mismatch(a(");                  // unbalanced parens
    h.checkFailCompile("option localsubs(0) localtypes(1)");   // missing comma
}

// Errors - local subs disabled
AFL_TEST("interpreter.StatementCompiler:Option:LocalSubs:disabled", a)
{
    TestHarness h(a);
    h.checkFailCompile("local sub foo\nendsub\n");
    h.checkFailCompile("local function foo\nreturn 1\nendfunction\n");
}

// Errors - local types disabled
AFL_TEST("interpreter.StatementCompiler:Option:LocalTypes:disabled", a)
{
    TestHarness h(a);
    h.checkFailCompile("local struct ss\nendstruct\n");
}

/** Test compileList(), expression case.
    Normally, compileList() is only used with ExpressionsAreStatements,
    which never produces a CompiledExpression result.
    This tests compileList() without said flag. */
AFL_TEST("interpreter.StatementCompiler:compileList", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Build a command source
    interpreter::MemoryCommandSource mcs;
    mcs.addLine("1");

    // Compile
    interpreter::DefaultStatementCompilationContext scc(world);
    scc.withFlag(scc.LinearExecution);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    StatementCompiler::Result r = StatementCompiler(mcs).compileList(*bco, scc);
    a.checkEqual("01. compileList", r, StatementCompiler::EndOfInput);

    // Run
    interpreter::Process proc(world, "testCompileList", 9);
    proc.pushFrame(bco, false);
    proc.run(0);
    a.checkEqual("11. getState", proc.getState(), Process::Ended);
    a.checkEqual("12. getStackSize", proc.getStackSize(), 0U);
}

/** Test special command behaviour. */
AFL_TEST("interpreter.StatementCompiler:SpecialCommand", a)
{
    class TestSpecial : public interpreter::SpecialCommand {
     public:
        TestSpecial(afl::test::Assert a, String_t& out)
            : m_assert(a), m_out(out)
            { }
        virtual void compileCommand(interpreter::Tokenizer& line, interpreter::BytecodeObject& /*bco*/, const interpreter::StatementCompilationContext& /*scc*/)
            {
                // Skip 'SPECIAL'
                m_assert.checkEqual("01. getCurrentToken", line.getCurrentToken(), interpreter::Tokenizer::tIdentifier);
                m_assert.checkEqual("02. getCurrentString", line.getCurrentString(), "SPECIAL");
                line.readNextToken();

                // Skip argument which needs to be a string literal
                m_assert.checkEqual("11. getCurrentToken", line.getCurrentToken(), interpreter::Tokenizer::tString);
                m_out = line.getCurrentString();
                line.readNextToken();
            }
     private:
        afl::test::Assert m_assert;
        String_t& m_out;
    };

    // Standard case
    {
        TestHarness h(a);
        String_t result;
        h.world().addNewSpecialCommand("SPECIAL", new TestSpecial(a("21. standard"), result));
        h.checkCompile("special 'foo'");
        a.checkEqual("22. result", result, "foo");
    }

    // Error case: StatementCompiler checks that special ate up everything
    {
        TestHarness h(a);
        String_t result;
        h.world().addNewSpecialCommand("SPECIAL", new TestSpecial(a("31. error"), result));
        h.checkFailCompile("special 'foo', 2");

        a.checkEqual("32. result", result, "foo");      // Special has been called
    }

    // Error case: cannot define a thing named like our special
    {
        TestHarness h(a);
        String_t result;
        h.world().addNewSpecialCommand("SPECIAL", new TestSpecial(a("41. error"), result));
        h.checkFailCompile("dim special");
        h.checkFailCompile("sub special\nendsub\n");
        h.checkFailCompile("sub blah(special)\nendsub\n");
        h.checkFailCompile("struct special\nendstruct\n");
    }
}
