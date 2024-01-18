/**
  *  \file test/server/host/file/fileitemtest.cpp
  *  \brief Test for server::host::file::FileItem
  */

#include "server/host/file/fileitem.hpp"

#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"
#include <stdexcept>

namespace hf = server::host::file;

/** Test FileItem interface methods. */
AFL_TEST("server.host.file.FileItem:basics", a)
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

        a.checkEqual("01. getName", testee.getName(), "fn");
        a.checkEqual("02. size", testee.getInfo().size.orElse(0), 99);
        AFL_CHECK_THROWS(a("03. find"), testee.find("x"), std::runtime_error);

        hf::Item::ItemVector_t vec;
        AFL_CHECK_THROWS(a("11. listContent"), testee.listContent(vec), std::runtime_error);

        a.checkEqual("21. getContent", testee.getContent(), "content");
    }

    // Test admin file
    {
        hf::FileItem testee(fs, "dir/f", "", i);
        a.checkEqual("31. getContent", testee.getContent(), "content");
    }

    // Test wrong user file
    {
        hf::FileItem testee(fs, "dir/f", "not_u", i);
        AFL_CHECK_THROWS(a("41. getContent"), testee.getContent(), std::runtime_error);
    }

    // Test nonexistant
    {
        hf::FileItem testee(fs, "dir/fx", "", i);
        AFL_CHECK_THROWS(a("51. getContent"), testee.getContent(), std::runtime_error);
    }
}

/** Test FileItem::listFileServerContent(). */
AFL_TEST("server.host.file.FileItem:listFileServerContent", a)
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
        a.checkEqual("01. size", vec.size(), 3U);
        a.checkEqual("02. getName", vec[0]->getName(), "a");
        a.checkEqual("03. getName", vec[1]->getName(), "b");
        a.checkEqual("04. getName", vec[2]->getName(), "f");
        a.checkEqual("05. getContent", vec[0]->getContent(), "ca");
        a.checkEqual("06. getContent", vec[1]->getContent(), "cb");
        a.checkEqual("07. getContent", vec[2]->getContent(), "cf");
    }

    // Try as owner
    {
        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "u", vec);
        a.checkEqual("11. size", vec.size(), 3U);
    }

    // Try as other
    {
        hf::Item::ItemVector_t vec;
        AFL_CHECK_THROWS(a("21. listFileServerContent"), hf::FileItem::listFileServerContent(fs, "dir", "other_u", vec), std::exception);
        a.checkEqual("22. size", vec.size(), 0U);
    }
}

/** Test FileItem::listFileServerContent(), limited version. */
AFL_TEST("server.host.file.FileItem:listFileServerContent:limited", a)
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
        AFL_CHECK_SUCCEEDS(a("01. listFileServerContent"), hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec));
        a.checkEqual("02. size", vec.size(), 0U);
    }

    // List with empty filter, correct user
    {
        afl::data::StringList_t filter;
        hf::Item::ItemVector_t vec;
        AFL_CHECK_SUCCEEDS(a("11. listFileServerContent"), hf::FileItem::listFileServerContent(fs, "dir", "u", filter, vec));
        a.checkEqual("12. size", vec.size(), 0U);
    }

    // List with mismatching singleton filter
    {
        afl::data::StringList_t filter;
        filter.push_back("q");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        a.checkEqual("21. size", vec.size(), 0U);
    }

    // List with mismatching singleton filter, wrong user
    {
        afl::data::StringList_t filter;
        filter.push_back("q");

        hf::Item::ItemVector_t vec;
        AFL_CHECK_THROWS(a("31. listFileServerContent"), hf::FileItem::listFileServerContent(fs, "dir", "other_u", filter, vec), std::runtime_error);
    }

    // List with matching singleton filter
    {
        afl::data::StringList_t filter;
        filter.push_back("b");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        a.checkEqual("41. size", vec.size(), 1U);
        a.checkEqual("42. getName", vec[0]->getName(), "b");
        a.checkEqual("43. getInfo", vec[0]->getInfo().size.orElse(0), 2);
    }

    // List with matching singleton filter, wrong user
    {
        afl::data::StringList_t filter;
        filter.push_back("b");

        hf::Item::ItemVector_t vec;
        AFL_CHECK_THROWS(a("51. listFileServerContent"), hf::FileItem::listFileServerContent(fs, "dir", "other_u", filter, vec), std::runtime_error);
    }

    // List with matching general filter
    {
        afl::data::StringList_t filter;
        filter.push_back("b");
        filter.push_back("c");
        filter.push_back("a");

        hf::Item::ItemVector_t vec;
        hf::FileItem::listFileServerContent(fs, "dir", "", filter, vec);
        a.checkEqual("61. size", vec.size(), 2U);
        a.checkEqual("62. getName", vec[0]->getName(), "a");
        a.checkEqual("63. getInfo", vec[0]->getInfo().size.orElse(0), 1);
        a.checkEqual("64. getName", vec[1]->getName(), "b");
        a.checkEqual("65. getInfo", vec[1]->getInfo().size.orElse(0), 2);
    }
}
