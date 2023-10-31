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
    void testOffset1();
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

class TestServerPlayEnginePacker : public CxxTest::TestSuite {
 public:
    void testIt();
    void testOffset1();
};

class TestServerPlayFlakConfigurationPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayFriendlyCodePacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPlayGameAccess : public CxxTest::TestSuite {
 public:
    void testGetStatus();
    void testGetBeam();
    void testGetTorp();
    void testGetEngine();
    void testGetHull();
    void testGetTruehull();
    void testGetAbilities();
    void testGetMultiple();
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
    void testOffset1();
};

class TestServerPlayTruehullPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
