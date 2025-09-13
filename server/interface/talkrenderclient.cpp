/**
  *  \file server/interface/talkrenderclient.cpp
  *  \brief Class server::interface::TalkRenderClient
  */

#include <memory>
#include "server/interface/talkrenderclient.hpp"
#include "server/types.hpp"

server::interface::TalkRenderClient::TalkRenderClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkRenderClient::~TalkRenderClient()
{ }

void
server::interface::TalkRenderClient::setOptions(const Options& opts)
{
    afl::data::Segment command;
    command.pushBackString("RENDEROPTION");
    packOptions(command, opts);
    m_commandHandler.callVoid(command);
}

String_t
server::interface::TalkRenderClient::render(const String_t& text, const Options& opts)
{
    afl::data::Segment command;
    command.pushBackString("RENDER");
    command.pushBackString(text);
    packOptions(command, opts);

    return m_commandHandler.callString(command);
}

void
server::interface::TalkRenderClient::check(const String_t& text, std::vector<Warning>& out)
{
    afl::data::Segment command;
    command.pushBackString("RENDERCHECK");
    command.pushBackString(text);

    std::auto_ptr<Value_t> p(m_commandHandler.call(command));
    afl::data::Access a(p.get());
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        out.push_back(unpackWarning(a[i]));
    }
}

void
server::interface::TalkRenderClient::packOptions(afl::data::Segment& command, const Options& opts)
{
    if (const String_t* p = opts.baseUrl.get()) {
        command.pushBackString("BASEURL");
        command.pushBackString(*p);
    }
    if (const String_t* p = opts.format.get()) {
        command.pushBackString("FORMAT");
        command.pushBackString(*p);
    }
}

server::interface::TalkRender::Warning
server::interface::TalkRenderClient::unpackWarning(afl::data::Access a)
{
    Warning w;
    w.type  = a("type").toString();
    w.token = a("token").toString();
    w.extra = a("extra").toString();
    w.pos   = a("pos").toInteger();
    return w;
}
