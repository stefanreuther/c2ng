/**
  *  \file server/interface/hostplayerserver.hpp
  *  \brief Class server::interface::HostPlayerServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYERSERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYERSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostplayer.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Server for host player access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostPlayer implementation. */
    class HostPlayerServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostPlayerServer(HostPlayer& impl);

        /** Destructor. */
        ~HostPlayerServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack a HostPlayer::Info into a Value tree.
            @param i Input
            @return newly-allocated Value tree; caller assumes ownership. */
        static Value_t* packInfo(const HostPlayer::Info& i);

     private:
        HostPlayer& m_implementation;
    };

} }

#endif
