/**
  *  \file u/t_game_map_selections.cpp
  *  \brief Test for game::map::Selections
  */

#include "game/map/selections.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
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
TestGameMapSelections::testInit()
{
    game::map::Selections testee;
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 0U);

    // Query number of layers
    TS_ASSERT(testee.get(game::map::Selections::Ship).size() > 0);
    TS_ASSERT(testee.get(game::map::Selections::Planet).size() > 0);
    TS_ASSERT(testee.getNumLayers() > 0);

    // Numbeer of layers must agree
    TS_ASSERT_EQUALS(testee.getNumLayers(), testee.get(game::map::Selections::Ship).size());
    TS_ASSERT_EQUALS(testee.getNumLayers(), testee.get(game::map::Selections::Planet).size());

    // Layer 0 must exist
    TS_ASSERT(testee.get(game::map::Selections::Ship, 0) != 0);
    TS_ASSERT(testee.get(game::map::Selections::Planet, 0) != 0);

    // Layer 0 must be empty
    TS_ASSERT(testee.get(game::map::Selections::Ship, 0)->getNumMarkedObjects() == 0);
    TS_ASSERT(testee.get(game::map::Selections::Planet, 0)->getNumMarkedObjects() == 0);

    // One-past-end layer must not exist
    TS_ASSERT(testee.get(game::map::Selections::Ship, testee.getNumLayers()) == 0);
    TS_ASSERT(testee.get(game::map::Selections::Planet, testee.getNumLayers()) == 0);
}

/** Test copyFrom/copyTo/limitToExistingObjects. */
void
TestGameMapSelections::testCopy()
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
    game::map::Selections testee;
    const size_t LAYER = 3;
    TS_ASSERT(testee.get(game::map::Selections::Planet, LAYER) != 0);
    TS_ASSERT(testee.get(game::map::Selections::Ship, LAYER) != 0);

    // Read into SelectionVector
    testee.copyFrom(univ, LAYER);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 2U);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship, LAYER)->getNumMarkedObjects(), 1U);

    // Set some bits
    testee.get(game::map::Selections::Planet, LAYER)->set(1, true);
    testee.get(game::map::Selections::Planet, LAYER)->set(5, true);
    testee.get(game::map::Selections::Planet, LAYER)->set(4, false);
    testee.get(game::map::Selections::Planet, LAYER)->set(105, true);
    testee.get(game::map::Selections::Ship, LAYER)->set(9, false);
    testee.get(game::map::Selections::Ship, LAYER)->set(105, true);

    // Write back
    testee.copyTo(univ, LAYER);
    TS_ASSERT_EQUALS(univ.planets().get(1)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.planets().get(3)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(4)->isMarked(), false);
    TS_ASSERT_EQUALS(univ.planets().get(5)->isMarked(), true);
    TS_ASSERT_EQUALS(univ.ships().get(9)->isMarked(), false);

    // Limit
    testee.limitToExistingObjects(univ, LAYER);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 3U);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship, LAYER)->getNumMarkedObjects(), 0U);

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 0U);
}

/** Test executeCompiledExpression().
    A: create universe with some selections. Execute an expression.
    E: verify expected result of expression. */
void
TestGameMapSelections::testExecute()
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
    game::map::Selections testee;
    static const char EXPR[] = { SelectionExpression::opFirstLayer, SelectionExpression::opPlanet, SelectionExpression::opAnd, '\0' };
    testee.executeCompiledExpression(EXPR, 4, u);

    // Verify
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 4)->get(2), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 4)->get(3), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship, 4)->get(3), false);
}

/** Test setCurrentLayer()/getCurrentLayer().
    A: create a universe with some selections. Switch to layer B, back to A.
    E: layer switch updates object selections accordingly. */
void
TestGameMapSelections::testSetLayer()
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
    game::map::Selections testee;
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

/** Test behaviour of opCurrent reference.
    A: populate layers A+B, activate layer A. Execute expression 'C := Current + B'.
    E: active layer is A, so result should have the content of A+B.  */
void
TestGameMapSelections::testCurrent()
{
    using interpreter::SelectionExpression;

    // Setup
    game::map::Universe u;
    game::map::Planet& p = createPlanet(u, 1);
    game::map::Ship& s = createShip(u, 1);

    // Prepare
    game::map::Selections testee;

    // Set up layer 0
    testee.setCurrentLayer(0, u);
    TS_ASSERT(!p.isMarked());
    TS_ASSERT(!s.isMarked());
    p.setIsMarked(true);

    // Set up layer 1
    testee.setCurrentLayer(1, u);
    TS_ASSERT(!p.isMarked());
    TS_ASSERT(!s.isMarked());
    s.setIsMarked(true);

    // Execute
    testee.setCurrentLayer(0, u);
    static const char EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opFirstLayer+1, SelectionExpression::opOr, '\0' };
    testee.executeCompiledExpression(EXPR, 2, u);

    // Verify
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 2)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   2)->get(1), true);
}

/** Test executeCompiledExpressionAll.
    A: populate universe and some layers. Execute expression 'Not Current'.
    E: verify expected content of layers.  */
void
TestGameMapSelections::testExecuteAll()
{
    using interpreter::SelectionExpression;

    // Setup
    game::map::Universe u;
    game::map::Planet& p = createPlanet(u, 1);
    game::map::Ship& s = createShip(u, 1);

    // Prepare
    game::map::Selections testee;

    // Set up layer 0
    testee.setCurrentLayer(0, u);
    TS_ASSERT(!p.isMarked());
    TS_ASSERT(!s.isMarked());
    p.setIsMarked(true);

    // Set up layer 1
    testee.setCurrentLayer(1, u);
    TS_ASSERT(!p.isMarked());
    TS_ASSERT(!s.isMarked());
    s.setIsMarked(true);

    // Execute
    static const char EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opNot, '\0' };
    testee.executeCompiledExpressionAll(EXPR, u);

    // Verify
    // - content of layers
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   0)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 1)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   1)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 2)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   2)->get(1), true);

    // - units in universe
    TS_ASSERT_EQUALS(p.isMarked(), true);
    TS_ASSERT_EQUALS(s.isMarked(), false);
}

/** Test executeCompiledExpressionAll(), opShip opcode.
    A: populate universe and some layers. Execute expression 'Ship'.
    E: verify expected content of layers.  */
void
TestGameMapSelections::testExecuteAllShip()
{
    using interpreter::SelectionExpression;

    // Setup
    game::map::Universe u;
    game::map::Planet& p = createPlanet(u, 1);
    game::map::Ship& s = createShip(u, 1);

    // Prepare
    game::map::Selections testee;

    // Execute
    static const char EXPR[] = { SelectionExpression::opShip, '\0' };
    testee.executeCompiledExpressionAll(EXPR, u);

    // Verify
    // - content of layers
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   0)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 1)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   1)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 2)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   2)->get(1), true);

    // - units in universe
    TS_ASSERT_EQUALS(p.isMarked(), false);
    TS_ASSERT_EQUALS(s.isMarked(), true);
}

/** Test markList(), current layer.
    A: populate universe. Call markList() for current layer.
    E: verify that objects have been marked, layer has been changed. */
void
TestGameMapSelections::testMarkListCurrent()
{
    using game::Reference;

    // Setup
    game::map::Universe u;
    game::map::Planet& p1  = createPlanet(u, 1);
    game::map::Planet& p2  = createPlanet(u, 2);
    game::map::Planet& p3  = createPlanet(u, 3);
    game::map::Ship& s1  = createShip(u, 1);
    game::map::Ship& s2  = createShip(u, 2);

    // List
    game::ref::List list;
    list.add(Reference(Reference::Planet, 1));
    list.add(Reference(Reference::Starbase, 3));
    list.add(Reference(Reference::Planet, 99));
    list.add(Reference(Reference::Ship, 2));
    list.add(Reference(Reference::Player, 7));

    // Prepare
    game::map::Selections testee;

    // Execute
    testee.markList(0, list, true, u);

    // Verify
    // - content of layers
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(2), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(3), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 0)->get(99), false);  // because it does not exist
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   0)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   0)->get(2), true);

    // - units in universe
    TS_ASSERT_EQUALS(p1.isMarked(), true);
    TS_ASSERT_EQUALS(s2.isMarked(), true);
}

/** Test markList(), other layer.
    A: populate universe. Call markList() for other layer.
    E: verify that objects have not been marked, but layer has been changed. */
void
TestGameMapSelections::testMarkListOther()
{
    using game::Reference;

    // Setup
    game::map::Universe u;
    game::map::Planet& p1  = createPlanet(u, 1);
    game::map::Planet& p2  = createPlanet(u, 2);
    game::map::Planet& p3  = createPlanet(u, 3);
    game::map::Ship& s1  = createShip(u, 1);
    game::map::Ship& s2  = createShip(u, 2);

    // List
    game::ref::List list;
    list.add(Reference(Reference::Planet, 1));
    list.add(Reference(Reference::Starbase, 3));
    list.add(Reference(Reference::Planet, 99));
    list.add(Reference(Reference::Ship, 2));
    list.add(Reference(Reference::Player, 7));

    // Prepare
    game::map::Selections testee;

    // Execute
    testee.markList(3, list, true, u);

    // Verify
    // - content of layers
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 3)->get(1), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 3)->get(2), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 3)->get(3), true);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Planet, 3)->get(99), false);  // because it does not exist
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   3)->get(1), false);
    TS_ASSERT_EQUALS(testee.get(game::map::Selections::Ship,   3)->get(2), true);

    // - units in universe
    TS_ASSERT_EQUALS(p1.isMarked(), false);
    TS_ASSERT_EQUALS(s2.isMarked(), false);
}

/** Test setCurrentLayer() with relative expressions.
    A: call setCurrentLayer() with all sorts of layer references.
    E: verify correct result */
void
TestGameMapSelections::testSetRelative()
{
    game::map::Selections testee;
    game::map::Universe u;
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 0U);

    // Previous
    testee.setCurrentLayer(game::map::Selections::PreviousLayer, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), testee.getNumLayers()-1);

    // Next
    testee.setCurrentLayer(game::map::Selections::NextLayer, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 0U);
    testee.setCurrentLayer(game::map::Selections::NextLayer, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 1U);

    // Current
    testee.setCurrentLayer(game::map::Selections::CurrentLayer, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 1U);

    // Absolute
    testee.setCurrentLayer(3, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 3U);

    // Previous
    testee.setCurrentLayer(game::map::Selections::PreviousLayer, u);
    TS_ASSERT_EQUALS(testee.getCurrentLayer(), 2U);
}

