/**
  *  \file u/t_game_test_files.cpp
  *  \brief Test for game::test::Files
  */

#include "game/test/files.hpp"

#include "t_game_test.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/checksums/adler32.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"

/** Test file content.
    Primarily intended to validate ports. */
void
TestGameTestFiles::testFiles()
{
    afl::checksums::Adler32 cksum;
    TS_ASSERT_EQUALS(cksum.add(game::test::getResultFile30(),             1), 0x95e76de0U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getResultFile35(),             1), 0xf7067982U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getComplexResultFile(),        1), 0xc64b5ae2U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV0(),                1), 0xf31513b1U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV1(),                1), 0x2cee0ebeU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV2(),                1), 0x49e61340U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV3(),                1), 0xd92323ceU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV4(),                1), 0xcfeb0b1bU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getSimFileV5(),                1), 0x01631173U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultRegKey(),            1), 0xed1138daU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultRaceNames(),         1), 0xe372be16U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultPlanetCoordinates(), 1), 0x0e060a5dU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultPlanetNames(),       1), 0x16ddfaa3U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultBeams(),             1), 0xfe0f372aU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultTorpedoes(),         1), 0x626639f2U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultHulls(),             1), 0x824c3decU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultEngines(),           1), 0xc9ac6a41U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultHullAssignments(),   1), 0xd40525beU);
}

/** Test makeEmptyResult(). */
void
TestGameTestFiles::testMakeEmptyResult()
{
    using game::v3::ResultFile;

    // Coarse check
    afl::base::GrowableBytes_t data = game::test::makeEmptyResult(3, 70, game::Timestamp(2003, 12, 10, 12, 0, 0));
    TS_ASSERT(data.size() > 1000);

    // Check interoperability with ResultFile
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(data);
    ResultFile rst(ms, tx);
    TS_ASSERT(rst.hasSection(ResultFile::GenSection));
    TS_ASSERT(rst.hasSection(ResultFile::ShipSection));

    // Check content
    game::v3::structures::ResultGen g;
    rst.seekToSection(ResultFile::GenSection);
    ms.fullRead(afl::base::fromObject(g));
    TS_ASSERT_EQUALS(int(g.playerId), 3);
    TS_ASSERT_EQUALS(int(g.turnNumber), 70);
}

/** Test makeGenFile. */
void
TestGameTestFiles::testMakeGen()
{
    afl::base::GrowableBytes_t data = game::test::makeGenFile(9, 28, game::Timestamp(2003, 12, 10, 12, 0, 0));
    game::v3::structures::Gen g;
    TS_ASSERT_EQUALS(data.size(), sizeof(g));
    afl::base::fromObject(g).copyFrom(data);
    TS_ASSERT_EQUALS(int(g.playerId), 9);
    TS_ASSERT_EQUALS(int(g.turnNumber), 28);
}

/** Test makeSimpleTurn(). */
void
TestGameTestFiles::testMakeSimpleTurn()
{
    using game::v3::TurnFile;

    // Coarse check
    afl::base::GrowableBytes_t data = game::test::makeSimpleTurn(3, game::Timestamp(2003, 12, 10, 12, 0, 0));
    TS_ASSERT(data.size() > 100);

    // Check interoperability with TurnFile
    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(data);
    TurnFile tf(cs, tx, ms);
    TS_ASSERT_EQUALS(tf.getPlayer(), 3);
    TS_ASSERT_EQUALS(tf.getNumCommands(), 1U);

    TurnFile::CommandCode_t cc;
    TS_ASSERT_EQUALS(tf.getCommandCode(0, cc), true);
    TS_ASSERT_EQUALS(cc, TurnFile::CommandCode_t(game::v3::tcm_SendMessage));
}

