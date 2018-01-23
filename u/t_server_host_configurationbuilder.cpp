/**
  *  \file u/t_server_host_configurationbuilder.cpp
  *  \brief Test for server::host::ConfigurationBuilder
  */

#include "server/host/configurationbuilder.hpp"

#include "t_server_host.hpp"

void
TestServerHostConfigurationBuilder::testIt()
{
    // Base case
    {
        server::host::ConfigurationBuilder testee;
        testee.addValue("a", "b");
        TS_ASSERT(testee.getContent().equalContent(afl::string::toBytes("a=b\n")));
    }
    // Refused keys
    {
        server::host::ConfigurationBuilder testee;
        testee.addValue("0a", "b");
        testee.addValue("", "b");
        testee.addValue("a*b", "b");
        testee.addValue("+", "b");
        TS_ASSERT(testee.getContent().empty());
    }
    // Escaped value
    {
        server::host::ConfigurationBuilder testee;
        testee.addValue("a", "b, c, d");
        TS_ASSERT(testee.getContent().equalContent(afl::string::toBytes("a=b,\\ c,\\ d\n")));
    }
}

