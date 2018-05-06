/**
  *  \file u/t_util_syntax_lisphighlighter.cpp
  *  \brief Test for util::syntax::LispHighlighter
  */

#include "util/syntax/lisphighlighter.hpp"

#include "t_util_syntax.hpp"
#include "util/syntax/segment.hpp"

namespace {
    /** Parse a continuation segment.
        The highlighter makes no guarantee about the size of individual segments and may spit out many small segments of the same format.
        This function collects continuation segments.
        Structure of a test therefore is:
        - perform initial "scan" invocation
        - repeatedly,
          - verify the segment format
          - call parseContinuation() and verify the result text. This will leave the next segment in seg. */
    String_t parseContinuation(util::syntax::Highlighter& hl, util::syntax::Segment& seg)
    {
        String_t result = afl::string::fromMemory(seg.getText());
        util::syntax::Format fmt = seg.getFormat();
        while (hl.scan(seg) && seg.getFormat() == fmt) {
            result += afl::string::fromMemory(seg.getText());
        }
        return result;
    }
}

/** Simple test. */
void
TestUtilSyntaxLispHighlighter::testIt()
{
    util::syntax::LispHighlighter testee;
    util::syntax::Segment r;

    // Simple command
    testee.init(afl::string::toMemory("(setq a ?\\\") ; doc\n(set 'b \"x\\ny\")"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "(setq a ?\\\") ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "; doc");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\n(set 'b ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"x\\ny\"");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ")");
    TS_ASSERT_EQUALS(testee.scan(r), false);
}

