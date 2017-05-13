/**
  *  \file u/t_server_format_enginepacker.cpp
  *  \brief Test for server::format::EnginePacker
  */

#include "server/format/enginepacker.hpp"

#include "t_server_format.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"

/** Simple test. */
void
TestServerFormatEnginePacker::testIt()
{
    // Two engines
    static const uint8_t ENGSPEC[] = {
        0x53, 0x74, 0x61, 0x72, 0x44, 0x72, 0x69, 0x76, 0x65, 0x20, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x8c, 0x0a, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xd4, 0x30,
        0x00, 0x00, 0x60, 0x54, 0x00, 0x00, 0xfc, 0x85, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00, 0xc4, 0x1c,
        0x01, 0x00, 0x53, 0x74, 0x61, 0x72, 0x44, 0x72, 0x69, 0x76, 0x65, 0x20, 0x32, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x02, 0x00, 0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0x00,
        0x64, 0x00, 0x00, 0x00, 0xae, 0x01, 0x00, 0x00, 0x8c, 0x0a, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00,
        0xd4, 0x30, 0x00, 0x00, 0x60, 0x54, 0x00, 0x00, 0xfc, 0x85, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x00,
        0xc4, 0x1c, 0x01, 0x00
    };

    // Unpack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::EnginePacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(ENGSPEC), cs));
    afl::data::Access a(p);

    // Verify
    TS_ASSERT_EQUALS(a.getArraySize(), 2U);

    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "StarDrive 1");
    TS_ASSERT_EQUALS(a[0]("COST")("T").toInteger(), 5);
    TS_ASSERT_EQUALS(a[0]("COST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("COST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("COST")("MC").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR").getArraySize(), 10U);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[0].toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[1].toInteger(), 100);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[2].toInteger(), 800);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[3].toInteger(), 2700);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[4].toInteger(), 6400);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[5].toInteger(), 12500);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[6].toInteger(), 21600);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[7].toInteger(), 34300);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[8].toInteger(), 51200);
    TS_ASSERT_EQUALS(a[0]("FUELFACTOR")[9].toInteger(), 72900);

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "StarDrive 2");
    TS_ASSERT_EQUALS(a[1]("COST")("T").toInteger(), 5);
    TS_ASSERT_EQUALS(a[1]("COST")("D").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("COST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("COST")("MC").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR").getArraySize(), 10U);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[0].toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[1].toInteger(), 100);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[2].toInteger(), 430);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[3].toInteger(), 2700);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[4].toInteger(), 6400);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[5].toInteger(), 12500);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[6].toInteger(), 21600);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[7].toInteger(), 34300);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[8].toInteger(), 51200);
    TS_ASSERT_EQUALS(a[1]("FUELFACTOR")[9].toInteger(), 72900);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(ENGSPEC));
}

