/**
  *  \file test/server/file/filegametest.cpp
  *  \brief Test for server::file::FileGame
  */

#include "server/file/filegame.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/filebase.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include <algorithm>

#define AFL_CHECK_THROWS_CODE(a, call, code)                            \
                              do {                                      \
                                  bool threw = false;                   \
                                  try {                                 \
                                      call;                             \
                                  }                                     \
                                  catch (std::exception& e) {           \
                                      a.checkEqual("01. what", String_t(e.what()).substr(0, 3), code); \
                                      threw = true;                     \
                                  }                                     \
                                  catch (...) {                         \
                                      a.fail("02. Wrong exception");    \
                                      threw = true;                     \
                                  }                                     \
                                  a.check("03. threw", threw);          \
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
AFL_TEST("server.file.FileGame:empty", a)
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    FileGame::GameInfo gi;
    FileGame::KeyInfo ki;
    afl::container::PtrVector<FileGame::GameInfo> gis;
    afl::container::PtrVector<FileGame::KeyInfo> kis;

    // Attempt to access root (root cannot be named)
    AFL_CHECK_THROWS_CODE(a("01. getGameInfo"),  testee.getGameInfo("", gi), "400");
    AFL_CHECK_THROWS_CODE(a("02. listGameInfo"), testee.listGameInfo("", gis), "400");
    AFL_CHECK_THROWS_CODE(a("03. getKeyInfo"),   testee.getKeyInfo("", ki), "400");
    AFL_CHECK_THROWS_CODE(a("04. listKeyInfo"),  testee.listKeyInfo("", FileGame::Filter(), kis), "400");

    // Create an empty directory and attempt to read it
    FileBase(tb.session, tb.root).createDirectory("x");
    AFL_CHECK_THROWS_CODE(a("11. getGameInfo"),  testee.getGameInfo("x", gi), "404");
    AFL_CHECK_SUCCEEDS(a   ("12. listGameInfo"), testee.listGameInfo("x", gis));
    a.checkEqual           ("13. size",          gis.size(), 0U);
    AFL_CHECK_THROWS_CODE(a("14. getKeyInfo"),   testee.getKeyInfo("x", ki), "404");
    AFL_CHECK_SUCCEEDS(a   ("15. listKeyInfo"),  testee.listKeyInfo("x", FileGame::Filter(), kis));
    a.checkEqual           ("16. size",          kis.size(), 0U);

    AFL_CHECK_THROWS_CODE(a("21. listGameInfo"), testee.listGameInfo("x/y/z", gis), "404");
    AFL_CHECK_THROWS_CODE(a("22. listKeyInfo"),  testee.listKeyInfo("x/y/z", FileGame::Filter(), kis), "404");

    // Missing permissions
    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("31. getGameInfo"),  testee.getGameInfo("x", gi), "403");
    AFL_CHECK_THROWS_CODE(a("32. listGameInfo"), testee.listGameInfo("x", gis), "403");
    AFL_CHECK_THROWS_CODE(a("33. getKeyInfo"),   testee.getKeyInfo("x", ki), "403");
    AFL_CHECK_THROWS_CODE(a("34. listKeyInfo"),  testee.listKeyInfo("x", FileGame::Filter(), kis), "403");

    AFL_CHECK_THROWS_CODE(a("41. listGameInfo"), testee.listGameInfo("x/y/z", gis), "403");
    AFL_CHECK_THROWS_CODE(a("42. listKeyInfo"),  testee.listKeyInfo("x/y/z", FileGame::Filter(), kis), "403");
}

/** Test operation on directories that contain keys. */
AFL_TEST("server.file.FileGame:keys", a)
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench
    {
        FileBase b(tb.session, tb.root);
        b.createDirectoryTree("a/b/c");
        b.createDirectoryTree("a/b/d");
        b.putFile("a/b/c/fizz.bin", afl::string::fromBytes(game::test::getDefaultRegKey()));
        b.putFile("a/b/fizz.bin", afl::string::fromBytes(game::test::getDefaultRegKey()));
        b.setDirectoryPermissions("a/b", "1001", "r");
        b.setDirectoryPermissions("a/b/c", "1002", "r");
    }

    // Single stat
    {
        FileGame::KeyInfo ki;
        AFL_CHECK_SUCCEEDS(a("01. getKeyInfo"), testee.getKeyInfo("a/b", ki));
        a.checkEqual("02. fileName", ki.fileName, "a/b/fizz.bin");
        a.checkEqual("03. pathName", ki.pathName, "a/b");
        a.checkEqual("04. isRegistered", ki.isRegistered, false);
    }

    // List
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        AFL_CHECK_SUCCEEDS(a("11. listKeyInfo"), testee.listKeyInfo("a/b", FileGame::Filter(), kis));
        a.checkEqual("12. size", kis.size(), 2U);
        a.checkEqual("13", kis[0]->fileName, "a/b/fizz.bin");
        a.checkEqual("14", kis[1]->fileName, "a/b/c/fizz.bin");
        a.checkEqual("15", kis[1]->keyId.orElse(""), "611a7f755848a9605ad15d92266c0fb77161cf69");
    }

    // List with uniquisation
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.unique = 1;
        AFL_CHECK_SUCCEEDS(a("21. listKeyInfo"), testee.listKeyInfo("a/b", f, kis));
        a.checkEqual("22. size", kis.size(), 1U);
        a.checkEqual("23. fileName", kis[0]->fileName, "a/b/fizz.bin");
        a.checkEqual("24. useCount", kis[0]->useCount.orElse(-1), 2);
    }

    // List with filter (mismatch)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.keyId = "?";
        AFL_CHECK_SUCCEEDS(a("31. listKeyInfo"), testee.listKeyInfo("a/b", f, kis));
        a.checkEqual("32. size", kis.size(), 0U);
    }

    // List with filter (match)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        FileGame::Filter f;
        f.keyId = "611a7f755848a9605ad15d92266c0fb77161cf69";
        AFL_CHECK_SUCCEEDS(a("41. listKeyInfo"), testee.listKeyInfo("a/b", f, kis));
        a.checkEqual("42. size", kis.size(), 2U);
    }

    // Stat as user 1001
    tb.session.setUser("1001");
    {
        FileGame::KeyInfo ki;
        AFL_CHECK_SUCCEEDS(a("51. getKeyInfo"), testee.getKeyInfo("a/b", ki));
        a.checkEqual("52. fileName", ki.fileName, "a/b/fizz.bin");
        a.checkEqual("53. pathName", ki.pathName, "a/b");
        a.checkEqual("54. isRegistered", ki.isRegistered, false);

        AFL_CHECK_THROWS_CODE(a("55. getKeyInfo"), testee.getKeyInfo("a/b/c", ki), "403");
    }

    // List as user 1001 (gets only available content)
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        AFL_CHECK_SUCCEEDS(a("61. listKeyInfo"), testee.listKeyInfo("a/b", FileGame::Filter(), kis));
        a.checkEqual("62. size", kis.size(), 1U);
        a.checkEqual("63. fileName", kis[0]->fileName, "a/b/fizz.bin");
    }

    // List as user 1002 (gets only available content)
    tb.session.setUser("1002");
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        AFL_CHECK_THROWS_CODE(a("71. listKeyInfo"), testee.listKeyInfo("a/b", FileGame::Filter(), kis), "403");
        a.checkEqual("72. size", kis.size(), 0U);
    }
    {
        afl::container::PtrVector<FileGame::KeyInfo> kis;
        AFL_CHECK_SUCCEEDS(a("73. listKeyInfo"), testee.listKeyInfo("a/b/c", FileGame::Filter(), kis));
        a.checkEqual("73. size", kis.size(), 1U);
        a.checkEqual("74. fileName", kis[0]->fileName, "a/b/c/fizz.bin");
    }
}

AFL_TEST("server.file.FileGame:game-info", a)
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench [same structure as testReg]
    {
        FileBase b(tb.session, tb.root);
        b.createDirectoryTree("a/b/c");
        b.createDirectoryTree("a/b/d");
        b.putFile("a/b/c/player7.rst", afl::string::fromBytes(game::test::getResultFile35()));
        b.putFile("a/b/race.nm", afl::string::fromBytes(game::test::getDefaultRaceNames()));
        b.putFile("a/b/player7.rst", afl::string::fromBytes(game::test::getResultFile35()));
        b.setDirectoryPermissions("a/b", "1001", "r");
        b.setDirectoryPermissions("a/b/c", "1002", "r");
    }

    // Single stat
    {
        FileGame::GameInfo gi;
        AFL_CHECK_SUCCEEDS(a("01. getGameInfo"), testee.getGameInfo("a/b", gi));
        a.checkEqual("02. pathName", gi.pathName, "a/b");
        a.checkEqual("03. size",     gi.slots.size(), 1U);
        a.checkEqual("04. first",    gi.slots[0].first, 7);
    }

    // List
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        AFL_CHECK_SUCCEEDS(a("11. listGameInfo"), testee.listGameInfo("a/b", gis));
        a.checkEqual("12. size",       gis.size(), 2U);
        a.checkEqual("13. pathName",   gis[0]->pathName, "a/b");
        a.checkEqual("14. slots size", gis[0]->slots.size(), 1U);
        a.checkEqual("15. first",      gis[0]->slots[0].first, 7);
        a.checkEqual("16. second",     gis[0]->slots[0].second, "The Crystal Confederation");
        a.checkEqual("17. pathName",   gis[1]->pathName, "a/b/c");
        a.checkEqual("18. slots size", gis[1]->slots.size(), 1U);
        a.checkEqual("19. first",      gis[1]->slots[0].first, 7);
        a.checkEqual("20. second",     gis[1]->slots[0].second, "Player 7");
    }

    // Stat as user 1001
    tb.session.setUser("1001");
    {
        FileGame::GameInfo gi;
        AFL_CHECK_SUCCEEDS(a("21. getGameInfo"), testee.getGameInfo("a/b", gi));
        a.checkEqual("22. pathName", gi.pathName, "a/b");
        a.checkEqual("23. size", gi.slots.size(), 1U);
        a.checkEqual("24. first", gi.slots[0].first, 7);

        AFL_CHECK_THROWS_CODE(a("25. getGameInfo"), testee.getGameInfo("a/b/c", gi), "403");
    }

    // List as user 1001 (gets only available content)
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        AFL_CHECK_SUCCEEDS(a("31. listGameInfo"), testee.listGameInfo("a/b", gis));
        a.checkEqual("32. size",     gis.size(), 1U);
        a.checkEqual("33. pathName", gis[0]->pathName, "a/b");
    }

    // List as user 1002 (gets only available content)
    tb.session.setUser("1002");
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        AFL_CHECK_THROWS_CODE(a("41. listGameInfo"), testee.listGameInfo("a/b", gis), "403");
        a.checkEqual("42. size", gis.size(), 0U);
    }
    {
        afl::container::PtrVector<FileGame::GameInfo> gis;
        AFL_CHECK_SUCCEEDS(a("51. listGameInfo"), testee.listGameInfo("a/b/c", gis));
        a.checkEqual("52. size", gis.size(), 1U);
        a.checkEqual("53. pathName", gis[0]->pathName, "a/b/c");
    }
}

AFL_TEST("server.file.FileGame:game-properties", a)
{
    Testbench tb;
    server::file::FileGame testee(tb.session, tb.root);

    // Prepare the test bench
    {
        FileBase b(tb.session, tb.root);
        b.createDirectory("a");
        b.putFile("a/player7.rst", afl::string::fromBytes(game::test::getResultFile35()));
        b.setDirectoryProperty("a", "game", "42");
        b.setDirectoryProperty("a", "finished", "1");
        b.setDirectoryProperty("a", "name", "Forty Two");
        b.setDirectoryProperty("a", "hosttime", "998877");
        b.putFile("a/xyplan7.dat", "");

        b.createDirectory("b");
        b.putFile("b/player7.rst", afl::string::fromBytes(game::test::getResultFile35()));
        b.setDirectoryProperty("b", "game", "what?");
        b.setDirectoryProperty("b", "finished", "yep");
    }

    // Query a
    FileGame::GameInfo gi;
    AFL_CHECK_SUCCEEDS(a("01. getGameInfo"), testee.getGameInfo("a", gi));
    a.checkEqual("02. pathName",   gi.pathName, "a");
    a.checkEqual("03. size",       gi.slots.size(), 1U);
    a.checkEqual("04. first",      gi.slots[0].first, 7);
    a.checkEqual("05. second",     gi.slots[0].second, "Player 7");
    a.checkEqual("06. gameName",   gi.gameName, "Forty Two");
    a.checkEqual("07. isFinished", gi.isFinished, true);
    a.checkEqual("08. gameId",     gi.gameId, 42);
    a.checkEqual("09. hostTime",   gi.hostTime, 998877);
    a.check("10. missingFiles", std::find(gi.missingFiles.begin(), gi.missingFiles.end(), "xyplan.dat") == gi.missingFiles.end());

    // Query b (which has bogus properties)
    AFL_CHECK_SUCCEEDS(a("11. getGameInfo"), testee.getGameInfo("b", gi));
    a.checkEqual("12. pathName",   gi.pathName, "b");
    a.checkEqual("13. size",       gi.slots.size(), 1U);
    a.checkEqual("14. first",      gi.slots[0].first, 7);
    a.checkEqual("15. second",     gi.slots[0].second, "Player 7");
    a.checkEqual("16. gameName",   gi.gameName, "");
    a.checkEqual("17. isFinished", gi.isFinished, false);
    a.checkEqual("18. gameId",     gi.gameId, 0);
    a.checkEqual("19. hostTime",   gi.hostTime, 0);
    a.check("20. missingFiles", std::find(gi.missingFiles.begin(), gi.missingFiles.end(), "xyplan.dat") != gi.missingFiles.end());
}
