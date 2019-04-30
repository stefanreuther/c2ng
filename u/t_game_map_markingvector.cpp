/**
  *  \file u/t_game_map_markingvector.cpp
  *  \brief Test for game::map::MarkingVector
  */

#include "game/map/markingvector.hpp"

#include "t_game_map.hpp"
#include "game/exception.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "game/map/universe.hpp"
#include "interpreter/selectionexpression.hpp"

namespace {
    class TestObject : public game::map::Object {
     public:
        TestObject(game::Id_t id)
            : m_id(id)
            { }
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return String_t(); }
        virtual game::Id_t getId() const
            { return m_id; }
        virtual bool getOwner(int& /*result*/) const
            { return false; }
     private:
        game::Id_t m_id;
    };

    class TestObjectVectorType : public game::map::ObjectVectorType<TestObject> {
     public:
        TestObjectVectorType(game::map::Universe& univ, game::map::ObjectVector<TestObject>& vec)
            : game::map::ObjectVectorType<TestObject>(univ, vec)
            { }
        virtual bool isValid(const TestObject&) const
            { return true; }
    };
}

/** Test initial state. */
void
TestGameMapMarkingVector::testInit()
{
    game::map::MarkingVector testee;
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(0), false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(100), false);
    TS_ASSERT_EQUALS(testee.get(30000), false);
}

/** Test set/get. */
void
TestGameMapMarkingVector::testSetGet()
{
    game::map::MarkingVector testee;

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
TestGameMapMarkingVector::testCopy()
{
    // Setup objects
    game::map::ObjectVector<TestObject> vec;
    vec.create(1);
    vec.create(3);
    vec.create(4)->setIsMarked(true);
    vec.create(5);
    vec.create(100)->setIsMarked(true);

    // Setup type
    game::map::Universe univ;
    TestObjectVectorType type(univ, vec);

    // Read into MarkingVector
    game::map::MarkingVector testee;
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
TestGameMapMarkingVector::testExecute()
{
    using interpreter::SelectionExpression;
    game::map::MarkingVector testee;

    // Load '1'
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load '0'
    static const char ZERO_EXPR[] = { SelectionExpression::opZero, '\0' };
    testee.executeCompiledExpression(ZERO_EXPR, afl::base::Nothing, 20, false);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);

    // Load 'P' on planet
    static const char PLANET_EXPR[] = { SelectionExpression::opPlanet, '\0' };
    testee.executeCompiledExpression(PLANET_EXPR, afl::base::Nothing, 20, true);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load 'P' on ship
    testee.executeCompiledExpression(PLANET_EXPR, afl::base::Nothing, 20, false);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);

    // Load 'S' on planet
    static const char SHIP_EXPR[] = { SelectionExpression::opShip, '\0' };
    testee.executeCompiledExpression(SHIP_EXPR, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // Load 'S' on planet
    testee.executeCompiledExpression(SHIP_EXPR, afl::base::Nothing, 20, true);
    TS_ASSERT_EQUALS(testee.getNumMarkedObjects(), 0U);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(20), false);
}

/** Test executeCompiledExpression(), size handling. */
void
TestGameMapMarkingVector::testExecuteSize()
{
    using interpreter::SelectionExpression;
    game::map::MarkingVector testee;

    // 20 elements (same as above)
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, afl::base::Nothing, 20, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 20U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(20), true);

    // 32 elements
    testee.executeCompiledExpression(ONE_EXPR, afl::base::Nothing, 32, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 32U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(32), true);

    // 3200 elements
    testee.executeCompiledExpression(ONE_EXPR, afl::base::Nothing, 3200, false);
    TS_ASSERT(testee.getNumMarkedObjects() >= 3200U);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(3200), true);
}

/** Test executeCompiledExpression(), various operations. */
void
TestGameMapMarkingVector::testExecuteOp()
{
    using interpreter::SelectionExpression;
    game::map::MarkingVector testee;

    // Environment
    game::map::MarkingVector a[2];
    a[0].set(2, true);
    a[0].set(4, true);
    a[1].set(3, true);
    a[1].set(4, true);

    // AND
    static const char AND_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opAnd,
                                     '\0' };
    testee.executeCompiledExpression(AND_EXPR, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(3), false);
    TS_ASSERT_EQUALS(testee.get(4), true);

    // OR
    static const char OR_EXPR[] = { SelectionExpression::opFirstLayer,
                                    SelectionExpression::opFirstLayer+1,
                                    SelectionExpression::opOr,
                                    '\0' };
    testee.executeCompiledExpression(OR_EXPR, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), true);
    TS_ASSERT_EQUALS(testee.get(3), true);
    TS_ASSERT_EQUALS(testee.get(4), true);

    // XOR
    static const char XOR_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opXor,
                                     '\0' };
    testee.executeCompiledExpression(XOR_EXPR, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), false);
    TS_ASSERT_EQUALS(testee.get(2), true);
    TS_ASSERT_EQUALS(testee.get(3), true);
    TS_ASSERT_EQUALS(testee.get(4), false);

    // Negate self
    static const char NOT_EXPR[] = { SelectionExpression::opCurrent,
                                     SelectionExpression::opNot,
                                     '\0' };
    testee.executeCompiledExpression(NOT_EXPR, a, 4, false);
    TS_ASSERT_EQUALS(testee.get(1), true);
    TS_ASSERT_EQUALS(testee.get(2), false);
    TS_ASSERT_EQUALS(testee.get(3), false);
    TS_ASSERT_EQUALS(testee.get(4), true);
}

/** Test executeCompiledExpression(), invalid operations. */
void
TestGameMapMarkingVector::testExecuteError()
{
    using interpreter::SelectionExpression;
    game::map::MarkingVector testee;

    // No result produced
    TS_ASSERT_THROWS(testee.executeCompiledExpression(String_t(), afl::base::Nothing, 4, false), game::Exception);

    // Too many results produced
    static const char TWO_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opCurrent, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(TWO_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Missing input parameters: AND
    static const char AND0_EXPR[] = { SelectionExpression::opAnd, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(AND0_EXPR, afl::base::Nothing, 4, false), game::Exception);
    static const char AND1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opAnd, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(AND1_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Missing input parameters: OR
    static const char OR0_EXPR[] = { SelectionExpression::opOr, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(OR0_EXPR, afl::base::Nothing, 4, false), game::Exception);
    static const char OR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opOr, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(OR1_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Missing input parameters: XOR
    static const char XOR0_EXPR[] = { SelectionExpression::opXor, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(XOR0_EXPR, afl::base::Nothing, 4, false), game::Exception);
    static const char XOR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opXor, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(XOR1_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Missing input parameters: NOT
    static const char NOT0_EXPR[] = { SelectionExpression::opNot, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(NOT0_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Invalid reference
    static const char REF_EXPR[] = { SelectionExpression::opFirstLayer, '\0' };
    TS_ASSERT_THROWS(testee.executeCompiledExpression(REF_EXPR, afl::base::Nothing, 4, false), game::Exception);

    // Invalid opcode
    TS_ASSERT_THROWS(testee.executeCompiledExpression("\xC3", afl::base::Nothing, 4, false), game::Exception);
}

