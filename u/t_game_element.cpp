/**
  *  \file u/t_game_element.cpp
  *  \brief Test for game::Element
  */

#include "game/element.hpp"

#include "t_game.hpp"

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
