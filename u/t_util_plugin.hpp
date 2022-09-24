/**
  *  \file u/t_util_plugin.hpp
  *  \brief Tests for util::plugin
  */
#ifndef C2NG_U_T_UTIL_PLUGIN_HPP
#define C2NG_U_T_UTIL_PLUGIN_HPP

#include <cxxtest/TestSuite.h>

class TestUtilPluginManager : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCycle();
    void testNull();
    void testDescribe();
    void testDescribeNull();
};

class TestUtilPluginPlugin : public CxxTest::TestSuite {
 public:
    void testVersion();
    void testInit();
    void testInitPlugin();
    void testInitResource();
    void testInitScript();
    void testInitConfig();
    void testInitScript2();
    void testInitScript3();
    void testSelfDepend();
    void testDepend();
    void testUpdate();
    void testUnrelated();
};

#endif
