/**
  *  \file server/host/gamecreator.cpp
  *  \brief Class server::host::GameCreator
  */

#include "server/host/gamecreator.hpp"
#include "afl/base/countof.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/host/schedule.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hostgame.hpp"
#include "server/types.hpp"

namespace {
    /* Copy a game's tools.
       \param src Source game
       \param dst Destination game
       \param toolRoot Database root of "tools:" tree */
    void copyTools(afl::net::redis::Subtree& src, afl::net::redis::Subtree& dst, server::host::Root::ToolTree toolRoot)
    {
        afl::data::StringList_t tools;
        src.stringSetKey("tools").getAll(tools);
        for (size_t i = 0; i < tools.size(); ++i) {
            // - tools
            const String_t& tool = tools[i];
            dst.stringSetKey("tools").add(tool);

            // - tool:$TOOL:settings
            afl::data::StringList_t settings;
            src.subtree("tool").subtree(tool).hashKey("settings").getAll(settings);
#if 0
            // We don't have a setAll(), so use the for loop below
            if (!settings.empty()) {
                dst.subtree("tool").subtree(tool).hashKey("settings").setAll(settings);
            }
#else
            for (size_t i = 0; i+1 < settings.size(); i += 2) {
                dst.subtree("tool").subtree(tool).hashKey("settings").stringField(settings[i]).set(settings[i+1]);
            }
#endif

            // - toolkind. Copy from master data, not from source game.
            String_t kind = toolRoot.byName(tool).stringField("kind").get();
            if (!kind.empty()) {
                dst.hashKey("toolkind").stringField(kind).set(tool);
            }
        }
    }

    /* Copy a game's schedules.
       \param src Source game
       \param dst Destination game
       \param daytime Place destination schedules on this daytime */
    void copySchedule(afl::net::redis::Subtree& src, afl::net::redis::Subtree& dst, int32_t daytime)
    {
        afl::net::redis::Subtree srcSched(src.subtree("schedule"));
        afl::net::redis::Subtree dstSched(dst.subtree("schedule"));

        // Get list of schedules
        afl::data::IntegerList_t list;
        srcSched.intListKey("list").getAll(list);

        // Copy them one by one, modifying the daytime
        for (std::size_t i = list.size(); i > 0; --i) {
            server::host::Schedule sch;
            sch.loadFrom(srcSched.hashKey(afl::string::Format("%d", list[i-1])));
            sch.setDaytime(daytime);

            int32_t newId = ++dstSched.intKey("lastId");
            sch.saveTo(dstSched.hashKey(afl::string::Format("%d", newId)));
            dstSched.intListKey("list").pushFront(newId);
        }
    }
}


// Constructor.
server::host::GameCreator::GameCreator(Root& root)
    : m_root(root)
{ }

// Create a new game.
int32_t
server::host::GameCreator::createNewGame()
{
    // ex planetscentral/host/newgame.h:createNewGame
    afl::net::redis::Subtree root(m_root.gameRoot());

    // Database operations
    // - Allocate new Id
    const int32_t id = ++root.intKey("lastid");
    const String_t dirName = afl::string::Format("games/%04d", id);

    // - Initial configuration
    // FIXME: should we use the Game class here?
    afl::net::redis::Subtree(root.subtree(id)).stringKey("dir").set(dirName);

    // File operations
    server::interface::BaseClient(m_root.hostFile()).setUserContext(String_t());
    server::interface::FileBaseClient file(m_root.hostFile());
    try {
        file.removeDirectory(dirName);
    }
    catch (...)
    { }

    // Create game directory. Use createDirectoryTree, so it works on an empty filespace.
    file.createDirectoryTree(dirName);

    // Create child directories.
    file.createDirectory(dirName + "/data");
    file.createDirectory(dirName + "/backup");
    file.createDirectory(dirName + "/in");
    file.createDirectory(dirName + "/in/new");
    file.createDirectory(dirName + "/out");
    file.createDirectory(dirName + "/out/all");
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        file.createDirectory(afl::string::Format("%s/out/%d", dirName, i));
    }

    return id;
}

// Initialize a game.
void
server::host::GameCreator::initializeGame(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doNewGame (part)
    // Prepare it
    // FIXME: use higher-level primitives!
    afl::net::redis::Subtree root(m_root.gameRoot());
    afl::net::redis::Subtree game(root.subtree(gameId));
    game.stringKey("name").set("New Game");
    game.stringKey("owner").set("");
    game.intKey("schedule:lastId").set(0);
    game.hashKey("settings").stringField("description").set("New Game");
    game.hashKey("settings").stringField("host")       .set(m_root.hostRoot().defaultName().get());
    game.hashKey("settings").stringField("master")     .set(m_root.masterRoot().defaultName().get());
    game.hashKey("settings").stringField("shiplist")   .set(m_root.shipListRoot().defaultName().get());

    // Player slots
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        afl::net::redis::HashKey h = game.subtree("player").subtree(i).hashKey("status");
        h.intField("slot").set(1);  // Slot is open
        h.intField("turn").set(0);  // Turn is missing
    }
}

// Copy a game.
void
server::host::GameCreator::copyGame(int32_t srcId, int32_t dstId)
{
    // ex planetscentral/host/newgame.h:copyGame
    afl::net::redis::Subtree root(m_root.gameRoot());
    afl::net::redis::Subtree src(root.subtree(srcId));
    afl::net::redis::Subtree dst(root.subtree(dstId));

    // Build new name
    String_t srcName = src.stringKey("name").get();
    dst.stringKey("name").set(afl::string::Format("%s %d", srcName, ++root.hashKey("bynameprefix").intField(srcName)));

    // Open slots
    for (int slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        // FIXME: if this game has run, use game:$GID:turn:1:info->turnstatus to find open slots
        afl::net::redis::HashKey dstH = dst.subtree("player").subtree(slot).hashKey("status");
        afl::net::redis::HashKey srcH = src.subtree("player").subtree(slot).hashKey("status");
        dstH.intField("slot").set(srcH.intField("slot").get());
        dstH.intField("turn").set(0);
    }

    // Copy schedule
    copySchedule(src, dst, pickDayTime());

    // Copy settings
    afl::net::redis::HashKey srcSet = src.hashKey("settings");
    afl::net::redis::HashKey dstSet = dst.hashKey("settings");

    // - copy some fields raw
    static const char*const fieldsToCopy[] = {
        // - do not copy lastHostTime, lastTurnSubmitted, lastPlayerJoined, nextHostTime
        // - do not copy turn, timestamp, rankTurn, hostRunNow
        // - do not copy copyEnable, copyNext
        "description",
        "host",
        "master",
        "shiplist",
        "endCondition",
        "endTurn",
        "endProbability",
        "endScore",
        "endScoreName",
        "rankDisable",
        "joinMulti",
    };
    for (size_t i = 0; i < countof(fieldsToCopy); ++i) {
        std::auto_ptr<afl::data::Value> v(srcSet.field(fieldsToCopy[i]).getRawValue());
        if (v.get() != 0) {
            dstSet.stringField(fieldsToCopy[i]).set(toString(v.get()));
        }
    }

    // - set some fields to fixed values
    dstSet.intField("masterHasRun").set(0);
    dstSet.intField("copyOf").set(srcId);

    // Copy tools
    copyTools(src, dst, m_root.toolRoot());

    // Do not copy state. This is set by createNewGameFinish.
    // Do not copy type. This is set by createNewGameFinish.
    // Do not copy owner.

}

// Finish game creation.
void
server::host::GameCreator::finishNewGame(int32_t id, server::interface::HostGame::State state, server::interface::HostGame::Type type)
{
    // ex planetscentral/host/newgame.h:createNewGameFinish
    using server::interface::HostGame;
    const char PREPARE_STATE[] = "preparing";

    afl::net::redis::Subtree root(m_root.gameRoot());
    afl::net::redis::Subtree game(root.subtree(id));

    // Create the game in state "preparing". This is less efficient than creating it
    // in the correct state directly, but allows us to re-use the transition handling
    // of Game::setState for proper interaction with the rest of the system.
    game.stringKey("state").set(PREPARE_STATE);
    game.stringKey("type").set(HostGame::formatType(type));
    root.subtree("state").intSetKey(PREPARE_STATE).add(id);
    if (type == HostGame::PublicGame) {
        root.subtree("pubstate").intSetKey(PREPARE_STATE).add(id);
    }
    root.intSetKey("all").add(id);

    // Perform a regular state transition
    Game(m_root, id, Game::NoExistanceCheck).setState(state, m_root.getForum(), m_root);
}

// Pick daytime for a new game.
int32_t
server::host::GameCreator::pickDayTime()
{
    // ex planetscentral/host/schedule.cc:pickDayTime
    // FIXME: I'm not 100% happy with this function's placement
    // Pick an hour
    afl::net::redis::HashKey hours(m_root.gameRoot().hashKey("hours"));
    int32_t bestHour = 6;
    int32_t bestLoad = 0;
    for (int32_t i = 0; i < 24; ++i) {
        const int32_t hourToTest = (6 + i) % 24;
        const int32_t loadAtThisTime = hours.intField(afl::string::Format("%d", hourToTest)).get();
        if (i == 0 || loadAtThisTime < bestLoad) {
            bestHour = hourToTest;
            bestLoad = loadAtThisTime;
        }
    }
    ++hours.intField(afl::string::Format("%d", bestHour));

    // Convert hour to minutes
    return 60*bestHour;
}
