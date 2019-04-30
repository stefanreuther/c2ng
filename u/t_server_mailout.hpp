/**
  *  \file u/t_server_mailout.hpp
  *  \brief Tests for server::mailout
  */
#ifndef C2NG_U_T_SERVER_MAILOUT_HPP
#define C2NG_U_T_SERVER_MAILOUT_HPP

#include <cxxtest/TestSuite.h>

class TestServerMailoutCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerMailoutConfiguration : public CxxTest::TestSuite {
 public:
    void testDefault();
};

class TestServerMailoutMailQueue : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSequenceError();
    void testSequenceError2();
    void testRequest();
    void testConfirmSuccess();
    void testConfirmFailure();
    void testConfirmSuccessTransmit();
    void testRunQueue();
    void testRunQueueTransmitter();
};

class TestServerMailoutMessage : public CxxTest::TestSuite {
 public:
    void testDatabase();
    void testRemove();
    void testSend();
};

class TestServerMailoutRoot : public CxxTest::TestSuite {
 public:
    void testAllocateMessage();
    void testResolveMail();
    void testResolveMailBlocked();
    void testResolveUserNoMail();
    void testResolveUserUnconfirmed();
    void testResolveUserRequested();
    void testResolveUserExpired();
    void testResolveUserConfirmed();
    void testResolveUserBlocked();
    void testConfirmMail();
    void testConfirmMailFail();
    void testPrepareQueue();
    void testGetUserStatus();
    void testGetUserStatusEmpty();
    void testGetUserStatusUnconfirmed();
};

class TestServerMailoutSession : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerMailoutTemplate : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testHeaderOverride();
    void testVariable();
    void testConditional();
    void testAttachment();
};

class TestServerMailoutTransmitter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerMailoutTransmitterImpl : public CxxTest::TestSuite {
 public:
    void testStartup();
};

#endif
