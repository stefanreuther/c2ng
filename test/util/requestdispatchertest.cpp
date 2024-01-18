/**
  *  \file test/util/requestdispatchertest.cpp
  *  \brief Test for util::RequestDispatcher
  */

#include "util/requestdispatcher.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. This is an interface; just test that we can create it as expect it. */
AFL_TEST_NOARG("util.RequestDispatcher")
{
    class Tester : public util::RequestDispatcher {
     public:
        virtual void postNewRunnable(afl::base::Runnable* /*op*/)
            { }
    };
    Tester t;
}
