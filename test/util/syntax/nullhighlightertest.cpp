/**
  *  \file test/util/syntax/nullhighlightertest.cpp
  *  \brief Test for util::syntax::NullHighlighter
  */

#include "util/syntax/nullhighlighter.hpp"

#include "afl/test/testrunner.hpp"
#include "util/syntax/segment.hpp"

/** Simple test. */
AFL_TEST("util.syntax.NullHighlighter", a)
{
    util::syntax::NullHighlighter testee;
    util::syntax::Segment seg;

    // Initial state: no result
    a.check("01", !testee.scan(seg));

    // Initialize with string
    testee.init(afl::string::toMemory("foobar"));
    a.check("11", testee.scan(seg));
    a.checkEqual("12. getFormat", seg.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. getText", afl::string::fromMemory(seg.getText()), "foobar");
    a.check("14", !testee.scan(seg));
    a.check("15", !testee.scan(seg));
    a.check("16", !testee.scan(seg));

    // Initialize with empty
    testee.init(afl::base::Nothing);
    a.check("21", !testee.scan(seg));

    // Initialize with empty
    testee.init(afl::string::ConstStringMemory_t());
    a.check("31", !testee.scan(seg));
}
