/**
  *  \file u/t_server_talk.hpp
  *  \brief Tests for server::talk
  */
#ifndef C2NG_U_T_SERVER_TALK_HPP
#define C2NG_U_T_SERVER_TALK_HPP

#include <cxxtest/TestSuite.h>

class TestServerTalkAccessChecker : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkConfiguration : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkForum : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNewsgroup();
    void testSort();
};

class TestServerTalkGroup : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSetParent();
};

class TestServerTalkInlineRecognizer : public CxxTest::TestSuite {
 public:
    void testUrl();
    void testSmiley();
    void testGeneral();
    void testGetSmiley();
};

class TestServerTalkLinkFormatter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkMessage : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMessageIds();
    void testEmail();
    void testParent();
    void testSort();
};

class TestServerTalkNewsrc : public CxxTest::TestSuite {
 public:
    void testIt();
    void testBackward();
};

class TestServerTalkRoot : public CxxTest::TestSuite {
 public:
    void testCheckUserPermission();
    void testGetUserIdFromLogin();
};

class TestServerTalkSession : public CxxTest::TestSuite {
 public:
    void testPermission();
    void testRenderOptions();
};

class TestServerTalkSorter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerTalkSpam : public CxxTest::TestSuite {
 public:
    static const char SPAM_MESSAGE[];
    void testIt();
};

class TestServerTalkTalkAddress : public CxxTest::TestSuite {
 public:
    void testParse();
    void testRenderRaw();
    void testRenderHTML();
    void testRenderOther();
    void testCompat();
};

class TestServerTalkTalkFolder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRoot();
    void testMessageFlags();
};

class TestServerTalkTalkForum : public CxxTest::TestSuite {
 public:
    void testListOperation();
    void testIt();
    void testFindForum();
};

class TestServerTalkTalkGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkTalkNNTP : public CxxTest::TestSuite {
 public:
    void testGroups();
    void testFindMessage();
    void testListMessages();
    void testMessageHeader();
};

class TestServerTalkTalkPM : public CxxTest::TestSuite {
 public:
    void testRender();
    void testIt();
    void testRoot();
    void testReceivers();
    void testReceiverErrors();
    void testSuggestedFolder();
};

class TestServerTalkTalkPost : public CxxTest::TestSuite {
 public:
    void testCreate();
    void testCreateErrors();
    void testCreateSpam();
    void testPermissions();
    void testRender();
    void testGetInfo();
    void testGetNewest();
    void testGetNewest2();
    void testGetHeader();
    void testRemove();
};

class TestServerTalkTalkRender : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkTalkSyntax : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkTalkThread : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkTalkUser : public CxxTest::TestSuite {
 public:
    void testNewsrc();
    void testNewsrcErrors();
    void testNewsrcSingle();
    void testNewsrcSet();
    void testRoot();
    void testWatch();
    void testPostedMessages();
};

class TestServerTalkTextNode : public CxxTest::TestSuite {
 public:
    void testQuote();
    void testBasic();
    void testSimpleList();
    void testSimpleList2();
    void testTextContent();
};

class TestServerTalkTopic : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testRemove();
    void testSort();
};

class TestServerTalkUser : public CxxTest::TestSuite {
 public:
    void testBasicProperties();
    void testMailPMType();
    void testAutowatch();
    void testWatchIndividual();
};

class TestServerTalkUserFolder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testAllocate();
    void testMixedProperties();
    void testFindFolder();
    void testFindSuggestedFolder();
};

class TestServerTalkUserPM : public CxxTest::TestSuite {
 public:
    void testIt();
    void testAllocate();
    void testSort();
};

#endif
