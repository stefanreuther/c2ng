/**
  *  \file test/server/console/environmenttest.cpp
  *  \brief Test for server::console::Environment
  */

#include "server/console/environment.hpp"

#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Test named value operation. */
AFL_TEST("server.console.Environment:named", a)
{
    using server::console::Environment;
    Environment testee;

    // Initial state
    a.checkNull("01. get a", testee.get("a"));
    a.checkNull("02. get b", testee.get("b"));

    // Add a value
    testee.setNew("a", Environment::ValuePtr_t(server::makeIntegerValue(38)));
    a.checkNonNull("11. get a", testee.get("a"));
    a.checkNull   ("12. get b", testee.get("b"));
    a.checkEqual  ("13. int a", server::toInteger(testee.get("a")), 38);

    // Push values
    Environment::ValuePtr_t oldA(testee.pushNew("a", Environment::ValuePtr_t(server::makeIntegerValue(7))));
    Environment::ValuePtr_t oldB(testee.pushNew("b", Environment::ValuePtr_t(server::makeIntegerValue(8))));
    a.checkNonNull("21. get a", testee.get("a"));
    a.checkNonNull("22. get b", testee.get("b"));
    a.checkEqual("23. int a", server::toInteger(testee.get("a")), 7);
    a.checkEqual("24. int a", server::toInteger(testee.get("b")), 8);

    // Pop values
    testee.popNew("a", oldA);
    testee.popNew("b", oldB);
    a.checkNonNull("31. get a", testee.get("a"));
    a.checkNull   ("32. get b", testee.get("b"));
    a.checkEqual  ("33. int a", server::toInteger(testee.get("a")), 38);

    // Enumerate. Must produce just a.
    afl::data::Segment result;
    testee.listContent(result);
    a.checkEqual("41. listContent size", result.size(), 2U);
    a.checkEqual("42. listContent a",    server::toString(result[0]), "a");
    a.checkEqual("43. listContent int",  server::toInteger(result[1]), 38);
}

/** Test naming errors.
    Whereas 0 is a valid variable name and accepted, positive numbers are not. */
AFL_TEST("server.console.Environment:named:error", a)
{
    using server::console::Environment;
    Environment testee;

    AFL_CHECK_SUCCEEDS(a("01. zero"), testee.setNew("0", Environment::ValuePtr_t(server::makeIntegerValue(1))));
    AFL_CHECK_THROWS(a("02. one"), testee.setNew("1", Environment::ValuePtr_t(server::makeIntegerValue(2))), std::exception);
    AFL_CHECK_THROWS(a("03. one"), testee.setNew("01", Environment::ValuePtr_t(server::makeIntegerValue(3))), std::exception);
    AFL_CHECK_THROWS(a("04. big"), testee.setNew("9999999", Environment::ValuePtr_t(server::makeIntegerValue(4))), std::exception);

    // Enumerate. Must produce just 0.
    afl::data::Segment result;
    testee.listContent(result);
    a.checkEqual("11. listContent size", result.size(), 2U);
    a.checkEqual("12. listContent 0",    server::toString(result[0]), "0");
    a.checkEqual("13. listContent int",  server::toInteger(result[1]), 1);
}

/** Test positional parameter operation. */
AFL_TEST("server.console.Environment:positional", a)
{
    using server::console::Environment;
    Environment testee;

    // No parameters set yet
    a.checkNull("01. get 1", testee.get("1"));
    a.checkNull("02. get 2", testee.get("2"));
    a.checkNull("03. get 3", testee.get("3"));

    // Set some parameters
    Environment::SegmentPtr_t p;
    {
        afl::data::Segment seg;
        seg.pushBackInteger(7);
        seg.pushBackString("q");
        seg.pushBackInteger(2);
        p = testee.pushPositionalParameters(seg);
    }

    // Verify
    a.checkEqual("11. get 1", server::toInteger(testee.get("1")), 7);
    a.checkEqual("12. get 2", server::toString(testee.get("2")), "q");
    a.checkEqual("13. get 3", server::toInteger(testee.get("3")), 2);

    // Push another set of parameters
    Environment::SegmentPtr_t q;
    {
        afl::data::Segment seg;
        seg.pushBackInteger(55);
        q = testee.pushPositionalParameters(seg);
    }

    // Verify
    a.checkEqual("21. get 1", server::toInteger(testee.get("1")), 55);
    a.checkNull ("22. get 2", testee.get("2"));
    a.checkNull ("23. get 3", testee.get("3"));

    // Enumerate. Must produce just 1.
    afl::data::Segment result;
    testee.listContent(result);
    a.checkEqual("31. listContent size", result.size(), 2U);
    a.checkEqual("32. listContent 1",   server::toString(result[0]), "1");
    a.checkEqual("33. listContent int", server::toInteger(result[1]), 55);

    // Pop once
    testee.popPositionalParameters(q);
    a.checkEqual("41. get 1", server::toInteger(testee.get("1")), 7);
    a.checkEqual("42. get 2", server::toString(testee.get("2")), "q");
    a.checkEqual("43. get 3", server::toInteger(testee.get("3")), 2);

    // Pop again
    testee.popPositionalParameters(q);
    a.checkNull("51. get 1", testee.get("1"));
    a.checkNull("52. get 2", testee.get("2"));
    a.checkNull("53. get 3", testee.get("3"));
}
