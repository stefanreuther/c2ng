/**
  *  \file test/util/syntax/inihighlightertest.cpp
  *  \brief Test for util::syntax::IniHighlighter
  */

#include "util/syntax/inihighlighter.hpp"

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

/** Test comments. */
AFL_TEST("util.syntax.IniHighlighter:comments", a)
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "x");
    util::syntax::Segment r;

    // Single comment
    testee.init(afl::string::toMemory(" # x"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), " ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "# x");
    a.check("06. scan", !testee.scan(r));

    // Single comment + newline
    testee.init(afl::string::toMemory(" # x\n"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), " ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "# x");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), "\n");
    a.check("18. scan", !testee.scan(r));

    // Section comment
    testee.init(afl::string::toMemory(" ## x"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), " ");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::Comment2Format);
    a.checkEqual("25. parse", parseContinuation(testee, r), "## x");
    a.check("26. scan", !testee.scan(r));

    // Single comment with semicolon
    testee.init(afl::string::toMemory(" ; x"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), " ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "; x");
    a.check("36. scan", !testee.scan(r));

    // Section comment with semicolon
    testee.init(afl::string::toMemory(" ;; x"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), " ");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::Comment2Format);
    a.checkEqual("45. parse", parseContinuation(testee, r), ";; x");
    a.check("46. scan", !testee.scan(r));

    // Variants...
    testee.init(afl::string::toMemory(" ;# x"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), " ");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), ";# x");
    a.check("56. scan", !testee.scan(r));

    testee.init(afl::string::toMemory("#x"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "#x");
    a.check("64. scan", !testee.scan(r));
}

/** Test sections. */
AFL_TEST("util.syntax.IniHighlighter:sections", a)
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "x");
    util::syntax::Segment r;

    // Brackets
    testee.init(afl::string::toMemory("[foo]"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "[foo]");
    a.check("04. scan", !testee.scan(r));

    // ...with newline
    testee.init(afl::string::toMemory("[foo]\n"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "\n");
    a.check("16. scan", !testee.scan(r));

    // ...with space
    testee.init(afl::string::toMemory("[foo]  \ni=1"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), "  \n");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), "i");
    a.checkEqual("28. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("29. parse", parseContinuation(testee, r), "=1");
    a.check("30. scan", !testee.scan(r));

    // ...indented
    testee.init(afl::string::toMemory("    [foo]"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "    ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "[foo]");
    a.check("36. scan", !testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("[foo]#bar"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), "#bar");
    a.check("46. scan", !testee.scan(r));

    // ...with comment and newline
    testee.init(afl::string::toMemory("[foo]#bar\ni=1"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), "#bar\n");
    a.checkEqual("56. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("57. parse", parseContinuation(testee, r), "i");
    a.checkEqual("58. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("59. parse", parseContinuation(testee, r), "=1");
    a.check("60. scan", !testee.scan(r));

    // ...with space and comment
    testee.init(afl::string::toMemory("[foo]  #bar"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("64. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("65. parse", parseContinuation(testee, r), "  ");
    a.checkEqual("66. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("67. parse", parseContinuation(testee, r), "#bar");
    a.check("68. scan", !testee.scan(r));

    // ...with garbage
    testee.init(afl::string::toMemory("[foo] bar"));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("73. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("74. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("75. parse", parseContinuation(testee, r), " bar");
    a.check("76. scan", !testee.scan(r));

    // ...with more garbage
    testee.init(afl::string::toMemory("[foo] bar ; baz"));
    a.check("81. scan", testee.scan(r));
    a.checkEqual("82. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("83. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("84. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("85. parse", parseContinuation(testee, r), " bar ; baz");
    a.check("86. scan", !testee.scan(r));

    // Percent
    testee.init(afl::string::toMemory("%foo"));
    a.check("91. scan", testee.scan(r));
    a.checkEqual("92. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("93. parse", parseContinuation(testee, r), "%foo");
    a.check("94. scan", !testee.scan(r));

    // ...with newline
    testee.init(afl::string::toMemory("%foo\n"));
    a.check("101. scan", testee.scan(r));
    a.checkEqual("102. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("103. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("104. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("105. parse", parseContinuation(testee, r), "\n");
    a.check("106. scan", !testee.scan(r));

    // ...indented
    testee.init(afl::string::toMemory("    %foo"));
    a.check("111. scan", testee.scan(r));
    a.checkEqual("112. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("113. parse", parseContinuation(testee, r), "    ");
    a.checkEqual("114. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("115. parse", parseContinuation(testee, r), "%foo");
    a.check("116. scan", !testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("[foo]#bar"));
    a.check("121. scan", testee.scan(r));
    a.checkEqual("122. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("123. parse", parseContinuation(testee, r), "[foo]");
    a.checkEqual("124. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("125. parse", parseContinuation(testee, r), "#bar");
    a.check("126. scan", !testee.scan(r));

    // ...with space and comment
    testee.init(afl::string::toMemory("%foo  #bar"));
    a.check("131. scan", testee.scan(r));
    a.checkEqual("132. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("133. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("134. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("135. parse", parseContinuation(testee, r), "  ");
    a.checkEqual("136. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("137. parse", parseContinuation(testee, r), "#bar");
    a.check("138. scan", !testee.scan(r));

    // ...with garbage
    testee.init(afl::string::toMemory("%foo bar"));
    a.check("141. scan", testee.scan(r));
    a.checkEqual("142. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("143. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("144. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("145. parse", parseContinuation(testee, r), " bar");
    a.check("146. scan", !testee.scan(r));

    // ...with more garbage
    testee.init(afl::string::toMemory("%foo bar ; baz"));
    a.check("151. scan", testee.scan(r));
    a.checkEqual("152. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("153. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("154. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("155. parse", parseContinuation(testee, r), " bar ; baz");
    a.check("156. scan", !testee.scan(r));
}

/** Test assignments. */
AFL_TEST("util.syntax.IniHighlighter:assignment", a)
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "a");
    util::syntax::Segment r;

    // Preload the table
    tab.add("ini.foo.f1.link", "first link");
    tab.add("ini.foo.f2.link", "second link");
    tab.add("ini.foo.f2.info", "second info");
    tab.add("ini.a.x.info", "ex info");
    tab.add("ini.a.y[2].info", "array info");

    // Assignments in section a
    testee.init(afl::string::toMemory("x = hi"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("03. getInfo", r.getInfo(), "ex info");
    a.checkEqual("04. getLink", r.getLink(), "");
    a.checkEqual("05. parse", parseContinuation(testee, r), "x");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. getInfo", r.getInfo(), "");
    a.checkEqual("08. getLink", r.getLink(), "");
    a.checkEqual("09. parse", parseContinuation(testee, r), " = hi");
    a.check("10. scan", !testee.scan(r));

    // ...with no assigment
    testee.init(afl::string::toMemory("x\n"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("13. getInfo", r.getInfo(), "ex info");
    a.checkEqual("14. parse", parseContinuation(testee, r), "x");
    a.checkEqual("15. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("16. parse", parseContinuation(testee, r), "\n");
    a.check("17. scan", !testee.scan(r));

    // ...with no assigment, with space
    testee.init(afl::string::toMemory("x \n"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("23. getInfo", r.getInfo(), "ex info");
    a.checkEqual("24. parse", parseContinuation(testee, r), "x");
    a.checkEqual("25. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("26. parse", parseContinuation(testee, r), " \n");
    a.check("27. scan", !testee.scan(r));

    // ...with array
    testee.init(afl::string::toMemory("  y[2] = ho"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "  ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("35. getInfo", r.getInfo(), "array info");
    a.checkEqual("36. getLink", r.getLink(), "");
    a.checkEqual("37. parse", parseContinuation(testee, r), "y[2]");
    a.checkEqual("38. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("39. parse", parseContinuation(testee, r), " = ho");
    a.check("40. scan", !testee.scan(r));

    // ...with comment
    testee.init(afl::string::toMemory("x = hi # ok"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("43. getInfo", r.getInfo(), "ex info");
    a.checkEqual("44. getLink", r.getLink(), "");
    a.checkEqual("45. parse", parseContinuation(testee, r), "x");
    a.checkEqual("46. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("47. getInfo", r.getInfo(), "");
    a.checkEqual("48. getLink", r.getLink(), "");
    a.checkEqual("49. parse", parseContinuation(testee, r), " = hi # ok");
    a.check("50. scan", !testee.scan(r));

    // ...unknown
    testee.init(afl::string::toMemory("yy = 3"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("53. getInfo", r.getInfo(), "");
    a.checkEqual("54. getLink", r.getLink(), "");
    a.checkEqual("55. parse", parseContinuation(testee, r), "yy");
    a.checkEqual("56. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("57. parse", parseContinuation(testee, r), " = 3");
    a.check("58. scan", !testee.scan(r));

    // ...namespaced
    testee.init(afl::string::toMemory("a.x = ax"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("63. getInfo", r.getInfo(), "ex info");
    a.checkEqual("64. getLink", r.getLink(), "");
    a.checkEqual("65. parse", parseContinuation(testee, r), "a.x");
    a.checkEqual("66. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("67. getInfo", r.getInfo(), "");
    a.checkEqual("68. getLink", r.getLink(), "");
    a.checkEqual("69. parse", parseContinuation(testee, r), " = ax");
    a.check("70. scan", !testee.scan(r));

    // ...capitalized namespaced
    testee.init(afl::string::toMemory("A.x = ax"));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("73. getInfo", r.getInfo(), "ex info");
    a.checkEqual("74. getLink", r.getLink(), "");
    a.checkEqual("75. parse", parseContinuation(testee, r), "A.x");
    a.checkEqual("76. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("77. getInfo", r.getInfo(), "");
    a.checkEqual("78. getLink", r.getLink(), "");
    a.checkEqual("79. parse", parseContinuation(testee, r), " = ax");
    a.check("80. scan", !testee.scan(r));

    // Elsewhere
    testee.init(afl::string::toMemory("foo.f1 = fx"));
    a.check("81. scan", testee.scan(r));
    a.checkEqual("82. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("83. getInfo", r.getInfo(), "");                           // not found because we're in section a!
    a.checkEqual("84. getLink", r.getLink(), "");
    a.checkEqual("85. parse", parseContinuation(testee, r), "foo.f1");
    a.checkEqual("86. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("87. parse", parseContinuation(testee, r), " = fx");
    a.check("88. scan", !testee.scan(r));

    // Elsewhere with delimiter
    testee.init(afl::string::toMemory("%foo\nf1 = fx"));
    a.check("91. scan", testee.scan(r));
    a.checkEqual("92. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("93. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("94. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("95. parse", parseContinuation(testee, r), "\n");
    a.checkEqual("96. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("97. getInfo", r.getInfo(), "");
    a.checkEqual("98. getLink", r.getLink(), "first link");
    a.checkEqual("99. parse", parseContinuation(testee, r), "f1");
    a.checkEqual("100. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("101. parse", parseContinuation(testee, r), " = fx");
    a.check("102. scan", !testee.scan(r));

    // Elsewhere with delimiter + namespace
    testee.init(afl::string::toMemory("%foo\nfoo.f2 = fy"));
    a.check("111. scan", testee.scan(r));
    a.checkEqual("112. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("113. parse", parseContinuation(testee, r), "%foo");
    a.checkEqual("114. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("115. parse", parseContinuation(testee, r), "\n");
    a.checkEqual("116. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("117. getInfo", r.getInfo(), "second info");
    a.checkEqual("118. getLink", r.getLink(), "second link");
    a.checkEqual("119. parse", parseContinuation(testee, r), "foo.f2");
    a.checkEqual("120. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("121. parse", parseContinuation(testee, r), " = fy");
    a.check("122. scan", !testee.scan(r));
}

AFL_TEST("util.syntax.IniHighlighter:other", a)
{
    util::syntax::KeywordTable tab;
    util::syntax::IniHighlighter testee(tab, "x");
    util::syntax::Segment r;

    // Invalid line (not highlighted)
    testee.init(afl::string::toMemory("=#\n"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "=#\n");
    a.check("04. scan", !testee.scan(r));
}
