/**
  *  \file test/server/talk/configurationtest.cpp
  *  \brief Test for server::talk::Configuration
  */

#include "server/talk/configuration.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test.
    Just verifies that everything is initialized to sane values. */
AFL_TEST("server.talk.Configuration", a)
{
    server::talk::Configuration testee;
    a.check("01", testee.pathHost.size() > 0);
    a.check("02", testee.baseUrl.size() > 0);
    a.check("03", testee.messageIdSuffix.size() > 0);
    a.check("04", testee.messageIdSuffix.find('@') != String_t::npos);
    a.check("05", testee.rateMinimum < 0);
    a.check("06", testee.rateMaximum > testee.rateMinimum);
    a.check("07", testee.rateCooldown > 0);
    a.check("08", testee.rateInterval > 0);
    a.check("09", testee.rateCostPerMail >= 0);
    a.check("10", testee.rateCostPerReceiver >= 0);
    a.check("11", testee.rateCostPerPost >= 0);
}
