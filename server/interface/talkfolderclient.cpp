/**
  *  \file server/interface/talkfolderclient.cpp
  */

#include <memory>
#include "server/interface/talkfolderclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/interface/talkforumclient.hpp"

using afl::data::Segment;

server::interface::TalkFolderClient::TalkFolderClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkFolderClient::~TalkFolderClient()
{ }

void
server::interface::TalkFolderClient::getFolders(afl::data::IntegerList_t& result)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("FOLDERLS")));
    afl::data::Access(p).toIntegerList(result);
}

server::interface::TalkFolder::Info
server::interface::TalkFolderClient::getInfo(int32_t ufid)
{
    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(Segment().pushBackString("FOLDERSTAT").pushBackInteger(ufid)));
    return unpackInfo(p.get());
}

void
server::interface::TalkFolderClient::getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results)
{
    Segment cmd;
    cmd.pushBackString("FOLDERMSTAT");
    while (const int32_t* p = ufids.eat()) {
        cmd.pushBackInteger(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        if (const afl::data::Value* pv = a[i].getValue()) {
            results.pushBackNew(new Info(unpackInfo(pv)));
        } else {
            results.pushBackNew(0);
        }
    }
}

int32_t
server::interface::TalkFolderClient::create(String_t name, afl::base::Memory<const String_t> args)
{
    Segment cmd;
    cmd.pushBackString("FOLDERNEW");
    cmd.pushBackString(name);
    while (const String_t* p = args.eat()) {
        cmd.pushBackString(*p);
    }

    return m_commandHandler.callInt(cmd);
}

bool
server::interface::TalkFolderClient::remove(int32_t ufid)
{
    return m_commandHandler.callInt(Segment().pushBackString("FOLDERRM").pushBackInteger(ufid)) != 0;
}

void
server::interface::TalkFolderClient::configure(int32_t ufid, afl::base::Memory<const String_t> args)
{
    Segment cmd;
    cmd.pushBackString("FOLDERSET");
    cmd.pushBackInteger(ufid);
    while (const String_t* p = args.eat()) {
        cmd.pushBackString(*p);
    }
    m_commandHandler.callVoid(cmd);
}

afl::data::Value*
server::interface::TalkFolderClient::getPMs(int32_t ufid, const ListParameters& params)
{
    Segment cmd;
    cmd.pushBackString("FOLDERLSPM");
    cmd.pushBackInteger(ufid);
    TalkForumClient::packListParameters(cmd, params);
    return m_commandHandler.call(cmd);
}

server::interface::TalkFolder::Info
server::interface::TalkFolderClient::unpackInfo(const afl::data::Value* p)
{
    afl::data::Access a(p);
    Info result;
    result.name              = a("name").toString();
    result.description       = a("description").toString();
    result.numMessages       = a("messages").toInteger();
    result.hasUnreadMessages = a("unread").toInteger() != 0;
    result.isFixedFolder     = a("fixed").toInteger() != 0;
    return result;
}
