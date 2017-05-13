/**
  *  \file u/t_util_syntax_nullhighlighter.cpp
  *  \brief Test for util::syntax::NullHighlighter
  */

#include "util/syntax/nullhighlighter.hpp"

#include "t_util_syntax.hpp"
#include "util/syntax/segment.hpp"

/** Simple test. */
void
TestUtilSyntaxNullHighlighter::testIt()
{
    util::syntax::NullHighlighter testee;
    util::syntax::Segment seg;

    // Initial state: no result
    TS_ASSERT(!testee.scan(seg));

    // Initialize with string
    testee.init(afl::string::toMemory("foobar"));
    TS_ASSERT(testee.scan(seg));
    TS_ASSERT_EQUALS(seg.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(afl::string::fromMemory(seg.getText()), "foobar");
    TS_ASSERT(!testee.scan(seg));
    TS_ASSERT(!testee.scan(seg));
    TS_ASSERT(!testee.scan(seg));

    // Initialize with empty
    testee.init(afl::base::Nothing);
    TS_ASSERT(!testee.scan(seg));

    // Initialize with empty
    testee.init(afl::string::ConstStringMemory_t());
    TS_ASSERT(!testee.scan(seg));
}

