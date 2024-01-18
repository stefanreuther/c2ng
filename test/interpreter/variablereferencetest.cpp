/**
  *  \file test/interpreter/variablereferencetest.cpp
  *  \brief Test for interpreter::VariableReference
  */

#include "interpreter/variablereference.hpp"

#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("interpreter.VariableReference:basics", a)
{
    // Environment
    Environment env;

    // Create a process
    interpreter::Process& proc = env.list.create(env.world, a.getLocation());

    // Create some references
    interpreter::VariableReference::Maker m(proc);
    afl::data::IntegerValue iv(42);
    interpreter::VariableReference r1 = m.make("IV", &iv);
    interpreter::VariableReference r2 = m.make("NULL", 0);

    // Verify
    std::auto_ptr<afl::data::Value> p(r1.get(env.list));
    a.checkDifferent("01. get", p.get(), &iv);
    a.checkEqual("02. toInteger", afl::data::Access(p.get()).toInteger(), 42);

    std::auto_ptr<afl::data::Value> p2(r2.get(env.list));
    a.checkNull("11.get", p2.get());
}

/** Test null reference. */
AFL_TEST("interpreter.VariableReference:null", a)
{
    // Environment
    Environment env;

    // Null reference should produce null value
    interpreter::VariableReference r;
    std::auto_ptr<afl::data::Value> p(r.get(env.list));
    a.checkNull("01. get", p.get());
}

/** Test overwrite behaviour. */
AFL_TEST("interpreter.VariableReference:overwrite", a)
{
    // Environment
    Environment env;

    // Create a process
    interpreter::Process& proc = env.list.create(env.world, a.getLocation());

    // Create some references
    interpreter::VariableReference::Maker m(proc);
    afl::data::IntegerValue iv(42);
    interpreter::VariableReference r1 = m.make("IV", &iv);
    afl::data::IntegerValue iv2(69);
    interpreter::VariableReference r2 = m.make("IV", &iv2);

    // Verify
    std::auto_ptr<afl::data::Value> p(r2.get(env.list));
    a.checkDifferent("01. get", p.get(), &iv);
    a.checkEqual("02. toInteger", afl::data::Access(p.get()).toInteger(), 69);

    // No statement to be made about r1
}
