/**
  *  \file u/t_game_map_objectvector.cpp
  *  \brief Test for game::map::ObjectVector
  */

#include "game/map/objectvector.hpp"

#include "t_game_map.hpp"

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

void
TestGameMapObjectVector::testIt()
{
    game::map::ObjectVector<Tester> t;

    // Creation, success cases
    TS_ASSERT(t.create(1) != 0);
    TS_ASSERT(t.create(5) != 0);
    TS_ASSERT(t.create(6) != 0);

    // Creation, failure cases
    TS_ASSERT(t.create(0) == 0);
    TS_ASSERT(t.create(-1) == 0);

    // Access
    TS_ASSERT(t.get(-1) == 0);
    TS_ASSERT(t.get(0) == 0);
    TS_ASSERT(t.get(1) != 0);
    TS_ASSERT(t.get(2) == 0);
    TS_ASSERT(t.get(3) == 0);
    TS_ASSERT(t.get(4) == 0);
    TS_ASSERT(t.get(5) != 0);
    TS_ASSERT(t.get(6) != 0);
    TS_ASSERT(t.get(7) == 0);

    TS_ASSERT_EQUALS(t.size(), 6);
    TS_ASSERT_EQUALS(numLiveObjects, 3);

    TS_ASSERT_EQUALS(t.get(1)->id, 1);
    TS_ASSERT_EQUALS(t.get(5)->id, 5);
    TS_ASSERT_EQUALS(t.get(6)->id, 6);

    // Clear
    t.clear();
    TS_ASSERT_EQUALS(t.size(), 0);
    TS_ASSERT_EQUALS(numLiveObjects, 0);
    TS_ASSERT(t.get(1) == 0);
}

