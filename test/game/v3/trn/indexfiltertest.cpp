/**
  *  \file test/game/v3/trn/indexfiltertest.cpp
  *  \brief Test for game::v3::trn::IndexFilter
  */

#include "game/v3/trn/indexfilter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
AFL_TEST("game.v3.trn.IndexFilter", a)
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Test it
    a.check("01",  game::v3::trn::IndexFilter(1, 1).accept(trn, 0));
    a.check("02", !game::v3::trn::IndexFilter(1, 1).accept(trn, 1));

    a.check("11", !game::v3::trn::IndexFilter(5, 8).accept(trn, 3));
    a.check("12",  game::v3::trn::IndexFilter(5, 8).accept(trn, 4));
    a.check("13",  game::v3::trn::IndexFilter(5, 8).accept(trn, 5));
    a.check("14",  game::v3::trn::IndexFilter(5, 8).accept(trn, 6));
    a.check("15",  game::v3::trn::IndexFilter(5, 8).accept(trn, 7));
    a.check("16", !game::v3::trn::IndexFilter(5, 8).accept(trn, 8));
}
