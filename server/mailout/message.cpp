/**
  *  \file server/mailout/message.cpp
  *  \brief Class server::mailout::Message
  */

#include "server/mailout/message.hpp"

// Constructor.
server::mailout::Message::Message(afl::net::redis::Subtree root, int32_t mid, String_t state)
    : m_root(root),
      m_message(root.subtree("msg").subtree(mid)),
      m_messageId(mid),
      m_state(state)
{
    // ex Message::Message
}

// Access template name.
afl::net::redis::StringField
server::mailout::Message::templateName()
{
    // ex Message::templateName
    return m_message.hashKey("data").stringField("template");
}

// Access message unique identifier.
afl::net::redis::StringField
server::mailout::Message::uniqueId()
{
    // ex Message::uniqueId
    return m_message.hashKey("data").stringField("uniqid");
}

// Access message parameter hash.
afl::net::redis::HashKey
server::mailout::Message::arguments()
{
    // ex Message::arguments
    return m_message.hashKey("args");
}

// Access message attachment list.
afl::net::redis::StringListKey
server::mailout::Message::attachments()
{
    // ex Message::attachments
    return m_message.stringListKey("attach");
}

// Access message receiver set.
afl::net::redis::StringSetKey
server::mailout::Message::receivers()
{
    // ex Message::receivers
    return m_message.stringSetKey("to");
}

// Access message expiration time.
afl::net::redis::IntegerField
server::mailout::Message::expireTime()
{
    // ex Message::expireTime
    return m_message.hashKey("data").intField("expire");
}

// Remove message from database.
void
server::mailout::Message::remove()
{
    // ex Message::remove
    m_message.hashKey("data").remove();
    arguments().remove();
    attachments().remove();
    receivers().remove();
    m_root.intSetKey(m_state).remove(m_messageId);
}

// Prepare message for sending.
void
server::mailout::Message::send()
{
    // ex Message::send
    String_t uniqid = uniqueId().get();
    if (!uniqid.empty()) {
        m_root.hashKey("uniqid").intField(uniqid).set(m_messageId);
    }
    if (m_state != "sending") {
        m_root.intSetKey(m_state).moveTo(m_messageId, m_root.intSetKey("sending"));
        m_state = "sending";
    }
}

// Get message Id.
int32_t
server::mailout::Message::getId() const
{
    return m_messageId;
}
