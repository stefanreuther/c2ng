/**
  *  \file test/gfx/gen/perlinnoisetest.cpp
  *  \brief Test for gfx::gen::PerlinNoise
  */

#include "gfx/gen/perlinnoise.hpp"

#include "afl/test/testrunner.hpp"
#include "util/randomnumbergenerator.hpp"

/** Test some basic properties.
    This also acts as a regression test. */
AFL_TEST("gfx.gen.PerlinNoise", a)
{
    // Create
    util::RandomNumberGenerator rng(0);
    gfx::gen::PerlinNoise testee(rng);

    // Value is 0.5 at all integers.
    for (int x = 0; x < 30; ++x) {
        for (int y = 0; y < 30; ++y) {
            a.checkEqual("01", testee.noise(x,y,0), 0.5);
            a.checkEqual("02", testee.noise(x,y),   0.5);
            for (int z = 0; z < 30; ++z) {
                a.checkEqual("03", testee.noise(x,y,z), 0.5);
            }
        }
    }

    // Check some other values (regression test)
    a.checkEqual("11", testee.noise(0.5, 0, 0), 0.625);
    a.checkEqual("12", testee.noise(0.5, 0),    0.625);
    a.checkEqual("13", testee.noise(1.5, 0, 0), 0.375);
    a.checkEqual("14", testee.noise(1.5, 0),    0.375);
}
