/**
  *  \file u/t_interpreter_binaryexecution.cpp
  *  \brief Test for interpreter::BinaryExecution
  */

#include "interpreter/binaryexecution.hpp"

#include "t_interpreter.hpp"
#include "afl/data/access.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/keymapvalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/world.hpp"

using afl::data::BooleanValue;
using afl::data::FloatValue;
using afl::data::Hash;
using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::data::Value;
using interpreter::ArrayValue;
using interpreter::FileValue;
using interpreter::HashValue;
using interpreter::KeymapValue;
using interpreter::SubroutineValue;
using interpreter::executeBinaryOperation;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;
        std::auto_ptr<afl::data::Value> p;

        TestHarness()
            : log(), fileSystem(), world(log, fileSystem)
            { }

        void exec(uint8_t op, const afl::data::Value* a, const afl::data::Value* b)
            { p.reset(executeBinaryOperation(world, op, a, b)); }

        int32_t toInteger() const
            {
                IntegerValue* iv = dynamic_cast<IntegerValue*>(p.get());
                if (iv == 0) {
                    throw interpreter::Error::typeError();
                }
                return iv->getValue();
            }

        double toFloat() const
            {
                FloatValue* fv = dynamic_cast<FloatValue*>(p.get());
                if (fv == 0) {
                    throw interpreter::Error::typeError();
                }
                return fv->getValue();
            }

        bool toBoolean() const
            {
                BooleanValue* bv = dynamic_cast<BooleanValue*>(p.get());
                if (bv == 0) {
                    throw interpreter::Error::typeError();
                }
                return bv->getValue();
            }

        String_t toString() const
            {
                StringValue* sv = dynamic_cast<StringValue*>(p.get());
                if (sv == 0) {
                    throw interpreter::Error::typeError();
                }
                return sv->getValue();
            }

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
TestInterpreterBinaryExecution::testAnd()
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biAnd, 0,                     0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), 0);
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biAnd, 0,                     addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biAnd, 0,                     addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    // Type variants
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
}

void
TestInterpreterBinaryExecution::testOr()
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biOr, 0,                     0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biOr, addr(IntegerValue(1)), 0);
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), 0);
    TS_ASSERT(h.isNull());

    h.exec(interpreter::biOr, 0,                     addr(IntegerValue(0)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biOr, 0,                     addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    // Type variants
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
}

void
TestInterpreterBinaryExecution::testXor()
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biXor, 0,                     0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(0)), 0);
    TS_ASSERT(h.isNull());

    h.exec(interpreter::biXor, 0,                     addr(IntegerValue(0)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biXor, 0,                     addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    // Type variants
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
}

void
TestInterpreterBinaryExecution::testAdd()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biAdd, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biAdd, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(IntegerValue(7)));
    TS_ASSERT_EQUALS(h.toInteger(), 8);
    h.exec(interpreter::biAdd, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 10);

    // Float
    h.exec(interpreter::biAdd, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 9.0);

    // Mixed
    h.exec(interpreter::biAdd, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toFloat(), 4.5);
    h.exec(interpreter::biAdd, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 4.5);

    // String
    h.exec(interpreter::biAdd, addr(StringValue("x")), addr(StringValue("y")));
    TS_ASSERT_EQUALS(h.toString(), "xy");

    // Bogus mix
    TS_ASSERT_THROWS(h.exec(interpreter::biAdd, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testSub()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biSub, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biSub, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biSub, addr(IntegerValue(10)), addr(IntegerValue(7)));
    TS_ASSERT_EQUALS(h.toInteger(), 3);
    h.exec(interpreter::biSub, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 8);

    // Float
    h.exec(interpreter::biSub, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    TS_ASSERT_EQUALS(h.toFloat(), -6.0);

    // Mixed
    h.exec(interpreter::biSub, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toFloat(), -1.5);
    h.exec(interpreter::biSub, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 1.5);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biSub, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biSub, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testMult()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMult, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biMult, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biMult, addr(IntegerValue(10)), addr(IntegerValue(7)));
    TS_ASSERT_EQUALS(h.toInteger(), 70);
    h.exec(interpreter::biMult, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);

    // Float
    h.exec(interpreter::biMult, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 11.25);

    // Mixed
    h.exec(interpreter::biMult, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toFloat(), 4.5);
    h.exec(interpreter::biMult, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 4.5);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biMult, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biMult, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biMult, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biMult, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testDivide()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biDivide, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biDivide, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar - integer result
    h.exec(interpreter::biDivide, addr(IntegerValue(10)), addr(IntegerValue(5)));
    TS_ASSERT_EQUALS(h.toInteger(), 2);
    h.exec(interpreter::biDivide, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);

    // Float result
    h.exec(interpreter::biDivide, addr(IntegerValue(5)), addr(IntegerValue(10)));
    TS_ASSERT_EQUALS(h.toFloat(), 0.5);

    // Float
    h.exec(interpreter::biDivide, addr(FloatValue(4.5)), addr(FloatValue(1.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 3.0);

    // Mixed
    h.exec(interpreter::biDivide, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toFloat(), 0.5);
    h.exec(interpreter::biDivide, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 2.0);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biDivide, addr(FloatValue(1)), addr(FloatValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testIntegerDivide()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biIntegerDivide, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(10)), addr(IntegerValue(5)));
    TS_ASSERT_EQUALS(h.toInteger(), 2);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(9)), addr(IntegerValue(10)));
    TS_ASSERT_EQUALS(h.toInteger(), 0);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(-12)), addr(IntegerValue(5)));
    TS_ASSERT_EQUALS(h.toInteger(), -2);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(FloatValue(4.5)), addr(FloatValue(1.5))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    TS_ASSERT_THROWS(h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testRemainder()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRemainder, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biRemainder, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biRemainder, addr(IntegerValue(10)), addr(IntegerValue(5)));
    TS_ASSERT_EQUALS(h.toInteger(), 0);
    h.exec(interpreter::biRemainder, addr(IntegerValue(9)), addr(IntegerValue(10)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);
    h.exec(interpreter::biRemainder, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 0);
    h.exec(interpreter::biRemainder, addr(IntegerValue(-12)), addr(IntegerValue(5)));
    TS_ASSERT_EQUALS(h.toInteger(), -2);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(FloatValue(4.5)), addr(FloatValue(1.5))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    TS_ASSERT_THROWS(h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testPow()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biPow, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biPow, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Scalar
    h.exec(interpreter::biPow, addr(IntegerValue(10)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toInteger(), 1000);
    h.exec(interpreter::biPow, addr(IntegerValue(9)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);
    h.exec(interpreter::biPow, addr(IntegerValue(0)), addr(IntegerValue(10000)));
    TS_ASSERT_EQUALS(h.toInteger(), 0);
    h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(IntegerValue(10000)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);
    h.exec(interpreter::biPow, addr(IntegerValue(-1)), addr(IntegerValue(10000)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);

    // Overflow to float
    h.exec(interpreter::biPow, addr(IntegerValue(16)), addr(IntegerValue(10)));
    TS_ASSERT_EQUALS(h.toFloat(), 1099511627776.0);

    // Float
    h.exec(interpreter::biPow, addr(FloatValue(1.5)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toFloat(), 2.25);

    // Type errors
    TS_ASSERT_THROWS(h.exec(interpreter::biPow, addr(IntegerValue(10)), addr(FloatValue(2.5))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biPow, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biPow, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testConcat()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biConcat, 0, 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biConcat, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Not null
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toString(), "12");
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toString(), "1x");
}

void
TestInterpreterBinaryExecution::testConcatEmpty()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biConcatEmpty, 0, 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biConcatEmpty, 0, addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toString(), "1");
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), 0);
    TS_ASSERT_EQUALS(h.toString(), "1");

    // Not null
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toString(), "12");
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toString(), "1x");
}

void
TestInterpreterBinaryExecution::testCompare()
{
    TestHarness h;

    // Comparing anything with null must produce null, with all relations
    static const uint8_t RELATIONS[] = {
        interpreter::biCompareEQ,
        interpreter::biCompareEQ_NC,
        interpreter::biCompareNE,
        interpreter::biCompareNE_NC,
        interpreter::biCompareGE,
        interpreter::biCompareGE_NC,
        interpreter::biCompareGT,
        interpreter::biCompareGT_NC,
        interpreter::biCompareLE,
        interpreter::biCompareLE_NC,
        interpreter::biCompareLT,
        interpreter::biCompareLT_NC,
    };
    for (size_t i = 0; i < sizeof(RELATIONS)/sizeof(RELATIONS[0]); ++i) {
        h.exec(RELATIONS[i], 0, addr(IntegerValue(1)));
        TS_ASSERT(h.isNull());
        h.exec(RELATIONS[i], 0, addr(StringValue("x")));
        TS_ASSERT(h.isNull());
        h.exec(RELATIONS[i], addr(StringValue("x")), 0);
        TS_ASSERT(h.isNull());
    }

    // Integer comparisons
    h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareEQ_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareEQ, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareNE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGT, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareGT, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareGT_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareLE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    // Float comparison (specimen only for brevity)
    h.exec(interpreter::biCompareEQ, addr(FloatValue(1)), addr(FloatValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareNE, addr(FloatValue(1)), addr(FloatValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareGT, addr(FloatValue(3)), addr(FloatValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLT, addr(FloatValue(1)), addr(FloatValue(3)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    // Mixed
    h.exec(interpreter::biCompareEQ, addr(FloatValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLT, addr(IntegerValue(1)), addr(FloatValue(3.5)));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    // String comparisons
    h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(StringValue("A")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(StringValue("a")), addr(StringValue("A")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareEQ, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(StringValue("a")), addr(StringValue("A")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareGE_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareGT, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareGT, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(StringValue("a")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), true);
    h.exec(interpreter::biCompareLT_NC, addr(StringValue("B")), addr(StringValue("a")));
    TS_ASSERT_EQUALS(h.toBoolean(), false);

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testMin()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMin, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biMin, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biMin_NC, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Integer
    h.exec(interpreter::biMin, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);
    h.exec(interpreter::biMin_NC, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);
    h.exec(interpreter::biMin_NC, addr(IntegerValue(3)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 2);

    // Float
    h.exec(interpreter::biMin, addr(FloatValue(1)), addr(FloatValue(2)));
    TS_ASSERT_EQUALS(h.toFloat(), 1);

    // Mixed
    h.exec(interpreter::biMin, addr(IntegerValue(9)), addr(FloatValue(2.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 2.5);
    h.exec(interpreter::biMin, addr(IntegerValue(1)), addr(FloatValue(2.5)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);

    // String
    h.exec(interpreter::biMin, addr(StringValue("a")), addr(StringValue("B")));
    TS_ASSERT_EQUALS(h.toString(), "B");
    h.exec(interpreter::biMin_NC, addr(StringValue("a")), addr(StringValue("B")));
    TS_ASSERT_EQUALS(h.toString(), "a");
    h.exec(interpreter::biMin_NC, addr(StringValue("a")), addr(StringValue("A"))); // on tie, second arg wins
    TS_ASSERT_EQUALS(h.toString(), "A");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biMin, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biMin, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testMax()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMax, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biMax, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biMax_NC, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());

    // Integer
    h.exec(interpreter::biMax, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 2);
    h.exec(interpreter::biMax_NC, addr(IntegerValue(1)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 2);
    h.exec(interpreter::biMax_NC, addr(IntegerValue(3)), addr(IntegerValue(2)));
    TS_ASSERT_EQUALS(h.toInteger(), 3);

    // Float
    h.exec(interpreter::biMax, addr(FloatValue(1)), addr(FloatValue(2)));
    TS_ASSERT_EQUALS(h.toFloat(), 2);

    // Mixed
    h.exec(interpreter::biMax, addr(IntegerValue(9)), addr(FloatValue(2.5)));
    TS_ASSERT_EQUALS(h.toInteger(), 9);
    h.exec(interpreter::biMax, addr(IntegerValue(1)), addr(FloatValue(2.5)));
    TS_ASSERT_EQUALS(h.toFloat(), 2.5);

    // String
    h.exec(interpreter::biMax, addr(StringValue("a")), addr(StringValue("B")));
    TS_ASSERT_EQUALS(h.toString(), "a");
    h.exec(interpreter::biMax_NC, addr(StringValue("a")), addr(StringValue("B")));
    TS_ASSERT_EQUALS(h.toString(), "B");
    h.exec(interpreter::biMax_NC, addr(StringValue("a")), addr(StringValue("A"))); // on tie, second arg wins
    TS_ASSERT_EQUALS(h.toString(), "A");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biMax, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biMax, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testFirstStr()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biFirstStr, addr(StringValue("a")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biFirstStr, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biFirstStr_NC, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toString(), "Rhabarber-");
    h.exec(interpreter::biFirstStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toString(), "Rha");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    TS_ASSERT_EQUALS(h.toString(), "Rhabarber-Barbara");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    TS_ASSERT_EQUALS(h.toString(), "");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toString(), "");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biFirstStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biFirstStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testRestStr()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRestStr, addr(StringValue("a")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biRestStr, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biRestStr_NC, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toString(), "bara");
    h.exec(interpreter::biRestStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toString(), "ber-Barbara");
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    TS_ASSERT_EQUALS(h.toString(), "habarber-Barbara");
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toString(), "Rhabarber-Barbara");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRestStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRestStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testFindStr()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biFindStr, addr(StringValue("a")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biFindStr, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biFindStr_NC, 0, addr(StringValue("a")));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toInteger(), 11);
    h.exec(interpreter::biFindStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    TS_ASSERT_EQUALS(h.toInteger(), 4);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    TS_ASSERT_EQUALS(h.toInteger(), 0);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    TS_ASSERT_EQUALS(h.toInteger(), 1);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toInteger(), 1);

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biFindStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biFindStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testBitAnd()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitAnd, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biBitAnd, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biBitAnd, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    TS_ASSERT_EQUALS(h.toInteger(), 0x0F0);
    h.exec(interpreter::biBitAnd, addr(BooleanValue(1)), addr(IntegerValue(0x0FF)));
    TS_ASSERT_EQUALS(h.toInteger(), 1);

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biBitAnd, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitAnd, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitAnd, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testBitOr()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitOr, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biBitOr, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biBitOr, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    TS_ASSERT_EQUALS(h.toInteger(), 0xFFF);
    h.exec(interpreter::biBitOr, addr(IntegerValue(0xFF0)), addr(BooleanValue(1)));
    TS_ASSERT_EQUALS(h.toInteger(), 0xFF1);

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biBitOr, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitOr, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitOr, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testBitXor()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitXor, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biBitXor, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biBitXor, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    TS_ASSERT_EQUALS(h.toInteger(), 0xF0F);
    h.exec(interpreter::biBitXor, addr(BooleanValue(1)), addr(IntegerValue(0x0FF)));
    TS_ASSERT_EQUALS(h.toInteger(), 0x0FE);

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biBitXor, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitXor, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biBitXor, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testStr()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biStr, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biStr, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toString(), "42");
    h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toString(), "42.000");
    h.exec(interpreter::biStr, addr(FloatValue(42.0125)), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toString(), "42.013");
    h.exec(interpreter::biStr, addr(BooleanValue(1)), addr(IntegerValue(7)));
    TS_ASSERT_EQUALS(h.toString(), "YES");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(-1))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(FloatValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biStr, addr(StringValue("x")), addr(IntegerValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biStr, addr(HashValue(Hash::create())), addr(IntegerValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testATan()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biATan, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biATan, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biATan, addr(IntegerValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toFloat(), 45);
    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toFloat(), 45);
    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(FloatValue(1)));
    TS_ASSERT_EQUALS(h.toFloat(), 45);

    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(FloatValue(0)));
    TS_ASSERT_EQUALS(h.toFloat(), 90);
    h.exec(interpreter::biATan, addr(FloatValue(0)), addr(FloatValue(1)));
    TS_ASSERT_EQUALS(h.toFloat(), 0);

    // Undefined
    h.exec(interpreter::biATan, addr(FloatValue(0)), addr(FloatValue(0)));
    TS_ASSERT(h.isNull());

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biATan, addr(StringValue("x")), addr(IntegerValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biATan, addr(HashValue(Hash::create())), addr(IntegerValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testLCut()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biLCut, addr(StringValue("")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biLCut, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toString(), "llo");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    TS_ASSERT_EQUALS(h.toString(), "");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toString(), "hello");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toString(), "hello");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biLCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biLCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testRCut()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRCut, addr(StringValue("")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biRCut, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toString(), "hel");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    TS_ASSERT_EQUALS(h.toString(), "hello");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toString(), "h");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toString(), "");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biRCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biRCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testEndCut()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biEndCut, addr(StringValue("")), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biEndCut, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    TS_ASSERT_EQUALS(h.toString(), "llo");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    TS_ASSERT_EQUALS(h.toString(), "hello");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    TS_ASSERT_EQUALS(h.toString(), "o");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    TS_ASSERT_EQUALS(h.toString(), "");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biEndCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biEndCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testStrMult()
{
    TestHarness h;

    // Null
    h.exec(interpreter::biStrMult, addr(IntegerValue(1)), 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biStrMult, 0, addr(StringValue("")));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biStrMult, addr(IntegerValue(100000)), addr(StringValue("")));
    TS_ASSERT_EQUALS(h.toString(), "");
    h.exec(interpreter::biStrMult, addr(IntegerValue(3)), addr(StringValue("x")));
    TS_ASSERT_EQUALS(h.toString(), "xxx");
    h.exec(interpreter::biStrMult, addr(IntegerValue(5)), addr(StringValue("ha")));
    TS_ASSERT_EQUALS(h.toString(), "hahahahaha");

    // Errors
    TS_ASSERT_THROWS(h.exec(interpreter::biStrMult, addr(IntegerValue(5)), addr(IntegerValue(5))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biStrMult, addr(FloatValue(5)), addr(StringValue("X"))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testKeyAddParent()
{
    TestHarness h;
    KeymapValue a(h.world.keymaps().createKeymap("A"));
    KeymapValue b(h.world.keymaps().createKeymap("B"));

    // Null
    h.exec(interpreter::biKeyAddParent, &a, 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biKeyAddParent, 0, &b);
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biKeyAddParent, &a, &b);
    KeymapValue* kv = dynamic_cast<KeymapValue*>(h.p.get());
    TS_ASSERT(kv != 0);
    TS_ASSERT(kv->getKeymap() == a.getKeymap());
    TS_ASSERT(a.getKeymap()->hasParent(*b.getKeymap()));

    // Error - duplicate, loop. These are handled by util::Keymap and thus do not throw an interpreter error.
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyAddParent, &a, &b), std::runtime_error);
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyAddParent, &b, &a), std::runtime_error);

    // Error - types
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyAddParent, addr(IntegerValue(5)), &b), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyAddParent, &a, addr(IntegerValue(5))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testKeyFind()
{
    TestHarness h;
    KeymapValue a(h.world.keymaps().createKeymap("A"));
    a.getKeymap()->addKey('q', 42, 23);

    // Null
    h.exec(interpreter::biKeyFind, &a, 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biKeyFind, 0, addr(StringValue("k")));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biKeyFind, &a, addr(StringValue("q")));  // found
    TS_ASSERT_EQUALS(h.toInteger(), 42);
    h.exec(interpreter::biKeyFind, &a, addr(StringValue("z")));  // not found
    TS_ASSERT(h.isNull());

    // Error - invalid key name (should this actually be an error?)
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyFind, &a, addr(StringValue("escape meta cokebottle"))), interpreter::Error);

    // Error
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyFind, &a, addr(IntegerValue(5))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biKeyFind, addr(IntegerValue(5)), addr(StringValue("y"))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testArrayDim()
{
    class Tester : public interpreter::CallableValue {
     public:
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { TS_ASSERT(!"call"); }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int32_t getDimension(int32_t which) const
            { return which + 2; }
        virtual interpreter::Context* makeFirstContext()
            { TS_ASSERT(!"makeFirstContext"); return 0; }
        virtual Tester* clone() const
            { TS_ASSERT(!"clone"); return 0; }
        virtual String_t toString(bool /*readable*/) const
            { TS_ASSERT(!"toString"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { TS_ASSERT(!"store"); }
        virtual void visit(afl::data::Visitor& /*visitor*/) const
            { TS_ASSERT(!"visit"); }
    };

    TestHarness h;
    Tester t;

    // Null
    h.exec(interpreter::biArrayDim, &t, 0);
    TS_ASSERT(h.isNull());
    h.exec(interpreter::biArrayDim, 0, addr(IntegerValue(1)));
    TS_ASSERT(h.isNull());

    // Normal
    h.exec(interpreter::biArrayDim, &t, addr(BooleanValue(1)));      // 1st dimension
    TS_ASSERT_EQUALS(h.toInteger(), 3);
    h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(2)));      // 2nd dimension
    TS_ASSERT_EQUALS(h.toInteger(), 4);

    // Errors - range
    TS_ASSERT_THROWS(h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(3))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(-1))), interpreter::Error);

    // Errors - type
    TS_ASSERT_THROWS(h.exec(interpreter::biArrayDim, &t, addr(FloatValue(0))), interpreter::Error);
    TS_ASSERT_THROWS(h.exec(interpreter::biArrayDim, addr(IntegerValue(0)), addr(IntegerValue(0))), interpreter::Error);
}

void
TestInterpreterBinaryExecution::testExecuteComparison()
{
    // This is a subset of testCompare
    // - null
    TS_ASSERT_EQUALS(-1, interpreter::executeComparison(interpreter::biCompareEQ, 0, 0));

    // - integers
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareNE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareGE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareGT, addr(IntegerValue(1)), addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareLE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareLT, addr(IntegerValue(1)), addr(IntegerValue(1))));

    // - strings
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareEQ, addr(StringValue("a")), addr(StringValue("A"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareEQ_NC, addr(StringValue("a")), addr(StringValue("A"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareNE, addr(StringValue("a")), addr(StringValue("A"))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareNE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareGE, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareGE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareGT, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareGT_NC, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareLE, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareLE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    TS_ASSERT_EQUALS(0, interpreter::executeComparison(interpreter::biCompareLT, addr(StringValue("a")), addr(StringValue("B"))));
    TS_ASSERT_EQUALS(1, interpreter::executeComparison(interpreter::biCompareLT_NC, addr(StringValue("a")), addr(StringValue("B"))));

    // Error - type
    TS_ASSERT_THROWS(interpreter::executeComparison(interpreter::biCompareEQ, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);

    // Error - wrong opcode
    TS_ASSERT_THROWS(interpreter::executeComparison(interpreter::biAdd, addr(IntegerValue(1)), addr(IntegerValue(1))), interpreter::Error);
}

