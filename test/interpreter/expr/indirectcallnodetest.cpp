/**
  *  \file test/interpreter/expr/indirectcallnodetest.cpp
  *  \brief Test for interpreter::expr::IndirectCallNode
  */

#include "interpreter/expr/indirectcallnode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "util/string.hpp"

using interpreter::expr::LiteralNode;

namespace {
    typedef std::map<String_t,int> Data_t;

    String_t packArgs(interpreter::Arguments& args)
    {
        String_t result;
        while (args.getNumArgs() > 0) {
            util::addListItem(result, ",", interpreter::toString(args.getNext(), false));
        }
        return result;
    }

    class TestCallable : public interpreter::IndexableValue {
     public:
        TestCallable(Data_t& data)
            : m_data(data)
            { }
        virtual afl::data::Value* get(interpreter::Arguments& args)
            { return interpreter::makeIntegerValue(m_data[packArgs(args)]); }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            {
                int32_t iv = 0;
                interpreter::checkIntegerArg(iv, value);
                m_data[packArgs(args)] = iv;
            }
        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual CallableValue* clone() const
            { return new TestCallable(*this); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<test>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        Data_t& m_data;
    };

    struct Environment {
        // Test data
        Data_t data;
        LiteralNode func;

        // Execution
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment(afl::test::Assert a)
            : data(),
              func(std::auto_ptr<afl::data::Value>(new TestCallable(data))),
              log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }

    };
}

/** Test compileValue(). */
AFL_TEST("interpreter.expr.IndirectCallNode:compileValue", a)
{
    // Environment
    Environment env(a);

    // Test object
    interpreter::expr::IndirectCallNode testee(env.func);
    LiteralNode lit1(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    LiteralNode lit2(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(20)));
    testee.addArgument(lit1);
    testee.addArgument(lit2);

    // Compile
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    env.data["10,20"] = 42;
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 42);
}

/** Test compileStore(). */
AFL_TEST("interpreter.expr.IndirectCallNode:compileStore", a)
{
    // Environment
    Environment env(a);

    // Test object
    interpreter::expr::IndirectCallNode testee(env.func);
    LiteralNode lit1(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(42)));
    LiteralNode lit2(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(63)));
    testee.addArgument(lit1);
    testee.addArgument(lit2);

    // Value
    LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(555)));

    // Compile: basically, 'testee := value'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileStore(*bco, interpreter::CompilationContext(env.world), value);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    // - Value must remain on stack
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 555);

    // - Store must have been executed
    a.checkEqual("21. result", env.data["42,63"], 555);
}

/** Test compileCondition(). */
AFL_TEST("interpreter.expr.IndirectCallNode:compileCondition", a)
{
    // Environment
    Environment env(a);

    // Test object
    interpreter::expr::IndirectCallNode testee(env.func);
    LiteralNode lit1(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(8)));
    LiteralNode lit2(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(9)));
    testee.addArgument(lit1);
    testee.addArgument(lit2);

    // Compile: basically, "if (testee, 2, 3)".
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

    // Run with data nonzero
    {
        env.data["8,9"] = 77;
        env.proc.pushFrame(bco, true);
        AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

        const afl::data::Value* pv = env.proc.getResult();
        int32_t iv = 0;
        a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
        a.checkEqual("12. result", iv, 2);
    }

    // Run with data nonzero
    {
        env.data["8,9"] = 0;
        env.proc.pushFrame(bco, true);
        AFL_CHECK_SUCCEEDS(a("21. run"), env.proc.run(0));

        const afl::data::Value* pv = env.proc.getResult();
        int32_t iv = 0;
        a.checkEqual("31. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
        a.checkEqual("32. result", iv, 3);
    }
}

/** Test compileRead(), compileWrite() (read-modify-write operation). */
AFL_TEST("interpreter.expr.IndirectCallNode:read+write", a)
{
    // Environment
    Environment env(a);

    // Test object
    interpreter::expr::IndirectCallNode testee(env.func);
    LiteralNode lit1(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(31)));
    LiteralNode lit2(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(41)));
    testee.addArgument(lit1);
    testee.addArgument(lit2);

    // Compile: read value, increment, store back
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileRead(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maUnary, interpreter::unInc, 0);
    testee.compileWrite(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    env.data["31,41"] = 10;
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    // - Updated value must remain on stack
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 11);

    // - Store must have been executed
    a.checkEqual("21. result", env.data["31,41"], 11);
}
