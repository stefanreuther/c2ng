/**
  *  \file test/server/common/racenamestest.cpp
  *  \brief Test for server::common::RaceNames
  */

#include "server/common/racenames.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"

/** Test success case.
    Load the default race name file and see that it arrives correctly. */
AFL_TEST("server.common.RaceNames:success", a)
{
    server::common::RaceNames testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    // We start out blank
    a.checkEqual("01. longNames",      testee.longNames().get(3), "");
    a.checkEqual("02. shortNames",     testee.shortNames().get(3), "");
    a.checkEqual("03. adjectiveNames", testee.adjectiveNames().get(3), "");

    // Load
    AFL_CHECK_SUCCEEDS(a("11. load"), testee.load(game::test::getDefaultRaceNames(), cs));

    // Verify
    a.checkEqual("21. longNames",      testee.longNames().get(1), "The Solar Federation");
    a.checkEqual("22. shortNames",     testee.shortNames().get(1), "The Feds");
    a.checkEqual("23. adjectiveNames", testee.adjectiveNames().get(1), "Fed");

    a.checkEqual("31. longNames",      testee.longNames().get(11), "The Missing Colonies of Man");
    a.checkEqual("32. shortNames",     testee.shortNames().get(11), "The Colonies");
    a.checkEqual("33. adjectiveNames", testee.adjectiveNames().get(11), "Colonial");

    // Out-of-range access
    a.checkEqual("41. shortNames", testee.shortNames().get(0), "");
    a.checkEqual("42. shortNames", testee.shortNames().get(100), "");
}

/** Test error cases.
    Too short files must be rejected. */
AFL_TEST("server.common.RaceNames:error", a)
{
    server::common::RaceNames testee;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

    AFL_CHECK_THROWS(a("01. empty"),     testee.load(afl::base::Nothing, cs), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("02. too short"), testee.load(afl::string::toBytes("hi"), cs), afl::except::FileProblemException);
}
