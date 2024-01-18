/**
  *  \file test/game/alliance/leveltest.cpp
  *  \brief Test for game::alliance::Level
  */

#include "game/alliance/level.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.alliance.Level", a)
{
    using game::alliance::Level;

    Level t("n", "i", Level::Flags_t(Level::IsEnemy));
    a.checkEqual("01. getName",  t.getName(), "n");
    a.checkEqual("02. getId",    t.getId(), "i");
    a.checkEqual("03 .getFlags", t.getFlags(), Level::Flags_t(Level::IsEnemy));
    a.checkEqual("04. hasFlag",  t.hasFlag(Level::IsEnemy), true);
    a.checkEqual("05. hasFlag",  t.hasFlag(Level::IsOffer), false);
}
