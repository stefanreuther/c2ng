/**
  *  \file u/t_server_play.hpp
  *  \brief Tests for server::play
  */
#ifndef C2NG_U_T_SERVER_PLAY_HPP
#define C2NG_U_T_SERVER_PLAY_HPP

#include <cxxtest/TestSuite.h>

class TestServerPlayBasicHullFunctionPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayBeamPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayCommandHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerPlayConfigurationPacker : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSlices();
};

class TestServerPlayFriendlyCodePacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayHullPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayOutMessageCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
    void testError();
};

class TestServerPlayOutMessageIndexPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayOutMessagePacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayPacker : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerPlayPackerList : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayTorpedoPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
