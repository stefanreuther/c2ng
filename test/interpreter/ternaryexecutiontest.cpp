/**
  *  \file test/interpreter/ternaryexecutiontest.cpp
  *  \brief Test for interpreter::TernaryExecution
  */

#include "interpreter/ternaryexecution.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/world.hpp"
#include <memory>

using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::data::Value;
using interpreter::KeymapValue;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;
        std::auto_ptr<afl::data::Value> p;

        TestHarness()
            : log(), tx(), fileSystem(), world(log, tx, fileSystem)
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

AFL_TEST("interpreter.TernaryExecution:teKeyAdd", a)
{
    TestHarness h;
    util::KeymapRef_t k = h.world.keymaps().createKeymap("K");

    // Null
    h.exec(interpreter::teKeyAdd, 0, addr(StringValue("q")), addr(StringValue("cmd")));
    a.check("01. isNull", h.isNull());
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), 0, addr(StringValue("cmd")));
    a.check("02. isNull", h.isNull());
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q")), 0);
    a.check("03. isNull", h.isNull());

    // Regular case (string)
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q")), addr(StringValue("cmd")));
    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(h.p.get());
    a.check("11. KeymapValue", kv);
    a.check("12. getKeymap", kv->getKeymap() == k);
    a.check("13. lookupCommand", k->lookupCommand('q') != 0);

    // Regular case (int)
    h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("r")), addr(IntegerValue(12345)));
    kv = dynamic_cast<const interpreter::KeymapValue*>(h.p.get());
    a.check("21. KeymapValue", kv);
    a.check("22. getKeymap", kv->getKeymap() == k);
    a.checkEqual("23. lookupCommand", k->lookupCommand('r'), 12345U);

    // Error case
    // - type error on keymap
    AFL_CHECK_THROWS(a("31. type error"), h.exec(interpreter::teKeyAdd, addr(IntegerValue(1)), addr(StringValue("q")), addr(StringValue("cmd"))), interpreter::Error);
    // - type error on key
    AFL_CHECK_THROWS(a("32. type error"), h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(IntegerValue(1)), addr(StringValue("cmd"))), interpreter::Error);
    // - invalid key
    AFL_CHECK_THROWS(a("33. invalid key"), h.exec(interpreter::teKeyAdd, addr(KeymapValue(k)), addr(StringValue("q-q-q-q")), addr(StringValue("cmd"))), interpreter::Error);
}

AFL_TEST("interpreter.TernaryExecution:invalid", a)
{
    TestHarness h;
    AFL_CHECK_THROWS(a, h.exec(200, 0, 0, 0), interpreter::Error);
}
