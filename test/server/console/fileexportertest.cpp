/**
  *  \file test/server/console/fileexportertest.cpp
  *  \brief Test for server::console::FileExporter
  */

#include <memory>
#include "server/console/fileexporter.hpp"

#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/test/testrunner.hpp"

using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using server::console::FileExporter;

/** Interface test. */
AFL_TEST_NOARG("server.console.FileExporter:interface")
{
    class Tester : public FileExporter {
     public:
        virtual String_t exportFile(const String_t& /*fileName*/, const String_t& /*fileContent*/)
            { return String_t(); }
    };
    Tester t;
}

/** Test behavior for NoFiles. */
AFL_TEST("server.console.FileExporter:NoFiles", a)
{
    InternalFileSystem fs;
    std::auto_ptr<FileExporter> testee(FileExporter::create(FileExporter::NoFiles, fs, "svc"));
    a.checkNull("01. create", testee.get());
}

/** Test behavior for InlineFiles. */
AFL_TEST("server.console.FileExporter:InlineFiles", a)
{
    InternalFileSystem fs;
    std::auto_ptr<FileExporter> testee(FileExporter::create(FileExporter::InlineFiles, fs, "svc"));
    a.checkNonNull("01. create", testee.get());

    a.checkEqual("11. result", testee->exportFile("sub/dir/one.txt", "hello"), "hello");
    a.checkEqual("12. result", testee->exportFile("sub/dir/two.txt", "a b\nc"), "\"a b\\nc\"");
}

/** Test behavior for NormalFiles. */
AFL_TEST("server.console.FileExporter:NormalFiles", a)
{
    InternalFileSystem fs;
    std::auto_ptr<FileExporter> testee(FileExporter::create(FileExporter::NormalFiles, fs, "svc"));
    a.checkNonNull("01. create", testee.get());

    a.checkEqual("11. result", testee->exportFile("sub/dir/one.txt", "hello"), "<svc/sub/dir/one.txt");
    a.checkEqual("12. result", testee->exportFile("sub/dir/two.txt", "a b\nc"), "<svc/sub/dir/two.txt");

    a.checkEqualContent("21. content", fs.openFile("svc/sub/dir/one.txt", FileSystem::OpenRead)->createVirtualMapping()->get(), afl::string::toBytes("hello"));
    a.checkEqualContent("22. content", fs.openFile("svc/sub/dir/two.txt", FileSystem::OpenRead)->createVirtualMapping()->get(), afl::string::toBytes("a b\nc"));
}

/** Test behavior for DedupFiles. */
AFL_TEST("server.console.FileExporter:dedup", a)
{
    InternalFileSystem fs;
    std::auto_ptr<FileExporter> testee(FileExporter::create(FileExporter::DedupFiles, fs, "svc"));
    a.checkNonNull("01. create", testee.get());

    a.checkEqual("11. result", testee->exportFile("sub/dir/one.txt", "hello"), "<svc/aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
    a.checkEqual("12. result", testee->exportFile("sub/dir/two.txt", "a b\nc"), "<svc/e693c1a928191a20147dfff8d999c99009fbc10c");

    a.checkEqualContent("21. content", fs.openFile("svc/aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d", FileSystem::OpenRead)->createVirtualMapping()->get(), afl::string::toBytes("hello"));
    a.checkEqualContent("22. content", fs.openFile("svc/e693c1a928191a20147dfff8d999c99009fbc10c", FileSystem::OpenRead)->createVirtualMapping()->get(), afl::string::toBytes("a b\nc"));
}

