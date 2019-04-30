/**
  *  \file u/t_server_talk_talkaddress.cpp
  *  \brief Test for server::talk::TalkAddress
  */

#include "server/talk/talkaddress.hpp"

#include "t_server_talk.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/string/format.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"

namespace {
    struct TestHarness {
        afl::net::redis::InternalDatabase db;
        afl::net::NullCommandHandler mailout;
        server::talk::Session session;
        server::talk::Root root;

        TestHarness()
            : db(), mailout(), session(), root(db, mailout, server::talk::Configuration())
            {
                using afl::net::redis::StringKey;
                using afl::net::redis::HashKey;
                using afl::net::redis::IntegerSetKey;
                using afl::string::Format;

                // Create two users
                StringKey(db, "uid:fred").set("1000");
                StringKey(db, "uid:wilma").set("1001");
                StringKey(db, "user:1000:name").set("fred");
                StringKey(db, "user:1001:name").set("wilma");
                HashKey(db, "user:1000:profile").stringField("screenname").set("Fred F");
                HashKey(db, "user:1001:profile").stringField("screenname").set("Wilma F");

                // Create a game
                IntegerSetKey(db, "game:all").add(12);
                IntegerSetKey(db, "game:pubstate:joining").add(12);
                IntegerSetKey(db, "game:state:joining").add(12);
                StringKey(db, "game:12:state").set("joining");
                StringKey(db, "game:12:type").set("public");
                StringKey(db, "game:12:name").set("Twelve");
                for (int i = 1; i <= 11; ++i) {
                    HashKey(db, Format("game:12:player:%d:status", i)).intField("slot").set(1);
                }
            }
    };

    String_t parseSingle(server::interface::TalkAddress& a, String_t u)
    {
        String_t in[] = {u};
        afl::data::StringList_t out;
        a.parse(in, out);
        TS_ASSERT_EQUALS(out.size(), 1U);
        return out[0];
    }

    String_t renderSingle(server::interface::TalkAddress& a, String_t u)
    {
        String_t in[] = {u};
        afl::data::StringList_t out;
        a.render(in, out);
        TS_ASSERT_EQUALS(out.size(), 1U);
        return out[0];
    }
}

/** Test parse(). */
void
TestServerTalkTalkAddress::testParse()
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    // Normal
    TS_ASSERT_EQUALS(parseSingle(testee, "fred"), "u:1000");
    TS_ASSERT_EQUALS(parseSingle(testee, "wilma"), "u:1001");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:12"), "g:12");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:12:3"), "g:12:3");

    // Variants
    TS_ASSERT_EQUALS(parseSingle(testee, "--fred--"), "u:1000");
    TS_ASSERT_EQUALS(parseSingle(testee, "WiLmA"), "u:1001");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:012"), "g:12");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:012:003"), "g:12:3");

    // Errors
    TS_ASSERT_EQUALS(parseSingle(testee, ""), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "barney"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:4294967308"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "u:"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:-1"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:10"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:12:0"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:12:"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "g:12:12"), "");
    TS_ASSERT_EQUALS(parseSingle(testee, "G:"), "");
}

/** Test render(), raw format. */
void
TestServerTalkTalkAddress::testRenderRaw()
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    // Default format is "raw"
    TS_ASSERT_EQUALS(h.session.renderOptions().getFormat(), "raw");

    // Normal
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "fred");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "g:12");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "g:12:3");

    // Errors
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "whoops"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:9999"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:13"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:2222"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12x"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:x"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:"), "");
}

/** Test render(), HTML format. */
void
TestServerTalkTalkAddress::testRenderHTML()
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    h.session.renderOptions().setFormat("html");

    // Normal
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "<a class=\"userlink\" href=\"userinfo.cgi/fred\">Fred F</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "player 3 in <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");

    // Errors
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "whoops"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:9999"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:13"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:2222"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12x"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:x"), "");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:"), "");
}

/** Test render(), other formats. */
void
TestServerTalkTalkAddress::testRenderOther()
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);
    h.session.renderOptions().setBaseUrl("http://x/");

    // Mail
    h.session.renderOptions().setFormat("mail");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "<http://x/userinfo.cgi/fred>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "players of <http://x/host/game.cgi/12-Twelve>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "player 3 in <http://x/host/game.cgi/12-Twelve>");
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");

    // News
    h.session.renderOptions().setFormat("news");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "<http://x/userinfo.cgi/fred>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "players of <http://x/host/game.cgi/12-Twelve>");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "player 3 in <http://x/host/game.cgi/12-Twelve>");
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");

    // Text
    h.session.renderOptions().setFormat("text");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "fred");    // FIXME: is this the desired behaviour?
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "players of Twelve");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "player 3 in Twelve");
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");

    // BBCode
    h.session.renderOptions().setFormat("forum");
    TS_ASSERT_EQUALS(renderSingle(testee, "u:1000"), "[user]fred[/user]");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12"), "players of [game]12[/game]");
    TS_ASSERT_EQUALS(renderSingle(testee, "g:12:3"), "player 3 in [game]12[/game]");
    TS_ASSERT_EQUALS(renderSingle(testee, ""), "");
}

/** Test compatibility of render() and parse(). */
void
TestServerTalkTalkAddress::testCompat()
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    h.session.renderOptions().setFormat("html");

    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "fred")),   "<a class=\"userlink\" href=\"userinfo.cgi/fred\">Fred F</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "g:12")),   "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "g:012")),  "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "g:12:3")), "player 3 in <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "")), "");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "foo")), "");
    TS_ASSERT_EQUALS(renderSingle(testee, parseSingle(testee, "g:3")), "");
}

