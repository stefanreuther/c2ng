/**
  *  \file test/server/format/torpedopackertest.cpp
  *  \brief Test for server::format::TorpedoPacker
  */

#include "server/format/torpedopacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("server.format.TorpedoPacker:basics", a)
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
    afl::data::Access ap(p);

    // Verify
    a.checkEqual("01. getArraySize", ap.getArraySize(), 3U);

    a.checkEqual("11", ap[0]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("12", ap[0]("MASS").toInteger(), 2);
    a.checkEqual("13", ap[0]("TECH").toInteger(), 1);
    a.checkEqual("14", ap[0]("KILL1").toInteger(), 4);
    a.checkEqual("15", ap[0]("DAMAGE1").toInteger(), 5);
    a.checkEqual("16", ap[0]("TUBECOST")("T").toInteger(), 1);
    a.checkEqual("17", ap[0]("TUBECOST")("D").toInteger(), 1);
    a.checkEqual("18", ap[0]("TUBECOST")("M").toInteger(), 0);
    a.checkEqual("19", ap[0]("TUBECOST")("MC").toInteger(), 1);
    a.checkEqual("20", ap[0]("TORPCOST")("T").toInteger(), 1);
    a.checkEqual("21", ap[0]("TORPCOST")("D").toInteger(), 1);
    a.checkEqual("22", ap[0]("TORPCOST")("M").toInteger(), 1);
    a.checkEqual("23", ap[0]("TORPCOST")("MC").toInteger(), 1);

    a.checkEqual("31", ap[1]("NAME").toString(), "Proton torp");
    a.checkEqual("32", ap[1]("MASS").toInteger(), 2);
    a.checkEqual("33", ap[1]("TECH").toInteger(), 2);
    a.checkEqual("34", ap[1]("KILL1").toInteger(), 6);
    a.checkEqual("35", ap[1]("DAMAGE1").toInteger(), 8);
    a.checkEqual("36", ap[1]("TUBECOST")("T").toInteger(), 1);
    a.checkEqual("37", ap[1]("TUBECOST")("D").toInteger(), 0);
    a.checkEqual("38", ap[1]("TUBECOST")("M").toInteger(), 0);
    a.checkEqual("39", ap[1]("TUBECOST")("MC").toInteger(), 4);
    a.checkEqual("40", ap[1]("TORPCOST")("T").toInteger(), 1);
    a.checkEqual("41", ap[1]("TORPCOST")("D").toInteger(), 1);
    a.checkEqual("42", ap[1]("TORPCOST")("M").toInteger(), 1);
    a.checkEqual("43", ap[1]("TORPCOST")("MC").toInteger(), 2);

    a.checkEqual("51", ap[2]("NAME").toString(), "Mark 2 Photon");
    a.checkEqual("52", ap[2]("MASS").toInteger(), 2);
    a.checkEqual("53", ap[2]("TECH").toInteger(), 3);
    a.checkEqual("54", ap[2]("KILL1").toInteger(), 3);
    a.checkEqual("55", ap[2]("DAMAGE1").toInteger(), 10);
    a.checkEqual("56", ap[2]("TUBECOST")("T").toInteger(), 1);
    a.checkEqual("57", ap[2]("TUBECOST")("D").toInteger(), 4);
    a.checkEqual("58", ap[2]("TUBECOST")("M").toInteger(), 0);
    a.checkEqual("59", ap[2]("TUBECOST")("MC").toInteger(), 4);
    a.checkEqual("60", ap[2]("TORPCOST")("T").toInteger(), 1);
    a.checkEqual("61", ap[2]("TORPCOST")("D").toInteger(), 1);
    a.checkEqual("62", ap[2]("TORPCOST")("M").toInteger(), 1);
    a.checkEqual("63", ap[2]("TORPCOST")("MC").toInteger(), 5);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("71", repacked, afl::string::fromBytes(TORPSPEC));
}

/** Test unpacking a large file.
    We do not unpack more than 10 torpedoes. */
AFL_TEST("server.format.TorpedoPacker:large", a)
{
    // Pack
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::TorpedoPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(String_t(10000, ' '), cs));
    afl::data::Access ap(p);

    a.checkEqual("01. getArraySize", ap.getArraySize(), 10U);
    a.checkEqual("02", ap[0]("MASS").toInteger(), 0x2020);
    a.checkEqual("03", ap[9]("MASS").toInteger(), 0x2020);
}
