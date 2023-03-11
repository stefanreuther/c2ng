/**
  *  \file u/t_util_syntax_chighlighter.cpp
  *  \brief Test for util::syntax::CHighlighter
  */

#include "util/syntax/chighlighter.hpp"

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

/** Test preprocessor. */
void
TestUtilSyntaxCHighlighter::testPreproc()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // #if foo
    testee.init(afl::string::toMemory("#if foo"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#if");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo");
    TS_ASSERT(!testee.scan(r));

    //   #   if   /*what*/ foo
    testee.init(afl::string::toMemory("  #   if   /*what*/ foo"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "  ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#   if");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "   ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/*what*/");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo");
    TS_ASSERT(!testee.scan(r));

    // #i\nf f\noo
    testee.init(afl::string::toMemory("#i\\\nf f\\\noo"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#i\\\nf");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " f\\\noo");
    TS_ASSERT(!testee.scan(r));

    // #define foo bar
    testee.init(afl::string::toMemory("#define foo bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#define");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::NameFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " bar");
    TS_ASSERT(!testee.scan(r));

    // #define (foo) -- invalid
    testee.init(afl::string::toMemory("#define (foo)"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#define");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " (foo)");
    TS_ASSERT(!testee.scan(r));

    // #include <foo>
    testee.init(afl::string::toMemory("#include <foo>"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#include");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "<foo>");
    TS_ASSERT(!testee.scan(r));

    // #include "foo"
    testee.init(afl::string::toMemory("#include \"foo\""));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#include");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"foo\"");
    TS_ASSERT(!testee.scan(r));

    // #include <foo\nxx -- invalid; newline should not be part of string
    testee.init(afl::string::toMemory("#include <foo\nxx"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::SectionFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#include");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "<foo");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\nxx");
    TS_ASSERT(!testee.scan(r));

    // aa#if -- not a preprocessor directive
    testee.init(afl::string::toMemory("aa#if"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "aa#");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "if");
    TS_ASSERT(!testee.scan(r));

    // 9#if -- not a preprocessor directive
    testee.init(afl::string::toMemory("9#if"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "9#");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "if");
    TS_ASSERT(!testee.scan(r));
}

/** Test strings. */
void
TestUtilSyntaxCHighlighter::testString()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // a "\"" a
    testee.init(afl::string::toMemory("a \"\\\"\" a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"\\\"\"");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a");
    TS_ASSERT(!testee.scan(r));

    // a '\'' a
    testee.init(afl::string::toMemory("a \'\\\'\' a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\'\\\'\'");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a");
    TS_ASSERT(!testee.scan(r));

    // a "'" a
    testee.init(afl::string::toMemory("a \"\'\" a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"\'\"");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a");
    TS_ASSERT(!testee.scan(r));

    // a '"' a
    testee.init(afl::string::toMemory("a \'\"\' a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\'\"\'");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a");
    TS_ASSERT(!testee.scan(r));

    // a "\\n"" a (end-of-line-quote within quoted-quote)
    testee.init(afl::string::toMemory("a \"\\\\\n\"\" a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\"\\\\\n\"\"");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " a");
    TS_ASSERT(!testee.scan(r));
}

/** Some identifiers. */
void
TestUtilSyntaxCHighlighter::testIdentifiers()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // foo, bar
    testee.init(afl::string::toMemory("foo\nbar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo\nbar");
    TS_ASSERT(!testee.scan(r));

    // foo, if (
    testee.init(afl::string::toMemory("foo\nif ("));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo\n");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "if");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " (");
    TS_ASSERT(!testee.scan(r));

    // foo, if (
    testee.init(afl::string::toMemory("} i\\\nf ("));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "} ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "i\\\nf");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " (");
    TS_ASSERT(!testee.scan(r));

    // 99if -- this is actually one token in C, but we interpret it as token+keyword
    testee.init(afl::string::toMemory("99if"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "99");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "if");
    TS_ASSERT(!testee.scan(r));
}

/** Test comments. */
void
TestUtilSyntaxCHighlighter::testComment()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // foo /*bar*/ baz
    testee.init(afl::string::toMemory("foo /*bar*/ baz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/*bar*/");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " baz");
    TS_ASSERT(!testee.scan(r));

    // foo /*bar (unterminated comment)
    testee.init(afl::string::toMemory("foo /*bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/*bar");
    TS_ASSERT(!testee.scan(r));

    // foo //bar\nbaz
    testee.init(afl::string::toMemory("foo //bar\nbaz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "//bar");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "\nbaz");
    TS_ASSERT(!testee.scan(r));

    // foo //bar\nbaz
    testee.init(afl::string::toMemory("foo //bar\\\nbaz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "//bar\\\nbaz");
    TS_ASSERT(!testee.scan(r));

    // foo //bar\nbaz (with CRLF)
    testee.init(afl::string::toMemory("foo //bar\\\r\nbaz"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "//bar\\\r\nbaz");
    TS_ASSERT(!testee.scan(r));

    // foo /\n/bar
    testee.init(afl::string::toMemory("foo /\\\n/bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/\\\n/bar");
    TS_ASSERT(!testee.scan(r));

    // foo /\r\n/bar (with CRLF)
    testee.init(afl::string::toMemory("foo /\\\r\n/bar"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/\\\r\n/bar");
    TS_ASSERT(!testee.scan(r));

    // foo /
    testee.init(afl::string::toMemory("foo /"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "foo /");
    TS_ASSERT(!testee.scan(r));
}

/** Test some C specifics. */
void
TestUtilSyntaxCHighlighter::testC()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangC);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "_Bool");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " abstract const_cast ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "break");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " var ");
    TS_ASSERT(!testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = /foo[a/b]/;");
    TS_ASSERT(!testee.scan(r));
}

/** Test some C++ specifics. */
void
TestUtilSyntaxCHighlighter::testCXX()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangCXX);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo _Bool abstract ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "const_cast");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "break");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " var ");
    TS_ASSERT(!testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = /foo[a/b]/;");
    TS_ASSERT(!testee.scan(r));
}

/** Test some JavaScript specifics. */
void
TestUtilSyntaxCHighlighter::testJS()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangJavaScript);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo _Bool abstract const_cast ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "break");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "var");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " ");
    TS_ASSERT(!testee.scan(r));

    // No preprocessor
    testee.init(afl::string::toMemory("#ifdef a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#ifdef a");
    TS_ASSERT(!testee.scan(r));

    // Regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/foo[a/b]/");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ";");
    TS_ASSERT(!testee.scan(r));

    // Regexps (backslash quote)
    testee.init(afl::string::toMemory("a = /\\//;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/\\//");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), ";");
    TS_ASSERT(!testee.scan(r));

    // Regexps syntax error. This is a regexp, followed by a slash.
    testee.init(afl::string::toMemory("a = /i//i;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::StringFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/i/");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "/i;");
    TS_ASSERT(!testee.scan(r));

    // This is a comment, not a regexp.
    testee.init(afl::string::toMemory("a = //i;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::CommentFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "//i;");
    TS_ASSERT(!testee.scan(r));
}

/** Test some Java specifics. */
void
TestUtilSyntaxCHighlighter::testJava()
{
    util::syntax::CHighlighter testee(util::syntax::CHighlighter::LangJava);
    util::syntax::Segment r;

    // Keywords
    testee.init(afl::string::toMemory(" foo _Bool abstract const_cast break var "));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " foo _Bool ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "abstract");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " const_cast ");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::KeywordFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "break");
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), " var ");
    TS_ASSERT(!testee.scan(r));

    // No preprocessor
    testee.init(afl::string::toMemory("#ifdef a"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "#ifdef a");
    TS_ASSERT(!testee.scan(r));

    // No regexps
    testee.init(afl::string::toMemory("a = /foo[a/b]/;"));
    TS_ASSERT(testee.scan(r));
    TS_ASSERT_EQUALS(r.getFormat(), util::syntax::DefaultFormat);
    TS_ASSERT_EQUALS(parseContinuation(testee, r), "a = /foo[a/b]/;");
    TS_ASSERT(!testee.scan(r));
}

