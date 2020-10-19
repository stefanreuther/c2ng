/**
  *  \file u/t_interpreter_ternaryexecution.cpp
  *  \brief Test for interpreter::TernaryExecution
  */

#include "interpreter/ternaryexecution.hpp"

#include <memory>
#include "t_interpreter.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/error.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/world.hpp"

using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::data::Value;
using interpreter::KeymapValue;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;
        std::auto_ptr<afl::data::Value> p;

        TestHarness()
            : log(), fileSystem(), world(log, fileSystem)
            { }

        void exec(uint8_t op, const afl::data::Value* a, const afl::data::Value* b, const afl::data::Value* c)
            { p.reset(interpreter::executeTernaryOperation(world, op, a, b, c)); }

        bool isNull() const
            { return p.get() == 0; }
    };

    // Shortcut for getting the address of a temporary
    const Value* addr(const Value& v)
    {
        return &v;
    }
}

void
TestInterpreterTernaryExecution::testKeyAdd()
{
    TestHarness h;
    util::KeymapRef_t k = h.world.keymaps().createKeymap("K");

    // Null
    h.exec(interpreter::teKeyAdd, 0, addr(StringValue("q")), addr(StringValue("cmd")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), 0, addr(StringValue("cmd")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q")), 0);
    TS_ASSERT(h.isNull());

    // Regular case (string)
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q")), addr(StringValue("cmd")));
    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(h.p.get());
    TS_ASSERT(kv);
    TS_ASSERT(kv->getKeymap() == k);
    TS_ASSERT(k->lookupCommand('q') != 0);

    // Regular case (int)
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("r")), addr(IntegerValue(12345)));
    kv = dynamic_cast<const interpreter::KeymapValue*>(h.p.get());
    TS_ASSERT(kv);
    TS_ASSERT(kv->getKeymap() == k);
    TS_ASSERT_EQUALS(k->lookupCommand('r'), 12345U);

    // Error case
    // - type error on keymap
    TS_ASSERT_THROWS(h.exec(interpreter::teKeyAdd, addr(IntegerValue(1)), addr(StringValue("q")), addr(StringValue("cmd"))), interpreter::Error);
    // - type error on key
    TS_ASSERT_THROWS(h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(IntegerValue(1)), addr(StringValue("cmd"))), interpreter::Error);
    // - invalid key
    TS_ASSERT_THROWS(h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q-q-q-q")), addr(StringValue("cmd"))), interpreter::Error);
}

void
TestInterpreterTernaryExecution::testInvalid()
{
    TestHarness h;
    TS_ASSERT_THROWS(h.exec(200, 0, 0, 0), interpreter::Error);
}

