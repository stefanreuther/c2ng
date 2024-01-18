/**
  *  \file test/game/map/simpleobjectcursortest.cpp
  *  \brief Test for game::map::SimpleObjectCursor
  */

#include "game/map/simpleobjectcursor.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/ionstormtype.hpp"
#include "game/map/objectvector.hpp"

/* For simplicity, we are using IonStorm/IonStormType as foundation. */
using game::map::IonStorm;
using game::map::IonStormType;
using game::map::ObjectVector;

namespace {
    void add(ObjectVector<IonStorm>& sv, game::Id_t id)
    {
        IonStorm* is = sv.create(id);
        is->setVoltage(10);
        is->setRadius(20);
        is->setPosition(game::map::Point(1000 + 50*id, 1000));
    }
}


/** Test normal operation. */
AFL_TEST("game.map.SimpleObjectCursor:basics", a)
{
    ObjectVector<IonStorm> sv;
    add(sv, 5);
    add(sv, 7);
    add(sv, 9);
    IonStormType ty(sv);

    // Create; check initial cursor
    game::map::SimpleObjectCursor testee;
    testee.setObjectType(&ty);
    a.checkEqual("01. getCurrentIndex", testee.getCurrentIndex(), 5);

    // Index can be changed
    testee.setCurrentIndex(9);
    a.checkEqual("11. getCurrentIndex", testee.getCurrentIndex(), 9);

    // Remove #9, but give it a hint
    sv.get(9)->setVoltage(0);
    ty.sig_setChange.raise(7);
    a.checkEqual("21. getCurrentIndex", testee.getCurrentIndex(), 7);
}

/** Test change to different types (turns). */
AFL_TEST("game.map.SimpleObjectCursor:change", a)
{
    // Set 1
    ObjectVector<IonStorm> sv1;
    add(sv1, 5);
    add(sv1, 7);
    add(sv1, 9);
    IonStormType ty1(sv1);

    // Set 2
    ObjectVector<IonStorm> sv2;
    add(sv2, 5);
    add(sv2, 9);
    IonStormType ty2(sv2);

    // Set 3
    ObjectVector<IonStorm> sv3;
    add(sv3, 13);
    IonStormType ty3(sv3);

    // Test
    // - initially 0
    game::map::SimpleObjectCursor testee;
    a.checkEqual("01. getCurrentIndex", testee.getCurrentIndex(), 0);

    // - auto-select 5
    testee.setObjectType(&ty1);
    a.checkEqual("11. getCurrentIndex", testee.getCurrentIndex(), 5);

    // - keep 5 which is also in this set
    testee.setObjectType(&ty2);
    a.checkEqual("21. getCurrentIndex", testee.getCurrentIndex(), 5);

    // - auto-select 13
    testee.setObjectType(&ty3);
    a.checkEqual("31. getCurrentIndex", testee.getCurrentIndex(), 13);

    // - back to 0
    testee.setObjectType(0);
    a.checkEqual("41. getCurrentIndex", testee.getCurrentIndex(), 0);
}

/** Test change to different types (turns). */
AFL_TEST("game.map.SimpleObjectCursor:change-to-empty", a)
{
    // Set 1
    ObjectVector<IonStorm> sv1;
    add(sv1, 5);
    add(sv1, 7);
    add(sv1, 9);
    IonStormType ty1(sv1);

    // Set 2 - empty
    ObjectVector<IonStorm> sv2;
    IonStormType ty2(sv2);

    // Test
    // - initially 0
    game::map::SimpleObjectCursor testee;
    a.checkEqual("01. getCurrentIndex", testee.getCurrentIndex(), 0);

    // - auto-select 5
    testee.setObjectType(&ty1);
    a.checkEqual("11. getCurrentIndex", testee.getCurrentIndex(), 5);

    // - select 0 because set is empty
    testee.setObjectType(&ty2);
    a.checkEqual("21. getCurrentIndex", testee.getCurrentIndex(), 0);
}

/** Test copy constructor. */
AFL_TEST("game.map.SimpleObjectCursor:copy", a)
{
    ObjectVector<IonStorm> sv;
    add(sv, 5);
    add(sv, 7);
    add(sv, 9);
    IonStormType ty(sv);

    // Create original
    game::map::SimpleObjectCursor testee;
    testee.setObjectType(&ty);
    testee.setCurrentIndex(9);
    a.checkEqual("01. getCurrentIndex", testee.getCurrentIndex(), 9);

    // Create copy
    game::map::SimpleObjectCursor other((const game::map::ObjectCursor&) testee);
    a.checkEqual("11. getCurrentIndex", other.getCurrentIndex(), 9);
    a.checkEqual("12. getObjectType", other.getObjectType(), &ty);
}
