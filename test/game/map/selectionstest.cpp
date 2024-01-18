/**
  *  \file test/game/map/selectionstest.cpp
  *  \brief Test for game::map::Selections
  */

#include "game/map/selections.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(), 15, tx, log);
        p.setPlayability(game::map::Object::NotPlayable);
        return p;
    }

    game::map::Ship& createShip(game::map::Universe& u, game::Id_t id)
    {
        game::map::Ship& s = *u.ships().create(id);
        s.addShipXYData(game::map::Point(1000, 1000+id), 3, 222, game::PlayerSet_t(1));
        s.internalCheck(game::PlayerSet_t(1), 15);
        s.setPlayability(game::map::Object::NotPlayable);
        return s;
    }
}


/** Test initialisation behaviour. */
AFL_TEST("game.map.Selections:init", a)
{
    game::map::Selections testee;
    a.checkEqual("01. getCurrentLayer", testee.getCurrentLayer(), 0U);

    // Query number of layers
    a.check("11. get", testee.get(game::map::Selections::Ship).size() > 0);
    a.check("12. get", testee.get(game::map::Selections::Planet).size() > 0);
    a.check("13. getNumLayers", testee.getNumLayers() > 0);

    // Numbeer of layers must agree
    a.checkEqual("21. getNumLayers", testee.getNumLayers(), testee.get(game::map::Selections::Ship).size());
    a.checkEqual("22. getNumLayers", testee.getNumLayers(), testee.get(game::map::Selections::Planet).size());

    // Layer 0 must exist
    a.checkNonNull("31. get", testee.get(game::map::Selections::Ship, 0));
    a.checkNonNull("32. get", testee.get(game::map::Selections::Planet, 0));

    // Layer 0 must be empty
    a.checkEqual("41. get", testee.get(game::map::Selections::Ship, 0)->getNumMarkedObjects(), 0U);
    a.checkEqual("42. get", testee.get(game::map::Selections::Planet, 0)->getNumMarkedObjects(), 0U);

    // One-past-end layer must not exist
    a.checkNull("51. get", testee.get(game::map::Selections::Ship, testee.getNumLayers()));
    a.checkNull("52. get", testee.get(game::map::Selections::Planet, testee.getNumLayers()));
}

/** Test copyFrom/copyTo/limitToExistingObjects. */
AFL_TEST("game.map.Selections:copy", a)
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
    a.checkNonNull("01. get", testee.get(game::map::Selections::Planet, LAYER));
    a.checkNonNull("02. get", testee.get(game::map::Selections::Ship, LAYER));

    // Read into SelectionVector
    testee.copyFrom(univ, LAYER);
    a.checkEqual("11. get", testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 2U);
    a.checkEqual("12. get", testee.get(game::map::Selections::Ship, LAYER)->getNumMarkedObjects(), 1U);

    // Set some bits
    testee.get(game::map::Selections::Planet, LAYER)->set(1, true);
    testee.get(game::map::Selections::Planet, LAYER)->set(5, true);
    testee.get(game::map::Selections::Planet, LAYER)->set(4, false);
    testee.get(game::map::Selections::Planet, LAYER)->set(105, true);
    testee.get(game::map::Selections::Ship, LAYER)->set(9, false);
    testee.get(game::map::Selections::Ship, LAYER)->set(105, true);

    // Write back
    testee.copyTo(univ, LAYER);
    a.checkEqual("21. isMarked", univ.planets().get(1)->isMarked(), true);
    a.checkEqual("22. isMarked", univ.planets().get(3)->isMarked(), false);
    a.checkEqual("23. isMarked", univ.planets().get(4)->isMarked(), false);
    a.checkEqual("24. isMarked", univ.planets().get(5)->isMarked(), true);
    a.checkEqual("25. isMarked", univ.ships().get(9)->isMarked(), false);

    // Limit
    testee.limitToExistingObjects(univ, LAYER);
    a.checkEqual("31. get", testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 3U);
    a.checkEqual("32. get", testee.get(game::map::Selections::Ship, LAYER)->getNumMarkedObjects(), 0U);

    // Clear
    testee.clear();
    a.checkEqual("41. get", testee.get(game::map::Selections::Planet, LAYER)->getNumMarkedObjects(), 0U);
}

/** Test executeCompiledExpression().
    A: create universe with some selections. Execute an expression.
    E: verify expected result of expression. */
AFL_TEST("game.map.Selections:executeCompiledExpression", a)
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
    a.checkEqual("01. get", testee.get(game::map::Selections::Planet, 4)->get(2), true);
    a.checkEqual("02. get", testee.get(game::map::Selections::Planet, 4)->get(3), false);
    a.checkEqual("03. get", testee.get(game::map::Selections::Ship, 4)->get(3), false);
}

/** Test setCurrentLayer()/getCurrentLayer().
    A: create a universe with some selections. Switch to layer B, back to A.
    E: layer switch updates object selections accordingly. */
AFL_TEST("game.map.Selections:layer-switch", a)
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
    a.checkEqual("01. getCurrentLayer", testee.getCurrentLayer(), 0U);

    // Layer 1: unmarks everything
    testee.setCurrentLayer(1, u);
    a.checkEqual("11. isMarked", u.planets().get(2)->isMarked(), false);
    a.checkEqual("12. isMarked", u.ships().get(3)->isMarked(), false);

    // Layer 0: restore
    testee.setCurrentLayer(0, u);
    a.checkEqual("21. isMarked", u.planets().get(2)->isMarked(), true);
    a.checkEqual("22. isMarked", u.ships().get(3)->isMarked(), true);
}

/** Test behaviour of opCurrent reference.
    A: populate layers A+B, activate layer A. Execute expression 'C := Current + B'.
    E: active layer is A, so result should have the content of A+B.  */
AFL_TEST("game.map.Selections:opCurrent", a)
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
    a.check("01. isMarked", !p.isMarked());
    a.check("02. isMarked", !s.isMarked());
    p.setIsMarked(true);

    // Set up layer 1
    testee.setCurrentLayer(1, u);
    a.check("11. isMarked", !p.isMarked());
    a.check("12. isMarked", !s.isMarked());
    s.setIsMarked(true);

    // Execute
    testee.setCurrentLayer(0, u);
    static const char EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opFirstLayer+1, SelectionExpression::opOr, '\0' };
    testee.executeCompiledExpression(EXPR, 2, u);

    // Verify
    a.checkEqual("21. get", testee.get(game::map::Selections::Planet, 2)->get(1), true);
    a.checkEqual("22. get", testee.get(game::map::Selections::Ship,   2)->get(1), true);
}

/** Test executeCompiledExpressionAll.
    A: populate universe and some layers. Execute expression 'Not Current'.
    E: verify expected content of layers.  */
AFL_TEST("game.map.Selections:executeCompiledExpressionAll:opCurrent", a)
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
    a.check("01. isMarked", !p.isMarked());
    a.check("02. isMarked", !s.isMarked());
    p.setIsMarked(true);

    // Set up layer 1
    testee.setCurrentLayer(1, u);
    a.check("11. isMarked", !p.isMarked());
    a.check("12. isMarked", !s.isMarked());
    s.setIsMarked(true);

    // Execute
    static const char EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opNot, '\0' };
    testee.executeCompiledExpressionAll(EXPR, u);

    // Verify
    // - content of layers
    a.checkEqual("21. get", testee.get(game::map::Selections::Planet, 0)->get(1), false);
    a.checkEqual("22. get", testee.get(game::map::Selections::Ship,   0)->get(1), true);
    a.checkEqual("23. get", testee.get(game::map::Selections::Planet, 1)->get(1), true);
    a.checkEqual("24. get", testee.get(game::map::Selections::Ship,   1)->get(1), false);
    a.checkEqual("25. get", testee.get(game::map::Selections::Planet, 2)->get(1), true);
    a.checkEqual("26. get", testee.get(game::map::Selections::Ship,   2)->get(1), true);

    // - units in universe
    a.checkEqual("31. isMarked", p.isMarked(), true);
    a.checkEqual("32. isMarked", s.isMarked(), false);
}

/** Test executeCompiledExpressionAll(), opShip opcode.
    A: populate universe and some layers. Execute expression 'Ship'.
    E: verify expected content of layers.  */
AFL_TEST("game.map.Selections:executeCompiledExpressionAll:opShip", a)
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
    a.checkEqual("01. get", testee.get(game::map::Selections::Planet, 0)->get(1), false);
    a.checkEqual("02. get", testee.get(game::map::Selections::Ship,   0)->get(1), true);
    a.checkEqual("03. get", testee.get(game::map::Selections::Planet, 1)->get(1), false);
    a.checkEqual("04. get", testee.get(game::map::Selections::Ship,   1)->get(1), true);
    a.checkEqual("05. get", testee.get(game::map::Selections::Planet, 2)->get(1), false);
    a.checkEqual("06. get", testee.get(game::map::Selections::Ship,   2)->get(1), true);

    // - units in universe
    a.checkEqual("11. isMarked", p.isMarked(), false);
    a.checkEqual("12. isMarked", s.isMarked(), true);
}

/** Test markList(), current layer.
    A: populate universe. Call markList() for current layer.
    E: verify that objects have been marked, layer has been changed. */
AFL_TEST("game.map.Selections:markList:current", a)
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
    a.checkEqual("01. get", testee.get(game::map::Selections::Planet, 0)->get(1), true);
    a.checkEqual("02. get", testee.get(game::map::Selections::Planet, 0)->get(2), false);
    a.checkEqual("03. get", testee.get(game::map::Selections::Planet, 0)->get(3), true);
    a.checkEqual("04. get", testee.get(game::map::Selections::Planet, 0)->get(99), false);  // because it does not exist
    a.checkEqual("05. get", testee.get(game::map::Selections::Ship,   0)->get(1), false);
    a.checkEqual("06. get", testee.get(game::map::Selections::Ship,   0)->get(2), true);

    // - units in universe
    a.checkEqual("11. isMarked", p1.isMarked(), true);
    a.checkEqual("12. isMarked", s2.isMarked(), true);
}

/** Test markList(), other layer.
    A: populate universe. Call markList() for other layer.
    E: verify that objects have not been marked, but layer has been changed. */
AFL_TEST("game.map.Selections:markList:other", a)
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
    a.checkEqual("01. get", testee.get(game::map::Selections::Planet, 3)->get(1), true);
    a.checkEqual("02. get", testee.get(game::map::Selections::Planet, 3)->get(2), false);
    a.checkEqual("03. get", testee.get(game::map::Selections::Planet, 3)->get(3), true);
    a.checkEqual("04. get", testee.get(game::map::Selections::Planet, 3)->get(99), false);  // because it does not exist
    a.checkEqual("05. get", testee.get(game::map::Selections::Ship,   3)->get(1), false);
    a.checkEqual("06. get", testee.get(game::map::Selections::Ship,   3)->get(2), true);

    // - units in universe
    a.checkEqual("11. isMarked", p1.isMarked(), false);
    a.checkEqual("12. isMarked", s2.isMarked(), false);
}

/** Test setCurrentLayer() with relative expressions.
    A: call setCurrentLayer() with all sorts of layer references.
    E: verify correct result */
AFL_TEST("game.map.Selections:setCurrentLayer:relative", a)
{
    game::map::Selections testee;
    game::map::Universe u;
    a.checkEqual("01. getCurrentLayer", testee.getCurrentLayer(), 0U);

    // Previous
    testee.setCurrentLayer(game::map::Selections::PreviousLayer, u);
    a.checkEqual("11. getCurrentLayer", testee.getCurrentLayer(), testee.getNumLayers()-1);

    // Next
    testee.setCurrentLayer(game::map::Selections::NextLayer, u);
    a.checkEqual("21. getCurrentLayer", testee.getCurrentLayer(), 0U);
    testee.setCurrentLayer(game::map::Selections::NextLayer, u);
    a.checkEqual("22. getCurrentLayer", testee.getCurrentLayer(), 1U);

    // Current
    testee.setCurrentLayer(game::map::Selections::CurrentLayer, u);
    a.checkEqual("31. getCurrentLayer", testee.getCurrentLayer(), 1U);

    // Absolute
    testee.setCurrentLayer(3, u);
    a.checkEqual("41. getCurrentLayer", testee.getCurrentLayer(), 3U);

    // Previous
    testee.setCurrentLayer(game::map::Selections::PreviousLayer, u);
    a.checkEqual("51. getCurrentLayer", testee.getCurrentLayer(), 2U);
}
