/**
  *  \file test/server/talk/linkparsertest.cpp
  *  \brief Test for server::talk::LinkParser
  */

#include "server/talk/linkparser.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.talk.LinkParser")
{
    class Tester : public server::talk::LinkParser {
     public:
        virtual afl::base::Optional<Result_t> parseGameLink(String_t /*text*/) const
            { return afl::base::Optional<Result_t>(); }
        virtual afl::base::Optional<Result_t> parseForumLink(String_t /*text*/) const
            { return afl::base::Optional<Result_t>(); }
        virtual afl::base::Optional<Result_t> parseTopicLink(String_t /*text*/) const
            { return afl::base::Optional<Result_t>(); }
        virtual afl::base::Optional<Result_t> parseMessageLink(String_t /*text*/) const
            { return afl::base::Optional<Result_t>(); }
        virtual afl::base::Optional<String_t> parseUserLink(String_t /*text*/) const
            { return afl::base::Optional<String_t>(); }
    };
    Tester t;
}
