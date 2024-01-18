/**
  *  \file test/game/actions/basefixrecycletest.cpp
  *  \brief Test for game::actions::BaseFixRecycle
  */

#include "game/actions/basefixrecycle.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/test/simpleturn.hpp"
#include <stdexcept>

using afl::string::NullTranslator;
using game::actions::BaseFixRecycle;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;
using game::test::SimpleTurn;

namespace {
    Planet& addBase(Planet& p)
    {
        // Create base
        const int owner = p.getOwner().orElse(0);
        p.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(owner));

        // Update m_baseKind
        NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), 15, tx, log);
        return p;
    }
}

/** Test behaviour with no base.
    A: create planet with no base.
    E: creation of BaseFixRecycle fails */
AFL_TEST("game.actions.BaseFixRecycle:error:no-base", a)
{
    // Environment
    SimpleTurn t;
    Planet& p = t.addPlanet(99, 5, Object::Playable);

    // Creation fails
    AFL_CHECK_THROWS(a, (game::actions::BaseFixRecycle(p)), std::exception);
}

/** Test behaviour with no ships.
    A: create base, no played ships, and an entirely unknown ship.
    E: BaseFixRecycle reports no valid actions */
AFL_TEST("game.actions.BaseFixRecycle:no-ships", a)
{
    // Environment
    SimpleTurn t;
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));
    Ship* sh = t.universe().ships().create(77);
    a.checkNonNull("01. sh", sh);

    // No actions reported for ship
    BaseFixRecycle testee(p);
    a.check("11. getValidActions", testee.getValidActions(*sh).empty());
    a.check("12. getValidActions", testee.getValidActions(t.universe()).empty());
    a.check("13. getValidShipIds", testee.getValidShipIds(t.universe(), game::FixShipyardAction).empty());
}

/** Test normal behaviour.
    A: create base and some ships.
    E: BaseFixRecycle reports correct ships for fix/recycle */
AFL_TEST("game.actions.BaseFixRecycle:normal", a)
{
    // Environment
    SimpleTurn t;

    t.setPosition(game::map::Point(1000, 1000));
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));
    Ship& s1 = t.addShip(100, 1, Object::Playable);
    Ship& s2 = t.addShip(200, 5, Object::Playable);
    Ship& s3 = t.addShip(201, 5, Object::Playable);

    t.setPosition(game::map::Point(1200, 1000));
    Ship& s4 = t.addShip(300, 5, Object::Playable);

    // Check actions reported for ship
    BaseFixRecycle testee(p);
    a.check("01. getValidActions", !testee.getValidActions(s1).contains(game::RecycleShipyardAction));
    a.check("02. getValidActions",  testee.getValidActions(s2).contains(game::RecycleShipyardAction));
    a.check("03. getValidActions",  testee.getValidActions(s3).contains(game::RecycleShipyardAction));
    a.check("04. getValidActions", !testee.getValidActions(s4).contains(game::RecycleShipyardAction));

    a.check("11. getValidActions",  testee.getValidActions(s1).contains(game::FixShipyardAction));
    a.check("12. getValidActions",  testee.getValidActions(s2).contains(game::FixShipyardAction));
    a.check("13. getValidActions",  testee.getValidActions(s3).contains(game::FixShipyardAction));
    a.check("14. getValidActions", !testee.getValidActions(s4).contains(game::FixShipyardAction));

    // Check actions reported for universe
    a.check("21. getValidActions",  testee.getValidActions(t.universe()).contains(game::RecycleShipyardAction));
    a.check("22. getValidActions",  testee.getValidActions(t.universe()).contains(game::FixShipyardAction));

    // Check ships for action
    std::vector<game::Id_t> rs = testee.getValidShipIds(t.universe(), game::RecycleShipyardAction);
    a.checkEqual("31. getValidShipIds", rs.size(), 2U);
    a.checkEqual("32. getValidShipIds", rs[0], 200);
    a.checkEqual("33. getValidShipIds", rs[1], 201);
}

/** Test set().
    A: create base and ship. Set an action.
    E: action correctly set */
AFL_TEST("game.actions.BaseFixRecycle:set", a)
{
    // Environment
    SimpleTurn t;
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));
    Ship& sh = t.addShip(100, 1, Object::Playable);

    // Set
    BaseFixRecycle testee(p);
    a.checkEqual("01. set", testee.set(game::FixShipyardAction, t.universe(), &sh), true);

    // Verify status after
    a.checkEqual("11. getBaseShipyardAction", p.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    a.checkEqual("12. getBaseShipyardId", p.getBaseShipyardId().orElse(-1), 100);

    // Reset
    a.checkEqual("21. set", testee.set(game::NoShipyardAction, t.universe(), 0), true);
    a.checkEqual("22. getBaseShipyardAction", p.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    a.checkEqual("23. getBaseShipyardId", p.getBaseShipyardId().orElse(-1), 0);
}

/** Test set() failure.
    A: create base and ship at different positions. Set an action.
    E: action correctly refused */
AFL_TEST("game.actions.BaseFixRecycle:set:fail", a)
{
    // Environment
    SimpleTurn t;

    t.setPosition(game::map::Point(1000, 1000));
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));

    t.setPosition(game::map::Point(1200, 1000));
    Ship& sh = t.addShip(300, 5, Object::Playable);

    p.setBaseShipyardOrder(game::NoShipyardAction, 0);

    // Set -> fails
    BaseFixRecycle testee(p);
    a.checkEqual("01. set", testee.set(game::FixShipyardAction, t.universe(), &sh), false);

    // Verify status after: unchanged
    a.checkEqual("11. getBaseShipyardAction", p.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    a.checkEqual("12. getBaseShipyardId", p.getBaseShipyardId().orElse(-1), 0);
}
