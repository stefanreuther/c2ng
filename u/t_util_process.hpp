/**
  *  \file u/t_util_process.hpp
  *  \brief Tests for util::process
  */
#ifndef C2NG_U_T_UTIL_PROCESS_HPP
#define C2NG_U_T_UTIL_PROCESS_HPP

#include <cxxtest/TestSuite.h>

class TestUtilProcessFactory : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilProcessSubprocess : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
