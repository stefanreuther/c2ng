/**
  *  \file test/interpreter/binaryexecutiontest.cpp
  *  \brief Test for interpreter::BinaryExecution
  */

#include "interpreter/binaryexecution.hpp"

#include <stdexcept>
#include "afl/data/access.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/errorvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
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
using afl::data::ErrorValue;
using afl::data::FloatValue;
using afl::data::Hash;
using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::data::Value;
using afl::data::Vector;
using interpreter::ArrayValue;
using interpreter::BytecodeObject;
using interpreter::FileValue;
using interpreter::HashValue;
using interpreter::KeymapValue;
using interpreter::SubroutineValue;
using interpreter::executeBinaryOperation;

namespace {
    struct TestHarness {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fileSystem;
        interpreter::World world;
        std::auto_ptr<afl::data::Value> p;

        TestHarness()
            : log(), fileSystem(), world(log, tx, fileSystem)
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

AFL_TEST("interpreter.BinaryExecution:biAnd", a)
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biAnd, 0,                     0);
    a.check("01", h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), 0);
    a.checkEqual("03", h.toBoolean(), false);

    h.exec(interpreter::biAnd, 0,                     addr(IntegerValue(0)));
    a.checkEqual("11", h.toBoolean(), false);
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(IntegerValue(0)));
    a.checkEqual("12", h.toBoolean(), false);
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(IntegerValue(0)));
    a.checkEqual("13", h.toBoolean(), false);

    h.exec(interpreter::biAnd, 0,                     addr(IntegerValue(1)));
    a.check("21", h.isNull());
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("22", h.toBoolean(), true);
    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(IntegerValue(1)));
    a.checkEqual("23", h.toBoolean(), false);

    // Type variants
    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(StringValue("x")));
    a.checkEqual("31", h.toBoolean(), true);

    h.exec(interpreter::biAnd, addr(IntegerValue(0)), addr(StringValue("x")));
    a.checkEqual("41", h.toBoolean(), false);

    h.exec(interpreter::biAnd, addr(IntegerValue(1)), addr(StringValue("")));
    a.checkEqual("51", h.toBoolean(), false);
}

AFL_TEST("interpreter.BinaryExecution:biOr", a)
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biOr, 0,                     0);
    a.check("01", h.isNull());
    h.exec(interpreter::biOr, addr(IntegerValue(1)), 0);
    a.checkEqual("02", h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), 0);
    a.check("03", h.isNull());

    h.exec(interpreter::biOr, 0,                     addr(IntegerValue(0)));
    a.check("11", h.isNull());
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(IntegerValue(0)));
    a.checkEqual("12", h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(IntegerValue(0)));
    a.checkEqual("13", h.toBoolean(), false);

    h.exec(interpreter::biOr, 0,                     addr(IntegerValue(1)));
    a.checkEqual("21", h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("22", h.toBoolean(), true);
    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(IntegerValue(1)));
    a.checkEqual("23", h.toBoolean(), true);

    // Type variants
    h.exec(interpreter::biOr, addr(IntegerValue(1)), addr(StringValue("x")));
    a.checkEqual("31", h.toBoolean(), true);

    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(StringValue("x")));
    a.checkEqual("41", h.toBoolean(), true);

    h.exec(interpreter::biOr, addr(IntegerValue(0)), addr(StringValue("")));
    a.checkEqual("51", h.toBoolean(), false);
}

AFL_TEST("interpreter.BinaryExecution:biXor", a)
{
    TestHarness h;

    // Logic table
    h.exec(interpreter::biXor, 0,                     0);
    a.check("01", h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(0)), 0);
    a.check("03", h.isNull());

    h.exec(interpreter::biXor, 0,                     addr(IntegerValue(0)));
    a.check("11", h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(IntegerValue(0)));
    a.checkEqual("12", h.toBoolean(), true);
    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(IntegerValue(0)));
    a.checkEqual("13", h.toBoolean(), false);

    h.exec(interpreter::biXor, 0,                     addr(IntegerValue(1)));
    a.check("21", h.isNull());
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("22", h.toBoolean(), false);
    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(IntegerValue(1)));
    a.checkEqual("23", h.toBoolean(), true);

    // Type variants
    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(StringValue("x")));
    a.checkEqual("31", h.toBoolean(), false);

    h.exec(interpreter::biXor, addr(IntegerValue(0)), addr(StringValue("x")));
    a.checkEqual("41", h.toBoolean(), true);

    h.exec(interpreter::biXor, addr(IntegerValue(1)), addr(StringValue("")));
    a.checkEqual("51", h.toBoolean(), true);
}

AFL_TEST("interpreter.BinaryExecution:biAdd", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biAdd, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biAdd, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(IntegerValue(7)));
    a.checkEqual("11", h.toInteger(), 8);
    h.exec(interpreter::biAdd, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 10);

    // Float
    h.exec(interpreter::biAdd, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    a.checkEqual("21", h.toFloat(), 9.0);

    // Mixed
    h.exec(interpreter::biAdd, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    a.checkEqual("31", h.toFloat(), 4.5);
    h.exec(interpreter::biAdd, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    a.checkEqual("32", h.toFloat(), 4.5);

    // String
    h.exec(interpreter::biAdd, addr(StringValue("x")), addr(StringValue("y")));
    a.checkEqual("41", h.toString(), "xy");

    // Bogus mix
    AFL_CHECK_THROWS(a("51. str+int"), h.exec(interpreter::biAdd, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("52. int+str"), h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("53. int+hash"), h.exec(interpreter::biAdd, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biSub", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biSub, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biSub, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biSub, addr(IntegerValue(10)), addr(IntegerValue(7)));
    a.checkEqual("11", h.toInteger(), 3);
    h.exec(interpreter::biSub, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 8);

    // Float
    h.exec(interpreter::biSub, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    a.checkEqual("21", h.toFloat(), -6.0);

    // Mixed
    h.exec(interpreter::biSub, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    a.checkEqual("31", h.toFloat(), -1.5);
    h.exec(interpreter::biSub, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    a.checkEqual("32", h.toFloat(), 1.5);

    // Type errors
    AFL_CHECK_THROWS(a("41. str-str"), h.exec(interpreter::biSub, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. str-int"), h.exec(interpreter::biSub, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("43. int-str"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("44. int-hash"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biMult", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMult, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biMult, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biMult, addr(IntegerValue(10)), addr(IntegerValue(7)));
    a.checkEqual("11", h.toInteger(), 70);
    h.exec(interpreter::biMult, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 9);

    // Float
    h.exec(interpreter::biMult, addr(FloatValue(1.5)), addr(FloatValue(7.5)));
    a.checkEqual("21", h.toFloat(), 11.25);

    // Mixed
    h.exec(interpreter::biMult, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    a.checkEqual("31", h.toFloat(), 4.5);
    h.exec(interpreter::biMult, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    a.checkEqual("32", h.toFloat(), 4.5);

    // Type errors
    AFL_CHECK_THROWS(a("41. str*str"), h.exec(interpreter::biMult, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. str*int"), h.exec(interpreter::biMult, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("43. int*str"), h.exec(interpreter::biMult, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("44. int*hash"), h.exec(interpreter::biMult, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biDivide", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biDivide, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biDivide, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar - integer result
    h.exec(interpreter::biDivide, addr(IntegerValue(10)), addr(IntegerValue(5)));
    a.checkEqual("11", h.toInteger(), 2);
    h.exec(interpreter::biDivide, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 9);

    // Float result
    h.exec(interpreter::biDivide, addr(IntegerValue(5)), addr(IntegerValue(10)));
    a.checkEqual("21", h.toFloat(), 0.5);

    // Float
    h.exec(interpreter::biDivide, addr(FloatValue(4.5)), addr(FloatValue(1.5)));
    a.checkEqual("31", h.toFloat(), 3.0);

    // Mixed
    h.exec(interpreter::biDivide, addr(FloatValue(1.5)), addr(IntegerValue(3)));
    a.checkEqual("41", h.toFloat(), 0.5);
    h.exec(interpreter::biDivide, addr(IntegerValue(3)), addr(FloatValue(1.5)));
    a.checkEqual("42", h.toFloat(), 2.0);

    // Type errors
    AFL_CHECK_THROWS(a("51. str/str"), h.exec(interpreter::biDivide, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("52. str/int"), h.exec(interpreter::biDivide, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("53. int/str"), h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("54. int/hash"), h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    AFL_CHECK_THROWS(a("61. int/0"), h.exec(interpreter::biDivide, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("62. float/0"), h.exec(interpreter::biDivide, addr(FloatValue(1)), addr(FloatValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biIntegerDivide", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biIntegerDivide, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(10)), addr(IntegerValue(5)));
    a.checkEqual("11", h.toInteger(), 2);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(9)), addr(IntegerValue(10)));
    a.checkEqual("12", h.toInteger(), 0);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("13", h.toInteger(), 9);
    h.exec(interpreter::biIntegerDivide, addr(IntegerValue(-12)), addr(IntegerValue(5)));
    a.checkEqual("14", h.toInteger(), -2);

    // Type errors
    AFL_CHECK_THROWS(a("21. float/float"), h.exec(interpreter::biIntegerDivide, addr(FloatValue(4.5)), addr(FloatValue(1.5))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. str/str"), h.exec(interpreter::biIntegerDivide, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str/int"), h.exec(interpreter::biIntegerDivide, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("24, int/str"), h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("25. int/hash"), h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    AFL_CHECK_THROWS(a("31. int/0"), h.exec(interpreter::biIntegerDivide, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biRemainder", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRemainder, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biRemainder, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biRemainder, addr(IntegerValue(10)), addr(IntegerValue(5)));
    a.checkEqual("11", h.toInteger(), 0);
    h.exec(interpreter::biRemainder, addr(IntegerValue(9)), addr(IntegerValue(10)));
    a.checkEqual("12", h.toInteger(), 9);
    h.exec(interpreter::biRemainder, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("13", h.toInteger(), 0);
    h.exec(interpreter::biRemainder, addr(IntegerValue(-12)), addr(IntegerValue(5)));
    a.checkEqual("14", h.toInteger(), -2);

    // Type errors
    AFL_CHECK_THROWS(a("21. float/float"), h.exec(interpreter::biRemainder, addr(FloatValue(4.5)), addr(FloatValue(1.5))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. str/str"), h.exec(interpreter::biRemainder, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str/int"), h.exec(interpreter::biRemainder, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("24. int/str"), h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("25. int/hash"), h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);

    // Divide by zero
    AFL_CHECK_THROWS(a("31. int/0"), h.exec(interpreter::biRemainder, addr(IntegerValue(1)), addr(IntegerValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biPow", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biPow, 0, addr(IntegerValue(1)));
    a.check("01", h.isNull());
    h.exec(interpreter::biPow, addr(IntegerValue(1)), 0);
    a.check("02", h.isNull());

    // Scalar
    h.exec(interpreter::biPow, addr(IntegerValue(10)), addr(IntegerValue(3)));
    a.checkEqual("11", h.toInteger(), 1000);
    h.exec(interpreter::biPow, addr(IntegerValue(9)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 9);
    h.exec(interpreter::biPow, addr(IntegerValue(0)), addr(IntegerValue(10000)));
    a.checkEqual("13", h.toInteger(), 0);
    h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(IntegerValue(10000)));
    a.checkEqual("14", h.toInteger(), 1);
    h.exec(interpreter::biPow, addr(IntegerValue(-1)), addr(IntegerValue(10000)));
    a.checkEqual("15", h.toInteger(), 1);

    // Overflow to float
    h.exec(interpreter::biPow, addr(IntegerValue(16)), addr(IntegerValue(10)));
    a.checkEqual("21", h.toFloat(), 1099511627776.0);

    // Float
    h.exec(interpreter::biPow, addr(FloatValue(1.5)), addr(IntegerValue(2)));
    a.checkEqual("31", h.toFloat(), 2.25);

    // Type errors
    AFL_CHECK_THROWS(a("41. int^float"), h.exec(interpreter::biPow, addr(IntegerValue(10)), addr(FloatValue(2.5))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. str^str"), h.exec(interpreter::biPow, addr(StringValue("x")), addr(StringValue("y"))), interpreter::Error);
    AFL_CHECK_THROWS(a("43. str^int"), h.exec(interpreter::biPow, addr(StringValue("x")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("44. int^str"), h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(StringValue("x"))), interpreter::Error);
    AFL_CHECK_THROWS(a("45. int^hash"), h.exec(interpreter::biPow, addr(IntegerValue(1)), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biConcat", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biConcat, 0, 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biConcat, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), 0);
    a.check("03", h.isNull());

    // Not null
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("11", h.toString(), "12");
    h.exec(interpreter::biConcat, addr(IntegerValue(1)), addr(StringValue("x")));
    a.checkEqual("12", h.toString(), "1x");
}

AFL_TEST("interpreter.BinaryExecution:biConcatEmpty", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biConcatEmpty, 0, 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biConcatEmpty, 0, addr(IntegerValue(1)));
    a.checkEqual("02", h.toString(), "1");
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), 0);
    a.checkEqual("03", h.toString(), "1");

    // Not null
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("11", h.toString(), "12");
    h.exec(interpreter::biConcatEmpty, addr(IntegerValue(1)), addr(StringValue("x")));
    a.checkEqual("12", h.toString(), "1x");
}

AFL_TEST("interpreter.BinaryExecution:biCompare", a)
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
        a.check("01", h.isNull());
        h.exec(RELATIONS[i], 0, addr(StringValue("x")));
        a.check("02", h.isNull());
        h.exec(RELATIONS[i], addr(StringValue("x")), 0);
        a.check("03", h.isNull());
    }

    // Integer comparisons
    h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("11", h.toBoolean(), true);
    h.exec(interpreter::biCompareEQ_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("12", h.toBoolean(), true);

    h.exec(interpreter::biCompareEQ, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("21", h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("22", h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("31", h.toBoolean(), false);
    h.exec(interpreter::biCompareNE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("32", h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("41", h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("42", h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("51", h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("52", h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("61", h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("62", h.toBoolean(), true);

    h.exec(interpreter::biCompareGT, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("71", h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("72", h.toBoolean(), false);

    h.exec(interpreter::biCompareGT, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("81", h.toBoolean(), true);
    h.exec(interpreter::biCompareGT_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("82", h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("91", h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("92", h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("101", h.toBoolean(), false);
    h.exec(interpreter::biCompareLE_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("102", h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("111", h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("112", h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("121", h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(IntegerValue(2)), addr(IntegerValue(1)));
    a.checkEqual("122", h.toBoolean(), false);

    // Float comparison (specimen only for brevity)
    h.exec(interpreter::biCompareEQ, addr(FloatValue(1)), addr(FloatValue(1)));
    a.checkEqual("131", h.toBoolean(), true);
    h.exec(interpreter::biCompareNE, addr(FloatValue(1)), addr(FloatValue(1)));
    a.checkEqual("132", h.toBoolean(), false);
    h.exec(interpreter::biCompareGT, addr(FloatValue(3)), addr(FloatValue(1)));
    a.checkEqual("133", h.toBoolean(), true);
    h.exec(interpreter::biCompareLT, addr(FloatValue(1)), addr(FloatValue(3)));
    a.checkEqual("134", h.toBoolean(), true);

    // Mixed
    h.exec(interpreter::biCompareEQ, addr(FloatValue(1)), addr(IntegerValue(1)));
    a.checkEqual("141", h.toBoolean(), true);
    h.exec(interpreter::biCompareLT, addr(IntegerValue(1)), addr(FloatValue(3.5)));
    a.checkEqual("142", h.toBoolean(), true);

    // String comparisons
    h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(StringValue("A")));
    a.checkEqual("151", h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(StringValue("a")), addr(StringValue("A")));
    a.checkEqual("152", h.toBoolean(), true);

    h.exec(interpreter::biCompareEQ, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("161", h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("162", h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(StringValue("a")), addr(StringValue("A")));
    a.checkEqual("171", h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("172", h.toBoolean(), false);

    h.exec(interpreter::biCompareNE, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("181", h.toBoolean(), true);
    h.exec(interpreter::biCompareNE_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("182", h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("191", h.toBoolean(), true);
    h.exec(interpreter::biCompareGE_NC, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("192", h.toBoolean(), true);

    h.exec(interpreter::biCompareGE, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("201", h.toBoolean(), false);
    h.exec(interpreter::biCompareGE_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("202", h.toBoolean(), true);

    h.exec(interpreter::biCompareGT, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("211", h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("212", h.toBoolean(), false);

    h.exec(interpreter::biCompareGT, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("221", h.toBoolean(), false);
    h.exec(interpreter::biCompareGT_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("222", h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("231", h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("232", h.toBoolean(), true);

    h.exec(interpreter::biCompareLE, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("241", h.toBoolean(), true);
    h.exec(interpreter::biCompareLE_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("242", h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("251", h.toBoolean(), false);
    h.exec(interpreter::biCompareLT_NC, addr(StringValue("a")), addr(StringValue("a")));
    a.checkEqual("252", h.toBoolean(), false);

    h.exec(interpreter::biCompareLT, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("261", h.toBoolean(), true);
    h.exec(interpreter::biCompareLT_NC, addr(StringValue("B")), addr(StringValue("a")));
    a.checkEqual("262", h.toBoolean(), false);

    // Bool comparisons
    h.exec(interpreter::biCompareEQ, addr(BooleanValue(true)), addr(BooleanValue(true)));
    a.checkEqual("265", h.toBoolean(), true);
    h.exec(interpreter::biCompareEQ, addr(BooleanValue(false)), addr(BooleanValue(true)));
    a.checkEqual("266", h.toBoolean(), false);
    h.exec(interpreter::biCompareEQ, addr(BooleanValue(true)), addr(IntegerValue(1)));
    a.checkEqual("267", h.toBoolean(), true);
    h.exec(interpreter::biCompareGT_NC, addr(IntegerValue(2)), addr(BooleanValue(false)));
    a.checkEqual("268", h.toBoolean(), true);

    // Errors
    AFL_CHECK_THROWS(a("271. str=int"), h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("272. str=hash"), h.exec(interpreter::biCompareEQ, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biMin", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMin, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biMin, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());
    h.exec(interpreter::biMin_NC, addr(IntegerValue(1)), 0);
    a.check("03", h.isNull());

    // Integer
    h.exec(interpreter::biMin, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("11", h.toInteger(), 1);
    h.exec(interpreter::biMin_NC, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("12", h.toInteger(), 1);
    h.exec(interpreter::biMin_NC, addr(IntegerValue(3)), addr(IntegerValue(2)));
    a.checkEqual("13", h.toInteger(), 2);

    // Float
    h.exec(interpreter::biMin, addr(FloatValue(1)), addr(FloatValue(2)));
    a.checkEqual("21", h.toFloat(), 1);

    // Mixed
    h.exec(interpreter::biMin, addr(IntegerValue(9)), addr(FloatValue(2.5)));
    a.checkEqual("31", h.toFloat(), 2.5);
    h.exec(interpreter::biMin, addr(IntegerValue(1)), addr(FloatValue(2.5)));
    a.checkEqual("32", h.toInteger(), 1);

    // String
    h.exec(interpreter::biMin, addr(StringValue("a")), addr(StringValue("B")));
    a.checkEqual("41", h.toString(), "B");
    h.exec(interpreter::biMin_NC, addr(StringValue("a")), addr(StringValue("B")));
    a.checkEqual("42", h.toString(), "a");
    h.exec(interpreter::biMin_NC, addr(StringValue("a")), addr(StringValue("A"))); // on tie, second arg wins
    a.checkEqual("43", h.toString(), "A");

    // Errors
    AFL_CHECK_THROWS(a("51. str+int"), h.exec(interpreter::biMin, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("52. str+hash"), h.exec(interpreter::biMin, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biMax", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biMax, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biMax, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());
    h.exec(interpreter::biMax_NC, addr(IntegerValue(1)), 0);
    a.check("03", h.isNull());

    // Integer
    h.exec(interpreter::biMax, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("11", h.toInteger(), 2);
    h.exec(interpreter::biMax_NC, addr(IntegerValue(1)), addr(IntegerValue(2)));
    a.checkEqual("12", h.toInteger(), 2);
    h.exec(interpreter::biMax_NC, addr(IntegerValue(3)), addr(IntegerValue(2)));
    a.checkEqual("13", h.toInteger(), 3);

    // Float
    h.exec(interpreter::biMax, addr(FloatValue(1)), addr(FloatValue(2)));
    a.checkEqual("21", h.toFloat(), 2);

    // Mixed
    h.exec(interpreter::biMax, addr(IntegerValue(9)), addr(FloatValue(2.5)));
    a.checkEqual("31", h.toInteger(), 9);
    h.exec(interpreter::biMax, addr(IntegerValue(1)), addr(FloatValue(2.5)));
    a.checkEqual("32", h.toFloat(), 2.5);

    // String
    h.exec(interpreter::biMax, addr(StringValue("a")), addr(StringValue("B")));
    a.checkEqual("41", h.toString(), "a");
    h.exec(interpreter::biMax_NC, addr(StringValue("a")), addr(StringValue("B")));
    a.checkEqual("42", h.toString(), "B");
    h.exec(interpreter::biMax_NC, addr(StringValue("a")), addr(StringValue("A"))); // on tie, second arg wins
    a.checkEqual("43", h.toString(), "A");

    // Errors
    AFL_CHECK_THROWS(a("51. str+int"), h.exec(interpreter::biMax, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("52. str+hash"), h.exec(interpreter::biMax, addr(StringValue("a")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biFirstStr", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biFirstStr, addr(StringValue("a")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biFirstStr, 0, addr(StringValue("a")));
    a.check("02", h.isNull());
    h.exec(interpreter::biFirstStr_NC, 0, addr(StringValue("a")));
    a.check("03", h.isNull());

    // Normal
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("11", h.toString(), "Rhabarber-");
    h.exec(interpreter::biFirstStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("12", h.toString(), "Rha");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    a.checkEqual("13", h.toString(), "Rhabarber-Barbara");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    a.checkEqual("14", h.toString(), "");
    h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    a.checkEqual("15", h.toString(), "");

    // Errors
    AFL_CHECK_THROWS(a("21. str+int"), h.exec(interpreter::biFirstStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biFirstStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str+hash"), h.exec(interpreter::biFirstStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biRestStr", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRestStr, addr(StringValue("a")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biRestStr, 0, addr(StringValue("a")));
    a.check("02", h.isNull());
    h.exec(interpreter::biRestStr_NC, 0, addr(StringValue("a")));
    a.check("03", h.isNull());

    // Normal
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("11", h.toString(), "bara");
    h.exec(interpreter::biRestStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("12", h.toString(), "ber-Barbara");
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    a.check("13", h.isNull());
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    a.checkEqual("14", h.toString(), "habarber-Barbara");
    h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    a.checkEqual("15", h.toString(), "Rhabarber-Barbara");

    // Errors
    AFL_CHECK_THROWS(a("21. str+int"), h.exec(interpreter::biRestStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biRestStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str+hash"), h.exec(interpreter::biRestStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biFindStr", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biFindStr, addr(StringValue("a")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biFindStr, 0, addr(StringValue("a")));
    a.check("02", h.isNull());
    h.exec(interpreter::biFindStr_NC, 0, addr(StringValue("a")));
    a.check("03", h.isNull());

    // Normal
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("11", h.toInteger(), 11);
    h.exec(interpreter::biFindStr_NC, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("Bar")));
    a.checkEqual("12", h.toInteger(), 4);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("XYZ")));
    a.checkEqual("13", h.toInteger(), 0);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("R")));
    a.checkEqual("14", h.toInteger(), 1);
    h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(StringValue("")));
    a.checkEqual("15", h.toInteger(), 1);

    // Errors
    AFL_CHECK_THROWS(a("21. str+int"), h.exec(interpreter::biFindStr, addr(StringValue("Rhabarber-Barbara")), addr(IntegerValue(3))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biFindStr, addr(IntegerValue(3)), addr(IntegerValue(33))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str+hash"), h.exec(interpreter::biFindStr, addr(StringValue("")), addr(HashValue(Hash::create()))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biBitAnd", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitAnd, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biBitAnd, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biBitAnd, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    a.checkEqual("11", h.toInteger(), 0x0F0);
    h.exec(interpreter::biBitAnd, addr(BooleanValue(1)), addr(IntegerValue(0x0FF)));
    a.checkEqual("12", h.toInteger(), 1);

    // Errors
    AFL_CHECK_THROWS(a("21. float&int"), h.exec(interpreter::biBitAnd, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. str&int"), h.exec(interpreter::biBitAnd, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. hash&int"), h.exec(interpreter::biBitAnd, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biBitOr", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitOr, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biBitOr, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biBitOr, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    a.checkEqual("11", h.toInteger(), 0xFFF);
    h.exec(interpreter::biBitOr, addr(IntegerValue(0xFF0)), addr(BooleanValue(1)));
    a.checkEqual("12", h.toInteger(), 0xFF1);

    // Errors
    AFL_CHECK_THROWS(a("21. float&int"), h.exec(interpreter::biBitOr, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. str&int"), h.exec(interpreter::biBitOr, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. hash&int"), h.exec(interpreter::biBitOr, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biBitXor", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biBitXor, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biBitXor, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biBitXor, addr(IntegerValue(0xFF0)), addr(IntegerValue(0x0FF)));
    a.checkEqual("11", h.toInteger(), 0xF0F);
    h.exec(interpreter::biBitXor, addr(BooleanValue(1)), addr(IntegerValue(0x0FF)));
    a.checkEqual("12", h.toInteger(), 0x0FE);

    // Errors
    AFL_CHECK_THROWS(a("21. float&int"), h.exec(interpreter::biBitXor, addr(FloatValue(1)), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. str&int"), h.exec(interpreter::biBitXor, addr(StringValue("")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. hash&int"), h.exec(interpreter::biBitXor, addr(HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biStr", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biStr, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biStr, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(0)));
    a.checkEqual("11", h.toString(), "42");
    h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(3)));
    a.checkEqual("12", h.toString(), "42.000");
    // The following test used to check '42.0125', but that fails on certain systems due to FP precision issues.
    // Digits produced for formatting:
    // - success: 420125000000000028413035813024123399372911080718040466308593750000000000000000000000000000000000000
    // - failure: 420124999999999992894572642398998141288757324218750000000000000000000000000000000000000000000000000
    h.exec(interpreter::biStr, addr(FloatValue(42.0126)), addr(IntegerValue(3)));
    a.checkEqual("13", h.toString(), "42.013");
    h.exec(interpreter::biStr, addr(BooleanValue(1)), addr(IntegerValue(7)));
    a.checkEqual("14", h.toString(), "YES");

    // Errors
    AFL_CHECK_THROWS(a("21. int+neg"), h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(IntegerValue(-1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+float"), h.exec(interpreter::biStr, addr(IntegerValue(42)), addr(FloatValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("23. str+int"), h.exec(interpreter::biStr, addr(StringValue("x")), addr(IntegerValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("24. hash+int"), h.exec(interpreter::biStr, addr(HashValue(Hash::create())), addr(IntegerValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biATan", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biATan, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biATan, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biATan, addr(IntegerValue(1)), addr(IntegerValue(1)));
    a.checkEqual("11", h.toFloat(), 45);
    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(IntegerValue(1)));
    a.checkEqual("12", h.toFloat(), 45);
    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(FloatValue(1)));
    a.checkEqual("13", h.toFloat(), 45);

    h.exec(interpreter::biATan, addr(FloatValue(1)), addr(FloatValue(0)));
    a.checkEqual("21", h.toFloat(), 90);
    h.exec(interpreter::biATan, addr(FloatValue(0)), addr(FloatValue(1)));
    a.checkEqual("22", h.toFloat(), 0);

    // Undefined
    h.exec(interpreter::biATan, addr(FloatValue(0)), addr(FloatValue(0)));
    a.check("31", h.isNull());

    // Errors
    AFL_CHECK_THROWS(a("41. str+int"), h.exec(interpreter::biATan, addr(StringValue("x")), addr(IntegerValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. hash+int"), h.exec(interpreter::biATan, addr(HashValue(Hash::create())), addr(IntegerValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biLCut", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biLCut, addr(StringValue("")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biLCut, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    a.checkEqual("11", h.toString(), "llo");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    a.checkEqual("12", h.toString(), "");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    a.checkEqual("13", h.toString(), "hello");
    h.exec(interpreter::biLCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    a.checkEqual("14", h.toString(), "hello");

    // Errors
    AFL_CHECK_THROWS(a("21. str+float"), h.exec(interpreter::biLCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biLCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biRCut", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biRCut, addr(StringValue("")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biRCut, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    a.checkEqual("11", h.toString(), "hel");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    a.checkEqual("12", h.toString(), "hello");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    a.checkEqual("13", h.toString(), "h");
    h.exec(interpreter::biRCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    a.checkEqual("14", h.toString(), "");

    // Errors
    AFL_CHECK_THROWS(a("21. str+float"), h.exec(interpreter::biRCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biRCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biEndCut", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biEndCut, addr(StringValue("")), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biEndCut, 0, addr(IntegerValue(1)));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(3)));
    a.checkEqual("11", h.toString(), "llo");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(99)));
    a.checkEqual("12", h.toString(), "hello");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(1)));
    a.checkEqual("13", h.toString(), "o");
    h.exec(interpreter::biEndCut, addr(StringValue("hello")), addr(IntegerValue(0)));
    a.checkEqual("14", h.toString(), "");

    // Errors
    AFL_CHECK_THROWS(a("21. str+float"), h.exec(interpreter::biEndCut, addr(StringValue("x")), addr(FloatValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+int"), h.exec(interpreter::biEndCut, addr(IntegerValue(3)), addr(IntegerValue(1))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biStrMult", a)
{
    TestHarness h;

    // Null
    h.exec(interpreter::biStrMult, addr(IntegerValue(1)), 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biStrMult, 0, addr(StringValue("")));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biStrMult, addr(IntegerValue(100000)), addr(StringValue("")));
    a.checkEqual("11", h.toString(), "");
    h.exec(interpreter::biStrMult, addr(IntegerValue(3)), addr(StringValue("x")));
    a.checkEqual("12", h.toString(), "xxx");
    h.exec(interpreter::biStrMult, addr(IntegerValue(5)), addr(StringValue("ha")));
    a.checkEqual("13", h.toString(), "hahahahaha");

    // Errors
    AFL_CHECK_THROWS(a("21. int+int"), h.exec(interpreter::biStrMult, addr(IntegerValue(5)), addr(IntegerValue(5))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. float+str"), h.exec(interpreter::biStrMult, addr(FloatValue(5)), addr(StringValue("X"))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biKeyAddParent", a)
{
    TestHarness h;
    KeymapValue ka(h.world.keymaps().createKeymap("A"));
    KeymapValue kb(h.world.keymaps().createKeymap("B"));

    // Null
    h.exec(interpreter::biKeyAddParent, &ka, 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biKeyAddParent, 0, &kb);
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biKeyAddParent, &ka, &kb);
    KeymapValue* kv = dynamic_cast<KeymapValue*>(h.p.get());
    a.checkNonNull("11", kv);
    a.check("12", kv->getKeymap() == ka.getKeymap());
    a.check("13", ka.getKeymap()->hasParent(*kb.getKeymap()));

    // Error - duplicate, loop. These are handled by util::Keymap and thus do not throw an interpreter error.
    AFL_CHECK_THROWS(a("21. dup"), h.exec(interpreter::biKeyAddParent, &ka, &kb), std::runtime_error);
    AFL_CHECK_THROWS(a("22. loop"), h.exec(interpreter::biKeyAddParent, &kb, &ka), std::runtime_error);

    // Error - types
    AFL_CHECK_THROWS(a("31. int+keymap"), h.exec(interpreter::biKeyAddParent, addr(IntegerValue(5)), &kb), interpreter::Error);
    AFL_CHECK_THROWS(a("32. keymap+int"), h.exec(interpreter::biKeyAddParent, &ka, addr(IntegerValue(5))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biKeyFind", a)
{
    TestHarness h;
    KeymapValue ka(h.world.keymaps().createKeymap("A"));
    ka.getKeymap()->addKey('q', 42, 23);

    // Null
    h.exec(interpreter::biKeyFind, &ka, 0);
    a.check("01", h.isNull());
    h.exec(interpreter::biKeyFind, 0, addr(StringValue("k")));
    a.check("02", h.isNull());

    // Normal
    h.exec(interpreter::biKeyFind, &ka, addr(StringValue("q")));  // found
    a.checkEqual("11", h.toInteger(), 42);
    h.exec(interpreter::biKeyFind, &ka, addr(StringValue("z")));  // not found
    a.check("12", h.isNull());

    // Error - invalid key name (should this actually be an error?)
    AFL_CHECK_THROWS(a("21. invalid key"), h.exec(interpreter::biKeyFind, &ka, addr(StringValue("escape meta cokebottle"))), interpreter::Error);

    // Error
    AFL_CHECK_THROWS(a("31. keymap+int"), h.exec(interpreter::biKeyFind, &ka, addr(IntegerValue(5))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. int+str"), h.exec(interpreter::biKeyFind, addr(IntegerValue(5)), addr(StringValue("y"))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:biArrayDim", a)
{
    class Tester : public interpreter::CallableValue {
     public:
        virtual void call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
            { throw std::runtime_error("call unexpected"); }
        virtual bool isProcedureCall() const
            { return false; }
        virtual size_t getDimension(size_t which) const
            { return which + 2; }
        virtual interpreter::Context* makeFirstContext()
            { throw std::runtime_error("makeFirstContext unexpected"); }
        virtual Tester* clone() const
            { throw std::runtime_error("clone unexpected"); }
        virtual String_t toString(bool /*readable*/) const
            { throw std::runtime_error("toString unexpected"); }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("store unexpected"); }
        virtual void visit(afl::data::Visitor& /*visitor*/) const
            { throw std::runtime_error("visit unexpected"); }
    };

    TestHarness h;
    Tester t;

    // Null
    h.exec(interpreter::biArrayDim, &t, 0);
    a.check("11", h.isNull());
    h.exec(interpreter::biArrayDim, 0, addr(IntegerValue(1)));
    a.check("12", h.isNull());

    // Normal
    h.exec(interpreter::biArrayDim, &t, addr(BooleanValue(1)));      // 1st dimension
    a.checkEqual("21", h.toInteger(), 3);
    h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(2)));      // 2nd dimension
    a.checkEqual("22", h.toInteger(), 4);

    // Errors - range
    AFL_CHECK_THROWS(a("31. range"), h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. range"), h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(3))), interpreter::Error);
    AFL_CHECK_THROWS(a("33. range"), h.exec(interpreter::biArrayDim, &t, addr(IntegerValue(-1))), interpreter::Error);

    // Errors - type
    AFL_CHECK_THROWS(a("41. array+float"), h.exec(interpreter::biArrayDim, &t, addr(FloatValue(0))), interpreter::Error);
    AFL_CHECK_THROWS(a("42. int+int"), h.exec(interpreter::biArrayDim, addr(IntegerValue(0)), addr(IntegerValue(0))), interpreter::Error);
}

AFL_TEST("interpreter.BinaryExecution:executeComparison", a)
{
    // This is a subset of testCompare
    // - null
    a.checkEqual("01", -1, interpreter::executeComparison(interpreter::biCompareEQ, 0, 0));

    // - integers
    a.checkEqual("11", 1, interpreter::executeComparison(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(IntegerValue(1))));
    a.checkEqual("12", 0, interpreter::executeComparison(interpreter::biCompareNE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    a.checkEqual("13", 1, interpreter::executeComparison(interpreter::biCompareGE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    a.checkEqual("14", 0, interpreter::executeComparison(interpreter::biCompareGT, addr(IntegerValue(1)), addr(IntegerValue(1))));
    a.checkEqual("15", 1, interpreter::executeComparison(interpreter::biCompareLE, addr(IntegerValue(1)), addr(IntegerValue(1))));
    a.checkEqual("16", 0, interpreter::executeComparison(interpreter::biCompareLT, addr(IntegerValue(1)), addr(IntegerValue(1))));

    // - strings
    a.checkEqual("21", 0, interpreter::executeComparison(interpreter::biCompareEQ, addr(StringValue("a")), addr(StringValue("A"))));
    a.checkEqual("22", 1, interpreter::executeComparison(interpreter::biCompareEQ_NC, addr(StringValue("a")), addr(StringValue("A"))));
    a.checkEqual("23", 1, interpreter::executeComparison(interpreter::biCompareNE, addr(StringValue("a")), addr(StringValue("A"))));
    a.checkEqual("24", 0, interpreter::executeComparison(interpreter::biCompareNE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("25", 1, interpreter::executeComparison(interpreter::biCompareGE, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("26", 1, interpreter::executeComparison(interpreter::biCompareGE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("27", 0, interpreter::executeComparison(interpreter::biCompareGT, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("28", 0, interpreter::executeComparison(interpreter::biCompareGT_NC, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("29", 1, interpreter::executeComparison(interpreter::biCompareLE, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("30", 1, interpreter::executeComparison(interpreter::biCompareLE_NC, addr(StringValue("a")), addr(StringValue("a"))));
    a.checkEqual("31", 0, interpreter::executeComparison(interpreter::biCompareLT, addr(StringValue("a")), addr(StringValue("B"))));
    a.checkEqual("32", 1, interpreter::executeComparison(interpreter::biCompareLT_NC, addr(StringValue("a")), addr(StringValue("B"))));

    // Error - type
    AFL_CHECK_THROWS(a("41. str+int"), interpreter::executeComparison(interpreter::biCompareEQ, addr(StringValue("a")), addr(IntegerValue(1))), interpreter::Error);

    // Error - wrong opcode
    AFL_CHECK_THROWS(a("51. opcode"), interpreter::executeComparison(interpreter::biAdd, addr(IntegerValue(1)), addr(IntegerValue(1))), interpreter::Error);
}

/** Invalid types must be rejected: arithmetic. */
AFL_TEST("interpreter.BinaryExecution:bad-types:arith", a)
{
    TestHarness h;

    AFL_CHECK_THROWS(a("01. hash+int"), h.exec(interpreter::biSub, addr(afl::data::HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("02. int+hash"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(afl::data::HashValue(Hash::create()))), interpreter::Error);

    AFL_CHECK_THROWS(a("11. vector+int"), h.exec(interpreter::biSub, addr(afl::data::VectorValue(Vector::create())), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("12. int+vector"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(afl::data::VectorValue(Vector::create()))), interpreter::Error);

    AFL_CHECK_THROWS(a("21. subr+int"), h.exec(interpreter::biSub, addr(SubroutineValue(BytecodeObject::create(false))), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+subr"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(SubroutineValue(BytecodeObject::create(false)))), interpreter::Error);

    AFL_CHECK_THROWS(a("31. error+int"), h.exec(interpreter::biSub, addr(ErrorValue("a", "b")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. int+error"), h.exec(interpreter::biSub, addr(IntegerValue(1)), addr(ErrorValue("a", "b"))), interpreter::Error);
}

/** Invalid types must be rejected: comparison. */
AFL_TEST("interpreter.BinaryExecution:bad-types:compare", a)
{
    TestHarness h;

    AFL_CHECK_THROWS(a("01. hash+int"), h.exec(interpreter::biCompareEQ, addr(afl::data::HashValue(Hash::create())), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("02. int+hash"), h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(afl::data::HashValue(Hash::create()))), interpreter::Error);

    AFL_CHECK_THROWS(a("11. vector+int"), h.exec(interpreter::biCompareEQ, addr(afl::data::VectorValue(Vector::create())), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("12. int+vector"), h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(afl::data::VectorValue(Vector::create()))), interpreter::Error);

    AFL_CHECK_THROWS(a("21. subr+int"), h.exec(interpreter::biCompareEQ, addr(SubroutineValue(BytecodeObject::create(false))), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("22. int+subr"), h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(SubroutineValue(BytecodeObject::create(false)))), interpreter::Error);

    AFL_CHECK_THROWS(a("31. error+int"), h.exec(interpreter::biCompareEQ, addr(ErrorValue("a", "b")), addr(IntegerValue(1))), interpreter::Error);
    AFL_CHECK_THROWS(a("32. int+error"), h.exec(interpreter::biCompareEQ, addr(IntegerValue(1)), addr(ErrorValue("a", "b"))), interpreter::Error);
}
