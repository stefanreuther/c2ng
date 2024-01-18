/**
  *  \file test/server/host/file/toolitemtest.cpp
  *  \brief Test for server::host::file::ToolItem
  */

#include "server/host/file/toolitem.hpp"

#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/session.hpp"
#include "server/interface/filebaseclient.hpp"
#include <stdexcept>

/** Basic test. */
AFL_TEST("server.host.file.ToolItem:basics", a)
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
    a.checkEqual("01. getName", testee.getName(), "testee");

    // - Info
    a.checkEqual("11. type",     testee.getInfo().type, server::interface::FileBase::IsDirectory);
    a.checkEqual("12. label",    testee.getInfo().label, server::interface::HostFile::ToolLabel);
    a.checkEqual("13. toolName", testee.getInfo().toolName.orElse(""), "My Tool");

    // - Directory content
    server::host::file::Item::ItemVector_t vec;
    AFL_CHECK_SUCCEEDS(a("21. listContent"), testee.listContent(vec));
    a.checkEqual("22. size", vec.size(), 2U);

    server::host::file::Item* p = (vec[0]->getName() == "race.nm" ? vec[0] : vec[1]);
    a.checkEqual("31. getName",    p->getName(), "race.nm");
    a.checkEqual("32. type",       p->getInfo().type, server::interface::FileBase::IsFile);
    a.checkEqual("33. getContent", p->getContent(), "content");

    p = (vec[0]->getName() == "race.nm" ? vec[1] : vec[0]);
    a.checkEqual("41. getName",    p->getName(), "truehull.dat");
    a.checkEqual("42. type",       p->getInfo().type, server::interface::FileBase::IsFile);
    a.checkEqual("43. getContent", p->getContent(), "content2");

    // - File content.
    AFL_CHECK_THROWS(a("51. getContent"), testee.getContent(), std::exception);
}

/** Test restricted tool. */
AFL_TEST("server.host.file.ToolItem:restricted", a)
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
        server::host::file::ToolItem it(session, fs, "testee", "tooldir", "My Tool", String_t());
        server::host::file::Item::ItemVector_t vec;
        AFL_CHECK_SUCCEEDS(a("01. listContent"), it.listContent(vec));
        a.checkEqual("02. size", vec.size(), 0U);
    }

    // Single restriction
    {
        server::host::file::ToolItem it(session, fs, "testee", "tooldir", "My Tool", String_t("race.nm"));
        server::host::file::Item::ItemVector_t vec;
        AFL_CHECK_SUCCEEDS(a("11. listContent"), it.listContent(vec));
        a.checkEqual("12. size", vec.size(), 1U);
        a.checkEqual("13. name", vec[0]->getName(), "race.nm");
    }

    // Generic restriction (still just one match)
    {
        server::host::file::ToolItem it(session, fs, "testee", "tooldir", "My Tool", String_t("storm.nm,race.nm"));
        server::host::file::Item::ItemVector_t vec;
        AFL_CHECK_SUCCEEDS(a("21. listContent"), it.listContent(vec));
        a.checkEqual("22. size", vec.size(), 1U);
        a.checkEqual("23. name", vec[0]->getName(), "race.nm");
    }
}
