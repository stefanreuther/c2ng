/**
  *  \file server/host/hostplayer.cpp
  *  \brief Class server::host::HostPlayer
  */

#include "server/host/hostplayer.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/errors.hpp"
#include "server/host/game.hpp"
#include "server/host/gamearbiter.hpp"
#include "server/host/installer.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hostgame.hpp"
#include "server/host/user.hpp"

using server::interface::HostGame;

namespace {
    size_t indexOf(const afl::data::StringList_t& list, const String_t& ele)
    {
        size_t i = 0;
        size_t size = list.size();
        while (i < size && list[i] != ele) {
            ++i;
        }
        return i;
    }
}

// Constructor.
server::host::HostPlayer::HostPlayer(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::host::HostPlayer::join(int32_t gameId, int32_t slot, String_t userId)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerJoin

    // Obtain critical access; player modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Check user
    if (!User::exists(m_root, userId)) {
        throw std::runtime_error(USER_NOT_FOUND);
    }

    // Check preconditions

    /* Only joining/running games can be joined */
    HostGame::State gameState = game.getState();
    if (gameState != HostGame::Joining && gameState != HostGame::Running) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    /* Admin can join anyone, anyone can join unlisted/public as himself,
       owner can put anyone into his games */
    HostGame::Type gameType = game.getType();
    bool isAdminAccess = (m_session.isAdmin() || m_session.getUser() == game.getOwner());
    if (!(isAdminAccess
          || ((gameType == HostGame::PublicGame || gameType == HostGame::UnlistedGame) && m_session.getUser() == userId)))
    {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    /* User must not already be playing elsewhere */
    if (!isAdminAccess && game.isUserOnGameAsPrimary(userId) && !game.isMultiJoinAllowed()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    /* Slot must be empty */
    if (!game.isSlotInGame(slot) || game.isSlotPlayed(slot)) {
        throw std::runtime_error(SLOT_NOT_AVAILABLE);
    }

    /* All conditions fulfilled */
    game.pushPlayerSlot(slot, userId, m_root);
    game.addUserHistoryItem(m_root, userId == m_session.getUser() ? "game-join" : "game-join-other", afl::string::Format("%s:%d", userId, slot), userId);
    if (!game.hasAnyOpenSlot()) {
        // CronImpl needs lastPlayerJoined to generate the correct time.
        // Because we're running under mutex protection, CronImpl will not interfere with us and see a partial state.
        game.setConfigInt("lastPlayerJoined", m_root.getTime());
        m_root.handleGameChange(gameId);
    }
}

void
server::host::HostPlayer::substitute(int32_t gameId, int32_t slot, String_t userId)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerSubst

    // Obtain critical access; player modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Check user
    // FIXME: check existence of user? Classic didn't.

    // Check preconditions

    /* Only joining/running games */
    HostGame::State gameState = game.getState();
    if (gameState != HostGame::Joining && gameState != HostGame::Running) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    /* Check player list */
    afl::data::StringList_t players;
    game.listPlayers(slot, players);
    size_t numPlayers = players.size();
    if (numPlayers == 0) {
        /* If list is empty, they can use PLAYERJOIN instead, which also
           checks that the slot actually exists */
        throw std::runtime_error(SLOT_EMPTY);
    }

    size_t userIndex = indexOf(players, userId);
    if (m_session.isAdmin() || m_session.getUser() == game.getOwner()) {
        /* Admin version: if player is on list, drop all its replacements; otherwise, add him,
           no more questions asked. */
        if (userIndex >= numPlayers) {
            /* Not on list */
            game.pushPlayerSlot(slot, userId, m_root);
        } else {
            /* Is on list */
            while (numPlayers > userIndex+1) {
                game.popPlayerSlot(slot, m_root);
                --numPlayers;
            }
        }
    } else {
        /* User version: caller must be on list. */
        size_t callerIndex = indexOf(players, m_session.getUser());
        if (callerIndex >= numPlayers || userIndex < callerIndex) {
            /* Caller not on list, or user on list before caller */
            throw std::runtime_error(PERMISSION_DENIED);
        }
        while (numPlayers > callerIndex+1) {
            game.popPlayerSlot(slot, m_root);
            --numPlayers;
        }
        if (callerIndex != userIndex) {
            game.pushPlayerSlot(slot, userId, m_root);
        }
    }
    game.addUserHistoryItem(m_root, "game-subst", afl::string::Format("%s:%d", userId, slot), userId);
}

void
server::host::HostPlayer::resign(int32_t gameId, int32_t slot, String_t userId)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerResign

    // Obtain critical access; player modifications cannot parallel anything
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Check preconditions

    /* Only joining/running games */
    HostGame::State gameState = game.getState();
    if (gameState != HostGame::Joining && gameState != HostGame::Running) {
        throw std::runtime_error(WRONG_GAME_STATE);
    }

    /* User must be in the game */
    afl::data::StringList_t players;
    game.listPlayers(slot, players);
    size_t numPlayers = players.size();
    size_t userIndex = indexOf(players, userId);
    if (userIndex >= numPlayers) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    /* Caller must be admin, in the list before the user, or owner of the game */
    size_t callerIndex = indexOf(players, m_session.getUser());
    if (callerIndex > userIndex && !m_session.isAdmin() && game.getOwner() != m_session.getUser()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    /* OK */
    while (numPlayers > userIndex) {
        game.popPlayerSlot(slot, m_root);
        --numPlayers;
    }

    /* If we've made a slot empty, notify the scheduler.
       - if this game is joining, this will stop the Master run
       - if this game is running and in run-when-all-turns-are-in mode,
         host may run because all other turns are in. */
    if (userIndex == 0) {
        /* Is this slot dead now? If so, drop it. */
        bool dead = false;
        if (gameState == HostGame::Running) {
            String_t packedScore = game.turn(game.turnNumber().get()).scores().stringField("timscore").get();
            if (slot <= 0
                || packedScore.size() < slot*4U
                || afl::bits::Int32LE::unpack(*reinterpret_cast<const afl::bits::Int32LE::Bytes_t*>(packedScore.data() + 4*(slot-1))) <= 0)
            {
                dead = true;
            }
        }
        if (dead) {
            game.getSlot(slot).slotStatus().set(0);
        }

        /* History */
        game.addUserHistoryItem(m_root, userId == m_session.getUser() ? dead ? "game-resign-dead" : "game-resign" : "game-resign-other", afl::string::Format("%s:%d", userId, slot), userId);

        // This was commented out in c2host classic:
        // /* If user drops himself, penalize him */
        // if (!dead && userId == conn.getUser() && !game.isRankingDisabled()) {
        //     handlePlayerDrop(database_connection, userId, game, slotId);
        //     handlePlayerRankChanges(database_connection, userId);
        // }

        /* Notify cron to recompute host time */
        m_root.handleGameChange(gameId);
    }
}

void
server::host::HostPlayer::add(int32_t gameId, String_t userId)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerAdd
    
    // Obtain simple access; this only changes permissions
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ConfigPermission);

    // Do it
    game.userReferenceCounters().intField(userId) += 0;
    User(m_root, userId).gameReferenceCount(gameId) += 0;
}

void
server::host::HostPlayer::list(int32_t gameId, bool all, std::map<int,Info>& result)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerList

    // Obtain simple access; read-only
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Load race names
    server::common::RaceNames raceNames;
    game.loadRaceNames(raceNames, m_root);

    // Adjust allPlayers
    String_t turn1State;
    if (all) {
        HostGame::State gameState = game.getState();
        if ((gameState != HostGame::Running && gameState != HostGame::Finished) || game.turnNumber().get() <= 0) {
            all = false;
        } else {
            turn1State = game.turn(1).info().turnStatus().get();
        }
    }

    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        if ((turn1State.size() >= 2U*i
             && afl::bits::Int16LE::unpack(*reinterpret_cast<const afl::bits::Int16LE::Bytes_t*>(turn1State.data() + 2*i)) >= 0)
            || game.isSlotInGame(i))
        {
            result.insert(std::make_pair(i, game.describeSlot(i, m_session.getUser(), raceNames)));
        }
    }
}

server::interface::HostPlayer::Info
server::host::HostPlayer::getInfo(int32_t gameId, int32_t slot)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerStat

    // Obtain simple access; read-only
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);
    if (!game.isSlotInGame(slot)) {
        throw std::runtime_error(SLOT_EMPTY);
    }

    // Load race names
    server::common::RaceNames raceNames;
    game.loadRaceNames(raceNames, m_root);

    // Produce result
    return game.describeSlot(slot, m_session.getUser(), raceNames);
}

void
server::host::HostPlayer::setDirectory(int32_t gameId, String_t userId, String_t dirName)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerSetDir

    // Obtain critical access; installation cannot go parallel with hosting
    // FIXME: if the game is hosting, we can still configure, just not install
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    if ((!m_session.isAdmin() && m_session.getUser() != userId) || !game.isUserOnGame(userId)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // Create the new game directory. This will throw an error if we don't
    // have permissions to do that. Also make sure we don't overwrite a
    // different game's data.
    // Use the target user permissions, not the session permissions,
    // actual installation will happen using target user permissions!
    server::interface::BaseClient(m_root.userFile()).setUserContext(userId);
    server::interface::FileBaseClient file(m_root.userFile());
    if (!dirName.empty()) {
        file.createDirectoryTree(dirName);
        int32_t oldGameId = file.getDirectoryIntegerProperty(dirName, "game");
        if (oldGameId != 0 && oldGameId != gameId) {
            throw std::runtime_error(DIRECTORY_IN_USE);
        }
    }

    // Get old state.
    String_t oldDir = game.getPlayerConfig(userId, "gameDir");
    if (oldDir != dirName && !oldDir.empty()) {
        Installer(m_root).uninstallGameData(userId, oldDir);
    }

    // Change game directory.
    game.setPlayerConfig(userId, "gameDir", dirName);

    // Install game data
    if (!dirName.empty()) {
        Installer(m_root).installGameData(game, game.getSlotsByPlayer(userId), userId, dirName);
    }
}

String_t
server::host::HostPlayer::getDirectory(int32_t gameId, String_t userId)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerGetDir

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    if ((!m_session.isAdmin() && m_session.getUser() != userId) || !game.isUserOnGame(userId)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    return game.getPlayerConfig(userId, "gameDir");
}

server::interface::HostPlayer::FileStatus
server::host::HostPlayer::checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName)
{
    // ex planetscentral/host/cmdplayer.h:doPlayerCheckFile

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    if ((!m_session.isAdmin() && m_session.getUser() != userId) || !game.isUserOnGame(userId)) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // Check game directory. Must be present, and the same as the specified one.
    const String_t gameDir = game.getPlayerConfig(userId, "gameDir");
    if (gameDir.empty()) {
        return Stale;
    }
    if (const String_t* p = dirName.get()) {
        if (*p != gameDir) {
            return Stale;
        }
    }

    // Check file name.
    if (Installer(m_root).isPreciousFile(fileName)) {
        return Allow;
    }

    // Could it be a turn file?
    int slot;
    if (fileName.size() >= 11
        && fileName.compare(0, 6, "player", 6) == 0
        && fileName.compare(fileName.size()-4, 4, ".trn", 4) == 0
        && afl::string::strToInteger(fileName.substr(6, fileName.size()-10), slot)
        && slot > 0
        && slot <= Game::NUM_PLAYERS)
    {
        afl::data::StringList_t players;
        game.listPlayers(slot, players);
        if (std::find(players.begin(), players.end(), userId) != players.end()) {
            return Turn;
        }
    }

    // Not permitted.
    return Refuse;
}

