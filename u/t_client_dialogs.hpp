/**
  *  \file u/t_client_dialogs.hpp
  *  \brief Tests for client::dialogs
  */
#ifndef C2NG_U_T_CLIENT_DIALOGS_HPP
#define C2NG_U_T_CLIENT_DIALOGS_HPP

#include <cxxtest/TestSuite.h>

class TestClientDialogsObjectSelectionDialog : public CxxTest::TestSuite {
 public:
    void testOK();
    void testCancel();
};

#endif
