/**
  *  \file u/t_game_test_registrationkey.cpp
  *  \brief Test for game::test::RegistrationKey
  */

#include "game/test/registrationkey.hpp"

#include "t_game_test.hpp"

/** Simple coverage test. */
void
TestGameTestRegistrationKey::testIt()
{
    game::test::RegistrationKey testee(game::RegistrationKey::Registered, 4);
    TS_ASSERT_EQUALS(testee.getStatus(), game::RegistrationKey::Registered);
    TS_ASSERT_EQUALS(testee.getMaxTechLevel(game::HullTech), 4);
    TS_ASSERT(!testee.getLine(game::RegistrationKey::Line1).empty());
    TS_ASSERT(!testee.setLine(game::RegistrationKey::Line1, "f"));
}

