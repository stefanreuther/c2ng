/**
  *  \file u/t_interpreter_values.cpp
  *  \brief Test for interpreter::Values
  */

#include "interpreter/values.hpp"

#include "t_interpreter.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/errorvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/visitor.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/error.hpp"

/** Test toString for strings. */
void
TestInterpreterValues::testStringToString()
{
    // ex IntValueTestSuite::testString

    // Verify printed form
    {
        afl::data::StringValue sv("foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"foo\"");
    }
    {
        afl::data::StringValue sv("");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"\"");
    }
    {
        afl::data::StringValue sv("'foo'foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "'foo'foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"'foo'foo\"");
    }
    {
        afl::data::StringValue sv("\"foo\"foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\"foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "'\"foo\"foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\\foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "'\"foo\\foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\\foo'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"\\\"foo\\\\foo'\"");
    }
    {
        afl::data::StringValue sv("foo\"bar'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "foo\"bar'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"foo\\\"bar'\"");
    }
    {
        afl::data::StringValue sv("a\nb");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "a\nb");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"a\\nb\"");
    }

    // FIXME: verify serialisation
}

/** Test toString with others (BaseValue). */
void
TestInterpreterValues::testOtherToString()
{
    class TheBaseValue : public interpreter::BaseValue {
     public:
        virtual String_t toString(bool readable) const
            { return readable ? "READ" : "NON"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { }
        virtual TheBaseValue* clone() const
            { return new TheBaseValue(); }
    };
    class TheOtherValue : public afl::data::Value {
     public:
        virtual void visit(afl::data::Visitor& visitor) const
            { visitor.visitOther(*this); }
        virtual TheOtherValue* clone() const
            { return new TheOtherValue(); }
    };
    {
        TheBaseValue bv;
        TS_ASSERT_EQUALS(interpreter::toString(&bv, false), "NON");
        TS_ASSERT_EQUALS(interpreter::toString(&bv, true), "READ");
    }
    {
        TheOtherValue ov;
        TS_ASSERT_EQUALS(interpreter::toString(&ov, false), "#<unknown>");
        TS_ASSERT_EQUALS(interpreter::toString(&ov, true), "#<unknown>");
    }
}

/** Test toString with integers. */
void
TestInterpreterValues::testIntToString()
{
    {
        afl::data::IntegerValue iv(99);
        TS_ASSERT_EQUALS(interpreter::toString(&iv, false), "99");
        TS_ASSERT_EQUALS(interpreter::toString(&iv, true), "99");
    }
    {
        afl::data::IntegerValue iv(0);
        TS_ASSERT_EQUALS(interpreter::toString(&iv, false), "0");
        TS_ASSERT_EQUALS(interpreter::toString(&iv, true), "0");
    }
    {
        afl::data::IntegerValue iv(-42);
        TS_ASSERT_EQUALS(interpreter::toString(&iv, false), "-42");
        TS_ASSERT_EQUALS(interpreter::toString(&iv, true), "-42");
    }
}

/** Test toString with bools. */
void
TestInterpreterValues::testBoolToString()
{
    {
        afl::data::BooleanValue bv(false);
        TS_ASSERT_EQUALS(interpreter::toString(&bv, false), "NO");
        TS_ASSERT_EQUALS(interpreter::toString(&bv, true), "False");
    }
    {
        afl::data::BooleanValue bv(true);
        TS_ASSERT_EQUALS(interpreter::toString(&bv, false), "YES");
        TS_ASSERT_EQUALS(interpreter::toString(&bv, true), "True");
    }
}

/** Test toString with floats. */
void
TestInterpreterValues::testFloatToString()
{
    {
        afl::data::FloatValue fv(2.5);
        TS_ASSERT_EQUALS(interpreter::toString(&fv, false), "2.5");
        TS_ASSERT_EQUALS(interpreter::toString(&fv, true), "2.5");
    }
    {
        afl::data::FloatValue fv(0);
        TS_ASSERT_EQUALS(interpreter::toString(&fv, false), "0");
        TS_ASSERT_EQUALS(interpreter::toString(&fv, true), "0");
    }
    {
        afl::data::FloatValue fv(-1.25);
        TS_ASSERT_EQUALS(interpreter::toString(&fv, false), "-1.25");
        TS_ASSERT_EQUALS(interpreter::toString(&fv, true), "-1.25");
    }
}

/** Test some other toString. */
void
TestInterpreterValues::testMiscToString()
{
    // Null
    {
        TS_ASSERT_EQUALS(interpreter::toString(0, false), "");
        TS_ASSERT_EQUALS(interpreter::toString(0, true), "Z(0)");
    }

    // afl::data types
    {
        afl::data::HashValue hv(afl::data::Hash::create());
        TS_ASSERT_EQUALS(interpreter::toString(&hv, false), "#<hash>");
        TS_ASSERT_EQUALS(interpreter::toString(&hv, true), "#<hash>");
    }
    {
        afl::data::VectorValue vv(afl::data::Vector::create());
        TS_ASSERT_EQUALS(interpreter::toString(&vv, false), "#<vector>");
        TS_ASSERT_EQUALS(interpreter::toString(&vv, true), "#<vector>");
    }

    // Error
    {
        afl::data::ErrorValue ev("source", "boom");
        TS_ASSERT_THROWS(interpreter::toString(&ev, false), interpreter::Error);
        TS_ASSERT_THROWS(interpreter::toString(&ev, true), interpreter::Error);
    }
}
