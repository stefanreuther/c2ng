/**
  *  \file test/interpreter/simplefunctiontest.cpp
  *  \brief Test for interpreter::SimpleFunction
  */

#include "interpreter/simplefunction.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include <memory>

namespace {
    afl::data::Value* intFunc(int stateArg, interpreter::Arguments& args)
    {
        args.checkArgumentCount(0);
        return interpreter::makeIntegerValue(stateArg);
    }

    afl::data::Value* voidFunc(interpreter::Arguments& args)
    {
        args.checkArgumentCount(0);
        return interpreter::makeIntegerValue(42);
    }
}

/** Test with non-void state. */
AFL_TEST("interpreter.SimpleFunction:value", a)
{
    interpreter::SimpleFunction<int> testee(77, intFunc);

    // get()
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> p(testee.get(args));
    a.checkEqual("01. get", interpreter::mustBeScalarValue(p.get(), interpreter::Error::ExpectInteger), 77);

    // clone()
    std::auto_ptr<interpreter::FunctionValue> clone(testee.clone());
    a.checkNonNull("11. clone", clone.get());
}

/** Test with void state. */
AFL_TEST("interpreter.SimpleFunction:void", a)
{
    interpreter::SimpleFunction<void> testee(voidFunc);

    // get()
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> p(testee.get(args));
    a.checkEqual("01. get", interpreter::mustBeScalarValue(p.get(), interpreter::Error::ExpectInteger), 42);

    // clone()
    std::auto_ptr<interpreter::FunctionValue> clone(testee.clone());
    a.checkNonNull("11. clone", clone.get());
}
