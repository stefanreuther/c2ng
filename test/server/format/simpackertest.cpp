/**
  *  \file test/server/format/simpackertest.cpp
  *  \brief Test for server::format::SimPacker
  *
  *  Test cases are the same as for game::sim::Loader.
  */

#include "server/format/simpacker.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include <memory>
#include <stdexcept>

/** Test unpacking a V0 file. */
AFL_TEST("server.format.SimPacker:unpack:v0", a)
{
    // xref TestGameSimLoader::testV0
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV0()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 2U);
    a.checkNonNull("02", ap("planet").getValue());

    // First ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "C.C.S.S. Joker");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 61);  // Emerald
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 7);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 117);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "NTP");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 258);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 8);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 7);
    a.checkEqual("21", ap("ships")[0]("AUX.COUNT").toInteger(), 3);
    a.checkEqual("22", ap("ships")[0]("AUX").toInteger(), 10);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 40);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 7);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 0);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    a.checkNonNull("31", ap("ships")[1].getValue());
    a.checkEqual("32", ap("ships")[1]("NAME").toString(), "C.C.S.S. Claudrin II");
    a.checkEqual("33", ap("ships")[1]("HULL").toInteger(), 22);  // LCC
    a.checkEqual("34", ap("ships")[1]("OWNER").toInteger(), 7);
    a.checkEqual("35", ap("ships")[1]("ID").toInteger(), 9);
    a.checkEqual("36", ap("ships")[1]("FCODE").toString(), "NTP");
    a.checkEqual("37", ap("ships")[1]("DAMAGE").toInteger(), 0);
    a.checkEqual("38", ap("ships")[1]("CREW").toInteger(), 430);
    a.checkEqual("39", ap("ships")[1]("BEAM.COUNT").toInteger(), 4);
    a.checkEqual("40", ap("ships")[1]("BEAM").toInteger(), 6);
    a.checkEqual("41", ap("ships")[1]("AUX.COUNT").toInteger(), 3);
    a.checkEqual("42", ap("ships")[1]("AUX").toInteger(), 6);
    a.checkEqual("43", ap("ships")[1]("AUX.AMMO").toInteger(), 50);
    a.checkEqual("44", ap("ships")[1]("ENGINE").toInteger(), 9);
    a.checkEqual("45", ap("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("46", ap("ships")[1]("FLAGS").toInteger(), 0);
    a.checkEqual("47", ap("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("48", ap("ships")[1]("LEVEL").toInteger(), 0);

    // Planet
    a.checkNonNull("51", ap("planet").getValue());
    a.checkEqual("52", ap("planet")("ID").toInteger(), 1);
    a.checkEqual("53", ap("planet")("OWNER").toInteger(), 2);
    a.checkEqual("54", ap("planet")("FCODE").toString(), "i9m");
    a.checkEqual("55", ap("planet")("DEFENSE").toInteger(), 62);
    a.checkEqual("56", ap("planet")("FLAGS").toInteger(), 0);
    a.checkEqual("57", ap("planet")("LEVEL").toInteger(), 0);
    a.checkEqual("58", ap("planet")("TECH.BEAM").toInteger(), 0);
}

/** Test unpacking a V1 file. */
AFL_TEST("server.format.SimPacker:unpack:v1", a)
{
    // xref TestGameSimLoader::testV1
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV1()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 1U);
    a.checkNonNull("02", ap("planet").getValue());

    // Ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "Ship 201");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 76);  // SSC
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 8);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 201);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "?""?""?");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 352);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 6);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 6);
    a.checkEqual("21", ap("ships")[0]("AUX").toInteger(), 11);
    a.checkEqual("22", ap("ships")[0]("AUX.COUNT").toInteger(), 4);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 85);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 9);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 0);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 0);

    // Planet
    a.checkNonNull("31", ap("planet").getValue());
    a.checkEqual("32", ap("planet")("ID").toInteger(), 459);
    a.checkEqual("33", ap("planet")("OWNER").toInteger(), 6);
    a.checkEqual("34", ap("planet")("FCODE").toString(), "NUK");
    a.checkEqual("35", ap("planet")("DEFENSE").toInteger(), 129);
    a.checkEqual("36", ap("planet")("FLAGS").toInteger(), 0);
    a.checkEqual("37", ap("planet")("LEVEL").toInteger(), 0);
    a.checkEqual("38", ap("planet")("TECH.BEAM").toInteger(), 1);
    a.checkEqual("39", ap("planet")("STORAGE.AMMO")[10].toInteger(), 22);
    a.checkEqual("40", ap("planet")("DEFENSE.BASE").toInteger(), 150);
    a.checkEqual("41", ap("planet")("TECH.TORPEDO").toInteger(), 1);
}

/** Test unpacking a V2 file. */
AFL_TEST("server.format.SimPacker:unpack:v2", a)
{
    // xref TestGameSimLoader::testV2
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV2()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 2U);

    // First ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "Ship 4");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 1);  // Outrider
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 12);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 4);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "?""?""?");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 180);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 1);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 10);
    a.checkEqual("21", ap("ships")[0]("AUX").toInteger(), 0);
    a.checkEqual("22", ap("ships")[0]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 9);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), 13 /* NoFuel */);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 0);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    a.checkNonNull("31", ap("ships")[1].getValue());
    a.checkEqual("32", ap("ships")[1]("NAME").toString(), "Ship 5");
    a.checkEqual("33", ap("ships")[1]("HULL").toInteger(), 73);  // Mig Scout
    a.checkEqual("34", ap("ships")[1]("OWNER").toInteger(), 8);
    a.checkEqual("35", ap("ships")[1]("ID").toInteger(), 5);
    a.checkEqual("36", ap("ships")[1]("FCODE").toString(), "123");
    a.checkEqual("37", ap("ships")[1]("DAMAGE").toInteger(), 0);
    a.checkEqual("38", ap("ships")[1]("CREW").toInteger(), 10);
    a.checkEqual("39", ap("ships")[1]("BEAM.COUNT").toInteger(), 2);
    a.checkEqual("40", ap("ships")[1]("BEAM").toInteger(), 10);
    a.checkEqual("41", ap("ships")[1]("AUX").toInteger(), 0);
    a.checkEqual("42", ap("ships")[1]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("43", ap("ships")[1]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("44", ap("ships")[1]("ENGINE").toInteger(), 9);
    a.checkEqual("45", ap("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("46", ap("ships")[1]("FLAGS").toInteger(), 0);
    a.checkEqual("47", ap("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("48", ap("ships")[1]("LEVEL").toInteger(), 0);

    // Planet
    a.checkNonNull("51", ap("planet").getValue());
    a.checkEqual("52", ap("planet")("ID").toInteger(), 1);
    a.checkEqual("53", ap("planet")("OWNER").toInteger(), 12);
    a.checkEqual("54", ap("planet")("FCODE").toString(), "NUK");
    a.checkEqual("55", ap("planet")("DEFENSE").toInteger(), 10);
    a.checkEqual("56", ap("planet")("FLAGS").toInteger(), 0);
    a.checkEqual("57", ap("planet")("LEVEL").toInteger(), 0);
    a.checkEqual("58", ap("planet")("TECH.BEAM").toInteger(), 0);
    // a.checkEqual("59", ap("planet")("STORAGE.AMMO")[10].toInteger(), 0); // not set
    // a.checkEqual("60", ap("planet")("DEFENSE.BASE").toInteger(), 0);     // not set
    // a.checkEqual("61", ap("planet")("TECH.TORPEDO").toInteger(), 0); // not set
}

/** Test unpacking a V3 file. */
AFL_TEST("server.format.SimPacker:unpack:v3", a)
{
    // xref TestGameSimLoader::testV3
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV3()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 3U);

    // First ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "Ultra Elite Alien");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 1);  // Outrider
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 12);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 1);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "?""?""?");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 58);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 1);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 10);
    a.checkEqual("21", ap("ships")[0]("AUX").toInteger(), 0);
    a.checkEqual("22", ap("ships")[0]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 9);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 6144 /* CommanderSet + Commander */);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 4);

     // Second ship
    a.checkNonNull("31", ap("ships")[1].getValue());
    a.checkEqual("32", ap("ships")[1]("NAME").toString(), "Recruit Alien");
    a.checkEqual("33", ap("ships")[1]("HULL").toInteger(), 1);  // Outrider
    a.checkEqual("34", ap("ships")[1]("OWNER").toInteger(), 12);
    a.checkEqual("35", ap("ships")[1]("ID").toInteger(), 2);
    a.checkEqual("36", ap("ships")[1]("FCODE").toString(), "?""?""?");
    a.checkEqual("37", ap("ships")[1]("DAMAGE").toInteger(), 0);
    a.checkEqual("38", ap("ships")[1]("CREW").toInteger(), 58);
    a.checkEqual("39", ap("ships")[1]("BEAM.COUNT").toInteger(), 1);
    a.checkEqual("40", ap("ships")[1]("BEAM").toInteger(), 10);
    a.checkEqual("41", ap("ships")[1]("AUX").toInteger(), 0);
    a.checkEqual("42", ap("ships")[1]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("43", ap("ships")[1]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("44", ap("ships")[1]("ENGINE").toInteger(), 9);
    a.checkEqual("45", ap("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("46", ap("ships")[1]("FLAGS").toInteger(), 0);
    a.checkEqual("47", ap("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("48", ap("ships")[1]("LEVEL").toInteger(), 0);

    // Third ship
    a.checkNonNull("51", ap("ships")[2].getValue());
    a.checkEqual("52", ap("ships")[2]("NAME").toString(), "Recruit Borg");
    a.checkEqual("53", ap("ships")[2]("HULL").toInteger(), 58);  // Quietus
    a.checkEqual("54", ap("ships")[2]("OWNER").toInteger(), 6);
    a.checkEqual("55", ap("ships")[2]("ID").toInteger(), 3);
    a.checkEqual("56", ap("ships")[2]("FCODE").toString(), "?""?""?");
    a.checkEqual("57", ap("ships")[2]("DAMAGE").toInteger(), 0);
    a.checkEqual("58", ap("ships")[2]("CREW").toInteger(), 517);
    a.checkEqual("59", ap("ships")[2]("BEAM.COUNT").toInteger(), 9);
    a.checkEqual("60", ap("ships")[2]("BEAM").toInteger(), 10);
    a.checkEqual("61", ap("ships")[2]("AUX.COUNT").toInteger(), 9);
    a.checkEqual("62", ap("ships")[2]("AUX").toInteger(), 10);
    a.checkEqual("63", ap("ships")[2]("AUX.AMMO").toInteger(), 260);
    a.checkEqual("64", ap("ships")[2]("ENGINE").toInteger(), 9);
    a.checkEqual("65", ap("ships")[2]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("66", ap("ships")[2]("FLAGS").toInteger(), 0);
    a.checkEqual("67", ap("ships")[2]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("68", ap("ships")[2]("LEVEL").toInteger(), 0);

    // Planet
    a.checkNonNull("71", ap("planet").getValue());
    a.checkEqual("72", ap("planet")("ID").toInteger(), 1);
    a.checkEqual("73", ap("planet")("OWNER").toInteger(), 12);
    a.checkEqual("74", ap("planet")("FCODE").toString(), "?""?""?");
    a.checkEqual("75", ap("planet")("DEFENSE").toInteger(), 10);
    a.checkEqual("76", ap("planet")("FLAGS").toInteger(), 0);
    a.checkEqual("77", ap("planet")("LEVEL").toInteger(), 0);
    a.checkEqual("78", ap("planet")("TECH.BEAM").toInteger(), 0);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("81", repacked, afl::string::fromBytes(game::test::getSimFileV3()));
}

/** Test unpacking a V4 file. */
AFL_TEST("server.format.SimPacker:unpack:v4", a)
{
    // xref TestGameSimLoader::testV4
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV4()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 1U);
    a.checkNull("02", ap("planet").getValue());

    // The ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "Ship 1");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 1);  // Outrider
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 12);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 1);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "?""?""?");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 58);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 1);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 10);
    a.checkEqual("21", ap("ships")[0]("AUX").toInteger(), 0);
    a.checkEqual("22", ap("ships")[0]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 9);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 16 /* RatingOverride */);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 0);
    a.checkEqual("29", ap("ships")[0]("RATING.R").toInteger(), 240);
    a.checkEqual("30", ap("ships")[0]("RATING.C").toInteger(), 23);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("31", repacked, afl::string::fromBytes(game::test::getSimFileV4()));
}

/** Test unpacking a V5 file. */
AFL_TEST("server.format.SimPacker:unpack:v5", a)
{
    // xref TestGameSimLoader::testV5
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV5()), cs));
    afl::data::Access ap(p);

    // Basic properties
    a.checkEqual("01", ap("ships").getArraySize(), 2U);
    a.checkNull("02", ap("planet").getValue());

    // First ship
    a.checkNonNull("11", ap("ships")[0].getValue());
    a.checkEqual("12", ap("ships")[0]("NAME").toString(), "Mike Oldfield");
    a.checkEqual("13", ap("ships")[0]("HULL").toInteger(), 16);  // MDSF
    a.checkEqual("14", ap("ships")[0]("OWNER").toInteger(), 9);
    a.checkEqual("15", ap("ships")[0]("ID").toInteger(), 1);
    a.checkEqual("16", ap("ships")[0]("FCODE").toString(), "_{=");
    a.checkEqual("17", ap("ships")[0]("DAMAGE").toInteger(), 0);
    a.checkEqual("18", ap("ships")[0]("CREW").toInteger(), 6);
    a.checkEqual("19", ap("ships")[0]("BEAM.COUNT").toInteger(), 0);
    a.checkEqual("20", ap("ships")[0]("BEAM").toInteger(), 0);
    a.checkEqual("21", ap("ships")[0]("AUX").toInteger(), 0);
    a.checkEqual("22", ap("ships")[0]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("23", ap("ships")[0]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("24", ap("ships")[0]("ENGINE").toInteger(), 8);
    a.checkEqual("25", ap("ships")[0]("AGGRESSIVENESS").toInteger(), 0);
    a.checkEqual("26", ap("ships")[0]("FLAGS").toInteger(), 0);
    a.checkEqual("27", ap("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("28", ap("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    a.checkNonNull("31", ap("ships")[1].getValue());
    a.checkEqual("32", ap("ships")[1]("NAME").toString(), "Ma Baker");
    a.checkEqual("33", ap("ships")[1]("HULL").toInteger(), 17);  // LDSF
    a.checkEqual("34", ap("ships")[1]("OWNER").toInteger(), 9);
    a.checkEqual("35", ap("ships")[1]("ID").toInteger(), 6);
    a.checkEqual("36", ap("ships")[1]("FCODE").toString(), "4R{");
    a.checkEqual("37", ap("ships")[1]("DAMAGE").toInteger(), 0);
    a.checkEqual("38", ap("ships")[1]("CREW").toInteger(), 102);
    a.checkEqual("39", ap("ships")[1]("BEAM.COUNT").toInteger(), 0);
    a.checkEqual("40", ap("ships")[1]("BEAM").toInteger(), 0);
    a.checkEqual("41", ap("ships")[1]("AUX").toInteger(), 0);
    a.checkEqual("42", ap("ships")[1]("AUX.COUNT").toInteger(), 0);
    a.checkEqual("43", ap("ships")[1]("AUX.AMMO").toInteger(), 0);
    a.checkEqual("44", ap("ships")[1]("ENGINE").toInteger(), 9);
    a.checkEqual("45", ap("ships")[1]("AGGRESSIVENESS").toInteger(), 0);
    a.checkEqual("46", ap("ships")[1]("FLAGS").toInteger(), (64+128)*65536 /* Elusive + ElusiveSet */);
    a.checkEqual("47", ap("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("48", ap("ships")[1]("LEVEL").toInteger(), 0);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    a.checkEqual("51", repacked, afl::string::fromBytes(game::test::getSimFileV5()));
}

/** Test error behaviour. */
AFL_TEST("server.format.SimPacker:error", a)
{
    // xref TestGameSimLoader::testError
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;

    // v0
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x73, 0x69, 0x6d, 0x1a, 0x02, 0x80, 0x43, 0x2e, 0x43, 0x2e,
        };
        AFL_CHECK_THROWS(a("01. v0"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v1
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x1a, 0x01, 0x80, 0x53, 0x68,
        };
        AFL_CHECK_THROWS(a("11. v1"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v2
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x31, 0x1a, 0x02, 0x80, 0x53, 0x68,
        };
        AFL_CHECK_THROWS(a("21. v2"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v3
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x32, 0x1a, 0x03, 0x80, 0x55, 0x6c,
        };
        AFL_CHECK_THROWS(a("31. v3"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v4
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x33, 0x1a, 0x01, 0x00, 0x53, 0x68,
        };
        AFL_CHECK_THROWS(a("41. v4"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v5
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34, 0x1a, 0x02, 0x00, 0x4d, 0x69,
        };
        AFL_CHECK_THROWS(a("51. v5"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // truncated signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34,
        };
        AFL_CHECK_THROWS(a("61. truncated"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // future signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x39, 0x1a,
        };
        AFL_CHECK_THROWS(a("71. future"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
        };
        AFL_CHECK_THROWS(a("81. bad signature"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43,
        };
        AFL_CHECK_THROWS(a("91. bad signature"), testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // empty file
    {
        AFL_CHECK_THROWS(a("101. empty"), testee.unpack(String_t(), cs), std::runtime_error);
    }
}
