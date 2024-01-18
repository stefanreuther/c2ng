/**
  *  \file test/util/simplerequestdispatchertest.cpp
  *  \brief Test for util::SimpleRequestDispatcher
  */

#include "util/simplerequestdispatcher.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.SimpleRequestDispatcher", a)
{
    // Create object. Must immediately report nothing to do.
    util::SimpleRequestDispatcher testee;
    a.checkEqual("01", testee.wait(0), false);

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
    a.checkEqual("11", n, 0);
    a.checkEqual("12", testee.wait(0), true);
    a.checkEqual("13", n, 1);

    // Same thing, with parameterless wait
    testee.postNewRunnable(new Task(n));
    a.checkEqual("21", n, 1);
    testee.wait();
    a.checkEqual("22", n, 2);
}
