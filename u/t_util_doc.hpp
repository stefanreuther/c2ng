/**
  *  \file u/t_util_doc.hpp
  *  \brief Tests for util::doc
  */
#ifndef C2NG_U_T_UTIL_DOC_HPP
#define C2NG_U_T_UTIL_DOC_HPP

#include <cxxtest/TestSuite.h>

class TestUtilDocBlobStore : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilDocFileBlobStore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPortability();
};

class TestUtilDocHelpImport : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIt2();
    void testIt3();
    void testIt4();
};

class TestUtilDocHtmlRenderer : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testLink();
    void testKeyList();
    void testImage();
    void testImageScaled();
    void testImageCropped();
    void testTable();
    void testDefinition();
    void testKey();
    void testMarkup();
    void testMarkup2();
    void testMarkup3();
    void testMarkup4();
    void testMarkup5();
    void testMarkup6();
};

class TestUtilDocIndex : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testEmptySave();
    void testBuild();
    void testAttributes();
    void testStructureIO();
    void testErrors();
    void testRelated();
};

class TestUtilDocInternalBlobStore : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilDocLoggingVerifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilDocRenderOptions : public CxxTest::TestSuite {
 public:
    void testSet();
    void testLink();
};

class TestUtilDocSingleBlobStore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPortability();
    void testReuse();
    void testFail();
    void testZero();
};

class TestUtilDocSummarizingVerifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilDocTextImport : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilDocVerifier : public CxxTest::TestSuite {
 public:
    void testStatic();
    void testGetNodeName();
    void testWarnNodeHasNoId();
    void testWarnNodeHasNoTitle();
    void testWarnNodeIsEmpty();
    void testWarnUnresolvableContent();
    void testWarnUniqueSecondaryId();
    void testWarnDuplicateAddress();
    void testWarnInvalidComment();
    void testWarnAssetLink();
    void testWarnDocumentImage();
    void testWarnInvalidAsset();
    void testWarnDeadLink();
    void testWarnBadAnchor();
    void testInfoUsedTags();
    void testInfoUsedClasses();
    void testInfoExternalLinks();
    void testInfoSiteLinks();
};

#endif
