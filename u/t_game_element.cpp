/**
  *  \file u/t_game_element.cpp
  *  \brief Test for game::Element
  */

#include "game/element.hpp"

#include "t_game.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test operators. */
void
TestGameElement::testOperator()
{
    game::Element::Type t = game::Element::Tritanium;

    // Postincrement
    {
        game::Element::Type x = t++;
        TS_ASSERT_EQUALS(x, game::Element::Tritanium);
        TS_ASSERT_EQUALS(t, game::Element::Duranium);
    }

    // Preincrement
    {
        game::Element::Type x = ++t;
        TS_ASSERT_EQUALS(x, game::Element::Molybdenum);
        TS_ASSERT_EQUALS(t, game::Element::Molybdenum);
    }

    // Postdecrement
    {
        game::Element::Type x = t--;
        TS_ASSERT_EQUALS(x, game::Element::Molybdenum);
        TS_ASSERT_EQUALS(t, game::Element::Duranium);
    }

    // Predecrement
    {
        game::Element::Type x = --t;
        TS_ASSERT_EQUALS(x, game::Element::Tritanium);
        TS_ASSERT_EQUALS(t, game::Element::Tritanium);
    }
}

/** Test conversion to torpedoes. */
void
TestGameElement::testTorpedo()
{
    int n;
    TS_ASSERT(!game::Element::isTorpedoType(game::Element::Tritanium, n));
    TS_ASSERT(!game::Element::isTorpedoType(game::Element::Neutronium, n));
    TS_ASSERT(!game::Element::isTorpedoType(game::Element::Money, n));

    for (int i = 1; i <= 10; ++i) {
        game::Element::Type t = game::Element::fromTorpedoType(i);
        TS_ASSERT(game::Element::isTorpedoType(t, n));
        TS_ASSERT_EQUALS(n, i);
    }
}

/** Test iteration. */
void
TestGameElement::testIteration()
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
            TS_ASSERT(!sawN);
            sawN = true;
        } else if (t == Element::Money) {
            TS_ASSERT(!sawMC);
            sawMC = true;
        } else if (t == Element::Colonists) {
            TS_ASSERT(!sawColonists);
            sawColonists = true;
        } else if (Element::isTorpedoType(t, tt)) {
            if (tt == 1) {
                TS_ASSERT(!sawTorp1);
                sawTorp1 = true;
            }
            if (tt == 2) {
                TS_ASSERT(!sawTorp2);
                sawTorp2 = true;
            }
        } else {
            // ok
        }
    }

    TS_ASSERT(sawN);
    TS_ASSERT(sawMC);
    TS_ASSERT(sawColonists);
    TS_ASSERT(sawTorp1);
    TS_ASSERT(sawTorp2);
}

/** Test getName, getUnit. */
void
TestGameElement::testName()
{
    using game::Element;
    game::spec::ShipList sl;
    sl.launchers().create(1)->setName("One");
    sl.launchers().create(2)->setName("Two");

    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(Element::getName(Element::Neutronium, tx, sl), "Neutronium");
    TS_ASSERT_EQUALS(Element::getUnit(Element::Neutronium, tx, sl), "kt");

    TS_ASSERT_EQUALS(Element::getName(Element::Supplies, tx, sl), "Supplies");
    TS_ASSERT_EQUALS(Element::getUnit(Element::Supplies, tx, sl), "kt");

    TS_ASSERT_EQUALS(Element::getName(Element::Colonists, tx, sl), "Colonists");
    TS_ASSERT_EQUALS(Element::getUnit(Element::Colonists, tx, sl), "clans");

    TS_ASSERT_EQUALS(Element::getName(Element::Fighters, tx, sl), "Fighters");
    TS_ASSERT_EQUALS(Element::getUnit(Element::Fighters, tx, sl), "");

    TS_ASSERT_EQUALS(Element::getName(Element::Money, tx, sl), "Money");
    TS_ASSERT_EQUALS(Element::getUnit(Element::Money, tx, sl), "mc");

    TS_ASSERT_EQUALS(Element::getName(Element::fromTorpedoType(1), tx, sl), "One");
    TS_ASSERT_EQUALS(Element::getUnit(Element::fromTorpedoType(1), tx, sl), "");

    TS_ASSERT_EQUALS(Element::getName(Element::fromTorpedoType(2), tx, sl), "Two");
    TS_ASSERT_EQUALS(Element::getUnit(Element::fromTorpedoType(2), tx, sl), "");

    TS_ASSERT_EQUALS(Element::getName(Element::fromTorpedoType(9), tx, sl), "");
    TS_ASSERT_EQUALS(Element::getUnit(Element::fromTorpedoType(9), tx, sl), "");

    // All elements in an iteration must have a name
    for (Element::Type t = Element::begin(), e = Element::end(sl); t != e; ++t) {
        TS_ASSERT(!Element::getName(t, tx, sl).empty());
    }
}

