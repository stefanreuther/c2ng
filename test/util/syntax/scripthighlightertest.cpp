/**
  *  \file test/util/syntax/scripthighlightertest.cpp
  *  \brief Test for util::syntax::ScriptHighlighter
  */

#include "util/syntax/scripthighlighter.hpp"

#include "afl/test/testrunner.hpp"
#include "util/syntax/keywordtable.hpp"
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

/** Test behaviour with strings. */
AFL_TEST("util.syntax.ScriptHighlighter:string", a)
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // x := "foo\"mod" % xy"
    testee.init(afl::string::toMemory("x := \"foo\\\"mod\" % xy\""));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "x := ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "\"foo\\\"mod\"");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "% xy\"");
    a.check("10. scan", !testee.scan(r));

    // y := 'foo\'mod' % xy'
    testee.init(afl::string::toMemory("y := 'foo\\'mod' % xy'"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "y := ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "'foo\\'");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), "mod");
    a.checkEqual("18. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("19. parse", parseContinuation(testee, r), "' % xy'");
    a.check("20. scan", !testee.scan(r));
}

/** Test declaration commands. */
AFL_TEST("util.syntax.ScriptHighlighter:declarations", a)
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // sub foo(bar, optional baz)
    testee.init(afl::string::toMemory("sub foo(bar, optional baz)"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "sub");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), " ");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), "foo");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "(");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), "bar");
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), ", ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "optional");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), " ");
    a.checkEqual("18. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("19. parse", parseContinuation(testee, r), "baz");
    a.checkEqual("20. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("21. parse", parseContinuation(testee, r), ")");
    a.check("22. scan", !testee.scan(r));

    // local sub hurz()
    testee.init(afl::string::toMemory("local sub hurz()"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "local");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), " ");
    a.checkEqual("36. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("37. parse", parseContinuation(testee, r), "sub");
    a.checkEqual("38. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("39. parse", parseContinuation(testee, r), " ");
    a.checkEqual("40. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("41. parse", parseContinuation(testee, r), "hurz");
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "()");
    a.check("44. scan", !testee.scan(r));

    // endsub
    testee.init(afl::string::toMemory("endsub"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "endsub");
    a.check("54. scan", !testee.scan(r));

    // dim local i
    testee.init(afl::string::toMemory("dim local i"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "dim");
    a.checkEqual("64. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("65. parse", parseContinuation(testee, r), " ");
    a.checkEqual("66. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("67. parse", parseContinuation(testee, r), "local");
    a.checkEqual("68. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("69. parse", parseContinuation(testee, r), " ");
    a.checkEqual("70. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("71. parse", parseContinuation(testee, r), "i");
    a.check("72. scan", !testee.scan(r));

    // dim a(1),b
    testee.init(afl::string::toMemory("dim a(1),b"));
    a.check("81. scan", testee.scan(r));
    a.checkEqual("82. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("83. parse", parseContinuation(testee, r), "dim");
    a.checkEqual("84. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("85. parse", parseContinuation(testee, r), " ");
    a.checkEqual("86. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("87. parse", parseContinuation(testee, r), "a");
    a.checkEqual("88. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("89. parse", parseContinuation(testee, r), "(1),");
    a.checkEqual("90. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("91. parse", parseContinuation(testee, r), "b");
    a.check("92. scan", !testee.scan(r));

    // sub foo(bar(baz)) - the "baz" is not a name
    testee.init(afl::string::toMemory("sub foo(bar(baz))"));
    a.check("101. scan", testee.scan(r));
    a.checkEqual("102. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("103. parse", parseContinuation(testee, r), "sub");
    a.checkEqual("104. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("105. parse", parseContinuation(testee, r), " ");
    a.checkEqual("106. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("107. parse", parseContinuation(testee, r), "foo");
    a.checkEqual("108. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("109. parse", parseContinuation(testee, r), "(");
    a.checkEqual("110. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("111. parse", parseContinuation(testee, r), "bar");
    a.checkEqual("112. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("113. parse", parseContinuation(testee, r), "(baz))");
    a.check("114. scan", !testee.scan(r));
}

/** Test commands. */
AFL_TEST("util.syntax.ScriptHighlighter:commands", a)
{
    util::syntax::KeywordTable table;
    util::syntax::ScriptHighlighter testee(table);
    util::syntax::Segment r;

    // if this then that
    testee.init(afl::string::toMemory("if this then that"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "if");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), " this ");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), "then");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), " that");
    a.check("10. scan", !testee.scan(r));

    // if this then that
    testee.init(afl::string::toMemory("if this then\nthat"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "if");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), " this ");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), "then");
    a.checkEqual("18. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("19. parse", parseContinuation(testee, r), "\nthat");
    a.check("20. scan", !testee.scan(r));

    // for i:=a to b do c
    testee.init(afl::string::toMemory("for i:=a to b do c"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "for");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), " i:=a ");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), "to");
    a.checkEqual("28. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("29. parse", parseContinuation(testee, r), " b ");
    a.checkEqual("30. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("31. parse", parseContinuation(testee, r), "do");
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), " c");
    a.check("34. scan", !testee.scan(r));

    // case is > 3
    testee.init(afl::string::toMemory("case is > 3"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "case");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), " ");
    a.checkEqual("46. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("47. parse", parseContinuation(testee, r), "is");
    a.checkEqual("48. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("49. parse", parseContinuation(testee, r), " > 3");
    a.check("50. scan", !testee.scan(r));

    // a := b xor c
    testee.init(afl::string::toMemory("a := b xor c"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "a := b ");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), "xor");
    a.checkEqual("56. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("57. parse", parseContinuation(testee, r), " c");
    a.check("58. scan", !testee.scan(r));

    // what is love? baby dont hurt me -- "is" is not a keyword here, and the "?" should not confuse us
    testee.init(afl::string::toMemory("what is love? baby dont hurt me"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "what is love? baby dont hurt me");
    a.check("64. scan", !testee.scan(r));

    // a:=true.or (not a keyword)
    testee.init(afl::string::toMemory("a:=true.or"));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("73. parse", parseContinuation(testee, r), "a:=true.or");
    a.check("74. scan", !testee.scan(r));

    // text with newlines
    testee.init(afl::string::toMemory("a\nb\nc"));
    a.check("81. scan", testee.scan(r));
    a.checkEqual("82. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("83. parse", parseContinuation(testee, r), "a\nb\nc");
    a.check("84. scan", !testee.scan(r));

    // Loop Until x
    testee.init(afl::string::toMemory("Loop Until x"));
    a.check("91. scan", testee.scan(r));
    a.checkEqual("92. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("93. parse", parseContinuation(testee, r), "Loop");
    a.checkEqual("94. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("95. parse", parseContinuation(testee, r), " ");
    a.checkEqual("96. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("97. parse", parseContinuation(testee, r), "Until");
    a.checkEqual("98. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("99. parse", parseContinuation(testee, r), " x");
    a.check("100. scan", !testee.scan(r));

    // With a Do b
    testee.init(afl::string::toMemory("With a Do b"));
    a.check("101. scan", testee.scan(r));
    a.checkEqual("102. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("103. parse", parseContinuation(testee, r), "With");
    a.checkEqual("104. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("105. parse", parseContinuation(testee, r), " a ");
    a.checkEqual("106. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("107. parse", parseContinuation(testee, r), "Do");
    a.checkEqual("108. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("109. parse", parseContinuation(testee, r), " b");
    a.check("110. scan", !testee.scan(r));

    // Dim a As Int
    testee.init(afl::string::toMemory("Dim a As Int"));
    a.check("111. scan", testee.scan(r));
    a.checkEqual("112. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("113. parse", parseContinuation(testee, r), "Dim");
    a.checkEqual("114. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("115. parse", parseContinuation(testee, r), " ");
    a.checkEqual("116. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("117. parse", parseContinuation(testee, r), "a");
    a.checkEqual("118. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("119. parse", parseContinuation(testee, r), " ");
    a.checkEqual("120. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("121. parse", parseContinuation(testee, r), "As");
    a.checkEqual("122. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("123. parse", parseContinuation(testee, r), " Int");
    a.check("124. scan", !testee.scan(r));
}
