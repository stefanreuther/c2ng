/**
  *  \file interpreter/test/expressionverifier.cpp
  *  \brief Class interpreter::test::ExpressionVerifier
  */

#include "interpreter/test/expressionverifier.hpp"
#include "afl/base/deleter.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
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

using afl::test::Assert;
using afl::except::AssertionFailedException;

class interpreter::test::ExpressionVerifier::TestContext : public SingleContext, public Context::PropertyAccessor {
 public:
    TestContext(ExpressionVerifier& parent)
        : m_parent(parent)
        { }
    virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
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

    virtual void set(PropertyIndex_t index, const afl::data::Value* value)
        {
            // ex GlobalTestVars::set
            int32_t iv;
            if (checkIntegerArg(iv, value)) {
                m_parent.set(index, iv);
            }
        }

    virtual afl::data::Value* get(PropertyIndex_t index)
        {
            // ex GlobalTestVars::get
            return makeIntegerValue(m_parent.get(index));
        }

    virtual String_t toString(bool) const
        { return "#<TestContext>"; }
    virtual game::map::Object* getObject()
        { return 0; }
    virtual void enumProperties(PropertyAcceptor&) const
        { }
    virtual TestContext* clone() const
        { return new TestContext(m_parent); }
    virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
        { rejectStore(out, aux, ctx); }

 private:
    ExpressionVerifier& m_parent;
};



interpreter::test::ExpressionVerifier::ExpressionVerifier(afl::test::Assert a)
    : m_assert(a),
      m_extraContext(),
      m_extraKeymap()
{
    // ex ExpressionTestHelper::ExpressionTestHelper
    clear();
}

void
interpreter::test::ExpressionVerifier::setNewExtraContext(Context* ctx)
{
    m_extraContext.reset(ctx);
}

void
interpreter::test::ExpressionVerifier::setNewExtraKeymap(util::Keymap* km)
{
    m_extraKeymap.reset(km);
}

int32_t
interpreter::test::ExpressionVerifier::get(size_t index) const
{
    m_assert.check("index", index < NUM_VALUES);
    return m_values[index];
}

void
interpreter::test::ExpressionVerifier::set(size_t index, int32_t value)
{
    m_assert.check("index", index < NUM_VALUES);
    m_values[index] = value;
}

void
interpreter::test::ExpressionVerifier::clear()
{
    for (size_t i = 0; i < NUM_VALUES; ++i) {
        m_values[i] = 0;
    }
}

void
interpreter::test::ExpressionVerifier::verifyInteger(const char* expr, int result)
{
    // ex ExpressionTestHelper::checkIntegerExpression, u/t_int.h:checkIntExpr
    verifyScalar(expr, result, false);
}

void
interpreter::test::ExpressionVerifier::verifyBoolean(const char* expr, bool result)
{
    // ex ExpressionTestHelper::checkBooleanExpression, u/t_int.h:checkBoolExpr
    verifyScalar(expr, result, true);
}

void
interpreter::test::ExpressionVerifier::verifyFile(const char* expr, int result)
{
    // ex ExpressionTestHelper::checkFileExpression, u/t_int.h:checkFileExpr
    Assert me(m_assert(expr));
    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));

        Process exec(world, "verifyFile", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run succeeds", exec.getState() == Process::Ended);

        const afl::data::Value* resv = exec.getResult();
        me.check("non-null result", resv != 0);

        const FileValue* iv = dynamic_cast<const FileValue*>(resv);
        me.check("file result", iv != 0);
        me.checkEqual("file number", iv->getFileNumber(), result);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }
}

void
interpreter::test::ExpressionVerifier::verifyNull(const char* expr)
{
    // ex ExpressionTestHelper::checkNullExpression, u/t_int.h:checkNullExpr
    Assert me(m_assert(expr));
    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));

        Process exec(world, "verifyNull", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run succeeds", exec.getState() == Process::Ended);
        me.check("null result", exec.getResult() == 0);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }
}

void
interpreter::test::ExpressionVerifier::verifyString(const char* expr, const char* result)
{
    // ex ExpressionTestHelper::checkStringExpression, u/t_int.h:checkStringExpr
    Assert me(m_assert(expr));
    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));

        Process exec(world, "verifyString", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run succeeds", exec.getState() == Process::Ended);

        const afl::data::Value* resv = exec.getResult();
        me.check("non-null result", resv != 0);

        const afl::data::StringValue* iv = dynamic_cast<const afl::data::StringValue*>(resv);
        me.check("string result", iv != 0);
        me.checkEqual("string value", iv->getValue(), result);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }

}

void
interpreter::test::ExpressionVerifier::verifyFloat(const char* expr, double result)
{
    // ex ExpressionTestHelper::checkFloatExpression, u/t_int.h:checkFloatExpr
    Assert me(m_assert(expr));

    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));

        Process exec(world, "verifyFloat", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run succeeds", exec.getState() == Process::Ended);

        const afl::data::Value* resv = exec.getResult();
        me.check("non-null result", resv != 0);

        const afl::data::FloatValue* iv = dynamic_cast<const afl::data::FloatValue*>(resv);
        me.check("float result", iv != 0);
        me.check("float lower bound", iv->getValue() <= result + 0.01);
        me.check("float upper bound", result - 0.01 <= iv->getValue());
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }
}

void
interpreter::test::ExpressionVerifier::verifyExecutionError(const char* expr)
{
    // ex ExpressionTestHelper::checkFailureExpression, u/t_int.h:checkExecFailure
    Assert me(m_assert(expr));
    bool compiled = false;
    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));
        compiled = true;

        Process exec(world, "verifyExecutionError", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run fails", exec.getState() == Process::Failed);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (...) {
        me.check("compiled", compiled);
    }
}

void
interpreter::test::ExpressionVerifier::verifyCompileError(const char* expr)
{
    // ex ExpressionTestHelper::checkBadExpression
    Assert me(m_assert(expr));

    afl::sys::Log logger;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    World world(logger, tx, fs);
    afl::base::Deleter del;

    Tokenizer tok(expr);
    const interpreter::expr::Node* node = 0;
    try {
        node = &interpreter::expr::Parser(tok, del).parse();
    }
    catch (...) {
        me.fail("exception during parse");
    }
    me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

    BCORef_t bco = BytecodeObject::create(true);
    bool threw = false;
    try {
        node->compileValue(*bco, CompilationContext(world));
    }
    catch (Error&) {
        threw = true;
    }
    catch (...) {
    }
    me.check("expect interpreter::Error", threw);
}

void
interpreter::test::ExpressionVerifier::verifyParseError(const char* expr)
{
    // ex ExpressionTestHelper::checkRejectedExpression
    Assert me(m_assert(expr));

    afl::base::Deleter del;
    Tokenizer tok(expr);
    bool threw = false;
    try {
        interpreter::expr::Parser(tok, del).parse();
    }
    catch (Error&) {
        threw = true;
    }
    catch (...) {
    }
    me.check("expect interpreter::Error", threw);
}

void
interpreter::test::ExpressionVerifier::verifyStatement(const char* stmt)
{
    // ex ExpressionTestHelper::checkStatement, u/t_int_statement.cc:testStatement
    Assert me(m_assert(stmt));
    try {
        // Build a command source
        MemoryCommandSource mcs;
        mcs.addLines(afl::string::toMemory(stmt));

        // Build environment
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);

        // Build compilation environment
        Process exec(world, "checkStatement", 9);
        setupContexts(exec);

        DefaultStatementCompilationContext scc(world);
        scc.withStaticContext(&exec);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.ExpressionsAreStatements);

        BCORef_t bco = BytecodeObject::create(true);
        StatementCompiler::Result result = StatementCompiler(mcs).compileList(*bco, scc);

        me.check("compile result", result != StatementCompiler::CompiledExpression);

        exec.pushFrame(bco, false);
        exec.run();
        me.check("run ok", exec.getState() == Process::Ended);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }
}

void
interpreter::test::ExpressionVerifier::verifyScalar(const char* expr, int result, bool isBool)
{
    // ex ExpressionTestHelper::checkScalarExpression, u/t_int.h:checkIntegralExpr
    Assert me(m_assert(expr));
    try {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        World world(logger, tx, fs);
        afl::base::Deleter del;

        Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        me.check("parse complete", tok.getCurrentToken() == tok.tEnd);

        BCORef_t bco = BytecodeObject::create(true);
        node.compileValue(*bco, CompilationContext(world));

        Process exec(world, "verifyScalar", 9);
        setupContexts(exec);
        exec.pushFrame(bco, false);
        exec.run();
        me.check("run ok", exec.getState() == Process::Ended);

        const afl::data::Value* resv = exec.getResult();
        me.check("non-null result", resv);

        const afl::data::ScalarValue* iv;
        if (isBool) {
            iv = dynamic_cast<const afl::data::BooleanValue*>(resv);
        } else {
            iv = dynamic_cast<const afl::data::IntegerValue*>(resv);
        }
        me.check("scalar result", iv);
        me.checkEqual("scalar value", iv->getValue(), result);
    }
    catch (AssertionFailedException&) {
        throw;
    }
    catch (std::exception& e) {
        me.fail(std::string("exception: ") + e.what());
    }
    catch (...) {
        me.fail("exception");
    }
}

void
interpreter::test::ExpressionVerifier::setupContexts(Process& exec)
{
    exec.pushNewContext(new TestContext(*this));
    if (m_extraContext.get() != 0) {
        exec.pushNewContext(m_extraContext->clone());
    }
    if (m_extraKeymap.get() != 0) {
        // Manually copy the keymap: an actual keymap is not copyable.
        // - create target (owned by process' world)
        util::KeymapRef_t km = exec.world().keymaps().createKeymap(m_extraKeymap->getName());

        // - get set of keys
        util::KeySet_t set;
        m_extraKeymap->enumKeys(set);

        // - copy keys
        for (util::KeySet_t::const_iterator it = set.begin(); it != set.end(); ++it) {
            km->addKey(*it, m_extraKeymap->lookupCommand(*it), m_extraKeymap->lookupCondition(*it));
        }
    }
}
