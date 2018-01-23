/**
  *  \file u/t_server_file_clientdirectoryhandler.cpp
  *  \brief Test for server::file::ClientDirectoryHandler
  */

#include "server/file/clientdirectoryhandler.hpp"

#include <memory>
#include "t_server_file.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/file/utils.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test against CommandHandler. */
void
TestServerFileClientDirectoryHandler::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::file::ClientDirectoryHandler testee(mock, "b");

    // Inquiry
    TS_ASSERT_EQUALS(testee.getName(), "b");

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
    TS_ASSERT_EQUALS(content.size(), 3U);
    TS_ASSERT_EQUALS(content[0].name, "f.txt");
    TS_ASSERT_EQUALS(content[1].name, "sub");
    TS_ASSERT_EQUALS(content[2].name, "ufo");

    // Get file content
    {
        mock.expectCall("GET, b/f.txt");
        mock.provideNewResult(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFile(content[0]));
        TS_ASSERT_EQUALS(map->get().size(), 10U);
        TS_ASSERT(map->get().equalContent(afl::string::toBytes("content...")));
    }
    {
        mock.expectCall("GET, b/f.txt");
        mock.provideNewResult(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFileByName("f.txt"));
        TS_ASSERT_EQUALS(map->get().size(), 10U);
        TS_ASSERT(map->get().equalContent(afl::string::toBytes("content...")));
    }

    // Create file
    {
        mock.expectCall("PUT, b/new.txt, new text");
        mock.provideNewResult(0);
        server::file::DirectoryHandler::Info newFileInfo = testee.createFile("new.txt", afl::string::toBytes("new text"));
        TS_ASSERT_EQUALS(newFileInfo.name, "new.txt");
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
        TS_ASSERT_EQUALS(newDirInfo.name, "q");
    }
    
    // Remove subdirectory
    mock.expectCall("RM, b/other");
    mock.provideNewResult(0);
    testee.removeDirectory("other");

    mock.checkFinish();
}
