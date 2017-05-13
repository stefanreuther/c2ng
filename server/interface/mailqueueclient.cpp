/**
  *  \file server/interface/mailqueueclient.cpp
  */

#include "server/interface/mailqueueclient.hpp"
#include "afl/data/segment.hpp"

using afl::data::Segment;

server::interface::MailQueueClient::MailQueueClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::MailQueueClient::~MailQueueClient()
{ }

void
server::interface::MailQueueClient::startMessage(String_t templateName, afl::base::Optional<String_t> uniqueId)
{
    // ex MailClient::startMail
    Segment cmd;
    cmd.pushBackString("MAIL");
    cmd.pushBackString(templateName);
    if (const String_t* p = uniqueId.get()) {
        cmd.pushBackString(*p);
    }
    m_commandHandler.callVoid(cmd);
}

void
server::interface::MailQueueClient::addParameter(String_t parameterName, String_t value)
{
    // ex MailClient::addParameter
    m_commandHandler.callVoid(Segment().pushBackString("PARAM").pushBackString(parameterName).pushBackString(value));
}

void
server::interface::MailQueueClient::addAttachment(String_t url)
{
    // ex MailClient::attachFile
    m_commandHandler.callVoid(Segment().pushBackString("ATTACH").pushBackString(url));
}

void
server::interface::MailQueueClient::send(afl::base::Memory<const String_t> receivers)
{
    // ex MailClient::sendTo
    Segment cmd;
    cmd.pushBackString("SEND");
    while (const String_t* p = receivers.eat()) {
        cmd.pushBackString(*p);
    }
    m_commandHandler.callVoid(cmd);
}

void
server::interface::MailQueueClient::cancelMessage(String_t uniqueId)
{
    // ex MailClient::cancel
    m_commandHandler.callVoid(Segment().pushBackString("CANCEL").pushBackString(uniqueId));
}

void
server::interface::MailQueueClient::confirmAddress(String_t address, String_t key, afl::base::Optional<String_t> info)
{
    Segment cmd;
    cmd.pushBackString("CONFIRM");
    cmd.pushBackString(address);
    cmd.pushBackString(key);
    if (const String_t* p = info.get()) {
        cmd.pushBackString(*p);
    }
    m_commandHandler.callVoid(cmd);
}

void
server::interface::MailQueueClient::requestAddress(String_t user)
{
    m_commandHandler.callVoid(Segment().pushBackString("REQUEST").pushBackString(user));
}

void
server::interface::MailQueueClient::runQueue()
{
    m_commandHandler.callVoid(Segment().pushBackString("RUNQUEUE"));
}
