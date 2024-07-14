/**
  *  \file test/server/talk/notifiertest.cpp
  *  \brief Test for server::talk::Notifier
  */

#include "server/talk/notifier.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.talk.Notifier")
{
    class Tester : public server::talk::Notifier {
     public:
        virtual void notifyMessage(server::talk::Message& /*msg*/)
            { }
        virtual void notifyPM(server::talk::UserPM& /*msg*/, const afl::data::StringList_t& /*notifyIndividual*/, const afl::data::StringList_t& /*notifyGroup*/)
            { }
    };
    Tester t;
}

