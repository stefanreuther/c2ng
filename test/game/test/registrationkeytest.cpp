/**
  *  \file test/game/test/registrationkeytest.cpp
  *  \brief Test for game::test::RegistrationKey
  */

#include "game/test/registrationkey.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test. */
AFL_TEST("game.test.RegistrationKey", a)
{
    game::test::RegistrationKey testee(game::RegistrationKey::Registered, 4);
    a.checkEqual("01. getStatus", testee.getStatus(), game::RegistrationKey::Registered);
    a.checkEqual("02. getMaxTechLevel", testee.getMaxTechLevel(game::HullTech), 4);
    a.check("03. getLine", !testee.getLine(game::RegistrationKey::Line1).empty());
    a.check("04. setLine", !testee.setLine(game::RegistrationKey::Line1, "f"));
}
