/**
  *  \file u/t_server_monitor_badnessfileobserver.cpp
  *  \brief Test for server::monitor::BadnessFileObserver
  */

#include "server/monitor/badnessfileobserver.hpp"

#include "t_server_monitor.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/directory.hpp"

namespace {
    void testFile(String_t content, server::monitor::Observer::Status expectedResult)
    {
        // Create file
        const char*const FILE_NAME = "__test.tmp";
        afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
        fs.openFile(FILE_NAME, fs.Create)->fullWrite(afl::string::toBytes(content));

        // Create testee
        server::monitor::BadnessFileObserver testee("n", "KEY", fs);
        testee.handleConfiguration("KEY", FILE_NAME);

        // Test
        TSM_ASSERT_EQUALS(content, testee.checkStatus(), expectedResult);

        // Cleanup
        fs.openDirectory(fs.getDirectoryName(FILE_NAME))->eraseNT(fs.getFileName(FILE_NAME));
    }
}


/** Simple test for basic operations. */
void
TestServerMonitorBadnessFileObserver::testBasic()
{
    afl::io::NullFileSystem fs;
    server::monitor::BadnessFileObserver testee("the name", "KEY", fs);

    // getName
    TS_ASSERT_EQUALS(testee.getName(), "the name");

    // handleConfiguration
    TS_ASSERT(testee.handleConfiguration("KEY", "file.txt"));
    TS_ASSERT(!testee.handleConfiguration("OTHER", ""));

    // checkStatus
    TS_ASSERT_EQUALS(testee.checkStatus(), server::monitor::Observer::Down);
}

/** Test various file content. */
void
TestServerMonitorBadnessFileObserver::testContent()
{
    // Success cases
    testFile("0", server::monitor::Observer::Running);
    testFile("1", server::monitor::Observer::Running);

    // Degenerate success cases
    testFile("", server::monitor::Observer::Running);
    testFile("0000000000", server::monitor::Observer::Running);
    testFile("0000000001", server::monitor::Observer::Running);

    // Whitespace will be accepted
    testFile("0\n", server::monitor::Observer::Running);
    testFile("     0", server::monitor::Observer::Running);
    testFile("0     ", server::monitor::Observer::Running);

    // Error cases
    testFile("2",   server::monitor::Observer::Broken);
    testFile("999", server::monitor::Observer::Broken);
    testFile("1x",  server::monitor::Observer::Broken);
    testFile("x1",  server::monitor::Observer::Broken);
    testFile("0x1", server::monitor::Observer::Broken);
}

