/**
  *  \file test/util/requesttest.cpp
  *  \brief Test for util::Request
  */

#include "util/request.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. This is an interface; just test that we can create it as expect it. */
AFL_TEST_NOARG("util.Request")
{
    class Tester : public util::Request<int> {
     public:
       virtual void handle(int&)
            { }
    };
    Tester t;
}
