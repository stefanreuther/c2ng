/**
  *  \file u/t_game_test_stringverifier.cpp
  *  \brief Test for game::test::StringVerifier
  */

#include "game/test/stringverifier.hpp"

#include <memory>
#include "t_game_test.hpp"

/** Simple coverage test. */
void
TestGameTestStringVerifier::testIt()
{
    game::test::StringVerifier testee;
    TS_ASSERT(testee.isValidString(game::StringVerifier::PlanetName, "Terra"));
    TS_ASSERT(testee.isValidCharacter(game::StringVerifier::PlanetName, 'a'));
    TS_ASSERT(testee.getMaxStringLength(game::StringVerifier::PlanetName) > 100);

    std::auto_ptr<game::StringVerifier> clone;
    TS_ASSERT_THROWS_NOTHING(clone.reset(testee.clone()));
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT(clone->isValidCharacter(game::StringVerifier::PlanetName, 'a'));
}

