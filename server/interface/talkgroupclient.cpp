/**
  *  \file server/interface/talkgroupclient.cpp
  */

#include <memory>
#include "server/interface/talkgroupclient.hpp"
#include "server/types.hpp"
#include "afl/data/access.hpp"

server::interface::TalkGroupClient::TalkGroupClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkGroupClient::~TalkGroupClient()
{ }

void
server::interface::TalkGroupClient::add(String_t groupId, const Description& info)
{
    afl::data::Segment command;
    command.pushBackString("GROUPADD");
    command.pushBackString(groupId);
    packDescription(command, info);
    m_commandHandler.callVoid(command);
}

void
server::interface::TalkGroupClient::set(String_t groupId, const Description& info)
{
    afl::data::Segment command;
    command.pushBackString("GROUPSET");
    command.pushBackString(groupId);
    packDescription(command, info);
    m_commandHandler.callVoid(command);
}

String_t
server::interface::TalkGroupClient::getField(String_t groupId, String_t fieldName)
{
    afl::data::Segment command;
    command.pushBackString("GROUPGET");
    command.pushBackString(groupId);
    command.pushBackString(fieldName);

    return m_commandHandler.callString(command);
}

void
server::interface::TalkGroupClient::list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums)
{
    afl::data::Segment command;
    command.pushBackString("GROUPLS");
    command.pushBackString(groupId);

    std::auto_ptr<afl::data::Value> result(m_commandHandler.call(command));
    afl::data::Access a(result);
    a("groups").toStringList(groups);
    a("forums").toIntegerList(forums);
}

server::interface::TalkGroup::Description
server::interface::TalkGroupClient::getDescription(String_t groupId)
{
    afl::data::Segment command;
    command.pushBackString("GROUPSTAT");
    command.pushBackString(groupId);

    std::auto_ptr<afl::data::Value> result(m_commandHandler.call(command));
    return unpackDescription(result.get());
}

void
server::interface::TalkGroupClient::getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results)
{
    afl::data::Segment command;
    command.pushBackString("GROUPMSTAT");
    command.pushBackElements(groups);

    std::auto_ptr<afl::data::Value> result(m_commandHandler.call(command));
    afl::data::Access a(result);
    for (size_t i = 0, n = groups.size(); i < n; ++i) {
        if (a[i].isNull()) {
            results.pushBackNew(0);
        } else {
            results.pushBackNew(new Description(unpackDescription(a[i].getValue())));
        }
    }
}

void
server::interface::TalkGroupClient::packDescription(afl::data::Segment& command, const Description& info)
{
    if (const String_t* p = info.name.get()) {
        command.pushBackString("name");
        command.pushBackString(*p);
    }
    if (const String_t* p = info.description.get()) {
        command.pushBackString("description");
        command.pushBackString(*p);
    }
    if (const String_t* p = info.parentGroup.get()) {
        command.pushBackString("parent");
        command.pushBackString(*p);
    }
    if (const String_t* p = info.key.get()) {
        command.pushBackString("key");
        command.pushBackString(*p);
    }
    if (const bool* p = info.unlisted.get()) {
        command.pushBackString("unlisted");
        command.pushBackInteger(*p);
    }
}

server::interface::TalkGroup::Description
server::interface::TalkGroupClient::unpackDescription(const Value_t* value)
{
    Description result;
    afl::data::Access a(value);
    result.name = toOptionalString(a("name").getValue());
    result.description = toOptionalString(a("description").getValue());
    result.parentGroup = toOptionalString(a("parent").getValue());
    result.key = toOptionalString(a("key").getValue());

    if (const Value_t* p = a("unlisted").getValue()) {
        result.unlisted = toInteger(p) != 0;
    }
    return result;
}
