/**
  *  \file test/server/play/commandhandlertest.cpp
  *  \brief Test for server::play::CommandHandler
  */

#include "server/play/commandhandler.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.play.CommandHandler")
{
    class Tester : public server::play::CommandHandler {
     public:
        virtual void processCommand(const String_t& /*cmd*/, interpreter::Arguments& /*args*/, server::play::PackerList& /*objs*/)
            { }
    };
    Tester t;
}
