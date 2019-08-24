/**
  *  \file server/interface/usermanagementclient.cpp
  *  \brief Class server::interface::UserManagementClient
  */

#include "server/interface/usermanagementclient.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;

server::interface::UserManagementClient::UserManagementClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

String_t
server::interface::UserManagementClient::add(String_t userName, String_t password, afl::base::Memory<const String_t> config)
{
    Segment seg;
    seg.pushBackString("ADDUSER");
    seg.pushBackString(userName);
    seg.pushBackString(password);
    while (const String_t* p = config.eat()) {
        seg.pushBackString(*p);
    }
    return m_commandHandler.callString(seg);
}

void
server::interface::UserManagementClient::remove(String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("DELUSER").pushBackString(userId));
}

String_t
server::interface::UserManagementClient::login(String_t userName, String_t password)
{
    return m_commandHandler.callString(Segment().pushBackString("LOGIN").pushBackString(userName).pushBackString(password));
}

String_t
server::interface::UserManagementClient::getUserIdByName(String_t userName)
{
    return m_commandHandler.callString(Segment().pushBackString("LOOKUP").pushBackString(userName));
}

String_t
server::interface::UserManagementClient::getNameByUserId(String_t userId)
{
    return m_commandHandler.callString(Segment().pushBackString("NAME").pushBackString(userId));
}

void
server::interface::UserManagementClient::getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames)
{
    Segment seg;
    seg.pushBackString("MNAME");
    while (const String_t* p = userIds.eat()) {
        seg.pushBackString(*p);
    }

    std::auto_ptr<Value_t> result(m_commandHandler.call(seg));
    afl::data::Access(result).toStringList(userNames);
}

server::Value_t*
server::interface::UserManagementClient::getProfileRaw(String_t userId, String_t key)
{
    return m_commandHandler.call(Segment().pushBackString("GET").pushBackString(userId).pushBackString(key));
}

server::Value_t*
server::interface::UserManagementClient::getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys)
{
    Segment seg;
    seg.pushBackString("MGET");
    seg.pushBackString(userId);
    while (const String_t* p = keys.eat()) {
        seg.pushBackString(*p);
    }
    return m_commandHandler.call(seg);
}

void
server::interface::UserManagementClient::setProfile(String_t userId, afl::base::Memory<const String_t> config)
{
    Segment seg;
    seg.pushBackString("SET");
    seg.pushBackString(userId);
    while (const String_t* p = config.eat()) {
        seg.pushBackString(*p);
    }
    m_commandHandler.callVoid(seg);
}

void
server::interface::UserManagementClient::setPassword(String_t userId, String_t password)
{
    m_commandHandler.callVoid(Segment().pushBackString("PASSWD").pushBackString(userId).pushBackString(password));
}
