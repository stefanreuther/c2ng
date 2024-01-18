/**
  *  \file test/server/talk/parse/bblexertest.cpp
  *  \brief Test for server::talk::parse::BBLexer
  */

#include "server/talk/parse/bblexer.hpp"

#include "afl/base/countof.hpp"
#include "afl/test/testrunner.hpp"

using server::talk::parse::BBLexer;

namespace {
    /** The lexer is allowed to split text tokens arbitrarily.
        This function verifies that we're sitting at a text token, and reads possibly following text tokens.
        The collected text is returned in \c text.
        \param lex [in/out] Lexer
        \param tok [in/out] Token type
        \param text [out] Collected text */
    void parseText(afl::test::Assert a, BBLexer& lex, BBLexer::Token& tok, String_t& text)
    {
        // Check initial text token
        a.checkEqual("01. is text", tok, BBLexer::Text);
        a.checkEqual("02. getTokenType", lex.getTokenType(), BBLexer::Text);
        text = lex.getTokenString();

        // Read more text tokens
        while ((tok = lex.read()) == BBLexer::Text) {
            text += lex.getTokenString();
        }
    }
}


/** Simple test. */
AFL_TEST("server.talk.parse.BBLexer:simple", a)
{
    BBLexer testee("simple");
    BBLexer::Token t = testee.read();
    String_t text;
    parseText(a("t1"), testee, t, text);
    a.checkEqual("01", text, "simple");

    a.checkEqual("11", t, BBLexer::Eof);
}

/** Test paragraphs. */
AFL_TEST("server.talk.parse.BBLexer:paragraph", a)
{
    BBLexer testee("a\nb\n\nc");
    BBLexer::Token t = testee.read();
    String_t text;
    parseText(a("t1"), testee, t, text);
    a.checkEqual("01", text, "a\nb");

    a.checkEqual("11", t, BBLexer::Paragraph);

    t = testee.read();
    parseText(a("t2"), testee, t, text);
    a.checkEqual("21", text, "c");

    a.checkEqual("31", t, BBLexer::Eof);
}

/** Test some tags. */
AFL_TEST("server.talk.parse.BBLexer:tags", a)
{
    BBLexer testee("a [*] b [B]foo[/b] [:grin:][url=hi]what[url=\"hi[]\"]huh?[");
    BBLexer::Token t = testee.read();
    String_t text;

    // "a "
    parseText(a("t1"), testee, t, text);
    a.checkEqual("01", text, "a ");

    // "[*]"
    a.checkEqual("11", t, BBLexer::TagStart);
    a.checkEqual("12. getTag", testee.getTag(), "*");
    a.checkEqual("13. getAttribute", testee.getAttribute(), "");
    t = testee.read();

    // " b "
    parseText(a("t2"), testee, t, text);
    a.checkEqual("21", text, " b ");

    // "[B]"
    a.checkEqual("31", t, BBLexer::TagStart);
    a.checkEqual("32. getTag", testee.getTag(), "b");
    a.checkEqual("33. getAttribute", testee.getAttribute(), "");
    t = testee.read();

    // "foo"
    parseText(a("t3"), testee, t, text);
    a.checkEqual("41", text, "foo");

    // "[/b]"
    a.checkEqual("51", t, BBLexer::TagEnd);
    a.checkEqual("52. getTag", testee.getTag(), "b");
    a.checkEqual("53. getAttribute", testee.getAttribute(), "");
    t = testee.read();

    // " "
    parseText(a("t4"), testee, t, text);
    a.checkEqual("61", text, " ");

    // "[:grin:]"
    a.checkEqual("71", t, BBLexer::Smiley);
    a.checkEqual("72. getTag", testee.getTag(), "grin");
    t = testee.read();

    // "[url=hi]"
    a.checkEqual("81", t, BBLexer::TagStart);
    a.checkEqual("82. getTag", testee.getTag(), "url");
    a.checkEqual("83. getAttribute", testee.getAttribute(), "hi");
    t = testee.read();

    // "what"
    parseText(a("t5"), testee, t, text);
    a.checkEqual("91", text, "what");

    // "[url="hi[]"]"
    a.checkEqual("101", t, BBLexer::TagStart);
    a.checkEqual("102. getTag", testee.getTag(), "url");
    a.checkEqual("103. getAttribute", testee.getAttribute(), "hi[]");
    t = testee.read();

    // "huh?["
    parseText(a("t6"), testee, t, text);
    a.checkEqual("111", text, "huh?[");

    a.checkEqual("121", t, BBLexer::Eof);
}

/** Test at-links. */
AFL_TEST("server.talk.parse.BBLexer:atlink", a)
{
    BBLexer testee("a @ b @user c");
    BBLexer::Token t = testee.read();
    String_t text;

    // "a @ b "
    parseText(a("t1"), testee, t, text);
    a.checkEqual("01", text, "a @ b ");

    // @user
    a.checkEqual("11", t, BBLexer::AtLink);
    a.checkEqual("12. getAttribute", testee.getAttribute(), "user");
    t = testee.read();

    // "huh?["
    parseText(a("t2"), testee, t, text);
    a.checkEqual("21", text, " c");

    a.checkEqual("31", t, BBLexer::Eof);
}

/** Test partial markup that is all recognized as text. */
AFL_TEST("server.talk.parse.BBLexer:partial", a)
{
    const char*const TESTCASES[] = {
        "a [/b",
        "a [/b c",
        "a [/b c] d",
        "a [*b c",
        "a [*b*] c",
        "a [:b",
        "a [:b c",
        "a [:b] c",
        "a [b",
        "a [",
        "a [b=",
        "a [b=\"",
        "a [b c",
        "a b@c d",
        "a b@ c",
    };
    for (size_t i = 0; i < countof(TESTCASES); ++i) {
        BBLexer testee(TESTCASES[i]);
        BBLexer::Token t = testee.read();
        String_t text;

        parseText(a("t1"), testee, t, text);
        a.checkEqual("01", text, TESTCASES[i]);

        a.checkEqual("11", t, BBLexer::Eof);
    }
}
