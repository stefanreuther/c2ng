/**
  *  \file u/t_game_spec_info.hpp
  *  \brief Tests for game::spec::info
  */
#ifndef C2NG_U_T_GAME_SPEC_INFO_HPP
#define C2NG_U_T_GAME_SPEC_INFO_HPP

#include <cxxtest/TestSuite.h>

class TestGameSpecInfoBrowser : public CxxTest::TestSuite {
 public:
    void testDescribePlayer();
    void testDescribeHull();
    void testDescribeRacial();
    void testDescribeShip();
    void testDescribeEngine();
    void testDescribeBeam();
    void testDescribeTorpedo();
    void testDescribeFighter();
    void testListPlayer();
    void testListHull();
    void testListRacial();
    void testListShip();
    void testListEngine();
    void testListBeam();
    void testListTorpedo();
    void testListFighter();
    void testDescribeFilter();
};

class TestGameSpecInfoFilter : public CxxTest::TestSuite {
 public:
    void testInit();
    void testDescribeElement();
    void testDescribeElement2();
    void testModify();
};

class TestGameSpecInfoInfo : public CxxTest::TestSuite {
 public:
    void testDescribeHull();
    void testDescribeEngine();
    void testDescribeBeam();
    void testDescribeTorp();
    void testDescribeFighter();
    void testGetHullAttribute();
    void testGetEngineAttribute();
    void testGetBeamAttribute();
    void testGetTorpAttribute();
    void testGetFighterAttribute();
    void testDescribeWeaponEffectsTim();
    void testDescribeWeaponEffectsPHostAC();
    void testDescribeWeaponEffectsPHostNonAC();
    void testDescribeWeaponEffectsPHostMixedFighters();
    void testDescribeWeaponEffectsPHostExp();
    void testDescribeWeaponEffectsPHostExpNonAC();
    void testDescribeHullFunction();
    void testDescribeHullFunctionPicture();
};

class TestGameSpecInfoNullPictureNamer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecInfoPictureNamer : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameSpecInfoTypes : public CxxTest::TestSuite {
 public:
    void testAttribute();
    void testAbility();
    void testPageContent();
    void testListEntry();
    void testListContent();
    void testFilterElement();
    void testFilterInfo();
};

class TestGameSpecInfoUtils : public CxxTest::TestSuite {
 public:
    void testFilterAttributeToString();
    void testConvertRangeToSet();
    void testGetLevelRange();
    void testGetHullRange();
    void testGetPlayerRange();
    void testAttributeRange();
};

#endif
