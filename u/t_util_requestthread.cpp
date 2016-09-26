/**
  *  \file u/t_util_requestthread.cpp
  *  \brief Test for util::RequestThread
  */

#include "util/requestthread.hpp"

#include "t_util.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/sys/log.hpp"

/** Simple test. */
void
TestUtilRequestThread::testIt()
{
    afl::sys::Log log;
    util::RequestThread testee("TestUtilRequestThread", log);

    // Test load
    afl::sys::Semaphore sem(0);
    class Tester : public afl::base::Runnable {
     public:
        Tester(afl::sys::Semaphore& sem)
            : m_sem(sem)
            { }
        virtual void run()
            { m_sem.post(); }
     private:
        afl::sys::Semaphore& m_sem;
    };

    // Test that the test load is actually executed.
    // Do so multiple times in different sequences: post once/wait once; post twice/wait twice; etc.
    for (int i = 1; i <= 10; ++i) {
        for (int j = 0; j < i; ++j) {
            testee.postNewRunnable(new Tester(sem));
        }
        for (int j = 0; j < i; ++j) {
            sem.wait();
        }
    }
}

