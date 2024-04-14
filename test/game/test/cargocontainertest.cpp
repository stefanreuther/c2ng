/**
  *  \file test/game/test/cargocontainertest.cpp
  *  \brief Test for game::test::CargoContainer
  */

#include "game/test/cargocontainer.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

using afl::string::NullTranslator;
using game::Element;

/** Simple coverage test. */
AFL_TEST("game.test.CargoContainer", a)
{
    NullTranslator tx;
    game::test::CargoContainer testee;
    a.check("01. getName",                     !testee.getName(tx).empty());
    a.check("02. getFlags",                     testee.getFlags().empty());
    a.check("03. canHaveElement Neutronium",    testee.canHaveElement(Element::Neutronium));
    a.check("04. canHaveElement Torpedoes",     testee.canHaveElement(Element::fromTorpedoType(9)));
    a.checkEqual("05. getMaxAmount Neutronium", testee.getMaxAmount(Element::Neutronium), 10000);
    a.checkEqual("06. getMinAmount Neutronium", testee.getMinAmount(Element::Neutronium), 0);
    a.checkEqual("07. getAmount Neutronium",    testee.getAmount(Element::Neutronium), 5000);
    a.checkEqual("08. getInfo1",                testee.getInfo1(tx), "");
    a.checkEqual("09. getInfo2",                testee.getInfo2(tx), "");

    AFL_CHECK_SUCCEEDS(a("11. commit"),         testee.commit());
}
