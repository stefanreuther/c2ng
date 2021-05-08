/**
  *  \file server/test/consolecommandhandlermock.hpp
  *  \brief Class server::test::CallReceiver
  */
#ifndef C2NG_SERVER_TEST_CONSOLECOMMANDHANDLERMOCK_HPP
#define C2NG_SERVER_TEST_CONSOLECOMMANDHANDLERMOCK_HPP

#include "server/console/commandhandler.hpp"
#include "afl/test/callreceiver.hpp"

namespace server { namespace test {

    /** Mock for server::console::CommandHandler.

        Usage:
        - use expectCall("command|arg|arg|arg...") to expect a command.
        - use provideReturnValue(mode, value) to provide return values. */
    class ConsoleCommandHandlerMock : public server::console::CommandHandler, public afl::test::CallReceiver
    {
     public:
        /** Constructor.
            \param a Location identifier */
        explicit ConsoleCommandHandlerMock(afl::test::Assert a);

        enum Mode {
            Success,            ///< call() shall return true (=execution succeeded).
            Unrecognized,       ///< call() shall return false (=command not recognized).
            Failure             ///< call() shall throw an exception.
        };

        // CommandHandler:
        virtual bool call(const String_t& cmd, interpreter::Arguments args, server::console::Parser& parser, std::auto_ptr<afl::data::Value>& result);

        /** Provide a return value for a matching expectCall().
            \param mode Mode
            \param p    Result to return if non-null; null means return no result */
        void provideReturnValue(Mode mode, afl::data::Value* p);
    };

} }

#endif
