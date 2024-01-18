/**
  *  \file test/server/common/roottest.cpp
  *  \brief Test for server::common::Root
  */

#include "server/common/root.hpp"

#include "afl/net/nullcommandhandler.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST_NOARG("server.common.Root")
{
    // We cannot sensibly test much more than that this class can be instantiated as intended.
    // Whether the accessors are actually correct is shown in the users' tests.
    afl::net::NullCommandHandler nch;
    server::common::Root testee(nch);
}
