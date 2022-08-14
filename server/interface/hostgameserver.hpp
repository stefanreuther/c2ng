/**
  *  \file server/interface/hostgameserver.hpp
  *  \brief Class server::interface::HostGameServer
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTGAMESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTGAMESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostgame.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Server for host game access.
        Implements a ComposableCommandHandler and dispatches received commands to a HostGame implementation. */
    class HostGameServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            @param impl Implementation; must live sufficiently long. */
        explicit HostGameServer(HostGame& impl);

        /** Destructor. */
        ~HostGameServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        /** Pack a HostGame::Info into a Value tree.
            @param info Input
            @return newly-allocated Value tree; caller assumes ownership. */
        static Value_t* packInfo(const HostGame::Info& info);

        /** Pack a HostGame::VictoryCondition into a Value tree.
            @param vc Input
            @return newly-allocated Value tree; caller assumes ownership. */
        static Value_t* packVictoryCondition(const HostGame::VictoryCondition& vc);

     private:
        HostGame& m_implementation;
    };

} }

#endif
