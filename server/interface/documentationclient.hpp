/**
  *  \file server/interface/documentationclient.hpp
  *  \brief Class server::interface::DocumentationClient
  */
#ifndef C2NG_SERVER_INTERFACE_DOCUMENTATIONCLIENT_HPP
#define C2NG_SERVER_INTERFACE_DOCUMENTATIONCLIENT_HPP

#include "server/interface/documentation.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/data/access.hpp"

namespace server { namespace interface {

    /** Client for Documentation server. */
    class DocumentationClient : public Documentation {
     public:
        /** Constructor.
            @param commandHandler CommandHandler. Must live longer than DocumentationClient. */
        DocumentationClient(afl::net::CommandHandler& commandHandler);
        ~DocumentationClient();

        // Interface methods:
        virtual String_t getBlob(String_t blobId);
        virtual String_t renderNode(String_t nodeId, const RenderOptions& opts);
        virtual NodeInfo getNodeInfo(String_t nodeId);
        virtual std::vector<NodeInfo> getNodeChildren(String_t nodeId, const ChildOptions& opts);
        virtual std::vector<NodeInfo> getNodeParents(String_t nodeId);
        virtual std::vector<NodeInfo> getNodeNavigationContext(String_t nodeId);
        virtual std::vector<NodeInfo> getNodeRelatedVersions(String_t nodeId);

        static NodeInfo unpackNodeInfo(afl::data::Access a);
        static std::vector<NodeInfo> unpackNodeInfos(afl::data::Access a);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
