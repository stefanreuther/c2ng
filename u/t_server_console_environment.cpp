/**
  *  \file u/t_server_console_environment.cpp
  *  \brief Test for server::console::Environment
  */

#include "server/console/environment.hpp"

#include "t_server_console.hpp"
#include "server/types.hpp"

/** Test named value operation. */
void
TestServerConsoleEnvironment::testNamed()
{
    using server::console::Environment;
    Environment testee;

    // Initial state
    TS_ASSERT(testee.get("a") == 0);
    TS_ASSERT(testee.get("b") == 0);

    // Add a value
    testee.setNew("a", Environment::ValuePtr_t(server::makeIntegerValue(38)));
    TS_ASSERT(testee.get("a") != 0);
    TS_ASSERT(testee.get("b") == 0);
    TS_ASSERT_EQUALS(server::toInteger(testee.get("a")), 38);

    // Push values
    Environment::ValuePtr_t oldA(testee.pushNew("a", Environment::ValuePtr_t(server::makeIntegerValue(7))));
    Environment::ValuePtr_t oldB(testee.pushNew("b", Environment::ValuePtr_t(server::makeIntegerValue(8))));
    TS_ASSERT(testee.get("a") != 0);
    TS_ASSERT(testee.get("b") != 0);
    TS_ASSERT_EQUALS(server::toInteger(testee.get("a")), 7);
    TS_ASSERT_EQUALS(server::toInteger(testee.get("b")), 8);

    // Pop values
    testee.popNew("a", oldA);
    testee.popNew("b", oldB);
    TS_ASSERT(testee.get("a") != 0);
    TS_ASSERT(testee.get("b") == 0);
    TS_ASSERT_EQUALS(server::toInteger(testee.get("a")), 38);

    // Enumerate. Must produce just a.
    afl::data::Segment result;
    testee.listContent(result);
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(server::toString(result[0]), "a");
    TS_ASSERT_EQUALS(server::toInteger(result[1]), 38);
}

/** Test naming errors.
    Whereas 0 is a valid variable name and accepted, positive numbers are not. */
void
TestServerConsoleEnvironment::testNameError()
{
    using server::console::Environment;
    Environment testee;

    TS_ASSERT_THROWS_NOTHING(testee.setNew("0", Environment::ValuePtr_t(server::makeIntegerValue(1))));
    TS_ASSERT_THROWS(testee.setNew("1", Environment::ValuePtr_t(server::makeIntegerValue(2))), std::exception);
    TS_ASSERT_THROWS(testee.setNew("01", Environment::ValuePtr_t(server::makeIntegerValue(3))), std::exception);
    TS_ASSERT_THROWS(testee.setNew("9999999", Environment::ValuePtr_t(server::makeIntegerValue(4))), std::exception);

    // Enumerate. Must produce just 0.
    afl::data::Segment result;
    testee.listContent(result);
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(server::toString(result[0]), "0");
    TS_ASSERT_EQUALS(server::toInteger(result[1]), 1);
}

/** Test positional parameter operation. */
void
TestServerConsoleEnvironment::testPositional()
{
    using server::console::Environment;
    Environment testee;

    // No parameters set yet
    TS_ASSERT(testee.get("1") == 0);
    TS_ASSERT(testee.get("2") == 0);
    TS_ASSERT(testee.get("3") == 0);

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
    TS_ASSERT_EQUALS(server::toInteger(testee.get("1")), 7);
    TS_ASSERT_EQUALS(server::toString(testee.get("2")), "q");
    TS_ASSERT_EQUALS(server::toInteger(testee.get("3")), 2);

    // Push another set of parameters
    Environment::SegmentPtr_t q;
    {
        afl::data::Segment seg;
        seg.pushBackInteger(55);
        q = testee.pushPositionalParameters(seg);
    }

    // Verify
    TS_ASSERT_EQUALS(server::toInteger(testee.get("1")), 55);
    TS_ASSERT(testee.get("2") == 0);
    TS_ASSERT(testee.get("3") == 0);

    // Enumerate. Must produce just 1.
    afl::data::Segment result;
    testee.listContent(result);
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(server::toString(result[0]), "1");
    TS_ASSERT_EQUALS(server::toInteger(result[1]), 55);
    
    // Pop once
    testee.popPositionalParameters(q);
    TS_ASSERT_EQUALS(server::toInteger(testee.get("1")), 7);
    TS_ASSERT_EQUALS(server::toString(testee.get("2")), "q");
    TS_ASSERT_EQUALS(server::toInteger(testee.get("3")), 2);

    // Pop again
    testee.popPositionalParameters(q);
    TS_ASSERT(testee.get("1") == 0);
    TS_ASSERT(testee.get("2") == 0);
    TS_ASSERT(testee.get("3") == 0);
}

