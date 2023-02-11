/**
  *  \file u/t_game_map_simpleobjectcursor.cpp
  *  \brief Test for game::map::SimpleObjectCursor
  */

#include "game/map/simpleobjectcursor.hpp"

#include "t_game_map.hpp"
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
void
TestGameMapSimpleObjectCursor::testIt()
{
    ObjectVector<IonStorm> sv;
    add(sv, 5);
    add(sv, 7);
    add(sv, 9);
    IonStormType ty(sv);

    // Create; check initial cursor
    game::map::SimpleObjectCursor testee;
    testee.setObjectType(&ty);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 5);

    // Index can be changed
    testee.setCurrentIndex(9);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 9);

    // Remove #9, but give it a hint
    sv.get(9)->setVoltage(0);
    ty.sig_setChange.raise(7);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 7);
}

/** Test change to different types (turns). */
void
TestGameMapSimpleObjectCursor::testChange()
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
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 0);

    // - auto-select 5
    testee.setObjectType(&ty1);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 5);

    // - keep 5 which is also in this set
    testee.setObjectType(&ty2);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 5);

    // - auto-select 13
    testee.setObjectType(&ty3);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 13);

    // - back to 0
    testee.setObjectType(0);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 0);
}

/** Test change to different types (turns). */
void
TestGameMapSimpleObjectCursor::testChange2()
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
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 0);

    // - auto-select 5
    testee.setObjectType(&ty1);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 5);

    // - select 0 because set is empty
    testee.setObjectType(&ty2);
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 0);
}

/** Test copy constructor. */
void
TestGameMapSimpleObjectCursor::testCopy()
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
    TS_ASSERT_EQUALS(testee.getCurrentIndex(), 9);

    // Create copy
    game::map::SimpleObjectCursor other((const game::map::ObjectCursor&) testee);
    TS_ASSERT_EQUALS(other.getCurrentIndex(), 9);
    TS_ASSERT_EQUALS(other.getObjectType(), &ty);
}

