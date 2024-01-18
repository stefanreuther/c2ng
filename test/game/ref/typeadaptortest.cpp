/**
  *  \file test/game/ref/typeadaptortest.cpp
  *  \brief Test for game::ref::TypeAdaptor
  */

#include "game/ref/typeadaptor.hpp"

#include "afl/test/testrunner.hpp"
#include "game/ref/list.hpp"

/** Simple functionality test. */
AFL_TEST("game.ref.TypeAdaptor", a)
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
    a.checkEqual("01. countObjects", testee.countObjects(), 3);

    // - forward iteration
    a.checkEqual("11. findNextIndexNoWrap", testee.findNextIndexNoWrap(0, false), 1);
    a.checkEqual("12. findNextIndexNoWrap", testee.findNextIndexNoWrap(1, false), 3);
    a.checkEqual("13. findNextIndexNoWrap", testee.findNextIndexNoWrap(3, false), 4);
    a.checkEqual("14. findNextIndexNoWrap", testee.findNextIndexNoWrap(4, false), 0);

    // - backward iteration
    a.checkEqual("21. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(0, false), 4);
    a.checkEqual("22. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(4, false), 3);
    a.checkEqual("23. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(3, false), 1);
    a.checkEqual("24. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(1, false), 0);

    // - object access
    a.checkNull ("31. getObjectByIndex", testee.getObjectByIndex(0));
    a.checkEqual("32. getObjectByIndex", testee.getObjectByIndex(1), s1);
    a.checkNull ("33. getObjectByIndex", testee.getObjectByIndex(2));
    a.checkEqual("34. getObjectByIndex", testee.getObjectByIndex(3), p7);
    a.checkEqual("35. getObjectByIndex", testee.getObjectByIndex(4), s2);
}
