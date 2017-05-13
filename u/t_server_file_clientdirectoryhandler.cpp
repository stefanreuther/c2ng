/**
  *  \file u/t_server_file_clientdirectoryhandler.cpp
  *  \brief Test for server::file::ClientDirectoryHandler
  */

#include <memory>
#include "server/file/clientdirectoryhandler.hpp"

#include "t_server_file.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/types.hpp"
#include "server/file/utils.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test against CommandHandlerMock. */
void
TestServerFileClientDirectoryHandler::testIt()
{
    CommandHandlerMock mock;
    server::file::ClientDirectoryHandler testee(mock, "b");

    // User configuration
    mock.expectCall("USER|a");
    mock.provideReturnValue(0);
    testee.setUser("a");

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
        mock.expectCall("LS|b");
        mock.provideReturnValue(new VectorValue(in));
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
        mock.expectCall("GET|b/f.txt");
        mock.provideReturnValue(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFile(content[0]));
        TS_ASSERT_EQUALS(map->get().size(), 10U);
        TS_ASSERT(map->get().equalContent(afl::string::toBytes("content...")));
    }
    {
        mock.expectCall("GET|b/f.txt");
        mock.provideReturnValue(server::makeStringValue("content..."));
        afl::base::Ref<afl::io::FileMapping> map(testee.getFileByName("f.txt"));
        TS_ASSERT_EQUALS(map->get().size(), 10U);
        TS_ASSERT(map->get().equalContent(afl::string::toBytes("content...")));
    }

    // Create file
    {
        mock.expectCall("PUT|b/new.txt|new text");
        mock.provideReturnValue(0);
        server::file::DirectoryHandler::Info newFileInfo = testee.createFile("new.txt", afl::string::toBytes("new text"));
        TS_ASSERT_EQUALS(newFileInfo.name, "new.txt");
    }

    // Remove file
    mock.expectCall("RM|b/old.txt");
    mock.provideReturnValue(0);
    testee.removeFile("old.txt");

    // Get and access subdirectory
    std::auto_ptr<server::file::DirectoryHandler> sub(testee.getDirectory(content[1]));
    mock.expectCall("PUT|b/sub/a.txt|a");
    mock.provideReturnValue(0);
    sub->createFile("a.txt", afl::string::toBytes("a"));

    // Create subdirectory
    {
        mock.expectCall("MKDIR|b/sub/q");
        mock.provideReturnValue(0);
        server::file::DirectoryHandler::Info newDirInfo = sub->createDirectory("q");
        TS_ASSERT_EQUALS(newDirInfo.name, "q");
    }
    
    // Remove subdirectory
    mock.expectCall("RM|b/other");
    mock.provideReturnValue(0);
    testee.removeDirectory("other");

    mock.checkFinish();
}
