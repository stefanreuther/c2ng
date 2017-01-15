/**
  *  \file u/t_game_v3_trn_indexfilter.cpp
  *  \brief Test for game::v3::trn::IndexFilter
  */

#include "game/v3/trn/indexfilter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
void
TestGameV3TrnIndexFilter::testIt()
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Test it
    TS_ASSERT( game::v3::trn::IndexFilter(1, 1).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::IndexFilter(1, 1).accept(trn, 1));

    TS_ASSERT(!game::v3::trn::IndexFilter(5, 8).accept(trn, 3));
    TS_ASSERT( game::v3::trn::IndexFilter(5, 8).accept(trn, 4));
    TS_ASSERT( game::v3::trn::IndexFilter(5, 8).accept(trn, 5));
    TS_ASSERT( game::v3::trn::IndexFilter(5, 8).accept(trn, 6));
    TS_ASSERT( game::v3::trn::IndexFilter(5, 8).accept(trn, 7));
    TS_ASSERT(!game::v3::trn::IndexFilter(5, 8).accept(trn, 8));
}

