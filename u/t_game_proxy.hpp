/**
  *  \file u/t_game_proxy.hpp
  *  \brief Tests for game::proxy
  */
#ifndef C2NG_U_T_GAME_PROXY_HPP
#define C2NG_U_T_GAME_PROXY_HPP

#include <cxxtest/TestSuite.h>

class TestGameProxyBaseStorageProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testGetParts();
    void testGetAllHulls();
    void testUpdate();
    void testCustom();
};

class TestGameProxyBuildAmmoProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testPlanet();
    void testShip();
    void testFarShip();
};

class TestGameProxyBuildPartsProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testSignal();
    void testError();
    void testErrorResources();
};

class TestGameProxyBuildQueueProxy : public CxxTest::TestSuite {
 public:
    void testInit();
    void testIncrease();
    void testDecrease();
    void testSet();
    void testSignal();
    void testCommit();
    void testEmpty();
};

class TestGameProxyBuildShipProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testSetBuildOrder();
    void testPreexisting();
    void testClone();
    void testCustom();
    void testDisabledTech();
};

class TestGameProxyBuildStarbaseProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testCancel();
    void testMissing();
};

class TestGameProxyBuildStructuresProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testBuild();
    void testAutoBuild();
};

class TestGameProxyCargoTransferProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testOverload();
    void testMulti();
    void testMultiMoveExt();
    void testMultiMoveAll();
    void testDistribute();
    void testAddHoldSpace();
};

class TestGameProxyCargoTransferSetupProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testConflict();
};

class TestGameProxyChunnelProxy : public CxxTest::TestSuite {
 public:
    void testCandidates();
    void testGetCandidates();
    void testSetupChunnel();
    void testSetupChunnelError();
};

class TestGameProxyClassicVcrPlayerProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrorBadAlgo();
    void testErrorBadContent();
    void testErrorBadIndex();
    void testErrorBadCapabilities();
};

class TestGameProxyCloneShipProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyCommandListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCreate();
    void testNotify();
    void testFailureEmptySession();
    void testFailureUnsupported();
};

class TestGameProxyConfigurationEditorAdaptor : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyConfigurationEditorProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testIntegration();
    void testObserverIntegration();
};

class TestGameProxyConfigurationObserverProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyConfigurationProxy : public CxxTest::TestSuite {
 public:
    void testNumberFormatter();
    void testIntAccess();
    void testStringAccess();
    void testMarkerAccess();
};

class TestGameProxyConvertSuppliesProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testSell();
    void testBuy();
};

class TestGameProxyCostSummaryAdaptor : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testMake();
    void testIntegration();
};

class TestGameProxyCurrentStarbaseAdaptor : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testFindShipCloningHere();
};

class TestGameProxyCursorObserverProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyDrawingProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testCreateMarker();
    void testCreateLines();
    void testCreateRectangle();
    void testCreateCircle();
    void testFindNearest();
    void testErase();
    void testColorAdjacent();
    void testTagAdjacent();
    void testEraseAdjacent();
    void testParallel();
    void testSelectMarker();
    void testSetTagName();
    void testPackTagList();
    void testPackTagListEmpty();
    void testCreateCannedMarker();
    void testQueueing();
};

class TestGameProxyExportProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSetCharsetIndex();
    void testSetFormat();
    void testAdd();
    void testSwap();
    void testRemove();
    void testClear();
    void testSetFieldName();
    void testSetFieldWidth();
    void testChangeFieldWidth();
    void testToggleFieldAlignment();
    void testExportFile();
    void testExportFileNullFS();
    void testExportFileNullContext();
    void testLoad();
    void testLoadError();
    void testSave();
    void testSaveError();
};

class TestGameProxyExpressionListProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyFictiveStarbaseAdaptor : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testZero();
    void testMixed();
    void testExtra();
    void testFactory();
};

class TestGameProxyFlakVcrPlayerProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyFleetCostProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
};

class TestGameProxyFleetProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoShip();
    void testNormal();
    void testDeleteMid();
    void testDeleteEnd();
    void testDeleteAll();
};

class TestGameProxyFriendlyCodeProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyGlobalActionProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyHistoryShipListProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyHistoryShipProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoShip();
    void testNormal();
    void testExtBrowse();
};

class TestGameProxyHullSpecificationProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSetQuery();
};

class TestGameProxyImperialStatsProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNonempty();
    void testOptions();
    void testContentOptions();
};

class TestGameProxyInboxAdaptor : public CxxTest::TestSuite {
 public:
    void testInboxAdaptor();
    void testPlanet();
    void testShip();
    void testIndex();
    void testFilter();
};

class TestGameProxyIonStormProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoStorms();
    void testNormal();
    void testUninit();
    void testBrowse();
    void testObjectListener();
};

class TestGameProxyKeymapProxy : public CxxTest::TestSuite {
 public:
    void testGetInfo();
    void testListener();
};

class TestGameProxyLabelProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyLockProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testEmptyName();
    void testNormal();
    void testNormalName();
    void testRepeat();
    void testRepeatName();
    void testMarked();
    void testRange();
    void testRangeName();
    void testSetOrigin();
    void testSetDrawingTagFilter();
    void testNoDrawings();
};

class TestGameProxyMailboxAdaptor : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyMailboxProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSummary();
    void testToggleFiltered();
    void testAction();
    void testSearch();
    void testWrite();
    void testWriteMulti();
    void testWriteError();
    void testQuote();
    void testQuote2();
    void testData();
};

class TestGameProxyMapConfigurationProxy : public CxxTest::TestSuite {
 public:
    void testMapConfig();
    void testRenderOptions();
    void testMarkerConfig();
};

class TestGameProxyMapLocationProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testPoint();
    void testReference();
    void testBrowse();
    void testConfigChange();
    void testGetOtherPosition();
};

class TestGameProxyMapRendererProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testTagFilter();
    void testToggleOptions();
    void testSetConfiguration();
    void testModifyPreferences();
};

class TestGameProxyMinefieldProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoMine();
    void testNormal();
    void testPlanetOwn();
    void testPlanetSeen();
    void testPlanetOther();
    void testPassageRate();
    void testSweepInfo();
    void testObjectListener();
    void testBrowse();
    void testBrowseUnmarked();
    void testErase();
};

class TestGameProxyMutexListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyObjectListExportAdaptor : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNull();
    void testConfigError();
};

class TestGameProxyObjectListener : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyObjectObserver : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyOutboxProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testAdaptor();
    void testMailboxProxy();
    void testFileEmpty();
    void testFileNormal();
    void testFileError();
    void testLoadMessage();
    void testLoadMessageFail();
    void testLoadMessageRoot();
};

class TestGameProxyPlanetInfoProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testOverride();
};

class TestGameProxyPlanetPredictorProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testUpdate();
    void testEffectors();
};

class TestGameProxyPlayerProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyPluginManagerProxy : public CxxTest::TestSuite {
 public:
    void testRequest();
    void testDebounce();
    void testInstall();
    void testInstallNoDir();
    void testInstallNoFile();
    void testInstallBadFile();
    void testInstallConflict();
    void testInstallConflict2();
    void testInstallOneAlternative();
    void testInstallMultipleAlternatives();
    void testInstallMissingFile();
    void testInstallBadSequence();
    void testRemove();
    void testRemoveDepend();
    void testRemoveMissing();
    void testUninstallBadName();
};

class TestGameProxyPredictedStarbaseAdaptor : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormalTrue();
    void testNormalFalse();
    void testNoTask();
    void testExtra();
    void testFactory();
};

class TestGameProxyProcessListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testResumeConfirmed();
};

class TestGameProxyReferenceListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testConfigSelection();
};

class TestGameProxyReferenceObserverProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyReferenceProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyReverterProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyScoreProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testChart();
    void testTable();
    void testTurns();
    void testOverview();
};

class TestGameProxyScriptEditorProxy : public CxxTest::TestSuite {
 public:
    void testBuildCompletion();
    void testBuildCompletionForObject();
    void testBuildPropertyList();
};

class TestGameProxySearchProxy : public CxxTest::TestSuite {
 public:
    void testSuccess();
    void testFailCompile();
    void testFailSuspend();
    void testFailEndString();
    void testFailEndOther();
    void testFailTerminate();
    void testFailException();
    void testSave();
};

class TestGameProxySelectionProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testInit();
    void testSignalExternal();
    void testSignalInternal();
    void testClearLayer();
    void testClearAllLayers();
    void testInvertLayer();
    void testInvertAllLayers();
    void testExecute();
    void testExecuteFail();
    void testMarkList();
    void testMarkRange();
    void testMarkRangeWrap();
};

class TestGameProxyShipSpeedProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testSimple();
    void testHyper();
};

class TestGameProxySimulationRunProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testSeries();
    void testInfinite();
    void testNoFight();
    void testClassResultBattleAdaptor();
    void testUnitResultBattleAdaptor();
};

class TestGameProxySimulationSetupProxy : public CxxTest::TestSuite {
 public:
    void testUninit();
    void testEmpty();
    void testAddShip();
    void testAddPlanet();
    void testSwapShips();
    void testRemoveObject();
    void testClear();
    void testGetObject();
    void testIsDuplicateId();
    void testGetNumBaseTorpedoes();
    void testSetFlags();
    void testToggleDisabled();
    void testToggleCloak();
    void testToggleRandomFriendlyCode();
    void testSetAbilities();
    void testSetSequentialFriendlyCode();
    void testSetId();
    void testSetName();
    void testSetFriendlyCode();
    void testSetDamage();
    void testSetShield();
    void testSetOwner();
    void testSetExperienceLevel();
    void testSetFlakRatingOverride();
    void testSetFlakCompensationOverride();
    void testSetCrew();
    void testSetHullType();
    void testSetHullTypeAfterAdd();
    void testSetHullTypeAfterAddCloak();
    void testSetHullTypeAfterAddDamage();
    void testSetHullTypeAfterAddSelfAggression();
    void testSetMass();
    void testSetBeams();
    void testSetTorpedoes();
    void testSetFighters();
    void testSetEngineType();
    void testSetAggressiveness();
    void testSetAggressivenessInteraction();
    void testSetInterceptId();
    void testSetDefense();
    void testSetPopulation();
    void testSetBaseDefense();
    void testSetBaseBeamTech();
    void testSetBaseTorpedoTech();
    void testSetNumBaseFighters();
    void testSetNumBaseTorpedoes();
    void testGetAbilityChoices();
    void testGetAbilityChoicesPlanet();
    void testGetFriendlyCodeChoices();
    void testGetOwnerChoices();
    void testGetExperienceLevelChoices();
    void testGetHullTypeChoices();
    void testGetPrimaryChoices();
    void testGetSecondaryChoices();
    void testGetEngineTypeChoices();
    void testGetAggressivenessChoices();
    void testGetBaseBeamLevelChoices();
    void testGetBaseTorpedoLevelChoices();
    void testGetPlanetNameChoices();
    void testGetPopulationChoices();
    void testGetIdRange();
    void testGetDamageRange();
    void testGetShieldRange();
    void testGetCrewRange();
    void testGetInterceptIdRange();
    void testGetBaseDefenseRange();
    void testGetNumBaseFightersRange();
    void testSetSlot();
    void testConfig();
    void testSort();
    void testSortByBattleOrder();
    void testCopy();
    void testLoad();
    void testLoadFail();
};

class TestGameProxySimulationTransferProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testShip();
    void testPlanet();
    void testList();
    void testInteraction();
};

class TestGameProxySpecBrowserProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFilter();
    void testSort();
    void testSetPageId();
};

class TestGameProxyStarbaseAdaptor : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyTaskEditorProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testShipStatus();
    void testMessage();
    void testBase();
    void testEdit();
};

class TestGameProxyTaxationProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testChangeRevenue();
    void testModifyRevert();
    void testSafeTax();
    void testSetNumBuildings();
    void testSignal();
};

class TestGameProxyTeamProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyTechUpgradeProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testSignal();
    void testUpgrade();
    void testReserve();
};

class TestGameProxyUfoProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNoUfos();
    void testNormal();
    void testUnknown();
    void testBrowse();
    void testObjectListener();
    void testToggle();
    void testBrowseOtherEnd();
};

class TestGameProxyVcrDatabaseAdaptor : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyVcrDatabaseProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGetTeamSettings();
    void testGetTeamSettings2();
    void testGetPlayerNames();
};

class TestGameProxyVcrOverviewProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testBuildDiagram();
    void testBuildScores();
};

class TestGameProxyVisibilityRangeProxy : public CxxTest::TestSuite {
 public:
    void testSequence();
    void testEmpty();
};

#endif
