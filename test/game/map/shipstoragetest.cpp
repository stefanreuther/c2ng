/**
  *  \file test/game/map/shipstoragetest.cpp
  *  \brief Test for game::map::ShipStorage
  */

#include "game/map/shipstorage.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;

/** Simple test: add some cargo, check that inquiry and commit works ok. */
AFL_TEST("game.map.ShipStorage", a)
{
    SimpleTurn h;
    game::test::initPListBeams(h.shipList());
    game::test::initPListTorpedoes(h.shipList());

    Ship& sh = h.addShip(10, 5, Object::Playable);
    sh.setName("Jason Statham");
    sh.setFriendlyCode(String_t("abc"));
    sh.setDamage(5);
    sh.setBeamType(3);
    sh.setNumBeams(4);
    sh.setTorpedoType(5);
    sh.setNumLaunchers(6);
    h.hull().setName("REMMLER");
    afl::string::NullTranslator tx;

    game::map::ShipStorage testee(sh, h.shipList());

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    a.checkEqual("01. max Molybdenum", testee.getMaxAmount(Element::Neutronium), 100);
    a.checkEqual("02. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);
    a.checkEqual("03. max Duranium",   testee.getMaxAmount(Element::Duranium), 60);
    a.checkEqual("04. max Money",      testee.getMaxAmount(Element::Money), 10000);
    a.checkEqual("05. getName",        testee.getName(tx), "Jason Statham");
    a.checkEqual("06. getInfo1",       testee.getInfo1(tx), "REMMLER, 4\xC3\x97""Desintegrator, 6\xC3\x97""Photon Torp");
    a.checkEqual("07. getInfo2",       testee.getInfo2(tx), "FCode: \"abc\", Damage: 5%");

    // Add some cargo
    testee.change(Element::Tritanium, 10);
    testee.change(Element::Tritanium, 10);
    a.checkEqual("11. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    a.checkEqual("12. max Duranium",   testee.getMaxAmount(Element::Duranium), 40);    // -20

    testee.change(Element::Neutronium, 30);
    a.checkEqual("21. max Neutronium", testee.getMaxAmount(Element::Neutronium), 100); // unchanged
    a.checkEqual("22. max Tritanium",  testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    a.checkEqual("23. max Duranium",   testee.getMaxAmount(Element::Duranium), 40);    // unchanged

    // Commit
    testee.commit();

    a.checkEqual("31. Neutronium", sh.getCargo(Element::Neutronium).orElse(0), 40);
    a.checkEqual("32. Tritanium",  sh.getCargo(Element::Tritanium).orElse(0), 30);
    a.checkEqual("33. Duranium",   sh.getCargo(Element::Duranium).orElse(0), 10);
    a.checkEqual("34. Molybdenum", sh.getCargo(Element::Molybdenum).orElse(0), 10);
}
