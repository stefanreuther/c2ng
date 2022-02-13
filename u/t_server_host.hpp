/**
  *  \file u/t_server_host.hpp
  *  \brief Tests for server::host
  */
#ifndef C2NG_U_T_SERVER_HOST_HPP
#define C2NG_U_T_SERVER_HOST_HPP

#include <cxxtest/TestSuite.h>

class TestServerHostCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHelp();
};

class TestServerHostConfiguration : public CxxTest::TestSuite {
 public:
    void testBase();
    void testTime();
};

class TestServerHostConfigurationBuilder : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerHostCron : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerHostCronImpl : public CxxTest::TestSuite {
 public:
    void testMaster();
    void testMasterJoin();
    void testMasterJoinTimeless();
    void testPreparing();
    void testFinished();
    void testRunningInitial();
    void testRunningInitial2();
    void testRunningNoSchedule();
    void testRunningWeeklyNormal();
    void testRunningWeeklyDelayed();
    void testRunningWeeklyDelayedEdge();
    void testRunningDailyNormal();
    void testRunningDailyNormal2();
    void testRunningDailyMid();
    void testRunningDailyEarly();
    void testRunningManual();
    void testRunningManualTrigger();
    void testRunningManualEarly();
    void testRunningManualEarlyMiss();
    void testRunningQuick();
    void testRunningQuickMiss();
    void testRunningQuickPartial();
    void testRunningExpireWeekly();
    void testRunningExpireWeekly2();
    void testRunningExpireDaily();
    void testRunningExpireDaily2();
    void testRunningExpireDate();
    void testRunningExpireUpdate();
    void testRunningChangeProtection();
    void testSuspend();
    void testSuspendStartup();
};

class TestServerHostExporter : public CxxTest::TestSuite {
 public:
    void testIt();
    void testUnpackBackups();
};

class TestServerHostGame : public CxxTest::TestSuite {
 public:
    void testCreateNormal();
    void testCreateNonexistant();
    void testCreateUnchecked();
    void testDescribe();
    void testGetState();
    void testSetStateNormal();
    void testSetStatePrivate();
    void testSetStateFinish();
    void testSetStateFinishAmbiguous();
    void testGetType();
    void testSetType();
    void testSetOwner();
    void testDescribeSlot();
    void testDescribeVictoryNone();
    void testDescribeVictoryTurn();
    void testDescribeVictoryScore();
    void testDescribeVictoryReferee();
};

class TestServerHostGameArbiter : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGuard();
};

class TestServerHostGameCreator : public CxxTest::TestSuite {
 public:
    void testPickDayTime();
    void testPickDayTime2();
    void testCreateGame();
    void testInitializeGame();
    void testCopy();
};

class TestServerHostHostCron : public CxxTest::TestSuite {
 public:
    void testNull();
    void testNonNull();
    void testListPermissions();
};

class TestServerHostHostFile : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerHostHostGame : public CxxTest::TestSuite {
 public:
    void testNewGame();
    void testCloneGame();
    void testCloneGameStatus();
    void testCloneGameErrorUser();
    void testCloneGameErrorLocked();
    void testCloneGameId();
    void testListGame();
    void testGameInfo();
    void testSetConfigSimple();
    void testSetConfigTool();
    void testSetConfigToolError();
    void testSetConfigEnd();
    void testSetConfigEndHide();
    void testTools();
    void testUpdateAdmin();
    void testUpdateUser();
    void testGetPermissions();
    void testVictoryCondition();
    void testListUserGames();
    void testFilters();
};

class TestServerHostHostKey : public CxxTest::TestSuite {
 public:
    void testAdmin();
    void testNormal();
    void testFileError();
    void testReg();
    void testEmpty();
    void testGenerate();
};

class TestServerHostHostPlayer : public CxxTest::TestSuite {
 public:
    void testJoin();
    void testJoinFail();
    void testJoinFailUser();
    void testResign();
    void testResignCombo();
    void testResignCombo2();
    void testResignComboPerm();
    void testSubstitute();
    void testSubstituteUser();
    void testSubstituteEmpty();
    void testAddPlayer();
    void testSlotInfo();
    void testDirectory();
    void testDirectoryErrorFilePerm();
    void testDirectoryErrorUserPerm();
    void testDirectoryErrorGame();
    void testDirectoryErrorChange();
    void testDirectoryConflict();
    void testDirectoryMove();
    void testCheckFile();
    void testGameState();
    void testGetSet();
    void testProfilePermission();
};

class TestServerHostHostSchedule : public CxxTest::TestSuite {
 public:
    void testAddQuery();
    void testAddAll();
    void testInit();
    void testDaytime();
    void testDrop();
    void testPreview();
};

class TestServerHostHostTool : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testList();
    void testCopy();
    void testErrors();
    void testDifficulty();
    void testComputedDifficulty();
};

class TestServerHostHostTurn : public CxxTest::TestSuite {
 public:
    void testSubmit();
    void testSubmitEmpty();
    void testSubmitEmptyGame();
    void testSubmitStale();
    void testSubmitStaleGame();
    void testSubmitWrongUser();
    void testSubmitEmail();
    void testSubmitEmailCase();
    void testSubmitWrongEmail();
    void testSubmitEmailUser();
    void testSubmitEmailStale();
    void testStatus();
    void testStatusTempEnable();
    void testStatusErrors();
};

class TestServerHostInstaller : public CxxTest::TestSuite {
 public:
    void testPrecious();
};

class TestServerHostKeyStore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testListEmpty();
    void testExpire();
    void testNoStore();
    void testNoLimit();
};

class TestServerHostResultSender : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testMulti();
    void testConfig();
    void testProfile();
    void testDefaultProfile();
    void testProfileOverride();
    void testGameOverride();
    void testGameDefault();
    void testExtraFiles();
};

class TestServerHostSchedule : public CxxTest::TestSuite {
 public:
    void testHostDate();
    void testData();
    void testPersist();
    void testCondition();
    void testDescribe();
    void testDescribe2();
};

class TestServerHostSession : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerHostTalkAdapter : public CxxTest::TestSuite {
 public:
    void testGameStart();
    void testGameStartPrivate();
    void testGameEndNoForum();
    void testGameEndNormal();
    void testGameEndElsewhere();
    void testNameChangeNoForum();
    void testNameChangeNormal();
    void testTypeChangeNoForum();
    void testTypeChangeNormal();
};

class TestServerHostTalkListener : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
