/**
  *  \file server/interface/hostkeyserver.hpp
  *  \brief Class server::interface::HostKeyServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTKEYSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTKEYSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostkey.hpp"

namespace server { namespace interface {

    /** Server for host key access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostKey implementation. */
    class HostKeyServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            \param impl Implementation; must live sufficiently long. */
        explicit HostKeyServer(HostKey& impl);

        /** Destructor. */
        ~HostKeyServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack an Info structure into a Value tree.
            \param info Input
            \return newly-allocated Value tree; caller assumes ownership. */
        static Value_t* packInfo(const HostKey::Info& info);

     private:
        HostKey& m_implementation;
    };

} }

#endif
