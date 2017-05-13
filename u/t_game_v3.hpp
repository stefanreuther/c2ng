/**
  *  \file u/t_game_v3.hpp
  */
#ifndef C2NG_U_T_GAME_V3_HPP
#define C2NG_U_T_GAME_V3_HPP

#include <cxxtest/TestSuite.h> 

class TestGameV3Command : public CxxTest::TestSuite {
 public:
    void testCommands();
    void testMessageIntroducer();
    void testProto();
};

class TestGameV3CommandContainer : public CxxTest::TestSuite {
 public:
    void testContainer();
    void testSequence();
};

class TestGameV3CommandExtra : public CxxTest::TestSuite {
 public:
    void testEvents();
};

class TestGameV3ControlFile : public CxxTest::TestSuite {
 public:
    void testSave();
    void testSaveDOS();
    void testSaveWin();
    void testSaveBig();
    void testLoadDOS();
    void testLoadWindows();
    void testLoadEmpty();
    void testRange();
};

class TestGameV3ResultFile : public CxxTest::TestSuite {
 public:
    void test30();
    void test35();
};

class TestGameV3StringVerifier : public CxxTest::TestSuite {
 public:
    void testMain();
    void testFCode();
    void testMessage();
};

class TestGameV3Structures : public CxxTest::TestSuite {
 public:
    void testHeader();
};

#endif
