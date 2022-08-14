/**
  *  \file server/interface/hostturnserver.hpp
  *  \brief Class server::interface::HostTurnServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURNSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURNSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class HostTurn;

    /** Server for turn submission.
        Implements a ComposableCommandHandler and dispatches received commands to a HostTurn implementation. */
    class HostTurnServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostTurnServer(HostTurn& impl);

        /** Destructor. */
        ~HostTurnServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostTurn& m_implementation;
    };

} }

#endif
