/**
  *  \file test/server/format/enginepackertest.cpp
  *  \brief Test for server::format::EnginePacker
  */

#include "server/format/enginepacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/data/value.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.format.EnginePacker", a)
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
    afl::data::Access ap(p);

    // Verify
    a.checkEqual("01. getArraySize", ap.getArraySize(), 2U);

    a.checkEqual("11", ap[0]("NAME").toString(), "StarDrive 1");
    a.checkEqual("12", ap[0]("COST")("T").toInteger(), 5);
    a.checkEqual("13", ap[0]("COST")("D").toInteger(), 1);
    a.checkEqual("14", ap[0]("COST")("M").toInteger(), 0);
    a.checkEqual("15", ap[0]("COST")("MC").toInteger(), 1);
    a.checkEqual("16", ap[0]("TECH").toInteger(), 1);
    a.checkEqual("17", ap[0]("FUELFACTOR").getArraySize(), 10U);
    a.checkEqual("18", ap[0]("FUELFACTOR")[0].toInteger(), 0);
    a.checkEqual("19", ap[0]("FUELFACTOR")[1].toInteger(), 100);
    a.checkEqual("20", ap[0]("FUELFACTOR")[2].toInteger(), 800);
    a.checkEqual("21", ap[0]("FUELFACTOR")[3].toInteger(), 2700);
    a.checkEqual("22", ap[0]("FUELFACTOR")[4].toInteger(), 6400);
    a.checkEqual("23", ap[0]("FUELFACTOR")[5].toInteger(), 12500);
    a.checkEqual("24", ap[0]("FUELFACTOR")[6].toInteger(), 21600);
    a.checkEqual("25", ap[0]("FUELFACTOR")[7].toInteger(), 34300);
    a.checkEqual("26", ap[0]("FUELFACTOR")[8].toInteger(), 51200);
    a.checkEqual("27", ap[0]("FUELFACTOR")[9].toInteger(), 72900);

    a.checkEqual("31", ap[1]("NAME").toString(), "StarDrive 2");
    a.checkEqual("32", ap[1]("COST")("T").toInteger(), 5);
    a.checkEqual("33", ap[1]("COST")("D").toInteger(), 2);
    a.checkEqual("34", ap[1]("COST")("M").toInteger(), 1);
    a.checkEqual("35", ap[1]("COST")("MC").toInteger(), 2);
    a.checkEqual("36", ap[1]("TECH").toInteger(), 2);
    a.checkEqual("37", ap[1]("FUELFACTOR").getArraySize(), 10U);
    a.checkEqual("38", ap[1]("FUELFACTOR")[0].toInteger(), 0);
    a.checkEqual("39", ap[1]("FUELFACTOR")[1].toInteger(), 100);
    a.checkEqual("40", ap[1]("FUELFACTOR")[2].toInteger(), 430);
    a.checkEqual("41", ap[1]("FUELFACTOR")[3].toInteger(), 2700);
    a.checkEqual("42", ap[1]("FUELFACTOR")[4].toInteger(), 6400);
    a.checkEqual("43", ap[1]("FUELFACTOR")[5].toInteger(), 12500);
    a.checkEqual("44", ap[1]("FUELFACTOR")[6].toInteger(), 21600);
    a.checkEqual("45", ap[1]("FUELFACTOR")[7].toInteger(), 34300);
    a.checkEqual("46", ap[1]("FUELFACTOR")[8].toInteger(), 51200);
    a.checkEqual("47", ap[1]("FUELFACTOR")[9].toInteger(), 72900);

    // Repack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("51", repacked, afl::string::fromBytes(ENGSPEC));
}
