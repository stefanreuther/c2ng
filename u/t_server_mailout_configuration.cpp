/**
  *  \file u/t_server_mailout_configuration.cpp
  *  \brief Test for server::mailout::Configuration
  */

#include "server/mailout/configuration.hpp"

#include "t_server_mailout.hpp"

/** Test defaults. */
void
TestServerMailoutConfiguration::testDefault()
{
    server::mailout::Configuration testee;
    TS_ASSERT_EQUALS(testee.baseUrl, "unconfigured");
    TS_ASSERT_EQUALS(testee.confirmationKey, "");
    TS_ASSERT_DIFFERS(testee.maximumAge, 0);
    TS_ASSERT_EQUALS(testee.useTransmitter, true);

    server::mailout::Configuration copy(testee);
    TS_ASSERT_EQUALS(copy.baseUrl,         testee.baseUrl);
    TS_ASSERT_EQUALS(copy.confirmationKey, testee.confirmationKey);
    TS_ASSERT_EQUALS(copy.maximumAge,      testee.maximumAge);
    TS_ASSERT_EQUALS(copy.useTransmitter,  testee.useTransmitter);
}

