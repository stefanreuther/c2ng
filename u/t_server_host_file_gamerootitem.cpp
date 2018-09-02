/**
  *  \file u/t_server_host_file_gamerootitem.cpp
  *  \brief Test for server::host::file::GameRootItem
  */

#include <stdexcept>
#include "server/host/file/gamerootitem.hpp"

#include "t_server_host_file.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/gamecreator.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

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
    int checkItemTree(server::host::file::Item& item, int level)
    {
        // Information
        TS_ASSERT_DIFFERS(item.getName(), "");
        TS_ASSERT_EQUALS(item.getInfo().name, item.getName());
        TS_ASSERT_LESS_THAN(level, 10);

        // printf("%*s%s\n", 3*level+5, "", item.getName().c_str());

        server::host::file::Item::ItemVector_t vec;
        int result = 0;
        switch (item.getInfo().type) {
         case server::interface::FileBase::IsDirectory:
            // Must be listable but not readable
            TS_ASSERT_THROWS(item.getContent(), std::runtime_error);
            TS_ASSERT_THROWS_NOTHING(item.listContent(vec));
            ++result;
            for (size_t i = 0, n = vec.size(); i < n; ++i) {
                // Verify subtree
                TS_ASSERT(vec[i] != 0);

                int subtreeResult = checkItemTree(*vec[i], level+1);
                result += subtreeResult;

                // Verify that looking up the item will find it (a comparable one)
                std::auto_ptr<server::host::file::Item> foundItem(item.find(vec[i]->getName()));
                TS_ASSERT(foundItem.get() != 0);
                TS_ASSERT(foundItem.get() != vec[i]);
                TS_ASSERT_EQUALS(foundItem->getName(), vec[i]->getName());
                TS_ASSERT_EQUALS(foundItem->getInfo().type, vec[i]->getInfo().type);

                // Verify the content. Note that this brings the runtime of this test to O(n^m).
                TS_ASSERT_EQUALS(checkItemTree(*foundItem, level+1), subtreeResult);
            }
            break;

         case server::interface::FileBase::IsFile:
            // Must be readable but not listable
            TS_ASSERT_DIFFERS(item.getContent(), "");
            TS_ASSERT_THROWS(item.listContent(vec), std::runtime_error);
            TS_ASSERT_EQUALS(vec.size(), 0U);
            ++result;
            break;

         default:
            TS_ASSERT(!"bad type");
            break;
        }
        return result;
    }

    /* Check file system tree, entry point. */
    int checkTree(server::host::Root& root, String_t path, String_t user)
    {
        server::host::Session session;
        session.setUser(user);

        server::host::file::GameRootItem item(session, root);

        // The GameRootItem is not listable.
        server::host::file::Item::ItemVector_t vec;
        item.listContent(vec);
        TS_ASSERT_EQUALS(vec.size(), 0U);

        // We can obtain information
        TS_ASSERT_EQUALS(item.getName(), "game");
        TS_ASSERT_EQUALS(item.getInfo().name, "game");
        TS_ASSERT_EQUALS(item.getInfo().type, server::interface::FileBase::IsDirectory);
        TS_ASSERT_EQUALS(item.getInfo().label, server::interface::HostFile::NoLabel);
        TS_ASSERT_THROWS(item.getContent(), std::runtime_error);

        // We can locate the thing we want to work on
        std::auto_ptr<server::host::file::Item> pItem(item.find(path));
        TS_ASSERT(pItem.get() != 0);
        TS_ASSERT_EQUALS(pItem->getName(), path);

        return checkItemTree(*pItem, 0);
    }

    /* Check for a file and returns its content. */
    String_t checkFileContent(server::host::Root& root, String_t path, String_t user)
    {
        server::host::Session session;
        session.setUser(user);

        server::host::file::GameRootItem item(session, root);
        server::host::file::Item::ItemVector_t vec;
        server::host::file::Item& file = item.resolvePath(path, vec);
        TS_ASSERT_EQUALS(file.getInfo().type, server::interface::FileBase::IsFile);
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
void
TestServerHostFileGameRootItem::testGame()
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
    TS_ASSERT_EQUALS(gameId, 42);
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
    TS_ASSERT_EQUALS(checkTree(root, "42", "a"), 218);

    // FIXME -> Player b is sees 20 turns. <- fails because not on game. Should they?
    // TS_ASSERT_EQUALS(checkTree(root, "42", "b"), ...);

    // Player c sees 10 turns (and 30 results).
    //    12 turn files (19-29 + current)
    //    22 result files (9-29 + current)
    //    21 spec files (9-29)
    //  2x29 folders for history
    //     3 folders (42/. history/, 2/)
    //     2 current spec files
    //  => 118
    TS_ASSERT_EQUALS(checkTree(root, "42", "c"), 118);

    // Player d sees 30 turns for one player. Same thing for e who replaces them.
    //     30 turn files (1-29 + current)
    //     22 result files (9-29 + current)
    //     21 spec files
    //   2x29 folders for history
    //      3 folders (42/, history/, 3/)
    //      2 current spec files
    //  => 136
    TS_ASSERT_EQUALS(checkTree(root, "42", "d"), 136);
    TS_ASSERT_EQUALS(checkTree(root, "42", "e"), 136);

    // Same thing for f.
    TS_ASSERT_EQUALS(checkTree(root, "42", "f"), 136);

    // Admin sees everything:
    //   5x30 turn files
    //   5x22 result files
    //     21 spec files
    //  12x29 folders
    //     13 folders
    //      2 current spec files
    // => 644
    TS_ASSERT_EQUALS(checkTree(root, "42", ""), 644);

    // Check content of some files.
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/25/race.nm", "f"), "pre-spec-26");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/25/4/player4.rst", "f"), "pre-26-4");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/25/4/player4.trn", "f"), "turn-26-4");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/25/4/player4.trn", "f"), "turn-26-4");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/xyplan.dat", "a"), "current-spec");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/12/2/player2.rst", "c"), "pre-13-2");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/22/2/player2.rst", "c"), "pre-23-2");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/history/22/2/player2.trn", "c"), "turn-23-2");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/2/player2.trn", "c"), "current-turn-2");
    TS_ASSERT_EQUALS(checkFileContent(root, "42/2/player2.rst", "c"), "current-rst-2");

    // Check nonexistance/inaccessibility of some files
    TS_ASSERT_THROWS(checkFile(root, "77/xyplan.dat", "f"),               std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/25/race.nm", "x"),       std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/50/race.nm", ""),        std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/025/race.nm", "") ,      std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/150/race.nm", ""),       std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/25/4/player4.rst", "b"), std::runtime_error);
    TS_ASSERT_THROWS(checkFile(root, "42/history/12/2/player2.trn", "c"), std::runtime_error);
}

