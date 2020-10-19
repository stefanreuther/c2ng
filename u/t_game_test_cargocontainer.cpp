/**
  *  \file u/t_game_test_cargocontainer.cpp
  *  \brief Test for game::test::CargoContainer
  */

#include "game/test/cargocontainer.hpp"

#include "t_game_test.hpp"
#include "afl/string/nulltranslator.hpp"

/** Simple coverage test. */
void
TestGameTestCargoContainer::testIt()
{
    afl::string::NullTranslator tx;
    game::test::CargoContainer testee;
    TS_ASSERT(!testee.getName(tx).empty());
    TS_ASSERT(testee.getFlags().empty());
    TS_ASSERT(testee.canHaveElement(game::Element::Neutronium));
    TS_ASSERT(testee.canHaveElement(game::Element::fromTorpedoType(9)));
    TS_ASSERT_EQUALS(testee.getMaxAmount(game::Element::Neutronium), 10000);
    TS_ASSERT_EQUALS(testee.getMinAmount(game::Element::Neutronium), 0);
    TS_ASSERT_EQUALS(testee.getAmount(game::Element::Neutronium), 5000);
    TS_ASSERT_THROWS_NOTHING(testee.commit());
}

