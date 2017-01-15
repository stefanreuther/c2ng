/**
  *  \file u/t_game_v3_trn_andfilter.cpp
  *  \brief Test for game::v3::trn::AndFilter
  */

#include "game/v3/trn/andfilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/trn/constantfilter.hpp"
#include "game/v3/trn/indexfilter.hpp"

/** Simple test. */
void
TestGameV3TrnAndFilter::testIt()
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    game::v3::trn::ConstantFilter t(true), f(false);
    game::v3::trn::IndexFilter x(4, 4);

    // Test it
    TS_ASSERT( game::v3::trn::AndFilter(t, t).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::AndFilter(t, f).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::AndFilter(f, t).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::AndFilter(f, f).accept(trn, 0));

    // Test that index is passed down correctly
    TS_ASSERT( game::v3::trn::AndFilter(t, x).accept(trn, 3));
    TS_ASSERT(!game::v3::trn::AndFilter(t, x).accept(trn, 4));
    TS_ASSERT(!game::v3::trn::AndFilter(f, x).accept(trn, 3));
    TS_ASSERT(!game::v3::trn::AndFilter(f, x).accept(trn, 4));
}
