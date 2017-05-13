/**
  *  \file u/helper/commandhandlermock.hpp
  */
#ifndef C2NG_U_HELPER_COMMANDHANDLERMOCK_HPP
#define C2NG_U_HELPER_COMMANDHANDLERMOCK_HPP

#include "u/helper/callreceiver.hpp"
#include "afl/net/commandhandler.hpp"

class CommandHandlerMock : public afl::net::CommandHandler, public CallReceiver {
 public:
    virtual Value_t* call(const Segment_t& command);
    virtual void callVoid(const Segment_t& command);

    void provideReturnValue(Value_t* value);
};

#endif
