/**
  *  \file u/t_game_v3.hpp
  */
#ifndef C2NG_U_T_GAME_V3_HPP
#define C2NG_U_T_GAME_V3_HPP

#include <cxxtest/TestSuite.h> 

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

#endif
