/**
  *  \file u/t_interpreter_vmio.hpp
  *  \brief Tests for interpreter::vmio
  */
#ifndef C2NG_U_T_INTERPRETER_VMIO_HPP
#define C2NG_U_T_INTERPRETER_VMIO_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterVmioFileSaveContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCycle();
};

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

class TestInterpreterVmioObjectLoader : public CxxTest::TestSuite {
 public:
    void testLoadBCO();
    void testLoadHash();
    void testLoadArray();
};

class TestInterpreterVmioProcessLoadContext : public CxxTest::TestSuite {
 public:
    void testLoadMutex();
};

class TestInterpreterVmioProcessSaveContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestInterpreterVmioStructures : public CxxTest::TestSuite {
 public:
    void testProcessKind();
};

class TestInterpreterVmioValueLoader : public CxxTest::TestSuite {
 public:
    void testReal();
    void testInteger();
    void testLoadSegment();
    void testLoadSegment2();
};

#endif
