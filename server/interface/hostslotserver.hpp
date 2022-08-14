/**
  *  \file server/interface/hostslotserver.hpp
  *  \brief Class server::interface::HostSlotServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSLOTSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTSLOTSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostslot.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Server for host player access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostSlot implementation. */
    class HostSlotServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostSlotServer(HostSlot& impl);

        /** Destructor. */
        ~HostSlotServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        HostSlot& m_implementation;
    };

} }

#endif
