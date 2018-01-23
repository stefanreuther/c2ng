/**
  *  \file u/t_server_console_context.cpp
  *  \brief Test for server::console::Context
  */

#include "server/console/context.hpp"

#include "t_server_console.hpp"

/** Interface test. */
void
TestServerConsoleContext::testInterface()
{
    class Tester : public server::console::Context {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
        virtual String_t getName()
            { return String_t(); }
    };
    Tester t;
}

