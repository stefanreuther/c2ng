/**
  *  \file u/t_client.hpp
  *  \brief Tests for client
  */
#ifndef C2NG_U_T_CLIENT_HPP
#define C2NG_U_T_CLIENT_HPP

#include <cxxtest/TestSuite.h>

class TestClientApplicationParameters : public CxxTest::TestSuite {
 public:
    void testInit();
    void testDirectory();
    void testPlayer();
    void testSize();
    void testRequestDelay();
    void testBadRequestDelay();
    void testDir();
    void testLog();
    void testPassword();
    void testProxy();
    void testResource();
    void testHelp();
    void testBadOption();
    void testBadParameter();
};

class TestClientDownlink : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestClientMarker : public CxxTest::TestSuite {
 public:
    void testUserAccess();
    void testShip();
};

class TestClientPictureNamer : public CxxTest::TestSuite {
 public:
    void testHull();
    void testEngine();
    void testBeam();
    void testLauncher();
    void testPlayer();
    void testAbility();
    void testVcrObject();
};

#endif
