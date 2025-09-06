/**
  *  \file test/server/talk/render/contexttest.cpp
  *  \brief Test for server::talk::render::Context
  */

#include "server/talk/render/context.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/root.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"

using afl::base::Optional;
using afl::net::redis::HashKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using server::talk::LinkParser;

namespace {
    struct Environment {
        afl::net::redis::InternalDatabase db;
        server::talk::Root root;
        server::talk::render::Context ctx;

        Environment()
            : db(), root(db, server::talk::Configuration()), ctx(root, "1000")
            { }
    };
}

/** Simple test. */
AFL_TEST("server.talk.render.Context", a)
{
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    server::talk::render::Context testee(root, "u");

    // Initial state
    a.checkEqual("01. getUser",          testee.getUser(), "u");
    a.checkEqual("02. getMessageId",     testee.getMessageId(), 0);
    a.checkEqual("03. getMessageAuthor", testee.getMessageAuthor(), "");

    // Message Id
    testee.setMessageId(42);
    a.checkEqual("11. getUser",          testee.getUser(), "u");
    a.checkEqual("12. getMessageId",     testee.getMessageId(), 42);
    a.checkEqual("13. getMessageAuthor", testee.getMessageAuthor(), "");

    // Message Author
    testee.setMessageAuthor("a");
    a.checkEqual("21. getUser",          testee.getUser(), "u");
    a.checkEqual("22. getMessageId",     testee.getMessageId(), 0);  // reset!
    a.checkEqual("23. getMessageAuthor", testee.getMessageAuthor(), "a");

    // Id again
    testee.setMessageId(13);
    a.checkEqual("31. getUser",          testee.getUser(), "u");
    a.checkEqual("32. getMessageId",     testee.getMessageId(), 13);
    a.checkEqual("33. getMessageAuthor", testee.getMessageAuthor(), ""); // reset!
}

/** Test parseGameLink. */
AFL_TEST("server.talk.render.Context:parseGameLink", a)
{
    Environment env;

    StringSetKey(env.db, "game:all").add("7");
    StringKey(env.db, "game:7:state").set("running");
    StringKey(env.db, "game:7:type").set("public");
    StringKey(env.db, "game:7:name").set("Seven of Nine");

    StringSetKey(env.db, "game:all").add("11");
    StringKey(env.db, "game:11:state").set("deleted");
    StringKey(env.db, "game:11:type").set("public");
    StringKey(env.db, "game:11:name").set("Eleven");

    StringSetKey(env.db, "game:all").add("23");
    StringKey(env.db, "game:23:state").set("running");
    StringKey(env.db, "game:23:type").set("private");
    StringKey(env.db, "game:23:owner").set("1000");
    StringKey(env.db, "game:23:name").set("Twenty-three");

    StringSetKey(env.db, "game:all").add("24");
    StringKey(env.db, "game:24:state").set("running");
    StringKey(env.db, "game:24:type").set("private");
    StringKey(env.db, "game:24:owner").set("1002");
    StringKey(env.db, "game:24:name").set("Twenty-four");

    StringSetKey(env.db, "game:all").add("0");

    // Invalid Id
    a.check("01. invalid", !env.ctx.parseGameLink("").isValid());
    a.check("02. invalid", !env.ctx.parseGameLink("0").isValid());
    a.check("03. invalid", !env.ctx.parseGameLink("-1").isValid());
    a.check("04. invalid", !env.ctx.parseGameLink("a").isValid());
    a.check("05. invalid", !env.ctx.parseGameLink("0x7").isValid());

    // Not in all
    a.check("11. non-existant", !env.ctx.parseGameLink("3").isValid());

    // Wrong state
    a.check("21. wrong-state", !env.ctx.parseGameLink("11").isValid());

    // Wrong owner
    a.check("31. wrong-owner", !env.ctx.parseGameLink("24").isValid());

    // Success for public game
    Optional<LinkParser::Result_t> r = env.ctx.parseGameLink("7");
    a.checkNonNull("41. public", r.get());
    a.checkEqual("42. public id", r.get()->first, 7);
    a.checkEqual("43. public name", r.get()->second, "Seven of Nine");

    // Success for private game
    Optional<LinkParser::Result_t> r2 = env.ctx.parseGameLink("23");
    a.checkNonNull("41. private", r2.get());
    a.checkEqual("42. private id", r2.get()->first, 23);
    a.checkEqual("43. private name", r2.get()->second, "Twenty-three");
}

/** Test parseForumLink. */
AFL_TEST("server.talk.render.Context:parseForumLink", a)
{
    Environment env;

    StringSetKey(env.db, "forum:all").add("3");
    HashKey(env.db, "forum:3:header").stringField("name").set("Chat Room");

    // Invalid Id
    a.check("01. invalid", !env.ctx.parseForumLink("").isValid());
    a.check("02. invalid", !env.ctx.parseForumLink("0").isValid());
    a.check("03. invalid", !env.ctx.parseForumLink("-1").isValid());
    a.check("04. invalid", !env.ctx.parseForumLink("a").isValid());
    a.check("05. invalid", !env.ctx.parseForumLink("0x7").isValid());

    // Not in all
    a.check("11. non-existant", !env.ctx.parseForumLink("5").isValid());

    // Valid
    Optional<LinkParser::Result_t> r = env.ctx.parseForumLink("3");
    a.checkNonNull("21. valid", r.get());
    a.checkEqual("22. valid id", r.get()->first, 3);
    a.checkEqual("23. valid name", r.get()->second, "Chat Room");
}

/** Test parseTopicLink. */
AFL_TEST("server.talk.render.Context:parseTopicLink", a)
{
    Environment env;

    StringSetKey(env.db, "topic:all").add("3");
    HashKey(env.db, "topic:3:header").stringField("name").set("Chat Room");
    HashKey(env.db, "thread:9:header").stringField("subject").set("Hi There");
    HashKey(env.db, "thread:9:header").stringField("forum").set("3");

    // Invalid Id
    a.check("01. invalid", !env.ctx.parseTopicLink("").isValid());
    a.check("02. invalid", !env.ctx.parseTopicLink("0").isValid());
    a.check("03. invalid", !env.ctx.parseTopicLink("-1").isValid());
    a.check("04. invalid", !env.ctx.parseTopicLink("a").isValid());
    a.check("05. invalid", !env.ctx.parseTopicLink("0x7").isValid());

    // Not in all
    a.check("11. non-existant", !env.ctx.parseTopicLink("5").isValid());

    // Valid
    Optional<LinkParser::Result_t> r = env.ctx.parseTopicLink("9");
    a.checkNonNull("21. valid", r.get());
    a.checkEqual("22. valid id", r.get()->first, 9);
    a.checkEqual("23. valid name", r.get()->second, "Hi There");
}

/** Test parseMessageLink. */
AFL_TEST("server.talk.render.Context:parseMessageLink", a)
{
    Environment env;

    StringSetKey(env.db, "message:all").add("3");
    HashKey(env.db, "message:3:header").stringField("name").set("Chat Room");
    HashKey(env.db, "thread:9:header").stringField("subject").set("Hi There");
    HashKey(env.db, "thread:9:header").stringField("forum").set("3");
    HashKey(env.db, "msg:12:header").stringField("subject").set("Re: Hi There");
    HashKey(env.db, "msg:12:header").stringField("thread").set("9");

    // Invalid Id
    a.check("01. invalid", !env.ctx.parseMessageLink("").isValid());
    a.check("02. invalid", !env.ctx.parseMessageLink("0").isValid());
    a.check("03. invalid", !env.ctx.parseMessageLink("-1").isValid());
    a.check("04. invalid", !env.ctx.parseMessageLink("a").isValid());
    a.check("05. invalid", !env.ctx.parseMessageLink("0x7").isValid());

    // Not in all
    a.check("11. non-existant", !env.ctx.parseMessageLink("5").isValid());

    // Valid
    Optional<LinkParser::Result_t> r = env.ctx.parseMessageLink("12");
    a.checkNonNull("21. valid", r.get());
    a.checkEqual("22. valid id", r.get()->first, 12);
    a.checkEqual("23. valid name", r.get()->second, "Re: Hi There");
}

/** Test parseUserLink. */
AFL_TEST("server.talk.render.Context:parseUserLink", a)
{
    Environment env;
    StringKey(env.db, "uid:fred").set("2000");
    StringKey(env.db, "user:2000:name").set("fred");

    a.check("01. invalid", !env.ctx.parseUserLink("wilma").isValid());

    a.checkEqual("11. valid",  env.ctx.parseUserLink("fred").orElse("?"), "2000");
}
