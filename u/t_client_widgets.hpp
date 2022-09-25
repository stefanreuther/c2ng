/**
  *  \file u/t_client_widgets.hpp
  *  \brief Tests for client::widgets
  */
#ifndef C2NG_U_T_CLIENT_WIDGETS_HPP
#define C2NG_U_T_CLIENT_WIDGETS_HPP

#include <cxxtest/TestSuite.h>

class TestClientWidgetsPluginList : public CxxTest::TestSuite {
 public:
    void testFormat();
    void testContent();
};

#endif
