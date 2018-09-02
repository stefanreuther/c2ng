/**
  *  \file u/t_server_host_file_fileitem.cpp
  *  \brief Test for server::host::file::FileItem
  */

#include <stdexcept>
#include "server/host/file/fileitem.hpp"

#include "t_server_host_file.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"

namespace hf = server::host::file;

/** Test FileItem interface methods. */
void
TestServerHostFileFileItem::testIt()
{
    // Set up a filer
    server::file::InternalFileServer fs;
    server::interface::FileBaseClient client(fs);
    client.createDirectoryAsUser("dir", "u");
    client.putFile("dir/f", "content");

    // Some file information
    hf::Item::Info_t i;
    i.name = "fn";
    i.size = 99;

    // Test user file
    {
        hf::FileItem testee(fs, "dir/f", "u", i);

        TS_ASSERT_EQUALS(testee.getName(), "fn");
        TS_ASSERT_EQUALS(testee.getInfo().size.orElse(0), 99);
        TS_ASSERT_THROWS(testee.find("x"), std::runtime_error);

        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS(testee.listContent(vec), std::runtime_error);

        TS_ASSERT_EQUALS(testee.getContent(), "content");
    }

    // Test admin file
    {
        hf::FileItem testee(fs, "dir/f", "", i);
        TS_ASSERT_EQUALS(testee.getContent(), "content");
    }

    // Test wrong user file
    {
        hf::FileItem testee(fs, "dir/f", "not_u", i);
        TS_ASSERT_THROWS(testee.getContent(), std::runtime_error);
    }

    // Test nonexistant
    {
        hf::FileItem testee(fs, "dir/fx", "", i);
        TS_ASSERT_THROWS(testee.getContent(), std::runtime_error);
    }
}

/** Test FileItem::listFileServerContent(). */
void
TestServerHostFileFileItem::testList()
{
    // Set up a filer
    server::file::InternalFileServer fs;
    server::interface::FileBaseClient client(fs);
    client.createDirectoryAsUser("dir", "u");
    client.putFile("dir/a", "ca");
    client.putFile("dir/b", "cb");
    client.putFile("dir/f", "cf");
    client.createDirectory("dir/d");

    // Try as admin
    {
        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", vec);
        TS_ASSERT_EQUALS(vec.size(), 3U);
        TS_ASSERT_EQUALS(vec[0]->getName(), "a");
        TS_ASSERT_EQUALS(vec[1]->getName(), "b");
        TS_ASSERT_EQUALS(vec[2]->getName(), "f");
        TS_ASSERT_EQUALS(vec[0]->getContent(), "ca");
        TS_ASSERT_EQUALS(vec[1]->getContent(), "cb");
        TS_ASSERT_EQUALS(vec[2]->getContent(), "cf");
    }

    // Try as owner
    {
        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "u", vec);
        TS_ASSERT_EQUALS(vec.size(), 3U);
    }

    // Try as other
    {
        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS(hf::FileItem::listFileServerContent(fs, "dir", "other_u", vec), std::exception);
        TS_ASSERT_EQUALS(vec.size(), 0U);
    }
}

/** Test FileItem::listFileServerContent(), limited version. */
void
TestServerHostFileFileItem::testListLimited()
{
    // Set up a filer
    server::file::InternalFileServer fs;
    server::interface::FileBaseClient client(fs);
    client.createDirectoryAsUser("dir", "u");
    client.putFile("dir/a", "a");
    client.putFile("dir/b", "bb");
    client.putFile("dir/f", "ffffff");
    client.createDirectory("dir/d");

    // List with empty filter
    {
        afl::data::StringList_t filter;
        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS_NOTHING(hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec));
        TS_ASSERT_EQUALS(vec.size(), 0U);
    }

    // List with empty filter, correct user
    {
        afl::data::StringList_t filter;
        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS_NOTHING(hf::FileItem::listFileServerContent(fs, "dir", "u", filter, vec));
        TS_ASSERT_EQUALS(vec.size(), 0U);
    }

    // List with mismatching singleton filter
    {
        afl::data::StringList_t filter;
        filter.push_back("q");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        TS_ASSERT_EQUALS(vec.size(), 0U);
    }

    // List with mismatching singleton filter, wrong user
    {
        afl::data::StringList_t filter;
        filter.push_back("q");

        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS(hf::FileItem::listFileServerContent(fs, "dir", "other_u", filter, vec), std::runtime_error);
    }

    // List with matching singleton filter
    {
        afl::data::StringList_t filter;
        filter.push_back("b");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        TS_ASSERT_EQUALS(vec.size(), 1U);
        TS_ASSERT_EQUALS(vec[0]->getName(), "b");
        TS_ASSERT_EQUALS(vec[0]->getInfo().size.orElse(0), 2);
    }

    // List with matching singleton filter, wrong user
    {
        afl::data::StringList_t filter;
        filter.push_back("b");

        hf::Item::ItemVector_t vec;
        TS_ASSERT_THROWS(hf::FileItem::listFileServerContent(fs, "dir", "other_u", filter, vec), std::runtime_error);
    }

    // List with matching general filter
    {
        afl::data::StringList_t filter;
        filter.push_back("b");
        filter.push_back("c");
        filter.push_back("a");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        TS_ASSERT_EQUALS(vec.size(), 2U);
        TS_ASSERT_EQUALS(vec[0]->getName(), "a");
        TS_ASSERT_EQUALS(vec[0]->getInfo().size.orElse(0), 1);
        TS_ASSERT_EQUALS(vec[1]->getName(), "b");
        TS_ASSERT_EQUALS(vec[1]->getInfo().size.orElse(0), 2);
    }
}
