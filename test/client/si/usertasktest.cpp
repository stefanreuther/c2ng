/**
  *  \file test/client/si/usertasktest.cpp
  *  \brief Test for client::si::UserTask
  */

#include "client/si/usertask.hpp"

#include "afl/test/testrunner.hpp"
#include "client/si/requestlink2.hpp"

/** Interface test. */
AFL_TEST_NOARG("client.si.UserTask")
{
    class Tester : public client::si::UserTask {
     public:
        virtual void handle(client::si::Control& /*ctl*/, client::si::RequestLink2 /*link*/)
            { }
    };
    Tester t;
}
