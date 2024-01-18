/**
  *  \file test/server/format/hullpackertest.cpp
  *  \brief Test for server::format::HullPacker
  */

#include "server/format/hullpacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.format.HullPacker", a)
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
    afl::data::Access aa(p);

    // Verify
    a.checkEqual("01. getArraySize", aa.getArraySize(), 2U);

    a.checkEqual("11", aa[0]("NAME").toString(), "OUTRIDER CLASS SCOUT");
    a.checkEqual("12", aa[0]("COST")("MC").toInteger(), 50);
    a.checkEqual("13", aa[0]("COST")("T").toInteger(), 40);
    a.checkEqual("14", aa[0]("COST")("D").toInteger(), 20);
    a.checkEqual("15", aa[0]("COST")("M").toInteger(), 5);
    a.checkEqual("16", aa[0]("MASS").toInteger(), 75);
    a.checkEqual("17", aa[0]("TECH").toInteger(), 1);
    a.checkEqual("18", aa[0]("BEAM.MAX").toInteger(), 1);
    a.checkEqual("19", aa[0]("TORP.LMAX").toInteger(), 0);
    a.checkEqual("20", aa[0]("FIGHTER.BAYS").toInteger(), 0);
    a.checkEqual("21", aa[0]("ENGINE.COUNT").toInteger(), 1);
    a.checkEqual("22", aa[0]("CARGO.MAXFUEL").toInteger(), 260);
    a.checkEqual("23", aa[0]("CARGO.MAX").toInteger(), 40);
    a.checkEqual("24", aa[0]("CREW.NORMAL").toInteger(), 180);

    a.checkEqual("31", aa[1]("NAME").toString(), "NOCTURNE CLASS DESTROYER");
    a.checkEqual("32", aa[1]("COST")("MC").toInteger(), 70);
    a.checkEqual("33", aa[1]("COST")("T").toInteger(), 50);
    a.checkEqual("34", aa[1]("COST")("D").toInteger(), 25);
    a.checkEqual("35", aa[1]("COST")("M").toInteger(), 7);
    a.checkEqual("36", aa[1]("MASS").toInteger(), 90);
    a.checkEqual("37", aa[1]("TECH").toInteger(), 2);
    a.checkEqual("38", aa[1]("BEAM.MAX").toInteger(), 4);
    a.checkEqual("39", aa[1]("TORP.LMAX").toInteger(), 2);
    a.checkEqual("40", aa[1]("FIGHTER.BAYS").toInteger(), 0);
    a.checkEqual("41", aa[1]("ENGINE.COUNT").toInteger(), 1);
    a.checkEqual("42", aa[1]("CARGO.MAXFUEL").toInteger(), 180);
    a.checkEqual("43", aa[1]("CARGO.MAX").toInteger(), 50);
    a.checkEqual("44", aa[1]("CREW.NORMAL").toInteger(), 190);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("51", repacked, afl::string::fromBytes(HULLSPEC));
}
