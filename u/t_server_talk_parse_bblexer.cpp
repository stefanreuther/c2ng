/**
  *  \file u/t_server_talk_parse_bblexer.cpp
  *  \brief Test for server::talk::parse::BBLexer
  */

#include "server/talk/parse/bblexer.hpp"

#include "t_server_talk_parse.hpp"
#include "afl/base/countof.hpp"

using server::talk::parse::BBLexer;

namespace {
    /** The lexer is allowed to split text tokens arbitrarily.
        This function verifies that we're sitting at a text token, and reads possibly following text tokens.
        The collected text is returned in \c text.
        \param lex [in/out] Lexer
        \param tok [in/out] Token type
        \param text [out] Collected text */
    void parseText(BBLexer& lex, BBLexer::Token& tok, String_t& text)
    {
        // Check initial text token
        TS_ASSERT_EQUALS(tok, BBLexer::Text);
        TS_ASSERT_EQUALS(lex.getTokenType(), BBLexer::Text);
        text = lex.getTokenString();

        // Read more text tokens
        while ((tok = lex.read()) == BBLexer::Text) {
            text += lex.getTokenString();
        }
    }
}


/** Simple test. */
void
TestServerTalkParseBBLexer::testIt()
{
    BBLexer testee("simple");
    BBLexer::Token t = testee.read();
    String_t text;
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "simple");

    TS_ASSERT_EQUALS(t, BBLexer::Eof);
}

/** Test paragraphs. */
void
TestServerTalkParseBBLexer::testPara()
{
    BBLexer testee("a\nb\n\nc");
    BBLexer::Token t = testee.read();
    String_t text;
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "a\nb");

    TS_ASSERT_EQUALS(t, BBLexer::Paragraph);

    t = testee.read();
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "c");

    TS_ASSERT_EQUALS(t, BBLexer::Eof);

}

/** Test some tags. */
void
TestServerTalkParseBBLexer::testTags()
{
    BBLexer testee("a [*] b [B]foo[/b] [:grin:][url=hi]what[url=\"hi[]\"]huh?[");
    BBLexer::Token t = testee.read();
    String_t text;

    // "a "
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "a ");

    // "[*]"
    TS_ASSERT_EQUALS(t, BBLexer::TagStart);
    TS_ASSERT_EQUALS(testee.getTag(), "*");
    TS_ASSERT_EQUALS(testee.getAttribute(), "");
    t = testee.read();

    // " b "
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, " b ");

    // "[B]"
    TS_ASSERT_EQUALS(t, BBLexer::TagStart);
    TS_ASSERT_EQUALS(testee.getTag(), "b");
    TS_ASSERT_EQUALS(testee.getAttribute(), "");
    t = testee.read();

    // "foo"
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "foo");

    // "[/b]"
    TS_ASSERT_EQUALS(t, BBLexer::TagEnd);
    TS_ASSERT_EQUALS(testee.getTag(), "b");
    TS_ASSERT_EQUALS(testee.getAttribute(), "");
    t = testee.read();

    // " "
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, " ");

    // "[:grin:]"
    TS_ASSERT_EQUALS(t, BBLexer::Smiley);
    TS_ASSERT_EQUALS(testee.getTag(), "grin");
    t = testee.read();

    // "[url=hi]"
    TS_ASSERT_EQUALS(t, BBLexer::TagStart);
    TS_ASSERT_EQUALS(testee.getTag(), "url");
    TS_ASSERT_EQUALS(testee.getAttribute(), "hi");
    t = testee.read();
    
    // "what"
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "what");

    // "[url="hi[]"]"
    TS_ASSERT_EQUALS(t, BBLexer::TagStart);
    TS_ASSERT_EQUALS(testee.getTag(), "url");
    TS_ASSERT_EQUALS(testee.getAttribute(), "hi[]");
    t = testee.read();
    
    // "huh?["
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "huh?[");

    TS_ASSERT_EQUALS(t, BBLexer::Eof);
}

/** Test at-links. */
void
TestServerTalkParseBBLexer::testAtLink()
{
    BBLexer testee("a @ b @user c");
    BBLexer::Token t = testee.read();
    String_t text;

    // "a @ b "
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, "a @ b ");

    // @user
    TS_ASSERT_EQUALS(t, BBLexer::AtLink);
    TS_ASSERT_EQUALS(testee.getAttribute(), "user");
    t = testee.read();

    // "huh?["
    parseText(testee, t, text);
    TS_ASSERT_EQUALS(text, " c");

    TS_ASSERT_EQUALS(t, BBLexer::Eof);
}

/** Test partial markup that is all recognized as text. */
void
TestServerTalkParseBBLexer::testPartials()
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

        parseText(testee, t, text);
        TS_ASSERT_EQUALS(text, TESTCASES[i]);

        TS_ASSERT_EQUALS(t, BBLexer::Eof);
    }
}
