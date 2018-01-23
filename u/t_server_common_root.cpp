/**
  *  \file u/t_server_common_root.cpp
  *  \brief Test for server::common::Root
  */

#include "server/common/root.hpp"

#include "t_server_common.hpp"
#include "afl/net/nullcommandhandler.hpp"

/** Simple test. */
void
TestServerCommonRoot::testIt()
{
    // We cannot sensibly test much more than that this class can be instantiated as intended.
    // Whether the accessors are actually correct is shown in the users' tests.
    afl::net::NullCommandHandler nch;
    server::common::Root testee(nch);
}

