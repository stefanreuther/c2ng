/**
  *  \file u/t_util.hpp
  *  \brief Tests for util
  */
#ifndef C2NG_U_T_UTIL_HPP
#define C2NG_U_T_UTIL_HPP

#include <cxxtest/TestSuite.h>

class TestUtilAnswerProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilAtomTable : public CxxTest::TestSuite {
 public:
    void testAtom();
};

class TestUtilBackupFile : public CxxTest::TestSuite {
 public:
    void testExpand();
};

class TestUtilBaseSlaveRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilBaseSlaveRequestSender : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCall();
};

class TestUtilConstantAnswerProvider : public CxxTest::TestSuite {
 public:
    void testIt();
    void testYes();
    void testNo();
};

class TestUtilKey : public CxxTest::TestSuite {
 public:
    void testParse();
    void testFormat();
    void testUnique();
};

class TestUtilKeyString : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilKeymap : public CxxTest::TestSuite {
 public:
    void testKeymap();
    void testChange();
};

class TestUtilKeymapTable : public CxxTest::TestSuite {
 public:
    void testKeymapTable();
};

class TestUtilMath : public CxxTest::TestSuite {
 public:
    void testDivideAndRound();
    void testDivideAndRoundToEven();
    void testGetHeading();
    void testSquareInteger();
};

class TestUtilRandomNumberGenerator : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRequestDispatcher : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRequestReceiver : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDie();
};

class TestUtilRequestThread : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSkinColor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveRequest : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSlaveRequestSender : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCall();
};

class TestUtilString : public CxxTest::TestSuite {
 public:
    void testStringMatch();
    void testParseRange();
    void testParsePlayer();
};

class TestUtilUnicodeChars : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
