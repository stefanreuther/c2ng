/**
  *  \file u/t_interpreter_exporter.hpp
  *  \brief Tests for interpreter::exporter
  */
#ifndef C2NG_U_T_INTERPRETER_EXPORTER_HPP
#define C2NG_U_T_INTERPRETER_EXPORTER_HPP

#include <cxxtest/TestSuite.h>

class TestInterpreterExporterFieldList : public CxxTest::TestSuite {
 public:
    void testAdd();
    void testAddList();
    void testModify();
};

#endif
