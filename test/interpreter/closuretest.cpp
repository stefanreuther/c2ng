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
        virtual size_t getDimension(size_t which) const
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
AFL_TEST("interpreter.Closure:basics", a)
{
    // ex IntCompoundTestSuite::testClosure
    // Create a test callable and make sure it works (yes, this leaks on failure)
    MyCallable* base = new MyCallable();
    a.checkEqual("01. getDimension", base->getDimension(0), 7U);
    a.checkEqual("02. getDimension", base->getDimension(1), 5U);
    a.checkEqual("03. getDimension", base->getDimension(7), 35U);
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
    a.checkEqual("22. getDimension", c->getDimension(0), 7U);
    a.checkEqual("23. getDimension", c->getDimension(1), 5U);
    a.checkEqual("24. getDimension", c->getDimension(7), 35U);

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
    a.checkEqual("61. getDimension", c->getDimension(0), 5U);
    a.checkEqual("62. getDimension", c->getDimension(1), 15U);
    a.checkEqual("63. getDimension", c->getDimension(5), 35U);

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
    a.checkEqual("81. getDimension", c->getDimension(0), 3U);
    a.checkEqual("82. getDimension", c->getDimension(1), 25U);
    a.checkEqual("83. getDimension", c->getDimension(3), 35U);

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

/** Test closure: most dimensions bound. */
AFL_TEST("interpreter.Closure:most-dimensions-bound", a)
{
    std::auto_ptr<interpreter::Closure> c(new interpreter::Closure());
    c->setNewFunction(new MyCallable());
    for (int i = 0; i < 5; ++i) {
        c->addNewArgument(interpreter::makeIntegerValue(i));
    }

    a.checkEqual("01", c->getDimension(0), 2U);
    a.checkEqual("02", c->getDimension(1), 30U);
    a.checkEqual("03", c->getDimension(2), 35U);
    a.checkEqual("04", c->getDimension(3), 0U);
}

/** Test closure: all dimensions bound. */
AFL_TEST("interpreter.Closure:all-dimensions-bound", a)
{
    std::auto_ptr<interpreter::Closure> c(new interpreter::Closure());
    c->setNewFunction(new MyCallable());
    for (int i = 0; i < 7; ++i) {
        c->addNewArgument(interpreter::makeIntegerValue(i));
    }

    a.checkEqual("01", c->getDimension(0), 0U);
    a.checkEqual("02", c->getDimension(1), 0U);
}
