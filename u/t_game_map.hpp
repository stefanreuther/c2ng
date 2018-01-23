/**
  *  \file u/t_game_map.hpp
  *  \brief Tests for game::map
  */
#ifndef C2NG_U_T_GAME_MAP_HPP
#define C2NG_U_T_GAME_MAP_HPP

#include <cxxtest/TestSuite.h>

class TestGameMapCircularObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapConfiguration : public CxxTest::TestSuite {
 public:
    void testFlat();
    void testFlatSmall();
    void testFlatOffset();
    void testWrapped();
    void testWrappedSmall();
};

class TestGameMapExplosion : public CxxTest::TestSuite {
 public:
    void testName();
};

class TestGameMapFleetMember : public CxxTest::TestSuite {
 public:
    void testSetFleetName();
    void testSetWaypoint();
    void testSetWarpFactor();
    void testSetMission();
    void testSetMissionToIntercept();
    void testSetMissionFromIntercept();
    void testSetFleetNumberFail();
    void testSetFleetNumberSuccess();
    void testSetFleetNumberDropLeader();
    void testSetFleetNumberDropMember();
    void testSetFleetNumberMoveMember();
    void testSetMissionTow();
    void testSetMissionTowOther();
    void testSetMissionTowInvalid();
    void testIsMissionLocked();
    void testIsMissionLockedMutex();
    void testSetFleetNumberForeign();
};

class TestGameMapIonStorm : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapMarkingVector : public CxxTest::TestSuite {
 public:
    void testInit();
    void testSetGet();
    void testCopy();
    void testExecute();
    void testExecuteSize();
    void testExecuteOp();
    void testExecuteError();
};

class TestGameMapMarkings : public CxxTest::TestSuite {
 public:
    void testInit();
    void testCopy();
    void testExecute();
    void testSetLayer();
};

class TestGameMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectCursor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectList : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testContent();
};

class TestGameMapObjectReference : public CxxTest::TestSuite {
 public:
    void testCompare();
    void testAccessor();
};

class TestGameMapPoint : public CxxTest::TestSuite {
 public:
    void testBasics();
};

class TestGameMapViewport : public CxxTest::TestSuite {
 public:
    void testRectangle();
};

#endif
