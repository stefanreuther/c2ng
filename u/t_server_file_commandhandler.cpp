/**
  *  \file u/t_server_file_commandhandler.cpp
  *  \brief Test for server::file::CommandHandler
  */

#include <memory>
#include <stdexcept>
#include "server/file/commandhandler.hpp"

#include "t_server_file.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/filesystemhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "afl/io/internaldirectory.hpp"

/** Simple test. */
void
TestServerFileCommandHandler::testIt()
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
    TS_ASSERT_THROWS(testee.call(empty), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);

    // - ping
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("ping")), "PONG");

    // - user
    testee.callVoid(Segment().pushBackString("USER").pushBackString("1024"));
    TS_ASSERT_EQUALS(session.getUser(), "1024");

    // - help
    TS_ASSERT(testee.callString(Segment().pushBackString("HELP")).size() > 30);

    // Actual commands
    // (all fail because NullFileSystem fails everything.)
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GET").pushBackString("foo")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LS").pushBackString("bar")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LSREG").pushBackString("bar")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LSGAME").pushBackString("bar")), std::exception);
}
