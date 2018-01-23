/**
  *  \file server/interface/hostturnclient.cpp
  */

#include <utility>
#include "server/interface/hostturnclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/types.hpp"

server::interface::HostTurnClient::HostTurnClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

// TRN content:Blob [GAME game:GID] [SLOT slot:Int] [MAIL email:Str] [INFO info:Str]
server::interface::HostTurn::Result
server::interface::HostTurnClient::submit(const String_t& blob,
                                          afl::base::Optional<int32_t> game,
                                          afl::base::Optional<int32_t> slot,
                                          afl::base::Optional<String_t> mail,
                                          afl::base::Optional<String_t> info)
{
    // Build command
    afl::data::Segment cmd;
    cmd.pushBackString("TRN");
    cmd.pushBackString(blob);
    if (const int32_t* p = game.get()) {
        cmd.pushBackString("GAME");
        cmd.pushBackInteger(*p);
    }
    if (const int32_t* p = slot.get()) {
        cmd.pushBackString("SLOT");
        cmd.pushBackInteger(*p);
    }
    if (const String_t* p = mail.get()) {
        cmd.pushBackString("MAIL");
        cmd.pushBackString(*p);
    }
    if (const String_t* p = info.get()) {
        cmd.pushBackString("INFO");
        cmd.pushBackString(*p);
    }

    // Submit
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);

    // Parse result
    Result r;
    r.state = a("status").toInteger();
    r.output = a("output").toString();
    r.gameId = a("game").toInteger();
    r.slot = a("slot").toInteger();
    r.previousState = a("previous").toInteger();
    r.userId = a("user").toString();
    return r;
}

// TRNMARKTEMP game:GID slot:Int flag:Int
void
server::interface::HostTurnClient::setTemporary(int32_t gameId, int32_t slot, bool flag)
{
    m_commandHandler.callVoid(afl::data::Segment().pushBackString("TRNMARKTEMP").pushBackInteger(gameId).pushBackInteger(slot).pushBackInteger(flag));
}
