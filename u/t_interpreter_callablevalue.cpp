/**
  *  \file u/t_interpreter_callablevalue.cpp
  *  \brief Test for interpreter::CallableValue
  */

#include "interpreter/callablevalue.hpp"

#include "t_interpreter.hpp"
#include "interpreter/error.hpp"

/** Interface test. */
void
TestInterpreterCallableValue::testInterface()
{
    class Tester : public interpreter::CallableValue {
     public:
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int32_t getDimension(int32_t /*which*/) const
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
    TS_ASSERT_THROWS(t.makeFirstContext(), interpreter::Error);
}

