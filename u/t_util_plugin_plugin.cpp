/**
  *  \file u/t_util_plugin_plugin.cpp
  *  \brief Test for util::plugin::Plugin
  */

#include "util/plugin/plugin.hpp"

#include "t_util_plugin.hpp"

void
TestUtilPluginPlugin::testVersion()
{
    // ex UtilPluginTestSuite::testVersion
    using util::plugin::compareVersions;

    TS_ASSERT( compareVersions("1.0", "1.0.1"));
    TS_ASSERT(!compareVersions("1.0.1", "1.0"));

    TS_ASSERT(!compareVersions("1.0", "1.0"));

    TS_ASSERT( compareVersions("1.0", "1.0a"));
    TS_ASSERT(!compareVersions("1.0a", "1.0"));

    TS_ASSERT( compareVersions("a", "b"));
    TS_ASSERT(!compareVersions("b", "a"));

    TS_ASSERT( compareVersions("a", "1"));
    TS_ASSERT(!compareVersions("1", "a"));

    TS_ASSERT( compareVersions("99", "100"));
    TS_ASSERT(!compareVersions("100", "99"));
}

