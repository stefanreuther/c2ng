/**
  *  \file test/interpreter/unaryexecutiontest.cpp
  *  \brief Test for interpreter::UnaryExecution
  */

#include "interpreter/unaryexecution.hpp"

#include <stdexcept>
#include "afl/data/access.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
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
using interpreter::executeUnaryOperation;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;

        TestHarness()
            : log(), tx(), fileSystem(), world(log, tx, fileSystem)
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
AFL_TEST("interpreter.UnaryExecution:invalid", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    AFL_CHECK_THROWS(a, executeUnaryOperation(h.world, 0xFF, 0), interpreter::Error);
}

/** Test unNot: logical negation (ternary logic). */
AFL_TEST("interpreter.UnaryExecution:unNot", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(IntegerValue(1))));
    a.checkEqual("11", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(IntegerValue(0))));
    a.checkEqual("21", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(StringValue("huhu"))));
    a.checkEqual("31", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(StringValue(""))));
    a.checkEqual("41", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot, addr(HashValue(Hash::create()))));
    a.checkEqual("51", toBoolean(p), false);
}

/** Test unBool: conversion to bool aka double negation (ternary logic). */
AFL_TEST("interpreter.UnaryExecution:unBool", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(IntegerValue(1))));
    a.checkEqual("11", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(IntegerValue(0))));
    a.checkEqual("21", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(StringValue("huhu"))));
    a.checkEqual("31", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(StringValue(""))));
    a.checkEqual("41", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unBool, addr(HashValue(Hash::create()))));
    a.checkEqual("51", toBoolean(p), true);
}

/** Test unNeg: arithmetic negation. */
AFL_TEST("interpreter.UnaryExecution:unNeg", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(IntegerValue(42))));
    a.checkEqual("11", toInteger(p), -42);

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(FloatValue(-2.5))));
    a.checkEqual("21", toFloat(p), 2.5);

    p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(BooleanValue(true))));
    a.checkEqual("31", toInteger(p), -1);

    AFL_CHECK_THROWS(a("41. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unNeg, addr(StringValue("x")))), interpreter::Error);
}

/** Test unPos: arithmetic equivalence (numbers only). */
AFL_TEST("interpreter.UnaryExecution:unPos", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(IntegerValue(42))));
    a.checkEqual("11", toInteger(p), 42);

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(FloatValue(-2.5))));
    a.checkEqual("21", toFloat(p), -2.5);

    p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(BooleanValue(true))));
    a.checkEqual("31", toInteger(p), 1);

    AFL_CHECK_THROWS(a("41. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unPos, addr(StringValue("x")))), interpreter::Error);
}

/** Test unSin: sine. */
AFL_TEST("interpreter.UnaryExecution:unSin", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(0))));
    a.checkNear("11", toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(0))));
    a.checkNear("21", toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(90))));
    a.checkNear("31", toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(90.0))));
    a.checkNear("41", toFloat(p), 1.0, 0.000001);

    // Type error
    AFL_CHECK_THROWS(a("51. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("61. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(IntegerValue(1000000000)))), interpreter::Error);
    AFL_CHECK_THROWS(a("62. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unSin, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unCos: cosine. */
AFL_TEST("interpreter.UnaryExecution:unCos", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(0))));
    a.checkNear("11", toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(0))));
    a.checkNear("21", toFloat(p), 1.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(90))));
    a.checkNear("31", toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(90.0))));
    a.checkNear("41", toFloat(p), 0.0, 0.000001);

    // Type error
    AFL_CHECK_THROWS(a("51. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("61. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(IntegerValue(1000000000)))), interpreter::Error);
    AFL_CHECK_THROWS(a("62. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unCos, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unTan: tangent. */
AFL_TEST("interpreter.UnaryExecution:unTan", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(0))));
    a.checkNear("11", toFloat(p), 0.0, 0.000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(0))));
    a.checkNear("21", toFloat(p), 0.0, 0.000001);

    // Divide by zero
    AFL_CHECK_THROWS(a("31. divide by zero"), p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(90)))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. divide by zero"), p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(90.0)))), interpreter::Error);

    // Type error
    AFL_CHECK_THROWS(a("41. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("51. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(IntegerValue(1000000000)))), interpreter::Error);
    AFL_CHECK_THROWS(a("52. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unTan, addr(FloatValue(1.0e9)))), interpreter::Error);
}

/** Test unZap: convert falsy to null. */
AFL_TEST("interpreter.UnaryExecution:unZap", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unZap, 0));
    a.checkNull("01", p.get());

    // Int
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(IntegerValue(0))));
    a.checkNull("11", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(IntegerValue(17))));
    a.checkEqual("12", toInteger(p), 17);

    // Float
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(FloatValue(0.0))));
    a.checkNull("21", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(FloatValue(17))));
    a.checkEqual("22", toFloat(p), 17.0);

    // String
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(StringValue(""))));
    a.checkNull("31", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(StringValue("hi"))));
    a.checkEqual("32", toString(p), "hi");

    // Other
    p.reset(executeUnaryOperation(h.world, interpreter::unZap, addr(HashValue(Hash::create()))));
    a.checkNonNull("41", dynamic_cast<HashValue*>(p.get()));
}

/** Test unAbs: absolute value. */
AFL_TEST("interpreter.UnaryExecution:unAbs", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(IntegerValue(-42))));
    a.checkEqual("11", toInteger(p), 42);
    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(IntegerValue(99))));
    a.checkEqual("12", toInteger(p), 99);

    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(FloatValue(-2.5))));
    a.checkEqual("21", toFloat(p), 2.5);
    p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(FloatValue(77))));
    a.checkEqual("22", toFloat(p), 77.0);

    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unAbs, addr(StringValue("x")))), interpreter::Error);
}

/** Test unExp: e^x. */
AFL_TEST("interpreter.UnaryExecution:unExp", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(IntegerValue(1))));
    a.checkNear("11", toFloat(p), 2.718281828, 0.0000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(FloatValue(2))));
    a.checkNear("21", toFloat(p), 7.389056099, 0.0000001);

    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unExp, addr(StringValue("x")))), interpreter::Error);
}

/** Test unLog: log(x). */
AFL_TEST("interpreter.UnaryExecution:unLog", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(IntegerValue(1))));
    a.checkNear("11", toFloat(p), 0.0, 0.0000001);

    p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(FloatValue(2.718281828))));
    a.checkNear("21", toFloat(p), 1.0, 0.0000001);

    // Type error
    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("41. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(IntegerValue(-1)))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unLog, addr(FloatValue(-1)))), interpreter::Error);
}

/** Test unBitNot: bitwise negation. */
AFL_TEST("interpreter.UnaryExecution:unBitNot", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(IntegerValue(1))));
    a.checkEqual("11", toInteger(p), -2);

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(IntegerValue(0xFFFF0000))));
    a.checkEqual("21", toInteger(p), 0x0000FFFF);

    p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(BooleanValue(true))));
    a.checkEqual("31", toInteger(p), -2);

    // Type error
    AFL_CHECK_THROWS(a("41. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(StringValue("x")))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unBitNot, addr(FloatValue(1)))), interpreter::Error);
}

/** Test unIsEmpty: check emptiness. */
AFL_TEST("interpreter.UnaryExecution:unIsEmpty", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, 0));
    a.checkEqual("01", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(IntegerValue(0))));
    a.checkEqual("11", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(FloatValue(1))));
    a.checkEqual("12", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(StringValue("2"))));
    a.checkEqual("13", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsEmpty, addr(HashValue(Hash::create()))));
    a.checkEqual("14", toBoolean(p), false);
}

/** Test unIsNum: check for numeric argument. */
AFL_TEST("interpreter.UnaryExecution:unIsNum", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null is not numeric!
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, 0));
    a.checkEqual("01", toBoolean(p), false);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(IntegerValue(0))));
    a.checkEqual("11", toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(BooleanValue(true))));
    a.checkEqual("12", toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(FloatValue(2.0))));
    a.checkEqual("13", toBoolean(p), true);

    // Others
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(StringValue("3"))));
    a.checkEqual("21", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsNum, addr(HashValue(Hash::create()))));
    a.checkEqual("22", toBoolean(p), false);
}

/** Test unIsString: check for string argument. */
AFL_TEST("interpreter.UnaryExecution:unIsString", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null is not a string!
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, 0));
    a.checkEqual("01", toBoolean(p), false);

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(StringValue("3"))));
    a.checkEqual("11", toBoolean(p), true);

    // Others
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(IntegerValue(0))));
    a.checkEqual("21", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(BooleanValue(true))));
    a.checkEqual("22", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(FloatValue(2.0))));
    a.checkEqual("23", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsString, addr(HashValue(Hash::create()))));
    a.checkEqual("24", toBoolean(p), false);
}

/** Test unAsc: string to character code. */
AFL_TEST("interpreter.UnaryExecution:unAsc", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, 0));
    a.checkNull("01", p.get());

    // Strings
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue(""))));
    a.checkNull("11", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("A"))));
    a.checkEqual("12", toInteger(p), 65);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("ABC"))));
    a.checkEqual("13", toInteger(p), 65);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("\xC3\x96"))));
    a.checkEqual("14", toInteger(p), 214);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(StringValue("\xC3\x96XYZ"))));
    a.checkEqual("15", toInteger(p), 214);

    // Not-strings: stringify
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(IntegerValue(42))));
    a.checkEqual("21", toInteger(p), 52);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(FloatValue(42.0))));
    a.checkEqual("22", toInteger(p), 52);
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(HashValue(Hash::create()))));
    a.checkEqual("23", toInteger(p), 35);      // "#<hash>"
    p.reset(executeUnaryOperation(h.world, interpreter::unAsc, addr(BooleanValue(true))));
    a.checkEqual("24", toInteger(p), 89);      // "YES"
}

/** Test unChr: character code to string. */
AFL_TEST("interpreter.UnaryExecution:unChr", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unChr, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(65))));
    a.checkEqual("11", toString(p), "A");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(1025))));
    a.checkEqual("12", toString(p), "\xd0\x81");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(0x10FFFF))));  // UNICODE_MAX
    a.checkEqual("13", toString(p), "\xf4\x8f\xbf\xbf");
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(0))));
    a.checkEqual("14", toString(p), String_t("", 1));
    p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(BooleanValue(true))));
    a.checkEqual("15", toString(p), "\1");

    // Range error
    AFL_CHECK_THROWS(a("21. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(-1)))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(IntegerValue(2000000)))), interpreter::Error);

    // Type error
    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unChr, addr(StringValue("")))), interpreter::Error);
}

/** Test unStr: stringify everything. */
AFL_TEST("interpreter.UnaryExecution:unStr", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unStr, 0));
    a.checkNull("01", p.get());

    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(IntegerValue(65))));
    a.checkEqual("11", toString(p), "65");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(BooleanValue(false))));
    a.checkEqual("12", toString(p), "NO");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(StringValue("hi mom"))));
    a.checkEqual("13", toString(p), "hi mom");
    p.reset(executeUnaryOperation(h.world, interpreter::unStr, addr(HashValue(Hash::create()))));
    a.checkEqual("14", toString(p), "#<hash>");
}

/** Test unSqrt: square root. */
AFL_TEST("interpreter.UnaryExecution:unSqrt", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(0))));
    a.checkNear("11", toFloat(p), 0.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(1))));
    a.checkNear("12", toFloat(p), 1.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(BooleanValue(1))));
    a.checkNear("13", toFloat(p), 1.0, 0.0000001);
    p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(FloatValue(9.0))));
    a.checkNear("14", toFloat(p), 3.0, 0.0000001);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("31. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(IntegerValue(-1)))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unSqrt, addr(FloatValue(-1)))), interpreter::Error);
}

/** Test unTrunc: conversion to integer by truncation. */
AFL_TEST("interpreter.UnaryExecution:unTrunc", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(0))));
    a.checkEqual("11", toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(1))));
    a.checkEqual("12", toInteger(p), 1);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(IntegerValue(-99999))));
    a.checkEqual("13", toInteger(p), -99999);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(3.7))));
    a.checkEqual("14", toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(-42.1))));
    a.checkEqual("15", toInteger(p), -42);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("31. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(3000000000.0)))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unTrunc, addr(FloatValue(-3000000000.0)))), interpreter::Error);
}

/** Test unRound: conversion to integer by rounding. */
AFL_TEST("interpreter.UnaryExecution:unRound", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unRound, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(IntegerValue(1))));
    a.checkEqual("11", toInteger(p), 1);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(IntegerValue(-99999))));
    a.checkEqual("12", toInteger(p), -99999);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3.7))));
    a.checkEqual("13", toInteger(p), 4);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3.2))));
    a.checkEqual("14", toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(2.5))));
    a.checkEqual("15", toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.7))));
    a.checkEqual("16", toInteger(p), -43);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.1))));
    a.checkEqual("17", toInteger(p), -42);
    p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-42.5))));
    a.checkEqual("18", toInteger(p), -43);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(StringValue("x")))), interpreter::Error);

    // Range error
    AFL_CHECK_THROWS(a("31. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(3000000000.0)))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. range error"), p.reset(executeUnaryOperation(h.world, interpreter::unRound, addr(FloatValue(-3000000000.0)))), interpreter::Error);
}

/** Test unLTrim: truncate left whitespace. */
AFL_TEST("interpreter.UnaryExecution:unLTrim", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(StringValue("foo"))));
    a.checkEqual("11", toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(StringValue("  x  y  "))));
    a.checkEqual("12", toString(p), "x  y  ");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unLTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unRTrim: truncate right whitespace. */
AFL_TEST("interpreter.UnaryExecution:unRTrim", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(StringValue("foo"))));
    a.checkEqual("11", toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(StringValue("  x  y  "))));
    a.checkEqual("12", toString(p), "  x  y");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unRTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLRTrim: truncate left and right whitespace. */
AFL_TEST("interpreter.UnaryExecution:unLRTrim", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("foo"))));
    a.checkEqual("11", toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("  x  y  "))));
    a.checkEqual("12", toString(p), "x  y");
    p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(StringValue("\tx\n"))));
    a.checkEqual("13", toString(p), "x");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unLRTrim, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLength: get string length. */
AFL_TEST("interpreter.UnaryExecution:unLength", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLength, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(StringValue("foo"))));
    a.checkEqual("11", toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(StringValue("\xd0\x81"))));
    a.checkEqual("12", toInteger(p), 1);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unLength, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unVal: parse string as number. */
AFL_TEST("interpreter.UnaryExecution:unVal", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unVal, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("3"))));
    a.checkEqual("11", toInteger(p), 3);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   27   "))));
    a.checkEqual("12", toInteger(p), 27);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   -5   "))));
    a.checkEqual("13", toInteger(p), -5);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("+7 "))));
    a.checkEqual("14", toInteger(p), 7);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("   27.25   "))));
    a.checkEqual("15", toFloat(p), 27.25);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("+99.0"))));
    a.checkEqual("16", toFloat(p), 99);
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue(".5"))));
    a.checkEqual("17", toFloat(p), 0.5);

    // Invalid values
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("0x3"))));
    a.checkNull("21", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("1.2.3"))));
    a.checkNull("22", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue(""))));
    a.checkNull("23", p.get());
    p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(StringValue("1.0e5"))));
    a.checkNull("24", p.get());

    // Type error
    AFL_CHECK_THROWS(a("31. tpey error"), p.reset(executeUnaryOperation(h.world, interpreter::unVal, addr(IntegerValue(3)))), interpreter::Error);
}

/** Test unTrace: write a log message. */
AFL_TEST("interpreter.UnaryExecution:unTrace", a)
{
    afl::test::LogListener log;
    TestHarness h;
    std::auto_ptr<Value> p;
    h.log.addListener(log);
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);

    p.reset(executeUnaryOperation(h.world, interpreter::unTrace, 0));
    a.checkNull("11. result", p.get());
    a.checkEqual("12. getNumMessages", log.getNumMessages(), 1U);

    p.reset(executeUnaryOperation(h.world, interpreter::unTrace, addr(IntegerValue(3))));
    a.checkEqual("21. result", toInteger(p), 3);
    a.checkEqual("22. getNumMessages", log.getNumMessages(), 2U);
}

/** Test unNot2: logical negation (binary logic). */
AFL_TEST("interpreter.UnaryExecution:unNot2", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, 0));
    a.checkEqual("01", toBoolean(p), true);                     // <- difference to unNot

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(IntegerValue(1))));
    a.checkEqual("11", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(IntegerValue(0))));
    a.checkEqual("21", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(StringValue("huhu"))));
    a.checkEqual("31", toBoolean(p), false);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(StringValue(""))));
    a.checkEqual("41", toBoolean(p), true);

    p.reset(executeUnaryOperation(h.world, interpreter::unNot2, addr(HashValue(Hash::create()))));
    a.checkEqual("51", toBoolean(p), false);
}

/** Test unAtom: internalize strings. */
AFL_TEST("interpreter.UnaryExecution:unAtom", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    int32_t aa = h.world.atomTable().getAtomFromString("aa");
    int32_t bb = h.world.atomTable().getAtomFromString("7");
    a.checkDifferent("01. different results", aa, bb);

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, 0));
    a.checkNull("11", p.get());

    // Values
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue(""))));
    a.checkEqual("21", toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue("aa"))));
    a.checkEqual("22", toInteger(p), aa);
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(IntegerValue(7))));
    a.checkEqual("23", toInteger(p), bb);

    // Create a new one
    p.reset(executeUnaryOperation(h.world, interpreter::unAtom, addr(StringValue("new"))));
    a.checkDifferent("31", toInteger(p), aa);
    a.checkDifferent("32", toInteger(p), bb);
    a.checkEqual("33", h.world.atomTable().getStringFromAtom(toInteger(p)), "new");
}

/** Test unAtomStr: get internalized strings. */
AFL_TEST("interpreter.UnaryExecution:unAtomStr", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    int32_t av = h.world.atomTable().getAtomFromString("aa");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, 0));
    a.checkNull("01", p.get());

    // Values
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(0))));
    a.checkEqual("11", toString(p), "");
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(av))));
    a.checkEqual("12", toString(p), "aa");
    p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(IntegerValue(av+2))));
    a.checkEqual("13", toString(p), "");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(FloatValue(7.0)))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unAtomStr, addr(StringValue("")))), interpreter::Error);
}

/** Test unKeyCreate: create keymap from string. */
AFL_TEST("interpreter.UnaryExecution:unKeyCreate", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;
    h.world.keymaps().createKeymap("TESTER");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, 0));
    a.checkNull("01", p.get());

    // Create
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(StringValue("MOO"))));
    a.checkNonNull("11", p.get());
    a.checkNonNull("12", dynamic_cast<KeymapValue*>(p.get()));
    a.checkNonNull("13", h.world.keymaps().getKeymapByName("MOO"));

    // Error - exists
    AFL_CHECK_THROWS(a("21. exists"), p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(StringValue("TESTER")))), std::runtime_error);

    // Type error
    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unKeyCreate, addr(IntegerValue(99)))), interpreter::Error);
}

/** Test unKeyLookup: get keymap from string. */
AFL_TEST("interpreter.UnaryExecution:unKeyLookup", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;
    h.world.keymaps().createKeymap("TESTER");

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, 0));
    a.checkNull("01", p.get());

    // Lookup
    p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(StringValue("TESTER"))));
    a.checkNonNull("11", p.get());
    a.checkNonNull("12", dynamic_cast<KeymapValue*>(p.get()));

    // Error, does not exist
    AFL_CHECK_THROWS(a("21. does not exist"), p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(StringValue("MOO")))), interpreter::Error);

    // Type error
    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unKeyLookup, addr(IntegerValue(99)))), interpreter::Error);
}

/** Test unInc: increment numerical. */
AFL_TEST("interpreter.UnaryExecution:unInc", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, 0));
    a.checkNull("01", p.get());

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(BooleanValue(true))));
    a.checkEqual("11", toInteger(p), 2);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(IntegerValue(23))));
    a.checkEqual("12", toInteger(p), 24);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(FloatValue(10))));
    a.checkEqual("13", toFloat(p), 11.0);
    p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(FloatValue(2.5))));
    a.checkEqual("14", toFloat(p), 3.5);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unInc, addr(StringValue("x")))), interpreter::Error);
}

/** Test unDec: decrement numerical. */
AFL_TEST("interpreter.UnaryExecution:unDec", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, 0));
    a.checkNull("01", p.get());

    // Numbers
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(BooleanValue(false))));
    a.checkEqual("11", toInteger(p), -1);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(IntegerValue(23))));
    a.checkEqual("12", toInteger(p), 22);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(FloatValue(10))));
    a.checkEqual("13", toFloat(p), 9.0);
    p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(FloatValue(2.5))));
    a.checkEqual("14", toFloat(p), 1.5);

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unDec, addr(StringValue("x")))), interpreter::Error);
}

/** Test unIsProcedure: check for CallableValue/isProcedureCall descendant. */
AFL_TEST("interpreter.UnaryExecution:unIsProcedure", a)
{
    // A mock CallableValue
    class TestCV : public interpreter::CallableValue {
     public:
        TestCV(bool isProc)
            : m_isProc(isProc)
            { }
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { throw std::runtime_error("call"); }
        virtual bool isProcedureCall() const
            { return m_isProc; }
        virtual int getDimension(int32_t) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { throw std::runtime_error("makeFirstContext"); }
        virtual TestCV* clone() const
            { return new TestCV(m_isProc); }
        virtual String_t toString(bool) const
            { throw std::runtime_error("toString"); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("store"); }
     private:
        bool m_isProc;
    };

    // Some BCOs
    interpreter::BCORef_t procBCO = interpreter::BytecodeObject::create(true);
    interpreter::BCORef_t funcBCO = interpreter::BytecodeObject::create(false);

    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, 0));
    a.checkNull("01", p.get());

    // Non-Procedures
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(BooleanValue(false))));
    a.checkEqual("11", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(IntegerValue(77))));
    a.checkEqual("12", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(TestCV(false))));
    a.checkEqual("13", toBoolean(p), false);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(SubroutineValue(funcBCO))));
    a.checkEqual("14", toBoolean(p), false);

    // Procedures
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(TestCV(true))));
    a.checkEqual("21", toBoolean(p), true);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsProcedure, addr(SubroutineValue(procBCO))));
    a.checkEqual("22", toBoolean(p), true);
}

/** Test unFileNr: scalar to FileValue. */
AFL_TEST("interpreter.UnaryExecution:unFileNr", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    // Null
    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, 0));
    a.checkNull("01", p.get());

    // Valid
    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(IntegerValue(7))));
    a.checkNonNull("11", dynamic_cast<FileValue*>(p.get()));
    a.checkEqual("12", dynamic_cast<FileValue*>(p.get())->getFileNumber(), 7);

    p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(FileValue(12))));
    a.checkNonNull("21", dynamic_cast<FileValue*>(p.get()));
    a.checkEqual("22", dynamic_cast<FileValue*>(p.get())->getFileNumber(), 12);

    // Invalid
    AFL_CHECK_THROWS(a("31. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unFileNr, addr(StringValue("x")))), interpreter::Error);
}

/** Test unIsArray: check for array (=get number of dimensions). */
AFL_TEST("interpreter.UnaryExecution:unIsArray", a)
{
    // A mock CallableValue
    class TestCV : public interpreter::CallableValue {
     public:
        TestCV(int32_t numDims)
            : m_numDims(numDims)
            { }
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { throw std::runtime_error("call"); }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int getDimension(int32_t n) const
            { return n == 0 ? m_numDims : 1; }
        virtual interpreter::Context* makeFirstContext()
            { throw std::runtime_error("makeFirstContext"); }
        virtual TestCV* clone() const
            { return new TestCV(m_numDims); }
        virtual String_t toString(bool) const
            { throw std::runtime_error("toString"); return String_t(); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("store"); }
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
    a.checkNull("01", p.get());

    // Arrays
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(TestCV(4))));
    a.checkEqual("11", toInteger(p), 4);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(ArrayValue(d))));
    a.checkEqual("12", toInteger(p), 2);

    // Non-arrays
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(TestCV(0))));
    a.checkEqual("21", toInteger(p), 0);
    p.reset(executeUnaryOperation(h.world, interpreter::unIsArray, addr(StringValue("a"))));
    a.checkEqual("22", toInteger(p), 0);
}

/** Test unUCase: string to upper-case. */
AFL_TEST("interpreter.UnaryExecution:unUCase", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(StringValue("foo"))));
    a.checkEqual("11", toString(p), "FOO");
    p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(StringValue(" a Bc d"))));
    a.checkEqual("12", toString(p), " A BC D");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unUCase, addr(IntegerValue(42)))), interpreter::Error);
}

/** Test unLCase: string to lower-case. */
AFL_TEST("interpreter.UnaryExecution:unLCase", a)
{
    TestHarness h;
    std::auto_ptr<Value> p;

    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, 0));
    a.checkNull("01", p.get());

    // Valid values
    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(StringValue("Foo"))));
    a.checkEqual("11", toString(p), "foo");
    p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(StringValue(" a Bc d"))));
    a.checkEqual("12", toString(p), " a bc d");

    // Type error
    AFL_CHECK_THROWS(a("21. type error"), p.reset(executeUnaryOperation(h.world, interpreter::unLCase, addr(IntegerValue(42)))), interpreter::Error);
}
