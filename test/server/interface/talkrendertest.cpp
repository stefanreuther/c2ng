/**
  *  \file test/server/interface/talkrendertest.cpp
  *  \brief Test for server::interface::TalkRender
  */

#include "server/interface/talkrender.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkRender")
{
    class Tester : public server::interface::TalkRender {
     public:
        virtual void setOptions(const Options& /*opts*/)
            { }
        virtual String_t render(const String_t& /*text*/, const Options& /*opts*/)
            { return String_t(); }
    };
    Tester t;
}
