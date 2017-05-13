/**
  *  \file u/t_server_format_torpedopacker.cpp
  *  \brief Test for server::format::TorpedoPacker
  */

#include "server/format/torpedopacker.hpp"

#include "t_server_format.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"

void
TestServerFormatTorpedoPacker::testIt()
{
    // Three torpedoes
    static const uint8_t TORPSPEC[] = {
        0x4d, 0x61, 0x72, 0x6b, 0x20, 0x31, 0x20, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x6e, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x01, 0x00, 0x04, 0x00, 0x05, 0x00, 0x50, 0x72, 0x6f, 0x74, 0x6f, 0x6e, 0x20, 0x74, 0x6f, 0x72,
        0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x02, 0x00, 0x04, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x06, 0x00, 0x08, 0x00, 0x4d, 0x61, 0x72, 0x6b,
        0x20, 0x32, 0x20, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x6e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x05, 0x00, 0x04, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x03, 0x00,
        0x0a, 0x00
    };

    // Unpack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::TorpedoPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(TORPSPEC), cs));
    afl::data::Access a(p);

    // Verify
    TS_ASSERT_EQUALS(a.getArraySize(), 3U);

    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a[0]("MASS").toInteger(), 2);
    TS_ASSERT_EQUALS(a[0]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("KILL1").toInteger(), 4);
    TS_ASSERT_EQUALS(a[0]("DAMAGE1").toInteger(), 5);
    TS_ASSERT_EQUALS(a[0]("TUBECOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TUBECOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TUBECOST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("TUBECOST")("MC").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TORPCOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TORPCOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TORPCOST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TORPCOST")("MC").toInteger(), 1);

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Proton torp");
    TS_ASSERT_EQUALS(a[1]("MASS").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("KILL1").toInteger(), 6);
    TS_ASSERT_EQUALS(a[1]("DAMAGE1").toInteger(), 8);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("D").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("MC").toInteger(), 4);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("MC").toInteger(), 2);

    TS_ASSERT_EQUALS(a[2]("NAME").toString(), "Mark 2 Photon");
    TS_ASSERT_EQUALS(a[2]("MASS").toInteger(), 2);
    TS_ASSERT_EQUALS(a[2]("TECH").toInteger(), 3);
    TS_ASSERT_EQUALS(a[2]("KILL1").toInteger(), 3);
    TS_ASSERT_EQUALS(a[2]("DAMAGE1").toInteger(), 10);
    TS_ASSERT_EQUALS(a[2]("TUBECOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[2]("TUBECOST")("D").toInteger(), 4);
    TS_ASSERT_EQUALS(a[2]("TUBECOST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[2]("TUBECOST")("MC").toInteger(), 4);
    TS_ASSERT_EQUALS(a[2]("TORPCOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[2]("TORPCOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[2]("TORPCOST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[2]("TORPCOST")("MC").toInteger(), 5);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(TORPSPEC));
}

/** Test unpacking a large file.
    We do not unpack more than 10 torpedoes. */
void
TestServerFormatTorpedoPacker::testLarge()
{
    // Pack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::TorpedoPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(String_t(10000, ' '), cs));
    afl::data::Access a(p);

    TS_ASSERT_EQUALS(a.getArraySize(), 10U);
    TS_ASSERT_EQUALS(a[0]("MASS").toInteger(), 0x2020);
    TS_ASSERT_EQUALS(a[9]("MASS").toInteger(), 0x2020);
}
