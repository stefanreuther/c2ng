/**
  *  \file test/game/exceptiontest.cpp
  *  \brief Test for game::Exception
  */

#include "game/exception.hpp"

#include "afl/test/testrunner.hpp"
#include <cstring>

/** Simple test. */
AFL_TEST("game.Exception", a)
{
    game::Exception testee("hurz");
    a.checkEqual("01", std::strcmp(testee.what(), "hurz"), 0);
}
