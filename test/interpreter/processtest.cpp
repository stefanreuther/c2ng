/**
  *  \file test/interpreter/processtest.cpp
  *  \brief Test for interpreter::Process
  */

#include "interpreter/process.hpp"

#include <stdexcept>
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/test/translator.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/context.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/ternaryoperation.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using afl::data::BooleanValue;
using afl::data::FloatValue;
using afl::data::Hash;
using afl::data::IntegerValue;
using afl::data::StringValue;
using interpreter::ArrayData;
using interpreter::ArrayValue;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::HashValue;
using interpreter::Opcode;
using interpreter::Process;
using interpreter::SubroutineValue;
using interpreter::StructureType;
using interpreter::StructureTypeData;
using interpreter::StructureValue;

namespace {
    /* Test implementation of Freezer.
       Because Freezer is only a tag interface, nothing to do here. */
    class NullFreezer : public Process::Freezer { };

    /* Test implementation of Finalizer */
    class CountingFinalizer : public Process::Finalizer {
     public:
        CountingFinalizer(int& callCount)
            : m_callCount(callCount)
            { }
        virtual void finalizeProcess(Process&)
            { ++m_callCount; }
     private:
        int& m_callCount;
    };

    /* Singular object context.
       We don't expect this context to be copied or examined in another way.
       It only provides a single object we give it. */
    class SingularObjectContext : public interpreter::SimpleContext {
     public:
        SingularObjectContext(afl::base::Deletable* pObject)
            : m_pObject(pObject)
            { }
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual bool next()
            { throw std::runtime_error("SingularObjectContext::next unexpected"); }
        virtual Context* clone() const
            { throw std::runtime_error("SingularObjectContext::clone unexpected"); }
        virtual afl::base::Deletable* getObject()
            { return m_pObject; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { throw std::runtime_error("SingularObjectContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { throw std::runtime_error("SingularObjectContext::toString unexpected"); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("SingularObjectContext::store unexpected"); }
     private:
        afl::base::Deletable* m_pObject;
    };

    /* Singular variable context.
       We don't expect this context to be copied or examined in another way.
       It only provides a single variable.
       (Turns out that optionally allowing cloning is helpful.) */
    class SingularVariableContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        SingularVariableContext(afl::test::Assert a, String_t name, String_t& value)
            : m_assert(a), m_name(name), m_value(value), m_clonable(false)
            { }
        void makeClonable()
            { m_clonable = true; }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match(m_name)) {
                    result = 77;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            {
                m_assert.checkEqual("set() index", index, PropertyIndex_t(77));
                m_value = interpreter::toString(value, false);
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                m_assert.checkEqual("get() index", index, PropertyIndex_t(77));
                return interpreter::makeStringValue(m_value);
            }
        virtual bool next()
            { m_assert.fail("SingularVariableContext::next unexpected"); return false; }
        virtual Context* clone() const
            {
                m_assert.check("clone() permitted", m_clonable);
                return new SingularVariableContext(*this);
            }
        virtual afl::base::Deletable* getObject()
            { m_assert.fail("SingularVariableContext::getObject unexpected"); return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { m_assert.fail("SingularVariableContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { m_assert.fail("SingularVariableContext::toString unexpected"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { m_assert.fail("SingularVariableContext::store unexpected"); }
     private:
        afl::test::Assert m_assert;
        String_t m_name;
        String_t& m_value;
        bool m_clonable;
    };

    /* Counting context.
       Exposes a single variable whose value changes with next(). */
    class CountingContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        CountingContext(afl::test::Assert a, String_t name, int32_t value)
            : m_assert(a), m_name(name), m_value(value)
            { }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match(m_name)) {
                    result = 66;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual void set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
            { m_assert.fail("CountingContext::set unexpected"); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                m_assert.checkEqual("get() index", index, PropertyIndex_t(66));
                return interpreter::makeIntegerValue(m_value);
            }
        virtual bool next()
            { ++m_value; return true; }
        virtual Context* clone() const
            { return new CountingContext(*this); }
        virtual afl::base::Deletable* getObject()
            { m_assert.fail("CountingContext::getObject unexpected"); return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { m_assert.fail("CountingContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { m_assert.fail("CountingContext::toString unexpected"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { m_assert.fail("CountingContext::store unexpected"); }
     private:
        afl::test::Assert m_assert;
        String_t m_name;
        int32_t  m_value;
    };

    /* Null object.
       Just a dummy object, we do not look into it. */
    class NullObject : public afl::base::Deletable {
    };

    /* Simple callable */
    class SimpleCallable : public interpreter::CallableValue {
     public:
        SimpleCallable(String_t value, bool isProcedure, int& callCount)
            : m_value(value),
              m_isProcedure(isProcedure),
              m_callCount(callCount)
            { }
        virtual void call(Process& proc, afl::data::Segment& /*args*/, bool want_result)
            {
                ++m_callCount;
                if (want_result) {
                    proc.pushNewValue(interpreter::makeStringValue(m_value));
                }
            }
        virtual bool isProcedureCall() const
            { return m_isProcedure; }
        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual SimpleCallable* clone() const
            { return new SimpleCallable(*this); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<SimpleCallable:" + m_value + ">"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        String_t m_value;
        bool m_isProcedure;
        int& m_callCount;
    };

    /* Simple indexable */
    class SimpleIndexable : public interpreter::IndexableValue {
     public:
        SimpleIndexable(String_t& value, int numArgs)
            : m_value(value),
              m_numArgs(numArgs)
            { }
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                args.checkArgumentCount(m_numArgs);
                return interpreter::makeStringValue(m_value);
            }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            {
                args.checkArgumentCount(m_numArgs);
                m_value = interpreter::toString(value, false);
            }
        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual SimpleIndexable* clone() const
            { return new SimpleIndexable(*this); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<SimpleIndexable>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
     private:
        String_t& m_value;
        int m_numArgs;
    };

    /* Tracing context. Traces the onContextEntered/onContextLeft calls. */
    class TracingContext : public interpreter::Context {
     public:
        TracingContext(String_t& trace, bool reject)
            : m_trace(trace), m_reject(reject)
            { }
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual bool next()
            { return false; }
        virtual interpreter::Context* clone() const
            { return new TracingContext(m_trace, m_reject); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }
        virtual void onContextEntered(interpreter::Process& /*proc*/)
            {
                m_trace += "(enter)";
                if (m_reject) {
                    throw interpreter::Error("fail");
                }
            }
        virtual void onContextLeft()
            { m_trace += "(leave)"; }
        virtual String_t toString(bool /*readable*/) const
            { return "#<trace>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("TracingContext::store unexpected"); }
     private:
        String_t& m_trace;
        bool m_reject;
    };

    /* Common environment for all tests. */
    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        Process proc;

        Environment()
            : log(), tx(), fs(), world(log, tx, fs), proc(world, "test", 99)
            { }
    };

    BCORef_t makeBCO()
    {
        return BytecodeObject::create(false);
    }

    afl::base::Ref<ArrayData> make2DArray()
    {
        afl::base::Ref<ArrayData> ad(*new ArrayData());
        ad->addDimension(1);
        ad->addDimension(2);
        return ad;
    }

    void runBCO(Environment& env, BCORef_t bco)
    {
        env.proc.pushFrame(bco, true);
        env.proc.run();
    }

    void runInstruction(Environment& env, uint8_t major, uint8_t minor, uint16_t arg)
    {
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::Major(major), minor, arg);
        runBCO(env, bco);
    }

    int32_t toInteger(const Environment& env)
    {
        const IntegerValue* iv = dynamic_cast<const IntegerValue*>(env.proc.getResult());
        if (iv == 0) {
            throw interpreter::Error::typeError();
        }
        return iv->getValue();
    }

    double toFloat(const Environment& env)
    {
        const FloatValue* fv = dynamic_cast<const FloatValue*>(env.proc.getResult());
        if (fv == 0) {
            throw interpreter::Error::typeError();
        }
        return fv->getValue();
    }

    bool toBoolean(const Environment& env)
    {
        const BooleanValue* bv = dynamic_cast<const BooleanValue*>(env.proc.getResult());
        if (bv == 0) {
            throw interpreter::Error::typeError();
        }
        return bv->getValue();
    }

    String_t toString(const Environment& env)
    {
        const StringValue* sv = dynamic_cast<const StringValue*>(env.proc.getResult());
        if (sv == 0) {
            throw interpreter::Error::typeError();
        }
        return sv->getValue();
    }

    bool isNull(const Environment& env)
    {
        return env.proc.getResult() == 0;
    }

    bool isError(const Environment& env)
    {
        return !String_t(env.proc.getError().what()).empty();
    }

    BCORef_t makeJdzSample()
    {
        // Make a single function. This implements the translation:
        //    1 -> 0:100
        //    2 -> 0:200
        //    3 -> 0:300
        //    N -> N-3:100
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maJump, Opcode::jDecZero, 3);
        bco->addInstruction(Opcode::maJump, Opcode::jDecZero, 5);
        bco->addInstruction(Opcode::maJump, Opcode::jDecZero, 7);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 100);
        bco->addInstruction(Opcode::maJump, Opcode::jAlways, 8);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 200);
        bco->addInstruction(Opcode::maJump, Opcode::jAlways, 8);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 300);
        return bco;
    }
}


/** Test process properties. */
AFL_TEST("interpreter.Process:basics", a)
{
    Environment env;

    // We're testing process properties, so create our own private process
    Process testee(env.world, "processName", 42);

    // Initial states
    a.checkEqual("01. getName",      testee.getName(), "processName");
    a.checkEqual("02. getProcessId", testee.getProcessId(), 42U);
    a.checkEqual("03. getState",     testee.getState(), testee.Suspended);

    // Initial group Id is unset
    a.checkEqual("11. getProcessGroupId", testee.getProcessGroupId(), 0U);
    testee.setProcessGroupId(23);
    a.checkEqual("12. getProcessGroupId", testee.getProcessGroupId(), 23U);

    // Initial priority is 50
    a.checkEqual("21. getPriority", testee.getPriority(), 50);
    testee.setPriority(12);
    a.checkEqual("22. getPriority", testee.getPriority(), 12);

    // No initial kind
    a.checkEqual("31. getProcessKind", testee.getProcessKind(), testee.pkDefault);
    testee.setProcessKind(testee.pkBaseTask);
    a.checkEqual("32. getProcessKind", testee.getProcessKind(), testee.pkBaseTask);

    // Name
    testee.setName("otherName");
    a.checkEqual("41. getName", testee.getName(), "otherName");

    // State
    testee.setState(Process::Ended);
    a.checkEqual("51. getState", testee.getState(), testee.Ended);

    // Stack
    a.checkEqual("61. getStackSize",  testee.getStackSize(), 0U);
    a.checkEqual("62. getValueStack", testee.getValueStack().size(), 0U);

    // toString
    afl::test::Translator tx("<", ">");
    a.checkEqual("71. toString", interpreter::toString(Process::Runnable, tx), "<Runnable>");
}

/** Test freezing: correct state, collision. */
AFL_TEST("interpreter.Process:freeze", a)
{
    Environment env;

    // We can freeze a fresh process
    NullFreezer fz;
    AFL_CHECK_SUCCEEDS(a("01. freeze"), env.proc.freeze(fz));
    a.checkEqual("02. getState",   env.proc.getState(), Process::Frozen);
    a.checkEqual("03. getFreezer", env.proc.getFreezer(), static_cast<Process::Freezer*>(&fz));

    // We cannot freeze it again, not even re-using the same freezer
    {
        NullFreezer fz2;
        AFL_CHECK_THROWS(a("11. freeze"), env.proc.freeze(fz2), interpreter::Error);
        AFL_CHECK_THROWS(a("12. freeze"), env.proc.freeze(fz), interpreter::Error);
    }

    // Unfreeze
    AFL_CHECK_SUCCEEDS(a("21. unfreeze"), env.proc.unfreeze());
    a.checkEqual("22. getState", env.proc.getState(), Process::Suspended);
    a.checkEqual("23. getFreezer", env.proc.getFreezer(), (Process::Freezer*) 0);

    // Can freeze again
    AFL_CHECK_SUCCEEDS(a("31. freeze"), env.proc.freeze(fz));
    a.checkEqual("32. getState", env.proc.getState(), Process::Frozen);
    a.checkEqual("33. getFreezer", env.proc.getFreezer(), static_cast<Process::Freezer*>(&fz));
}

/** Test freezing: wrong state. */
AFL_TEST("interpreter.Process:freeze:wrong-state", a)
{
    Environment env;

    // Change state
    env.proc.setState(Process::Waiting);

    // Process cannot be frozen in wrong state
    NullFreezer fz;
    AFL_CHECK_THROWS(a("01. freeze"), env.proc.freeze(fz), interpreter::Error);
    a.checkEqual("02. getState", env.proc.getState(), Process::Waiting);
    a.checkEqual("03. getFreezer", env.proc.getFreezer(), (Process::Freezer*) 0);

    // Process cannot be unfrozen in wrong state (but this does not throw)
    AFL_CHECK_SUCCEEDS(a("11. unfreeze"), env.proc.unfreeze());
    a.checkEqual("12. getState", env.proc.getState(), Process::Waiting);
}

/** Test finalize(): finalizer is not called implicitly upon process destruction */
AFL_TEST("interpreter.Process:finalize:not-implicit", a)
{
    int callCount = 0;
    {
        Environment env;
        env.proc.setNewFinalizer(new CountingFinalizer(callCount));
    }
    a.checkEqual("01. callCount", callCount, 0);
}

/** Test finalize(): finalizer is called once no matter how often we explicitly finalize. */
AFL_TEST("interpreter.Process:finalize:normal", a)
{
    int callCount = 0;
    Environment env;
    env.proc.setNewFinalizer(new CountingFinalizer(callCount));
    env.proc.finalize();
    env.proc.finalize();
    a.checkEqual("01. callCount", callCount, 1);
}

/** Test context stack: getInvokingObject(), getCurrentObject(), markContextTOS(). */
AFL_TEST("interpreter.Process:context-stack", a)
{
    Environment env;

    // Initial context stack is empty
    a.check("01. globalContexts", env.world.globalContexts().empty());
    a.check("02. getContexts", env.proc.getContexts().empty());
    a.checkEqual("03. getContextTOS", env.proc.getContextTOS(), 0U);

    // Push some contexts
    NullObject one, two;
    env.proc.pushNewContext(new SingularObjectContext(0));
    env.proc.pushNewContext(new SingularObjectContext(&one));
    env.proc.markContextTOS();
    env.proc.pushNewContext(new SingularObjectContext(&two));
    env.proc.pushNewContext(new SingularObjectContext(0));
    a.checkEqual("11. getContextTOS", env.proc.getContextTOS(), 2U);

    // Check objects
    a.checkEqual("21. getInvokingObject", env.proc.getInvokingObject(), &one);
    a.checkEqual("22. getCurrentObject",  env.proc.getCurrentObject(), &two);

    // Modify TOS
    a.checkEqual("31. setContextTOS",     env.proc.setContextTOS(4), true);
    a.checkEqual("32. getInvokingObject", env.proc.getInvokingObject(), &two);
    a.checkEqual("33. getCurrentObject",  env.proc.getCurrentObject(), &two);

    // Pop context. This must fix up contextTOS.
                   env.proc.popContext();
    a.checkEqual("41. getContextTOS",     env.proc.getContextTOS(), 3U);
    a.checkEqual("42. getInvokingObject", env.proc.getInvokingObject(), &two);
    a.checkEqual("43. getCurrentObject",  env.proc.getCurrentObject(), &two);

    // Out-of-range values refused
    a.checkEqual("51. setContextTOS", env.proc.setContextTOS(9), false);
}

/** Test context stack: pushContextsFrom(). */
AFL_TEST("interpreter.Process:pushContextsFrom", a)
{
    Environment env;

    // Starts with no current object
    a.checkNull("01. getCurrentObject",  env.proc.getCurrentObject());
    a.checkNull("02. getInvokingObject", env.proc.getInvokingObject());

    // Make a context vector
    afl::container::PtrVector<interpreter::Context> vec;
    NullObject one, two;
    vec.pushBackNew(new SingularObjectContext(&one));
    vec.pushBackNew(new SingularObjectContext(&two));
    env.proc.pushContextsFrom(vec);

    // Verify
    a.checkEqual("11. getCurrentObject",  env.proc.getCurrentObject(), &two);
    a.checkNull ("12. getInvokingObject", env.proc.getInvokingObject());
}

/** Test variable access: setVariable(), getVariable(). */
AFL_TEST("interpreter.Process:setVariable", a)
{
    Environment env;

    // Make two variable contexts; we'll be modifying the inner one
    String_t inner = "i", outer = "o";
    env.proc.pushNewContext(new SingularVariableContext(a("outer"), "VALUE", outer));
    env.proc.pushNewContext(new SingularVariableContext(a("inner"), "VALUE", inner));

    // Check value
    std::auto_ptr<afl::data::Value> p(env.proc.getVariable("VALUE"));
    a.checkEqual("01. value", interpreter::toString(p.get(), false), "i");

    // Set value
    afl::data::StringValue sv("nv");
    a.checkEqual("11. set", env.proc.setVariable("VALUE", &sv), true);
    a.checkEqual("12. inner", inner, "nv");
    a.checkEqual("13. outer", outer, "o");

    // Accessing unknown values is harmless
    a.checkEqual("21. set", env.proc.setVariable("OTHER", &sv), false);
    p = env.proc.getVariable("OTHER");
    a.checkNull("22. get", p.get());
}

/** Test execution: invalid opcode. */
AFL_TEST("interpreter.Process:run:invalid", a)
{
    struct TestCase {
        uint8_t major;
        uint8_t minor;
        uint16_t arg;
        const char* label;
    };
    static const TestCase CASES[] = {
        { Opcode::maPush,     200,              0, "invalid push" },
        { Opcode::maBinary,   200,              0, "invalid binary" },
        { Opcode::maUnary,    200,              0, "invalid unary" },
        { Opcode::maTernary,  200,              0, "invalid ternary" },
        { Opcode::maJump,     127,              1, "invalid jump" },
        // Opcode::maIndirect has no reachable invalid encodings
        { Opcode::maStack,    200,              0, "invalid stack" },
        { Opcode::maStore,    200,              0, "invalid store" },
        { Opcode::maStore,    Opcode::sLiteral, 0, "invalid store(2)" },
        { Opcode::maPop,      200,              0, "invalid pop" },
        { Opcode::maPop,      Opcode::sLiteral, 0, "invalid pop(2)" },
        { Opcode::maMemref,   200,              0, "invalid memref" },
        { Opcode::maDim,      200,              0, "invalid dim" },
        { Opcode::maDim,      Opcode::sLiteral, 0, "invalid dim(2)" },
        { Opcode::maSpecial,  200,              0, "invalid special" },
        { 200,                0,                0, "invalid major" },

        // Fused opcodes are refused if code too short even if opcode itself is valid
        { Opcode::maFusedUnary,       Opcode::sLiteral,         0, "short fused unary" },
        { Opcode::maFusedBinary,      Opcode::sLiteral,         0, "short fused binary" },
        { Opcode::maFusedComparison,  interpreter::biCompareEQ, 0, "short fused comparison" },
        { Opcode::maFusedComparison2, Opcode::sLiteral,         0, "short fused comparison(2)" },
        { Opcode::maInplaceUnary,     Opcode::sLocal,           0, "short inplace unary" },
    };

    // Invalid push
    for (size_t i = 0; i < sizeof(CASES)/sizeof(CASES[0]); ++i) {
        Environment env;
        for (int j = 0; j < 20; ++j) {
            // Make sure we don't detect lack of stack before invalid opcode
            env.proc.pushNewValue(0);
        }
        runInstruction(env, CASES[i].major, CASES[i].minor, CASES[i].arg);
        a(CASES[i].label).checkEqual("getState", env.proc.getState(), Process::Failed);
        a(CASES[i].label).check("isError", isError(env));
    }
}

/** Test instruction: pushvar. */
AFL_TEST("interpreter.Process:run:pushvar", a)
{
    // Execute a single standalone 'pushvar' instruction
    Environment env;
    String_t value("theValue");
    env.proc.pushNewContext(new SingularVariableContext(a("value"), "VALUE", value));
    env.proc.pushNewContext(new SingularObjectContext(0));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VALUE"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "theValue");
}

/** Test instruction: pushloc. */
AFL_TEST("interpreter.Process:run:pushloc", a)
{
    // Execute a single 'pushloc' instruction on a frame containing a local value.
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sLocal, 3);

    Process::Frame& f = env.proc.pushFrame(bco, true);
    f.localValues.setNew(3, interpreter::makeStringValue("local"));

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "local");
}

/** Test instruction: pushtop. */
AFL_TEST("interpreter.Process:run:pushtop", a)
{
    // Make an outer BCO containing the static variable.
    // Execute a single 'pushtop' instruction in an inner BCO.
    Environment env;

    Process::Frame& outerFrame = env.proc.pushFrame(makeBCO(), true);
    outerFrame.localValues.setNew(7, interpreter::makeStringValue("outer"));

    BCORef_t innerBCO = makeBCO();
    innerBCO->addInstruction(Opcode::maPush, Opcode::sStatic, 7);
    Process::Frame& innerFrame = env.proc.pushFrame(innerBCO, true);
    innerFrame.localValues.setNew(7, interpreter::makeStringValue("inner"));

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "outer");
}

/** Test instruction: pushglob. */
AFL_TEST("interpreter.Process:run:pushglob", a)
{
    // Set a global value. Execute single 'pushglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    runInstruction(env, Opcode::maPush, Opcode::sShared, 99);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "v");
}

/** Test instruction: pushgvar.
    Set a global value by name. Execute single 'pushgvar' instruction. */
AFL_TEST("interpreter.Process:run:pushgvar", a)
{
    Environment env;
    env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("GV"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "q");
}

/** Test instruction: pushgvar.
    Error case. */
AFL_TEST("interpreter.Process:run:pushgvar:error", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("XXXXX"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: pushlit. */
AFL_TEST("interpreter.Process:run:pushlit", a)
{
    // Execute single standalone 'pushlit' instruction.
    Environment env;

    FloatValue fv(2.5);
    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&fv);
    a.checkEqual("01. major", (*bco)(0).major, Opcode::maPush);
    a.checkEqual("02, minor", (*bco)(0).minor, Opcode::sLiteral);
    runBCO(env, bco);

    a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("12. result", toFloat(env), 2.5);
}

/** Test instruction: pushint.
    Execute single standalone 'pushint' instruction. */
AFL_TEST("interpreter.Process:run:pushint", a)
{
    Environment env;
    runInstruction(env, Opcode::maPush, Opcode::sInteger, 45);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 45);
}

/** Test instruction: pushint, negative value */
AFL_TEST("interpreter.Process:run:pushint:neg", a)
{
    Environment env;
    runInstruction(env, Opcode::maPush, Opcode::sInteger, 0xFFFE);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), -2);
}

/** Test instruction: pushbool. */
AFL_TEST("interpreter.Process:run:pushbool:true", a)
{
    Environment env;
    runInstruction(env, Opcode::maPush, Opcode::sBoolean, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toBoolean(env), true);
}

/** Test instruction: pushbool, null case */
AFL_TEST("interpreter.Process:run:pushbool:null", a)
{
    Environment env;
    runInstruction(env, Opcode::maPush, Opcode::sBoolean, -1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: uinc (as specimen for unary).
    Good case: execute single uinc instruction on stack with one element. */
AFL_TEST("interpreter.Process:run:unary", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(32));
    runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 33);
}

/** Test instruction: uinc (as specimen for unary).
    Bad case: execute single uinc instruction on empty stack. */
AFL_TEST("interpreter.Process:run:unary:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
    a.checkEqual("11. getState", env.proc.getState(), Process::Failed);
    a.check("12. isError", isError(env));
}

/** Test instruction: uinc (as specimen for unary).
    Extra bad case: type error needs to be reflected into process state */
AFL_TEST("interpreter.Process:run:unary:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("Q"));
    runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
    a.checkEqual("21. getState", env.proc.getState(), Process::Failed);
    a.check("22. isError", isError(env));
}

/** Test instruction: badd (as specimen for binary).
    Good case: execute single badd instruction on stack with one element. */
AFL_TEST("interpreter.Process:run:binary", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("aa"));
    env.proc.pushNewValue(interpreter::makeStringValue("bbb"));
    runInstruction(env, Opcode::maBinary, interpreter::biAdd, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "aabbb");
}

/** Test instruction: badd (as specimen for binary).
    Bad case: execute single badd instruction on stack with too few elements. */
AFL_TEST("interpreter.Process:run:binary:error:stack", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("aa"));
    runInstruction(env, Opcode::maBinary, interpreter::biAdd, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: tkeyadd (as specimen for ternary). */
    // Good case: set up a keymap and add a key (this is our only ternary op so far)
AFL_TEST("interpreter.Process:run:ternary", a)
{
    Environment env;
    util::KeymapRef_t k = env.world.keymaps().createKeymap("K");
    env.proc.pushNewValue(new interpreter::KeymapValue(k));
    env.proc.pushNewValue(interpreter::makeStringValue("q"));
    env.proc.pushNewValue(interpreter::makeStringValue("cmd"));
    runInstruction(env, Opcode::maTernary, interpreter::teKeyAdd, 0);

    a.check("01. result", !isNull(env));

    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(env.proc.getResult());
    a.check("11. KeymapValue", kv);
    a.check("12. getKeymap", kv->getKeymap() == k);
    a.check("13. lookupCommand", k->lookupCommand('q') != 0);
}

/** Test instruction: tkeyadd (as specimen for ternary).
    Bad case: execute instruction on stack with too few elements */
AFL_TEST("interpreter.Process:run:ternary:error:stack", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maTernary, interpreter::teKeyAdd, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: conditional jump, taken.
    pushint 42 / pushint 1 / jtp end / pushint 43: result must be 42 */
AFL_TEST("interpreter.Process:run:jccp:taken", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    bco->addInstruction(Opcode::maJump, Opcode::jIfTrue | Opcode::jPopAlways, 4);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 42);
}

/** Test instruction: conditional jump, taken.
    pushint 42 / jt end / pushint 43: result must be 42 (same thing without implicit pop) */
AFL_TEST("interpreter.Process:run:jcc:taken", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
    bco->addInstruction(Opcode::maJump, Opcode::jIfTrue, 4);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 42);
}

/** Test instruction: conditional jump.
    jt end: fails, no value to test on stack */
AFL_TEST("interpreter.Process:run:jcc:error:stack", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maJump, Opcode::jIfTrue, 1);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: conditional jump, not taken.
    pushint 42 / pushint 1 / jfp end / pushint 43: result must be 43 */
AFL_TEST("interpreter.Process:run:jccp:not-taken", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    bco->addInstruction(Opcode::maJump, Opcode::jIfFalse | Opcode::jPopAlways, 4);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 43);
}

/** Test instruction: conditional jump, not taken.
    pushint 42 / jf end / pushint 43: result must be 43 (same thing without implicit pop) */
AFL_TEST("interpreter.Process:run:jcc:not-taken", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
    bco->addInstruction(Opcode::maJump, Opcode::jIfFalse, 4);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 43);
}

/** Test instruction: unconditional jump.
    Unconditional jump can be executed without stuff on stack.
    j 2 / <invalid> / pushint 89: result must be 89. */
AFL_TEST("interpreter.Process:run:j", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maJump, Opcode::jAlways, 2);
    bco->addInstruction(Opcode::maDim, 200, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 89);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 89);
}

/** Test instruction: unconditional with pop
    pushint 17 / pushint 18 / jp end: result must be 17 */
AFL_TEST("interpreter.Process:run:jp", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 17);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 18);
    bco->addInstruction(Opcode::maJump, Opcode::jAlways | Opcode::jPopAlways, 3);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 17);
}

/** Test instruction: unconditional with pop fails if stack empty. */
AFL_TEST("interpreter.Process:run:jp:error:stack", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maJump, Opcode::jAlways | Opcode::jPopAlways, 1);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: catch.
    A command sequence where the exception is caught */
AFL_TEST("interpreter.Process:run:catch", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 10);               // 0 - 10
    bco->addInstruction(Opcode::maJump, Opcode::jCatch, 6);                  // 1
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 90);               // 2 - 10:90
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 91);               // 3 - 10:90:91
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);       // 4 - 10:"91"
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 92);               // 5 (not reached)
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 93);               // 6 - 10:"91":93

    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 3U);
    a.checkEqual("12. result", toInteger(env), 93);
    env.proc.dropValue();
    a.checkEqual("13. result", toString(env), "91");
    env.proc.dropValue();
    a.checkEqual("14. result", toInteger(env), 10);

    a.checkEqual("21. getExceptionHandlers", env.proc.getExceptionHandlers().size(), 0U);
}

/** Test instruction: catch.
    A command sequence where no exception happens */
AFL_TEST("interpreter.Process:run:catch:no-exception", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 10);               // 0 - 10
    bco->addInstruction(Opcode::maJump, Opcode::jCatch, 3);                  // 1
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 90);               // 2 - 10:90

    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("12. result", toInteger(env), 90);
    env.proc.dropValue();
    a.checkEqual("13. result", toInteger(env), 10);

    a.checkEqual("21. getExceptionHandlers", env.proc.getExceptionHandlers().size(), 0U);
}

/** Test instruction: jdz: Integer 0 */
AFL_TEST("interpreter.Process:run:jdz:int0", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(0));
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("12. result", toInteger(env), 100);
    env.proc.dropValue();
    a.checkEqual("13. result", toInteger(env), -3);
}

/** Test instruction: jdz: Integer 2. */
AFL_TEST("interpreter.Process:run:jdz:int2", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("12. result", toInteger(env), 200);
    env.proc.dropValue();
    a.checkEqual("13. result", toInteger(env), 0);
}

/** Test instruction: jdz: Float 3 */
AFL_TEST("interpreter.Process:run:jdz:float3", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeFloatValue(3));
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("12. result", toInteger(env), 300);
    env.proc.dropValue();
    a.checkEqual("13. result", toFloat(env), 0.0);
}

/** Test instruction: jdz: Float 2.5 never hits */
AFL_TEST("interpreter.Process:run:jdz:float2.5", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeFloatValue(2.5));
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    a.checkEqual("11. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("12. result", toInteger(env), 100);
    env.proc.dropValue();
    a.checkEqual("13. result", toFloat(env), -0.5);
}

/** Test instruction: jdz: Null fails */
AFL_TEST("interpreter.Process:run:jdz:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: jdz: String fails */
AFL_TEST("interpreter.Process:run:jdz:str", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("x"));
    runBCO(env, makeJdzSample());
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callind/procind.
    callind 1 => 1:null -> empty */
AFL_TEST("interpreter.Process:run:callind:null", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.check("03. result", isNull(env));
}

/** Test instruction: callind/procind.
    procind 1 => 1:null -> empty (null can be called as function, not as procedure) */
AFL_TEST("interpreter.Process:run:procind:null", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callind/procind.
    procind 1 => 1:"foo" -> error */
AFL_TEST("interpreter.Process:run:procind:str", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeStringValue("foo"));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callind/procind.
    callind 1 => 1:Callable -> empty */
AFL_TEST("interpreter.Process:run:callind", a)
{
    int callCount = 0;
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.check("03. result", isNull(env));
    a.checkEqual("04. callCount", callCount, 1);
}

/** Test instruction: callind/procind.
    check refuse procedures branch */
AFL_TEST("interpreter.Process:run:callind:refuse-proc", a)
{
    int callCount = 0;
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseProcedures, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callind/procind.
    check refuse functions branch */
AFL_TEST("interpreter.Process:run:callind:refuse-func", a)
{
    int callCount = 0;
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(new SimpleCallable("v", false, callCount));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: loadind.
    loadind 1 => 1:null -> null */
AFL_TEST("interpreter.Process:run:loadind:null", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 1U);
    a.check("03. result", isNull(env));
}

/** Test instruction: loadind.
    loadind 1 => 1:"foo" -> error */
AFL_TEST("interpreter.Process:run:loadind:str", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeStringValue("foo"));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: loadind.
    loadind 1 => 1:Callable -> empty */
AFL_TEST("interpreter.Process:run:loadind", a)
{
    int callCount = 0;
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "v");
    a.checkEqual("03. callCount", callCount, 1);
}

/** Test instruction: storeind.
    storeind 2 => 1:2:"new":Callable -> "new" */
AFL_TEST("interpreter.Process:run:storeind", a)
{
    String_t value("old");
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeStringValue("new"));
    env.proc.pushNewValue(new SimpleIndexable(value, 2));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMStore, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 1U);
    a.checkEqual("03. result", toString(env), "new");
    a.checkEqual("04. value", value, "new");
}

/** Test instruction: storeind.
    storeind 1 => 1:2:3 -> error */
AFL_TEST("interpreter.Process:run:storeind:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMStore, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: popind.
    popind 2 => 1:2:"new":Callable -> empty */
AFL_TEST("interpreter.Process:run:popind", a)
{
    String_t value("old");
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeStringValue("new"));
    env.proc.pushNewValue(new SimpleIndexable(value, 2));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMPop, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. value", value, "new");
}

/** Test instruction: popind.
    popind 1 => 1:2:3 -> error */
AFL_TEST("interpreter.Process:run:popind:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maIndirect, Opcode::miIMPop, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: dup.
    Good case: dup 1 => 1:2:3 -> 1:2:3:1 */
AFL_TEST("interpreter.Process:run:dup", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maStack, Opcode::miStackDup, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 4U);
    a.checkEqual("03. result", toInteger(env), 2);   // the new value
    env.proc.dropValue();
    a.checkEqual("04. result", toInteger(env), 3);   // previous value
}

/** Test instruction: dup. Bad case */
AFL_TEST("interpreter.Process:run:dup:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maStack, Opcode::miStackDup, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: drop.
    Good case: drop 2 => 1:2:3 -> 1 */
AFL_TEST("interpreter.Process:run:drop", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maStack, Opcode::miStackDrop, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 1U);
    a.checkEqual("03. result", toInteger(env), 1);
}

/** Test instruction: drop. Bad case */
AFL_TEST("interpreter.Process:run:drop:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maStack, Opcode::miStackDrop, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: swap.
    Good case: swap 1 => 1:2:3 -> 1:3:2 */
AFL_TEST("interpreter.Process:run:swap", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maStack, Opcode::miStackSwap, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 3U);
    a.checkEqual("03. result 1", toInteger(env), 2);
    env.proc.dropValue();
    a.checkEqual("04. result 2", toInteger(env), 3);
    env.proc.dropValue();
    a.checkEqual("05. result 3", toInteger(env), 1);
}

/** Test instruction: swap.
    Bad case */
AFL_TEST("interpreter.Process:run:swap:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maStack, Opcode::miStackSwap, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: storevar.
    Execute a single standalone 'storevar' instruction, good case */
AFL_TEST("interpreter.Process:run:storevar", a)
{
    Environment env;
    String_t value("theValue");
    env.proc.pushNewContext(new SingularVariableContext(a, "VALUE", value));
    env.proc.pushNewContext(new SingularObjectContext(0));
    env.proc.pushNewValue(interpreter::makeIntegerValue(17));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("VALUE"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 17);    // original value
    a.checkEqual("03. value", value, "17");           // stringified by SingularVariableContext
}

/** Test instruction: storevar. Bad case */
AFL_TEST("interpreter.Process:run:storevar:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(17));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("UNKNOWN_VALUE"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: storeloc. */
AFL_TEST("interpreter.Process:run:storeloc", a)
{
    // We cannot test 'storeloc' directly because the frame is discarded before we can see it.
    // Therefore, make a sequence involving 'storeloc'.
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 10);      // stack=10     local="local"
    bco->addInstruction(Opcode::maStore, Opcode::sLocal, 3);        // stack=10     local=10
    bco->addInstruction(Opcode::maUnary, interpreter::unInc, 0);    // stack=11     local=10
    bco->addInstruction(Opcode::maPush, Opcode::sLocal, 3);         // stack=11:10
    bco->addInstruction(Opcode::maBinary, interpreter::biAdd, 0);   // stack=21

    Process::Frame& f = env.proc.pushFrame(bco, true);
    f.localValues.setNew(3, interpreter::makeStringValue("local"));  // will immediately be overwritten

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 21);
}

/** Test instruction: storetop. */
AFL_TEST("interpreter.Process:run:storetop", a)
{
    // We cannot test 'storetop' directly because the frame is discarded before we can see it.
    // Make an outer BCO referencing the static variable, and an inner BCO setting it.
    Environment env;
    const uint16_t ADDR = 17;

    BCORef_t outerBCO = makeBCO();
    Process::Frame& outerFrame = env.proc.pushFrame(outerBCO, true);
    outerFrame.localValues.setNew(7, interpreter::makeStringValue("outer"));
    outerBCO->addInstruction(Opcode::maPush, Opcode::sLocal, ADDR);

    BCORef_t innerBCO = makeBCO();
    innerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 12);
    innerBCO->addInstruction(Opcode::maStore, Opcode::sStatic, ADDR);
    innerBCO->addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    Process::Frame& innerFrame = env.proc.pushFrame(innerBCO, true);
    innerFrame.localValues.setNew(7, interpreter::makeStringValue("inner"));

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 12);
}

/** Test instruction: storeglob. */
AFL_TEST("interpreter.Process:run:storeglob", a)
{
    // Set a global value. Execute single 'storeglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    runInstruction(env, Opcode::maStore, Opcode::sShared, 99);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "nv");
    a.checkEqual("03", interpreter::toString(env.world.globalValues().get(99), false), "nv");
}

/** Test instruction: storegvar.
    Set a global value by name. Execute single 'storegvar' instruction. */
AFL_TEST("interpreter.Process:run:storegvar", a)
{
    Environment env;
    env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maStore, Opcode::sNamedShared, bco->addName("GV"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "nv");
    a.checkEqual("03. variable", interpreter::toString(env.world.getGlobalValue("GV"), false), "nv");
}

/** Test instruction: storegvar, Error case */
AFL_TEST("interpreter.Process:run:storegvar:error", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maStore, Opcode::sNamedShared, bco->addName("XXXXX"));
    env.proc.pushNewValue(0);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: popvar.
    Execute a single standalone 'popvar' instruction, good case */
AFL_TEST("interpreter.Process:run:popvar", a)
{
    Environment env;
    String_t value("theValue");
    env.proc.pushNewContext(new SingularVariableContext(a, "VALUE", value));
    env.proc.pushNewContext(new SingularObjectContext(0));
    env.proc.pushNewValue(interpreter::makeIntegerValue(17));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPop, Opcode::sNamedVariable, bco->addName("VALUE"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. value", value, "17");           // stringified by SingularVariableContext
}

/** Test instruction: popvar, Bad case */
AFL_TEST("interpreter.Process:run:popvar:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(17));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPop, Opcode::sNamedVariable, bco->addName("UNKNOWN_VALUE"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: poploc. */
AFL_TEST("interpreter.Process:run:poploc", a)
{
    // We cannot test 'poploc' directly because the frame is discarded before we can see it.
    // Therefore, make a sequence involving 'poploc'.
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 5);       // stack=5      local="local"
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 10);      // stack=5:10   local="local"
    bco->addInstruction(Opcode::maPop, Opcode::sLocal, 3);          // stack=5      local=10
    bco->addInstruction(Opcode::maUnary, interpreter::unInc, 0);    // stack=6      local=10
    bco->addInstruction(Opcode::maPush, Opcode::sLocal, 3);         // stack=6:10
    bco->addInstruction(Opcode::maBinary, interpreter::biAdd, 0);   // stack=16

    Process::Frame& f = env.proc.pushFrame(bco, true);
    f.localValues.setNew(3, interpreter::makeStringValue("local"));  // will immediately be overwritten

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 16);
}

/** Test instruction: poptop. */
AFL_TEST("interpreter.Process:run:poptop", a)
{
    // We cannot test 'poptop' directly because the frame is discarded before we can see it.
    // Make an outer BCO referencing the static variable, and an inner BCO setting it.
    Environment env;
    const uint16_t ADDR = 17;

    BCORef_t outerBCO = makeBCO();
    Process::Frame& outerFrame = env.proc.pushFrame(outerBCO, true);
    outerFrame.localValues.setNew(7, interpreter::makeStringValue("outer"));
    outerBCO->addInstruction(Opcode::maPush, Opcode::sLocal, ADDR);

    BCORef_t innerBCO = makeBCO();
    innerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 12);
    innerBCO->addInstruction(Opcode::maPop, Opcode::sStatic, ADDR);
    Process::Frame& innerFrame = env.proc.pushFrame(innerBCO, true);
    innerFrame.localValues.setNew(7, interpreter::makeStringValue("inner"));

    env.proc.run();

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 12);
}

/** Test instruction: popglob. */
AFL_TEST("interpreter.Process:run:popglob", a)
{
    // Set a global value. Execute single 'popglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    runInstruction(env, Opcode::maPop, Opcode::sShared, 99);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. value", interpreter::toString(env.world.globalValues().get(99), false), "nv");
}

/** Test instruction: popgvar.
    Set a global value by name. Execute single 'popgvar' instruction. */
AFL_TEST("interpreter.Process:run:popgvar", a)
{
    Environment env;
    env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPop, Opcode::sNamedShared, bco->addName("GV"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03", interpreter::toString(env.world.getGlobalValue("GV"), false), "nv");
}

/** Test instruction: popgvar, error case */
AFL_TEST("interpreter.Process:run:popgvar:error", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPop, Opcode::sNamedShared, bco->addName("XXXXX"));
    env.proc.pushNewValue(0);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: loadmem. Good case */
AFL_TEST("interpreter.Process:run:loadmem", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(new SingularVariableContext(a, "V", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "v");
}

/** Test instruction: loadmem. Null case. */
AFL_TEST("interpreter.Process:run:loadmem:null", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(0);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: loadmem. Error case: unknown name */
AFL_TEST("interpreter.Process:run:loadmem:error:name", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(new SingularVariableContext(a, "OTHER", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: loadmem. Error case: type error */
AFL_TEST("interpreter.Process:run:loadmem:error:type", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(77));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callmem.
    Note that this instruction is pretty useless;
    It effectively only probes accessability of a variable but does not produce a stack result.
    It only exists for symmetry with (maIndirect,miIMCall).
    Good case. */
AFL_TEST("interpreter.Process:run:callmem", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMCall, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(new SingularVariableContext(a, "V", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
}

/** Test instruction: callmem. Null case. */
AFL_TEST("interpreter.Process:run:callmem:null", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMCall, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(0);
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
}

/** Test instruction: callmem. Error case: unknown name. */
AFL_TEST("interpreter.Process:run:callmem:error:name", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMCall, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(new SingularVariableContext(a, "OTHER", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: callmem. Error case: type error. */
AFL_TEST("interpreter.Process:run:callmem:error:type", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMCall, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(77));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: popmem. Good case. */
AFL_TEST("interpreter.Process:run:popmem", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMPop, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(new SingularVariableContext(a, "V", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. value", value, "nv");
}

/** Test instruction: popmem. Bad case: unknown name */
AFL_TEST("interpreter.Process:run:popmem:error:name", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMPop, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(new SingularVariableContext(a, "OTHER", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
    a.checkEqual("03. value", value, "v");
}

/** Test instruction: popmem. Bad case: type error */
AFL_TEST("interpreter.Process:run:popmem:error:type", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMPop, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: storemem. Good case */
AFL_TEST("interpreter.Process:run:storemem", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMStore, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(new SingularVariableContext(a, "V", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "nv");
    a.checkEqual("03. value", value, "nv");
}

/** Test instruction: storemem. Bad case: unknown name */
AFL_TEST("interpreter.Process:run:storemem:error:name", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMStore, bco->addName("V"));
    Environment env;
    String_t value("v");
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(new SingularVariableContext(a, "OTHER", value));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
    a.checkEqual("03. value", value, "v");
}

/** Test instruction: storemem. Bad case: type error */
AFL_TEST("interpreter.Process:run:storemem:error:type", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMStore, bco->addName("V"));
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: dimloc.
    New variable being created.
    We cannot directly observe the local variable frame, so create the variable and read it back. */
AFL_TEST("interpreter.Process:run:dimloc", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    bco->addInstruction(Opcode::maDim, Opcode::sLocal, bco->addName("LV"));
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("LV"));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 7);
}

/** Test instruction: dimloc.
    Variable already exists. */
AFL_TEST("interpreter.Process:run:dimloc:exists", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addLocalVariable("LV");
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    bco->addInstruction(Opcode::maDim, Opcode::sLocal, bco->addName("LV"));
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("LV"));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: dimloc.
    Error: name is empty. */
AFL_TEST("interpreter.Process:run:dimloc:error:empty", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    bco->addInstruction(Opcode::maDim, Opcode::sLocal, bco->addName(""));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
}

/** Test instruction: dimtop. */
AFL_TEST("interpreter.Process:run:dimtop", a)
{
    // Create variable from inner BCO, read it from outer.
    // Only test the "new variable" case; the "already exists" case is already covered by the "dimloc" test case.
    Environment env;
    BCORef_t outerBCO = makeBCO();
    outerBCO->addInstruction(Opcode::maPush, Opcode::sNamedVariable, outerBCO->addName("TV"));
    env.proc.pushFrame(outerBCO, true);

    BCORef_t innerBCO = makeBCO();
    innerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    innerBCO->addInstruction(Opcode::maDim, Opcode::sStatic, innerBCO->addName("TV"));
    env.proc.pushFrame(innerBCO, true);

    env.proc.run();
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 7);
}

/** Test instruction: dimglob. */
AFL_TEST("interpreter.Process:run:dimglob", a)
{
    // We can directly test the effect of "dimglob".
    // In fact, the indirect test (create in inner, read in outer using pushvar) would require a GlobalContext we don't have here.
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    bco->addInstruction(Opcode::maDim, Opcode::sShared, bco->addName("GV"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);

    afl::data::NameMap::Index_t index = env.world.globalPropertyNames().getIndexByName("GV");
    a.check("11. index", index != afl::data::NameMap::nil);

    const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(env.world.globalValues().get(index));
    a.check("21. IntegerValue", iv);
    a.checkEqual("22. getValue", iv->getValue(), 7);
}

/** Test instruction: suncatch.
    Execute a sequence consisting of catch and uncatch. */
AFL_TEST("interpreter.Process:run:suncatch", a)
{
    Environment env;
    String_t value;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maJump, Opcode::jCatch, 5);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, -1);
    bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("VAR"));
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    // this instruction throws/terminates execution:
    bco->addInstruction(Opcode::maUnary, interpreter::unSqrt, 0);
    // catch would jump here:
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, -2);
    bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("VAR"));
    env.proc.pushNewContext(new SingularVariableContext(a, "VAR", value));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
    a.checkEqual("03. value", value, "-1");
}

/** Test instruction: suncatch.
    Error case: uncatch without previous catch */
AFL_TEST("interpreter.Process:run:suncatch:error", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sreturn.
    This instruction is essentially equivalent to a jump-to-end.
    The parameter is pretty pointless; result transfer is handled by
    matching the frame's wantResult and the BCO's isProcedure.
    Test it just for completeness.
    Good case. */
AFL_TEST("interpreter.Process:run:sreturn:1", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 2);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 1);
}

/** Test instruction: sreturn 0. Good case 2 */
AFL_TEST("interpreter.Process:run:sreturn:0", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialReturn, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
}

/** Test instruction: sreturn. Bad case: stack violation */
AFL_TEST("interpreter.Process:run:sreturn:error", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}


/** Test instruction: swith. Good case */
AFL_TEST("interpreter.Process:run:swith", a)
{
    String_t value("v");
    SingularVariableContext ctx(a, "VAR", value);
    ctx.makeClonable();

    Environment env;
    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&ctx);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VAR"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "v");
}

/** Test instruction: swith. Bad case: no stack */
AFL_TEST("interpreter.Process:run:swith:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialWith, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: swith. Bad case: wrong type */
AFL_TEST("interpreter.Process:run:swith:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialWith, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sendwith. Good case. */
AFL_TEST("interpreter.Process:run:sendwith", a)
{
    Environment env;
    String_t outerValue("ov");
    env.proc.pushNewContext(new SingularVariableContext(a("outer"), "VAR", outerValue));

    String_t innerValue("iv");
    SingularVariableContext innerContext(a("inner"), "VAR", innerValue);
    innerContext.makeClonable();

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&innerContext);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VAR"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "ov");
}

/** Test instruction: sendwith. Bad case: no context */
AFL_TEST("interpreter.Process:run:sendwith:error", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    for (int i = 0; i < 10; ++i) {
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
    }
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sfirstindex.
    Good case: non-empty iterable. Pushes true and activates context; proven with pushvar */
AFL_TEST("interpreter.Process:run:sfirstindex", a)
{
    Hash::Ref_t hash = Hash::create();
    hash->setNew("kk", interpreter::makeIntegerValue(1));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("KEY"));

    Environment env;
    env.proc.pushNewValue(new HashValue(hash));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "kk");      // result of the pushvar
    env.proc.dropValue();
    a.checkEqual("03. result", toBoolean(env), true);     // result of the sfirstindex
}

/** Test instruction: sfirstindex.
    Good case: empty iterable. Pushes null and does not modify context stack. */
AFL_TEST("interpreter.Process:run:sfirstindex:null", a)
{
    Environment env;
    size_t n = env.proc.getContexts().size();
    env.proc.pushNewValue(new HashValue(Hash::create()));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
    a.checkEqual("03", env.proc.getContexts().size(), n);
}

/** Test instruction: sfirstindex. Bad case: not iterable */
AFL_TEST("interpreter.Process:run:sfirstindex:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snextindex.
    Good case: unit iterable. */
AFL_TEST("interpreter.Process:run:snextindex:unit", a)
{
    Hash::Ref_t hash = Hash::create();
    hash->setNew("kk", interpreter::makeIntegerValue(1));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);            // pushes true
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);             // pushes null, end of iteration, drops the context

    Environment env;
    size_t n = env.proc.getContexts().size();
    env.proc.pushNewValue(new HashValue(hash));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02", env.proc.getContexts().size(), n);
    a.check("03. result", isNull(env));                     // result of the snextindex
    env.proc.dropValue();
    a.checkEqual("04. result", toBoolean(env), true);     // result of the sfirstindex
}

/** Test instruction: snextindex.
    Good case: multiple entry iterable. */
AFL_TEST("interpreter.Process:run:snextindex:multi", a)
{
    Hash::Ref_t hash = Hash::create();
    hash->setNew("a1", interpreter::makeIntegerValue(1));
    hash->setNew("b2", interpreter::makeIntegerValue(2));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);            // pushes true
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);             // pushes true
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("KEY"));  // pushes "b2"

    Environment env;
    env.proc.pushNewValue(new HashValue(hash));
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "b2");      // result of the pushvar
    env.proc.dropValue();
    a.checkEqual("03. result", toBoolean(env), true);     // result of the snextindex
    env.proc.dropValue();
    a.checkEqual("04. result", toBoolean(env), true);     // result of the sfirstindex
}

/** Test instruction: snextindex.
    Bad case: no context */
AFL_TEST("interpreter.Process:run:snextindex:error", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    for (int i = 0; i < 10; ++i) {
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
    }
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sendindex.
    Good case */
AFL_TEST("interpreter.Process:run:sendindex", a)
{
    Hash::Ref_t hash = Hash::create();
    hash->setNew("kk", interpreter::makeIntegerValue(1));

    Environment env;
    String_t outerValue("ov");
    env.proc.pushNewContext(new SingularVariableContext(a, "KEY", outerValue));
    env.proc.pushNewValue(new HashValue(hash));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("KEY"));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "ov");
}

/** Test instruction: sendindex.
    Bad case: no context */
AFL_TEST("interpreter.Process:run:sendindex:error", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    for (int i = 0; i < 10; ++i) {
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
    }
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sevals. Good case - single line */
AFL_TEST("interpreter.Process:run:sevals:single", a)
{
    String_t value("a");
    Environment env;
    env.proc.pushNewContext(new SingularVariableContext(a, "VAR", value));
    env.proc.pushNewValue(interpreter::makeStringValue("var := 'b'"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getValueStack", env.proc.getValueStack().size(), 0U);
    a.checkEqual("03. value", value, "b");
}

/** Test instruction: sevals. Good case - multiple lines */
AFL_TEST("interpreter.Process:run:sevals:multi", a)
{
    String_t value("a");
    Environment env;
    env.proc.pushNewContext(new SingularVariableContext(a, "VAR", value));
    env.proc.pushNewValue(interpreter::makeStringValue("if var='a'"));
    env.proc.pushNewValue(interpreter::makeStringValue("  var := 'c'"));
    env.proc.pushNewValue(interpreter::makeStringValue("endif"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 3);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getValueStack", env.proc.getValueStack().size(), 0U);
    a.checkEqual("03. value", value, "c");
}

/** Test instruction: sevals. Bad case - single line syntax error */
AFL_TEST("interpreter.Process:run:sevals:error:syntax", a)
{
    String_t value("a");
    Environment env;
    env.proc.pushNewContext(new SingularVariableContext(a, "VAR", value));
    env.proc.pushNewValue(interpreter::makeStringValue("if var='a'"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sevals.  Bad case - stack error */
AFL_TEST("interpreter.Process:run:sevals:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sevalx. Good case */
AFL_TEST("interpreter.Process:run:sevalx", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("47+11"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toInteger(env), 58);
}

/** Test instruction: sevalx. Null */
AFL_TEST("interpreter.Process:run:sevalx:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: sevalx. Bad case - parse error */
AFL_TEST("interpreter.Process:run:sevalx:error:syntax", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("47)"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sevalx. Bad case - stack error */
AFL_TEST("interpreter.Process:run:sevalx:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sdefsub.
    Note that this opcode is expected to deal with SubroutineValue's only, so we test it with them only.
    In fact it currently works with every type.
    Good case - new sub. */
AFL_TEST("interpreter.Process:run:sdefsub:sub", a)
{
    BCORef_t subjectBCO = makeBCO();
    subjectBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefSub, subjectBCO->addName("SUBN"));

    // Execute first sdefsub instruction
    Environment env;
    BCORef_t firstBCO = makeBCO();
    env.proc.pushNewValue(new SubroutineValue(firstBCO));
    runBCO(env, subjectBCO);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    // Verify results
    SubroutineValue* subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("SUBN")));
    a.check("11. SubroutineValue", subv);
    a.checkEqual("12. bco", &*subv->getBytecodeObject(), &*firstBCO);

    // Execute second sdefsub instruction to overwrite result
    BCORef_t secondBCO = makeBCO();
    env.proc.pushNewValue(new SubroutineValue(secondBCO));
    runBCO(env, subjectBCO);

    // Verify results
    subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("SUBN")));
    a.check("21. SubroutineValue", subv);
    a.checkEqual("22. bco", &*subv->getBytecodeObject(), &*secondBCO);
    a.checkEqual("23. getState", env.proc.getState(), Process::Ended);
}

/** Test instruction: sdefsub.
     Error case - no stack */
AFL_TEST("interpreter.Process:run:sdefsub:error", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialDefSub, 0);
    a.checkEqual("31. getState", env.proc.getState(), Process::Failed);
    a.check("32. isError", isError(env));
}

/** Test instruction: sdefshipp. */
AFL_TEST("interpreter.Process:run:sdefshipp", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefShipProperty, bco->addName("PROP"));

    Environment env;
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. property", env.world.shipPropertyNames().getIndexByName("PROP") != afl::data::NameMap::nil);
}

/** Test instruction: sdefplanetp. */
AFL_TEST("interpreter.Process:run:sdefplanetp", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefPlanetProperty, bco->addName("PROP"));

    Environment env;
    runBCO(env, bco);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. property", env.world.planetPropertyNames().getIndexByName("PROP") != afl::data::NameMap::nil);
}

/** Test instruction: sload.
    Good case: file found. Define a subroutine and check that it got defined. */
AFL_TEST("interpreter.Process:run:sload", a)
{
    static const char CODE[] = "sub loaded_sub\nendsub\n";
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    afl::base::Ref<afl::io::Stream> file = *new afl::io::ConstMemoryStream(afl::string::toBytes(CODE));
    dir->addStream("loaded.q", file);

    Environment env;
    env.world.setSystemLoadDirectory(dir.asPtr());
    env.proc.pushNewValue(interpreter::makeStringValue("loaded.q"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    SubroutineValue* subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("LOADED_SUB")));
    a.check("11. SubroutineValue", subv);
    a.checkEqual("12. getFileName", subv->getBytecodeObject()->getFileName(), file->getName());
}

/** Test instruction: sload.
    Error: file found, but has syntax error. */
AFL_TEST("interpreter.Process:run:sload:error:syntax", a)
{
    static const char CODE[] = "1+";
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("loaded.q", *new afl::io::ConstMemoryStream(afl::string::toBytes(CODE)));

    Environment env;
    env.world.setSystemLoadDirectory(dir.asPtr());
    env.proc.pushNewValue(interpreter::makeStringValue("loaded.q"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sload.
    OK'ish case: file not found */
AFL_TEST("interpreter.Process:run:sload:file-not-found", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("non.existant.q"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", !isNull(env));
}

/** Test instruction: sload. Null case */
AFL_TEST("interpreter.Process:run:sload:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: sload.
    Error case: no stack */
AFL_TEST("interpreter.Process:run:sload:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sprint. */
AFL_TEST("interpreter.Process:run:sprint", a)
{
    // Normal case: generates a message.
    // Because the interpreter generates a number of additional messages as well,
    // do not check for absolute counts, but just note the value.
    size_t normalCount;
    {
        afl::test::LogListener log;
        Environment env;
        env.log.addListener(log);
        env.proc.pushNewValue(interpreter::makeIntegerValue(42));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialPrint, 0);
        normalCount = log.getNumMessages();
        a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
        a.check("02", normalCount >= 1);
        a.checkEqual("03. getStackSize", env.proc.getStackSize(), 0U);
    }

    // Null case: no message generated, so one message less than before.
    {
        afl::test::LogListener log;
        Environment env;
        env.log.addListener(log);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialPrint, 0);
        a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("12. getNumMessages", log.getNumMessages(), normalCount-1U);
        a.checkEqual("13. getStackSize", env.proc.getStackSize(), 0U);
    }
}

/** Test instruction: saddhook.
    Good case: add two entries to a hook */
AFL_TEST("interpreter.Process:run:saddhook", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialAddHook, 0);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    env.proc.pushNewValue(new SubroutineValue(makeBCO()));
    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    env.proc.pushNewValue(new SubroutineValue(makeBCO()));
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);

    // Placing the hooks in global properties is not contractual,
    // but has been used since PCC1, so let's assume it stays for a while.
    // (It is never reflected in file formats, though.)
    a.check("11. global", env.world.globalPropertyNames().getIndexByName("ON HN") != afl::data::NameMap::nil);
}

/** Test instruction: saddhook.
    Null case */
AFL_TEST("interpreter.Process:run:saddhook:null", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. global", env.world.globalPropertyNames().getIndexByName("ON HN"), afl::data::NameMap::nil);
}

/** Test instruction: saddhook.
    Error case: addend is not a subroutine */
AFL_TEST("interpreter.Process:run:saddhook:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: saddhook.
    Error case: hook is not a subroutine (cannot normally happen) */
AFL_TEST("interpreter.Process:run:saddhook:error:hook-type", a)
{
    Environment env;
    env.world.globalValues().setNew(env.world.globalPropertyNames().add("ON HN"), interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    env.proc.pushNewValue(new SubroutineValue(makeBCO()));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: saddhook.
    Error case: invalid opcode (reserved for hooks with args) */
AFL_TEST("interpreter.Process:run:saddhook:error:opcode", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: saddhook.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:saddhook:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: srunhook.
    Good case: this does
        On HN Do gv:=42
        RunHook HN
    and examines that 'gv:=42' has been executed. */
AFL_TEST("interpreter.Process:run:srunhook", a)
{
    Environment env;

    BCORef_t hookBCO = makeBCO();
    hookBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
    hookBCO->addInstruction(Opcode::maStore, Opcode::sNamedVariable, hookBCO->addName("GV"));
    SubroutineValue hookValue(hookBCO);
    StringValue hookName("HN");

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&hookName);
    bco->addPushLiteral(&hookValue);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialAddHook, 0);
    bco->addPushLiteral(&hookName);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialRunHook, 0);

    String_t value;
    env.proc.pushNewContext(new SingularVariableContext(a, "GV", value));

    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    a.checkEqual("03. value", value, "42");
}

/** Test instruction: srunhook.
    OK'ish case: null hook content */
AFL_TEST("interpreter.Process:run:srunhook:null-content", a)
{
    Environment env;
    env.world.globalValues().setNew(env.world.globalPropertyNames().add("ON HN"), 0);
    env.proc.pushNewValue(interpreter::makeStringValue("HN"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
}

/** Test instruction: srunhook.
    Null value */
AFL_TEST("interpreter.Process:run:srunhook:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
}

/** Test instruction: srunhook.
    Error case: invalid opcode (reserved for hooks with args) */
AFL_TEST("interpreter.Process:run:srunhook:error:opcode", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: srunhook.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:srunhook:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sthrow.
    Normal case */
AFL_TEST("interpreter.Process:run:sthrow", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeStringValue("oops"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.checkEqual("02. getError", String_t(env.proc.getError().what()), "oops");
}

/** Test instruction: sthrow.
    Normal case with nonlocal error handler
    * outer installs exception handler, calls inner
    * inner produces exception
    This therefore also tests 'catch'. */
AFL_TEST("interpreter.Process:run:sthrow:catch", a)
{
    Environment env;

    BCORef_t innerBCO = makeBCO();
    innerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 8888);
    innerBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    SubroutineValue innerValue(innerBCO);

    BCORef_t outerBCO = makeBCO();
    outerBCO->addInstruction(Opcode::maJump, Opcode::jCatch, 4);
    outerBCO->addPushLiteral(&innerValue);
    outerBCO->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0); // will call the throwing function
    outerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 4444);  // will not be executed
    outerBCO->addInstruction(Opcode::maPush, Opcode::sInteger, 5555);  // will be executed

    runBCO(env, outerBCO);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 2U);
    a.checkEqual("03. result", toInteger(env), 5555);           // value pushed by catch handler
    env.proc.dropValue();
    a.checkEqual("04. result", toString(env), "8888");          // thrown value, stringified by throwing
}

/** Test instruction: sthrow.
    OK'ish case: null */
AFL_TEST("interpreter.Process:run:sthrow:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sthrow.
    Error case: missing stack (still fails) */
AFL_TEST("interpreter.Process:run:sthrow:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sterminate. */
AFL_TEST("interpreter.Process:run:sterminate", a)
{
    // Execute instruction. This should not affect the stack.
    const size_t N = 8;
    Environment env;
    for (size_t i = 0; i < N; ++i) {
        env.proc.pushNewValue(0);
    }
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialTerminate, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Terminated);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), N);
}

/** Test instruction: ssuspend. */
AFL_TEST("interpreter.Process:run:ssuspend", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Suspended);
}

/** Test instruction: snewarray.
    Normal case */
AFL_TEST("interpreter.Process:run:snewarray", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    const ArrayValue* av = dynamic_cast<const ArrayValue*>(env.proc.getResult());
    a.check("11. ArrayValue", av);
    a.checkEqual("12. getNumDimensions", av->getData()->getNumDimensions(), 2U);
    a.checkEqual("13. getDimension(0)",  av->getData()->getDimension(0), 3U);
    a.checkEqual("14. getDimension(1)",  av->getData()->getDimension(1), 4U);
}

/** Test instruction: snewarray.
    Error case: wrong type */
AFL_TEST("interpreter.Process:run:snewarray:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeStringValue("X"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snewarray.
    Error case: null dimension */
AFL_TEST("interpreter.Process:run:snewarray:error:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snewarray.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:snewarray:error:stack", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 3);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snewarray.
    Error case: 0 dimensions */
AFL_TEST("interpreter.Process:run:snewarray:error:zero", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snewarray.
    Error case: too large */
AFL_TEST("interpreter.Process:run:snewarray:error:too-large", a)
{
    Environment env;
    for (int i = 0; i < 10; ++i) {
        env.proc.pushNewValue(interpreter::makeIntegerValue(1000));
    }
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 10);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
}

/** Test instruction: smakelist.
    Normal case */
AFL_TEST("interpreter.Process:run:smakelist", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(0);
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialMakeList, 4);

    // Result type
    const ArrayValue* av = dynamic_cast<const ArrayValue*>(env.proc.getResult());
    a.check("01. ArrayValue", av);
    a.checkEqual("02. getNumDimensions", av->getData()->getNumDimensions(), 1U);
    a.checkEqual("03. getDimension",     av->getData()->getDimension(0), 4U);

    // Array content
    a.checkEqual("11. value 0", interpreter::toString(av->getData()->content().get(0), false), "1");
    a.checkEqual("12. value 1", interpreter::toString(av->getData()->content().get(1), false), "2");
    a.checkNull ("13. value 2", av->getData()->content().get(2));
    a.checkEqual("14. value 3", interpreter::toString(av->getData()->content().get(3), false), "4");
}

/** Test instruction: smakelist.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:smakelist:error", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialMakeList, 3);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}


/** Test instruction: snewhash.
    Normal case */
AFL_TEST("interpreter.Process:run:snewhash", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkNonNull("02. HashValue", dynamic_cast<const HashValue*>(env.proc.getResult()));
}

/** Test instruction: snewhash.
    Error case: invalid opcode */
AFL_TEST("interpreter.Process:run:snewhash:error", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewHash, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sinstance.
    Normal case */
AFL_TEST("interpreter.Process:run:sinstance", a)
{
    StructureTypeData::Ref_t type = *new StructureTypeData();
    Environment env;
    env.proc.pushNewValue(new StructureType(type));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    const StructureValue* sv = dynamic_cast<const StructureValue*>(env.proc.getResult());
    a.check("11. StructureValue", sv);
    a.checkEqual("12. type", &sv->getValue()->type(), &*type);
}

/** Test instruction: sinstance.
    Error case: wrong type */
AFL_TEST("interpreter.Process:run:sinstance:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sinstance.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:sinstance:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Normal case */
AFL_TEST("interpreter.Process:run:sresizearray", a)
{
    afl::base::Ref<ArrayData> ad(make2DArray());
    Environment env;
    env.proc.pushNewValue(new ArrayValue(ad));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);

    a.checkEqual("11. getNumDimensions", ad->getNumDimensions(), 2U);
    a.checkEqual("12. getDimension 0", ad->getDimension(0), 3U);
    a.checkEqual("13. getDimension 1", ad->getDimension(1), 4U);
}

/** Test instruction: sresizearray.
    Error case: wrong dimension type */
AFL_TEST("interpreter.Process:run:sresizearray:error:type:dim", a)
{
    Environment env;
    env.proc.pushNewValue(new ArrayValue(make2DArray()));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    env.proc.pushNewValue(interpreter::makeStringValue("X"));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Error case: wrong array type */
AFL_TEST("interpreter.Process:run:sresizearray:error:type:array", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    env.proc.pushNewValue(interpreter::makeIntegerValue(2));
    env.proc.pushNewValue(interpreter::makeIntegerValue(3));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Error case: null dimension */
AFL_TEST("interpreter.Process:run:sresizearray:error:null", a)
{
    Environment env;
    env.proc.pushNewValue(new ArrayValue(make2DArray()));
    env.proc.pushNewValue(0);
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Error case: missing stack */
AFL_TEST("interpreter.Process:run:sresizearray:error:stack", a)
{
    Environment env;
    env.proc.pushNewValue(new ArrayValue(make2DArray()));
    env.proc.pushNewValue(interpreter::makeIntegerValue(4));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Error case: incompatible dimensions */
AFL_TEST("interpreter.Process:run:sresizearray:error:incompatible", a)
{
    Environment env;
    env.proc.pushNewValue(new ArrayValue(make2DArray()));
    env.proc.pushNewValue(interpreter::makeIntegerValue(7));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 1);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sresizearray.
    Error case: no dimension */
AFL_TEST("interpreter.Process:run:sresizearray:error:zero", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snewarray.
    Error case: too large */
AFL_TEST("interpreter.Process:run:sresizearray:error:too-large", a)
{
    const int NDIM = 10;
    Environment env;
    afl::base::Ref<ArrayData> ad(*new ArrayData());
    for (int i = 0; i < NDIM; ++i) {
        a.check("01. addDimension", ad->addDimension(1));
    }
    env.proc.pushNewValue(new ArrayValue(ad));
    for (int i = 0; i < NDIM; ++i) {
        env.proc.pushNewValue(interpreter::makeIntegerValue(1000));
    }
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 10);
    a.checkEqual("11. getState", env.proc.getState(), Process::Failed);
}

/** Test instruction: sbind.
    Normal case: test the entire feature: outerBCO binds innerBCO and calls it
    For simplicity, this runs the 'bind' in a separate process. */
AFL_TEST("interpreter.Process:run:sbind", a)
{
    Environment env;

    BCORef_t innerBCO = makeBCO();
    innerBCO->addArgument("A", false);
    innerBCO->addArgument("B", false);
    innerBCO->addInstruction(Opcode::maPush, Opcode::sLocal, 0);
    innerBCO->addInstruction(Opcode::maPush, Opcode::sLocal, 1);
    innerBCO->addInstruction(Opcode::maBinary, interpreter::biConcat, 0);

    // Bind
    BCORef_t firstBCO = makeBCO();
    Process firstProcess(env.world, "first", 1);
    firstProcess.pushNewValue(interpreter::makeStringValue("A"));           // arg to bind
    firstProcess.pushNewValue(new SubroutineValue(innerBCO));               // function to bind
    firstBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialBind, 1);  // produces bound function
    firstProcess.pushFrame(firstBCO, true);
    firstProcess.run();

    // Result must be valid and callable
    a.checkEqual("01. getState", firstProcess.getState(), Process::Ended);
    a.checkNonNull("02. CallableValue", dynamic_cast<const interpreter::CallableValue*>(firstProcess.getResult()));

    // Execute new callable in regular Environment process for easier evaluation
    env.proc.pushNewValue(interpreter::makeStringValue("B"));
    env.proc.pushNewValue(firstProcess.getResult()->clone());
    runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);

    // Result must be valid
    a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("12. result", toString(env), "AB");
}

/** Test instruction: sbind.
    Error case: null callable */
AFL_TEST("interpreter.Process:run:sbind:error:null", a)
{
    Environment env;
    env.proc.pushNewValue(0);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialBind, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sbind.
    Error case: no stack */
AFL_TEST("interpreter.Process:run:sbind:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialBind, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sfirst.
    Good case: non-empty iterable. */
AFL_TEST("interpreter.Process:run:sfirst", a)
{
    Hash::Ref_t hash = Hash::create();
    hash->setNew("kk", interpreter::makeIntegerValue(1));

    Environment env;
    env.proc.pushNewValue(new HashValue(hash));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);

    // Result must be context
    const interpreter::Context* ctx = dynamic_cast<const interpreter::Context*>(env.proc.getResult());
    a.checkNonNull("11. Context", ctx);

    // Result must have correct key
    interpreter::Context::PropertyIndex_t idx;
    a.check("21. lookup", const_cast<interpreter::Context*>(ctx)->lookup("KEY", idx));
}

/** Test instruction: sfirst.
    Good case: empty iterable. */
AFL_TEST("interpreter.Process:run:sfirst:empty", a)
{
    Environment env;
    env.proc.pushNewValue(new HashValue(Hash::create()));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.check("02. result", isNull(env));
}

/** Test instruction: sfirst.
    Error case: type error */
AFL_TEST("interpreter.Process:run:sfirst:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: sfirst.
    Error case: no stack */
AFL_TEST("interpreter.Process:run:sfirst:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snext.
    Good case: non-empty iterable. */
AFL_TEST("interpreter.Process:run:snext", a)
{
    // Create hash and point iterator at first element
    Hash::Ref_t hash = Hash::create();
    hash->setNew("k1", interpreter::makeIntegerValue(1));
    hash->setNew("k2", interpreter::makeIntegerValue(2));
    interpreter::Context* iter = HashValue(hash).makeFirstContext();
    a.check("01. makeFirstContext", iter);

    Environment env;
    env.proc.pushNewValue(iter);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
    a.checkEqual("11. getState", env.proc.getState(), Process::Ended);

    // Result must still be context
    const interpreter::Context* ctx = dynamic_cast<const interpreter::Context*>(env.proc.getResult());
    a.checkNonNull("21. Context", ctx);

    // Result must have correct key
    interpreter::Context::PropertyIndex_t idx;
    a.check("31. lookup", const_cast<interpreter::Context*>(ctx)->lookup("KEY", idx));
}

/** Test instruction: snext.
    Good case: final element of iterable. */
AFL_TEST("interpreter.Process:run:snext:final", a)
{
    // Create hash and point iterator at first element
    Hash::Ref_t hash = Hash::create();
    hash->setNew("kk", interpreter::makeIntegerValue(1));
    interpreter::Context* iter = HashValue(hash).makeFirstContext();
    a.check("01. makeFirstContext", iter);

    Environment env;
    env.proc.pushNewValue(iter);
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
    a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
    a.check("12. result", isNull(env));
}

/** Test instruction: snext.
    Exercise use of independant copies */
AFL_TEST("interpreter.Process:run:snext:copies", a)
{
    Environment env;
    env.proc.pushNewValue(new CountingContext(a, "COUNT", 10));

    BCORef_t bco = makeBCO();

    // Store 3 copies containing values 10,11,12
    for (int i = 0; i < 3; ++i) {
        bco->addInstruction(Opcode::maStore, Opcode::sLocal, uint16_t(i));
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNext, 0);
    }

    // TOS contains 13; load that
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("COUNT"));

    // Load values from the copies
    for (int i = 0; i < 3; ++i) {
        bco->addInstruction(Opcode::maPush, Opcode::sLocal, uint16_t(i));
        bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("COUNT"));
    }

    runBCO(env, bco);

    // Stack must now contain 12:11:10:13
    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. getStackSize", env.proc.getStackSize(), 4U);
    a.checkEqual("03. result", toInteger(env), 12);
    env.proc.dropValue();
    a.checkEqual("04. result", toInteger(env), 11);
    env.proc.dropValue();
    a.checkEqual("05. result", toInteger(env), 10);
    env.proc.dropValue();
    a.checkEqual("06. result", toInteger(env), 13);;
}

/** Test instruction: snext.
    Error case: type error */
AFL_TEST("interpreter.Process:run:snext:error:type", a)
{
    Environment env;
    env.proc.pushNewValue(interpreter::makeIntegerValue(1));
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: snext.
    Error case: no stack */
AFL_TEST("interpreter.Process:run:snext:error:stack", a)
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
    a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
    a.check("02. isError", isError(env));
}

/** Test instruction: fused unary (push + unary). */
AFL_TEST("interpreter.Process:run:fused-unary", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    env.world.globalValues().setNew(77, interpreter::makeIntegerValue(1337));
    bco->addInstruction(Opcode::maFusedUnary, Opcode::sShared, 77);
    bco->addInstruction(Opcode::maUnary, interpreter::unStr, 0);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "1337");
}

/** Test instruction: fused binary (push + binary). */
AFL_TEST("interpreter.Process:run:fused-binary", a)
{
    Environment env;
    BCORef_t bco = makeBCO();
    env.world.globalValues().setNew(77, interpreter::makeStringValue("a"));    // second arg
    env.proc.pushNewValue(interpreter::makeStringValue("b"));                  // first arg
    bco->addInstruction(Opcode::maFusedBinary, Opcode::sShared, 77);
    bco->addInstruction(Opcode::maBinary, interpreter::biConcat, 0);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. result", toString(env), "ba");
}

/** Test instruction: fused comparison (bcmp + j). */
AFL_TEST("interpreter.Process:run:fused-comparison", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maFusedComparison, interpreter::biCompareEQ, 0);
    bco->addInstruction(Opcode::maJump, Opcode::jIfTrue | Opcode::jPopAlways, 3);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);

    // Taken jump
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runBCO(env, bco);

        a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    }

    // Not taken jump
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        runBCO(env, bco);

        a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("12. getStackSize", env.proc.getStackSize(), 1U);
        a.checkEqual("13. result", toInteger(env), 42);
    }
}

/** Test instruction: fused comparison (push + bcmp + j). */
AFL_TEST("interpreter.Process:run:fused-comparison2", a)
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maFusedComparison2, Opcode::sShared, 55);
    bco->addInstruction(Opcode::maBinary, interpreter::biCompareLT, 0);
    bco->addInstruction(Opcode::maJump, Opcode::jIfTrue | Opcode::jPopAlways, 4);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);

    // Taken jump
    {
        Environment env;
        env.world.globalValues().setNew(55, interpreter::makeIntegerValue(10)); // second arg
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));                // first arg
        runBCO(env, bco);

        a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("02. getStackSize", env.proc.getStackSize(), 0U);
    }

    // Not taken jump
    {
        Environment env;
        env.world.globalValues().setNew(55, interpreter::makeIntegerValue(10)); // second arg
        env.proc.pushNewValue(interpreter::makeIntegerValue(100));              // first arg
        runBCO(env, bco);

        a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("12. getStackSize", env.proc.getStackSize(), 1U);
        a.checkEqual("13. result", toInteger(env), 42);
    }
}

/** Test instruction: in-place unary (pushloc + uinc/udec). */
AFL_TEST("interpreter.Process:run:inplace-unary", a)
{
    BCORef_t incBCO = makeBCO();
    incBCO->addInstruction(Opcode::maInplaceUnary, Opcode::sLocal, 12);
    incBCO->addInstruction(Opcode::maUnary, interpreter::unInc, 0);

    BCORef_t decBCO = makeBCO();
    decBCO->addInstruction(Opcode::maInplaceUnary, Opcode::sLocal, 12);
    decBCO->addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    // Increment integer
    {
        Environment env;
        Process::Frame& frame = env.proc.pushFrame(incBCO, true);
        frame.localValues.setNew(12, interpreter::makeIntegerValue(4));
        env.proc.run();

        a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("02. result", toInteger(env), 5);
    }

    // Decement float
    {
        Environment env;
        Process::Frame& frame = env.proc.pushFrame(decBCO, true);
        frame.localValues.setNew(12, interpreter::makeFloatValue(2.5));
        env.proc.run();

        a.checkEqual("11. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("12. result", toFloat(env), 1.5);
    }

    // Increment bool (value not optimized, type change)
    {
        Environment env;
        Process::Frame& frame = env.proc.pushFrame(incBCO, true);
        frame.localValues.setNew(12, interpreter::makeBooleanValue(1));
        env.proc.run();

        a.checkEqual("21. getState", env.proc.getState(), Process::Ended);
        a.checkEqual("22. result", toInteger(env), 2);
    }

    // Decrement null (value not optimized and handled normally, but doesn't generate an error)
    {
        Environment env;
        /*Process::Frame& frame =*/ env.proc.pushFrame(decBCO, true);
        env.proc.run();

        a.checkEqual("31. getState", env.proc.getState(), Process::Ended);
        a.check("32. result", isNull(env));
    }
}

// Test onContextEntered(), onContextLeft().
AFL_TEST("interpreter.Process:context-callback", a)
{
    // Execute 'swith', 'sendwith'
    Environment env;

    String_t trace;
    TracingContext ctx(trace, false);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&ctx);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. trace", trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context is left abnormally. */
AFL_TEST("interpreter.Process:context-callback:abnormal-exit", a)
{
    // Execute 'swith', 'sthrow' > context is left implicitly, not by 'sendwith'
    String_t trace;
    TracingContext ctx(trace, false);

    {
        Environment env;

        BCORef_t bco = makeBCO();
        bco->addPushLiteral(&ctx);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 3);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        runBCO(env, bco);

        a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
        // Context will be destroyed here
    }

    a.checkEqual("11. trace", trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context is left abnormally, but error is caught. */
AFL_TEST("interpreter.Process:context-callback:catch", a)
{
    // Execute 'swith', 'sendwith'
    Environment env;

    String_t trace;
    TracingContext ctx(trace, false);

    BCORef_t bco = makeBCO();
    BytecodeObject::Label_t lcatch = bco->makeLabel();
    bco->addInstruction(Opcode::maJump, Opcode::jCatch | Opcode::jSymbolic, lcatch);
    bco->addPushLiteral(&ctx);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 3);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    bco->addLabel(lcatch);
    runBCO(env, bco);

    a.checkEqual("01. getState", env.proc.getState(), Process::Ended);
    a.checkEqual("02. trace", trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context rejects entering.
    In this case, the leave callback must not be called. */
AFL_TEST("interpreter.Process:context-callback:refuse", a)
{
    // Execute 'swith', 'sthrow' > context is left implicitly, not by 'sendwith'
    String_t trace;
    TracingContext ctx(trace, true);

    {
        Environment env;

        BCORef_t bco = makeBCO();
        bco->addPushLiteral(&ctx);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        runBCO(env, bco);

        a.checkEqual("01. getState", env.proc.getState(), Process::Failed);
        // Context will be destroyed here
    }

    a.checkEqual("11. trace", trace, "(enter)");
}

namespace {
    struct Insn {
        Opcode::Major major;
        uint8_t minor;
        uint16_t arg;
    };

    void doNameErrorTest(afl::test::Assert a, afl::base::Memory<const Insn> opc)
    {
        Environment env;
        BCORef_t bco = makeBCO();
        while (const Insn* p = opc.eat()) {
            bco->addInstruction(p->major, p->minor, p->arg);
        }
        runBCO(env, bco);
        a.checkEqual("getState", env.proc.getState(), Process::Failed);
    }
}

/*
 *  Out-of-range names.
 *  Check that we correctly verify the "name" parameter of parameters that use local names.
 */

AFL_TEST("interpreter.Process:name-error:pushvar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sNamedVariable, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:pushgvar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sNamedShared, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:storevar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maStore, Opcode::sNamedVariable, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:storegvar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maStore, Opcode::sNamedShared, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:popvar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maPop, Opcode::sNamedVariable, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:popgvar", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maPop, Opcode::sNamedShared, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:loadmem", a)
{
    // Note: to trigger the original problem, this would have to push a context
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maMemref, Opcode::miIMLoad, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:storemem", a)
{
    // Note: to trigger the original problem, this would have to push a context
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maMemref, Opcode::miIMStore, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:sdefsub", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maSpecial, Opcode::miSpecialDefSub, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:sdefsp", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maSpecial, Opcode::miSpecialDefShipProperty, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:sdefpp", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maSpecial, Opcode::miSpecialDefPlanetProperty, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:dim", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maDim, Opcode::sLocal, 99 }};
    doNameErrorTest(a, code);
}

AFL_TEST("interpreter.Process:name-error:fusedunary", a)
{
    static const Insn code[] = {{ Opcode::maPush, Opcode::sInteger, 1 },
                                { Opcode::maFusedUnary, Opcode::sNamedShared, 99 },
                                { Opcode::maUnary, interpreter::unInc, 0 }};
    doNameErrorTest(a, code);
}

