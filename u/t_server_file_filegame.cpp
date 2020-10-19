/**
  *  \file u/t_server_file_filegame.cpp
  *  \brief Test for server::file::FileGame
  */

#include <algorithm>
#include "server/file/filegame.hpp"

#include "t_server_file.hpp"
#include "afl/io/internaldirectory.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/filebase.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "u/files.hpp"


#define TS_ASSERT_THROWS_CODE(call, code)                               \
                              do {                                      \
                                  bool threw = false;                   \
                                  try {                                 \
                                      call;                             \
                                  }                                     \
                                  catch (std::exception& e) {           \
                                      TS_ASSERT_EQUALS(String_t(e.what()).substr(0, 3), code); \
                                      threw = true;                     \
                                  }                                     \
                                  catch (...) {                         \
                                      TS_ASSERT(!"Wrong exception");    \
                                      threw = true;                     \
                                  }                                     \
                                  TS_ASSERT(threw);                     \
                              } while (0)

using server::file::InternalDirectoryHandler;
using server::interface::FileGame;
using server::file::FileBase;

namespace {
    struct Testbench {
        InternalDirectoryHandler::Directory dir;
        server::file::DirectoryItem item;
        server::file::Root root;
        server::file::Session session;

        Testbench()
            : dir(""),
              item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("(root)", dir))),
              root(item, afl::io::InternalDirectory::create("(spec)")),
              session()
            { }
    };
}

/** Test operation on empty directories and other errors. */
void
TestServerFileFileGame::testEmpty()
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    FileGame::GameInfo gi;
    FileGame::KeyInfo ki;
    afl::container::PtrVector<FileGame::GameInfo> gis;
    afl::container::PtrVector<FileGame::KeyInfo> kis;

    // Attempt to access root (root cannot be named)
    TS_ASSERT_THROWS_CODE(testee.getGameInfo("", gi), "400");
    TS_ASSERT_THROWS_CODE(testee.listGameInfo("", gis), "400");
    TS_ASSERT_THROWS_CODE(testee.getKeyInfo("", ki), "400");
    TS_ASSERT_THROWS_CODE(testee.listKeyInfo("", FileGame::Filter(), kis), "400");

    // Create an empty directory and attempt to read it
    FileBase(tb.session, tb.root).createDirectory("x");
    TS_ASSERT_THROWS_CODE(testee.getGameInfo("x", gi), "404");
    TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("x", gis));
    TS_ASSERT_EQUALS(gis.size(), 0U);
    TS_ASSERT_THROWS_CODE(testee.getKeyInfo("x", ki), "404");
    TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("x", FileGame::Filter(), kis));
    TS_ASSERT_EQUALS(kis.size(), 0U);

    TS_ASSERT_THROWS_CODE(testee.listGameInfo("x/y/z", gis), "404");
    TS_ASSERT_THROWS_CODE(testee.listKeyInfo("x/y/z", FileGame::Filter(), kis), "404");

    // Missing permissions
    tb.session.setUser("1001");
    TS_ASSERT_THROWS_CODE(testee.getGameInfo("x", gi), "403");
    TS_ASSERT_THROWS_CODE(testee.listGameInfo("x", gis), "403");
    TS_ASSERT_THROWS_CODE(testee.getKeyInfo("x", ki), "403");
    TS_ASSERT_THROWS_CODE(testee.listKeyInfo("x", FileGame::Filter(), kis), "403");

    TS_ASSERT_THROWS_CODE(testee.listGameInfo("x/y/z", gis), "403");
    TS_ASSERT_THROWS_CODE(testee.listKeyInfo("x/y/z", FileGame::Filter(), kis), "403");
}

/** Test operation on directories that contain keys. */
void
TestServerFileFileGame::testReg()
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench
    {
        FileBase b(tb.session, tb.root);
        b.createDirectoryTree("a/b/c");
        b.createDirectoryTree("a/b/d");
        b.putFile("a/b/c/fizz.bin", afl::string::fromBytes(getDefaultRegKey()));
        b.putFile("a/b/fizz.bin", afl::string::fromBytes(getDefaultRegKey()));
        b.setDirectoryPermissions("a/b", "1001", "r");
        b.setDirectoryPermissions("a/b/c", "1002", "r");
    }

    // Single stat
    {
        FileGame::KeyInfo ki;
        TS_ASSERT_THROWS_NOTHING(testee.getKeyInfo("a/b", ki));
        TS_ASSERT_EQUALS(ki.fileName, "a/b/fizz.bin");
        TS_ASSERT_EQUALS(ki.pathName, "a/b");
        TS_ASSERT_EQUALS(ki.isRegistered, false);
    }

    // List
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b", FileGame::Filter(), kis));
        TS_ASSERT_EQUALS(kis.size(), 2U);
        TS_ASSERT_EQUALS(kis[0]->fileName, "a/b/fizz.bin");
        TS_ASSERT_EQUALS(kis[1]->fileName, "a/b/c/fizz.bin");
        TS_ASSERT_EQUALS(kis[1]->keyId.orElse(""), "611a7f755848a9605ad15d92266c0fb77161cf69");
    }

    // List with uniquisation
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.unique = 1;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b", f, kis));
        TS_ASSERT_EQUALS(kis.size(), 1U);
        TS_ASSERT_EQUALS(kis[0]->fileName, "a/b/fizz.bin");
        TS_ASSERT_EQUALS(kis[0]->useCount.orElse(-1), 2);
    }

    // List with filter (mismatch)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.keyId = "?";
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b", f, kis));
        TS_ASSERT_EQUALS(kis.size(), 0U);
    }

    // List with filter (match)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.keyId = "611a7f755848a9605ad15d92266c0fb77161cf69";
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b", f, kis));
        TS_ASSERT_EQUALS(kis.size(), 2U);
    }

    // Stat as user 1001
    tb.session.setUser("1001");
    {
        FileGame::KeyInfo ki;
        TS_ASSERT_THROWS_NOTHING(testee.getKeyInfo("a/b", ki));
        TS_ASSERT_EQUALS(ki.fileName, "a/b/fizz.bin");
        TS_ASSERT_EQUALS(ki.pathName, "a/b");
        TS_ASSERT_EQUALS(ki.isRegistered, false);

        TS_ASSERT_THROWS_CODE(testee.getKeyInfo("a/b/c", ki), "403");
    }

    // List as user 1001 (gets only available content)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b", FileGame::Filter(), kis));
        TS_ASSERT_EQUALS(kis.size(), 1U);
        TS_ASSERT_EQUALS(kis[0]->fileName, "a/b/fizz.bin");
    }

    // List as user 1002 (gets only available content)
    tb.session.setUser("1002");
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        TS_ASSERT_THROWS_CODE(testee.listKeyInfo("a/b", FileGame::Filter(), kis), "403");
        TS_ASSERT_EQUALS(kis.size(), 0U);
    }
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        TS_ASSERT_THROWS_NOTHING(testee.listKeyInfo("a/b/c", FileGame::Filter(), kis));
        TS_ASSERT_EQUALS(kis.size(), 1U);
        TS_ASSERT_EQUALS(kis[0]->fileName, "a/b/c/fizz.bin");
    }
}

void
TestServerFileFileGame::testGame()
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench [same structure as testReg]
    {
        FileBase b(tb.session, tb.root);
        b.createDirectoryTree("a/b/c");
        b.createDirectoryTree("a/b/d");
        b.putFile("a/b/c/player7.rst", afl::string::fromBytes(getResultFile35()));
        b.putFile("a/b/race.nm", afl::string::fromBytes(getDefaultRaceNames()));
        b.putFile("a/b/player7.rst", afl::string::fromBytes(getResultFile35()));
        b.setDirectoryPermissions("a/b", "1001", "r");
        b.setDirectoryPermissions("a/b/c", "1002", "r");
    }

    // Single stat
    {
        FileGame::GameInfo gi;
        TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("a/b", gi));
        TS_ASSERT_EQUALS(gi.pathName, "a/b");
        TS_ASSERT_EQUALS(gi.slots.size(), 1U);
        TS_ASSERT_EQUALS(gi.slots[0].first, 7);
    }

    // List
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("a/b", gis));
        TS_ASSERT_EQUALS(gis.size(), 2U);
        TS_ASSERT_EQUALS(gis[0]->pathName, "a/b");
        TS_ASSERT_EQUALS(gis[0]->slots.size(), 1U);
        TS_ASSERT_EQUALS(gis[0]->slots[0].first, 7);
        TS_ASSERT_EQUALS(gis[0]->slots[0].second, "The Crystal Confederation");
        TS_ASSERT_EQUALS(gis[1]->pathName, "a/b/c");
        TS_ASSERT_EQUALS(gis[1]->slots.size(), 1U);
        TS_ASSERT_EQUALS(gis[1]->slots[0].first, 7);
        TS_ASSERT_EQUALS(gis[1]->slots[0].second, "Player 7");
    }

    // Stat as user 1001
    tb.session.setUser("1001");
    {
        FileGame::GameInfo gi;
        TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("a/b", gi));
        TS_ASSERT_EQUALS(gi.pathName, "a/b");
        TS_ASSERT_EQUALS(gi.slots.size(), 1U);
        TS_ASSERT_EQUALS(gi.slots[0].first, 7);

        TS_ASSERT_THROWS_CODE(testee.getGameInfo("a/b/c", gi), "403");
    }

    // List as user 1001 (gets only available content)
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("a/b", gis));
        TS_ASSERT_EQUALS(gis.size(), 1U);
        TS_ASSERT_EQUALS(gis[0]->pathName, "a/b");
    }

    // List as user 1002 (gets only available content)
    tb.session.setUser("1002");
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        TS_ASSERT_THROWS_CODE(testee.listGameInfo("a/b", gis), "403");
        TS_ASSERT_EQUALS(gis.size(), 0U);
    }
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        TS_ASSERT_THROWS_NOTHING(testee.listGameInfo("a/b/c", gis));
        TS_ASSERT_EQUALS(gis.size(), 1U);
        TS_ASSERT_EQUALS(gis[0]->pathName, "a/b/c");
    }
}

void
TestServerFileFileGame::testGameProps()
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench
    {
        FileBase b(tb.session, tb.root);
        b.createDirectory("a");
        b.putFile("a/player7.rst", afl::string::fromBytes(getResultFile35()));
        b.setDirectoryProperty("a", "game", "42");
        b.setDirectoryProperty("a", "finished", "1");
        b.setDirectoryProperty("a", "name", "Forty Two");
        b.setDirectoryProperty("a", "hosttime", "998877");
        b.putFile("a/xyplan7.dat", "");

        b.createDirectory("b");
        b.putFile("b/player7.rst", afl::string::fromBytes(getResultFile35()));
        b.setDirectoryProperty("b", "game", "what?");
        b.setDirectoryProperty("b", "finished", "yep");
    }

    // Query a
    FileGame::GameInfo gi;
    TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("a", gi));
    TS_ASSERT_EQUALS(gi.pathName, "a");
    TS_ASSERT_EQUALS(gi.slots.size(), 1U);
    TS_ASSERT_EQUALS(gi.slots[0].first, 7);
    TS_ASSERT_EQUALS(gi.slots[0].second, "Player 7");
    TS_ASSERT_EQUALS(gi.gameName, "Forty Two");
    TS_ASSERT_EQUALS(gi.isFinished, true);
    TS_ASSERT_EQUALS(gi.gameId, 42);
    TS_ASSERT_EQUALS(gi.hostTime, 998877);
    TS_ASSERT(std::find(gi.missingFiles.begin(), gi.missingFiles.end(), "xyplan.dat") == gi.missingFiles.end());

    // Query b (which has bogus properties)
    TS_ASSERT_THROWS_NOTHING(testee.getGameInfo("b", gi));
    TS_ASSERT_EQUALS(gi.pathName, "b");
    TS_ASSERT_EQUALS(gi.slots.size(), 1U);
    TS_ASSERT_EQUALS(gi.slots[0].first, 7);
    TS_ASSERT_EQUALS(gi.slots[0].second, "Player 7");
    TS_ASSERT_EQUALS(gi.gameName, "");
    TS_ASSERT_EQUALS(gi.isFinished, false);
    TS_ASSERT_EQUALS(gi.gameId, 0);
    TS_ASSERT_EQUALS(gi.hostTime, 0);
    TS_ASSERT(std::find(gi.missingFiles.begin(), gi.missingFiles.end(), "xyplan.dat") != gi.missingFiles.end());
}
