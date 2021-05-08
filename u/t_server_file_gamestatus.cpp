/**
  *  \file u/t_server_file_gamestatus.cpp
  *  \brief Test for server::file::GameStatus
  */

#include <algorithm>
#include "server/file/gamestatus.hpp"

#include "t_server_file.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/test/files.hpp"

using server::file::InternalDirectoryHandler;

namespace {
    struct Testbench {
        InternalDirectoryHandler::Directory dir;
        server::file::DirectoryItem item;
        server::file::Root root;

        Testbench()
            : dir(""),
              item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("(root)", dir))),
              root(item, afl::io::InternalDirectory::create("(spec)"))
            { }
    };
}

/** Test GameStatus on empty directory. */
void
TestServerFileGameStatus::testEmpty()
{
    Testbench tb;

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // No result expected
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    TS_ASSERT(gi == 0);
    TS_ASSERT(ki == 0);
}

/** Test GameStatus on a directory containing just a reg key. */
void
TestServerFileGameStatus::testReg()
{
    Testbench tb;
    tb.item.createFile("fizz.bin", game::test::getDefaultRegKey());

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // Verify result
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    TS_ASSERT(gi == 0);
    TS_ASSERT(ki != 0);

    TS_ASSERT_EQUALS(ki->fileName, "fizz.bin");
    TS_ASSERT_EQUALS(ki->isRegistered, false);
    TS_ASSERT_EQUALS(ki->label1, "VGA Planets shareware");
    TS_ASSERT_EQUALS(ki->label2, "Version 3.00");
}

/** Test GameStatus on a directory containing just a result. */
void
TestServerFileGameStatus::testGame()
{
    Testbench tb;
    tb.item.createFile("player7.rst", game::test::getResultFile35());

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // Verify result
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    TS_ASSERT(gi != 0);
    TS_ASSERT(ki == 0);

    TS_ASSERT_EQUALS(gi->slots.size(), 1U);
    TS_ASSERT_EQUALS(gi->slots[0].first, 7);
    TS_ASSERT_EQUALS(gi->slots[0].second, "Player 7");  // default because we have no race names
    TS_ASSERT(std::find(gi->missingFiles.begin(), gi->missingFiles.end(), "xyplan.dat") != gi->missingFiles.end());
}

/** Test GameStatus on a directory containing result, key, and race names. */
void
TestServerFileGameStatus::testBoth()
{
    Testbench tb;
    tb.item.createFile("fizz.bin", game::test::getDefaultRegKey());
    tb.item.createFile("player7.rst", game::test::getResultFile35());
    tb.item.createFile("race.nm", game::test::getDefaultRaceNames());
    tb.item.createFile("xyplan7.dat", afl::base::Nothing);

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // Verify result
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    TS_ASSERT(gi != 0);
    TS_ASSERT(ki != 0);

    TS_ASSERT_EQUALS(ki->fileName, "fizz.bin");
    TS_ASSERT_EQUALS(ki->isRegistered, false);
    TS_ASSERT_EQUALS(ki->label1, "VGA Planets shareware");
    TS_ASSERT_EQUALS(ki->label2, "Version 3.00");

    TS_ASSERT_EQUALS(gi->slots.size(), 1U);
    TS_ASSERT_EQUALS(gi->slots[0].first, 7);
    TS_ASSERT_EQUALS(gi->slots[0].second, "The Crystal Confederation");
    TS_ASSERT(std::find(gi->missingFiles.begin(), gi->missingFiles.end(), "xyplan.dat") == gi->missingFiles.end());
}

/** Test GameStatus on a directory containing an invalid result. */
void
TestServerFileGameStatus::testInvalidResult()
{
    // Misattributed file will not be recognized
    {
        Testbench tb;
        tb.item.createFile("player3.rst", game::test::getResultFile35());

        server::file::GameStatus testee;
        testee.load(tb.root, tb.item);

        TS_ASSERT(testee.getGameInfo() == 0);
        TS_ASSERT(testee.getKeyInfo() == 0);
    }

    // Truncated file will not be recognized
    {
        Testbench tb;
        tb.item.createFile("player7.rst", game::test::getResultFile35().subrange(0, 1000));

        server::file::GameStatus testee;
        testee.load(tb.root, tb.item);

        TS_ASSERT(testee.getGameInfo() == 0);
        TS_ASSERT(testee.getKeyInfo() == 0);
    }

    // Empty file will not be recognized
    {
        Testbench tb;
        tb.item.createFile("player7.rst", afl::base::Nothing);

        server::file::GameStatus testee;
        testee.load(tb.root, tb.item);

        TS_ASSERT(testee.getGameInfo() == 0);
        TS_ASSERT(testee.getKeyInfo() == 0);
    }
}

/** Test GameStatus on a directory containing an invalid key. */
void
TestServerFileGameStatus::testInvalidKey()
{
    // Presence of a file will cause KeyInfo to be set.
    // The key will be unregistered.
    Testbench tb;
    tb.item.createFile("fizz.bin", afl::base::Nothing);

    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    TS_ASSERT(gi == 0);
    TS_ASSERT(ki != 0);

    TS_ASSERT_EQUALS(ki->fileName, "fizz.bin");
    TS_ASSERT_EQUALS(ki->isRegistered, false);
    TS_ASSERT_EQUALS(ki->label1, "VGA Planets shareware");
    // label2 will be set to a program name
}
