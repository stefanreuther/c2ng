/**
  *  \file u/t_util_syntax_segment.cpp
  *  \brief Test for util::syntax::Segment
  */

#include "util/syntax/segment.hpp"

#include "t_util_syntax.hpp"

/** Test setters/getters. */
void
TestUtilSyntaxSegment::testSet()
{
    util::syntax::Segment testee;

    // Verify initial state
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(testee.getText()), "");
    TS_ASSERT_EQUALS(testee.getLink(), "");
    TS_ASSERT_EQUALS(testee.getInfo(), "");

    // Set and verify
    testee.set(util::syntax::CommentFormat, afl::string::toMemory("/* x */"));
    testee.setLink("link");
    testee.setInfo("info");
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(testee.getText()), "/* x */");
    TS_ASSERT_EQUALS(testee.getLink(), "link");
    TS_ASSERT_EQUALS(testee.getInfo(), "info");

    // Set. This clears link/info.
    testee.set(util::syntax::KeywordFormat, afl::string::toMemory("poke"));
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(testee.getText()), "poke");
    TS_ASSERT_EQUALS(testee.getLink(), "");
    TS_ASSERT_EQUALS(testee.getInfo(), "");

    // Set format
    testee.setFormat(util::syntax::NameFormat);
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::NameFormat);
}

/** Test start/finish. */
void
TestUtilSyntaxSegment::testStartFinish()
{
    util::syntax::Segment testee;

    // Define a token
    afl::string::ConstStringMemory_t mem(afl::string::toMemory("hello, world"));
    testee.start(mem);
    mem.split(5);
    testee.finish(util::syntax::StringFormat, mem);

    // Verify
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(testee.getText()), "hello");
    TS_ASSERT_EQUALS(testee.getLink(), "");
    TS_ASSERT_EQUALS(testee.getInfo(), "");
}

/** Test initialisation. */
void
TestUtilSyntaxSegment::testInit()
{
    // Verify state after construction
    util::syntax::Segment testee(util::syntax::KeywordFormat, afl::string::toMemory("do"));
    TS_ASSERT_EQUALS(testee.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(testee.getText()), "do");
    TS_ASSERT_EQUALS(testee.getLink(), "");
    TS_ASSERT_EQUALS(testee.getInfo(), "");
}

