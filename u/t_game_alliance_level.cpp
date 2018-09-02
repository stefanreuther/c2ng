/**
  *  \file u/t_game_alliance_level.cpp
  *  \brief Test for game::alliance::Level
  */

#include "game/alliance/level.hpp"

#include "t_game_alliance.hpp"

/** Simple functionality test. */
void
TestGameAllianceLevel::testIt()
{
    using game::alliance::Level;

    Level t("n", "i", Level::Flags_t(Level::IsEnemy));
    TS_ASSERT_EQUALS(t.getName(), "n");
    TS_ASSERT_EQUALS(t.getId(), "i");
    TS_ASSERT_EQUALS(t.getFlags(), Level::Flags_t(Level::IsEnemy));
    TS_ASSERT_EQUALS(t.hasFlag(Level::IsEnemy), true);
    TS_ASSERT_EQUALS(t.hasFlag(Level::IsOffer), false);
}

