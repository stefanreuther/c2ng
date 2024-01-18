/**
  *  \file test/game/browser/typestest.cpp
  *  \brief Test for game::browser::Types
  */

#include "game/browser/types.hpp"
#include "afl/test/testrunner.hpp"

/* Nothing to test; just test that the header-file is well-formed */
AFL_TEST_NOARG("game.browser.Types")
{
    game::browser::LoadContentTask_t* t1 = 0;
    game::browser::LoadGameRootTask_t* t2 = 0;
    (void) t1;
    (void) t2;
}
