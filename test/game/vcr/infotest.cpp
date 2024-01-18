/**
  *  \file test/game/vcr/infotest.cpp
  *  \brief Test for game::vcr::Info
  */

#include "game/vcr/info.hpp"
#include "afl/test/testrunner.hpp"

/** Verify initialisation (and self-containedness of header file). */
AFL_TEST("game.vcr.Info", a)
{
    game::vcr::ObjectInfo t;
    a.checkEqual("01", t.color[0], util::SkinColor::Static);
    a.checkEqual("02", t.text[0], "");
}
