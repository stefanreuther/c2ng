/**
  *  \file test/game/v3/genfiletest.cpp
  *  \brief Test for game::v3::GenFile
  */

#include "game/v3/genfile.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.v3.GenFile:basics", a)
{
    afl::io::ConstMemoryStream ms(GEN8_DAT);
    GenFile t;
    t.loadFromFile(ms);

    // Verify attributes
    a.checkEqual("01. getPlayerId",     t.getPlayerId(), 8);
    a.checkEqual("02. getTurnNumber",   t.getTurnNumber(), 91);
    a.checkEqual("03. getTimestamp",    t.getTimestamp().getTimestampAsString(), "12-17-201517:48:02");
    a.checkEqual("04. hasPassword",     t.hasPassword(), false);

    a.checkEqual("11. NumPlanets",      t.getScore(1, GenFile::NumPlanets), 7);
    a.checkEqual("12. NumCapitalShips", t.getScore(1, GenFile::NumCapitalShips), 1);
    a.checkEqual("13. NumFreighters",   t.getScore(1, GenFile::NumFreighters), 0);
    a.checkEqual("14. NumBases",        t.getScore(1, GenFile::NumBases), 2);

    a.checkEqual("21. NumPlanets",      t.getScore(11, GenFile::NumPlanets), 75);
    a.checkEqual("22. NumCapitalShips", t.getScore(11, GenFile::NumCapitalShips), 67);
    a.checkEqual("23. NumFreighters",   t.getScore(11, GenFile::NumFreighters), 15);
    a.checkEqual("24. NumBases",        t.getScore(11, GenFile::NumBases), 55);

    a.checkEqual("31. NumPlanets",      t.getScore(0,   GenFile::NumPlanets), -1);
    a.checkEqual("32. NumPlanets",      t.getScore(12,  GenFile::NumPlanets), -1);
    a.checkEqual("33. NumPlanets",      t.getScore(123, GenFile::NumPlanets), -1);

    a.checkEqual("41. ShipSection",     t.getSectionChecksum(gt::ShipSection),   0x04FCA7u);
    a.checkEqual("42. PlanetSection",   t.getSectionChecksum(gt::PlanetSection), 0x076EDBu);
    a.checkEqual("43. BaseSection",     t.getSectionChecksum(gt::BaseSection),   0x008153u);

    // Must be able to reproduce the data
    gt::Gen data;
    t.getData(data);
    a.checkEqual("51. size", sizeof(data), sizeof(GEN8_DAT));
    a.checkEqualContent("52. content", afl::base::ConstBytes_t(afl::base::fromObject(data)), afl::base::ConstBytes_t(GEN8_DAT));

    // Must be able to construct from data
    GenFile t2(data);
    a.checkEqual("61. getTurnNumber", t2.getTurnNumber(), 91);
}

/** Test password access.
    A: set password.
    E: verify that password has been set */
AFL_TEST("game.v3.GenFile:password", a)
{
    GenFile t;

    t.setPassword("fun");
    a.check("01. hasPassword", t.hasPassword());
    a.check("02. isPassword", t.isPassword("fun"));

    t.setPassword("NOPASSWORD");
    a.check("11. hasPassword", !t.hasPassword());
}

/** Test result file access.
    A: load a RST file.
    E: verify loaded attributes */
AFL_TEST("game.v3.GenFile:rst", a)
{
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(game::test::getResultFile30());
    game::v3::ResultFile rst(ms, tx);

    afl::io::Stream::FileSize_t pos;
    a.check("01. getSectionOffset", rst.getSectionOffset(rst.GenSection, pos));
    ms.setPos(pos);

    GenFile t;
    t.loadFromResult(ms);

    a.checkEqual("11. getPlayerId",   t.getPlayerId(), 7);
    a.checkEqual("12. getTurnNumber", t.getTurnNumber(), 1);
    a.checkEqual("13. getTimestamp",  t.getTimestamp().getTimestampAsString(), "02-02-201620:44:02");
    a.checkEqual("14. hasPassword",   t.hasPassword(), false);
}

/** Test score extraction.
    A: load a file. Use copyScoresTo.
    E: verify correct scores */
AFL_TEST("game.v3.GenFile:score", a)
{
    afl::io::ConstMemoryStream ms(GEN8_DAT);
    GenFile t;
    t.loadFromFile(ms);

    game::score::TurnScoreList scores;
    t.copyScoresTo(scores);

    // Our file is turn 91, so we need to have that
    const game::score::TurnScore* score = scores.getTurn(91);
    a.check("01. getTurn", score);
    a.checkEqual("02. getTurnNumber", score->getTurnNumber(), 91);
    a.checkEqual("03. getTimestamp", score->getTimestamp().getTimestampAsString(), "12-17-201517:48:02");

    game::score::TurnScore::Slot_t pla = 0, cap = 0, fre = 0, bas = 0;
    a.check("11. ScoreId_Planets",    scores.getSlot(game::score::ScoreId_Planets)   .get(pla));
    a.check("12. ScoreId_Capital",    scores.getSlot(game::score::ScoreId_Capital)   .get(cap));
    a.check("13. ScoreId_Freighters", scores.getSlot(game::score::ScoreId_Freighters).get(fre));
    a.check("14. ScoreId_Bases",      scores.getSlot(game::score::ScoreId_Bases)     .get(bas));

    a.checkEqual("21. pla", score->get(pla, 1).orElse(-1), 7);
    a.checkEqual("22. cap", score->get(cap, 1).orElse(-1), 1);
    a.checkEqual("23. fre", score->get(fre, 1).orElse(-1), 0);
    a.checkEqual("24. bas", score->get(bas, 1).orElse(-1), 2);

    a.checkEqual("31. pla", score->get(pla, 11).orElse(-1), 75);
    a.checkEqual("32. cap", score->get(cap, 11).orElse(-1), 67);
    a.checkEqual("33. fre", score->get(fre, 11).orElse(-1), 15);
    a.checkEqual("34. bas", score->get(bas, 11).orElse(-1), 55);
}
