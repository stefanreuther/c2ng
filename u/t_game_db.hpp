/**
  *  \file u/t_game_db.hpp
  *  \brief Tests for game::db
  */
#ifndef C2NG_U_T_GAME_DB_HPP
#define C2NG_U_T_GAME_DB_HPP

#include <cxxtest/TestSuite.h>

class TestGameDbDrawingAtomMap : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSave();
};

class TestGameDbStructures : public CxxTest::TestSuite {
 public:
    void testHeader();
};

#endif
