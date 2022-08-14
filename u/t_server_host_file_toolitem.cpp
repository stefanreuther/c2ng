/**
  *  \file u/t_server_host_file_toolitem.cpp
  *  \brief Test for server::host::file::ToolItem
  */

#include <stdexcept>
#include "server/host/file/toolitem.hpp"

#include "t_server_host_file.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"

/** Basic test. */
void
TestServerHostFileToolItem::testIt()
{
    // Set up a filer
    server::file::InternalFileServer fs;
    server::interface::FileBaseClient client(fs);
    client.createDirectory("tooldir");
    client.putFile("tooldir/race.nm", "content");
    client.putFile("tooldir/truehull.dat", "content2");
    client.setDirectoryPermissions("tooldir", "foo", "rl");
    client.createDirectory("tooldir/subdir");

    server::host::Session session;
    session.setUser("foo");

    // Testee
    server::host::file::ToolItem testee(session, fs, "testee", "tooldir", "My Tool", afl::base::Nothing);

    // - Name
    TS_ASSERT_EQUALS(testee.getName(), "testee");

    // - Info
    TS_ASSERT_EQUALS(testee.getInfo().type, server::interface::FileBase::IsDirectory);
    TS_ASSERT_EQUALS(testee.getInfo().label, server::interface::HostFile::ToolLabel);
    TS_ASSERT_EQUALS(testee.getInfo().toolName.orElse(""), "My Tool");

    // - Directory content
    server::host::file::Item::ItemVector_t vec;
    TS_ASSERT_THROWS_NOTHING(testee.listContent(vec));
    TS_ASSERT_EQUALS(vec.size(), 2U);

    server::host::file::Item* p = (vec[0]->getName() == "race.nm" ? vec[0] : vec[1]);
    TS_ASSERT_EQUALS(p->getName(), "race.nm");
    TS_ASSERT_EQUALS(p->getInfo().type, server::interface::FileBase::IsFile);
    TS_ASSERT_EQUALS(p->getContent(), "content");

    p = (vec[0]->getName() == "race.nm" ? vec[1] : vec[0]);
    TS_ASSERT_EQUALS(p->getName(), "truehull.dat");
    TS_ASSERT_EQUALS(p->getInfo().type, server::interface::FileBase::IsFile);
    TS_ASSERT_EQUALS(p->getContent(), "content2");

    // - File content.
    TS_ASSERT_THROWS(testee.getContent(), std::exception);
}

/** Test restricted tool. */
void
TestServerHostFileToolItem::testRestricted()
{
    // Set up a filer
    server::file::InternalFileServer fs;
    server::interface::FileBaseClient client(fs);
    client.createDirectory("tooldir");
    client.putFile("tooldir/race.nm", "content");
    client.putFile("tooldir/truehull.dat", "content2");
    client.setDirectoryPermissions("tooldir", "foo", "rl");
    client.createDirectory("tooldir/subdir");

    server::host::Session session;
    session.setUser("foo");

    // Empty restriction (=nothing listed)
    {
        server::host::file::ToolItem a(session, fs, "testee", "tooldir", "My Tool", String_t());
        server::host::file::Item::ItemVector_t vec;
        TS_ASSERT_THROWS_NOTHING(a.listContent(vec));
        TS_ASSERT_EQUALS(vec.size(), 0U);
    }

    // Single restriction
    {
        server::host::file::ToolItem a(session, fs, "testee", "tooldir", "My Tool", String_t("race.nm"));
        server::host::file::Item::ItemVector_t vec;
        TS_ASSERT_THROWS_NOTHING(a.listContent(vec));
        TS_ASSERT_EQUALS(vec.size(), 1U);
        TS_ASSERT_EQUALS(vec[0]->getName(), "race.nm");
    }

    // Generic restriction (still just one match)
    {
        server::host::file::ToolItem a(session, fs, "testee", "tooldir", "My Tool", String_t("storm.nm,race.nm"));
        server::host::file::Item::ItemVector_t vec;
        TS_ASSERT_THROWS_NOTHING(a.listContent(vec));
        TS_ASSERT_EQUALS(vec.size(), 1U);
        TS_ASSERT_EQUALS(vec[0]->getName(), "race.nm");
    }
}

