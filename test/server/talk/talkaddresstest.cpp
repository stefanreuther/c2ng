/**
  *  \file test/server/talk/talkaddresstest.cpp
  *  \brief Test for server::talk::TalkAddress
  */

#include "server/talk/talkaddress.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
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

    String_t parseSingle(afl::test::Assert a, server::interface::TalkAddress& ta, String_t u)
    {
        String_t in[] = {u};
        afl::data::StringList_t out;
        ta.parse(in, out);
        a(u).checkEqual("parse result size", out.size(), 1U);
        return out[0];
    }

    String_t renderSingle(afl::test::Assert a, server::interface::TalkAddress& ta, String_t u)
    {
        String_t in[] = {u};
        afl::data::StringList_t out;
        ta.render(in, out);
        a(u).checkEqual("render result size", out.size(), 1U);
        return out[0];
    }
}

/** Test parse(). */
AFL_TEST("server.talk.TalkAddress:parse", a)
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    // Normal
    a.checkEqual("01", parseSingle(a, testee, "fred"), "u:1000");
    a.checkEqual("02", parseSingle(a, testee, "wilma"), "u:1001");
    a.checkEqual("03", parseSingle(a, testee, "g:12"), "g:12");
    a.checkEqual("04", parseSingle(a, testee, "g:12:3"), "g:12:3");

    // Variants
    a.checkEqual("11", parseSingle(a, testee, "--fred--"), "u:1000");
    a.checkEqual("12", parseSingle(a, testee, "WiLmA"), "u:1001");
    a.checkEqual("13", parseSingle(a, testee, "g:012"), "g:12");
    a.checkEqual("14", parseSingle(a, testee, "g:012:003"), "g:12:3");

    // Errors
    a.checkEqual("21", parseSingle(a, testee, ""), "");
    a.checkEqual("22", parseSingle(a, testee, "barney"), "");
    a.checkEqual("23", parseSingle(a, testee, "g:4294967308"), "");
    a.checkEqual("24", parseSingle(a, testee, "u:"), "");
    a.checkEqual("25", parseSingle(a, testee, "g:"), "");
    a.checkEqual("26", parseSingle(a, testee, "g:-1"), "");
    a.checkEqual("27", parseSingle(a, testee, "g:10"), "");
    a.checkEqual("28", parseSingle(a, testee, "g:12:0"), "");
    a.checkEqual("29", parseSingle(a, testee, "g:12:"), "");
    a.checkEqual("30", parseSingle(a, testee, "g:12:12"), "");
    a.checkEqual("31", parseSingle(a, testee, "G:"), "");
}

/** Test render(), raw format. */
AFL_TEST("server.talk.TalkAddress:render:raw", a)
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    // Default format is "raw"
    a.checkEqual("01", h.session.renderOptions().getFormat(), "raw");

    // Normal
    a.checkEqual("11", renderSingle(a, testee, "u:1000"), "fred");
    a.checkEqual("12", renderSingle(a, testee, "g:12"), "g:12");
    a.checkEqual("13", renderSingle(a, testee, "g:12:3"), "g:12:3");

    // Errors
    a.checkEqual("21", renderSingle(a, testee, ""), "");
    a.checkEqual("22", renderSingle(a, testee, "whoops"), "");
    a.checkEqual("23", renderSingle(a, testee, "g:9999"), "");
    a.checkEqual("24", renderSingle(a, testee, "g:12:13"), "");
    a.checkEqual("25", renderSingle(a, testee, "u:2222"), "");
    a.checkEqual("26", renderSingle(a, testee, "g:12x"), "");
    a.checkEqual("27", renderSingle(a, testee, "g:x"), "");
    a.checkEqual("28", renderSingle(a, testee, "g:"), "");
}

/** Test render(), HTML format. */
AFL_TEST("server.talk.TalkAddress:render:html", a)
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    h.session.renderOptions().setFormat("html");

    // Normal
    a.checkEqual("01", renderSingle(a, testee, "u:1000"), "<a class=\"userlink\" href=\"userinfo.cgi/fred\">Fred F</a>");
    a.checkEqual("02", renderSingle(a, testee, "g:12"), "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    a.checkEqual("03", renderSingle(a, testee, "g:12:3"), "player 3 in <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");

    // Errors
    a.checkEqual("11", renderSingle(a, testee, ""), "");
    a.checkEqual("12", renderSingle(a, testee, "whoops"), "");
    a.checkEqual("13", renderSingle(a, testee, "g:9999"), "");
    a.checkEqual("14", renderSingle(a, testee, "g:12:13"), "");
    a.checkEqual("15", renderSingle(a, testee, "u:2222"), "");
    a.checkEqual("16", renderSingle(a, testee, "g:12x"), "");
    a.checkEqual("17", renderSingle(a, testee, "g:x"), "");
    a.checkEqual("18", renderSingle(a, testee, "g:"), "");
}

/** Test render(), other formats. */
AFL_TEST("server.talk.TalkAddress:render:other-formats", a)
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);
    h.session.renderOptions().setBaseUrl("http://x/");

    // Mail
    h.session.renderOptions().setFormat("mail");
    a.checkEqual("01", renderSingle(a, testee, "u:1000"), "<http://x/userinfo.cgi/fred>");
    a.checkEqual("02", renderSingle(a, testee, "g:12"), "players of <http://x/host/game.cgi/12-Twelve>");
    a.checkEqual("03", renderSingle(a, testee, "g:12:3"), "player 3 in <http://x/host/game.cgi/12-Twelve>");
    a.checkEqual("04", renderSingle(a, testee, ""), "");

    // News
    h.session.renderOptions().setFormat("news");
    a.checkEqual("11", renderSingle(a, testee, "u:1000"), "<http://x/userinfo.cgi/fred>");
    a.checkEqual("12", renderSingle(a, testee, "g:12"), "players of <http://x/host/game.cgi/12-Twelve>");
    a.checkEqual("13", renderSingle(a, testee, "g:12:3"), "player 3 in <http://x/host/game.cgi/12-Twelve>");
    a.checkEqual("14", renderSingle(a, testee, ""), "");

    // Text
    h.session.renderOptions().setFormat("text");
    a.checkEqual("21", renderSingle(a, testee, "u:1000"), "fred");    // FIXME: is this the desired behaviour?
    a.checkEqual("22", renderSingle(a, testee, "g:12"), "players of Twelve");
    a.checkEqual("23", renderSingle(a, testee, "g:12:3"), "player 3 in Twelve");
    a.checkEqual("24", renderSingle(a, testee, ""), "");

    // BBCode
    h.session.renderOptions().setFormat("forum");
    a.checkEqual("31", renderSingle(a, testee, "u:1000"), "[user]fred[/user]");
    a.checkEqual("32", renderSingle(a, testee, "g:12"), "players of [game]12[/game]");
    a.checkEqual("33", renderSingle(a, testee, "g:12:3"), "player 3 in [game]12[/game]");
    a.checkEqual("34", renderSingle(a, testee, ""), "");
}

/** Test compatibility of render() and parse(). */
AFL_TEST("server.talk.TalkAddress:roundtrip", a)
{
    TestHarness h;
    server::talk::TalkAddress testee(h.session, h.root);

    h.session.renderOptions().setFormat("html");

    a.checkEqual("01", renderSingle(a, testee, parseSingle(a, testee, "fred")),   "<a class=\"userlink\" href=\"userinfo.cgi/fred\">Fred F</a>");
    a.checkEqual("02", renderSingle(a, testee, parseSingle(a, testee, "g:12")),   "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    a.checkEqual("03", renderSingle(a, testee, parseSingle(a, testee, "g:012")),  "players of <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    a.checkEqual("04", renderSingle(a, testee, parseSingle(a, testee, "g:12:3")), "player 3 in <a href=\"host/game.cgi/12-Twelve\">Twelve</a>");
    a.checkEqual("05", renderSingle(a, testee, parseSingle(a, testee, "")), "");
    a.checkEqual("06", renderSingle(a, testee, parseSingle(a, testee, "foo")), "");
    a.checkEqual("07", renderSingle(a, testee, parseSingle(a, testee, "g:3")), "");
}
