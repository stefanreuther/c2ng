/**
  *  \file u/t_game_ref_typeadaptor.cpp
  *  \brief Test for game::ref::TypeAdaptor
  */

#include "game/ref/typeadaptor.hpp"

#include "t_game_ref.hpp"
#include "game/ref/list.hpp"

/** Simple functionality test. */
void
TestGameRefTypeAdaptor::testIt()
{
    // Universe
    game::map::Universe univ;
    game::map::Ship* s1 = univ.ships().create(1);
    game::map::Ship* s2 = univ.ships().create(2);
    game::map::Planet* p7 = univ.planets().create(7);

    // List
    game::ref::List list;
    list.add(game::Reference(game::Reference::Ship, 1));
    list.add(game::Reference(game::Reference::Ship, 99));     // nonexistant ship
    list.add(game::Reference(game::Reference::Planet, 7));
    list.add(game::Reference(game::Reference::Ship, 2));

    // Verify
    game::ref::TypeAdaptor testee(list, univ);

    // - count
    TS_ASSERT_EQUALS(testee.countObjects(), 3);

    // - forward iteration
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(0, false), 1);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(1, false), 3);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(3, false), 4);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(4, false), 0);

    // - backward iteration
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(0, false), 4);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(4, false), 3);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(3, false), 1);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(1, false), 0);

    // - object access
    TS_ASSERT(testee.getObjectByIndex(0) == 0);
    TS_ASSERT_EQUALS(testee.getObjectByIndex(1), s1);
    TS_ASSERT(testee.getObjectByIndex(2) == 0);
    TS_ASSERT_EQUALS(testee.getObjectByIndex(3), p7);
    TS_ASSERT_EQUALS(testee.getObjectByIndex(4), s2);
}

