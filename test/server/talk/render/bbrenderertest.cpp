/**
  *  \file test/server/talk/render/bbrenderertest.cpp
  *  \brief Test for server::talk::render::BBRenderer
  */

#include "server/talk/render/bbrenderer.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/talk/configuration.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/textnode.hpp"

/** Render plaintext. */
AFL_TEST("server.talk.render.BBRenderer:plaintext", a)
{
    using server::talk::render::renderBB;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());

    const InlineRecognizer::Kinds_t noKinds;
    const InlineRecognizer::Kinds_t allKinds = InlineRecognizer::Kinds_t() + InlineRecognizer::Smiley + InlineRecognizer::Link;

    // A single paragraph containing just text
    TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
    TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
    TextNode& text(*par.children.pushBackNew(new TextNode(TextNode::maPlain, 0)));

    // Basic test
    {
        text.text = "hi mom";
        a.checkEqual("01", renderBB(tn, ctx, opts, root, noKinds), "hi mom");
        a.checkEqual("02", renderBB(tn, ctx, opts, root, allKinds), "hi mom");
    }

    // Looks like a paragraph
    {
        text.text = "hi\n\n\nmom";
        a.checkEqual("11", renderBB(tn, ctx, opts, root, noKinds), "hi mom");
        a.checkEqual("12", renderBB(tn, ctx, opts, root, allKinds), "hi mom");
    }

    // Looks like a tag
    {
        text.text = "a[b]c";
        a.checkEqual("21", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][b][/noparse]c");
        a.checkEqual("22", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][b][/noparse]c");
    }

    // Looks like a tag
    {
        text.text = "a[b]b[b]c";
        a.checkEqual("31", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][b]b[b][/noparse]c");
        a.checkEqual("32", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][b]b[b][/noparse]c");
    }

    // Looks like a tag
    {
        text.text = "a[b]b[/b]c";
        a.checkEqual("41", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][b]b[/b][/noparse]c");
        a.checkEqual("42", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][b]b[/b][/noparse]c");
    }

    // Not a tag
    {
        text.text = "a[bbb]c";
        a.checkEqual("51", renderBB(tn, ctx, opts, root, noKinds), "a[bbb]c");
        a.checkEqual("52", renderBB(tn, ctx, opts, root, allKinds), "a[bbb]c");
    }

    // "noparse" tag
    {
        text.text = "a[noparse]b";
        a.checkEqual("61", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][noparse][/noparse]b");
        a.checkEqual("62", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][noparse][/noparse]b");
    }

    // "/noparse" tag
    {
        text.text = "a[/noparse]b";
        a.checkEqual("71", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][/[/noparse]noparse]b");
        a.checkEqual("72", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][/[/noparse]noparse]b");
    }

    // Smiley
    {
        text.text = "I :-) U";
        a.checkEqual("81", renderBB(tn, ctx, opts, root, noKinds), "I :-) U");
        a.checkEqual("82", renderBB(tn, ctx, opts, root, allKinds), "I [noparse]:-)[/noparse] U");
    }

    // Smiley
    {
        text.text = "I :smile: U";
        a.checkEqual("91", renderBB(tn, ctx, opts, root, noKinds), "I :smile: U");
        a.checkEqual("92", renderBB(tn, ctx, opts, root, allKinds), "I [noparse]:smile:[/noparse] U");
    }

    // URL
    {
        text.text = "see http://url for more";
        a.checkEqual("101", renderBB(tn, ctx, opts, root, noKinds), "see http://url for more");
        a.checkEqual("102", renderBB(tn, ctx, opts, root, allKinds), "see [noparse]http://url[/noparse] for more");
    }

    // Ends with tag
    {
        text.text = "a[b]";
        a.checkEqual("111", renderBB(tn, ctx, opts, root, noKinds), "a[noparse][b][/noparse]");
        a.checkEqual("112", renderBB(tn, ctx, opts, root, allKinds), "a[noparse][b][/noparse]");
    }

    // At-link
    {
        text.text = "hi @user";
        a.checkEqual("121", renderBB(tn, ctx, opts, root, noKinds), "hi [noparse]@user[/noparse]");
        a.checkEqual("122", renderBB(tn, ctx, opts, root, allKinds), "hi [noparse]@user[/noparse]");
    }

    // Not an at-link
    {
        text.text = "game @ host";
        a.checkEqual("131", renderBB(tn, ctx, opts, root, noKinds), "game @ host");
        a.checkEqual("132", renderBB(tn, ctx, opts, root, allKinds), "game @ host");
    }
}

/** Render some regular text. */
AFL_TEST("server.talk.render.BBRenderer:complex", a)
{
    using server::talk::render::renderBB;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());

    const InlineRecognizer::Kinds_t noKinds;

    // Two paragraphs
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi"));

        TextNode& par2(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par2.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));

        a.checkEqual("01", renderBB(tn, ctx, opts, root, noKinds), "hi\n\nmom");
    }

    // Paragraph with inline formatting (bold)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("11", renderBB(tn, ctx, opts, root, noKinds), "hi [b]mom[/b]!");
    }

    // Same thing, italic
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInItalic));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("21", renderBB(tn, ctx, opts, root, noKinds), "hi [i]mom[/i]!");
    }

    // Same thing, strikethrough
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInStrikeThrough));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("31", renderBB(tn, ctx, opts, root, noKinds), "hi [s]mom[/s]!");
    }

    // Same thing, underlined
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInUnderline));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("41", renderBB(tn, ctx, opts, root, noKinds), "hi [u]mom[/u]!");
    }

    // Same thing, monospaced
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInMonospace));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("51", renderBB(tn, ctx, opts, root, noKinds), "hi [tt]mom[/tt]!");
    }

    // Same thing, invalid maInline
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, 99));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("61", renderBB(tn, ctx, opts, root, noKinds), "hi mom!");
    }

    // Same thing, colored
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAColor, "#ff0000"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("71", renderBB(tn, ctx, opts, root, noKinds), "hi [color=#ff0000]mom[/color]!");
    }

    // Same thing, font
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAFont, "courier"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("81", renderBB(tn, ctx, opts, root, noKinds), "hi [font=courier]mom[/font]!");
    }

    // Same thing, font that needs quoting
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIAFont, "arial[tm]"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("91", renderBB(tn, ctx, opts, root, noKinds), "hi [font=\"arial[tm]\"]mom[/font]!");
    }

    // Same thing, size
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIASize, "3"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("101", renderBB(tn, ctx, opts, root, noKinds), "hi [size=3]mom[/size]!");
    }

    // Same thing, attributeless size
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, TextNode::miIASize, ""));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("111", renderBB(tn, ctx, opts, root, noKinds), "hi [size]mom[/size]!");
    }

    // Same thing, invalid maInlineAttr
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInlineAttr, 99, "3"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("121", renderBB(tn, ctx, opts, root, noKinds), "hi mom!");
    }
}

/** Test rendering of links. */
AFL_TEST("server.talk.render.BBRenderer:link", a)
{
    using server::talk::render::renderBB;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());

    const InlineRecognizer::Kinds_t noKinds;

    // A link with differing content and target
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("01", renderBB(tn, ctx, opts, root, noKinds), "before [url=http://web]text[/url] after");
    }

    // A link with no content (=shortened form)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("11", renderBB(tn, ctx, opts, root, noKinds), "before [url]http://web[/url] after");
    }

    // A link with no content (=shortened form)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "user@host"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("21", renderBB(tn, ctx, opts, root, noKinds), "before [url]user@host[/url] after");
    }

    // Unshortenable link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "@foo"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("31", renderBB(tn, ctx, opts, root, noKinds), "before [url=@foo][/url] after");
    }

    // Unshortenable link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "bar @foo"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("41", renderBB(tn, ctx, opts, root, noKinds), "before [url=bar @foo][/url] after");
    }

    // Unshortenable link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://x/y?a[1]=2"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("51", renderBB(tn, ctx, opts, root, noKinds), "before [url=\"http://x/y?a[1]=2\"][/url] after");
    }
}

/** Test specials. */
AFL_TEST("server.talk.render.BBRenderer:special", a)
{
    using server::talk::render::renderBB;
    using server::talk::TextNode;
    using server::talk::InlineRecognizer;

    // Environment
    const server::talk::render::Context ctx("u");   // context, required for quoting [not required?]
    const server::talk::render::Options opts;       // options [not required?]

    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler mail;
    server::talk::Root root(db, mail, server::talk::Configuration());

    const InlineRecognizer::Kinds_t noKinds;
    const InlineRecognizer::Kinds_t allKinds = InlineRecognizer::Kinds_t() + InlineRecognizer::Smiley + InlineRecognizer::Link;

    // Image link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialImage, "http://xyz"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("01", renderBB(tn, ctx, opts, root, noKinds), "before [img]http://xyz[/img] after");
        a.checkEqual("02", renderBB(tn, ctx, opts, root, allKinds), "before [img]http://xyz[/img] after");
    }

    // Break
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialBreak));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("11", renderBB(tn, ctx, opts, root, noKinds), "before [nl] after");
        a.checkEqual("12", renderBB(tn, ctx, opts, root, allKinds), "before [nl] after");
    }

    // Smiley
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialSmiley, "smile"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("21", renderBB(tn, ctx, opts, root, noKinds), "before [:smile:] after");
        a.checkEqual("22", renderBB(tn, ctx, opts, root, allKinds), "before :smile: after");
    }
}
