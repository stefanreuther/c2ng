/**
  *  \file test/game/map/selectionvectortest.cpp
  *  \brief Test for game::map::SelectionVector
  */

#include "game/map/selectionvector.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.map.SelectionVector:init", a)
{
    game::map::SelectionVector testee;
    a.checkEqual("01. getNumMarkedObjects", testee.getNumMarkedObjects(), 0U);
    a.checkEqual("02. get", testee.get(0), false);
    a.checkEqual("03. get", testee.get(1), false);
    a.checkEqual("04. get", testee.get(100), false);
    a.checkEqual("05. get", testee.get(30000), false);
}

/** Test set/get. */
AFL_TEST("game.map.SelectionVector:access", a)
{
    game::map::SelectionVector testee;

    testee.set(1, true);
    testee.set(10, true);
    testee.set(100, true);

    a.checkEqual("01. getNumMarkedObjects", testee.getNumMarkedObjects(), 3U);

    a.checkEqual("11. get", testee.get(1), true);
    a.checkEqual("12. get", testee.get(2), false);
    a.checkEqual("13. get", testee.get(10), true);
    a.checkEqual("14. get", testee.get(11), false);
    a.checkEqual("15. get", testee.get(100), true);
    a.checkEqual("16. get", testee.get(101), false);

    testee.set(10, false);
    a.checkEqual("21. get", testee.get(10), false);

    testee.set(-1, true);
    testee.set(-1000, true);
    a.checkEqual("31. get", testee.get(-1), false);
    a.checkEqual("32. get", testee.get(-1000), false);
}

/** Test copyFrom/copyTo/limitToExistingObjects. */
AFL_TEST("game.map.SelectionVector:copy", a)
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
    a.checkEqual("01. get", testee.get(1), false);
    a.checkEqual("02. get", testee.get(2), false);
    a.checkEqual("03. get", testee.get(3), false);
    a.checkEqual("04. get", testee.get(4), true);

    a.checkEqual("11. get", testee.get(99), false);
    a.checkEqual("12. get", testee.get(100), true);
    a.checkEqual("13. get", testee.get(101), false);

    // Set some bits
    testee.set(1, true);
    testee.set(5, true);
    testee.set(4, false);
    testee.set(105, true);

    // Write back
    testee.copyTo(type);
    a.checkEqual("21. isMarked", vec.get(1)->isMarked(), true);
    a.checkEqual("22. isMarked", vec.get(3)->isMarked(), false);
    a.checkEqual("23. isMarked", vec.get(4)->isMarked(), false);
    a.checkEqual("24. isMarked", vec.get(5)->isMarked(), true);

    // Limit
    a.checkEqual("31. getNumMarkedObjects", testee.getNumMarkedObjects(), 4U); // 1,5,100,105
    testee.limitToExistingObjects(type);
    a.checkEqual("32. getNumMarkedObjects", testee.getNumMarkedObjects(), 3U); // 1,5,100
    a.checkEqual("33. get", testee.get(100), true);
    a.checkEqual("34. get", testee.get(105), false);

    // Clear
    testee.clear();
    a.checkEqual("41. getNumMarkedObjects", testee.getNumMarkedObjects(), 0U);
    a.checkEqual("42. get", testee.get(100), false);
}

/** Test executeCompiledExpression(), simple cases. */
AFL_TEST("game.map.SelectionVector:executeCompiledExpression:simple", a)
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // Load '1'
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 20, false);
    a.check("01. getNumMarkedObjects", testee.getNumMarkedObjects() >= 20U);
    a.checkEqual("02. get", testee.get(1), true);
    a.checkEqual("03. get", testee.get(20), true);

    // Load '0'
    static const char ZERO_EXPR[] = { SelectionExpression::opZero, '\0' };
    testee.executeCompiledExpression(ZERO_EXPR, 0, afl::base::Nothing, 20, false);
    a.checkEqual("11. getNumMarkedObjects", testee.getNumMarkedObjects(), 0U);
    a.checkEqual("12. get", testee.get(1), false);
    a.checkEqual("13. get", testee.get(20), false);

    // Load 'P' on planet
    static const char PLANET_EXPR[] = { SelectionExpression::opPlanet, '\0' };
    testee.executeCompiledExpression(PLANET_EXPR, 0, afl::base::Nothing, 20, true);
    a.check("21. getNumMarkedObjects", testee.getNumMarkedObjects() >= 20U);
    a.checkEqual("22. get", testee.get(1), true);
    a.checkEqual("23. get", testee.get(20), true);

    // Load 'P' on ship
    testee.executeCompiledExpression(PLANET_EXPR, 0, afl::base::Nothing, 20, false);
    a.checkEqual("31. getNumMarkedObjects", testee.getNumMarkedObjects(), 0U);
    a.checkEqual("32. get", testee.get(1), false);
    a.checkEqual("33. get", testee.get(20), false);

    // Load 'S' on planet
    static const char SHIP_EXPR[] = { SelectionExpression::opShip, '\0' };
    testee.executeCompiledExpression(SHIP_EXPR, 0, afl::base::Nothing, 20, false);
    a.check("41. getNumMarkedObjects", testee.getNumMarkedObjects() >= 20U);
    a.checkEqual("42. get", testee.get(1), true);
    a.checkEqual("43. get", testee.get(20), true);

    // Load 'S' on planet
    testee.executeCompiledExpression(SHIP_EXPR, 0, afl::base::Nothing, 20, true);
    a.checkEqual("51. getNumMarkedObjects", testee.getNumMarkedObjects(), 0U);
    a.checkEqual("52. get", testee.get(1), false);
    a.checkEqual("53. get", testee.get(20), false);
}

/** Test executeCompiledExpression(), size handling. */
AFL_TEST("game.map.SelectionVector:executeCompiledExpression:size", a)
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // 20 elements (same as above)
    static const char ONE_EXPR[] = { SelectionExpression::opOne, '\0' };
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 20, false);
    a.check("01. getNumMarkedObjects", testee.getNumMarkedObjects() >= 20U);
    a.checkEqual("02. get", testee.get(1), true);
    a.checkEqual("03. get", testee.get(20), true);

    // 32 elements
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 32, false);
    a.check("11. getNumMarkedObjects", testee.getNumMarkedObjects() >= 32U);
    a.checkEqual("12. get", testee.get(1), true);
    a.checkEqual("13. get", testee.get(32), true);

    // 3200 elements
    testee.executeCompiledExpression(ONE_EXPR, 0, afl::base::Nothing, 3200, false);
    a.check("21. getNumMarkedObjects", testee.getNumMarkedObjects() >= 3200U);
    a.checkEqual("22. get", testee.get(1), true);
    a.checkEqual("23. get", testee.get(3200), true);
}

/** Test executeCompiledExpression(), various operations. */
AFL_TEST("game.map.SelectionVector:executeCompiledExpression:operators", a)
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector testee;

    // Environment
    game::map::SelectionVector av[2];
    av[0].set(2, true);
    av[0].set(4, true);
    av[1].set(3, true);
    av[1].set(4, true);

    // AND
    static const char AND_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opAnd,
                                     '\0' };
    testee.executeCompiledExpression(AND_EXPR, 99, av, 4, false);
    a.checkEqual("01. get", testee.get(1), false);
    a.checkEqual("02. get", testee.get(2), false);
    a.checkEqual("03. get", testee.get(3), false);
    a.checkEqual("04. get", testee.get(4), true);

    // OR
    static const char OR_EXPR[] = { SelectionExpression::opFirstLayer,
                                    SelectionExpression::opFirstLayer+1,
                                    SelectionExpression::opOr,
                                    '\0' };
    testee.executeCompiledExpression(OR_EXPR, 99, av, 4, false);
    a.checkEqual("11. get", testee.get(1), false);
    a.checkEqual("12. get", testee.get(2), true);
    a.checkEqual("13. get", testee.get(3), true);
    a.checkEqual("14. get", testee.get(4), true);

    // XOR
    static const char XOR_EXPR[] = { SelectionExpression::opFirstLayer,
                                     SelectionExpression::opFirstLayer+1,
                                     SelectionExpression::opXor,
                                     '\0' };
    testee.executeCompiledExpression(XOR_EXPR, 99, av, 4, false);
    a.checkEqual("21. get", testee.get(1), false);
    a.checkEqual("22. get", testee.get(2), true);
    a.checkEqual("23. get", testee.get(3), true);
    a.checkEqual("24. get", testee.get(4), false);

    // Negate A
    static const char NOT_EXPR[] = { SelectionExpression::opCurrent,
                                     SelectionExpression::opNot,
                                     '\0' };
    testee.executeCompiledExpression(NOT_EXPR, 0, av, 4, false);
    a.checkEqual("31. get", testee.get(1), true);
    a.checkEqual("32. get", testee.get(2), false);
    a.checkEqual("33. get", testee.get(3), true);
    a.checkEqual("34. get", testee.get(4), false);
}

/** Test executeCompiledExpression(), invalid operations. */
AFL_TEST("game.map.SelectionVector:executeCompiledExpression:errors", a)
{
    using interpreter::SelectionExpression;
    game::map::SelectionVector vectors[1];
    game::map::SelectionVector& testee = vectors[0];

    // No result produced
    AFL_CHECK_THROWS(a("01. no result"), testee.executeCompiledExpression(String_t(), 0, afl::base::Nothing, 4, false), game::Exception);

    // Too many results produced
    static const char TWO_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opCurrent, '\0' };
    AFL_CHECK_THROWS(a("11. two results"), testee.executeCompiledExpression(TWO_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: AND
    static const char AND0_EXPR[] = { SelectionExpression::opAnd, '\0' };
    AFL_CHECK_THROWS(a("21. and missing input"), testee.executeCompiledExpression(AND0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char AND1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opAnd, '\0' };
    AFL_CHECK_THROWS(a("22. and missing input"), testee.executeCompiledExpression(AND1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: OR
    static const char OR0_EXPR[] = { SelectionExpression::opOr, '\0' };
    AFL_CHECK_THROWS(a("31. or missing input"), testee.executeCompiledExpression(OR0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char OR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opOr, '\0' };
    AFL_CHECK_THROWS(a("32. or missing input"), testee.executeCompiledExpression(OR1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: XOR
    static const char XOR0_EXPR[] = { SelectionExpression::opXor, '\0' };
    AFL_CHECK_THROWS(a("41. xor missing input"), testee.executeCompiledExpression(XOR0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char XOR1_EXPR[] = { SelectionExpression::opCurrent, SelectionExpression::opXor, '\0' };
    AFL_CHECK_THROWS(a("42. xor missing input"), testee.executeCompiledExpression(XOR1_EXPR, 0, vectors, 4, false), game::Exception);

    // Missing input parameters: NOT
    static const char NOT0_EXPR[] = { SelectionExpression::opNot, '\0' };
    AFL_CHECK_THROWS(a("51. not missing input"), testee.executeCompiledExpression(NOT0_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);

    // Invalid reference
    static const char REF_EXPR[] = { SelectionExpression::opFirstLayer, '\0' };
    AFL_CHECK_THROWS(a("61. bad reference"), testee.executeCompiledExpression(REF_EXPR, 0, afl::base::Nothing, 4, false), game::Exception);
    static const char REF1_EXPR[] = { SelectionExpression::opCurrent, '\0' };
    AFL_CHECK_THROWS(a("62. bad reference"), testee.executeCompiledExpression(REF1_EXPR, 99, vectors, 4, false), game::Exception);

    // Invalid opcode
    AFL_CHECK_THROWS(a("71. bad opcode"), testee.executeCompiledExpression("\xC3", 0, afl::base::Nothing, 4, false), game::Exception);
}
