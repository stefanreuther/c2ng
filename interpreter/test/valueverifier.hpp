/**
  *  \file interpreter/test/valueverifier.hpp
  *  \brief Class interpreter::test::ValueVerifier
  */
#ifndef C2NG_INTERPRETER_TEST_VALUEVERIFIER_HPP
#define C2NG_INTERPRETER_TEST_VALUEVERIFIER_HPP

#include "afl/test/assert.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter { namespace test {

    /** Value verifier.
        Utilities to verify a Value implementation. */
    class ValueVerifier {
     public:
        /** Constructor.
            \param v   Value; must live longer than ValueVerifier; therefore, non-const.
            \param as  Asserter; will trigger exceptions when verification fails */
        ValueVerifier(BaseValue& v, afl::test::Assert as);

        /** Verify basic properties.
            Tests toString(), clone():
            - toString() must not be empty;
            - clone() must be non-null, new object;
            - clone must stringify identically to original.
            \throw afl::except::AssertionFailedException on error */
        void verifyBasics();

        /** Verify that value is not serializable.
            Tests store():
            - must be rejected using interpreter::Error.
            \throw afl::except::AssertionFailedException on error */
        void verifyNotSerializable();

        /** Verify that value is serializable.
            Tests store().
            - must produce the given tag/value/data
            \throw afl::except::AssertionFailedException on error */
        void verifySerializable(uint16_t tag, uint32_t value, afl::base::ConstBytes_t data);

     private:
        BaseValue& m_value;
        afl::test::Assert m_assert;
    };

    /** Verify that a value is an integer.
        \param as     Asserter
        \param value  Newly-allocated value; function takes ownership
        \param expect Integer value to expect
        \throw afl::except::AssertionFailedException on error */
    void verifyNewInteger(const afl::test::Assert& as, afl::data::Value* value, int32_t expect);

    /** Verify that a value is an float.
        \param as     Asserter
        \param value  Newly-allocated value; function takes ownership
        \param expect Float value to expect
        \param delta  Permissible difference
        \throw afl::except::AssertionFailedException on error */
    void verifyNewFloat(const afl::test::Assert& as, afl::data::Value* value, double expect, double delta);

    /** Verify that a value is a boolean.
        \param as     Asserter
        \param value  Newly-allocated value; function takes ownership
        \param expect Boolean value to expect
        \throw afl::except::AssertionFailedException on error */
    void verifyNewBoolean(const afl::test::Assert& as, afl::data::Value* value, bool expect);

    /** Verify that a value is a string; return it.
        \param as     Asserter
        \param value  Newly-allocated value; function takes ownership
        \throw afl::except::AssertionFailedException on error */
    String_t verifyNewString(const afl::test::Assert& as, afl::data::Value* value);

    /** Verify that a value is a specific string.
        \param as     Asserter
        \param value  Newly-allocated value; function takes ownership
        \param expect String value to expect
        \throw afl::except::AssertionFailedException on error */
    void verifyNewString(const afl::test::Assert& as, afl::data::Value* value, const char* expect);

    /** Verify that a value is null.
        \param as    Asserter
        \param value Newly-allocated value; function takes ownership
        \throw afl::except::AssertionFailedException on error */
    void verifyNewNull(const afl::test::Assert& as, afl::data::Value* value);

} }

#endif
