/**
  *  \file server/interface/filegameclient.cpp
  *  \brief Class server::interface::FileGameClient
  */

#include <memory>
#include "server/interface/filegameclient.hpp"
#include "afl/data/access.hpp"

using afl::data::Segment;
using afl::data::Access;

namespace {
    /** Convert to integer, safe version.
        In general, failure to provide a valid integer where one is expected is a fatal type error.
        Here, we're dealing with auxiliary data and ignoring bogus elements is appropriate:
        a bogus property on a game slot could prevent people doing LSGAME on their home directory otherwise. */
    int32_t safeToInteger(Access a) {
        try {
            return a.toInteger();
        }
        catch (...) {
            return 0;
        }
    }
}

server::interface::FileGameClient::FileGameClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::FileGameClient::~FileGameClient()
{ }

void
server::interface::FileGameClient::getGameInfo(String_t path, GameInfo& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("STATGAME").pushBackString(path)));
    unpackGameInfo(result, p.get());
}

void
server::interface::FileGameClient::listGameInfo(String_t path, afl::container::PtrVector<GameInfo>& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("LSGAME").pushBackString(path)));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        GameInfo* gi = result.pushBackNew(new GameInfo());
        unpackGameInfo(*gi, a[i].getValue());
    }
}

void
server::interface::FileGameClient::getKeyInfo(String_t path, KeyInfo& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("STATREG").pushBackString(path)));
    unpackKeyInfo(result, p.get());
}

void
server::interface::FileGameClient::listKeyInfo(String_t path, const Filter& filter, afl::container::PtrVector<KeyInfo>& result)
{
    Segment seg;
    seg.pushBackString("LSREG");
    seg.pushBackString(path);
    if (filter.unique) {
        seg.pushBackString("UNIQ");
    }
    if (const String_t* keyId = filter.keyId.get()) {
        seg.pushBackString("ID");
        seg.pushBackString(*keyId);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(seg));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        KeyInfo* ki = result.pushBackNew(new KeyInfo());
        unpackKeyInfo(*ki, a[i].getValue());
    }
}

void
server::interface::FileGameClient::unpackGameInfo(GameInfo& result, const afl::data::Value* p)
{
    Access a(p);
    result.pathName   = a("path").toString();
    result.gameName   = a("name").toString();
    result.hostVersion = a("hostversion").toString();
    result.gameId     = safeToInteger(a("game"));
    result.hostTime   = safeToInteger(a("hosttime"));
    result.isFinished = safeToInteger(a("finished")) != 0;

    {
        Access slots = a("races");
        for (size_t i = 1, n = slots.getArraySize(); i < n; i += 2) {
            if (int32_t slot = safeToInteger(slots[i-1])) {
                result.slots.push_back(Slot_t(slot, slots[i].toString()));
            }
        }
    }

    {
        Access missing = a("missing");
        for (size_t i = 0, n = missing.getArraySize(); i < n; ++i) {
            result.missingFiles.push_back(missing[i].toString());
        }
    }

    {
        Access conflict = a("conflict");
        for (size_t i = 0, n = conflict.getArraySize(); i < n; ++i) {
            if (int32_t slot = safeToInteger(conflict[i])) {
                result.conflictSlots.push_back(slot);
            }
        }
    }
}

void
server::interface::FileGameClient::unpackKeyInfo(KeyInfo& result, const afl::data::Value* p)
{
    Access a(p);
    result.pathName     = a("path").toString();
    result.fileName     = a("file").toString();
    result.isRegistered = safeToInteger(a("reg")) != 0;
    result.label1       = a("key1").toString();
    result.label2       = a("key2").toString();
    if (const afl::data::Value* pp = a("useCount").getValue()) {
        result.useCount = safeToInteger(Access(pp));
    }
    if (const afl::data::Value* pp = a("id").getValue()) {
        result.keyId = Access(pp).toString();
    }
}
