/**
  *  \file u/t_interpreter_error.cpp
  *  \brief Test for interpreter::Error
  */

#include "interpreter/error.hpp"

#include "t_interpreter.hpp"

namespace {
    /* Verify freshly-created Error instance */
    void verifyInstance(interpreter::Error e)
    {
        // Must have nonempty text
        const char* w = e.what();
        TS_ASSERT(w != 0);
        TS_ASSERT(*w != '\0');

        // Must have empty trace
        TS_ASSERT_EQUALS(e.getTrace(), "");
    }
}


/** Test "addTrace" function. */
void
TestInterpreterError::testTrace()
{
    interpreter::Error testee("Hi");
    TS_ASSERT(testee.what() != 0);
    TS_ASSERT_EQUALS(String_t("Hi"), testee.what());
    TS_ASSERT_EQUALS(testee.getTrace(), "");

    testee.addTrace("line 1");
    TS_ASSERT_EQUALS(testee.getTrace(), "line 1");

    testee.addTrace("file 7");
    TS_ASSERT_EQUALS(testee.getTrace(), "line 1\nfile 7");

    // Copy must preserve everything
    interpreter::Error copy(testee);
    TS_ASSERT_EQUALS(String_t("Hi"), copy.what());
    TS_ASSERT_EQUALS(copy.getTrace(), "line 1\nfile 7");
}

/** Test instances. */
void
TestInterpreterError::testInstances()
{
    verifyInstance(interpreter::Error("Hi"));
    verifyInstance(interpreter::Error::unknownIdentifier("FOO"));
    verifyInstance(interpreter::Error::typeError());
    verifyInstance(interpreter::Error::typeError(interpreter::Error::ExpectString));
    verifyInstance(interpreter::Error::internalError("boom"));
    verifyInstance(interpreter::Error::notSerializable());
    verifyInstance(interpreter::Error::notAssignable());
    verifyInstance(interpreter::Error::rangeError());
    verifyInstance(interpreter::Error::invalidMultiline());
    verifyInstance(interpreter::Error::expectKeyword("a"));
    verifyInstance(interpreter::Error::expectKeyword("a", "b"));
    verifyInstance(interpreter::Error::expectSymbol("+"));
    verifyInstance(interpreter::Error::expectSymbol("+", "-"));
    verifyInstance(interpreter::Error::misplacedKeyword("End"));
    verifyInstance(interpreter::Error::garbageAtEnd(false));
    verifyInstance(interpreter::Error::garbageAtEnd(true));
    verifyInstance(interpreter::Error::expectIdentifier("name"));
    verifyInstance(interpreter::Error::contextError());
}
