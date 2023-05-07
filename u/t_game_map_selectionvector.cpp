/**
  *  \file u/t_game_map_selectionvector.cpp
  *  \brief Test for game::map::SelectionVector
  */

#include "game/map/selectionvector.hpp"

#include "t_game_map.hpp"
#include "game/exception.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "interpreter/selectionexpression.hpp"

namespace {
    class TestObject : public game::map::Object {
     public:
        TestObject(game::Id_t id)
            : Object(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual afl::base::Optional<int> getOwner() const
            { return afl::base::Nothing; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
    };

    class TestObjectVectorType : public game::map::ObjectVectorType<TestObject> {
     public:
        TestObjectVectorType(game::map::ObjectVector<TestObject>& vec)
            : game::map::ObjectVectorType<TestObject>(vec)
            { }
        virtual bool isValid(const TestObject&) const
            { return true; }
    };
}

/** Test initial state. */
void
TestGameMapSelectionVector::testInit()
{
    game::map::SelectionVector testee;
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(0), false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(100), false);
    TS_ASSERT_EQUALS(testee.get(30000), false);
}

/** Test set/get. */
void
TestGameMapSelectionVector::testSetGet()
{
    game::map::SelectionVector testee;

    testee.set(1, true);
    testee.set(10, true);
    testee.set(100, true);

    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 3U);

    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(10), true);
    TS_ASSERT_EQUALS(testee.get(11), false);
    TS_ASSERT_EQUALS(testee.get(100), true);
    TS_ASSERT_EQUALS(testee.get(101), false);

    testee.set(10, false);
    TS_ASSERT_EQUALS(testee.get(10), false);

    testee.set(-1, true);
    testee.set(-1000, true);
    TS_ASSERT_EQUALS(testee.get(-1), false);
    TS_ASSERT_EQUALS(testee.get(-1000), false);
}

/** Test copyFrom/copyTo/limitToExistingObjects. */
void
TestGameMapSelectionVector::testCopy()
{
    // Setup objects
    game::map::ObjectVector<TestObject> vec;
    vec.create(1);
    vec.create(3);
    vec.create(4)->setIsMarked(true);
    vec.create(5);
    vec.create(100)->setIsMarked(true);

    // Setup type
    TestObjectVectorType type(vec);

    // Read into SelectionVector
    game::map::SelectionVector testee;
    testee.copyFrom(type);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(3), false);
    TS_ASSERT_EQUALS(testee.get(4), true);

    TS_ASSERT_EQUALS(testee.get(99), false);
    TS_ASSERT_EQUALS(testee.get(100), true);
    TS_ASSERT_EQUALS(testee.get(101), false);

    // Set some bits
    testee.set(1, true);
    testee.set(5, true);
    testee.set(4, false);
    testee.set(105, true);

    // Write back
    testee.copyTo(type);
    TS_ASSERT_EQUALS(vec.get(1)->isMarked(), true);
    TS_ASSERT_EQUALS(vec.get(3)->isMarked(), false);
    TS_ASSERT_EQUALS(vec.get(4)->isMarked(), false);
    TS_ASSERT_EQUALS(vec.get(5)->isMarked(), true);

    // Limit
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 4U); // 1,5,100,105
    testee.limitToExistingObjects(type);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 3U); // 1,5,100
    TS_ASSERT_EQUALS(testee.get(100), true);
    TS_ASSERT_EQUALS(testee.get(105), false);

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(100), false);
}

/** Test executeCompiledExpression(), simple cases. */
void
TestGameMapSelectionVector::testExecute()
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // Load '1'
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load '0'
    static const char ZERO_EXPR[] = { SelectionExpression::opZero, '\0' };
    testee.executeCompiledExpression(ZERO_EXPR, 0, afl::base::Nothing, 20, false);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);

    // Load 'P' on planet
    static const char PLANET_EXPR[] = { SelectionExpression::opPlanet, '\0' };
    testee.executeCompiledExpression(PLANET_EXPR, 0, afl::base::Nothing, 20, true);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load 'P' on ship
    testee.executeCompiledExpression(PLANET_EXPR, 0, afl::base::Nothing, 20, false);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);

    // Load 'S' on planet
    static const char SHIP_EXPR[] = { SelectionExpression::opShip, '\0' };
    testee.executeCompiledExpression(SHIP_EXPR, 0, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load 'S' on planet
    testee.executeCompiledExpression(SHIP_EXPR, 0, afl::base::Nothing, 20, true);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);
}

/** Test executeCompiledExpression(), size handling. */
void
TestGameMapSelectionVector::testExecuteSize()
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // 20 elements (same as above)
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // 32 elements
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 32, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 32U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(32), true);

    // 3200 elements
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 3200, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 3200U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(3200), true);
}

/** Test executeCompiledExpression(), various operations. */
void
TestGameMapSelectionVector::testExecuteOp()
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // Environment
    game::map::SelectionVector a[2];
    a[0].set(2, true);
    a[0].set(4, true);
    a[1].set(3, true);
    a[1].set(4, true);

    // AND
    static const char AND_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opAnd,
                                     '\0' };
    testee.executeCompiledExpression(AND_EXPR, 99, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(3), false);
    TS_ASSERT_EQUALS(testee.get(4), true);

    // OR
    static const char OR_EXPR[] = { SelectionExpression::opFirstLayer,
                                    SelectionExpression::opFirstLayer+1,
                                    SelectionExpression::opOr,
                                    '\0' };
    testee.executeCompiledExpression(OR_EXPR, 99, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), true);
    TS_ASSERT_EQUALS(testee.get(3), true);
    TS_ASSERT_EQUALS(testee.get(4), true);

    // XOR
    static const char XOR_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opXor,
                                     '\0' };
    testee.executeCompiledExpression(XOR_EXPR, 99, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), true);
    TS_ASSERT_EQUALS(testee.get(3), true);
    TS_ASSERT_EQUALS(testee.get(4), false);

    // Negate A
    static const char NOT_EXPR[] = { SelectionExpression::opCurrent,
                                     SelectionExpression::opNot,
                                     '\0' };
    testee.executeCompiledExpression(NOT_EXPR, 0, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(3), true);
    TS_ASSERT_EQUALS(testee.get(4), false);
}

/** Test executeCompiledExpression(), invalid operations. */
void
TestGameMapSelectionVector::testExecuteError()
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector vectors[1];
    game::map::SelectionVector& testee = vectors[0];

    // No result produced
    TS_ASSERT_THROWS(testee.executeCompiledExpression(String_t(), 0, afl::base::Nothing, 4, false), game::Exception);

    // Too many results produced
    static const char TWO_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opCurrent, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(TWO_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: AND
    static const char AND0_EXPR[] = { SelectionExpression::opAnd, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(AND0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char AND1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opAnd, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(AND1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: OR
    static const char OR0_EXPR[] = { SelectionExpression::opOr, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(OR0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char OR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opOr, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(OR1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: XOR
    static const char XOR0_EXPR[] = { SelectionExpression::opXor, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(XOR0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char XOR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opXor, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(XOR1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: NOT
    static const char NOT0_EXPR[] = { SelectionExpression::opNot, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(NOT0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);

    // Invalid reference
    static const char REF_EXPR[] = { SelectionExpression::opFirstLayer, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(REF_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char REF1_EXPR[] = { SelectionExpression::opCurrent, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(REF1_EXPR, 99, vectors, 4, false), game::Exception);

    // Invalid opcode
    TS_ASSERT_THROWS(testee.executeCompiledExpression("\xC3", 0, afl::base::Nothing, 4, false), game::Exception);
}

