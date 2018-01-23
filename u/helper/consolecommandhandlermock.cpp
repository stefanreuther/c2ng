/**
  *  \file u/helper/consolecommandhandlermock.cpp
  */

#include <memory>
#include <stdexcept>
#include "u/helper/consolecommandhandlermock.hpp"
#include "server/types.hpp"

bool
ConsoleCommandHandlerMock::call(const String_t& cmd, interpreter::Arguments args, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& result)
{
    // Verify that this is the correct call
    String_t text = cmd;
    while (args.getNumArgs() > 0) {
        text += "|";
        text += server::toString(args.getNext());
    }
    checkCall(text);

    // Check
    Mode mode = consumeReturnValue<Mode>();
    std::auto_ptr<afl::data::Value> p(consumeReturnValue<afl::data::Value*>());

    // This is intentionally conditional to mirror that most commands that produce a null return value do not touch result.
    if (p.get() != 0) {
        result = p;
    }

    if (mode == Success) {
        return true;
    } else if (mode == Unrecognized) {
        return false;
    } else {
        throw std::runtime_error("boom");
    }
}

void
ConsoleCommandHandlerMock::provideReturnValue(Mode mode, afl::data::Value* p)
{
    CallReceiver::provideReturnValue(mode);
    CallReceiver::provideReturnValue(p);
}
