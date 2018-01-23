/**
  *  \file u/t_server_monitor.hpp
  *  \brief Tests for server::monitor
  */
#ifndef C2NG_U_T_SERVER_MONITOR_HPP
#define C2NG_U_T_SERVER_MONITOR_HPP

#include <cxxtest/TestSuite.h>

class TestServerMonitorBadnessFileObserver : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testContent();
};

class TestServerMonitorObserver : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerMonitorStatus : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNull();
    void testSingle();
    void testMulti();
};

class TestServerMonitorStatusObserver : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerMonitorTimeSeries : public CxxTest::TestSuite {
 public:
    void testAddGet();
    void testCompact();
    void testRender();
    void testRenderEmpty();
    void testRenderSimple();
    void testRenderAges();
};

class TestServerMonitorTimeSeriesLoader : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerMonitorTimeSeriesWriter : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
};

#endif
