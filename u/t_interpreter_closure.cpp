/**
  *  \file u/t_interpreter_closure.cpp
  *  \brief Test for interpreter::Closure
  */

#include <memory>
#include "interpreter/closure.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "interpreter/world.hpp"
#include "util/consolelogger.hpp"

namespace {
    class MyCallable : public interpreter::CallableValue {
     public:
        MyCallable()
            { ++num_instances; }
        ~MyCallable()
            { --num_instances; }

        // State inquiry
        String_t getState()
            { return m_state; }
        void clear()
            { m_state.clear(); }

        // IntCallableValue:
        virtual void call(interpreter::Process& /*exc*/, afl::data::Segment& args, bool want_result)
            {
                // Fold all arguments into a string
                for (size_t i = 0; i < args.size(); ++i) {
                    m_state += interpreter::toString(args[i], true);
                    m_state += ',';
                }
                m_state += want_result ? "y" : "n";
            }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int32_t getDimension(int32_t which) const
            {
                if (which == 0) {
                    return 7;
                } else {
                    return 5*which;
                }
            }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }

        // IntValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<MyCallable>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw interpreter::Error::notSerializable(); }
        virtual MyCallable* clone() const
            { return new MyCallable(); }

        static int num_instances;

     private:
        String_t m_state;
    };
    int MyCallable::num_instances;
}

/** Test all closure methods. */
void
TestInterpreterClosure::testClosure()
{
    // ex IntCompoundTestSuite::testClosure
    // Create a test callable and make sure it works (yes, this leaks on failure)
    MyCallable* base = new MyCallable();
    TS_ASSERT_EQUALS(base->getDimension(0), 7);
    TS_ASSERT_EQUALS(base->getDimension(1), 5);
    TS_ASSERT_EQUALS(base->getDimension(7), 35);
    TS_ASSERT_EQUALS(MyCallable::num_instances, 1);

    // Try cloning
    {
        std::auto_ptr<afl::data::Value> copy(base->clone());
        TS_ASSERT_DIFFERS(base, copy.get());
        TS_ASSERT_EQUALS(MyCallable::num_instances, 2);
        copy.reset();
        TS_ASSERT_EQUALS(MyCallable::num_instances, 1);
    }

    // Create a closure that binds no args and make sure it works
    std::auto_ptr<interpreter::Closure> c(new interpreter::Closure());
    c->setNewFunction(base);
    TS_ASSERT_EQUALS(MyCallable::num_instances, 1);
    TS_ASSERT_EQUALS(c->getDimension(0), 7);
    TS_ASSERT_EQUALS(c->getDimension(1), 5);
    TS_ASSERT_EQUALS(c->getDimension(7), 35);

    // Closure properties
    TS_ASSERT(!c->isProcedureCall());
    TS_ASSERT_THROWS(c->makeFirstContext(), interpreter::Error);
    TS_ASSERT_EQUALS(c->toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(c->toString(false), c->toString(true));
    {
        interpreter::TagNode out;
        afl::io::InternalSink aux;
        afl::charset::Utf8Charset cs;
        interpreter::vmio::NullSaveContext ctx;
        TS_ASSERT_THROWS(c->store(out, aux, cs, ctx), interpreter::Error);
    }

    // Clone the closure
    {
        std::auto_ptr<afl::data::Value> cc(c->clone());
        TS_ASSERT_EQUALS(MyCallable::num_instances, 1);
        TS_ASSERT_DIFFERS(cc.get(), c.get());
    }

    // Test call
    util::ConsoleLogger log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "dummy", 9);
    {
        afl::data::Segment dseg;
        dseg.pushBackNew(interpreter::makeIntegerValue(1));
        dseg.pushBackNew(interpreter::makeIntegerValue(9));
        dseg.pushBackNew(interpreter::makeIntegerValue(5));
        c->call(proc, dseg, true);
        TS_ASSERT_EQUALS(base->getState(), "1,9,5,y");
        base->clear();
    }

    // Bind some args
    c->addNewArgument(interpreter::makeIntegerValue(3));
    c->addNewArgument(interpreter::makeStringValue("zz"));
    TS_ASSERT_EQUALS(c->getDimension(0), 5);
    TS_ASSERT_EQUALS(c->getDimension(1), 15);
    TS_ASSERT_EQUALS(c->getDimension(5), 35);

    {
        afl::data::Segment dseg;
        dseg.pushBackNew(interpreter::makeIntegerValue(1));
        dseg.pushBackNew(interpreter::makeIntegerValue(9));
        dseg.pushBackNew(interpreter::makeIntegerValue(5));
        c->call(proc, dseg, true);
        TS_ASSERT_EQUALS(base->getState(), "3,\"zz\",1,9,5,y");
        base->clear();
    }

    // Bind some more args
    {
        afl::data::Segment a;
        a.pushBackNew(interpreter::makeIntegerValue(999));
        a.pushBackNew(interpreter::makeIntegerValue(42));
        a.pushBackNew(interpreter::makeBooleanValue(1));
        c->addNewArgumentsFrom(a, 2);
    }
    TS_ASSERT_EQUALS(c->getDimension(0), 3);
    TS_ASSERT_EQUALS(c->getDimension(1), 25);
    TS_ASSERT_EQUALS(c->getDimension(3), 35);

    {
        afl::data::Segment dseg;
        dseg.pushBackNew(interpreter::makeIntegerValue(1));
        dseg.pushBackNew(interpreter::makeIntegerValue(9));
        dseg.pushBackNew(interpreter::makeIntegerValue(5));
        c->call(proc, dseg, true);
        TS_ASSERT_EQUALS(base->getState(), "3,\"zz\",42,True,1,9,5,y");
        base->clear();
    }
}
