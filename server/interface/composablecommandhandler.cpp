/**
  *  \file server/interface/composablecommandhandler.cpp
  *  \brief Class server::interface::ComposableCommandHandler
  */

#include <stdexcept>
#include "server/interface/composablecommandhandler.hpp"
#include "afl/string/string.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

server::interface::ComposableCommandHandler::Value_t*
server::interface::ComposableCommandHandler::call(const Segment_t& command)
{
    // Fetch command
    interpreter::Arguments args(command, 0, command.size());
    args.checkArgumentCountAtLeast(1);
    String_t cmd = afl::string::strUCase(toString(args.getNext()));

    // Process it
    std::auto_ptr<Value_t> result;
    if (!handleCommand(cmd, args, result)) {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
    return result.release();
}

void
server::interface::ComposableCommandHandler::callVoid(const Segment_t& command)
{
    delete call(command);
}
