/**
  *  \file test/interpreter/test/valueverifiertest.cpp
  *  \brief Test for interpreter::test::ValueVerifier
  */

#include "interpreter/test/valueverifier.hpp"

#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/values.hpp"

using interpreter::BaseValue;
using interpreter::SaveContext;
using interpreter::TagNode;
using interpreter::test::ValueVerifier;
using afl::except::AssertionFailedException;

/** Test validation of toString(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyBasics:toString", a)
{
    class Tester : public BaseValue {
     public:
        Tester(String_t yes, String_t no)
            : m_yes(yes), m_no(no)
            { }
        virtual String_t toString(bool readable) const
            { return readable ? m_yes : m_no; }
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual BaseValue* clone() const
            { return new Tester(*this); }
     private:
        String_t m_yes;
        String_t m_no;
    };

    // Success case
    {
        Tester t("yes", "no");
        AFL_CHECK_SUCCEEDS(a("01. success"), ValueVerifier(t, a("01. success")).verifyBasics());
    }

    // Failure cases
    {
        Tester t("", "no");
        AFL_CHECK_THROWS(a("11. fail1"), ValueVerifier(t, a("11. fail1")).verifyBasics(), AssertionFailedException);
    }
    {
        Tester t("yes", "");
        AFL_CHECK_THROWS(a("12. fail2"), ValueVerifier(t, a("12. fail2")).verifyBasics(), AssertionFailedException);
    }
}

/** Test failure to clone.
    Tests the "return 0" case. The "return this" case would invoke undefined behaviour.
    Success case implicitly tested in verifyBasics:toString. */
AFL_TEST("interpreter.test.ValueVerifier:verifyBasics:clone:null", a)
{
    class Tester : public BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "?"; }
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual BaseValue* clone() const
            { return 0; }
    };

    Tester t;
    AFL_CHECK_THROWS(a, ValueVerifier(t, a).verifyBasics(), AssertionFailedException);
}

/** Test failure to clone, toString mismatch.
    This would be an indicator for an imperfect clone. */
AFL_TEST("interpreter.test.ValueVerifier:verifyBasics:clone:bad-clone", a)
{
    class Tester : public BaseValue {
     public:
        Tester(int n)
            : m_n(n)
            { }
        virtual String_t toString(bool /*readable*/) const
            { return afl::string::Format("%d", m_n); }
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual BaseValue* clone() const
            { return new Tester(m_n+1); }
     private:
        int m_n;
    };

    Tester t(0);
    AFL_CHECK_THROWS(a, ValueVerifier(t, a).verifyBasics(), AssertionFailedException);
}

/** Test non-serializability, good case. */
AFL_TEST("interpreter.test.ValueVerifier:verifyNotSerializable:success", a)
{
    class Tester : public BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "?"; }
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }
        virtual BaseValue* clone() const
            { return new Tester(); }
    };

    Tester t;
    AFL_CHECK_SUCCEEDS(a, ValueVerifier(t, a).verifyNotSerializable());
}

/** Test non-serializability, bad case (should be unserializable, but is in fact serializable). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNotSerializable:mismatch", a)
{
    class Tester : public BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "?"; }
        virtual void store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
            { out.tag = 0x4000; out.value = 0x666; }
        virtual BaseValue* clone() const
            { return new Tester(); }
    };

    Tester t;
    AFL_CHECK_THROWS(a, ValueVerifier(t, a).verifyNotSerializable(), AssertionFailedException);
}

/** Test non-serializability, error (serialisation fails with a wrong error). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNotSerializable:error", a)
{
    class Tester : public BaseValue {
     public:
        virtual String_t toString(bool /*readable*/) const
            { return "?"; }
        virtual void store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
            { throw std::runtime_error("store"); }
        virtual BaseValue* clone() const
            { return new Tester(); }
    };

    Tester t;
    AFL_CHECK_THROWS(a, ValueVerifier(t, a).verifyNotSerializable(), AssertionFailedException);
}

/** Test verifyNewInteger(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNewInteger:success", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewInteger(a, interpreter::makeIntegerValue(10), 10));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewInteger:error:bad-value", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewInteger(a, interpreter::makeIntegerValue(20), 10), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewInteger:error:string", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewInteger(a, interpreter::makeStringValue(""), 10), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewInteger:error:null", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewInteger(a, 0, 10), AssertionFailedException);
}

/** Test verifyNewFloat(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:success:exact", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewFloat(a, interpreter::makeFloatValue(10), 10, 0));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:success:bigger", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewFloat(a, interpreter::makeFloatValue(10.5), 10, 0.6));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:success:smaller", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewFloat(a, interpreter::makeFloatValue(9.5), 10, 0.6));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:success:int", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewFloat(a, interpreter::makeIntegerValue(10), 10, 0.6));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:error:bad-value", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewFloat(a, interpreter::makeFloatValue(20), 10, 0.6), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:error:string", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewFloat(a, interpreter::makeStringValue(""), 10, 0.6), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewFloat:error:null", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewFloat(a, 0, 10, 0.6), AssertionFailedException);
}

/** Test verifyNewBoolean(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNewBoolean:success", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewBoolean(a, interpreter::makeBooleanValue(true), true));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewBoolean:error:bad-value", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewBoolean(a, interpreter::makeBooleanValue(false), true), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewBoolean:error:int", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewBoolean(a, interpreter::makeIntegerValue(1), true), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewBoolean:error:string", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewBoolean(a, interpreter::makeStringValue("x"), true), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewBoolean:error:null", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewBoolean(a, 0, true), AssertionFailedException);
}

/** Test verifyNewString(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNewString:success", a)
{
    // Ternary signature
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewString(a, interpreter::makeStringValue("x"), "x"));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewString:success:value", a)
{
    // Binary signature that returns a value
    a.checkEqual("value", interpreter::test::verifyNewString(a, interpreter::makeStringValue("x")), "x");
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewString:error:value", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewString(a, interpreter::makeStringValue("y"), "x"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewString:error:int", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewString(a, interpreter::makeIntegerValue(42), "x"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewString:error:null", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewString(a, 0, "x"), AssertionFailedException);
}

/** Test verifyNewNull(). */
AFL_TEST("interpreter.test.ValueVerifier:verifyNewNull:success", a)
{
    AFL_CHECK_SUCCEEDS(a, interpreter::test::verifyNewNull(a, 0));
}

AFL_TEST("interpreter.test.ValueVerifier:verifyNewNull:error", a)
{
    AFL_CHECK_THROWS(a, interpreter::test::verifyNewNull(a, interpreter::makeIntegerValue(10)), AssertionFailedException);
}
