/**
  *  \file test/game/vcr/nulldatabasetest.cpp
  *  \brief Test for game::vcr::NullDatabase
  */

#include "game/vcr/nulldatabase.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.vcr.NullDatabase", a)
{
    game::vcr::NullDatabase testee;
    a.checkEqual("01. getNumBattles", testee.getNumBattles(), 0U);
    a.checkNull("02. getBattle", testee.getBattle(0));
    a.checkNull("03. getBattle", testee.getBattle(10000));
}
