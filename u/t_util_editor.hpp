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

class TestUtilEditorEditor : public CxxTest::TestSuite {
 public:
    void testConfig();
    void testText();
    void testCommandMoveLineUp();
    void testCommandMoveLineDown();
    void testCommandMoveCharacterLeft();
    void testCommandMoveCharacterRight();
    void testCommandMoveWordLeft();
    void testCommandMoveWordRight();
    void testCommandMoveBeginningOfLine();
    void testCommandMoveEndOfLine();
    void testCommandMoveBeginningOfDocument();
    void testCommandMoveEndOfDocument();
    void testCommandDeleteCharacter();
    void testCommandDeleteCharacterBackward();
    void testCommandDeleteLine();
    void testCommandDeleteEndOfLine();
    void testCommandDeleteWordBackward();
    void testCommandDeleteWordForward();
    void testCommandTransposeCharacters();
    void testCommandToggleInsert();
    void testCommandToggleWrap();
    void testCommandInsertTab();
    void testCommandInsertNewline();
    void testCommandInsertNewlineAbove();
    void testCommandNull();
    void testHandleInsert();
    void testInsertLine();
    void testDeleteLine();
};

#endif
