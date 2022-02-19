/**
  *  \file u/t_server_file_ca.hpp
  *  \brief Tests for server::file::ca
  */
#ifndef C2NG_U_T_SERVER_FILE_CA_HPP
#define C2NG_U_T_SERVER_FILE_CA_HPP

#include <cxxtest/TestSuite.h>

class TestServerFileCaCommit : public CxxTest::TestSuite {
 public:
    void testStore();
    void testParse();
};

class TestServerFileCaDirectoryEntry : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testOther();
    void testConstruct();
    void testCompare();
    void testCompare2();
};

class TestServerFileCaDirectoryHandler : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testDir();
    void testTree();
    void testOrder();
    void testRefCount();
    void testSubdir();
    void testCopy();
};

class TestServerFileCaGarbageCollector : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testGarbage();
    void testSliced();
    void testModified();
    void testErrorCommit();
    void testErrorTree();
};

class TestServerFileCaInternalObjectCache : public CxxTest::TestSuite {
 public:
    void testIt();
    void testExpire();
};

class TestServerFileCaInternalReferenceCounter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileCaObjectCache : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerFileCaObjectId : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHash();
};

class TestServerFileCaObjectStore : public CxxTest::TestSuite {
 public:
    void testGetObject();
    void testAddObject();
    void testLarge();
    void testCache();
    void testCache2();
};

class TestServerFileCaReferenceCounter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerFileCaReferenceUpdater : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerFileCaRoot : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testPreload();
    void testGarbage();
};

#endif
