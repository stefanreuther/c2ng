/**
  *  \file u/t_game_interface.hpp
  *  \brief Tests for game::interface
  */
#ifndef C2NG_U_T_GAME_INTERFACE_HPP
#define C2NG_U_T_GAME_INTERFACE_HPP

#include <cxxtest/TestSuite.h>

class TestGameInterfaceCargoFunctions : public CxxTest::TestSuite {
 public:
    void testCheckCargoSpecArg();
    void testCAdd();
    void testCCompare();
    void testCDiv();
    void testCExtract();
    void testCMul();
    void testCRemove();
    void testCSub();
};

class TestGameInterfaceEngineContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceEngineProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testSet();
};

class TestGameInterfaceExplosionContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceFriendlyCodeContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceIteratorProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceMissionContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceRichTextFunctions : public CxxTest::TestSuite {
 public:
    void testRAdd();
    void testRMid();
    void testRString();
    void testRLen();
    void testRStyle();
    void testRLink();
    void testRXml();
};

class TestGameInterfaceUfoContext : public CxxTest::TestSuite {
 public:
    void testTypes();
};

class TestGameInterfaceUserInterfaceProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceUserInterfacePropertyAccessor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceUserInterfacePropertyStack : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testMulti();
};

#endif
