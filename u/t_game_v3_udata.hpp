/**
  *  \file u/t_game_v3_udata.hpp
  *  \brief Tests for game::v3::udata
  */
#ifndef C2NG_U_T_GAME_V3_UDATA_HPP
#define C2NG_U_T_GAME_V3_UDATA_HPP

#include <cxxtest/TestSuite.h>

class TestGameV3UdataMessageBuilder : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testUndefined();
    void testAlias();
    void testBadAlias();
    void testAliasLoop();
    void testContentLoop();
    void testFormatS();
    void testFormatSmiss();
    void testFormatX();
    void testFormatl();
    void testFormatF();
    void testFormatFNeg();
    void testFormatlMiss();
    void testFormatb();
    void testFormatbMiss();
    void testFormatPercent();
    void testFormatSpace();
    void testFormatg();
    void testFormath();
    void testFormatH();
    void testFormatn();
    void testFormatB();
    void testFormatBempty();
    void testFormatd();
    void testFormatdMiss();
    void testFormatp();
    void testFormatr();
    void testFormatu();
    void testFormatuMiss();
    void testFormatW();
    void testFormatR();
    void testFormatx();
    void testFormatEnum();
    void testFormatEnumMismatch();
    void testFormatEmpty();
    void testFormatEmptyValue();
    void testFormatEmptyId();
    void testFormatEmptyForce();
    void testFormatEmptyHide();
    void testReorder();
    void testLimit();
    void testLimitLoop();
};

class TestGameV3UdataNameProvider : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameV3UdataReader : public CxxTest::TestSuite {
 public:
    void testCheck();
    void testCheckOffset();
    void testCheckText();
    void testCheckEmpty();
    void testCheckTrunc1();
    void testCheckTrunc2();
    void testRead();
    void testReadFail();
};

class TestGameV3UdataSessionNameProvider : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testPopulated();
};

#endif
