/**
  *  \file test/util/syntax/chighlightertest.cpp
  *  \brief Test for util::syntax::CHighlighter
  */

#include "util/syntax/chighlighter.hpp"

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

/** Test preprocessor. */
AFL_TEST("util.syntax.CHighlighter:preprocessor", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // #if foo
    testee.init(afl::string::toMemory("#if foo"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "#if");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), " foo");
    a.check("06. scan", !testee.scan(r));

    //   #   if   /*what*/ foo
    testee.init(afl::string::toMemory("  #   if   /*what*/ foo"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "  ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "#   if");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), "   ");
    a.checkEqual("18. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("19. parse", parseContinuation(testee, r), "/*what*/");
    a.checkEqual("20. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("21. parse", parseContinuation(testee, r), " foo");
    a.check("22. scan", !testee.scan(r));

    // #i\nf f\noo
    testee.init(afl::string::toMemory("#i\\\nf f\\\noo"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "#i\\\nf");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), " f\\\noo");
    a.check("36. scan", !testee.scan(r));

    // #define foo bar
    testee.init(afl::string::toMemory("#define foo bar"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "#define");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), " ");
    a.checkEqual("46. getFormat", r.getFormat(), util::syntax::NameFormat);
    a.checkEqual("47. parse", parseContinuation(testee, r), "foo");
    a.checkEqual("48. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("49. parse", parseContinuation(testee, r), " bar");
    a.check("50. scan", !testee.scan(r));

    // #define (foo) -- invalid
    testee.init(afl::string::toMemory("#define (foo)"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "#define");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), " (foo)");
    a.check("56. scan", !testee.scan(r));

    // #include <foo>
    testee.init(afl::string::toMemory("#include <foo>"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "#include");
    a.checkEqual("64. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("65. parse", parseContinuation(testee, r), " ");
    a.checkEqual("66. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("67. parse", parseContinuation(testee, r), "<foo>");
    a.check("68. scan", !testee.scan(r));

    // #include "foo"
    testee.init(afl::string::toMemory("#include \"foo\""));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("73. parse", parseContinuation(testee, r), "#include");
    a.checkEqual("74. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("75. parse", parseContinuation(testee, r), " ");
    a.checkEqual("76. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("77. parse", parseContinuation(testee, r), "\"foo\"");
    a.check("78. scan", !testee.scan(r));

    // #include <foo\nxx -- invalid; newline should not be part of string
    testee.init(afl::string::toMemory("#include <foo\nxx"));
    a.check("81. scan", testee.scan(r));
    a.checkEqual("82. getFormat", r.getFormat(), util::syntax::SectionFormat);
    a.checkEqual("83. parse", parseContinuation(testee, r), "#include");
    a.checkEqual("84. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("85. parse", parseContinuation(testee, r), " ");
    a.checkEqual("86. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("87. parse", parseContinuation(testee, r), "<foo");
    a.checkEqual("88. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("89. parse", parseContinuation(testee, r), "\nxx");
    a.check("90. scan", !testee.scan(r));

    // aa#if -- not a preprocessor directive
    testee.init(afl::string::toMemory("aa#if"));
    a.check("91. scan", testee.scan(r));
    a.checkEqual("92. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("93. parse", parseContinuation(testee, r), "aa#");
    a.checkEqual("94. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("95. parse", parseContinuation(testee, r), "if");
    a.check("96. scan", !testee.scan(r));

    // 9#if -- not a preprocessor directive
    testee.init(afl::string::toMemory("9#if"));
    a.check("101. scan", testee.scan(r));
    a.checkEqual("102. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("103. parse", parseContinuation(testee, r), "9#");
    a.checkEqual("104. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("105. parse", parseContinuation(testee, r), "if");
    a.check("106. scan", !testee.scan(r));
}

/** Test strings. */
AFL_TEST("util.syntax.CHighlighter:strings", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // a "\"" a
    testee.init(afl::string::toMemory("a \"\\\"\" a"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "a ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "\"\\\"\"");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " a");
    a.check("08. scan", !testee.scan(r));

    // a '\'' a
    testee.init(afl::string::toMemory("a \'\\\'\' a"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "a ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "\'\\\'\'");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), " a");
    a.check("18. scan", !testee.scan(r));

    // a "'" a
    testee.init(afl::string::toMemory("a \"\'\" a"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "a ");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), "\"\'\"");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), " a");
    a.check("28. scan", !testee.scan(r));

    // a '"' a
    testee.init(afl::string::toMemory("a \'\"\' a"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "a ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "\'\"\'");
    a.checkEqual("36. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("37. parse", parseContinuation(testee, r), " a");
    a.check("38. scan", !testee.scan(r));

    // a "\\n"" a (end-of-line-quote within quoted-quote)
    testee.init(afl::string::toMemory("a \"\\\\\n\"\" a"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "a ");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), "\"\\\\\n\"\"");
    a.checkEqual("46. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("47. parse", parseContinuation(testee, r), " a");
    a.check("48. scan", !testee.scan(r));
}

/** Some identifiers. */
AFL_TEST("util.syntax.CHighlighter:identifiers", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // foo, bar
    testee.init(afl::string::toMemory("foo\nbar"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "foo\nbar");
    a.check("04. scan", !testee.scan(r));

    // foo, if (
    testee.init(afl::string::toMemory("foo\nif ("));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "foo\n");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "if");
    a.checkEqual("16. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("17. parse", parseContinuation(testee, r), " (");
    a.check("18. scan", !testee.scan(r));

    // foo, if (
    testee.init(afl::string::toMemory("} i\\\nf ("));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "} ");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), "i\\\nf");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), " (");
    a.check("28. scan", !testee.scan(r));

    // 99if -- this is actually one token in C, but we interpret it as token+keyword
    testee.init(afl::string::toMemory("99if"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "99");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "if");
    a.check("36. scan", !testee.scan(r));
}

/** Test comments. */
AFL_TEST("util.syntax.CHighlighter:comments", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // foo /*bar*/ baz
    testee.init(afl::string::toMemory("foo /*bar*/ baz"));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "/*bar*/");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " baz");
    a.check("08. scan", !testee.scan(r));

    // foo /*bar (unterminated comment)
    testee.init(afl::string::toMemory("foo /*bar"));
    a.check("11. scan", testee.scan(r));
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), "/*bar");
    a.check("16. scan", !testee.scan(r));

    // foo //bar\nbaz
    testee.init(afl::string::toMemory("foo //bar\nbaz"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("24. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("25. parse", parseContinuation(testee, r), "//bar");
    a.checkEqual("26. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("27. parse", parseContinuation(testee, r), "\nbaz");
    a.check("28. scan", !testee.scan(r));

    // foo //bar\nbaz
    testee.init(afl::string::toMemory("foo //bar\\\nbaz"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "//bar\\\nbaz");
    a.check("36. scan", !testee.scan(r));

    // foo //bar\nbaz (with CRLF)
    testee.init(afl::string::toMemory("foo //bar\\\r\nbaz"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), "//bar\\\r\nbaz");
    a.check("46. scan", !testee.scan(r));

    // foo /\n/bar
    testee.init(afl::string::toMemory("foo /\\\n/bar"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), "/\\\n/bar");
    a.check("56. scan", !testee.scan(r));

    // foo /\r\n/bar (with CRLF)
    testee.init(afl::string::toMemory("foo /\\\r\n/bar"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "foo ");
    a.checkEqual("64. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("65. parse", parseContinuation(testee, r), "/\\\r\n/bar");
    a.check("66. scan", !testee.scan(r));

    // foo /
    testee.init(afl::string::toMemory("foo /"));
    a.check("71. scan", testee.scan(r));
    a.checkEqual("72. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("73. parse", parseContinuation(testee, r), "foo /");
    a.check("74. scan", !testee.scan(r));
}

/** Test some C specifics. */
AFL_TEST("util.syntax.CHighlighter:c-specifics", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), " foo ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "_Bool");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " abstract const_cast ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "break");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), " var ");
    a.check("12. scan", !testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "a = /foo[a/b]/;");
    a.check("24. scan", !testee.scan(r));
}

/** Test some C++ specifics. */
AFL_TEST("util.syntax.CHighlighter:c++-specifics", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangCXX);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break requires var "));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), " foo _Bool abstract ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "const_cast");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "break");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), " ");
    a.checkEqual("12. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("13. parse", parseContinuation(testee, r), "requires");
    a.checkEqual("14. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("15. parse", parseContinuation(testee, r), " var ");
    a.check("16. scan", !testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "a = /foo[a/b]/;");
    a.check("24. scan", !testee.scan(r));
}

/** Test some JavaScript specifics. */
AFL_TEST("util.syntax.CHighlighter:js-specifics", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangJavaScript);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), " foo _Bool abstract const_cast ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "break");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "var");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), " ");
    a.check("12. scan", !testee.scan(r));

    // No preprocessor
    testee.init(afl::string::toMemory("#ifdef a"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "#ifdef a");
    a.check("24. scan", !testee.scan(r));

    // Regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "a = ");
    a.checkEqual("34. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("35. parse", parseContinuation(testee, r), "/foo[a/b]/");
    a.checkEqual("36. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("37. parse", parseContinuation(testee, r), ";");
    a.check("38. scan", !testee.scan(r));

    // Regexps (backslash quote)
    testee.init(afl::string::toMemory("a = /\\//;"));
    a.check("41. scan", testee.scan(r));
    a.checkEqual("42. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("43. parse", parseContinuation(testee, r), "a = ");
    a.checkEqual("44. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("45. parse", parseContinuation(testee, r), "/\\//");
    a.checkEqual("46. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("47. parse", parseContinuation(testee, r), ";");
    a.check("48. scan", !testee.scan(r));

    // Regexps syntax error. This is a regexp, followed by a slash.
    testee.init(afl::string::toMemory("a = /i//i;"));
    a.check("51. scan", testee.scan(r));
    a.checkEqual("52. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("53. parse", parseContinuation(testee, r), "a = ");
    a.checkEqual("54. getFormat", r.getFormat(), util::syntax::StringFormat);
    a.checkEqual("55. parse", parseContinuation(testee, r), "/i/");
    a.checkEqual("56. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("57. parse", parseContinuation(testee, r), "/i;");
    a.check("58. scan", !testee.scan(r));

    // This is a comment, not a regexp.
    testee.init(afl::string::toMemory("a = //i;"));
    a.check("61. scan", testee.scan(r));
    a.checkEqual("62. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("63. parse", parseContinuation(testee, r), "a = ");
    a.checkEqual("64. getFormat", r.getFormat(), util::syntax::CommentFormat);
    a.checkEqual("65. parse", parseContinuation(testee, r), "//i;");
    a.check("66. scan", !testee.scan(r));
}

/** Test some Java specifics. */
AFL_TEST("util.syntax.CHighlighter:java-specifics", a)
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangJava);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    a.check("01. scan", testee.scan(r));
    a.checkEqual("02. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("03. parse", parseContinuation(testee, r), " foo _Bool ");
    a.checkEqual("04. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("05. parse", parseContinuation(testee, r), "abstract");
    a.checkEqual("06. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("07. parse", parseContinuation(testee, r), " const_cast ");
    a.checkEqual("08. getFormat", r.getFormat(), util::syntax::KeywordFormat);
    a.checkEqual("09. parse", parseContinuation(testee, r), "break");
    a.checkEqual("10. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("11. parse", parseContinuation(testee, r), " var ");
    a.check("12. scan", !testee.scan(r));

    // No preprocessor
    testee.init(afl::string::toMemory("#ifdef a"));
    a.check("21. scan", testee.scan(r));
    a.checkEqual("22. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("23. parse", parseContinuation(testee, r), "#ifdef a");
    a.check("24. scan", !testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    a.check("31. scan", testee.scan(r));
    a.checkEqual("32. getFormat", r.getFormat(), util::syntax::DefaultFormat);
    a.checkEqual("33. parse", parseContinuation(testee, r), "a = /foo[a/b]/;");
    a.check("34. scan", !testee.scan(r));
}
