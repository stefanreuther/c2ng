/**
  *  \file u/t_util_randomnumbergenerator.cpp
  *  \brief Test for util::RandomNumberGenerator
  */

#include "util/randomnumbergenerator.hpp"

#include "t_util.hpp"

/** Test probability distribution.
    This is just a simple test: generate N*M random numbers sort them into N buckets,
    and check that each bucket appears M times, allowing 20% difference. */
void
TestUtilRandomNumberGenerator::testIt()
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
        TS_ASSERT_LESS_THAN_EQUALS(freqs[i], M*6/5);
        TS_ASSERT_LESS_THAN_EQUALS(M*4/5, freqs[i]);
    }
}

/** Test range behaviour.
    We require that asking for a smaller range produces the same numbers, just scaled down
    (i.e. it scales and does not compute modulus). */
void
TestUtilRandomNumberGenerator::testRange()
{
    util::RandomNumberGenerator a(1);
    util::RandomNumberGenerator b(1);
    util::RandomNumberGenerator c(1);
    for (int i = 0; i < 10000; ++i) {
        int aa = a(1000);
        int bb = b(500);
        int cc = c(100);
        TS_ASSERT_EQUALS(aa/2,  bb);
        TS_ASSERT_EQUALS(aa/10, cc);
        TS_ASSERT_EQUALS(bb/5,  cc);
    }
}

/** Test full range behaviour.
    Test that unscaled calls produces full 16-bit range. */
void
TestUtilRandomNumberGenerator::testFullRange()
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
    TS_ASSERT(got0);
    TS_ASSERT(got64k);
}

/** Test seed access.
    A sequence must be reproducible given a seed. */
void
TestUtilRandomNumberGenerator::testReset()
{
    util::RandomNumberGenerator testee(42);
    TS_ASSERT_EQUALS(testee.getSeed(), 42U);

    const int N = 10;
    int n[N];
    for (int i = 0; i < N; ++i) {
        n[i] = testee(10000);
    }

    testee.setSeed(42);
    for (int i = 0; i < N; ++i) {
        int z = testee(10000);
        TS_ASSERT_EQUALS(z, n[i]);
    }
}
