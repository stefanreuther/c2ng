/**
  *  \file u/t_game_test_waitindicator.cpp
  *  \brief Test for game::test::WaitIndicator
  */

#include "game/test/waitindicator.hpp"

#include "t_game_test.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "util/requestthread.hpp"

namespace {
    struct TestObject {
        int n;
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
    afl::string::NullTranslator tx;
    util::RequestThread thread("TestGameTestWaitIndicator::testIt", log, tx, 0);
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

