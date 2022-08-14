/**
  *  \file server/interface/hostcronserver.hpp
  *  \brief Class server::interface::HostCronServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRONSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRONSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostcron.hpp"

namespace server { namespace interface {

    /** Server for host scheduler access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostCron implementation. */
    class HostCronServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostCronServer(HostCron& impl);

        /** Destructor. */
        ~HostCronServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostCron& m_implementation;

        static Value_t* packEvent(const HostCron::Event& event);
    };

} }

#endif
