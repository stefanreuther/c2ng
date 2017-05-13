/**
  *  \file server/talk/accesschecker.cpp
  */

#include <stdexcept>
#include "server/talk/accesschecker.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/message.hpp"
#include "server/talk/session.hpp"
#include "server/talk/forum.hpp"
#include "server/errors.hpp"

server::talk::AccessChecker::AccessChecker(Root& root, Session& session)
    : m_root(root),
      m_session(session),
      m_lastTopicId(0),
      m_lastTopicPermitted(false)
{
    // ex PostAccessChecker::PostAccessChecker
}

bool
server::talk::AccessChecker::isAllowed(Message& m)
{
    // ex PostAccessChecker::isAllowed
    Topic t(m.topic(m_root));
    return isAllowed(t) || m.author().get() == m_session.getUser();
}

bool
server::talk::AccessChecker::isAllowed(Topic& t)
{
    // ex PostAccessChecker::isAllowed
    if (t.getId() != m_lastTopicId) {
        m_lastTopicId = t.getId();
        m_lastTopicPermitted = false;

        String_t readPermissions = t.readPermissions().get();
        if (readPermissions.empty()) {
            readPermissions = t.forum(m_root).readPermissions().get();
        }
        m_lastTopicPermitted = m_session.hasPrivilege(readPermissions, m_root);
    }
    return m_lastTopicPermitted;
}

void
server::talk::AccessChecker::checkMessage(Message& m)
{
    // ex PostAccessChecker::checkMessage
    if (!isAllowed(m)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}

void
server::talk::AccessChecker::checkTopic(Topic& t)
{
    // ex PostAccessChecker::checkTopic
    if (!isAllowed(t)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}
