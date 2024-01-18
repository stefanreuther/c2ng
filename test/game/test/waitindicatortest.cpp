/**
  *  \file test/game/test/waitindicatortest.cpp
  *  \brief Test for game::test::WaitIndicator
  */

#include "game/test/waitindicator.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "util/requestthread.hpp"

namespace {
    struct TestObject {
        int n;
    };
}

/** Test posting request to an object. */
AFL_TEST("game.test.WaitIndicator:basics", a)
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
        Task(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual void handle(TestObject& obj)
            {
                m_assert.checkEqual("Task::handle", obj.n, 1);
                obj.n = 2;
            }
     private:
        afl::test::Assert m_assert;
    };
    Task t(a);
    testee.call(recv.getSender(), t);

    // Verify result
    a.checkEqual("result", obj.n, 2);
}

/** Test behaviour as RequestDispatcher. */
AFL_TEST("game.test.WaitIndicator:postNewRunnable", a)
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

    a.checkEqual("result", value, 44);
}
