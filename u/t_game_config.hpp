/**
  *  \file u/t_game_config.hpp
  */
#ifndef C2NG_U_T_GAME_CONFIG_HPP
#define C2NG_U_T_GAME_CONFIG_HPP

#include <cxxtest/TestSuite.h> 

class TestGameConfigAliasOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigBitsetValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigBooleanValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigCollapsibleIntegerArrayOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigConfiguration : public CxxTest::TestSuite {
 public:
    void testIndexing();
    void testAccess();
    void testEnum();
    void testMerge();
};

class TestGameConfigConfigurationEditor : public CxxTest::TestSuite {
 public:
    void testIt();
    void testToggle();
    void testSetValue();
    void testSourceSingle();
    void testSourceEmpty();
    void testSourceMulti();
    void testChange();
    void testAlias();
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
    void testFormat();
};

class TestGameConfigEnumValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIt2();
};

class TestGameConfigExpressionLists : public CxxTest::TestSuite {
 public:
    void testAccess();
    void testPackComplex();
    void testLoadRecent();
    void testLoadPredefined();
    void testSave();
};

class TestGameConfigGenericIntegerArrayOption : public CxxTest::TestSuite {
 public:
    void testIt();
    void testZero();
};

class TestGameConfigHostConfiguration : public CxxTest::TestSuite {
 public:
    void testRace();
    void testAlias();
    void testDependant1();
    void testDependant2();
    void testExperienceName();
    void testExperienceBonus();
    void testGetExperienceLevelFromPoints();
};

class TestGameConfigIntegerArrayOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigIntegerOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigIntegerValueParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigMarkerOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigStringOption : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameConfigUserConfiguration : public CxxTest::TestSuite {
 public:
    void testDefaults();
    void testGameType();
    void testFormat();
    void testCannedMarker();
};

class TestGameConfigValueParser : public CxxTest::TestSuite {
 public:
    void testArray();
};

#endif
