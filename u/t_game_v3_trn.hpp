/**
  *  \file u/t_game_v3_trn.hpp
  *  \brief Tests for game::v3::trn
  */
#ifndef C2NG_U_T_GAME_V3_TRN_HPP
#define C2NG_U_T_GAME_V3_TRN_HPP

#include <cxxtest/TestSuite.h>

class TestGameV3TrnAndFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnConstantFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnFilter : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testParser();
    void testParserFailure();
};

class TestGameV3TrnIdFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnIndexFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnNameFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnNegateFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnOrFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnParseException : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameV3TrnStringFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
