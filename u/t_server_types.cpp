/**
  *  \file u/t_server_types.cpp
  *  \brief Test for server::Types
  */

#include "server/types.hpp"

#include "t_server.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
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

/** Test addOptionalIntegerKey(), addOptionalStringKey(), toOptionalString(), toOptionalInteger(). */
void
TestServerTypes::testOptional()
{
    afl::data::Hash::Ref_t h = afl::data::Hash::create();

    // addOptional
    server::addOptionalStringKey(*h, "ks", afl::base::Optional<String_t>("known"));
    server::addOptionalStringKey(*h, "us", afl::base::Optional<String_t>());
    server::addOptionalIntegerKey(*h, "ki", afl::base::Optional<int32_t>(77));
    server::addOptionalIntegerKey(*h, "ui", afl::base::Optional<int32_t>());

    TS_ASSERT(h->get("ks") != 0);
    TS_ASSERT(h->get("us") == 0);
    TS_ASSERT_EQUALS(server::toString(h->get("ks")), "known");

    TS_ASSERT(h->get("ki") != 0);
    TS_ASSERT(h->get("ui") == 0);
    TS_ASSERT_EQUALS(server::toInteger(h->get("ki")), 77);

    // toOptional
    afl::data::StringValue sv("sv");
    afl::data::IntegerValue iv(99);
    TS_ASSERT_EQUALS(server::toOptionalString(&sv).orElse("x"), "sv");
    TS_ASSERT_EQUALS(server::toOptionalString(0).orElse("x"), "x");
    TS_ASSERT_EQUALS(server::toOptionalInteger(&iv).orElse(-1), 99);
    TS_ASSERT_EQUALS(server::toOptionalInteger(0).orElse(-1), -1);

    TS_ASSERT_EQUALS(server::toOptionalString(h->get("ks")).orElse("x"), "known");
    TS_ASSERT_EQUALS(server::toOptionalString(h->get("us")).orElse("x"), "x");
    TS_ASSERT_EQUALS(server::toOptionalInteger(h->get("ki")).orElse(-1), 77);
    TS_ASSERT_EQUALS(server::toOptionalInteger(h->get("ui")).orElse(-1), -1);
}

