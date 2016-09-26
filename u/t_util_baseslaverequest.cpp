/**
  *  \file u/t_util_baseslaverequest.cpp
  *  \brief Test for util::BaseSlaveRequest
  */

#include "util/baseslaverequest.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilBaseSlaveRequest::testIt()
{
    class Tester : public util::BaseSlaveRequest<int> {
     public:
        virtual void handle(int& /*master*/, util::SlaveObject<int>& /*slave*/)
            { }
    };
    Tester t;
}

