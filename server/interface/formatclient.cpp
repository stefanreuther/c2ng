/**
  *  \file server/interface/formatclient.cpp
  */

#include "server/interface/formatclient.hpp"

server::interface::FormatClient::FormatClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::FormatClient::~FormatClient()
{ }

afl::data::Value*
server::interface::FormatClient::pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
{
    afl::data::Segment command;
    command.pushBackString("PACK");
    command.pushBackString(formatName);
    command.pushBack(data);     // FIXME: clones
    if (const String_t* f = format.get()) {
        command.pushBackString("FORMAT");
        command.pushBackString(*f);
    }
    if (const String_t* cs = charset.get()) {
        command.pushBackString("CHARSET");
        command.pushBackString(*cs);
    }
    return m_commandHandler.call(command);
}

afl::data::Value*
server::interface::FormatClient::unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
{
    afl::data::Segment command;
    command.pushBackString("UNPACK");
    command.pushBackString(formatName);
    command.pushBack(data);     // FIXME: clones
    if (const String_t* f = format.get()) {
        command.pushBackString("FORMAT");
        command.pushBackString(*f);
    }
    if (const String_t* cs = charset.get()) {
        command.pushBackString("CHARSET");
        command.pushBackString(*cs);
    }
    return m_commandHandler.call(command);
}
