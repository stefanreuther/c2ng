/**
  *  \file u/t_server_talk_inlinerecognizer.cpp
  *  \brief Test for server::talk::InlineRecognizer
  */

#include "server/talk/inlinerecognizer.hpp"

#include "t_server_talk.hpp"

/** Test recognition of URLs. */
void
TestServerTalkInlineRecognizer::testUrl()
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    const InlineRecognizer::Kinds_t KINDS(InlineRecognizer::Link);
    bool ok;

    /*
     *  Simple tests
     */

    // URL that fills the whole string
    ok = testee.find("http://foo/", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 0U);
    TS_ASSERT_EQUALS(result.length, 11U);
    TS_ASSERT_EQUALS(result.text, "http://foo/");

    // Email address that fills the whole string
    ok = testee.find("mailto:me@here.example", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 0U);
    TS_ASSERT_EQUALS(result.length, 22U);
    TS_ASSERT_EQUALS(result.text, "mailto:me@here.example");

    // URL with stuff before and after
    ok = testee.find("see http://foo/ for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 4U);
    TS_ASSERT_EQUALS(result.length, 11U);
    TS_ASSERT_EQUALS(result.text, "http://foo/");

    /*
     *  Specific tests
     */

    // Unrecognized protocol
    ok = testee.find("see foo://foo/ for more", 0, KINDS, result);
    TS_ASSERT(!ok);

    // Protocol preceded by letter
    ok = testee.find("see thttp://foo/ for more", 0, KINDS, result);
    TS_ASSERT(!ok);

    // Angle bracket
    ok = testee.find("see <http://foo/That Page> for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 5U);
    TS_ASSERT_EQUALS(result.length, 20U);
    TS_ASSERT_EQUALS(result.text, "http://foo/That Page");

    // Missing angle bracket
    ok = testee.find("see <http://foo/That Page\nfor more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 5U);
    TS_ASSERT_EQUALS(result.length, 15U);
    TS_ASSERT_EQUALS(result.text, "http://foo/That");

    // Missing angle bracket
    ok = testee.find("see <http://foo/That Page", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 5U);
    TS_ASSERT_EQUALS(result.length, 15U);
    TS_ASSERT_EQUALS(result.text, "http://foo/That");

    // Strange character after protocol name
    ok = testee.find("see http:@xy maybe", 0, KINDS, result);
    TS_ASSERT(!ok);

    // Regular URL in parens
    ok = testee.find("see page (http://foo/bar/baz) for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 10U);
    TS_ASSERT_EQUALS(result.length, 18U);
    TS_ASSERT_EQUALS(result.text, "http://foo/bar/baz");

    // Wiki URL in parens
    ok = testee.find("see page (http://foo/wiki/Foo_(Bar)) for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 10U);
    TS_ASSERT_EQUALS(result.length, 25U);
    TS_ASSERT_EQUALS(result.text, "http://foo/wiki/Foo_(Bar)");

    // Wiki URL without parens
    ok = testee.find("see page http://foo/wiki/Foo_(Baz) for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 9U);
    TS_ASSERT_EQUALS(result.length, 25U);
    TS_ASSERT_EQUALS(result.text, "http://foo/wiki/Foo_(Baz)");

    // MSDN URL in parens
    ok = testee.find("see page (http://foo/bla(4.2).aspx) for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 10U);
    TS_ASSERT_EQUALS(result.length, 24U);
    TS_ASSERT_EQUALS(result.text, "http://foo/bla(4.2).aspx");

    // MSDN URL without parens
    ok = testee.find("see page http://foo/bla(5.1).aspx for more", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 9U);
    TS_ASSERT_EQUALS(result.length, 24U);
    TS_ASSERT_EQUALS(result.text, "http://foo/bla(5.1).aspx");

    // URL in quotes
    ok = testee.find("url = \"http://host/path\";", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 7U);
    TS_ASSERT_EQUALS(result.length, 16U);
    TS_ASSERT_EQUALS(result.text, "http://host/path");

    // URL with parens in quotes
    ok = testee.find("url = \"http://host/path/(what\";", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 7U);
    TS_ASSERT_EQUALS(result.length, 22U);
    TS_ASSERT_EQUALS(result.text, "http://host/path/(what");

    // URL with parens ending in '>'
    ok = testee.find("<url = http://host/path/(what>;", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 7U);
    TS_ASSERT_EQUALS(result.length, 22U);
    TS_ASSERT_EQUALS(result.text, "http://host/path/(what");

    // URL with dot and '>'
    ok = testee.find("<look here http://host/path.>", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 11U);
    TS_ASSERT_EQUALS(result.length, 17U);
    TS_ASSERT_EQUALS(result.text, "http://host/path.");

    // URL with dot
    ok = testee.find("look here http://host/path.", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 10U);
    TS_ASSERT_EQUALS(result.length, 16U);
    TS_ASSERT_EQUALS(result.text, "http://host/path");

    // URL preceded by word is not recognized
    TS_ASSERT(!testee.find("see nothttp://foo/ for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    // FIXME: should this be rejected? Right now it is recognized.
    // It makes no difference in practical use because no search leaves off at the given place.
    // TS_ASSERT(!testee.find("see nothttp://foo/ for more", 7, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
}

/** Test recognition of smileys. */
void
TestServerTalkInlineRecognizer::testSmiley()
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    const InlineRecognizer::Kinds_t KINDS(InlineRecognizer::Smiley);
    bool ok;

    /*
     *  Simple tests
     */

    // Named smiley that fills the whole string
    ok = testee.find(":lol:", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 0U);
    TS_ASSERT_EQUALS(result.length, 5U);
    TS_ASSERT_EQUALS(result.text, "lol");

    // Named smiley in text
    ok = testee.find("haha :lol: haha", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 5U);
    TS_ASSERT_EQUALS(result.length, 5U);
    TS_ASSERT_EQUALS(result.text, "lol");

    // Regular smiley that fills the whole string
    ok = testee.find(":-(", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 0U);
    TS_ASSERT_EQUALS(result.length, 3U);
    TS_ASSERT_EQUALS(result.text, "sad");

    // Regular smiley in text
    ok = testee.find("boohoo :-( boohoo", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 7U);
    TS_ASSERT_EQUALS(result.length, 3U);
    TS_ASSERT_EQUALS(result.text, "sad");

    // Short smiley that fills the whole string
    ok = testee.find(":(", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 0U);
    TS_ASSERT_EQUALS(result.length, 2U);
    TS_ASSERT_EQUALS(result.text, "sad");

    // Short smiley in text
    ok = testee.find("bu :( bu", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 3U);
    TS_ASSERT_EQUALS(result.length, 2U);
    TS_ASSERT_EQUALS(result.text, "sad");

    /*
     *  Specific tests
     */

    // Unrecognized named smiley
    ok = testee.find(" :notasmiley: ", 0, KINDS, result);
    TS_ASSERT(!ok);

    // We're case-sensitive
    ok = testee.find(" :LOL: ", 0, KINDS, result);
    TS_ASSERT(!ok);

    // Symbol smiley starting with letter
    ok = testee.find("hey B-)", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 4U);
    TS_ASSERT_EQUALS(result.length, 3U);
    TS_ASSERT_EQUALS(result.text, "cool");

    // Symbol smiley starting with letter preceded by text
    ok = testee.find("heyB-)", 0, KINDS, result);
    TS_ASSERT(!ok);
    
    // Symbol smiley ending with letter
    ok = testee.find("hey :-P lol", 0, KINDS, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 4U);
    TS_ASSERT_EQUALS(result.length, 3U);
    TS_ASSERT_EQUALS(result.text, "tongue");

    // Symbol smiley ending with letter followed by text
    ok = testee.find("hey :-Plol", 0, KINDS, result);
    TS_ASSERT(!ok);
}

/** General tests. */
void
TestServerTalkInlineRecognizer::testGeneral()
{
    using server::talk::InlineRecognizer;
    InlineRecognizer testee;
    InlineRecognizer::Info result;
    bool ok;

    // Test how startAt parameter affects result
    TS_ASSERT(testee.find("see http://foo/ for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    TS_ASSERT(testee.find("see http://foo/ for more", 3, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    TS_ASSERT(testee.find("see http://foo/ for more", 4, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    TS_ASSERT(!testee.find("see http://foo/ for more", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    TS_ASSERT(!testee.find("see http://foo/ for more", 8, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));
    TS_ASSERT(!testee.find("see http://foo/ for more", 10, InlineRecognizer::Kinds_t(InlineRecognizer::Link), result));

    TS_ASSERT(testee.find("lol :-) lol", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    TS_ASSERT(testee.find("lol :-) lol", 3, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    TS_ASSERT(testee.find("lol :-) lol", 4, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    TS_ASSERT(!testee.find("lol :-) lol", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));
    TS_ASSERT(!testee.find("lol :-) lol", 8, InlineRecognizer::Kinds_t(InlineRecognizer::Smiley), result));

    // Test recognition of multiple kinds
    ok = testee.find("see http://foo/B-) for more", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Link);
    TS_ASSERT_EQUALS(result.start, 4U);
    TS_ASSERT_EQUALS(result.length, 13U);
    TS_ASSERT_EQUALS(result.text, "http://foo/B-");

    // Starting after the beginning of the URL will find the smiley
    ok = testee.find("see http://foo/B-) for more", 5, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(result.kind, InlineRecognizer::Smiley);
    TS_ASSERT_EQUALS(result.start, 15U);
    TS_ASSERT_EQUALS(result.length, 3U);
    TS_ASSERT_EQUALS(result.text, "cool");

    // Boundary case
    ok = testee.find("", 0, InlineRecognizer::Kinds_t(InlineRecognizer::Link) + InlineRecognizer::Smiley, result);
    TS_ASSERT(!ok);
}

/** Test getSmileyDefinitionByName. */
void
TestServerTalkInlineRecognizer::testGetSmiley()
{
    server::talk::InlineRecognizer testee;
    const server::talk::InlineRecognizer::SmileyDefinition* p;

    // Border case
    p = testee.getSmileyDefinitionByName("");
    TS_ASSERT(p == 0);

    // Find one
    p = testee.getSmileyDefinitionByName("lol");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(String_t(p->name), "lol");

    // Find another one
    p = testee.getSmileyDefinitionByName("wink");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(String_t(p->name), "wink");

    // Find yet another one
    p = testee.getSmileyDefinitionByName("cool");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(String_t(p->name), "cool");

    // Mismatch: case sensitive
    p = testee.getSmileyDefinitionByName("LOL");
    TS_ASSERT(p == 0);

    // Mismatch: prefix
    p = testee.getSmileyDefinitionByName("lolol");
    TS_ASSERT(p == 0);

    // Mismatch: symbol
    p = testee.getSmileyDefinitionByName(":-)");
    TS_ASSERT(p == 0);
}
