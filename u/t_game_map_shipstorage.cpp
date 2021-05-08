/**
  *  \file u/t_game_map_shipstorage.cpp
  *  \brief Test for game::map::ShipStorage
  */

#include "game/map/shipstorage.hpp"

#include "t_game_map.hpp"
#include "game/test/simpleturn.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/shiplist.hpp"

using game::Element;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;

/** Simple test: add some cargo, check that inquiry and commit works ok. */
void
TestGameMapShipStorage::testIt()
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

    game::map::ShipStorage testee(sh, h.shipList(), tx);

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Money), 10000);
    TS_ASSERT_EQUALS(testee.getName(tx), "Jason Statham");
    TS_ASSERT_EQUALS(testee.getInfo1(tx), "REMMLER, 4\xC3\x97""Desintegrator, 6\xC3\x97""Photon Torp");
    TS_ASSERT_EQUALS(testee.getInfo2(tx), "FCode: \"abc\", Damage: 5%");

    // Add some cargo
    testee.change(Element::Tritanium, 10);
    testee.change(Element::Tritanium, 10);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 40);    // -20

    testee.change(Element::Neutronium, 30);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100); // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);   // unchanged
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 40);    // unchanged

    // Commit
    testee.commit();

    TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(0), 40);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Tritanium).orElse(0), 30);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Duranium).orElse(0), 10);
    TS_ASSERT_EQUALS(sh.getCargo(Element::Molybdenum).orElse(0), 10);
}
