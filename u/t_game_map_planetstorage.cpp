/**
  *  \file u/t_game_map_planetstorage.cpp
  *  \brief Test for game::map::PlanetStorage
  */

#include "game/map/planetstorage.hpp"

#include "t_game_map.hpp"
#include "game/test/simpleturn.hpp"
#include "game/map/planet.hpp"
#include "afl/string/nulltranslator.hpp"

using game::Element;
using game::map::Object;
using game::map::Planet;
using game::test::SimpleTurn;

void
TestGameMapPlanetStorage::testPlanet()
{
    SimpleTurn h;
    Planet& pl = h.addPlanet(99, 5, Object::Playable);
    pl.setName("Cardassia Prime");
    afl::string::NullTranslator tx;

    game::map::PlanetStorage testee(pl, h.config());

    /*
     *  Planet has 1000 of each mineral.
     */

    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Neutronium), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Molybdenum), true);
    TS_ASSERT_EQUALS(testee.canHaveElement(Element::Fighters), false);

    TS_ASSERT(testee.getMaxAmount(Element::Neutronium) > 1000000);

    TS_ASSERT_EQUALS(testee.getAmount(Element::Neutronium), 1000);
    TS_ASSERT_EQUALS(testee.getAmount(Element::Tritanium), 1000);
    TS_ASSERT_EQUALS(testee.getName(tx), "Cardassia Prime");

    // Add some cargo
    testee.change(Element::Tritanium, 10);
    testee.change(Element::Tritanium, 10);
    TS_ASSERT_EQUALS(testee.getEffectiveAmount(Element::Tritanium), 1020); // changed
    TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(0), 1000);     // unchanged

    // Commit
    testee.commit();

    TS_ASSERT_EQUALS(pl.getCargo(Element::Tritanium).orElse(0), 1020);
}

