/**
  *  \file u/t_game_maint_sweeper.cpp
  *  \brief Test for game::maint::Sweeper
  */

#include "game/maint/sweeper.hpp"

#include "t_game_maint.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/directoryentry.hpp"

namespace {
    void createFile(afl::io::Directory& dir, const char* name)
    {
        dir.openFile(name, afl::io::FileSystem::Create);
    }

    bool hasFile(afl::io::Directory& dir, const char* name)
    {
        return dir.openFileNT(name, afl::io::FileSystem::OpenRead).get() != 0;
    }

    void populate(afl::io::Directory& dir)
    {
        // player 1
        createFile(dir, "gen1.dat");
        createFile(dir, "pdata1.dat");

        // player 2
        createFile(dir, "gen2.dat");
        createFile(dir, "vpa2.db");

        // player 3 (not counted, no gen3.dat)
        createFile(dir, "pdata3.dat");

        createFile(dir, "init.tmp");
        createFile(dir, "temp.dat");
        createFile(dir, "score.cc");
    }
}

/** Test scan(). */
void
TestGameMaintSweeper::testScan()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");

    // player 1
    createFile(*dir, "gen1.dat");
    createFile(*dir, "pdata1.dat");

    // player 2
    createFile(*dir, "gen2.dat");

    // player 3 (not counted, no gen3.dat)
    createFile(*dir, "pdata3.dat");

    game::maint::Sweeper testee;
    testee.scan(*dir);

    TS_ASSERT(testee.getRemainingPlayers().contains(1));
    TS_ASSERT(testee.getRemainingPlayers().contains(2));
    TS_ASSERT(!testee.getRemainingPlayers().contains(3));
    TS_ASSERT(!testee.getRemainingPlayers().contains(4));

    TS_ASSERT(testee.getPlayers().empty());
}

/** Test removal of files, where some files remain. */
void
TestGameMaintSweeper::testRemove()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 2 + 3 + 4);
    TS_ASSERT(!testee.getPlayers().contains(1));
    TS_ASSERT(testee.getPlayers().contains(2));
    TS_ASSERT(testee.getPlayers().contains(3));
    TS_ASSERT(testee.getPlayers().contains(4));
    TS_ASSERT(!testee.getPlayers().contains(5));

    testee.execute(*dir);

    // Verify
    TS_ASSERT(testee.getRemainingPlayers().contains(1));
    TS_ASSERT(!testee.getRemainingPlayers().contains(2));
    TS_ASSERT(!testee.getRemainingPlayers().contains(3));
    TS_ASSERT(!testee.getRemainingPlayers().contains(4));
    TS_ASSERT(hasFile(*dir, "gen1.dat"));
    TS_ASSERT(hasFile(*dir, "pdata1.dat"));
    TS_ASSERT(!hasFile(*dir, "gen2.dat"));
    TS_ASSERT(hasFile(*dir, "vpa2.db"));
    TS_ASSERT(!hasFile(*dir, "pdata3.dat"));
    TS_ASSERT(hasFile(*dir, "init.tmp"));
    TS_ASSERT(!hasFile(*dir, "temp.dat"));
    TS_ASSERT(hasFile(*dir, "score.cc"));

    // Execution does not change selection
    TS_ASSERT(!testee.getPlayers().contains(1));
    TS_ASSERT(testee.getPlayers().contains(2));
    TS_ASSERT(testee.getPlayers().contains(3));
    TS_ASSERT(testee.getPlayers().contains(4));
    TS_ASSERT(!testee.getPlayers().contains(5));
}

/** Test removal of files, where no races remain. */
void
TestGameMaintSweeper::testRemoveLast()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 1 + 2);
    testee.execute(*dir);

    // Verify
    TS_ASSERT(testee.getRemainingPlayers().empty());
    TS_ASSERT(!hasFile(*dir, "gen1.dat"));
    TS_ASSERT(!hasFile(*dir, "pdata1.dat"));
    TS_ASSERT(!hasFile(*dir, "gen2.dat"));
    TS_ASSERT(hasFile(*dir, "vpa2.db"));
    TS_ASSERT(hasFile(*dir, "pdata3.dat")); // file was kept but is not counted as a remaining race!
    TS_ASSERT(!hasFile(*dir, "init.tmp"));
    TS_ASSERT(!hasFile(*dir, "temp.dat"));
    TS_ASSERT(hasFile(*dir, "score.cc"));
}

/** Test removal of files including database files. */
void
TestGameMaintSweeper::testRemoveDB()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 2 + 3 + 4);
    testee.setEraseDatabase(true);
    testee.execute(*dir);

    // Verify
    TS_ASSERT(testee.getRemainingPlayers().contains(1));
    TS_ASSERT(!testee.getRemainingPlayers().contains(2));
    TS_ASSERT(!testee.getRemainingPlayers().contains(3));
    TS_ASSERT(!testee.getRemainingPlayers().contains(4));
    TS_ASSERT(hasFile(*dir, "gen1.dat"));
    TS_ASSERT(hasFile(*dir, "pdata1.dat"));
    TS_ASSERT(!hasFile(*dir, "gen2.dat"));
    TS_ASSERT(!hasFile(*dir, "vpa2.db"));
    TS_ASSERT(!hasFile(*dir, "pdata3.dat"));
    TS_ASSERT(hasFile(*dir, "init.tmp"));
    TS_ASSERT(!hasFile(*dir, "temp.dat"));
    TS_ASSERT(hasFile(*dir, "score.cc"));

    // Verify init.tmp; should contain player 1
    static const uint8_t EXPECTED[] = {1,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0};
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("init.tmp")->getFileSize(), 22U);
    TS_ASSERT(dir->openFile("init.tmp", afl::io::FileSystem::OpenRead)->createVirtualMapping()->get().equalContent(EXPECTED));
}

/** Test removal of files including database files, nothing remains. */
void
TestGameMaintSweeper::testRemoveDBLast()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 1 + 2);
    testee.setEraseDatabase(true);
    testee.execute(*dir);

    // Verify
    TS_ASSERT(testee.getRemainingPlayers().empty());
    TS_ASSERT(!hasFile(*dir, "gen1.dat"));
    TS_ASSERT(!hasFile(*dir, "pdata1.dat"));
    TS_ASSERT(!hasFile(*dir, "gen2.dat"));
    TS_ASSERT(!hasFile(*dir, "vpa2.db"));
    TS_ASSERT(hasFile(*dir, "pdata3.dat"));
    TS_ASSERT(!hasFile(*dir, "init.tmp"));
    TS_ASSERT(!hasFile(*dir, "temp.dat"));
    TS_ASSERT(!hasFile(*dir, "score.cc"));
}

