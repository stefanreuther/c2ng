/**
  *  \file u/t_game_map_markings.cpp
  *  \brief Test for game::map::Markings
  */

#include "game/map/markings.hpp"

#include "t_game_map.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/selectionexpression.hpp"

namespace {
    game::map::Planet& createPlanet(game::map::Universe& u, game::Id_t id)
    {
        game::map::Planet& p = *u.planets().create(id);
        p.setPosition(game::map::Point(1000, 1000+id));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), tx, log);
        p.setPlayability(game::map::Object::NotPlayable);
        return p;
    }

    game::map::Ship& createShip(game::map::Universe& u, game::Id_t id)
    {
        game::map::Ship& s = *u.ships().create(id);
        s.addShipXYData(game::map::Point(1000, 1000+id), 3, 222, game::PlayerSet_t(1));
        s.internalCheck();
        s.setPlayability(game::map::Object::NotPlayable);
        return s;
    }
}


/** Test initialisation behaviour. */
void
TestGameMapMarkings::testInit()
{
    game::map::Markings testee;
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 0U);

    // Query number of layers
    TS_ASSERT(testee.get(game::map::Markings::Ship).size() > 0);
    TS_ASSERT(testee.get(game::map::Markings::Planet).size() > 0);
    TS_ASSERT(testee.getNumLayers() > 0);

    // Numbeer of layers must agree
    TS_ASSERT_EQUALS(testee.getNumLayers(), testee.get(game::map::Markings::Ship).size());
    TS_ASSERT_EQUALS(testee.getNumLayers(), testee.get(game::map::Markings::Planet).size());

    // Layer 0 must exist
    TS_ASSERT(testee.get(game::map::Markings::Ship, 0) != 0);
    TS_ASSERT(testee.get(game::map::Markings::Planet, 0) != 0);

    // Layer 0 must be empty
    TS_ASSERT(testee.get(game::map::Markings::Ship, 0)->getNumMarkedObjects() == 0);
    TS_ASSERT(testee.get(game::map::Markings::Planet, 0)->getNumMarkedObjects() == 0);

    // One-past-end layer must not exist
    TS_ASSERT(testee.get(game::map::Markings::Ship, testee.getNumLayers()) == 0);
    TS_ASSERT(testee.get(game::map::Markings::Planet, testee.getNumLayers()) == 0);
}

/** Test copyFrom/copyTo/limitToExistingObjects. */
void
TestGameMapMarkings::testCopy()
{
    // Setup objects
    game::map::Universe univ;
    createPlanet(univ, 1);
    createPlanet(univ, 3);
    createPlanet(univ, 4).setIsMarked(true);
    createPlanet(univ, 5);
    createPlanet(univ, 100).setIsMarked(true);
    createShip(univ, 9).setIsMarked(true);

    // Must have the layer we're querying
    game::map::Markings testee;
    const size_t LAYER = 3;
    TS_ASSERT(testee.get(game::map::Markings::Planet, LAYER) != 0);
    TS_ASSERT(testee.get(game::map::Markings::Ship, LAYER) != 0);

    // Read into MarkingVector
    testee.copyFrom(univ, LAYER);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Planet, LAYER)->getNumMarkedObjects(), 2U);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Ship, LAYER)->getNumMarkedObjects(), 1U);

    // Set some bits
    testee.get(game::map::Markings::Planet, LAYER)->set(1, true);
    testee.get(game::map::Markings::Planet, LAYER)->set(5, true);
    testee.get(game::map::Markings::Planet, LAYER)->set(4, false);
    testee.get(game::map::Markings::Planet, LAYER)->set(105, true);
    testee.get(game::map::Markings::Ship, LAYER)->set(9, false);
    testee.get(game::map::Markings::Ship, LAYER)->set(105, true);

    // Write back
    testee.copyTo(univ, LAYER);
    TS_ASSERT_EQUALS(univ.planets().get(1)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(3)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(4)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(5)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(9)->isMarked(), false);

    // Limit
    testee.limitToExistingObjects(univ, LAYER);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Planet, LAYER)->getNumMarkedObjects(), 3U);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Ship, LAYER)->getNumMarkedObjects(), 0U);

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Planet, LAYER)->getNumMarkedObjects(), 0U);
}

/** Test executeCompiledExpression(). */
void
TestGameMapMarkings::testExecute()
{
    using interpreter::SelectionExpression;

    // Setup
    game::map::Universe u;
    createPlanet(u, 1);
    createPlanet(u, 2).setIsMarked(true);
    createPlanet(u, 3);
    createShip(u, 1);
    createShip(u, 2);
    createShip(u, 3).setIsMarked(true);

    // Execute
    // Note that opCurrent means target layer here!
    game::map::Markings testee;
    static const char EXPR[] = { SelectionExpression::opFirstLayer, SelectionExpression::opPlanet, SelectionExpression::opAnd, '\0' };
    testee.executeCompiledExpression(EXPR, 4, u);

    // Verify
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Planet, 4)->get(2), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Planet, 4)->get(3), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Markings::Ship, 4)->get(3), false);
}

/** Test setCurrentLayer()/getCurrentLayer(). */
void
TestGameMapMarkings::testSetLayer()
{
    // Setup
    game::map::Universe u;
    createPlanet(u, 1);
    createPlanet(u, 2).setIsMarked(true);
    createPlanet(u, 3);
    createShip(u, 1);
    createShip(u, 2);
    createShip(u, 3).setIsMarked(true);

    // Test
    game::map::Markings testee;
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 0U);

    // Layer 1: unmarks everything
    testee.setCurrentLayer(1, u);
    TS_ASSERT_EQUALS(u.planets().get(2)->isMarked(), false);
    TS_ASSERT_EQUALS(u.ships().get(3)->isMarked(), false);

    // Layer 0: restore
    testee.setCurrentLayer(0, u);
    TS_ASSERT_EQUALS(u.planets().get(2)->isMarked(), true);
    TS_ASSERT_EQUALS(u.ships().get(3)->isMarked(), true);
}

