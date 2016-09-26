/**
  *  \file u/t_client_si_requestlink1.cpp
  *  \brief Test for client::si::RequestLink1
  */

#include "client/si/requestlink1.hpp"

#include "t_client_si.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Simple test. */
void
TestClientSiRequestLink1::testIt()
{
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World w(log, fs);
    interpreter::Process p(w, "TestClientSiRequestLink1", 99);
    client::si::RequestLink1 testee(p, true);

    TS_ASSERT_EQUALS(&testee.getProcess(), &p);
    TS_ASSERT(testee.isWantResult());
}
