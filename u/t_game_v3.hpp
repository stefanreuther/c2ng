/**
  *  \file u/t_game_v3.hpp
  */
#ifndef C2NG_U_T_GAME_V3_HPP
#define C2NG_U_T_GAME_V3_HPP

#include <cxxtest/TestSuite.h>

class TestGameV3AttachmentConfiguration : public CxxTest::TestSuite {
 public:
    void testSame();
    void testDifferent();
    void testRaceNameDefault();
    void testRaceNameAccept();
    void testRaceNameReject();
    void testEmpty();
};

class TestGameV3AttachmentUnpacker : public CxxTest::TestSuite {
 public:
    void testInit();
    void testResult();
    void testResultDisabled();
    void testResult30();
    void testResultLeech();
    void testResultDamagedNames();
    void testUtilConfig();
    void testUtilFileGood();
    void testUtilFileTruncated();
    void testUtilFileName();
    void testUtilFileInvalidName();
    void testUtilFileInvalidName2();
    void testUtilFileInvalidName3();
    void testUtilFileBlacklistedName();
    void testUtilFileBlacklistedName2();
    void testUtilFileBlacklistedName3();
    void testUtilFileBlacklistedName4();
    void testUtilFileNameBorderCase();
    void testUtilFileNameBorderCase2();
    void testUtilFileMulti();
    void testUtilFileOrder();
    void testUtilFileOrder2();
    void testSave();
    void testSaveBlacklist();
    void testSaveBlacklistEnabled();
    void testDropUnchanged();
    void testDropUnchanged2();
    void testDropUnchanged3();
    void testDropUnchanged4();
    void testLoadDirectory();
    void testLoadDirectoryEmpty();
    void testToString();
    void testMultipleTimestamps();
    void testUtilLong();
    void testUtilLongShort();
    void testUtilLongBadLink();
    void testUtilLongBadLink2();
    void testUtilLongBadContent();
};

class TestGameV3Command : public CxxTest::TestSuite {
 public:
    void testCommands();
    void testMessageIntroducer();
    void testProto();
    void testAffectedShip();
    void testAffectedPlanet();
    void testAffectedMinefield();
    void testOrderConstraints();
    void testGetCommandInfo();
};

class TestGameV3CommandContainer : public CxxTest::TestSuite {
 public:
    void testContainer();
    void testSequence();
    void testReplacePointer();
    void testNonReplaceable();
    void testLoadNormal();
    void testLoadTimeMatch();
    void testLoadTimeMismatch();
    void testRemove();
    void testRemoveByReference();
};

class TestGameV3CommandExtra : public CxxTest::TestSuite {
 public:
    void testEvents();
    void testGet();
};

class TestGameV3ControlFile : public CxxTest::TestSuite {
 public:
    void testSave();
    void testSaveDOS();
    void testSaveWin();
    void testSaveBig();
    void testLoadDOS();
    void testLoadWindows();
    void testLoadEmpty();
    void testRange();
};

class TestGameV3DirectoryScanner : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testResult();
    void testMultiResult();
    void testNewResult();
    void testBrokenResult();
    void testTruncatedResult();
    void testGen();
    void testMultiGen();
    void testConflictGen();
    void testBadGen();
    void testGenAndNewResult();
    void testGenAndSameResult();
    void testGenAndOldResult();
    void testGenOnlyResult();
    void testResultAndTurn();
    void testResultAndMismatchingTurn();
    void testResultAndWrongOwnerTurn();
    void testResultAndBadTurn();
    void testHostVersion();
    void testHostVersionResult();
};

class TestGameV3FizzFile : public CxxTest::TestSuite {
 public:
    void testMissing();
    void testShort();
    void testNormal();
};

class TestGameV3GenExtra : public CxxTest::TestSuite {
 public:
    void testAccess();
};

class TestGameV3GenFile : public CxxTest::TestSuite {
 public:
    void testFile();
    void testPassword();
    void testResult();
    void testScore();
};

class TestGameV3HConfig : public CxxTest::TestSuite {
 public:
    void testPack();
    void testRoundtrip();
};

class TestGameV3InboxFile : public CxxTest::TestSuite {
 public:
    void testDecodeMessage();
    void testInboxFile();
    void testInboxFileErrors();
    void testInboxFileTweakCC();
    void testInboxFileTweakUniversal();
};

class TestGameV3Loader : public CxxTest::TestSuite {
 public:
    void testLoadPlanets();
    void testLoadShips();
    void testLoadBases();
    void testLoadTurnFile();
    void testMissingShip();
    void testMissingPlanet();
    void testMissingBase();
    void testUnplayedShip();
    void testUnplayedPlanet();
    void testUnplayedBase();
    void testNoBase();
    void testInvalidCommand();
    void testInvalidFile();
    void testInvalidPlayer();
    void testAllianceCommand();
    void testMessageCommand();
};

class TestGameV3OutboxReader : public CxxTest::TestSuite {
 public:
    void testLoad30Empty();
    void testLoad30Zero();
    void testLoad30ZeroLength();
    void testLoad30One();
    void testLoad30Host();
    void testLoad35Empty();
    void testLoad35Zero();
    void testLoad35ZeroLength();
    void testLoad35One();
    void testLoad35Two();
    void testLoad35Invalid();
};

class TestGameV3Packer : public CxxTest::TestSuite {
 public:
    void testUnpackShip();
    void testUnpackPlanet();
    void testUnpackBase();
};

class TestGameV3PasswordChecker : public CxxTest::TestSuite {
 public:
    void testNoPassword();
    void testCheckDisabled();
    void testAskSuccess();
    void testAskFailure();
    void testAskCancel();
};

class TestGameV3RegistrationKey : public CxxTest::TestSuite {
 public:
    void testInit();
    void testFileRoundtrip();
    void testBufferRoundtrip();
    void testSetLine();
    void testInitRoundTrip();
};

class TestGameV3ResultFile : public CxxTest::TestSuite {
 public:
    void test30();
    void test35();
};

class TestGameV3ResultLoader : public CxxTest::TestSuite {
 public:
    void testLoadTurnFile();
    void testInvalidFile();
};

class TestGameV3Reverter : public CxxTest::TestSuite {
 public:
    void testGetPreviousFriendlyCode();
    void testShipMission();
    void testMinBuildings();
    void testLocation();
    void testLocationEmpty();
    void testLocationHalf();
};

class TestGameV3SpecificationLoader : public CxxTest::TestSuite {
 public:
    void testStandard();
    void testTruehullVerification();
    void testLoadHullfunc();
    void testMissing();
    void testHullfuncAssignment();
    void testHullfuncAssignmentShip();
    void testHullfuncAssignmentPlayerRace();
    void testHullfuncAssignmentLevel();
    void testLoadFCodes();
    void testLoadMissions();
};

class TestGameV3StringVerifier : public CxxTest::TestSuite {
 public:
    void testMain();
    void testFCode();
    void testMessage();
    void testClone();
};

class TestGameV3Structures : public CxxTest::TestSuite {
 public:
    void testHeader();
};

class TestGameV3TurnFile : public CxxTest::TestSuite {
 public:
    void testInit();
    void testParse30();
    void testParse30Attachment();
    void testParse30AttachmentHeader();
    void testParse35();
    void testParse35Header();
    void testParseDamaged();
    void testDeleteCommand();
    void testSendTHostAllies();
    void testSendMessageData();
    void testMakeShipCommands();
    void testMakePlanetCommands();
    void testMakeBaseCommands();
    void testModify();
    void testSort();
    void testSetRegistrationKey();
    void testAddFile();
    void testStatics();
};

class TestGameV3UndoInformation : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoPlanet();
    void testInit();
    void testSupplySale();
    void testTorpedoUpgrade();
    void testTorpedoShip();
    void testSupplyShip();
};

class TestGameV3Utils : public CxxTest::TestSuite {
 public:
    void testLoadRaceNames();
    void testEncryptTarget();
};

#endif
