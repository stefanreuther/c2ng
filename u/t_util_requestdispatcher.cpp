/**
  *  \file u/t_util_requestdispatcher.cpp
  *  \brief Test for util::RequestDispatcher
  */

#include "util/requestdispatcher.hpp"

#include "t_util.hpp"

/** Simple test. This is an interface; just test that we can create it as expect it. */
void
TestUtilRequestDispatcher::testIt()
{
    class Tester : public util::RequestDispatcher {
     public:
        virtual void postNewRunnable(afl::base::Runnable* /*op*/)
            { }
    };
    Tester t;
}
