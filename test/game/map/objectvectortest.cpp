/**
  *  \file test/game/map/objectvectortest.cpp
  *  \brief Test for game::map::ObjectVector
  */

#include "game/map/objectvector.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    int numLiveObjects;
    struct Tester {
        Tester(game::Id_t id)
            : id(id)
            { ++numLiveObjects; }
        ~Tester()
            { --numLiveObjects; }
        game::Id_t id;
    };
}

AFL_TEST("game.map.ObjectVector", a)
{
    game::map::ObjectVector<Tester> t;

    // Creation, success cases
    a.checkNonNull("01. create", t.create(1));
    a.checkNonNull("02. create", t.create(5));
    a.checkNonNull("03. create", t.create(6));

    // Creation, failure cases
    a.checkNull("11. create", t.create(0));
    a.checkNull("12. create", t.create(-1));

    // Access
    a.checkNull("21. get", t.get(-1));
    a.checkNull("22. get", t.get(0));
    a.checkNonNull("23. get", t.get(1));
    a.checkNull("24. get", t.get(2));
    a.checkNull("25. get", t.get(3));
    a.checkNull("26. get", t.get(4));
    a.checkNonNull("27. get", t.get(5));
    a.checkNonNull("28. get", t.get(6));
    a.checkNull("29. get", t.get(7));

    a.checkEqual("31. size", t.size(), 6);
    a.checkEqual("32. numLiveObjects", numLiveObjects, 3);

    a.checkEqual("41. get", t.get(1)->id, 1);
    a.checkEqual("42. get", t.get(5)->id, 5);
    a.checkEqual("43. get", t.get(6)->id, 6);

    // Clear
    t.clear();
    a.checkEqual("51. size", t.size(), 0);
    a.checkEqual("52. numLiveObjects", numLiveObjects, 0);
    a.checkNull("53. get", t.get(1));
}
