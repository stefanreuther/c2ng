/**
  *  \file test/server/talk/render/rendertest.cpp
  *  \brief Test for server::talk::render::Render
  */

#include "server/talk/render/render.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"

using server::interface::TalkRender;

namespace {
    struct Environment {
        server::talk::Root root;
        server::talk::render::Context ctx;
        server::talk::render::Options opts;
        afl::net::NullCommandHandler cmdh;
        Environment()
            : root(cmdh, server::talk::Configuration()), ctx(root, "user"), opts(), cmdh()
            { }
    };
}

/** "format" format. */
AFL_TEST("server.talk.render.Render:format", a)
{
    Environment env;
    env.opts.setFormat("format");

    const char*const TEXT = "forum:"
        "para 1\n\n"
        "para 2\n\n"
        "para 3\n\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "forum");
}

/** "format" format, missing tag. */
AFL_TEST("server.talk.render.Render:format:error", a)
{
    Environment env;
    env.opts.setFormat("format");

    const char*const TEXT = "text text text";
    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "text text text");
}

/** Abstract handling, base case.
    Paragraphs must be separated by space. */
AFL_TEST("server.talk.render.Render:abstract", a)
{
    Environment env;
    env.opts.setFormat("abstract:text");

    const char*const TEXT = "forum:"
        "para 1\n\n"
        "para 2\n\n"
        "para 3\n\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "para 1 para 2");
}

/** Abstract handling, long paragraph and link.
    The given text causes the link text to be removed.
    Thus, abstract generation must remove the link to not cause the renderer to emit the URL. */
AFL_TEST("server.talk.render.Render:abstract:long+link", a)
{
    Environment env;
    env.opts.setFormat("abstract:text");

    const char*const TEXT = "forum:Aaaaa aaaa aaa aaaa aaaa aaa aaaaaaaa aaaaaaaa. Aaa aaaaaa aaa aaaa aa: A aaa aaaaaaaa aaaa aaaaaa. "
        "Aaa aaa aaaa aaaaaa aaaaaaaaa aaa aaaaaaaa 3-A aaaaaa aaa aaa aaaaa. Aaa aaa aaa aaaa aa aaa AAA etc etc etc [url=http://link/]click here[/url]";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root),
                 "Aaaaa aaaa aaa aaaa aaaa aaa aaaaaaaa aaaaaaaa. Aaa aaaaaa aaa aaaa aa: A aaa aaaaaaaa aaaa aaaaaa. "
                 "Aaa aaa aaaa aaaaaa aaaaaaaaa aaa aaaaaaaa 3-A aaaaaa aaa aaa aaaaa. Aaa aaa aaa aaaa aa aaa AAA ...");
}

/** Abstract handling, HTML with CRLF.
    CRLF must correctly be removed. */
AFL_TEST("server.talk.render.Render:abstract:html+crlf", a)
{
    Environment env;
    env.opts.setFormat("abstract:html");

    const char*const TEXT = "forum:"
        "para 1\r\n\r\n"
        "para 2\r\n\r\n"
        "para 3\r\n\r\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "<p>para 1</p>\n<p>para 2</p>\n");
}

/** Abstract handling, [break]. */
AFL_TEST("server.talk.render.Render:abstract:break", a)
{
    Environment env;
    env.opts.setFormat("abstract:text");

    const char*const TEXT = "forum:"
        "para 1\n\n"
        "[break]"
        "para 2\n\n"
        "para 3\n\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "para 1");
}

/** "break:" format. */
AFL_TEST("server.talk.render.Render:break", a)
{
    Environment env;
    env.opts.setFormat("break:text");

    const char*const TEXT = "forum:"
        "para 1\n\n"
        "para 2\n\n"
        "para 3\n\n"
        "para 4\n\n"
        "[break]"
        "para 5\n\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "para 1 para 2 para 3 para 4");
}

/** "quote:" format. */
AFL_TEST("server.talk.render.Render:quote", a)
{
    Environment env;
    env.opts.setFormat("quote:forum");

    const char*const TEXT = "forum:hello";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "[quote]\nhello[/quote]");
}

/** "noquote:" format. */
AFL_TEST("server.talk.render.Render:noquote", a)
{
    Environment env;
    env.opts.setFormat("noquote:forum");

    const char*const TEXT = "forum:[quote]hello[/quote]world";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "world");
}

/** "text:" input. */
AFL_TEST("server.talk.render.Render:input:text", a)
{
    Environment env;
    env.opts.setFormat("html");

    const char*const TEXT = "text:"
        "para 1\r\n"
        "para 2\n";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "<p>para 1</p>\n<p>para 2</p>\n");
}

/** "code:" input. */
AFL_TEST("server.talk.render.Render:input:code", a)
{
    Environment env;
    env.opts.setFormat("html");

    const char*const TEXT = "code:c:int main()";

    a.checkEqual("", renderText(TEXT, env.ctx, env.opts, env.root), "<pre><span class=\"syn-kw\">int</span> main()</pre>\n");
}

/* renderCheck, forum input, produces warning */
AFL_TEST("server.talk.render.Render:check:forum:warn", a)
{
    Environment env;
    std::vector<TalkRender::Warning> ws;

    renderCheck("forumABC:hello [b]world", env.ctx, env.root, ws);

    a.checkEqual("01. size", ws.size(), 1U);
    a.checkEqual("02. type", ws[0].type, "MissingClose");
    a.checkEqual("03. extra", ws[0].extra, "b");
}

/* renderCheck, forum input, no warning */
AFL_TEST("server.talk.render.Render:check:forum:ok", a)
{
    Environment env;
    std::vector<TalkRender::Warning> ws;

    renderCheck("forumABC:hello [b]world[/b]", env.ctx, env.root, ws);

    a.checkEqual("01. size", ws.size(), 0U);
}

/* renderCheck, unsupported */
AFL_TEST("server.talk.render.Render:check:other", a)
{
    Environment env;
    std::vector<TalkRender::Warning> ws;

    renderCheck("other:foobar", env.ctx, env.root, ws);

    a.checkEqual("01. size", ws.size(), 1U);
    a.checkEqual("02. type", ws[0].type, "Unsupported");
}
