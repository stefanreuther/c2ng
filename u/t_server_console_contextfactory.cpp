/**
  *  \file u/t_server_console_contextfactory.cpp
  *  \brief Test for server::console::ContextFactory
  */

#include "server/console/contextfactory.hpp"

#include "t_server_console.hpp"

/** Interface test. */
void
TestServerConsoleContextFactory::testInterface()
{
    class Tester : public server::console::ContextFactory {
     public:
        virtual String_t getCommandName()
            { return String_t(); }
        virtual server::console::Context* create()
            { return 0; }
        virtual bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
    };
    Tester t;
}

