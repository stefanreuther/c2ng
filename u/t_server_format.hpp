/**
  *  \file u/t_server_format.hpp
  *  \brief Tests for server::format
  */
#ifndef C2NG_U_T_SERVER_FORMAT_HPP
#define C2NG_U_T_SERVER_FORMAT_HPP

#include <cxxtest/TestSuite.h>

class TestServerFormatBeamPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFormatEnginePacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFormatFormat : public CxxTest::TestSuite {
 public:
    void testPack();
    void testUnpack();
    void testUnpackAll();
};

class TestServerFormatHullPacker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFormatPacker : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerFormatSimPacker : public CxxTest::TestSuite {
 public:
    void testV0();
    void testV1();
    void testV2();
    void testV3();
    void testV4();
    void testV5();
    void testError();
};

class TestServerFormatStringPacker : public CxxTest::TestSuite {
 public:
    void testUtf8();
    void testCodepage();
};

class TestServerFormatTorpedoPacker : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLarge();
};

class TestServerFormatTruehullPacker : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSmall();
};

class TestServerFormatutils : public CxxTest::TestSuite {
 public:
    void testPackCost();
    void testUnpackCost();
};

#endif
