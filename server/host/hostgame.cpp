/**
  *  \file server/host/hostgame.cpp
  *  \brief Class server::host::HostGame
  */

#include <stdexcept>
#include "server/host/hostgame.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/errors.hpp"
#include "server/host/actions.hpp"
#include "server/host/game.hpp"
#include "server/host/gamecreator.hpp"
#include "server/host/rank/victory.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/talkadapter.hpp"
#include "server/host/user.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/hosttool.hpp"

server::host::HostGame::HostGame(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

int32_t
server::host::HostGame::createNewGame()
{
    // ex planetscentral/host/cmdgame.h:doNewGame

    // NEWGAME
    GameCreator creator(m_root);
    int32_t id = creator.createNewGame();
    creator.initializeGame(id);
    creator.finishNewGame(id, Preparing, PrivateGame);

    return id;
}

int32_t
server::host::HostGame::cloneGame(int32_t gameId, afl::base::Optional<State> newState)
{
    // ex planetscentral/host/cmdgame.h:doCloneGame

    // Obtain critical access; we cannot clone a game that is being hosted
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    // Obtain type
    Type gameType = game.getType();

    // Clone the game
    GameCreator creator(m_root);
    int32_t newId = creator.createNewGame();
    creator.copyGame(gameId, newId);

    // Finish it
    creator.finishNewGame(newId, newState.orElse(Joining), gameType);

    return newId;
}

void
server::host::HostGame::setType(int32_t gameId, Type type)
{
    // ex planetscentral/host/cmdgame.h:doSetGameType

    // Obtain critical access; we cannot change the type of a game that is being hosted
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    game.setType(type, m_root.getForum(), m_root);
}

void
server::host::HostGame::setState(int32_t gameId, State state)
{
    // ex planetscentral/host/cmdgame.h:doSetGameState

    // Obtain critical access; we cannot change the state of a game that is being hosted
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    // Special handling for states
    if (state == Joining) {
        // reset change flags
        game.configChanged().remove();
        game.endChanged().remove();
        game.scheduleChanged().remove();
    }
    if (state == Finished) {
        // do ranks first so setState sees the ranks for generating history
        rank::checkForcedGameEnd(game);
        rank::computeGameRankings(m_root, game);
    }

    game.setState(state, m_root.getForum(), m_root);
}

void
server::host::HostGame::setOwner(int32_t gameId, String_t user)
{
    // ex planetscentral/host/cmdgame.h:doSetGameOwner

    // Obtain critical access; we cannot change the owner of a game that is being hosted
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    game.setOwner(user, m_root);
}

void
server::host::HostGame::setName(int32_t gameId, String_t name)
{
    // ex planetscentral/host/cmdgame.h:doSetGameName

    // Obtain simple access; changing the name is uncritical
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);

    game.setName(name, m_root.getForum());
}

server::interface::HostGame::Info
server::host::HostGame::getInfo(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doStatGame

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.describe(true, m_session.getUser(), String_t(), m_root);
}

void
server::host::HostGame::getInfos(const Filter& filter, bool verbose, std::vector<Info>& result)
{
    // ex planetscentral/host/cmdgame.h:doListGames
    afl::data::IntegerList_t list;
    listGames(filter, list);
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        result.push_back(Game(m_root, list[i], Game::NoExistanceCheck).describe(verbose, m_session.getUser(), filter.requiredUser.orElse(String_t()), m_root));
    }
}

void
server::host::HostGame::getGames(const Filter& filter, afl::data::IntegerList_t& result)
{
    // ex planetscentral/host/cmdgame.h:doListGames
    listGames(filter, result);
}

void
server::host::HostGame::setConfig(int32_t gameId, const afl::data::StringList_t& keyValues)
{
    // ex planetscentral/host/cmdgame.h:doSetGameConfig

    // Obtain critical access; we cannot change the owner of a game that is being hosted
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ConfigPermission);

    // Validate
    bool cronChange = false;
    bool configChanged = false;
    bool endChanged = false;
    bool configSet = false;
    bool endSet = false;
    for (size_t i = 0, n = keyValues.size(); i+1 < n; i += 2) {
        const String_t& option = keyValues[i];
        const String_t& value = keyValues[i+1];

        // Verify options
        if (option == "host" && !m_root.hostRoot().all().contains(value)) {
            // @change INVALID_VALUE is error 400 in c2ng; was 412 previously.
            throw std::runtime_error(INVALID_VALUE);
        }
        if (option == "master" && !m_root.masterRoot().all().contains(value)) {
            throw std::runtime_error(INVALID_VALUE);
        }
        if (option == "shiplist" && !m_root.shipListRoot().all().contains(value)) {
            throw std::runtime_error(INVALID_VALUE);
        }

        // Options that trigger special behaviour
        if (option == "hostRunNow") {
            cronChange = true;
        }
        if (option == "host" || option == "master" || option == "shiplist") {
            configChanged = true;
        }
        if (option.length() >= 3 && option.compare(0, 3, "end", 3) == 0) {
            endChanged = true;
        }
        if (option == "endChanged") {
            endSet = true;
        }
        if (option == "configChanged") {
            configSet = true;
        }
    }

    // Execute
    for (size_t i = 0, n = keyValues.size(); i+1 < n; i += 2) {
        game.setConfig(keyValues[i], keyValues[i+1]);
    }
    game.clearCache();

    // Set status bits
    if (endChanged && !endSet) {
        game.endChanged().set(1);
    }
    if (configChanged && !configSet) {
        game.configChanged().set(1);
    }

    // Postprocess
    if (cronChange) {
        m_root.handleGameChange(gameId);
    }
}

String_t
server::host::HostGame::getConfig(int32_t gameId, String_t key)
{
    // ex planetscentral/host/cmdgame.h:doGetGameConfig

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.getConfig(key);
}

void
server::host::HostGame::getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values)
{
    // ex planetscentral/host/cmdgame.h:doGetGameConfigMultiple

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Do it
    for (size_t i = 0, n = keys.size(); i < n; ++i) {
        values.push_back(game.getConfig(keys[i]));
    }
}

String_t
server::host::HostGame::getComputedValue(int32_t gameId, String_t key)
{
    // ex planetscentral/host/cmdgame.h:doGetGameCache

    // Obtain simple access; read-only access
    // FIXME: this should actually be Critical access because it will access game data and other
    // to compute the rating if not known. This might cause conflicts in the future (currently, it doesn't).
    // It does not lock at all in classic.
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Do it
    if (key == "difficulty") {
        return afl::string::Format("%d", game.getDifficulty(m_root));
    } else {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }
}

server::interface::HostGame::State
server::host::HostGame::getState(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGetGameProperty

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);
    return game.getState();
}

server::interface::HostGame::Type
server::host::HostGame::getType(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGetGameProperty

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);
    return game.getType();
}

String_t
server::host::HostGame::getOwner(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGetGameProperty

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.getOwner();
}

String_t
server::host::HostGame::getName(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGetGameProperty

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.getName();
}

String_t
server::host::HostGame::getDirectory(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGetGameProperty

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.getDirectory();
}

server::interface::HostGame::Permissions_t
server::host::HostGame::getPermissions(int32_t gameId, String_t userId)
{
    // ex planetscentral/host/cmdgame.h:doCheckGamePermission
    // FIXME: this bypasses pretty much everything; same as in PCC2. Why?

    afl::net::redis::Subtree root(m_root.gameRoot());
    if (!root.intSetKey("all").contains(gameId)) {
        throw std::runtime_error(GAME_NOT_FOUND);
    }

    Permissions_t value;
    afl::net::redis::Subtree game(root.subtree(gameId));

    String_t state = game.stringKey("state").get();
    if (state != "deleted" && state != "preparing") {
        if (game.stringKey("owner").get() == userId) {
            // 1 - owner
            value += UserIsOwner;
        }

        // Player checks
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            afl::data::StringList_t list;
            game.subtree("player").subtree(i).stringListKey("users").getAll(list);
            if (!list.empty()) {
                if (userId == list.back()) {
                    // 4 - current
                    value += UserIsActive;
                }
                if (userId == list.front()) {
                    // 2 - primary
                    value += UserIsPrimary;
                }
                for (size_t i = 1; i+1 < list.size(); ++i) {
                    if (userId == list[i]) {
                        // 8 - inactive replacement
                        value += UserIsInactive;
                        break;
                    }
                }
            }
        }

        if (value.empty() && game.stringKey("type").get() == "public") {
            // You're a member of the general public
            // FIXME: unlisted?
            value += GameIsPublic;
        }
    }

    return value;
}

bool
server::host::HostGame::addTool(int32_t gameId, String_t toolId)
{
    return addRemoveTool(gameId, toolId, true);
}

bool
server::host::HostGame::removeTool(int32_t gameId, String_t toolId)
{
    return addRemoveTool(gameId, toolId, false);
}

void
server::host::HostGame::getTools(int32_t gameId, std::vector<server::interface::HostTool::Info>& result)
{
    // ex planetscentral/host/cmdgame.h:doListGameTools

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    // Operate
    afl::data::StringList_t tools;
    game.tools().getAll(tools);
    for (size_t i = 0, n = tools.size(); i < n; ++i) {
        // FIXME: this DB-to-Info conversion is duplicated in HostTool::getAll()
        const String_t& id = tools[i];
        afl::net::redis::HashKey h(m_root.toolRoot().byName(id));
        result.push_back(server::interface::HostTool::Info(id, h.stringField("description").get(), h.stringField("kind").get(), false));
    }
}

server::interface::HostGame::Totals
server::host::HostGame::getTotals()
{
    // ex planetscentral/host/cmdgame.h:doGameTotals

    afl::net::redis::Subtree pubstate(m_root.gameRoot().subtree("pubstate"));
    return Totals(pubstate.intSetKey("joining").size(),
                  pubstate.intSetKey("running").size(),
                  pubstate.intSetKey("finished").size());
}

server::interface::HostGame::VictoryCondition
server::host::HostGame::getVictoryCondition(int32_t gameId)
{
    // ex planetscentral/host/cmdgame.h:doGameGetVictoryCondition

    // Obtain simple access; read-only access
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Simple);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::ReadPermission);

    return game.describeVictoryCondition(m_root);
}

void
server::host::HostGame::updateGames(const afl::data::IntegerList_t& gameIds)
{
    // ex planetscentral/host/cmdgame.h:doGameUpdate
    m_session.checkAdmin();

    server::interface::BaseClient(m_root.hostFile()).setUserContext(String_t());
    server::interface::FileBaseClient hostFile(m_root.hostFile());

    for (size_t i = 0, n = gameIds.size(); i < n; ++i) {
        // Fetch a game
        const int32_t gameId = gameIds[i];
        Game game(m_root, gameId);

        // Get game specs
        const State gameState = game.getState();
        const Type  gameType  = game.getType();

        // Update this game's forum
        if (gameState == Joining || gameState == Running) {
            if (TalkListener* talk = m_root.getForum()) {
                talk->handleGameStart(game, gameType);
            }
        }

        // Update file history
        importAllFileHistory(hostFile, game);
    }
}

void
server::host::HostGame::listGames(const Filter& filter, afl::data::IntegerList_t& result)
{
    // ex planetscentral/host/cmdgame.h:doListGames [part]

    // FIXME: this is a very straightforward port using the old in-band-signalling strings.
    // Rewrite to take advantage of correct types.
    String_t stateLimit;
    String_t typeLimit;
    String_t forUser;

    if (const State* p = filter.requiredState.get()) {
        stateLimit = formatState(*p);
    }
    if (const Type* p = filter.requiredType.get()) {
        typeLimit = formatType(*p);
    }
    if (const String_t* p = filter.requiredUser.get()) {
        forUser = *p;
    }

    // Permission checks
    bool needPermissionCheck;
    bool needTypeCheck;
    bool needStateCheck;
    if (!m_session.isAdmin()) {
        needPermissionCheck = (stateLimit != "joining" && stateLimit != "running" && stateLimit != "finished")
            || (typeLimit != "public" || typeLimit != "unlisted");
    } else {
        needPermissionCheck = false;
    }

    // Work
    afl::data::IntegerList_t games;
    if (!forUser.empty()) {
        // User's games.
        // Permission check will wield out deleted/preparing games.
        afl::data::StringList_t gameRefs;
        User(m_root, forUser).gameReferenceCounts().getAll(gameRefs);
        needTypeCheck = true;
        needStateCheck = true;
        for (size_t i = 0; i+1 < gameRefs.size(); i += 2) {
            int32_t ref, game;
            if (afl::string::strToInteger(gameRefs[i], game) && afl::string::strToInteger(gameRefs[i+1], ref) && ref > 0) {
                games.push_back(game);
            }
        }
    } else if (stateLimit.empty()) {
        // No state limit given
        m_root.gameRoot().intSetKey("all").getAll(games);
        needTypeCheck = true;
        needStateCheck = false;
    } else {
        // State limit given, so use by-state lists
        needStateCheck = false;
        if (typeLimit == "public") {
            m_root.gameRoot().subtree("pubstate").intSetKey(stateLimit).getAll(games);
            needTypeCheck = false;
        } else {
            m_root.gameRoot().subtree("state").intSetKey(stateLimit).getAll(games);
            needTypeCheck = true;
        }
    }
    if (typeLimit.empty()) {
        needTypeCheck = false;
    }
    if (stateLimit.empty()) {
        needStateCheck = false;
    }

    const String_t* requiredHost     = filter.requiredHost.get();
    const String_t* requiredTool     = filter.requiredTool.get();
    const String_t* requiredShipList = filter.requiredShipList.get();
    const String_t* requiredMaster   = filter.requiredMaster.get();

    for (size_t i = 0; i < games.size(); ++i) {
        Game game(m_root, games[i], Game::NoExistanceCheck);
        if ((!needPermissionCheck || game.hasPermission(m_session.getUser(), Game::ReadPermission))
            && (!needTypeCheck || formatType(game.getType()) == typeLimit)
            && (!needStateCheck || formatState(game.getState()) == stateLimit)
            && (!requiredHost || game.settings().stringField("host").get() == *requiredHost)
            && (!requiredShipList || game.settings().stringField("shiplist").get() == *requiredShipList)
            && (!requiredMaster || game.settings().stringField("master").get() == *requiredMaster)
            && (!requiredTool || game.tools().contains(*requiredTool)))
        {
            result.push_back(games[i]);
        }
    }
}

bool
server::host::HostGame::addRemoveTool(int32_t gameId, String_t toolId, bool add)
{
    // ex planetscentral/host/cmdgame.h:doChangeGameTool

    // Obtain critical access: cannot modify tools while hosting
    GameArbiter::Guard guard(m_root.arbiter(), gameId, GameArbiter::Critical);

    // Check existence and permission
    Game game(m_root, gameId);
    m_session.checkPermission(game, Game::AdminPermission);
    if (!m_root.toolRoot().all().contains(toolId)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Do it
    afl::net::redis::StringSetKey tools = game.tools();
    bool result = add ? tools.add(toolId) : tools.remove(toolId);

    // If this was a change, update tool kinds
    if (result) {
        afl::net::redis::HashKey tool(m_root.toolRoot().byName(toolId));
        String_t kind = tool.stringField("kind").get();
        if (kind.empty()) {
            // Tool has no kind, no conflict check
        } else if (add) {
            // We have added a tool. Check for another tool of the same kind.
            String_t oldTool = game.toolsByKind().stringField(kind).get();
            if (oldTool.size() && oldTool != toolId) {
                tools.remove(oldTool);
                game.toolData(oldTool).hashKey("settings").remove();
            }
            game.toolsByKind().stringField(kind).set(toolId);
        } else {
            // We have removed a tool.
            game.toolsByKind().field(kind).remove();
            game.toolData(toolId).hashKey("settings").remove();
        }
        game.clearCache();
        game.configChanged().set(1);
    }

    return result;
}
