/**
  *  \file server/host/game.cpp
  *  \brief Class server::host::Game
  */

#include <stdexcept>
#include "server/host/game.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/errors.hpp"
#include "server/host/cron.hpp"
#include "server/host/gamerating.hpp"
#include "server/host/installer.hpp"
#include "server/host/root.hpp"
#include "server/host/schedule.hpp"
#include "server/host/talklistener.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/types.hpp"
#include "server/host/user.hpp"

const int server::host::Game::NUM_PLAYERS;

using server::interface::HostGame;

namespace {
    const char LOG_NAME[] = "host.game";

    bool tryLoadRaceNames(server::common::RaceNames& raceNames,
                          String_t dir,
                          server::interface::FileBase& file)
    {
        if (dir.empty()) {
            // No directory set. This does not happen in regular games (host/master/shiplist do have a directory),
            // but happens a lot in testing. It could happen in regular games if we start using dummy entries for
            // pre-configured (externally-hosted) games.
            return false;
        }
        try {
            afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
            raceNames.load(afl::string::toBytes(file.getFile(dir + "/race.nm")), cs);
            return true;
        }
        catch (std::exception& e) {
            return false;
        }
    }
}


/******************************* Game::Slot ******************************/

// Constructor.
server::host::Game::Slot::Slot(afl::net::redis::Subtree tree)
    : m_tree(tree)
{
    // ex Game::Slot::Slot
}

// Access list of all users.
afl::net::redis::StringListKey
server::host::Game::Slot::players()
{
    // ex Game::Slot::players
    return m_tree.stringListKey("users");
}

// Access slot status.
afl::net::redis::IntegerField
server::host::Game::Slot::slotStatus()
{
    // ex Game::Slot::slotStatus
    return m_tree.hashKey("status").intField("slot");
}

// Access turn status.
afl::net::redis::IntegerField
server::host::Game::Slot::turnStatus()
{
    // ex Game::Slot::turnStatus
    return m_tree.hashKey("status").intField("turn");
}

// Access ranks.
afl::net::redis::IntegerField
server::host::Game::Slot::rank()
{
    // ex Game::Slot::rank
    return m_tree.hashKey("status").intField("rank");
}


/******************************* Game::Turn ******************************/

// Constructor.
server::host::Game::Turn::Turn(afl::net::redis::Subtree tree)
    : m_tree(tree)
{ }

// Access scores.
afl::net::redis::HashKey
server::host::Game::Turn::scores()
{
    return m_tree.hashKey("scores");
}

// Access turn information.
afl::net::redis::HashKey
server::host::Game::Turn::info()
{
    return m_tree.hashKey("info");
}

// Access player status.
afl::net::redis::HashKey
server::host::Game::Turn::playerId()
{
    return m_tree.hashKey("player");
}

/********************************** Game *********************************/

// Constructor.
server::host::Game::Game(Root& root, int32_t gameId)
    : m_game(root.gameRoot().subtree(gameId)),
      m_gameId(gameId)
{
    // ex Game::Game
    if (!root.gameRoot().intSetKey("all").contains(gameId)) {
        throw std::runtime_error(GAME_NOT_FOUND);
    }
}

// Constructor.
server::host::Game::Game(Root& root, int32_t gameId, NoExistanceCheck_t)
    : m_game(root.gameRoot().subtree(gameId)),
      m_gameId(gameId)
{ }

// Destructor.
server::host::Game::~Game()
{ }

// Get game Id.
int32_t
server::host::Game::getId() const
{
    // ex Game::getId
    return m_gameId;
}

// Get game state.
server::host::Game::State_t
server::host::Game::getState()
{
    // ex Game::getState
    State_t result;
    if (!HostGame::parseState(m_game.stringKey("state").get(), result)) {
        throw std::runtime_error(DATABASE_ERROR);
    }
    return result;
}

// Set game state.
void
server::host::Game::setState(State_t newState, TalkListener* talk, Root& root)
{
    // ex Game::setState
    const String_t newStateFormatted = HostGame::formatState(newState);
    const String_t oldState = m_game.stringKey("state").replaceBy(newStateFormatted);
    if (oldState != newStateFormatted) {
        // There was a change
        const Type_t gameType = getType();

        // Update history
        String_t histMessage = newStateFormatted;
        if (newState == HostGame::Finished) {
            String_t victor;
            for (int i = 1; i <= NUM_PLAYERS; ++i) {
                Slot slot(getSlot(i));
                if (slot.slotStatus().get() != 0 && slot.rank().get() == 1) {
                    // This is a rank-1 player
                    String_t player = slot.players()[0];
                    if (!player.empty() && !victor.empty()) {
                        // Ambiguous
                        victor.clear();
                        break;
                    }
                    victor = player;
                }
            }
            if (!victor.empty()) {
                histMessage += ":";
                histMessage += victor;
            }
        }
        addGameHistoryItem(root, "game-state", histMessage, gameType == HostGame::PublicGame && newState != HostGame::Preparing && newState != HostGame::Deleted);

        // Update indexes
        root.gameRoot().subtree("state").intSetKey(oldState).moveTo(m_gameId, root.gameRoot().subtree("state").intSetKey(newStateFormatted));
        if (gameType == HostGame::PublicGame) {
            root.gameRoot().subtree("pubstate").intSetKey(oldState).moveTo(m_gameId, root.gameRoot().subtree("pubstate").intSetKey(newStateFormatted));
        }
        if (newState == HostGame::Running) {
            if (getConfigInt("copyOf") != 0) {
                setConfigInt("copyPending", 1);
            }
            if (Cron* pCron = root.getCron()) {
                pCron->handleGameChange(m_gameId);
            }
        }
        if (talk) {
            if (newState == HostGame::Joining || newState == HostGame::Running) {
                talk->handleGameStart(*this, gameType);
            }
            if (newState == HostGame::Finished) {
                talk->handleGameEnd(*this, gameType);
            }
        }
    }
}

// Get game type.
server::host::Game::Type_t
server::host::Game::getType()
{
    // ex Game::getType
    Type_t result;
    if (!HostGame::parseType(m_game.stringKey("type").get(), result)) {
        throw std::runtime_error(DATABASE_ERROR);
    }
    return result;
}

// Set game type.
void
server::host::Game::setType(Type_t newType, TalkListener* talk, Root& root)
{
    // ex Game::setType
    // Change type
    const String_t newTypeFormatted = HostGame::formatType(newType);
    const String_t oldType = m_game.stringKey("type").replaceBy(newTypeFormatted);
    if (oldType != newTypeFormatted) {
        // There was a change, and we may have to move it
        const State_t state = getState();
        const String_t stateFormatted = HostGame::formatState(state);
        if (oldType == "public") {
            root.gameRoot().subtree("pubstate").intSetKey(stateFormatted).remove(m_gameId);
        }
        if (newType == HostGame::PublicGame) {
            root.gameRoot().subtree("pubstate").intSetKey(stateFormatted).add(m_gameId);
        }
        if (talk) {
            talk->handleGameTypeChange(*this, state, newType);
        }
    }
}

// Get game name.
String_t
server::host::Game::getName()
{
    // ex Game::getName
    return m_game.stringKey("name").get();
}

// Set game name.
void
server::host::Game::setName(String_t newName, TalkListener* talk)
{
    // ex Game::setName
    m_game.stringKey("name").set(newName);
    if (talk) {
        talk->handleGameNameChange(*this, newName);
    }
}

// Get game owner.
String_t
server::host::Game::getOwner()
{
    // ex Game::getOwner
    return m_game.stringKey("owner").get();
}

// Set game owner.
void
server::host::Game::setOwner(String_t newOwner, Root& root)
{
    // ex Game::setOwner
    String_t oldOwner = m_game.stringKey("owner").replaceBy(newOwner);
    if (oldOwner != newOwner) {
        if (!oldOwner.empty()) {
            User(root, oldOwner).ownedGames().remove(m_gameId);
        }
        if (!newOwner.empty()) {
            User(root, newOwner).ownedGames().add(m_gameId);
        }
    }
}

// Get configuration string value.
String_t
server::host::Game::getConfig(String_t name)
{
    // ex Game::getConfig
    return settings().stringField(name).get();
}

// Set configuration string value.
void
server::host::Game::setConfig(String_t name, String_t value)
{
    // ex Game::setConfig
    settings().stringField(name).set(value);
}

// Get configuration integer value.
int32_t
server::host::Game::getConfigInt(String_t name)
{
    // ex Game::getConfigInt
    return settings().intField(name).get();
}

// Set configuration inter value.
void
server::host::Game::setConfigInt(String_t name, int32_t value)
{
    // ex Game::setConfigInt
    settings().intField(name).set(value);

}
// Remove game configuration property.
void
server::host::Game::removeConfig(String_t name)
{
    // ex Game::removeConfig
    settings().field(name).remove();
}

// Get game directory.
String_t
server::host::Game::getDirectory()
{
    // ex Game::getDirectory
    return m_game.stringKey("dir").get();
}

// Add a history item to the game history.
void
server::host::Game::addGameHistoryItem(Root& root, String_t what, String_t args, bool global)
{
    // ex Game::addGameHistoryItem
    String_t message = afl::string::Format("%d:%s:%d:%s", root.config().getUserTimeFromTime(root.getTime()), what, m_gameId, args);
    m_game.stringListKey("history").pushFront(message);
    if (global) {
        root.globalHistory().pushFront(message);
    }
}

// Add a history item to user history.
void
server::host::Game::addUserHistoryItem(Root& root, String_t what, String_t args, String_t player)
{
    // ex Game::addUserHistoryItem
    String_t message = afl::string::Format("%d:%s:%d:%s", root.config().getUserTimeFromTime(root.getTime()), what, m_gameId, args);
    m_game.stringListKey("history").pushFront(message);
    User(root, player).history().pushFront(message);
}

// Get per-user string configuration value.
String_t
server::host::Game::getPlayerConfig(String_t player, String_t name)
{
    // ex Game::getPlayerConfig
    return m_game.subtree("user").hashKey(player).stringField(name).get();
}

// Set per-user string configuration value.
void
server::host::Game::setPlayerConfig(String_t player, String_t name, String_t value)
{
    // ex Game::setPlayerConfig
    m_game.subtree("user").hashKey(player).stringField(name).set(value);
}

// Get per-user integer configuration value.
int32_t
server::host::Game::getPlayerConfigInt(String_t player, String_t name)
{
    // ex Game::getPlayerConfigInt
    return m_game.subtree("user").hashKey(player).intField(name).get();
}

// Set per-user integer configuration value.
void
server::host::Game::setPlayerConfigInt(String_t player, String_t name, int32_t value)
{
    // ex Game::setPlayerConfigInt
    m_game.subtree("user").hashKey(player).intField(name).set(value);
}

// Get name of score used to determine the game end.
String_t
server::host::Game::getRefereeScoreName()
{
    // ex Game::getRefereeScoreName
    String_t score = getConfig("endScoreName");
    if (score.empty() && turnNumber().get() > 0) {
        // We can find an implicit name only for games that have started!
        if (m_game.hashKey("scores").field("score").exists()) {
            score = "score";
        } else {
            score = "timscore";
        }
    }
    return score;
}

// Access a slot (player position).
server::host::Game::Slot
server::host::Game::getSlot(int32_t slot)
{
    // ex Game::getSlot
    return Slot(m_game.subtree("player").subtree(slot));
}

// Check whether slot exists in the game.
bool
server::host::Game::isSlotInGame(int32_t slot)
{
    // ex Game::isSlotInGame
    // Check slot range first; although it would normally fail at the database anyway
    return slot > 0
        && slot <= NUM_PLAYERS
        && getSlot(slot).slotStatus().get() != 0;
}

// Check whether a slot is played.
bool
server::host::Game::isSlotPlayed(int32_t slot)
{
    // ex Game::isSlotPlayed
    return !getSlot(slot).players().empty();
}

// Check whether game has any open slots.
bool
server::host::Game::hasAnyOpenSlot()
{
    // ex Game::hasAnyOpenSlot
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (isSlotInGame(i) && !isSlotPlayed(i)) {
            return true;
        }
    }
    return false;
}

// Add player to a slot.
void
server::host::Game::pushPlayerSlot(int32_t slot, String_t player, Root& root)
{
    // ex Game::pushPlayerSlot
    // Add to database
    getSlot(slot).players().pushBack(player);
    ++userReferenceCounters().intField(player);
    ++User(root, player).gameReferenceCount(m_gameId);

    // Grant him access to the game's transfer folder
    String_t gameDir = m_game.stringKey("dir").get();

    afl::net::CommandHandler& hostFile(root.hostFile());
    server::interface::BaseClient(hostFile).setUserContext(String_t());
    server::interface::FileBaseClient hostFileClient(hostFile);
    hostFileClient.setDirectoryPermissions(gameDir + "/in/new", player, "w");
    hostFileClient.setDirectoryPermissions(gameDir + "/out/all", player, "rl");
    hostFileClient.setDirectoryPermissions(gameDir + "/out/" + String_t(afl::string::Format("%d", slot)), player, "rl");

    // Give him player files
    try {
        Installer(root).installChangedGameFiles(*this, player, slot, true);
    }
    catch (std::exception& e) {
        root.log().write(afl::sys::LogListener::Info, LOG_NAME, "install failure", e);
    }
}

// Remove player from a slot.
String_t
server::host::Game::popPlayerSlot(int32_t slot, Root& root)
{
    // ex Game::popPlayerSlot
    String_t player = getSlot(slot).players().popBack();
    if (!player.empty()) {
        // Remove
        --userReferenceCounters().intField(player);
        int32_t ref = --User(root, player).gameReferenceCount(m_gameId);

        // Revoke file permissions
        String_t gameDir = m_game.stringKey("dir").get();

        afl::net::CommandHandler& hostFile(root.hostFile());
        server::interface::BaseClient(hostFile).setUserContext(String_t());
        server::interface::FileBaseClient hostFileClient(hostFile);
        hostFileClient.setDirectoryPermissions(gameDir + "/out/" + String_t(afl::string::Format("%d", slot)), player, "0");
        if (ref == 0) {
            // Revoke file permissions for public directories
            hostFileClient.setDirectoryPermissions(gameDir + "/in/new", player, "0");
            hostFileClient.setDirectoryPermissions(gameDir + "/out/all", player, "0");
        }
    }

    // Uninstall game files
    try {
        Installer(root).installChangedGameFiles(*this, player, slot, false);
    }
    catch (std::exception& e) {
        root.log().write(afl::sys::LogListener::Info, LOG_NAME, "install failure", e);
    }
    return player;
}

// Get all players in a slot.
void
server::host::Game::listPlayers(int32_t slot, afl::data::StringList_t& players)
{
    // ex Game::listPlayers
    getSlot(slot).players().getAll(players);
}

// Get all slots played by a player.
game::PlayerSet_t
server::host::Game::getSlotsByPlayer(String_t player)
{
    // ex Game::getSlotsByPlayer
    game::PlayerSet_t slots;
    for (int slot = 1; slot <= NUM_PLAYERS; ++slot) {
        afl::data::StringList_t players;
        listPlayers(slot, players);
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i] == player) {
                slots += slot;
                break;
            }
        }
    }
    return slots;
}

// Get all slots.
game::PlayerSet_t
server::host::Game::getGameSlots()
{
    // ex Game::getGameSlots
    game::PlayerSet_t result;
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        if (isSlotInGame(i)) {
            result += i;
        }
    }
    return result;
}

// Clear cache.
void
server::host::Game::clearCache()
{
    // ex Game::clearCache
    m_game.hashKey("cache").remove();
}

// Get difficulty.
int32_t
server::host::Game::getDifficulty(Root& root)
{
    // ex Game::getDifficulty
    afl::net::redis::IntegerField f = m_game.hashKey("cache").intField("difficulty");
    std::auto_ptr<afl::data::Value> iv(f.getRawValue());
    if (!iv.get()) {
        int value = computeGameRating(root, *this);
        f.set(value);
        return value;
    } else {
        return toInteger(iv.get());
    }
}

// Mark game broken.
void
server::host::Game::markBroken(String_t message, Root& root)
{
    // ex Game::markBroken
    m_game.stringKey("crashmessage").set(message);
    root.gameRoot().intSetKey("broken").add(m_gameId);
}

// Get schedule subtree.
afl::net::redis::Subtree
server::host::Game::getSchedule()
{
    // ex Game::getSchedule
    return m_game.subtree("schedule");
}

// Access tools by kind.
afl::net::redis::HashKey
server::host::Game::toolsByKind()
{
    return m_game.hashKey("toolkind");
}

// Access tool data.
afl::net::redis::Subtree
server::host::Game::toolData(const String_t& toolId)
{
    // @change classic used "subtree(toolId)" [game:$gid:$toolId], although the documented way has always been [game:$gid:tool:$toolId].
    // Since this is not yet used anywhere, it has no observable effect.
    return m_game.subtree("tool").subtree(toolId);
}

// Access tools.
afl::net::redis::StringSetKey
server::host::Game::tools()
{
    return m_game.stringSetKey("tools");
}

// Access user reference counters.
afl::net::redis::HashKey
server::host::Game::userReferenceCounters()
{
    return m_game.hashKey("users");
}

// Access score descriptions.
afl::net::redis::HashKey
server::host::Game::scoreDescriptions()
{
    return m_game.hashKey("scores");
}

// Access settings.
afl::net::redis::HashKey
server::host::Game::settings()
{
    return m_game.hashKey("settings");
}

// Access rank points.
afl::net::redis::HashKey
server::host::Game::rankPoints()
{
    return m_game.hashKey("rankpoints");
}

// Access turn.
server::host::Game::Turn
server::host::Game::turn(int32_t nr)
{
    return Turn(m_game.subtree("turn").subtree(nr));
}

// Check whether user is or was on a game.
bool
server::host::Game::isUserOnGame(String_t user)
{
    // ex Game::isUserOnGame
    return userReferenceCounters().field(user).exists();
}

// Check whether user is on this game as primary player.
bool
server::host::Game::isUserOnGameAsPrimary(String_t user)
{
    // ex Game::isUserOnGameAsPrimary
    // Quick check first
    if (!isUserOnGame(user)) {
        return false;
    }

    // Check slots
    for (int i = 1; i <= NUM_PLAYERS; ++i) {
        Slot s = getSlot(i);
        if (s.slotStatus().get() != 0 && s.players()[0] == user) {
            return true;
        }
    }

    return false;
}

// Check whether ranking is disabled in this game.
bool
server::host::Game::isRankingDisabled()
{
    // ex Game::isRankingDisabled
    return getConfigInt("rankDisable") != 0 || getType() == HostGame::TestGame;
}

// Check whether joining as multiple races is allowed in this game.
bool
server::host::Game::isMultiJoinAllowed()
{
    // ex Game::isMultiJoinAllowed
    return getConfigInt("joinMulti") != 0 || getType() == HostGame::TestGame;
}

// Describe this game.
server::interface::HostGame::Info
server::host::Game::describe(bool verbose, String_t forUser, Root& root)
{
    // ex Game::describe
    /* @type HostGameInfo
       Information about a game.
       Game information can be provided in a "normal" and "verbose" format.
       Items marked ** are only included in verbose reports.
       @key id:GID                       (Game Id)
       @key state:HostGameState          (Game state)
       @key type:HostGameType            (Game type)
       @key name:Str                     (Game name, {game:$GID:name})
       @key description:Str              (**Game description)
       @key difficulty:Int               (Difficulty)
       @key currentSchedule:HostSchedule (current schedule)
       @key slots:StrList                (**Slot states. For each slot, one of "occupied", "open", "self", or "dead")
       @key turns:IntList                (**Turn states. For each slot, a {@type HostTurnStatus} value)
       @key joinable:Int                 (**1 if user can join this game)
       @key scores:IntList               (**Scores, if game is running or finished)
       @key host:Str                     (Host Id, {game:$GID:settings}->host)
       @key hostDescription:Str          (Description for that host)
       @key shiplist:Str                 (Shiplist Id, {game:$GID:settings}->shiplist)
       @key shiplistDescription:Str      (Description for that shiplist)
       @key master:Str                   (**Master Id, {game:$GID:settings}->master)
       @key masterDescription:Str        (**Description for that master)
       @key turn:Int                     (turn number)
       @key lastHostTime:Time            (last host time)
       @key nextHostTime:Time            (next host time, if known)
       @key forum:FID                    (**associated forum, if any) */
    HostGame::Info result;

    int32_t turnNr = turnNumber().get();
    const HostGame::State state = getState();

    // Id
    result.gameId = m_gameId;

    // State
    result.state = state;

    // Type
    result.type = getType();

    // Name
    result.name = getName();

    // Description
    if (verbose) {
        result.description = settings().stringField("description").get();
    }

    // Difficulty
    result.difficulty = getDifficulty(root);

    // Schedule
    afl::net::redis::Subtree schedule = m_game.subtree("schedule");
    String_t currentSchedule = schedule.stringListKey("list")[0];
    if (!currentSchedule.empty()) {
        Schedule sch;
        sch.loadFrom(schedule.hashKey(currentSchedule));
        result.currentSchedule = sch.describe(root.config());
    }

    // Slot states
    if (verbose) {
        std::vector<HostGame::SlotState> slotStates;
        std::vector<int32_t> turnStates;
        bool onGameAsPrimary = false;
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            Slot slot(getSlot(i));
            int turnState;
            if (slot.slotStatus().get() != 0) {
                afl::data::StringList_t players;
                slot.players().getAll(players);
                if (players.size() == 0) {
                    slotStates.push_back(HostGame::OpenSlot);
                    turnState = 0;
                } else if (std::find(players.begin(), players.end(), forUser) != players.end()) {
                    slotStates.push_back(HostGame::SelfSlot);
                    turnState = slot.turnStatus().get();
                    if (players[0] == forUser) {
                        onGameAsPrimary = true;
                    }
                } else {
                    slotStates.push_back(HostGame::OccupiedSlot);
                    turnState = slot.turnStatus().get();
                    if (!forUser.empty()) {
                        turnState &= TurnStateMask;
                        if (turnState == TurnYellow || turnState == TurnGreen) {
                            turnState = TurnGreen;
                        } else if (turnState == TurnDead) {
                            turnState = TurnDead;
                        } else {
                            turnState = TurnMissing;
                        }
                    }
                }
            } else {
                slotStates.push_back(HostGame::DeadSlot);
                turnState = 0;
            }
            turnStates.push_back(turnState);
        }

        result.slotStates = slotStates;
        result.turnStates = turnStates;
        result.joinable = (!onGameAsPrimary || isMultiJoinAllowed());
    }

    // Scores
    if (verbose && (state == HostGame::Running || state == HostGame::Finished)) {
        String_t scoredesc = m_game.hashKey("scores").stringField("score").get();
        String_t scorename;
        if (scoredesc.empty()) {
            scoredesc = "Classic Score";
            scorename = "timscore";
        } else {
            scorename = "score";
        }
        String_t scores = m_game.subtree("turn").subtree(turnNr).hashKey("scores").stringField(scorename).get();
        if (scores.size() == 4*NUM_PLAYERS) {
            std::vector<int32_t> packedScores(NUM_PLAYERS);
            afl::bits::unpackArray<afl::bits::Int32LE>(packedScores, afl::string::toBytes(scores));
            result.scores = packedScores;
            result.scoreName = scorename;
            result.scoreDescription = scoredesc;
        }
    }

    // Host
    String_t host = settings().stringField("host").get();
    result.hostName = host;
    result.hostDescription = root.hostRoot().byName(host).stringField("description").get();

    // Ship list
    String_t shipList = settings().stringField("shiplist").get();
    result.shipListName = shipList;
    result.shipListDescription = root.shipListRoot().byName(shipList).stringField("description").get();

    // Master
    if (verbose) {
        String_t master = settings().stringField("master").get();
        result.masterName = master;
        result.masterDescription = root.masterRoot().byName(master).stringField("description").get();
    }

    // Turn
    result.turnNumber = turnNr;

    // Host times
    result.lastHostTime = root.config().getUserTimeFromTime(lastHostTime().get());

    String_t nextHostTimeStr = getConfig("nextHostTime");
    int32_t nextHostTime;
    if (!nextHostTimeStr.empty() && afl::string::strToInteger(nextHostTimeStr, nextHostTime)) {
        result.nextHostTime = root.config().getUserTimeFromTime(nextHostTime);
    }

    // Forum
    if (verbose) {
        result.forumId = forumId().get();
    }

    return result;
}

// Describe a slot.
server::interface::HostPlayer::Info
server::host::Game::describeSlot(int32_t slot, String_t forUser, const server::common::RaceNames& raceNames)
{
    // ex Game::describeSlot
    server::interface::HostPlayer::Info result;

    if (const String_t* p = raceNames.longNames().at(slot)) {
        result.longName = *p;
    }
    if (const String_t* p = raceNames.shortNames().at(slot)) {
        result.shortName = *p;
    }
    if (const String_t* p = raceNames.adjectiveNames().at(slot)) {
        result.adjectiveName = *p;
    }

    bool counting = (forUser.empty() || forUser == getOwner());
    int32_t numEditable = 0;

    listPlayers(slot, result.userIds);
    for (size_t i = 0; i < result.userIds.size(); ++i) {
        if (forUser == result.userIds[i]) {
            counting = true;
        }
        if (counting) {
            ++numEditable;
        }
    }
    bool occupied = result.userIds.size() > 0;

    result.numEditable = numEditable;
    result.joinable = !occupied && (!isUserOnGameAsPrimary(forUser) || isMultiJoinAllowed());
    return result;
}

// Describe victory condition.
server::interface::HostGame::VictoryCondition
server::host::Game::describeVictoryCondition(Root& root)
{
    // ex planetscentral/host/victory.cc:describeVictoryCondition

    server::interface::HostGame::VictoryCondition result;

    /* Condition */
    String_t cond = getConfig("endCondition");
    result.endCondition = cond;

    if (cond == "turn") {
        /* Report parameters for 'turn' */
        int32_t endTurn = getConfigInt("endTurn");
        int32_t endProbability = getConfigInt("endProbability");
        if (endProbability <= 0 || endProbability >= 100) {
            endProbability = 100;
        }
        result.endTurn = endTurn;
        result.endProbability = endProbability;
    } else if (cond == "score") {
        /* Report parameters for 'score' */
        int32_t endTurn = getConfigInt("endTurn");
        if (endTurn < 1) {
            endTurn = 1;
        }
        int32_t endScore = getConfigInt("endScore");
        String_t endScoreName = getRefereeScoreName();
        result.endTurn = endTurn;
        result.endScore = endScore;
        result.endScoreName = endScoreName;
        if (endScoreName.empty()) {
            result.endScoreDescription = String_t();
        } else {
            result.endScoreDescription = scoreDescriptions().stringField(endScoreName).get();
        }
    } else {
        /* End is determined by an add-on. Find it. */
        String_t addonName = toolsByKind().stringField("referee").get();
        if (!addonName.empty()) {
            result.referee = addonName;
            result.refereeDescription = root.toolRoot().byName(addonName).stringField("description").get();
        }
    }
    return result;
}

// Check permissions.
bool
server::host::Game::hasPermission(String_t user, PermissionLevel level)
{
    // ex Game::hasPermission
    // xref talk/htmlout.cc, renderGameLink
    // Admin has all permissions
    if (user.empty()) {
        return true;
    }

    switch (level) {
     case ReadPermission:
        // Everyone can read joining/running/finished x unlisted/public
        {
            HostGame::State state = getState();
            if (state != HostGame::Joining && state != HostGame::Running && state != HostGame::Finished) {
                return getOwner() == user;
            }

            Type_t type = getType();
            return type == HostGame::UnlistedGame
                || type == HostGame::PublicGame
                || getOwner() == user
                || isUserOnGame(user);
        }

     case ConfigPermission:
     case AdminPermission:
        // Only owner has these permissions
        return getOwner() == user;
    }
    return false;

    // - admin         --> admin permissions (=no USER command)
    // - none          --> no permissions needed, always available
    // - game:read     --> everyone for joining/running/finished x unlisted/public, owner+players for j/r/f private/test
    // - game:admin    --> ?
    // - game:config   --> owner
}

// Load race names.
void
server::host::Game::loadRaceNames(server::common::RaceNames& raceNames, Root& root)
{
    // ex Game::loadRaceNames

    // Read race.nm file. Locations to try:
    // - game directory
    // - shiplist directory
    // - master directory
    // - host directory
    // - defaults
    // Note that we need admin permissions to read all these files.

    // Configure filer
    afl::net::CommandHandler& hostFile = root.hostFile();
    server::interface::BaseClient(hostFile).setUserContext(String_t());
    server::interface::FileBaseClient hostFileClient(hostFile);

    if (tryLoadRaceNames(raceNames, m_game.stringKey("dir").get() + "/data", hostFileClient)
        || tryLoadRaceNames(raceNames,
                            root.shipListRoot().byName(getConfig("shiplist")).stringField("path").get(), hostFileClient)
        || tryLoadRaceNames(raceNames,
                            root.masterRoot().byName(getConfig("master")).stringField("path").get(), hostFileClient)
        || tryLoadRaceNames(raceNames,
                            root.hostRoot().byName(getConfig("host")).stringField("path").get(), hostFileClient)
        || tryLoadRaceNames(raceNames, "defaults", hostFileClient))
    {
        // ok
    } else {
        // not ok. We could generate some defaults, but that would fail
        // when the game is ultimately hosted.
    }
}

// Access "configuration changed" settings value.
afl::net::redis::IntegerField
server::host::Game::configChanged()
{
    // ex Game::configChanged
    return settings().intField("configChanged");
}

// Access "schedule changed" settings value.
afl::net::redis::IntegerField
server::host::Game::scheduleChanged()
{
    // ex Game::scheduleChanged
    return settings().intField("scheduleChanged");
}

// Access "end condition changed" settings value.
afl::net::redis::IntegerField
server::host::Game::endChanged()
{
    // ex Game::endChanged
    return settings().intField("endChanged");
}

// Access turn number.
afl::net::redis::IntegerField
server::host::Game::turnNumber()
{
    return settings().intField("turn");
}

// Access time of last schedule change.
afl::net::redis::IntegerField
server::host::Game::lastScheduleChangeTime()
{
    return settings().intField("lastScheduleChange");
}

// Access time of last host.
afl::net::redis::IntegerField
server::host::Game::lastHostTime()
{
    return settings().intField("lastHostTime");
}

// Access time of last turn submission.
afl::net::redis::IntegerField
server::host::Game::lastTurnSubmissionTime()
{
    return settings().intField("lastTurnSubmitted");
}

// Access forum number.
afl::net::redis::IntegerField
server::host::Game::forumId()
{
    return settings().intField("forum");
}

// Access "forum disabled" status.
afl::net::redis::IntegerField
server::host::Game::forumDisabled()
{
    return settings().intField("forumDisable");
}
