/**
  *  \file test/game/v3/trn/constantfiltertest.cpp
  *  \brief Test for game::v3::trn::ConstantFilter
  */

#include "game/v3/trn/constantfilter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"

/** Simple test. */
AFL_TEST("game.v3.trn.ConstantFilter", a)
{
    // Make a dummy turnfile
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    // Test it
    {
        game::v3::trn::ConstantFilter testee(true);
        a.check("01", testee.accept(trn, 0));
    }
    {
        game::v3::trn::ConstantFilter testee(false);
        a.check("02", !testee.accept(trn, 0));
    }
}
