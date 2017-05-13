/**
  *  \file u/t_interpreter_arguments.cpp
  *  \brief Test for interpreter::Arguments
  */

#include <memory>
#include "interpreter/arguments.hpp"

#include "t_interpreter.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "util/atomtable.hpp"

/** Simple test. */
void
TestInterpreterArguments::testIt()
{
    // Prepare a segment
    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBack(0);
    seg.pushBackString("x");
    TS_ASSERT_EQUALS(seg.size(), 3U);

    // Testee
    interpreter::Arguments testee(seg, 0, 3);
    TS_ASSERT_EQUALS(testee.getNumArgs(), 3U);

    TS_ASSERT_THROWS_NOTHING(testee.checkArgumentCount(3));
    TS_ASSERT_THROWS_NOTHING(testee.checkArgumentCountAtLeast(3));
    TS_ASSERT_THROWS_NOTHING(testee.checkArgumentCountAtLeast(2));
    TS_ASSERT_THROWS_NOTHING(testee.checkArgumentCount(0, 3));
    TS_ASSERT_THROWS_NOTHING(testee.checkArgumentCount(3, 4));

    TS_ASSERT_THROWS(testee.checkArgumentCount(2), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkArgumentCount(4), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkArgumentCountAtLeast(4), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkArgumentCount(0, 2), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkArgumentCount(4, 5), interpreter::Error);

    // Consume args
    afl::data::Value* p1 = testee.getNext();
    afl::data::Value* p2 = testee.getNext();
    TS_ASSERT_EQUALS(testee.getNumArgs(), 1U);
    TS_ASSERT_EQUALS(p1, seg[0]);
    TS_ASSERT_EQUALS(p2, seg[1]);

    afl::data::Value* p3 = testee.getNext();
    afl::data::Value* p4 = testee.getNext();
    TS_ASSERT_EQUALS(testee.getNumArgs(), 0U);
    TS_ASSERT_EQUALS(p3, seg[2]);
    TS_ASSERT_EQUALS(p4, (afl::data::Value*) 0);
}

/** Test checkArgumentCount(). */
void
TestInterpreterArguments::testArgumentCount()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(0, 0, 0));

    TS_ASSERT_THROWS(interpreter::checkArgumentCount(1, 0, 0), interpreter::Error);
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(1, 0, 1));
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(1, 1, 1));

    // These are the Argument test-cases:
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(3, 3, 3));
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(3, 2, 3));
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(3, 0, 3));
    TS_ASSERT_THROWS_NOTHING(interpreter::checkArgumentCount(3, 3, 4));

    TS_ASSERT_THROWS(interpreter::checkArgumentCount(3, 2, 2), interpreter::Error);
    TS_ASSERT_THROWS(interpreter::checkArgumentCount(3, 4, 4), interpreter::Error);
    TS_ASSERT_THROWS(interpreter::checkArgumentCount(3, 4, 3), interpreter::Error);
    TS_ASSERT_THROWS(interpreter::checkArgumentCount(3, 0, 2), interpreter::Error);
    TS_ASSERT_THROWS(interpreter::checkArgumentCount(3, 4, 5), interpreter::Error);
}

/** Test checkIntegerArg(). */
void
TestInterpreterArguments::testInteger()
{
    int32_t iv = 0;

    // Null
    TS_ASSERT(!interpreter::checkIntegerArg(iv, 0));
    TS_ASSERT(!interpreter::checkIntegerArg(iv, 0, 1, 10));

    // Integer
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get()));
        TS_ASSERT_EQUALS(iv, 3);
        iv = 0;

        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get(), 1, 10));
        TS_ASSERT_EQUALS(iv, 3);
        iv = 0;

        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 0, 2), interpreter::Error);
    }

    // String
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
    }

    // String: no implicit destringification!
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("7"));
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
    }

    // Bool
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get()));
        TS_ASSERT_EQUALS(iv, 1);
        iv = 0;

        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get(), 1, 10));
        TS_ASSERT_EQUALS(iv, 1);
        iv = 0;

        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 2, 5), interpreter::Error);
    }

    // Float
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get()));
        TS_ASSERT_EQUALS(iv, 16);
        iv = 0;

        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get(), 1, 16));
        TS_ASSERT_EQUALS(iv, 16);
        iv = 0;

        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 0, 5), interpreter::Error);
    }

    // Float overflow
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0E20));
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
        TS_ASSERT_THROWS(interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
    }
}

/** Test checkBooleanArg(). */
void
TestInterpreterArguments::testBoolean()
{
    bool bv;

    // Null
    TS_ASSERT(!interpreter::checkBooleanArg(bv, 0));

    // Integer
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, true);
    }
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(0));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, false);
    }

    // String
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, true);
    }
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue(""));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, false);
    }

    // Bool
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, true);
        bv = false;
    }
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(0));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, false);
    }

    // Float
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
        TS_ASSERT(interpreter::checkBooleanArg(bv, p.get()));
        TS_ASSERT_EQUALS(bv, true);
    }
}

/** Test checkStringArg(). */
void
TestInterpreterArguments::testString()
{
    String_t sv;

    // Null
    TS_ASSERT(!interpreter::checkStringArg(sv, 0));

    // Integer
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
        TS_ASSERT(interpreter::checkStringArg(sv, p.get()));
        TS_ASSERT_EQUALS(sv, "3");
    }

    // String
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
        TS_ASSERT(interpreter::checkStringArg(sv, p.get()));
        TS_ASSERT_EQUALS(sv, "hi");
    }

    // Bool
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
        TS_ASSERT(interpreter::checkStringArg(sv, p.get()));
        TS_ASSERT_EQUALS(sv, "YES");
    }

    // Float
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
        TS_ASSERT(interpreter::checkStringArg(sv, p.get()));
        TS_ASSERT_EQUALS(sv, "16.25");
    }

    // Huge float
    {
        std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0E20));
        TS_ASSERT(interpreter::checkStringArg(sv, p.get()));
        TS_ASSERT_EQUALS(sv, "100000000000000000000");
    }
}

/** Test checkFlagArg. */
void
TestInterpreterArguments::testFlagArg()
{
    // Null
    {
        int32_t flags = 0, value = 0;
        TS_ASSERT(!interpreter::checkFlagArg(flags, &value, 0, "XYZ"));
    }

    // Integer
    {
        int32_t flags = 0, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
        TS_ASSERT(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 0);
        TS_ASSERT_EQUALS(value, 3);

        // Fails if no value requested: integer should go in value slot
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
    }

    // String
    {
        int32_t flags = 64, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XY7"));
        TS_ASSERT(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 67);      // new flags added to existing value! FIXME: is this correct?
        TS_ASSERT_EQUALS(value, 7);

        // Fails if no value requested: value speciifed in string
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
    }

    // String, value at back
    {
        int32_t flags = 0, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("9XY"));
        TS_ASSERT(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 3);
        TS_ASSERT_EQUALS(value, 9);

        // Fails if no value requested: value specified in string
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
    }

    // String, just flags
    {
        int32_t flags = 0, value = 77;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XZ"));
        TS_ASSERT(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 5);
        TS_ASSERT_EQUALS(value, 77);      // unchanged because not specified!

        // Succeeds if no value requested
        flags = 0;
        TS_ASSERT(interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 5);
    }

    // String, just flags
    {
        int32_t flags = 0, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("X3Z"));
        TS_ASSERT(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
        TS_ASSERT_EQUALS(flags, 5);
        TS_ASSERT_EQUALS(value, 3);

        // Fails if no value requested: value specified in string
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
    }

    // String, multiple numbers (bad syntax)
    {
        int32_t flags = 0, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("X3Z5"));
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"), interpreter::Error);
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0,      p.get(), "XYZ"), interpreter::Error);
    }

    // String, bad flags
    {
        int32_t flags = 0, value = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XA"));
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"), interpreter::Error);
        TS_ASSERT_THROWS(interpreter::checkFlagArg(flags, 0,      p.get(), "XYZ"), interpreter::Error);
    }
}

/** Test checkCommandAtomArg. */
void
TestInterpreterArguments::testAtomArg()
{
    // Atom table
    util::AtomTable tab;
    util::Atom_t a = tab.getAtomFromString("foo");

    // Null
    {
        util::Atom_t result = 0;
        TS_ASSERT(!interpreter::checkCommandAtomArg(result, 0, tab));
    }

    // Integer
    {
        util::Atom_t result = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(333));
        TS_ASSERT(interpreter::checkCommandAtomArg(result, p.get(), tab));
        TS_ASSERT_EQUALS(result, util::Atom_t(333));
    }

    // String
    {
        util::Atom_t result = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("foo"));
        TS_ASSERT(interpreter::checkCommandAtomArg(result, p.get(), tab));
        TS_ASSERT_EQUALS(result, a);
    }

    // String (new atom)
    {
        util::Atom_t result = 0;
        std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("bar"));
        TS_ASSERT(interpreter::checkCommandAtomArg(result, p.get(), tab));
        TS_ASSERT_DIFFERS(result, a);
        TS_ASSERT_EQUALS(result, tab.getAtomFromString("bar"));
    }
}

