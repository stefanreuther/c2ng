/**
  *  \file test/server/file/fileshareimpltest.cpp
  *  \brief Test for server::file::FileShareImpl
  */

#include "server/file/fileshareimpl.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/interface/filegame.hpp"
#include "server/interface/fileshare.hpp"

using afl::container::PtrVector;
using server::file::FileShareImpl;
using server::file::InternalDirectoryHandler;
using server::file::Session;
using server::interface::FileGame;
using server::interface::FileShare;

namespace {
    struct Testbench {
        InternalDirectoryHandler::Directory dir;
        server::file::DirectoryItem item;
        server::file::Root root;
        Session adminSession;
        Session userSession;

        Testbench()
            : dir(""),
              item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("(root)", dir))),
              root(item, afl::io::InternalDirectory::create("(spec)")),
              adminSession(),
              userSession()
            {
                userSession.setUser("a");
            }
    };
}

/** Test all commands in a simple setup. */
AFL_TEST("server.file.FileShareImpl:basics", a)
{
    Testbench h;
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));
    shareDir->files.pushBackNew(new InternalDirectoryHandler::File("u42"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/g\nESEQ=23\nEOWNER=u10\n"));

    h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("u"))
        ->subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("g"));

    FileShareImpl testee(h.adminSession, h.root);

    // getSequenceNumber
    a.checkEqual("01. seq", testee.getSequenceNumber("u42"), 77);
    a.checkEqual("02. seq", testee.getSequenceNumber("u99"), 0);

    // listDirectories
    {
        PtrVector<FileShare::Info> shareList;
        testee.listDirectories("u42", shareList);
        a.checkEqual("11. size", shareList.size(), 1U);
        a.checkEqual("12. path", shareList[0]->pathName, "u/g");
        a.checkEqual("13. user", shareList[0]->owner, "u10");
        a.checkEqual("14. seq#", shareList[0]->seqNr, 23);
    }
    {
        PtrVector<FileShare::Info> shareList;
        testee.listDirectories("u99", shareList);
        a.checkEqual("21. size", shareList.size(), 0U);
    }

    // listGameInfo
    {
        PtrVector<FileGame::GameInfo> gameList;
        testee.listGameInfo("u42", gameList);
        a.checkEqual("31. size", gameList.size(), 0U);
    }
    {
        PtrVector<FileGame::GameInfo> gameList;
        testee.listGameInfo("u99", gameList);
        a.checkEqual("41. size", gameList.size(), 0U);
    }
}

/** listGameInfo with an actual game. */
AFL_TEST("server.file.FileShareImpl:listGameInfo", a)
{
    Testbench h;
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));
    shareDir->files.pushBackNew(new InternalDirectoryHandler::File("u42"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/g\nESEQ=23\nEOWNER=u10\n"));

    InternalDirectoryHandler::Directory* gameDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("u"))
        ->subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("g"));
    gameDir->files.pushBackNew(new InternalDirectoryHandler::File("player7.rst"))
        ->content.append(game::test::getResultFile35());
    gameDir->files.pushBackNew(new InternalDirectoryHandler::File("race.nm"))
        ->content.append(game::test::getDefaultRaceNames());
    gameDir->files.pushBackNew(new InternalDirectoryHandler::File("fizz.bin"))
        ->content.append(game::test::getDefaultRegKey());
    gameDir->files.pushBackNew(new InternalDirectoryHandler::File(".c2file"))
        ->content.append(afl::string::toBytes("owner=u10\n"));

    FileShareImpl testee(h.adminSession, h.root);

    // Test listGameInfo
    PtrVector<FileGame::GameInfo> gameList;
    testee.listGameInfo("u42", gameList);
    a.checkEqual("01. size", gameList.size(), 1U);
    a.checkEqual("02. path", gameList[0]->pathName, "u/g");
    a.checkEqual("03. slot", gameList[0]->slots.size(), 1U);
    a.checkEqual("04. race", gameList[0]->slots[0].first, 7);
    a.checkEqual("05. name", gameList[0]->slots[0].second, "The Crystal Confederation");
    a.checkEqual("06. user", gameList[0]->userId, "u10");
}

/** listGameInfo with a bad link. */
AFL_TEST("server.file.FileShareImpl:listGameInfo:bad-link", a)
{
    Testbench h;
    InternalDirectoryHandler::Directory* shareDir = h.dir.subdirectories.pushBackNew(new InternalDirectoryHandler::Directory("_share"));
    shareDir->files.pushBackNew(new InternalDirectoryHandler::File("u42"))
        ->content.append(afl::string::toBytes("SEQ=77\nENTRY=u/g\nESEQ=23\nEOWNER=u10\n"));
    // Do NOT create "u/g"

    FileShareImpl testee(h.adminSession, h.root);

    // Test listGameInfo - must return empty list
    PtrVector<FileGame::GameInfo> gameList;
    testee.listGameInfo("u42", gameList);
    a.checkEqual("01. size", gameList.size(), 0U);
}

/** Test permission handling. */
AFL_TEST("server.file.FileShareImpl:permissions", a)
{
    Testbench h;

    // Admin can list all
    AFL_CHECK_SUCCEEDS(a("admin/a"), FileShareImpl(h.adminSession, h.root).getSequenceNumber("a"));
    AFL_CHECK_SUCCEEDS(a("admin/b"), FileShareImpl(h.adminSession, h.root).getSequenceNumber("b"));

    // User can only list their own
    AFL_CHECK_SUCCEEDS(a("user/a"), FileShareImpl(h.userSession, h.root).getSequenceNumber("a"));
    AFL_CHECK_THROWS  (a("user/b"), FileShareImpl(h.userSession, h.root).getSequenceNumber("b"), std::runtime_error);
}
