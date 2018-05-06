/**
  *  \file u/t_util_syntax_pascalhighlighter.cpp
  *  \brief Test for util::syntax::PascalHighlighter
  */

#include "util/syntax/pascalhighlighter.hpp"

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
TestUtilSyntaxPascalHighlighter::testIt()
{
    util::syntax::PascalHighlighter testee;
    util::syntax::Segment r;

    // Simple command
    testee.init(afl::string::toMemory("CONST foo = '17'; { doc }"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "CONST");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo = ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "'17'");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "; ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "{ doc }");
    TS_ASSERT(!testee.scan(r));

    // Simple command
    testee.init(afl::string::toMemory("(*$I foo*) a = 1/2; // end"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::Comment2Format);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "(*$I foo*)");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a = 1/2; ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "// end");
    TS_ASSERT(!testee.scan(r));
}

