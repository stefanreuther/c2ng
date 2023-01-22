/**
  *  \file u/t_interpreter_process.cpp
  *  \brief Test for interpreter::Process
  */

#include "interpreter/process.hpp"

#include "t_interpreter.hpp"
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
            { TS_FAIL("SingularObjectContext::next unexpected"); return false; }
        virtual Context* clone() const
            { TS_FAIL("SingularObjectContext::clone unexpected"); return 0; }
        virtual afl::base::Deletable* getObject()
            { return m_pObject; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { TS_FAIL("SingularObjectContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { TS_FAIL("SingularObjectContext::toString unexpected"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("SingularObjectContext::store unexpected"); }
     private:
        afl::base::Deletable* m_pObject;
    };

    /* Singular variable context.
       We don't expect this context to be copied or examined in another way.
       It only provides a single variable.
       (Turns out that optionally allowing cloning is helpful.) */
    class SingularVariableContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        SingularVariableContext(String_t name, String_t& value)
            : m_name(name), m_value(value), m_clonable(false)
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
                TS_ASSERT_EQUALS(index, PropertyIndex_t(77));
                m_value = interpreter::toString(value, false);
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                TS_ASSERT_EQUALS(index, PropertyIndex_t(77));
                return interpreter::makeStringValue(m_value);
            }
        virtual bool next()
            { TS_FAIL("SingularVariableContext::next unexpected"); return false; }
        virtual Context* clone() const
            {
                TS_ASSERT(m_clonable);
                return new SingularVariableContext(*this);
            }
        virtual afl::base::Deletable* getObject()
            { TS_FAIL("SingularVariableContext::getObject unexpected"); return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { TS_FAIL("SingularVariableContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { TS_FAIL("SingularVariableContext::toString unexpected"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("SingularVariableContext::store unexpected"); }
     private:
        String_t m_name;
        String_t& m_value;
        bool m_clonable;
    };

    /* Counting context.
       Exposes a single variable whose value changes with next(). */
    class CountingContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        CountingContext(String_t name, int32_t value)
            : m_name(name), m_value(value)
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
            { TS_FAIL("CountingContext::set unexpected"); }
        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                TS_ASSERT_EQUALS(index, PropertyIndex_t(66));
                return interpreter::makeIntegerValue(m_value);
            }
        virtual bool next()
            { ++m_value; return true; }
        virtual Context* clone() const
            { return new CountingContext(*this); }
        virtual afl::base::Deletable* getObject()
            { TS_FAIL("CountingContext::getObject unexpected"); return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { TS_FAIL("CountingContext::enumProperties unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { TS_FAIL("CountingContext::toString unexpected"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("CountingContext::store unexpected"); }
     private:
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
        virtual int32_t getDimension(int32_t /*which*/) const
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
        virtual void set(interpreter::Arguments& args, afl::data::Value* value)
            {
                args.checkArgumentCount(m_numArgs);
                m_value = interpreter::toString(value, false);
            }
        virtual int32_t getDimension(int32_t /*which*/) const
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
            { TS_FAIL("TracingContext::store unexpected"); }
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
}


/** Test process properties. */
void
TestInterpreterProcess::testProperties()
{
    Environment env;

    // We're testing process properties, so create our own private process
    Process testee(env.world, "processName", 42);

    // Initial states
    TS_ASSERT_EQUALS(testee.getName(), "processName");
    TS_ASSERT_EQUALS(testee.getProcessId(), 42U);
    TS_ASSERT_EQUALS(testee.getState(), testee.Suspended);

    // Initial group Id is unset
    TS_ASSERT_EQUALS(testee.getProcessGroupId(), 0U);
    testee.setProcessGroupId(23);
    TS_ASSERT_EQUALS(testee.getProcessGroupId(), 23U);

    // Initial priority is 50
    TS_ASSERT_EQUALS(testee.getPriority(), 50);
    testee.setPriority(12);
    TS_ASSERT_EQUALS(testee.getPriority(), 12);

    // No initial kind
    TS_ASSERT_EQUALS(testee.getProcessKind(), testee.pkDefault);
    testee.setProcessKind(testee.pkBaseTask);
    TS_ASSERT_EQUALS(testee.getProcessKind(), testee.pkBaseTask);

    // Name
    testee.setName("otherName");
    TS_ASSERT_EQUALS(testee.getName(), "otherName");

    // State
    testee.setState(Process::Ended);
    TS_ASSERT_EQUALS(testee.getState(), testee.Ended);

    // Stack
    TS_ASSERT_EQUALS(testee.getStackSize(), 0U);
    TS_ASSERT_EQUALS(testee.getValueStack().size(), 0U);

    // toString
    afl::test::Translator tx("<", ">");
    TS_ASSERT_EQUALS(interpreter::toString(Process::Runnable, tx), "<Runnable>");
}

/** Test freezing: correct state, collision. */
void
TestInterpreterProcess::testFreeze()
{
    Environment env;

    // We can freeze a fresh process
    NullFreezer fz;
    TS_ASSERT_THROWS_NOTHING(env.proc.freeze(fz));
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(env.proc.getFreezer(), static_cast<Process::Freezer*>(&fz));

    // We cannot freeze it again, not even re-using the same freezer
    {
        NullFreezer fz2;
        TS_ASSERT_THROWS(env.proc.freeze(fz2), interpreter::Error);
        TS_ASSERT_THROWS(env.proc.freeze(fz), interpreter::Error);
    }

    // Unfreeze
    TS_ASSERT_THROWS_NOTHING(env.proc.unfreeze());
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(env.proc.getFreezer(), (Process::Freezer*) 0);

    // Can freeze again
    TS_ASSERT_THROWS_NOTHING(env.proc.freeze(fz));
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(env.proc.getFreezer(), static_cast<Process::Freezer*>(&fz));
}

/** Test freezing: wrong state. */
void
TestInterpreterProcess::testFreeze2()
{
    Environment env;

    // Change state
    env.proc.setState(Process::Waiting);

    // Process cannot be frozen in wrong state
    NullFreezer fz;
    TS_ASSERT_THROWS(env.proc.freeze(fz), interpreter::Error);
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Waiting);
    TS_ASSERT_EQUALS(env.proc.getFreezer(), (Process::Freezer*) 0);

    // Process cannot be unfrozen in wrong state (but this does not throw)
    TS_ASSERT_THROWS_NOTHING(env.proc.unfreeze());
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Waiting);
}

/** Test finalize(): finalizer is not called implicitly upon process destruction */
void
TestInterpreterProcess::testFinalize()
{
    int callCount = 0;
    {
        Environment env;
        env.proc.setNewFinalizer(new CountingFinalizer(callCount));
    }
    TS_ASSERT_EQUALS(callCount, 0);
}

/** Test finalize(): finalizer is called once no matter how often we explicitly finalize. */
void
TestInterpreterProcess::testFinalize2()
{
    int callCount = 0;
    Environment env;
    env.proc.setNewFinalizer(new CountingFinalizer(callCount));
    env.proc.finalize();
    env.proc.finalize();
    TS_ASSERT_EQUALS(callCount, 1);
}

/** Test context stack: getInvokingObject(), getCurrentObject(), markContextTOS(). */
void
TestInterpreterProcess::testContextStack()
{
    Environment env;

    // Initial context stack is empty
    TS_ASSERT(env.world.globalContexts().empty());
    TS_ASSERT(env.proc.getContexts().empty());
    TS_ASSERT_EQUALS(env.proc.getContextTOS(), 0U);

    // Push some contexts
    NullObject one, two;
    env.proc.pushNewContext(new SingularObjectContext(0));
    env.proc.pushNewContext(new SingularObjectContext(&one));
    env.proc.markContextTOS();
    env.proc.pushNewContext(new SingularObjectContext(&two));
    env.proc.pushNewContext(new SingularObjectContext(0));
    TS_ASSERT_EQUALS(env.proc.getContextTOS(), 2U);

    // Check objects
    TS_ASSERT(env.proc.getInvokingObject() == &one);
    TS_ASSERT(env.proc.getCurrentObject() == &two);

    // Modify TOS
    TS_ASSERT_EQUALS(env.proc.setContextTOS(4), true);
    TS_ASSERT(env.proc.getInvokingObject() == &two);
    TS_ASSERT(env.proc.getCurrentObject() == &two);

    // Pop context. This must fix up contextTOS.
    env.proc.popContext();
    TS_ASSERT_EQUALS(env.proc.getContextTOS(), 3U);
    TS_ASSERT(env.proc.getInvokingObject() == &two);
    TS_ASSERT(env.proc.getCurrentObject() == &two);

    // Out-of-range values refused
    TS_ASSERT_EQUALS(env.proc.setContextTOS(9), false);
}

/** Test context stack: pushContextsFrom(). */
void
TestInterpreterProcess::testContextStack2()
{
    Environment env;

    // Starts with no current object
    TS_ASSERT(env.proc.getCurrentObject() == 0);
    TS_ASSERT(env.proc.getInvokingObject() == 0);

    // Make a context vector
    afl::container::PtrVector<interpreter::Context> vec;
    NullObject one, two;
    vec.pushBackNew(new SingularObjectContext(&one));
    vec.pushBackNew(new SingularObjectContext(&two));
    env.proc.pushContextsFrom(vec);

    // Verify
    TS_ASSERT(env.proc.getCurrentObject() == &two);
    TS_ASSERT(env.proc.getInvokingObject() == 0);
}

/** Test variable access: setVariable(), getVariable(). */
void
TestInterpreterProcess::testVariable()
{
    Environment env;

    // Make two variable contexts; we'll be modifying the inner one
    String_t inner = "i", outer = "o";
    env.proc.pushNewContext(new SingularVariableContext("VALUE", outer));
    env.proc.pushNewContext(new SingularVariableContext("VALUE", inner));

    // Check value
    std::auto_ptr<afl::data::Value> p(env.proc.getVariable("VALUE"));
    TS_ASSERT_EQUALS(interpreter::toString(p.get(), false), "i");

    // Set value
    afl::data::StringValue sv("nv");
    TS_ASSERT_EQUALS(env.proc.setVariable("VALUE", &sv), true);
    TS_ASSERT_EQUALS(inner, "nv");
    TS_ASSERT_EQUALS(outer, "o");

    // Accessing unknown values is harmless
    TS_ASSERT_EQUALS(env.proc.setVariable("OTHER", &sv), false);
    p = env.proc.getVariable("OTHER");
    TS_ASSERT(p.get() == 0);
}

/** Test execution: invalid opcode. */
void
TestInterpreterProcess::testExecInvalid()
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
        TSM_ASSERT_EQUALS(CASES[i].label, env.proc.getState(), Process::Failed);
        TSM_ASSERT(CASES[i].label, isError(env));
    }
}

/** Test instruction: pushvar. */
void
TestInterpreterProcess::testExecPushNamed()
{
    // Execute a single standalone 'pushvar' instruction
    Environment env;
    String_t value("theValue");
    env.proc.pushNewContext(new SingularVariableContext("VALUE", value));
    env.proc.pushNewContext(new SingularObjectContext(0));

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VALUE"));
    runBCO(env, bco);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "theValue");
}

/** Test instruction: pushloc. */
void
TestInterpreterProcess::testExecPushLocal()
{
    // Execute a single 'pushloc' instruction on a frame containing a local value.
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sLocal, 3);

    Process::Frame& f = env.proc.pushFrame(bco, true);
    f.localValues.setNew(3, interpreter::makeStringValue("local"));

    env.proc.run();

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "local");
}

/** Test instruction: pushtop. */
void
TestInterpreterProcess::testExecPushStatic()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "outer");
}

/** Test instruction: pushglob. */
void
TestInterpreterProcess::testExecPushShared()
{
    // Set a global value. Execute single 'pushglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    runInstruction(env, Opcode::maPush, Opcode::sShared, 99);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "v");
}

/** Test instruction: pushgvar. */
void
TestInterpreterProcess::testExecPushNamedShared()
{
    // Set a global value by name. Execute single 'pushgvar' instruction.
    {
        Environment env;
        env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("GV"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "q");
    }

    // Error case
    {
        Environment env;

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("XXXXX"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: pushlit. */
void
TestInterpreterProcess::testExecPushLiteral()
{
    // Execute single standalone 'pushlit' instruction.
    Environment env;

    FloatValue fv(2.5);
    BCORef_t bco = makeBCO();
    bco->addPushLiteral(&fv);
    TS_ASSERT_EQUALS((*bco)(0).major, Opcode::maPush);
    TS_ASSERT_EQUALS((*bco)(0).minor, Opcode::sLiteral);
    runBCO(env, bco);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toFloat(env), 2.5);
}

/** Test instruction: pushint. */
void
TestInterpreterProcess::testExecPushInteger()
{
    // Execute single standalone 'pushint' instruction.
    {
        Environment env;
        runInstruction(env, Opcode::maPush, Opcode::sInteger, 45);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 45);
    }

    // Same thing, negative value
    {
        Environment env;
        runInstruction(env, Opcode::maPush, Opcode::sInteger, 0xFFFE);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), -2);
    }
}

/** Test instruction: pushbool. */
void
TestInterpreterProcess::testExecPushBoolean()
{
    // Execute single standalone 'pushbool' instruction.
    // Two cases because this produces not only bools but also EMPTY.
    {
        Environment env;
        runInstruction(env, Opcode::maPush, Opcode::sBoolean, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toBoolean(env), true);
    }
    {
        Environment env;
        runInstruction(env, Opcode::maPush, Opcode::sBoolean, -1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }
}

/** Test instruction: uinc (as specimen for unary). */
void
TestInterpreterProcess::testExecUnary()
{
    // Good case: execute single uinc instruction on stack with one element
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(32));
        runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 33);
    }

    // Bad case: execute single uinc instruction on empty stack
    {
        Environment env;
        runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Extra bad case: type error needs to be reflected into process state
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("Q"));
        runInstruction(env, Opcode::maUnary, interpreter::unInc, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: badd (as specimen for binary). */
void
TestInterpreterProcess::testExecBinary()
{
    // Good case: execute single badd instruction on stack with one element
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("aa"));
        env.proc.pushNewValue(interpreter::makeStringValue("bbb"));
        runInstruction(env, Opcode::maBinary, interpreter::biAdd, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "aabbb");
    }

    // Bad case: execute single badd instruction on stack with too few elements
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("aa"));
        runInstruction(env, Opcode::maBinary, interpreter::biAdd, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: tkeyadd (as specimen for ternary). */
void
TestInterpreterProcess::testExecTernary()
{
    // Good case: set up a keymap and add a key (this is our only ternary op so far)
    {
        Environment env;
        util::KeymapRef_t k = env.world.keymaps().createKeymap("K");
        env.proc.pushNewValue(new interpreter::KeymapValue(k));
        env.proc.pushNewValue(interpreter::makeStringValue("q"));
        env.proc.pushNewValue(interpreter::makeStringValue("cmd"));
        runInstruction(env, Opcode::maTernary, interpreter::teKeyAdd, 0);

        TS_ASSERT(!isNull(env));

        const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(env.proc.getResult());
        TS_ASSERT(kv);
        TS_ASSERT(kv->getKeymap() == k);
        TS_ASSERT(k->lookupCommand('q') != 0);
    }

    // Bad case: execute instruction on stack with too few elements
    {
        Environment env;
        env.proc.pushNewValue(0);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maTernary, interpreter::teKeyAdd, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: conditional jump, taken. */
void
TestInterpreterProcess::testExecJumpCondTaken()
{
    // pushint 42 / pushint 1 / jtp end / pushint 43: result must be 42
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
        bco->addInstruction(Opcode::maJump, Opcode::jIfTrue | Opcode::jPopAlways, 4);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 42);
    }

    // pushint 42 / jt end / pushint 43: result must be 42 (same thing without implicit pop)
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
        bco->addInstruction(Opcode::maJump, Opcode::jIfTrue, 4);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 42);
    }

    // jt end: fails, no value to test on stack
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maJump, Opcode::jIfTrue, 1);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: conditional jump, not taken. */
void
TestInterpreterProcess::testExecJumpCondMiss()
{
    // pushint 42 / pushint 1 / jfp end / pushint 43: result must be 43
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
        bco->addInstruction(Opcode::maJump, Opcode::jIfFalse | Opcode::jPopAlways, 4);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 43);
    }
    // pushint 42 / jf end / pushint 43: result must be 43 (same thing without implicit pop)
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 42);
        bco->addInstruction(Opcode::maJump, Opcode::jIfFalse, 4);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 43);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 43);
    }
}

/** Test instruction: unconditional jump. */
void
TestInterpreterProcess::testExecJumpAlways()
{
    // Unconditional jump can be executed without stuff on stack
    // j 2 / <invalid> / pushint 89: result must be 89.
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maJump, Opcode::jAlways, 2);
        bco->addInstruction(Opcode::maDim, 200, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 89);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 89);
    }

    // Unconditional with pop
    // pushint 17 / pushint 18 / jp end: result must be 17
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 17);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 18);
        bco->addInstruction(Opcode::maJump, Opcode::jAlways | Opcode::jPopAlways, 3);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 17);
    }

    // Unconditional with pop fails if stack empty
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maJump, Opcode::jAlways | Opcode::jPopAlways, 1);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: catch. */
void
TestInterpreterProcess::testExecJumpCatch()
{
    // A command sequence where the exception is caught
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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 3U);
        TS_ASSERT_EQUALS(toInteger(env), 93);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toString(env), "91");
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 10);

        TS_ASSERT_EQUALS(env.proc.getExceptionHandlers().size(), 0U);
    }

    // A command sequence where no exception happens
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 10);               // 0 - 10
        bco->addInstruction(Opcode::maJump, Opcode::jCatch, 3);                  // 1
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 90);               // 2 - 10:90

        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 90);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 10);

        TS_ASSERT_EQUALS(env.proc.getExceptionHandlers().size(), 0U);
    }
}

/** Test instruction: jdz. */
void
TestInterpreterProcess::testExecJumpDecZero()
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

    // Integer 0
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(0));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 100);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), -3);
    }

    // Integer 2
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 200);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 0);
    }

    // Float 3
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeFloatValue(3));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 300);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toFloat(env), 0.0);
    }

    // Float 2.5 never hits
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeFloatValue(2.5));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 100);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toFloat(env), -0.5);
    }

    // Null fails
    {
        Environment env;
        env.proc.pushNewValue(0);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // String fails
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("x"));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: callind/procind. */
void
TestInterpreterProcess::testExecIndirectCall()
{
    // callind 1 => 1:null -> empty
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT(isNull(env));
    }

    // procind 1 => 1:null -> empty (null can be called as function, not as procedure)
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // procind 1 => 1:"foo" -> error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeStringValue("foo"));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // callind 1 => 1:Callable -> empty
    {
        int callCount = 0;
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT(isNull(env));
        TS_ASSERT_EQUALS(callCount, 1);
    }

    // check refuse procedures branch
    {
        int callCount = 0;
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseProcedures, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // check refuse functions branch
    {
        int callCount = 0;
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(new SimpleCallable("v", false, callCount));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMCall | Opcode::miIMRefuseFunctions, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: loadind. */
void
TestInterpreterProcess::testExecIndirectLoad()
{
    // loadind 1 => 1:null -> null
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 1U);
        TS_ASSERT(isNull(env));
    }

    // loadind 1 => 1:"foo" -> error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeStringValue("foo"));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // loadind 1 => 1:Callable -> empty
    {
        int callCount = 0;
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(new SimpleCallable("v", true, callCount));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "v");
        TS_ASSERT_EQUALS(callCount, 1);
    }
}

/** Test instruction: storeind. */
void
TestInterpreterProcess::testExecIndirectStore()
{
    // storeind 2 => 1:2:"new":Callable -> "new"
    {
        String_t value("old");
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeStringValue("new"));
        env.proc.pushNewValue(new SimpleIndexable(value, 2));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMStore, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 1U);
        TS_ASSERT_EQUALS(toString(env), "new");
        TS_ASSERT_EQUALS(value, "new");
    }

    // storeind 1 => 1:2:3 -> error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMStore, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: popind. */
void
TestInterpreterProcess::testExecIndirectPop()
{
    // storeind 2 => 1:2:"new":Callable -> empty
    {
        String_t value("old");
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeStringValue("new"));
        env.proc.pushNewValue(new SimpleIndexable(value, 2));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMPop, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(value, "new");
    }

    // popind 1 => 1:2:3 -> error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maIndirect, Opcode::miIMPop, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: dup. */
void
TestInterpreterProcess::testExecStackDup()
{
    // Good case: dup 1 => 1:2:3 -> 1:2:3:1
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maStack, Opcode::miStackDup, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 4U);
        TS_ASSERT_EQUALS(toInteger(env), 2);   // the new value
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 3);   // previous value
    }

    // Bad case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maStack, Opcode::miStackDup, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: drop. */
void
TestInterpreterProcess::testExecStackDrop()
{
    // Good case: drop 2 => 1:2:3 -> 1
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maStack, Opcode::miStackDrop, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 1U);
        TS_ASSERT_EQUALS(toInteger(env), 1);
    }

    // Bad case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maStack, Opcode::miStackDrop, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: swap. */
void
TestInterpreterProcess::testExecStackSwap()
{
    // Good case: swap 1 => 1:2:3 -> 1:3:2
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maStack, Opcode::miStackSwap, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 3U);
        TS_ASSERT_EQUALS(toInteger(env), 2);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 3);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 1);
    }

    // Bad case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maStack, Opcode::miStackSwap, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: storevar. */
void
TestInterpreterProcess::testExecStoreNamedVariable()
{
    // Execute a single standalone 'storevar' instruction, good case
    {
        Environment env;
        String_t value("theValue");
        env.proc.pushNewContext(new SingularVariableContext("VALUE", value));
        env.proc.pushNewContext(new SingularObjectContext(0));
        env.proc.pushNewValue(interpreter::makeIntegerValue(17));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("VALUE"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 17);    // original value
        TS_ASSERT_EQUALS(value, "17");           // stringified by SingularVariableContext
    }

    // Bad case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(17));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maStore, Opcode::sNamedVariable, bco->addName("UNKNOWN_VALUE"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: storeloc. */
void
TestInterpreterProcess::testExecStoreLocal()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toInteger(env), 21);
}

/** Test instruction: storetop. */
void
TestInterpreterProcess::testExecStoreStatic()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toInteger(env), 12);
}

/** Test instruction: storeglob. */
void
TestInterpreterProcess::testExecStoreShared()
{
    // Set a global value. Execute single 'storeglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    runInstruction(env, Opcode::maStore, Opcode::sShared, 99);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "nv");
    TS_ASSERT_EQUALS(interpreter::toString(env.world.globalValues().get(99), false), "nv");
}

/** Test instruction: storegvar. */
void
TestInterpreterProcess::testExecStoreNamedShared()
{
    // Set a global value by name. Execute single 'storegvar' instruction.
    {
        Environment env;
        env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maStore, Opcode::sNamedShared, bco->addName("GV"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "nv");
        TS_ASSERT_EQUALS(interpreter::toString(env.world.getGlobalValue("GV"), false), "nv");
    }

    // Error case
    {
        Environment env;

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maStore, Opcode::sNamedShared, bco->addName("XXXXX"));
        env.proc.pushNewValue(0);
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: popvar. */
void
TestInterpreterProcess::testExecPopNamedVariable()
{
    // Execute a single standalone 'popvar' instruction, good case
    {
        Environment env;
        String_t value("theValue");
        env.proc.pushNewContext(new SingularVariableContext("VALUE", value));
        env.proc.pushNewContext(new SingularObjectContext(0));
        env.proc.pushNewValue(interpreter::makeIntegerValue(17));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPop, Opcode::sNamedVariable, bco->addName("VALUE"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(value, "17");           // stringified by SingularVariableContext
    }

    // Bad case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(17));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPop, Opcode::sNamedVariable, bco->addName("UNKNOWN_VALUE"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: poploc. */
void
TestInterpreterProcess::testExecPopLocal()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toInteger(env), 16);
}

/** Test instruction: poptop. */
void
TestInterpreterProcess::testExecPopStatic()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toInteger(env), 12);
}

/** Test instruction: popglob. */
void
TestInterpreterProcess::testExecPopShared()
{
    // Set a global value. Execute single 'popglob' instruction.
    Environment env;
    env.world.globalValues().setNew(99, interpreter::makeStringValue("v"));
    env.proc.pushNewValue(interpreter::makeStringValue("nv"));
    runInstruction(env, Opcode::maPop, Opcode::sShared, 99);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    TS_ASSERT_EQUALS(interpreter::toString(env.world.globalValues().get(99), false), "nv");
}

/** Test instruction: popgvar. */
void
TestInterpreterProcess::testExecPopNamedShared()
{
    // Set a global value by name. Execute single 'popgvar' instruction.
    {
        Environment env;
        env.world.globalValues().setNew(env.world.globalPropertyNames().add("GV"), interpreter::makeStringValue("q"));
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPop, Opcode::sNamedShared, bco->addName("GV"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(interpreter::toString(env.world.getGlobalValue("GV"), false), "nv");
    }

    // Error case
    {
        Environment env;

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPop, Opcode::sNamedShared, bco->addName("XXXXX"));
        env.proc.pushNewValue(0);
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: loadmem. */
void
TestInterpreterProcess::testExecMemrefLoad()
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco->addName("V"));

    // Good case
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(new SingularVariableContext("V", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "v");
    }

    // Null case
    {
        Environment env;
        env.proc.pushNewValue(0);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }

    // Error case: unknown name
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(new SingularVariableContext("OTHER", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(77));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: callmem. */
void
TestInterpreterProcess::testExecMemrefCall()
{
    // Note that this instruction is pretty useless;
    // it effectively only probes accessability of a variable but does not produce a stack result.
    // It only exists for symmetry with (maIndirect,miIMCall).
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMCall, bco->addName("V"));

    // Good case
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(new SingularVariableContext("V", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Null case
    {
        Environment env;
        env.proc.pushNewValue(0);
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Error case: unknown name
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(new SingularVariableContext("OTHER", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(77));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: popmem. */
void
TestInterpreterProcess::testExecMemrefPop()
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMPop, bco->addName("V"));

    // Good case
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(new SingularVariableContext("V", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(value, "nv");
    }

    // Bad case: unknown name
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(new SingularVariableContext("OTHER", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
        TS_ASSERT_EQUALS(value, "v");
    }

    // Bad case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: storemem. */
void
TestInterpreterProcess::testExecMemrefStore()
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maMemref, Opcode::miIMStore, bco->addName("V"));

    // Good case
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(new SingularVariableContext("V", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "nv");
        TS_ASSERT_EQUALS(value, "nv");
    }

    // Bad case: unknown name
    {
        Environment env;
        String_t value("v");
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(new SingularVariableContext("OTHER", value));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
        TS_ASSERT_EQUALS(value, "v");
    }

    // Bad case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("nv"));
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: dimloc. */
void
TestInterpreterProcess::testExecDimLocal()
{
    // New variable being created.
    // We cannot directly observe the local variable frame, so create the variable and read it back.
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
        bco->addInstruction(Opcode::maDim, Opcode::sLocal, bco->addName("LV"));
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("LV"));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 7);
    }

    // Variable already exists.
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addLocalVariable("LV");
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
        bco->addInstruction(Opcode::maDim, Opcode::sLocal, bco->addName("LV"));
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("LV"));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }
}

/** Test instruction: dimtop. */
void
TestInterpreterProcess::testExecDimStatic()
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
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toInteger(env), 7);
}

/** Test instruction: dimglob. */
void
TestInterpreterProcess::testExecDimShared()
{
    // We can directly test the effect of "dimglob".
    // In fact, the indirect test (create in inner, read in outer using pushvar) would require a GlobalContext we don't have here.
    Environment env;
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 7);
    bco->addInstruction(Opcode::maDim, Opcode::sShared, bco->addName("GV"));
    runBCO(env, bco);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);

    afl::data::NameMap::Index_t index = env.world.globalPropertyNames().getIndexByName("GV");
    TS_ASSERT(index != afl::data::NameMap::nil);

    const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(env.world.globalValues().get(index));
    TS_ASSERT(iv);
    TS_ASSERT_EQUALS(iv->getValue(), 7);
}

/** Test instruction: suncatch. */
void
TestInterpreterProcess::testExecUncatch()
{
    // Execute a sequence consisting of catch and uncatch.
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
        env.proc.pushNewContext(new SingularVariableContext("VAR", value));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
        TS_ASSERT_EQUALS(value, "-1");
    }

    // Error case: uncatch without previous catch
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sreturn. */
void
TestInterpreterProcess::testExecReturn()
{
    // This instruction is essentially equivalent to a jump-to-end.
    // The parameter is pretty pointless; result transfer is handled by
    // matching the frame's wantResult and the BCO's isProcedure.
    // Test it just for completeness.

    // Good case
    {
        Environment env;
        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, 2);
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 1);
    }

    // Good case 2
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialReturn, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    }

    // Bad case: stack violation
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialReturn, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: swith. */
void
TestInterpreterProcess::testExecWith()
{
    // Good case
    {
        String_t value("v");
        SingularVariableContext ctx("VAR", value);
        ctx.makeClonable();

        Environment env;
        BCORef_t bco = makeBCO();
        bco->addPushLiteral(&ctx);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VAR"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "v");
    }

    // Bad case: no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialWith, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Bad case: wrong type
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialWith, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sendwith. */
void
TestInterpreterProcess::testExecEndWith()
{
    // Good case
    {
        Environment env;
        String_t outerValue("ov");
        env.proc.pushNewContext(new SingularVariableContext("VAR", outerValue));

        String_t innerValue("iv");
        SingularVariableContext innerContext("VAR", innerValue);
        innerContext.makeClonable();

        BCORef_t bco = makeBCO();
        bco->addPushLiteral(&innerContext);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("VAR"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "ov");
    }

    // Bad case: no context
    {
        Environment env;
        BCORef_t bco = makeBCO();
        for (int i = 0; i < 10; ++i) {
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        }
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sfirstindex. */
void
TestInterpreterProcess::testExecFirstIndex()
{
    // Good case: non-empty iterable. Pushes true and activates context; proven with pushvar
    {
        Hash::Ref_t hash = Hash::create();
        hash->setNew("kk", interpreter::makeIntegerValue(1));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("KEY"));

        Environment env;
        env.proc.pushNewValue(new HashValue(hash));
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "kk");      // result of the pushvar
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toBoolean(env), true);     // result of the sfirstindex
    }

    // Good case: empty iterable. Pushes null and does not modify stack.
    {
        Environment env;
        size_t n = env.proc.getContexts().size();
        env.proc.pushNewValue(new HashValue(Hash::create()));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
        TS_ASSERT_EQUALS(env.proc.getContexts().size(), n);
    }

    // Bad case: not iterable
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: snextindex. */
void
TestInterpreterProcess::testExecNextIndex()
{
    // Good case: unit iterable.
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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getContexts().size(), n);
        TS_ASSERT(isNull(env));                     // result of the snextindex
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toBoolean(env), true);     // result of the sfirstindex
    }

    // Good case: multiple entry iterable.
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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "b2");      // result of the pushvar
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toBoolean(env), true);     // result of the snextindex
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toBoolean(env), true);     // result of the sfirstindex
    }

    // Bad case: no context
    {
        Environment env;
        BCORef_t bco = makeBCO();
        for (int i = 0; i < 10; ++i) {
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
        }
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sendindex. */
void
TestInterpreterProcess::testExecEndIndex()
{
    // Good case
    {
        Hash::Ref_t hash = Hash::create();
        hash->setNew("kk", interpreter::makeIntegerValue(1));

        Environment env;
        String_t outerValue("ov");
        env.proc.pushNewContext(new SingularVariableContext("KEY", outerValue));
        env.proc.pushNewValue(new HashValue(hash));

        BCORef_t bco = makeBCO();
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
        bco->addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco->addName("KEY"));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "ov");
    }

    // Bad case: no context
    {
        Environment env;
        BCORef_t bco = makeBCO();
        for (int i = 0; i < 10; ++i) {
            bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
        }
        runBCO(env, bco);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sevals. */
void
TestInterpreterProcess::testExecEvalStatement()
{
    // Good case - single line
    {
        String_t value("a");
        Environment env;
        env.proc.pushNewContext(new SingularVariableContext("VAR", value));
        env.proc.pushNewValue(interpreter::makeStringValue("var := 'b'"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getValueStack().size(), 0U);
        TS_ASSERT_EQUALS(value, "b");
    }

    // Good case - multiple lines
    {
        String_t value("a");
        Environment env;
        env.proc.pushNewContext(new SingularVariableContext("VAR", value));
        env.proc.pushNewValue(interpreter::makeStringValue("if var='a'"));
        env.proc.pushNewValue(interpreter::makeStringValue("  var := 'c'"));
        env.proc.pushNewValue(interpreter::makeStringValue("endif"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 3);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getValueStack().size(), 0U);
        TS_ASSERT_EQUALS(value, "c");
    }

    // Bad case - single line syntax error
    {
        String_t value("a");
        Environment env;
        env.proc.pushNewContext(new SingularVariableContext("VAR", value));
        env.proc.pushNewValue(interpreter::makeStringValue("if var='a'"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Bad case - stack error
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sevalx. */
void
TestInterpreterProcess::testExecEvalExpression()
{
    // Good case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("47+11"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 58);
    }

    // Null
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }

    // Bad case - parse error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("47)"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Bad case - stack error
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sdefsub. */
void
TestInterpreterProcess::testExecDefSub()
{
    // Note that this opcode is expected to deal with SubroutineValue's only, so we test it with them only.
    // In fact it currently works with every type.
    // Good case - new sub
    {
        BCORef_t subjectBCO = makeBCO();
        subjectBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefSub, subjectBCO->addName("SUBN"));

        // Execute first sdefsub instruction
        Environment env;
        BCORef_t firstBCO = makeBCO();
        env.proc.pushNewValue(new SubroutineValue(firstBCO));
        runBCO(env, subjectBCO);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        // Verify results
        SubroutineValue* subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("SUBN")));
        TS_ASSERT(subv);
        TS_ASSERT_EQUALS(&*subv->getBytecodeObject(), &*firstBCO);

        // Execute second sdefsub instruction to overwrite result
        BCORef_t secondBCO = makeBCO();
        env.proc.pushNewValue(new SubroutineValue(secondBCO));
        runBCO(env, subjectBCO);

        // Verify results
        subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("SUBN")));
        TS_ASSERT(subv);
        TS_ASSERT_EQUALS(&*subv->getBytecodeObject(), &*secondBCO);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    }

    // Error case - no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialDefSub, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sdefshipp. */
void
TestInterpreterProcess::testExecDefShipProperty()
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefShipProperty, bco->addName("PROP"));

    Environment env;
    runBCO(env, bco);
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT(env.world.shipPropertyNames().getIndexByName("PROP") != afl::data::NameMap::nil);
}

/** Test instruction: sdefplanetp. */
void
TestInterpreterProcess::testExecDefPlanetProperty()
{
    BCORef_t bco = makeBCO();
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialDefPlanetProperty, bco->addName("PROP"));

    Environment env;
    runBCO(env, bco);
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT(env.world.planetPropertyNames().getIndexByName("PROP") != afl::data::NameMap::nil);
}

/** Test instruction: sload. */
void
TestInterpreterProcess::testExecLoad()
{
    // Good case: file found. Define a subroutine and check that it got defined.
    {
        static const char CODE[] = "sub loaded_sub\nendsub\n";
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
        afl::base::Ref<afl::io::Stream> file = *new afl::io::ConstMemoryStream(afl::string::toBytes(CODE));
        dir->addStream("loaded.q", file);

        Environment env;
        env.world.setSystemLoadDirectory(dir.asPtr());
        env.proc.pushNewValue(interpreter::makeStringValue("loaded.q"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        SubroutineValue* subv = dynamic_cast<SubroutineValue*>(env.world.globalValues().get(env.world.globalPropertyNames().getIndexByName("LOADED_SUB")));
        TS_ASSERT(subv);
        TS_ASSERT_EQUALS(subv->getBytecodeObject()->getFileName(), file->getName());
    }

    // Error: file found, but has syntax error.
    {
        static const char CODE[] = "1+";
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
        dir->addStream("loaded.q", *new afl::io::ConstMemoryStream(afl::string::toBytes(CODE)));

        Environment env;
        env.world.setSystemLoadDirectory(dir.asPtr());
        env.proc.pushNewValue(interpreter::makeStringValue("loaded.q"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // OK'ish case: file not found
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("non.existant.q"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(!isNull(env));
    }

    // Null case
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }

    // Error case: no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sprint. */
void
TestInterpreterProcess::testExecPrint()
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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(normalCount >= 1);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Null case: no message generated, so one message less than before.
    {
        afl::test::LogListener log;
        Environment env;
        env.log.addListener(log);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialPrint, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(log.getNumMessages(), normalCount-1U);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }
}

/** Test instruction: saddhook. */
void
TestInterpreterProcess::testExecAddHook()
{
    // Good case: add two entries to a hook
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);

        // Placing the hooks in global properties is not contractual,
        // but has been used since PCC1, so let's assume it stays for a while.
        // (It is never reflected in file formats, though.)
        TS_ASSERT(env.world.globalPropertyNames().getIndexByName("ON HN") != afl::data::NameMap::nil);
    }

    // Null case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("HN"));
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(env.world.globalPropertyNames().getIndexByName("ON HN"), afl::data::NameMap::nil);
    }

    // Error case: addend is not a subroutine
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("HN"));
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: hook is not a subroutine (cannot normally happen)
    {
        Environment env;
        env.world.globalValues().setNew(env.world.globalPropertyNames().add("ON HN"), interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeStringValue("HN"));
        env.proc.pushNewValue(new SubroutineValue(makeBCO()));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 0);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: invalid opcode (reserved for hooks with args)
    {
        Environment env;
        env.proc.pushNewValue(0);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialAddHook, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: srunhook. */
void
TestInterpreterProcess::testExecRunHook()
{
    // Good case: this does
    //     On HN Do gv:=42
    //     RunHook HN
    // and examines that 'gv:=42' has been executed.
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
        env.proc.pushNewContext(new SingularVariableContext("GV", value));

        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
        TS_ASSERT_EQUALS(value, "42");
    }

    // OK'ish case: null hook content
    {
        Environment env;
        env.world.globalValues().setNew(env.world.globalPropertyNames().add("ON HN"), 0);
        env.proc.pushNewValue(interpreter::makeStringValue("HN"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Null value
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Error case: invalid opcode (reserved for hooks with args)
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialRunHook, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sthrow. */
void
TestInterpreterProcess::testExecThrow()
{
    // Normal case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeStringValue("oops"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT_EQUALS(String_t(env.proc.getError().what()), "oops");
    }

    // Normal case with nonlocal error handler
    //    outer installs exception handler, calls inner
    //    inner produces exception
    // This therefore also tests 'catch'.
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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 2U);
        TS_ASSERT_EQUALS(toInteger(env), 5555);           // value pushed by catch handler
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toString(env), "8888");          // thrown value, stringified by throwing
    }

    // OK'ish case: null
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack (still fails)
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sterminate. */
void
TestInterpreterProcess::testExecTerminate()
{
    // Execute instruction. This should not affect the stack.
    const size_t N = 8;
    Environment env;
    for (size_t i = 0; i < N; ++i) {
        env.proc.pushNewValue(0);
    }
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialTerminate, 0);
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(env.proc.getStackSize(), N);
}

/** Test instruction: ssuspend. */
void
TestInterpreterProcess::testExecSuspend()
{
    Environment env;
    runInstruction(env, Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    TS_ASSERT_EQUALS(env.proc.getState(), Process::Suspended);
}


/** Test instruction: snewarray. */
void
TestInterpreterProcess::testExecNewArray()
{
    // Normal case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        const ArrayValue* a = dynamic_cast<const ArrayValue*>(env.proc.getResult());
        TS_ASSERT(a);
        TS_ASSERT_EQUALS(a->getData()->getNumDimensions(), 2U);
        TS_ASSERT_EQUALS(a->getData()->getDimension(0), 3U);
        TS_ASSERT_EQUALS(a->getData()->getDimension(1), 4U);
    }

    // Error case: wrong type
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeStringValue("X"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: null dimension
    {
        Environment env;
        env.proc.pushNewValue(0);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 3);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: 0 dimensions
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewArray, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: smakelist. */
void
TestInterpreterProcess::testExecMakeList()
{
    // Normal case
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(0);
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialMakeList, 4);

        // Result type
        const ArrayValue* a = dynamic_cast<const ArrayValue*>(env.proc.getResult());
        TS_ASSERT(a);
        TS_ASSERT_EQUALS(a->getData()->getNumDimensions(), 1U);
        TS_ASSERT_EQUALS(a->getData()->getDimension(0), 4U);

        // Array content
        TS_ASSERT_EQUALS(interpreter::toString(a->getData()->content().get(0), false), "1");
        TS_ASSERT_EQUALS(interpreter::toString(a->getData()->content().get(1), false), "2");
        TS_ASSERT (a->getData()->content().get(2) == 0);
        TS_ASSERT_EQUALS(interpreter::toString(a->getData()->content().get(3), false), "4");
    }

    // Error case: missing stack
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialMakeList, 3);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: snewhash. */
void
TestInterpreterProcess::testExecNewHash()
{
    // Normal case
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(dynamic_cast<const HashValue*>(env.proc.getResult()) != 0);
    }

    // Error case: invalid opcode
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNewHash, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sinstance. */
void
TestInterpreterProcess::testExecInstance()
{
    // Normal case
    {
        StructureTypeData::Ref_t type = *new StructureTypeData();
        Environment env;
        env.proc.pushNewValue(new StructureType(type));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        const StructureValue* sv = dynamic_cast<const StructureValue*>(env.proc.getResult());
        TS_ASSERT(sv);
        TS_ASSERT_EQUALS(&*sv->getValue()->type, &*type);
    }

    // Error case: wrong type
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialInstance, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sresizearray. */
void
TestInterpreterProcess::testExecResizeArray()
{
    // Normal case
    {
        afl::base::Ref<ArrayData> ad(make2DArray());
        Environment env;
        env.proc.pushNewValue(new ArrayValue(ad));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);

        TS_ASSERT_EQUALS(ad->getNumDimensions(), 2U);
        TS_ASSERT_EQUALS(ad->getDimension(0), 3U);
        TS_ASSERT_EQUALS(ad->getDimension(1), 4U);
    }

    // Error case: wrong dimension type
    {
        Environment env;
        env.proc.pushNewValue(new ArrayValue(make2DArray()));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        env.proc.pushNewValue(interpreter::makeStringValue("X"));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: wrong array type
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        env.proc.pushNewValue(interpreter::makeIntegerValue(3));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: null dimension
    {
        Environment env;
        env.proc.pushNewValue(new ArrayValue(make2DArray()));
        env.proc.pushNewValue(0);
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: missing stack
    {
        Environment env;
        env.proc.pushNewValue(new ArrayValue(make2DArray()));
        env.proc.pushNewValue(interpreter::makeIntegerValue(4));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 2);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: incompatible dimensions
    {
        Environment env;
        env.proc.pushNewValue(new ArrayValue(make2DArray()));
        env.proc.pushNewValue(interpreter::makeIntegerValue(7));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 1);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: no dimension
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialResizeArray, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sbind. */
void
TestInterpreterProcess::testExecBind()
{
    // Normal case: test the entire feature: outerBCO binds innerBCO and calls it
    // For simplicity, this runs the 'bind' in a separate process.
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
        TS_ASSERT_EQUALS(firstProcess.getState(), Process::Ended);
        TS_ASSERT(dynamic_cast<const interpreter::CallableValue*>(firstProcess.getResult()) != 0);

        // Execute new callable in regular Environment process for easier evaluation
        env.proc.pushNewValue(interpreter::makeStringValue("B"));
        env.proc.pushNewValue(firstProcess.getResult()->clone());
        runInstruction(env, Opcode::maIndirect, Opcode::miIMLoad, 1);

        // Result must be valid
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toString(env), "AB");
    }

    // Error case: null callable
    {
        Environment env;
        env.proc.pushNewValue(0);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialBind, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialBind, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: sfirst. */
void
TestInterpreterProcess::testExecFirst()
{
    // Good case: non-empty iterable.
    {
        Hash::Ref_t hash = Hash::create();
        hash->setNew("kk", interpreter::makeIntegerValue(1));

        Environment env;
        env.proc.pushNewValue(new HashValue(hash));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        // Result must be context
        const interpreter::Context* ctx = dynamic_cast<const interpreter::Context*>(env.proc.getResult());
        TS_ASSERT(ctx != 0);

        // Result must have correct key
        interpreter::Context::PropertyIndex_t idx;
        TS_ASSERT(const_cast<interpreter::Context*>(ctx)->lookup("KEY", idx));
    }

    // Good case: empty iterable.
    {
        Environment env;
        env.proc.pushNewValue(new HashValue(Hash::create()));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }

    // Error case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialFirst, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: snext. */
void
TestInterpreterProcess::testExecNext()
{
    // Good case: non-empty iterable.
    {
        // Create hash and point iterator at first element
        Hash::Ref_t hash = Hash::create();
        hash->setNew("k1", interpreter::makeIntegerValue(1));
        hash->setNew("k2", interpreter::makeIntegerValue(2));
        interpreter::Context* iter = HashValue(hash).makeFirstContext();
        TS_ASSERT(iter);

        Environment env;
        env.proc.pushNewValue(iter);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);

        // Result must still be context
        const interpreter::Context* ctx = dynamic_cast<const interpreter::Context*>(env.proc.getResult());
        TS_ASSERT(ctx != 0);

        // Result must have correct key
        interpreter::Context::PropertyIndex_t idx;
        TS_ASSERT(const_cast<interpreter::Context*>(ctx)->lookup("KEY", idx));
    }

    // Good case: final element of iterable.
    {
        // Create hash and point iterator at first element
        Hash::Ref_t hash = Hash::create();
        hash->setNew("kk", interpreter::makeIntegerValue(1));
        interpreter::Context* iter = HashValue(hash).makeFirstContext();
        TS_ASSERT(iter);

        Environment env;
        env.proc.pushNewValue(iter);
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }

    // Exercise use of independant copies
    {
        Environment env;
        env.proc.pushNewValue(new CountingContext("COUNT", 10));

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
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 4U);
        TS_ASSERT_EQUALS(toInteger(env), 12);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 11);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 10);
        env.proc.dropValue();
        TS_ASSERT_EQUALS(toInteger(env), 13);;
    }

    // Error case: type error
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }

    // Error case: no stack
    {
        Environment env;
        runInstruction(env, Opcode::maSpecial, Opcode::miSpecialNext, 0);
        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        TS_ASSERT(isError(env));
    }
}

/** Test instruction: fused unary (push + unary). */
void
TestInterpreterProcess::testExecFusedUnary()
{
    Environment env;
    BCORef_t bco = makeBCO();
    env.world.globalValues().setNew(77, interpreter::makeIntegerValue(1337));
    bco->addInstruction(Opcode::maFusedUnary, Opcode::sShared, 77);
    bco->addInstruction(Opcode::maUnary, interpreter::unStr, 0);
    runBCO(env, bco);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "1337");
}

/** Test instruction: fused binary (push + binary). */
void
TestInterpreterProcess::testExecFusedBinary()
{
    Environment env;
    BCORef_t bco = makeBCO();
    env.world.globalValues().setNew(77, interpreter::makeStringValue("a"));    // second arg
    env.proc.pushNewValue(interpreter::makeStringValue("b"));                  // first arg
    bco->addInstruction(Opcode::maFusedBinary, Opcode::sShared, 77);
    bco->addInstruction(Opcode::maBinary, interpreter::biConcat, 0);
    runBCO(env, bco);

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(env), "ba");
}

/** Test instruction: fused comparison (bcmp + j). */
void
TestInterpreterProcess::testExecFusedComparison()
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Not taken jump
    {
        Environment env;
        env.proc.pushNewValue(interpreter::makeIntegerValue(1));
        env.proc.pushNewValue(interpreter::makeIntegerValue(2));
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 1U);
        TS_ASSERT_EQUALS(toInteger(env), 42);
    }
}

/** Test instruction: fused comparison (push + bcmp + j). */
void
TestInterpreterProcess::testExecFusedComparison2()
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 0U);
    }

    // Not taken jump
    {
        Environment env;
        env.world.globalValues().setNew(55, interpreter::makeIntegerValue(10)); // second arg
        env.proc.pushNewValue(interpreter::makeIntegerValue(100));              // first arg
        runBCO(env, bco);

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(env.proc.getStackSize(), 1U);
        TS_ASSERT_EQUALS(toInteger(env), 42);
    }
}

/** Test instruction: in-place unary (pushloc + uinc/udec). */
void
TestInterpreterProcess::testExecInplaceUnary()
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 5);
    }

    // Decement float
    {
        Environment env;
        Process::Frame& frame = env.proc.pushFrame(decBCO, true);
        frame.localValues.setNew(12, interpreter::makeFloatValue(2.5));
        env.proc.run();

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toFloat(env), 1.5);
    }

    // Increment bool (value not optimized, type change)
    {
        Environment env;
        Process::Frame& frame = env.proc.pushFrame(incBCO, true);
        frame.localValues.setNew(12, interpreter::makeBooleanValue(1));
        env.proc.run();

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT_EQUALS(toInteger(env), 2);
    }

    // Decrement null (value not optimized and handled normally, but doesn't generate an error)
    {
        Environment env;
        /*Process::Frame& frame =*/ env.proc.pushFrame(decBCO, true);
        env.proc.run();

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
        TS_ASSERT(isNull(env));
    }
}

/** Test onContextEntered(), onContextLeft(). */
void
TestInterpreterProcess::testContextEnter()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context is left abnormally. */
void
TestInterpreterProcess::testContextEnterError()
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        // Context will be destroyed here
    }

    TS_ASSERT_EQUALS(trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context is left abnormally, but error is caught. */
void
TestInterpreterProcess::testContextEnterCatch()
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

    TS_ASSERT_EQUALS(env.proc.getState(), Process::Ended);
    TS_ASSERT_EQUALS(trace, "(enter)(leave)");
}

/** Test onContextEntered(), onContextLeft() when context rejects entering.
    In this case, the leave callback must not be called. */
void
TestInterpreterProcess::testContextEnterReject()
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

        TS_ASSERT_EQUALS(env.proc.getState(), Process::Failed);
        // Context will be destroyed here
    }

    TS_ASSERT_EQUALS(trace, "(enter)");
}

