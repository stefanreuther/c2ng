/**
  *  \file u/t_server_host_configurationbuilder.cpp
  *  \brief Test for server::host::ConfigurationBuilder
  */

#include "server/host/configurationbuilder.hpp"

#include "t_server_host.hpp"

/** Test ConfigurationBuilder.
    A: perform various addValue() calls with valid and invalid parameters
    E: correct result produced */
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
    // Bad value
    {
        server::host::ConfigurationBuilder testee;
        testee.addValue("a", "x\ny");
        TS_ASSERT(testee.getContent().equalContent(afl::string::toBytes("a=x\n")));
    }
    // Unicode value
    {
        server::host::ConfigurationBuilder testee;
        testee.addValue("qq", "x\xc3\xb6y");
        TS_ASSERT(testee.getContent().equalContent(afl::string::toBytes("qq=x\xc3\xb6y\n")));
    }
}

