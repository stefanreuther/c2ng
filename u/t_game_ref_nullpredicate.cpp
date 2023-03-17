/**
  *  \file u/t_game_ref_nullpredicate.cpp
  *  \brief Test for game::ref::NullPredicate
  */

#include "game/ref/nullpredicate.hpp"

#include "t_game_ref.hpp"

/** Simple test. */
void
TestGameRefNullPredicate::testIt()
{
    game::Reference a;

    TS_ASSERT_EQUALS(game::ref::NullPredicate().compare(a, a), 0);
    TS_ASSERT_EQUALS(game::ref::NullPredicate().getClass(a), "");
}

