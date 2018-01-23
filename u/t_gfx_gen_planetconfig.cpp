/**
  *  \file u/t_gfx_gen_planetconfig.cpp
  *  \brief Test for gfx::gen::PlanetConfig
  */

#include "gfx/gen/planetconfig.hpp"

#include "t_gfx_gen.hpp"
#include "afl/base/countof.hpp"
#include "util/randomnumbergenerator.hpp"

/** Regression test: default configuration. */
void
TestGfxGenPlanetConfig::testDefault()
{
    // Prepare
    util::RandomNumberGenerator rng(0);
    gfx::gen::PlanetConfig testee;
    testee.setSize(gfx::Point(10, 10));

    // Produce result
    afl::base::Ref<gfx::RGBAPixmap> pix = testee.render(rng);

    // Compare expectation
    static const gfx::ColorQuad_t EXPECTED[] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x2F575BFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x376D2CFF, 0x6C7CAFFF, 0x376F2DFF, 0x3B6728FF, 0x34672AFF, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x386B2BFF, 0x377839FF, 0x69976FFF, 0x83AA87FF, 0x396829FF, 0x869158FF, 0x2E652AFF, 0x00000000,
        0x00000000, 0x00000000, 0x8794B8FF, 0x336655FF, 0x4B7078FF, 0x357430FF, 0x386A2BFF, 0x3B6527FF, 0x73906DFF, 0x00000000,
        0x00000000, 0x929FB2FF, 0x99A1BAFF, 0xA7C4A6FF, 0x608977FF, 0x335C69FF, 0x293F7BFF, 0x142259FF, 0x2D5B42FF, 0x0B1532FF,
        0x00000000, 0x00000000, 0x3E6075FF, 0x376C2CFF, 0x61915CFF, 0x366D2CFF, 0x315865FF, 0x1C3263FF, 0x3E4D7BFF, 0x00000000,
        0x00000000, 0x00000000, 0x30595CFF, 0x327330FF, 0x2D4384FF, 0x739A71FF, 0x306C2DFF, 0x486D5EFF, 0x223B4CFF, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x334677FF, 0x3A4D83FF, 0x293C78FF, 0x294955FF, 0x1F2E5BFF, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x274023FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    };
    TS_ASSERT_EQUALS(pix->pixels().size(), countof(EXPECTED));
    TS_ASSERT_SAME_DATA(pix->pixels().unsafeData(), EXPECTED, sizeof(EXPECTED));
}

