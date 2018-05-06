/**
  *  \file u/t_game_v3.hpp
  */
#ifndef C2NG_U_T_GAME_V3_HPP
#define C2NG_U_T_GAME_V3_HPP

#include <cxxtest/TestSuite.h> 

class TestGameV3Command : public CxxTest::TestSuite {
 public:
    void testCommands();
    void testMessageIntroducer();
    void testProto();
};

class TestGameV3CommandContainer : public CxxTest::TestSuite {
 public:
    void testContainer();
    void testSequence();
};

class TestGameV3CommandExtra : public CxxTest::TestSuite {
 public:
    void testEvents();
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

class TestGameV3HConfig : public CxxTest::TestSuite {
 public:
    void testPack();
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
};

class TestGameV3StringVerifier : public CxxTest::TestSuite {
 public:
    void testMain();
    void testFCode();
    void testMessage();
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
