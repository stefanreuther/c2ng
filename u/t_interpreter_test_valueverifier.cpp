/**
  *  \file u/t_interpreter_test_valueverifier.cpp
  *  \brief Test for interpreter::test::ValueVerifier
  */

#include "interpreter/test/valueverifier.hpp"

#include "t_interpreter_test.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/format.hpp"
#include "interpreter/values.hpp"

using interpreter::BaseValue;
using interpreter::SaveContext;
using interpreter::TagNode;
using interpreter::test::ValueVerifier;
using afl::except::AssertionFailedException;

/** Test validation of toString(). */
void
TestInterpreterTestValueVerifier::testFailBasicString()
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
        TS_ASSERT_THROWS_NOTHING(ValueVerifier(t, "testFailBasicString success").verifyBasics());
    }

    // Failure cases
    {
        Tester t("", "no");
        TS_ASSERT_THROWS(ValueVerifier(t, "testFailBasicString fail1").verifyBasics(), AssertionFailedException);
    }
    {
        Tester t("yes", "");
        TS_ASSERT_THROWS(ValueVerifier(t, "testFailBasicString fail2").verifyBasics(), AssertionFailedException);
    }
}

/** Test failure to clone.
    Tests the "return 0" case. The "return this" case would invoke undefined behaviour.
    Success case implicitly tested in testFailBasicString(). */
void
TestInterpreterTestValueVerifier::testFailBasicClone()
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
    TS_ASSERT_THROWS(ValueVerifier(t, "testFailBasicClone").verifyBasics(), AssertionFailedException);
}

/** Test failure to clone, toString mismatch.
    This would be an indicator for an imperfect clone. */
void
TestInterpreterTestValueVerifier::testFailBasicClonedString()
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
    TS_ASSERT_THROWS(ValueVerifier(t, "testFailBasicClonedString").verifyBasics(), AssertionFailedException);
}

/** Test non-serializability, good case. */
void
TestInterpreterTestValueVerifier::testFailSerializeGood()
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
    TS_ASSERT_THROWS_NOTHING(ValueVerifier(t, "testFailSerializeGood").verifyNotSerializable());
}

/** Test non-serializability, bad case (should be unserializable, but is in fact serializable). */
void
TestInterpreterTestValueVerifier::testFailSerializeBad()
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
    TS_ASSERT_THROWS(ValueVerifier(t, "testFailSerializeBad").verifyNotSerializable(), AssertionFailedException);
}

/** Test non-serializability, error (serialisation fails with a wrong error). */
void
TestInterpreterTestValueVerifier::testFailSerializeFail()
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
    TS_ASSERT_THROWS(ValueVerifier(t, "testFailSerializeFail").verifyNotSerializable(), AssertionFailedException);
}

/** Test verifyNewInteger(). */
void
TestInterpreterTestValueVerifier::testVerifyNewInteger()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewInteger("int ok", interpreter::makeIntegerValue(10), 10));

    TS_ASSERT_THROWS(interpreter::test::verifyNewInteger("int bad value", interpreter::makeIntegerValue(20), 10), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewInteger("int string",    interpreter::makeStringValue(""),  10), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewInteger("int null",      0,                                 10), AssertionFailedException);
}

/** Test verifyNewFloat(). */
void
TestInterpreterTestValueVerifier::testVerifyNewFloat()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewFloat("float ok",           interpreter::makeFloatValue(10), 10, 0));
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewFloat("float ok (bigger)",  interpreter::makeFloatValue(10.5), 10, 0.6));
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewFloat("float ok (smaller)", interpreter::makeFloatValue(9.5), 10, 0.6));
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewFloat("float ok (int)",     interpreter::makeIntegerValue(10), 10, 0.6));

    TS_ASSERT_THROWS(interpreter::test::verifyNewFloat("float bad value", interpreter::makeFloatValue(20),  10, 0.6), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewFloat("float string",    interpreter::makeStringValue(""), 10, 0.6), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewFloat("float null",      0,                                10, 0.6), AssertionFailedException);
}

/** Test verifyNewBoolean(). */
void
TestInterpreterTestValueVerifier::testVerifyNewBoolean()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewBoolean("bool ok", interpreter::makeBooleanValue(true), true));

    TS_ASSERT_THROWS(interpreter::test::verifyNewBoolean("bool bad value", interpreter::makeBooleanValue(false), true), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewBoolean("bool int",       interpreter::makeIntegerValue(1),     true), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewBoolean("bool string",    interpreter::makeStringValue("x"),    true), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewBoolean("bool null",      0,                                    true), AssertionFailedException);
}

/** Test verifyNewString(). */
void
TestInterpreterTestValueVerifier::testVerifyNewString()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewString("str ok", interpreter::makeStringValue("x"), "x"));
    TS_ASSERT_EQUALS(interpreter::test::verifyNewString("str ok1", interpreter::makeStringValue("x")), "x");

    TS_ASSERT_THROWS(interpreter::test::verifyNewString("str bad value", interpreter::makeStringValue("y"), "x"), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewString("str int",       interpreter::makeIntegerValue(42), "x"), AssertionFailedException);
    TS_ASSERT_THROWS(interpreter::test::verifyNewString("str null",      0,                                 "x"), AssertionFailedException);
}

/** Test verifyNewNull(). */
void
TestInterpreterTestValueVerifier::testVerifyNewNull()
{
    TS_ASSERT_THROWS_NOTHING(interpreter::test::verifyNewNull("null ok", 0));
    TS_ASSERT_THROWS(interpreter::test::verifyNewNull("null int", interpreter::makeIntegerValue(10)), AssertionFailedException);
}

