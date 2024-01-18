/**
  *  \file test/server/file/gamestatustest.cpp
  *  \brief Test for server::file::GameStatus
  */

#include "server/file/gamestatus.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include <algorithm>

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
AFL_TEST("server.file.GameStatus:empty", a)
{
    Testbench tb;

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // No result expected
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    a.checkNull("01. getKeyInfo", gi);
    a.checkNull("02. getGameInfo", ki);
}

/** Test GameStatus on a directory containing just a reg key. */
AFL_TEST("server.file.GameStatus:key", a)
{
    Testbench tb;
    tb.item.createFile("fizz.bin", game::test::getDefaultRegKey());

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // Verify result
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    a.checkNull("01. getGameInfo", gi);
    a.checkNonNull("02. getKeyInfo", ki);

    a.checkEqual("11. fileName",     ki->fileName, "fizz.bin");
    a.checkEqual("12. isRegistered", ki->isRegistered, false);
    a.checkEqual("13. label1",       ki->label1, "VGA Planets shareware");
    a.checkEqual("14. label2",       ki->label2, "Version 3.00");
}

/** Test GameStatus on a directory containing just a result. */
AFL_TEST("server.file.GameStatus:rst", a)
{
    Testbench tb;
    tb.item.createFile("player7.rst", game::test::getResultFile35());

    // Do it
    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    // Verify result
    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    a.checkNonNull("01. getGameInfo", gi);
    a.checkNull("02. getKeyInfo", ki);

    a.checkEqual("11. size",   gi->slots.size(), 1U);
    a.checkEqual("12. first",  gi->slots[0].first, 7);
    a.checkEqual("13. second", gi->slots[0].second, "Player 7");  // default because we have no race names
    a.check("14. missingFiles", std::find(gi->missingFiles.begin(), gi->missingFiles.end(), "xyplan.dat") != gi->missingFiles.end());
}

/** Test GameStatus on a directory containing result, key, and race names. */
AFL_TEST("server.file.GameStatus:rst+key+names", a)
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

    a.checkNonNull("01. getGameInfo", gi);
    a.checkNonNull("02. getKeyInfo", ki);

    a.checkEqual("11. fileName",     ki->fileName, "fizz.bin");
    a.checkEqual("12. isRegistered", ki->isRegistered, false);
    a.checkEqual("13. label1",       ki->label1, "VGA Planets shareware");
    a.checkEqual("14. label2",       ki->label2, "Version 3.00");

    a.checkEqual("21. size",   gi->slots.size(), 1U);
    a.checkEqual("22. first",  gi->slots[0].first, 7);
    a.checkEqual("23. second", gi->slots[0].second, "The Crystal Confederation");
    a.check("24. missingFiles", std::find(gi->missingFiles.begin(), gi->missingFiles.end(), "xyplan.dat") == gi->missingFiles.end());
}

/*
 *  Test GameStatus on a directory containing an invalid result.
 */

// Misattributed file will not be recognized
AFL_TEST("server.file.GameStatus:invalid-rst:misattributed", a)
{
    Testbench tb;
    tb.item.createFile("player3.rst", game::test::getResultFile35());

    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    a.checkNull("getGameInfo", testee.getGameInfo());
    a.checkNull("getKeyInfo", testee.getKeyInfo());
}

// Truncated file will not be recognized
AFL_TEST("server.file.GameStatus:invalid-rst:truncated", a)
{
    Testbench tb;
    tb.item.createFile("player7.rst", game::test::getResultFile35().subrange(0, 1000));

    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    a.checkNull("getGameInfo", testee.getGameInfo());
    a.checkNull("getKeyInfo", testee.getKeyInfo());
}

// Empty file will not be recognized
AFL_TEST("server.file.GameStatus:invalid-rst:empty", a)
{
    Testbench tb;
    tb.item.createFile("player7.rst", afl::base::Nothing);

    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    a.checkNull("getGameInfo", testee.getGameInfo());
    a.checkNull("getKeyInfo", testee.getKeyInfo());
}


/** Test GameStatus on a directory containing an invalid key. */
AFL_TEST("server.file.GameStatus:invalid-key", a)
{
    // Presence of a file will cause KeyInfo to be set.
    // The key will be unregistered.
    Testbench tb;
    tb.item.createFile("fizz.bin", afl::base::Nothing);

    server::file::GameStatus testee;
    testee.load(tb.root, tb.item);

    const server::file::GameStatus::KeyInfo* ki = testee.getKeyInfo();
    const server::file::GameStatus::GameInfo* gi = testee.getGameInfo();

    a.checkNull("01. getGameInfo", gi);
    a.checkNonNull("02. getKeyInfo", ki);

    a.checkEqual("11. fileName",     ki->fileName, "fizz.bin");
    a.checkEqual("12. isRegistered", ki->isRegistered, false);
    a.checkEqual("13. label1",       ki->label1, "VGA Planets shareware");
    // label2 will be set to a program name
}
