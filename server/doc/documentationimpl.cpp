/**
  *  \file server/doc/documentationimpl.cpp
  *  \brief Class server::doc::DocumentationImpl
  */

#include <stdexcept>
#include "server/doc/documentationimpl.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/parser.hpp"
#include "afl/io/xml/reader.hpp"
#include "server/doc/root.hpp"
#include "server/errors.hpp"
#include "util/charsetfactory.hpp"
#include "util/doc/htmlrenderer.hpp"
#include "util/doc/renderoptions.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::FileMapping;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Nodes_t;
using afl::io::xml::Parser;
using afl::io::xml::Reader;
using server::doc::Root;
using server::interface::Documentation;
using util::CharsetFactory;
using util::doc::BlobStore;
using util::doc::Index;

namespace {
    const int DEFAULT_MAX_DEPTH = 2;

    // Shortcut for looking up a node.
    // Throws exception on error.
    // @param [in]  root    Service root
    // @param [in]  nodeId  Node Id given by user
    // @param [out] docId   Document Id containing the node
    Index::Handle_t findNode(const Root& root, const String_t& nodeId, String_t& docId)
    {
        if (nodeId.empty()) {
            docId = String_t();
            return root.index().root();
        } else {
            Index::Handle_t hdl;
            if (!root.index().findNodeByAddress(nodeId, hdl, docId)) {
                throw std::runtime_error(server::BLOB_NOT_FOUND);
            }
            return hdl;
        }
    }

    // Convert TaggedNode to NodeInfo
    // @param node   TaggedNode
    // @param index  Index to obtain meta-information
    // @param docId  Document Id for generating links
    Documentation::NodeInfo convertTaggedNode(const Index::TaggedNode& node,
                                              const Index& index,
                                              const String_t& docId)
    {
        Documentation::NodeInfo result;
        result.nodeId = index.getNodeAddress(node.node, docId);
        result.title = index.getNodeTitle(node.node);
        for (size_t i = 0, n = index.getNumNodeTags(node.node); i < n; ++i) {
            result.tags.push_back(index.getNodeTagByIndex(node.node, i));
        }
        result.isPage = index.isNodePage(node.node);
        result.hasChildren = index.getNumNodeChildren(node.node) != 0;
        result.infoTag = node.tag;
        return result;
    }

    // Convert RelatedNode to NodeInfo
    // @param node      TaggedNode
    // @param index     Index to obtain meta-information
    // @param origNode  Original node Id (for detecting current)
    // @param docId     Document Id for generating links.
    //                  This means the list refers to the originating node under its original name
    //                  even if the document name differs (-current vs. -1.2.3 case) (but not if the node name differs).
    Documentation::NodeInfo convertRelatedNode(const Index::RelatedNode& node,
                                               const Index& index,
                                               const Index::Handle_t origNode,
                                               const String_t& docId)
    {
        Documentation::NodeInfo result;
        result.nodeId = index.getNodeAddress(node.node, docId);
        result.title = index.getNodeTitle(node.docNode);
        for (size_t i = 0, n = index.getNumNodeTags(node.node); i < n; ++i) {
            result.tags.push_back(index.getNodeTagByIndex(node.node, i));
        }
        for (size_t i = 0, n = index.getNumNodeTags(node.docNode); i < n; ++i) {
            result.tags.push_back(index.getNodeTagByIndex(node.docNode, i));
        }
        result.isPage = index.isNodePage(node.node);
        result.hasChildren = index.getNumNodeChildren(node.node) != 0;

        const String_t& myContentId = index.getNodeContentId(origNode);
        result.infoTag = !myContentId.empty() && myContentId == index.getNodeContentId(node.node);
        return result;
    }
}



server::doc::DocumentationImpl::DocumentationImpl(const Root& root)
    : m_root(root)
{ }

server::doc::DocumentationImpl::~DocumentationImpl()
{ }

String_t
server::doc::DocumentationImpl::getBlob(String_t blobId)
{
    // For getBlob(), we have to censor the error messages.
    // This may get blob Ids the user invented themselves.
    // We don't want to tell them why their blob Ids do not work.
    // For the other functions that take a node Id, findNode() correctly throws a 404.
    // If, later on, a blob is not found, that's a service configuration error.
    try {
        return afl::string::fromBytes(m_root.blobStore().getObject(blobId)->get());
    }
    catch (std::exception& e) {
        throw std::runtime_error(BLOB_NOT_FOUND);
    }
}

String_t
server::doc::DocumentationImpl::renderNode(String_t nodeId, const RenderOptions& opts)
{
    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    // Build options
    util::doc::RenderOptions opts2;
    if (const String_t* p = opts.siteRoot.get()) {
        opts2.setSiteRoot(*p);
    }
    if (const String_t* p = opts.assetRoot.get()) {
        opts2.setAssetRoot(*p);
    }
    if (const String_t* p = opts.docRoot.get()) {
        opts2.setDocumentRoot(*p);
    }
    if (const String_t* p = opts.docSuffix.get()) {
        opts2.setDocumentLinkSuffix(*p);
    }
    opts2.setDocumentId(docId);

    // Retrieve document
    BlobStore::ObjectId_t objId = m_root.index().getNodeContentId(node);
    if (!objId.empty()) {
        // Parse XML
        Ref<FileMapping> content = m_root.blobStore().getObject(objId);
        ConstMemoryStream ms(content->get());
        CharsetFactory csFactory;
        DefaultEntityHandler eh;
        Nodes_t nodes;
        Reader rdr(ms, eh, csFactory);
        rdr.setWhitespaceMode(Reader::AllWS);
        Parser(rdr).parseNodes(nodes);

        // Render
        return renderHTML(nodes, opts2);
    } else {
        return String_t();
    }
}

Documentation::NodeInfo
server::doc::DocumentationImpl::getNodeInfo(String_t nodeId)
{
    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    return convertTaggedNode(Index::TaggedNode(node, 0), m_root.index(), docId);
}

std::vector<Documentation::NodeInfo>
server::doc::DocumentationImpl::getNodeChildren(String_t nodeId, const ChildOptions& opts)
{
    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    // List content
    std::vector<Index::TaggedNode> children = m_root.index().getNodeChildren(node, opts.maxDepth.orElse(DEFAULT_MAX_DEPTH), opts.acrossDocuments);

    // Build result
    std::vector<NodeInfo> result;
    for (size_t i = 0, n = children.size(); i < n; ++i) {
        result.push_back(convertTaggedNode(children[i], m_root.index(), docId));
    }
    return result;
}

std::vector<Documentation::NodeInfo>
server::doc::DocumentationImpl::getNodeParents(String_t nodeId)
{
    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    // List parents
    std::vector<Index::Handle_t> parents = m_root.index().getNodeParents(node);

    // Build result. Start at index 1; first is root which has no meaningful content.
    std::vector<NodeInfo> result;
    for (size_t i = 1, n = parents.size(); i < n; ++i) {
        result.push_back(convertTaggedNode(Index::TaggedNode(parents[i], 0), m_root.index(), docId));
    }
    return result;
}

std::vector<Documentation::NodeInfo>
server::doc::DocumentationImpl::getNodeNavigationContext(String_t nodeId)
{
    // We assume that our public identifiers are numerically identical to the internal ones.
    // If that were not the case, we'd have to convert.
    static_assert(NAV_PREVIOUS_INDIRECT == Index::NAV_PREVIOUS_INDIRECT, "NAV_PREVIOUS_INDIRECT");
    static_assert(NAV_PREVIOUS_DIRECT   == Index::NAV_PREVIOUS_DIRECT,   "NAV_PREVIOUS_DIRECT");
    static_assert(NAV_UP                == Index::NAV_UP,                "NAV_UP");
    static_assert(NAV_NEXT_DIRECT       == Index::NAV_NEXT_DIRECT,       "NAV_NEXT_DIRECT");
    static_assert(NAV_NEXT_INDIRECT     == Index::NAV_NEXT_INDIRECT,     "NAV_NEXT_INDIRECT");

    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    // List content
    std::vector<Index::TaggedNode> children = m_root.index().getNodeNavigationContext(node);

    // Build result
    std::vector<NodeInfo> result;
    for (size_t i = 0, n = children.size(); i < n; ++i) {
        result.push_back(convertTaggedNode(children[i], m_root.index(), docId));
    }
    return result;
}

std::vector<Documentation::NodeInfo>
server::doc::DocumentationImpl::getNodeRelatedVersions(String_t nodeId)
{
    // Look up node
    String_t docId;
    Index::Handle_t node = findNode(m_root, nodeId, docId);

    // List content
    std::vector<Index::RelatedNode> nodes = m_root.index().getNodeRelatedVersions(node);

    // Build result
    std::vector<NodeInfo> result;
    for (size_t i = 0, n = nodes.size(); i < n; ++i) {
        result.push_back(convertRelatedNode(nodes[i], m_root.index(), node, docId));
    }
    return result;
}
