/**
  *  \file test/server/host/gamearbitertest.cpp
  *  \brief Test for server::host::GameArbiter
  */

#include "server/host/gamearbiter.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Test GameArbiter. */
AFL_TEST("server.host.GameArbiter:basics", a)
{
    server::host::GameArbiter testee;

    // Obtain initial lock
    AFL_CHECK_SUCCEEDS(a("01. lock"),   testee.lock(10, server::host::GameArbiter::Critical));

    // A simple lock in parallel is ok (and releasing that still keeps the critical lock)
    AFL_CHECK_SUCCEEDS(a("11. lock"),   testee.lock(10, server::host::GameArbiter::Simple));
    AFL_CHECK_SUCCEEDS(a("12. unlock"), testee.unlock(10, server::host::GameArbiter::Simple));

    // Obtaining another critical lock fails
    AFL_CHECK_THROWS(a("21. lock"),     testee.lock(10, server::host::GameArbiter::Critical), std::runtime_error);

    // Obtaining a different lock is OK
    AFL_CHECK_SUCCEEDS(a("31. lock"),   testee.lock(99, server::host::GameArbiter::Critical));

    // Releasing the original lock allows re-acquiring it
    AFL_CHECK_SUCCEEDS(a("41. unlock"), testee.unlock(10, server::host::GameArbiter::Critical));
    AFL_CHECK_SUCCEEDS(a("42. lock"),   testee.lock(10, server::host::GameArbiter::Host));
}

/** Test GameArbiter::Guard. */
AFL_TEST("server.host.GameArbiter:Guard", a)
{
    server::host::GameArbiter testee;

    // Obtaining multiple locks in sequence
    { server::host::GameArbiter::Guard ga(testee, 17, server::host::GameArbiter::Critical); }
    { server::host::GameArbiter::Guard ga(testee, 17, server::host::GameArbiter::Critical); }
    { server::host::GameArbiter::Guard ga(testee, 17, server::host::GameArbiter::Critical); }

    // Nested locks
    {
        server::host::GameArbiter::Guard ga(testee, 17, server::host::GameArbiter::Critical);
        server::host::GameArbiter::Guard gb(testee, 17, server::host::GameArbiter::Simple);
    }

    // Nested conflicting locks
    {
        server::host::GameArbiter::Guard ga(testee, 17, server::host::GameArbiter::Critical);
        AFL_CHECK_THROWS(a, server::host::GameArbiter::Guard(testee, 17, server::host::GameArbiter::Host), std::runtime_error);
    }
}
