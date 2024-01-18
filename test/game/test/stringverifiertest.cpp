/**
  *  \file test/game/test/stringverifiertest.cpp
  *  \brief Test for game::test::StringVerifier
  */

#include "game/test/stringverifier.hpp"

#include "afl/test/testrunner.hpp"
#include <memory>

/** Simple coverage test. */
AFL_TEST("game.test.StringVerifier", a)
{
    game::test::StringVerifier testee;
    a.check("01. isValidString",      testee.isValidString(game::StringVerifier::PlanetName, "Terra"));
    a.check("02. isValidCharacter",   testee.isValidCharacter(game::StringVerifier::PlanetName, 'a'));
    a.checkGreaterThan("03. getMaxStringLength", testee.getMaxStringLength(game::StringVerifier::PlanetName), 100U);

    std::auto_ptr<game::StringVerifier> clone;
    AFL_CHECK_SUCCEEDS(a("11. clone"), clone.reset(testee.clone()));
    a.checkNonNull("12. clone", clone.get());
    a.check("13. clone isValidCharacter", clone->isValidCharacter(game::StringVerifier::PlanetName, 'a'));
}
