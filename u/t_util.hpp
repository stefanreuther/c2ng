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

class TestUtilApplication : public CxxTest::TestSuite {
 public:
    void testInit();
    void testExit();
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

class TestUtilCharsetFactory : public CxxTest::TestSuite {
 public:
    void testIteration();
    void testNames();
    void testCodes();
    void testErrors();
};

class TestUtilConfigurationFile : public CxxTest::TestSuite {
 public:
    void testLoad();
    void testSave();
    void testFind();
    void testMergePreserve();
    void testMergeNamespaced();
    void testRemove();
    void testAdd();
};

class TestUtilConfigurationFileParser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilConstantAnswerProvider : public CxxTest::TestSuite {
 public:
    void testIt();
    void testYes();
    void testNo();
};

class TestUtilDigest : public CxxTest::TestSuite {
 public:
    void testIt();
    void testStaticInstance();
    void testDynamicType();
};

class TestUtilFileNamePattern : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFail();
    void testLiterals();
    void testCopy();
    void testPrepared();
};

class TestUtilFileParser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilHelpIndex : public CxxTest::TestSuite {
 public:
    void testMulti();
    void testMissing();
};

class TestUtilInstructionList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testReadInsnOnly();
    void testAppend();
};

class TestUtilIo : public CxxTest::TestSuite {
 public:
    void testStorePascalString();
    void testStorePascalStringTruncate();
};

class TestUtilKey : public CxxTest::TestSuite {
 public:
    void testParse();
    void testFormat();
    void testUnique();
    void testClassify();
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
    void testRound();
    void testDistance();
};

class TestUtilMessageCollector : public CxxTest::TestSuite {
 public:
    void testForward();
    void testBackward();
    void testWrap();
};

class TestUtilMessageMatcher : public CxxTest::TestSuite {
 public:
    void testErrors();
    void testMatch();
};

class TestUtilMessageNotifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilPrefixArgument : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSequences();
    void testCancel();
};

class TestUtilProcessRunner : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRandomNumberGenerator : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRange();
    void testFullRange();
    void testReset();
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

class TestUtilRunLengthExpandTransform : public CxxTest::TestSuite {
 public:
    void testIt();
    void testBad();
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
    void testFormatOptions();
    void testEncodeMimeHeader();
    void testParseBoolean();
};

class TestUtilStringInstructionList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testReadWrong();
    void testSwap();
};

class TestUtilStringList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSort();
    void testCopy();
};

class TestUtilStringParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilUnicodeChars : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilUpdater : public CxxTest::TestSuite {
 public:
    void testTrue();
    void testFalse();
};

class TestUtilstring : public CxxTest::TestSuite {
 public:
    void testFormatName();
    void testEncodeHtml();
};

#endif
