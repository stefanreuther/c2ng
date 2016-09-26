/**
  *  \file u/t_game_config.hpp
  */
#ifndef C2NG_U_T_GAME_CONFIG_HPP
#define C2NG_U_T_GAME_CONFIG_HPP

#include <cxxtest/TestSuite.h> 

class TestGameConfigBitsetValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigBooleanValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigConfiguration : public CxxTest::TestSuite {
 public:
    void testIndexing();
    void testAccess();
};

class TestGameConfigConfigurationOption : public CxxTest::TestSuite {
 public:
    void testIt();
    void testUpdate();
};

class TestGameConfigCostArrayOption : public CxxTest::TestSuite {
 public:
    void testSet1();
    void testSet2();
    void testSet3();
};

class TestGameConfigHostConfiguration : public CxxTest::TestSuite {
 public:
    void testRace();
};

class TestGameConfigIntegerValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigValueParser : public CxxTest::TestSuite {
 public:
    void testArray();
};

#endif
