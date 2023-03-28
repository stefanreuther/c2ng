/**
  *  \file u/t_game_interface.hpp
  *  \brief Tests for game::interface
  */
#ifndef C2NG_U_T_GAME_INTERFACE_HPP
#define C2NG_U_T_GAME_INTERFACE_HPP

#include <cxxtest/TestSuite.h>

class TestGameInterfaceBaseProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNoBase();
    void testNoPlanet();
    void testShipyard();
    void testSet();
};

class TestGameInterfaceBaseTaskBuildCommandParser : public CxxTest::TestSuite {
 public:
    void testIt();
};

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

class TestGameInterfaceBeamContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testIteration();
    void testNull();
    void testCreate();
    void testSet();
};

class TestGameInterfaceBeamFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
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

class TestGameInterfaceCargoMethod : public CxxTest::TestSuite {
 public:
    void testCargoTransferPlanet();
    void testCargoTransferShip();
    void testCargoUnload();
};

class TestGameInterfaceCommandInterface : public CxxTest::TestSuite {
 public:
    void testAdd();
    void testAddNull();
    void testAddBadCommand();
    void testAddNoGame();
    void testAddNoCommand();
    void testDelete();
    void testDeleteNull();
    void testDeleteBadCommand();
    void testDeleteNoGame();
    void testDeleteNoCommand();
    void testGet();
    void testGetNoGame();
    void testGetNoCommandExtra();
    void testGetNoCommandContainer();
};

class TestGameInterfaceCompletionList : public CxxTest::TestSuite {
 public:
    void testInit();
    void testAddCandidate();
    void testAddCandidateDollar();
    void testAddCandidateMixedCase();
    void testAddBuildCompletionList();
};

class TestGameInterfaceComponentProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testSet();
};

class TestGameInterfaceConfigurationEditorContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testMake();
    void testSequence();
    void testSubtree();
    void testFailures();
};

class TestGameInterfaceConsoleCommands : public CxxTest::TestSuite {
 public:
    void testNormal();
};

class TestGameInterfaceContextProvider : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameInterfaceCostSummaryContext : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameInterfaceDrawingContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testSet();
    void testSetDeleted();
    void testCreate();
    void testCreateEmpty();
};

class TestGameInterfaceDrawingFunction : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceDrawingMethod : public CxxTest::TestSuite {
 public:
    void testUpdate();
    void testDelete();
};

class TestGameInterfaceDrawingProperty : public CxxTest::TestSuite {
 public:
    void testGetLine();
    void testGetRectangle();
    void testGetCircle();
    void testGetMarker();
    void testSetLine();
    void testSetCircle();
    void testSetMarker();
};

class TestGameInterfaceEngineContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIteration();
    void testNull();
    void testCreate();
    void testSet();
};

class TestGameInterfaceEngineFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceEngineProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testSet();
};

class TestGameInterfaceExplosionContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIteration();
    void testNull();
    void testCreate();
    void testCreateEmpty();
    void testSet();
};

class TestGameInterfaceExplosionFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceExplosionProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIt2();
};

class TestGameInterfaceFriendlyCodeContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEnum();
    void testRange();
};

class TestGameInterfaceFriendlyCodeFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceFriendlyCodeProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceGlobalActionContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFailures();
    void testContext();
    void testMake();
    void testMakeFail();
};

class TestGameInterfaceGlobalActions : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testNoShips();
    void testNoPlanets();
    void testNoUnmarked();
    void testNoNumericFC();
    void testNoSpecialFC();
    void testList();
    void testListNoShips();
    void testListNoPlanets();
    void testCancel();
    void testLock();
    void testLockIgnore();
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
    void testCheckPlayerArg2DArray();
    void testAddConfig();
    void testAddFCode();
    void testAddPref();
    void testAuthPlayer();
    void testCCHistoryShowTurn();
    void testCCSelectionExec();
    void testCreateConfigOption();
    void testCreatePrefOption();
    void testExport();
    void testNewCannedMarker();
    void testNewCircle();
    void testNewRectangle();
    void testNewRectangleRaw();
    void testNewLine();
    void testNewLineRaw();
    void testNewMarker();
    void testHistoryLoadTurn();
    void testSaveGame();
    void testSendMessage();
};

class TestGameInterfaceGlobalContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceGlobalFunctions : public CxxTest::TestSuite {
 public:
    void testAutoTask();
    void testCfg();
    void testCfgNoRoot();
    void testCfgNoGame();
    void testDistance();
    void testDistanceNoGame();
    void testFormat();
    void testIsSpecialFCode();
    void testIsSpecialFCodeNoShipList();
    void testObjectIsAt();
    void testPlanetAt();
    void testPlanetAtEmpty();
    void testPref();
    void testPrefNoRoot();
    void testQuote();
    void testRandom();
    void testRandomFCode();
    void testTranslate();
    void testTruehull();
    void testTruehullNoGame();
    void testTruehullNoRoot();
};

class TestGameInterfaceGlobalProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHalf();
    void testEmpty();
    void testSet();
    void testSetEmpty();
    void testHostVersions();
};

class TestGameInterfaceHullContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testIteration();
    void testNull();
    void testCreate();
    void testSet();
};

class TestGameInterfaceHullFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceHullProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testSet();
    void testSpecial();
};

class TestGameInterfaceInboxContext : public CxxTest::TestSuite {
 public:
    void testProperties();
    void testWrite();
    void testText();
    void testIteration();
};

class TestGameInterfaceInboxFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceInboxSubsetValue : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testIteration();
    void testIndexing();
};

class TestGameInterfaceIonStormContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testSet();
    void testCommand();
    void testCreate();
    void testCreateEmpty();
    void testAccessEmpty();
};

class TestGameInterfaceIonStormFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmptyUniverse();
    void testEmptySession();
};

class TestGameInterfaceIonStormProperty : public CxxTest::TestSuite {
 public:
    void testGet();
    void testGetEmpty();
    void testGetMostlyEmpty();
    void testSet();
};

class TestGameInterfaceIteratorContext : public CxxTest::TestSuite {
 public:
    void testCreateObjectShip();
    void testCreateObjectPlanet();
    void testCreateObjectMinefield();
    void testCreateObjectIonStorm();
    void testIteratorContextBasics();
    void testIteratorContextNativeCreate();
    void testIteratorContextNativeCreateFail();
    void testIteratorContextScriptCreate();
    void testIteratorContextScriptCreateFail();
    void testIteratorContextProperties();
    void testIteratorContextCurrent();
    void testIteratorContextSorted();
};

class TestGameInterfaceIteratorProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceLabelExtra : public CxxTest::TestSuite {
 public:
    void testLink();
    void testEarly();
    void testLate();
    void testSelfModify();
    void testOtherModify();
    void testConfig();
    void testConfigError();
    void testConfigError2();
    void testConfigEmpty();
    void testConfigEmpty2();
    void testClear();
    void testBadState();
};

class TestGameInterfaceLabelVector : public CxxTest::TestSuite {
 public:
    void testStorage();
    void testStatus();
    void testStatus2();
    void testStatus3();
    void testCompile();
};

class TestGameInterfaceLoadContext : public CxxTest::TestSuite {
 public:
    void testLoadContextNormal();
    void testLoadContextEmpty();
    void testOthers();
};

class TestGameInterfaceMailboxContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testAdd();
    void testLoadUtilData();
    void testLoadFile();
    void testInterface();
};

class TestGameInterfaceMinefieldContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testIteration();
    void testCommand();
    void testCreate();
    void testCreateEmpty();
};

class TestGameInterfaceMinefieldFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceMinefieldMethod : public CxxTest::TestSuite {
 public:
    void testMark();
    void testDelete();
};

class TestGameInterfaceMinefieldProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceMissionContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIteration();
    void testNull();
};

class TestGameInterfaceMissionFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceMissionProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIt2();
};

class TestGameInterfaceNotificationFunctions : public CxxTest::TestSuite {
 public:
    void testNotifyConfirmed();
    void testScenario();
    void testErrors();
};

class TestGameInterfaceNotificationStore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHeader();
    void testResume();
    void testResume2();
    void testReplace();
};

class TestGameInterfaceObjectCommand : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMark4();
    void testMark2();
    void testUnmark4();
    void testUnmark2();
};

class TestGameInterfacePlanetContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
    void testNull();
    void testIteration();
    void testCreate();
};

class TestGameInterfacePlanetFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfacePlanetMethod : public CxxTest::TestSuite {
 public:
    void testParseBuildShipCommand();
    void testMarkUnmark();
    void testSetComment();
    void testFixShip();
    void testRecycleShip();
    void testBuildBase();
    void testAutoBuild();
    void testBuildDefense();
    void testBuildFactories();
    void testBuildMines();
    void testSetColonistTax();
    void testSetNativeTax();
    void testSetFCode();
    void testSetMission();
    void testBuildBaseDefense();
    void testSetTech();
    void testBuildFighters();
    void testBuildEngines();
    void testBuildHulls();
    void testBuildLaunchers();
    void testBuildBeams();
    void testBuildTorps();
    void testSellSupplies();
    void testBuildShip();
    void testCargoTransfer();
    void testAutoTaxColonists();
    void testAutoTaxNatives();
    void testApplyBuildGoals();
};

class TestGameInterfacePlanetProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfacePlayerContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testIteration();
    void testCreate();
    void testCreateEmpty();
};

class TestGameInterfacePlayerFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfacePlayerProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfacePluginContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNonExistant();
    void testCreateRegular();
    void testCreateNull();
    void testCreateUnknown();
    void testCreateErrors();
};

class TestGameInterfacePluginProperty : public CxxTest::TestSuite {
 public:
    void testGet();
};

class TestGameInterfacePlugins : public CxxTest::TestSuite {
 public:
    void testLoadResource();
    void testLoadHelpFile();
    void testLoadScript();
    void testExecScript();
    void testUnloaded();
    void testLoadSuccess();
    void testLoadFailure();
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

class TestGameInterfaceReferenceContext : public CxxTest::TestSuite {
 public:
    void testGetReferenceProperty();
    void testMakeObjectValue();
    void testGetReferenceTypeName();
    void testParseReferenceTypeName();
    void testReferenceContext();
    void testIFLocationReference();
    void testIFReference();
    void testCheckReferenceArg();
};

class TestGameInterfaceReferenceListContext : public CxxTest::TestSuite {
 public:
    void testCreate();
    void testAdd();
    void testAddObjects();
    void testAddObjectsAt();
    void testObjects();
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
    void testRAlign();
};

class TestGameInterfaceRichTextValue : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceSelectionFunctions : public CxxTest::TestSuite {
 public:
    void testSelectionSave();
    void testSelectionLoad();
};

class TestGameInterfaceShipContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
    void testNull();
    void testIteration();
    void testCreate();
};

class TestGameInterfaceShipFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceShipMethod : public CxxTest::TestSuite {
 public:
    void testMarkUnmark();
    void testSetComment();
    void testSetFCode();
    void testSetEnemy();
    void testSetSpeed();
    void testSetName();
    void testSetMission();
    void testFixShip();
    void testRecycleShip();
    void testSetWaypoint();
    void testCargoTransfer();
    void testCargoUnload();
    void testCargoUpload();
    void testSetFleet();
};

class TestGameInterfaceShipProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCarrier();
    void testEmpty();
    void testFreighter();
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

class TestGameInterfaceTaskEditorContext : public CxxTest::TestSuite {
 public:
    void testTaskEditorPropertyNull();
    void testTaskEditorPropertyShip();
    void testTaskEditorPropertyPlanet();
    void testTaskEditorLinesProperty();
    void testInsertMovementCommand();
    void testCommandAdd();
    void testCommandAddMovement();
    void testCommandConfirmMessage();
    void testCommandInsert();
    void testCommandDelete();
    void testContext();
    void testCreate();
};

class TestGameInterfaceTorpedoContext : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testIteration();
    void testNull();
    void testCreate();
    void testSet();
};

class TestGameInterfaceTorpedoFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceUfoContext : public CxxTest::TestSuite {
 public:
    void testTypes();
    void testIteration();
    void testEmpty();
    void testCommands();
};

class TestGameInterfaceUfoFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
};

class TestGameInterfaceUfoMethod : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceUfoProperty : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
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

class TestGameInterfaceVcrContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIteration();
    void testCreate();
    void testCreateEmpty();
};

class TestGameInterfaceVcrFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceVcrProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceVcrSideContext : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
    void testCreate();
    void testCreateEmpty();
};

class TestGameInterfaceVcrSideFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameInterfaceVcrSideProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameInterfaceVmFile : public CxxTest::TestSuite {
 public:
    void testLoad();
    void testLoad2();
    void testLoadMissing();
    void testLoadBad();
    void testLoadForeign();
    void testSave();
    void testSaveEmpty();
    void testSaveNull();
};

class TestGameInterfaceWeaponProperty : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
