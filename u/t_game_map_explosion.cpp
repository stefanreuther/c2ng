/**
  *  \file u/t_game_map_explosion.cpp
  *  \brief Test for game::map::Explosion
  */

#include "game/map/explosion.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/interpreterinterface.hpp"

using game::map::Explosion;
using game::map::Point;

/** Test initialisation and getters. */
void
TestGameMapExplosion::testInit()
{
    Explosion t(30, Point(20,30));

    TS_ASSERT_EQUALS(t.getId(), 30);
    TS_ASSERT_EQUALS(t.getShipId(), 0);
    TS_ASSERT_EQUALS(t.getShipName(), "");

    Point pt;
    TS_ASSERT_EQUALS(t.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt, Point(20, 30));

    int n;
    TS_ASSERT_EQUALS(t.getOwner().get(n), true);
    TS_ASSERT_EQUALS(n, 0);
}

/** Test getName().
    A: create explosions with different ship name/Id. Call getName().
    E: correct names generated */
void
TestGameMapExplosion::testName()
{
    // Infrastructure
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;

    // Default
    {
        Explosion t(0, Point(1,1));
        TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Explosion");
    }

    // Just a ship name
    {
        Explosion t(0, Point(1,1));
        t.setShipName("USS Dull");
        TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Explosion of USS Dull");
    }

    // Just a ship Id
    {
        Explosion t(0, Point(1,1));
        t.setShipId(42);
        TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Explosion of ship #42");
    }

    // Name and Id
    {
        Explosion t(0, Point(1,1));
        t.setShipName("USS Dull");
        t.setShipId(42);
        TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Explosion of USS Dull (#42)");
    }
}

/** Test merge(), failure cases. */
void
TestGameMapExplosion::testMergeFail()
{
    // Different location
    {
        Explosion a(0, Point(1,1));
        Explosion b(0, Point(2,2));
        TS_ASSERT_EQUALS(a.merge(b), false);
    }

    // Different Id
    {
        Explosion a(1, Point(1,1));
        Explosion b(2, Point(1,1));
        TS_ASSERT_EQUALS(a.merge(b), false);
    }

    // Different ship name
    {
        Explosion a(1, Point(1,1));
        a.setShipName("Excelsior");
        Explosion b(1, Point(1,1));
        b.setShipName("Enterprise");
        TS_ASSERT_EQUALS(a.merge(b), false);
    }

    // Different ship Id, with name
    {
        Explosion a(1, Point(1,1));
        a.setShipName("Scout");
        a.setShipId(10);
        Explosion b(1, Point(1,1));
        b.setShipName("Scout");
        b.setShipId(20);
        TS_ASSERT_EQUALS(a.merge(b), false);
    }

    // Different ship Id, without name
    {
        Explosion a(1, Point(1,1));
        a.setShipId(10);
        Explosion b(1, Point(1,1));
        b.setShipId(20);
        TS_ASSERT_EQUALS(a.merge(b), false);
    }
}

/** Test merge(), success cases. */
void
TestGameMapExplosion::testMergeSuccess()
{
    // Name taken over
    {
        Explosion a(1, Point(1,1));
        Explosion b(1, Point(1,1));
        b.setShipName("Scout");
        b.setShipId(20);
        TS_ASSERT_EQUALS(a.merge(b), true);
        TS_ASSERT_EQUALS(a.getShipId(), 20);
        TS_ASSERT_EQUALS(a.getShipName(), "Scout");
    }

    // Ship Id taken over, name kept
    {
        Explosion a(1, Point(1,1));
        a.setShipName("Dreadnought");
        Explosion b(1, Point(1,1));
        b.setShipId(20);
        TS_ASSERT_EQUALS(a.merge(b), true);
        TS_ASSERT_EQUALS(a.getShipId(), 20);
        TS_ASSERT_EQUALS(a.getShipName(), "Dreadnought");
    }

    // Explosion Id taken over
    {
        Explosion a(0, Point(1,1));
        Explosion b(50, Point(1,1));
        TS_ASSERT_EQUALS(a.merge(b), true);
        TS_ASSERT_EQUALS(a.getId(), 50);
    }

    // Meta-information from a Id-less explosion added to one with Id
    {
        Explosion a(15, Point(1,1));
        Explosion b(0, Point(1,1));
        b.setShipName("Scout");
        b.setShipId(20);
        TS_ASSERT_EQUALS(a.merge(b), true);
        TS_ASSERT_EQUALS(a.getId(), 15);
        TS_ASSERT_EQUALS(a.getShipId(), 20);
        TS_ASSERT_EQUALS(a.getShipName(), "Scout");
    }
}

