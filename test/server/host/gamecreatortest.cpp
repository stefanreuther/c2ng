/**
  *  \file test/server/host/gamecreatortest.cpp
  *  \brief Test for server::host::GameCreator
  */

#include "server/host/gamecreator.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.host.GameCreator:pickDayTime", a)
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
                a.checkDifferent("distinct", results[i], results[j]);
            }
        }
    }
}

/** Test pickDayTime().
    This tests the physical storage format. */
AFL_TEST("server.host.GameCreator:pickDayTime:storage", a)
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
        a.checkEqual("daytime", daytime, 600);
    }
}

/** Test createGame() and finishNewGame(). */
AFL_TEST("server.host.GameCreator:createGame", a)
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
    int32_t aa = testee.createNewGame();
    int32_t bb = testee.createNewGame();

    // Must get Ids 1 and 2.
    a.checkEqual("01. createNewGame", aa, 1);
    a.checkEqual("02. createNewGame", bb, 2);

    // Randomly verify file system content
    a.checkEqual("11. file", server::interface::FileBaseClient(hostFile).getFileInformation("games/0001/out/2").type, server::interface::FileBase::IsDirectory);
    a.checkEqual("12. file", server::interface::FileBaseClient(hostFile).getFileInformation("games/0002/data").type, server::interface::FileBase::IsDirectory);

    // Finish the games
    testee.finishNewGame(aa, HostGame::Joining, HostGame::PublicGame);
    testee.finishNewGame(bb, HostGame::Joining, HostGame::PrivateGame);

    // Verify database
    a.check("21. db",  IntegerSetKey(db, "game:state:joining").contains(aa));
    a.check("22. db",  IntegerSetKey(db, "game:state:joining").contains(bb));
    a.check("23. db",  IntegerSetKey(db, "game:pubstate:joining").contains(aa));
    a.check("24. db", !IntegerSetKey(db, "game:pubstate:joining").contains(bb));
    a.check("25. db",  IntegerSetKey(db, "game:all").contains(aa));
    a.check("26. db",  IntegerSetKey(db, "game:all").contains(bb));
}

/** Test createGame() and initializeGame(). */
AFL_TEST("server.host.GameCreator:initializeGame", a)
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
    int32_t aa = testee.createNewGame();
    a.checkEqual("01. createNewGame", aa, 1);

    // Initialize the game
    testee.initializeGame(aa);

    // Randomly verify DB content
    a.checkEqual("11. db", HashKey(db, "game:1:settings").stringField("host").get(), "Ho");
    a.checkEqual("12. db", HashKey(db, "game:1:settings").stringField("master").get(), "Ma");
    a.checkEqual("13. db", HashKey(db, "game:1:settings").stringField("shiplist").get(), "SL");
    a.checkEqual("14. db", HashKey(db, "game:1:settings").stringField("description").get(), "New Game");
    a.checkEqual("15. db", StringKey(db, "game:1:name").get(), "New Game");
}

/** Test copyGame(). */
AFL_TEST("server.host.GameCreator:copyGame", a)
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
    a.checkEqual("01. createNewGame", newId, 1);
    AFL_CHECK_SUCCEEDS(a("02. copyGame"), testee.copyGame(GAME_ID, newId));
    AFL_CHECK_SUCCEEDS(a("03. finishNewGame"), testee.finishNewGame(newId, HostGame::Joining, HostGame::PublicGame));

    // Verify
    Subtree t2(Subtree(db, "game:").subtree(newId));
    a.checkEqual("11. db", t2.stringKey("name").get(), "the name 1");                 // changed (numbered)
    a.checkEqual("12. db", t2.hashKey("settings").intField("turn").get(), 0);         // changed (turn 0)
    a.checkEqual("13. db", t2.hashKey("settings").stringField("description").get(), "the description");
    a.checkEqual("14. db", t2.hashKey("settings").stringField("host").get(), "P");
    a.checkEqual("15. db", t2.hashKey("settings").stringField("master").get(), "M");
    a.checkEqual("16. db", t2.hashKey("settings").stringField("shiplist").get(), "S");
    a.checkEqual("17. db", t2.hashKey("toolkind").stringField("akind").get(), "a");
    a.checkEqual("18. db", t2.hashKey("toolkind").stringField("bkind").get(), "b");
    a.check     ("19. db", t2.stringSetKey("tools").contains("a"));
    a.check     ("20. db", t2.stringSetKey("tools").contains("b"));
    a.checkEqual("21. db", t2.hashKey("tool:a:settings").stringField("hopp").get(), "topp");

    a.checkEqual("31. db", t2.stringListKey("schedule:list").size(), 2);
    a.checkEqual("32. db", t2.stringListKey("schedule:list")[0], "2");                // changed (normalized)
    a.checkEqual("33. db", t2.stringListKey("schedule:list")[1], "1");                // changed (normalized)
    a.checkEqual("34. db", t2.hashKey("schedule:2").intField("type").get(), 0);
    a.checkEqual("35. db", t2.hashKey("schedule:2").intField("daytime").get(), 360);  // changed (default daytime)
    a.checkEqual("36. db", t2.hashKey("schedule:1").intField("type").get(), 4);
    a.checkEqual("37. db", t2.hashKey("schedule:1").intField("daytime").get(), 360);  // changed (default daytime)
}
