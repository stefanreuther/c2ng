/**
  *  \file u/t_server_host_gamearbiter.cpp
  *  \brief Test for server::host::GameArbiter
  */

#include "server/host/gamearbiter.hpp"

#include "t_server_host.hpp"

/** Test GameArbiter. */
void
TestServerHostGameArbiter::testIt()
{
    server::host::GameArbiter testee;

    // Obtain initial lock
    TS_ASSERT_THROWS_NOTHING(testee.lock(10, server::host::GameArbiter::Critical));

    // A simple lock in parallel is ok (and releasing that still keeps the critical lock)
    TS_ASSERT_THROWS_NOTHING(testee.lock(10, server::host::GameArbiter::Simple));
    TS_ASSERT_THROWS_NOTHING(testee.unlock(10, server::host::GameArbiter::Simple));

    // Obtaining another critical lock fails
    TS_ASSERT_THROWS(testee.lock(10, server::host::GameArbiter::Critical), std::runtime_error);

    // Obtaining a different lock is OK
    TS_ASSERT_THROWS_NOTHING(testee.lock(99, server::host::GameArbiter::Critical));

    // Releasing the original lock allows re-acquiring it
    TS_ASSERT_THROWS_NOTHING(testee.unlock(10, server::host::GameArbiter::Critical));
    TS_ASSERT_THROWS_NOTHING(testee.lock(10, server::host::GameArbiter::Host));
}

/** Test GameArbiter::Guard. */
void
TestServerHostGameArbiter::testGuard()
{
    server::host::GameArbiter testee;

    // Obtaining multiple locks in sequence
    { server::host::GameArbiter::Guard a(testee, 17, server::host::GameArbiter::Critical); }
    { server::host::GameArbiter::Guard a(testee, 17, server::host::GameArbiter::Critical); }
    { server::host::GameArbiter::Guard a(testee, 17, server::host::GameArbiter::Critical); }

    // Nested locks
    {
        server::host::GameArbiter::Guard a(testee, 17, server::host::GameArbiter::Critical);
        server::host::GameArbiter::Guard b(testee, 17, server::host::GameArbiter::Simple);
    }

    // Nested conflicting locks
    {
        server::host::GameArbiter::Guard a(testee, 17, server::host::GameArbiter::Critical);
        TS_ASSERT_THROWS(server::host::GameArbiter::Guard(testee, 17, server::host::GameArbiter::Host), std::runtime_error);
    }
}

