/**
  *  \file server/test/mailmock.cpp
  *  \brief Class server::test::MailMock
  */

#include "server/test/mailmock.hpp"

server::test::MailMock::MailMock(afl::test::Assert a)
    : m_assert(a), m_current(), m_queue()
{ }

void
server::test::MailMock::startMessage(String_t templateName, afl::base::Optional<String_t> /*uniqueId*/)
{
    m_assert.checkNull("startMessage > m_current", m_current.get());
    m_current.reset(new Message());
    m_current->templateName = templateName;
}

void
server::test::MailMock::addParameter(String_t parameterName, String_t value)
{
    m_assert.checkNonNull("addParameter > m_current", m_current.get());
    m_assert.check("addParameter > new parameter", m_current->parameters.find(parameterName) == m_current->parameters.end());
    m_current->parameters.insert(std::make_pair(parameterName, value));
}

void
server::test::MailMock::addAttachment(String_t url)
{
    m_assert.checkNonNull("addAttachment > m_current", m_current.get());
    m_current->attachments.insert(url);
}

void
server::test::MailMock::send(afl::base::Memory<const String_t> receivers)
{
    m_assert.checkNonNull("send > m_current", m_current.get());
    while (const String_t* p = receivers.eat()) {
        m_current->receivers.insert(*p);
    }
    m_queue.pushBackNew(m_current.release());
}

void
server::test::MailMock::cancelMessage(String_t /*uniqueId*/)
{
    // Ignore this
}

void
server::test::MailMock::confirmAddress(String_t /*address*/, String_t /*key*/, afl::base::Optional<String_t> /*info*/)
{
    m_assert.fail("confirmAddress unexpected");
}

void
server::test::MailMock::requestAddress(String_t /*user*/)
{
    m_assert.fail("requestAddress unexpected");
}

void
server::test::MailMock::runQueue()
{
    m_assert.fail("runQueue unexpected");
}

server::interface::MailQueue::UserStatus
server::test::MailMock::getUserStatus(String_t /*user*/)
{
    m_assert.fail("getUserStatus unexpected");
    return UserStatus();
}

server::test::MailMock::Message*
server::test::MailMock::extract(String_t receiver)
{
    for (size_t i = 0; i < m_queue.size(); ++i) {
        Message* p = m_queue[i];
        std::set<String_t>::iterator it = p->receivers.find(receiver);
        if (it != p->receivers.end()) {
            p->receivers.erase(it);
            return p;
        }
    }
    return 0;
}

std::auto_ptr<server::test::MailMock::Message>
server::test::MailMock::extractFirst()
{
    std::auto_ptr<Message> p;
    if (!m_queue.empty()) {
        p.reset(m_queue.extractElement(0));
        m_queue.erase(m_queue.begin());
    }
    return p;
}

bool
server::test::MailMock::empty() const
{
    for (size_t i = 0; i < m_queue.size(); ++i) {
        if (!m_queue[i]->receivers.empty()) {
            return false;
        }
    }
    return true;
}
