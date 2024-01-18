/**
  *  \file test/interpreter/expr/membernodetest.cpp
  *  \brief Test for interpreter::expr::MemberNode
  */

#include "interpreter/expr/membernode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    /*
     *  Context for testing: publish a single member MEM referring to an integer variable
     */
    class TestContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        TestContext(afl::test::Assert a, int32_t& var)
            : m_assert(a), m_var(var)
            { }
        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            {
                m_assert.checkEqual("TestContext: set", index, 42U);
                interpreter::checkIntegerArg(m_var, value);
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                m_assert.checkEqual("TestContext: get", index, 42U);
                return interpreter::makeIntegerValue(m_var);
            }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("MEM")) {
                    result = 42;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual bool next()
            { return false; }
        virtual TestContext* clone() const
            { return new TestContext(m_assert, m_var); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { m_assert.fail("TestContext: enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<Test>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        afl::test::Assert m_assert;
        int32_t& m_var;
    };

    /*
     *  Canned environment
     */
    struct Environment {
        int32_t data;
        interpreter::expr::LiteralNode contextNode;

        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment(afl::test::Assert a)
            : data(10),
              contextNode(std::auto_ptr<afl::data::Value>(new TestContext(a, data))),
              log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

AFL_TEST("interpreter.expr.MemberNode:compileValue", a)
{
    Environment env(a);
    interpreter::expr::MemberNode testee("MEM", env.contextNode);

    // Compile: read it
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.data = 42;
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 42);
}

AFL_TEST("interpreter.expr.MemberNode:compileStore", a)
{
    Environment env(a);
    interpreter::expr::MemberNode testee("MEM", env.contextNode);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(55)));

    // Compile: write it
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileStore(*bco, interpreter::CompilationContext(env.world), value);

    // Run
    env.data = 42;
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    // - updated value must remain on stack
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 55);

    // - data must have been changed
    a.checkEqual("21. data", env.data, 55);
}

AFL_TEST("interpreter.expr.MemberNode:compileCondition", a)
{
    Environment env(a);
    interpreter::expr::MemberNode testee("MEM", env.contextNode);

    // Compile: basically, "if (x.MEM, 2, 3)".
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    interpreter::BytecodeObject::Label_t lthen = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lelse = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lend = bco->makeLabel();

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
    env.data = 10;
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 2);
}

AFL_TEST("interpreter.expr.MemberNode:read+write", a)
{
    Environment env(a);
    interpreter::expr::MemberNode testee("MEM", env.contextNode);

    // Compile: 'incr x.MEM'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileRead(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maUnary, interpreter::unInc, 0);
    testee.compileWrite(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.data = 23;
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 24);
    a.checkEqual("13. data", env.data, 24);
}
