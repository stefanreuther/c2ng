/**
  *  \file test/server/interface/talkusertest.cpp
  *  \brief Test for server::interface::TalkUser
  */

#include "server/interface/talkuser.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkUser")
{
    class Tester : public server::interface::TalkUser {
     public:
        virtual afl::data::Value* accessNewsrc(Modification /*modif*/, Result /*res*/, afl::base::Memory<const Selection> /*selection*/, afl::base::Memory<const int32_t> /*posts*/)
            { return 0; }
        virtual void watch(afl::base::Memory<const Selection> /*selection*/)
            { }
        virtual void unwatch(afl::base::Memory<const Selection> /*selection*/)
            { }
        virtual void markSeen(afl::base::Memory<const Selection> /*selection*/)
            { }
        virtual afl::data::Value* getWatchedThreads(const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getWatchedForums(const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getPostedMessages(String_t /*user*/, const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getCrosspostToGameCandidates(const ListParameters& /*params*/)
            { return 0; }
    };
    Tester t;
}
