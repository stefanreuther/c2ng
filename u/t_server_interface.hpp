/**
  *  \file u/t_server_interface.hpp
  *  \brief Tests for server::interface
  */
#ifndef C2NG_U_T_SERVER_INTERFACE_HPP
#define C2NG_U_T_SERVER_INTERFACE_HPP

#include <cxxtest/TestSuite.h>

class TestServerInterfaceBase : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceBaseClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceComposableCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileBase : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testProperty();
    void testGetFileNT();
};

class TestServerInterfaceFileBaseClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileBaseServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceFileGame : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceFileGameClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFileGameServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceFormat : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceFormatClient : public CxxTest::TestSuite {
 public:
    void testChain();
};

class TestServerInterfaceFormatServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
};

class TestServerInterfaceGameAccess : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostCron : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostCronClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceHostCronServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostFile : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testFormat();
    void testMergeInfo();
};

class TestServerInterfaceHostFileClient : public CxxTest::TestSuite {
 public:
    void testIt();
    void testUnpack();
};

class TestServerInterfaceHostFileServer : public CxxTest::TestSuite {
 public:
    void testServer();
    void testError();
    void testRoundtrip();
    void testInteroperability();
};

class TestServerInterfaceHostGame : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testFormat();
    void testParse();
    void testInit();
};

class TestServerInterfaceHostGameClient : public CxxTest::TestSuite {
 public:
    void testIt();
    void testStat();
    void testTools();
    void testVC();
    void testErrors();
};

class TestServerInterfaceHostGameServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostHistory : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostHistoryClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceHostHistoryServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostPlayer : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testFormat();
    void testParse();
    void testInit();
};

class TestServerInterfaceHostPlayerClient : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFail();
};

class TestServerInterfaceHostPlayerServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostRanking : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostRankingClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceHostRankingServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostSchedule : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testFormat();
    void testParse();
};

class TestServerInterfaceHostScheduleClient : public CxxTest::TestSuite {
 public:
    void testModify();
    void testOther();
    void testErrors();
};

class TestServerInterfaceHostScheduleServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceHostSlot : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostSlotClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceHostSlotServer : public CxxTest::TestSuite {
 public:
    void testServer();
    void testError();
    void testRoundtrip();
};

class TestServerInterfaceHostTool : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceHostToolClient : public CxxTest::TestSuite {
 public:
    void testIt();
    void testModes();
};

class TestServerInterfaceHostToolServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
    void testArea();
    void testAreaMismatch();
};

class TestServerInterfaceHostTurn : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testInit();
};

class TestServerInterfaceHostTurnClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceHostTurnServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceMailQueue : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testAddressStatus();
};

class TestServerInterfaceMailQueueClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceMailQueueServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRoundtrip();
};

class TestServerInterfaceSessionRouter : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testParse();
    void testFormat();
};

class TestServerInterfaceTalkAddress : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkAddressClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkAddressServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkFolder : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkFolderClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkFolderServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkForum : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGetValue();
};

class TestServerInterfaceTalkForumClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkForumServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkGroupClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkGroupServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkNNTP : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkNNTPClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkNNTPServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkPM : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkPMClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkPMServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkPost : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkPostClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkPostServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkRender : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkRenderClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkRenderServer : public CxxTest::TestSuite {
 public:
    void testServer();
    void testRoundtrip();
};

class TestServerInterfaceTalkSyntax : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkSyntaxClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkSyntaxServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRoundtrip();
};

class TestServerInterfaceTalkThread : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkThreadClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkThreadServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceTalkUser : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceTalkUserClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceTalkUserServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceUserData : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceUserDataClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceUserDataServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceUserManagement : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceUserManagementClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceUserManagementServer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErrors();
    void testRoundtrip();
};

class TestServerInterfaceUserToken : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerInterfaceUserTokenClient : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerInterfaceUserTokenServer : public CxxTest::TestSuite {
 public:
    void testServer();
    void testErrors();
    void testRoundtrip();
};

#endif
