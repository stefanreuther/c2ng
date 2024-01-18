/**
  *  \file test/game/spec/nullcomponentnameprovidertest.cpp
  *  \brief Test for game::spec::NullComponentNameProvider
  */

#include "game/spec/nullcomponentnameprovider.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.spec.NullComponentNameProvider", a)
{
    game::spec::NullComponentNameProvider testee;
    a.checkEqual("01. getName",      testee.getName(game::spec::ComponentNameProvider::Beam, 1, "long name"), "long name");
    a.checkEqual("02. getShortName", testee.getShortName(game::spec::ComponentNameProvider::Beam, 1, "long name", "short name"), "short name");
    a.checkEqual("03. getShortName", testee.getShortName(game::spec::ComponentNameProvider::Beam, 1, "long name", ""), "long name");
}
