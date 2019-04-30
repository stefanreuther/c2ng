/**
  *  \file u/t_server_play_commandhandler.cpp
  *  \brief Test for server::play::CommandHandler
  */

#include "server/play/commandhandler.hpp"

#include "t_server_play.hpp"

/** Interface test. */
void
TestServerPlayCommandHandler::testInterface()
{
    class Tester : public server::play::CommandHandler {
     public:
        virtual void processCommand(const String_t& /*cmd*/, interpreter::Arguments& /*args*/, server::play::PackerList& /*objs*/)
            { }
    };
    Tester t;
}

