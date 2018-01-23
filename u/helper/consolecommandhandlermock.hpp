/**
  *  \file u/helper/consolecommandhandlermock.hpp
  */
#ifndef C2NG_U_HELPER_CONSOLECOMMANDHANDLERMOCK_HPP
#define C2NG_U_HELPER_CONSOLECOMMANDHANDLERMOCK_HPP

#include "server/console/commandhandler.hpp"
#include "afl/test/callreceiver.hpp"

class ConsoleCommandHandlerMock : public server::console::CommandHandler, public afl::test::CallReceiver
{
 public:
    ConsoleCommandHandlerMock(afl::test::Assert a)
        : CallReceiver(a)
        { }
    enum Mode {
        Success,
        Unrecognized,
        Failure
    };
    virtual bool call(const String_t& cmd, interpreter::Arguments args, server::console::Parser& parser, std::auto_ptr<afl::data::Value>& result);

    void provideReturnValue(Mode mode, afl::data::Value* p);
};

#endif
