/**
  *  \file test/game/ref/nullpredicatetest.cpp
  *  \brief Test for game::ref::NullPredicate
  */

#include "game/ref/nullpredicate.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.ref.NullPredicate", a)
{
    game::Reference ra;

    a.checkEqual("01. compare", game::ref::NullPredicate().compare(ra, ra), 0);
    a.checkEqual("02. getClass", game::ref::NullPredicate().getClass(ra), "");
}
