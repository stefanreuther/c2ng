/**
  *  \file u/t_game_v3_trn_constantfilter.cpp
  *  \brief Test for game::v3::trn::ConstantFilter
  */

#include "game/v3/trn/constantfilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
void
TestGameV3TrnConstantFilter::testIt()
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Test it
    {
        game::v3::trn::ConstantFilter testee(true);
        TS_ASSERT(testee.accept(trn, 0));
    }
    {
        game::v3::trn::ConstantFilter testee(false);
        TS_ASSERT(!testee.accept(trn, 0));
    }
}

