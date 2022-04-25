/**
  *  \file u/t_client_map.hpp
  *  \brief Tests for client::map
  */
#ifndef C2NG_U_T_CLIENT_MAP_HPP
#define C2NG_U_T_CLIENT_MAP_HPP

#include <cxxtest/TestSuite.h>

class TestClientMapLocation : public CxxTest::TestSuite {
 public:
    void testInit();
    void testBuildAgain();
    void testBuildAgainAbs();
    void testBuildNull();
    void testBuildJump();
    void testBuildJump2();
    void testLock();
    void testBuildLock();
    void testLockAgain();
    void testBuildJumpLock();
    void testBuildJumpLock2();
    void testBuildAgainJump();
    void testBuildAgainLock();
    void testMoveAbs();
    void testMoveRel();
    void testJump();
    void testJumpLock();
    void testFocusedObject();
    void testFocusedObjectPreset();
    void testCycleFocus();
    void testCycleFocusEmpty();
    void testCycleFocusUnmarked();
    void testLoseFocusedObject();
    void testKeepFocusedObject();
};

class TestClientMapOverlay : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
