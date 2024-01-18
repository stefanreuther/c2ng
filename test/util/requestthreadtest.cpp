/**
  *  \file test/util/requestthreadtest.cpp
  *  \brief Test for util::RequestThread
  */

#include "util/requestthread.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/sys/semaphore.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("util.RequestThread", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    util::RequestThread testee(a.getLocation(), log, tx);

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
