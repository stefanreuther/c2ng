/**
  *  \file test/interpreter/callablevaluetest.cpp
  *  \brief Test for interpreter::CallableValue
  */

#include "interpreter/callablevalue.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

/** Interface test. */
AFL_TEST("interpreter.CallableValue", a)
{
    class Tester : public interpreter::CallableValue {
     public:
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { }
        virtual bool isProcedureCall() const
            { return false; }
        virtual size_t getDimension(size_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return rejectFirstContext(); }
        virtual interpreter::CallableValue* clone() const
            { return 0; }
        virtual String_t toString(bool /*readable*/) const
            { return String_t(); }
        virtual void store(interpreter::TagNode&, afl::io::DataSink&, interpreter::SaveContext&) const
            { }
    };
    Tester t;

    // Test makeFirstContext() / rejectFirstContext():
    AFL_CHECK_THROWS(a("01. makeFirstContext"), t.makeFirstContext(), interpreter::Error);
}
