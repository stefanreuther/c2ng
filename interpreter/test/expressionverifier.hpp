/**
  *  \file interpreter/test/expressionverifier.hpp
  *  \brief Class interpreter::test::ExpressionVerifier
  */
#ifndef C2NG_INTERPRETER_TEST_EXPRESSIONVERIFIER_HPP
#define C2NG_INTERPRETER_TEST_EXPRESSIONVERIFIER_HPP

#include "afl/test/assert.hpp"

namespace interpreter { namespace test {

    /** Helper for verifying expression compilation/execution.
        In addition to methods to compile and execute code,
        this provides state consisting of 3 integer variables A,B,C
        that can be used in test expressions.

        All methods throw an AssertionFailedException on error. */
    class ExpressionVerifier {
     public:
        class TestContext;
        friend class TestContext;

        /** Constructor.
            \param a Location information */
        ExpressionVerifier(afl::test::Assert a);

        /** Get variable value.
            \param index Index [0,2]
            \return value */
        int32_t get(size_t index) const;

        /** Set variable value.
            \param index Index [0,2]
            \param value New value */
        void set(size_t index, int32_t value);

        /** Set all variables to zero. */
        void clear();

        /** Verify that expression parses, compiles and executes successfully and produces an integer result.
            \param expr Expression
            \param result Expected result */
        void verifyInteger(const char* expr, int result);

        /** Verify that expression parses, compiles and executes successfully and produces a boolean result.
            \param expr Expression
            \param result Expected result */
        void verifyBoolean(const char* expr, bool result);

        /** Verify that expression parses, compiles and executes successfully and produces a file descriptor result.
            \param expr Expression
            \param result Expected result */
        void verifyFile(const char* expr, int result);

        /** Verify that expression parses, compiles and executes successfully and produces a null result.
            \param expr Expression */
        void verifyNull(const char* expr);

        /** Verify that expression parses, compiles and executes successfully and produces a string result.
            \param expr Expression
            \param result Expected result */
        void verifyString(const char* expr, const char* result);

        /** Verify that expression parses, compiles and executes successfully and produces a float result.
            The result is permitted to differ by 0.01 from the required value.
            \param expr Expression
            \param result Expected result */
        void verifyFloat(const char* expr, double result);

        /** Verify that expression fails during execution.
            The expression must parse and compile, but not execute.
            \param expr Expression */
        void verifyExecutionError(const char* expr);

        /** Verify that expression fails during compilation.
            The expression must parse (i.e. produce a valid parse tree), but must not compile (i.e. generate code).
            \param expr Expression */
        void verifyCompileError(const char* expr);

        /** Verify that expression fails to parse.
            The expression must fail during parsing.
            Note that incomplete parsing (i.e. lone ")") is not considered a parse failure here and will therefore fail the test.
            \param expr Expression */
        void verifyParseError(const char* expr);

        /** Verify that statement compiles and executes correctly.
            Given a (possibly multi-line) statement, Verifies that
            - the statement compiles into anything but an expression statement
              (expressions are converted to statements by the compiler using the ExpressionsAreStatements flag)
            - run correctly

            \param stmt Statements, separated by '\n' */
        void verifyStatement(const char* stmt);

     private:
        static const size_t NUM_VALUES = 3;

        afl::test::Assert m_assert;
        int32_t m_values[NUM_VALUES];

        void verifyScalar(const char* expr, int result, bool isBool);
    };

} }

#endif
