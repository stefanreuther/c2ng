/**
  *  \file test/util/syntax/segmenttest.cpp
  *  \brief Test for util::syntax::Segment
  */

#include "util/syntax/segment.hpp"
#include "afl/test/testrunner.hpp"

/** Test setters/getters. */
AFL_TEST("util.syntax.Segment:basics", a)
{
    util::syntax::Segment testee;

    // Verify initial state
    a.checkEqual("01. getFormat", testee.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("02. getText",   afl::string::fromMemory(testee.getText()), "");
    a.checkEqual("03. getLink",   testee.getLink(), "");
    a.checkEqual("04. getInfo",   testee.getInfo(), "");

    // Set and verify
    testee.set(util::syntax::CommentFormat, afl::string::toMemory("/* x */"));
    testee.setLink("link");
    testee.setInfo("info");
    a.checkEqual("11. getFormat", testee.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("12. getText",   afl::string::fromMemory(testee.getText()), "/* x */");
    a.checkEqual("13. getLink",   testee.getLink(), "link");
    a.checkEqual("14. getInfo",   testee.getInfo(), "info");

    // Set. This clears link/info.
    testee.set(util::syntax::KeywordFormat, afl::string::toMemory("poke"));
    a.checkEqual("21. getFormat", testee.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("22. getText",   afl::string::fromMemory(testee.getText()), "poke");
    a.checkEqual("23. getLink",   testee.getLink(), "");
    a.checkEqual("24. getInfo",   testee.getInfo(), "");

    // Set format
    testee.setFormat(util::syntax::NameFormat);
    a.checkEqual("31. getFormat", testee.getFormat(), util::syntax::NameFormat);
}

/** Test start/finish. */
AFL_TEST("util.syntax.Segment:start+finish", a)
{
    util::syntax::Segment testee;

    // Define a token
    afl::string::ConstStringMemory_t mem(afl::string::toMemory("hello, world"));
    testee.start(mem);
    mem.split(5);
    testee.finish(util::syntax::StringFormat, mem);

    // Verify
    a.checkEqual("01. getFormat", testee.getFormat(), util::syntax::StringFormat);
    a.checkEqual("02. getText",   afl::string::fromMemory(testee.getText()), "hello");
    a.checkEqual("03. getLink",   testee.getLink(), "");
    a.checkEqual("04. getInfo",   testee.getInfo(), "");
}

/** Test initialisation. */
AFL_TEST("util.syntax.Segment:init", a)
{
    // Verify state after construction
    util::syntax::Segment testee(util::syntax::KeywordFormat, afl::string::toMemory("do"));
    a.checkEqual("01. getFormat", testee.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("02. getText",   afl::string::fromMemory(testee.getText()), "do");
    a.checkEqual("03. getLink",   testee.getLink(), "");
    a.checkEqual("04. getInfo",   testee.getInfo(), "");
}
