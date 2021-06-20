/**
  *  \file u/t_util_editor.hpp
  *  \brief Tests for util::editor
  */
#ifndef C2NG_U_T_UTIL_EDITOR_HPP
#define C2NG_U_T_UTIL_EDITOR_HPP

#include <cxxtest/TestSuite.h>

class TestUtilEditorCommand : public CxxTest::TestSuite {
 public:
    void testToString();
    void testLookup();
    void testHandleCommand();
    void testHandleInsert();
};

#endif
