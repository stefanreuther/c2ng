/**
  *  \file u/t_game_v3_genfile.cpp
  *  \brief Test for game::v3::GenFile
  */

#include "game/v3/genfile.hpp"

#include "t_game_v3.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/test/files.hpp"
#include "game/v3/resultfile.hpp"

namespace {
    using game::v3::GenFile;
    namespace gt = game::v3::structures;

    const uint8_t GEN8_DAT[] = {
        0x31, 0x32, 0x2d, 0x31, 0x37, 0x2d, 0x32, 0x30, 0x31, 0x35, 0x31, 0x37,
        0x3a, 0x34, 0x38, 0x3a, 0x30, 0x32, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x48, 0x00, 0x37, 0x00, 0x09, 0x00, 0x24, 0x00, 0x20, 0x00,
        0x0e, 0x00, 0x05, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x3d, 0x00, 0x81, 0x00, 0x12, 0x00, 0x29, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x21, 0x00, 0x08, 0x00,
        0x0a, 0x00, 0x6e, 0x00, 0x2e, 0x00, 0x14, 0x00, 0x39, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5b, 0x00, 0x44, 0x00, 0x0b, 0x00,
        0x48, 0x00, 0x4b, 0x00, 0x43, 0x00, 0x0f, 0x00, 0x37, 0x00, 0x08, 0x00,
        0x73, 0x6a, 0x69, 0x49, 0x58, 0x62, 0x6d, 0x5f, 0x6d, 0x50, 0x2c, 0x3b,
        0x30, 0x36, 0x2f, 0x25, 0x28, 0x39, 0x3b, 0x45, 0x00, 0xa7, 0xfc, 0x04,
        0x00, 0xdb, 0x6e, 0x07, 0x00, 0x53, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5b, 0x00, 0x97,
        0x03
    };
}

/** Test file access.
    A: load a file.
    E: verify loaded attributes */
void
TestGameV3GenFile::testFile()
{
    afl::io::ConstMemoryStream ms(GEN8_DAT);
    GenFile t;
    t.loadFromFile(ms);

    // Verify attributes
    TS_ASSERT_EQUALS(t.getPlayerId(), 8);
    TS_ASSERT_EQUALS(t.getTurnNumber(), 91);
    TS_ASSERT_EQUALS(t.getTimestamp().getTimestampAsString(), "12-17-201517:48:02");
    TS_ASSERT_EQUALS(t.hasPassword(), false);

    TS_ASSERT_EQUALS(t.getScore(1, GenFile::NumPlanets), 7);
    TS_ASSERT_EQUALS(t.getScore(1, GenFile::NumCapitalShips), 1);
    TS_ASSERT_EQUALS(t.getScore(1, GenFile::NumFreighters), 0);
    TS_ASSERT_EQUALS(t.getScore(1, GenFile::NumBases), 2);

    TS_ASSERT_EQUALS(t.getScore(11, GenFile::NumPlanets), 75);
    TS_ASSERT_EQUALS(t.getScore(11, GenFile::NumCapitalShips), 67);
    TS_ASSERT_EQUALS(t.getScore(11, GenFile::NumFreighters), 15);
    TS_ASSERT_EQUALS(t.getScore(11, GenFile::NumBases), 55);

    TS_ASSERT_EQUALS(t.getScore(0,   GenFile::NumPlanets), -1);
    TS_ASSERT_EQUALS(t.getScore(12,  GenFile::NumPlanets), -1);
    TS_ASSERT_EQUALS(t.getScore(123, GenFile::NumPlanets), -1);

    TS_ASSERT_EQUALS(t.getSectionChecksum(gt::ShipSection),   0x04FCA7u);
    TS_ASSERT_EQUALS(t.getSectionChecksum(gt::PlanetSection), 0x076EDBu);
    TS_ASSERT_EQUALS(t.getSectionChecksum(gt::BaseSection),   0x008153u);

    // Must be able to reproduce the data
    gt::Gen data;
    t.getData(data);
    TS_ASSERT_EQUALS(sizeof(data), sizeof(GEN8_DAT));
    TS_ASSERT_SAME_DATA(&data, &GEN8_DAT, sizeof(data));

    // Must be able to construct from data
    GenFile t2(data);
    TS_ASSERT_EQUALS(t2.getTurnNumber(), 91);
}

/** Test password access.
    A: set password.
    E: verify that password has been set */
void
TestGameV3GenFile::testPassword()
{
    GenFile t;

    t.setPassword("fun");
    TS_ASSERT(t.hasPassword());
    TS_ASSERT(t.isPassword("fun"));

    t.setPassword("NOPASSWORD");
    TS_ASSERT(!t.hasPassword());
}

/** Test result file access.
    A: load a RST file.
    E: verify loaded attributes */
void
TestGameV3GenFile::testResult()
{
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(game::test::getResultFile30());
    game::v3::ResultFile rst(ms, tx);

    afl::io::Stream::FileSize_t pos;
    TS_ASSERT(rst.getSectionOffset(rst.GenSection, pos));
    ms.setPos(pos);
        
    GenFile t;
    t.loadFromResult(ms);

    TS_ASSERT_EQUALS(t.getPlayerId(), 7);
    TS_ASSERT_EQUALS(t.getTurnNumber(), 1);
    TS_ASSERT_EQUALS(t.getTimestamp().getTimestampAsString(), "02-02-201620:44:02");
    TS_ASSERT_EQUALS(t.hasPassword(), false);
}

/** Test score extraction.
    A: load a file. Use copyScoresTo.
    E: verify correct scores */
void
TestGameV3GenFile::testScore()
{
    afl::io::ConstMemoryStream ms(GEN8_DAT);
    GenFile t;
    t.loadFromFile(ms);

    game::score::TurnScoreList scores;
    t.copyScoresTo(scores);

    // Our file is turn 91, so we need to have that
    const game::score::TurnScore* score = scores.getTurn(91);
    TS_ASSERT(score);
    TS_ASSERT_EQUALS(score->getTurnNumber(), 91);
    TS_ASSERT_EQUALS(score->getTimestamp().getTimestampAsString(), "12-17-201517:48:02");

    game::score::TurnScore::Slot_t pla, cap, fre, bas;
    TS_ASSERT(scores.getSlot(game::score::ScoreId_Planets,    pla));
    TS_ASSERT(scores.getSlot(game::score::ScoreId_Capital,    cap));
    TS_ASSERT(scores.getSlot(game::score::ScoreId_Freighters, fre));
    TS_ASSERT(scores.getSlot(game::score::ScoreId_Bases,      bas));

    TS_ASSERT_EQUALS(score->get(pla, 1).orElse(-1), 7);
    TS_ASSERT_EQUALS(score->get(cap, 1).orElse(-1), 1);
    TS_ASSERT_EQUALS(score->get(fre, 1).orElse(-1), 0);
    TS_ASSERT_EQUALS(score->get(bas, 1).orElse(-1), 2);

    TS_ASSERT_EQUALS(score->get(pla, 11).orElse(-1), 75);
    TS_ASSERT_EQUALS(score->get(cap, 11).orElse(-1), 67);
    TS_ASSERT_EQUALS(score->get(fre, 11).orElse(-1), 15);
    TS_ASSERT_EQUALS(score->get(bas, 11).orElse(-1), 55);
}

