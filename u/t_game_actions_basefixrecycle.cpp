/**
  *  \file u/t_game_actions_basefixrecycle.cpp
  *  \brief Test for game::actions::BaseFixRecycle
  */

#include <stdexcept>
#include "game/actions/basefixrecycle.hpp"

#include "t_game_actions.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/test/simpleturn.hpp"

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
void
TestGameActionsBaseFixRecycle::testNoBase()
{
    // Environment
    SimpleTurn t;
    Planet& p = t.addPlanet(99, 5, Object::Playable);

    // Creation fails
    TS_ASSERT_THROWS((game::actions::BaseFixRecycle(p)), std::exception);
}

/** Test behaviour with no ships.
    A: create base, no played ships, and an entirely unknown ship.
    E: BaseFixRecycle reports no valid actions */
void
TestGameActionsBaseFixRecycle::testEmpty()
{
    // Environment
    SimpleTurn t;
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));
    Ship* sh = t.universe().ships().create(77);
    TS_ASSERT(sh);

    // No actions reported for ship
    BaseFixRecycle testee(p);
    TS_ASSERT(testee.getValidActions(*sh).empty());
    TS_ASSERT(testee.getValidActions(t.universe()).empty());
    TS_ASSERT(testee.getValidShipIds(t.universe(), game::FixShipyardAction).empty());
}

/** Test normal behaviour.
    A: create base and some ships.
    E: BaseFixRecycle reports correct ships for fix/recycle */
void
TestGameActionsBaseFixRecycle::testNormal()
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
    TS_ASSERT(!testee.getValidActions(s1).contains(game::RecycleShipyardAction));
    TS_ASSERT( testee.getValidActions(s2).contains(game::RecycleShipyardAction));
    TS_ASSERT( testee.getValidActions(s3).contains(game::RecycleShipyardAction));
    TS_ASSERT(!testee.getValidActions(s4).contains(game::RecycleShipyardAction));

    TS_ASSERT( testee.getValidActions(s1).contains(game::FixShipyardAction));
    TS_ASSERT( testee.getValidActions(s2).contains(game::FixShipyardAction));
    TS_ASSERT( testee.getValidActions(s3).contains(game::FixShipyardAction));
    TS_ASSERT(!testee.getValidActions(s4).contains(game::FixShipyardAction));

    // Check actions reported for universe
    TS_ASSERT( testee.getValidActions(t.universe()).contains(game::RecycleShipyardAction));
    TS_ASSERT( testee.getValidActions(t.universe()).contains(game::FixShipyardAction));

    // Check ships for action
    std::vector<game::Id_t> rs = testee.getValidShipIds(t.universe(), game::RecycleShipyardAction);
    TS_ASSERT_EQUALS(rs.size(), 2U);
    TS_ASSERT_EQUALS(rs[0], 200);
    TS_ASSERT_EQUALS(rs[1], 201);
}

/** Test set().
    A: create base and ship. Set an action.
    E: action correctly set */
void
TestGameActionsBaseFixRecycle::testSet()
{
    // Environment
    SimpleTurn t;
    Planet& p = addBase(t.addPlanet(99, 5, Object::Playable));
    Ship& sh = t.addShip(100, 1, Object::Playable);

    // Set
    BaseFixRecycle testee(p);
    TS_ASSERT_EQUALS(testee.set(game::FixShipyardAction, t.universe(), &sh), true);

    // Verify status after
    TS_ASSERT_EQUALS(p.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    TS_ASSERT_EQUALS(p.getBaseShipyardId().orElse(-1), 100);

    // Reset
    TS_ASSERT_EQUALS(testee.set(game::NoShipyardAction, t.universe(), 0), true);
    TS_ASSERT_EQUALS(p.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    TS_ASSERT_EQUALS(p.getBaseShipyardId().orElse(-1), 0);
}

/** Test set() failure.
    A: create base and ship at different positions. Set an action.
    E: action correctly refused */
void
TestGameActionsBaseFixRecycle::testSetFail()
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
    TS_ASSERT_EQUALS(testee.set(game::FixShipyardAction, t.universe(), &sh), false);

    // Verify status after: unchanged
    TS_ASSERT_EQUALS(p.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    TS_ASSERT_EQUALS(p.getBaseShipyardId().orElse(-1), 0);
}

