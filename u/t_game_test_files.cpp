/**
  *  \file u/t_game_test_files.cpp
  *  \brief Test for game::test::Files
  */

#include "game/test/files.hpp"

#include "t_game_test.hpp"
#include "afl/checksums/adler32.hpp"

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
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultHulls(),             1), 0x824c3decU);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultEngines(),           1), 0xc9ac6a41U);
    TS_ASSERT_EQUALS(cksum.add(game::test::getDefaultHullAssignments(),   1), 0xd40525beU);
}

