/**
  *  \file test/server/format/formattest.cpp
  *  \brief Test for server::format::Format
  */

#include "server/format/format.hpp"

#include "afl/data/access.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/value.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include <stdexcept>

using afl::data::Access;

/*
 *  pack()
 */

// Simple string, plain
AFL_TEST("server.format.Format:pack:string", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "x");
}

// Simple string, tagged "obj"
AFL_TEST("server.format.Format:pack:string:obj", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, String_t("obj"), afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "x");
}

// Simple string with umlaut, default charset (latin1)
AFL_TEST("server.format.Format:pack:string:l1", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("\xC3\xA4");
    std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "\xE4");
}

// Simple string with umlaut, given a charset
AFL_TEST("server.format.Format:pack:string:cp437", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("\xC3\xA4");
    std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, String_t("cp437")));
    a.checkEqual("", Access(p).toString(), "\x84");
}

// Truehull, given as partial JSON
AFL_TEST("server.format.Format:pack:truehull:json", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("[[1,2,3],[4,5,6]]");
    std::auto_ptr<afl::data::Value> p(testee.pack("truehull", &sv, String_t("json"), afl::base::Nothing));

    String_t result = Access(p).toString();
    a.checkEqual("01. size", result.size(), 440U);
    a.checkEqual("02. result", result[0], 1);
    a.checkEqual("03. result", result[1], 0);
    a.checkEqual("04. result", result[2], 2);
    a.checkEqual("05. result", result[3], 0);
    a.checkEqual("06. result", result[4], 3);
    a.checkEqual("07. result", result[5], 0);
    a.checkEqual("08. result", result[40], 4);
    a.checkEqual("09. result", result[11], 0);
}

// JSON string
AFL_TEST("server.format.Format:pack:string:json", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("\"x\"");
    std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, String_t("json"), afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "x");
}

// Error: not JSON
AFL_TEST("server.format.Format:pack:error:not-json", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a, testee.pack("string", &sv, String_t("json"), afl::base::Nothing), std::runtime_error);
}

// Error: bad type name
AFL_TEST("server.format.Format:pack:error:bad-type", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.pack("what", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"),   testee.pack("", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
}

// Error: bad format name
AFL_TEST("server.format.Format:pack:error:bad-format", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.pack("string", &sv, String_t("what"), afl::base::Nothing), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"),   testee.pack("string", &sv, String_t(""), afl::base::Nothing), std::runtime_error);
}

// Error: bad charset name
AFL_TEST("server.format.Format:pack:error:bad-charset", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.pack("string", &sv, afl::base::Nothing, String_t("what")), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"), testee.pack("string", &sv, afl::base::Nothing, String_t("")), std::runtime_error);
}

/*
 *  Test unpack
 */


// Simple string, plain
AFL_TEST("server.format.Format:unpack:string", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "x");
}

// Simple string, tagged "obj"
AFL_TEST("server.format.Format:unpack:string:obj", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, String_t("obj"), afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "x");
}

// Simple string with umlaut, default charset (latin1)
AFL_TEST("server.format.Format:unpack:string:l1", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("\xE4");
    std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "\xC3\xA4");
}

// Simple string with umlaut, given a charset
AFL_TEST("server.format.Format:unpack:string:cp437", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("\x84");
    std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, String_t("cp437")));
    a.checkEqual("", Access(p).toString(), "\xC3\xA4");
}

// JSON string
AFL_TEST("server.format.Format:unpack:string:json", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, String_t("json"), afl::base::Nothing));
    a.checkEqual("", Access(p).toString(), "\"x\"");
}

// Error: bad type name
AFL_TEST("server.format.Format:unpack:error:bad-type", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.unpack("what", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"),   testee.unpack("", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
}

// Error: bad format name
AFL_TEST("server.format.Format:unpack:error:bad-format", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.unpack("string", &sv, String_t("what"), afl::base::Nothing), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"),   testee.unpack("string", &sv, String_t(""), afl::base::Nothing), std::runtime_error);
}

// Error: bad charset name
AFL_TEST("server.format.Format:unpack:error:bad-charset", a)
{
    server::format::Format testee;
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("unknown"), testee.unpack("string", &sv, afl::base::Nothing, String_t("what")), std::runtime_error);
    AFL_CHECK_THROWS(a("empty"),   testee.unpack("string", &sv, afl::base::Nothing, String_t("")), std::runtime_error);
}


/*
 *  Test unpack() with a multitude of formats.
 *  This mainly exercises the Packer factory function; the individual packers already have their test.
 */

// Engines
AFL_TEST("server.format.Format:unpack:engspec", a)
{
    server::format::Format testee;
    static const uint8_t ENGSPEC[] = {
        0x53, 0x74, 0x61, 0x72, 0x44, 0x72, 0x69, 0x76, 0x65, 0x20, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x8c, 0x0a, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xd4, 0x30,
        0x00, 0x00, 0x60, 0x54, 0x00, 0x00, 0xfc, 0x85, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0xc4, 0x1c,
        0x01, 0x00
    };
    afl::data::StringValue sv(afl::string::fromBytes(ENGSPEC));
    std::auto_ptr<afl::data::Value> p(testee.unpack("engspec", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("NAME",       Access(p)[0]("NAME").toString(), "StarDrive 1");
    a.checkEqual("FUELFACTOR", Access(p)[0]("FUELFACTOR")[9].toInteger(), 72900);
}

// Beams
AFL_TEST("server.format.Format:unpack:beamspec", a)
{
    server::format::Format testee;
    static const uint8_t BEAMSPEC[] = {
        0x4c, 0x61, 0x73, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x0a, 0x00, 0x03, 0x00
    };
    afl::data::StringValue sv(afl::string::fromBytes(BEAMSPEC));
    std::auto_ptr<afl::data::Value> p(testee.unpack("beamspec", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("NAME", Access(p)[0]("NAME").toString(), "Laser");
    a.checkEqual("KILL", Access(p)[0]("KILL").toInteger(), 10);
}

// Torpedoes
AFL_TEST("server.format.Format:torpspec", a)
{
    server::format::Format testee;
    static const uint8_t TORPSPEC[] = {
        0x4d, 0x61, 0x72, 0x6b, 0x20, 0x31, 0x20, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x6e, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x01, 0x00, 0x04, 0x00, 0x05, 0x00
    };
    afl::data::StringValue sv(afl::string::fromBytes(TORPSPEC));
    std::auto_ptr<afl::data::Value> p(testee.unpack("torpspec", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("NAME",    Access(p)[0]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("DAMAGE1", Access(p)[0]("DAMAGE1").toInteger(), 5);
}

// Hulls
AFL_TEST("server.format.Format:hullspec", a)
{
    server::format::Format testee;
    static const uint8_t HULLSPEC[] = {
        0x4e, 0x4f, 0x43, 0x54, 0x55, 0x52, 0x4e, 0x45, 0x20, 0x43, 0x4c, 0x41, 0x53, 0x53, 0x20, 0x44,
        0x45, 0x53, 0x54, 0x52, 0x4f, 0x59, 0x45, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x00,
        0x01, 0x00, 0x32, 0x00, 0x19, 0x00, 0x07, 0x00, 0xb4, 0x00, 0xbe, 0x00, 0x01, 0x00, 0x5a, 0x00,
        0x02, 0x00, 0x32, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x46, 0x00
    };
    afl::data::StringValue sv(afl::string::fromBytes(HULLSPEC));
    std::auto_ptr<afl::data::Value> p(testee.unpack("hullspec", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("NAME", Access(p)[0]("NAME").toString(), "NOCTURNE CLASS DESTROYER");
    a.checkEqual("MASS", Access(p)[0]("MASS").toInteger(), 90);
}

// Simulation
AFL_TEST("server.format.Format:unpack:sim", a)
{
    server::format::Format testee;
    afl::data::StringValue sv(afl::string::fromBytes(game::test::getSimFileV1()));
    std::auto_ptr<afl::data::Value> p(testee.unpack("sim", &sv, afl::base::Nothing, afl::base::Nothing));
    a.checkEqual("NAME", Access(p)("ships")[0]("NAME").toString(), "Ship 201");
    a.checkEqual("HULL", Access(p)("ships")[0]("HULL").toInteger(), 76);
}

// Unpacking a simulation can fail
AFL_TEST("server.format.Format:unpack:sim:error", a)
{
    server::format::Format testee;
    static const uint8_t FILE[] = {
        0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
    };
    afl::data::StringValue sv(afl::string::fromBytes(FILE));
    AFL_CHECK_THROWS(a, testee.unpack("sim", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
}
