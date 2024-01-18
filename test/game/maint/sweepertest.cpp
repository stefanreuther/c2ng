/**
  *  \file test/game/maint/sweepertest.cpp
  *  \brief Test for game::maint::Sweeper
  */

#include "game/maint/sweeper.hpp"

#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("game.maint.Sweeper:scan", a)
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

    a.check("01. getRemainingPlayers",  testee.getRemainingPlayers().contains(1));
    a.check("02. getRemainingPlayers",  testee.getRemainingPlayers().contains(2));
    a.check("03. getRemainingPlayers", !testee.getRemainingPlayers().contains(3));
    a.check("04. getRemainingPlayers", !testee.getRemainingPlayers().contains(4));

    a.check("11. getPlayers", testee.getPlayers().empty());
}

/** Test removal of files, where some files remain. */
AFL_TEST("game.maint.Sweeper:remove", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 2 + 3 + 4);
    a.check("01. getPlayers", !testee.getPlayers().contains(1));
    a.check("02. getPlayers",  testee.getPlayers().contains(2));
    a.check("03. getPlayers",  testee.getPlayers().contains(3));
    a.check("04. getPlayers",  testee.getPlayers().contains(4));
    a.check("05. getPlayers", !testee.getPlayers().contains(5));

    testee.execute(*dir);

    // Verify
    a.check("11. getRemainingPlayers",  testee.getRemainingPlayers().contains(1));
    a.check("12. getRemainingPlayers", !testee.getRemainingPlayers().contains(2));
    a.check("13. getRemainingPlayers", !testee.getRemainingPlayers().contains(3));
    a.check("14. getRemainingPlayers", !testee.getRemainingPlayers().contains(4));
    a.check("15. hasFile",  hasFile(*dir, "gen1.dat"));
    a.check("16. hasFile",  hasFile(*dir, "pdata1.dat"));
    a.check("17. hasFile", !hasFile(*dir, "gen2.dat"));
    a.check("18. hasFile",  hasFile(*dir, "vpa2.db"));
    a.check("19. hasFile", !hasFile(*dir, "pdata3.dat"));
    a.check("20. hasFile",  hasFile(*dir, "init.tmp"));
    a.check("21. hasFile", !hasFile(*dir, "temp.dat"));
    a.check("22. hasFile",  hasFile(*dir, "score.cc"));

    // Execution does not change selection
    a.check("31. getPlayers", !testee.getPlayers().contains(1));
    a.check("32. getPlayers",  testee.getPlayers().contains(2));
    a.check("33. getPlayers",  testee.getPlayers().contains(3));
    a.check("34. getPlayers",  testee.getPlayers().contains(4));
    a.check("35. getPlayers", !testee.getPlayers().contains(5));
}

/** Test removal of files, where no races remain. */
AFL_TEST("game.maint.Sweeper:remove-last", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 1 + 2);
    testee.execute(*dir);

    // Verify
    a.check("01. getRemainingPlayers", testee.getRemainingPlayers().empty());
    a.check("02. hasFile", !hasFile(*dir, "gen1.dat"));
    a.check("03. hasFile", !hasFile(*dir, "pdata1.dat"));
    a.check("04. hasFile", !hasFile(*dir, "gen2.dat"));
    a.check("05. hasFile",  hasFile(*dir, "vpa2.db"));
    a.check("06. hasFile",  hasFile(*dir, "pdata3.dat")); // file was kept but is not counted as a remaining race!
    a.check("07. hasFile", !hasFile(*dir, "init.tmp"));
    a.check("08. hasFile", !hasFile(*dir, "temp.dat"));
    a.check("09. hasFile",  hasFile(*dir, "score.cc"));
}

/** Test removal of files including database files. */
AFL_TEST("game.maint.Sweeper:remove-db", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 2 + 3 + 4);
    testee.setEraseDatabase(true);
    testee.execute(*dir);

    // Verify
    a.check("01. getRemainingPlayers",  testee.getRemainingPlayers().contains(1));
    a.check("02. getRemainingPlayers", !testee.getRemainingPlayers().contains(2));
    a.check("03. getRemainingPlayers", !testee.getRemainingPlayers().contains(3));
    a.check("04. getRemainingPlayers", !testee.getRemainingPlayers().contains(4));
    a.check("05. hasFile",  hasFile(*dir, "gen1.dat"));
    a.check("06. hasFile",  hasFile(*dir, "pdata1.dat"));
    a.check("07. hasFile", !hasFile(*dir, "gen2.dat"));
    a.check("08. hasFile", !hasFile(*dir, "vpa2.db"));
    a.check("09. hasFile", !hasFile(*dir, "pdata3.dat"));
    a.check("10. hasFile",  hasFile(*dir, "init.tmp"));
    a.check("11. hasFile", !hasFile(*dir, "temp.dat"));
    a.check("12. hasFile",  hasFile(*dir, "score.cc"));

    // Verify init.tmp; should contain player 1
    static const uint8_t EXPECTED[] = {1,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0};
    a.checkEqual("21. init.tmp file size", dir->getDirectoryEntryByName("init.tmp")->getFileSize(), 22U);
    a.check("22. init.tmp content", dir->openFile("init.tmp", afl::io::FileSystem::OpenRead)->createVirtualMapping()->get().equalContent(EXPECTED));
}

/** Test removal of files including database files, nothing remains. */
AFL_TEST("game.maint.Sweeper:remove-db-last", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    populate(*dir);

    // Execute
    game::maint::Sweeper testee;
    testee.setPlayers(game::PlayerSet_t() + 1 + 2);
    testee.setEraseDatabase(true);
    testee.execute(*dir);

    // Verify
    a.check("01. getRemainingPlayers", testee.getRemainingPlayers().empty());
    a.check("02. hasFile", !hasFile(*dir, "gen1.dat"));
    a.check("03. hasFile", !hasFile(*dir, "pdata1.dat"));
    a.check("04. hasFile", !hasFile(*dir, "gen2.dat"));
    a.check("05. hasFile", !hasFile(*dir, "vpa2.db"));
    a.check("06. hasFile",  hasFile(*dir, "pdata3.dat"));
    a.check("07. hasFile", !hasFile(*dir, "init.tmp"));
    a.check("08. hasFile", !hasFile(*dir, "temp.dat"));
    a.check("09. hasFile", !hasFile(*dir, "score.cc"));
}
