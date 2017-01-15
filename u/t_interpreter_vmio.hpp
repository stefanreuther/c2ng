/**
  *  \file u/t_interpreter_vmio.hpp
  *  \brief Tests for interpreter::vmio
  */
#ifndef C2NG_U_T_INTERPRETER_VMIO_HPP
#define C2NG_U_T_INTERPRETER_VMIO_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterVmioLoadContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterVmioNullLoadContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterVmioNullSaveContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterVmioValueLoader : public CxxTest::TestSuite {
 public:
    void testReal();
    void testInteger();
    void testLoadSegment();
    void testLoadSegment2();
};

#endif
