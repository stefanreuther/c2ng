/**
  *  \file u/t_server_format_format.cpp
  *  \brief Test for server::format::Format
  */

#include <stdexcept>
#include "server/format/format.hpp"

#include "t_server_format.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringvalue.hpp"
#include "u/files.hpp"

/** Test pack(). */
void
TestServerFormatFormat::testPack()
{
    using afl::data::Access;
    server::format::Format testee;

    // Simple string, plain
    {
        afl::data::StringValue sv("x");
        std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "x");
    }

    // Simple string, tagged "obj"
    {
        afl::data::StringValue sv("x");
        std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, String_t("obj"), afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "x");
    }

    // Simple string with umlaut, default charset (latin1)
    {
        afl::data::StringValue sv("\xC3\xA4");
        std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "\xE4");
    }

    // Simple string with umlaut, given a charset
    {
        afl::data::StringValue sv("\xC3\xA4");
        std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, afl::base::Nothing, String_t("cp437")));
        TS_ASSERT_EQUALS(Access(p).toString(), "\x84");
    }

    // Truehull, given as partial JSON
    {
        afl::data::StringValue sv("[[1,2,3],[4,5,6]]");
        std::auto_ptr<afl::data::Value> p(testee.pack("truehull", &sv, String_t("json"), afl::base::Nothing));

        String_t result = Access(p).toString();
        TS_ASSERT_EQUALS(result.size(), 440U);
        TS_ASSERT_EQUALS(result[0], 1);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 2);
        TS_ASSERT_EQUALS(result[3], 0);
        TS_ASSERT_EQUALS(result[4], 3);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[40], 4);
        TS_ASSERT_EQUALS(result[11], 0);
    }

    // JSON string
    {
        afl::data::StringValue sv("\"x\"");
        std::auto_ptr<afl::data::Value> p(testee.pack("string", &sv, String_t("json"), afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "x");
    }

    // Error: not JSON
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.pack("string", &sv, String_t("json"), afl::base::Nothing), std::runtime_error);
    }

    // Error: bad type name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.pack("what", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
        TS_ASSERT_THROWS(testee.pack("", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
    }

    // Error: bad format name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.pack("string", &sv, String_t("what"), afl::base::Nothing), std::runtime_error);
        TS_ASSERT_THROWS(testee.pack("string", &sv, String_t(""), afl::base::Nothing), std::runtime_error);
    }

    // Error: bad charset name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.pack("string", &sv, afl::base::Nothing, String_t("what")), std::runtime_error);
        TS_ASSERT_THROWS(testee.pack("string", &sv, afl::base::Nothing, String_t("")), std::runtime_error);
    }
}

/** Test unpack. */
void
TestServerFormatFormat::testUnpack()
{
    using afl::data::Access;
    server::format::Format testee;

    // Simple string, plain
    {
        afl::data::StringValue sv("x");
        std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "x");
    }

    // Simple string, tagged "obj"
    {
        afl::data::StringValue sv("x");
        std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, String_t("obj"), afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "x");
    }

    // Simple string with umlaut, default charset (latin1)
    {
        afl::data::StringValue sv("\xE4");
        std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "\xC3\xA4");
    }

    // Simple string with umlaut, given a charset
    {
        afl::data::StringValue sv("\x84");
        std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, afl::base::Nothing, String_t("cp437")));
        TS_ASSERT_EQUALS(Access(p).toString(), "\xC3\xA4");
    }

    // JSON string
    {
        afl::data::StringValue sv("x");
        std::auto_ptr<afl::data::Value> p(testee.unpack("string", &sv, String_t("json"), afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p).toString(), "\"x\"");
    }

    // Error: bad type name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.unpack("what", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
        TS_ASSERT_THROWS(testee.unpack("", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
    }

    // Error: bad format name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.unpack("string", &sv, String_t("what"), afl::base::Nothing), std::runtime_error);
        TS_ASSERT_THROWS(testee.unpack("string", &sv, String_t(""), afl::base::Nothing), std::runtime_error);
    }

    // Error: bad charset name
    {
        afl::data::StringValue sv("x");
        TS_ASSERT_THROWS(testee.unpack("string", &sv, afl::base::Nothing, String_t("what")), std::runtime_error);
        TS_ASSERT_THROWS(testee.unpack("string", &sv, afl::base::Nothing, String_t("")), std::runtime_error);
    }
}

/** Test unpack() with a multitude of formats.
    This mainly exercises the Packer factory function; the individual packers already have their test. */
void
TestServerFormatFormat::testUnpackAll()
{
    using afl::data::Access;
    server::format::Format testee;

    // Engines
    {
        static const uint8_t ENGSPEC[] = {
            0x53, 0x74, 0x61, 0x72, 0x44, 0x72, 0x69, 0x76, 0x65, 0x20, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x64, 0x00,
            0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x8c, 0x0a, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xd4, 0x30,
            0x00, 0x00, 0x60, 0x54, 0x00, 0x00, 0xfc, 0x85, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0xc4, 0x1c,
            0x01, 0x00
        };
        afl::data::StringValue sv(afl::string::fromBytes(ENGSPEC));
        std::auto_ptr<afl::data::Value> p(testee.unpack("engspec", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p)[0]("NAME").toString(), "StarDrive 1");
        TS_ASSERT_EQUALS(Access(p)[0]("FUELFACTOR")[9].toInteger(), 72900);
    }

    // Beams
    {
        static const uint8_t BEAMSPEC[] = {
            0x4c, 0x61, 0x73, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
            0x0a, 0x00, 0x03, 0x00
        };
        afl::data::StringValue sv(afl::string::fromBytes(BEAMSPEC));
        std::auto_ptr<afl::data::Value> p(testee.unpack("beamspec", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p)[0]("NAME").toString(), "Laser");
        TS_ASSERT_EQUALS(Access(p)[0]("KILL").toInteger(), 10);
    }

    // Torpedoes
    {
        static const uint8_t TORPSPEC[] = {
            0x4d, 0x61, 0x72, 0x6b, 0x20, 0x31, 0x20, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x6e, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
            0x01, 0x00, 0x04, 0x00, 0x05, 0x00
        };
        afl::data::StringValue sv(afl::string::fromBytes(TORPSPEC));
        std::auto_ptr<afl::data::Value> p(testee.unpack("torpspec", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p)[0]("NAME").toString(), "Mark 1 Photon");
        TS_ASSERT_EQUALS(Access(p)[0]("DAMAGE1").toInteger(), 5);
    }

    // Hulls
    {
        static const uint8_t HULLSPEC[] = {
            0x4e, 0x4f, 0x43, 0x54, 0x55, 0x52, 0x4e, 0x45, 0x20, 0x43, 0x4c, 0x41, 0x53, 0x53, 0x20, 0x44,
            0x45, 0x53, 0x54, 0x52, 0x4f, 0x59, 0x45, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x00,
            0x01, 0x00, 0x32, 0x00, 0x19, 0x00, 0x07, 0x00, 0xb4, 0x00, 0xbe, 0x00, 0x01, 0x00, 0x5a, 0x00,
            0x02, 0x00, 0x32, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x46, 0x00
        };
        afl::data::StringValue sv(afl::string::fromBytes(HULLSPEC));
        std::auto_ptr<afl::data::Value> p(testee.unpack("hullspec", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p)[0]("NAME").toString(), "NOCTURNE CLASS DESTROYER");
        TS_ASSERT_EQUALS(Access(p)[0]("MASS").toInteger(), 90);
    }

    // Simulation
    {
        afl::data::StringValue sv(afl::string::fromBytes(getSimFileV1()));
        std::auto_ptr<afl::data::Value> p(testee.unpack("sim", &sv, afl::base::Nothing, afl::base::Nothing));
        TS_ASSERT_EQUALS(Access(p)("ships")[0]("NAME").toString(), "Ship 201");
        TS_ASSERT_EQUALS(Access(p)("ships")[0]("HULL").toInteger(), 76);
    }

    // Unpacking a simulation can fail
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
        };
        afl::data::StringValue sv(afl::string::fromBytes(FILE));
        TS_ASSERT_THROWS(testee.unpack("sim", &sv, afl::base::Nothing, afl::base::Nothing), std::runtime_error);
    }
}

