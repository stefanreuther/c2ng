/**
  *  \file u/helper/commandhandlermock.cpp
  */

#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"

CommandHandlerMock::Value_t*
CommandHandlerMock::call(const Segment_t& command)
{
    String_t expect;
    for (size_t i = 0; i < command.size(); ++i) {
        if (i != 0) {
            expect += "|";
        }
        expect += server::toString(command[i]);
    }
    checkCall(expect);
    return consumeReturnValue<Value_t*>();
}

void
CommandHandlerMock::callVoid(const Segment_t& command)
{
    delete call(command);
}

void
CommandHandlerMock::provideReturnValue(Value_t* value)
{
    CallReceiver::provideReturnValue(value);
}
