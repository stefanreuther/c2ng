/**
  *  \file u/t_game_parser.hpp
  *  \brief Tests for game::parser
  */
#ifndef C2NG_U_T_GAME_PARSER_HPP
#define C2NG_U_T_GAME_PARSER_HPP

#include <cxxtest/TestSuite.h>

class TestGameParserDataInterface : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameParserMessageTemplate : public CxxTest::TestSuite {
 public:
    void testValues();
    void testValuesX100();
    void testValuesEnum();
};

#endif
