/**
  *  \file test/game/v3/trn/parseexceptiontest.cpp
  *  \brief Test for game::v3::trn::ParseException
  */

#include "game/v3/trn/parseexception.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.v3.trn.ParseException", a)
{
    game::v3::trn::ParseException testee("foo");
    a.checkEqual("01", String_t(testee.what()), "foo");

    std::exception& ex = testee;
    a.checkEqual("11", String_t(ex.what()), "foo");
}
