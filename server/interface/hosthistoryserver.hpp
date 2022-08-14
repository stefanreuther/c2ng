/**
  *  \file server/interface/hosthistoryserver.hpp
  *  \brief Class server::interface::HostHistoryServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTHISTORYSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTHISTORYSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostHistory;

    /** Server for host history access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostHistory implementation. */
    class HostHistoryServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostHistoryServer(HostHistory& impl);

        /** Destructor. */
        ~HostHistoryServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostHistory& m_implementation;
    };

} }

#endif
