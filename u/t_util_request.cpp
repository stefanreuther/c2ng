/**
  *  \file u/t_util_request.cpp
  *  \brief Test for util::Request
  */

#include "util/request.hpp"

#include "t_util.hpp"

/** Simple test. This is an interface; just test that we can create it as expect it. */
void
TestUtilRequest::testIt()
{
    class Tester : public util::Request<int> {
     public:
       virtual void handle(int&)
            { }
    };
    Tester t;
}

