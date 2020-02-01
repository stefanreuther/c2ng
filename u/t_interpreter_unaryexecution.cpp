/**
  *  \file u/t_interpreter_unaryexecution.cpp
  *  \brief Test for interpreter::UnaryExecution
  */

#include "interpreter/unaryexecution.hpp"

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

using afl::data::Access;
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
using interpreter::executeUnaryOperation;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;

        TestHarness()
            : log(), fileSystem(), world(log, fileSystem)
            { }
    };

    // Shortcut for getting the address of a temporary
    const Value* addr(const Value& v)
    {
        return &v;
    }

    int32_t toInteger(const std::auto_ptr<Value>& p)
    {
        IntegerValue* iv = dynamic_cast<IntegerValue*>(p.get());
        if (iv == 0) {
            throw interpreter::Error::typeError();
        }
        return iv->getValue();
    }

    double toFloat(const std::auto_ptr<Value>& p)
    {
        FloatValue* fv = dynamic_cast<FloatValue*>(p.get());
        if (fv == 0) {
            throw interpreter::Error::typeError();
        }
        return fv->getValue();
    }

    bool toBoolean(const std::auto_ptr<Value>& p)
    {
        BooleanValue* bv = dynamic_cast<BooleanValue*>(p.get());
        if (bv == 0) {
            throw interpreter::Error::typeError();
        }
        return bv->getValue();
    }

    String_t toString(const std::auto_ptr<Value>& p)
    {
        StringValue* sv = dynamic_cast<StringValue*>(p.get());
        if (sv == 0) {
            throw interpreter::Error::typeError();
        }
        return sv->getValue();
    }
}

/** Test invalid opcode. */
void
TestInterpreterUnaryExecution::testInvalid()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    TS_ASSERT_THROWS(executeUnaryOperation(h.world, 0xFF, 0), interpreter::Error);
}

/** Test unNot: logical negation (ternary logic). */
void
TestInterpreterUnaryExecution::testNot()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(StringValue("huhu"))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(StringValue(""))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
}

/** Test unBool: conversion to bool aka double negation (ternary logic). */
void
TestInterpreterUnaryExecution::testBool()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(StringValue("huhu"))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(StringValue(""))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), true);
}

/** Test unNeg: arithmetic negation. */
void
TestInterpreterUnaryExecution::testNeg()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(IntegerValue(42))));
    TS_ASSERT_EQUALS(toInteger(p), -42);

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(FloatValue(-2.5))));
    TS_ASSERT_EQUALS(toFloat(p), 2.5);

    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(StringValue("x")))), interpreter::Error);
}

/** Test unPos: arithmetic equivalence (numbers only). */
void
TestInterpreterUnaryExecution::testPos()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(IntegerValue(42))));
    TS_ASSERT_EQUALS(toInteger(p), 42);

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(FloatValue(-2.5))));
    TS_ASSERT_EQUALS(toFloat(p), -2.5);

    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(StringValue("x")))), interpreter::Error);
}

/** Test unSin: sine. */
void
TestInterpreterUnaryExecution::testSin()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(90))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(90.0))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.000001);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(1000000000)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unCos: cosine. */
void
TestInterpreterUnaryExecution::testCos()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(90))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(90.0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(1000000000)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unTan: tangent. */
void
TestInterpreterUnaryExecution::testTan()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.000001);

    // Divide by zero
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(90)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(90.0)))), interpreter::Error);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(1000000000)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unZap: convert falsy to null. */
void
TestInterpreterUnaryExecution::testZap()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unZap, 0));
    TS_ASSERT(p.get() == 0);

    // Int
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(IntegerValue(0))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(IntegerValue(17))));
    TS_ASSERT_EQUALS(toInteger(p), 17);

    // Float
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(FloatValue(0.0))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(FloatValue(17))));
    TS_ASSERT_EQUALS(toFloat(p), 17.0);

    // String
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(StringValue(""))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(StringValue("hi"))));
    TS_ASSERT_EQUALS(toString(p), "hi");

    // Other
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(HashValue(Hash::create()))));
    TS_ASSERT(dynamic_cast<HashValue*>(p.get()) != 0);
}

/** Test unAbs: absolute value. */
void
TestInterpreterUnaryExecution::testAbs()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(IntegerValue(-42))));
    TS_ASSERT_EQUALS(toInteger(p), 42);
    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(IntegerValue(99))));
    TS_ASSERT_EQUALS(toInteger(p), 99);

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(FloatValue(-2.5))));
    TS_ASSERT_EQUALS(toFloat(p), 2.5);
    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(FloatValue(77))));
    TS_ASSERT_EQUALS(toFloat(p), 77.0);

    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(StringValue("x")))), interpreter::Error);
}

/** Test unExp: e^x. */
void
TestInterpreterUnaryExecution::testExp()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(IntegerValue(1))));
    TS_ASSERT_DELTA(toFloat(p), 2.718281828, 0.0000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(FloatValue(2))));
    TS_ASSERT_DELTA(toFloat(p), 7.389056099, 0.0000001);

    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(StringValue("x")))), interpreter::Error);
}

/** Test unLog: log(x). */
void
TestInterpreterUnaryExecution::testLog()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(IntegerValue(1))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.0000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(FloatValue(2.718281828))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.0000001);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(IntegerValue(-1)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(FloatValue(-1)))), interpreter::Error);
}

/** Test unBitNot: bitwise negation. */
void
TestInterpreterUnaryExecution::testBitNot()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toInteger(p), -2);

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(IntegerValue(0xFFFF0000))));
    TS_ASSERT_EQUALS(toInteger(p), 0x0000FFFF);

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toInteger(p), -2);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(StringValue("x")))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(FloatValue(1)))), interpreter::Error);
}

/** Test unIsEmpty: check emptiness. */
void
TestInterpreterUnaryExecution::testIsEmpty()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, 0));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(FloatValue(1))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(StringValue("2"))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
}

/** Test unIsNum: check for numeric argument. */
void
TestInterpreterUnaryExecution::testIsNum()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null is not numeric!
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, 0));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(FloatValue(2.0))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    // Others
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(StringValue("3"))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
}

/** Test unIsString: check for string argument. */
void
TestInterpreterUnaryExecution::testIsString()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null is not a string!
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, 0));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(StringValue("3"))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    // Others
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(FloatValue(2.0))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
}

/** Test unAsc: string to character code. */
void
TestInterpreterUnaryExecution::testAsc()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, 0));
    TS_ASSERT(p.get() == 0);

    // Strings
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue(""))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("A"))));
    TS_ASSERT_EQUALS(toInteger(p), 65);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("ABC"))));
    TS_ASSERT_EQUALS(toInteger(p), 65);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("\xC3\x96"))));
    TS_ASSERT_EQUALS(toInteger(p), 214);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("\xC3\x96XYZ"))));
    TS_ASSERT_EQUALS(toInteger(p), 214);

    // Not-strings: stringify
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(IntegerValue(42))));
    TS_ASSERT_EQUALS(toInteger(p), 52);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(FloatValue(42.0))));
    TS_ASSERT_EQUALS(toInteger(p), 52);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toInteger(p), 35);      // "#<hash>"
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toInteger(p), 89);      // "YES"
}

/** Test unChr: character code to string. */
void
TestInterpreterUnaryExecution::testChr()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unChr, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(65))));
    TS_ASSERT_EQUALS(toString(p), "A");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(1025))));
    TS_ASSERT_EQUALS(toString(p), "\xd0\x81");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(0x10FFFF))));  // UNICODE_MAX
    TS_ASSERT_EQUALS(toString(p), "\xf4\x8f\xbf\xbf");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toString(p), String_t("", 1));
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toString(p), "\1");

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(-1)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(2000000)))), interpreter::Error);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(StringValue("")))), interpreter::Error);
}

/** Test unStr: stringify everything. */
void
TestInterpreterUnaryExecution::testStr()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unStr, 0));
    TS_ASSERT(p.get() == 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(IntegerValue(65))));
    TS_ASSERT_EQUALS(toString(p), "65");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(BooleanValue(false))));
    TS_ASSERT_EQUALS(toString(p), "NO");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(StringValue("hi mom"))));
    TS_ASSERT_EQUALS(toString(p), "hi mom");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toString(p), "#<hash>");
}

/** Test unSqrt: square root. */
void
TestInterpreterUnaryExecution::testSqrt()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(0))));
    TS_ASSERT_DELTA(toFloat(p), 0.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(1))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(BooleanValue(1))));
    TS_ASSERT_DELTA(toFloat(p), 1.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(FloatValue(9.0))));
    TS_ASSERT_DELTA(toFloat(p), 3.0, 0.0000001);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(-1)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(FloatValue(-1)))), interpreter::Error);
}

/** Test unTrunc: conversion to integer by truncation. */
void
TestInterpreterUnaryExecution::testTrunc()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toInteger(p), 1);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(-99999))));
    TS_ASSERT_EQUALS(toInteger(p), -99999);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(3.7))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(-42.1))));
    TS_ASSERT_EQUALS(toInteger(p), -42);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(3000000000.0)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(-3000000000.0)))), interpreter::Error);
}

/** Test unRound: conversion to integer by rounding. */
void
TestInterpreterUnaryExecution::testRound()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unRound, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toInteger(p), 1);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(IntegerValue(-99999))));
    TS_ASSERT_EQUALS(toInteger(p), -99999);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3.7))));
    TS_ASSERT_EQUALS(toInteger(p), 4);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3.2))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(2.5))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.7))));
    TS_ASSERT_EQUALS(toInteger(p), -43);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.1))));
    TS_ASSERT_EQUALS(toInteger(p), -42);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.5))));
    TS_ASSERT_EQUALS(toInteger(p), -43);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(StringValue("x")))), interpreter::Error);

    // Range error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3000000000.0)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-3000000000.0)))), interpreter::Error);
}

/** Test unLTrim: truncate left whitespace. */
void
TestInterpreterUnaryExecution::testLTrim()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(StringValue("foo"))));
    TS_ASSERT_EQUALS(toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(StringValue("  x  y  "))));
    TS_ASSERT_EQUALS(toString(p), "x  y  ");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unRTrim: truncate right whitespace. */
void
TestInterpreterUnaryExecution::testRTrim()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(StringValue("foo"))));
    TS_ASSERT_EQUALS(toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(StringValue("  x  y  "))));
    TS_ASSERT_EQUALS(toString(p), "  x  y");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLRTrim: truncate left and right whitespace. */
void
TestInterpreterUnaryExecution::testLRTrim()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("foo"))));
    TS_ASSERT_EQUALS(toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("  x  y  "))));
    TS_ASSERT_EQUALS(toString(p), "x  y");
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("\tx\n"))));
    TS_ASSERT_EQUALS(toString(p), "x");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLength: get string length. */
void
TestInterpreterUnaryExecution::testLength()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLength, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(StringValue("foo"))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(StringValue("\xd0\x81"))));
    TS_ASSERT_EQUALS(toInteger(p), 1);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unVal: parse string as number. */
void
TestInterpreterUnaryExecution::testVal()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unVal, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("3"))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   27   "))));
    TS_ASSERT_EQUALS(toInteger(p), 27);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   -5   "))));
    TS_ASSERT_EQUALS(toInteger(p), -5);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("+7 "))));
    TS_ASSERT_EQUALS(toInteger(p), 7);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   27.25   "))));
    TS_ASSERT_EQUALS(toFloat(p), 27.25);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("+99.0"))));
    TS_ASSERT_EQUALS(toFloat(p), 99);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue(".5"))));
    TS_ASSERT_EQUALS(toFloat(p), 0.5);

    // Invalid values
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("0x3"))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("1.2.3"))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue(""))));
    TS_ASSERT(p.get() == 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("1.0e5"))));
    TS_ASSERT(p.get() == 0);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(IntegerValue(3)))), interpreter::Error);
}

/** Test unTrace: write a log message. */
void
TestInterpreterUnaryExecution::testTrace()
{
    class Listener : public afl::sys::LogListener {
     public:
        Listener()
            : m_count(0)
            { }
        virtual void handleMessage(const Message& /*msg*/)
            { ++m_count; }
        int get() const
            { return m_count; }
     private:
        int m_count;
    };

    Listener log;
    TestHarness h;
    std::auto_ptr<Value> p;
    h.log.addListener(log);
    TS_ASSERT_EQUALS(log.get(), 0);

    p.reset(executeUnaryOperation(h.world, interpreter::unTrace, 0));
    TS_ASSERT(p.get() == 0);
    TS_ASSERT_EQUALS(log.get(), 1);

    p.reset(executeUnaryOperation(h.world, interpreter::unTrace, addr(IntegerValue(3))));
    TS_ASSERT_EQUALS(toInteger(p), 3);
    TS_ASSERT_EQUALS(log.get(), 2);
}

/** Test unNot2: logical negation (binary logic). */
void
TestInterpreterUnaryExecution::testNot2()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, 0));
    TS_ASSERT_EQUALS(toBoolean(p), true);                     // <- difference to unNot

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(IntegerValue(1))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(StringValue("huhu"))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(StringValue(""))));
    TS_ASSERT_EQUALS(toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(HashValue(Hash::create()))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
}

/** Test unAtom: internalize strings. */
void
TestInterpreterUnaryExecution::testAtom()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    int32_t a = h.world.atomTable().getAtomFromString("aa");
    int32_t b = h.world.atomTable().getAtomFromString("7");
    TS_ASSERT_DIFFERS(a, b);

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, 0));
    TS_ASSERT(p.get() == 0);

    // Values
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue(""))));
    TS_ASSERT_EQUALS(toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue("aa"))));
    TS_ASSERT_EQUALS(toInteger(p), a);
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(IntegerValue(7))));
    TS_ASSERT_EQUALS(toInteger(p), b);

    // Create a new one
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue("new"))));
    TS_ASSERT_DIFFERS(toInteger(p), a);
    TS_ASSERT_DIFFERS(toInteger(p), b);
    TS_ASSERT_EQUALS(h.world.atomTable().getStringFromAtom(toInteger(p)), "new");
}

/** Test unAtomStr: get internalized strings. */
void
TestInterpreterUnaryExecution::testAtomStr()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    int32_t a = h.world.atomTable().getAtomFromString("aa");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, 0));
    TS_ASSERT(p.get() == 0);

    // Values
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(0))));
    TS_ASSERT_EQUALS(toString(p), "");
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(a))));
    TS_ASSERT_EQUALS(toString(p), "aa");
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(a+2))));
    TS_ASSERT_EQUALS(toString(p), "");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(FloatValue(7.0)))), interpreter::Error);
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(StringValue("")))), interpreter::Error);
}

/** Test unKeyCreate: create keymap from string. */
void
TestInterpreterUnaryExecution::testKeyCreate()
{
    TestHarness h;
    std::auto_ptr<Value> p;
    h.world.keymaps().createKeymap("TESTER");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, 0));
    TS_ASSERT(p.get() == 0);

    // Create
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(StringValue("MOO"))));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT(dynamic_cast<KeymapValue*>(p.get()) != 0);
    TS_ASSERT(h.world.keymaps().getKeymapByName("MOO") != 0);

    // Error - exists
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(StringValue("TESTER")))), std::runtime_error);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(IntegerValue(99)))), interpreter::Error);
}

/** Test unKeyLookup: get keymap from string. */
void
TestInterpreterUnaryExecution::testKeyLookup()
{
    TestHarness h;
    std::auto_ptr<Value> p;
    h.world.keymaps().createKeymap("TESTER");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, 0));
    TS_ASSERT(p.get() == 0);

    // Lookup
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(StringValue("TESTER"))));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT(dynamic_cast<KeymapValue*>(p.get()) != 0);

    // Error, does not exist
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(StringValue("MOO")))), interpreter::Error);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(IntegerValue(99)))), interpreter::Error);
}

/** Test unInc: increment numerical. */
void
TestInterpreterUnaryExecution::testInc()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, 0));
    TS_ASSERT(p.get() == 0);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(BooleanValue(true))));
    TS_ASSERT_EQUALS(toInteger(p), 2);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(IntegerValue(23))));
    TS_ASSERT_EQUALS(toInteger(p), 24);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(FloatValue(10))));
    TS_ASSERT_EQUALS(toFloat(p), 11.0);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(FloatValue(2.5))));
    TS_ASSERT_EQUALS(toFloat(p), 3.5);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(StringValue("x")))), interpreter::Error);
}

/** Test unInc: decrement numerical. */
void
TestInterpreterUnaryExecution::testDec()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, 0));
    TS_ASSERT(p.get() == 0);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(BooleanValue(false))));
    TS_ASSERT_EQUALS(toInteger(p), -1);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(IntegerValue(23))));
    TS_ASSERT_EQUALS(toInteger(p), 22);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(FloatValue(10))));
    TS_ASSERT_EQUALS(toFloat(p), 9.0);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(FloatValue(2.5))));
    TS_ASSERT_EQUALS(toFloat(p), 1.5);

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(StringValue("x")))), interpreter::Error);
}

/** Test unIsProcedure: check for CallableValue/isProcedureCall descendant. */
void
TestInterpreterUnaryExecution::testIsProcedure()
{
    // A mock CallableValue
    class TestCV : public interpreter::CallableValue {
     public:
        TestCV(bool isProc)
            : m_isProc(isProc)
            { }
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { TS_FAIL("call"); }
        virtual bool isProcedureCall() const
            { return m_isProc; }
        virtual int getDimension(int32_t) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { TS_FAIL("makeFirstContext"); return 0; }
        virtual TestCV* clone() const
            { return new TestCV(m_isProc); }
        virtual String_t toString(bool) const
            { TS_FAIL("toString"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("store"); }
     private:
        bool m_isProc;
    };

    // Some BCOs
    interpreter::BCORef_t procBCO = *new interpreter::BytecodeObject();
    procBCO->setIsProcedure(true);
    interpreter::BCORef_t funcBCO = *new interpreter::BytecodeObject();
    funcBCO->setIsProcedure(false);


    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, 0));
    TS_ASSERT(p.get() == 0);

    // Non-Procedures
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(BooleanValue(false))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(IntegerValue(77))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(TestCV(false))));
    TS_ASSERT_EQUALS(toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(SubroutineValue(funcBCO))));
    TS_ASSERT_EQUALS(toBoolean(p), false);

    // Procedures
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(TestCV(true))));
    TS_ASSERT_EQUALS(toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(SubroutineValue(procBCO))));
    TS_ASSERT_EQUALS(toBoolean(p), true);
}

/** Test unFileNr: scalar to FileValue. */
void
TestInterpreterUnaryExecution::testFileNr()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, 0));
    TS_ASSERT(p.get() == 0);

    // Valid
    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(IntegerValue(7))));
    TS_ASSERT(dynamic_cast<FileValue*>(p.get()) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<FileValue*>(p.get())->getFileNumber(), 7);

    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(FileValue(12))));
    TS_ASSERT(dynamic_cast<FileValue*>(p.get()) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<FileValue*>(p.get())->getFileNumber(), 12);

    // Invalid
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(StringValue("x")))), interpreter::Error);
}

/** Test unIsArray: check for array (=get number of dimensions). */
void
TestInterpreterUnaryExecution::testIsArray()
{
    // A mock CallableValue
    class TestCV : public interpreter::CallableValue {
     public:
        TestCV(int32_t numDims)
            : m_numDims(numDims)
            { }
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { TS_FAIL("call"); }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int getDimension(int32_t n) const
            { return n == 0 ? m_numDims : 1; }
        virtual interpreter::Context* makeFirstContext()
            { TS_FAIL("makeFirstContext"); return 0; }
        virtual TestCV* clone() const
            { return new TestCV(m_numDims); }
        virtual String_t toString(bool) const
            { TS_FAIL("toString"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("store"); }
     private:
        int32_t m_numDims;
    };

    // A real array
    afl::base::Ref<interpreter::ArrayData> d = *new interpreter::ArrayData();
    d->addDimension(4);
    d->addDimension(3);


    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, 0));
    TS_ASSERT(p.get() == 0);

    // Arrays
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(TestCV(4))));
    TS_ASSERT_EQUALS(toInteger(p), 4);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(ArrayValue(d))));
    TS_ASSERT_EQUALS(toInteger(p), 2);

    // Non-arrays
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(TestCV(0))));
    TS_ASSERT_EQUALS(toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(StringValue("a"))));
    TS_ASSERT_EQUALS(toInteger(p), 0);
}

/** Test unUCase: string to upper-case. */
void
TestInterpreterUnaryExecution::testUCase()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(StringValue("foo"))));
    TS_ASSERT_EQUALS(toString(p), "FOO");
    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(StringValue(" a Bc d"))));
    TS_ASSERT_EQUALS(toString(p), " A BC D");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLCase: string to lower-case. */
void
TestInterpreterUnaryExecution::testLCase()
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, 0));
    TS_ASSERT(p.get() == 0);

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(StringValue("Foo"))));
    TS_ASSERT_EQUALS(toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(StringValue(" a Bc d"))));
    TS_ASSERT_EQUALS(toString(p), " a bc d");

    // Type error
    TS_ASSERT_THROWS(p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(IntegerValue(42)))), interpreter::Error);
}
