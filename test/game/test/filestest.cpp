/**
  *  \file test/game/test/filestest.cpp
  *  \brief Test for game::test::Files
  */

#include "game/test/files.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/checksums/adler32.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"

/** Test file content.
    Primarily intended to validate ports. */
AFL_TEST("game.test.Files:file-content", a)
{
    afl::checksums::Adler32 cksum;
    a.checkEqual("01", cksum.add(game::test::getResultFile30(),             1), 0x95e76de0U);
    a.checkEqual("02", cksum.add(game::test::getResultFile35(),             1), 0xf7067982U);
    a.checkEqual("03", cksum.add(game::test::getComplexResultFile(),        1), 0xc64b5ae2U);
    a.checkEqual("04", cksum.add(game::test::getSimFileV0(),                1), 0xf31513b1U);
    a.checkEqual("05", cksum.add(game::test::getSimFileV1(),                1), 0x2cee0ebeU);
    a.checkEqual("06", cksum.add(game::test::getSimFileV2(),                1), 0x49e61340U);
    a.checkEqual("07", cksum.add(game::test::getSimFileV3(),                1), 0xd92323ceU);
    a.checkEqual("08", cksum.add(game::test::getSimFileV4(),                1), 0xcfeb0b1bU);
    a.checkEqual("09", cksum.add(game::test::getSimFileV5(),                1), 0x01631173U);
    a.checkEqual("10", cksum.add(game::test::getDefaultRegKey(),            1), 0xed1138daU);
    a.checkEqual("11", cksum.add(game::test::getDefaultRaceNames(),         1), 0xe372be16U);
    a.checkEqual("12", cksum.add(game::test::getDefaultPlanetCoordinates(), 1), 0x0e060a5dU);
    a.checkEqual("13", cksum.add(game::test::getDefaultPlanetNames(),       1), 0x16ddfaa3U);
    a.checkEqual("14", cksum.add(game::test::getDefaultBeams(),             1), 0xfe0f372aU);
    a.checkEqual("15", cksum.add(game::test::getDefaultTorpedoes(),         1), 0x626639f2U);
    a.checkEqual("16", cksum.add(game::test::getDefaultHulls(),             1), 0x824c3decU);
    a.checkEqual("17", cksum.add(game::test::getDefaultEngines(),           1), 0xc9ac6a41U);
    a.checkEqual("18", cksum.add(game::test::getDefaultHullAssignments(),   1), 0xd40525beU);
    a.checkEqual("19", cksum.add(game::test::getDefaultIonStormNames(),     1), 0x86b8cd14U);
}

/** Test makeEmptyResult(). */
AFL_TEST("game.test.Files:makeEmptyResult", a)
{
    using game::v3::ResultFile;

    // Coarse check
    afl::base::GrowableBytes_t data = game::test::makeEmptyResult(3, 70, game::Timestamp(2003, 12, 10, 12, 0, 0));
    a.checkGreaterThan("01. size", data.size(), 1000U);

    // Check interoperability with ResultFile
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(data);
    ResultFile rst(ms, tx);
    a.check("11. GenSection", rst.hasSection(ResultFile::GenSection));
    a.check("12. ShipSection", rst.hasSection(ResultFile::ShipSection));

    // Check content
    game::v3::structures::ResultGen g;
    rst.seekToSection(ResultFile::GenSection);
    ms.fullRead(afl::base::fromObject(g));
    a.checkEqual("21. playerId",   int(g.playerId), 3);
    a.checkEqual("22. turnNumber", int(g.turnNumber), 70);
}

/** Test makeGenFile. */
AFL_TEST("game.test.Files:makeGenFile", a)
{
    afl::base::GrowableBytes_t data = game::test::makeGenFile(9, 28, game::Timestamp(2003, 12, 10, 12, 0, 0));
    game::v3::structures::Gen g;
    a.checkEqual("01. size", data.size(), sizeof(g));
    afl::base::fromObject(g).copyFrom(data);
    a.checkEqual("02. playerId",   int(g.playerId), 9);
    a.checkEqual("03. turnNumber", int(g.turnNumber), 28);
}

/** Test makeSimpleTurn(). */
AFL_TEST("game.test.Files:makeSimpleTurn", a)
{
    using game::v3::TurnFile;

    // Coarse check
    afl::base::GrowableBytes_t data = game::test::makeSimpleTurn(3, game::Timestamp(2003, 12, 10, 12, 0, 0));
    a.checkGreaterThan("01. size", data.size(), 100U);

    // Check interoperability with TurnFile
    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(data);
    TurnFile tf(cs, tx, ms);
    a.checkEqual("11. getPlayer",      tf.getPlayer(), 3);
    a.checkEqual("12. getNumCommands", tf.getNumCommands(), 1U);

    TurnFile::CommandCode_t cc;
    a.checkEqual("21. getCommandCode", tf.getCommandCode(0, cc), true);
    a.checkEqual("22. command code",   cc, TurnFile::CommandCode_t(game::v3::tcm_SendMessage));
}
