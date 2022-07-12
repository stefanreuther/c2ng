/**
  *  \file u/t_game_test.hpp
  *  \brief Tests for game::test
  */
#ifndef C2NG_U_T_GAME_TEST_HPP
#define C2NG_U_T_GAME_TEST_HPP

#include <cxxtest/TestSuite.h>

class TestGameTestCargoContainer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameTestDefaultShipList : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameTestFiles : public CxxTest::TestSuite {
 public:
    void testFiles();
};

class TestGameTestRegistrationKey : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameTestSessionThread : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFileSystem();
};

class TestGameTestShipList : public CxxTest::TestSuite {
 public:
    void testStandardBeams();
    void testStandardTorpedoes();
    void testPListBeams();
    void testPListTorpedoes();
};

class TestGameTestStringVerifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameTestWaitIndicator : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRequestDispatcher();
};

#endif
