/**
  *  \file server/interface/documentation.hpp
  *  \brief Interface server::interface::Documentation
  */
#ifndef C2NG_SERVER_INTERFACE_DOCUMENTATION_HPP
#define C2NG_SERVER_INTERFACE_DOCUMENTATION_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Documentation Server interface.
        Provides access to a documentation repository.
        A documentation repository consists of a structured tree of documents and pages, and a set of blobs (assets/images).
        These can be retrieved over the network interface.

        @see util::doc::Index, util::doc::BlobStore */
    class Documentation : public afl::base::Deletable {
     public:
        /** Options for renderNode(). */
        struct RenderOptions {
            afl::base::Optional<String_t> assetRoot;      ///< Prefix for "asset:" links.
            afl::base::Optional<String_t> siteRoot;       ///< Prefix for "site:" links.
            afl::base::Optional<String_t> docRoot;        ///< Prefix for documentation links.
            afl::base::Optional<String_t> docSuffix;      ///< Suffix for documentation links.
        };

        /** Options for getNodeChildren(). */
        struct ChildOptions {
            afl::base::Optional<int> maxDepth;            ///< Maximum depth of recursion.
            bool acrossDocuments;                         ///< If true, recure into different documents.
            ChildOptions()
                : maxDepth(), acrossDocuments()
                { }
        };

        /** Information about a node. */
        struct NodeInfo {
            String_t nodeId;                              ///< Id (=path) of node.
            String_t title;                               ///< Title (=heading). See util::doc::Index::getNodeTitle().
            String_t blobId;                              ///< Blob Id, for use with getBlob().
            std::vector<String_t> tags;                   ///< Node tags (extra labels). See util::doc::Index::getNodeTagByIndex().
            bool isPage;                                  ///< Type flag: false if it is a document, true if page.
            bool hasChildren;                             ///< true if node has any children.
            int infoTag;                                  ///< Info tag. See util::doc::TaggedNode::tag.
            NodeInfo()
                : nodeId(), title(), blobId(), tags(), isPage(), hasChildren(), infoTag()
                { }
        };

        static const int NAV_PREVIOUS_INDIRECT = -2;      ///< Previous indirect (e.g. last child of previous sibling).
        static const int NAV_PREVIOUS_DIRECT = -1;        ///< Previous direct (previous sibling).
        static const int NAV_UP = 0;                      ///< Up (direct parent).
        static const int NAV_NEXT_DIRECT = +1;            ///< Next direct (next sibling).
        static const int NAV_NEXT_INDIRECT = +2;          ///< Next indirect (e.g. first child).


        /** Get blob (BLOB).
            This can be used to retrieve assets.

            @param blobId Blob Id (not including a potential "/file.jpg" suffix)
            @return blob */
        virtual String_t getBlob(String_t blobId) = 0;

        /** Render node content (RENDER).
            @param nodeId  Node Id
            @param opts    Options
            @return formatted blob, UTF-8 */
        virtual String_t renderNode(String_t nodeId, const RenderOptions& opts) = 0;

        /** Get information about a node (STAT).
            @param nodeId  Node Id
            @return node information */
        virtual NodeInfo getNodeInfo(String_t nodeId) = 0;

        /** List children of a node (LS).
            @param nodeId  Node Id
            @param opts    Options
            @return node information for all children, recursively; infoTag is depth */
        virtual std::vector<NodeInfo> getNodeChildren(String_t nodeId, const ChildOptions& opts) = 0;

        /** Get parents of a node (PATH).
            @param nodeId  Node Id
            @return node information for all parents */
        virtual std::vector<NodeInfo> getNodeParents(String_t nodeId) = 0;

        /** Get navigation context for a node (NAV).
            @param nodeId  Node Id
            @return related nodes; infoTag is type */
        virtual std::vector<NodeInfo> getNodeNavigationContext(String_t nodeId) = 0;

        /** Get related versions of a node (VER).
            @param nodeId  Node Id
            @return related nodes; infoTag is nonzero if text is identical to current page */
        virtual std::vector<NodeInfo> getNodeRelatedVersions(String_t nodeId) = 0;
    };

} }

#endif
