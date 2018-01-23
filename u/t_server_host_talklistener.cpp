/**
  *  \file u/t_server_host_talklistener.cpp
  *  \brief Test for server::host::TalkListener
  */

#include "server/host/talklistener.hpp"

#include "t_server_host.hpp"

/** Interface test. */
void
TestServerHostTalkListener::testInterface()
{
    class Tester : public server::host::TalkListener {
     public:
        virtual void handleGameStart(server::host::Game& /*game*/, server::interface::HostGame::Type /*gameType*/)
            { }
        virtual void handleGameEnd(server::host::Game& /*game*/, server::interface::HostGame::Type /*gameType*/)
            { }
        virtual void handleGameNameChange(server::host::Game& /*game*/, const String_t& /*newName*/)
            { }
        virtual void handleGameTypeChange(server::host::Game& /*game*/, server::interface::HostGame::State /*gameState*/, server::interface::HostGame::Type /*gameType*/)
            { }
    };
    Tester t;
}

