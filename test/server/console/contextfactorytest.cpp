/**
  *  \file test/server/console/contextfactorytest.cpp
  *  \brief Test for server::console::ContextFactory
  */

#include "server/console/contextfactory.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.console.ContextFactory")
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
