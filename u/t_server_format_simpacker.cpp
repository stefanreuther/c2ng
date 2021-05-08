/**
  *  \file u/t_server_format_simpacker.cpp
  *  \brief Test for server::format::SimPacker
  *
  *  Test cases are the same as in TestGameSimLoader for game::sim::Loader.
  */

#include <memory>
#include <stdexcept>
#include "server/format/simpacker.hpp"

#include "t_server_format.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/data/access.hpp"
#include "game/test/files.hpp"

/** Test unpacking a V0 file. */
void
TestServerFormatSimPacker::testV0()
{
    // xref TestGameSimLoader::testV0
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV0()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 2U);
    TS_ASSERT(a("planet").getValue() != 0);

    // First ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "C.C.S.S. Joker");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 61);  // Emerald
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 7);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 117);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "NTP");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 258);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 8);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 7);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 3);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 40);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 7);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    TS_ASSERT(a("ships")[1].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[1]("NAME").toString(), "C.C.S.S. Claudrin II");
    TS_ASSERT_EQUALS(a("ships")[1]("HULL").toInteger(), 22);  // LCC
    TS_ASSERT_EQUALS(a("ships")[1]("OWNER").toInteger(), 7);
    TS_ASSERT_EQUALS(a("ships")[1]("ID").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("FCODE").toString(), "NTP");
    TS_ASSERT_EQUALS(a("ships")[1]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("CREW").toInteger(), 430);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM.COUNT").toInteger(), 4);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.COUNT").toInteger(), 3);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.AMMO").toInteger(), 50);
    TS_ASSERT_EQUALS(a("ships")[1]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[1]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("LEVEL").toInteger(), 0);

    // Planet
    TS_ASSERT(a("planet").getValue() != 0);
    TS_ASSERT_EQUALS(a("planet")("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("planet")("OWNER").toInteger(), 2);
    TS_ASSERT_EQUALS(a("planet")("FCODE").toString(), "i9m");
    TS_ASSERT_EQUALS(a("planet")("DEFENSE").toInteger(), 62);
    TS_ASSERT_EQUALS(a("planet")("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("LEVEL").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("TECH.BEAM").toInteger(), 0);
}

/** Test unpacking a V1 file. */
void
TestServerFormatSimPacker::testV1()
{
    // xref TestGameSimLoader::testV1
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV1()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 1U);
    TS_ASSERT(a("planet").getValue() != 0);

    // Ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "Ship 201");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 76);  // SSC
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 8);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 201);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 352);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 11);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 4);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 85);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 0);

    // Planet
    TS_ASSERT(a("planet").getValue() != 0);
    TS_ASSERT_EQUALS(a("planet")("ID").toInteger(), 459);
    TS_ASSERT_EQUALS(a("planet")("OWNER").toInteger(), 6);
    TS_ASSERT_EQUALS(a("planet")("FCODE").toString(), "NUK");
    TS_ASSERT_EQUALS(a("planet")("DEFENSE").toInteger(), 129);
    TS_ASSERT_EQUALS(a("planet")("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("LEVEL").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("TECH.BEAM").toInteger(), 1);
    TS_ASSERT_EQUALS(a("planet")("STORAGE.AMMO")[10].toInteger(), 22);
    TS_ASSERT_EQUALS(a("planet")("DEFENSE.BASE").toInteger(), 150);
    TS_ASSERT_EQUALS(a("planet")("TECH.TORPEDO").toInteger(), 1);
}

/** Test unpacking a V2 file. */
void
TestServerFormatSimPacker::testV2()
{
    // xref TestGameSimLoader::testV2
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV2()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 2U);

    // First ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "Ship 4");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 1);  // Outrider
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 4);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 180);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), 13 /* NoFuel */);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    TS_ASSERT(a("ships")[1].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[1]("NAME").toString(), "Ship 5");
    TS_ASSERT_EQUALS(a("ships")[1]("HULL").toInteger(), 73);  // Mig Scout
    TS_ASSERT_EQUALS(a("ships")[1]("OWNER").toInteger(), 8);
    TS_ASSERT_EQUALS(a("ships")[1]("ID").toInteger(), 5);
    TS_ASSERT_EQUALS(a("ships")[1]("FCODE").toString(), "123");
    TS_ASSERT_EQUALS(a("ships")[1]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("CREW").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM.COUNT").toInteger(), 2);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[1]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("LEVEL").toInteger(), 0);

    // Planet
    TS_ASSERT(a("planet").getValue() != 0);
    TS_ASSERT_EQUALS(a("planet")("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("planet")("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("planet")("FCODE").toString(), "NUK");
    TS_ASSERT_EQUALS(a("planet")("DEFENSE").toInteger(), 10);
    TS_ASSERT_EQUALS(a("planet")("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("LEVEL").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("TECH.BEAM").toInteger(), 0);
    // TS_ASSERT_EQUALS(a("planet")("STORAGE.AMMO")[10].toInteger(), 0); // not set
    // TS_ASSERT_EQUALS(a("planet")("DEFENSE.BASE").toInteger(), 0);     // not set
    // TS_ASSERT_EQUALS(a("planet")("TECH.TORPEDO").toInteger(), 0); // not set
}

/** Test unpacking a V3 file. */
void
TestServerFormatSimPacker::testV3()
{
    // xref TestGameSimLoader::testV3
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV3()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 3U);

    // First ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "Ultra Elite Alien");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 1);  // Outrider
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 58);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 6144 /* CommanderSet + Commander */);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 4);

     // Second ship
    TS_ASSERT(a("ships")[1].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[1]("NAME").toString(), "Recruit Alien");
    TS_ASSERT_EQUALS(a("ships")[1]("HULL").toInteger(), 1);  // Outrider
    TS_ASSERT_EQUALS(a("ships")[1]("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("ships")[1]("ID").toInteger(), 2);
    TS_ASSERT_EQUALS(a("ships")[1]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[1]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("CREW").toInteger(), 58);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[1]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("LEVEL").toInteger(), 0);

    // Third ship
    TS_ASSERT(a("ships")[2].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[2]("NAME").toString(), "Recruit Borg");
    TS_ASSERT_EQUALS(a("ships")[2]("HULL").toInteger(), 58);  // Quietus
    TS_ASSERT_EQUALS(a("ships")[2]("OWNER").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[2]("ID").toInteger(), 3);
    TS_ASSERT_EQUALS(a("ships")[2]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[2]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[2]("CREW").toInteger(), 517);
    TS_ASSERT_EQUALS(a("ships")[2]("BEAM.COUNT").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[2]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[2]("AUX.COUNT").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[2]("AUX").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[2]("AUX.AMMO").toInteger(), 260);
    TS_ASSERT_EQUALS(a("ships")[2]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[2]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[2]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[2]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[2]("LEVEL").toInteger(), 0);

    // Planet
    TS_ASSERT(a("planet").getValue() != 0);
    TS_ASSERT_EQUALS(a("planet")("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("planet")("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("planet")("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("planet")("DEFENSE").toInteger(), 10);
    TS_ASSERT_EQUALS(a("planet")("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("LEVEL").toInteger(), 0);
    TS_ASSERT_EQUALS(a("planet")("TECH.BEAM").toInteger(), 0);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(game::test::getSimFileV3()));
}

/** Test unpacking a V4 file. */
void
TestServerFormatSimPacker::testV4()
{
    // xref TestGameSimLoader::testV4
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV4()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 1U);
    TS_ASSERT(a("planet").getValue() == 0);

    // The ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "Ship 1");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 1);  // Outrider
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 12);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "?""?""?");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 58);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 10);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), -1);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 16 /* RatingOverride */);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("RATING.R").toInteger(), 240);
    TS_ASSERT_EQUALS(a("ships")[0]("RATING.C").toInteger(), 23);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(game::test::getSimFileV4()));
}

/** Test unpacking a V5 file. */
void
TestServerFormatSimPacker::testV5()
{
    // xref TestGameSimLoader::testV5
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;
    std::auto_ptr<afl::data::Value> p(testee.unpack(afl::string::fromBytes(game::test::getSimFileV5()), cs));
    afl::data::Access a(p);

    // Basic properties
    TS_ASSERT_EQUALS(a("ships").getArraySize(), 2U);
    TS_ASSERT(a("planet").getValue() == 0);

    // First ship
    TS_ASSERT(a("ships")[0].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[0]("NAME").toString(), "Mike Oldfield");
    TS_ASSERT_EQUALS(a("ships")[0]("HULL").toInteger(), 16);  // MDSF
    TS_ASSERT_EQUALS(a("ships")[0]("OWNER").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[0]("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("ships")[0]("FCODE").toString(), "_{=");
    TS_ASSERT_EQUALS(a("ships")[0]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("CREW").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("BEAM").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("ENGINE").toInteger(), 8);
    TS_ASSERT_EQUALS(a("ships")[0]("AGGRESSIVENESS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("FLAGS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[0]("LEVEL").toInteger(), 0);

    // Second ship
    TS_ASSERT(a("ships")[1].getValue() != 0);
    TS_ASSERT_EQUALS(a("ships")[1]("NAME").toString(), "Ma Baker");
    TS_ASSERT_EQUALS(a("ships")[1]("HULL").toInteger(), 17);  // LDSF
    TS_ASSERT_EQUALS(a("ships")[1]("OWNER").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("ID").toInteger(), 6);
    TS_ASSERT_EQUALS(a("ships")[1]("FCODE").toString(), "4R{");
    TS_ASSERT_EQUALS(a("ships")[1]("DAMAGE").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("CREW").toInteger(), 102);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("BEAM").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.COUNT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("AUX.AMMO").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("ENGINE").toInteger(), 9);
    TS_ASSERT_EQUALS(a("ships")[1]("AGGRESSIVENESS").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("FLAGS").toInteger(), (64+128)*65536 /* Elusive + ElusiveSet */);
    TS_ASSERT_EQUALS(a("ships")[1]("MISSION.INTERCEPT").toInteger(), 0);
    TS_ASSERT_EQUALS(a("ships")[1]("LEVEL").toInteger(), 0);

    // Re-pack
    String_t repacked = testee.pack(p.get(), cs);
    TS_ASSERT_EQUALS(repacked, afl::string::fromBytes(game::test::getSimFileV5()));
}

/** Test error behaviour. */
void
TestServerFormatSimPacker::testError()
{
    // xref TestGameSimLoader::testError
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    server::format::SimPacker testee;

    // v0
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x73, 0x69, 0x6d, 0x1a, 0x02, 0x80, 0x43, 0x2e, 0x43, 0x2e,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v1
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x1a, 0x01, 0x80, 0x53, 0x68,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v2
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x31, 0x1a, 0x02, 0x80, 0x53, 0x68,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v3
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x32, 0x1a, 0x03, 0x80, 0x55, 0x6c,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v4
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x33, 0x1a, 0x01, 0x00, 0x53, 0x68,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // v5
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34, 0x1a, 0x02, 0x00, 0x4d, 0x69,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // truncated signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x34,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // future signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x39, 0x1a,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x30, 0x00,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // bad signature
    {
        static const uint8_t FILE[] = {
            0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43,
        };
        TS_ASSERT_THROWS(testee.unpack(afl::string::fromBytes(FILE), cs), std::runtime_error);
    }

    // empty file
    {
        TS_ASSERT_THROWS(testee.unpack(String_t(), cs), std::runtime_error);
    }
}
