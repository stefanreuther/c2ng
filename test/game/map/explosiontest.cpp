/**
  *  \file test/game/map/explosiontest.cpp
  *  \brief Test for game::map::Explosion
  */

#include "game/map/explosion.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/interpreterinterface.hpp"

using game::map::Explosion;
using game::map::Point;

/** Test initialisation and getters. */
AFL_TEST("game.map.Explosion:init", a)
{
    Explosion t(30, Point(20,30));

    a.checkEqual("01. getId", t.getId(), 30);
    a.checkEqual("02. getShipId", t.getShipId(), 0);
    a.checkEqual("03. getShipName", t.getShipName(), "");

    Point pt;
    a.checkEqual("11. getPosition", t.getPosition().get(pt), true);
    a.checkEqual("12. pt", pt, Point(20, 30));

    int n;
    a.checkEqual("21. getOwner", t.getOwner().get(n), true);
    a.checkEqual("22. owner", n, 0);
}

/** Test getName().
    A: create explosions with different ship name/Id. Call getName().
    E: correct names generated */

// Default
AFL_TEST("game.map.Explosion:getName:default", a)
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;
    Explosion t(0, Point(1,1));
    a.checkEqual("", t.getName(game::PlainName, tx, iface), "Explosion");
}

// Just a ship name
AFL_TEST("game.map.Explosion:getName:just-ship-name", a)
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;
    Explosion t(0, Point(1,1));
    t.setShipName("USS Dull");
    a.checkEqual("", t.getName(game::PlainName, tx, iface), "Explosion of USS Dull");
}

// Just a ship Id
AFL_TEST("game.map.Explosion:getName:just-ship-id", a)
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;
    Explosion t(0, Point(1,1));
    t.setShipId(42);
    a.checkEqual("", t.getName(game::PlainName, tx, iface), "Explosion of ship #42");
}

// Name and Id
AFL_TEST("game.map.Explosion:getName:name-and-id", a)
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;
    Explosion t(0, Point(1,1));
    t.setShipName("USS Dull");
    t.setShipId(42);
    a.checkEqual("", t.getName(game::PlainName, tx, iface), "Explosion of USS Dull (#42)");
}


/** Test merge(), failure cases. */

// Different location
AFL_TEST("game.map.Explosion:merge:fail:different-position", a)
{
    Explosion ax(0, Point(1,1));
    Explosion bx(0, Point(2,2));
    a.checkEqual("", ax.merge(bx), false);
}

// Different Id
AFL_TEST("game.map.Explosion:merge:fail:different-id", a)
{
    Explosion ax(1, Point(1,1));
    Explosion bx(2, Point(1,1));
    a.checkEqual("", ax.merge(bx), false);
}

// Different ship name
AFL_TEST("game.map.Explosion:merge:fail:different-name", a)
{
    Explosion ax(1, Point(1,1));
    ax.setShipName("Excelsior");
    Explosion bx(1, Point(1,1));
    bx.setShipName("Enterprise");
    a.checkEqual("", ax.merge(bx), false);
}

// Different ship Id, with name
AFL_TEST("game.map.Explosion:merge:fail:different-id-same-name", a)
{
    Explosion ax(1, Point(1,1));
    ax.setShipName("Scout");
    ax.setShipId(10);
    Explosion bx(1, Point(1,1));
    bx.setShipName("Scout");
    bx.setShipId(20);
    a.checkEqual("", ax.merge(bx), false);
}

// Different ship Id, without name
AFL_TEST("game.map.Explosion:merge:fail:different-ship-id", a)
{
    Explosion ax(1, Point(1,1));
    ax.setShipId(10);
    Explosion bx(1, Point(1,1));
    bx.setShipId(20);
    a.checkEqual("", ax.merge(bx), false);
}

/** Test merge(), success cases. */

// Name taken over
AFL_TEST("game.map.Explosion:merge:copy-name", a)
{
    Explosion ax(1, Point(1,1));
    Explosion bx(1, Point(1,1));
    bx.setShipName("Scout");
    bx.setShipId(20);
    a.checkEqual("merge",       ax.merge(bx), true);
    a.checkEqual("getShipId",   ax.getShipId(), 20);
    a.checkEqual("getShipName", ax.getShipName(), "Scout");
}

// Ship Id taken over, name kept
AFL_TEST("game.map.Explosion:merge:keep-name", a)
{
    Explosion ax(1, Point(1,1));
    ax.setShipName("Dreadnought");
    Explosion bx(1, Point(1,1));
    bx.setShipId(20);
    a.checkEqual("merge",       ax.merge(bx), true);
    a.checkEqual("getShipId",   ax.getShipId(), 20);
    a.checkEqual("getShipName", ax.getShipName(), "Dreadnought");
}

// Explosion Id taken over
AFL_TEST("game.map.Explosion:merge:copy-id", a)
{
    Explosion ax(0, Point(1,1));
    Explosion bx(50, Point(1,1));
    a.checkEqual("merge", ax.merge(bx), true);
    a.checkEqual("getId", ax.getId(), 50);
}

// Meta-information from a Id-less explosion added to one with Id
AFL_TEST("game.map.Explosion:merge:copy-meta", a)
{
    Explosion ax(15, Point(1,1));
    Explosion bx(0, Point(1,1));
    bx.setShipName("Scout");
    bx.setShipId(20);
    a.checkEqual("merge",       ax.merge(bx), true);
    a.checkEqual("getId",       ax.getId(), 15);
    a.checkEqual("getShipId",   ax.getShipId(), 20);
    a.checkEqual("getShipName", ax.getShipName(), "Scout");
}
