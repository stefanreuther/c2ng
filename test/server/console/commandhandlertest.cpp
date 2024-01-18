/**
  *  \file test/server/console/commandhandlertest.cpp
  *  \brief Test for server::console::CommandHandler
  */

#include "server/console/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.console.CommandHandler")
{
    class Tester : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
    Tester t;
}
