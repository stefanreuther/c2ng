/**
  *  \file u/t_game_map_shipstorage.cpp
  *  \brief Test for game::map::ShipStorage
  */

#include "game/map/shipstorage.hpp"

#include "t_game_map.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::map::Object;
using game::map::Ship;
using game::test::SimpleTurn;

/** Simple test: add some cargo, check that inquiry and commit works ok. */
void
TestGameMapShipStorage::testIt()
{
    SimpleTurn h;
    Ship& sh = h.addShip(10, 5, Object::Playable);

    game::map::ShipStorage testee(sh, h.interface(), h.shipList());

    /*
     *  Ship has a fuel tank of 100 with 10N (=100 max).
     *  Ship has a cargo bay of 100 with 10T, 10D, 10M, 10S, 10C (=60 max of each).
     */

    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Neutronium), 100);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Tritanium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Duranium), 60);
    TS_ASSERT_EQUALS(testee.getMaxAmount(Element::Money), 10000);

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
