/**
  *  \file u/t_server_console_commandhandler.cpp
  *  \brief Test for server::console::CommandHandler
  */

#include "server/console/commandhandler.hpp"

#include "t_server_console.hpp"

/** Interface test. */
void
TestServerConsoleCommandHandler::testInterface()
{
    class Tester : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
    Tester t;
}

