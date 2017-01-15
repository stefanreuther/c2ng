/**
  *  \file u/t_gfx_gen_perlinnoise.cpp
  *  \brief Test for gfx::gen::PerlinNoise
  */

#include "gfx/gen/perlinnoise.hpp"

#include "t_gfx_gen.hpp"
#include "util/randomnumbergenerator.hpp"

/** Test some basic properties.
    This also acts as a regression test. */
void
TestGfxGenPerlinNoise::testIt()
{
    // Create
    util::RandomNumberGenerator rng(0);
    gfx::gen::PerlinNoise testee(rng);

    // Value is 0.5 at all integers.
    for (int x = 0; x < 30; ++x) {
        for (int y = 0; y < 30; ++y) {
            TS_ASSERT_EQUALS(testee.noise(x,y,0), 0.5);
            TS_ASSERT_EQUALS(testee.noise(x,y),   0.5);
            for (int z = 0; z < 30; ++z) {
                TS_ASSERT_EQUALS(testee.noise(x,y,z), 0.5);
                
            }
        }
    }

    // Check some other values (regression test)
    TS_ASSERT_EQUALS(testee.noise(0.5, 0, 0), 0.625);
    TS_ASSERT_EQUALS(testee.noise(0.5, 0),    0.625);
    TS_ASSERT_EQUALS(testee.noise(1.5, 0, 0), 0.375);
    TS_ASSERT_EQUALS(testee.noise(1.5, 0),    0.375);
}

