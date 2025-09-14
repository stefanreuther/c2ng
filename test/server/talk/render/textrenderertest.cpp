/**
  *  \file test/server/talk/render/textrenderertest.cpp
  *  \brief Test for server::talk::render::TextRenderer
  */

#include "server/talk/render/textrenderer.hpp"

#include "afl/test/testrunner.hpp"

using server::talk::render::renderPlainText;
using server::talk::TextNode;

namespace {
    class TestLinkParser : public server::talk::LinkParser {
     public:
        virtual afl::base::Optional<Result_t> parseGameLink(String_t text) const
            { if (text == "bad_game") { return afl::base::Nothing; } else { return Result_t(1, "g"); } }
        virtual afl::base::Optional<Result_t> parseForumLink(String_t text) const
            { if (text == "bad_forum") { return afl::base::Nothing; } else { return Result_t(1, "f"); } }
        virtual afl::base::Optional<Result_t> parseTopicLink(String_t text) const
            { if (text == "bad_topic") { return afl::base::Nothing; } else { return Result_t(1, "t"); } }
        virtual afl::base::Optional<Result_t> parseMessageLink(String_t text) const
            {
                if (text == "bad_message") {
                    return afl::base::Nothing;
                } else if (text == "long_message") {
                    return Result_t(1, "This is a very long subject that will be abbreviated in output");
                } else if (text == "empty_message") {
                    return Result_t(1, "");
                } else {
                    return Result_t(1, "m");
                }
            }
        virtual afl::base::Optional<String_t> parseUserLink(String_t text) const
            { if (text == "bad_user") { return afl::base::Nothing; } else { return String_t("u"); } }
    };
}

/** Render plaintext. */
AFL_TEST("server.talk.render.TextRenderer:plaintext", a)
{
    // Environment
    TestLinkParser lp;

    // A single paragraph containing just text
    TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
    tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal))
        ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi mom"));

    // Basic test
    a.checkEqual("01", renderPlainText(tn, lp), "hi mom");
}

/** Render some regular text. */
AFL_TEST("server.talk.render.TextRenderer:complex", a)
{
    // Environment
    TestLinkParser lp;

    // Two paragraphs
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi"));

        TextNode& par2(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par2.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));

        a.checkEqual("01", renderPlainText(tn, lp), "hi mom");
    }

    // Paragraph with inline formatting (bold)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "hi "));
        par.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "mom"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "!"));

        a.checkEqual("11", renderPlainText(tn, lp), "hi mom!");
    }
}

/** Test rendering of links. */
AFL_TEST("server.talk.render.TextRenderer:link", a)
{
    // Environment
    TestLinkParser lp;

    // A link with differing content and target
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.back()->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("01", renderPlainText(tn, lp), "before text after");
    }

    // A link with no content (=shortened form)
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUrl, "http://web"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("11", renderPlainText(tn, lp), "before http://web after");
    }

    // Smiley
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "before "));
        par.children.pushBackNew(new TextNode(TextNode::maSpecial, TextNode::miSpecialSmiley, "smile"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " after"));

        a.checkEqual("21", renderPlainText(tn, lp), "before  after");
    }
}

/** Test rendering user links.
    User links are not expanded through LinkParser. */
AFL_TEST("server.talk.render.TextRenderer:link:user", a)
{
    // Environment
    TestLinkParser lp;

    // Regular user link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("01", renderPlainText(tn, lp), "[ wilma ]");
    }

    // Named user link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "wilma"))
            ->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "Text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("02", renderPlainText(tn, lp), "[ Text ]");
    }

    // Unknown user
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkUser, "bad_user"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("03", renderPlainText(tn, lp), "[ bad_user ]");
    }

    // Email link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkEmail, "a@b.c"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("04", renderPlainText(tn, lp), "[ a@b.c ]");
    }
}

/** Test more links. */
AFL_TEST("server.talk.render.TextRenderer:link:other", a)
{
    // Environment
    TestLinkParser lp;

    // Forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "good_forum"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("01", renderPlainText(tn, lp), "[ f ]");
    }

    // Bad forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "bad_forum"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("02", renderPlainText(tn, lp), "[ bad_forum ]");
    }

    // Named forum link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        TextNode& link(*par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkForum, "3")));
        link.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "text"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("03", renderPlainText(tn, lp), "[ text ]");
    }

    // Thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "good_topic"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("11", renderPlainText(tn, lp), "[ t ]");
    }

    // Bad thread link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkThread, "bad_topic"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("12", renderPlainText(tn, lp), "[ bad_topic ]");
    }

    // Post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "good_message"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("21", renderPlainText(tn, lp), "[ m ]");
    }

    // Bad post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "bad_message"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("22", renderPlainText(tn, lp), "[ bad_message ]");
    }

    // Abbreviated post link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "long_message"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("23", renderPlainText(tn, lp), "[ This is a very long subject... ]");
    }

    // Post link without subject
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkPost, "empty_message"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("23", renderPlainText(tn, lp), "[ (no subject) ]");
    }

    // Game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "good_game"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("31", renderPlainText(tn, lp), "[ g ]");
    }

    // Bad game link
    {
        TextNode tn(TextNode::maGroup, TextNode::miGroupRoot);
        TextNode& par(*tn.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal)));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "[ "));
        par.children.pushBackNew(new TextNode(TextNode::maLink, TextNode::miLinkGame, "bad_game"));
        par.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " ]"));

        a.checkEqual("42", renderPlainText(tn, lp), "[ bad_game ]");
    }
}
