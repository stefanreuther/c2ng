/**
  *  \file u/t_util_randomnumbergenerator.cpp
  *  \brief Test for util::RandomNumberGenerator
  */

#include "util/randomnumbergenerator.hpp"

#include "t_util.hpp"

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
