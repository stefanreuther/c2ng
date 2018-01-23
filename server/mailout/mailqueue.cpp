/**
  *  \file server/mailout/mailqueue.cpp
  *  \brief Class server::mailout::MailQueue
  */

#include <memory>
#include "server/mailout/mailqueue.hpp"
#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "server/mailout/message.hpp"
#include "server/mailout/root.hpp"
#include "server/mailout/session.hpp"
#include "server/mailout/transmitter.hpp"

using afl::string::Format;

namespace {
    const char*const LOG_NAME = "mailout.command";
}

server::mailout::MailQueue::MailQueue(Root& root, Session& session)
    : m_root(root),
      m_session(session)
{ }

void
server::mailout::MailQueue::startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId)
{
    // ex MailoutConnection::doMailNew
    if (m_session.currentMessage.get() != 0) {
        throw std::runtime_error(SEQUENCE_ERROR);
    }

    // Work
    m_session.currentMessage = m_root.allocateMessage();
    m_session.currentMessage->templateName().set(templateName);
    if (const String_t* p = uniqueId.get()) {
        m_session.currentMessage->uniqueId().set(*p);
    }
}

void
server::mailout::MailQueue::addParameter(String_t parameterName, String_t value)
{
    // ex MailoutConnection::doMailParam
    currentMessage().arguments().stringField(parameterName).set(value);
}

void
server::mailout::MailQueue::addAttachment(String_t url)
{
    // ex MailoutConnection::doMailAttach
    currentMessage().attachments().pushBack(url);
}

void
server::mailout::MailQueue::send(afl::base::Memory<const String_t> receivers)
{
    // ex MailoutConnection::doMailSend
    Message& m = currentMessage();
    while (const String_t* p = receivers.eat()) {
        m.receivers().add(*p);
    }

    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[msg:%d] queued", m.getId()));
    m.send();
    if (Transmitter* p = m_root.getTransmitter()) {
        p->send(m.getId());
    }

    // Reset state
    m_session.currentMessage.reset();
}

void
server::mailout::MailQueue::cancelMessage(String_t uniqueId)
{
    // ex MailoutConnection::doCancel
    m_root.mailRoot().hashKey("uniqid").field(uniqueId).remove();
}

void
server::mailout::MailQueue::confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info)
{
    // ex MailoutConnection::doConfirm
    // Check
    if (m_root.confirmMail(address, key, info.orElse(""))) {
        if (Transmitter* p = m_root.getTransmitter()) {
            p->notifyAddress(address);
        }
    } else {
        throw std::runtime_error("401 Authentication error");
    }
}

void
server::mailout::MailQueue::requestAddress(String_t user)
{
    // ex MailoutConnection::doRequest, Transmitter::maybeRequestConfirmation
    String_t authUser;
    String_t smtpAddress;
    try {
        // Try to resolve the user name into an address.
        // As a byproduct, this will send a confirmation request if needed.
        // A hard failure is reported as exception, which is ignored; the CONFIRM command does not fail.
        m_root.resolveAddress("user:" + user, smtpAddress, authUser);
    }
    catch (...) { }
}

void
server::mailout::MailQueue::runQueue()
{
    if (Transmitter* p = m_root.getTransmitter()) {
        p->runQueue();
    }
}

server::mailout::Message&
server::mailout::MailQueue::currentMessage()
{
    if (m_session.currentMessage.get() == 0) {
        throw std::runtime_error(SEQUENCE_ERROR);
    }
    return *m_session.currentMessage;
}
