/**
  *  \file test/util/syntax/pascalhighlightertest.cpp
  *  \brief Test for util::syntax::PascalHighlighter
  */

#include "util/syntax/pascalhighlighter.hpp"

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
AFL_TEST("util.syntax.PascalHighlighter", a)
{
    util::syntax::PascalHighlighter testee;
    util::syntax::Segment r;

    // Simple mix
    testee.init(afl::string::toMemory("CONST foo = '17'; { doc }"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "CONST");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), " foo = ");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), "'17'");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "; ");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), "{ doc }");
    a.check("12. scan", !testee.scan(r));

    // Another simple mix
    testee.init(afl::string::toMemory("(*$I foo*) a = 1/2; // end"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::Comment2Format);
    a.checkEqual("23. parse", parseContinuation(testee, r), "(*$I foo*)");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), " a = 1/2; ");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), "// end");
    a.check("28. scan", !testee.scan(r));

    // Unterminated comment
    testee.init(afl::string::toMemory("{ foo"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "{ foo");
    a.check("34. scan", !testee.scan(r));

    // Unterminated comment
    testee.init(afl::string::toMemory("(* foo"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "(* foo");
    a.check("44. scan", !testee.scan(r));

    // Unterminated comment
    testee.init(afl::string::toMemory("(* foo *"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "(* foo *");
    a.check("54. scan", !testee.scan(r));

    // Paren
    testee.init(afl::string::toMemory("a:=b*(c+d)"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "a:=b*(c+d)");
    a.check("64. scan", !testee.scan(r));

    // Newline
    testee.init(afl::string::toMemory("a:=b\n+c;"));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("73. parse", parseContinuation(testee, r), "a:=b\n+c;");
    a.check("74. scan", !testee.scan(r));
}
