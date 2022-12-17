/**
  *  \file server/interface/hostgameclient.cpp
  *  \brief Class server::interface::HostGameClient
  */

#include "server/interface/hostgameclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/except/invaliddataexception.hpp"
#include "server/interface/hostscheduleclient.hpp"
#include "server/interface/hosttoolclient.hpp"

using afl::data::Access;
using afl::data::Segment;

server::interface::HostGameClient::HostGameClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

// NEWGAME
int32_t
server::interface::HostGameClient::createNewGame()
{
    return m_commandHandler.callInt(Segment().pushBackString("NEWGAME"));
}

// CLONEGAME src:GID [state:HostGameState]
int32_t
server::interface::HostGameClient::cloneGame(int32_t gameId, afl::base::Optional<State> newState)
{
    Segment cmd;
    cmd.pushBackString("CLONEGAME");
    cmd.pushBackInteger(gameId);
    if (const State* st = newState.get()) {
        cmd.pushBackString(formatState(*st));
    }
    return m_commandHandler.callInt(cmd);
}

// GAMESETTYPE game:GID type:HostGameType
void
server::interface::HostGameClient::setType(int32_t gameId, Type type)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMESETTYPE").pushBackInteger(gameId).pushBackString(formatType(type)));
}

// GAMESETSTATE game:GID type:HostGameState
void
server::interface::HostGameClient::setState(int32_t gameId, State state)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMESETSTATE").pushBackInteger(gameId).pushBackString(formatState(state)));
}

// GAMESETOWNER game:GID user:UID
void
server::interface::HostGameClient::setOwner(int32_t gameId, String_t user)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMESETOWNER").pushBackInteger(gameId).pushBackString(user));
}

// GAMESETNAME game:GID name:Str
void
server::interface::HostGameClient::setName(int32_t gameId, String_t name)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMESETNAME").pushBackInteger(gameId).pushBackString(name));
}

// GAMESTAT game:GID
server::interface::HostGame::Info
server::interface::HostGameClient::getInfo(int32_t gameId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("GAMESTAT").pushBackInteger(gameId)));
    return unpackInfo(p.get());
}

// GAMELIST [STATE state:HostGameState] [TYPE type:HostGameType] [USER user:UID] [VERBOSE|ID]
void
server::interface::HostGameClient::getInfos(const Filter& filter, bool verbose, std::vector<Info>& result)
{
    Segment cmd;
    buildGameListCommand(cmd, filter);
    if (verbose) {
        cmd.pushBackString("VERBOSE");
    }
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(unpackInfo(a[i].getValue()));
    }
}

void
server::interface::HostGameClient::getGames(const Filter& filter, afl::data::IntegerList_t& result)
{
    Segment cmd;
    buildGameListCommand(cmd, filter);
    cmd.pushBackString("ID");
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access(p).toIntegerList(result);
}

// GAMESET game:GID [key:Str value:Str ...]
void
server::interface::HostGameClient::setConfig(int32_t gameId, const afl::data::StringList_t& keyValues)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMESET").pushBackInteger(gameId).pushBackElements(keyValues));
}

// GAMEGET game:GID key:Str
String_t
server::interface::HostGameClient::getConfig(int32_t gameId, String_t key)
{
    return m_commandHandler.callString(Segment().pushBackString("GAMEGET").pushBackInteger(gameId).pushBackString(key));
}

// GAMEMGET game:GID key:Str...
void
server::interface::HostGameClient::getConfig(int32_t gameId, const afl::data::StringList_t& keys, afl::data::StringList_t& values)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("GAMEMGET").pushBackInteger(gameId).pushBackElements(keys)));
    Access(p).toStringList(values);
}

// GAMEGETCC game:GID key:Str
String_t
server::interface::HostGameClient::getComputedValue(int32_t gameId, String_t key)
{
    return m_commandHandler.callString(Segment().pushBackString("GAMEGETCC").pushBackInteger(gameId).pushBackString(key));
}

// GAMEGETSTATE game:GID
server::interface::HostGame::State
server::interface::HostGameClient::getState(int32_t gameId)
{
    State result;
    if (!parseState(m_commandHandler.callString(Segment().pushBackString("GAMEGETSTATE").pushBackInteger(gameId)), result)) {
        throw afl::except::InvalidDataException("<HostGame.getState>");
    }
    return result;
}

// GAMEGETTYPE game:GID
server::interface::HostGame::Type
server::interface::HostGameClient::getType(int32_t gameId)
{
    Type result;
    if (!parseType(m_commandHandler.callString(Segment().pushBackString("GAMEGETTYPE").pushBackInteger(gameId)), result)) {
        throw afl::except::InvalidDataException("<HostGame.getType>");
    }
    return result;
}

// GAMEGETOWNER game:GID
String_t
server::interface::HostGameClient::getOwner(int32_t gameId)
{
    return m_commandHandler.callString(Segment().pushBackString("GAMEGETOWNER").pushBackInteger(gameId));
}

// GAMEGETNAME game:GID
String_t
server::interface::HostGameClient::getName(int32_t gameId)
{
    return m_commandHandler.callString(Segment().pushBackString("GAMEGETNAME").pushBackInteger(gameId));
}

// GAMEGETDIR game:GID
String_t
server::interface::HostGameClient::getDirectory(int32_t gameId)
{
    return m_commandHandler.callString(Segment().pushBackString("GAMEGETDIR").pushBackInteger(gameId));
}

// GAMECHECKPERM game:GID
server::interface::HostGame::Permissions_t
server::interface::HostGameClient::getPermissions(int32_t gameId, String_t userId)
{
    return Permissions_t::fromInteger(m_commandHandler.callInt(Segment().pushBackString("GAMECHECKPERM").pushBackInteger(gameId).pushBackString(userId)));
}

// GAMEADDTOOL game:GID tool:Str
bool
server::interface::HostGameClient::addTool(int32_t gameId, String_t toolId)
{
    return m_commandHandler.callInt(Segment().pushBackString("GAMEADDTOOL").pushBackInteger(gameId).pushBackString(toolId));
}

// GAMERMTOOL game:GID tool:Str
bool
server::interface::HostGameClient::removeTool(int32_t gameId, String_t toolId)
{
    return m_commandHandler.callInt(Segment().pushBackString("GAMERMTOOL").pushBackInteger(gameId).pushBackString(toolId));
}

// GAMELSTOOLS game:GID
void
server::interface::HostGameClient::getTools(int32_t gameId, std::vector<HostTool::Info>& result)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("GAMELSTOOLS").pushBackInteger(gameId)));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(HostToolClient::unpackInfo(a[i].getValue()));
    }
}

// GAMETOTALS
server::interface::HostGame::Totals
server::interface::HostGameClient::getTotals()
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("GAMETOTALS")));
    Access a(p);
    return Totals(a("joining").toInteger(),
                  a("running").toInteger(),
                  a("finished").toInteger());
}

// GAMEGETVC game:GID
server::interface::HostGame::VictoryCondition
server::interface::HostGameClient::getVictoryCondition(int32_t gameId)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("GAMEGETVC").pushBackInteger(gameId)));
    Access a(p);

    VictoryCondition vc;

    // endCondition
    vc.endCondition = a("endCondition").toString();

    // endTurn [optional]
    vc.endTurn = toOptionalInteger(a("endTurn").getValue());

    // endProbability [optional]
    vc.endProbability = toOptionalInteger(a("endProbability").getValue());

    // endScore [optional]
    vc.endScore = toOptionalInteger(a("endScore").getValue());

    // endScoreName [optional]
    vc.endScoreName = toOptionalString(a("endScoreName").getValue());

    // endScoreDescription [optional]
    vc.endScoreDescription = toOptionalString(a("endScoreDescription").getValue());

    // referee [optional]
    vc.referee = toOptionalString(a("referee").getValue());

    // refereeDescription [optional]
    vc.refereeDescription = toOptionalString(a("refereeDescription").getValue());

    return vc;
}

// GAMEUPDATE game:GID...
void
server::interface::HostGameClient::updateGames(const afl::data::IntegerList_t& gameIds)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMEUPDATE").pushBackElements(gameIds));
}

// GAMERESET game:GID turn:Int
void
server::interface::HostGameClient::resetToTurn(int32_t gameId, int turnNr)
{
    m_commandHandler.callVoid(Segment().pushBackString("GAMERESET").pushBackInteger(gameId).pushBackInteger(turnNr));
}

server::interface::HostGame::Info
server::interface::HostGameClient::unpackInfo(const Value_t* value)
{
    Access a(value);
    Info result;

    // gameId / "id"
    result.gameId = a("id").toInteger();

    // state
    if (!parseState(a("state").toString(), result.state)) {
        throw afl::except::InvalidDataException("<HostGame.unpackInfo: state>");
    }

    // type
    if (!parseType(a("type").toString(), result.type)) {
        throw afl::except::InvalidDataException("<HostGame.unpackInfo: type>");
    }

    // name
    result.name = a("name").toString();

    // description [optional]
    result.description = toOptionalString(a("description").getValue());

    // difficulty
    result.difficulty = a("difficulty").toInteger();

    // currentSchedule
    if (const Value_t* p = a("currentSchedule").getValue()) {
        result.currentSchedule = HostScheduleClient::unpackSchedule(p);
    }

    // slotStates / "slots" [optional]
    if (const Value_t* p = a("slots").getValue()) {
        std::vector<SlotState> states;
        for (size_t i = 0, n = Access(p).getArraySize(); i < n; ++i) {
            SlotState st;
            if (!parseSlotState(Access(p)[i].toString(), st)) {
                throw afl::except::InvalidDataException("<HostGame.unpackInfo: slots>");
            }
            states.push_back(st);
        }
        result.slotStates = states;
    }

    // turnStates / "turns" [optional]
    if (const Value_t* p = a("turns").getValue()) {
        std::vector<int32_t> states;
        for (size_t i = 0, n = Access(p).getArraySize(); i < n; ++i) {
            states.push_back(Access(p)[i].toInteger());
        }
        result.turnStates = states;
    }

    // joinable [optional]
    if (const Value_t* p = a("joinable").getValue()) {
        result.joinable = (Access(p).toInteger() != 0);
    }

    // userPlays [optional]
    if (const Value_t* p = a("userPlays").getValue()) {
        result.userPlays = (Access(p).toInteger() != 0);
    }

    // scores [optional]
    if (const Value_t* p = a("scores").getValue()) {
        std::vector<int32_t> scores;
        for (size_t i = 0, n = Access(p).getArraySize(); i < n; ++i) {
            scores.push_back(Access(p)[i].toInteger());
        }
        result.scores = scores;
    }

    // scoreName [optional]
    result.scoreName = toOptionalString(a("scoreName").getValue());

    // scoreDescription [optional]
    result.scoreDescription = toOptionalString(a("scoreDescription").getValue());

    // hostName / "host"
    result.hostName = a("host").toString();

    // hostDescription
    result.hostDescription = a("hostDescription").toString();

    // hostKind
    result.hostKind = a("hostKind").toString();

    // shipListName / "shiplist"
    result.shipListName = a("shiplist").toString();

    // shipListDescription / "shiplistDescription"
    result.shipListDescription = a("shiplistDescription").toString();

    // shipListKind
    result.shipListKind = a("shiplistKind").toString();

    // masterName / "master" [optional]
    result.masterName = toOptionalString(a("master").getValue());

    // masterDescription [optional]
    result.masterDescription = toOptionalString(a("masterDescription").getValue());

    // masterKind [optional]
    result.masterKind = toOptionalString(a("masterKind").getValue());

    // turnNumber / "turn"
    result.turnNumber = a("turn").toInteger();

    // lastHostTime [optional]
    result.lastHostTime = toOptionalInteger(a("lastHostTime").getValue());

    // nextHostTime [optional]
    result.nextHostTime = toOptionalInteger(a("nextHostTime").getValue());

    // forumId / "forum" [optional]
    result.forumId = toOptionalInteger(a("forum").getValue());

    // userRank
    result.userRank = toOptionalInteger(a("userRank").getValue());

    // otherRank
    result.otherRank = toOptionalInteger(a("otherRank").getValue());

    return result;
}

void
server::interface::HostGameClient::buildGameListCommand(afl::data::Segment& cmd, const Filter& filter)
{
    cmd.pushBackString("GAMELIST");
    if (const State* st = filter.requiredState.get()) {
        cmd.pushBackString("STATE");
        cmd.pushBackString(formatState(*st));
    }
    if (const Type* ty = filter.requiredType.get()) {
        cmd.pushBackString("TYPE");
        cmd.pushBackString(formatType(*ty));
    }
    if (const String_t* u = filter.requiredUser.get()) {
        cmd.pushBackString("USER");
        cmd.pushBackString(*u);
    }
    if (const String_t* s = filter.requiredHost.get()) {
        cmd.pushBackString("HOST");
        cmd.pushBackString(*s);
    }
    if (const String_t* s = filter.requiredTool.get()) {
        cmd.pushBackString("TOOL");
        cmd.pushBackString(*s);
    }
    if (const String_t* s = filter.requiredShipList.get()) {
        cmd.pushBackString("SHIPLIST");
        cmd.pushBackString(*s);
    }
    if (const String_t* s = filter.requiredMaster.get()) {
        cmd.pushBackString("MASTER");
        cmd.pushBackString(*s);
    }
    if (const int32_t* i = filter.requiredCopyOf.get()) {
        cmd.pushBackString("COPYOF");
        cmd.pushBackInteger(*i);
    }
}
