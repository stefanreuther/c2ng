/**
  *  \file server/interface/hostgameserver.cpp
  */

#include <stdexcept>
#include "server/interface/hostgameserver.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/interface/hostscheduleserver.hpp"
#include "afl/base/staticassert.hpp"
#include "server/interface/hosttoolserver.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;

server::interface::HostGameServer::HostGameServer(HostGame& impl)
    : ComposableCommandHandler(),
      m_implementation(impl)
{ }

server::interface::HostGameServer::~HostGameServer()
{ }

bool
server::interface::HostGameServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "NEWGAME") {
        /* @q NEWGAME (Host Command)
           Create new game.
           The game is created in status "preparing", type "private".
           Use other commands to configure it.

           Permissions: ?

           @retval GID new game Id
           @uses game:state:$STATE, game:all, game:lastid, game:$GID:name, game:$GID:settings
           @change PCC2 accepted and ignored parameters */
        args.checkArgumentCount(0);
        result.reset(makeIntegerValue(m_implementation.createNewGame()));
        return true;
    } else if (upcasedCommand == "CLONEGAME") {
        /* @q CLONEGAME src:GID [state:HostGameState] (Host Command)
           Clone a game.
           The new game is created in state "joining" if no initial state is given.

           Permissions: admin for game.

           @retval GID new game Id */
        args.checkArgumentCount(1, 2);
        int32_t gameId = toInteger(args.getNext());
        afl::base::Optional<HostGame::State> newState;
        if (args.getNumArgs() > 0) {
            HostGame::State st;
            if (!HostGame::parseState(toString(args.getNext()), st)) {
                throw std::runtime_error(INVALID_VALUE);
            }
            newState = st;
        }
        result.reset(makeIntegerValue(m_implementation.cloneGame(gameId, newState)));
        return true;
    } else if (upcasedCommand == "GAMESETTYPE") {
        /* @q GAMESETTYPE game:GID type:HostGameType (Host Command)
           Set game type.

           Permissions: admin for game.

           @uses game:$GID:type, game:pubstate:$STATE */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        HostGame::Type type;
        if (!HostGame::parseType(toString(args.getNext()), type)) {
            throw std::runtime_error(INVALID_VALUE);
        }
        m_implementation.setType(gameId, type);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GAMESETSTATE") {
        /* @q GAMESETSTATE game:GID type:HostGameState (Host Command)
           Set game state.
           This will possibly start or stop the scheduler.

           Permissions: admin for game.

           @uses game:pubstate:$STATE, game:state:$STATE, game:$GID:state */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        HostGame::State state;
        if (!HostGame::parseState(toString(args.getNext()), state)) {
            throw std::runtime_error(INVALID_VALUE);
        }
        m_implementation.setState(gameId, state);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GAMESETOWNER") {
        /* @q GAMESETOWNER game:GID user:UID (Host Command)
           Set game owner.

           Permissions: admin for game.

           @uses game:$GID:owner, user:$UID:ownedGames */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t user  = toString(args.getNext());
        m_implementation.setOwner(gameId, user);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GAMESETNAME") {
        /* @q GAMESETNAME game:GID name:Str (Host Command)
           Set game name.

           Permissions: admin for game.

           @uses game:$GID:name */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t name  = toString(args.getNext());
        m_implementation.setName(gameId, name);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GAMESTAT") {
        /* @q GAMESTAT game:GID (Host Command)
           Get information about one game.

           Permissions: read-access to game.

           @retval HostGameInfo Verbose information */
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());

        HostGame::Info i = m_implementation.getInfo(gameId);
        result.reset(packInfo(i));
        return true;
    } else if (upcasedCommand == "GAMELIST") {
        /* @q GAMELIST [STATE state:HostGameState] [TYPE type:HostGameType] [USER user:UID] [...] [VERBOSE|ID] (Host Command)
           List games.
           Optional parameters %state, %type, %user limit output to games of that state, type, or played by that user.
           Additional filter clauses:
           - HOST id:Str (only games using this host version, since 2.40.9)
           - MASTER id:Str (only games using this master version, since 2.40.9)
           - SHIPLIST id:Str (only games using this ship list, since 2.40.9)
           - TOOL id:Str (only games using this tool, since 2.40.9)

           The command returns a list of {@type HostGameInfo} (normal) by default.
           %VERBOSE returns the verbose variant, %ID returns just a list of {@type GID|game Ids}.

           Permissions: returns only games for which user has read access.

           @rettype HostGameInfo
           @rettype IntList
           @rettype GID
           @uses game:state:$STATE, game:pubstate:$STATE, game:all, user:$UID:ownedGames */
        HostGame::Filter filter;
        enum { Normal, Verbose, Ids } mode = Normal;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "STATE") {
                HostGame::State st;
                args.checkArgumentCountAtLeast(1);
                if (!HostGame::parseState(toString(args.getNext()), st)) {
                    throw std::runtime_error(INVALID_VALUE);
                }
                filter.requiredState = st;
            } else if (keyword == "TYPE") {
                HostGame::Type ty;
                args.checkArgumentCountAtLeast(1);
                if (!HostGame::parseType(toString(args.getNext()), ty)) {
                    throw std::runtime_error(INVALID_VALUE);
                }
                filter.requiredType = ty;
            } else if (keyword == "USER") {
                args.checkArgumentCountAtLeast(1);
                filter.requiredUser = toString(args.getNext());
            } else if (keyword == "HOST") {
                args.checkArgumentCountAtLeast(1);
                filter.requiredHost = toString(args.getNext());
            } else if (keyword == "TOOL") {
                args.checkArgumentCountAtLeast(1);
                filter.requiredTool = toString(args.getNext());
            } else if (keyword == "SHIPLIST") {
                args.checkArgumentCountAtLeast(1);
                filter.requiredShipList = toString(args.getNext());
            } else if (keyword == "MASTER") {
                args.checkArgumentCountAtLeast(1);
                filter.requiredMaster = toString(args.getNext());
            } else if (keyword == "VERBOSE") {
                mode = Verbose;
            } else if (keyword == "ID") {
                mode = Ids;
            } else {
                throw std::runtime_error(SYNTAX_ERROR);
            }
        }

        Vector::Ref_t v = Vector::create();
        if (mode == Ids) {
            // GAMELIST...ID
            afl::data::IntegerList_t result;
            m_implementation.getGames(filter, result);
            v->pushBackElements(result);
        } else {
            // GAMELIST..., GAMELIST...VERBOSE
            std::vector<HostGame::Info> result;
            m_implementation.getInfos(filter, (mode == Verbose), result);
            for (size_t i = 0, n = result.size(); i < n; ++i) {
                v->pushBackNew(packInfo(result[i]));
            }
        }
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "GAMESET") {
        /* @q GAMESET game:GID [key:Str value:Str ...] (Host Command)
           Set game properties.
           Any number of properties can be set at once by listing multiple key/value pairs.
           See {game:$GID:settings} for possible keys.

           Permissions: config-access to game.

           @err 412 Value not permitted (the specified value is not valid for this key)

           @uses game:$GID:settings */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());

        afl::data::StringList_t keyValues;
        while (args.getNumArgs() > 0) {
            keyValues.push_back(toString(args.getNext()));
        }
        m_implementation.setConfig(gameId, keyValues);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "GAMEGET") {
        /* @q GAMEGET game:GID key:Str (Host Command)
           Get game property.
           See {game:$GID:settings} for possible keys.

           Permissions: read-access to game.

           @retval Str property
           @uses game:$GID:settings
           @see GAMESET, GAMEMGET */
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t key = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.getConfig(gameId, key)));
        return true;
    } else if (upcasedCommand == "GAMEMGET") {
        /* @q GAMEMGET game:GID key:Str... (Host Command)
           Get game properties.
           Like {GAMEGET}, but returns multiple values at once as an array.

           Permissions: read-access to game.

           @retval StrList values
           @uses game:$GID:settings
           @see GAMESET, GAMEGET */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        afl::data::StringList_t fields;
        while (args.getNumArgs() > 0) {
            fields.push_back(toString(args.getNext()));
        }

        afl::data::StringList_t values;
        m_implementation.getConfig(gameId, fields, values);

        Vector::Ref_t v = Vector::create();
        v->pushBackElements(values);
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "GAMEGETCC") {
        /* @q GAMEGETCC game:GID key:Str (Host Command)
           Get computed/cached value.
           The value is fetched from the cache; if it is not yet cached, it is computed
           Value keys:
           - %difficulty

           Permissions: read-access to game.

           @uses game:$GID:cache */
        // @change PCC2 accepts and ignores >2 args
        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t key = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.getComputedValue(gameId, key)));
        return true;
    } else if (upcasedCommand == "GAMEGETSTATE") {
        /* @q GAMEGETSTATE game:GID (Host Command)
           Get game state.

           Permissions: read-access to game.

           @uses game:$GID:state
           @retval HostGameState */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(HostGame::formatState(m_implementation.getState(toInteger(args.getNext())))));
        return true;
    } else if (upcasedCommand == "GAMEGETTYPE") {
        /* @q GAMEGETTYPE game:GID (Host Command)
           Get game type.

           Permissions: read-access to game.

           @uses game:$GID:type
           @retval HostGameType */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(HostGame::formatType(m_implementation.getType(toInteger(args.getNext())))));
        return true;
    } else if (upcasedCommand == "GAMEGETOWNER") {
        /* @q GAMEGETOWNER game:GID (Host Command)
           Get game owner.

           Permissions: read-access to game.

           @uses game:$GID:owner
           @retval UID */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getOwner(toInteger(args.getNext()))));
        return true;
    } else if (upcasedCommand == "GAMEGETNAME") {
        /* @q GAMEGETNAME game:GID (Host Command)
           Get game name.

           Permissions: read-access to game.

           @uses game:$GID:name
           @retval Str */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getName(toInteger(args.getNext()))));
        return true;
    } else if (upcasedCommand == "GAMEGETDIR") {
        /* @q GAMEGETDIR game:GID (Host Command)
           Get game directory in host filer.

           Permissions: read-access to game.

           @uses game:$GID:dir
           @retval FileName */
        args.checkArgumentCount(1);
        result.reset(makeStringValue(m_implementation.getDirectory(toInteger(args.getNext()))));
        return true;
    } else if (upcasedCommand == "GAMECHECKPERM") {
        /* @q GAMECHECKPERM game:GID user:UID (Host Command)
           Get game access permissions.

           Permissions: none.

           @err 404 Game does not exist
           @retval Int Sum of permissions the current user has on this game
           - 1 (user is owner)
           - 2 (user is primary player in this game)
           - 4 (user is active (in-charge) replacement player in this game)
           - 8 (user is inactive replacement in this game)
           - 16 (game is public) */
        static_assert(int(HostGame::UserIsOwner)    == 0, "Permission representation");
        static_assert(int(HostGame::UserIsPrimary)  == 1, "Permission representation");
        static_assert(int(HostGame::UserIsActive)   == 2, "Permission representation");
        static_assert(int(HostGame::UserIsInactive) == 3, "Permission representation");
        static_assert(int(HostGame::GameIsPublic)   == 4, "Permission representation");

        args.checkArgumentCount(2);
        int32_t gameId = toInteger(args.getNext());
        String_t userId = toString(args.getNext());

        result.reset(makeIntegerValue(m_implementation.getPermissions(gameId, userId).toInteger()));
        return true;
    } else if (upcasedCommand == "GAMEADDTOOL") {
        /* @q GAMEADDTOOL game:GID tool:Str (Host Command)
           Add a tool.
           If another tool of the same type already is on the game, remove that first.

           Permissions: admin for game.

           @err 404 Invalid tool
           @uses game:$GID:tools, game:$GID:toolkind, game:$GID:tool:$TOOL:settings
           @retval Int 1=tool changed, 0=no change */
        args.checkArgumentCount(2);
        int32_t gameId  = toInteger(args.getNext());
        String_t toolId = toString(args.getNext());

        result.reset(makeIntegerValue(m_implementation.addTool(gameId, toolId)));
        return true;
    } else if (upcasedCommand == "GAMERMTOOL") {
        /* @q GAMERMTOOL game:GID tool:Str (Host Command)
           Add a tool.

           Permissions: admin for game.

           @err 404 Invalid tool
           @uses game:$GID:tools, game:$GID:toolkind, game:$GID:tool:$TOOL:settings
           @retval Int 1=tool changed, 0=no change */
        args.checkArgumentCount(2);
        int32_t gameId  = toInteger(args.getNext());
        String_t toolId = toString(args.getNext());

        result.reset(makeIntegerValue(m_implementation.removeTool(gameId, toolId)));
        return true;
    } else if (upcasedCommand == "GAMELSTOOLS") {
        /* @q GAMELSTOOLS game:GID (Host Command)
           List game tools (addons).

           Permissions: read-access to game.

           @retval HostToolInfo[]
           @uses game:$GID:tools, prog:tool:prog:$TOOL */
        // Parse args
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());

        std::vector<HostTool::Info> tools;
        m_implementation.getTools(gameId, tools);

        Vector::Ref_t v = Vector::create();
        for (size_t i = 0, n = tools.size(); i < n; ++i) {
            v->pushBackNew(HostToolServer::packTool(tools[i]));
        }
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "GAMETOTALS") {
        /* @q GAMETOTALS (Host Command)
           Get host statistics.

           Permissions: none.

           @retkey joining:Int Number of joinable public games
           @retkey running:Int Number of running public games
           @retkey finished:Int Number of finished public games
           @uses game:pubstate:$STATE */
        args.checkArgumentCount(0);

        HostGame::Totals t = m_implementation.getTotals();

        Hash::Ref_t h = Hash::create();
        h->setNew("joining",  makeIntegerValue(t.numJoiningGames));
        h->setNew("running",  makeIntegerValue(t.numRunningGames));
        h->setNew("finished", makeIntegerValue(t.numFinishedGames));
        result.reset(new HashValue(h));
        return true;
    } else if (upcasedCommand == "GAMEGETVC") {
        /* @q GAMEGETVC game:GID (Host Command)
           Get victory condition.

           Permissions: read-access to game.

           @retkey endCondition:Str (end condition type, "turn", "score", or none)
           @retkey endTurn:Int (for "turn" condition, ending turn; for "score" condition, number of turns to hold a score)
           @retkey endProbability:Int (for "turn" condition, probability that game ends at that turn)
           @retkey endScore:Int (for "score" condition: score to reach and hold)
           @retkey endScoreName:Str (for "score" condition: name of score)
           @retkey endScoreDescription:Str (for "score" condition: description of score)
           @retkey referee:Str (for no condition: name of referee add-on)
           @retkey refereeDescription:Str (for no condition: description of referee add-on, see {prog:tool:prog:$TOOL}->description)
           @uses game:$GID:settings */
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());

        result.reset(packVictoryCondition(m_implementation.getVictoryCondition(gameId)));
        return true;
    } else if (upcasedCommand == "GAMEUPDATE") {
        /* @q GAMEUPDATE game:GID... (Host Command)
           Ad-hoc, admin-only command to update a game to the latest data formats.

           Permissions: admin. */
        afl::data::IntegerList_t gameIds;
        while (args.getNumArgs() > 0) {
            gameIds.push_back(toInteger(args.getNext()));
        }
        m_implementation.updateGames(gameIds);
        result.reset(makeStringValue("OK"));
        return true;
    } else {
        return false;
    }
}

server::Value_t*
server::interface::HostGameServer::packInfo(const HostGame::Info& info)
{
    // ex Game::describe [part]
    Hash::Ref_t h = Hash::create();

    // id
    h->setNew("id", makeIntegerValue(info.gameId));

    // state
    h->setNew("state", makeStringValue(HostGame::formatState(info.state)));

    // type
    h->setNew("type", makeStringValue(HostGame::formatType(info.type)));

    // name
    h->setNew("name", makeStringValue(info.name));

    // description
    if (const String_t* p = info.description.get()) {
        h->setNew("description", makeStringValue(*p));
    }

    // difficulty
    h->setNew("difficulty", makeIntegerValue(info.difficulty));

    // currentSchedule
    if (const HostSchedule::Schedule* p = info.currentSchedule.get()) {
        h->setNew("currentSchedule", HostScheduleServer::packSchedule(*p));
    }

    // slotStates
    if (const std::vector<HostGame::SlotState>* p = info.slotStates.get()) {
        Vector::Ref_t ss = Vector::create();
        for (size_t i = 0, n = p->size(); i < n; ++i) {
            ss->pushBackString(HostGame::formatSlotState((*p)[i]));
        }
        h->setNew("slots", new VectorValue(ss));
    }

    // turnStates
    if (const std::vector<int32_t>* p = info.turnStates.get()) {
        Vector::Ref_t ts = Vector::create();
        ts->pushBackElements(*p);
        h->setNew("turns", new VectorValue(ts));
    }

    // joinable
    if (const bool* p = info.joinable.get()) {
        h->setNew("joinable", makeIntegerValue(*p));
    }

    // scores
    if (const std::vector<int32_t>* p = info.scores.get()) {
        Vector::Ref_t scores = Vector::create();
        scores->pushBackElements(*p);
        h->setNew("scores", new VectorValue(scores));
    }

    // scoreName
    if (const String_t* p = info.scoreName.get()) {
        h->setNew("scoreName", makeStringValue(*p));
    }

    // scoreDescription
    if (const String_t* p = info.scoreDescription.get()) {
        h->setNew("scoreDescription", makeStringValue(*p));
    }

    // hostName
    h->setNew("host", makeStringValue(info.hostName));

    // hostDescription
    h->setNew("hostDescription", makeStringValue(info.hostDescription));

    // shipListName
    h->setNew("shiplist", makeStringValue(info.shipListName));

    // shipListDescription
    h->setNew("shiplistDescription", makeStringValue(info.shipListDescription));

    // masterName
    if (const String_t* p = info.masterName.get()) {
        h->setNew("master", makeStringValue(*p));
    }

    // masterDescription
    if (const String_t* p = info.masterDescription.get()) {
        h->setNew("masterDescription", makeStringValue(*p));
    }

    // turnNumber
    h->setNew("turn", makeIntegerValue(info.turnNumber));

    // lastHostTime
    if (const int32_t* p = info.lastHostTime.get()) {
        h->setNew("lastHostTime", makeIntegerValue(*p));
    }

    // nextHostTime
    if (const int32_t* p = info.nextHostTime.get()) {
        h->setNew("nextHostTime", makeIntegerValue(*p));
    }

    // forumId
    if (const int32_t* p = info.forumId.get()) {
        h->setNew("forum", makeIntegerValue(*p));
    }

    return new HashValue(h);
}

server::Value_t*
server::interface::HostGameServer::packVictoryCondition(const HostGame::VictoryCondition& vc)
{
    Hash::Ref_t h = Hash::create();

    // endCondition
    h->setNew("endCondition", makeStringValue(vc.endCondition));

    // endTurn
    if (const int32_t* p = vc.endTurn.get()) {
        h->setNew("endTurn", makeIntegerValue(*p));
    }

    // endProbability
    if (const int32_t* p = vc.endProbability.get()) {
        h->setNew("endProbability", makeIntegerValue(*p));
    }

    // endScore
    if (const int32_t* p = vc.endScore.get()) {
        h->setNew("endScore", makeIntegerValue(*p));
    }

    // endScoreName
    if (const String_t* p = vc.endScoreName.get()) {
        h->setNew("endScoreName", makeStringValue(*p));
    }

    // endScoreDescription
    if (const String_t* p = vc.endScoreDescription.get()) {
        h->setNew("endScoreDescription", makeStringValue(*p));
    }

    // referee
    if (const String_t* p = vc.referee.get()) {
        h->setNew("referee", makeStringValue(*p));
    }

    // refereeDescription
    if (const String_t* p = vc.refereeDescription.get()) {
        h->setNew("refereeDescription", makeStringValue(*p));
    }

    return new HashValue(h);
}
