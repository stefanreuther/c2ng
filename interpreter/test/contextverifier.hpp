/**
  *  \file interpreter/test/contextverifier.hpp
  *  \brief Class interpreter::test::ContextVerifier
  */
#ifndef C2NG_INTERPRETER_TEST_CONTEXTVERIFIER_HPP
#define C2NG_INTERPRETER_TEST_CONTEXTVERIFIER_HPP

#include "interpreter/context.hpp"
#include "afl/test/assert.hpp"
#include "afl/data/value.hpp"

namespace interpreter { namespace test {

    /** Context verifier.
        Utilities to verify a Context implementation. */
    class ContextVerifier {
     public:
        /** Constructor.
            \param ctx Context to verify
            \param as  Asserter; will trigger exceptions when verification fails */
        ContextVerifier(Context& ctx, afl::test::Assert as);

        /** Verify types of all properties.
            Enumerates properties using enumProperties(),
            and verifies that all properties are resolveable to a matching type. */
        void verifyTypes() const;

        /** Verify integer property.
            Look up the named property and check that it produces the desired integer value.
            \param name  Name
            \param value Expected value */
        void verifyInteger(const char* name, int value) const;

        /** Verify boolean property.
            Look up the named property and check that it produces the desired boolean value.
            \param name  Name
            \param value Expected value */
        void verifyBoolean(const char* name, bool value) const;

        /** Verify string property.
            Look up the named property and check that it produces the desired string value.
            \param name  Name
            \param value Expected value */
        void verifyString(const char* name, const char* value) const;

        /** Verify null property.
            Look up the named property and check that it is null.
            \param name Name */
        void verifyNull(const char* name) const;

        /** Get property value.
            Look up the named property and return its value.
            \param name Name
            \return Newly-allocated value */
        afl::data::Value* getValue(const char* name) const;

     private:
        Context& m_context;
        afl::test::Assert m_assert;
    };

} }

#endif
