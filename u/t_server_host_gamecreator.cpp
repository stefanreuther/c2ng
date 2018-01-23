/**
  *  \file u/t_server_host_gamecreator.cpp
  *  \brief Test for server::host::GameCreator
  */

#include "server/host/gamecreator.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/configuration.hpp"
#include "server/host/root.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using afl::net::redis::StringSetKey;
using afl::net::redis::Subtree;
using server::interface::HostGame;

/** Test pickDayTime().
    This tests the basic guarantees without looking at the representation. */
void
TestServerHostGameCreator::testPickDayTime()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, null, null, mail, runner, fs, server::host::Configuration());

    // Testee
    server::host::GameCreator testee(root);

    // Pick 20 daytimes. They must each be different.
    const int N = 20;
    int32_t results[N];
    for (int i = 0; i < N; ++i) {
        results[i] = testee.pickDayTime();
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i != j) {
                TS_ASSERT_DIFFERS(results[i], results[j]);
            }
        }
    }
}

/** Test pickDayTime().
    This tests the physical storage format. */
void
TestServerHostGameCreator::testPickDayTime2()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, null, null, mail, runner, fs, server::host::Configuration());

    // Block all hours but hour 10 with 20 games
    for (int i = 0; i < 24; ++i) {
        if (i != 10) {
            db.callVoid(afl::data::Segment().pushBackString("hset").pushBackString("game:hours").pushBackInteger(i).pushBackInteger(20));
        }
    }

    // Testee
    server::host::GameCreator testee(root);
    for (int i = 0; i < 19; ++i) {
        int32_t daytime = testee.pickDayTime();
        TS_ASSERT_EQUALS(daytime, 600);
    }
}

/** Test createGame() and finishNewGame(). */
void
TestServerHostGameCreator::testCreateGame()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    server::file::InternalFileServer hostFile;
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, hostFile, null, mail, runner, fs, server::host::Configuration());

    // Testee
    server::host::GameCreator testee(root);

    // Create two games.
    int32_t a = testee.createNewGame();
    int32_t b = testee.createNewGame();

    // Must get Ids 1 and 2.
    TS_ASSERT_EQUALS(a, 1);
    TS_ASSERT_EQUALS(b, 2);

    // Randomly verify file system content
    TS_ASSERT_EQUALS(server::interface::FileBaseClient(hostFile).getFileInformation("games/0001/out/2").type, server::interface::FileBase::IsDirectory);
    TS_ASSERT_EQUALS(server::interface::FileBaseClient(hostFile).getFileInformation("games/0002/data").type, server::interface::FileBase::IsDirectory);

    // Finish the games
    testee.finishNewGame(a, HostGame::Joining, HostGame::PublicGame);
    testee.finishNewGame(b, HostGame::Joining, HostGame::PrivateGame);

    // Verify database
    TS_ASSERT( IntegerSetKey(db, "game:state:joining").contains(a));
    TS_ASSERT( IntegerSetKey(db, "game:state:joining").contains(b));
    TS_ASSERT( IntegerSetKey(db, "game:pubstate:joining").contains(a));
    TS_ASSERT(!IntegerSetKey(db, "game:pubstate:joining").contains(b));
    TS_ASSERT( IntegerSetKey(db, "game:all").contains(a));
    TS_ASSERT( IntegerSetKey(db, "game:all").contains(b));
}

/** Test createGame() and initializeGame(). */
void
TestServerHostGameCreator::testInitializeGame()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    server::file::InternalFileServer hostFile;
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, hostFile, null, mail, runner, fs, server::host::Configuration());

    // Database
    StringKey(db, "prog:host:default").set("Ho");
    StringKey(db, "prog:master:default").set("Ma");
    StringKey(db, "prog:sl:default").set("SL");

    // Testee
    server::host::GameCreator testee(root);

    // Create a game
    int32_t a = testee.createNewGame();
    TS_ASSERT_EQUALS(a, 1);

    // Initialize the game
    testee.initializeGame(a);

    // Randomly verify DB content
    TS_ASSERT_EQUALS(HashKey(db, "game:1:settings").stringField("host").get(), "Ho");
    TS_ASSERT_EQUALS(HashKey(db, "game:1:settings").stringField("master").get(), "Ma");
    TS_ASSERT_EQUALS(HashKey(db, "game:1:settings").stringField("shiplist").get(), "SL");
    TS_ASSERT_EQUALS(HashKey(db, "game:1:settings").stringField("description").get(), "New Game");
    TS_ASSERT_EQUALS(StringKey(db, "game:1:name").get(), "New Game");
}

/** Test copyGame(). */
void
TestServerHostGameCreator::testCopy()
{
    // Environment
    afl::net::redis::InternalDatabase db;
    afl::net::NullCommandHandler null;
    server::interface::MailQueueClient mail(null);
    server::file::InternalFileServer hostFile;
    util::ProcessRunner runner;
    afl::io::NullFileSystem fs;
    server::host::Root root(db, hostFile, null, mail, runner, fs, server::host::Configuration());

    // Database
    StringSetKey(db, "prog:host:all").add("P");
    StringSetKey(db, "prog:master:all").add("M");
    StringSetKey(db, "prog:sl:all").add("S");
    StringSetKey(db, "prog:tool:all").add("a");
    StringSetKey(db, "prog:tool:all").add("b");
    HashKey(db, "prog:tool:prog:a").stringField("kind").set("akind");
    HashKey(db, "prog:tool:prog:b").stringField("kind").set("bkind");

    // Create game by whacking the database
    const int GAME_ID = 80;
    IntegerSetKey(db, "game:all").add(GAME_ID);
    Subtree t(Subtree(db, "game:").subtree(GAME_ID));
    t.stringKey("name").set("the name");
    t.stringKey("state").set("running");
    t.stringKey("type").set("unlisted");
    t.hashKey("settings").intField("turn").set(12);
    t.hashKey("settings").stringField("description").set("the description");
    t.hashKey("settings").stringField("host").set("P");
    t.hashKey("settings").stringField("master").set("M");
    t.hashKey("settings").stringField("shiplist").set("S");
    t.hashKey("toolkind").stringField("akind").set("a");
    t.hashKey("toolkind").stringField("bkind").set("b");
    t.stringSetKey("tools").add("a");
    t.stringSetKey("tools").add("b");
    t.hashKey("tool:a:settings").stringField("hopp").set("topp");
    t.stringListKey("schedule:list").pushBack("79");
    t.stringListKey("schedule:list").pushBack("15");
    t.hashKey("schedule:79").intField("type").set(0);
    t.hashKey("schedule:79").intField("daytime").set(70);
    t.hashKey("schedule:15").intField("type").set(4);
    t.hashKey("schedule:15").intField("daytime").set(80);

    // Copy it
    server::host::GameCreator testee(root);
    int32_t newId = testee.createNewGame();
    TS_ASSERT_EQUALS(newId, 1);
    TS_ASSERT_THROWS_NOTHING(testee.copyGame(GAME_ID, newId));
    TS_ASSERT_THROWS_NOTHING(testee.finishNewGame(newId, HostGame::Joining, HostGame::PublicGame));

    // Verify
    Subtree t2(Subtree(db, "game:").subtree(newId));
    TS_ASSERT_EQUALS(t2.stringKey("name").get(), "the name 1");                 // changed (numbered)
    TS_ASSERT_EQUALS(t2.hashKey("settings").intField("turn").get(), 0);         // changed (turn 0)
    TS_ASSERT_EQUALS(t2.hashKey("settings").stringField("description").get(), "the description");
    TS_ASSERT_EQUALS(t2.hashKey("settings").stringField("host").get(), "P");
    TS_ASSERT_EQUALS(t2.hashKey("settings").stringField("master").get(), "M");
    TS_ASSERT_EQUALS(t2.hashKey("settings").stringField("shiplist").get(), "S");
    TS_ASSERT_EQUALS(t2.hashKey("toolkind").stringField("akind").get(), "a");
    TS_ASSERT_EQUALS(t2.hashKey("toolkind").stringField("bkind").get(), "b");
    TS_ASSERT(t2.stringSetKey("tools").contains("a"));
    TS_ASSERT(t2.stringSetKey("tools").contains("b"));
    TS_ASSERT_EQUALS(t2.hashKey("tool:a:settings").stringField("hopp").get(), "topp");

    TS_ASSERT_EQUALS(t2.stringListKey("schedule:list").size(), 2);
    TS_ASSERT_EQUALS(t2.stringListKey("schedule:list")[0], "2");                // changed (normalized)
    TS_ASSERT_EQUALS(t2.stringListKey("schedule:list")[1], "1");                // changed (normalized)
    TS_ASSERT_EQUALS(t2.hashKey("schedule:2").intField("type").get(), 0);
    TS_ASSERT_EQUALS(t2.hashKey("schedule:2").intField("daytime").get(), 360);  // changed (default daytime)
    TS_ASSERT_EQUALS(t2.hashKey("schedule:1").intField("type").get(), 4);
    TS_ASSERT_EQUALS(t2.hashKey("schedule:1").intField("daytime").get(), 360);  // changed (default daytime)
}

