/**
  *  \file test/server/file/filesnapshottest.cpp
  *  \brief Test for server::file::FileSnapshot
  */

#include <algorithm>
#include "server/file/filesnapshot.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/io/internaldirectory.hpp"
#include "server/file/session.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/ca/root.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/root.hpp"

using server::file::Session;
using server::file::FileSnapshot;

namespace {
    /* Environment with a CA-backed service */
    struct Environment {
        server::file::InternalDirectoryHandler::Directory dir;
        server::file::InternalDirectoryHandler dirHandler;
        server::file::ca::Root caRoot;
        server::file::DirectoryItem caItem;
        server::file::Root serviceRoot;

        Environment()
            : dir("root"),
              dirHandler("root", dir),
              caRoot(dirHandler),
              caItem("root", 0, std::auto_ptr<server::file::DirectoryHandler>(caRoot.createRootHandler())),
              serviceRoot(caItem, afl::io::InternalDirectory::create("spec"))
            { }
    };

    /* Environment with a non-CA-backed service */
    struct PlainEnvironment {
        server::file::InternalDirectoryHandler::Directory dir;
        server::file::DirectoryItem dirItem;
        server::file::Root serviceRoot;

        PlainEnvironment()
            : dir("root"),
              dirItem("root", 0, std::auto_ptr<server::file::DirectoryHandler>(new server::file::InternalDirectoryHandler("root", dir))),
              serviceRoot(dirItem, afl::io::InternalDirectory::create("spec"))
            { }
    };
}

/*
 *  createSnapshot
 */

/* createSnapshot, success case */
AFL_TEST("server.file.FileSnapshot:createSnapshot", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    testee.createSnapshot("a");

    afl::data::StringList_t list;
    env.caRoot.listSnapshots(list);
    a.checkEqual("num snaps", list.size(), 1U);
    a.checkEqual("snap name", list[0], "a");
}

/* createSnapshot, error case: bad name */
AFL_TEST("server.file.FileSnapshot:createSnapshot:error:bad-name", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a("empty name"),      testee.createSnapshot(""),         std::runtime_error);
    AFL_CHECK_THROWS(a("start with dot"),  testee.createSnapshot(".foo"),     std::runtime_error);
    AFL_CHECK_THROWS(a("end with dot"),    testee.createSnapshot("foo."),     std::runtime_error);
    AFL_CHECK_THROWS(a("double-dot"),      testee.createSnapshot("foo..bar"), std::runtime_error);
    AFL_CHECK_THROWS(a("bad char dollar"), testee.createSnapshot("a$b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char colon"),  testee.createSnapshot("a:b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char slash"),  testee.createSnapshot("a/b"),      std::runtime_error);
}

/* createSnapshot, error case: not admin */
AFL_TEST("server.file.FileSnapshot:createSnapshot:error:user", a)
{
    Environment env;
    Session session;
    session.setUser("x");
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a, testee.createSnapshot("a"), std::runtime_error);
}

/* createSnapshot, error case: not CA backend */
AFL_TEST("server.file.FileSnapshot:createSnapshot:error:plain", a)
{
    PlainEnvironment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a, testee.createSnapshot("a"), std::runtime_error);
}

/*
 *  copySnapshot
 */

/* copySnapshot, success case */
AFL_TEST("server.file.FileSnapshot:copySnapshot", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());

    testee.copySnapshot("a", "b");

    afl::data::StringList_t list;
    env.caRoot.listSnapshots(list);
    std::sort(list.begin(), list.end());
    a.checkEqual("num snaps", list.size(), 2U);
    a.checkEqual("snap name a", list[0], "a");
    a.checkEqual("snap name b", list[1], "b");
}

/* copySnapshot, error case: source does not exist */
AFL_TEST("server.file.FileSnapshot:copySnapshot:error:missing", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a, testee.copySnapshot("a", "b"), std::exception);
}

/* copySnapshot, error case: bad name */
AFL_TEST("server.file.FileSnapshot:copySnapshot:error:bad-name", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());

    AFL_CHECK_THROWS(a("empty name"),      testee.copySnapshot("a", ""),         std::runtime_error);
    AFL_CHECK_THROWS(a("start with dot"),  testee.copySnapshot("a", ".foo"),     std::runtime_error);
    AFL_CHECK_THROWS(a("end with dot"),    testee.copySnapshot("a", "foo."),     std::runtime_error);
    AFL_CHECK_THROWS(a("double-dot"),      testee.copySnapshot("a", "foo..bar"), std::runtime_error);
    AFL_CHECK_THROWS(a("bad char dollar"), testee.copySnapshot("a", "a$b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char colon"),  testee.copySnapshot("a", "a:b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char slash"),  testee.copySnapshot("a", "a/b"),      std::runtime_error);
}

/* copySnapshot, error case: not admin */
AFL_TEST("server.file.FileSnapshot:copySnapshot:error:user", a)
{
    Environment env;
    Session session;
    session.setUser("x");
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());

    AFL_CHECK_THROWS(a, testee.copySnapshot("a", "b"), std::runtime_error);
}

/* copySnapshot, error case: not CA backend */
AFL_TEST("server.file.FileSnapshot:copySnapshot:error:plain", a)
{
    PlainEnvironment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a, testee.copySnapshot("a", "b"), std::runtime_error);
}

/*
 *  removeSnapshot
 */

/* removeSnapshot, success case */
AFL_TEST("server.file.FileSnapshot:removeSnapshot", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());
    env.caRoot.setSnapshotCommitId("b", env.caRoot.getMasterCommitId());

    testee.removeSnapshot("a");

    afl::data::StringList_t list;
    env.caRoot.listSnapshots(list);
    std::sort(list.begin(), list.end());
    a.checkEqual("num snaps", list.size(), 1U);
    a.checkEqual("snap name b", list[0], "b");
}

/* removeSnapshot: removing non-existant is not an error */
AFL_TEST("server.file.FileSnapshot:removeSnapshot:error:missing", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_SUCCEEDS(a, testee.removeSnapshot("a"));
}

/* removeSnapshot, error case: bad name */
AFL_TEST("server.file.FileSnapshot:removeSnapshot:error:bad-name", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a("empty name"),      testee.removeSnapshot(""),         std::runtime_error);
    AFL_CHECK_THROWS(a("start with dot"),  testee.removeSnapshot(".foo"),     std::runtime_error);
    AFL_CHECK_THROWS(a("end with dot"),    testee.removeSnapshot("foo."),     std::runtime_error);
    AFL_CHECK_THROWS(a("double-dot"),      testee.removeSnapshot("foo..bar"), std::runtime_error);
    AFL_CHECK_THROWS(a("bad char dollar"), testee.removeSnapshot("a$b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char colon"),  testee.removeSnapshot("a:b"),      std::runtime_error);
    AFL_CHECK_THROWS(a("bad char slash"),  testee.removeSnapshot("a/b"),      std::runtime_error);
}

/* removeSnapshot, error case: not admin */
AFL_TEST("server.file.FileSnapshot:removeSnapshot:error:user", a)
{
    Environment env;
    Session session;
    session.setUser("x");
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());

    AFL_CHECK_THROWS(a, testee.removeSnapshot("a"), std::runtime_error);

    // Still there
    afl::data::StringList_t list;
    env.caRoot.listSnapshots(list);
    std::sort(list.begin(), list.end());
    a.checkEqual("num snaps", list.size(), 1U);
    a.checkEqual("snap name a", list[0], "a");
}

/* removeSnapshot, error case: not CA backend */
AFL_TEST("server.file.FileSnapshot:removeSnapshot:error:plain", a)
{
    PlainEnvironment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    AFL_CHECK_THROWS(a, testee.removeSnapshot("a"), std::runtime_error);
}

/*
 *  listSnapshots
 */

/* listSnapshots, success case */
AFL_TEST("server.file.FileSnapshot:listSnapshots", a)
{
    Environment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());
    env.caRoot.setSnapshotCommitId("b", env.caRoot.getMasterCommitId());

    afl::data::StringList_t list;
    testee.listSnapshots(list);

    std::sort(list.begin(), list.end());
    a.checkEqual("num snaps", list.size(), 2U);
    a.checkEqual("snap name a", list[0], "a");
    a.checkEqual("snap name b", list[1], "b");
}

/* listSnapshots, error case: not admin */
AFL_TEST("server.file.FileSnapshot:listSnapshots:error:user", a)
{
    Environment env;
    Session session;
    session.setUser("x");
    FileSnapshot testee(session, env.serviceRoot);
    env.caRoot.setSnapshotCommitId("a", env.caRoot.getMasterCommitId());
    env.caRoot.setSnapshotCommitId("b", env.caRoot.getMasterCommitId());

    afl::data::StringList_t list;
    AFL_CHECK_THROWS(a, testee.listSnapshots(list), std::exception);
}

/* listSnapshots, error case: not CA backend */
AFL_TEST("server.file.FileSnapshot:listSnapshots:error:plain", a)
{
    PlainEnvironment env;
    Session session;
    FileSnapshot testee(session, env.serviceRoot);

    afl::data::StringList_t list;
    AFL_CHECK_THROWS(a, testee.listSnapshots(list), std::exception);
}
