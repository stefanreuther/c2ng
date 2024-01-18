/**
  *  \file test/server/format/beampackertest.cpp
  *  \brief Test for server::format::BeamPacker
  */

#include "server/format/beampacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/test/testrunner.hpp"

/** Test pack/unpack for Beams. */
AFL_TEST("server.format.BeamPacker", a)
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
    afl::data::Access ap(p);

    // Verify
    a.checkEqual("01. getArraySize", ap.getArraySize(), 3U);

    a.checkEqual("11", ap[0]("NAME").toString(), "Laser");
    a.checkEqual("12", ap[0]("MASS").toInteger(), 1);
    a.checkEqual("13", ap[0]("TECH").toInteger(), 1);
    a.checkEqual("14", ap[0]("KILL").toInteger(), 10);
    a.checkEqual("15", ap[0]("DAMAGE").toInteger(), 3);
    a.checkEqual("16", ap[0]("COST")("T").toInteger(), 1);
    a.checkEqual("17", ap[0]("COST")("D").toInteger(), 0);
    a.checkEqual("18", ap[0]("COST")("M").toInteger(), 0);
    a.checkEqual("19", ap[0]("COST")("MC").toInteger(), 1);

    a.checkEqual("21", ap[1]("NAME").toString(), "X-Ray Laser");
    a.checkEqual("22", ap[1]("MASS").toInteger(), 1);
    a.checkEqual("23", ap[1]("TECH").toInteger(), 1);
    a.checkEqual("24", ap[1]("KILL").toInteger(), 15);
    a.checkEqual("25", ap[1]("DAMAGE").toInteger(), 1);
    a.checkEqual("26", ap[1]("COST")("T").toInteger(), 1);
    a.checkEqual("27", ap[1]("COST")("D").toInteger(), 0);
    a.checkEqual("28", ap[1]("COST")("M").toInteger(), 0);
    a.checkEqual("29", ap[1]("COST")("MC").toInteger(), 2);

    a.checkEqual("31", ap[2]("NAME").toString(), "Plasma Bolt");
    a.checkEqual("32", ap[2]("MASS").toInteger(), 2);
    a.checkEqual("33", ap[2]("TECH").toInteger(), 2);
    a.checkEqual("34", ap[2]("KILL").toInteger(), 3);
    a.checkEqual("35", ap[2]("DAMAGE").toInteger(), 10);
    a.checkEqual("36", ap[2]("COST")("T").toInteger(), 1);
    a.checkEqual("37", ap[2]("COST")("D").toInteger(), 2);
    a.checkEqual("38", ap[2]("COST")("M").toInteger(), 0);
    a.checkEqual("39", ap[2]("COST")("MC").toInteger(), 5);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("41", repacked, afl::string::fromBytes(BEAMSPEC));
}
