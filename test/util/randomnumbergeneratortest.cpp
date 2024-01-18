/**
  *  \file test/util/randomnumbergeneratortest.cpp
  *  \brief Test for util::RandomNumberGenerator
  */

#include "util/randomnumbergenerator.hpp"
#include "afl/test/testrunner.hpp"

/** Test probability distribution.
    This is just a simple test: generate N*M random numbers sort them into N buckets,
    and check that each bucket appears M times, allowing 20% difference. */
AFL_TEST("util.RandomNumberGenerator:distribution", a)
{
    util::RandomNumberGenerator testee(0);

    const int N = 100;
    const int M = 300;
    int freqs[N];

    for (int i = 0; i < N; ++i) {
        freqs[i] = 0;
    }
    for (int i = 0; i < N*M; ++i) {
        ++freqs[testee(N)];
    }

    for (int i = 0; i < N; ++i) {
        a.checkLessEqual("01", freqs[i], M*6/5);
        a.checkLessEqual("02", M*4/5, freqs[i]);
    }
}

/** Test range behaviour.
    We require that asking for a smaller range produces the same numbers, just scaled down
    (i.e. it scales and does not compute modulus). */
AFL_TEST("util.RandomNumberGenerator:range", a)
{
    util::RandomNumberGenerator ga(1);
    util::RandomNumberGenerator gb(1);
    util::RandomNumberGenerator gc(1);
    for (int i = 0; i < 10000; ++i) {
        int aa = ga(1000);
        int bb = gb(500);
        int cc = gc(100);
        a.checkEqual("01", aa/2,  bb);
        a.checkEqual("02", aa/10, cc);
        a.checkEqual("03", bb/5,  cc);
    }
}

/** Test full range behaviour.
    Test that unscaled calls produces full 16-bit range. */
AFL_TEST("util.RandomNumberGenerator:full-range", a)
{
    util::RandomNumberGenerator testee(99);
    bool got0 = false;
    bool got64k = false;
    for (int32_t i = 0; i < 100000; ++i) {
        uint16_t n = testee();
        if (n == 0) {
            got0 = true;
        }
        if (n == 65535) {
            got64k = true;
        }
    }
    a.check("01", got0);
    a.check("02", got64k);
}

/** Test seed access.
    A sequence must be reproducible given a seed. */
AFL_TEST("util.RandomNumberGenerator:setSeed", a)
{
    util::RandomNumberGenerator testee(42);
    a.checkEqual("getSeed", testee.getSeed(), 42U);

    const int N = 10;
    int n[N];
    for (int i = 0; i < N; ++i) {
        n[i] = testee(10000);
    }

    testee.setSeed(42);
    for (int i = 0; i < N; ++i) {
        int z = testee(10000);
        a.checkEqual("value", z, n[i]);
    }
}
