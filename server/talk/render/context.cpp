/**
  *  \file server/talk/render/context.cpp
  *  \brief Class server::talk::render::Context
  */

#include "server/talk/render/context.hpp"
#include "afl/net/redis/field.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/topic.hpp"

server::talk::render::Context::Context(Root& root, String_t user)
    : m_root(root),
      m_user(user),
      m_messageId(0),
      m_messageAuthor()
{ }

void
server::talk::render::Context::setMessageId(int32_t id)
{
    // ex RenderState::setMessageId
    m_messageId = id;
    m_messageAuthor.clear();
}

void
server::talk::render::Context::setMessageAuthor(String_t author)
{
    // ex RenderState::setMessageAuthor
    m_messageId = 0;
    m_messageAuthor = author;
}

const String_t&
server::talk::render::Context::getUser() const
{
    // ex RenderState::getUser
    return m_user;
}

int32_t
server::talk::render::Context::getMessageId() const
{
    // ex RenderState::getMessageId
    return m_messageId;
}

const String_t&
server::talk::render::Context::getMessageAuthor() const
{
    // ex RenderState::getMessageAuthor
    return m_messageAuthor;
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::render::Context::parseGameLink(String_t text) const
{
    int32_t gameId;
    if (!afl::string::strToInteger(text, gameId) || gameId <= 0) {
        return afl::base::Nothing;
    }

    // Access game, check permissions
    // xref host/game.cc, Game::hasPermission
    afl::net::redis::Subtree root(m_root.gameRoot());
    if (!root.intSetKey("all").contains(gameId)) {
        return afl::base::Nothing;
    }

    afl::net::redis::Subtree game(root.subtree(gameId));
    const String_t gameState = game.stringKey("state").get();
    if (gameState != "joining" && gameState != "running" && gameState != "finished") {
        return afl::base::Nothing;
    }

    const String_t gameType = game.stringKey("type").get();
    if (gameType != "unlisted"
        && gameType != "public"
        && game.stringKey("owner").get() != m_user
        && !game.hashKey("users").field(m_user).exists())
    {
        return afl::base::Nothing;
    }

    return Result_t(gameId, game.stringKey("name").get());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::render::Context::parseForumLink(String_t text) const
{
    int32_t forumId;
    if (!afl::string::strToInteger(text, forumId) || forumId <= 0) {
        return afl::base::Nothing;
    }

    // Access forum, check permissions
    server::talk::Forum forum(m_root, forumId);
    if (!m_root.allForums().contains(forumId)) {
        // FIXME: turn this into a method of Forum?
        // FIXME permission check!
        return afl::base::Nothing;
    }

    return Result_t(forumId, forum.name().get());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::render::Context::parseTopicLink(String_t text) const
{
    int32_t topicId;
    if (!afl::string::strToInteger(text, topicId) || topicId <= 0) {
        return afl::base::Nothing;
    }

    // Access thread, check permissions
    server::talk::Topic t(m_root, topicId);
    if (!t.exists()) {
        // FIXME permission check!
        return afl::base::Nothing;
    }

    return Result_t(topicId, t.subject().get());
}

afl::base::Optional<server::talk::LinkParser::Result_t>
server::talk::render::Context::parseMessageLink(String_t text) const
{
    int32_t messageId;
    if (!afl::string::strToInteger(text, messageId) || messageId <= 0) {
        return afl::base::Nothing;
    }

    // Access thread, check permissions
    server::talk::Message m(m_root, messageId);
    if (!m.exists()) {
        // FIXME permission check!
        return afl::base::Nothing;
    }
    return Result_t(messageId, m.subject().get());
}

afl::base::Optional<String_t>
server::talk::render::Context::parseUserLink(String_t text) const
{
    String_t userId = m_root.getUserIdFromLogin(text);
    if (userId.empty()) {
        return afl::base::Nothing;
    }

    return userId;
}
