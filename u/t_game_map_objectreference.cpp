/**
  *  \file u/t_game_map_objectreference.cpp
  *  \brief Test for game::map::ObjectReference
  */

#include "game/map/objectreference.hpp"

#include "t_game_map.hpp"
#include "game/map/universe.hpp"
#include "game/map/anyplanettype.hpp"
#include "game/map/anyshiptype.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"

/** Test comparisons.
    Comparison results are not affected by whether the pointed-to objects exist, so we don't have to create some. */
void
TestGameMapObjectReference::testCompare()
{
    game::map::Universe univ;
    game::map::AnyPlanetType ty(univ);
    game::map::AnyShipType ty2(univ);

    game::map::ObjectReference a;
    game::map::ObjectReference b(ty, 1);
    game::map::ObjectReference c(ty, 42);
    game::map::ObjectReference d(ty2, 1);

    TS_ASSERT(a == a);
    TS_ASSERT(a != b);
    TS_ASSERT(a != c);
    TS_ASSERT(a != d);

    TS_ASSERT(b != a);
    TS_ASSERT(b == b);
    TS_ASSERT(b != c);
    TS_ASSERT(b != d);

    TS_ASSERT(c != a);
    TS_ASSERT(c != b);
    TS_ASSERT(c == c);
    TS_ASSERT(c != d);

    TS_ASSERT(d != a);
    TS_ASSERT(d != b);
    TS_ASSERT(d != c);
    TS_ASSERT(d == d);
}

/** Test accessors. */
void
TestGameMapObjectReference::testAccessor()
{
    game::map::Universe univ;
    game::map::AnyPlanetType ty(univ);

    game::map::ObjectReference a;
    game::map::ObjectReference b(ty, 1);
    game::map::ObjectReference c(ty, 42);

    // Create a planet
    afl::sys::Log nullLog;
    afl::string::NullTranslator nullTx;
    game::map::Planet* p = univ.planets().create(42);
    p->setPosition(game::map::Point(1000,1000));
    p->internalCheck(game::map::Configuration(), nullTx, nullLog);

    // Test validity
    TS_ASSERT(!a.isValid());
    TS_ASSERT(!b.isValid());
    TS_ASSERT(c.isValid());

    // Test indexes
    TS_ASSERT_EQUALS(a.getObjectIndex(), 0);
    TS_ASSERT_EQUALS(b.getObjectIndex(), 1);
    TS_ASSERT_EQUALS(c.getObjectIndex(), 42);

    // Test object
    TS_ASSERT(a.get() == 0);
    TS_ASSERT(b.get() == 0);
    TS_ASSERT(c.get() == p);

    // Test type
    TS_ASSERT(a.getObjectType() == 0);
    TS_ASSERT(b.getObjectType() == &ty);
    TS_ASSERT(c.getObjectType() == &ty);

    // Test universe
    TS_ASSERT(a.getUniverse() == 0);
    // b is unspecified
    TS_ASSERT(c.getUniverse() == &univ);
}
