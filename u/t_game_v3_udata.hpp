/**
  *  \file u/t_game_v3_udata.hpp
  *  \brief Tests for game::v3::udata
  */
#ifndef C2NG_U_T_GAME_V3_UDATA_HPP
#define C2NG_U_T_GAME_V3_UDATA_HPP

#include <cxxtest/TestSuite.h>

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

#endif
