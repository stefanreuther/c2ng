/**
  *  \file u/t_game_map_objectlist.cpp
  *  \brief Test for game::map::ObjectList
  */

#include "game/map/objectlist.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/universe.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/planet.hpp"

/** Test behaviour for an empty list. */
void
TestGameMapObjectList::testEmpty()
{
    game::map::ObjectList t;
    TS_ASSERT_EQUALS(t.getNextIndex(0), 0);
    TS_ASSERT_EQUALS(t.getNextIndex(1), 0);
    TS_ASSERT_EQUALS(t.getPreviousIndex(0), 0);
    TS_ASSERT_EQUALS(t.getPreviousIndex(1), 0);
}

/** Test behaviour for a populated list. */
void
TestGameMapObjectList::testContent()
{
    // Create some objects
    game::map::Universe univ;
    game::map::AnyPlanetType ty(univ);

    afl::sys::Log nullLog;
    afl::string::NullTranslator nullTx;
    for (int i = 1; i <= 10; ++i) {
        game::map::Planet* p = univ.planets().create(i);
        p->setPosition(game::map::Point(1000,1000+i));
        p->internalCheck(game::map::Configuration(), nullTx, nullLog);
    }

    // Create a list
    game::map::ObjectList t;
    t.addObject(ty, 1);
    t.addObject(ty, 10);
    t.addObject(game::map::ObjectReference());
    t.addObject(game::map::ObjectReference(ty, 5));
    t.addObject(ty, 3);

    // Iterate
    game::Id_t i = t.getNextIndex(0);
    TS_ASSERT_EQUALS(i, 1);
    TS_ASSERT(t.getObjectByIndex(i) != 0);
    TS_ASSERT_EQUALS(t.getObjectByIndex(i)->getId(), 1);

    i = t.getNextIndex(i);
    TS_ASSERT_EQUALS(i, 2);
    TS_ASSERT(t.getObjectByIndex(i) != 0);
    TS_ASSERT_EQUALS(t.getObjectByIndex(i)->getId(), 10);

    i = t.getNextIndex(i);
    TS_ASSERT_EQUALS(i, 3);
    TS_ASSERT(t.getObjectByIndex(i) == 0);

    i = t.getNextIndex(i);
    TS_ASSERT_EQUALS(i, 4);
    TS_ASSERT(t.getObjectByIndex(i) != 0);
    TS_ASSERT_EQUALS(t.getObjectByIndex(i)->getId(), 5);

    i = t.getNextIndex(i);
    TS_ASSERT_EQUALS(i, 5);
    TS_ASSERT(t.getObjectByIndex(i) != 0);
    TS_ASSERT_EQUALS(t.getObjectByIndex(i)->getId(), 3);

    i = t.getNextIndex(i);
    TS_ASSERT_EQUALS(i, 0);

    // Check index
    // - fourth element is Id 5
    game::map::Object* p = ty.getObjectByIndex(5);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(t.getIndexFor(*p), 4);
    TS_ASSERT_EQUALS(t.getIndexFor(game::map::ObjectReference(ty, 5)), 4);

    // - Id 2 does not appear
    p = ty.getObjectByIndex(2);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(t.getIndexFor(*p), 0);
    TS_ASSERT_EQUALS(t.getIndexFor(game::map::ObjectReference(ty, 2)), 0);

    // - border case
    TS_ASSERT_EQUALS(t.getIndexFor(game::map::ObjectReference()), 3);
}
