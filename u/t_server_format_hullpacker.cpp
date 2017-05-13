/**
  *  \file u/t_server_format_hullpacker.cpp
  *  \brief Test for server::format::HullPacker
  */

#include "server/format/hullpacker.hpp"

#include "t_server_format.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"

/** Simple test. */
void
TestServerFormatHullPacker::testIt()
{
    // Two hulls
    static const uint8_t HULLSPEC[] = {
        0x4f, 0x55, 0x54, 0x52, 0x49, 0x44, 0x45, 0x52, 0x20, 0x43, 0x4c, 0x41, 0x53, 0x53, 0x20, 0x53,
        0x43, 0x4f, 0x55, 0x54, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x09, 0x00,
        0x01, 0x00, 0x28, 0x00, 0x14, 0x00, 0x05, 0x00, 0x04, 0x01, 0xb4, 0x00, 0x01, 0x00, 0x4b, 0x00,
        0x01, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x32, 0x00, 0x4e, 0x4f, 0x43, 0x54,
        0x55, 0x52, 0x4e, 0x45, 0x20, 0x43, 0x4c, 0x41, 0x53, 0x53, 0x20, 0x44, 0x45, 0x53, 0x54, 0x52,
        0x4f, 0x59, 0x45, 0x52, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x00, 0x01, 0x00, 0x32, 0x00,
        0x19, 0x00, 0x07, 0x00, 0xb4, 0x00, 0xbe, 0x00, 0x01, 0x00, 0x5a, 0x00, 0x02, 0x00, 0x32, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x46, 0x00
    };

    // Unpack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::HullPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(HULLSPEC), cs));
    afl::data::Access a(p);

    // Verify
    TS_ASSERT_EQUALS(a.getArraySize(), 2U);

    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(a[0]("COST")("MC").toInteger(), 50);
    TS_ASSERT_EQUALS(a[0]("COST")("T").toInteger(), 40);
    TS_ASSERT_EQUALS(a[0]("COST")("D").toInteger(), 20);
    TS_ASSERT_EQUALS(a[0]("COST")("M").toInteger(), 5);
    TS_ASSERT_EQUALS(a[0]("MASS").toInteger(), 75);
    TS_ASSERT_EQUALS(a[0]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("BEAM.MAX").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TORP.LMAX").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("FIGHTER.BAYS").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("ENGINE.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("CARGO.MAXFUEL").toInteger(), 260);
    TS_ASSERT_EQUALS(a[0]("CARGO.MAX").toInteger(), 40);
    TS_ASSERT_EQUALS(a[0]("CREW.NORMAL").toInteger(), 180);

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "NOCTURNE CLASS DESTROYER");
    TS_ASSERT_EQUALS(a[1]("COST")("MC").toInteger(), 70);
    TS_ASSERT_EQUALS(a[1]("COST")("T").toInteger(), 50);
    TS_ASSERT_EQUALS(a[1]("COST")("D").toInteger(), 25);
    TS_ASSERT_EQUALS(a[1]("COST")("M").toInteger(), 7);
    TS_ASSERT_EQUALS(a[1]("MASS").toInteger(), 90);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("BEAM.MAX").toInteger(), 4);
    TS_ASSERT_EQUALS(a[1]("TORP.LMAX").toInteger(), 2);
    TS_ASSERT_EQUALS(a[1]("FIGHTER.BAYS").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("ENGINE.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("CARGO.MAXFUEL").toInteger(), 180);
    TS_ASSERT_EQUALS(a[1]("CARGO.MAX").toInteger(), 50);
    TS_ASSERT_EQUALS(a[1]("CREW.NORMAL").toInteger(), 190);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(HULLSPEC));
}

