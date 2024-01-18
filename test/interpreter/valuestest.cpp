/**
  *  \file test/interpreter/valuestest.cpp
  *  \brief Test for interpreter::Values
  */

#include "interpreter/values.hpp"

#include "afl/charset/utf8.hpp"
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
#include "afl/test/testrunner.hpp"
#include "game/types.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tokenizer.hpp"
#include <memory>

/** Test toString for strings. */
AFL_TEST("interpreter.Values:toString:StringValue", a)
{
    // ex IntValueTestSuite::testString

    // Verify printed form
    {
        afl::data::StringValue sv("foo");
        a.checkEqual("01", interpreter::toString(&sv, false), "foo");
        a.checkEqual("02", interpreter::toString(&sv, true), "\"foo\"");
    }
    {
        afl::data::StringValue sv("");
        a.checkEqual("03", interpreter::toString(&sv, false), "");
        a.checkEqual("04", interpreter::toString(&sv, true), "\"\"");
    }
    {
        afl::data::StringValue sv("'foo'foo");
        a.checkEqual("05", interpreter::toString(&sv, false), "'foo'foo");
        a.checkEqual("06", interpreter::toString(&sv, true), "\"'foo'foo\"");
    }
    {
        afl::data::StringValue sv("\"foo\"foo");
        a.checkEqual("07", interpreter::toString(&sv, false), "\"foo\"foo");
        a.checkEqual("08", interpreter::toString(&sv, true), "'\"foo\"foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo");
        a.checkEqual("09", interpreter::toString(&sv, false), "\"foo\\foo");
        a.checkEqual("10", interpreter::toString(&sv, true), "'\"foo\\foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo'");
        a.checkEqual("11", interpreter::toString(&sv, false), "\"foo\\foo'");
        a.checkEqual("12", interpreter::toString(&sv, true), "\"\\\"foo\\\\foo'\"");
    }
    {
        afl::data::StringValue sv("foo\"bar'");
        a.checkEqual("13", interpreter::toString(&sv, false), "foo\"bar'");
        a.checkEqual("14", interpreter::toString(&sv, true), "\"foo\\\"bar'\"");
    }
    {
        afl::data::StringValue sv("a\nb");
        a.checkEqual("15", interpreter::toString(&sv, false), "a\nb");
        a.checkEqual("16", interpreter::toString(&sv, true), "\"a\\nb\"");
    }

    // FIXME: verify serialisation
}

/** Test toString with others (BaseValue). */
AFL_TEST("interpreter.Values:toString:BaseValue", a)
{
    class TheBaseValue : public interpreter::BaseValue {
     public:
        virtual String_t toString(bool readable) const
            { return readable ? "READ" : "NON"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
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
        a.checkEqual("01", interpreter::toString(&bv, false), "NON");
        a.checkEqual("02", interpreter::toString(&bv, true), "READ");
    }
    {
        TheOtherValue ov;
        a.checkEqual("03", interpreter::toString(&ov, false), "#<unknown>");
        a.checkEqual("04", interpreter::toString(&ov, true), "#<unknown>");
    }
}

/** Test toString with integers. */
AFL_TEST("interpreter.Values:toString:IntegerValue", a)
{
    {
        afl::data::IntegerValue iv(99);
        a.checkEqual("01", interpreter::toString(&iv, false), "99");
        a.checkEqual("02", interpreter::toString(&iv, true), "99");
    }
    {
        afl::data::IntegerValue iv(0);
        a.checkEqual("03", interpreter::toString(&iv, false), "0");
        a.checkEqual("04", interpreter::toString(&iv, true), "0");
    }
    {
        afl::data::IntegerValue iv(-42);
        a.checkEqual("05", interpreter::toString(&iv, false), "-42");
        a.checkEqual("06", interpreter::toString(&iv, true), "-42");
    }
}

/** Test toString with bools. */
AFL_TEST("interpreter.Values:toString:BooleanValue", a)
{
    {
        afl::data::BooleanValue bv(false);
        a.checkEqual("01", interpreter::toString(&bv, false), "NO");
        a.checkEqual("02", interpreter::toString(&bv, true), "False");
    }
    {
        afl::data::BooleanValue bv(true);
        a.checkEqual("03", interpreter::toString(&bv, false), "YES");
        a.checkEqual("04", interpreter::toString(&bv, true), "True");
    }
}

/** Test toString with floats. */
AFL_TEST("interpreter.Values:toString:FloatValue", a)
{
    {
        afl::data::FloatValue fv(2.5);
        a.checkEqual("01", interpreter::toString(&fv, false), "2.5");
        a.checkEqual("02", interpreter::toString(&fv, true), "2.5");
    }
    {
        afl::data::FloatValue fv(0);
        a.checkEqual("03", interpreter::toString(&fv, false), "0");
        a.checkEqual("04", interpreter::toString(&fv, true), "0");
    }
    {
        afl::data::FloatValue fv(-1.25);
        a.checkEqual("05", interpreter::toString(&fv, false), "-1.25");
        a.checkEqual("06", interpreter::toString(&fv, true), "-1.25");
    }
}

/** Test some other toString. */
AFL_TEST("interpreter.Values:toString:others", a)
{
    // Null
    {
        a.checkEqual("01", interpreter::toString(0, false), "");
        a.checkEqual("02", interpreter::toString(0, true), "Z(0)");
    }

    // afl::data types
    {
        afl::data::HashValue hv(afl::data::Hash::create());
        a.checkEqual("11", interpreter::toString(&hv, false), "#<hash>");
        a.checkEqual("12", interpreter::toString(&hv, true), "#<hash>");
    }
    {
        afl::data::VectorValue vv(afl::data::Vector::create());
        a.checkEqual("13", interpreter::toString(&vv, false), "#<vector>");
        a.checkEqual("14", interpreter::toString(&vv, true), "#<vector>");
    }

    // Error
    {
        afl::data::ErrorValue ev("source", "boom");
        AFL_CHECK_THROWS(a("21. toString"), interpreter::toString(&ev, false), interpreter::Error);
        AFL_CHECK_THROWS(a("22. toString"), interpreter::toString(&ev, true), interpreter::Error);
    }
}

/** Test make functions. */
AFL_TEST("interpreter.Values:make", a)
{
    std::auto_ptr<afl::data::Value> p;

    // makeBooleanValue true
    p.reset(interpreter::makeBooleanValue(1));
    a.checkNonNull("01", dynamic_cast<afl::data::BooleanValue*>(p.get()));
    a.checkEqual("02", dynamic_cast<afl::data::BooleanValue*>(p.get())->getValue(), 1);
    a.checkEqual("03", interpreter::getBooleanValue(p.get()), 1);

    // makeBooleanValue false
    p.reset(interpreter::makeBooleanValue(0));
    a.checkNonNull("11", dynamic_cast<afl::data::BooleanValue*>(p.get()));
    a.checkEqual("12", dynamic_cast<afl::data::BooleanValue*>(p.get())->getValue(), 0);
    a.checkEqual("13", interpreter::getBooleanValue(p.get()), 0);

    // makeBooleanValue null
    p.reset(interpreter::makeBooleanValue(-1));
    a.checkNull("21", p.get());
    a.checkEqual("22", interpreter::getBooleanValue(p.get()), -1);

    // makeIntegerValue
    p.reset(interpreter::makeIntegerValue(42));
    a.checkNonNull("31", dynamic_cast<afl::data::IntegerValue*>(p.get()));
    a.checkEqual("32", dynamic_cast<afl::data::IntegerValue*>(p.get())->getValue(), 42);
    a.checkEqual("33", interpreter::getBooleanValue(p.get()), 1);

    // makeFloatValue
    p.reset(interpreter::makeFloatValue(3.25));
    a.checkNonNull("41", dynamic_cast<afl::data::FloatValue*>(p.get()));
    a.checkEqual("42", dynamic_cast<afl::data::FloatValue*>(p.get())->getValue(), 3.25);
    a.checkEqual("43", interpreter::getBooleanValue(p.get()), 1);

    // makeStringValue (NTBS)
    p.reset(interpreter::makeStringValue("abc"));
    a.checkNonNull("51", dynamic_cast<afl::data::StringValue*>(p.get()));
    a.checkEqual("52", dynamic_cast<afl::data::StringValue*>(p.get())->getValue(), "abc");
    a.checkEqual("53", interpreter::getBooleanValue(p.get()), 1);

    // makeStringValue (String)
    p.reset(interpreter::makeStringValue(String_t("xyz")));
    a.checkNonNull("61", dynamic_cast<afl::data::StringValue*>(p.get()));
    a.checkEqual("62", dynamic_cast<afl::data::StringValue*>(p.get())->getValue(), "xyz");
    a.checkEqual("63", interpreter::getBooleanValue(p.get()), 1);

    // makeStringValue (String)
    p.reset(interpreter::makeStringValue(String_t()));
    a.checkNonNull("71", dynamic_cast<afl::data::StringValue*>(p.get()));
    a.checkEqual("72", dynamic_cast<afl::data::StringValue*>(p.get())->getValue(), "");
    a.checkEqual("73", interpreter::getBooleanValue(p.get()), 0);

    // makeOptionalIntegerValue
    p.reset(interpreter::makeOptionalIntegerValue(game::IntegerProperty_t(9)));
    a.checkNonNull("81", dynamic_cast<afl::data::IntegerValue*>(p.get()));
    a.checkEqual("82", dynamic_cast<afl::data::IntegerValue*>(p.get())->getValue(), 9);
    a.checkEqual("83", interpreter::getBooleanValue(p.get()), 1);

    // makeOptionalIntegerValue empty
    p.reset(interpreter::makeOptionalIntegerValue(game::IntegerProperty_t()));
    a.checkNull("91", p.get());

    // makeOptionalIntegerValue [Optional]
    p.reset(interpreter::makeOptionalIntegerValue(afl::base::Optional<int32_t>(77)));
    a.checkNonNull("101", dynamic_cast<afl::data::IntegerValue*>(p.get()));
    a.checkEqual("102", dynamic_cast<afl::data::IntegerValue*>(p.get())->getValue(), 77);
    a.checkEqual("103", interpreter::getBooleanValue(p.get()), 1);

    // makeOptionalIntegerValue [empty Optional]
    p.reset(interpreter::makeOptionalIntegerValue(afl::base::Optional<int32_t>()));
    a.checkNull("111", p.get());

    // makeOptionalStringValue
    p.reset(interpreter::makeOptionalStringValue(String_t("hi")));
    a.checkNonNull("121", dynamic_cast<afl::data::StringValue*>(p.get()));
    a.checkEqual("122", dynamic_cast<afl::data::StringValue*>(p.get())->getValue(), "hi");
    a.checkEqual("123", interpreter::getBooleanValue(p.get()), 1);

    // makeOptionalIntegerValue empty
    p.reset(interpreter::makeOptionalStringValue(afl::base::Nothing));
    a.checkNull("131", p.get());

    // getBooleanValue with afl native types
    p.reset(new afl::data::HashValue(afl::data::Hash::create()));
    a.checkEqual("141", interpreter::getBooleanValue(p.get()), 1);

    p.reset(new afl::data::VectorValue(afl::data::Vector::create()));
    a.checkEqual("151", interpreter::getBooleanValue(p.get()), 1);
}

/** Test some hardcoded quoteString() values. */
AFL_TEST("interpreter.Values:quoteString", a)
{
    // Preference for double-quotes
    a.checkEqual("01", interpreter::quoteString(""), "\"\"");
    a.checkEqual("02", interpreter::quoteString("a"), "\"a\"");

    // Preference for not using backslashes
    a.checkEqual("11", interpreter::quoteString("\""), "'\"'");

    // Backslash if needed
    a.checkEqual("21", interpreter::quoteString("\"a'"), "\"\\\"a'\"");
}

/** Test quoteString() round-trip compatibility for parsing. */
AFL_TEST("interpreter.Values:quoteString:roundtrip", a)
{
    for (afl::charset::Unichar_t ch = 0; ch < 500; ++ch) {
        // String with one unicode character
        String_t s;
        afl::charset::Utf8().append(s, ch);

        // Format it
        String_t formatted = interpreter::quoteString(s);

        // Read it again
        interpreter::Tokenizer tok(formatted);
        a.checkEqual("01. getCurrentToken", tok.getCurrentToken(), interpreter::Tokenizer::tString);
        a.checkEqual("02. getCurrentString", tok.getCurrentString(), s);

        a.checkEqual("11. readNextToken", tok.readNextToken(), interpreter::Tokenizer::tEnd);
    }
}

/** Test formatFloat(). */
AFL_TEST("interpreter.Values:formatFloat", a)
{
    a.checkEqual("01", interpreter::formatFloat(1.0), "1");
    a.checkEqual("02", interpreter::formatFloat(2.5), "2.5");
    a.checkEqual("03", interpreter::formatFloat(1e10), "10000000000");

    a.checkEqual("11", interpreter::formatFloat(0.125), "0.125");
    a.checkEqual("12", interpreter::formatFloat(-0.125), "-0.125");
}
