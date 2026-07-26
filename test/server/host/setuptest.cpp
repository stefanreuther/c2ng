/**
  *  \file test/server/host/setuptest.cpp
  *  \brief Test for server::host::Setup
  */

#include "server/host/setup.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

using afl::net::redis::HashKey;
using afl::net::redis::IntegerSetKey;
using afl::net::redis::StringKey;
using server::host::Game;
using server::interface::FileBaseClient;

namespace {
    const char EAST_RACENAMES[] =
        "The Eastern Federation        "
        "The Eastern Lizard Alliance   "
        "The Eastern Bird Empire       "
        "The Eastern Klingon Empire    "
        "The Eastern Privateers        "
        "The Eastern Cyborg            "
        "The Eastern Crystals          "
        "The Eastern Evil Empire       "
        "The Eastern Robotic Imperium  "
        "The Eastern Rebels            "
        "The Eastern Colonies of Man   "
        "The Eastern Feds    "
        "The Eastern Lizards "
        "The Eastern Birds   "
        "The Eastern Klingons"
        "The East Privateers "
        "The Eastern Cyborg  "
        "The Eastern Crystals"
        "The Eastern Empire  "
        "The Eastern Robots  "
        "The Eastern Rebels  "
        "The Eastern Colonies"
        "East Fed    "
        "East Lizard "
        "East Bird   "
        "East Klingon"
        "East Privs  "
        "East Cyborg "
        "East Crystal"
        "East Empire "
        "East Robotic"
        "East Rebels "
        "East Colony ";
    const char WEST_RACENAMES[] =
        "The Western Federation        "
        "The Western Lizard Alliance   "
        "The Western Bird Empire       "
        "The Western Klingon Empire    "
        "The Western Privateers        "
        "The Western Cyborg            "
        "The Western Crystals          "
        "The Western Evil Empire       "
        "The Western Robotic Imperium  "
        "The Western Rebels            "
        "The Western Colonies of Man   "
        "The Western Feds    "
        "The Western Lizards "
        "The Western Birds   "
        "The Western Klingons"
        "The West Privateers "
        "The Western Cyborg  "
        "The Western Crystals"
        "The Western Empire  "
        "The Western Robots  "
        "The Western Rebels  "
        "The Western Colonies"
        "West Fed    "
        "West Lizard "
        "West Bird   "
        "West Klingon"
        "West Privs  "
        "West Cyborg "
        "West Crystal"
        "West Empire "
        "West Robotic"
        "West Rebels "
        "West Colony ";

    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

        server::file::InternalFileServer& hostFile()
            { return m_hostFile; }

     private:
        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };

    void createDefaults(TestHarness& h)
    {
        FileBaseClient(h.hostFile()).createDirectory("defaults");
        FileBaseClient(h.hostFile()).putFile("defaults/race-east.nm", EAST_RACENAMES);
        FileBaseClient(h.hostFile()).putFile("defaults/race-west.nm", WEST_RACENAMES);
        FileBaseClient(h.hostFile()).putFile("defaults/race-none.nm", String_t(682, ' '));
        FileBaseClient(h.hostFile()).createDirectory("gg");
        FileBaseClient(h.hostFile()).createDirectory("gg/data");
    }
}

/** Setup: base case. */
AFL_TEST("server.host.Setup:base", a)
{
    const int32_t GAME_ID = 42;
    TestHarness h;
    IntegerSetKey(h.db(), "game:all").add(GAME_ID);

    Game g(h.root(), GAME_ID);
    AFL_CHECK_SUCCEEDS(a, performPreHostSetup(h.root(), g));
}

/** Setup: dual duel. */
AFL_TEST("server.host.Setup:dd:normal", a)
{
    const int32_t GAME_ID = 42;
    TestHarness h;
    createDefaults(h);

    IntegerSetKey(h.db(), "game:all").add(GAME_ID);
    HashKey(h.db(), "game:42:settings").intField("kind").set(1);
    HashKey(h.db(), "game:42:player:1:status").stringField("race").set("4,5");
    HashKey(h.db(), "game:42:player:2:status").stringField("race").set("6,7");
    StringKey(h.db(), "game:42:dir").set("gg");

    Game g(h.root(), GAME_ID);
    AFL_CHECK_SUCCEEDS(a("01. execution succeeds"), performPreHostSetup(h.root(), g));

    // Verify
    a.checkEqual("11. playerRace", HashKey(h.db(), "game:42:settings").stringField("playerRace").get(), "4,6,6,4,1,2,3,5,7,8,9");
    a.checkEqual("12. race.nm", FileBaseClient(h.hostFile()).getFile("gg/data/race.nm"),
                 "The Western Klingon Empire    "
                 "The Western Cyborg            "
                 "The Eastern Cyborg            "
                 "The Eastern Klingon Empire    "
                 "                              "
                 "                              "
                 "                              "
                 "                              "
                 "                              "
                 "                              "
                 "                              "
                 "The Western Klingons"
                 "The Western Cyborg  "
                 "The Eastern Cyborg  "
                 "The Eastern Klingons"
                 "                    "
                 "                    "
                 "                    "
                 "                    "
                 "                    "
                 "                    "
                 "                    "
                 "West Klingon"
                 "West Cyborg "
                 "East Cyborg "
                 "East Klingon"
                 "            "
                 "            "
                 "            "
                 "            "
                 "            "
                 "            "
                 "            ");
}

/** Setup: dual duel, first-choice clash. */
AFL_TEST("server.host.Setup:dd:first-clash", a)
{
    const int32_t GAME_ID = 42;
    TestHarness h;
    createDefaults(h);

    IntegerSetKey(h.db(), "game:all").add(GAME_ID);
    HashKey(h.db(), "game:42:settings").intField("kind").set(1);
    HashKey(h.db(), "game:42:player:1:status").stringField("race").set("5,3");
    HashKey(h.db(), "game:42:player:2:status").stringField("race").set("5,4");
    StringKey(h.db(), "game:42:dir").set("gg");

    Game g(h.root(), GAME_ID);
    AFL_CHECK_SUCCEEDS(a("01. execution succeeds"), performPreHostSetup(h.root(), g));

    // Verify
    a.checkEqual("11. playerRace", HashKey(h.db(), "game:42:settings").stringField("playerRace").get(), "3,4,4,3,1,2,5,6,7,8,9");
}

/** Setup: dual duel, first and second choice clash. */
AFL_TEST("server.host.Setup:dd:second-clash", a)
{
    const int32_t GAME_ID = 42;
    TestHarness h;
    createDefaults(h);

    IntegerSetKey(h.db(), "game:all").add(GAME_ID);
    HashKey(h.db(), "game:42:settings").intField("kind").set(1);
    HashKey(h.db(), "game:42:player:1:status").stringField("race").set("7,11");
    HashKey(h.db(), "game:42:player:2:status").stringField("race").set("7,11");
    StringKey(h.db(), "game:42:dir").set("gg");

    Game g(h.root(), GAME_ID);
    AFL_CHECK_SUCCEEDS(a("01. execution succeeds"), performPreHostSetup(h.root(), g));

    // Verify
    a.checkEqual("11. playerRace", HashKey(h.db(), "game:42:settings").stringField("playerRace").get(), "7,11,11,7,1,2,3,4,5,6,8");
}

/** Setup: dual duel, garbage choice. */
AFL_TEST("server.host.Setup:dd:garbage", a)
{
    const int32_t GAME_ID = 42;
    TestHarness h;
    createDefaults(h);

    IntegerSetKey(h.db(), "game:all").add(GAME_ID);
    HashKey(h.db(), "game:42:settings").intField("kind").set(1);
    HashKey(h.db(), "game:42:player:1:status").stringField("race").set("12,aa,99,-1,xx");   // treated as 1,2
    HashKey(h.db(), "game:42:player:2:status").stringField("race").set("0,ee,7,www,8");     // treated as 7,8
    StringKey(h.db(), "game:42:dir").set("gg");

    Game g(h.root(), GAME_ID);
    AFL_CHECK_SUCCEEDS(a("01. execution succeeds"), performPreHostSetup(h.root(), g));

    // Verify
    a.checkEqual("11. playerRace", HashKey(h.db(), "game:42:settings").stringField("playerRace").get(), "1,7,7,1,2,3,4,5,6,8,9");
}
