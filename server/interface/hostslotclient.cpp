/**
  *  \file server/interface/hostslotclient.cpp
  *  \brief Class server::interface::HostSlotClient
  */

#include <memory>
#include "server/interface/hostslotclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/types.hpp"

using afl::data::Segment;

namespace {
    void addAll(Segment& out, afl::base::Memory<const int32_t> slotNrs)
    {
        while (const int32_t* p = slotNrs.eat()) {
            out.pushBackInteger(*p);
        }
    }
}


server::interface::HostSlotClient::HostSlotClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

void
server::interface::HostSlotClient::add(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
{
    Segment seg;
    seg.pushBackString("SLOTADD").pushBackInteger(gameId);
    addAll(seg, slotNrs);
    m_commandHandler.callVoid(seg);
}

void
server::interface::HostSlotClient::remove(int32_t gameId, afl::base::Memory<const int32_t> slotNrs)
{
    Segment seg;
    seg.pushBackString("SLOTRM").pushBackInteger(gameId);
    addAll(seg, slotNrs);
    m_commandHandler.callVoid(seg);
}

void
server::interface::HostSlotClient::getAll(int32_t gameId, afl::data::IntegerList_t& result)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("SLOTLS").pushBackInteger(gameId)));
    afl::data::Access(p.get()).toIntegerList(result);
}
