/**
  *  \file server/interface/talkpmclient.cpp
  */

#include <memory>
#include "server/interface/talkpmclient.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/access.hpp"
#include "server/interface/talkrenderclient.hpp"

using afl::data::Segment;

server::interface::TalkPMClient::TalkPMClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

server::interface::TalkPMClient::~TalkPMClient()
{ }

int32_t
server::interface::TalkPMClient::create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent)
{
    Segment cmd;
    cmd.pushBackString("PMNEW");
    cmd.pushBackString(receivers);
    cmd.pushBackString(subject);
    cmd.pushBackString(text);
    if (const int32_t* p = parent.get()) {
        cmd.pushBackString("PARENT");
        cmd.pushBackInteger(*p);
    }
    return m_commandHandler.callInt(cmd);
}
    
server::interface::TalkPMClient::Info
server::interface::TalkPMClient::getInfo(int32_t folder, int32_t pmid)
{
    Segment cmd;
    cmd.pushBackString("PMSTAT");
    cmd.pushBackInteger(folder);
    cmd.pushBackInteger(pmid);

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    return unpackInfo(p.get());
}

void
server::interface::TalkPMClient::getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results)
{
    Segment cmd;
    cmd.pushBackString("PMMSTAT");
    cmd.pushBackInteger(folder);
    while (const int32_t* p = pmids.eat()) {
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
server::interface::TalkPMClient::copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
{
    Segment cmd;
    cmd.pushBackString("PMCP");
    cmd.pushBackInteger(sourceFolder);
    cmd.pushBackInteger(destFolder);
    while (const int32_t* p = pmids.eat()) {
        cmd.pushBackInteger(*p);
    }
    return m_commandHandler.callInt(cmd);
}

int32_t
server::interface::TalkPMClient::move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
{
    Segment cmd;
    cmd.pushBackString("PMMV");
    cmd.pushBackInteger(sourceFolder);
    cmd.pushBackInteger(destFolder);
    while (const int32_t* p = pmids.eat()) {
        cmd.pushBackInteger(*p);
    }
    return m_commandHandler.callInt(cmd);
}

int32_t
server::interface::TalkPMClient::remove(int32_t folder, afl::base::Memory<const int32_t> pmids)
{
    Segment cmd;
    cmd.pushBackString("PMRM");
    cmd.pushBackInteger(folder);
    while (const int32_t* p = pmids.eat()) {
        cmd.pushBackInteger(*p);
    }
    return m_commandHandler.callInt(cmd);
}

String_t
server::interface::TalkPMClient::render(int32_t folder, int32_t pmid, const Options& options)
{
    Segment cmd;
    cmd.pushBackString("PMRENDER");
    cmd.pushBackInteger(folder);
    cmd.pushBackInteger(pmid);
    TalkRenderClient::packOptions(cmd, options);
    return m_commandHandler.callString(cmd);
}

void
server::interface::TalkPMClient::render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result)
{
    Segment cmd;
    cmd.pushBackString("PMMRENDER");
    cmd.pushBackInteger(folder);
    while (const int32_t* p = pmids.eat()) {
        cmd.pushBackInteger(*p);
    }

    std::auto_ptr<afl::data::Value> p(m_commandHandler.call(cmd));
    afl::data::Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        if (const afl::data::Value* pv = a[i].getValue()) {
            result.pushBackNew(new String_t(server::toString(pv)));
        } else {
            result.pushBackNew(0);
        }
    }
}

int32_t
server::interface::TalkPMClient::changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids)
{
    Segment cmd;
    cmd.pushBackString("PMFLAG");
    cmd.pushBackInteger(folder);
    cmd.pushBackInteger(flagsToClear);
    cmd.pushBackInteger(flagsToSet);
    while (const int32_t* p = pmids.eat()) {
        cmd.pushBackInteger(*p);
    }
    return m_commandHandler.callInt(cmd);    
}

server::interface::TalkPMClient::Info
server::interface::TalkPMClient::unpackInfo(const afl::data::Value* p)
{
    afl::data::Access a(p);
    Info result;
    result.author    = a("author").toString();
    result.receivers = a("to").toString();
    result.time      = a("time").toInteger();
    result.subject   = a("subject").toString();
    result.flags     = a("flags").toInteger();
    if (int32_t parent = a("parent").toInteger()) {
        result.parent = parent;
    }
    return result;
}
