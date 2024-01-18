/**
  *  \file test/server/console/contexttest.cpp
  *  \brief Test for server::console::Context
  */

#include "server/console/context.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.console.Context")
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
