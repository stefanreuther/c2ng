/**
  *  \file server/interface/talkaddressclient.cpp
  */

#include <memory>
#include "server/interface/talkaddressclient.hpp"
#include "server/types.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;

server::interface::TalkAddressClient::TalkAddressClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkAddressClient::~TalkAddressClient()
{ }

void
server::interface::TalkAddressClient::parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
{
    Segment seg;
    seg.pushBackString("ADDRMPARSE");
    while (const String_t* p = in.eat()) {
        seg.pushBackString(*p);
    }

    std::auto_ptr<Value_t> p(m_commandHandler.call(seg));
    afl::data::Access(p).toStringList(out);
}

void
server::interface::TalkAddressClient::render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out)
{
    Segment seg;
    seg.pushBackString("ADDRMRENDER");
    while (const String_t* p = in.eat()) {
        seg.pushBackString(*p);
    }

    std::auto_ptr<Value_t> p(m_commandHandler.call(seg));
    afl::data::Access(p).toStringList(out);
}
