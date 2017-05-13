/**
  *  \file server/talk/render/context.cpp
  */

#include "server/talk/render/context.hpp"

server::talk::render::Context::Context(String_t user)
    : m_user(user),
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
