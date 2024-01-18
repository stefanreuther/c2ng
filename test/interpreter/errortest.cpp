/**
  *  \file test/interpreter/errortest.cpp
  *  \brief Test for interpreter::Error
  */

#include "interpreter/error.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    /* Verify freshly-created Error instance */
    void verifyInstance(afl::test::Assert a, interpreter::Error e)
    {
        // Must have nonempty text
        const char* w = e.what();
        a.checkNonNull("01. what", w);
        a.check("02. what", *w != '\0');

        // Must have empty trace
        a.checkEqual("11. getTrace", e.getTrace(), "");
    }
}


/** Test "addTrace" function. */
AFL_TEST("interpreter.Error:addTrace", a)
{
    interpreter::Error testee("Hi");
    a.checkNonNull("01. what", testee.what());
    a.checkEqual("02. what", String_t("Hi"), testee.what());
    a.checkEqual("03. getTrace", testee.getTrace(), "");

    testee.addTrace("line 1");
    a.checkEqual("11. getTrace", testee.getTrace(), "line 1");

    testee.addTrace("file 7");
    a.checkEqual("21. getTrace", testee.getTrace(), "line 1\nfile 7");

    // Copy must preserve everything
    interpreter::Error copy(testee);
    a.checkEqual("31. what", String_t("Hi"), copy.what());
    a.checkEqual("32. getTrace", copy.getTrace(), "line 1\nfile 7");
}

/** Test instances. */
AFL_TEST("interpreter.Error:instances", a)
{
    verifyInstance(a("t01"), interpreter::Error("Hi"));
    verifyInstance(a("t01"), interpreter::Error::unknownIdentifier("FOO"));
    verifyInstance(a("t02"), interpreter::Error::typeError());
    verifyInstance(a("t03"), interpreter::Error::typeError(interpreter::Error::ExpectString));
    verifyInstance(a("t04"), interpreter::Error::typeError(interpreter::Error::ExpectArray));
    verifyInstance(a("t05"), interpreter::Error::internalError("boom"));
    verifyInstance(a("t06"), interpreter::Error::notSerializable());
    verifyInstance(a("t07"), interpreter::Error::notAssignable());
    verifyInstance(a("t08"), interpreter::Error::rangeError());
    verifyInstance(a("t09"), interpreter::Error::invalidMultiline());
    verifyInstance(a("t10"), interpreter::Error::expectKeyword("a"));
    verifyInstance(a("t11"), interpreter::Error::expectKeyword("a", "b"));
    verifyInstance(a("t12"), interpreter::Error::expectSymbol("+"));
    verifyInstance(a("t13"), interpreter::Error::expectSymbol("+", "-"));
    verifyInstance(a("t14"), interpreter::Error::misplacedKeyword("End"));
    verifyInstance(a("t15"), interpreter::Error::garbageAtEnd(false));
    verifyInstance(a("t16"), interpreter::Error::garbageAtEnd(true));
    verifyInstance(a("t17"), interpreter::Error::expectIdentifier("name"));
    verifyInstance(a("t18"), interpreter::Error::contextError());
    verifyInstance(a("t19"), interpreter::Error::tooComplex());
}
