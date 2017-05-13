/**
  *  \file u/t_server_format_beampacker.cpp
  *  \brief Test for server::format::BeamPacker
  */

#include "server/format/beampacker.hpp"

#include "t_server_format.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"

/** Test pack/unpack for Beams. */
void
TestServerFormatBeamPacker::testIt()
{
    // Three beams
    static const uint8_t BEAMSPEC[] = {
        0x4c, 0x61, 0x73, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x0a, 0x00, 0x03, 0x00, 0x58, 0x2d, 0x52, 0x61, 0x79, 0x20, 0x4c, 0x61, 0x73, 0x65, 0x72, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x01, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x50, 0x6c, 0x61, 0x73, 0x6d, 0x61, 0x20, 0x42,
        0x6f, 0x6c, 0x74, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x05, 0x00, 0x01, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x00, 0x0a, 0x00
    };

    // Unpack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::BeamPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(BEAMSPEC), cs));
    afl::data::Access a(p);

    // Verify
    TS_ASSERT_EQUALS(a.getArraySize(), 3U);

    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[0]("MASS").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("KILL").toInteger(), 10);
    TS_ASSERT_EQUALS(a[0]("DAMAGE").toInteger(), 3);
    TS_ASSERT_EQUALS(a[0]("COST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[0]("COST")("D").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("COST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[0]("COST")("MC").toInteger(), 1);

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "X-Ray Laser");
    TS_ASSERT_EQUALS(a[1]("MASS").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("KILL").toInteger(), 15);
    TS_ASSERT_EQUALS(a[1]("DAMAGE").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("COST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("COST")("D").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("COST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("COST")("MC").toInteger(), 2);

    TS_ASSERT_EQUALS(a[2]("NAME").toString(), "Plasma Bolt");
    TS_ASSERT_EQUALS(a[2]("MASS").toInteger(), 2);
    TS_ASSERT_EQUALS(a[2]("TECH").toInteger(), 2);
    TS_ASSERT_EQUALS(a[2]("KILL").toInteger(), 3);
    TS_ASSERT_EQUALS(a[2]("DAMAGE").toInteger(), 10);
    TS_ASSERT_EQUALS(a[2]("COST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[2]("COST")("D").toInteger(), 2);
    TS_ASSERT_EQUALS(a[2]("COST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[2]("COST")("MC").toInteger(), 5);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(BEAMSPEC));
}

