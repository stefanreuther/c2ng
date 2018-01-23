/**
  *  \file server/interface/hostplayerclient.cpp
  */

#include <memory>
#include "server/interface/hostplayerclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "afl/except/invaliddataexception.hpp"

using afl::data::Segment;
using afl::data::Access;

server::interface::HostPlayerClient::HostPlayerClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::HostPlayerClient::~HostPlayerClient()
{ }

// PLAYERJOIN game:GID slot:Int user:UID
void
server::interface::HostPlayerClient::join(int32_t gameId, int32_t slot, String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("PLAYERJOIN").pushBackInteger(gameId).pushBackInteger(slot).pushBackString(userId));
}

// PLAYERSUBST game:GID slot:Int user:UID
void
server::interface::HostPlayerClient::substitute(int32_t gameId, int32_t slot, String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("PLAYERSUBST").pushBackInteger(gameId).pushBackInteger(slot).pushBackString(userId));
}

// PLAYERRESIGN game:GID slot:Int user:UID
void
server::interface::HostPlayerClient::resign(int32_t gameId, int32_t slot, String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("PLAYERRESIGN").pushBackInteger(gameId).pushBackInteger(slot).pushBackString(userId));
}

// PLAYERADD game:GID user:UID
void
server::interface::HostPlayerClient::add(int32_t gameId, String_t userId)
{
    m_commandHandler.callVoid(Segment().pushBackString("PLAYERADD").pushBackInteger(gameId).pushBackString(userId));
}

// PLAYERLS game:GID [ALL]
void
server::interface::HostPlayerClient::list(int32_t gameId, bool all, std::map<int,Info>& result)
{
    Segment cmd;
    cmd.pushBackString("PLAYERLS");
    cmd.pushBackInteger(gameId);
    if (all) {
        cmd.pushBackString("ALL");
    }

    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access a(p);

    for (size_t i = 0, n = a.getArraySize(); i+1 < n; i += 2) {
        result[a[i].toInteger()] = unpackInfo(a[i+1].getValue());
    }
}

// PLAYERSTAT game:GID slot:Int
server::interface::HostPlayer::Info
server::interface::HostPlayerClient::getInfo(int32_t gameId, int32_t slot)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("PLAYERSTAT").pushBackInteger(gameId).pushBackInteger(slot)));
    return unpackInfo(p.get());
}

// PLAYERSETDIR game:GID user:UID dir:FileName
void
server::interface::HostPlayerClient::setDirectory(int32_t gameId, String_t userId, String_t dirName)
{
    m_commandHandler.callVoid(Segment().pushBackString("PLAYERSETDIR").pushBackInteger(gameId).pushBackString(userId).pushBackString(dirName));
}

// PLAYERGETDIR game:GID user:UID
String_t
server::interface::HostPlayerClient::getDirectory(int32_t gameId, String_t userId)
{
    return m_commandHandler.callString(Segment().pushBackString("PLAYERGETDIR").pushBackInteger(gameId).pushBackString(userId));
}

// PLAYERCHECKFILE game:GID user:UID name:Str [DIR dir:FileName]
server::interface::HostPlayer::FileStatus
server::interface::HostPlayerClient::checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName)
{
    Segment cmd;
    cmd.pushBackString("PLAYERCHECKFILE");
    cmd.pushBackInteger(gameId);
    cmd.pushBackString(userId);
    cmd.pushBackString(fileName);
    if (const String_t* p = dirName.get()) {
        cmd.pushBackString("DIR");
        cmd.pushBackString(*p);
    }

    FileStatus result;
    if (!parseFileStatus(m_commandHandler.callString(cmd), result)) {
        throw afl::except::InvalidDataException("<HostPlayer.checkFile>");
    }
    return result;
}

server::interface::HostPlayer::Info
server::interface::HostPlayerClient::unpackInfo(const Value_t* p)
{
    Access a(p);
    Info result;
    result.longName      = a("long").toString();
    result.shortName     = a("short").toString();
    result.adjectiveName = a("adj").toString();
    a("users").toStringList(result.userIds);
    result.numEditable   = a("editable").toInteger();
    result.joinable      = a("joinable").toInteger() != 0;
    return result;
}
