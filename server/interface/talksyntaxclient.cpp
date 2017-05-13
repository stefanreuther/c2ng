/**
  *  \file server/interface/talksyntaxclient.cpp
  *  \brief Class server::interface::TalkSyntaxClient
  */

#include <memory>
#include "server/interface/talksyntaxclient.hpp"
#include "afl/data/access.hpp"
#include "server/types.hpp"

// Constructor.
server::interface::TalkSyntaxClient::TalkSyntaxClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

// Destructor.
server::interface::TalkSyntaxClient::~TalkSyntaxClient()
{ }

// Get single syntax entry.
String_t
server::interface::TalkSyntaxClient::get(String_t key)
{
    afl::data::Segment command;
    command.pushBackString("SYNTAXGET");
    command.pushBackString(key);
    return m_commandHandler.callString(command);
}

// Get multiple syntax entries.
afl::base::Ref<afl::data::Vector>
server::interface::TalkSyntaxClient::mget(afl::base::Memory<const String_t> keys)
{
    afl::data::Segment command;
    command.pushBackString("SYNTAXMGET");
    while (const String_t* p = keys.eat()) {
        command.pushBackString(*p);
    }

    std::auto_ptr<afl::data::Value> result(m_commandHandler.call(command));
    afl::data::Access a(result);

    afl::base::Ref<afl::data::Vector> v = afl::data::Vector::create();
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        afl::data::Access ele = a[i];
        if (ele.isNull()) {
            v->pushBackNew(0);
        } else {
            v->pushBackString(ele.toString());
        }
    }
    return v;
}
