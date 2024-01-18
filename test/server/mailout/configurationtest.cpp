/**
  *  \file test/server/mailout/configurationtest.cpp
  *  \brief Test for server::mailout::Configuration
  */

#include "server/mailout/configuration.hpp"
#include "afl/test/testrunner.hpp"

/** Test defaults. */
AFL_TEST("server.mailout.Configuration:default", a)
{
    server::mailout::Configuration testee;
    a.checkEqual    ("01. baseUrl",         testee.baseUrl, "unconfigured");
    a.checkEqual    ("02. confirmationKey", testee.confirmationKey, "");
    a.checkDifferent("03. maximumAge",      testee.maximumAge, 0);
    a.checkEqual    ("04. useTransmitter",  testee.useTransmitter, true);

    server::mailout::Configuration copy(testee);
    a.checkEqual("11. baseUrl",         copy.baseUrl,         testee.baseUrl);
    a.checkEqual("12. confirmationKey", copy.confirmationKey, testee.confirmationKey);
    a.checkEqual("13. maximumAge",      copy.maximumAge,      testee.maximumAge);
    a.checkEqual("14. useTransmitter",  copy.useTransmitter,  testee.useTransmitter);
}
