/**
  *  \file test/server/talk/render/mailrenderertest.cpp
  *  \brief Test for server::talk::render::MailRenderer
  */

#include "server/talk/render/mailrenderer.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"

using server::talk::render::renderMail;
using server::talk::TextNode;
using afl::net::redis::StringKey;
using afl::net::redis::HashKey;
using afl::net::redis::StringSetKey;

/** Render plaintext. */
AFL_TEST("server.talk.render.MailRenderer:plaintext", a)
{
    // Environment
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;             // options [not required?]

    // A single paragraph containing just text
    TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
    TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
    TextNode& text(*par.children.pushBackNew(new TextNode(TextNode::maPlain, 0)));

    // Basic test
    {
        text.text = "hi mom";
        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "hi mom\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "hi mom\n");
    }

    // Word wrap
    {
        text.text = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.";
        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem\nvelit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem\nvelit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n");
    }
}

/** Render some regular text. */
AFL_TEST("server.talk.render.MailRenderer:complex", a)
{
    // Environment
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;             // options [not required?]

    // Two paragraphs
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi"));

        TextNode& par2(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par2.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "hi\n\nmom\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "hi\n\nmom\n");
    }

    // Paragraph with inline formatting (bold)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "hi mom!\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "hi mom!\n");
    }

    // Same thing, colored (maInlineAttr instead of maInline)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAColor, "#ff0000"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("21", renderMail(tn, ctx, opts, root, false), "hi mom!\n");
        a.checkEqual("22", renderMail(tn, ctx, opts, root, true),  "hi mom!\n");
    }
}

/** Render some code. */
AFL_TEST("server.talk.render.MailRenderer:code", a)
{
    // Environment
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;             // options [not required?]

    // Normal
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "void foo()\n{\n}"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "  void foo()\n  {\n  }\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "  void foo()\n  {\n  }\n");
    }

    // DOS linefeeds
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "void foo()\r\n{\r\n}"));

        a.checkEqual("03", renderMail(tn, ctx, opts, root, false), "  void foo()\n  {\n  }\n");
        a.checkEqual("04", renderMail(tn, ctx, opts, root, true),  "  void foo()\n  {\n  }\n");
    }
}

/** Test rendering of links. */
AFL_TEST("server.talk.render.MailRenderer:link", a)
{
    // Environment
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;             // options [not required?]

    // A link with differing content and target
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "before text <http://web> after\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "before text <http://web> after\n");
    }

    // A link with no content (=shortened form)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "before <http://web> after\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "before <http://web> after\n");
    }
}

/** Test specials. */
AFL_TEST("server.talk.render.MailRenderer:special", a)
{
    // Environment
    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "u");   // context, required for quoting [not required?]
    server::talk::render::Options opts;                   // options
    opts.setBaseUrl("http://base/path/");

    // Image link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialImage, "http://xyz"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "before <http://xyz> after\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "before <http://xyz> after\n");
    }

    // Bad image link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialImage, "javascript:alert(\"hi\")"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "before <javascript:alert(\"hi\")> after\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "before <javascript:alert(\"hi\")> after\n");
    }

    // Image link with alt text
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialImage, "http://xyz"))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "some text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("21", renderMail(tn, ctx, opts, root, false), "before some text <http://xyz> after\n");
        a.checkEqual("22", renderMail(tn, ctx, opts, root, true),  "before some text <http://xyz> after\n");
    }

    // Break
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialBreak));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("31", renderMail(tn, ctx, opts, root, false), "before\nafter\n");
        a.checkEqual("32", renderMail(tn, ctx, opts, root, true),  "before\nafter\n");
    }

    // Smiley
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialSmiley, "smile"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("41", renderMail(tn, ctx, opts, root, false), "before :smile: after\n");
        a.checkEqual("42", renderMail(tn, ctx, opts, root, true),  "before :smile: after\n");
    }
}

/** Test rendering user links. */
AFL_TEST("server.talk.render.MailRenderer:link:user", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    // Create two users
    StringKey(db, "uid:fred").set("1000");
    StringKey(db, "uid:wilma").set("1001");
    StringKey(db, "user:1000:name").set("fred");
    StringKey(db, "user:1001:name").set("wilma");
    HashKey(db, "user:1000:profile").stringField("screenname").set("Fred F");
    HashKey(db, "user:1001:profile").stringField("screenname").set("Wilma F");

    // Regular user link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "[ <http://base/path/userinfo.cgi/wilma> ]\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "[ <http://base/path/userinfo.cgi/wilma> ]\n");
    }

    // Named user link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "Text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "[ Text <http://base/path/userinfo.cgi/wilma> ]\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "[ Text <http://base/path/userinfo.cgi/wilma> ]\n");
    }

    // Unknown user
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "barney"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("21", renderMail(tn, ctx, opts, root, false), "[ <user:barney> ]\n");
        a.checkEqual("22", renderMail(tn, ctx, opts, root, true),  "[ <user:barney> ]\n");
    }

    // Partial tree, just a paragraph fragment
    {
        TextNode tn(TextNode::maParagraph, TextNode::miParFragment);
        tn.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        tn.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"));
        tn.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("31", renderMail(tn, ctx, opts, root, false), "[ <http://base/path/userinfo.cgi/wilma> ]");
        a.checkEqual("32", renderMail(tn, ctx, opts, root, true),  "[ <http://base/path/userinfo.cgi/wilma> ]");
    }
}

/** Test more links. */
AFL_TEST("server.talk.render.MailRenderer:link:other", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    // Create environment
    // - a game
    StringSetKey(db, "game:all").add("7");
    StringKey(db, "game:7:state").set("running");
    StringKey(db, "game:7:type").set("public");
    StringKey(db, "game:7:name").set("Seven of Nine");

    // - a forum
    StringSetKey(db, "forum:all").add("3");
    HashKey(db, "forum:3:header").stringField("name").set("Chat Room");
    HashKey(db, "forum:3:header").stringField("newsgroup").set("news.group.name");

    // - another forum
    StringSetKey(db, "forum:all").add("4");
    HashKey(db, "forum:4:header").stringField("name").set("Other Room");

    // - a thread
    HashKey(db, "thread:9:header").stringField("subject").set("Hi There");
    HashKey(db, "thread:9:header").stringField("forum").set("3");
    HashKey(db, "thread:9:header").stringField("firstpost").set("12");

    // - a posting
    HashKey(db, "msg:12:header").stringField("subject").set("Re: Hi There");
    HashKey(db, "msg:12:header").stringField("thread").set("9");
    HashKey(db, "msg:13:header").stringField("subject").set("We can also use a very long title which will be abbreviated when linked");
    HashKey(db, "msg:13:header").stringField("thread").set("9");
    HashKey(db, "msg:14:header").stringField("subject").set("");
    HashKey(db, "msg:14:header").stringField("thread").set("9");

    // Forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "3"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "[ <http://base/path/talk/forum.cgi/3-Chat-Room> ]\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "[ <news:news.group.name> ]\n");
    }

    // Forum that does not have a newsgroup name
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "4"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("03", renderMail(tn, ctx, opts, root, false), "[ <http://base/path/talk/forum.cgi/4-Other-Room> ]\n");
        a.checkEqual("04", renderMail(tn, ctx, opts, root, true),  "[ <http://base/path/talk/forum.cgi/4-Other-Room> ]\n");
    }

    // Bad forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "5"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("05", renderMail(tn, ctx, opts, root, false), "[ <forum:5> ]\n");
        a.checkEqual("06", renderMail(tn, ctx, opts, root, true),  "[ <forum:5> ]\n");
    }

    // Named forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "3")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "[ text <http://base/path/talk/forum.cgi/3-Chat-Room> ]\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "[ text <news:news.group.name> ]\n");
    }

    // Bad named forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "5")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("13", renderMail(tn, ctx, opts, root, false), "[ text <forum:5> ]\n");
        a.checkEqual("14", renderMail(tn, ctx, opts, root, true),  "[ text <forum:5> ]\n");
    }

    // Thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "9"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("21", renderMail(tn, ctx, opts, root, false), "[ <thread:9> ]\n");
        a.checkEqual("22", renderMail(tn, ctx, opts, root, true),  "[ <12.0@localhost> ]\n");
    }

    // Bad thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "bad"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("23", renderMail(tn, ctx, opts, root, false), "[ <thread:bad> ]\n");
        a.checkEqual("23", renderMail(tn, ctx, opts, root, true),  "[ <thread:bad> ]\n");
    }

    // Named thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "9")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "label"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("31", renderMail(tn, ctx, opts, root, false), "[ label <thread:9> ]\n");
        a.checkEqual("32", renderMail(tn, ctx, opts, root, true),  "[ label <12.0@localhost> ]\n");
    }

    // Post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "12"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("41", renderMail(tn, ctx, opts, root, false), "[ <post:12> ]\n");
        a.checkEqual("42", renderMail(tn, ctx, opts, root, true),  "[ <12.0@localhost> ]\n");
    }

    // Bad post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "999"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("43", renderMail(tn, ctx, opts, root, false), "[ <post:999> ]\n");
        a.checkEqual("44", renderMail(tn, ctx, opts, root, true),  "[ <post:999> ]\n");
    }

    // Named post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "12")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("51", renderMail(tn, ctx, opts, root, false), "[ text <post:12> ]\n");
        a.checkEqual("52", renderMail(tn, ctx, opts, root, true),  "[ text <12.0@localhost> ]\n");
    }

    // Game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "7"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("61", renderMail(tn, ctx, opts, root, false), "[ <http://base/path/host/game.cgi/7-Seven-of-Nine> ]\n");
        a.checkEqual("62", renderMail(tn, ctx, opts, root, true),  "[ <http://base/path/host/game.cgi/7-Seven-of-Nine> ]\n");
    }

    // Named game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "7")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "play"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("71", renderMail(tn, ctx, opts, root, false), "[ play <http://base/path/host/game.cgi/7-Seven-of-Nine> ]\n");
        a.checkEqual("72", renderMail(tn, ctx, opts, root, true),  "[ play <http://base/path/host/game.cgi/7-Seven-of-Nine> ]\n");
    }

    // Bad game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "17"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("81", renderMail(tn, ctx, opts, root, false), "[ <game:17> ]\n");
        a.checkEqual("82", renderMail(tn, ctx, opts, root, true), "[ <game:17> ]\n");
    }

    // Email link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkEmail, "a@b.c"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("91", renderMail(tn, ctx, opts, root, false), "[ <mailto:a@b.c> ]\n");
        a.checkEqual("92", renderMail(tn, ctx, opts, root, true),  "[ <mailto:a@b.c> ]\n");
    }
}

/** Test quotes. */
AFL_TEST("server.talk.render.MailRenderer:quote", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    // Create environment
    StringKey(db, "user:1000:name").set("fred");
    StringKey(db, "uid:fred").set("1000");
    HashKey(db, "user:1000:profile").stringField("screenname").set("Fred F");

    // - a forum
    StringSetKey(db, "forum:all").add("3");
    HashKey(db, "forum:3:header").stringField("name").set("Chat Room");

    // - a thread
    HashKey(db, "thread:9:header").stringField("subject").set("Hi There");
    HashKey(db, "thread:9:header").stringField("forum").set("3");

    // - a posting
    HashKey(db, "msg:12:header").stringField("subject").set("Re: Hi There");
    HashKey(db, "msg:12:header").stringField("thread").set("9");

    // Existing user
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, "fred"))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "* Fred F:\n> text\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "* Fred F:\n> text\n");
    }

    // Nonexisting user
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, "barney"))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("03", renderMail(tn, ctx, opts, root, false), "* barney:\n> text\n");
        a.checkEqual("04", renderMail(tn, ctx, opts, root, true),  "* barney:\n> text\n");
    }

    // User and posting
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, "fred;12"))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("05", renderMail(tn, ctx, opts, root, false), "* Fred F in <post:12>:\n> text\n");
        a.checkEqual("06", renderMail(tn, ctx, opts, root, true),  "* Fred F in <12.0@localhost>:\n> text\n");
    }

    // Nonexistant user, existing posting
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, "barney;12"))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("07", renderMail(tn, ctx, opts, root, false), "* barney in <post:12>:\n> text\n");
        a.checkEqual("08", renderMail(tn, ctx, opts, root, true),  "* barney in <12.0@localhost>:\n> text\n");
    }

    // Existant user, nonexistant posting
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, "fred;77"))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("09", renderMail(tn, ctx, opts, root, false), "* Fred F in <post:77>:\n> text\n");
        a.checkEqual("10", renderMail(tn, ctx, opts, root, true),  "* Fred F in <post:77>:\n> text\n");
    }

    // No attribution
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, ""))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));

        a.checkEqual("11", renderMail(tn, ctx, opts, root, false), "> text\n");
        a.checkEqual("12", renderMail(tn, ctx, opts, root, true),  "> text\n");
    }

    // Word wrap
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote, ""))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque. "));

        a.checkEqual("13", renderMail(tn, ctx, opts, root, false), "> In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros,\n> sit amet sagittis nunc mi ac neque.\n");
        a.checkEqual("14", renderMail(tn, ctx, opts, root, true),  "> In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros,\n> sit amet sagittis nunc mi ac neque.\n");
    }
}

/** Test list. */
AFL_TEST("server.talk.render.MailRenderer:list", a)
{
    // Environment
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, server::talk::Configuration());
    const server::talk::render::Context ctx(root, "1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    // Compact form
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& list(*tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList)));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "first"));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "second"));

        a.checkEqual("01", renderMail(tn, ctx, opts, root, false), "* first\n\n* second\n");
        a.checkEqual("02", renderMail(tn, ctx, opts, root, true),  "* first\n\n* second\n");
    }

    // Compact form; numbering requested but not honored by MailRenderer
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& list(*tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList, "1")));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "first"));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "second"));

        a.checkEqual("03", renderMail(tn, ctx, opts, root, false), "* first\n\n* second\n");
        a.checkEqual("04", renderMail(tn, ctx, opts, root, true),  "* first\n\n* second\n");
    }

    // Full form
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& list(*tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList)));
        TextNode& n1 = *list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
        n1.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "first top"));
        n1.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "first bottom"));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "second"));

        a.checkEqual("05", renderMail(tn, ctx, opts, root, false), "* first top\n\n  first bottom\n\n* second\n");
        a.checkEqual("06", renderMail(tn, ctx, opts, root, true),  "* first top\n\n  first bottom\n\n* second\n");
    }

    // Word wrap
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& list(*tn.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList)));
        list.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem))
            ->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque. "));

        a.checkEqual("07", renderMail(tn, ctx, opts, root, false), "* In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros,\n  sit amet sagittis nunc mi ac neque.\n");
        a.checkEqual("08", renderMail(tn, ctx, opts, root, true),  "* In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros,\n  sit amet sagittis nunc mi ac neque.\n");
    }
}
