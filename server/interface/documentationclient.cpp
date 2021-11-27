/**
  *  \file server/interface/documentationclient.cpp
  *  \brief Class server::interface::DocumentationClient
  */

#include "server/interface/documentationclient.hpp"
#include "afl/data/segment.hpp"
#include "server/types.hpp"

using afl::data::Segment;
using server::interface::Documentation;

namespace {
    void packRenderOptions(Segment& cmd, const Documentation::RenderOptions& opts)
    {
        if (const String_t* p = opts.assetRoot.get()) {
            cmd.pushBackString("ASSET");
            cmd.pushBackString(*p);
        }
        if (const String_t* p = opts.siteRoot.get()) {
            cmd.pushBackString("SITE");
            cmd.pushBackString(*p);
        }
        if (const String_t* p = opts.docRoot.get()) {
            cmd.pushBackString("DOC");
            cmd.pushBackString(*p);
        }
        if (const String_t* p = opts.docSuffix.get()) {
            cmd.pushBackString("DOCSUFFIX");
            cmd.pushBackString(*p);
        }
    }

    void packChildOptions(Segment& cmd, const Documentation::ChildOptions& opts)
    {
        if (const int* p = opts.maxDepth.get()) {
            cmd.pushBackString("DEPTH");
            cmd.pushBackInteger(*p);
        }
        if (opts.acrossDocuments) {
            cmd.pushBackString("ACROSS");
        }
    }
}

server::interface::DocumentationClient::DocumentationClient(afl::net::CommandHandler& commandHandler)
    : Documentation(),
      m_commandHandler(commandHandler)
{ }

server::interface::DocumentationClient::~DocumentationClient()
{ }

String_t
server::interface::DocumentationClient::getBlob(String_t blobId)
{
    return m_commandHandler.callString(Segment().pushBackString("GET").pushBackString(blobId));
}

String_t
server::interface::DocumentationClient::renderNode(String_t nodeId, const RenderOptions& opts)
{
    Segment cmd;
    cmd.pushBackString("RENDER");
    cmd.pushBackString(nodeId);
    packRenderOptions(cmd, opts);
    return m_commandHandler.callString(cmd);
}

Documentation::NodeInfo
server::interface::DocumentationClient::getNodeInfo(String_t nodeId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("STAT").pushBackString(nodeId)));
    return unpackNodeInfo(p.get());
}

std::vector<Documentation::NodeInfo>
server::interface::DocumentationClient::getNodeChildren(String_t nodeId, const ChildOptions& opts)
{
    Segment cmd;
    cmd.pushBackString("LS");
    cmd.pushBackString(nodeId);
    packChildOptions(cmd, opts);

    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    return unpackNodeInfos(p.get());
}

std::vector<Documentation::NodeInfo>
server::interface::DocumentationClient::getNodeParents(String_t nodeId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("PATH").pushBackString(nodeId)));
    return unpackNodeInfos(p.get());
}

std::vector<Documentation::NodeInfo>
server::interface::DocumentationClient::getNodeNavigationContext(String_t nodeId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("NAV").pushBackString(nodeId)));
    return unpackNodeInfos(p.get());
}

std::vector<Documentation::NodeInfo>
server::interface::DocumentationClient::getNodeRelatedVersions(String_t nodeId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("VER").pushBackString(nodeId)));
    return unpackNodeInfos(p.get());
}

Documentation::NodeInfo
server::interface::DocumentationClient::unpackNodeInfo(afl::data::Access a)
{
    NodeInfo info;
    info.nodeId = a("id").toString();
    info.title = a("title").toString();
    for (size_t i = 0, n = a("tags").getArraySize(); i < n; ++i) {
        info.tags.push_back(a("tags")[i].toString());
    }
    info.isPage = (a("type").toInteger() == 0);
    info.hasChildren = (a("children").toInteger() != 0);
    info.infoTag = a("info").toInteger();
    return info;
}

std::vector<Documentation::NodeInfo>
server::interface::DocumentationClient::unpackNodeInfos(afl::data::Access a)
{
    std::vector<Documentation::NodeInfo> result;
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(unpackNodeInfo(a[i]));
    }
    return result;
}
