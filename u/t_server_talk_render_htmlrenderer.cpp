/**
  *  \file u/t_server_talk_render_htmlrenderer.cpp
  *  \brief Test for server::talk::render::HtmlRenderer
  */

#include "server/talk/render/htmlrenderer.hpp"

#include "t_server_talk_render.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"

/** Test some code highlighting.
    This is bug #330 which applies to the highlighter, but we're testing the full stack here. */
void
TestServerTalkRenderHtmlRenderer::testCode()
{
    using server::talk::TextNode;

    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, nch, server::talk::Configuration());
    server::talk::render::Context ctx("u");
    server::talk::render::Options opts;

    root.keywordTable().add("ini.phost.GameName.link", "http://phost.de/phost4doc/config.html#GameName");
    root.keywordTable().add("ini.phost.GameName.info", "Name of the game");

    // forum:[code=pconfig.src]pHost.Gamename=foo
    TextNode n1(TextNode::maGroup, TextNode::miGroupRoot);
    n1.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode, "pconfig.src"));
    n1.children[0]->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "pHost.Gamename=foo"));
    TS_ASSERT_EQUALS(server::talk::render::renderHTML(n1, ctx, opts, root),
                     "<pre><a href=\"http://phost.de/phost4doc/config.html#GameName\" title=\"Name of the game\" class=\"syn-name\">pHost.Gamename</a>=foo</pre>\n");

    // forum:[code=pconfig.src]%foo\nbar
    TextNode n2(TextNode::maGroup, TextNode::miGroupRoot);
    n2.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode, "pconfig.src"));
    n2.children[0]->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "%foo\nbar"));
    TS_ASSERT_EQUALS(server::talk::render::renderHTML(n2, ctx, opts, root),
                     "<pre><span class=\"syn-sec\">%foo</span>\n<span class=\"syn-name\">bar</span></pre>\n");
}

/** Render plaintext. */
void
TestServerTalkRenderHTMLRenderer::testPlaintext()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, nch, server::talk::Configuration());

    // A single paragraph containing just text
    TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
    TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
    TextNode& text(*par.children.pushBackNew(new TextNode(TextNode::maPlain, 0)));

    // Basic test
    {
        text.text = "hi mom";
        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi mom</p>\n");
    }

    // Looks like a tag
    {
        text.text = "a<b>c";
        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>a&lt;b&gt;c</p>\n");
    }

    // Ampersand
    {
        text.text = "a&c";
        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>a&amp;c</p>\n");
    }
}

/** Render some regular text. */
void
TestServerTalkRenderHTMLRenderer::testText()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, nch, server::talk::Configuration());

    // Two paragraphs
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi"));

        TextNode& par2(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par2.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi</p>\n<p>mom</p>\n");
    }

    // Paragraph with inline formatting (bold)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <b>mom</b>!</p>\n");
    }

    // Same thing, italic
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInItalic));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <em>mom</em>!</p>\n");
    }

    // Same thing, strikethrough
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInStrikeThrough));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <s>mom</s>!</p>\n");
    }

    // Same thing, underlined
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInUnderline));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <u>mom</u>!</p>\n");
    }

    // Same thing, monospaced
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInMonospace));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <tt>mom</tt>!</p>\n");
    }

    // Same thing, invalid maInline
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, 99));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi mom!</p>\n");
    }

    // Same thing, colored
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAColor, "#ff0000"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <font color=\"#ff0000\">mom</font>!</p>\n");
    }

    // Same thing, font
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAFont, "courier"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <span style=\"font-family: courier;\">mom</span>!</p>\n");
    }

    // Same thing, font that needs quoting
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAFont, "x&y"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <span style=\"font-family: x&amp;y;\">mom</span>!</p>\n");
    }

    // Same thing, increased size
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIASize, "3"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <span style=\"font-size: 195%;\">mom</span>!</p>\n");
    }

    // Same thing, reduced size
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIASize, "-1"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi <span style=\"font-size: 80%;\">mom</span>!</p>\n");
    }

    // Same thing, attributeless size
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIASize, ""));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi mom!</p>\n");
    }

    // Same thing, invalid maInlineAttr
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, 99, "3"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>hi mom!</p>\n");
    }
}

/** Test rendering of links. */
void
TestServerTalkRenderHTMLRenderer::testLink()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, nch, server::talk::Configuration());

    // A link with differing content and target
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <a href=\"http://web\" rel=\"nofollow\">text</a> after</p>\n");
    }

    // A link with no content (=shortened form)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <a href=\"http://web\" rel=\"nofollow\">http://web</a> after</p>\n");
    }

    // Quoted link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://a/x<y>z"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <a href=\"http://a/x&lt;y&gt;z\" rel=\"nofollow\">http://a/x&lt;y&gt;z</a> after</p>\n");
    }
}

/** Test specials. */
void
TestServerTalkRenderHTMLRenderer::testSpecial()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    server::talk::render::Options opts;             // options
    opts.setBaseUrl("http://base/path/");

    afl::net::NullCommandHandler nch;
    server::talk::Root root(nch, nch, server::talk::Configuration());

    // Image link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialImage, "http://xyz"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <img src=\"http://xyz\" /> after</p>\n");
    }

    // Break
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialBreak));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <br /> after</p>\n");
    }

    // Smiley
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialSmiley, "smile"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>before <img src=\"http://base/path/res/smileys/smile.png\" width=\"16\" height=\"16\" alt=\":smile:\" /> after</p>\n");
    }
}

/** Test rendering user links. */
void
TestServerTalkRenderHTMLRenderer::testUser()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;
    using afl::net::redis::StringKey;
    using afl::net::redis::HashKey;

    // Environment
    const server::talk::render::Context ctx("1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    afl::net::NullCommandHandler nch;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, nch, server::talk::Configuration());

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

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a class=\"userlink\" href=\"http://base/path/userinfo.cgi/wilma\">Wilma F</a> ]</p>\n");
    }

    // Regular user link to user himself
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "fred"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a class=\"userlink userlink-me\" href=\"http://base/path/userinfo.cgi/fred\">Fred F</a> ]</p>\n");
    }

    // Unknown user
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "barney"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <span class=\"tfailedlink\">user barney</span> ]</p>\n");
    }

    // Partial tree, just a paragraph fragment
    {
        TextNode tn(TextNode::maParagraph, TextNode::miParFragment);
        tn.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        tn.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"));
        tn.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "[ <a class=\"userlink\" href=\"http://base/path/userinfo.cgi/wilma\">Wilma F</a> ]");
    }
}

/** Test more links. */
void
TestServerTalkRenderHTMLRenderer::testLinks2()
{
    using server::talk::render::renderHTML;
    using server::talk::TextNode;
    using afl::net::redis::StringKey;
    using afl::net::redis::HashKey;
    using afl::net::redis::StringSetKey;

    // Environment
    const server::talk::render::Context ctx("1000");
    server::talk::render::Options opts;
    opts.setBaseUrl("http://base/path/");

    afl::net::NullCommandHandler nch;
    afl::net::redis::InternalDatabase db;
    server::talk::Root root(db, nch, server::talk::Configuration());

    // Create environment
    // - a game
    StringSetKey(db, "game:all").add("7");
    StringKey(db, "game:7:state").set("running");
    StringKey(db, "game:7:type").set("public");
    StringKey(db, "game:7:name").set("Seven of Nine");

    // - a forum
    StringSetKey(db, "forum:all").add("3");
    HashKey(db, "forum:3:header").stringField("name").set("Chat Room");

    // - a thread
    HashKey(db, "thread:9:header").stringField("subject").set("Hi There");
    HashKey(db, "thread:9:header").stringField("forum").set("3");

    // - a posting
    HashKey(db, "msg:12:header").stringField("subject").set("Re: Hi There");
    HashKey(db, "msg:12:header").stringField("thread").set("9");

    // Forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "3"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/forum.cgi/3-Chat-Room\">Chat Room</a> ]</p>\n");
    }

    // Named forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "3")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/forum.cgi/3-Chat-Room\">text</a> ]</p>\n");
    }

    // Thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "9"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/thread.cgi/9-Hi-There\">Hi There</a> ]</p>\n");
    }

    // Named thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "9")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "label"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/thread.cgi/9-Hi-There\">label</a> ]</p>\n");
    }

    // Post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "12"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/thread.cgi/9-Hi-There#p12\">Re: Hi There</a> ]</p>\n");
    }

    // Named post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "12")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/talk/thread.cgi/9-Hi-There#p12\">text</a> ]</p>\n");
    }

    // Game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "7"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/host/game.cgi/7-Seven-of-Nine\">Seven of Nine</a> ]</p>\n");
    }

    // Named game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "7")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "play"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <a href=\"http://base/path/host/game.cgi/7-Seven-of-Nine\">play</a> ]</p>\n");
    }

    // Bad game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "17"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        TS_ASSERT_EQUALS(renderHTML(tn, ctx, opts, root), "<p>[ <span class=\"tfailedlink\">game 17</span> ]</p>\n");
    }
}

