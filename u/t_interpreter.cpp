/**
  *  \file u/t_interpreter.cpp
  */

#include "u/t_interpreter.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "afl/io/nullfilesystem.hpp"

class ExpressionTestHelper::TestContext : public interpreter::SingleContext {
 public:
    TestContext(ExpressionTestHelper& parent)
        : m_parent(parent)
        { }
    virtual TestContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
        {
            // ex GlobalTestVars::lookup(const IntNameQuery& name)
            if (name.match("A")) {
                result = 0;
                return this;
            } else if (name.match("B")) {
                result = 1;
                return this;
            } else if (name.match("C")) {
                result = 2;
                return this;
            } else {
                return 0;
            }
        }

    virtual void set(PropertyIndex_t index, afl::data::Value* value)
        {
            // ex GlobalTestVars::set
            interpreter::checkIntegerArg(getVariable(index), value);
        }

    virtual afl::data::Value* get(PropertyIndex_t index)
        {
            // ex GlobalTestVars::get
            return interpreter::makeIntegerValue(getVariable(index));
        }

    virtual String_t toString(bool) const
        { return "#<TestContext>"; }
    virtual game::map::Object* getObject()
        { return 0; }
    virtual void enumProperties(interpreter::PropertyAcceptor&)
        { }
    virtual TestContext* clone() const
        { return new TestContext(m_parent); }
    virtual void store(interpreter::TagNode&, afl::io::DataSink&, afl::charset::Charset&, interpreter::SaveContext&) const
        { throw interpreter::Error::notSerializable(); }

 private:
    ExpressionTestHelper& m_parent;

    int32_t& getVariable(PropertyIndex_t index)
        {
            switch (index) {
             case 0: return m_parent.a;
             case 1: return m_parent.b;
             case 2: return m_parent.c;
             default:
                TS_ASSERT(!"bad variable");
                return m_parent.a;
            }
        }
};


ExpressionTestHelper::ExpressionTestHelper()
    : a(0), b(0), c(0)
{ }

// /** Check expression for integer result.
//     \param expr Expression
//     \param result Required result */
void
ExpressionTestHelper::checkIntegerExpression(const char* expr, int result)
{
    // ex u/t_int.h:checkIntExpr
    checkScalarExpression(expr, result, false);
}

// /** Check expression for boolean result.
//     \param expr Expression
//     \param result Required result */
void
ExpressionTestHelper::checkBooleanExpression(const char* expr, int result)
{
    // ex u/t_int.h:checkBoolExpr
    checkScalarExpression(expr, result, true);
}

// /** Check expression for file number result. */
void
ExpressionTestHelper::checkFileExpression(const char* expr, int result)
{
    // ex u/t_int.h:checkFileExpr
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));

        interpreter::Process exec(world, "checkScalarExpression", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, ok);

        const afl::data::Value* resv = exec.getResult();
        TSM_ASSERT(expr, resv != 0);

        const interpreter::FileValue* iv = dynamic_cast<const interpreter::FileValue*>(resv);
        TSM_ASSERT(expr, iv != 0);
        TSM_ASSERT_EQUALS(expr, iv->getFileNumber(), result);
    }
    catch (...) {
        TSM_ASSERT(expr, 0);
    }
}

// /** Check expression for null result.
//     \param expr Expression */
void
ExpressionTestHelper::checkNullExpression(const char* expr)
{
    // ex u/t_int.h:checkNullExpr
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));

        interpreter::Process exec(world, "checkNullExpression", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, ok);

        TSM_ASSERT(expr, exec.getResult() == 0);
    }
    catch (...) {
        TSM_ASSERT(expr, 0);
    }
}

// /** Check expression for string result.
//     \param expr Expression
//     \param result Required result */
void
ExpressionTestHelper::checkStringExpression(const char* expr, const char* result)
{
    // ex u/t_int.h:checkStringExpr
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));

        interpreter::Process exec(world, "checkStringExpression", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, ok);

        const afl::data::Value* resv = exec.getResult();
        TSM_ASSERT(expr, resv != 0);

        const afl::data::StringValue* iv = dynamic_cast<const afl::data::StringValue*>(resv);
        TSM_ASSERT(expr, iv != 0);
        TSM_ASSERT_EQUALS(expr, iv->getValue(), result);
    }
    catch (...) {
        TSM_ASSERT(expr, 0);
    }
}

// /** Check expression for float result.
//     \param expr Expression
//     \param result Required result, within 0.01 of real result */
void
ExpressionTestHelper::checkFloatExpression(const char* expr, double result)
{
    // ex u/t_int.h:checkFloatExpr
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));

        interpreter::Process exec(world, "checkStringExpression", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, ok);

        const afl::data::Value* resv = exec.getResult();
        TSM_ASSERT(expr, resv != 0);

        const afl::data::FloatValue* iv = dynamic_cast<const afl::data::FloatValue*>(resv);
        TSM_ASSERT(expr, iv != 0);
        TSM_ASSERT_LESS_THAN_EQUALS(expr, iv->getValue(), result + 0.01);
        TSM_ASSERT_LESS_THAN_EQUALS(expr, result - 0.01, iv->getValue());
    }
    catch (...) {
        TSM_ASSERT(expr, 0);
    }
}

void
ExpressionTestHelper::checkFailureExpression(const char* expr)
{
    // ex u/t_int.h:checkExecFailure
    bool compiled = false;
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));
        compiled = true;

        interpreter::Process exec(world, "checkExecFailure", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, !ok);
    }
    catch (...) {
        TSM_ASSERT(expr, compiled);
    }
}

void
ExpressionTestHelper::checkBadExpression(const char* expr)
{
    afl::sys::Log logger;
    afl::io::NullFileSystem fs;
    interpreter::World world(logger, fs);

    interpreter::Tokenizer tok(expr);
    std::auto_ptr<interpreter::expr::Node> node;
    TSM_ASSERT_THROWS_NOTHING(expr, node.reset(interpreter::expr::Parser(tok).parse()));
    TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

    interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
    TSM_ASSERT_THROWS(expr, node->compileValue(*bco, interpreter::CompilationContext(world)), interpreter::Error);
}

void
ExpressionTestHelper::checkRejectedExpression(const char* expr)
{
    interpreter::Tokenizer tok(expr);
    std::auto_ptr<interpreter::expr::Node> node;
    TSM_ASSERT_THROWS(expr, node.reset(interpreter::expr::Parser(tok).parse()), interpreter::Error);
}


void
ExpressionTestHelper::checkScalarExpression(const char* expr, int result, bool isBool)
{
    // ex u/t_int.h:checkIntegralExpr
    try {
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        interpreter::Tokenizer tok(expr);
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(tok).parse());
        TSM_ASSERT_EQUALS(expr, tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        node->compileValue(*bco, interpreter::CompilationContext(world));

        interpreter::Process exec(world, "checkScalarExpression", 9);
        exec.pushNewContext(new TestContext(*this));
        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(expr, ok);

        const afl::data::Value* resv = exec.getResult();
        TSM_ASSERT(expr, resv != 0);

        const afl::data::ScalarValue* iv;
        if (isBool) {
            iv = dynamic_cast<const afl::data::BooleanValue*>(resv);
        } else {
            iv = dynamic_cast<const afl::data::IntegerValue*>(resv);
        }
        TSM_ASSERT(expr, iv != 0);
        TSM_ASSERT_EQUALS(expr, iv->getValue(), result);
    }
    catch (...) {
        TSM_ASSERT(expr, 0);
    }
}

/** Test statements. Given a (possibly multi-line) statement, Verifies that
    - the statements compile into anything but an expression statement
      (expressions are converted to statements by the compiler using the ExpressionsAreStatements flag)
    - run correctly
    \param stmt Statements, separated by '\n' */
void
ExpressionTestHelper::checkStatement(const char* stmt)
{
    // ex u/t_int_statement.cc:testStatement
    try {
        // Build a command source
        interpreter::MemoryCommandSource mcs;
        mcs.addLines(afl::string::toMemory(stmt));

        // Build environment
        afl::sys::Log logger;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, fs);

        // Build compilation environment
        interpreter::Process exec(world, "checkStatement", 9);
        exec.pushNewContext(new TestContext(*this));

        interpreter::DefaultStatementCompilationContext scc(world);
        scc.withContextProvider(&exec);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.ExpressionsAreStatements);

        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        interpreter::StatementCompiler::Result result = interpreter::StatementCompiler(mcs).compileList(*bco, scc);

        TSM_ASSERT_DIFFERS(stmt, result, interpreter::StatementCompiler::CompiledExpression);

        exec.pushFrame(bco, false);
        bool ok = exec.runTemporary();
        TSM_ASSERT(stmt, ok);
    }
    catch (interpreter::Error& /*e*/) {
        TSM_ASSERT(stmt, 0);
    }
    catch (...) {
        TSM_ASSERT(stmt, 0);
    }
}
