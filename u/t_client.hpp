/**
  *  \file u/t_client.hpp
  *  \brief Tests for client
  */
#ifndef C2NG_U_T_CLIENT_HPP
#define C2NG_U_T_CLIENT_HPP

#include <cxxtest/TestSuite.h>

class TestClientDownlink : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestClientPictureNamer : public CxxTest::TestSuite {
 public:
    void testHull();
    void testEngine();
    void testBeam();
    void testLauncher();
    void testPlayer();
    void testAbility();
};

#endif
