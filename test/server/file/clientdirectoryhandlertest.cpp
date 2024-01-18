/**
  *  \file test/server/file/clientdirectoryhandlertest.cpp
  *  \brief Test for server::file::ClientDirectoryHandler
  */

#include "server/file/clientdirectoryhandler.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/utils.hpp"
#include "server/types.hpp"
#include <memory>

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test against CommandHandler. */
AFL_TEST("server.file.ClientDirectoryHandler", a)
{
    afl::test::CommandHandler mock(a);
    server::file::ClientDirectoryHandler testee(mock, "b");

    // Inquiry
    a.checkEqual("01. getName", testee.getName(), "b");

    // Read content
    {
        // Input data: 3 items in a folder
        Vector::Ref_t in = Vector::create();

        Hash::Ref_t file = Hash::create();
        file->setNew("type", server::makeStringValue("file"));
        file->setNew("size", server::makeIntegerValue(504));
        file->setNew("id",   server::makeStringValue("aaaaaaaf"));
        in->pushBackString("f.txt");
        in->pushBackNew(new HashValue(file));

        Hash::Ref_t dir = Hash::create();
        dir->setNew("type", server::makeStringValue("dir"));
        dir->setNew("visibility", server::makeIntegerValue(2));
        in->pushBackString("sub");
        in->pushBackNew(new HashValue(dir));

        Hash::Ref_t ufo = Hash::create();
        ufo->setNew("type", server::makeStringValue("ufo"));
        in->pushBackString("ufo");
        in->pushBackNew(new HashValue(ufo));

        // Test
        mock.expectCall("LS, b");
        mock.provideNewResult(new VectorValue(in));
    }

    server::file::InfoVector_t content;
    server::file::listDirectory(content, testee);

    // Verify content
    // (It is sorted alphabetically because it is a std::map inbetween.)
    a.checkEqual("11. size",    content.size(), 3U);
    a.checkEqual("12. content", content[0].name, "f.txt");
    a.checkEqual("13. content", content[1].name, "sub");
    a.checkEqual("14. content", content[2].name, "ufo");

    // Get file content
    {
        mock.expectCall("GET, b/f.txt");
        mock.provideNewResult(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFile(content[0]));
        a.checkEqual("21. size", map->get().size(), 10U);
        a.check("22. content", map->get().equalContent(afl::string::toBytes("content...")));
    }
    {
        mock.expectCall("GET, b/f.txt");
        mock.provideNewResult(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFileByName("f.txt"));
        a.checkEqual("23. size", map->get().size(), 10U);
        a.check("24. content", map->get().equalContent(afl::string::toBytes("content...")));
    }

    // Create file
    {
        mock.expectCall("PUT, b/new.txt, new text");
        mock.provideNewResult(0);
        server::file::DirectoryHandler::Info newFileInfo = testee.createFile("new.txt", afl::string::toBytes("new text"));
        a.checkEqual("31. name", newFileInfo.name, "new.txt");
    }

    // Remove file
    mock.expectCall("RM, b/old.txt");
    mock.provideNewResult(0);
    testee.removeFile("old.txt");

    // Get and access subdirectory
    std::auto_ptr<server::file::DirectoryHandler> sub(testee.getDirectory(content[1]));
    mock.expectCall("PUT, b/sub/a.txt, a");
    mock.provideNewResult(0);
    sub->createFile("a.txt", afl::string::toBytes("a"));

    // Create subdirectory
    {
        mock.expectCall("MKDIR, b/sub/q");
        mock.provideNewResult(0);
        server::file::DirectoryHandler::Info newDirInfo = sub->createDirectory("q");
        a.checkEqual("41. name", newDirInfo.name, "q");
    }

    // Remove subdirectory
    mock.expectCall("RM, b/other");
    mock.provideNewResult(0);
    testee.removeDirectory("other");

    mock.checkFinish();
}
