/**
  *  \file u/t_game_v3_trn_parseexception.cpp
  *  \brief Test for game::v3::trn::ParseException
  */

#include "game/v3/trn/parseexception.hpp"

#include "t_game_v3_trn.hpp"

/** Simple test. */
void
TestGameV3TrnParseException::testIt()
{
    game::v3::trn::ParseException testee("foo");
    TS_ASSERT_EQUALS(String_t(testee.what()), "foo");

    std::exception& ex = testee;
    TS_ASSERT_EQUALS(String_t(ex.what()), "foo");
}

