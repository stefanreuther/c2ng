/**
  *  \file u/t_interpreter_variablereference.cpp
  *  \brief Test for interpreter::VariableReference
  */

#include "interpreter/variablereference.hpp"

#include "t_interpreter.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/processlist.hpp"
#include "interpreter/world.hpp"

namespace {
    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::ProcessList list;

        Environment()
            : log(), tx(), fs(), world(log, tx, fs), list()
            { }
    };
}

/** Test normal behaviour. */
void
TestInterpreterVariableReference::testIt()
{
    // Environment
    Environment env;

    // Create a process
    interpreter::Process& proc = env.list.create(env.world, "testIt");

    // Create some references
    interpreter::VariableReference::Maker m(proc);
    afl::data::IntegerValue iv(42);
    interpreter::VariableReference r1 = m.make("IV", &iv);
    interpreter::VariableReference r2 = m.make("NULL", 0);

    // Verify
    std::auto_ptr<afl::data::Value> p(r1.get(env.list));
    TS_ASSERT_DIFFERS(p.get(), &iv);
    TS_ASSERT_EQUALS(afl::data::Access(p.get()).toInteger(), 42);

    std::auto_ptr<afl::data::Value> p2(r2.get(env.list));
    TS_ASSERT(p2.get() == 0);
}

/** Test null reference. */
void
TestInterpreterVariableReference::testNull()
{
    // Environment
    Environment env;

    // Create a process
    interpreter::Process& proc = env.list.create(env.world, "testIt");

    // Null reference should produce null value
    interpreter::VariableReference r;
    std::auto_ptr<afl::data::Value> p(r.get(env.list));
    TS_ASSERT(p.get() == 0);
}

/** Test overwrite behaviour. */
void
TestInterpreterVariableReference::testOverwrite()
{
    // Environment
    Environment env;

    // Create a process
    interpreter::Process& proc = env.list.create(env.world, "testOverwrite");

    // Create some references
    interpreter::VariableReference::Maker m(proc);
    afl::data::IntegerValue iv(42);
    interpreter::VariableReference r1 = m.make("IV", &iv);
    afl::data::IntegerValue iv2(69);
    interpreter::VariableReference r2 = m.make("IV", &iv2);

    // Verify
    std::auto_ptr<afl::data::Value> p(r2.get(env.list));
    TS_ASSERT_DIFFERS(p.get(), &iv);
    TS_ASSERT_EQUALS(afl::data::Access(p.get()).toInteger(), 69);

    // No statement to be made about r1
}

