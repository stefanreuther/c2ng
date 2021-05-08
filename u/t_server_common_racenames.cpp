/**
  *  \file u/t_server_common_racenames.cpp
  *  \brief Test for server::common::RaceNames
  */

#include "server/common/racenames.hpp"

#include "t_server_common.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/test/files.hpp"

/** Test success case.
    Load the default race name file and see that it arrives correctly. */
void
TestServerCommonRaceNames::testSuccess()
{
    server::common::RaceNames testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    // We start out blank
    TS_ASSERT_EQUALS(testee.longNames().get(3), "");
    TS_ASSERT_EQUALS(testee.shortNames().get(3), "");
    TS_ASSERT_EQUALS(testee.adjectiveNames().get(3), "");

    // Load
    TS_ASSERT_THROWS_NOTHING(testee.load(game::test::getDefaultRaceNames(), cs));

    // Verify
    TS_ASSERT_EQUALS(testee.longNames().get(1), "The Solar Federation");
    TS_ASSERT_EQUALS(testee.shortNames().get(1), "The Feds");
    TS_ASSERT_EQUALS(testee.adjectiveNames().get(1), "Fed");

    TS_ASSERT_EQUALS(testee.longNames().get(11), "The Missing Colonies of Man");
    TS_ASSERT_EQUALS(testee.shortNames().get(11), "The Colonies");
    TS_ASSERT_EQUALS(testee.adjectiveNames().get(11), "Colonial");

    // Out-of-range access
    TS_ASSERT_EQUALS(testee.shortNames().get(0), "");
    TS_ASSERT_EQUALS(testee.shortNames().get(100), "");
}

/** Test error cases.
    Too short files must be rejected. */
void
TestServerCommonRaceNames::testError()
{
    server::common::RaceNames testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    TS_ASSERT_THROWS(testee.load(afl::base::Nothing, cs), afl::except::FileProblemException);
    TS_ASSERT_THROWS(testee.load(afl::string::toBytes("hi"), cs), afl::except::FileProblemException);
}
