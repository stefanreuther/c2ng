/**
  *  \file u/t_game_proxy.hpp
  *  \brief Tests for game::proxy
  */
#ifndef C2NG_U_T_GAME_PROXY_HPP
#define C2NG_U_T_GAME_PROXY_HPP

#include <cxxtest/TestSuite.h>

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
};

class TestGameProxyCargoTransferSetupProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyChunnelProxy : public CxxTest::TestSuite {
 public:
    void testCandidates();
    void testGetCandidates();
    void testSetupChunnel();
    void testSetupChunnelError();
};

class TestGameProxyCommandListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCreate();
    void testNotify();
    void testFailureEmptySession();
    void testFailureUnsupported();
};

class TestGameProxyConfigurationProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyConvertSuppliesProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testSell();
    void testBuy();
};

class TestGameProxyCursorObserverProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyFriendlyCodeProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyHullSpecificationProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyKeymapProxy : public CxxTest::TestSuite {
 public:
    void testGetInfo();
    void testListener();
};

class TestGameProxyLockProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testRepeat();
    void testMarked();
    void testRange();
};

class TestGameProxyMapLocationProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testPoint();
    void testReference();
};

class TestGameProxyMutexListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameProxyObjectListener : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyObjectObserver : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameProxyPlanetInfoProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testOverride();
};

class TestGameProxyPlayerProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

class TestGameProxyProcessListProxy : public CxxTest::TestSuite {
 public:
    void testIt();
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

class TestGameProxyReverterProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
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
};

class TestGameProxyShipSpeedProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testSimple();
    void testHyper();
};

class TestGameProxySpecBrowserProxy : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFilter();
    void testSort();
};

class TestGameProxyTaskEditorProxy : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
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

#endif
