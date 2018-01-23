/**
  *  \file server/interface/hostgameserver.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTGAMESERVER_HPP
#define C2NG_SERVER_INTERFACE_HOSTGAMESERVER_HPP

#include "server/interface/composablecommandhandler.hpp"
#include "server/interface/hostgame.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostGameServer : public ComposableCommandHandler {
     public:
        explicit HostGameServer(HostGame& impl);
        ~HostGameServer();

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

        static Value_t* packInfo(const HostGame::Info& info);
        static Value_t* packVictoryCondition(const HostGame::VictoryCondition& vc);

     private:
        HostGame& m_implementation;
    };

} }

#endif
