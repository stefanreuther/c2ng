/**
  *  \file test/game/test/cargocontainertest.cpp
  *  \brief Test for game::test::CargoContainer
  */

#include "game/test/cargocontainer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Simple coverage test. */
AFL_TEST("game.test.CargoContainer", a)
{
    afl::string::NullTranslator tx;
    game::test::CargoContainer testee;
    a.check("01. getName",                     !testee.getName(tx).empty());
    a.check("02. getFlags",                     testee.getFlags().empty());
    a.check("03. canHaveElement Neutronium",    testee.canHaveElement(game::Element::Neutronium));
    a.check("04. canHaveElement Torpedoes",     testee.canHaveElement(game::Element::fromTorpedoType(9)));
    a.checkEqual("05. getMaxAmount Neutronium", testee.getMaxAmount(game::Element::Neutronium), 10000);
    a.checkEqual("06. getMinAmount Neutronium", testee.getMinAmount(game::Element::Neutronium), 0);
    a.checkEqual("07. getAmount Neutronium",    testee.getAmount(game::Element::Neutronium), 5000);
    AFL_CHECK_SUCCEEDS(a("08. commit"),         testee.commit());
}
