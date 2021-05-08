/**
  *  \file u/t_game_vcr_info.cpp
  *  \brief Test for game/vcr/info.hpp
  */

#include "game/vcr/info.hpp"

#include "t_game_vcr.hpp"

/** Verify initialisation (and self-containedness of header file). */
void
TestGameVcrInfo::testInit()
{
    game::vcr::ObjectInfo t;
    TS_ASSERT_EQUALS(t.color[0], util::SkinColor::Static);
    TS_ASSERT_EQUALS(t.text[0], "");
}

