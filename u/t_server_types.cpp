/**
  *  \file u/t_server_types.cpp
  *  \brief Test for server::Types
  */

#include "server/types.hpp"

#include "t_server.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/invaliddataexception.hpp"

/** Test toInteger. */
void
TestServerTypes::testToInteger()
{
    {
        TS_ASSERT_EQUALS(server::toInteger(0), 0);
    }
    {
        const afl::data::IntegerValue iv(42);
        TS_ASSERT_EQUALS(server::toInteger(&iv), 42);
    }
    {
        const afl::data::StringValue sv("");
        TS_ASSERT_EQUALS(server::toInteger(&sv), 0);
    }
    {
        const afl::data::StringValue sv("7");
        TS_ASSERT_EQUALS(server::toInteger(&sv), 7);
    }
    {
        const afl::data::StringValue sv("2.5");
        TS_ASSERT_THROWS(server::toInteger(&sv), afl::except::InvalidDataException);
    }
    {
        const afl::data::StringValue sv("-9");
        TS_ASSERT_EQUALS(server::toInteger(&sv), -9);
    }
}

/** Test toString. */
void
TestServerTypes::testToString()
{
    {
        TS_ASSERT_EQUALS(server::toString(0), "");
    }
    {
        const afl::data::IntegerValue iv(42);
        TS_ASSERT_EQUALS(server::toString(&iv), "42");
    }
    {
        const afl::data::StringValue sv("");
        TS_ASSERT_EQUALS(server::toString(&sv), "");
    }
    {
        const afl::data::StringValue sv("7");
        TS_ASSERT_EQUALS(server::toString(&sv), "7");
    }
    {
        const afl::data::StringValue sv("hi mom");
        TS_ASSERT_EQUALS(server::toString(&sv), "hi mom");
    }
}

/** Test time. */
void
TestServertypes::testTime()
{
    // unpack->pack roundtrip
    TS_ASSERT_EQUALS(server::packTime(server::unpackTime(10000)), 10000);
    TS_ASSERT_EQUALS(server::packTime(server::unpackTime(24802980)), 24802980);

    // pack->unpack roundtrip
    TS_ASSERT_EQUALS(server::unpackTime(server::packTime(afl::sys::Time::fromUnixTime(1485689224))), afl::sys::Time::fromUnixTime(1485689220));
}

