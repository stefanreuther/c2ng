/**
  *  \file server/host/hosthistory.cpp
  *  \brief Class server::host::HostHistory
  */

#include <algorithm>
#include "server/host/hosthistory.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/string/parse.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/host/user.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"

using server::interface::HostGame;
using util::strStartsWith;

namespace {
    /*
     *  Event History
     */

    /* Given an event filter, determine the StringListKey to use.
       \param [in] root Root
       \param [in] filter Filter
       \param [out] needUserFilter true if additional filter by user is needed */
    afl::net::redis::StringListKey findHistoryKey(server::host::Root& root, const server::interface::HostHistory::EventFilter& filter, bool& needUserFilter)
    {
        if (const int32_t* pGame = filter.gameId.get()) {
            // Use game history with additional user filter
            needUserFilter = filter.userId.isValid();
            return server::host::Game(root, *pGame, server::host::Game::NoExistanceCheck).history();
        } else if (const String_t* pUser = filter.userId.get()) {
            // Use user history
            needUserFilter = false;
            return server::host::User(root, *pUser).history();
        } else {
            // Use global history
            needUserFilter = false;
            return root.globalHistory();
        }
    }

    /*
     *  Event Conversion
     *
     *  Events have the form "<time>:<type>:<whatever>". We need to break the <whatever> up into pieces.
     *  Currently used parameters always have the same order, although no event has all of them.
     *  Some events (game-state, specifically) have their final parameters optional.
     *  Thus, our parsing rules are:
     *  - determine set of parameters depending on <type>
     *  - treat all parameters as optional
     */

    const int HasGame  = 1;
    const int HasState = 2;
    const int HasUser  = 4;
    const int HasSlot  = 8;

    int getEventParameters(const String_t& eventType)
    {
        if (strStartsWith(eventType, "game-join")                // or -other
            || strStartsWith(eventType, "game-kick")
            || strStartsWith(eventType, "game-resign")           // or -dead, -other
            || strStartsWith(eventType, "game-subst"))
        {
            return HasGame | HasUser | HasSlot;
        }
        if (strStartsWith(eventType, "game-state")) {
            return HasGame | HasState | HasUser;
        }
        return 0;
    }

    bool convertEvent(server::interface::HostHistory::Event& out, const String_t& in)
    {
        // Parse fixed, mandatory parameters
        util::StringParser p(in);
        if (!p.parseInt(out.time)
            || !p.parseCharacter(':')
            || !p.parseDelim(":", out.eventType))
        {
            return false;
        }

        // Parse variable, optional parameters
        int params = getEventParameters(out.eventType);
        if ((params & HasGame) != 0) {
            int32_t gid = 0;
            if (p.parseCharacter(':') && p.parseInt(gid)) {
                out.gameId = gid;
            }
        }
        if ((params & HasState) != 0) {
            String_t stateStr;
            server::interface::HostGame::State state;
            if (p.parseCharacter(':') && p.parseDelim(":", stateStr) && server::interface::HostGame::parseState(stateStr, state)) {
                out.gameState = state;
            }
        }
        if ((params & HasUser) != 0) {
            String_t userId;
            if (p.parseCharacter(':') && p.parseDelim(":", userId)) {
                out.userId = userId;
            }
        }
        if ((params & HasSlot) != 0) {
            int32_t slot = 0;
            if (p.parseCharacter(':') && p.parseInt(slot)) {
                out.slotNumber = slot;
            }
        }
        return true;
    }

    /*
     *  Turn Conversion
     */

    void packStates(afl::data::IntegerList_t& out, const String_t& in)
    {
        afl::base::ConstBytes_t bytes = afl::string::toBytes(in);
        afl::bits::Value<afl::bits::Int16LE> value;
        while (bytes.fullRead(value.m_bytes)) {
            out.push_back(value);
        }
    }

    void packPlayers(afl::data::StringList_t& out, afl::net::redis::HashKey in)
    {
        const int MAX_PLAYERS = 100;

        afl::data::StringList_t rawValues;
        in.getAll(rawValues);

        for (size_t i = 0, n = rawValues.size(); i+1 < n; i += 2) {
            int playerNr;
            if (afl::string::strToInteger(rawValues[i], playerNr) && playerNr > 0 && playerNr <= MAX_PLAYERS) {
                size_t index = static_cast<size_t>(playerNr - 1);
                while (out.size() <= index) {
                    out.push_back(String_t());
                }
                out[index] = rawValues[i+1];
            }
        }
    }

    void packScores(afl::data::IntegerList_t& out, afl::net::redis::HashKey scores, const afl::data::StringList_t& scoreNames)
    {
        // Fetch all scores at once
        for (size_t i = 0, n = scoreNames.size(); i < n; ++i) {
            String_t thisScore = scores.stringField(scoreNames[i]).get();

            afl::base::ConstBytes_t bytes = afl::string::toBytes(thisScore);
            afl::bits::Value<afl::bits::Int32LE> rawValue;
            size_t index = 0;
            while (bytes.fullRead(rawValue.m_bytes)) {
                // Value is present, make room in output
                while (out.size() <= index) {
                    out.push_back(-1);
                }

                // Merge
                int32_t thisValue = rawValue;
                if (thisValue != -1) {
                    if (out[index] == -1) {
                        out[index] = thisValue;
                    } else {
                        out[index] += thisValue;
                    }
                }
                ++index;
            }
        }
    }
}


server::host::HostHistory::HostHistory(const Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

void
server::host::HostHistory::getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result)
{
    // Game permission checks: if game filter is requested, it must exist and need to have Read access to the game
    if (const int32_t* pGame = filter.gameId.get()) {
        Game g(m_root, *pGame);
        m_session.checkPermission(g, Game::ReadPermission);
    }

    // FIXME: user permission checks? For now, all user history is public as far as this service is concerned

    // Determine action
    bool needUserFilter = false;
    afl::net::redis::StringListKey key = findHistoryKey(m_root, filter, needUserFilter);

    // Read data
    // Newest is at front
    afl::data::StringList_t data;
    if (const int32_t* pLimit = filter.limit.get()) {
        if (*pLimit > 0) {
            key.getRange(0, *pLimit, data);
        }
    } else {
        key.getAll(data);
    }

    // Process
    for (size_t i = 0, n = data.size(); i < n; ++i) {
        Event ev;
        if (convertEvent(ev, data[i])) {
            // Event converted successfully. Does it match the required user Id?
            if (!needUserFilter || (filter.userId.isSame(ev.userId))) {
                // Event matches. Fill in game name if required.
                if (const int32_t* pGame = ev.gameId.get()) {
                    ev.gameName = Game(m_root, *pGame, Game::NoExistanceCheck).getName();
                }

                // Time is in internal format, convert
                ev.time = m_root.config().getUserTimeFromTime(ev.time);

                // Produce output
                result.pushBackNew(new Event(ev));
            }
        }
    }
}

void
server::host::HostHistory::getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result)
{
    // Check permissions
    Game g(m_root, gameId);
    m_session.checkPermission(g, Game::ReadPermission);

    // Check game state. If no data available yet, just return empty list.
    Game::State_t gameState = g.getState();
    if (gameState != HostGame::Running && gameState != HostGame::Finished) {
        return;
    }

    // Check turn number and determine range.
    int32_t endTurn = g.turnNumber().get();
    if (endTurn < 1) {
        return;
    }
    if (const int32_t* pEnd = filter.endTurn.get()) {
        endTurn = std::min(*pEnd, endTurn);
    }

    int32_t numTurns = endTurn;
    if (const int32_t* pNum = filter.limit.get()) {
        numTurns = std::min(*pNum, numTurns);
    }

    // Determine scores
    afl::data::StringList_t scoreNames;
    if (const String_t* pScores = filter.scoreName.get()) {
        String_t::size_type n = 0, p;
        while ((p = pScores->find(',', n)) != String_t::npos) {
            scoreNames.push_back(pScores->substr(n, p-n));
            n = p+1;
        }
        scoreNames.push_back(pScores->substr(n));
    }

    // Time
    int32_t minTime = 1;
    if (const int32_t* p = filter.startTime.get()) {
        minTime = std::max(1, *p);
    }

    // Read turns
    for (int32_t turnNumber = endTurn - numTurns + 1; turnNumber <= endTurn; ++turnNumber) {
        // FIXME: should we make any attempt to filter out nonexistant scores?
        Game::Turn t = g.turn(turnNumber);
        const int32_t turnTime = m_root.config().getUserTimeFromTime(t.info().time().get());

        // This implicitly filters out non-existant turns which have a turnTime of 0.
        if (turnTime >= minTime) {
            std::auto_ptr<Turn> pTurn(new Turn());
            pTurn->turnNumber = turnNumber;
            pTurn->time = turnTime;
            pTurn->timestamp = t.info().timestamp().get();

            if (filter.reportStatus) {
                packStates(pTurn->slotStates, t.info().turnStatus().get());
            }
            if (filter.reportPlayers) {
                packPlayers(pTurn->slotPlayers, t.playerId());
            }
            if (!scoreNames.empty()) {
                packScores(pTurn->slotScores, t.scores(), scoreNames);
            }

            result.pushBackNew(pTurn.release());
        }
    }
}
