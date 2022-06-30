/**
  *  \file u/t_interpreter_simpleprocedure.cpp
  *  \brief Test for interpreter::SimpleProcedure
  */

#include "interpreter/simpleprocedure.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

namespace {
    void intFunc(int& state, interpreter::Process& /*proc*/, interpreter::Arguments& args)
    {
        args.checkArgumentCount(0);
        ++state;                       // externally-visible effect
    }

    void voidFunc(interpreter::Process& proc, interpreter::Arguments& args)
    {
        args.checkArgumentCount(0);
        proc.setName("renamed");       // externally-visible effect
    }

    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment()
            : log(), tx(), fs(), world(log, tx, fs), proc(world, "proc", 999)
            { }
    };
}

/** Test with non-void state parameter.
    Use `int&` to pass a visible result out of the function. */
void
TestInterpreterSimpleProcedure::testValue()
{
    int state = 0;
    interpreter::SimpleProcedure<int&> testee(state, intFunc);

    // call
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    testee.call(env.proc, args);
    TS_ASSERT_EQUALS(state, 1);

    // clone
    std::auto_ptr<interpreter::ProcedureValue> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
}

/** Test with non-void state parameter.
    Use the process name to pass a visible result out of the function. */
void
TestInterpreterSimpleProcedure::testVoid()
{
    interpreter::SimpleProcedure<void> testee(voidFunc);

    // call
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    testee.call(env.proc, args);
    TS_ASSERT_EQUALS(env.proc.getName(), "renamed");

    // clone
    std::auto_ptr<interpreter::ProcedureValue> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
}

