/**
  *  \file client/downlink.cpp
  *  \brief Class client::Downlink
  */

#include "client/downlink.hpp"

client::Downlink::Downlink(ui::Root& root, afl::string::Translator& tx)
    : WaitIndicator(root, tx)
{ }

client::Downlink::Downlink(client::si::UserSide& us)
    : WaitIndicator(us.root(), us.translator())
{
    indicator().sig_interrupt.add(&us, &client::si::UserSide::interruptRunningProcesses);
}
