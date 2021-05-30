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

class TestGameV3FizzFile : public CxxTest::TestSuite {
 public:
    void testMissing();
    void testShort();
    void testNormal();
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

class TestGameV3Loader : public CxxTest::TestSuite {
 public:
    void testLoadPlanets();
    void testLoadShips();
    void testLoadBases();
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

class TestGameV3RegistrationKey : public CxxTest::TestSuite {
 public:
    void testInit();
    void testFileRoundtrip();
    void testBufferRoundtrip();
};

class TestGameV3ResultFile : public CxxTest::TestSuite {
 public:
    void test30();
    void test35();
};

class TestGameV3ResultLoader : public CxxTest::TestSuite {
 public:
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

class TestGameV3Reverter : public CxxTest::TestSuite {
 public:
    void testGetPreviousFriendlyCode();
    void testShipMission();
    void testMinBuildings();
    void testLocation();
    void testLocationEmpty();
    void testLocationHalf();
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

#endif
