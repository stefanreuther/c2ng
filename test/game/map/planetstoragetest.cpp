/**
  *  \file test/game/map/planetstoragetest.cpp
  *  \brief Test for game::map::PlanetStorage
  */

#include "game/map/planetstorage.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/planet.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::map::Object;
using game::map::Planet;
using game::test::SimpleTurn;

AFL_TEST("game.map.PlanetStorage", a)
{
    SimpleTurn h;
    Planet& pl = h.addPlanet(99, 5, Object::Playable);
    pl.setName("Cardassia Prime");
    pl.setFriendlyCode(String_t("fgh"));
    afl::string::NullTranslator tx;

    game::map::PlanetStorage testee(pl, h.config());

    /*
     *  Planet has 1000 of each mineral.
     */

    a.checkEqual("01. canHaveElement", testee.canHaveElement(Element::Neutronium), true);
    a.checkEqual("02. canHaveElement", testee.canHaveElement(Element::Molybdenum), true);
    a.checkEqual("03. canHaveElement", testee.canHaveElement(Element::Fighters), false);

    a.check("11. getMaxAmount", testee.getMaxAmount(Element::Neutronium) > 1000000);

    a.checkEqual("21. getAmount", testee.getAmount(Element::Neutronium), 1000);
    a.checkEqual("22. getAmount", testee.getAmount(Element::Tritanium), 1000);
    a.checkEqual("23. getName", testee.getName(tx), "Cardassia Prime");
    a.checkEqual("24. getInfo1", testee.getInfo1(tx), "Planet");
    a.checkEqual("25. getInfo2", testee.getInfo2(tx), "FCode: \"fgh\"");

    // Add some cargo
    testee.change(Element::Tritanium, 10);
    testee.change(Element::Tritanium, 10);
    a.checkEqual("31. getEffectiveAmount", testee.getEffectiveAmount(Element::Tritanium), 1020); // changed
    a.checkEqual("32. getCargo", pl.getCargo(Element::Tritanium).orElse(0), 1000);     // unchanged

    // Commit
    testee.commit();

    a.checkEqual("41. getCargo", pl.getCargo(Element::Tritanium).orElse(0), 1020);
}
