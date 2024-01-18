/**
  *  \file test/util/syntax/lisphighlightertest.cpp
  *  \brief Test for util::syntax::LispHighlighter
  */

#include "util/syntax/lisphighlighter.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("util.syntax.LispHighlighter", a)
{
    util::syntax::LispHighlighter testee;
    util::syntax::Segment r;

    // Simple command
    testee.init(afl::string::toMemory("(setq a ?\\\") ; doc\n(set 'b \"x\\ny\")"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "(setq a ?\\\") ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "; doc");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), "\n(set 'b ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "\"x\\ny\"");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), ")");
    a.checkEqual("12. scan", testee.scan(r), false);
}
