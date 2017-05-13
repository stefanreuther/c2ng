/**
  *  \file server/talk/render/textrenderer.cpp
  */

#include "server/talk/render/textrenderer.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/message.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/topic.hpp"

namespace {
    using server::talk::render::Context;
    using server::talk::Root;

    String_t hackSubject(const String_t s)
    {
        if (s.empty()) {
            return "(no subject)";
        } else if (s.size() > 30) {
            size_t i = 28;
            while (i > 0 && (s[i-1] == ' ' || afl::charset::Utf8::isContinuationByte(s[i]))) {
                // Yes, this is intended to check i-1 for spaces, and i for continuation:
                // - if [i-1] is a space, remove it
                // - if [i] is a continuation, we must at least remove [i-1] to not leave a partial rune
                --i;
            }
            return String_t(s, 0, i) + "...";
        } else {
            return s;
        }
    }

    String_t makeGameName(const String_t& text, const Context& ctx, Root& root)
    {
        // Parse game Id
        int32_t gameId;
        if (!afl::string::strToInteger(text, gameId) || gameId <= 0) {
            return text;
        }

        // Access game, check permissions
        // FIXME xref host/game.cc, Game::hasPermission
        afl::net::redis::Subtree gameRoot(root.gameRoot());
        if (!gameRoot.intSetKey("all").contains(gameId)) {
            return text;
        }

        afl::net::redis::Subtree game(gameRoot.subtree(gameId));
        const String_t gameState = game.stringKey("state").get();
        if (gameState != "joining" && gameState != "running" && gameState != "finished") {
            return text;
        }

        const String_t gameType = game.stringKey("type").get();
        if (gameType != "unlisted"
            && gameType != "public"
            && game.stringKey("owner").get() != ctx.getUser()
            && !game.hashKey("users").field(ctx.getUser()).exists())
        {
            return text;
        }

        // OK, we are allowed to access it. Get its name.
        return game.stringKey("name").get();
    }

    String_t makeForumName(const String_t& text, const Context& /*ctx*/, Root& root)
    {
        // Parse forum Id
        int32_t forumId;
        if (!afl::string::strToInteger(text, forumId) || forumId <= 0) {
            return text;
        }

        // Access forum, check permissions
        // FIXME: access checks?
        server::talk::Forum forum(root, forumId);
        if (!root.allForums().contains(forumId)) {
            return text;
        }

        // OK, we are allowed to access it. Get its name.
        return forum.name().get();
    }

    String_t makePostName(String_t& text, const Context& /*ctx*/, Root& root)
    {
        // Validate
        int32_t messageId;
        if (!afl::string::strToInteger(text, messageId) || messageId <= 0) {
            return text;
        }

        // Access thread, check permissions
        server::talk::Message m(root, messageId);
        if (!m.exists()) {
            // FIXME permission check!
            return text;
        }

        // OK, we are allowed to access it. Get its name.
        return hackSubject(m.subject().get());
    }

    String_t makeThreadName(const String_t& text, const Context& /*ctx*/, Root& root)
    {
        // Validate
        int32_t topicId;
        if (!afl::string::strToInteger(text, topicId) || topicId <= 0) {
            return text;
        }

        // Access thread, check permissions
        server::talk::Topic t(root, topicId);
        if (!t.exists()) {
            // FIXME permission check!
            return text;
        }

        // OK, we are allowed to access it. Get its name.
        return hackSubject(t.subject().get());
    }

}

/** Render node as plaintext.
    This is a version of TextNode::rawTextContent(),
    but it fills in game names etc. */
String_t
server::talk::render::renderText(TextNode* node, const Context& ctx, Root& root)
{
    // FIXME: explain 10000?
    if (node->major == TextNode::maPlain) {
        return node->text;
    } else {
        String_t result;
        for (size_t i = 0, n = node->children.size(); i < n && result.size() < 10000; ++i) {
            result += renderText(node->children[i], ctx, root);
        }
        if (result.empty() && node->major == TextNode::maLink) {
            switch (TextNode::LinkFormat(node->minor)) {
             case TextNode::miLinkUrl:
             case TextNode::miLinkEmail:
             case TextNode::miLinkUser:
                // Fallback is using the text
                result = node->text;
                break;

             case TextNode::miLinkThread:
                result = makeThreadName(node->text, ctx, root);
                break;

             case TextNode::miLinkPost:
                result = makePostName(node->text, ctx, root);
                break;

             case TextNode::miLinkGame:
                result = makeGameName(node->text, ctx, root);
                break;

             case TextNode::miLinkForum:
                result = makeForumName(node->text, ctx, root);
                break;
            }
        }
        return result;
    }
}
