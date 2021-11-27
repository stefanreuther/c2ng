/**
  *  \file u/t_server_interface_documentation.cpp
  *  \brief Test for server::interface::Documentation
  */

#include "server/interface/documentation.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceDocumentation::testInterface()
{
    class Tester : public server::interface::Documentation {
     public:
        virtual String_t getBlob(String_t /*blobId*/)
            { return String_t(); }
        virtual String_t renderNode(String_t /*nodeId*/, const RenderOptions& /*opts*/)
            { return String_t(); }
        virtual NodeInfo getNodeInfo(String_t /*nodeId*/)
            { return NodeInfo(); }
        virtual std::vector<NodeInfo> getNodeChildren(String_t /*nodeId*/, const ChildOptions& /*opts*/)
            { return std::vector<NodeInfo>(); }
        virtual std::vector<NodeInfo> getNodeParents(String_t /*nodeId*/)
            { return std::vector<NodeInfo>(); }
        virtual std::vector<NodeInfo> getNodeNavigationContext(String_t /*nodeId*/)
            { return std::vector<NodeInfo>(); }
        virtual std::vector<NodeInfo> getNodeRelatedVersions(String_t /*nodeId*/)
            { return std::vector<NodeInfo>(); }
    };
    Tester t;
}

