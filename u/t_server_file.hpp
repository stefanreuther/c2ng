/**
  *  \file u/t_server_file.hpp
  *  \brief Tests for server::file
  */
#ifndef C2NG_U_T_SERVER_FILE_HPP
#define C2NG_U_T_SERVER_FILE_HPP

#include <cxxtest/TestSuite.h>

class TestServerFileClientDirectory : public CxxTest::TestSuite {
 public:
    void testRead();
    void testStat();
    void testList();
    void testRemoteError();
    void testLocalError();
    void testSubdir();
};

class TestServerFileClientDirectoryHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileDirectoryHandler : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testFind();
    void testConvertSize();
};

class TestServerFileDirectoryHandlerFactory : public CxxTest::TestSuite {
 public:
    void testPathName();
    void testCreate();
    void testCreateInternal();
    void testCreateCA();
    void testCreateSubdir();
    void testCreateErrors();
    void testCreateRemote();
    void testCreateCAPreload();
    void testCreateCAFail();
    void testCreateCAFail2();
    void testCreateCAFailNoGC();
};

class TestServerFileDirectoryWrapper : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEnum();
    void testEntry();
};

class TestServerFileFileBase : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testCreateDirectory();
    void testGet();
    void testTestFiles();
    void testProperty();
    void testPropertyPermissions();
    void testPropertyFile();
    void testCreateDirectoryTree();
    void testStat();
    void testGetDirPermission();
    void testGetDirContent();
    void testGetDirContent2();
    void testRemove();
    void testRemoveNonemptyDir();
    void testRemoveNonemptyDir2();
    void testRemoveNonemptyDir3();
    void testRemoveTree();
    void testRemoveTree1();
    void testRemoveTree2();
    void testRemoveTree3();
    void testRemoveTreeFail();
    void testRemoveTreePerm();
    void testUsage();
    void testPut();
    void testLimits();
    void testCopy();
    void testCopyUnderlay();
    void testSnoop();
};

class TestServerFileFileGame : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testReg();
    void testGame();
    void testGameProps();
};

class TestServerFileGameStatus : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testReg();
    void testGame();
    void testBoth();
    void testInvalidResult();
    void testInvalidKey();
};

class TestServerFileInternalFileServer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileItem : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerFileRaceNames : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileRoot : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileSession : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerFileUtils : public CxxTest::TestSuite {
 public:
    void testCopy();
    void testCopyFlat();
    void testCopyConflict();
    void testCopyConflict2();
    void testRemoveDir();
    void testSyncEmpty();
    void testSyncIntoEmpty();
    void testSyncFromEmpty();
    void testSyncSame();
    void testSyncFileOverDir();
    void testSyncDirOverFile();
};

#endif
