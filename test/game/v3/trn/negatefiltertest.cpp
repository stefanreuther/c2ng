/**
  *  \file test/game/v3/trn/negatefiltertest.cpp
  *  \brief Test for game::v3::trn::NegateFilter
  */

#include "game/v3/trn/negatefilter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/trn/constantfilter.hpp"
#include "game/v3/trn/indexfilter.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
AFL_TEST("game.v3.trn.NegateFilter", a)
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    game::v3::trn::ConstantFilter t(true), f(false);
    game::v3::trn::IndexFilter x(4, 4);

    // Test it
    a.check("01",  game::v3::trn::NegateFilter(f).accept(trn, 0));
    a.check("02", !game::v3::trn::NegateFilter(t).accept(trn, 0));

    // Test that index is passed down correctly
    a.check("11", !game::v3::trn::NegateFilter(x).accept(trn, 3));
    a.check("12",  game::v3::trn::NegateFilter(x).accept(trn, 4));
}
