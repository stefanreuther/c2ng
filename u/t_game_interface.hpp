/**
  *  \file u/t_game_interface.hpp
  *  \brief Tests for game::interface
  */
#ifndef C2NG_U_T_GAME_INTERFACE_HPP
#define C2NG_U_T_GAME_INTERFACE_HPP

#include <cxxtest/TestSuite.h>

class TestGameInterfaceBaseTaskPredictor : public CxxTest::TestSuite {
 public:
    void testBuild();
    void testRecycle();
    void testShipyard();
    void testBuildShipCommand();
    void testSetFCodeCommand();
    void testSetMissionCommand();
    void testFixShipCommand();
};

class TestGameInterfaceCargoFunctions : public CxxTest::TestSuite {
 public:
    void testCheckCargoSpecArg();
    void testCAdd();
    void testCCompare();
    void testCDiv();
    void testCExtract();
    void testCMul();
    void testCRemove();
    void testCSub();
};

class TestGameInterfaceCompletionList : public CxxTest::TestSuite {
 public:
    void testInit();
    void testAddCandidate();
    void testAddCandidateDollar();
    void testAddCandidateMixedCase();
    void testAddBuildCompletionList();
};

class TestGameInterfaceContextProvider : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameInterfaceDrawingProperty : public CxxTest::TestSuite {
 public:
    void testGetLine();
    void testGetCircle();
    void testGetMarker();
    void testSetLine();
    void testSetCircle();
    void testSetMarker();
};

class TestGameInterfaceEngineContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceEngineProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testSet();
};

class TestGameInterfaceExplosionContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceFriendlyCodeContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceGlobalCommands : public CxxTest::TestSuite {
 public:
    void testCheckPlayerArgNull();
    void testCheckPlayerArgWrong();
    void testCheckPlayerArgInt();
    void testCheckPlayerArgArray();
    void testCheckPlayerArgIntRange();
    void testCheckPlayerArgArrayRange();
    void testCheckPlayerArgVector();
};

class TestGameInterfaceInboxContext : public CxxTest::TestSuite {
 public:
    void testProperties();
    void testWrite();
    void testText();
    void testIteration();
};

class TestGameInterfaceInboxSubsetValue : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testIteration();
    void testIndexing();
};

class TestGameInterfaceIteratorProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceMissionContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceNotificationStore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHeader();
    void testResume();
    void testReplace();
};

class TestGameInterfacePluginProperty : public CxxTest::TestSuite {
 public:
    void testGet();
};

class TestGameInterfaceProcessListEditor : public CxxTest::TestSuite {
 public:
    void testInit();
    void testSetOneTerminated();
    void testSetOneSuspended();
    void testSetAllRunnable();
    void testSetAllSuspended();
    void testCommit();
    void testSetPriority();
    void testNotification();
    void testReadNotification();
};

class TestGameInterfacePropertyList : public CxxTest::TestSuite {
 public:
    void testShip();
    void testPlanet();
    void testEmpty();
    void testOther();
};

class TestGameInterfaceRichTextFunctions : public CxxTest::TestSuite {
 public:
    void testRAdd();
    void testRMid();
    void testRString();
    void testRLen();
    void testRStyle();
    void testRLink();
    void testRXml();
};

class TestGameInterfaceShipTaskPredictor : public CxxTest::TestSuite {
 public:
    void testMovement();
    void testMoveToCommand();
    void testSetWaypointCommand();
    void testMoveTowardsCommand();
    void testSetSpeedCommand();
    void testSetFCodeCommand();
    void testSetMissionCommand();
    void testSetFCodeHyperjump();
};

class TestGameInterfaceUfoContext : public CxxTest::TestSuite {
 public:
    void testTypes();
};

class TestGameInterfaceUserInterfaceProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceUserInterfacePropertyAccessor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceUserInterfacePropertyStack : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testMulti();
};

#endif
