/**
  *  \file server/interface/hosttoolserver.hpp
  *  \brief Class server::interface::HostToolServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTOOLSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTTOOLSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hosttool.hpp"

namespace server { namespace interface {

    /** Server for host tool access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostTool implementation. */
    class HostToolServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long.
            @param area Area to accept commands for */
        HostToolServer(HostTool& impl, HostTool::Area area);

        /** Destructor. */
        ~HostToolServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack a HostTool::Info into a Value tree.
            @param tool Input
            @return newly-allocated Value tree; caller assumes ownership. */
        static Value_t* packTool(const HostTool::Info& tool);

     private:
        HostTool& m_implementation;
        const HostTool::Area m_area;

        bool isCommand(const String_t& upcasedCommand, const char* suffix) const;
    };

} }

#endif
