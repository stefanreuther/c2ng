/**
  *  \file test/server/typestest.cpp
  *  \brief Test for server::Types
  */

#include "server/types.hpp"

#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "afl/test/testrunner.hpp"

/** Test toInteger. */
AFL_TEST("server.Types:toInteger:null", a) {
    a.checkEqual("", server::toInteger(0), 0);
}
AFL_TEST("server.Types:toInteger:IntegerValue", a) {
    const afl::data::IntegerValue iv(42);
    a.checkEqual("", server::toInteger(&iv), 42);
}
AFL_TEST("server.Types:toInteger:StringValue:empty", a) {
    const afl::data::StringValue sv("");
    a.checkEqual("", server::toInteger(&sv), 0);
}
AFL_TEST("server.Types:toInteger:StringValue:numeric", a) {
    const afl::data::StringValue sv("7");
    a.checkEqual("", server::toInteger(&sv), 7);
}
AFL_TEST("server.Types:toInteger:StringValue:float", a) {
    const afl::data::StringValue sv("2.5");
    AFL_CHECK_THROWS(a, server::toInteger(&sv), afl::except::InvalidDataException);
}
AFL_TEST("server.Types:toInteger:StringValue:negative", a) {
    const afl::data::StringValue sv("-9");
    a.checkEqual("", server::toInteger(&sv), -9);
}

/** Test toString. */
AFL_TEST("server.Types:toString:null", a) {
    a.checkEqual("", server::toString(0), "");
}
AFL_TEST("server.Types:toString:IntegerValue", a) {
    const afl::data::IntegerValue iv(42);
    a.checkEqual("", server::toString(&iv), "42");
}
AFL_TEST("server.Types:toString:StringValue:empty", a) {
    const afl::data::StringValue sv("");
    a.checkEqual("", server::toString(&sv), "");
}
AFL_TEST("server.Types:toString:StringValue:numeric", a) {
    const afl::data::StringValue sv("7");
    a.checkEqual("", server::toString(&sv), "7");
}
AFL_TEST("server.Types:toString:StringValue:generic", a) {
    const afl::data::StringValue sv("hi mom");
    a.checkEqual("", server::toString(&sv), "hi mom");
}

/** Test time. */
AFL_TEST("server.Types:time", a)
{
    // unpack->pack roundtrip
    a.checkEqual("01", server::packTime(server::unpackTime(10000)), 10000);
    a.checkEqual("02", server::packTime(server::unpackTime(24802980)), 24802980);

    // pack->unpack roundtrip
    a.check("11", server::unpackTime(server::packTime(afl::sys::Time::fromUnixTime(1485689224))) == afl::sys::Time::fromUnixTime(1485689220));
}

/** Test addOptionalIntegerKey(), addOptionalStringKey(), toOptionalString(), toOptionalInteger(). */
AFL_TEST("server.Types:optional", a)
{
    afl::data::Hash::Ref_t h = afl::data::Hash::create();

    // addOptional
    server::addOptionalStringKey(*h, "ks", afl::base::Optional<String_t>("known"));
    server::addOptionalStringKey(*h, "us", afl::base::Optional<String_t>());
    server::addOptionalIntegerKey(*h, "ki", afl::base::Optional<int32_t>(77));
    server::addOptionalIntegerKey(*h, "ui", afl::base::Optional<int32_t>());

    a.checkNonNull("01", h->get("ks"));
    a.checkNull("02", h->get("us"));
    a.checkEqual("03", server::toString(h->get("ks")), "known");

    a.checkNonNull("11", h->get("ki"));
    a.checkNull("12", h->get("ui"));
    a.checkEqual("13", server::toInteger(h->get("ki")), 77);

    // toOptional
    afl::data::StringValue sv("sv");
    afl::data::IntegerValue iv(99);
    a.checkEqual("21", server::toOptionalString(&sv).orElse("x"), "sv");
    a.checkEqual("22", server::toOptionalString(0).orElse("x"), "x");
    a.checkEqual("23", server::toOptionalInteger(&iv).orElse(-1), 99);
    a.checkEqual("24", server::toOptionalInteger(0).orElse(-1), -1);

    a.checkEqual("31", server::toOptionalString(h->get("ks")).orElse("x"), "known");
    a.checkEqual("32", server::toOptionalString(h->get("us")).orElse("x"), "x");
    a.checkEqual("33", server::toOptionalInteger(h->get("ki")).orElse(-1), 77);
    a.checkEqual("34", server::toOptionalInteger(h->get("ui")).orElse(-1), -1);
}
