/**
  *  \file u/t_interpreter_simplefunction.cpp
  *  \brief Test for interpreter::SimpleFunction
  */

#include <memory>
#include "interpreter/simplefunction.hpp"

#include "t_interpreter.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

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
void
TestInterpreterSimpleFunction::testValue()
{
    interpreter::SimpleFunction<int> testee(77, intFunc);

    // get()
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> p(testee.get(args));
    TS_ASSERT_EQUALS(interpreter::mustBeScalarValue(p.get()), 77);

    // clone()
    std::auto_ptr<interpreter::FunctionValue> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
}

/** Test with void state. */
void
TestInterpreterSimpleFunction::testVoid()
{
    interpreter::SimpleFunction<void> testee(voidFunc);

    // get()
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    std::auto_ptr<afl::data::Value> p(testee.get(args));
    TS_ASSERT_EQUALS(interpreter::mustBeScalarValue(p.get()), 42);

    // clone()
    std::auto_ptr<interpreter::FunctionValue> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
}

