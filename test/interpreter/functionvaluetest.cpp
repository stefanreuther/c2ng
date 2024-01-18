/**
  *  \file test/interpreter/functionvaluetest.cpp
  *  \brief Test for interpreter::FunctionValue
  */

#include "interpreter/functionvalue.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/valueverifier.hpp"

AFL_TEST("interpreter.FunctionValue", a)
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
    AFL_CHECK_THROWS(a("01. set"), t.set(setArgs, seg[0]), interpreter::Error);

    // - getDimension
    a.checkEqual("11. getDimension", t.getDimension(0), 0);

    // - makeFirstContext
    AFL_CHECK_THROWS(a("21. makeFirstContext"), t.makeFirstContext(), interpreter::Error);

    // - toString
    interpreter::test::ValueVerifier verif(t, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
}
