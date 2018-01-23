/**
  *  \file server/interface/hosttoolclient.cpp
  */

#include "server/interface/hosttoolclient.hpp"
#include "afl/data/access.hpp"

server::interface::HostToolClient::HostToolClient(afl::net::CommandHandler& commandHandler, Area area)
    : HostTool(),
      m_commandHandler(commandHandler),
      m_area(area)
{ }

server::interface::HostToolClient::~HostToolClient()
{ }

void
server::interface::HostToolClient::add(String_t id, String_t path, String_t program, String_t kind)
{
    afl::data::Segment command;
    addCommand(command, "ADD");
    command.pushBackString(id);
    command.pushBackString(path);
    command.pushBackString(program);
    command.pushBackString(kind);
    m_commandHandler.callVoid(command);
}

void
server::interface::HostToolClient::set(String_t id, String_t key, String_t value)
{
    afl::data::Segment command;
    addCommand(command, "SET");
    command.pushBackString(id);
    command.pushBackString(key);
    command.pushBackString(value);
    m_commandHandler.callVoid(command);
}

String_t
server::interface::HostToolClient::get(String_t id, String_t key)
{
    afl::data::Segment command;
    addCommand(command, "GET");
    command.pushBackString(id);
    command.pushBackString(key);
    return m_commandHandler.callString(command);
}

bool
server::interface::HostToolClient::remove(String_t id)
{
    afl::data::Segment command;
    addCommand(command, "RM");
    command.pushBackString(id);
    return m_commandHandler.callInt(command);
}

void
server::interface::HostToolClient::getAll(std::vector<Info>& result)
{
    afl::data::Segment command;
    addCommand(command, "LS");

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(command));
    afl::data::Access a(p);

    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(unpackInfo(a[i].getValue()));
    }
}

void
server::interface::HostToolClient::copy(String_t sourceId, String_t destinationId)
{
    afl::data::Segment command;
    addCommand(command, "CP");
    command.pushBackString(sourceId);
    command.pushBackString(destinationId);
    m_commandHandler.callVoid(command);
}

void
server::interface::HostToolClient::setDefault(String_t id)
{
    afl::data::Segment command;
    addCommand(command, "DEFAULT");
    command.pushBackString(id);
    m_commandHandler.callVoid(command);
}

int32_t
server::interface::HostToolClient::getDifficulty(String_t id)
{
    afl::data::Segment command;
    addCommand(command, "RATING");
    command.pushBackString(id);
    command.pushBackString("GET");
    return m_commandHandler.callInt(command);
}

void
server::interface::HostToolClient::clearDifficulty(String_t id)
{
    afl::data::Segment command;
    addCommand(command, "RATING");
    command.pushBackString(id);
    command.pushBackString("NONE");
    m_commandHandler.callVoid(command);
}

int32_t
server::interface::HostToolClient::setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use)
{
    afl::data::Segment command;
    addCommand(command, "RATING");
    command.pushBackString(id);
    if (const int32_t* p = value.get()) {
        command.pushBackString("SET");
        command.pushBackInteger(*p);
    } else {
        command.pushBackString("AUTO");
    }
    if (use) {
        command.pushBackString("USE");
    } else {
        command.pushBackString("SHOW");
    }
    return m_commandHandler.callInt(command);
}

void
server::interface::HostToolClient::addCommand(afl::data::Segment& seg, const char* suffix) const
{
    if (const char* p = toString(m_area)) {
        seg.pushBackString(String_t(p) + suffix);
    } else {
        // Cannot happen. Generate an invalid command if it does anyway.
        seg.pushBackString(String_t());
    }
}

server::interface::HostTool::Info
server::interface::HostToolClient::unpackInfo(const Value_t* p)
{
    afl::data::Access a(p);
    return Info(a("id").toString(),
                a("description").toString(),
                a("kind").toString(),
                a("default").toInteger());
}
