/**
  *  \file test/server/monitor/badnessfileobservertest.cpp
  *  \brief Test for server::monitor::BadnessFileObserver
  */

#include "server/monitor/badnessfileobserver.hpp"

#include "afl/io/directory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/temporarydirectory.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    void testFile(afl::test::Assert a, String_t content, server::monitor::Observer::Status expectedResult)
    {
        // Create file
        // Must use a real file system here because InternalFileSystem does not implement getModificationTime().
        afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
        afl::io::TemporaryDirectory dir(fs.openDirectory(fs.getWorkingDirectoryName()));

        const char*const FILE_NAME = "__test.tmp";
        dir.get()->openFile(FILE_NAME, fs.Create)->fullWrite(afl::string::toBytes(content));

        // Create testee
        server::monitor::BadnessFileObserver testee("n", "KEY", fs);
        testee.handleConfiguration("KEY", fs.makePathName(dir.get()->getDirectoryName(), FILE_NAME));

        // Test
        a(content).checkEqual("checkStatus", testee.checkStatus(), expectedResult);
    }
}


/** Simple test for basic operations. */
AFL_TEST("server.monitor.BadnessFileObserver:basics", a)
{
    afl::io::NullFileSystem fs;
    server::monitor::BadnessFileObserver testee("the name", "KEY", fs);

    // getName
    a.checkEqual("01. getName", testee.getName(), "the name");

    // handleConfiguration
    a.check("11. handleConfiguration", testee.handleConfiguration("KEY", "file.txt"));
    a.check("12. handleConfiguration", !testee.handleConfiguration("OTHER", ""));

    // checkStatus
    a.checkEqual("21. checkStatus", testee.checkStatus(), server::monitor::Observer::Down);
}

/** Test various file content. */
AFL_TEST("server.monitor.BadnessFileObserver:content", a)
{
    // Success cases
    testFile(a, "0", server::monitor::Observer::Running);
    testFile(a, "1", server::monitor::Observer::Running);

    // Degenerate success cases
    testFile(a, "", server::monitor::Observer::Running);
    testFile(a, "0000000000", server::monitor::Observer::Running);
    testFile(a, "0000000001", server::monitor::Observer::Running);

    // Whitespace will be accepted
    testFile(a, "0\n", server::monitor::Observer::Running);
    testFile(a, "     0", server::monitor::Observer::Running);
    testFile(a, "0     ", server::monitor::Observer::Running);

    // Error cases
    testFile(a, "2",   server::monitor::Observer::Broken);
    testFile(a, "999", server::monitor::Observer::Broken);
    testFile(a, "1x",  server::monitor::Observer::Broken);
    testFile(a, "x1",  server::monitor::Observer::Broken);
    testFile(a, "0x1", server::monitor::Observer::Broken);
}
