/**
  *  \file u/t_util_slaverequest.cpp
  *  \brief Test for util::SlaveRequest
  */

#include "util/slaverequest.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilSlaveRequest::testIt()
{
    class Tester : public util::SlaveRequest<int,float> {
     public:
        virtual void handle(int& /*master*/, float& /*slave*/)
            { }
    };
    Tester t;
}

