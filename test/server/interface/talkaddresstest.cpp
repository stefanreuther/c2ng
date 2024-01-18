/**
  *  \file test/server/interface/talkaddresstest.cpp
  *  \brief Test for server::interface::TalkAddress
  */

#include "server/interface/talkaddress.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkAddress")
{
    class Tester : public server::interface::TalkAddress {
     public:
        virtual void parse(afl::base::Memory<const String_t> /*in*/, afl::data::StringList_t& /*out*/)
            { }
        virtual void render(afl::base::Memory<const String_t> /*in*/, afl::data::StringList_t& /*out*/)
            { }
    };
    Tester t;
}
