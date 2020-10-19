/**
  *  \file u/t_util_simplerequestdispatcher.cpp
  *  \brief Test for util::SimpleRequestDispatcher
  */

#include "util/simplerequestdispatcher.hpp"

#include "t_util.hpp"

/** Simple test. */
void
TestUtilSimpleRequestDispatcher::testIt()
{
    // Create object. Must immediately report nothing to do.
    util::SimpleRequestDispatcher testee;
    TS_ASSERT_EQUALS(testee.wait(0), false);

    // Post a task. Must be executed when it is time.
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
    int n = 0;
    testee.postNewRunnable(new Task(n));
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(testee.wait(0), true);
    TS_ASSERT_EQUALS(n, 1);

    // Same thing, with parameterless wait
    testee.postNewRunnable(new Task(n));
    TS_ASSERT_EQUALS(n, 1);
    testee.wait();
    TS_ASSERT_EQUALS(n, 2);
}

