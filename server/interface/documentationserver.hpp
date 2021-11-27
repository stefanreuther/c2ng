/**
  *  \file server/interface/documentationserver.hpp
  *  \brief Class server::interface::DocumentationServer
  */
#ifndef C2NG_SERVER_INTERFACE_DOCUMENTATIONSERVER_HPP
#define C2NG_SERVER_INTERFACE_DOCUMENTATIONSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/documentation.hpp"

namespace server { namespace interface {

    /** Server for Documentation interface. */
    class DocumentationServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation */
        explicit DocumentationServer(Documentation& impl);
        ~DocumentationServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Serialize a NodeInfo structure.
            @param info Structure
            @return newly-allocated hash */
        static Value_t* packNodeInfo(const Documentation::NodeInfo& info);

        /** Serialize a NodeInfo array.
            @param infos Array of structures
            @return newly-allocated vector */
        static Value_t* packNodeInfos(const std::vector<Documentation::NodeInfo>& infos);

     private:
        Documentation& m_implementation;
    };

} }

#endif
