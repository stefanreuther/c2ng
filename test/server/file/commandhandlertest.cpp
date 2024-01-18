/**
  *  \file test/server/file/commandhandlertest.cpp
  *  \brief Test for server::file::CommandHandler
  */

#include "server/file/commandhandler.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/filesystemhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include <memory>
#include <stdexcept>

/** Simple test. */
AFL_TEST("server.file.CommandHandler", a)
{
    // Environment
    afl::io::NullFileSystem fs;
    std::auto_ptr<server::file::DirectoryHandler> handler(new server::file::FileSystemHandler(fs, "/"));
    server::file::DirectoryItem item("(root)", 0, handler);
    server::file::Root root(item, afl::io::InternalDirectory::create("(spec)"));
    server::file::Session session;

    // Testee
    server::file::CommandHandler testee(root, session);

    // Some calls
    using afl::data::Segment;

    // - invalid
    Segment empty;    // g++-3.4 sees an invocation of a copy constructor if I construct this object in-place.
    AFL_CHECK_THROWS(a("01. empty"), testee.call(empty), std::exception);
    AFL_CHECK_THROWS(a("02. empty"), testee.callVoid(empty), std::exception);

    // - ping
    a.checkEqual("11. ping", testee.callString(Segment().pushBackString("PING")), "PONG");
    a.checkEqual("12. ping", testee.callString(Segment().pushBackString("ping")), "PONG");

    // - user
    testee.callVoid(Segment().pushBackString("USER").pushBackString("1024"));
    a.checkEqual("21. getUser", session.getUser(), "1024");

    // - help
    a.check("31. help", testee.callString(Segment().pushBackString("HELP")).size() > 30);

    // Actual commands
    // (all fail because NullFileSystem fails everything.)
    AFL_CHECK_THROWS(a("41. get"),     testee.callVoid(Segment().pushBackString("GET").pushBackString("foo")), std::exception);
    AFL_CHECK_THROWS(a("42. ls"),      testee.callVoid(Segment().pushBackString("LS").pushBackString("bar")), std::exception);
    AFL_CHECK_THROWS(a("43. lsreg"),   testee.callVoid(Segment().pushBackString("LSREG").pushBackString("bar")), std::exception);
    AFL_CHECK_THROWS(a("44. lsgamet"), testee.callVoid(Segment().pushBackString("LSGAME").pushBackString("bar")), std::exception);
}
