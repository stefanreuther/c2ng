/**
  *  \file u/t_interpreter_values.cpp
  *  \brief Test for interpreter::Values
  */

#include "interpreter/values.hpp"

#include "t_interpreter.hpp"
#include "afl/data/stringvalue.hpp"

void
TestInterpreterValues::testStringToString()
{
    // ex IntValueTestSuite::testString

    // Verify printed form
    {
        afl::data::StringValue sv("foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"foo\"");
    }
    {
        afl::data::StringValue sv("");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"\"");
    }
    {
        afl::data::StringValue sv("'foo'foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "'foo'foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"'foo'foo\"");
    }
    {
        afl::data::StringValue sv("\"foo\"foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\"foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "'\"foo\"foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\\foo");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "'\"foo\\foo'");
    }
    {
        afl::data::StringValue sv("\"foo\\foo'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "\"foo\\foo'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"\\\"foo\\\\foo'\"");
    }
    {
        afl::data::StringValue sv("foo\"bar'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, false), "foo\"bar'");
        TS_ASSERT_EQUALS(interpreter::toString(&sv, true), "\"foo\\\"bar'\"");
    }

    // FIXME: verify serialisation
}
