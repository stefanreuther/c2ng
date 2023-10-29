/**
  *  \file server/interface/hostspecificationserver.hpp
  *  \brief Class server::interface::HostSpecificationServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSPECIFICATIONSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTSPECIFICATIONSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostspecification.hpp"

namespace server { namespace interface {

    /** Server for host specification access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostSpecification implementation. */
    class HostSpecificationServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostSpecificationServer(HostSpecification& impl);

        /** Destructor. */
        ~HostSpecificationServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostSpecification& m_implementation;
    };

} }

#endif
