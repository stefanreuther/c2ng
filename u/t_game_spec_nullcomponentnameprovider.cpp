/**
  *  \file u/t_game_spec_nullcomponentnameprovider.cpp
  *  \brief Test for game::spec::NullComponentNameProvider
  */

#include "game/spec/nullcomponentnameprovider.hpp"

#include "t_game_spec.hpp"

/** Simple test. */
void
TestGameSpecNullComponentNameProvider::testIt()
{
    game::spec::NullComponentNameProvider testee;
    TS_ASSERT_EQUALS(testee.getName(game::spec::ComponentNameProvider::Beam, 1, "long name"), "long name");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 1, "long name", "short name"), "short name");
    TS_ASSERT_EQUALS(testee.getShortName(game::spec::ComponentNameProvider::Beam, 1, "long name", ""), "long name");
}

