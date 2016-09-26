/**
  *  \file u/t_client_si_requestlink2.cpp
  *  \brief Test for client::si::RequestLink2
  */

#include "client/si/requestlink2.hpp"

#include "t_client_si.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Test basic behaviour. */
void
TestClientSiRequestLink2::testIt()
{
    {
        client::si::RequestLink2 testee(125, false);
        uint32_t pid;
        TS_ASSERT(testee.getProcessId(pid));
        TS_ASSERT_EQUALS(pid, 125U);
        TS_ASSERT_EQUALS(testee.isWantResult(), false);
    }
    {
        client::si::RequestLink2 testee(999999999, true);
        uint32_t pid;
        TS_ASSERT(testee.getProcessId(pid));
        TS_ASSERT_EQUALS(pid, 999999999U);
        TS_ASSERT_EQUALS(testee.isWantResult(), true);
    }
    {
        client::si::RequestLink2 testee;
        uint32_t pid;
        TS_ASSERT(!testee.getProcessId(pid));
    }
}

/** Test conversion from RequestLink1. */
void
TestClientSiRequestLink2::testConvert()
{
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World w(log, fs);
    interpreter::Process p(w, "TestClientSiRequestLink2", 99);
    client::si::RequestLink1 t(p, true);

    client::si::RequestLink2 testee(t);
    uint32_t pid;
    TS_ASSERT(testee.getProcessId(pid));
    TS_ASSERT_EQUALS(pid, 99U);
    TS_ASSERT_EQUALS(testee.isWantResult(), true);
}

