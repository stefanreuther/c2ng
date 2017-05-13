/**
  *  \file u/t_server_talk_configuration.cpp
  *  \brief Test for server::talk::Configuration
  */

#include "server/talk/configuration.hpp"

#include "t_server_talk.hpp"

/** Simple test.
    Just verifies that everything is initialized to sane values. */
void
TestServerTalkConfiguration::testIt()
{
    server::talk::Configuration testee;
    TS_ASSERT(testee.pathHost.size() > 0);
    TS_ASSERT(testee.userKey.size() > 0);
    TS_ASSERT(testee.baseUrl.size() > 0);
    TS_ASSERT(testee.messageIdSuffix.size() > 0);
    TS_ASSERT(testee.messageIdSuffix.find('@') != String_t::npos);
}
