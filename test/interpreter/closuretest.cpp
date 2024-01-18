/**
  *  \file test/interpreter/closuretest.cpp
  *  \brief Test for interpreter::Closure
  */

#include "interpreter/closure.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "util/consolelogger.hpp"
#include <memory>

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
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual MyCallable* clone() const
            { return new MyCallable(); }

        static int num_instances;

     private:
        String_t m_state;
    };
    int MyCallable::num_instances;
}

/** Test all closure methods. */
AFL_TEST("interpreter.Closure", a)
{
    // ex IntCompoundTestSuite::testClosure
    // Create a test callable and make sure it works (yes, this leaks on failure)
    MyCallable* base = new MyCallable();
    a.checkEqual("01. getDimension", base->getDimension(0), 7);
    a.checkEqual("02. getDimension", base->getDimension(1), 5);
    a.checkEqual("03. getDimension", base->getDimension(7), 35);
    a.checkEqual("04. num_instances", MyCallable::num_instances, 1);

    // Try cloning
    {
        std::auto_ptr<afl::data::Value> copy(base->clone());
        a.checkDifferent("11. clone", base, copy.get());
        a.checkEqual("12. num_instances", MyCallable::num_instances, 2);
        copy.reset();
        a.checkEqual("13. num_instances", MyCallable::num_instances, 1);
    }

    // Create a closure that binds no args and make sure it works
    std::auto_ptr<interpreter::Closure> c(new interpreter::Closure());
    c->setNewFunction(base);
    a.checkEqual("21. num_instances", MyCallable::num_instances, 1);
    a.checkEqual("22. getDimension", c->getDimension(0), 7);
    a.checkEqual("23. getDimension", c->getDimension(1), 5);
    a.checkEqual("24. getDimension", c->getDimension(7), 35);

    // Closure properties
    a.check("31. isProcedureCall", !c->isProcedureCall());
    AFL_CHECK_THROWS(a("32. makeFirstContext"), c->makeFirstContext(), interpreter::Error);
    a.checkEqual("33. toString", c->toString(false).substr(0, 2), "#<");

    interpreter::test::ValueVerifier verif(*c, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Clone the closure
    {
        std::auto_ptr<afl::data::Value> cc(c->clone());
        a.checkEqual("41. num_instances", MyCallable::num_instances, 1);
        a.checkDifferent("42. get", cc.get(), c.get());
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
        a.checkEqual("51. getState", base->getState(), "1,9,5,y");
        base->clear();
    }

    // Bind some args
    c->addNewArgument(interpreter::makeIntegerValue(3));
    c->addNewArgument(interpreter::makeStringValue("zz"));
    a.checkEqual("61. getDimension", c->getDimension(0), 5);
    a.checkEqual("62. getDimension", c->getDimension(1), 15);
    a.checkEqual("63. getDimension", c->getDimension(5), 35);

    {
        afl::data::Segment dseg;
        dseg.pushBackNew(interpreter::makeIntegerValue(1));
        dseg.pushBackNew(interpreter::makeIntegerValue(9));
        dseg.pushBackNew(interpreter::makeIntegerValue(5));
        c->call(proc, dseg, true);
        a.checkEqual("71. getState", base->getState(), "3,\"zz\",1,9,5,y");
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
    a.checkEqual("81. getDimension", c->getDimension(0), 3);
    a.checkEqual("82. getDimension", c->getDimension(1), 25);
    a.checkEqual("83. getDimension", c->getDimension(3), 35);

    {
        afl::data::Segment dseg;
        dseg.pushBackNew(interpreter::makeIntegerValue(1));
        dseg.pushBackNew(interpreter::makeIntegerValue(9));
        dseg.pushBackNew(interpreter::makeIntegerValue(5));
        c->call(proc, dseg, true);
        a.checkEqual("91. getState", base->getState(), "3,\"zz\",42,True,1,9,5,y");
        base->clear();
    }
}
