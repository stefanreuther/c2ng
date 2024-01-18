/**
  *  \file test/server/host/file/gamerootitemtest.cpp
  *  \brief Test for server::host::file::GameRootItem
  */

#include "server/host/file/gamerootitem.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/gamecreator.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"
#include <stdexcept>

namespace {
    const int TURN_NUMBER = 30;

    /* Create a user (for the purposes of this test), given a user Id */
    void createUser(afl::net::CommandHandler& db, String_t userName)
    {
        afl::net::redis::Subtree user(db, "user:");
        user.stringSetKey("all").add(userName);
        user.subtree(userName).stringKey("name").set(userName);
        user.subtree(userName).hashKey("profile").stringField("screenname").set(userName);
    }

    /* Create a game. Returns the Id. */
    int32_t createGame(server::host::Root& root)
    {
        server::host::GameCreator crea(root);
        int32_t id = crea.createNewGame();
        crea.initializeGame(id);
        crea.finishNewGame(id, server::interface::HostGame::Finished, server::interface::HostGame::PublicGame);
        return id;
    }

    /* Populate the game history.
       Creates all files and historical records. */
    void createGameHistory(server::host::Game& game, server::host::Root& root)
    {
        server::interface::FileBaseClient hfClient(root.hostFile());

        // Create game history
        String_t gameDir = game.getDirectory();
        for (int turn = 1; turn <= TURN_NUMBER; ++turn) {
            // Files
            hfClient.createDirectoryTree(afl::string::Format("%s/backup/pre-%03d", gameDir, turn));
            hfClient.createDirectoryTree(afl::string::Format("%s/backup/post-%03d", gameDir, turn));
            hfClient.createDirectoryTree(afl::string::Format("%s/backup/trn-%03d", gameDir, turn));
            for (int slot = 1; slot <= 5; ++slot) {
                if (turn > 1) {
                    hfClient.putFile(afl::string::Format("%s/backup/trn-%03d/player%d.trn", gameDir, turn, slot), afl::string::Format("turn-%d-%d", turn, slot));
                    hfClient.putFile(afl::string::Format("%s/backup/pre-%03d/player%d.rst", gameDir, turn, slot), afl::string::Format("pre-%d-%d", turn, slot));
                }
                hfClient.putFile(afl::string::Format("%s/backup/post-%03d/player%d.rst", gameDir, turn, slot), afl::string::Format("post-%d-%d", turn, slot));
            }
            if (turn > 1) {
                hfClient.putFile(afl::string::Format("%s/backup/pre-%03d/race.nm", gameDir, turn), afl::string::Format("pre-spec-%d", turn));
            }
            hfClient.putFile(afl::string::Format("%s/backup/post-%03d/race.nm", gameDir, turn), afl::string::Format("post-spec-%d", turn));

            // Database
            server::host::Game::Turn t = game.turn(turn);
            t.scores().stringField("timscore").set(String_t(22, '\1'));
            t.info().time().set(1000 + turn);
            t.info().timestamp().set(afl::string::Format("01-01-200019:20:%02d", turn));
            t.info().turnStatus().set(String_t("\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0\1\0", 44));
            if (turn >= 10) {
                // Pretend that recordings start at turn 10
                t.files().globalFiles().add("race.nm");
                for (int slot = 1; slot <= 5; ++slot) {
                    t.files().playerFiles(slot).add(afl::string::Format("player%d.rst", slot));
                }
            }
        }

        // Current turn
        hfClient.putFile(afl::string::Format("%s/out/all/xyplan.dat", gameDir), "current-spec");
        hfClient.putFile(afl::string::Format("%s/out/all/playerfiles.zip", gameDir), "current-zip");
        for (int slot = 1; slot <= 5; ++slot) {
            hfClient.putFile(afl::string::Format("%s/in/player%d.trn", gameDir, slot), afl::string::Format("current-turn-%d", slot));
            hfClient.putFile(afl::string::Format("%s/out/%d/player%d.rst", gameDir, slot, slot), afl::string::Format("current-rst-%d", slot));
            game.getSlot(slot).turnStatus().set(server::host::Game::TurnGreen);
        }
    }

    /* Populate player history.
       Adds players to the game and fills their historical records. */
    void createPlayerHistory(server::host::Game& game, server::host::Root& root)
    {
        // Set primary players
        // "a" plays Fed for whole game
        for (int turn = 1; turn <= TURN_NUMBER; ++turn) {
            game.turn(turn).playerId().stringField("1").set("a");
        }
        game.pushPlayerSlot(1, "a", root);

        // "b" plays Lizard and is replaced by "c" in turn 20
        for (int turn = 1; turn <= TURN_NUMBER; ++turn) {
            game.turn(turn).playerId().stringField("2").set(turn < 20 ? "b" : "c");
        }
        game.pushPlayerSlot(2, "a", root);
        game.popPlayerSlot(2, root);
        game.pushPlayerSlot(2, "c", root);

        // "d" plays Bird for whole game and has a replacement "e"
        for (int turn = 1; turn <= TURN_NUMBER; ++turn) {
            game.turn(turn).playerId().stringField("3").set("d");
        }
        game.pushPlayerSlot(3, "d", root);
        game.pushPlayerSlot(3, "e", root);

        // "f" plays Klingon, and has replacement "a"
        for (int turn = 1; turn <= TURN_NUMBER; ++turn) {
            game.turn(turn).playerId().stringField("4").set("f");
        }
        game.pushPlayerSlot(4, "f", root);
        game.pushPlayerSlot(4, "a", root);
    }

    /* Check file tree beneath an item for consistency. */
    int checkItemTree(afl::test::Assert a, server::host::file::Item& item, int level)
    {
        // Information
        a.checkDifferent("01. getName", item.getName(), "");
        a.checkEqual("02. name", item.getInfo().name, item.getName());
        a.checkLessThan("03. level", level, 10);

        // printf("%*s%s\n", 3*level+5, "", item.getName().c_str());

        server::host::file::Item::ItemVector_t vec;
        int result = 0;
        switch (item.getInfo().type) {
         case server::interface::FileBase::IsDirectory:
            // Must be listable but not readable
            AFL_CHECK_THROWS(a("11. getContent"), item.getContent(), std::runtime_error);
            AFL_CHECK_SUCCEEDS(a("12. listContent"), item.listContent(vec));
            ++result;
            for (size_t i = 0, n = vec.size(); i < n; ++i) {
                // Verify subtree
                a.checkNonNull("13. element valid", vec[i]);

                int subtreeResult = checkItemTree(a(vec[i]->getName()), *vec[i], level+1);
                result += subtreeResult;

                // Verify that looking up the item will find it (a comparable one)
                std::auto_ptr<server::host::file::Item> foundItem(item.find(vec[i]->getName()));
                a.checkNonNull("21. find", foundItem.get());
                a.check("22. find result", foundItem.get() != vec[i]);
                a.checkEqual("23. getName", foundItem->getName(), vec[i]->getName());
                a.checkEqual("24. type", foundItem->getInfo().type, vec[i]->getInfo().type);

                // Verify the content. Note that this brings the runtime of this test to O(n^m).
                a.checkEqual("31. checkItemTree", checkItemTree(a(foundItem->getName()), *foundItem, level+1), subtreeResult);
            }
            break;

         case server::interface::FileBase::IsFile:
            // Must be readable but not listable
            a.checkDifferent("41. getContent", item.getContent(), "");
            AFL_CHECK_THROWS(a("42. listContent"), item.listContent(vec), std::runtime_error);
            a.checkEqual("43. size", vec.size(), 0U);
            ++result;
            break;

         default:
            a.fail("51. bad type");
            break;
        }
        return result;
    }

    /* Check file system tree, entry point. */
    int checkTree(afl::test::Assert a, server::host::Root& root, String_t path, String_t user)
    {
        server::host::Session session;
        session.setUser(user);

        server::host::file::GameRootItem item(session, root);

        // The GameRootItem is not listable.
        server::host::file::Item::ItemVector_t vec;
        item.listContent(vec);
        a.checkEqual("01. size", vec.size(), 0U);

        // We can obtain information
        a.checkEqual("11. getName", item.getName(), "game");
        a.checkEqual("12. name", item.getInfo().name, "game");
        a.checkEqual("13. type", item.getInfo().type, server::interface::FileBase::IsDirectory);
        a.checkEqual("14. label", item.getInfo().label, server::interface::HostFile::NoLabel);
        AFL_CHECK_THROWS(a("15. getContent"), item.getContent(), std::runtime_error);

        // We can locate the thing we want to work on
        std::auto_ptr<server::host::file::Item> pItem(item.find(path));
        a.checkNonNull("21. find", pItem.get());
        a.checkEqual("22. getName", pItem->getName(), path);

        return checkItemTree(a(item.getName()), *pItem, 0);
    }

    /* Check for a file and returns its content. */
    String_t checkFileContent(afl::test::Assert a, server::host::Root& root, String_t path, String_t user)
    {
        server::host::Session session;
        session.setUser(user);

        server::host::file::GameRootItem item(session, root);
        server::host::file::Item::ItemVector_t vec;
        server::host::file::Item& file = item.resolvePath(path, vec);
        a.checkEqual("01. type", file.getInfo().type, server::interface::FileBase::IsFile);
        return file.getContent();
    }

    /* Check for a file (intended to be used for non-existant files in TS_ASSERT_THROWS). */
    void checkFile(server::host::Root& root, String_t path, String_t user)
    {
        server::host::Session session;
        session.setUser(user);

        server::host::file::GameRootItem item(session, root);
        server::host::file::Item::ItemVector_t vec;
        item.resolvePath(path, vec);
    }
}

/** Test GameRootItem hierarchy.
    This test sets up a game in a virtual database and tries to read the files starting at a GameRootItem.
    This will eventually test all items below GameRootItem in the hierarchy. */
AFL_TEST("server.host.file.GameRootItem", a)
{
    // Build a Root
    afl::net::redis::InternalDatabase db;
    server::file::InternalFileServer hostFile;
    server::file::InternalFileServer userFile;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mailQueue(null);
    util::ProcessRunner checkturnRunner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, hostFile, userFile, mailQueue, checkturnRunner, fs, server::host::Configuration());

    // Create users
    createUser(db, "a");
    createUser(db, "b");
    createUser(db, "c");
    createUser(db, "d");
    createUser(db, "e");
    createUser(db, "f");

    // Create game
    afl::net::redis::IntegerKey(db, "game:lastid").set(41);
    int32_t gameId = createGame(root);
    a.checkEqual("01. createGame", gameId, 42);
    server::host::Game game(root, gameId);

    // Configure
    game.turnNumber().set(TURN_NUMBER);
    createGameHistory(game, root);
    createPlayerHistory(game, root);

    // Check tree syntax and connectivity for each user
    // Player a sees 30 turns for player 1 and 4. This will be
    //   2x30 turn files (1-29 + current)
    //   2x22 result files (9-29 + current)
    //     21 spec files
    //   3x29 folders for history
    //      4 folders (42/, history/, 1/, 4/)
    //      2 current spec files
    //  => 216
    a.checkEqual("11. player a", checkTree(a("11. player a"), root, "42", "a"), 218);

    // FIXME -> Player b is sees 20 turns. <- fails because not on game. Should they?
    // a.checkEqual("21. player b", checkTree(a("21. player b"), root, "42", "b"), ...);

    // Player c sees 10 turns (and 30 results).
    //    12 turn files (19-29 + current)
    //    22 result files (9-29 + current)
    //    21 spec files (9-29)
    //  2x29 folders for history
    //     3 folders (42/. history/, 2/)
    //     2 current spec files
    //  => 118
    a.checkEqual("31. player b", checkTree(a("31. player b"), root, "42", "c"), 118);

    // Player d sees 30 turns for one player. Same thing for e who replaces them.
    //     30 turn files (1-29 + current)
    //     22 result files (9-29 + current)
    //     21 spec files
    //   2x29 folders for history
    //      3 folders (42/, history/, 3/)
    //      2 current spec files
    //  => 136
    a.checkEqual("41. player d", checkTree(a("41. player d"), root, "42", "d"), 136);
    a.checkEqual("42. player e", checkTree(a("42. player e"), root, "42", "e"), 136);

    // Same thing for f.
    a.checkEqual("51. player f", checkTree(a("51. player f"), root, "42", "f"), 136);

    // Admin sees everything:
    //   5x30 turn files
    //   5x22 result files
    //     21 spec files
    //  12x29 folders
    //     13 folders
    //      2 current spec files
    // => 644
    a.checkEqual("61. root", checkTree(a("root"), root, "42", ""), 644);

    // Check content of some files.
    a.checkEqual("71", checkFileContent(a("r71"), root, "42/history/25/race.nm", "f"), "pre-spec-26");
    a.checkEqual("72", checkFileContent(a("r72"), root, "42/history/25/4/player4.rst", "f"), "pre-26-4");
    a.checkEqual("73", checkFileContent(a("r73"), root, "42/history/25/4/player4.trn", "f"), "turn-26-4");
    a.checkEqual("74", checkFileContent(a("r74"), root, "42/xyplan.dat", "a"), "current-spec");
    a.checkEqual("75", checkFileContent(a("r75"), root, "42/history/12/2/player2.rst", "c"), "pre-13-2");
    a.checkEqual("76", checkFileContent(a("r76"), root, "42/history/22/2/player2.rst", "c"), "pre-23-2");
    a.checkEqual("77", checkFileContent(a("r77"), root, "42/history/22/2/player2.trn", "c"), "turn-23-2");
    a.checkEqual("78", checkFileContent(a("r78"), root, "42/2/player2.trn", "c"), "current-turn-2");
    a.checkEqual("79", checkFileContent(a("r79"), root, "42/2/player2.rst", "c"), "current-rst-2");

    // Check nonexistance/inaccessibility of some files
    AFL_CHECK_THROWS(a("81. checkFile"), checkFile(root, "77/xyplan.dat", "f"),               std::runtime_error);
    AFL_CHECK_THROWS(a("82. checkFile"), checkFile(root, "42/history/25/race.nm", "x"),       std::runtime_error);
    AFL_CHECK_THROWS(a("83. checkFile"), checkFile(root, "42/history/50/race.nm", ""),        std::runtime_error);
    AFL_CHECK_THROWS(a("84. checkFile"), checkFile(root, "42/history/025/race.nm", "") ,      std::runtime_error);
    AFL_CHECK_THROWS(a("85. checkFile"), checkFile(root, "42/history/150/race.nm", ""),       std::runtime_error);
    AFL_CHECK_THROWS(a("86. checkFile"), checkFile(root, "42/history/25/4/player4.rst", "b"), std::runtime_error);
    AFL_CHECK_THROWS(a("87. checkFile"), checkFile(root, "42/history/12/2/player2.trn", "c"), std::runtime_error);
}
