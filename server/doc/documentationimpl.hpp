/**
  *  \file server/doc/documentationimpl.hpp
  *  \brief Class server::doc::DocumentationImpl
  */
#ifndef C2NG_SERVER_DOC_DOCUMENTATIONIMPL_HPP
#define C2NG_SERVER_DOC_DOCUMENTATIONIMPL_HPP

#include "server/interface/documentation.hpp"

namespace server { namespace doc {

    class Root;

    /** Implementation of Documentation interface. */
    class DocumentationImpl : public server::interface::Documentation {
     public:
        /** Constructor.
            @param root Service root. Must live longer than the DocumentationImpl. */
        explicit DocumentationImpl(const Root& root);
        ~DocumentationImpl();

        // Interface methods:
        String_t getBlob(String_t blobId);
        String_t renderNode(String_t nodeId, const RenderOptions& opts);
        NodeInfo getNodeInfo(String_t nodeId);
        std::vector<NodeInfo> getNodeChildren(String_t nodeId, const ChildOptions& opts);
        std::vector<NodeInfo> getNodeParents(String_t nodeId);
        std::vector<NodeInfo> getNodeNavigationContext(String_t nodeId);
        std::vector<NodeInfo> getNodeRelatedVersions(String_t nodeId);

     private:
        const Root& m_root;
    };

} }

#endif
