/**
  *  \file test/game/elementtest.cpp
  *  \brief Test for game::Element
  */

#include "game/element.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"

/** Test operators. */
AFL_TEST("game.Element:operator", a)
{
    game::Element::Type t = game::Element::Tritanium;

    // Postincrement
    {
        game::Element::Type x = t++;
        a.checkEqual("01", x, game::Element::Tritanium);
        a.checkEqual("02", t, game::Element::Duranium);
    }

    // Preincrement
    {
        game::Element::Type x = ++t;
        a.checkEqual("11", x, game::Element::Molybdenum);
        a.checkEqual("12", t, game::Element::Molybdenum);
    }

    // Postdecrement
    {
        game::Element::Type x = t--;
        a.checkEqual("21", x, game::Element::Molybdenum);
        a.checkEqual("22", t, game::Element::Duranium);
    }

    // Predecrement
    {
        game::Element::Type x = --t;
        a.checkEqual("31", x, game::Element::Tritanium);
        a.checkEqual("32", t, game::Element::Tritanium);
    }
}

/** Test conversion to torpedoes. */
AFL_TEST("game.Element:torpedo", a)
{
    int n;
    a.check("01. isTorpedoType", !game::Element::isTorpedoType(game::Element::Tritanium, n));
    a.check("02. isTorpedoType", !game::Element::isTorpedoType(game::Element::Neutronium, n));
    a.check("03. isTorpedoType", !game::Element::isTorpedoType(game::Element::Money, n));

    for (int i = 1; i <= 10; ++i) {
        game::Element::Type t = game::Element::fromTorpedoType(i);
        a.check("11. isTorpedoType", game::Element::isTorpedoType(t, n));
        a.checkEqual("12. torpedo type", n, i);
    }
}

/** Test iteration. */
AFL_TEST("game.Element:iteration", a)
{
    using game::Element;
    game::spec::ShipList sl;
    sl.launchers().create(1);
    sl.launchers().create(2);

    bool sawN = false;
    bool sawMC = false;
    bool sawColonists = false;
    bool sawTorp1 = false;
    bool sawTorp2 = false;

    for (Element::Type t = Element::begin(), e = Element::end(sl); t != e; ++t) {
        int tt;
        if (t == Element::Neutronium) {
            a.check("01. Neutronium", !sawN);
            sawN = true;
        } else if (t == Element::Money) {
            a.check("02. Money", !sawMC);
            sawMC = true;
        } else if (t == Element::Colonists) {
            a.check("03. Colonists", !sawColonists);
            sawColonists = true;
        } else if (Element::isTorpedoType(t, tt)) {
            if (tt == 1) {
                a.check("04. Torp 1", !sawTorp1);
                sawTorp1 = true;
            }
            if (tt == 2) {
                a.check("05. Torp 2", !sawTorp2);
                sawTorp2 = true;
            }
        } else {
            // ok
        }
    }

    a.check("11. Neutronium", sawN);
    a.check("12. Money", sawMC);
    a.check("13. Colonists", sawColonists);
    a.check("14. Torp 1", sawTorp1);
    a.check("15. Torp 2", sawTorp2);
}

/** Test getName, getUnit. */
AFL_TEST("game.Element:names", a)
{
    using game::Element;
    game::spec::ShipList sl;
    sl.launchers().create(1)->setName("One");
    sl.launchers().create(2)->setName("Two");

    afl::string::NullTranslator tx;

    a.checkEqual("01", Element::getName(Element::Neutronium, tx, sl), "Neutronium");
    a.checkEqual("02", Element::getUnit(Element::Neutronium, tx, sl), "kt");

    a.checkEqual("11", Element::getName(Element::Supplies, tx, sl), "Supplies");
    a.checkEqual("12", Element::getUnit(Element::Supplies, tx, sl), "kt");

    a.checkEqual("21", Element::getName(Element::Colonists, tx, sl), "Colonists");
    a.checkEqual("22", Element::getUnit(Element::Colonists, tx, sl), "clans");

    a.checkEqual("31", Element::getName(Element::Fighters, tx, sl), "Fighters");
    a.checkEqual("32", Element::getUnit(Element::Fighters, tx, sl), "");

    a.checkEqual("41", Element::getName(Element::Money, tx, sl), "Money");
    a.checkEqual("42", Element::getUnit(Element::Money, tx, sl), "mc");

    a.checkEqual("51", Element::getName(Element::fromTorpedoType(1), tx, sl), "One");
    a.checkEqual("52", Element::getUnit(Element::fromTorpedoType(1), tx, sl), "");

    a.checkEqual("61", Element::getName(Element::fromTorpedoType(2), tx, sl), "Two");
    a.checkEqual("62", Element::getUnit(Element::fromTorpedoType(2), tx, sl), "");

    a.checkEqual("71", Element::getName(Element::fromTorpedoType(9), tx, sl), "");
    a.checkEqual("72", Element::getUnit(Element::fromTorpedoType(9), tx, sl), "");

    // All elements in an iteration must have a name
    for (Element::Type t = Element::begin(), e = Element::end(sl); t != e; ++t) {
        a.check("81", !Element::getName(t, tx, sl).empty());
    }
}
