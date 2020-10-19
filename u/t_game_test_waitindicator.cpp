/**
  *  \file u/t_game_test_waitindicator.cpp
  *  \brief Test for game::test::WaitIndicator
  */

#include "game/test/waitindicator.hpp"

#include "t_game_test.hpp"
#include "afl/sys/log.hpp"
#include "util/requestthread.hpp"

namespace {
    struct TestObject {
        int n;
    };

    struct TestSlave : public util::SlaveObject<TestObject> {
        int m;

        void init(TestObject& obj)
            { m = obj.n; }
        void done(TestObject&)
            { }
    };
}

/** Test posting request to an object. */
void
TestGameTestWaitIndicator::testIt()
{
    // Create test object and thread to work on it
    TestObject obj;
    obj.n = 1;

    afl::sys::Log log;
    util::RequestThread thread("TestGameTestWaitIndicator::testIt", log, 0);
    util::RequestReceiver<TestObject> recv(thread, obj);

    // Call into that thread
    game::test::WaitIndicator testee;
    class Task : public util::Request<TestObject> {
     public:
        virtual void handle(TestObject& obj)
            {
                TS_ASSERT_EQUALS(obj.n, 1);
                obj.n = 2;
            }
    };
    Task t;
    testee.call(recv.getSender(), t);

    // Verify result
    TS_ASSERT_EQUALS(obj.n, 2);
}

/** Test posting request to a slave object. */
void
TestGameTestWaitIndicator::testSlave()
{
    // Create test object and thread to work on it
    TestObject obj;
    obj.n = 1;

    afl::sys::Log log;
    util::RequestThread thread("TestGameTestWaitIndicator::testSlave", log, 0);
    util::RequestReceiver<TestObject> recv(thread, obj);
    util::SlaveRequestSender<TestObject,TestSlave> slave(recv.getSender(), new TestSlave());

    // Call into that thread
    game::test::WaitIndicator testee;
    class Task : public util::SlaveRequest<TestObject,TestSlave> {
     public:
        virtual void handle(TestObject& obj, TestSlave& slave)
            {
                TS_ASSERT_EQUALS(obj.n, 1);
                TS_ASSERT_EQUALS(slave.m, 1);
                obj.n = 2;
            }
    };
    Task t;
    testee.call(slave, t);

    // Verify result
    TS_ASSERT_EQUALS(obj.n, 2);
}

/** Test behaviour as RequestDispatcher. */
void
TestGameTestWaitIndicator::testRequestDispatcher()
{
    class Task : public afl::base::Runnable {
     public:
        Task(int& n)
            : m_n(n)
            { }
        virtual void run()
            { ++m_n; }
     private:
        int& m_n;
    };

    
    int value = 42;
    game::test::WaitIndicator ind;
    ind.postNewRunnable(new Task(value));
    ind.postNewRunnable(new Task(value));
    ind.processQueue();

    TS_ASSERT_EQUALS(value, 44);
}

