/**
  *  \file u/t_server_play.hpp
  *  \brief Tests for server::play
  */
#ifndef C2NG_U_T_SERVER_PLAY_HPP
#define C2NG_U_T_SERVER_PLAY_HPP

#include <cxxtest/TestSuite.h>

class TestServerPlayCommandHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerPlayPacker : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
