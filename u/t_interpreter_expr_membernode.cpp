/**
  *  \file u/t_interpreter_expr_membernode.cpp
  *  \brief Test for interpreter::expr::MemberNode
  */

#include "interpreter/expr/membernode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
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
        TestContext(int32_t& var)
            : m_var(var)
            { }
        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            {
                TS_ASSERT_EQUALS(index, 42U);
                interpreter::checkIntegerArg(m_var, value);
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                TS_ASSERT_EQUALS(index, 42U);
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
            { return new TestContext(m_var); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { TS_FAIL("enumProperties"); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<Test>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
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

        Environment(String_t name)
            : data(10),
              contextNode(std::auto_ptr<afl::data::Value>(new TestContext(data))),
              log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, name, 42)
            { }
    };
}

void
TestInterpreterExprMemberNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::MemberNode testee("MEM", env.contextNode);

    // Compile: read it
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.data = 42;
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 42);
}

void
TestInterpreterExprMemberNode::testStore()
{
    Environment env("testStore");
    interpreter::expr::MemberNode testee("MEM", env.contextNode);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(55)));

    // Compile: write it
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileStore(*bco, interpreter::CompilationContext(env.world), value);

    // Run
    env.data = 42;
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    // - updated value must remain on stack
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 55);

    // - data must have been changed
    TS_ASSERT_EQUALS(env.data, 55);
}

void
TestInterpreterExprMemberNode::testCondition()
{
    Environment env("testCondition");
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
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 2);
}

void
TestInterpreterExprMemberNode::testReadWrite()
{
    Environment env("testReadWrite");
    interpreter::expr::MemberNode testee("MEM", env.contextNode);

    // Compile: 'incr x.MEM'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileRead(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maUnary, interpreter::unInc, 0);
    testee.compileWrite(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.data = 23;
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 24);
    TS_ASSERT_EQUALS(env.data, 24);
}

