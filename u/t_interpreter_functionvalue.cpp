/**
  *  \file u/t_interpreter_functionvalue.cpp
  *  \brief Test for interpreter::FunctionValue
  */

#include "interpreter/functionvalue.hpp"

#include "t_interpreter.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Interface test. */
void
TestInterpreterFunctionValue::testIt()
{
    class Tester : public interpreter::FunctionValue {
     public:
        virtual afl::data::Value* get(interpreter::Arguments& /*args*/)
            { return 0; }
        virtual CallableValue* clone() const
            { return new Tester(); }
    };
    Tester t;

    // Verify
    // - set
    afl::data::Segment seg;
    seg.pushBackInteger(1);
    interpreter::Arguments setArgs(seg, 0, 1);
    TS_ASSERT_THROWS(t.set(setArgs, seg[0]), interpreter::Error);

    // - getDimension
    TS_ASSERT_EQUALS(t.getDimension(0), 0);

    // - makeFirstContext
    TS_ASSERT_THROWS(t.makeFirstContext(), interpreter::Error);

    // - toString
    interpreter::test::ValueVerifier verif(t, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();
}

