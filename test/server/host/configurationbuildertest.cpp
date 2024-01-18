/**
  *  \file test/server/host/configurationbuildertest.cpp
  *  \brief Test for server::host::ConfigurationBuilder
  */

#include "server/host/configurationbuilder.hpp"
#include "afl/test/testrunner.hpp"

/** Test ConfigurationBuilder.
    A: perform various addValue() calls with valid and invalid parameters
    E: correct result produced */

// Base case
AFL_TEST("server.host.ConfigurationBuilder:normal", a)
{
    server::host::ConfigurationBuilder testee;
    testee.addValue("a", "b");
    a.check("", testee.getContent().equalContent(afl::string::toBytes("a=b\n")));
}

// Refused keys
AFL_TEST("server.host.ConfigurationBuilder:invalid-key", a)
{
    server::host::ConfigurationBuilder testee;
    testee.addValue("0a", "b");
    testee.addValue("", "b");
    testee.addValue("a*b", "b");
    testee.addValue("+", "b");
    a.check("", testee.getContent().empty());
}

// Escaped value
AFL_TEST("server.host.ConfigurationBuilder:escaped-value", a)
{
    server::host::ConfigurationBuilder testee;
    testee.addValue("a", "b, c, d");
    a.check("", testee.getContent().equalContent(afl::string::toBytes("a=b,\\ c,\\ d\n")));
}

// Bad value
AFL_TEST("server.host.ConfigurationBuilder:bad-value", a)
{
    server::host::ConfigurationBuilder testee;
    testee.addValue("a", "x\ny");
    a.check("", testee.getContent().equalContent(afl::string::toBytes("a=x\n")));
}

// Unicode value
AFL_TEST("server.host.ConfigurationBuilder:unicode-value", a)
{
    server::host::ConfigurationBuilder testee;
    testee.addValue("qq", "x\xc3\xb6y");
    a.check("", testee.getContent().equalContent(afl::string::toBytes("qq=x\xc3\xb6y\n")));
}
