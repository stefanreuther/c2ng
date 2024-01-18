/**
  *  \file test/game/map/basestoragetest.cpp
  *  \brief Test for game::map::BaseStorage
  */

#include "game/map/basestorage.hpp"
#include "afl/test/testrunner.hpp"

/** Test general element access. */
AFL_TEST("game.map.BaseStorage:general", a)
{
    game::map::BaseStorage testee;

    // Initial value: everything is invalid
    a.check("01. isValid", !testee.get(0).isValid());
    a.check("02. isValid", !testee.get(1).isValid());
    a.check("03. isValid", !testee.get(2).isValid());
    a.check("04. isValid", !testee.get(3).isValid());
    a.check("05. isValid", !testee.get(4).isValid());

    // Set some values
    testee.set(0, 66);
    testee.set(1, 77);
    testee.set(3, 88);

    // Read back
    a.check("11. isValid", !testee.get(0).isValid());
    a.check("12. isValid",  testee.get(1).isValid());
    a.check("13. isValid", !testee.get(2).isValid());
    a.check("14. isValid",  testee.get(3).isValid());
    a.check("15. isValid", !testee.get(4).isValid());

    // Element access
    a.checkNull("21. at", testee.at(0));
    a.checkNonNull("22. at", testee.at(1));
    a.checkNonNull("23. at", testee.at(2));
    a.checkNonNull("24. at", testee.at(3));
    a.checkNull("25. at", testee.at(4));

    // Size access: maximum element we set is 3
    a.checkEqual("31. size", testee.size(), 4);
}

/** Test isValid(). */
AFL_TEST("game.map.BaseStorage:isValid", a)
{
    game::map::BaseStorage testee;
    a.check("01. isValid", !testee.isValid());

    testee.set(3, 7);
    a.check("11. isValid", testee.isValid());

    testee.set(3, afl::base::Nothing);
    a.check("21. isValid", !testee.isValid());

    testee.clear();
    a.check("31. isValid", !testee.isValid());
}

/** Test clear(). */
AFL_TEST("game.map.BaseStorage:clear", a)
{
    game::map::BaseStorage testee;

    // Initial value: everything is invalid
    a.check("01. isValid", !testee.get(0).isValid());
    a.check("02. isValid", !testee.get(1).isValid());
    a.check("03. isValid", !testee.get(2).isValid());

    // Set value
    testee.set(1, 77);
    a.checkEqual("11. get", testee.get(1).orElse(-1), 77);

    // Element access
    testee.clear();
    a.checkEqual("21. get", testee.get(1).orElse(-1), -1);
}
