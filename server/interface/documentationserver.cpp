/**
  *  \file server/interface/documentationserver.cpp
  *  \brief Class server::interface::DocumentationServer
  */

#include "server/interface/documentationserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/errors.hpp"
#include "server/interface/documentation.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;
using server::interface::Documentation;
using server::toString;

namespace {
    bool handleRenderOption(const String_t& keyword, interpreter::Arguments& args, Documentation::RenderOptions& opts)
    {
        if (keyword == "ASSET") {
            args.checkArgumentCountAtLeast(1);
            opts.assetRoot = toString(args.getNext());
            return true;
        } else if (keyword == "SITE") {
            args.checkArgumentCountAtLeast(1);
            opts.siteRoot = toString(args.getNext());
            return true;
        } else if (keyword == "DOC") {
            args.checkArgumentCountAtLeast(1);
            opts.docRoot = toString(args.getNext());
            return true;
        } else if (keyword == "DOCSUFFIX") {
            args.checkArgumentCountAtLeast(1);
            opts.docSuffix = toString(args.getNext());
            return true;
        } else {
            return false;
        }
    }
}

server::interface::DocumentationServer::DocumentationServer(Documentation& impl)
    : ComposableCommandHandler(),
      m_implementation(impl)
{ }

server::interface::DocumentationServer::~DocumentationServer()
{ }

bool
server::interface::DocumentationServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // Dispatch command
    if (upcasedCommand == "PING") {
        /* @q PING (Documentation Command)
           Alive test.
           @retval Str "PONG". */
        result.reset(makeStringValue("PONG"));
        return true;
    } else if (upcasedCommand == "HELP") {
        /* @q HELP (Documentation Command)
           @retval Str Help page. */
        result.reset(makeStringValue("Commands:\n"
                                     "  PING\n"
                                     "  HELP\n"
                                     "  GET blob\n"
                                     "  RENDER node [ASSET pfx] [SITE pfx] [DOC pfx] [DOCSUFFIX suf]\n"
                                     "  STAT node\n"
                                     "  LS node [DEPTH n] [ACROSS]\n"
                                     "  PATH node\n"));
        return true;
    } else if (upcasedCommand == "GET") {
        /* @q GET blobId:Str (Documentation Command)
           Get blob.
           This can be used to retrieve assets.
           @retval Str Blob content
           @err 404 Blob not found */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getBlob(toString(args.getNext()))));
        return true;
    } else if (upcasedCommand == "RENDER") {
        /* @q RENDER node:DocNodeId [ASSET pfx:Str] [SITE pfx:Str] [DOC pfx:Str] [DOCSUFFIX suf:Str] (Documentation Command)
           Render a document, given its Id.
           Parameters specify URL prefixes to use in rendering links:
           - ASSET: root of assets ("asset:" links)
           - SITE: root of site ("site:" links)
           - DOC: root of documentation (links referring to other documents)
           - DOCSUFFIX: suffix for documentation links

           @retval Str Rendered document
           @err 404 Document not found */
        args.checkArgumentCountAtLeast(1);
        String_t nodeId = toString(args.getNext());

        Documentation::RenderOptions opts;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (!handleRenderOption(keyword, args, opts)) {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(makeStringValue(m_implementation.renderNode(nodeId, opts)));
        return true;
    } else if (upcasedCommand == "STAT") {
        /* @q STAT node:DocNodeId (Documentation Command)
           Get node information.
           @retval DocNodeInfo Node information
           @err 404 Document not found */
        args.checkArgumentCount(1);
        String_t nodeId = toString(args.getNext());
        result.reset(packNodeInfo(m_implementation.getNodeInfo(nodeId)));
        return true;
    } else if (upcasedCommand == "LS") {
        /* @q LS node:DocNodeId [DEPTH n:Int] [ACROSS] (Documentation Command)
           List children of a node.
           Use DEPTH do specify a depth, ACROSS to allow recursion into documents.

           @retval DocNodeInfo[] List of nodes, with info=depth (1=immediate child)
           @err 404 Document not found */
        args.checkArgumentCountAtLeast(1);
        String_t nodeId = toString(args.getNext());

        Documentation::ChildOptions opts;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "DEPTH") {
                args.checkArgumentCountAtLeast(1);
                opts.maxDepth = toInteger(args.getNext());
            } else if (keyword == "ACROSS") {
                opts.acrossDocuments = true;
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(packNodeInfos(m_implementation.getNodeChildren(nodeId, opts)));
        return true;
    } else if (upcasedCommand == "PATH") {
        /* @q PATH node:DocNodeId (Documentation Command)
           Get parents of a node.

           @retval DocNodeInfo[] List of parents
           @err 404 Document not found */
        args.checkArgumentCount(1);
        String_t nodeId = toString(args.getNext());

        result.reset(packNodeInfos(m_implementation.getNodeParents(nodeId)));
        return true;
    } else if (upcasedCommand == "NAV") {
        /* @q NAV node:DocNodeId (Documentation Command)
           Get navigation context of a node.

           Nodes have the following "info" values:
           - -2: previous indirect
           - -1: previous direct
           - 0: up
           - 1: next direct
           - 2: next indirect

           @retval DocNodeInfo[] Related nodes
           @err 404 Document not found */
        args.checkArgumentCount(1);
        String_t nodeId = toString(args.getNext());

        result.reset(packNodeInfos(m_implementation.getNodeNavigationContext(nodeId)));
        return true;
    } else if (upcasedCommand == "VER") {
        /* @q VER node:DocNodeId (Documentation Command)
           Get related versions of a node.
           "info" is nonzero if text is identical to current node.

           @retval DocNodeInfo[] Related nodes
           @err 404 Document not found */
        args.checkArgumentCount(1);
        String_t nodeId = toString(args.getNext());

        result.reset(packNodeInfos(m_implementation.getNodeRelatedVersions(nodeId)));
        return true;
    } else {
        return false;
    }
}

server::Value_t*
server::interface::DocumentationServer::packNodeInfo(const Documentation::NodeInfo& info)
{
    /* @type DocNodeInfo
       Information about a node.

       @key id:DocNodeId (Id of node)
       @key title:Str    (Title)
       @key tags:Str[]   (Tags)
       @key blob:Str     (Blob Id)
       @key type:Int     (0=page, 1=document)
       @key children:Int (1 if node has children)
       @key info:Int     (Info tag, e.g. depth for LS) */
    Hash::Ref_t h = Hash::create();
    h->setNew("id", makeStringValue(info.nodeId));
    h->setNew("title", makeStringValue(info.title));
    h->setNew("blob", makeStringValue(info.blobId));

    Vector::Ref_t v = Vector::create();
    for (size_t i = 0, n = info.tags.size(); i < n; ++i) {
        v->pushBackNew(makeStringValue(info.tags[i]));
    }
    h->setNew("tags", new VectorValue(v));

    // Note that we use negative polarity for the "isPage" flag to allow for potential
    // future extension of the "type" field for different types of documents (group vs. actual document).
    h->setNew("type", makeIntegerValue(!info.isPage));

    h->setNew("children", makeIntegerValue(info.hasChildren));
    h->setNew("info", makeIntegerValue(info.infoTag));
    return new HashValue(h);
}

server::Value_t*
server::interface::DocumentationServer::packNodeInfos(const std::vector<Documentation::NodeInfo>& infos)
{
    Vector::Ref_t vec = Vector::create();
    for (size_t i = 0; i < infos.size(); ++i) {
        vec->pushBackNew(packNodeInfo(infos[i]));
    }
    return new VectorValue(vec);
}
